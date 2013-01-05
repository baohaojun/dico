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

#include <dico-priv.h>
#include <getopt.h>
#include <sysexits.h>

#line 21 "cmdline.opt"

#line 21
void print_help(void);
#line 21
void print_usage(void);
#line 193 "cmdline.opt"

#line 193
#include <grecs.h>
#line 193
/* Option codes */
#line 193
enum {
#line 193
	_OPTION_INIT=255,
#line 193
	#line 31 "cmdline.opt"

#line 31
	OPTION_HOST,
#line 51 "cmdline.opt"

#line 51
	OPTION_SOURCE,
#line 75 "cmdline.opt"

#line 75
	OPTION_LEVDIST,
#line 77 "cmdline.opt"

#line 77
	OPTION_LEVENSHTEIN_DISTANCE,
#line 130 "cmdline.opt"

#line 130
	OPTION_SASL,
#line 136 "cmdline.opt"

#line 136
	OPTION_NOSASL,
#line 150 "cmdline.opt"

#line 150
	OPTION_PASSWORD,
#line 155 "cmdline.opt"

#line 155
	OPTION_AUTOLOGIN,
#line 180 "cmdline.opt"

#line 180
	OPTION_TIME_STAMP,
#line 187 "cmdline.opt"

#line 187
	OPTION_SOURCE_INFO,
#line 193 "cmdline.opt"

#line 193
	OPTION_USAGE,

#line 193 "cmdline.opt"
	MAX_OPTION
#line 193
};
#line 193
#ifdef HAVE_GETOPT_LONG
#line 193
static struct option long_options[] = {
#line 193
	#line 31 "cmdline.opt"

#line 31
	{ "host", required_argument, 0, OPTION_HOST },
#line 37 "cmdline.opt"

#line 37
	{ "port", required_argument, 0, 'p' },
#line 45 "cmdline.opt"

#line 45
	{ "database", required_argument, 0, 'd' },
#line 51 "cmdline.opt"

#line 51
	{ "source", required_argument, 0, OPTION_SOURCE },
#line 62 "cmdline.opt"

#line 62
	{ "match", no_argument, 0, 'm' },
#line 68 "cmdline.opt"

#line 68
	{ "strategy", required_argument, 0, 's' },
#line 75 "cmdline.opt"

#line 75
	{ "levdist", required_argument, 0, OPTION_LEVDIST },
#line 77 "cmdline.opt"

#line 77
	{ "levenshtein-distance", required_argument, 0, OPTION_LEVENSHTEIN_DISTANCE },
#line 85 "cmdline.opt"

#line 85
	{ "dbs", no_argument, 0, 'D' },
#line 91 "cmdline.opt"

#line 91
	{ "strategies", no_argument, 0, 'S' },
#line 97 "cmdline.opt"

#line 97
	{ "serverhelp", no_argument, 0, 'H' },
#line 103 "cmdline.opt"

#line 103
	{ "info", required_argument, 0, 'i' },
#line 110 "cmdline.opt"

#line 110
	{ "serverinfo", no_argument, 0, 'I' },
#line 116 "cmdline.opt"

#line 116
	{ "quiet", no_argument, 0, 'q' },
#line 124 "cmdline.opt"

#line 124
	{ "noauth", no_argument, 0, 'a' },
#line 130 "cmdline.opt"

#line 130
	{ "sasl", no_argument, 0, OPTION_SASL },
#line 136 "cmdline.opt"

#line 136
	{ "nosasl", no_argument, 0, OPTION_NOSASL },
#line 142 "cmdline.opt"

#line 142
	{ "user", required_argument, 0, 'u' },
#line 148 "cmdline.opt"

#line 148
	{ "key", required_argument, 0, 'k' },
#line 150 "cmdline.opt"

#line 150
	{ "password", required_argument, 0, OPTION_PASSWORD },
#line 155 "cmdline.opt"

#line 155
	{ "autologin", required_argument, 0, OPTION_AUTOLOGIN },
#line 161 "cmdline.opt"

#line 161
	{ "client", required_argument, 0, 'c' },
#line 168 "cmdline.opt"

#line 168
	{ "transcript", no_argument, 0, 't' },
#line 174 "cmdline.opt"

#line 174
	{ "verbose", no_argument, 0, 'v' },
#line 180 "cmdline.opt"

#line 180
	{ "time-stamp", no_argument, 0, OPTION_TIME_STAMP },
#line 187 "cmdline.opt"

#line 187
	{ "source-info", no_argument, 0, OPTION_SOURCE_INFO },
#line 193 "cmdline.opt"

#line 193
	{ "help", no_argument, 0, 'h' },
#line 193 "cmdline.opt"

#line 193
	{ "usage", no_argument, 0, OPTION_USAGE },
#line 193 "cmdline.opt"

#line 193
	{ "version", no_argument, 0, 'V' },

#line 193 "cmdline.opt"
	{0, 0, 0, 0}
#line 193
};
#line 193
#endif
#line 193
static struct opthelp {
#line 193
	const char *opt;
#line 193
	const char *arg;
#line 193
	int is_optional;
#line 193
	const char *descr;
#line 193
} opthelp[] = {
#line 193
	#line 29 "cmdline.opt"

#line 29
	{ NULL, NULL, 0, N_("Server selection") },
#line 33 "cmdline.opt"

#line 33
	{
#line 33
#ifdef HAVE_GETOPT_LONG
#line 33
	  "--host",
#line 33
#else
#line 33
	  "",
#line 33
#endif
#line 33
				   N_("SERVER"), 0, N_("Connect to this server.") },
#line 39 "cmdline.opt"

#line 39
	{
#line 39
#ifdef HAVE_GETOPT_LONG
#line 39
	  "-p, --port",
#line 39
#else
#line 39
	  "-p",
#line 39
#endif
#line 39
				   N_("SERVICE"), 0, N_("Specify port to connect to.") },
#line 47 "cmdline.opt"

#line 47
	{
#line 47
#ifdef HAVE_GETOPT_LONG
#line 47
	  "-d, --database",
#line 47
#else
#line 47
	  "-d",
#line 47
#endif
#line 47
				   N_("NAME"), 0, N_("Select a database to search.") },
#line 53 "cmdline.opt"

#line 53
	{
#line 53
#ifdef HAVE_GETOPT_LONG
#line 53
	  "--source",
#line 53
#else
#line 53
	  "",
#line 53
#endif
#line 53
				   N_("ADDR"), 0, N_("Set a source address for TCP connections.") },
#line 60 "cmdline.opt"

#line 60
	{ NULL, NULL, 0, N_("Operation modes") },
#line 64 "cmdline.opt"

#line 64
	{
#line 64
#ifdef HAVE_GETOPT_LONG
#line 64
	  "-m, --match",
#line 64
#else
#line 64
	  "-m",
#line 64
#endif
#line 64
				   NULL, 0, N_("Match instead of define.") },
#line 70 "cmdline.opt"

#line 70
	{
#line 70
#ifdef HAVE_GETOPT_LONG
#line 70
	  "-s, --strategy",
#line 70
#else
#line 70
	  "-s",
#line 70
#endif
#line 70
				   N_("NAME"), 0, N_("Select a strategy for matching.  Implies --match.") },
#line 78 "cmdline.opt"

#line 78
	{
#line 78
#ifdef HAVE_GETOPT_LONG
#line 78
	  "--levdist, --levenshtein-distance",
#line 78
#else
#line 78
	  "",
#line 78
#endif
#line 78
				   N_("N"), 0, N_("Set maximum Levenshtein distance to N.") },
#line 87 "cmdline.opt"

#line 87
	{
#line 87
#ifdef HAVE_GETOPT_LONG
#line 87
	  "-D, --dbs",
#line 87
#else
#line 87
	  "-D",
#line 87
#endif
#line 87
				   NULL, 0, N_("Show available databases.") },
#line 93 "cmdline.opt"

#line 93
	{
#line 93
#ifdef HAVE_GETOPT_LONG
#line 93
	  "-S, --strategies",
#line 93
#else
#line 93
	  "-S",
#line 93
#endif
#line 93
				   NULL, 0, N_("Show available search strategies.") },
#line 99 "cmdline.opt"

#line 99
	{
#line 99
#ifdef HAVE_GETOPT_LONG
#line 99
	  "-H, --serverhelp",
#line 99
#else
#line 99
	  "-H",
#line 99
#endif
#line 99
				   NULL, 0, N_("show server help.") },
#line 105 "cmdline.opt"

#line 105
	{
#line 105
#ifdef HAVE_GETOPT_LONG
#line 105
	  "-i, --info",
#line 105
#else
#line 105
	  "-i",
#line 105
#endif
#line 105
				   N_("DBNAME"), 0, N_("Show information about database DBNAME.") },
#line 112 "cmdline.opt"

#line 112
	{
#line 112
#ifdef HAVE_GETOPT_LONG
#line 112
	  "-I, --serverinfo",
#line 112
#else
#line 112
	  "-I",
#line 112
#endif
#line 112
				   NULL, 0, N_("Show information about the server.") },
#line 118 "cmdline.opt"

#line 118
	{
#line 118
#ifdef HAVE_GETOPT_LONG
#line 118
	  "-q, --quiet",
#line 118
#else
#line 118
	  "-q",
#line 118
#endif
#line 118
				   NULL, 0, N_("Do not print the normal dico welcome.") },
#line 122 "cmdline.opt"

#line 122
	{ NULL, NULL, 0, N_("Authentication") },
#line 126 "cmdline.opt"

#line 126
	{
#line 126
#ifdef HAVE_GETOPT_LONG
#line 126
	  "-a, --noauth",
#line 126
#else
#line 126
	  "-a",
#line 126
#endif
#line 126
				   NULL, 0, N_("Disable authentication.") },
#line 132 "cmdline.opt"

#line 132
	{
#line 132
#ifdef HAVE_GETOPT_LONG
#line 132
	  "--sasl",
#line 132
#else
#line 132
	  "",
#line 132
#endif
#line 132
				   NULL, 0, N_("Enable SASL authentication (default).") },
#line 138 "cmdline.opt"

#line 138
	{
#line 138
#ifdef HAVE_GETOPT_LONG
#line 138
	  "--nosasl",
#line 138
#else
#line 138
	  "",
#line 138
#endif
#line 138
				   NULL, 0, N_("Disable SASL authentication.") },
#line 144 "cmdline.opt"

#line 144
	{
#line 144
#ifdef HAVE_GETOPT_LONG
#line 144
	  "-u, --user",
#line 144
#else
#line 144
	  "-u",
#line 144
#endif
#line 144
				   N_("NAME"), 0, N_("Set user name for authentication.") },
#line 151 "cmdline.opt"

#line 151
	{
#line 151
#ifdef HAVE_GETOPT_LONG
#line 151
	  "-k, --key, --password",
#line 151
#else
#line 151
	  "-k",
#line 151
#endif
#line 151
				   N_("STRING"), 0, N_("Set shared secret for authentication.") },
#line 157 "cmdline.opt"

#line 157
	{
#line 157
#ifdef HAVE_GETOPT_LONG
#line 157
	  "--autologin",
#line 157
#else
#line 157
	  "",
#line 157
#endif
#line 157
				   N_("NAME"), 0, N_("Set the name of autologin file to use.") },
#line 163 "cmdline.opt"

#line 163
	{
#line 163
#ifdef HAVE_GETOPT_LONG
#line 163
	  "-c, --client",
#line 163
#else
#line 163
	  "-c",
#line 163
#endif
#line 163
				   N_("STRING"), 0, N_("Additional text for client command.") },
#line 167 "cmdline.opt"

#line 167
	{ NULL, NULL, 0, N_("Debugging") },
#line 170 "cmdline.opt"

#line 170
	{
#line 170
#ifdef HAVE_GETOPT_LONG
#line 170
	  "-t, --transcript",
#line 170
#else
#line 170
	  "-t",
#line 170
#endif
#line 170
				   NULL, 0, N_("Enable session transcript.") },
#line 176 "cmdline.opt"

#line 176
	{
#line 176
#ifdef HAVE_GETOPT_LONG
#line 176
	  "-v, --verbose",
#line 176
#else
#line 176
	  "-v",
#line 176
#endif
#line 176
				   NULL, 0, N_("Increase debugging verbosity level.") },
#line 182 "cmdline.opt"

#line 182
	{
#line 182
#ifdef HAVE_GETOPT_LONG
#line 182
	  "--time-stamp",
#line 182
#else
#line 182
	  "",
#line 182
#endif
#line 182
				   NULL, 0, N_("Include time stamp in the debugging output.") },
#line 189 "cmdline.opt"

#line 189
	{
#line 189
#ifdef HAVE_GETOPT_LONG
#line 189
	  "--source-info",
#line 189
#else
#line 189
	  "",
#line 189
#endif
#line 189
				   NULL, 0, N_("Include source line information in the debugging output.") },
#line 193 "cmdline.opt"

#line 193
	{ NULL, NULL, 0, N_("Other options") },
#line 193 "cmdline.opt"

#line 193
	{
#line 193
#ifdef HAVE_GETOPT_LONG
#line 193
	  "-h, --help",
#line 193
#else
#line 193
	  "-h",
#line 193
#endif
#line 193
				   NULL, 0, N_("Give this help list") },
#line 193 "cmdline.opt"

#line 193
	{
#line 193
#ifdef HAVE_GETOPT_LONG
#line 193
	  "--usage",
#line 193
#else
#line 193
	  "",
#line 193
#endif
#line 193
				   NULL, 0, N_("Give a short usage message") },
#line 193 "cmdline.opt"

#line 193
	{
#line 193
#ifdef HAVE_GETOPT_LONG
#line 193
	  "-V, --version",
#line 193
#else
#line 193
	  "-V",
#line 193
#endif
#line 193
				   NULL, 0, N_("Print program version") },

#line 193 "cmdline.opt"
};
#line 21 "cmdline.opt"

#line 21
const char *program_version = "dico" " (" PACKAGE_NAME ") " PACKAGE_VERSION;
#line 21
static char doc[] = N_("GNU dictionary client program");
#line 21
static char args_doc[] = N_("[URL-or-WORD]");
#line 21
const char *program_bug_address = "<" PACKAGE_BUGREPORT ">";
#line 21
struct grecs_node *cmdline_tree;
#line 21

#line 21
#define DESCRCOLUMN 30
#line 21
#define RMARGIN 79
#line 21
#define GROUPCOLUMN 2
#line 21
#define USAGECOLUMN 13
#line 21

#line 21
static void
#line 21
indent (size_t start, size_t col)
#line 21
{
#line 21
  for (; start < col; start++)
#line 21
    putchar (' ');
#line 21
}
#line 21

#line 21
static void
#line 21
print_option_descr (const char *descr, size_t lmargin, size_t rmargin)
#line 21
{
#line 21
  while (*descr)
#line 21
    {
#line 21
      size_t s = 0;
#line 21
      size_t i;
#line 21
      size_t width = rmargin - lmargin;
#line 21

#line 21
      for (i = 0; ; i++)
#line 21
	{
#line 21
	  if (descr[i] == 0 || descr[i] == ' ' || descr[i] == '\t')
#line 21
	    {
#line 21
	      if (i > width)
#line 21
		break;
#line 21
	      s = i;
#line 21
	      if (descr[i] == 0)
#line 21
		break;
#line 21
	    }
#line 21
	}
#line 21
      printf ("%*.*s\n", s, s, descr);
#line 21
      descr += s;
#line 21
      if (*descr)
#line 21
	{
#line 21
	  indent (0, lmargin);
#line 21
	  descr++;
#line 21
	}
#line 21
    }
#line 21
}
#line 21

#line 21
#define NOPTHELP (sizeof (opthelp) / sizeof (opthelp[0]))
#line 21

#line 21
static int
#line 21
optcmp (const void *a, const void *b)
#line 21
{
#line 21
  struct opthelp const *ap = (struct opthelp const *)a;
#line 21
  struct opthelp const *bp = (struct opthelp const *)b;
#line 21
  const char *opta, *optb;
#line 21
  size_t alen, blen;
#line 21

#line 21
  for (opta = ap->opt; *opta == '-'; opta++)
#line 21
    ;
#line 21
  alen = strcspn (opta, ",");
#line 21
  
#line 21
  for (optb = bp->opt; *optb == '-'; optb++)
#line 21
    ;
#line 21
  blen = strcspn (optb, ",");
#line 21

#line 21
  if (alen > blen)
#line 21
    blen = alen;
#line 21
  
#line 21
  return strncmp (opta, optb, blen);
#line 21
}
#line 21

#line 21
static void
#line 21
sort_options (int start, int count)
#line 21
{
#line 21
  qsort (opthelp + start, count, sizeof (opthelp[0]), optcmp);
#line 21
}
#line 21

#line 21
static int
#line 21
sort_group (int start)
#line 21
{
#line 21
  int i;
#line 21
  
#line 21
  for (i = start; i < NOPTHELP && opthelp[i].opt; i++)
#line 21
    ;
#line 21
  sort_options (start, i - start);
#line 21
  return i + 1;
#line 21
}
#line 21

#line 21
static void
#line 21
sort_opthelp (void)
#line 21
{
#line 21
  int start;
#line 21

#line 21
  for (start = 0; start < NOPTHELP; )
#line 21
    {
#line 21
      if (!opthelp[start].opt)
#line 21
	start = sort_group (start + 1);
#line 21
      else 
#line 21
	start = sort_group (start);
#line 21
    }
#line 21
}
#line 21

#line 21
void (*print_help_hook) (FILE *stream);
#line 21

#line 21
void
#line 21
print_help (void)
#line 21
{
#line 21
  unsigned i;
#line 21
  int argsused = 0;
#line 21

#line 21
  printf ("%s %s [%s]... %s\n", _("Usage:"), "dico", _("OPTION"),
#line 21
	  args_doc[0] ? gettext (args_doc) : args_doc);
#line 21
  if (doc[0])
#line 21
    print_option_descr(gettext (doc), 0, RMARGIN);
#line 21
  putchar ('\n');
#line 21

#line 21
  sort_opthelp ();
#line 21
  for (i = 0; i < NOPTHELP; i++)
#line 21
    {
#line 21
      unsigned n;
#line 21
      if (opthelp[i].opt)
#line 21
	{
#line 21
	  n = printf ("  %s", opthelp[i].opt);
#line 21
	  if (opthelp[i].arg)
#line 21
	    {
#line 21
	      char *cb, *ce;
#line 21
	      argsused = 1;
#line 21
	      if (strlen (opthelp[i].opt) == 2)
#line 21
		{
#line 21
		  if (!opthelp[i].is_optional)
#line 21
		    {
#line 21
		      putchar (' ');
#line 21
		      n++;
#line 21
		    }
#line 21
		}
#line 21
	      else
#line 21
		{
#line 21
		  putchar ('=');
#line 21
		  n++;
#line 21
		}
#line 21
	      if (opthelp[i].is_optional)
#line 21
		{
#line 21
		  cb = "[";
#line 21
		  ce = "]";
#line 21
		}
#line 21
	      else
#line 21
		cb = ce = "";
#line 21
	      n += printf ("%s%s%s", cb, gettext (opthelp[i].arg), ce);
#line 21
	    }
#line 21
	  if (n >= DESCRCOLUMN)
#line 21
	    {
#line 21
	      putchar ('\n');
#line 21
	      n = 0;
#line 21
	    }
#line 21
	  indent (n, DESCRCOLUMN);
#line 21
	  print_option_descr (gettext (opthelp[i].descr), DESCRCOLUMN, RMARGIN);
#line 21
	}
#line 21
      else
#line 21
	{
#line 21
	  if (i)
#line 21
	    putchar ('\n');
#line 21
	  indent (0, GROUPCOLUMN);
#line 21
	  print_option_descr (gettext (opthelp[i].descr),
#line 21
			      GROUPCOLUMN, RMARGIN);
#line 21
	  putchar ('\n');
#line 21
	}
#line 21
    }
#line 21

#line 21
  putchar ('\n');
#line 21
  if (argsused)
#line 21
    {
#line 21
      print_option_descr (_("Mandatory or optional arguments to long options are also mandatory or optional for any corresponding short options."), 0, RMARGIN);
#line 21
      putchar ('\n');
#line 21
    }
#line 21
 
#line 21
  if (print_help_hook)
#line 21
    print_help_hook (stdout);
#line 21
    
#line 21
 /* TRANSLATORS: The placeholder indicates the bug-reporting address
    for this package.  Please add _another line_ saying
    "Report translation bugs to <...>\n" with the address for translation
    bugs (typically your translation team's web or email address).  */
#line 21
  printf (_("Report bugs to %s.\n"), program_bug_address);
#line 21
  
#line 21
#ifdef PACKAGE_URL
#line 21
  printf (_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
#line 21
#else
#line 21
  printf (_("%s home page: <http://www.gnu.org/software/%s/>\n"),
#line 21
	  PACKAGE_NAME, PACKAGE);
#line 21
#endif
#line 21
  fputs (_("General help using GNU software: <http://www.gnu.org/gethelp/>\n"),
#line 21
	 stdout);
#line 21
}
#line 21

#line 21
static int
#line 21
cmpidx_short (const void *a, const void *b)
#line 21
{
#line 21
  unsigned const *ai = (unsigned const *)a;
#line 21
  unsigned const *bi = (unsigned const *)b;
#line 21

#line 21
  return opthelp[*ai].opt[1] - opthelp[*bi].opt[1];
#line 21
}
#line 21
  
#line 21
#ifdef HAVE_GETOPT_LONG
#line 21
static int
#line 21
cmpidx_long (const void *a, const void *b)
#line 21
{
#line 21
  unsigned const *ai = (unsigned const *)a;
#line 21
  unsigned const *bi = (unsigned const *)b;
#line 21
  struct opthelp const *ap = opthelp + *ai;
#line 21
  struct opthelp const *bp = opthelp + *bi;
#line 21
  char const *opta, *optb;
#line 21
  size_t lena, lenb;
#line 21

#line 21
  if (ap->opt[1] == '-')
#line 21
    opta = ap->opt;
#line 21
  else
#line 21
    opta = ap->opt + 4;
#line 21
  lena = strcspn (opta, ",");
#line 21
  
#line 21
  if (bp->opt[1] == '-')
#line 21
    optb = bp->opt;
#line 21
  else
#line 21
    optb = bp->opt + 4;
#line 21
  lenb = strcspn (optb, ",");
#line 21
  return strncmp (opta, optb, lena > lenb ? lenb : lena);
#line 21
}
#line 21
#endif
#line 21

#line 21
void
#line 21
print_usage (void)
#line 21
{
#line 21
  unsigned i;
#line 21
  unsigned n;
#line 21
  char buf[RMARGIN+1];
#line 21
  unsigned *idxbuf;
#line 21
  unsigned nidx;
#line 21
  
#line 21
#define FLUSH                        do                                   {                              	  buf[n] = 0;              	  printf ("%s\n", buf);    	  n = USAGECOLUMN;         	  memset (buf, ' ', n);        }                                while (0)
#line 21
#define ADDC(c)   do { if (n == RMARGIN) FLUSH; buf[n++] = c; } while (0)
#line 21

#line 21
  idxbuf = calloc (NOPTHELP, sizeof (idxbuf[0]));
#line 21
  if (!idxbuf)
#line 21
    {
#line 21
      fprintf (stderr, "not enough memory");
#line 21
      abort ();
#line 21
    }
#line 21

#line 21
  n = snprintf (buf, sizeof buf, "%s %s ", _("Usage:"), "dico");
#line 21

#line 21
  /* Print a list of short options without arguments. */
#line 21
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 21
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
#line 21
	  && opthelp[i].arg == NULL)
#line 21
	idxbuf[nidx++] = i;
#line 21

#line 21
  if (nidx)
#line 21
    {
#line 21
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);
#line 21

#line 21
      ADDC ('[');
#line 21
      ADDC ('-');
#line 21
      for (i = 0; i < nidx; i++)
#line 21
	{
#line 21
	  ADDC (opthelp[idxbuf[i]].opt[1]);
#line 21
	}
#line 21
      ADDC (']');
#line 21
    }
#line 21

#line 21
  /* Print a list of short options with arguments. */
#line 21
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 21
    {
#line 21
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
#line 21
	  && opthelp[i].arg)
#line 21
	idxbuf[nidx++] = i;
#line 21
    }
#line 21

#line 21
  if (nidx)
#line 21
    {
#line 21
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);
#line 21
    
#line 21
      for (i = 0; i < nidx; i++)
#line 21
	{
#line 21
	  struct opthelp *opt = opthelp + idxbuf[i];
#line 21
	  size_t len = 5 + strlen (opt->arg)
#line 21
	                 + (opt->is_optional ? 2 : 1);
#line 21
      
#line 21
	  if (n + len > RMARGIN) FLUSH;
#line 21
	  buf[n++] = ' ';
#line 21
	  buf[n++] = '[';
#line 21
	  buf[n++] = '-';
#line 21
	  buf[n++] = opt->opt[1];
#line 21
	  if (opt->is_optional)
#line 21
	    {
#line 21
	      buf[n++] = '[';
#line 21
	      strcpy (&buf[n], opt->arg);
#line 21
	      n += strlen (opt->arg);
#line 21
	      buf[n++] = ']';
#line 21
	    }
#line 21
	  else
#line 21
	    {
#line 21
	      buf[n++] = ' ';
#line 21
	      strcpy (&buf[n], opt->arg);
#line 21
	      n += strlen (opt->arg);
#line 21
	    }
#line 21
	  buf[n++] = ']';
#line 21
	}
#line 21
    }
#line 21
  
#line 21
#ifdef HAVE_GETOPT_LONG
#line 21
  /* Print a list of long options */
#line 21
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 21
    {
#line 21
      if (opthelp[i].opt && opthelp[i].descr
#line 21
	  &&  (opthelp[i].opt[1] == '-' || opthelp[i].opt[2] == ','))
#line 21
	idxbuf[nidx++] = i;
#line 21
    }
#line 21

#line 21
  if (nidx)
#line 21
    {
#line 21
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_long);
#line 21
	
#line 21
      for (i = 0; i < nidx; i++)
#line 21
	{
#line 21
	  struct opthelp *opt = opthelp + idxbuf[i];
#line 21
	  size_t len;
#line 21
	  const char *longopt;
#line 21
	  
#line 21
	  if (opt->opt[1] == '-')
#line 21
	    longopt = opt->opt;
#line 21
	  else if (opt->opt[2] == ',')
#line 21
	    longopt = opt->opt + 4;
#line 21
	  else
#line 21
	    continue;
#line 21

#line 21
	  len = 3 + strlen (longopt)
#line 21
	          + (opt->arg ? 1 + strlen (opt->arg)
#line 21
		  + (opt->is_optional ? 2 : 0) : 0);
#line 21
	  if (n + len > RMARGIN) FLUSH;
#line 21
	  buf[n++] = ' ';
#line 21
	  buf[n++] = '[';
#line 21
	  strcpy (&buf[n], longopt);
#line 21
	  n += strlen (longopt);
#line 21
	  if (opt->arg)
#line 21
	    {
#line 21
	      buf[n++] = '=';
#line 21
	      if (opt->is_optional)
#line 21
		{
#line 21
		  buf[n++] = '[';
#line 21
		  strcpy (&buf[n], opt->arg);
#line 21
		  n += strlen (opt->arg);
#line 21
		  buf[n++] = ']';
#line 21
		}
#line 21
	      else
#line 21
		{
#line 21
		  strcpy (&buf[n], opt->arg);
#line 21
		  n += strlen (opt->arg);
#line 21
		}
#line 21
	    }
#line 21
	  buf[n++] = ']';
#line 21
	}
#line 21
    }
#line 21
#endif
#line 21
  FLUSH;
#line 21
  free (idxbuf);
#line 21
}
#line 21

#line 21
const char version_etc_copyright[] =
#line 21
  /* Do *not* mark this string for translation.  First %s is a copyright
     symbol suitable for this locale, and second %s are the copyright
     years.  */
#line 21
  "Copyright %s %s %s";
#line 21

#line 21
void
#line 21
print_version_only(const char *program_version, FILE *stream)
#line 21
{
#line 21
  fprintf (stream, "%s\n", program_version);
#line 21
  /* TRANSLATORS: Translate "(C)" to the copyright symbol
     (C-in-a-circle), if this symbol is available in the user's
     locale.  Otherwise, do not translate "(C)"; leave it as-is.  */
#line 21
  fprintf (stream, version_etc_copyright, _("(C)"),
#line 21
	   "2005-2012",
#line 21
	   "Free Software Foundation, Inc.");
#line 21
  fputc ('\n', stream);
#line 21
}
#line 21

#line 21

#line 21

#line 21
void (*print_version_hook)(FILE *stream);
#line 21

#line 21
void
#line 21
print_version(const char *program_version, FILE *stream)
#line 21
{
#line 21
  
#line 21
  print_version_only(program_version, stream);
#line 21

#line 21
  fputs (_("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n"),
#line 21
	 stream);
#line 21
  if (print_version_hook)
#line 21
    print_version_hook (stream);
#line 21
		
#line 21
}
#line 21

#line 193 "cmdline.opt"

#line 193


void
get_options (int argc, char *argv[], int *index)
{
    
#line 198
 {
#line 198
  int c;
#line 198

#line 198
#ifdef HAVE_GETOPT_LONG
#line 198
  while ((c = getopt_long(argc, argv, "p:d:ms:DSHi:Iqau:k:c:tvhV",
#line 198
			  long_options, NULL)) != EOF)
#line 198
#else
#line 198
  while ((c = getopt(argc, argv, "p:d:ms:DSHi:Iqau:k:c:tvhV")) != EOF)
#line 198
#endif
#line 198
    {
#line 198
      switch (c)
#line 198
	{
#line 198
	default:
#line 198
	   	   exit(EX_USAGE);
#line 198
	#line 33 "cmdline.opt"
	 case OPTION_HOST:
#line 33
	  {
#line 33

  xdico_assign_string(&dico_url.host, optarg);

#line 35
	     break;
#line 35
	  }
#line 39 "cmdline.opt"
	 case 'p':
#line 39
	  {
#line 39

  dico_url.port = str2port(optarg);
  if (dico_url.port == -1)
      dico_die(1, L_ERR, 0, _("Invalid port"));  

#line 43
	     break;
#line 43
	  }
#line 47 "cmdline.opt"
	 case 'd':
#line 47
	  {
#line 47

  xdico_assign_string(&dico_url.req.database, optarg);  

#line 49
	     break;
#line 49
	  }
#line 53 "cmdline.opt"
	 case OPTION_SOURCE:
#line 53
	  {
#line 53

  source_addr = get_ipaddr(optarg);
  if (source_addr == 0)
    dico_die(1, 0, L_ERR, _("%s: Invalid IP or unknown host name"),
             optarg);

#line 58
	     break;
#line 58
	  }
#line 64 "cmdline.opt"
	 case 'm':
#line 64
	  {
#line 64

  mode = mode_match;   

#line 66
	     break;
#line 66
	  }
#line 70 "cmdline.opt"
	 case 's':
#line 70
	  {
#line 70

  xdico_assign_string(&dico_url.req.strategy, optarg);
  mode = mode_match;

#line 73
	     break;
#line 73
	  }
#line 78 "cmdline.opt"
	 case OPTION_LEVDIST: case OPTION_LEVENSHTEIN_DISTANCE:
#line 78
	  {
#line 78

  char *p;
  levenshtein_threshold = strtoul(optarg, &p, 10);
  if (*p)
      dico_die(1, L_ERR, 0, _("%s: invalid number"), optarg);

#line 83
	     break;
#line 83
	  }
#line 87 "cmdline.opt"
	 case 'D':
#line 87
	  {
#line 87

  mode = mode_dbs;  

#line 89
	     break;
#line 89
	  }
#line 93 "cmdline.opt"
	 case 'S':
#line 93
	  {
#line 93

  mode = mode_strats;

#line 95
	     break;
#line 95
	  }
#line 99 "cmdline.opt"
	 case 'H':
#line 99
	  {
#line 99

  mode = mode_help;

#line 101
	     break;
#line 101
	  }
#line 105 "cmdline.opt"
	 case 'i':
#line 105
	  {
#line 105

  mode = mode_info;
  dico_url.req.database = optarg;

#line 108
	     break;
#line 108
	  }
#line 112 "cmdline.opt"
	 case 'I':
#line 112
	  {
#line 112

  mode = mode_server;

#line 114
	     break;
#line 114
	  }
#line 118 "cmdline.opt"
	 case 'q':
#line 118
	  {
#line 118

  quiet_option = 1;

#line 120
	     break;
#line 120
	  }
#line 126 "cmdline.opt"
	 case 'a':
#line 126
	  {
#line 126

  noauth_option = 1;

#line 128
	     break;
#line 128
	  }
#line 132 "cmdline.opt"
	 case OPTION_SASL:
#line 132
	  {
#line 132

  sasl_enable(1);

#line 134
	     break;
#line 134
	  }
#line 138 "cmdline.opt"
	 case OPTION_NOSASL:
#line 138
	  {
#line 138

  sasl_enable(0);

#line 140
	     break;
#line 140
	  }
#line 144 "cmdline.opt"
	 case 'u':
#line 144
	  {
#line 144

  default_cred.user = optarg;

#line 146
	     break;
#line 146
	  }
#line 151 "cmdline.opt"
	 case 'k': case OPTION_PASSWORD:
#line 151
	  {
#line 151

  default_cred.pass = optarg;

#line 153
	     break;
#line 153
	  }
#line 157 "cmdline.opt"
	 case OPTION_AUTOLOGIN:
#line 157
	  {
#line 157

  autologin_file = optarg;

#line 159
	     break;
#line 159
	  }
#line 163 "cmdline.opt"
	 case 'c':
#line 163
	  {
#line 163

  client = optarg;

#line 165
	     break;
#line 165
	  }
#line 170 "cmdline.opt"
	 case 't':
#line 170
	  {
#line 170

  transcript = 1;

#line 172
	     break;
#line 172
	  }
#line 176 "cmdline.opt"
	 case 'v':
#line 176
	  {
#line 176

  debug_level++;

#line 178
	     break;
#line 178
	  }
#line 182 "cmdline.opt"
	 case OPTION_TIME_STAMP:
#line 182
	  {
#line 182

  int n = 1;
  dico_stream_ioctl(debug_stream, DICO_DBG_CTL_SET_TS, &n);

#line 185
	     break;
#line 185
	  }
#line 189 "cmdline.opt"
	 case OPTION_SOURCE_INFO:
#line 189
	  {
#line 189

  debug_source_info = 1;

#line 191
	     break;
#line 191
	  }
#line 193 "cmdline.opt"
	 case 'h':
#line 193
	  {
#line 193

#line 193
		print_help ();
#line 193
		exit (0);
#line 193
	 
#line 193
	     break;
#line 193
	  }
#line 193 "cmdline.opt"
	 case OPTION_USAGE:
#line 193
	  {
#line 193

#line 193
		print_usage ();
#line 193
		exit (0);
#line 193
	 
#line 193
	     break;
#line 193
	  }
#line 193 "cmdline.opt"
	 case 'V':
#line 193
	  {
#line 193

#line 193
		/* Give version */
#line 193
		print_version(program_version, stdout);
#line 193
		exit (0);
#line 193
	 
#line 193
	     break;
#line 193
	  }

#line 198 "cmdline.opt"
	}
#line 198
    }
#line 198
  *index = optind;
#line 198
  if (cmdline_tree)
#line 198
    {
#line 198
      struct grecs_node *rn = grecs_node_create(grecs_node_root, NULL);
#line 198
      rn->down = cmdline_tree;
#line 198
      cmdline_tree = rn;
#line 198
    }
#line 198
 }
#line 198

}

