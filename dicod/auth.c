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
#include <md5.h>

static int
verify_apop(const char *password, const char *user_digest)
{
    int i;
    struct md5_ctx md5context;
    unsigned char md5digest[16];
    char buf[sizeof(md5digest) * 2 + 1];
    char *p;

    md5_init_ctx(&md5context);
    md5_process_bytes(msg_id, strlen(msg_id), &md5context);
    md5_process_bytes(password, strlen(password), &md5context);
    md5_finish_ctx(&md5context, md5digest);

    for (i = 0, p = buf; i < 16; i++, p += 2)
	sprintf(p, "%02x", md5digest[i]);
    return strcmp(user_digest, buf);
}

static int
auth(const char *username, const char *authstr)
{
    int rc = 1;
    char *password;
    
    if (dico_udb_open(user_db)) {
	dico_log(L_ERR, 0, _("failed to open user database"));
	return 1;
    }
    if (dico_udb_get_password(user_db, username, &password)) {
	dico_log(L_ERR, 0,
		 _("failed to get password for `%s' from the database"),
		 username);
    } else {
	if (!password) {
	    dico_log(L_ERR, 0, _("no such user `%s'"), username);
	    rc = 1;
	} else {
	    rc = verify_apop(password, authstr);
	    if (rc) 
		dico_log(L_ERR, 0,
			 _("authentication failed for `%s'"), username);
	    else {
		user_name = xstrdup(username);
		dico_udb_get_groups(user_db, username, &user_groups);
	    }
	    free(password);
	}
    }
    dico_udb_close(user_db);
    return rc;
}

void
dicod_auth(dico_stream_t str, int argc, char **argv)
{
    if (auth(argv[1], argv[2]) == 0) {
	stream_writez(str, "230 Authentication successful");
	check_db_visibility();
    } else
	stream_writez(str,
		      "531 Access denied, "
		      "use \"SHOW INFO\" for server information");
    dico_stream_write(str, "\n", 1);
}

static int
auth_init(void *ptr)
{
    if (user_db == NULL) {
	dico_log(L_ERR, 0, "auth capability required but user database is "
	       "not configured");
	return 1;
    }
    return 0;
}

void
register_auth()
{
    static struct dicod_command cmd[] = {
	{ "AUTH", 3, 3, "user string", "provide authentication information",
	  dicod_auth },
	{ NULL }
    };	
    dicod_capa_register("auth", cmd, auth_init, NULL);
}


void
init_auth_data()
{
    free(user_name);
    user_name = NULL;
    free(client_id);
    client_id = NULL;
    free(identity_name);
    identity_name = NULL;
    dico_list_destroy(&user_groups);
    reset_db_visibility();
}

