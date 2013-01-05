/* MeTA1 configuration lexer for Grecs. -*- c -*- */
%top {
/* MeTA1 configuration lexer for Grecs.
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

/* This file implements a lexical analyzer for MeTA1 main configuration file.
 */

#ifndef HAVE_CONFIG_H
# include <config.h>
#endif
#include "grecs.h"
#include "grecs-locus.h"
#include "meta1-gram.h"
#include <ctype.h>
}

%{
static int yywrap(void);
static void meta1_line_add_unescape_hex(const char *text, size_t len);	

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
X [0-9a-fA-F]
%%
         /* C-style comments */
"/*"                    BEGIN (COMMENT);
<COMMENT>[^*\n]*        /* eat anything that's not a '*' */
<COMMENT>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<COMMENT>\n             grecs_locus_point_advance_line(grecs_current_locus_point);
<COMMENT>"*"+"/"        BEGIN (INITIAL);
         /* End-of-line comments */
#.*\n     { grecs_locus_point_advance_line(grecs_current_locus_point); }
#.*       /* end-of-file comment */;
         /* Number */
0[xX]{X}+ |
0[0-7]+ |
[1-9][0-9]+             { grecs_line_begin();
	                  grecs_line_add(yytext, yyleng);
	                  yylval.string = grecs_line_finish();
                          return META1_STRING; }
        /* Identifiers (unquoted strings) */
[a-zA-Z0-9_\./:\*-]+    { grecs_line_begin();
	                  grecs_line_add(yytext, yyleng);
	                  yylval.string = grecs_line_finish();
                          return META1_IDENT; }
         /* Quoted strings */
\"[^\\"\n]*\"           { grecs_line_begin();
	                  grecs_line_add(yytext + 1, yyleng - 2);
	                  yylval.string = grecs_line_finish();
                          return META1_STRING; }
\"[^\\"\n]*\\x{X}{1,2}  { BEGIN(STR);
                          grecs_line_begin();
			  meta1_line_add_unescape_hex(yytext + 1, yyleng - 1);
		        }
\"[^\\"\n]*\\.          { BEGIN(STR);
	                  grecs_line_begin();
			  grecs_line_acc_grow_unescape_last(yytext + 1,
                                                            yyleng - 1); }
<STR>[^\\"\n]*\\x{X}{1,2} { meta1_line_add_unescape_hex(yytext, yyleng); }
<STR>[^\\"\n]*\\.       { grecs_line_acc_grow_unescape_last(yytext, yyleng); }
<STR>[^\\"\n]*\"        { BEGIN(INITIAL);
                          if (yyleng > 1) 
                            grecs_line_add(yytext, yyleng - 1); 
                          yylval.string = grecs_line_finish();
		          return META1_STRING; }
<STR>[^\\"\n]*\n        { BEGIN(INITIAL);
                          grecs_error(&yylloc, 0,
                                      _("newline in a string"));
                          grecs_line_add(yytext, yyleng - 1);
                          yylval.string = grecs_line_finish();
                          return META1_STRING; }
         /* Other tokens */
[ \t\f][ \t\f]*         ;
\n                      { grecs_locus_point_advance_line(grecs_current_locus_point); }
[,;{}=]                 return yytext[0];
.        { grecs_error(&yylloc, 0,
                       (isascii(yytext[0]) && isprint(yytext[0])) ?
                          _("stray character %c") :
                          _("stray character \\%03o"),
			         (unsigned char) yytext[0]); }
%%

int
yywrap()
{
        return 1;
}

static void
meta1_line_add_unescape_hex(const char *text, size_t len)
{
        for (; text[len-1] != 'x' && len > 0; len--)
		;
	grecs_line_acc_grow(text, len - 2);
	grecs_line_acc_grow_char((char) strtoul (text + len, NULL, 16));
}




