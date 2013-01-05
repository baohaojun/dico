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

static struct dict_connection *conn;

int
set_bool(int *pval, char *str)
{
    if (strcmp(str, "yes") == 0
	|| strcmp(str, "on") == 0
	|| strcmp(str, "true") == 0)
	*pval = 1;
    else if (strcmp(str, "no") == 0
	     || strcmp(str, "off") == 0
	     || strcmp(str, "false") == 0)
	*pval = 0;
    else {
	script_error(_("Expected boolean value"));
	return 1;
    }
    return 0;
}

void
ds_silent_close()
{
    if (conn) {
	stream_printf(conn->str, "QUIT\r\n");
	dict_read_reply(conn);
	dict_conn_close(conn);
	conn = NULL;
    }
}    

static int
get_list(struct dict_result **pres, char *cmd, char *code)
{
    if (conn && *pres == NULL) {
	stream_printf(conn->str, "%s\r\n", cmd);
	dict_read_reply(conn);
	if (dict_status_p(conn, code)) {
	    unsigned long count;
	    char *p;
	
	    count = strtoul (conn->buf + 3, &p, 10);
	    dict_multiline_reply(conn);
	    dict_result_create(conn, dict_result_match, count,
			       obstack_finish(&conn->stk));
	    dict_read_reply(conn);
	    *pres = dict_conn_last_result(conn);
	} else {
	    script_error(_("Cannot get listing: %s"), conn->buf);
	    return 1;
	}
    }
    return 0;
}

void
check_disconnect()
{
    if (conn) {
	int rc;
	fd_set rd, wr, ex;
	
	FD_ZERO(&rd);
	FD_SET(conn->fd, &rd);
	FD_ZERO(&wr);
	FD_SET(conn->fd, &wr);
	FD_ZERO(&ex);
	FD_SET(conn->fd, &ex);

	do 
	    rc = select(conn->fd + 1, &rd, &wr, &ex, NULL);
	while (rc == -1 && errno == EINTR);
	if (rc < 0 || FD_ISSET(conn->fd, &rd)) {
	    dict_conn_close(conn);
	    conn = NULL;
	}
    }
}

int
ensure_connection()
{
    check_disconnect();
    if (!conn) {
	if (!dico_url.host) {
	    script_error(_("Please specify server name or IP address"));
	    return 1;
	}
	
	if (dict_connect(&conn, &dico_url)) { 
	    script_error(_("Cannot connect to the server"));
	    return 1;
	}
	XDICO_DEBUG(1, _("Getting list of databases\n"));
	get_list(&conn->db_result, "SHOW DATABASES", "110");
	XDICO_DEBUG(1, _("Getting list of strategies\n"));
	get_list(&conn->strat_result, "SHOW STRATEGIES", "111");
	XDICO_DEBUG(1, _("Finished getting server information\n"));
    }
    return 0;
}

void
ds_open(int argc, char **argv)
{
    if (argc > 1) {
	if (argc == 3) {
	    int n = str2port(argv[2]);
	    if (n == -1) {
		script_error(_("Invalid port number or service name"));
		return;
	    }
	    dico_url.port = n;
	}
	xdico_assign_string(&dico_url.host, argv[1]);
    }

    if (!dico_url.host) {
	script_error(_("Please specify server name or IP address"));
	return;
    }
    
    ds_silent_close();
    ensure_connection();
}

void
ds_close(int argc, char **argv)
{
    if (!conn) 
	script_error(_("Nothing to close"));
    else 
	ds_silent_close();
}

void
ds_autologin(int argc, char **argv)
{
    if (argc == 1) {
	if (!autologin_file)
	    printf("%s\n", _("No autologin file."));
	else
	    printf("%s\n", autologin_file);
    } else {
	char *str;
	if (ds_tilde_expand(argv[1], &str)) {
	    free(autologin_file);
	    autologin_file = str;
	} else
	    xdico_assign_string(&autologin_file, argv[1]);
    }
}

void
ds_sasl(int argc, char **argv)
{
    if (argc == 1) {
	printf("%s\n", sasl_enabled_p() ? _("on") : _("off"));
    } else {
	int val;
	if (set_bool(&val, argv[1]) == 0)
	    sasl_enable(val);
    }
}

void
ds_database(int argc, char **argv)
{
    if (argc == 1) {
	printf("%s\n", dico_url.req.database ? dico_url.req.database : "!");
    } else
	/* FIXME: Check if such database is available?*/
	xdico_assign_string(&dico_url.req.database, argv[1]);
}

static char *
result_generator(struct dict_result *res, const char *text, int state)
{
    static int i, len;
    
    if (!state) { 
	i = 0;
	len = strlen(text);
    } 
    while (i < res->count) {
	char *s = res->set.mat[i].database;
	i++;
	if (strncmp(s, text, len) == 0)
	    return strdup(s);
    }
    return NULL;
}

static char *
db_generator(const char *text, int state)
{
    return result_generator(conn->db_result, text, state);
}

char **
ds_compl_database(int argc, char **argv, int ws)
{
    return dict_completion_matches(argc, argv, ws, db_generator);
}


void
ds_strategy(int argc, char **argv)
{
    if (argc == 1) {
	printf("%s\n", dico_url.req.strategy);
    } else
	/* FIXME: Check if such strategy is available?*/
	xdico_assign_string(&dico_url.req.strategy, argv[1]);
}

static char *
strat_generator(const char *text, int state)
{
    return result_generator(conn->strat_result, text, state);
}

char **
ds_compl_strategy(int argc, char **argv, int ws)
{
    return dict_completion_matches (argc, argv, ws, strat_generator);
}


void
ds_verbose(int argc, char **argv)
{
    if (argc == 1)
	printf("%d\n", debug_level);
    else {
	char *p;
	int n = strtoul(argv[1], &p, 10);
	if (*p)
	    script_error(_("invalid number"));
	else
	    debug_level = n;
    }
}

void
ds_transcript(int argc, char **argv)
{
    if (argc == 1) 
	printf(_("transcript is %s\n"), transcript ? _("on") : _("off"));
    else {
	set_bool(&transcript, argv[1]);
	if (conn)
	    dict_transcript(conn, transcript);
    }
}

void
ds_define(int argc, char **argv)
{
    if (ensure_connection())
	return;
    dico_url.req.type = DICO_REQUEST_DEFINE;
    xdico_assign_string(&dico_url.req.word, argv[1]);
    dict_lookup(conn, &dico_url);
}

void
ds_match(int argc, char **argv)
{
    int rc;
    
    if (ensure_connection())
	return;
    if (argc == 2 && argv[1][0]) {
	dict_result_free(conn->match_result);
	conn->match_result = NULL;
	dico_url.req.type = DICO_REQUEST_MATCH;
	xdico_assign_string(&dico_url.req.word, argv[1]);
	rc = dict_match(conn,
			quotearg_n (0, dico_url.req.database),
			quotearg_n (1, dico_url.req.strategy),
			quotearg_n (2, dico_url.req.word));
	if (rc == 0)
	    conn->match_result = dict_conn_last_result(conn);
	else {
	    print_reply(conn);
	    return;
	}
    } else if (!conn->match_result) {
	script_error(_("No previous match"));
	return;
    }

    print_match_result(conn->match_result);
}

/* Display a definition for the Nth result from a previous match. */
void
ds_define_nth(size_t num)
{
    struct dico_url url = dico_url;

    if (ensure_connection())
	return;
    if (!conn->match_result) {
	script_error(_("No previous match"));
	return;
    }
    if (num >= conn->match_result->count) {
	script_error(_("Invalid match number.  Type / to see the matches."));
	return;
    }
    url.req.type = DICO_REQUEST_DEFINE;
    url.req.database = conn->match_result->set.mat[num].database;
    url.req.word = conn->match_result->set.mat[num].word;
    dict_lookup(conn, &url);
}

void
ds_distance(int argc, char **argv)
{
    if (argc == 1) {
	if (conn) {
	    if (!dict_capa(conn, "xlev")) {
		printf(_("Server does not support XLEV extension"));
		return;
	    }
	    stream_printf(conn->str, "XLEV TELL\r\n");
	    dict_read_reply(conn);
	    if (dict_status_p(conn, "280")) 
		printf(_("Reported Levenshtein distance:%s\n"), conn->buf+3);
	    else {
		printf("%s\n",
		       _("Cannot query Levenshtein distance.  Server responded:"));
		printf("%s\n", conn->buf);
	    }
	}
	if (levenshtein_threshold == 0) 
	    printf(_("No distance configured\n"));
	else
	    printf(_("Configured Levenshtein distance: %u\n"),
		   levenshtein_threshold);
    } else {
	char *p;
	levenshtein_threshold = strtoul(argv[1], &p, 10);
	if (*p)
	    script_error(_("invalid number"));
    }
}

void
ds_show_db(int argc, char **argv)
{
    if (ensure_connection())
	return;
    print_result(conn->db_result);
}

void
ds_show_strat(int argc, char **argv)
{
    if (ensure_connection())
	return;
    print_result(conn->strat_result);
}

void
ds_show_info(int argc, char **argv)
{
    const char *dbname;
    if (argc == 1) 
	dbname = dico_url.req.database ? dico_url.req.database : "!";
    else
	dbname = argv[1];
    if (strcmp(dbname, "!") == 0)
	printf(_("Search all of the databases until a match is found,\n"
		 "and display all matches in that database.\n"));
    else if (strcmp(dbname, "*") == 0)
	printf(_("Search all of the databases and display all matches."));
    else {
	if (ensure_connection())
	    return;
	dict_run_single_command(conn, "SHOW INFO", dbname, "112");
    }
}

void
ds_version(int argc, char **argv)
{
    printf("%s\n", PACKAGE_STRING);
}

static char gplv3_text[] = "\
   GNU Dico is free software; you can redistribute it and/or modify\n\
   it under the terms of the GNU General Public License as published by\n\
   the Free Software Foundation; either version 3, or (at your option)\n\
   any later version.\n\
\n\
   GNU Dico is distributed in the hope that it will be useful,\n\
   but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
   GNU General Public License for more details.\n\
\n\
   You should have received a copy of the GNU General Public License\n\
   along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>.\n";

void
ds_warranty(int argc, char **argv)
{
    print_version_only(PACKAGE_STRING, stdout);
    putchar('\n');
    printf("%s", gplv3_text);
}
