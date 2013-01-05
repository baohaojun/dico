/* grecs - Gray's Extensible Configuration System       -*- c -*- */
%top {
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
}
%{
/* grecs - Gray's Extensible Configuration System
   Copyright (C) 2007-2012 Sergey Poznyakoff

   Grecs is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   Grecs is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with Grecs. If not, see <http://www.gnu.org/licenses/>. */

#include <grecs.h>
#include <grecs-locus.h>
#include <bind-gram.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
	
#include <wordsplit.h>

#define YY_USER_ACTION do {						\
		if (YYSTATE == 0) {					\
			yylloc.beg = grecs_current_locus_point;		\
			yylloc.beg.col++;				\
		}							\
  		grecs_current_locus_point.col += yyleng;		\
 		yylloc.end = grecs_current_locus_point;			\
   	} while (0);
%}
    
%x COMMENT STR

WS [ \t\f][ \t\f]*
ID [a-zA-Z_][a-zA-Z_0-9-]*
P [1-9][0-9]*

%%
         /* C-style comments */
"/*"           BEGIN(COMMENT);
<COMMENT>[^*\n]*        /* eat anything that's not a '*' */
<COMMENT>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<COMMENT>\n             grecs_locus_point_advance_line(grecs_current_locus_point);
<COMMENT>"*"+"/"        BEGIN(INITIAL);
         /* Line directive */
^[ \t]*#[ \t]*{P}[ \t]+\".*\".*\n { grecs_parse_line_directive_cpp(yytext,
						    &yylloc,
						    &grecs_current_locus_point,
						    NULL); }
^[ \t]*#[ \t]*line[ \t].*\n       { grecs_parse_line_directive(yytext,
						    &yylloc,
						    &grecs_current_locus_point,
						    NULL); }
         /* End-of-line comments */
#.*\n     { grecs_locus_point_advance_line(grecs_current_locus_point); }
#.*     /* end-of-file comment */;
"//".*\n  { grecs_locus_point_advance_line(grecs_current_locus_point); }
"//".*    /* end-of-file comment */;
        /* Identifiers */
{ID}      { grecs_line_begin();
	    grecs_line_add(yytext, yyleng);
	    yylval.string = grecs_line_finish();
	    return strcmp(yylval.string, "controls") == 0 ?
		    BIND_CONTROLS : BIND_IDENT;
          }
         /* Strings */
[a-zA-Z0-9_\.\*/:@-]([a-zA-Z0-9_\./:@-][a-zA-Z0-9_\.\*/:@-]*)? {
	                   grecs_line_begin();
	                   grecs_line_add(yytext, yyleng);
                           yylval.string = grecs_line_finish();
                           return BIND_STRING; }
         /* Quoted strings */
\"[^\\"\n]*\"         { grecs_line_begin();
                        grecs_line_add(yytext + 1, yyleng - 2);
                        yylval.string = grecs_line_finish();
                        return BIND_STRING; }
\"[^\\"\n]*\\. |
\"[^\\"\n]*\\\n        { BEGIN(STR);
                         grecs_line_begin();
		         grecs_line_acc_grow_unescape_last(yytext + 1,
                                                           yyleng - 1); }
\"[^\\"\n]*\n          { BEGIN(STR);
                         grecs_line_begin();
		         grecs_line_acc_grow(yytext + 1, yyleng - 1); }
<STR>[^\\"\n]*\\. |
<STR>\"[^\\"\n]*\\\n  { grecs_line_acc_grow_unescape_last(yytext, yyleng); }
<STR>[^\\"\n]*\n |
<STR>\"[^\\"\n]*\n    { grecs_line_acc_grow(yytext, yyleng); }
<STR>[^\\"\n]*\"      { BEGIN(INITIAL);
                        if (yyleng > 1) 
				grecs_line_add(yytext, yyleng - 1); 
                        yylval.string = grecs_line_finish();
		        return BIND_STRING; }
{WS}     ;
         /* Other tokens */
\n       { grecs_locus_point_advance_line(grecs_current_locus_point); } 
[,;{}()!] return yytext[0];
.        { if (isascii(yytext[0]) && isprint(yytext[0]))
		grecs_error(&yylloc, 0,
			     _("stray character %c"), yytext[0]);
           else
                grecs_error(&yylloc, 0, _("stray character \\%03o"),
		   	    (unsigned char) yytext[0]); }
%%

struct bind_input_context {
	ino_t i_node;
	dev_t i_dev;
        struct grecs_locus_point point;
	grecs_locus_t locus;		/* Current input location */
	YY_BUFFER_STATE state;
	FILE *input;
};

static struct grecs_list *input_stack;
static ino_t i_node;
static dev_t i_dev;

static void
free_context(void *ptr)
{
	free(ptr);
}

static int
cmp_context(const void *a, const void *b)
{
	struct bind_input_context const *ac = a;
	struct bind_input_context const *bc = b;

	return !(ac->i_node == bc->i_node && ac->i_dev == bc->i_dev);
}

static int
_push_context(const char *name, ino_t i_node, dev_t i_dev, grecs_locus_t *loc)
{
	struct bind_input_context ctx, *pctx;
	
	if (!input_stack) {
		input_stack = grecs_list_create();
		input_stack->free_entry = free_context;
		input_stack->cmp = cmp_context;
	} else {
		ctx.i_dev = i_dev;
		ctx.i_node = i_node;
		pctx = grecs_list_locate(input_stack, &ctx);
		if (pctx) {
			grecs_error(&yylloc, 0,
				    _("%s has already been included"), name);
			grecs_error(&pctx->locus, 0,
			    _("this is where the previous inclusion occurred"));
			return 1;
		}

		pctx = grecs_malloc(sizeof(*pctx));
		pctx->i_node = i_node;
		pctx->i_dev = i_dev;
                if (loc)
                        pctx->locus = *loc;
                else
                        memset(&pctx->locus, 0, sizeof(pctx->locus)); /* FIXME */
		pctx->point = grecs_current_locus_point;
		pctx->state = YY_CURRENT_BUFFER;
		pctx->input = yyin;
		grecs_list_push(input_stack, pctx);
	}
	return 0;
}

static int
_pop_context()
{
	struct bind_input_context *pctx;

	if (!yyin)
		return 1;
        if (grecs_preprocessor)
                pclose(yyin);
        else
	        fclose(yyin);
	pctx = grecs_list_pop(input_stack);
	if (!pctx) {
		yyin = NULL;
		return 1;
	}
	i_node = pctx->i_node;
	i_dev = pctx->i_dev;
	grecs_current_locus_point = pctx->point;
	yyin = pctx->input;
	yy_delete_buffer(YY_CURRENT_BUFFER);
	yy_switch_to_buffer(pctx->state);
	grecs_free(pctx);
	return 0;
}

int
yywrap()
{
	return _pop_context();
}

int
grecs_bind_new_source(const char *name, grecs_locus_t *loc)
{
	struct stat st;
	FILE *fp;
	
	if (access(name, F_OK)) {
		int ec = errno;
		char *tmp = grecs_find_include_file(name, 0);
		if (!tmp) {
			grecs_error(loc, ec, _("cannot open `%s'"), name);
			return 1;
		}
		name = grecs_install_text(tmp);
		free(tmp);
	}
	
	fp = fopen(name, "r");
	if (!fp) {
		grecs_error(loc, errno, _("cannot open `%s'"), name);
		return 1;
 	}
	if (fstat(fileno(fp), &st)) {
		grecs_error(loc, errno, _("can't state %s"), name);
		fclose(fp);
		return 1;
	}
        if (grecs_preprocessor) {
                char *cmd = NULL;
                size_t size = 0;

		fclose(fp);
                if (grecs_asprintf(&cmd, &size, "%s \"%s\"",
                                   grecs_preprocessor, name))
                        grecs_alloc_die();
              
                fp = popen(cmd, "r");
                if (!fp) {
		        grecs_error(loc, errno, _("cannot open `%s'"), cmd);
                        grecs_free(cmd);
                        return 1;
                }
                grecs_free(cmd);
        }

	if (_push_context(name, st.st_ino, st.st_dev, loc)) {
		return 1;
	}
	i_node = st.st_ino;
	i_dev = st.st_dev;
	yyin = fp;
	yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
	grecs_current_locus_point.file = grecs_install_text(name);
	grecs_current_locus_point.line = 1;
	grecs_current_locus_point.col = 0;
	return 0;
}

void
grecs_bind_close_sources()
{
	while (!_pop_context())
		;
	grecs_list_free(input_stack);
	input_stack = NULL;
}
