/* This file is part of GNU Dico.
   Copyright (C) 1998-2000, 2008, 2010, 2012 Sergey Poznyakoff

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

#include <dicod.h>

int sasl_enable = 1;
dico_list_t sasl_enabled_mech;
dico_list_t sasl_disabled_mech;
char *sasl_service;
char *sasl_realm;
dico_list_t sasl_anon_groups;

#ifdef WITH_GSASL
#include <gsaslstr.h>

static Gsasl *ctx;   

static int
cmp_names(const void *item, void *data)
{
    return c_strcasecmp(item, data);
}

static int
disabled_mechanism_p(char *name)
{
    if (sasl_enabled_mech
	&& !_dico_list_locate(sasl_enabled_mech, name, cmp_names))
	return 1;
    return !!_dico_list_locate(sasl_disabled_mech, name, cmp_names);
}

static void
send_challenge(dico_stream_t str, char *data)
{
    if (data[0]) { 
	 stream_printf(str, "130 challenge follows\n");
	 /* FIXME: use dicod_ostream_create */
	 stream_writez(str, data);
	 stream_writez(str, "\n.\n");
    }
    stream_printf(str, "330 send response\n");
}    

#define SASLRESP "SASLRESP"

static int
get_sasl_response(dico_stream_t str, char **pret, char **pbuf, size_t *psize)
{
    char *p;
    size_t rdbytes;
    
    if (get_input_line(str, pbuf, psize, &rdbytes))
	return 1;
    p = *pbuf;
    rdbytes = dico_trim_nl(p);
    if (rdbytes >= sizeof(SASLRESP)
	&& memcmp(p, SASLRESP, sizeof(SASLRESP) - 1) == 0
	&& isspace(p[sizeof(SASLRESP) - 1])) {
	for (p += sizeof(SASLRESP), rdbytes -= sizeof(SASLRESP); isspace(*p);
	     p++, rdbytes--);
	if (*p == '"') {
	    /* Simple unquoting */
	    p++;
	    rdbytes--;
	    p[rdbytes-1] = 0;
	}
	*pret = p;
	return 0;
    }
    dico_log(L_ERR, 0, _("Unexpected input instead of SASLRESP command: %s"),
	     *pbuf);
    return 1;
}

#define RC_SUCCESS 0
#define RC_FAIL    1
#define RC_NOMECH  2

struct sasl_data {
    const char *username;
    int anon;
};

static int
_append_item (void *item, void *data)
{
    char *copy = xstrdup (item);
    dico_list_t list = data;
    xdico_list_append (list, copy);
    return 0;
}

static int
sasl_auth(dico_stream_t str, char *mechanism, char *initresp,
	  Gsasl_session **psess)
{
    int rc;
    Gsasl_session *sess_ctx;
    char *input;
    char *output;
    char *inbuf;
    size_t insize;
    struct sasl_data sdata = { NULL, 0 };

    if (disabled_mechanism_p(mechanism)) 
	return RC_NOMECH;
    rc = gsasl_server_start (ctx, mechanism, &sess_ctx);
    if (rc != GSASL_OK) {
	dico_log(L_ERR, 0, _("SASL gsasl_server_start: %s"),
		 gsasl_strerror(rc));
	return rc == GSASL_UNKNOWN_MECHANISM ? RC_NOMECH : RC_FAIL;
    }

    gsasl_callback_hook_set(ctx, &sdata);
    output = NULL;
    if (initresp) {
	inbuf = xstrdup(initresp);
	insize = strlen(initresp) + 1;
    } else {
	inbuf = NULL;
	insize = 0;
    }
    input = inbuf;
    while ((rc = gsasl_step64(sess_ctx, input, &output)) == GSASL_NEEDS_MORE) {
	send_challenge(str, output);
	
	free(output);
	output = NULL;
	if (get_sasl_response(str, &input, &inbuf, &insize)) 
	    return RC_FAIL;
    }

    if (rc != GSASL_OK) {
	dico_log(L_ERR, 0, _("GSASL error: %s"), gsasl_strerror(rc));
	free(output);
	free(inbuf);
	gsasl_finish(sess_ctx);
	return RC_FAIL;
    }

    /* Some SASL mechanisms output data when GSASL_OK is returned */
    if (output[0]) 
	send_challenge(str, output);

    free(output);
    free(inbuf);
    
    if (sdata.username == NULL) {
	dico_log(L_ERR, 0, _("GSASL %s: cannot get username"), mechanism);
	gsasl_finish(sess_ctx);
	return RC_FAIL;
    }

    user_name = xstrdup(sdata.username);
    if (sdata.anon) {
	if (sasl_anon_groups) {
	    user_groups = xdico_list_create();
	    dico_list_iterate (sasl_anon_groups, _append_item, user_groups);
	}
    } else
	dico_udb_get_groups(user_db, sdata.username, &user_groups);
    check_db_visibility();

    *psess = sess_ctx;
    return RC_SUCCESS;
}

static void
dicod_saslauth(dico_stream_t str, int argc, char **argv)
{
    int rc;
    char *resp;
    Gsasl_session *sess;
    
    if (dico_udb_open(user_db)) {
	dico_log(L_ERR, 0, _("failed to open user database"));
	stream_writez(str,
		      "531 Access denied, "
		      "use \"SHOW INFO\" for server information\n");
	return;
    }
    rc = sasl_auth(str, argv[1], argv[2], &sess);
    dico_udb_close(user_db);
    switch (rc) {
    case RC_SUCCESS:
	resp = "230 Authentication successful";
	stream_printf(str, "%s\n", resp);
	/* FIXME: If insert_gsasl_stream fails, client gets wrong response */
	if (insert_gsasl_stream(sess, &str) == 0) {
	    replace_io_stream(str);
	    dicod_remove_command("SASLAUTH");
	}
	return;

    case RC_FAIL:
	resp = "531 Access denied, "
	       "use \"SHOW INFO\" for server information";
	break;

    case RC_NOMECH:
	resp = "532 Access denied, unknown mechanism";
	break;
    }
    stream_printf(str, "%s\n", resp);
}


static int
cb_validate(Gsasl *ctx, Gsasl_session *sctx)
{
    int rc;
    struct sasl_data *pdata = gsasl_callback_hook_get(ctx);
    const char *authid = gsasl_property_get(sctx, GSASL_AUTHID);
    const char *pass = gsasl_property_get(sctx, GSASL_PASSWORD);
    char *dbpass;

    if (!authid)
	return GSASL_NO_AUTHID;
    if (!pass)
	return GSASL_NO_PASSWORD;

    rc = dico_udb_check_password(user_db, authid, pass);
    if (rc == ENOSYS) {
        if (dico_udb_get_password(user_db, authid, &dbpass)) {
	    dico_log(L_ERR, 0,
		     _("failed to get password for `%s' from the database"),
		     authid);
	    return GSASL_AUTHENTICATION_ERROR;
	}
	rc = dicod_check_password(dbpass, pass);
	free(dbpass);
    }
    if (rc == 0) {
	pdata->username = xstrdup(authid);
	return GSASL_OK;
    } 
    return GSASL_AUTHENTICATION_ERROR;
}
    
static int
callback(Gsasl *ctx, Gsasl_session *sctx, Gsasl_property prop)
{
    int rc = GSASL_OK;
    struct sasl_data *pdata;
    const char *user;
    char *string;

    switch (prop) {
    case GSASL_PASSWORD:
	pdata = gsasl_callback_hook_get(ctx);
	user = pdata->username;
	if (!user) {
	    user = gsasl_property_get(sctx, GSASL_AUTHID);
	    if (!user) {
		dico_log(L_ERR, 0, _("user name not supplied"));
		return GSASL_NO_AUTHID;
	    }
	    pdata->username = user;
	}
	if (dico_udb_get_password(user_db, user, &string)) {
	    dico_log(L_ERR, 0,
		     _("failed to get password for `%s' from the database"),
		     user);
	    return GSASL_NO_PASSWORD;
	} 
	gsasl_property_set(sctx, prop, string);
	free(string);
	break;

    case GSASL_SERVICE:
	gsasl_property_set(sctx, prop, sasl_service);
	break;

    case GSASL_REALM:
	gsasl_property_set(sctx, prop, sasl_realm ? sasl_realm : hostname);
	break;

    case GSASL_HOSTNAME:
	gsasl_property_set(sctx, prop, hostname);
	break;

    case GSASL_VALIDATE_SIMPLE:
	rc = cb_validate(ctx, sctx);
	break;

#if 0
    FIXME:
    case GSASL_VALIDATE_EXTERNAL:
    case GSASL_VALIDATE_SECURID:
#endif

    case GSASL_VALIDATE_ANONYMOUS:
	pdata = gsasl_callback_hook_get(ctx);
	user = gsasl_property_get(sctx, GSASL_ANONYMOUS_TOKEN);
	pdata->username = user;
	pdata->anon = 1;
	break;

    case GSASL_VALIDATE_GSSAPI:
	pdata = gsasl_callback_hook_get(ctx);
	user = gsasl_property_get(sctx, GSASL_AUTHZID);
	pdata->username = user;
	break;
	
    default:
	rc = GSASL_NO_CALLBACK;
	/*dico_log(L_NOTICE, 0, _("Unsupported callback property %d"), prop);*/
	break;
    }

    return rc;
}

static int
init_sasl_0()
{
    int rc = gsasl_init(&ctx);
    if (rc != GSASL_OK) {
	dico_log(L_ERR, 0, _("cannot initialize libgsasl: %s"),
		 gsasl_strerror(rc));
	return 1;
    }
    gsasl_callback_set(ctx, callback);
    return 0;
}

static int
init_sasl_1()
{
    static struct dicod_command cmd =
	{ "SASLAUTH", 2, 3, "mechanism [initial-response]",
	  "Start SASL authentication",
	  dicod_saslauth };
    static int sasl_initialized;

    if (!sasl_initialized) {
	sasl_initialized = 1;
	dicod_add_command(&cmd);
	if (!sasl_service)
	    sasl_service = xstrdup ("dico");
    }
    return 0;
}

void
register_sasl()
{
  int rc;
  char *listmech;
  struct wordsplit ws;
  
  if (!sasl_enable || init_sasl_0())
      return;
  rc =  gsasl_server_mechlist(ctx, &listmech);
  if (rc != GSASL_OK) {
      dico_log(L_ERR, 0, _("cannot get list of available SASL mechanisms: "
			   "%s"),
	       gsasl_strerror (rc));
      return;
  }

  if (wordsplit(listmech, &ws,
		WRDSF_NOVAR | WRDSF_NOCMD | WRDSF_SQUEEZE_DELIMS) == 0) {
      int i;
      for (i = 0; i < ws.ws_wordc; i++) {
	  if (!disabled_mechanism_p(ws.ws_wordv[i])) {
	      char *name = xdico_sasl_mech_to_capa(ws.ws_wordv[i]);
	      dicod_capa_register(name, NULL, init_sasl_1, NULL);
	      dicod_capa_add(name);
	  }
      }
      wordsplit_free(&ws);
  }
  free(listmech);
}

#else
void
register_sasl()
{
    /* nothing */
}
#endif
