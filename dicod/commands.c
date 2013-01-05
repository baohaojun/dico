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

#include <dicod.h>
#include <sys/utsname.h>

int got_quit;

void
dicod_quit(dico_stream_t str, int argc, char **argv)
{
    got_quit = 1;
    stream_writez(str, "221 bye");
    report_current_timing(str, "dicod");
    stream_writez(str, "\n");
}

void dicod_show_std_help(dico_stream_t str);

void
dicod_help(dico_stream_t str, int argc, char **argv)
{
    const char *text = help_text;
    dico_stream_t ostr;
	
    stream_writez(str, "113 help text follows\n");
    ostr = dicod_ostream_create(str, NULL);
    
    if (text) {
	if (text[0] == '+') {
	    dicod_show_std_help(ostr);
	    text++;
	}
	stream_write_multiline(ostr, text);
    } else
	dicod_show_std_help(ostr);

    stream_writez(ostr, "\n");
    dico_stream_close(ostr);
    dico_stream_destroy(&ostr);

    stream_writez(str, ".\n");
    stream_writez(str, "250 ok\n");    
}

void
dicod_show_info(dico_stream_t str, int argc, char **argv)
{
    char *dbname = argv[2];
    dicod_database_t *dict = find_database(dbname);
    if (!dict) 
	stream_writez(str, "550 invalid database, use SHOW DB for a list\n");
    else {
	dico_stream_t ostr;
	char *info = dicod_get_database_info(dict);
	stream_printf(str, "112 information for %s\n", dbname);
	ostr = dicod_ostream_create(str, NULL);
	if (info) {
	    stream_write_multiline(ostr, info);
	    dicod_free_database_info(dict, info);
	} else
	    stream_writez(ostr, "No information available.\n");
	stream_writez(ostr, "\n");
	dico_stream_close(ostr);
	dico_stream_destroy(&ostr);
	
	stream_writez(str, ".\n");
	stream_writez(str, "250 ok\n");    
    }
}

static int
_show_database(void *item, void *data)
{
    dicod_database_t *dict = item;
    dico_stream_t str = data;
    char *descr = dicod_get_database_descr(dict);
    char *pdescr;

    if (utf8_quote(descr ? descr : "", &pdescr))
	xalloc_die();
    stream_printf(str, "%s \"%s\"\n", dict->name, pdescr);
    dicod_free_database_descr(dict, descr);
    free(pdescr);
    return 0;
}

void
dicod_show_databases(dico_stream_t str, int argc, char **argv)
{
    size_t count = database_count();
    if (count == 0) 
	stream_printf(str, "554 No databases present\n");
    else {
	dico_stream_t ostr;
	
	stream_printf(str, "110 %lu databases present\n",
		      (unsigned long) count);
	ostr = dicod_ostream_create(str, NULL);
	database_iterate(_show_database, ostr);
	dico_stream_close(ostr);
	dico_stream_destroy(&ostr);
	stream_writez(str, ".\n");
	stream_writez(str, "250 ok\n");
    }
}

static int
_show_strategy(void *item, void *data)
{
    dico_strategy_t sp = item;
    dico_stream_t str = data;

    stream_printf(str, "%s \"%s\"\n",
		  sp->name, quotearg(sp->descr));
    return 0;
}

void
dicod_show_strategies(dico_stream_t str, int argc, char **argv)
{
    size_t count = dico_strategy_count();
    if (count == 0)
	stream_printf(str, "555 No strategies available\n");
    else {
	dico_stream_t ostr;
	
	stream_printf(str, "111 %lu strategies present: list follows\n",
		      (unsigned long) count);
	ostr = dicod_ostream_create(str, NULL);
	dico_strategy_iterate(_show_strategy, ostr);
	dico_stream_close(ostr);
	dico_stream_destroy(&ostr);
	stream_writez(str, ".\n");
	stream_writez(str, "250 ok\n");
    }
}
	
void
dicod_show_server(dico_stream_t str, int argc, char **argv)
{
    dico_stream_t ostr;
    
    stream_writez(str, "114 server information\n");
    ostr = dicod_ostream_create(str, NULL);
    stream_writez(str, "dicod ");
    if (show_sys_info_p()) {
	struct utsname uts;
	uname(&uts);
	stream_printf(str, "(%s) on %s %s, ", PACKAGE_STRING,
		      uts.sysname, uts.release);
    } else
	stream_writez(str, "server on ");
    stream_writez(str, hostname);
    if (timing_option && show_sys_info_p()) {
	xdico_timer_t t;
	double x, fph;
	
	stream_writez(str, " up ");
	t = timer_stop("server");
	x = timer_get_real(t);
	timer_format_time(str, x);
	if (mode == MODE_DAEMON && !single_process) {
	    x /= 3600;
	    if (x < 0.0001)
		fph = 0;
	    else
		fph = (total_forks + 1) / x;
	    stream_printf(str, ", %lu forks (%.1f/hour)", total_forks, fph);
	}
    }
    stream_writez(str, "\n");
    if (server_info) {
	stream_write_multiline(ostr, server_info);
	stream_writez(ostr, "\n");
    }
    dico_stream_close(ostr);
    dico_stream_destroy(&ostr);
    stream_writez(str, ".\n");
    stream_writez(str, "250 ok\n");    
}

void
dicod_status(dico_stream_t str, int argc, char **argv)
{
    stream_writez(str, "210");
    if (timing_option) 
	report_timing(str, timer_stop("dicod"), &total_stat);
    else
	stream_writez(str, " No timing data available");
    stream_writez(str, "\n");
}

void
dicod_client(dico_stream_t str, int argc, char **argv)
{
    free(client_id);
    if (argc > 1) {
	int rc = dico_argcv_string (argc - 1, (char const **)(argv + 1),
				    &client_id);
	if (rc)
	    dico_log(L_ERR, rc, "dicod_client");
	else
	    dico_log(L_INFO, 0, "Client info: %s", client_id);
    }
    stream_writez(str, "250 ok\n");
}

void
dicod_match(dico_stream_t str, int argc, char **argv)
{
    const char *dbname = argv[1];
    const char *word = argv[3];
    const dico_strategy_t strat = dico_strategy_find(argv[2]);
    
    total_bytes_out = 0;
    if (!strat) 
	stream_writez(str,
		      "551 Invalid strategy, use SHOW STRAT for a list\n");
    else if (strcmp(dbname, "!") == 0) 
	dicod_match_word_first(str, strat, word);
    else if (strcmp(dbname, "*") == 0) 
	dicod_match_word_all(str, strat, word);
    else {
	dicod_database_t *db = find_database(dbname);
    
	if (!db) 
	    stream_writez(str,
			  "550 invalid database, use SHOW DB for a list\n");
	else
	    dicod_match_word_db(db, str, strat, word);
    }
    access_log(argc, argv);
}

void
dicod_define(dico_stream_t str, int argc, char **argv)
{
    char *dbname = argv[1];
    char *word = argv[2];
    
    total_bytes_out = 0;
    if (strcmp(dbname, "!") == 0) {
	dicod_define_word_first(str, word);
    } else if (strcmp(dbname, "*") == 0) {
	dicod_define_word_all(str, word);
    } else {   
	dicod_database_t *db = find_database(dbname);
    
	if (!db) 
	    stream_writez(str,
			  "550 invalid database, use SHOW DB for a list\n");
	else
	    dicod_define_word_db(db, str, word);
    }
    access_log(argc, argv);
}


struct dicod_command command_tab[] = {
    { "DEFINE", 3, 3, "database word", "look up word in database",
      dicod_define },
    { "MATCH", 4, 4, "database strategy word",
      "match word in database using strategy",
      dicod_match },
    { "SHOW DB", 2, 2, NULL, "list all accessible databases",
      dicod_show_databases, },
    { "SHOW DATABASES", 2, 2, NULL, "list all accessible databases",
      dicod_show_databases, },
    { "SHOW STRAT", 2, 2, NULL, "list available matching strategies",
      dicod_show_strategies },
    { "SHOW STRATEGIES", 2, 2, NULL, "list available matching strategies",
      dicod_show_strategies  },
    { "SHOW INFO", 3, 3, "database", "provide information about the database",
      dicod_show_info },
    { "SHOW SERVER", 2, 2, NULL, "provide site-specific information",
      dicod_show_server },
    { "CLIENT", 1, DICOD_MAXPARAM_INF, "info", "identify client to server",
      dicod_client },
    { "STATUS", 1, 1, NULL, "display timing information",
      dicod_status },
    { "HELP", 1, 1, NULL, "display this help information",
      dicod_help },
    { "QUIT", 1, 1, NULL, "terminate connection", dicod_quit },
    { NULL }
};

struct locate_data {
    int argc;
    char **argv;
};

static int
_cmd_arg_cmp(const void *item, void *data)
{
    const struct dicod_command *p = item;
    const struct locate_data *datptr = data;
    int i, off = 0;

    for (i = 0; i < datptr->argc; i++) {
	int len = strlen(datptr->argv[i]);
	if (c_strncasecmp(p->keyword + off, datptr->argv[i], len) == 0) {
	    off += len;
	    if (p->keyword[off] == 0) 
		return 0;
	    if (p->keyword[off] == ' ') {
		off++;
		continue;
	    }
	}
	break;
    }
    return 1;
}

dico_list_t /* of struct dicod_command */ command_list;
    
void
dicod_add_command(struct dicod_command *cmd)
{
    if (!command_list) {
	command_list = xdico_list_create();
	dico_list_set_comparator(command_list, _cmd_arg_cmp);
    }
    xdico_list_append(command_list, cmd);
}

static int
cmd_comp(const void *a, void *b)
{
    const struct dicod_command *ca = a;
    const struct dicod_command *cb = b;
    return c_strcasecmp (ca->keyword, cb->keyword);
}
	
void
dicod_remove_command(const char *name)
{
    struct dicod_command cmd;
    cmd.keyword = name;
    _dico_list_remove(command_list, &cmd, cmd_comp, NULL);
}

void
dicod_init_command_tab()
{
    struct dicod_command *p;
    
    for (p = command_tab; p->keyword; p++) 
	dicod_add_command(p);
}

static int
_print_help(void *item, void *data)
{
    struct dicod_command *p = item;
    dico_stream_t str = data;
    int len = strlen(p->keyword);

    stream_writez(str, p->keyword);
    if (p->param) {
	stream_printf(str, " %s", p->param);
	len += strlen(p->param) + 1;
    }
    
    if (len < 31)
	len = 31 - len;
    else
	len = 0;
    stream_printf(str, "%*.*s -- %s\n", len, len, "", p->help);
    return 0;
}

void
dicod_show_std_help(dico_stream_t str)
{
    dico_list_iterate(command_list, _print_help, str);
}

struct dicod_command *
locate_command(int argc, char **argv)
{
    struct locate_data ld;
    ld.argc = argc;
    ld.argv = argv;
    return dico_list_locate(command_list, &ld);
}

void
dicod_handle_command(dico_stream_t str, int argc, char **argv)
{
    struct dicod_command *cmd;
    int nargc = 0;
    char **nargv = NULL;

    if (alias_expand(argc, argv, &nargc, &nargv) == 0) {
	argc = nargc;
	argv = nargv;
    }

    cmd = locate_command(argc, argv);
    if (!cmd) 
	stream_writez(str, "500 unknown command\n");
    else if (argc < cmd->minparam
	     || (cmd->maxparam != DICOD_MAXPARAM_INF && argc > cmd->maxparam))
	stream_writez(str, "501 wrong number of arguments\n");
    else if (!cmd->handler)
	stream_writez(str, "502 command is not yet implemented, sorry\n");
    else
	cmd->handler(str, argc, argv);

    free(nargv);
}

