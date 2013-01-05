/* Git-style configuration file parser for Grecs.
   Copyright (C) 2011-2012 Sergey Poznyakoff

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <grecs.h>

static FILE *infile;
static int input_char;
static struct grecs_txtacc *acc;

#define TOK_EOF 0
#define TOK_EQ '='
#define TOK_SECTION 256
#define TOK_KEYWORD 257
#define TOK_VALUE   258
#define TOK_ERR -1

struct token {
	int type;
	char *buf;
	char chbuf[2];
	int putback;
	struct grecs_list *path;
	grecs_locus_t loc;
	unsigned prev_col;
} tok;

#define ISSPACE(c) (strchr(" \t\r\f\n", c) != NULL)
#define ISIDENT(c) ((isascii(c) && isalnum(c)) || (c) == '_')
#define ISINITIAL(c) ((isascii(c) && isalpha(c)) || (c) == '_')

static int
rawinput()
{
	if (!infile || feof(infile))
		return input_char = 0;
	input_char = fgetc(infile);
	if (input_char == '\n') {
		tok.prev_col = grecs_current_locus_point.col;
		grecs_locus_point_advance_line(grecs_current_locus_point);
	} else if (input_char < 0)
		input_char = 0;
	else
		grecs_current_locus_point.col++;
	return input_char;
}

static int
input()
{
	rawinput();
	if (input_char == '#' || input_char == ';') {
		while (rawinput() && input_char != '\n')
			;
	}
	return input_char;
}

static void
unput()
{
	if (!input_char)
		return;
	if (input_char == '\n') {
		grecs_current_locus_point.line--;
		grecs_current_locus_point.col = tok.prev_col;
	} else 
		grecs_current_locus_point.col--;
		
	ungetc(input_char, infile);
}

static void
error_recovery()
{
	while (input() && input_char != '\n')
		;
}

static void
collect_unquoted()
{
	do
		grecs_txtacc_grow_char(acc, input_char);
	while (input() &&
	       !(ISSPACE(input_char) || input_char == ']'));
		
}

static void
collect_subsection_name()
{
	do
		grecs_txtacc_grow_char(acc, input_char);
	while (input() &&
	       (isalnum(input_char) || input_char == '_' ||
		input_char == '-'));
}

static void
collect_substring()
{
	while (rawinput()) {
		if (input_char == '\\') {
			if (!input()) {
				grecs_error(&tok.loc, 0,
					    "unexpected EOF in string");
				break;
			}
			switch (input_char) {
			case 'n':
				input_char = '\n';
				break;
			case 't':
				input_char = '\t';
				break;
			case 'b':
				input_char = '\b';
			}
		} else if (input_char == '"')
			break;
		grecs_txtacc_grow_char(acc, input_char);
	}
}

#define endpoint(t,adj) do {					\
		(t).loc.end = grecs_current_locus_point;	\
		if (adj) {					\
		        if (input_char == '\n')			\
				(t).loc.end.col = (t).prev_col;	\
			else					\
				(t).loc.end.col -= (adj);	\
		}					        \
	} while (0)

static void
gettoken(void)
{
	int putback = tok.putback;
	tok.putback = 0;
	if (putback) {
		if (putback == '\n')
			grecs_locus_point_advance_line(grecs_current_locus_point);
		else
			grecs_current_locus_point.col++;
		return;
	}
	
	tok.buf = NULL;
        /* Skip whitespace */
	while (input() && ISSPACE(input_char))
		;

	tok.loc.beg = grecs_current_locus_point;
	
	if (input_char <= 0) {
		tok.type = TOK_EOF;
		endpoint(tok, 0);
		return;
	}
	
	if (input_char == '[') {
		int dot_delimited = -1;
		
		tok.type = TOK_SECTION;
		grecs_list_clear(tok.path);
		input();
		for (;;) {
			char *p;

			if (!dot_delimited)
				while (ISSPACE(input_char))
					input();
			else {
				if (input_char == ']')
					break;
				if (dot_delimited == 1)
					input();
			}
			
			if (input_char == TOK_EOF) {
				endpoint(tok, 0);
				grecs_error(&tok.loc, 0,
					    "unexpected EOF in section header");
				tok.type = TOK_ERR;
				return;
			}
			if (input_char == ']')
				break;
			if (input_char == '\n') {
				endpoint(tok, 1);
				grecs_error(&tok.loc, 0,
					    "unexpect newline in in section header");
				tok.type = TOK_ERR;
				return;
			}

			if (dot_delimited != 1 && input_char == '"') {
				collect_substring();
				input();
				dot_delimited = 0;
			} else if (dot_delimited == 1)
				collect_subsection_name();
			else
				collect_unquoted();
			if (dot_delimited == -1)
				dot_delimited = input_char == '.';
			else if (dot_delimited == 1) {
				if (input_char != '.' && input_char != ']') {
					endpoint(tok, 1);
					grecs_error(&tok.loc, 0,
						    "unexpected character in section header");
					tok.type = TOK_ERR;
					return;
				}
			}
			grecs_txtacc_grow_char(acc, 0);
			p = grecs_txtacc_finish(acc, 0);
			grecs_list_append(tok.path, p);
		}

		endpoint(tok, 1);
		if (grecs_list_size(tok.path) == 0) {
			grecs_error(&tok.loc, 0, "empty section header");
			tok.type = TOK_ERR;
			return;
		}

		tok.type = TOK_SECTION;
		return;
	}

	if (ISINITIAL(input_char)) {
		tok.type = TOK_KEYWORD;
		do 
			grecs_txtacc_grow_char(acc, input_char);
		while (input() && ISIDENT(input_char));
		unput();
		grecs_txtacc_grow_char(acc, 0);
		tok.buf = grecs_txtacc_finish(acc, 0);
		endpoint(tok, 0);
		return;
	}

	tok.chbuf[0] = input_char;
	tok.chbuf[1] = 0;
	tok.buf = tok.chbuf;
	tok.type = input_char;
	endpoint(tok, 0);
}

static void
collect_value()
{
	do {
		if (input_char == '"') {
			collect_substring();
			if (input_char == '"')
				continue;
			else
				break;
		}
		if (input_char == '\\') {
			if (!rawinput())
				break;
			switch (input_char) {
			case 'n':
				input_char = '\n';
				break;
			case 't':
				input_char = '\t';
				break;
			case 'b':
				input_char = '\b';
			}
		}
		grecs_txtacc_grow_char(acc, input_char);
	} while (input() && input_char != '\n');
}

static struct grecs_value *
getvalue()
{
	int len;
	struct grecs_value *val = grecs_malloc(sizeof(*val));

	while (input() && ISSPACE(input_char) && input_char != '\n')
		;

	val->locus.beg = grecs_current_locus_point;
	
	if (input_char != '\n')
		collect_value();
	val->locus.end = grecs_current_locus_point;
	val->locus.end.line--;
	val->locus.end.col = tok.prev_col;
	
	grecs_txtacc_grow_char(acc, 0);
	tok.type = TOK_VALUE;
	tok.buf = grecs_txtacc_finish(acc, 1);
	len = strlen(tok.buf);
	while (len > 0 && ISSPACE(tok.buf[len-1]))
		tok.buf[--len] = 0;
	val->type = GRECS_TYPE_STRING;
	val->v.string = tok.buf;
	return val;
}

static int
read_statement(struct grecs_node *parent)
{
	struct grecs_node *node;
	
	gettoken();
	if (tok.type == TOK_EOF || tok.type == TOK_SECTION) {
		tok.putback = 1;
		return 0;
	}
	if (tok.type != TOK_KEYWORD) {
		grecs_error(&tok.loc, 0, "syntax error");
		error_recovery();
		return 1;
	}
		
	node = grecs_node_create(grecs_node_stmt, &tok.loc);
	node->ident = grecs_strdup(tok.buf);
	node->idloc = tok.loc;
	
	gettoken();
	if (tok.type == TOK_EOF) {
		grecs_error(&tok.loc, 0, "unexpected EOF");
		grecs_node_free(node);
		return 0;
	}
	if (tok.type != TOK_EQ) {
		grecs_error(&tok.loc, 0,
			    "expected `=', but found `%s'", tok.buf);
		error_recovery();
		grecs_node_free(node);
		return 1;
	}
	node->v.value = getvalue();
	node->locus.end = node->v.value->locus.end;
	grecs_node_bind(parent, node, 1);
	return 1;
}
				
static void
read_statement_list(struct grecs_node *parent)
{
	while (read_statement(parent))
		;
}

struct grecs_node *
create_subsection_node(struct grecs_node *root)
{
	struct grecs_list_entry *ep;
	struct grecs_node *p;
	
	for (ep = tok.path->head; ep; ep = ep->next) {
		char *ident = ep->data;
		p = grecs_find_node(root, ident);
		if (!p) {
			p = grecs_node_create(grecs_node_block, &tok.loc);
			p->ident = grecs_strdup(ident);
			grecs_node_bind(root, p, 1);
		}
		root = p;
	}
	return root;
}

static int
read_section(struct grecs_node *parent)
{
	gettoken();
	if (tok.type == TOK_EOF)
		return 0;
	else if (tok.type == TOK_SECTION) {
		struct grecs_node *node = create_subsection_node(parent);
		read_statement_list(node);
	} else if (tok.type == TOK_KEYWORD) {
		read_statement(parent);
	} else {
		grecs_error(&tok.loc, 0, "syntax error");
		error_recovery();
	}
	return 1;
}

/* FIXME: traceflags not used */
struct grecs_node *
grecs_git_parser(const char *name, int traceflags)
{
	struct grecs_node *root;
	
	infile = fopen(name, "r");
	if (!infile) {
		grecs_error(NULL, errno, _("cannot open `%s'"), name);
		return NULL;
	}
	grecs_current_locus_point.file = grecs_install_text(name);
	grecs_current_locus_point.line = 1;
	grecs_current_locus_point.col = 0;

	acc = grecs_txtacc_create();
	tok.path = grecs_list_create();
	root = grecs_node_create(grecs_node_root, &tok.loc);
	
	while (read_section(root))
		;
	root->locus.end = grecs_current_locus_point;
	fclose(infile);
	grecs_txtacc_free(acc);
	grecs_list_free(tok.path);
	if (grecs_error_count) {
		grecs_tree_free(root);
		root = NULL;
	}
	return root;
}
