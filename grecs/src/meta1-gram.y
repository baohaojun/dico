%{
/* MeTA1 configuration parser for Grecs.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <errno.h>
#include <string.h>
#include "grecs.h"
#include "grecs-locus.h"

int yylex(void);
int yyerror(char *s);
	
static struct grecs_node *parse_tree;
extern int yy_flex_debug;
extern void yyset_in(FILE *);
%}

%error-verbose
%locations

%union {
	char *string;
	grecs_value_t svalue, *pvalue;
	struct grecs_list *list;
	struct { struct grecs_node *head, *tail; } node_list;
	struct grecs_node *node;
}

%token <string> META1_STRING META1_IDENT
%type <node> stmt simple block maybe_stmtlist
%type <node_list> stmtlist
%type <pvalue> tag value
%type <string> string slist
%type <list> slist0
%type <list> values list 
%%

input   : maybe_stmtlist
          {
		  parse_tree = grecs_node_create(grecs_node_root, &@1);
		  parse_tree->v.texttab = grecs_text_table();
		  grecs_node_bind(parse_tree, $1, 1);
	  }
        ;

maybe_stmtlist:
          /* empty */
          {
		  $$ = NULL;
	  }
        | stmtlist
	  {
		  $$ = $1.head;
	  }
        ;

stmtlist: stmt
          {
		  $$.head = $$.tail = $1;
	  }
        | stmtlist stmt
          {
		  grecs_node_bind($1.tail, $2, 0);
	  }
        ;

stmt    : simple
        | block
        ;

simple  : META1_IDENT '=' value opt_sc
          {
		  $$ = grecs_node_create_points(grecs_node_stmt,
						@1.beg, @3.end);
		  $$->ident = $1;
		  $$->idloc = @1;
		  $$->v.value = $3;
	  }
        ;

block   : META1_IDENT tag '{' stmtlist '}' opt_sc
          {
		  $$ = grecs_node_create_points(grecs_node_block,
						@1.beg, @5.end);
		  $$->ident = $1;
		  $$->idloc = @1;
		  $$->v.value = $2;
		  grecs_node_bind($$, $4.head, 1);
	  }
        ;

tag     : /* empty */
          {
		  $$ = NULL;
	  }
        | META1_IDENT
	  {
		  $$ = grecs_malloc(sizeof($$[0]));
		  $$->type = GRECS_TYPE_STRING;
		  $$->v.string = $1;
	  }		  
	;

value   : string
          {
		  $$ = grecs_malloc(sizeof($$[0]));
		  $$->type = GRECS_TYPE_STRING;
		  $$->locus = @1;
		  $$->v.string = $1;
	  }
        | list
          {
		  $$ = grecs_malloc(sizeof($$[0]));
		  $$->type = GRECS_TYPE_LIST;
		  $$->locus = @1;
		  $$->v.list = $1;
	  }
        ;

string  : META1_IDENT
        | slist
        ;

slist   : slist0
          {
		  struct grecs_list_entry *ep;
		  
		  grecs_line_begin();
		  for (ep = $1->head; ep; ep = ep->next) {
			  grecs_line_add(ep->data, strlen(ep->data));
			  free(ep->data);
			  ep->data = NULL;
		  }
		  $$ = grecs_line_finish();
		  grecs_list_free($1);
	  }		  

slist0  : META1_STRING 
          {
		  $$ = grecs_list_create();
		  grecs_list_append($$, $1);
	  }
        | slist0 META1_STRING
          {
		  grecs_list_append($1, $2);
		  $$ = $1;
	  }
        ;

list    : '{' values '}'
          {
		  $$ = $2;
	  }
        | '{' values ',' '}'
          {
		  $$ = $2;
	  }
        ;

values  : value
          {
		  $$ = grecs_value_list_create();
		  grecs_list_append($$, $1);
	  }
        | values ',' value
          {
		  grecs_list_append($1, $3);
		  $$ = $1;
	  }
        ;

opt_sc  : /* empty */
        | ';'
        ;
	  
%%
int
yyerror(char *s)
{
	grecs_error(&yylloc, 0, "%s", s);
	return 0;
}

struct grecs_node *
grecs_meta1_parser(const char *name, int traceflags)
{
	int rc;
	FILE *fp;

	fp = fopen(name, "r");
	if (!fp) {
		grecs_error(NULL, errno, _("Cannot open `%s'"), name);
		return NULL;
	}
	yyset_in(fp);
	yy_flex_debug = traceflags & GRECS_TRACE_LEX;
	yydebug = traceflags & GRECS_TRACE_GRAM;
	parse_tree = NULL;
	grecs_line_acc_create();
	rc = yyparse();
	fclose(fp);
	if (grecs_error_count)
		rc = 1;
	grecs_line_acc_free();
	if (rc) {
		grecs_tree_free(parse_tree);
		parse_tree = NULL;
	}
	return parse_tree;
}
	
