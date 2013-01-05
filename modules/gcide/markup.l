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
#include <errno.h>
#include <sysexits.h>
#include <setjmp.h>    
#include <appi18n.h>
#include "gcide.h"
#include "wordsplit.h"

#define yy_create_buffer      gcide_markup_yy_create_buffer       
#define yy_delete_buffer      gcide_markup_yy_delete_buffer       
#define yy_flex_debug	      gcide_markup_yy_flex_debug          
#define yy_init_buffer	      gcide_markup_yy_init_buffer         
#define yy_flush_buffer	      gcide_markup_yy_flush_buffer        
#define yy_load_buffer_state  gcide_markup_yy_load_buffer_state   
#define yy_switch_to_buffer   gcide_markup_yy_switch_to_buffer    
#define yyin		      gcide_markup_yyin                   
#define yyleng		      gcide_markup_yyleng                 
#define yylex		      gcide_markup_yylex                  
#define yylineno	      gcide_markup_yylineno               
#define yyout		      gcide_markup_yyout                  
#define yyrestart	      gcide_markup_yyrestart              
#define yytext		      gcide_markup_yytext                 
#define yywrap		      gcide_markup_yywrap                 
#define yyalloc		      gcide_markup_yyalloc                
#define yyrealloc	      gcide_markup_yyrealloc              
#define yyfree                gcide_markup_yyfree
#define yyunput               gcide_markup_yyunput

#define yylex_destroy         gcide_markup_yylex_destroy  
#define yyget_debug	      gcide_markup_yyget_debug    
#define yyset_debug	      gcide_markup_yyset_debug    
#define yyget_extra	      gcide_markup_yyget_extra    
#define yyset_extra	      gcide_markup_yyset_extra    
#define yyget_in	      gcide_markup_yyget_in       
#define yyset_in	      gcide_markup_yyset_in       
#define yyget_out	      gcide_markup_yyget_out      
#define yyset_out	      gcide_markup_yyset_out      
#define yyget_leng	      gcide_markup_yyget_leng     
#define yyget_text	      gcide_markup_yyget_text     
#define yyget_lineno	      gcide_markup_yyget_lineno   
#define yyset_lineno	      gcide_markup_yyset_lineno   
    
}
%{

static char const *input_buf;
static size_t input_len;
static unsigned token_beg;
static unsigned token_end;

static char *textspace;  /* Text storage space */
static size_t textsize;  /* Size of text space */
static size_t textpos;   /* Current position in the text space */
static size_t textstart; /* Start of the current text segment */

static dico_list_t tagstk;
static struct gcide_tag *current_tag;

static jmp_buf errbuf;

static void
memerr(const char *text)
{
    dico_log(L_ERR, ENOMEM, "%s", text);
    longjmp(errbuf, 1);
}

#define YY_USER_ACTION do {						\
	token_beg = token_end;						\
	token_end += yyleng;						\
    } while (0);
#define YY_INPUT(buf,result,max_size) do {				\
	size_t __n = (max_size) > input_len ? input_len : (max_size);	\
	if (__n)							\
	    memcpy((buf), input_buf, __n);				\
	input_len -= __n;						\
	(result) = __n;							\
    } while(0)

static int retstate;
static char *endtag;
#define BEGIN_COMMENT(end) \
    { retstate = YYSTATE; endtag = (end); BEGIN(COMMENT); }

static void
text_add_str(char const *s, size_t l)
{
    size_t rest = textsize - textpos;
    if (rest < l) {
	size_t nsize = 2 * textsize;
	char *newp = realloc(textspace, nsize);
	if (!newp)
	    memerr("text_add");
	textspace = newp;
	textsize = nsize;
    }
    memcpy(textspace + textpos, s, l);
    textpos += l;
}

static void
text_add_chr(int ch)
{
    char c = ch;
    text_add_str(&c, 1);
}

#define text_segment_length() (textpos - textstart)

static size_t 
text_segment_finish()
{
    size_t ret = textstart;
    text_add_chr(0);
    textstart = textpos;
    return ret;
}

static struct gcide_tag *
alloc_tag(const char *text, size_t len)
{
    struct gcide_tag *tag;
    struct wordsplit ws;

    if (wordsplit_len(text, len, &ws, WRDSF_DEFFLAGS & ~WRDSF_CESCAPES)) {
	dico_log(L_ERR, 0, _("cannot parse line %.*s: %s"),
		 len, text, wordsplit_strerror(&ws));
	longjmp(errbuf, 1);
    }
    tag = calloc(1, sizeof(*tag));
    if (tag) {
	tag->tag_parmc = ws.ws_wordc;
	tag->tag_parmv = ws.ws_wordv;
	ws.ws_wordc = 0;
	ws.ws_wordv = NULL; 
    } else
	memerr("gcide alloc_tag");
    wordsplit_free(&ws);
    return tag;
}

static int
free_tag(void *item, void *data)
{
    struct gcide_tag *tag = item;

    if (!tag)
	return 0;
    switch (tag->tag_type) {
    case gcide_content_unspecified:
    case gcide_content_text:
	break;
    case gcide_content_taglist:
	dico_list_destroy(&tag->tag_v.taglist);
    }
    return 0;
}

static void
append_tag(struct gcide_tag *tag)
{
    dico_list_t list;
	
    switch (current_tag->tag_type) {
    case gcide_content_unspecified:
	list = dico_list_create();
	if (!list) {
	    free(tag);
	    memerr("append_tag");
	}
	dico_list_set_free_item(list, free_tag, NULL);
	current_tag->tag_type = gcide_content_taglist;
	current_tag->tag_v.taglist = list;
	break;
	
    case gcide_content_text: {
	struct gcide_tag *subtag = calloc(1, sizeof(*tag));
	
	if (!subtag)
	    memerr("append_tag");
	subtag->tag_type = gcide_content_text;
	subtag->tag_v.textpos = current_tag->tag_v.textpos;
	list = dico_list_create();
	if (!list) {
	    free(subtag);
	    free(tag);
	    memerr("append_tag");
	}
	dico_list_set_free_item(list, free_tag, NULL);
	dico_list_append(list, subtag);
	current_tag->tag_type = gcide_content_taglist;
	current_tag->tag_v.taglist = list;
	break;
    }
	
    case gcide_content_taglist:
	break;
    }
    
    dico_list_append(current_tag->tag_v.taglist, tag);
}

static int in_grk;

static void
push_tag(struct gcide_tag *tag)
{
    append_tag(tag);
    dico_list_push(tagstk, current_tag);
    current_tag = tag;
    in_grk =
	current_tag->tag_parmc && strcmp(current_tag->tag_name, "grk") == 0;
}

static void
pop_tag(const char *tagstr, size_t taglen)
{
    size_t len;

    for (len = 0; len < taglen; len++)
	if (tagstr[len] == ' ' || tagstr[len] == '\t')
	    break;

    if (len == 3 && memcmp(tagstr, "grk", 3) == 0)
	in_grk = 0;
    
    if (TAG_HAS_NAME(current_tag) &&
	strlen(current_tag->tag_name) == len &&
	memcmp(current_tag->tag_name, tagstr, len) == 0)
	current_tag = dico_list_pop(tagstk);
    else
	dico_log(L_WARN, 0, "%u: unexpected close tag", token_beg);
}
%}

%option 8bit
%option nounput

%x COMMENT

XD [0-9a-f]
%%
<INITIAL>{
  "<--"    BEGIN_COMMENT("-->");
  "<!"     BEGIN_COMMENT("!>");
  "<p>"|"</p>" ;
  "<"[a-zA-Z][^/>]*">" {
      struct gcide_tag *tag;

      if (text_segment_length()) {
	  tag = calloc(1, sizeof(*tag));
	  if (!tag)
	      memerr("gcide lexer");
	  tag->tag_type = gcide_content_text;
	  tag->tag_v.textpos = text_segment_finish();
	  append_tag(tag);
      }
      
      push_tag(alloc_tag(yytext + 1, yyleng - 2));
  }
  "</"[a-zA-Z][^>]*">" {
      if (text_segment_length()) {
	  struct gcide_tag *tag = calloc(1, sizeof(*tag));
	  if (!tag)
	      memerr("gcide lexer");
	  tag->tag_type = gcide_content_text;
	  tag->tag_v.textpos = text_segment_finish();
	  append_tag(tag);
      }
      pop_tag(yytext + 2, yyleng - 3);
  }
  "<"[a-zA-Z?][a-zA-Z0-9]*"/" {
      char const *s = gcide_entity_to_utf8(yytext);
      if (s)
	  text_add_str(s, strlen(s));
      else
	  dico_log(L_WARN, 0, _("%u: unrecognized entity: %s"),
		   token_beg, yytext);
  }
  [""*`]   { if (in_grk)  text_add_chr(yytext[0]); }
  "\\'"{XD}{XD} {
      char const *s = gcide_escape_to_utf8(yytext+2);
		 
      if (s)
	  text_add_str(s, strlen(s));
      else {
	  text_add_str(yytext, yyleng);
	  dico_log(L_WARN, 0,
		   _("%u: unknown character sequence %s"),
		   token_beg, yytext);
      }
  }
  \r    ;
  .     text_add_str(yytext, yyleng);
  \n    text_add_str(yytext, yyleng);
}
<COMMENT>{
  [^< \t][^ \t>]*">" {
      if (strcmp(yytext, endtag) == 0)
	  BEGIN(retstate);
  }
  .          ;
  \n         ;
}
%%

int
yywrap()
{
    return 1;
}

struct walk_closure {
    int (*fun)(int, struct gcide_tag *, void *);
    void *data;
};
    
static int
inorder_helper(void *item, void *data)
{
    struct gcide_tag *tag = item;
    struct walk_closure *cp = data;

    if (cp->fun(0, tag, cp->data))
	return 1;
    if (tag->tag_type == gcide_content_taglist) {
	dico_list_iterate(tag->tag_v.taglist, inorder_helper, data);
	if (cp->fun(1, tag, cp->data))
	    return 1;
    }
    return 0;
}

int
gcide_parse_tree_inorder(struct gcide_parse_tree *tp,
			 int (*fun)(int, struct gcide_tag *, void *),
			 void *data)
{
    struct walk_closure clos;
    clos.fun = fun;
    clos.data = data;
    return inorder_helper(tp->root, &clos);
}


static int
tag_fixup(void *item, void *data)
{
    struct gcide_tag *tag = item;
    char *textspace = data;

    switch (tag->tag_type) {
    case gcide_content_unspecified:
	break;
    case gcide_content_text:
	tag->tag_v.text = textspace + tag->tag_v.textpos;
	break;
    case gcide_content_taglist:
	dico_list_iterate(tag->tag_v.taglist, tag_fixup, textspace);
	break;
    }
    return 0;
}

static size_t
greek_translit(size_t n)
{
    while (textspace[n]) {
	size_t rd;
	const char *greek = gcide_grk_to_utf8(textspace + n, &rd);

	if (greek) {
	    text_add_str(greek, strlen(greek));
	    n += rd;
	} else {
	    text_add_chr(textspace[n]);
	    n++;
	}
    }
    return text_segment_finish();
}

static int
early_fixup(void *item, void *data)
{
    struct gcide_tag *tag = item;
    int translate = data ? *(int*)data : 0;
    
    switch (tag->tag_type) {
    case gcide_content_unspecified:
	break;
    case gcide_content_text:
	if (translate)
	    tag->tag_v.textpos = greek_translit(tag->tag_v.textpos);
	break;
    case gcide_content_taglist:
	translate = (tag->tag_parmc && strcmp(tag->tag_name, "grk") == 0);
	dico_list_iterate(tag->tag_v.taglist, early_fixup, &translate);
	break;
    }
    return 0;
}

struct gcide_parse_tree *
gcide_markup_parse(char const *text, size_t len, int dbg)
{
    struct gcide_parse_tree *tp;
    struct gcide_tag *p;
    
    input_buf = text;
    input_len = len;
    token_beg = token_end = 0;

    if (setjmp(errbuf)) {
	free(textspace);
	dico_list_destroy(&tagstk);
	free_tag(current_tag, NULL);
    }
    
    textsize = 2 * len;
    textspace = malloc(textsize);
    if (!textspace)
	memerr("gcide_markup_parse");
    textpos = textstart = 0;

    tagstk = dico_list_create();
    if (!tagstk)
	memerr("gcide_markup_parse");
    dico_list_set_free_item(tagstk, free_tag, NULL);
    current_tag = calloc(1, sizeof(*current_tag));
    if (!current_tag)
	memerr("gcide_markup_parse");

    yy_flex_debug = dbg;
    BEGIN(INITIAL);
    while (yylex ())
        ;

    /* Append trailing text segment, if any */
    if (text_segment_length()) {
	struct gcide_tag *tag = calloc(1, sizeof(*tag));
	if (!tag)
	    memerr("gcide lexer");
	tag->tag_type = gcide_content_text;
	tag->tag_v.textpos = text_segment_finish();
	append_tag(tag);
    }
    
    while (p = dico_list_pop(tagstk))
	/* FIXME: Report unclosed tag */
	current_tag = p;

    dico_list_destroy(&tagstk);
    
    if (!current_tag) {
	free(textspace);
	return NULL;
    }
    
    early_fixup(current_tag, NULL);
    
    tp = malloc(sizeof(*tp));
    if (!tp)
	memerr("gcide_markup_parse");

    tp->textspace = textspace;
    tp->textsize = textsize;
    
    tp->root = current_tag;
    
    tag_fixup(tp->root, textspace);
    
    return tp;
}

void
gcide_parse_tree_free(struct gcide_parse_tree *tp)
{
    free(tp->textspace);
    free_tag(tp->root, NULL);
    free(tp);
}

