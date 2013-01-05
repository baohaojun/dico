dnl This file is part of grecs
dnl Copyright (C) 2007, 2009-2012 Sergey Poznyakoff
dnl
dnl Grecs is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3, or (at your option)
dnl any later version.
dnl
dnl Grecs is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with Grecs.  If not, see <http://www.gnu.org/licenses/>.
divert(-1)
changequote([<,>])
changecom(/*,*/)

dnl _getopt_mangle_option(NAME)
dnl ---------------------------
dnl Convert NAME to a valid m4 identifier, by replacing invalid characters
dnl with underscores, and prepend the _GETOPT_OPTION_ suffix to it.
define([<_getopt_mangle_option>],
  [<[<_GETOPT_OPTION_>]patsubst($1, [<[^a-zA-Z0-9_]>], [<_>])>])

dnl _getopt_set_option(NAME[=VAL])
dnl ------------------------------
dnl Set option NAME.  
define([<_getopt_set_option>],
  [<ifelse(index([<$1>],=),-1,[<define(_getopt_mangle_option($1))>],
    [<define(regexp([<$1>],\([^=]+\)=\(.*\),
             [<_getopt_mangle_option(\1),[<\2>]>]))>])>])

dnl _getopt_get_option(NAME[,DEFAULT])
dnl ------------------------------
define([<_getopt_get_option>],
  [<_getopt_if_option_set($1,[<indir(_getopt_mangle_option($1))>],[<$2>])>])
  
dnl _getopt_if_option_set(NAME,IF-SET,IF-NOT-SET)
dnl ---------------------------------------------
dnl Check if option NAME is set.
define([<_getopt_if_option_set>],
  [<ifdef(_getopt_mangle_option([<$1>]),[<$2>],[<$3>])>])

dnl _getopt_option_switch(NAME1,IF-SET1,[NAME2,IF-SET2,[...]],[IF-NOT-SET])
dnl ------------------------------------------------------------------------
dnl If NAME1 is set, run IF-SET1.  Otherwise, if NAME2 is set, run IF-SET2.
dnl Continue the process for all name-if-set pairs within [...].  If none
dnl of the options is set, run IF-NOT-SET.
define([<_getopt_option_switch>],
  [<ifelse([<$4>],,[<_getopt_if_option_set($@)>],
           [<$3>],,[<_getopt_if_option_set($@)>],
     [<_getopt_if_option_set([<$1>],[<$2>],
                             [<_getopt_option_switch(shift(shift($@)))>])>])>])

dnl _getopt_if_option_val(NAME,val,IF-TRUE,IF-FALSE)
dnl ---------------------------------------------
dnl Check if option NAME is set.
define([<_getopt_if_option_val>],
  [<ifelse(_getopt_get_option([<$1>]),[<$2>],[<$3>],[<$4>])>])

define([<__getopt_switch_option_val>],
[<ifelse([<$#>],2,[<$2>],dnl
         [<$#>],3,[<ifelse([<$1>],[<$2>],[<$3>])>],dnl
[<ifelse([<$1>],[<$2>],[<$3>],dnl
[<__getopt_switch_option_val([<$1>],shift(shift(shift($@))))>])>])>])
	            
dnl _getopt_switch_option_val(NAME,val1,IF-VAL1,val2,IF-VAL2...,ELSE)
dnl -----------------------------------------------------------------
dnl Check if option NAME is set.
define([<_getopt_switch_option_val>],
[<pushdef([<val>],[<_getopt_get_option($1)>])dnl
__getopt_switch_option_val(val, shift($@))[<>]dnl
popdef([<val>])>])

dnl _getopt_set_options(OPTION[,OPTION...])
dnl ---------------------------------------
dnl Set options given as arguments.
define([<_getopt_set_options>],
  [<ifelse([<$1>],,,
     [<_getopt_set_option([<$1>])
       _getopt_set_options(shift($@))>])>])

dnl format_authors(name[,name...])
dnl ------------------------------
define([<format_authors>],dnl
	[<ifelse([<$1>],,NULL,[<"$1",
format_authors(shift($@))>])>])
       
dnl upcase(ARGS...)
dnl Concatenate and convert ARGS to upper case.
dnl
define([<upcase>], [<translit([<$*>], [<a-z>], [<A-Z>])>])

dnl concat(ARGS...)
dnl Concatenate arguments, inserting ", " between each of pair of them.
dnl
define([<concat>],[<ifelse([<$#>],1,[<$1>],[<$1, concat(shift($@))>])>])

dnl flushleft(ARGS...)
dnl Concatenate ARGS and remove any leading whitespace
dnl
define([<flushleft>],
 [<patsubst([<concat($*)>], [<^[	]+>])>])

dnl chop(ARGS...)
dnl Concatenate ARGS and remove any trailing whitespace
dnl
define([<chop>],
 [<patsubst([<$*>], [<[	]+$>])>])

dnl escape(ARGS...)
dnl Concatenate ARGS and escape any occurrences of double-quotes with
dnl backslashes.
dnl
define([<escape>],
[<patsubst([<concat($*)>],[<[\"]>],[<\\\&>])>])

dnl prep(ARG)
dnl Prepare ARG for including in C strings: replace newlines with any amount
dnl of preceding and following whitespace by a single space character, remove
dnl leading whitespace, and escape double-quotes.
dnl
define([<prep>],
 [<escape(flushleft(patsubst([<$1>],[<[	]*
+[	]*>],[< >])))>])

dnl SHORT_OPTS
dnl Accumulator for the 3rd argument of getopt_long
dnl
define([<SHORT_OPTS>],[<>])

dnl GROUP(STRING)
dnl Begin a named group of options
dnl
define([<GROUP>],[<dnl
divert(3)
	{ NULL, NULL, 0, N_("prep([<$1>])") },
divert(-1)>])

define([<__GATHER_OPTIONS>],[<
define([<KEY>],ifelse([<$2>],,[<OPTION_>]upcase(patsubst($1,-,_)),'$2'))
ifelse([<$2>],,[<
divert(1)
	KEY,
divert(-1)
>])
define([<SELECTOR>],ifdef([<SELECTOR>],SELECTOR) case KEY:)
ifelse([<$1>],,,[<
divert(2)
	{ "$1", ARGTYPE, 0, KEY },
divert(-1)>])
dnl
define([<SHORT_OPTS>],SHORT_OPTS[<>]dnl
ifelse([<$2>],,,$2[<>]ifelse(ARGTYPE,[<no_argument>],,ARGTYPE,[<required_argument>],:,ARGTYPE,[<optional_argument>],::)))
dnl
ifelse([<$1>],,,dnl
[<define([<LONG_TAG>],ifelse(LONG_TAG,,[<--$1>],[<LONG_TAG; --$1>]))>])
ifelse([<$2>],,,dnl
[<define([<SHORT_TAG>],ifelse(SHORT_TAG,,[<-$2>],[<SHORT_TAG; -$2>]))>])
>])

dnl OPTION(long-opt, short-opt, [arg], [descr])
dnl Introduce a command line option. Arguments:
dnl   long-opt     Long option.
dnl   short-opt    Short option (a single char)
dnl   (At least one of long-opt or short-opt must be present)
dnl
dnl   Optional arguments:
dnl   arg          Option argument.
dnl   descr        Option description
dnl
dnl If arg is absent, the option does not take any arguments. If arg is
dnl enclosed in square brackets, the option takes an optional argument.
dnl Otherwise, the argument is required.
dnl
dnl If descr is not given the option will not appear in the --help and
dnl --usage outputs.
dnl
define([<OPTION>],[<
pushdef([<LONG_TAG>])
pushdef([<SHORT_TAG>])
pushdef([<ARGNAME>],[<$3>])
pushdef([<HIDDEN>],ifelse([<$4>],,1,0))
pushdef([<DOCSTRING>],[<prep([<$4>])>])
pushdef([<ARGTYPE>],[<ifelse([<$3>],,[<no_argument>],dnl
patsubst([<$3>],[<\[.*\]>]),,[<optional_argument>],dnl
[<required_argument>])>])
__GATHER_OPTIONS($@)
>])

dnl ALIAS(long-opt, short-opt)
dnl Declare aliases for the previous OPTION statement.
dnl   long-opt     Long option.
dnl   short-opt    Short option (a single char)
dnl   (At least one of long-opt or short-opt must be present)
dnl An OPTION statement may be followed by any number of ALIAS statements.
dnl
define([<ALIAS>],[<
__GATHER_OPTIONS($1,$2)
>])

dnl BEGIN
dnl Start an action associated with the declared option. Must follow OPTION
dnl statement, with optional ALIAS statements in between.
dnl
define([<BEGIN>],[<
ifelse(HIDDEN,1,,[<
divert(3)
	{
#ifdef HAVE_GETOPT_LONG
	  "translit(dnl
ifelse(SHORT_TAG,,LONG_TAG,[<SHORT_TAG[<>]ifelse(LONG_TAG,,,; LONG_TAG)>]),
		    [<;>],[<,>])",
#else
	  "translit(SHORT_TAG, [<;>],[<,>])",
#endif
				   ifelse(ARGNAME,,[<NULL, 0>],
[<ifelse(ARGTYPE,[<optional_argument>],
[<N_(>]"[<patsubst(ARGNAME,[<\[\(.*\)\]>],[<\1>])>][<"), 1>],[<N_("ARGNAME"), 0>])>]), N_("DOCSTRING") },
divert(-1)>])
popdef([<ARGTYPE>])
popdef([<ARGNAME>])
popdef([<DOCSTRING>])
popdef([<HIDDEN>])
divert(4)dnl
popdef([<LONG_TAG>])dnl
popdef([<SHORT_TAG>])dnl
	SELECTOR
	  {
>])

dnl END
dnl Finish the associated action
dnl
define([<END>],[<
	     break;
	  }
divert(-1)
undefine([<SELECTOR>])>])

dnl OPTNODE(name, value)
define([<OPTNODE>],[<do {
   struct grecs_node *node = grecs_node_from_path($1, $2);
   if (!cmdline_tree)
	cmdline_tree = node;
   else
	grecs_node_bind(cmdline_tree, node, 0);
} while(0)
>])

dnl GETOPT(argc, argv, [long_index], [onerr])
dnl Emit option parsing code. Arguments:
dnl
dnl  argc        Name of the 1st argument to getopt_long.
dnl  argv        Name of the 2nd argument to getopt_long.
dnl  long_index  5th argument to getopt_long.  If not given,
dnl              NULL will be passed.
dnl  onerr       Action to take if parsing fails.
dnl
define([<GETOPT>],[<
 {
  int c;

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long($1, $2, "SHORT_OPTS",
			  long_options, NULL)) != EOF)
#else
  while ((c = getopt($1, $2, "SHORT_OPTS")) != EOF)
#endif
    {
      switch (c)
	{
	default:
	   ifelse([<$4>],,,[<$4;>])dnl
	   exit(EX_USAGE);
	undivert(4)
	}
    }
  ifelse([<$3>],,[<
    if (optind < argc) {
	fprintf(stderr, "%s: unexpected arguments\n", $2[0]);
	exit(EX_USAGE);
    }
>],[<$3 = optind;>])
  if (cmdline_tree)
    {
      struct grecs_node *rn = grecs_node_create(grecs_node_root, NULL);
      rn->down = cmdline_tree;
      cmdline_tree = rn;
    }
 }
>])

define([<STDFUNC>],[<
divert(0)
void print_help(void);
void print_usage(void);
divert(5)
const char *program_version = [<$1>];
static char doc[] = ifelse([<$3>],,"",N_("$3"));
static char args_doc[] = ifelse([<$4>],,"",N_("$4"));
const char *program_bug_address = "<" PACKAGE_BUGREPORT ">";
struct grecs_node *cmdline_tree;

#define DESCRCOLUMN 30
#define RMARGIN 79
#define GROUPCOLUMN 2
#define USAGECOLUMN 13

static void
indent (size_t start, size_t col)
{
  for (; start < col; start++)
    putchar (' ');
}

static void
print_option_descr (const char *descr, size_t lmargin, size_t rmargin)
{
  while (*descr)
    {
      size_t s = 0;
      size_t i;
      size_t width = rmargin - lmargin;

      for (i = 0; ; i++)
	{
	  if (descr[i] == 0 || descr[i] == ' ' || descr[i] == '\t')
	    {
	      if (i > width)
		break;
	      s = i;
	      if (descr[i] == 0)
		break;
	    }
	}
      printf ("%*.*s\n", s, s, descr);
      descr += s;
      if (*descr)
	{
	  indent (0, lmargin);
	  descr++;
	}
    }
}

#define NOPTHELP (sizeof (opthelp) / sizeof (opthelp[0]))

static int
optcmp (const void *a, const void *b)
{
  struct opthelp const *ap = (struct opthelp const *)a;
  struct opthelp const *bp = (struct opthelp const *)b;
  const char *opta, *optb;
  size_t alen, blen;

  for (opta = ap->opt; *opta == '-'; opta++)
    ;
  alen = strcspn (opta, ",");
  
  for (optb = bp->opt; *optb == '-'; optb++)
    ;
  blen = strcspn (optb, ",");

  if (alen > blen)
    blen = alen;
  
  return strncmp (opta, optb, blen);
}

static void
sort_options (int start, int count)
{
  qsort (opthelp + start, count, sizeof (opthelp[0]), optcmp);
}

static int
sort_group (int start)
{
  int i;
  
  for (i = start; i < NOPTHELP && opthelp[i].opt; i++)
    ;
  sort_options (start, i - start);
  return i + 1;
}

static void
sort_opthelp (void)
{
  int start;

  for (start = 0; start < NOPTHELP; )
    {
      if (!opthelp[start].opt)
	start = sort_group (start + 1);
      else 
	start = sort_group (start);
    }
}

void (*print_help_hook) (FILE *stream);

void
print_help (void)
{
  unsigned i;
  int argsused = 0;

  printf ("%s %s [%s]... %s\n", _("Usage:"), [<$2>], _("[<OPTION>]"),
	  args_doc[0] ? gettext (args_doc) : args_doc);
  if (doc[0])
    print_option_descr(gettext (doc), 0, RMARGIN);
  putchar ('\n');

  sort_opthelp ();
  for (i = 0; i < NOPTHELP; i++)
    {
      unsigned n;
      if (opthelp[i].opt)
	{
	  n = printf ("  %s", opthelp[i].opt);
	  if (opthelp[i].arg)
	    {
	      char *cb, *ce;
	      argsused = 1;
	      if (strlen (opthelp[i].opt) == 2)
		{
		  if (!opthelp[i].is_optional)
		    {
		      putchar (' ');
		      n++;
		    }
		}
	      else
		{
		  putchar ('=');
		  n++;
		}
	      if (opthelp[i].is_optional)
		{
		  cb = "[";
		  ce = "]";
		}
	      else
		cb = ce = "";
	      n += printf ("%s%s%s", cb, gettext (opthelp[i].arg), ce);
	    }
	  if (n >= DESCRCOLUMN)
	    {
	      putchar ('\n');
	      n = 0;
	    }
	  indent (n, DESCRCOLUMN);
	  print_option_descr (gettext (opthelp[i].descr), DESCRCOLUMN, RMARGIN);
	}
      else
	{
	  if (i)
	    putchar ('\n');
	  indent (0, GROUPCOLUMN);
	  print_option_descr (gettext (opthelp[i].descr),
			      GROUPCOLUMN, RMARGIN);
	  putchar ('\n');
	}
    }

  putchar ('\n');
dnl **************************************************************************
dnl This string cannot be split over serveal lines, because this would trigger
dnl a bug in GNU M4 (version 1.4.9 and 1.4.10), which would insert #line
dnl directives between the lines.
dnl **************************************************************************
  if (argsused)
    {
      print_option_descr (_("Mandatory or optional arguments to long options are also mandatory or optional for any corresponding short options."), 0, RMARGIN);
      putchar ('\n');
    }
 
  if (print_help_hook)
    print_help_hook (stdout);
    
 /* TRANSLATORS: The placeholder indicates the bug-reporting address
    for this package.  Please add _another line_ saying
    "Report translation bugs to <...>\n" with the address for translation
    bugs (typically your translation team's web or email address).  */
  printf (_("Report bugs to %s.\n"), program_bug_address);
  
#ifdef PACKAGE_URL
  printf (_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
_getopt_if_option_set([<gnupackage>],[<#else
  printf (_("%s home page: <http://www.gnu.org/software/%s/>\n"),
	  PACKAGE_NAME, PACKAGE);
#endif
  fputs (_("General help using GNU software: <http://www.gnu.org/gethelp/>\n"),
	 stdout);>],
[<#endif>])
}

static int
cmpidx_short (const void *a, const void *b)
{
  unsigned const *ai = (unsigned const *)a;
  unsigned const *bi = (unsigned const *)b;

  return opthelp[*ai].opt[1] - opthelp[*bi].opt[1];
}
  
#ifdef HAVE_GETOPT_LONG
static int
cmpidx_long (const void *a, const void *b)
{
  unsigned const *ai = (unsigned const *)a;
  unsigned const *bi = (unsigned const *)b;
  struct opthelp const *ap = opthelp + *ai;
  struct opthelp const *bp = opthelp + *bi;
  char const *opta, *optb;
  size_t lena, lenb;

  if (ap->opt[1] == '-')
    opta = ap->opt;
  else
    opta = ap->opt + 4;
  lena = strcspn (opta, ",");
  
  if (bp->opt[1] == '-')
    optb = bp->opt;
  else
    optb = bp->opt + 4;
  lenb = strcspn (optb, ",");
  return strncmp (opta, optb, lena > lenb ? lenb : lena);
}
#endif

void
print_usage (void)
{
  unsigned i;
  unsigned n;
  char buf[RMARGIN+1];
  unsigned *idxbuf;
  unsigned nidx;
  
#define FLUSH                      dnl
  do                               dnl
    {                              dnl
	  buf[n] = 0;              dnl
	  printf ("%s\n", buf);    dnl
	  n = USAGECOLUMN;         dnl
	  memset (buf, ' ', n);    dnl
    }                              dnl
  while (0)
#define ADDC(c) dnl
  do { if (n == RMARGIN) FLUSH; buf[n++] = c; } while (0)

  idxbuf = calloc (NOPTHELP, sizeof (idxbuf[0]));
  if (!idxbuf)
    {
      fprintf (stderr, "not enough memory");
      abort ();
    }

  n = snprintf (buf, sizeof buf, "%s %s ", _("Usage:"), [<$2>]);

  /* Print a list of short options without arguments. */
  for (i = nidx = 0; i < NOPTHELP; i++)
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
	  && opthelp[i].arg == NULL)
	idxbuf[nidx++] = i;

  if (nidx)
    {
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);

      ADDC ('[');
      ADDC ('-');
      for (i = 0; i < nidx; i++)
	{
	  ADDC (opthelp[idxbuf[i]].opt[1]);
	}
      ADDC (']');
    }

  /* Print a list of short options with arguments. */
  for (i = nidx = 0; i < NOPTHELP; i++)
    {
      if (opthelp[i].opt && opthelp[i].descr && opthelp[i].opt[1] != '-'
	  && opthelp[i].arg)
	idxbuf[nidx++] = i;
    }

  if (nidx)
    {
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_short);
    
      for (i = 0; i < nidx; i++)
	{
	  struct opthelp *opt = opthelp + idxbuf[i];
	  size_t len = 5 + strlen (opt->arg)
	                 + (opt->is_optional ? 2 : 1);
      
	  if (n + len > RMARGIN) FLUSH;
	  buf[n++] = ' ';
	  buf[n++] = '[';
	  buf[n++] = '-';
	  buf[n++] = opt->opt[1];
	  if (opt->is_optional)
	    {
	      buf[n++] = '[';
	      strcpy (&buf[n], opt->arg);
	      n += strlen (opt->arg);
	      buf[n++] = ']';
	    }
	  else
	    {
	      buf[n++] = ' ';
	      strcpy (&buf[n], opt->arg);
	      n += strlen (opt->arg);
	    }
	  buf[n++] = ']';
	}
    }
  
#ifdef HAVE_GETOPT_LONG
  /* Print a list of long options */
  for (i = nidx = 0; i < NOPTHELP; i++)
    {
      if (opthelp[i].opt && opthelp[i].descr
	  &&  (opthelp[i].opt[1] == '-' || opthelp[i].opt[2] == ','))
	idxbuf[nidx++] = i;
    }

  if (nidx)
    {
      qsort (idxbuf, nidx, sizeof (idxbuf[0]), cmpidx_long);
	
      for (i = 0; i < nidx; i++)
	{
	  struct opthelp *opt = opthelp + idxbuf[i];
	  size_t len;
	  const char *longopt;
	  
	  if (opt->opt[1] == '-')
	    longopt = opt->opt;
	  else if (opt->opt[2] == ',')
	    longopt = opt->opt + 4;
	  else
	    continue;

	  len = 3 + strlen (longopt)
	          + (opt->arg ? 1 + strlen (opt->arg)
		  + (opt->is_optional ? 2 : 0) : 0);
	  if (n + len > RMARGIN) FLUSH;
	  buf[n++] = ' ';
	  buf[n++] = '[';
	  strcpy (&buf[n], longopt);
	  n += strlen (longopt);
	  if (opt->arg)
	    {
	      buf[n++] = '=';
	      if (opt->is_optional)
		{
		  buf[n++] = '[';
		  strcpy (&buf[n], opt->arg);
		  n += strlen (opt->arg);
		  buf[n++] = ']';
		}
	      else
		{
		  strcpy (&buf[n], opt->arg);
		  n += strlen (opt->arg);
		}
	    }
	  buf[n++] = ']';
	}
    }
#endif
  FLUSH;
  free (idxbuf);
}

const char version_etc_copyright[] =
  /* Do *not* mark this string for translation.  First %s is a copyright
     symbol suitable for this locale, and second %s are the copyright
     years.  */
  "Copyright %s %s %s";

void
print_version_only(const char *program_version, FILE *stream)
{
  fprintf (stream, "%s\n", program_version);
  /* TRANSLATORS: Translate "(C)" to the copyright symbol
     (C-in-a-circle), if this symbol is available in the user's
     locale.  Otherwise, do not translate "(C)"; leave it as-is.  */
  fprintf (stream, version_etc_copyright, _("(C)"),
	   "_getopt_get_option(copyright_year, 2010)",
	   "_getopt_get_option(copyright_holder)");
  fputc ('\n', stream);
}

_getopt_if_option_set([<authors>],[<
char *program_author[] = {
format_authors(_getopt_get_option(authors))
};>])

void (*print_version_hook)(FILE *stream);

void
print_version(const char *program_version, FILE *stream)
{
  _getopt_if_option_set([<authors>],[<int i;
  unsigned width;
  const char *written_by = _("Written by ");
  /* TRANSLATORS: This string is used as a delimiter between authors' names
     as in:

           Written by Winnie the Pooh, Piglet ...
   */
  const char *middle_delim = _(", ");
  /* TRANSLATORS: This string acts as a delimiter before the last author's
     names, e.g.:
  
           Written by Winnie the Pooh, Piglet and Christopher Robin.
  */
  const char *final_delim = _(" and ");
  
>])
  print_version_only(program_version, stream);

dnl **************************************************************************
dnl This string cannot be split over serveal lines, because this would trigger
dnl a bug in GNU M4 (version 1.4.9 and 1.4.10), which would insert #line
dnl directives between the lines.
dnl **************************************************************************
  fputs (_("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n"),
	 stream);
  if (print_version_hook)
    print_version_hook (stream);
_getopt_if_option_set([<authors>],[<
  width = strlen (written_by);
  fputs (written_by, stream);
  for (i = 0; ; )
    {
      char *author = program_author[i++];
      size_t len = strlen (author);
      const char *delim = NULL;
      
      if (program_author[i])
        {
	  delim = program_author[i+1] ? middle_delim : final_delim;
	  len += strlen (delim);
	}
      else
        len++;
      if (width + len > RMARGIN)
	{
	  fputc ('\n', stream);
	  width = 0;
	}
      fputs (author, stream);
      width += len;
      if (delim)
	fputs (delim, stream);
      else
	break;
    }
  fputc ('.', stream);
  fputc ('\n', stream);
>])		
}

divert(-1)
popdef([<ADDC>])
popdef([<FLUSH>])
>])

define([<OPTIONS_BEGIN>],
   [<divert(-1)
     _getopt_set_options(shift(shift(shift($@))))
     _getopt_if_option_set([<gnu>],
        [<STDFUNC([<$1 " (" PACKAGE_NAME ") " PACKAGE_VERSION>],
	          [<$1>], [<$2>], [<$3>])>])
>])

define([<OPTIONS_END>],[<
_getopt_if_option_set([<gnu>],[<
	 GROUP([<Other options>])
	 OPTION([<help>],h,,[<Give this help list>])
	 BEGIN
		print_help ();
		exit (0);
	 END
	 OPTION([<usage>],,,[<Give a short usage message>])
	 BEGIN
		print_usage ();
		exit (0);
	 END
	 OPTION([<version>],V,,[<Print program version>])
	 BEGIN
		/* Give version */
		print_version(program_version, stdout);
		exit (0);
	 END>])
divert
#include <grecs.h>
/* Option codes */
enum {
	_OPTION_INIT=255,
	undivert(1)
	MAX_OPTION
};
#ifdef HAVE_GETOPT_LONG
static struct option long_options[] = {
	undivert(2)
	{0, 0, 0, 0}
};
#endif
static struct opthelp {
	const char *opt;
	const char *arg;
	int is_optional;
	const char *descr;
} opthelp[] = {
	undivert(3)
};
undivert(5)
>])

divert(0)dnl
/* -*- buffer-read-only: t -*- vi: set ro:
   THIS FILE IS GENERATED AUTOMATICALLY.  PLEASE DO NOT EDIT.
*/
