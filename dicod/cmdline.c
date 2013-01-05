#line 832 "../grecs/build-aux/getopt.m4"
/* -*- buffer-read-only: t -*- vi: set ro:
   THIS FILE IS GENERATED AUTOMATICALLY.  PLEASE DO NOT EDIT.
*/
#line 1 "cmdline.opt"
/* This file is part of GNU Dico. -*- c -*-
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <gettext.h>

static struct grecs_txtacc *pp_cmd_acc;

#line 27 "cmdline.opt"

#line 27
void print_help(void);
#line 27
void print_usage(void);
#line 171 "cmdline.opt"

#line 171
#include <grecs.h>
#line 171
/* Option codes */
#line 171
enum {
#line 171
	_OPTION_INIT=255,
#line 171
	#line 56 "cmdline.opt"

#line 56
	OPTION_CONFIG,
#line 68 "cmdline.opt"

#line 68
	OPTION_STDERR,
#line 74 "cmdline.opt"

#line 74
	OPTION_SYSLOG,
#line 94 "cmdline.opt"

#line 94
	OPTION_NO_TRANSCRIPT,
#line 107 "cmdline.opt"

#line 107
	OPTION_SOURCE_INFO,
#line 113 "cmdline.opt"

#line 113
	OPTION_TRACE_GRAMMAR,
#line 119 "cmdline.opt"

#line 119
	OPTION_TRACE_LEX,
#line 127 "cmdline.opt"

#line 127
	OPTION_CONFIG_HELP,
#line 136 "cmdline.opt"

#line 136
	OPTION_PREPROCESSOR,
#line 142 "cmdline.opt"

#line 142
	OPTION_NO_PREPROCESSOR,
#line 171 "cmdline.opt"

#line 171
	OPTION_USAGE,

#line 171 "cmdline.opt"
	MAX_OPTION
#line 171
};
#line 171
#ifdef HAVE_GETOPT_LONG
#line 171
static struct option long_options[] = {
#line 171
	#line 42 "cmdline.opt"

#line 42
	{ "lint", no_argument, 0, 't' },
#line 48 "cmdline.opt"

#line 48
	{ "inetd", no_argument, 0, 'i' },
#line 56 "cmdline.opt"

#line 56
	{ "config", required_argument, 0, OPTION_CONFIG },
#line 62 "cmdline.opt"

#line 62
	{ "foreground", no_argument, 0, 'f' },
#line 68 "cmdline.opt"

#line 68
	{ "stderr", no_argument, 0, OPTION_STDERR },
#line 74 "cmdline.opt"

#line 74
	{ "syslog", no_argument, 0, OPTION_SYSLOG },
#line 80 "cmdline.opt"

#line 80
	{ "single-process", no_argument, 0, 's' },
#line 88 "cmdline.opt"

#line 88
	{ "transcript", no_argument, 0, 'T' },
#line 94 "cmdline.opt"

#line 94
	{ "no-transcript", no_argument, 0, OPTION_NO_TRANSCRIPT },
#line 100 "cmdline.opt"

#line 100
	{ "debug", required_argument, 0, 'x' },
#line 107 "cmdline.opt"

#line 107
	{ "source-info", no_argument, 0, OPTION_SOURCE_INFO },
#line 113 "cmdline.opt"

#line 113
	{ "trace-grammar", no_argument, 0, OPTION_TRACE_GRAMMAR },
#line 119 "cmdline.opt"

#line 119
	{ "trace-lex", no_argument, 0, OPTION_TRACE_LEX },
#line 127 "cmdline.opt"

#line 127
	{ "config-help", no_argument, 0, OPTION_CONFIG_HELP },
#line 136 "cmdline.opt"

#line 136
	{ "preprocessor", required_argument, 0, OPTION_PREPROCESSOR },
#line 142 "cmdline.opt"

#line 142
	{ "no-preprocessor", no_argument, 0, OPTION_NO_PREPROCESSOR },
#line 148 "cmdline.opt"

#line 148
	{ "include-dir", required_argument, 0, 'I' },
#line 154 "cmdline.opt"

#line 154
	{ "define", required_argument, 0, 'D' },
#line 171 "cmdline.opt"

#line 171
	{ "help", no_argument, 0, 'h' },
#line 171 "cmdline.opt"

#line 171
	{ "usage", no_argument, 0, OPTION_USAGE },
#line 171 "cmdline.opt"

#line 171
	{ "version", no_argument, 0, 'V' },

#line 171 "cmdline.opt"
	{0, 0, 0, 0}
#line 171
};
#line 171
#endif
#line 171
static struct opthelp {
#line 171
	const char *opt;
#line 171
	const char *arg;
#line 171
	int is_optional;
#line 171
	const char *descr;
#line 171
} opthelp[] = {
#line 171
	#line 35 "cmdline.opt"

#line 35
	{ NULL, NULL, 0, N_("Select program mode") },
#line 38 "cmdline.opt"

#line 38
	{
#line 38
#ifdef HAVE_GETOPT_LONG
#line 38
	  "-E",
#line 38
#else
#line 38
	  "-E",
#line 38
#endif
#line 38
				   NULL, 0, N_("Preprocess configuration file and exit") },
#line 44 "cmdline.opt"

#line 44
	{
#line 44
#ifdef HAVE_GETOPT_LONG
#line 44
	  "-t, --lint",
#line 44
#else
#line 44
	  "-t",
#line 44
#endif
#line 44
				   NULL, 0, N_("Check configuration file syntax and exit.") },
#line 50 "cmdline.opt"

#line 50
	{
#line 50
#ifdef HAVE_GETOPT_LONG
#line 50
	  "-i, --inetd",
#line 50
#else
#line 50
	  "-i",
#line 50
#endif
#line 50
				   NULL, 0, N_("Inetd mode.") },
#line 54 "cmdline.opt"

#line 54
	{ NULL, NULL, 0, N_("Modifiers") },
#line 58 "cmdline.opt"

#line 58
	{
#line 58
#ifdef HAVE_GETOPT_LONG
#line 58
	  "--config",
#line 58
#else
#line 58
	  "",
#line 58
#endif
#line 58
				   N_("FILE"), 0, N_("Read this configuration file.") },
#line 64 "cmdline.opt"

#line 64
	{
#line 64
#ifdef HAVE_GETOPT_LONG
#line 64
	  "-f, --foreground",
#line 64
#else
#line 64
	  "-f",
#line 64
#endif
#line 64
				   NULL, 0, N_("Operate in foreground.") },
#line 70 "cmdline.opt"

#line 70
	{
#line 70
#ifdef HAVE_GETOPT_LONG
#line 70
	  "--stderr",
#line 70
#else
#line 70
	  "",
#line 70
#endif
#line 70
				   NULL, 0, N_("Output diagnostic to stderr.") },
#line 76 "cmdline.opt"

#line 76
	{
#line 76
#ifdef HAVE_GETOPT_LONG
#line 76
	  "--syslog",
#line 76
#else
#line 76
	  "",
#line 76
#endif
#line 76
				   NULL, 0, N_("Output diagnostic to syslog (default).") },
#line 82 "cmdline.opt"

#line 82
	{
#line 82
#ifdef HAVE_GETOPT_LONG
#line 82
	  "-s, --single-process",
#line 82
#else
#line 82
	  "-s",
#line 82
#endif
#line 82
				   NULL, 0, N_("Single-process mode.") },
#line 86 "cmdline.opt"

#line 86
	{ NULL, NULL, 0, N_("Debugging") },
#line 90 "cmdline.opt"

#line 90
	{
#line 90
#ifdef HAVE_GETOPT_LONG
#line 90
	  "-T, --transcript",
#line 90
#else
#line 90
	  "-T",
#line 90
#endif
#line 90
				   NULL, 0, N_("Enable session transcript.") },
#line 96 "cmdline.opt"

#line 96
	{
#line 96
#ifdef HAVE_GETOPT_LONG
#line 96
	  "--no-transcript",
#line 96
#else
#line 96
	  "",
#line 96
#endif
#line 96
				   NULL, 0, N_("Disable session transcript.") },
#line 102 "cmdline.opt"

#line 102
	{
#line 102
#ifdef HAVE_GETOPT_LONG
#line 102
	  "-x, --debug",
#line 102
#else
#line 102
	  "-x",
#line 102
#endif
#line 102
				   N_("LEVEL-SPEC"), 0, N_("Set debug verbosity level.") },
#line 109 "cmdline.opt"

#line 109
	{
#line 109
#ifdef HAVE_GETOPT_LONG
#line 109
	  "--source-info",
#line 109
#else
#line 109
	  "",
#line 109
#endif
#line 109
				   NULL, 0, N_("Include source line information in the debugging output.") },
#line 115 "cmdline.opt"

#line 115
	{
#line 115
#ifdef HAVE_GETOPT_LONG
#line 115
	  "--trace-grammar",
#line 115
#else
#line 115
	  "",
#line 115
#endif
#line 115
				   NULL, 0, N_("Trace parsing of configuration file.") },
#line 121 "cmdline.opt"

#line 121
	{
#line 121
#ifdef HAVE_GETOPT_LONG
#line 121
	  "--trace-lex",
#line 121
#else
#line 121
	  "",
#line 121
#endif
#line 121
				   NULL, 0, N_("Trace config file lexer.") },
#line 125 "cmdline.opt"

#line 125
	{ NULL, NULL, 0, N_("Additional help") },
#line 129 "cmdline.opt"

#line 129
	{
#line 129
#ifdef HAVE_GETOPT_LONG
#line 129
	  "--config-help",
#line 129
#else
#line 129
	  "",
#line 129
#endif
#line 129
				   NULL, 0, N_("Show configuration file summary.") },
#line 134 "cmdline.opt"

#line 134
	{ NULL, NULL, 0, N_("Preprocessor control") },
#line 138 "cmdline.opt"

#line 138
	{
#line 138
#ifdef HAVE_GETOPT_LONG
#line 138
	  "--preprocessor",
#line 138
#else
#line 138
	  "",
#line 138
#endif
#line 138
				   N_("PROG"), 0, N_("Use PROG as a preprocessor for config file.") },
#line 144 "cmdline.opt"

#line 144
	{
#line 144
#ifdef HAVE_GETOPT_LONG
#line 144
	  "--no-preprocessor",
#line 144
#else
#line 144
	  "",
#line 144
#endif
#line 144
				   NULL, 0, N_("Do not use external preprocessor.") },
#line 150 "cmdline.opt"

#line 150
	{
#line 150
#ifdef HAVE_GETOPT_LONG
#line 150
	  "-I, --include-dir",
#line 150
#else
#line 150
	  "-I",
#line 150
#endif
#line 150
				   N_("DIR"), 0, N_("Add the directory DIR to the list of directories to be searched for preprocessor include files") },
#line 156 "cmdline.opt"

#line 156
	{
#line 156
#ifdef HAVE_GETOPT_LONG
#line 156
	  "-D, --define",
#line 156
#else
#line 156
	  "-D",
#line 156
#endif
#line 156
				   N_("SYMBOL[=VALUE]"), 0, N_("define a preprocessor symbol") },
#line 171 "cmdline.opt"

#line 171
	{ NULL, NULL, 0, N_("Other options") },
#line 171 "cmdline.opt"

#line 171
	{
#line 171
#ifdef HAVE_GETOPT_LONG
#line 171
	  "-h, --help",
#line 171
#else
#line 171
	  "-h",
#line 171
#endif
#line 171
				   NULL, 0, N_("Give this help list") },
#line 171 "cmdline.opt"

#line 171
	{
#line 171
#ifdef HAVE_GETOPT_LONG
#line 171
	  "--usage",
#line 171
#else
#line 171
	  "",
#line 171
#endif
#line 171
				   NULL, 0, N_("Give a short usage message") },
#line 171 "cmdline.opt"

#line 171
	{
#line 171
#ifdef HAVE_GETOPT_LONG
#line 171
	  "-V, --version",
#line 171
#else
#line 171
	  "-V",
#line 171
#endif
#line 171
				   NULL, 0, N_("Print program version") },

#line 171 "cmdline.opt"
};
#line 27 "cmdline.opt"

#line 27
const char *program_version = "dicod" " (" PACKAGE_NAME ") " PACKAGE_VERSION;
#line 27
static char doc[] = N_("GNU dictionary server");
#line 27
static char args_doc[] = "";
#line 27
const char *program_bug_address = "<" PACKAGE_BUGREPORT ">";
#line 27
struct grecs_node *cmdline_tree;
#line 27

#line 27
#define DESCRCOLUMN 30
#line 27
#define RMARGIN 79
#line 27
#define GROUPCOLUMN 2
#line 27
#define USAGECOLUMN 13
#line 27

#line 27
static void
#line 27
indent (size_t start, size_t col)
#line 27
{
#line 27
  for (; start < col; start++)
#line 27
    putchar (' ');
#line 27
}
#line 27

#line 27
static void
#line 27
print_option_descr (const char *descr, size_t lmargin, size_t rmargin)
#line 27
{
#line 27
  while (*descr)
#line 27
    {
#line 27
      size_t s = 0;
#line 27
      size_t i;
#line 27
      size_t width = rmargin - lmargin;
#line 27

#line 27
      for (i = 0; ; i++)
#line 27
	{
#line 27
	  if (descr[i] == 0 || descr[i] == ' ' || descr[i] == '\t')
#line 27
	    {
#line 27
	      if (i > width)
#line 27
		break;
#line 27
	      s = i;
#line 27
	      if (descr[i] == 0)
#line 27
		break;
#line 27
	    }
#line 27
	}
#line 27
      printf ("%*.*s\n", s, s, descr);
#line 27
      descr += s;
#line 27
      if (*descr)
#line 27
	{
#line 27
	  indent (0, lmargin);
#line 27
	  descr++;
#line 27
	}
#line 27
    }
#line 27
}
#line 27

#line 27
#define NOPTHELP (sizeof (opthelp) / sizeof (opthelp[0]))
#line 27

#line 27
static int
#line 27
optcmp (const void *a, const void *b)
#line 27
{
#line 27
  struct opthelp const *ap = (struct opthelp const *)a;
#line 27
  struct opthelp const *bp = (struct opthelp const *)b;
#line 27
  const char *opta, *optb;
#line 27
  size_t alen, blen;
#line 27

#line 27
  for (opta = ap->opt; *opta == '-'; opta++)
#line 27
    ;
#line 27
  alen = strcspn (opta, ",");
#line 27
  
#line 27
  for (optb = bp->opt; *optb == '-'; optb++)
#line 27
    ;
#line 27
  blen = strcspn (optb, ",");
#line 27

#line 27
  if (alen > blen)
#line 27
    blen = alen;
#line 27
  
#line 27
  return strncmp (opta, optb, blen);
#line 27
}
#line 27

#line 27
static void
#line 27
sort_options (int start, int count)
#line 27
{
#line 27
  qsort (opthelp + start, count, sizeof (opthelp[0]), optcmp);
#line 27
}
#line 27

#line 27
static int
#line 27
sort_group (int start)
#line 27
{
#line 27
  int i;
#line 27
  
#line 27
  for (i = start; i < NOPTHELP && opthelp[i].opt; i++)
#line 27
    ;
#line 27
  sort_options (start, i - start);
#line 27
  return i + 1;
#line 27
}
#line 27

#line 27
static void
#line 27
sort_opthelp (void)
#line 27
{
#line 27
  int start;
#line 27

#line 27
  for (start = 0; start < NOPTHELP; )
#line 27
    {
#line 27
      if (!opthelp[start].opt)
#line 27
	start = sort_group (start + 1);
#line 27
      else 
#line 27
	start = sort_group (start);
#line 27
    }
#line 27
}
#line 27

#line 27
void (*print_help_hook) (FILE *stream);
#line 27

#line 27
void
#line 27
print_help (void)
#line 27
{
#line 27
  unsigned i;
#line 27
  int argsused = 0;
#line 27

#line 27
  printf ("%s %s [%s]... %s\n", _("Usage:"), "dicod", _("OPTION"),
#line 27
	  args_doc[0] ? gettext (args_doc) : args_doc);
#line 27
  if (doc[0])
#line 27
    print_option_descr(gettext (doc), 0, RMARGIN);
#line 27
  putchar ('\n');
#line 27

#line 27
  sort_opthelp ();
#line 27
  for (i = 0; i < NOPTHELP; i++)
#line 27
    {
#line 27
      unsigned n;
#line 27
      if (opthelp[i].opt)
#line 27
	{
#line 27
	  n = printf ("  %s", opthelp[i].opt);
#line 27
	  if (opthelp[i].arg)
#line 27
	    {
#line 27
	      char *cb, *ce;
#line 27
	      argsused = 1;
#line 27
	      if (strlen (opthelp[i].opt) == 2)
#line 27
		{
#line 27
		  if (!opthelp[i].is_optional)
#line 27
		    {
#line 27
		      putchar (' ');
#line 27
		      n++;
#line 27
		    }
#line 27
		}
#line 27
	      else
#line 27
		{
#line 27
		  putchar ('=');
#line 27
		  n++;
#line 27
		}
#line 27
	      if (opthelp[i].is_optional)
#line 27
		{
#line 27
		  cb = "[";
#line 27
		  ce = "]";
#line 27
		}
#line 27
	      else
#line 27
		cb = ce = "";
#line 27
	      n += printf ("%s%s%s", cb, gettext (opthelp[i].arg), ce);
#line 27
	    }
#line 27
	  if (n >= DESCRCOLUMN)
#line 27
	    {
#line 27
	      putchar ('\n');
#line 27
	      n = 0;
#line 27
	    }
#line 27
	  indent (n, DESCRCOLUMN);
#line 27
	  print_option_descr (gettext (opthelp[i].descr), DESCRCOLUMN, RMARGIN);
#line 27
	}
#line 27
      else
#line 27
	{
#line 27
	  if (i)
#line 27
	    putchar ('\n');
#line 27
	  indent (0, GROUPCOLUMN);
#line 27
	  print_option_descr (gettext (opthelp[i].descr),
#line 27
			      GROUPCOLUMN, RMARGIN);
#line 27
	  putchar ('\n');
#line 27
	}
#line 27
    }
#line 27

#line 27
  putchar ('\n');
#line 27
  if (argsused)
#line 27
    {
#line 27
      print_option_descr (_("Mandatory or optional arguments to long options are also mandatory or optional for any corresponding short options."), 0, RMARGIN);
#line 27
      putchar ('\n');
#line 27
    }
#line 27
 
#line 27
  if (print_help_hook)
#line 27
    print_help_hook (stdout);
#line 27
    
#line 27
 /* TRANSLATORS: The placeholder indicates the bug-reporting address
    for this package.  Please add _another line_ saying
    "Report translation bugs to <...>\n" with the address for translation
    bugs (typically your translation team's web or email address).  */
#line 27
  printf (_("Report bugs to %s.\n"), program_bug_address);
#line 27
  
#line 27
#ifdef PACKAGE_URL
#line 27
  printf (_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
#line 27
#else
#line 27
  printf (_("%s home page: <http://www.gnu.org/software/%s/>\n"),
#line 27
	  PACKAGE_NAME, PACKAGE);
#line 27
#endif
#line 27
  fputs (_("General help using GNU software: <http://www.gnu.org/gethelp/>\n"),
#line 27
	 stdout);
#line 27
}
#line 27

#line 27
static int
#line 27
cmpidx_short (const void *a, const void *b)
#line 27
{
#line 27
  unsigned const *ai = (unsigned const *)a;
#line 27
  unsigned const *bi = (unsigned const *)b;
#line 27

#line 27
  return opthelp[*ai].opt[1] - opthelp[*bi].opt[1];
#line 27
}
#line 27
  
#line 27
#ifdef HAVE_GETOPT_LONG
#line 27
static int
#line 27
cmpidx_long (const void *a, const void *b)
#line 27
{
#line 27
  unsigned const *ai = (unsigned const *)a;
#line 27
  unsigned const *bi = (unsigned const *)b;
#line 27
  struct opthelp const *ap = opthelp + *ai;
#line 27
  struct opthelp const *bp = opthelp + *bi;
#line 27
  char const *opta, *optb;
#line 27
  size_t lena, lenb;
#line 27

#line 27
  if (ap->opt[1] == '-')
#line 27
    opta = ap->opt;
#line 27
  else
#line 27
    opta = ap->opt + 4;
#line 27
  lena = strcspn (opta, ",");
#line 27
  
#line 27
  if (bp->opt[1] == '-')
#line 27
    optb = bp->opt;
#line 27
  else
#line 27
    optb = bp->opt + 4;
#line 27
  lenb = strcspn (optb, ",");
#line 27
  return strncmp (opta, optb, lena > lenb ? lenb : lena);
#line 27
}
#line 27
#endif
#line 27

#line 27
void
#line 27
print_usage (void)
#line 27
{
#line 27
  unsigned i;
#line 27
  unsigned n;
#line 27
  char buf[RMARGIN+1];
#line 27
  unsigned *idxbuf;
#line 27
  unsigned nidx;
#line 27
  
#line 27
#define FLUSH                        do                                   {                              	  buf[n] = 0;              	  printf ("%s\n", buf);    	  n = USAGECOLUMN;         	  memset (buf, ' ', n);        }                                while (0)
#line 27
#define ADDC(c)   do { if (n == RMARGIN) FLUSH; buf[n++] = c; } while (0)
#line 27

#line 27
  idxbuf = calloc (NOPTHELP, sizeof (idxbuf[0]));
#line 27
  if (!idxbuf)
#line 27
    {
#line 27
      fprintf (stderr, "not enough memory");
#line 27
      abort ();
#line 27
    }
#line 27

#line 27
  n = snprintf (buf, sizeof buf, "%s %s ", _("Usage:"), "dicod");
#line 27

#line 27
  /* Print a list of short options without arguments. */
#line 27
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 27
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
#line 27
	  && opthelp[i].arg == NULL)
#line 27
	idxbuf[nidx++] = i;
#line 27

#line 27
  if (nidx)
#line 27
    {
#line 27
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);
#line 27

#line 27
      ADDC ('[');
#line 27
      ADDC ('-');
#line 27
      for (i = 0; i < nidx; i++)
#line 27
	{
#line 27
	  ADDC (opthelp[idxbuf[i]].opt[1]);
#line 27
	}
#line 27
      ADDC (']');
#line 27
    }
#line 27

#line 27
  /* Print a list of short options with arguments. */
#line 27
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 27
    {
#line 27
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
#line 27
	  && opthelp[i].arg)
#line 27
	idxbuf[nidx++] = i;
#line 27
    }
#line 27

#line 27
  if (nidx)
#line 27
    {
#line 27
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);
#line 27
    
#line 27
      for (i = 0; i < nidx; i++)
#line 27
	{
#line 27
	  struct opthelp *opt = opthelp + idxbuf[i];
#line 27
	  size_t len = 5 + strlen (opt->arg)
#line 27
	                 + (opt->is_optional ? 2 : 1);
#line 27
      
#line 27
	  if (n + len > RMARGIN) FLUSH;
#line 27
	  buf[n++] = ' ';
#line 27
	  buf[n++] = '[';
#line 27
	  buf[n++] = '-';
#line 27
	  buf[n++] = opt->opt[1];
#line 27
	  if (opt->is_optional)
#line 27
	    {
#line 27
	      buf[n++] = '[';
#line 27
	      strcpy (&buf[n], opt->arg);
#line 27
	      n += strlen (opt->arg);
#line 27
	      buf[n++] = ']';
#line 27
	    }
#line 27
	  else
#line 27
	    {
#line 27
	      buf[n++] = ' ';
#line 27
	      strcpy (&buf[n], opt->arg);
#line 27
	      n += strlen (opt->arg);
#line 27
	    }
#line 27
	  buf[n++] = ']';
#line 27
	}
#line 27
    }
#line 27
  
#line 27
#ifdef HAVE_GETOPT_LONG
#line 27
  /* Print a list of long options */
#line 27
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 27
    {
#line 27
      if (opthelp[i].opt && opthelp[i].descr
#line 27
	  &&  (opthelp[i].opt[1] == '-' || opthelp[i].opt[2] == ','))
#line 27
	idxbuf[nidx++] = i;
#line 27
    }
#line 27

#line 27
  if (nidx)
#line 27
    {
#line 27
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_long);
#line 27
	
#line 27
      for (i = 0; i < nidx; i++)
#line 27
	{
#line 27
	  struct opthelp *opt = opthelp + idxbuf[i];
#line 27
	  size_t len;
#line 27
	  const char *longopt;
#line 27
	  
#line 27
	  if (opt->opt[1] == '-')
#line 27
	    longopt = opt->opt;
#line 27
	  else if (opt->opt[2] == ',')
#line 27
	    longopt = opt->opt + 4;
#line 27
	  else
#line 27
	    continue;
#line 27

#line 27
	  len = 3 + strlen (longopt)
#line 27
	          + (opt->arg ? 1 + strlen (opt->arg)
#line 27
		  + (opt->is_optional ? 2 : 0) : 0);
#line 27
	  if (n + len > RMARGIN) FLUSH;
#line 27
	  buf[n++] = ' ';
#line 27
	  buf[n++] = '[';
#line 27
	  strcpy (&buf[n], longopt);
#line 27
	  n += strlen (longopt);
#line 27
	  if (opt->arg)
#line 27
	    {
#line 27
	      buf[n++] = '=';
#line 27
	      if (opt->is_optional)
#line 27
		{
#line 27
		  buf[n++] = '[';
#line 27
		  strcpy (&buf[n], opt->arg);
#line 27
		  n += strlen (opt->arg);
#line 27
		  buf[n++] = ']';
#line 27
		}
#line 27
	      else
#line 27
		{
#line 27
		  strcpy (&buf[n], opt->arg);
#line 27
		  n += strlen (opt->arg);
#line 27
		}
#line 27
	    }
#line 27
	  buf[n++] = ']';
#line 27
	}
#line 27
    }
#line 27
#endif
#line 27
  FLUSH;
#line 27
  free (idxbuf);
#line 27
}
#line 27

#line 27
const char version_etc_copyright[] =
#line 27
  /* Do *not* mark this string for translation.  First %s is a copyright
     symbol suitable for this locale, and second %s are the copyright
     years.  */
#line 27
  "Copyright %s %s %s";
#line 27

#line 27
void
#line 27
print_version_only(const char *program_version, FILE *stream)
#line 27
{
#line 27
  fprintf (stream, "%s\n", program_version);
#line 27
  /* TRANSLATORS: Translate "(C)" to the copyright symbol
     (C-in-a-circle), if this symbol is available in the user's
     locale.  Otherwise, do not translate "(C)"; leave it as-is.  */
#line 27
  fprintf (stream, version_etc_copyright, _("(C)"),
#line 27
	   "2005-2012",
#line 27
	   "Free Software Foundation, Inc.");
#line 27
  fputc ('\n', stream);
#line 27
}
#line 27

#line 27

#line 27

#line 27
void (*print_version_hook)(FILE *stream);
#line 27

#line 27
void
#line 27
print_version(const char *program_version, FILE *stream)
#line 27
{
#line 27
  
#line 27
  print_version_only(program_version, stream);
#line 27

#line 27
  fputs (_("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n"),
#line 27
	 stream);
#line 27
  if (print_version_hook)
#line 27
    print_version_hook (stream);
#line 27
		
#line 27
}
#line 27

#line 171 "cmdline.opt"

#line 171


void
get_options(int argc, char *argv[], struct dicod_conf_override *conf)
{
    
#line 176
 {
#line 176
  int c;
#line 176

#line 176
#ifdef HAVE_GETOPT_LONG
#line 176
  while ((c = getopt_long(argc, argv, "EtifsTx:I:D:hV",
#line 176
			  long_options, NULL)) != EOF)
#line 176
#else
#line 176
  while ((c = getopt(argc, argv, "EtifsTx:I:D:hV")) != EOF)
#line 176
#endif
#line 176
    {
#line 176
      switch (c)
#line 176
	{
#line 176
	default:
#line 176
	   exit(EX_USAGE);	   exit(EX_USAGE);
#line 176
	#line 38 "cmdline.opt"
	 case 'E':
#line 38
	  {
#line 38

   mode = MODE_PREPROC;

#line 40
	     break;
#line 40
	  }
#line 44 "cmdline.opt"
	 case 't':
#line 44
	  {
#line 44

   config_lint_option = 1;

#line 46
	     break;
#line 46
	  }
#line 50 "cmdline.opt"
	 case 'i':
#line 50
	  {
#line 50

   mode = MODE_INETD;

#line 52
	     break;
#line 52
	  }
#line 58 "cmdline.opt"
	 case OPTION_CONFIG:
#line 58
	  {
#line 58

   config_file = optarg;

#line 60
	     break;
#line 60
	  }
#line 64 "cmdline.opt"
	 case 'f':
#line 64
	  {
#line 64

   foreground = 1;

#line 66
	     break;
#line 66
	  }
#line 70 "cmdline.opt"
	 case OPTION_STDERR:
#line 70
	  {
#line 70

   log_to_stderr = 1;

#line 72
	     break;
#line 72
	  }
#line 76 "cmdline.opt"
	 case OPTION_SYSLOG:
#line 76
	  {
#line 76

   log_to_stderr = 0;

#line 78
	     break;
#line 78
	  }
#line 82 "cmdline.opt"
	 case 's':
#line 82
	  {
#line 82

   single_process = 1;

#line 84
	     break;
#line 84
	  }
#line 90 "cmdline.opt"
	 case 'T':
#line 90
	  {
#line 90

   conf->transcript = 1;

#line 92
	     break;
#line 92
	  }
#line 96 "cmdline.opt"
	 case OPTION_NO_TRANSCRIPT:
#line 96
	  {
#line 96

   conf->transcript = 0;

#line 98
	     break;
#line 98
	  }
#line 102 "cmdline.opt"
	 case 'x':
#line 102
	  {
#line 102

   debug_level_str = optarg;
   debug_level = atoi(optarg);

#line 105
	     break;
#line 105
	  }
#line 109 "cmdline.opt"
	 case OPTION_SOURCE_INFO:
#line 109
	  {
#line 109

  debug_source_info = 1;

#line 111
	     break;
#line 111
	  }
#line 115 "cmdline.opt"
	 case OPTION_TRACE_GRAMMAR:
#line 115
	  {
#line 115

   grecs_gram_trace(1);

#line 117
	     break;
#line 117
	  }
#line 121 "cmdline.opt"
	 case OPTION_TRACE_LEX:
#line 121
	  {
#line 121

   grecs_lex_trace(1);

#line 123
	     break;
#line 123
	  }
#line 129 "cmdline.opt"
	 case OPTION_CONFIG_HELP:
#line 129
	  {
#line 129

   config_help();
   exit(0);

#line 132
	     break;
#line 132
	  }
#line 138 "cmdline.opt"
	 case OPTION_PREPROCESSOR:
#line 138
	  {
#line 138

   grecs_preprocessor = optarg;

#line 140
	     break;
#line 140
	  }
#line 144 "cmdline.opt"
	 case OPTION_NO_PREPROCESSOR:
#line 144
	  {
#line 144

   grecs_preprocessor = NULL;

#line 146
	     break;
#line 146
	  }
#line 150 "cmdline.opt"
	 case 'I':
#line 150
	  {
#line 150

   grecs_preproc_add_include_dir(optarg); 

#line 152
	     break;
#line 152
	  }
#line 156 "cmdline.opt"
	 case 'D':
#line 156
	  {
#line 156

   char *p;

   if (!pp_cmd_acc)
       pp_cmd_acc = grecs_txtacc_create();
   grecs_txtacc_grow(pp_cmd_acc, " \"-D", 4);
   for (p = optarg; *p; p++) {
       if (*p == '\\' || *p == '"')
	   grecs_txtacc_grow_char(pp_cmd_acc, '\\');
       grecs_txtacc_grow_char(pp_cmd_acc, *p);
   }
   grecs_txtacc_grow_char(pp_cmd_acc, '"');			

#line 168
	     break;
#line 168
	  }
#line 171 "cmdline.opt"
	 case 'h':
#line 171
	  {
#line 171

#line 171
		print_help ();
#line 171
		exit (0);
#line 171
	 
#line 171
	     break;
#line 171
	  }
#line 171 "cmdline.opt"
	 case OPTION_USAGE:
#line 171
	  {
#line 171

#line 171
		print_usage ();
#line 171
		exit (0);
#line 171
	 
#line 171
	     break;
#line 171
	  }
#line 171 "cmdline.opt"
	 case 'V':
#line 171
	  {
#line 171

#line 171
		/* Give version */
#line 171
		print_version(program_version, stdout);
#line 171
		exit (0);
#line 171
	 
#line 171
	     break;
#line 171
	  }

#line 176 "cmdline.opt"
	}
#line 176
    }
#line 176
  
#line 176
    if (optind < argc) {
#line 176
	fprintf(stderr, "%s: unexpected arguments\n", argv[0]);
#line 176
	exit(EX_USAGE);
#line 176
    }
#line 176

#line 176
  if (cmdline_tree)
#line 176
    {
#line 176
      struct grecs_node *rn = grecs_node_create(grecs_node_root, NULL);
#line 176
      rn->down = cmdline_tree;
#line 176
      cmdline_tree = rn;
#line 176
    }
#line 176
 }
#line 176

    if (pp_cmd_acc && grecs_preprocessor) {
	char *args, *cmd;

	grecs_txtacc_grow_char(pp_cmd_acc, 0);
	args = grecs_txtacc_finish(pp_cmd_acc, 0);
	cmd = grecs_malloc(strlen(grecs_preprocessor) +
			   strlen(args) + 1);
	strcpy(cmd, grecs_preprocessor);
	strcat(cmd, args);
	grecs_preprocessor = cmd;
    }
    grecs_txtacc_free(pp_cmd_acc);
}
