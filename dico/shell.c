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

int interactive;
char *prompt;
#define DICO_PROMPT "dico> "

static void ds_prefix(int argc, char **argv);
static void ds_help(int argc, char **argv);
static void ds_quiet(int argc, char **argv);
static void ds_prompt(int argc, char **argv);
static char **no_compl(int argc, char **argv, int ws);
#ifdef WITH_READLINE
static void ds_history(int argc, char **argv);
static int retrieve_history(char *str);
#endif

char *helptext[][2] = {
    { N_("WORD"), N_("Define WORD.") },
    { N_("/WORD"), N_("Match WORD.") },
    { "/", N_("Redisplay previous matches.") },
    { N_("NUMBER"), N_("Define NUMBERth match.") },
#ifdef WITH_READLINE
    { "!NUMBER", N_("Edit NUMBERth previous command.") },
#endif
};

struct funtab funtab[] = {
    { "open",      1, 3, 
      N_("[HOST [PORT]]"), 
      N_("Connect to a DICT server."),
      ds_open, no_compl },
    { "close",     1, 1, 
      NULL,
      N_("Close the connection."),
      ds_close, },
    { "database",  1, 2, 
      N_("[NAME]"),
      N_("Set or display current database name."),
      ds_database, ds_compl_database },
    { "strategy",  1, 2, 
      N_("[NAME]"),
      N_("Set or display current strategy."),
      ds_strategy, ds_compl_strategy },
    { "distance", 1, 2, 
      N_("[NUM]"),
      N_("Set or query Levenshtein distance (server-dependent)."),
      ds_distance, no_compl },
    { "ls", 1, 1, 
      NULL,
      N_("List available matching strategies"),
      ds_show_strat, },
    { "ld", 1, 1, 
      NULL,
      N_("List all accessible databases"),
      ds_show_db, },
    { "info", 1, 2,
      N_("[DB]"),
      N_("Display the information about the database."),
      ds_show_info, ds_compl_database },
    { "prefix", 1, 2, 
      N_("[CHAR]"),
      N_("Set or display command prefix."),
      ds_prefix, },
    { "transcript", 1, 2, 
      N_("[BOOL]"),
      N_("Set or display session transcript mode."),
      ds_transcript, no_compl },
    { "verbose", 1, 2,
      N_("[NUMBER]"),
      N_("Set or display verbosity level."),
      ds_verbose, no_compl },
    { "prompt", 2, 2,
      N_("STRING"),
      N_("Change command line prompt."),
      ds_prompt, no_compl },
    { "pager", 1, 2,
      N_("STRING"),
      N_("Change or display pager settings."),
      ds_pager, no_compl },
    { "autologin", 1, 2, 
      N_("[FILE]"),
      N_("Set or display autologin file name."),
      ds_autologin,},
    { "sasl", 1, 2,
      N_("[BOOL]"),
      N_("Enable SASL authentication."),
      ds_sasl, no_compl },
#ifdef WITH_READLINE
    { "history", 1, 1,
      NULL,
      N_("Display command history."),
      ds_history, no_compl },
#endif
    { "help", 1, 1, 
      NULL,
      N_("Display this help text."),
      ds_help, },
    { "version", 1, 1, 
      NULL,
      N_("Print program version."),
      ds_version, },
    { "warranty", 1, 1, 
      NULL,
      N_("Print copyright statement."),
      ds_warranty, },
    { "quiet", 2, 2,
      NULL,
      NULL,
      ds_quiet },
    { "quit", 1, 1, 
      NULL,
      N_("Quit the shell.") },
    { NULL }
};

struct funtab *
find_funtab(const char *name)
{
    struct funtab *p;
    for (p = funtab; p->name; p++)
	if (strcmp(p->name, name) == 0)
	    return p;
    return NULL;
}

int cmdprefix;
char special_prefixes[2];

static void
ds_prefix(int argc, char **argv)
{
    if (argc == 1)
	printf(_("Command prefix is %c\n"), cmdprefix);
    else if (!(!argv[1][1] && argv[1][0] != '#' && ispunct(argv[1][0])))
	script_error(_("Expected a single punctuation character"));
    else {
	cmdprefix = argv[1][0];
	strcpy(special_prefixes, argv[1]);
#ifdef WITH_READLINE
	rl_special_prefixes = special_prefixes;
#endif
    }
}

static void
ds_help(int argc, char **argv)
{
    static size_t nlines;
    dico_stream_t str;
    struct funtab *ft;
    size_t i;

    if (nlines == 0) {
	nlines = DICO_ARRAY_SIZE(helptext) + 1;
	for (ft = funtab; ft->name; ft++) {
	    if (ft->docstring)
		nlines++;
	}
    }
    
    str = create_pager_stream(nlines);	
    for (i = 0; i < DICO_ARRAY_SIZE(helptext); i++) {
	stream_printf(str, "%-24s %s\n", gettext(helptext[i][0]),
		      gettext(helptext[i][1]));
    }
    dico_stream_write(str, "\n", 1);
    for (ft = funtab; ft->name; ft++) {
	int len = 0;
	const char *args;

	if (ft->docstring == NULL)
	    continue;
	if (cmdprefix) {
	    stream_printf(str, "%c", cmdprefix);
	    len++;
	}
	stream_printf(str, "%s ", ft->name);
	len += strlen(ft->name) + 1;
	if (ft->argdoc) 
	    args = gettext(ft->argdoc);
	else
	    args = "";
	if (len < 24)
	    len = 24 - len;
	else
	    len = 0;
	stream_printf(str, "%-*s %s\n",
		      len, args, gettext(ft->docstring));
    }
    dico_stream_close(str);
    dico_stream_destroy(&str);
}

static void
ds_quiet(int argc, char **argv)
{
    set_bool(&quiet_option, argv[1]);
}

typedef int (*script_getln_fn)(void *data, char **buf);


int line = 0;
const char *filename;

void
script_diag(int category, int errcode, const char *fmt, va_list ap)
{
    const char *pfx;
    char *newfmt;
    
    if (category == L_WARN) 
	pfx = _("warning: ");
    else 
	pfx = NULL;
    
    if (!filename) 
	asprintf(&newfmt, "%s%s", pfx ? pfx : "", fmt);
    else {
	asprintf(&newfmt, "%s:%d: %s%s",
		 filename, line,
		 pfx ? pfx : "", fmt);
    }
    dico_vlog(category, errcode, newfmt, ap);
    free (newfmt);
}

void
script_warning(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    script_diag(L_WARN, 0, fmt, ap);
    va_end(ap);
}    

void
script_error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    script_diag(L_ERR, 0, fmt, ap);
    va_end(ap);
}

static int
is_command(char **ptr)
{
    if (!cmdprefix)
	return 1;
    if ((*ptr)[0] == cmdprefix) {
	++*ptr;
	return 1;
    }
    return 0;
}


void
parse_script_file(const char *fname, script_getln_fn getln, void *data)
{
    char *buf = NULL;
    struct dico_tokbuf tb;
    int argc;
    char **argv;
    
    filename = fname;
    line = 0;
    dico_tokenize_begin(&tb);
    while (getln(data, &buf)) {
	char *p, *start;
	char *xargv[3];
	
	line++;

	start = skipws(buf);
	dico_trim_nl(start);
	if (*start == 0 || *start == '#')
	    continue;

	xdico_tokenize_string(&tb, start);
	argc = tb.tb_tokc;
	argv = tb.tb_tokv;
	if (argc == 0)
	    continue;

	p = argv[0];
	
#ifdef WITH_READLINE
	if (interactive) {
	    if (retrieve_history(buf))
		continue;
	    add_history(buf);
	}
#endif
	
	switch (p[0]) {
	case '/':
	    xargv[0] = "match";
	    xargv[1] = skipws(start + 1);
	    xargv[2] = NULL;
	    ds_match(2, xargv);
	    continue;

	case '?':
	    ds_help(0, NULL);
	    continue;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    if (argc == 1) {
		char *q;
		size_t num = strtoul(argv[0], &q, 10);
		if (*q == 0) {
		    ds_define_nth(num);
		    continue;
		}
	    }
	}
	    
	if (is_command(&p)) {
	    struct funtab *ft = find_funtab(p);
	    if (!ft) {
		script_error(_("unknown command"));
		continue;
	    }
	    if (ft->argmin == 0) {
		argc = 2;
		xargv[0] = argv[0];
		xargv[1] = skipws(buf + strlen(xargv[0]));
		xargv[2] = NULL;
		argv = xargv;
	    } else if (argc < ft->argmin) {
		script_error(_("not enough arguments"));
		continue;
	    } else if (argc > ft->argmax) {
		script_error(_("too many arguments"));
		continue;
	    }
	    
	    if (ft->fun)
		ft->fun(argc, argv);
	    else {
		ds_silent_close();
		break;
	    }
	} else {
	    xargv[0] = "define";
	    xargv[1] = start;
	    xargv[2] = NULL;
	    ds_define(2, xargv);
	}
    }
    dico_tokenize_end(&tb);
}
	

struct init_script {
    FILE *fp;
    char *buf;
    size_t size;
};

int
script_getline(void *data, char **buf)
{
    struct init_script *p = data;
    int rc = getline(&p->buf, &p->size, p->fp);
    *buf = p->buf;
    return rc > 0;
}

void
parse_init_script(const char *name)
{
    struct init_script scr;
    scr.fp = fopen(name, "r");
    if (!scr.fp) {
	if (errno != ENOENT) 
	    dico_log(L_ERR, errno, _("Cannot open init file %s"), name);
	return;
    }
    scr.buf = NULL;
    scr.size = 0;
    parse_script_file(name, script_getline, &scr);
    fclose(scr.fp);
    free(scr.buf);
}

void
parse_init_scripts()
{
    char *name = dico_full_file_name(get_homedir(), ".dico");
    parse_init_script(name);
    free(name);
    parse_init_script(".dico");
}



static void
ds_prompt(int argc, char **argv)
{
    xdico_assign_string(&prompt, argv[1]);
}

#ifdef WITH_READLINE

static char *
_command_generator(const char *text, int state)
{
    static int i, len;
    const char *name;
  
    if (!state) {
	i = 0;
	len = strlen (text);
	if (cmdprefix)
	    len--;
    }
    if (cmdprefix)
	text++;
    while ((name = funtab[i].name)) {
	i++;
	if (funtab[i-1].docstring
	    && strncmp(name, text, len) == 0) {
	    if (!cmdprefix)
		return strdup (name);
	    else {
		char *p = xmalloc(strlen(name) + 2);
		*p = cmdprefix;
		strcpy(p+1, name);
		return p;
	    }
	}
    }

    return NULL;
}

static char **
_command_completion(char *cmd, int start, int end)
{
    char **ret;
    struct wordsplit ws;
    
    /* FIXME: Use tokenizer */
    if (wordsplit_len (rl_line_buffer, end, &ws, WRDSF_DEFFLAGS))
	return NULL;
    rl_completion_append_character = ' ';
  
    if (ws.ws_wordc == 0 ||
	(ws.ws_wordc == 1 && strlen (ws.ws_wordv[0]) <= end - start)) {
	ret = rl_completion_matches (cmd, _command_generator);
	rl_attempted_completion_over = 1;
    } else {
	struct funtab *ft = find_funtab(ws.ws_wordv[0] + (cmdprefix ? 1 : 0));
	if (ft) {
	    if (ft->compl)
		ret = ft->compl(ws.ws_wordc, ws.ws_wordv, start == end);
	    else if (ft->argmax == 1) {
		rl_attempted_completion_over = 1;
		ret = NULL;
	    } else
		ret = NULL;
	} else {
	    rl_attempted_completion_over = 1;
	    ret = NULL;
	}
    }
    wordsplit_free(&ws);
    return ret;
}

#define HISTFILE_SUFFIX "_history"

static char *
get_history_file_name()
{
  static char *filename = NULL;

  if (!filename) {
	char *hname;
	
	hname = xmalloc(1 +
			strlen (rl_readline_name) + sizeof HISTFILE_SUFFIX);
	strcpy(hname, ".");
	strcat(hname, rl_readline_name);
	strcat(hname, HISTFILE_SUFFIX);
	filename = dico_full_file_name(get_homedir(), hname);
	free(hname);
  }
  return filename;
}

static void
ds_history(int argc, char **argv)
{
    dico_stream_t str;
    HIST_ENTRY **hlist;
    int i;

    str = create_pager_stream(history_length);
    hlist = history_list();
    for (i = 0; i < history_length; i++) 
	stream_printf(str, "%4d) %s\n", i + 1, hlist[i]->line);
    dico_stream_close(str);
    dico_stream_destroy(&str);
}

static char *pre_input_line;

static int
pre_input (void)
{
    rl_insert_text(pre_input_line);
    free(pre_input_line);
    rl_pre_input_hook = NULL;
    rl_redisplay();
    return 0;
}

static int
retrieve_history(char *str)
{
    char *out;
    int rc;
    
    rc = history_expand(str, &out);
    switch (rc)	{
    case -1:
	script_error("%s", out);
	free(out);
	return 1;
	
    case 0:
	break;

    case 1:
	pre_input_line = out;
	rl_pre_input_hook = pre_input;
	return 1;
	
    case 2:
	printf("%s\n", out);
	free(out);
	return 1;
    }
    return 0;
}
	

#endif

char **
dict_completion_matches(int argc, char **argv, int ws,
                        char *(*generator)(const char *, int))
{
#ifdef WITH_READLINE
    rl_attempted_completion_over = 1;
    if (ensure_connection())
	return NULL;
    return rl_completion_matches (ws ? "" : argv[argc-1], generator);
#endif
}


static char **
no_compl (int argc DICO_ARG_UNUSED,
	  char **argv DICO_ARG_UNUSED, int ws DICO_ARG_UNUSED)
{
#ifdef WITH_READLINE
    rl_attempted_completion_over = 1;
#endif
    return NULL;
}

void
shell_init(struct init_script *p)
{
    interactive = isatty(fileno(stdin));
#ifdef WITH_READLINE
    if (interactive) {
	rl_readline_name = dico_program_name;
	rl_attempted_completion_function = (CPPFunction*) _command_completion;
	read_history (get_history_file_name());
    } 
#endif
    p->fp = stdin;
    p->buf = NULL;
    p->size = 0;
}

void
shell_finish(struct init_script *p)
{
#ifdef WITH_READLINE
    if (interactive)
	write_history (get_history_file_name());
#endif
    free(p->buf);
}

int
shell_getline(void *data, char **buf)
{
    if (interactive) {
#ifdef WITH_READLINE
	char *p = readline(prompt);
	if (!p)
	    return 0;
	*buf = p;
	return 1;
#else
	fprintf(stdout, "%s", prompt);
	fflush(stdout);
#endif
    }
    return script_getline(data, buf);
}

void
shell_banner()
{
    print_version(PACKAGE_STRING, stdout);
    printf("%s\n\n", _("Type ? for help summary"));
}

void
dico_shell()
{
    struct init_script dat;
    shell_init(&dat);
    if (interactive) {
	xdico_assign_string(&prompt, DICO_PROMPT);
	if (!quiet_option)  
	    shell_banner();
    }
    if (!cmdprefix)
	cmdprefix = '.';
    parse_script_file(NULL, shell_getline, &dat);
    shell_finish(&dat);
}




