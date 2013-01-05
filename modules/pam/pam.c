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
#include <security/pam_appl.h>

static char *service = "dicod";

#define COPY_STRING(s) (s) ? strdup(s) : NULL

#define overwrite_and_free(ptr)			\
    do {					\
	char *s = ptr;				\
	while (*s)				\
	    *s++ = 0;				\
    }  while (0)


#ifndef PAM_AUTHTOK_RECOVER_ERR
# define PAM_AUTHTOK_RECOVER_ERR PAM_CONV_ERR
#endif

struct pam_cred {
    const char *user;
    const char *pass;
};

static int
_dico_conv(int num_msg, const struct pam_message **msg,
	   struct pam_response **resp, void *appdata_ptr)
{
    int status = PAM_SUCCESS;
    int i;
    struct pam_response *reply = NULL;
    struct pam_cred *cred = appdata_ptr;
    
    reply = calloc(num_msg, sizeof(*reply));
    if (!reply)
	return PAM_CONV_ERR;

    for (i = 0; i < num_msg && status == PAM_SUCCESS; i++) {
	switch (msg[i]->msg_style) {
	case PAM_PROMPT_ECHO_ON:
	    reply[i].resp_retcode = PAM_SUCCESS;
	    reply[i].resp = COPY_STRING(cred->user);
	    /* PAM frees resp */
	    break;

	case PAM_PROMPT_ECHO_OFF:
	    if (cred->pass) {
		reply[i].resp_retcode = PAM_SUCCESS;
		reply[i].resp = COPY_STRING(cred->pass);
		/* PAM frees resp */
	    } else
		status = PAM_AUTHTOK_RECOVER_ERR;
	  break;

	case PAM_TEXT_INFO:
	case PAM_ERROR_MSG:
	    reply[i].resp_retcode = PAM_SUCCESS;
	    reply[i].resp = NULL;
	    break;
	  
	default:
	    status = PAM_CONV_ERR;
	}
    }
    
    if (status != PAM_SUCCESS) {
      for (i = 0; i < num_msg; i++)
	  if (reply[i].resp) {
	      switch (msg[i]->msg_style) {
	      case PAM_PROMPT_ECHO_ON:
	      case PAM_PROMPT_ECHO_OFF:
		  overwrite_and_free(reply[i].resp);
		  break;
		
	      case PAM_ERROR_MSG:
	      case PAM_TEXT_INFO:
		  free(reply[i].resp);
	      }
	  }
      free(reply);
    } else
	*resp = reply;
    return status;
}


static int
db_check_pass(void *handle, const char *pwres,
	      const char *key, const char *pass)
{
    pam_handle_t *pamh;
    int pamerror;
    struct pam_cred cred = { key, pass };
    struct pam_conv _dico_pam_conv = { &_dico_conv, &cred };

#define PAM_ERROR if (pamerror != PAM_SUCCESS) break;

    do {
	pamerror = pam_start(pwres ? pwres : service, key,
			     &_dico_pam_conv, &pamh);
	PAM_ERROR;
	pamerror = pam_authenticate(pamh, 0);
	PAM_ERROR;
	pamerror = pam_acct_mgmt(pamh, 0);
	PAM_ERROR;
	pamerror = pam_setcred(pamh, PAM_ESTABLISH_CRED);
    } while(0);

    pam_end(pamh, PAM_SUCCESS);

    switch (pamerror) {
    case PAM_SUCCESS:
	return 0;
    case PAM_AUTH_ERR:
	return 1;
    }
    dico_log(L_ERR, 0, "PAM authentication error");
    return 1;
}

static int
db_get_groups(void *handle, const char *qgr, const char *key,
	      dico_list_t *pgroups)
{
    *pgroups = NULL;
    return 0;
}



static struct dico_udb_def pam_udb_def = {
    "pam",
    NULL,
    NULL,
    NULL,
    db_get_groups,
    db_check_pass
};


static int
dico_pam_init(int argc, char **argv)
{
    struct dico_option init_option[] = {
	{ DICO_OPTSTR(service), dico_opt_string, &service },
	{ NULL }
    };
    if (dico_parseopt(init_option, argc, argv, 0, NULL))
	return -1;
    return dico_udb_define(&pam_udb_def);
}


struct dico_database_module DICO_EXPORT(pam, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NODB,
    dico_pam_init,
};
