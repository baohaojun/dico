%top {
/* This file is part of GNU Dico.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <unistd.h>
#include <getopt.h>    
#include <stdio.h>
#include <stdlib.h>    
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
#include <obstack.h>
#include <errno.h>
#include <sysexits.h>    
#include <appi18n.h>
#include "gcide.h"
}
%{
    
int verbose_option;
int dry_run_option;

char *dictdir, *idxdir, *infile, *suf;
char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int letno;
unsigned line;

char *idxname;
FILE *idxfile;
struct gcide_idx_header idx_header = {
    GCIDE_IDX_MAGIC,
    GCIDE_IDX_HEADER_PAGESIZE
};
struct gcide_idx_page *idx_page;

unsigned long char_position;
unsigned long file_position;
unsigned long headword_position;
struct obstack stk;
struct obstack refstk;
int retstate;
char *endtag;

#define YY_USER_ACTION do {						\
    char_position = file_position;                                      \
    file_position += yyleng;				                \
    } while (0);

static void
full_write(void *data, size_t size)
{
    if (fwrite(data, size, 1, idxfile) != 1) {
	dico_log(L_ERR, errno, _("write error"));
	exit(EX_UNAVAILABLE);
    }
}

static void
flush_page()    
{
    if (idx_page->ipg_header.hdr.phdr_numentries == 0 ||
	idx_page->ipg_header.hdr.phdr_text_offset == 0) {
	dico_log(L_ERR, 0, _("page too small, aborting"));
	exit(EX_UNAVAILABLE);
    }
    full_write(idx_page, idx_header.ihdr_pagesize);
    memset(idx_page, 0, idx_header.ihdr_pagesize);
    idx_page->ipg_header.hdr.phdr_text_offset = idx_header.ihdr_pagesize / 2;
    idx_header.ihdr_num_pages++;
}

static void
store_ref(struct gcide_ref *ref)
{
    char *textarea;
    
    if (idx_page->ipg_header.hdr.phdr_numentries == idx_header.ihdr_maxpageref
	|| idx_page->ipg_header.hdr.phdr_text_offset + ref->ref_hwbytelen >
          	idx_header.ihdr_pagesize)
	flush_page();

    ref->ref_hwoff = idx_page->ipg_header.hdr.phdr_text_offset;
    textarea = (char*)idx_page + idx_page->ipg_header.hdr.phdr_text_offset;
    memcpy(textarea, ref->ref_headword, ref->ref_hwbytelen);
    idx_page->ipg_header.hdr.phdr_text_offset += ref->ref_hwbytelen;
    memcpy(idx_page->ipg_ref + idx_page->ipg_header.hdr.phdr_numentries++,
	   ref, sizeof(ref[0]));
}

static int
refcmp(const void *a, const void *b)
{
    struct gcide_ref const *aref = a;
    struct gcide_ref const *bref = b;

    return utf8_strcasecmp(aref->ref_headword, bref->ref_headword);
}

static void
flush_refs()
{
    int i;
    struct gcide_ref *ref = obstack_finish(&refstk);

    if (verbose_option)
        dico_log(L_INFO, 0, _("Sorting references"));
    qsort(ref, idx_header.ihdr_num_headwords, sizeof(ref[0]), refcmp);
    
    if (verbose_option)
        dico_log(L_INFO, 0, _("Writing references"));

    for (i = 0; i < idx_header.ihdr_num_headwords; i++, ref++)
	store_ref(ref);
    if (idx_page->ipg_header.hdr.phdr_numentries)
	flush_page();
}

static void
flush_headwords()
{
    char *p;
    unsigned long size;
    char *headword;

    obstack_1grow(&stk, 0);
    headword = obstack_finish(&stk);
    if (headword[0]) {
	size = char_position - headword_position;
	for (p = headword; *p; p += strlen(p) + 1) {
	    struct gcide_ref ref;
	
	    if (verbose_option > 1)
		dico_log(L_INFO, 0, "%s: %c %lu %lu", p, letters[letno-1],
			 headword_position, size);
	    memset(&ref, 0, sizeof(ref));

	    ref.ref_hwbytelen = strlen(p) + 1;
	    ref.ref_hwlen = utf8_strlen(p);
	    ref.ref_letter = letters[letno-1];
	    ref.ref_offset = headword_position;
	    ref.ref_size = size;
	    ref.ref_headword = p;
	    obstack_grow(&refstk, &ref, sizeof(ref));
	    idx_header.ihdr_num_headwords++;
	}
	idx_header.ihdr_num_defs++;
    }
    headword = NULL;
    headword_position = char_position;
}

#define BEGIN_HEADWORD(end) \
    { retstate = YYSTATE; endtag = (end); BEGIN(HEADWORD); }

%}
%option 8bit
%option nounput

%x HEADWORD MHW COMMENT HTMLCOM

XD [0-9a-f]
%%
<INITIAL>{
  "<--"      { BEGIN(COMMENT); }
  "<!"       { BEGIN(HTMLCOM); }
  ("\\'d8")?"<hw>"  {
	       flush_headwords();
	       BEGIN_HEADWORD("hw");
             }
  "<p><mhw>"|"<mhw>" {
	       flush_headwords();
               BEGIN(MHW);
             }
  "<p><ent>" flush_headwords();
  "<asp>"     BEGIN_HEADWORD("asp");
  "<altname>" BEGIN_HEADWORD("altname");
  "<decf>"    BEGIN_HEADWORD("decf");
  "<colf>"    BEGIN_HEADWORD("colf");
  "<conjf>"   BEGIN_HEADWORD("conjf");
  .          ;
  \n         line++;
}
<MHW>{
  "<hw>"     BEGIN_HEADWORD("hw");
  "</mhw>"   { BEGIN(INITIAL); }
  .          ;
  \n         line++;
}
<HEADWORD>{
  "</"[a-zA-Z][a-zA-Z]*">" {
      if (yyleng == strlen(endtag) + 3 &&
	  memcmp(endtag, yytext + 2, yyleng-3) == 0) {
	  BEGIN(retstate);
	  obstack_1grow(&stk, 0);
      }
  }
  "<"[a-zA-Z?][a-zA-Z0-9]*"/" {
      char const *s = gcide_entity_to_utf8(yytext);
      if (s) {
	  if (strcmp(s, "<?>") == 0)
	      dico_log(L_WARN, 0,
		       _("%s:%u: unknown or illegible character in a headword"),
		       infile, line);
          obstack_grow(&stk, s, strlen(s));
      } else
	  dico_log(L_WARN, 0, _("%s:%u: unrecognized entity: %s"),
		   infile, line, yytext);
  }
  "<"[a-zA-Z][^/>]*">"   /* ignore tag */;
  [""*`]      ;
  "\\'d8"     ;       /* ignore double vertical bar */
  "\\'"{XD}{XD} {
                 char const *s = gcide_escape_to_utf8(yytext+2);

                 if (s) {
                     int len = strlen(s);
                     obstack_grow(&stk, s, len);
                 } else {
                     obstack_grow(&stk, yytext, yyleng);
	             dico_log(L_WARN, 0,
                              _("%s:%u: unknown character sequence %s"),
		              infile, line, yytext);
                 }
             }
  .          obstack_grow(&stk, yytext, yyleng);
  \n         line++;
}
<COMMENT>{
  "-->"      { BEGIN(INITIAL); }
  .          ;
  \n         line++;
}
<HTMLCOM>{
  "!>"       { BEGIN(INITIAL); }
  .          ;
  \n         line++;
}
%%

int
yywrap()
{
    flush_headwords();

    if (letters[letno] == 0)
        return 1;
    *suf = letters[letno++];
    if (yyin)
         fclose(yyin);
    if (verbose_option)
         dico_log(L_INFO, 0, _("Indexing %s"), infile);
    yyin = fopen(infile, "r");
    if (!yyin) {
         dico_log(L_ERR, errno, _("cannot open file %s"), infile);
         exit(EX_NOINPUT);
    }
    file_position = char_position = headword_position = 0;
    line = 1;
    return 0;
}

#include "idxgcide-cli.h"

/* Usage: idxgcide DICTDIR [IDXDIR] */
int
main(int argc, char **argv)
{
    int index;
    
    appi18n_init();
    dico_set_program_name(argv[0]);
    yy_flex_debug = 0;

    get_options(argc, argv, &index);
    
    idx_page = malloc(idx_header.ihdr_pagesize);
    if (!idx_page) {
	dico_log(L_ERR, 0, _("not enough memory"));
	exit(EX_UNAVAILABLE);
    }
    idx_page->ipg_header.hdr.phdr_text_offset = idx_header.ihdr_pagesize / 2;

    idx_header.ihdr_maxpageref = idx_header.ihdr_pagesize / 2 /
	                         sizeof(struct gcide_ref);
    if (idx_header.ihdr_maxpageref < 1) {
	dico_log(L_ERR, 0, _("page size too small"));
	exit(EX_USAGE);
    }
    idx_header.ihdr_maxpageref--;
    
    argc -= index;
    argv += index;
    
    if (argc < 1 || argc > 2) {
        dico_log(L_ERR, 0, _("bad number of arguments"));
        exit(EX_USAGE);
    }
    dictdir = argv[0];
    idxdir = argc == 2 ? argv[1] : dictdir;

    infile = dico_full_file_name(dictdir, "CIDE.A");
    if (!infile) {
        dico_log(L_ERR, 0, _("not enough memory"));
        exit(EX_UNAVAILABLE);
    }    
    suf = infile + strlen(infile) - 1;
    
    if (dry_run_option)
	idxname = "/dev/null";
    else 
	idxname = dico_full_file_name(idxdir, "GCIDE.IDX");
    if (!idxname) {
        dico_log(L_ERR, 0, _("not enough memory"));
        exit(EX_UNAVAILABLE);
    }
    
    idxfile = fopen(idxname, "w");
    if (!idxfile) {
        dico_log(L_ERR, errno, _("cannot create index file `%s'"), idxname);
        exit(EX_CANTCREAT);
    }    
    fseek(idxfile, idx_header.ihdr_pagesize, SEEK_SET);
    
    obstack_init(&stk);
    obstack_init(&refstk);

    letno = 0;
    yywrap();
    while (yylex ())
        ;

    flush_refs();

    fseek(idxfile, 0, SEEK_SET);
    full_write(&idx_header, sizeof(idx_header));
    fclose(idxfile);
	
    if (verbose_option)
        dico_log(L_INFO, 0,
		 _("headwords=%lu, definitions=%lu, refs per page=%lu, "
		   "pages=%lu"),
                 idx_header.ihdr_num_headwords,
                 idx_header.ihdr_num_defs,
		 idx_header.ihdr_maxpageref,
		 idx_header.ihdr_num_pages);

    exit (0);
}

/* Local Variables: */
/* mode: c */
/* eval: (setq buffer-file-coding-system (if window-system 'utf-8 'raw-text-unix)) */
/* End: */
