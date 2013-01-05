#line 832 "../../grecs/build-aux/getopt.m4"
/* -*- buffer-read-only: t -*- vi: set ro:
   THIS FILE IS GENERATED AUTOMATICALLY.  PLEASE DO NOT EDIT.
*/
#line 1 "idxgcide-cli.opt"
/* This file is part of GNU Dico. -*- c -*-
   Copyright (C) 2012 Sergey Poznyakoff

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

#line 17 "idxgcide-cli.opt"

#line 17
void print_help(void);
#line 17
void print_usage(void);
#line 67 "idxgcide-cli.opt"

#line 67
#include <grecs.h>
#line 67
/* Option codes */
#line 67
enum {
#line 67
	_OPTION_INIT=255,
#line 67
	#line 67 "idxgcide-cli.opt"

#line 67
	OPTION_USAGE,

#line 67 "idxgcide-cli.opt"
	MAX_OPTION
#line 67
};
#line 67
#ifdef HAVE_GETOPT_LONG
#line 67
static struct option long_options[] = {
#line 67
	#line 24 "idxgcide-cli.opt"

#line 24
	{ "debug", no_argument, 0, 'd' },
#line 30 "idxgcide-cli.opt"

#line 30
	{ "dry-run", no_argument, 0, 'n' },
#line 37 "idxgcide-cli.opt"

#line 37
	{ "verbose", no_argument, 0, 'v' },
#line 43 "idxgcide-cli.opt"

#line 43
	{ "page-size", required_argument, 0, 'p' },
#line 67 "idxgcide-cli.opt"

#line 67
	{ "help", no_argument, 0, 'h' },
#line 67 "idxgcide-cli.opt"

#line 67
	{ "usage", no_argument, 0, OPTION_USAGE },
#line 67 "idxgcide-cli.opt"

#line 67
	{ "version", no_argument, 0, 'V' },

#line 67 "idxgcide-cli.opt"
	{0, 0, 0, 0}
#line 67
};
#line 67
#endif
#line 67
static struct opthelp {
#line 67
	const char *opt;
#line 67
	const char *arg;
#line 67
	int is_optional;
#line 67
	const char *descr;
#line 67
} opthelp[] = {
#line 67
	#line 26 "idxgcide-cli.opt"

#line 26
	{
#line 26
#ifdef HAVE_GETOPT_LONG
#line 26
	  "-d, --debug",
#line 26
#else
#line 26
	  "-d",
#line 26
#endif
#line 26
				   NULL, 0, N_("debug mode") },
#line 32 "idxgcide-cli.opt"

#line 32
	{
#line 32
#ifdef HAVE_GETOPT_LONG
#line 32
	  "-n, --dry-run",
#line 32
#else
#line 32
	  "-n",
#line 32
#endif
#line 32
				   NULL, 0, N_("dry run: do nothing, but print everything") },
#line 39 "idxgcide-cli.opt"

#line 39
	{
#line 39
#ifdef HAVE_GETOPT_LONG
#line 39
	  "-v, --verbose",
#line 39
#else
#line 39
	  "-v",
#line 39
#endif
#line 39
				   NULL, 0, N_("increase verbosity") },
#line 45 "idxgcide-cli.opt"

#line 45
	{
#line 45
#ifdef HAVE_GETOPT_LONG
#line 45
	  "-p, --page-size",
#line 45
#else
#line 45
	  "-p",
#line 45
#endif
#line 45
				   N_("NUMBER"), 0, N_("set index page size") },
#line 67 "idxgcide-cli.opt"

#line 67
	{ NULL, NULL, 0, N_("Other options") },
#line 67 "idxgcide-cli.opt"

#line 67
	{
#line 67
#ifdef HAVE_GETOPT_LONG
#line 67
	  "-h, --help",
#line 67
#else
#line 67
	  "-h",
#line 67
#endif
#line 67
				   NULL, 0, N_("Give this help list") },
#line 67 "idxgcide-cli.opt"

#line 67
	{
#line 67
#ifdef HAVE_GETOPT_LONG
#line 67
	  "--usage",
#line 67
#else
#line 67
	  "",
#line 67
#endif
#line 67
				   NULL, 0, N_("Give a short usage message") },
#line 67 "idxgcide-cli.opt"

#line 67
	{
#line 67
#ifdef HAVE_GETOPT_LONG
#line 67
	  "-V, --version",
#line 67
#else
#line 67
	  "-V",
#line 67
#endif
#line 67
				   NULL, 0, N_("Print program version") },

#line 67 "idxgcide-cli.opt"
};
#line 17 "idxgcide-cli.opt"

#line 17
const char *program_version = "idxgcide" " (" PACKAGE_NAME ") " PACKAGE_VERSION;
#line 17
static char doc[] = N_("standalone indexer for GCIDE dictionaries");
#line 17
static char args_doc[] = N_("DICTDIR [IDXDIR]");
#line 17
const char *program_bug_address = "<" PACKAGE_BUGREPORT ">";
#line 17
struct grecs_node *cmdline_tree;
#line 17

#line 17
#define DESCRCOLUMN 30
#line 17
#define RMARGIN 79
#line 17
#define GROUPCOLUMN 2
#line 17
#define USAGECOLUMN 13
#line 17

#line 17
static void
#line 17
indent (size_t start, size_t col)
#line 17
{
#line 17
  for (; start < col; start++)
#line 17
    putchar (' ');
#line 17
}
#line 17

#line 17
static void
#line 17
print_option_descr (const char *descr, size_t lmargin, size_t rmargin)
#line 17
{
#line 17
  while (*descr)
#line 17
    {
#line 17
      size_t s = 0;
#line 17
      size_t i;
#line 17
      size_t width = rmargin - lmargin;
#line 17

#line 17
      for (i = 0; ; i++)
#line 17
	{
#line 17
	  if (descr[i] == 0 || descr[i] == ' ' || descr[i] == '\t')
#line 17
	    {
#line 17
	      if (i > width)
#line 17
		break;
#line 17
	      s = i;
#line 17
	      if (descr[i] == 0)
#line 17
		break;
#line 17
	    }
#line 17
	}
#line 17
      printf ("%*.*s\n", s, s, descr);
#line 17
      descr += s;
#line 17
      if (*descr)
#line 17
	{
#line 17
	  indent (0, lmargin);
#line 17
	  descr++;
#line 17
	}
#line 17
    }
#line 17
}
#line 17

#line 17
#define NOPTHELP (sizeof (opthelp) / sizeof (opthelp[0]))
#line 17

#line 17
static int
#line 17
optcmp (const void *a, const void *b)
#line 17
{
#line 17
  struct opthelp const *ap = (struct opthelp const *)a;
#line 17
  struct opthelp const *bp = (struct opthelp const *)b;
#line 17
  const char *opta, *optb;
#line 17
  size_t alen, blen;
#line 17

#line 17
  for (opta = ap->opt; *opta == '-'; opta++)
#line 17
    ;
#line 17
  alen = strcspn (opta, ",");
#line 17
  
#line 17
  for (optb = bp->opt; *optb == '-'; optb++)
#line 17
    ;
#line 17
  blen = strcspn (optb, ",");
#line 17

#line 17
  if (alen > blen)
#line 17
    blen = alen;
#line 17
  
#line 17
  return strncmp (opta, optb, blen);
#line 17
}
#line 17

#line 17
static void
#line 17
sort_options (int start, int count)
#line 17
{
#line 17
  qsort (opthelp + start, count, sizeof (opthelp[0]), optcmp);
#line 17
}
#line 17

#line 17
static int
#line 17
sort_group (int start)
#line 17
{
#line 17
  int i;
#line 17
  
#line 17
  for (i = start; i < NOPTHELP && opthelp[i].opt; i++)
#line 17
    ;
#line 17
  sort_options (start, i - start);
#line 17
  return i + 1;
#line 17
}
#line 17

#line 17
static void
#line 17
sort_opthelp (void)
#line 17
{
#line 17
  int start;
#line 17

#line 17
  for (start = 0; start < NOPTHELP; )
#line 17
    {
#line 17
      if (!opthelp[start].opt)
#line 17
	start = sort_group (start + 1);
#line 17
      else 
#line 17
	start = sort_group (start);
#line 17
    }
#line 17
}
#line 17

#line 17
void (*print_help_hook) (FILE *stream);
#line 17

#line 17
void
#line 17
print_help (void)
#line 17
{
#line 17
  unsigned i;
#line 17
  int argsused = 0;
#line 17

#line 17
  printf ("%s %s [%s]... %s\n", _("Usage:"), "idxgcide", _("OPTION"),
#line 17
	  gettext (args_doc));
#line 17
  if (doc[0])
#line 17
    print_option_descr(gettext (doc), 0, RMARGIN);
#line 17
  putchar ('\n');
#line 17

#line 17
  sort_opthelp ();
#line 17
  for (i = 0; i < NOPTHELP; i++)
#line 17
    {
#line 17
      unsigned n;
#line 17
      if (opthelp[i].opt)
#line 17
	{
#line 17
	  n = printf ("  %s", opthelp[i].opt);
#line 17
	  if (opthelp[i].arg)
#line 17
	    {
#line 17
	      char *cb, *ce;
#line 17
	      argsused = 1;
#line 17
	      if (strlen (opthelp[i].opt) == 2)
#line 17
		{
#line 17
		  if (!opthelp[i].is_optional)
#line 17
		    {
#line 17
		      putchar (' ');
#line 17
		      n++;
#line 17
		    }
#line 17
		}
#line 17
	      else
#line 17
		{
#line 17
		  putchar ('=');
#line 17
		  n++;
#line 17
		}
#line 17
	      if (opthelp[i].is_optional)
#line 17
		{
#line 17
		  cb = "[";
#line 17
		  ce = "]";
#line 17
		}
#line 17
	      else
#line 17
		cb = ce = "";
#line 17
	      n += printf ("%s%s%s", cb, gettext (opthelp[i].arg), ce);
#line 17
	    }
#line 17
	  if (n >= DESCRCOLUMN)
#line 17
	    {
#line 17
	      putchar ('\n');
#line 17
	      n = 0;
#line 17
	    }
#line 17
	  indent (n, DESCRCOLUMN);
#line 17
	  print_option_descr (gettext (opthelp[i].descr), DESCRCOLUMN, RMARGIN);
#line 17
	}
#line 17
      else
#line 17
	{
#line 17
	  if (i)
#line 17
	    putchar ('\n');
#line 17
	  indent (0, GROUPCOLUMN);
#line 17
	  print_option_descr (gettext (opthelp[i].descr),
#line 17
			      GROUPCOLUMN, RMARGIN);
#line 17
	  putchar ('\n');
#line 17
	}
#line 17
    }
#line 17

#line 17
  putchar ('\n');
#line 17
  if (argsused)
#line 17
    {
#line 17
      print_option_descr (_("Mandatory or optional arguments to long options are also mandatory or optional for any corresponding short options."), 0, RMARGIN);
#line 17
      putchar ('\n');
#line 17
    }
#line 17
 
#line 17
  if (print_help_hook)
#line 17
    print_help_hook (stdout);
#line 17
    
#line 17
 /* TRANSLATORS: The placeholder indicates the bug-reporting address
    for this package.  Please add _another line_ saying
    "Report translation bugs to <...>\n" with the address for translation
    bugs (typically your translation team's web or email address).  */
#line 17
  printf (_("Report bugs to %s.\n"), program_bug_address);
#line 17
  
#line 17
#ifdef PACKAGE_URL
#line 17
  printf (_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
#line 17
#endif
#line 17
}
#line 17

#line 17
static int
#line 17
cmpidx_short (const void *a, const void *b)
#line 17
{
#line 17
  unsigned const *ai = (unsigned const *)a;
#line 17
  unsigned const *bi = (unsigned const *)b;
#line 17

#line 17
  return opthelp[*ai].opt[1] - opthelp[*bi].opt[1];
#line 17
}
#line 17
  
#line 17
#ifdef HAVE_GETOPT_LONG
#line 17
static int
#line 17
cmpidx_long (const void *a, const void *b)
#line 17
{
#line 17
  unsigned const *ai = (unsigned const *)a;
#line 17
  unsigned const *bi = (unsigned const *)b;
#line 17
  struct opthelp const *ap = opthelp + *ai;
#line 17
  struct opthelp const *bp = opthelp + *bi;
#line 17
  char const *opta, *optb;
#line 17
  size_t lena, lenb;
#line 17

#line 17
  if (ap->opt[1] == '-')
#line 17
    opta = ap->opt;
#line 17
  else
#line 17
    opta = ap->opt + 4;
#line 17
  lena = strcspn (opta, ",");
#line 17
  
#line 17
  if (bp->opt[1] == '-')
#line 17
    optb = bp->opt;
#line 17
  else
#line 17
    optb = bp->opt + 4;
#line 17
  lenb = strcspn (optb, ",");
#line 17
  return strncmp (opta, optb, lena > lenb ? lenb : lena);
#line 17
}
#line 17
#endif
#line 17

#line 17
void
#line 17
print_usage (void)
#line 17
{
#line 17
  unsigned i;
#line 17
  unsigned n;
#line 17
  char buf[RMARGIN+1];
#line 17
  unsigned *idxbuf;
#line 17
  unsigned nidx;
#line 17
  
#line 17
#define FLUSH                        do                                   {                              	  buf[n] = 0;              	  printf ("%s\n", buf);    	  n = USAGECOLUMN;         	  memset (buf, ' ', n);        }                                while (0)
#line 17
#define ADDC(c)   do { if (n == RMARGIN) FLUSH; buf[n++] = c; } while (0)
#line 17

#line 17
  idxbuf = calloc (NOPTHELP, sizeof (idxbuf[0]));
#line 17
  if (!idxbuf)
#line 17
    {
#line 17
      fprintf (stderr, "not enough memory");
#line 17
      abort ();
#line 17
    }
#line 17

#line 17
  n = snprintf (buf, sizeof buf, "%s %s ", _("Usage:"), "idxgcide");
#line 17

#line 17
  /* Print a list of short options without arguments. */
#line 17
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 17
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
#line 17
	  && opthelp[i].arg == NULL)
#line 17
	idxbuf[nidx++] = i;
#line 17

#line 17
  if (nidx)
#line 17
    {
#line 17
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);
#line 17

#line 17
      ADDC ('[');
#line 17
      ADDC ('-');
#line 17
      for (i = 0; i < nidx; i++)
#line 17
	{
#line 17
	  ADDC (opthelp[idxbuf[i]].opt[1]);
#line 17
	}
#line 17
      ADDC (']');
#line 17
    }
#line 17

#line 17
  /* Print a list of short options with arguments. */
#line 17
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 17
    {
#line 17
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
#line 17
	  && opthelp[i].arg)
#line 17
	idxbuf[nidx++] = i;
#line 17
    }
#line 17

#line 17
  if (nidx)
#line 17
    {
#line 17
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);
#line 17
    
#line 17
      for (i = 0; i < nidx; i++)
#line 17
	{
#line 17
	  struct opthelp *opt = opthelp + idxbuf[i];
#line 17
	  size_t len = 5 + strlen (opt->arg)
#line 17
	                 + (opt->is_optional ? 2 : 1);
#line 17
      
#line 17
	  if (n + len > RMARGIN) FLUSH;
#line 17
	  buf[n++] = ' ';
#line 17
	  buf[n++] = '[';
#line 17
	  buf[n++] = '-';
#line 17
	  buf[n++] = opt->opt[1];
#line 17
	  if (opt->is_optional)
#line 17
	    {
#line 17
	      buf[n++] = '[';
#line 17
	      strcpy (&buf[n], opt->arg);
#line 17
	      n += strlen (opt->arg);
#line 17
	      buf[n++] = ']';
#line 17
	    }
#line 17
	  else
#line 17
	    {
#line 17
	      buf[n++] = ' ';
#line 17
	      strcpy (&buf[n], opt->arg);
#line 17
	      n += strlen (opt->arg);
#line 17
	    }
#line 17
	  buf[n++] = ']';
#line 17
	}
#line 17
    }
#line 17
  
#line 17
#ifdef HAVE_GETOPT_LONG
#line 17
  /* Print a list of long options */
#line 17
  for (i = nidx = 0; i < NOPTHELP; i++)
#line 17
    {
#line 17
      if (opthelp[i].opt && opthelp[i].descr
#line 17
	  &&  (opthelp[i].opt[1] == '-' || opthelp[i].opt[2] == ','))
#line 17
	idxbuf[nidx++] = i;
#line 17
    }
#line 17

#line 17
  if (nidx)
#line 17
    {
#line 17
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_long);
#line 17
	
#line 17
      for (i = 0; i < nidx; i++)
#line 17
	{
#line 17
	  struct opthelp *opt = opthelp + idxbuf[i];
#line 17
	  size_t len;
#line 17
	  const char *longopt;
#line 17
	  
#line 17
	  if (opt->opt[1] == '-')
#line 17
	    longopt = opt->opt;
#line 17
	  else if (opt->opt[2] == ',')
#line 17
	    longopt = opt->opt + 4;
#line 17
	  else
#line 17
	    continue;
#line 17

#line 17
	  len = 3 + strlen (longopt)
#line 17
	          + (opt->arg ? 1 + strlen (opt->arg)
#line 17
		  + (opt->is_optional ? 2 : 0) : 0);
#line 17
	  if (n + len > RMARGIN) FLUSH;
#line 17
	  buf[n++] = ' ';
#line 17
	  buf[n++] = '[';
#line 17
	  strcpy (&buf[n], longopt);
#line 17
	  n += strlen (longopt);
#line 17
	  if (opt->arg)
#line 17
	    {
#line 17
	      buf[n++] = '=';
#line 17
	      if (opt->is_optional)
#line 17
		{
#line 17
		  buf[n++] = '[';
#line 17
		  strcpy (&buf[n], opt->arg);
#line 17
		  n += strlen (opt->arg);
#line 17
		  buf[n++] = ']';
#line 17
		}
#line 17
	      else
#line 17
		{
#line 17
		  strcpy (&buf[n], opt->arg);
#line 17
		  n += strlen (opt->arg);
#line 17
		}
#line 17
	    }
#line 17
	  buf[n++] = ']';
#line 17
	}
#line 17
    }
#line 17
#endif
#line 17
  FLUSH;
#line 17
  free (idxbuf);
#line 17
}
#line 17

#line 17
const char version_etc_copyright[] =
#line 17
  /* Do *not* mark this string for translation.  First %s is a copyright
     symbol suitable for this locale, and second %s are the copyright
     years.  */
#line 17
  "Copyright %s %s %s";
#line 17

#line 17
void
#line 17
print_version_only(const char *program_version, FILE *stream)
#line 17
{
#line 17
  fprintf (stream, "%s\n", program_version);
#line 17
  /* TRANSLATORS: Translate "(C)" to the copyright symbol
     (C-in-a-circle), if this symbol is available in the user's
     locale.  Otherwise, do not translate "(C)"; leave it as-is.  */
#line 17
  fprintf (stream, version_etc_copyright, _("(C)"),
#line 17
	   "2012",
#line 17
	   "Free Software Foundation, Inc.");
#line 17
  fputc ('\n', stream);
#line 17
}
#line 17

#line 17

#line 17

#line 17
void (*print_version_hook)(FILE *stream);
#line 17

#line 17
void
#line 17
print_version(const char *program_version, FILE *stream)
#line 17
{
#line 17
  
#line 17
  print_version_only(program_version, stream);
#line 17

#line 17
  fputs (_("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n"),
#line 17
	 stream);
#line 17
  if (print_version_hook)
#line 17
    print_version_hook (stream);
#line 17
		
#line 17
}
#line 17

#line 67 "idxgcide-cli.opt"

#line 67


void
get_options(int argc, char *argv[], int *index)
{
    
#line 72
 {
#line 72
  int c;
#line 72

#line 72
#ifdef HAVE_GETOPT_LONG
#line 72
  while ((c = getopt_long(argc, argv, "dnvp:hV",
#line 72
			  long_options, NULL)) != EOF)
#line 72
#else
#line 72
  while ((c = getopt(argc, argv, "dnvp:hV")) != EOF)
#line 72
#endif
#line 72
    {
#line 72
      switch (c)
#line 72
	{
#line 72
	default:
#line 72
	   exit(EX_USAGE);	   exit(EX_USAGE);
#line 72
	#line 26 "idxgcide-cli.opt"
	 case 'd':
#line 26
	  {
#line 26

   yy_flex_debug = 1;

#line 28
	     break;
#line 28
	  }
#line 32 "idxgcide-cli.opt"
	 case 'n':
#line 32
	  {
#line 32

   dry_run_option = 1;
   verbose_option++;

#line 35
	     break;
#line 35
	  }
#line 39 "idxgcide-cli.opt"
	 case 'v':
#line 39
	  {
#line 39

   verbose_option++;

#line 41
	     break;
#line 41
	  }
#line 45 "idxgcide-cli.opt"
	 case 'p':
#line 45
	  {
#line 45

   char *p;
   idx_header.ihdr_pagesize = strtoul(optarg, &p, 10);
   switch (*p) {
   case 0:
       break;
   case 'g':
   case 'G':
       idx_header.ihdr_pagesize <<= 10;
   case 'm':
   case 'M':
       idx_header.ihdr_pagesize <<= 10;
   case 'k':
   case 'K':
       idx_header.ihdr_pagesize <<= 10;
       break;
   default:
       dico_log(L_ERR, 0, _("not a valid size: %s"), optarg);
       exit(EX_USAGE);
   }

#line 65
	     break;
#line 65
	  }
#line 67 "idxgcide-cli.opt"
	 case 'h':
#line 67
	  {
#line 67

#line 67
		print_help ();
#line 67
		exit (0);
#line 67
	 
#line 67
	     break;
#line 67
	  }
#line 67 "idxgcide-cli.opt"
	 case OPTION_USAGE:
#line 67
	  {
#line 67

#line 67
		print_usage ();
#line 67
		exit (0);
#line 67
	 
#line 67
	     break;
#line 67
	  }
#line 67 "idxgcide-cli.opt"
	 case 'V':
#line 67
	  {
#line 67

#line 67
		/* Give version */
#line 67
		print_version(program_version, stdout);
#line 67
		exit (0);
#line 67
	 
#line 67
	     break;
#line 67
	  }

#line 72 "idxgcide-cli.opt"
	}
#line 72
    }
#line 72
  *index = optind;
#line 72
  if (cmdline_tree)
#line 72
    {
#line 72
      struct grecs_node *rn = grecs_node_create(grecs_node_root, NULL);
#line 72
      rn->down = cmdline_tree;
#line 72
      cmdline_tree = rn;
#line 72
    }
#line 72
 }
#line 72

}

