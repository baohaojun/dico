/* This file is part of GNU Dico.
   Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff

   GNU Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <appi18n.h>
#include <wordsplit.h>

#include <errno.h>
#include <ldap.h>
#include <lber.h>

struct _dico_ldap_handle {
    LDAP *ldap;
    const char *url;
    char *base;
    char *binddn;
    char *passwd;
    int tls;
    /* FIXME: sasl */
    int debug;
    char *user_filter;
    char *group_filter;
};

void
free_ldap_handle(struct _dico_ldap_handle *lp)
{
    free(lp->base);
    free(lp->binddn);
    free(lp->passwd);
    free(lp->user_filter);
    free(lp->group_filter);
    free(lp);
}

static int
_dico_conn_setup(struct _dico_ldap_handle *lp)
{
    int rc;
    LDAP *ld = NULL;
    int protocol = LDAP_VERSION3; /* FIXME: must be configurable */
  
    if (lp->debug) {
	if (ber_set_option(NULL, LBER_OPT_DEBUG_LEVEL, &lp->debug)
	    != LBER_OPT_SUCCESS )
	    dico_log(L_ERR, 0, _("cannot set LBER_OPT_DEBUG_LEVEL %d"),
		     lp->debug);

	if (ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, &lp->debug)
	    != LDAP_OPT_SUCCESS )
	    dico_log(L_ERR, 0, _("could not set LDAP_OPT_DEBUG_LEVEL %d"),
		     lp->debug);
    }


    rc = ldap_initialize(&ld, lp->url);
    if (rc != LDAP_SUCCESS) {
	dico_log(L_ERR, 0,
		 _("cannot create LDAP session handle for URI=%s (%d): %s"),
		 lp->url, rc, ldap_err2string(rc));
	return 1;
    }
  
    if (lp->tls) {
	rc = ldap_start_tls_s(ld, NULL, NULL);
	if (rc != LDAP_SUCCESS) {
	    dico_log(L_ERR, 0, _("ldap_start_tls failed: %s"),
		     ldap_err2string(rc));
	    return 1;
	}
    }

    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &protocol);

    /* FIXME: Timeouts, SASL, etc. */
    lp->ldap = ld;
    return 0;
}

static int
_dico_ldap_bind(struct _dico_ldap_handle *lp)
{
    int msgid, err, rc;
    LDAPMessage *result;
    LDAPControl **ctrls;
    char msgbuf[256];
    char *matched = NULL;
    char *info = NULL;
    char **refs = NULL;
    static struct berval passwd;

    passwd.bv_val = lp->passwd;
    passwd.bv_len = passwd.bv_val ? strlen (passwd.bv_val) : 0;

    msgbuf[0] = 0;
    
    rc = ldap_sasl_bind(lp->ldap, lp->binddn, LDAP_SASL_SIMPLE, &passwd,
			NULL, NULL, &msgid);
    if (msgid == -1) {
	dico_log(L_ERR, 0,
		 "ldap_sasl_bind(SIMPLE) failed: %s", ldap_err2string(rc));
	return 1;
    }

    if (ldap_result(lp->ldap, msgid, LDAP_MSG_ALL, NULL, &result) == -1) {
	dico_log(L_ERR, 0, "ldap_result failed");
	return 1;
    }

    rc = ldap_parse_result(lp->ldap, result, &err, &matched, &info, &refs,
			   &ctrls, 1);
    if (rc != LDAP_SUCCESS) {
	dico_log(L_ERR, 0, "ldap_parse_result failed: %s",
		 ldap_err2string(rc));
	return 1;
    }

    if (ctrls)
	ldap_controls_free(ctrls);

    if (err != LDAP_SUCCESS
	|| msgbuf[0]
	|| (matched && matched[0])
	|| (info && info[0])
	|| refs) {
	dico_log(L_ERR, 0, "ldap_bind: %s (%d)%s",
		 ldap_err2string(err), err, msgbuf);

	if (matched && *matched) 
	    dico_log(L_ERR, 0, "matched DN: %s", matched);
	
	if (info && *info)
	    dico_log(L_ERR, 0, "additional info: %s", info);
	
	if (refs && *refs) {
	    int i;
	    dico_log(L_ERR, 0, "referrals:");
	    for (i = 0; refs[i]; i++) 
		dico_log(L_ERR, 0, "%s", refs[i]);
	}
    }

    if (matched)
        ber_memfree(matched);
    if (info)
	ber_memfree(info);
    if (refs)
	ber_memvfree((void **)refs);

    return err != LDAP_SUCCESS;
}

static void
_dico_ldap_unbind(struct _dico_ldap_handle *lp)
{
    if (lp->ldap) {
	ldap_set_option(lp->ldap, LDAP_OPT_SERVER_CONTROLS, NULL);
	ldap_unbind_ext(lp->ldap, NULL, NULL);
    }
}

static int
db_open(void **phandle, dico_url_t url, const char *options)
{
    struct _dico_ldap_handle hstr, *handle;
    int rc;
    long debug = 0;
    
    struct dico_option option[] = {
	{ DICO_OPTSTR(base), dico_opt_string, &hstr.base },
	{ DICO_OPTSTR(binddn), dico_opt_string, &hstr.binddn },
	{ DICO_OPTSTR(passwd), dico_opt_string, &hstr.passwd },
	{ DICO_OPTSTR(tls), dico_opt_bool, &hstr.tls },
	{ DICO_OPTSTR(debug), dico_opt_long, &debug },
	{ DICO_OPTSTR(user-filter), dico_opt_string, &hstr.user_filter },
	{ DICO_OPTSTR(group-filter), dico_opt_string, &hstr.group_filter },
	{ NULL }
    };

    memset(&hstr, 0, sizeof(hstr));
    if (options) {
	struct wordsplit ws;

	if (wordsplit(options, &ws, WRDSF_DEFFLAGS)) {
	    dico_log(L_ERR, 0, _("cannot parse options `%s': %s"), options,
		     wordsplit_strerror(&ws));
	    return 1;
	}

	rc = dico_parseopt(option, ws.ws_wordc, ws.ws_wordv,
			   DICO_PARSEOPT_PARSE_ARGV0, NULL);
	wordsplit_free(&ws);
	if (rc)
	    return 1;
	hstr.debug = debug;
    }
    hstr.url = url->string;
    
    handle = malloc(sizeof(*handle));
    if (!handle) {
	dico_log(L_ERR, errno, _("cannot allocate handle"));
	return 1;
    }
    *handle = hstr;
    
    rc = _dico_conn_setup(handle) ||
	 _dico_ldap_bind(handle);
    if (rc) {
	free_ldap_handle(handle);
    } else {
	*phandle = handle;
    }
    return rc;
}

int
db_close(void *handle)
{
    struct _dico_ldap_handle *lp = handle;
    _dico_ldap_unbind(lp);
    free_ldap_handle(lp);
    return 0;
}


char *
_dico_ldap_expand_user(const char *query, const char *user)
{
    struct wordsplit ws;
    const char *env[3];
    char *res;
    
    env[0] = "user";
    env[1] = user;
    env[2] = NULL;

    ws.ws_env = env;
    if (wordsplit(query, &ws,
		  WRDSF_NOSPLIT | WRDSF_NOCMD |
		  WRDSF_ENV | WRDSF_ENV_KV)) {
	dico_log(L_ERR, 0, _("cannot expand query `%s': %s"), query,
		 wordsplit_strerror(&ws));
	return NULL;
    }

    res = ws.ws_wordv[0];
    ws.ws_wordv[0] = NULL;
    wordsplit_free(&ws);
    return res;
}    

static LDAPMessage *
_dico_ldap_search(struct _dico_ldap_handle *lp, const char *filter_pat,
		  const char *attr, const char *user)
{
    int rc;
    char *filter;
    LDAPMessage *res;
    ber_int_t msgid;
    char *attrs[2];

    attrs[0] = (char*) attr;
    attrs[1] = NULL;

    if (filter_pat) {
	filter = _dico_ldap_expand_user(filter_pat, user);
	if (!filter) {
	    dico_log(L_ERR, ENOMEM, "_dico_ldap_expand_user");
	    return NULL;
	}
    } else
	filter = NULL;
    rc = ldap_search_ext(lp->ldap, lp->base, LDAP_SCOPE_SUBTREE,
			 filter, attrs, 0,
			 NULL, NULL, NULL, -1, &msgid);
    if (filter)
	free(filter);

    if (rc != LDAP_SUCCESS) {
	dico_log(L_ERR, 0, "ldap_search_ext: %s", ldap_err2string(rc));
	/*if (rc == LDAP_NO_SUCH_OBJECT)*/
	return NULL;
    }

    rc = ldap_result(lp->ldap, msgid, LDAP_MSG_ALL, NULL, &res);
    if (rc < 0) {
	dico_log(L_ERR, 0, "ldap_result failed");
	return NULL;
    }
    return res;
}

static int
db_get_pass(void *handle, const char *qpw, const char *key, char **ppass)
{
    struct _dico_ldap_handle *lp = handle;
    LDAPMessage *res, *msg;
    int rc;
    struct berval **values;
    
    res = _dico_ldap_search(lp, lp->user_filter, qpw, key);
    if (!res)
	return 1;

    rc = ldap_count_entries(lp->ldap, res);
    if (rc == 0) {
	dico_log(L_ERR, 0, "not enough entires");
	ldap_msgfree(res);
	return 1;
    }

    msg = ldap_first_entry(lp->ldap, res);
    
    values = ldap_get_values_len(lp->ldap, msg, qpw);
    if (ldap_count_values_len(values) == 0) {
	dico_log(L_ERR, 0, "not enough entires");
	ldap_msgfree(res);
	return 1;
    }
    *ppass = strdup(values[0]->bv_val);
    rc = *ppass == NULL;
    if (rc)
	dico_log(L_ERR, 0, "not enough memory");
    ldap_value_free_len(values);
    ldap_msgfree(res);
    return rc;
}

static int
_free_group(void *item, void *data)
{
    free(item);
    return 0;
}

static int
db_get_groups(void *handle, const char *qgr, const char *key,
	      dico_list_t *pgroups)
{
    struct _dico_ldap_handle *lp = handle;
    LDAPMessage *res, *msg;
    int rc;
    dico_list_t groups = NULL;
    
    res = _dico_ldap_search(lp, lp->group_filter, qgr, key);
    if (!res)
	return 1;

    rc = ldap_count_entries(lp->ldap, res);
    if (rc == 0) {
	dico_log(L_INFO, 0, "no groups containing %s", key);
	ldap_msgfree(res);
	*pgroups = NULL;
	return 0;
    }

    groups = dico_list_create();
    dico_list_set_free_item(groups, _free_group, NULL);
    *pgroups = groups;

    rc = 0;
    for (msg = ldap_first_entry(lp->ldap, res); msg;
	 msg = ldap_next_entry(lp->ldap, msg)) {
	struct berval **values;
	size_t i, count;
	
	values = ldap_get_values_len(lp->ldap, msg, qgr);
	count = ldap_count_values_len(values);
	for (i = 0; i < count; i++) {
	    char *s = strdup(values[i]->bv_val);
	    dico_list_append(groups, s);
	}
	ldap_value_free_len(values);
    }
    ldap_msgfree(res);
    return rc;
}

static struct dico_udb_def ldap_udb_def = {
    "ldap",
    db_open,
    db_close,
    db_get_pass,
    db_get_groups
};


static int
dico_ldap_init(int argc, char **argv)
{
    return dico_udb_define(&ldap_udb_def);
}


struct dico_database_module DICO_EXPORT(ldap, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NODB,
    dico_ldap_init,
};
