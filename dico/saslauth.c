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

#include "dico-priv.h"

#ifdef WITH_GSASL
#include <gsaslstr.h>

static int
_free_el(void *item, void *data DICO_ARG_UNUSED)
{
    free(item);
    return 0;
}

static dico_list_t
get_implemented_mechs(Gsasl *ctx)
{
    char *listmech;
    dico_list_t supp = NULL;
    int rc;
    struct wordsplit ws;
    
    rc =  gsasl_server_mechlist(ctx, &listmech);
    if (rc != GSASL_OK) {
	dico_log(L_ERR, 0, _("cannot get list of available SASL mechanisms: "
			     "%s"),
		 gsasl_strerror (rc));
	return NULL;
    }

    if (wordsplit(listmech, &ws, WRDSF_DEFFLAGS) == 0) {
	int i;
	supp = xdico_list_create();
	dico_list_set_free_item(supp, _free_el, NULL);
	for (i = 0; i < ws.ws_wordc; i++) 
	    xdico_list_append(supp, ws.ws_wordv[i]);
	ws.ws_wordc = 0;
	wordsplit_free(&ws);
    }
    free(listmech);
    return supp;
}

static int
str_str_cmp(const void *item, void *data)
{
    return c_strcasecmp(item, data);
}

static void
upcase(char *str)
{
    for (; *str; str++)
	*str = toupper(*str);
}

#define CRED_HOSTNAME(c) ((c)->hostname ? (c)->hostname :  \
			  ((c)->hostname = xdico_local_hostname()))

static int
callback(Gsasl *ctx, Gsasl_session *sctx, Gsasl_property prop)
{
    int rc = GSASL_OK;
    struct auth_cred *cred = gsasl_callback_hook_get(ctx);

    switch (prop) {
    case GSASL_PASSWORD:
	gsasl_property_set(sctx, prop, cred->pass);
	break;

    case GSASL_AUTHID:
    case GSASL_ANONYMOUS_TOKEN:
	gsasl_property_set(sctx, prop, cred->user);
	break;

    case GSASL_AUTHZID:
	gsasl_property_set(sctx, prop, NULL);
	break;
	
    case GSASL_SERVICE:
	gsasl_property_set(sctx, prop,
			   cred->service ? cred->service : "dico");
	break;

    case GSASL_REALM:
	gsasl_property_set(sctx, prop,
			   cred->realm ? cred->realm : CRED_HOSTNAME(cred));
	break;

    case GSASL_HOSTNAME:
	gsasl_property_set(sctx, prop, CRED_HOSTNAME(cred));
	break;
	
    default:
	rc = GSASL_NO_CALLBACK;
	dico_log(L_NOTICE, 0, _("Unknown callback property %d"), prop);
	break;
    }

    return rc;
}

static int
sasl_read_response(struct dict_connection *conn, char **data)
{
    if (dict_read_reply(conn)) {
	dico_log(L_ERR, 0, _("GSASL handshake aborted"));
	return 1;
    }
    if (dict_status_p(conn, "130")) {
	if (dict_multiline_reply(conn)
	    || dict_read_reply(conn)) {
	    dico_log(L_ERR, 0, _("GSASL handshake aborted"));
	    return 1;
	}
	*data = obstack_finish(&conn->stk);
	dico_trim_nl(*data);
    } else
	*data = NULL;
    return 0;
}

static void
sasl_free_data(struct dict_connection *conn, char **pdata)
{
    if (*pdata) {
	obstack_free(&conn->stk, *pdata);
	*pdata = NULL;
    }
}
    
int
do_gsasl_auth(Gsasl *ctx, struct dict_connection *conn, char *mech)
{
    Gsasl_session *sess;
    int rc;
    char *output = NULL;
    char *input = NULL;
    
    rc = gsasl_client_start(ctx, mech, &sess);
    if (rc != GSASL_OK) {
	dico_log(L_ERR, 0, "SASL gsasl_client_start: %s",
		 gsasl_strerror(rc));
	return 1;
    }
    stream_printf(conn->str, "SASLAUTH %s\r\n", mech);

    if (sasl_read_response(conn, &input))
	return 1;

    do {
	if (!dict_status_p(conn, "330")) {
	    dico_log(L_ERR, 0,
		     _("GSASL handshake aborted: unexpected reply: %s"),
		     conn->buf);
	    return 1;
	}
	rc = gsasl_step64(sess, input, &output);
	sasl_free_data(conn, &input);
	if (rc != GSASL_NEEDS_MORE && rc != GSASL_OK)
	    break;
	stream_printf(conn->str, "SASLRESP %s\r\n",
		      output[0] ? output : "\"\"");
	free(output);
	output = NULL;
	if (sasl_read_response(conn, &input))
	    return 1;
    } while (rc == GSASL_NEEDS_MORE);

    free(output);
    if (rc != GSASL_OK) {
	dico_log(L_ERR, 0, _("GSASL error: %s"), gsasl_strerror(rc));
	return 1;
    }

    if (dict_status_p(conn, "330")) {
	/* Additional data are ignored. */
	sasl_free_data(conn, &input);
	if (sasl_read_response(conn, &input))
	    return 1;
    }

    if (dict_status_p(conn, "230")) {
	/* Authentication successful */
	insert_gsasl_stream(sess, &conn->str);
	return 0;
    }
    
    print_reply(conn);
    
    return 1;
}

struct authctx {
    Gsasl *ctx;
    dico_list_t mech;
};

static int
match_capa(struct dict_connection *conn, const char *p)
{
    int i;

    for (i = 0; i < conn->capac; i++)
	if (xdico_sasl_capa_match_p(p, conn->capav[i]))
	    return 1;
    return 0;
}

static int
getauthcontext(struct dict_connection *conn, struct authctx *authctx)
{
    Gsasl *ctx;
    int rc;
    dico_list_t mechs;
    dico_iterator_t itr;
    char *p;
    
    XDICO_DEBUG(1, _("Initializing SASL\n"));
    rc = gsasl_init(&ctx);
    if (rc != GSASL_OK)	{
	dico_log(L_ERR, 0, _("Cannot initialize libgsasl: %s"),
		 gsasl_strerror(rc));
	return 1;
    }

    mechs = get_implemented_mechs(ctx);
    if (!mechs) {
	gsasl_done(ctx);
	return 1;
    }

    itr = xdico_list_iterator(mechs);
    for (p = dico_iterator_first(itr); p; p = dico_iterator_next(itr)) {
	if (!match_capa(conn, p))
	    dico_iterator_remove_current(itr, NULL);
    }
    dico_iterator_destroy(&itr);

    if (dico_list_count(mechs) == 0) {
	dico_list_destroy(&mechs);
	gsasl_done(ctx);
	return 1;
    }

    authctx->ctx = ctx;
    authctx->mech = mechs;
    
    return 0;
}

static void
freeauthcontext(struct authctx *authctx)
{
    gsasl_done(authctx->ctx);
    dico_list_destroy(&authctx->mech);
}

int
saslauth0(struct dict_connection *conn, struct auth_cred *cred,
	  struct authctx *authctx)
{
    int rc;
    char *mech;
    dico_list_t mechlist;
    
    XDICO_DEBUG(1, _("Trying SASL\n"));
    gsasl_callback_hook_set(authctx->ctx, cred);
    gsasl_callback_set(authctx->ctx, callback);

    if (cred->mech)
	mechlist = dico_list_intersect(cred->mech, authctx->mech,
				       str_str_cmp);
    else
	mechlist = cred->mech;
    if (!mechlist || dico_list_count(mechlist) == 0) {
	dico_log(L_ERR, 0, _("No suitable SASL mechanism found"));
	if (mechlist && mechlist != cred->mech)
	    dico_list_destroy(&mechlist);
	return AUTH_CONT;
    }

    mech = dico_list_item(mechlist, 0);
    upcase(mech);
    
    dico_log(L_DEBUG, 0, _("Selected authentication mechanism %s"),
	     mech);
    
    rc = do_gsasl_auth(authctx->ctx, conn, mech);

    if (mechlist != cred->mech)
	dico_list_destroy(&mechlist);
    
    XDICO_DEBUG_F1(1, "%s\n",
		   rc == 0 ?
		   _("SASL authentication succeeded") :
		   _("SASL authentication failed"));

    /* FIXME */
    return rc == 0 ? AUTH_OK : AUTH_FAIL;
}

int
saslauth(struct dict_connection *conn, dico_url_t url)
{
    struct authctx authctx;
    int rc = AUTH_FAIL;
    struct auth_cred cred;

    if (getauthcontext(conn, &authctx))
	return AUTH_CONT;
    
    switch (auth_cred_get(url->host, &cred)) {
    case GETCRED_OK:
	if (cred.sasl) {
	    rc = saslauth0(conn, &cred, &authctx);
	    auth_cred_free(&cred);
	} else
	    rc = AUTH_CONT;
	break;

    case GETCRED_FAIL:
	dico_log(L_WARN, 0,
		 _("Not enough credentials for authentication"));
	rc = AUTH_FAIL;
	break;

    case GETCRED_NOAUTH:
	XDICO_DEBUG(1, _("Skipping authentication\n"));
	rc = AUTH_OK;
    }
    freeauthcontext(&authctx);
    return rc;
}

static int sasl_enable_state = 1;

void
sasl_enable(int val)
{
    sasl_enable_state = 1;
}

int
sasl_enabled_p()
{
    return sasl_enable_state;
}

#else
int
saslauth(struct dict_connection *conn, dico_url_t url)
{
    return AUTH_CONT;
}

void
sasl_enable(int val)
{
    dico_log(L_WARN, 0, _("Dico compiled without SASL support"));
}

int
sasl_enabled_p()
{
    return 0;
}
#endif
