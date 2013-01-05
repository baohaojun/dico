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

struct dico_url dico_url;
struct auth_cred default_cred;
char *client = DICO_CLIENT_ID;
enum dico_client_mode mode = mode_define;
int transcript;
IPADDR source_addr = INADDR_ANY;
int noauth_option;
unsigned levenshtein_threshold;
char *autologin_file;
int quiet_option;

int debug_level;
int debug_source_info;
dico_stream_t debug_stream;


void
fixup_url()
{
    xdico_assign_string(&dico_url.proto, "dict");
    if (!dico_url.host)
	xdico_assign_string(&dico_url.host, DEFAULT_DICT_SERVER);
    if (!dico_url.req.database)
	xdico_assign_string(&dico_url.req.database, "!");
    if (!dico_url.req.strategy)
	xdico_assign_string(&dico_url.req.strategy, ".");
    if (mode == mode_match)
	dico_url.req.type = DICO_REQUEST_MATCH;
}

int
main(int argc, char **argv)
{
    int index, rc = 0;

    appi18n_init();
    dico_set_program_name(argv[0]);
    debug_stream = dico_dbg_stream_create();
    parse_init_scripts();
    get_options(argc, argv, &index);
    fixup_url();
    set_quoting_style(NULL, escape_quoting_style);
    signal(SIGPIPE, SIG_IGN);

    argc -= index;
    argv += index;

    switch (mode) {
    case mode_define:
    case mode_match:
	if (!argc) {
	    dico_shell();
	    exit(0);
	}
	break;

    default:
	if (argc)
	    dico_log(L_WARN, 0,
		     _("extra command line arguments ignored"));
    }
    
    switch (mode) {
    case mode_define:
    case mode_match:
	if (!argc) 
	    dico_die(1, L_ERR, 0,
		     _("you should give a word to look for or an URL"));
	while (argc--) 
	    rc |= dict_word((*argv)++) != 0;
	break;
	
    case mode_dbs:
	rc = dict_single_command("SHOW DATABASES", NULL, "110");
	break;
	
    case mode_strats:
	rc = dict_single_command("SHOW STRATEGIES", NULL, "111");
	break;
	
    case mode_help:
	rc = dict_single_command("HELP", NULL, "113");
	break;
	
    case mode_info:
	if (!dico_url.req.database)
	    dico_die(1, L_ERR, 0, _("Database name not specified"));
	rc = dict_single_command("SHOW INFO", dico_url.req.database, "112");
	break;
	
    case mode_server:
	rc = dict_single_command("SHOW SERVER", NULL, "114");
	break;
    }	
    
    return rc;
}
