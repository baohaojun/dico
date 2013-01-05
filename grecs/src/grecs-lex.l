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
#include <grecs-gram.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
  
#include <wordsplit.h>

static char *multiline_delimiter;
static size_t multiline_delimiter_len;
static int multiline_unescape;         /* Unescape here-document contents */
static int (*char_to_strip)(char);    /* Strip matching characters of each
					  here-document line */

struct grecs_locus_point grecs_current_locus_point;   /* Input file location */
/* Line correction.  Equals to the number of #line directives inserted into
   the input by the preprocessor instance.  The external preprocessor, if
   any, counts these as input lines and therefore the line numbers in *its*
   #line directives are offset by the value of XLINES.
   
   Uff, running two preprocessors is confusing...
*/
static size_t xlines; 

static void multiline_begin(char *);
static void multiline_add(char *);
static char *multiline_strip_tabs(char *text);
static int ident(void);
static int isemptystr(int off);
static void qstring_locus_fixup(void);

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)					\
	do {								\
		if (grecs_preprocessor)					\
			result = fread(buf, 1, max_size, yyin);		\
		else							\
			result = grecs_preproc_fill_buffer(buf, max_size); \
	} while(0)

#define YY_USER_ACTION do {						\
		if (YYSTATE == 0) {					\
			yylloc.beg = grecs_current_locus_point;		\
			yylloc.beg.col++;				\
		}							\
  		grecs_current_locus_point.col += yyleng;		\
 		yylloc.end = grecs_current_locus_point;			\
   	} while (0);

%}
    
    
%x COMMENT ML STR

WS [ \t\f][ \t\f]*
ID [a-zA-Z_][a-zA-Z_0-9-]*
P [1-9][0-9]*

%%
         /* C-style comments */
"/*"         BEGIN(COMMENT);
<COMMENT>[^*\n]*        /* eat anything that's not a '*' */
<COMMENT>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<COMMENT>\n          grecs_locus_point_advance_line(grecs_current_locus_point);
<COMMENT>"*"+"/"        BEGIN(INITIAL);
         /* Line directive */
^[ \t]*#[ \t]*{P}[ \t]+\".*\".*\n { grecs_parse_line_directive_cpp(yytext,
						    &yylloc,
						    &grecs_current_locus_point,
						    &xlines); }
^[ \t]*#[ \t]*line[ \t].*\n       { grecs_parse_line_directive(yytext,
							       &yylloc,
							       &grecs_current_locus_point,
							       &xlines); }
         /* End-of-line comments */
#.*\n     { grecs_locus_point_advance_line(grecs_current_locus_point); }
#.*     /* end-of-file comment */;
"//".*\n  { grecs_locus_point_advance_line(grecs_current_locus_point); }
"//".*    /* end-of-file comment */;
        /* Identifiers */
<INITIAL>{ID}           return ident();
         /* Strings */
[a-zA-Z0-9_\.\*/:@-]([a-zA-Z0-9_\./:@-][a-zA-Z0-9_\.\*/:@-]*)? {
	                   grecs_line_begin();
	                   grecs_line_add(yytext, yyleng);
                           yylval.string = grecs_line_finish();
                           return STRING; }
         /* Quoted strings */
\"[^\\"\n]*\"         { grecs_line_begin();
                        grecs_line_add(yytext + 1, yyleng - 2);
                        yylval.string = grecs_line_finish();
                        qstring_locus_fixup();
                        return QSTRING; }
\"[^\\"\n]*\\\n        { BEGIN(STR);
                         grecs_line_begin();
		         grecs_line_acc_grow_unescape_last(yytext + 1,
                                                           yyleng - 1);
                         grecs_locus_point_advance_line(grecs_current_locus_point); }
\"[^\\"\n]*\\.         { BEGIN(STR);
                         grecs_line_begin();
		         grecs_line_acc_grow_unescape_last(yytext + 1,
                                                           yyleng - 1); }
<STR>\"[^\\"\n]*\\\n  { grecs_line_acc_grow_unescape_last(yytext, yyleng);
                        grecs_locus_point_advance_line(grecs_current_locus_point); }
<STR>[^\\"\n]*\\.     { grecs_line_acc_grow_unescape_last(yytext, yyleng); }
<STR>[^\\"\n]*\"      { BEGIN(INITIAL);
                        if (yyleng > 1) 
				grecs_line_add(yytext, yyleng - 1); 
                        yylval.string = grecs_line_finish();
			qstring_locus_fixup();    
		        return QSTRING; }
         /* Multiline strings */
"<<"(-" "?)?\\?{ID}[ \t]*#.*\n |
"<<"(-" "?)?\\?{ID}[ \t]*"//".*\n |
"<<"(-" "?)?\\?{ID}[ \t]*\n |
"<<"(-" "?)?\"{ID}\"[ \t]*#.*\n |
"<<"(-" "?)?\"{ID}\"[ \t]*"//".*\n |
"<<"(-" "?)?\"{ID}\"[ \t]*\n {
                BEGIN(ML);
		multiline_begin(yytext+2); }
         /* Ignore m4 line statements */
<ML>^"#line ".*\n {
	grecs_locus_point_advance_line(grecs_current_locus_point);
}
<ML>.*\n { char *p = multiline_strip_tabs(yytext);
	   
           if (!strncmp(p, multiline_delimiter, multiline_delimiter_len)
	       && isemptystr(p + multiline_delimiter_len - yytext)) {
		   grecs_free(multiline_delimiter);
		   multiline_delimiter = NULL;
		   BEGIN(INITIAL);
		   yylval.string = grecs_line_finish();

		   /* Update end pos */
		   yylloc.end.line--;
		   for (yylloc.end.col = 0,
				p = yylval.string + strlen(yylval.string) - 1;
			p > yylval.string && p[-1] != '\n';
			yylloc.end.col++, p--);
		   if (yylloc.end.col == 0)
			   yylloc.end.col = 1;
		   return MSTRING;
	   }
           grecs_locus_point_advance_line(grecs_current_locus_point);
	   multiline_add(p); } 
{WS}     ;
         /* Other tokens */
\n       { grecs_locus_point_advance_line(grecs_current_locus_point); } 
[,;{}()] return yytext[0];
.        { if (isascii(yytext[0]) && isprint(yytext[0]))
		grecs_error(&yylloc, 0,
			     _("stray character %c"), yytext[0]);
           else
		grecs_error(&yylloc, 0,
			     _("stray character \\%03o"),
			       (unsigned char) yytext[0]); }
%%

pid_t grecs_preproc_pid;

int
yywrap()
{
	if (yyin)
		grecs_preproc_extrn_shutdown(grecs_preproc_pid);
	else
		grecs_preproc_done();
	grecs_current_locus_point.file = NULL;
	return 1;
}

int
grecs_lex_begin(const char *name, int trace)
{
	yy_flex_debug = trace;

	grecs_line_acc_create();
	
	if (grecs_preprocessor) {
		int fd;
	
		fd = open(name, O_RDONLY);
		if (fd == -1) {
			grecs_error(NULL, errno, _("Cannot open `%s'"), name);
			return 1;
		}
		close(fd);

		yyin = grecs_preproc_extrn_start(name, &grecs_preproc_pid);
		if (!yyin) {
			grecs_error(NULL, errno,
				     _("Unable to start external preprocessor `%s'"),
				     grecs_preprocessor);
			return 1;
		}
	} else
		return grecs_preproc_init(name);

	return 0;
}

void
grecs_lex_end(int err)
{
	grecs_line_acc_free();
}

static int
isemptystr(int off)
{
	for (; yytext[off] && isspace(yytext[off]); off++)
		;
	if (yytext[off] == ';') {
		int i;
		for (i = off + 1; yytext[i]; i++) 
			if (!isspace(yytext[i]))
				return 0;
		yyless(off);
		return 1;
	}
	return yytext[off] == 0;
}

char *
multiline_strip_tabs(char *text)
{
	if (char_to_strip)
		for (; *text && char_to_strip(*text); text++)
			;
	return text;
}

static void
multiline_add(char *s)
{
	if (multiline_unescape) {
		for (; *s; s++)	{
			if (*s == '\\') {
				grecs_line_acc_grow_char_unescape(s[1]);
				++s;
			} else
				grecs_line_acc_grow_char(*s);
		}
	} else
		grecs_line_add(s, strlen(s));
}

static int
is_tab(char c)
{
	return c == '\t';
}
 
static int
is_ws(char c)
{
	return c == '\t' || c == ' ';
}

void
multiline_begin(char *p)
{
	if (*p == '-') {
		if (*++p == ' ') {
			char_to_strip = is_ws;
			p++;
		} else
			char_to_strip = is_tab;
	} else
		char_to_strip = NULL;
	if (*p == '\\') {
		p++;
		multiline_unescape = 0;
	} else if (*p == '"') {
		char *q;
 
		p++;
		multiline_unescape = 0;
		q = strchr(p, '"');
		multiline_delimiter_len = q - p;
	} else {
		multiline_delimiter_len = strcspn(p, " \t");
		multiline_unescape = 1;
	}

	/* Remove trailing newline */
	multiline_delimiter_len--;
	multiline_delimiter = grecs_malloc(multiline_delimiter_len + 1);
	memcpy(multiline_delimiter, p, multiline_delimiter_len);
	multiline_delimiter[multiline_delimiter_len] = 0;
	grecs_line_begin();

	/* Update locus */
	grecs_locus_point_advance_line(grecs_current_locus_point);
	yylloc.beg = grecs_current_locus_point;
	yylloc.beg.col++;
}

static int
ident()
{
	char *p;
	char *str;
	size_t len;
  
	for (p = yytext; *p && isspace(*p); p++)
		;

	len = strlen(p);
	str = grecs_malloc(len + 1);
	strcpy(str, p);
	yylval.string = str;
	return IDENT;
}

static void
qstring_locus_fixup()
{
	if (grecs_adjust_string_locations) {
		yylloc.beg.col++;
		yylloc.end.col--;
	}
}

grecs_value_t *
grecs_value_ptr_from_static(grecs_value_t *input)
{
	grecs_value_t *ptr = grecs_malloc(sizeof(*ptr));
	*ptr = *input;
	return ptr;
}


static int
assign_locus(struct grecs_locus_point *ploc,
	     char *name, char *line, size_t *pxlines)
{
	char *p;
	
	if (name) {
		if (pxlines && (!ploc->file || strcmp(name, ploc->file)))
			*pxlines = 0;
		ploc->file = grecs_install_text(name);
	}
	ploc->line = strtoul(line, &p, 10) - (pxlines ? *pxlines : 0);
	ploc->col = 0;
	return *p != 0;
}

void
grecs_parse_line_directive(char *text, grecs_locus_t *ploc,
			   struct grecs_locus_point *ppoint, size_t *pxlines)
{
	int rc = 1;
	struct wordsplit ws;
	
	if (wordsplit(text, &ws, WRDSF_DEFFLAGS))
		grecs_error(ploc, 0, _("cannot parse #line line"));
	else {
		if (ws.ws_wordc == 2)
			rc = assign_locus(ppoint, NULL,
					   ws.ws_wordv[1], pxlines);
		else if (ws.ws_wordc == 3) 
			rc = assign_locus(ppoint, ws.ws_wordv[2],
					   ws.ws_wordv[1], pxlines);
		else if (ws.ws_wordc == 4) {
			rc = assign_locus(ppoint, ws.ws_wordv[2],
					   ws.ws_wordv[1], 0);
			if (pxlines && rc == 0) {
				char *p;
				unsigned long x = strtoul(ws.ws_wordv[3],
							   &p, 10);
				rc = *p != 0;
				if (rc == 0)
					*pxlines = x;
			}
		} else 
			grecs_error(ploc, 0, _("invalid #line statement"));
	
		if (rc) 
			grecs_error(ploc, 0, _("malformed #line statement"));
		wordsplit_free(&ws);
	}
}

void
grecs_parse_line_directive_cpp(char *text, grecs_locus_t *ploc,
			       struct grecs_locus_point *ppoint,
			       size_t *pxlines)
{
	struct wordsplit ws;
	
	if (wordsplit(text, &ws, WRDSF_DEFFLAGS)) {
		grecs_error(ploc, 0, _("cannot parse #line line"));
		return;
	} else if (ws.ws_wordc < 3)
		grecs_error(ploc, 0, _("invalid #line statement"));
	else {
		if (assign_locus(ppoint, ws.ws_wordv[2],
				 ws.ws_wordv[1], pxlines))
			grecs_error(ploc, 0, _("malformed #line statement"));
	}
	wordsplit_free(&ws);
}

