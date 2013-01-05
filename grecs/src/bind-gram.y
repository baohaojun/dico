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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <grecs.h>
#include <grecs-locus.h>
#include <bind-gram.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

int yylex(void);
int yyerror(char *s);

static struct grecs_node *parse_tree;
extern int yy_flex_debug;
extern int grecs_bind_new_source(const char *name, grecs_locus_t *loc);
extern void grecs_bind_close_sources(void);

static struct grecs_value *stmtlist_to_value(struct grecs_node *node);
%}

%error-verbose
%locations

%union {
	char *string;
	grecs_value_t svalue, *pvalue;
	struct grecs_list *list;
	struct grecs_node *node;
	grecs_locus_t locus;
	struct { struct grecs_node *head, *tail; } node_list;
}

%token <string> BIND_STRING BIND_IDENT BIND_CONTROLS
%type <string> string
%type <svalue> value ctlsub
%type <pvalue> vallist tag
%type <list> vlist ctllist
%type <node> stmt simple block maybe_stmtlist
%type <node_list> stmtlist

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
		  if ($2) {
			  if (!$1.head)
				  $1.head = $1.tail = $2;
			  else
				  grecs_node_bind($1.tail, $2, 0);
		  }
		  $$ = $1;
	  }
        ;

stmt    : simple
        | block
        ;

simple  : BIND_IDENT vallist ';'
          {
		  if (strcmp($1, "include") == 0 &&
		      $2->type == GRECS_TYPE_STRING) {
			  grecs_bind_new_source($2->v.string, &@1);
			  $$ = NULL;
		  } else {
			  $$ = grecs_node_create_points(grecs_node_stmt,
							@1.beg, @2.end);
			  $$->ident = $1;
			  $$->idloc = @1;
			  $$->v.value = $2;
		  }
	  }
        | string ';'
	  {
		  $$ = grecs_node_create(grecs_node_stmt, &@1);
		  $$->ident = $1;
		  $$->idloc = @1;
		  $$->v.value = NULL;
	  }
        | '!' string ';'
	  {
		  $$ = grecs_node_create_points(grecs_node_stmt,
						@1.beg, @2.end);
		  $$->ident = grecs_strdup("!");
		  $$->idloc = @1;
		  $$->v.value = grecs_malloc(sizeof($$->v.value[0]));
		  $$->v.value->type = GRECS_TYPE_STRING;
		  $$->v.value->locus = @2;
		  $$->v.value->v.string = $2;
	  }
        ;

block   : BIND_IDENT tag '{' maybe_stmtlist '}' ';'
          {
		  $$ = grecs_node_create_points(grecs_node_block,
						@1.beg, @5.end);
		  $$->ident = $1;
		  $$->idloc = @1;
		  $$->v.value = $2;
		  grecs_node_bind($$, $4, 1);
	  }
        | BIND_CONTROLS '{' ctlsub '}' ';'
	  /* Special case:

	     controls {
                 inet 127.0.0.1 port 953
                     allow { 127.0.0.1; 127.0.0.2; } keys { "rndc-key"; };
             };

	     This produces:

	     .controls: (inet, 127.0.0.1, port, 953, allow, \
	      (127.0.0.1, 127.0.0.2), keys, (rndc-key))
	  */
	  {
		  $$ = grecs_node_create_points(grecs_node_stmt,
						@1.beg, @4.end);
		  $$->ident = $1;
		  $$->idloc = @1;
		  $$->v.value = grecs_value_ptr_from_static(&$3);
	  }
        ;

tag     : /* empty */
          {
		  $$ = NULL;
	  }
        | vallist		  
        ;

vallist : vlist
          {
		  size_t n;
		  
		  if ((n = grecs_list_size($1)) == 1) {
			  $$ = grecs_list_index($1, 0);
		  } else {
			  size_t i;
			  struct grecs_list_entry *ep;
		
			  $$ = grecs_malloc(sizeof($$[0]));
			  $$->type = GRECS_TYPE_ARRAY;
			  $$->locus = @1;
			  $$->v.arg.c = n;
			  $$->v.arg.v = grecs_calloc(n,
						     sizeof($$->v.arg.v[0]));
			  for (i = 0, ep = $1->head; ep; i++, ep = ep->next)
				  $$->v.arg.v[i] = ep->data;
		  }
		  $1->free_entry = NULL;
		  grecs_list_free($1);	      
	  }
	;

vlist   : value
          {
		  $$ = grecs_value_list_create();
		  grecs_list_append($$, grecs_value_ptr_from_static(&$1));
	  }
        | vlist value
          {
		  grecs_list_append($1, grecs_value_ptr_from_static(&$2));
	  }
        ;

value   : string
          {
		  $$.type = GRECS_TYPE_STRING;
		  $$.locus = @1;
		  $$.v.string = $1;
	  }
        ;        

string  : BIND_STRING
        | BIND_IDENT
        ;

ctlsub  : ctllist ';'
          {
		  $$.type = GRECS_TYPE_LIST;
		  $$.locus = @1;
		  $$.v.list = $1;
	  }
        ;

ctllist : value
          {
		  $$ = grecs_value_list_create();
		  grecs_list_append($$, grecs_value_ptr_from_static(&$1));
	  }
        | ctllist value
          {
		  grecs_list_append($1, grecs_value_ptr_from_static(&$2));
	  }
        | ctllist '{' stmtlist '}'
	  {
		  grecs_list_append($1, stmtlist_to_value($3.head));
		  /* FIXME: Free $3 */
	  }
        ;

%%

int
yyerror(char *s)
{
	grecs_error(&yylloc, 0, "%s", s);
	return 0;
}

struct grecs_node *
grecs_bind_parser(const char *name, int traceflags)
{
	int rc;

	if (grecs_bind_new_source(name, NULL))
		return NULL;
	yy_flex_debug = traceflags & GRECS_TRACE_LEX;
	yydebug = traceflags & GRECS_TRACE_GRAM;
	parse_tree = NULL;
	grecs_line_acc_create();
	rc = yyparse();
	grecs_bind_close_sources();
	if (grecs_error_count)
		rc = 1;
	grecs_line_acc_free();
	if (rc) {
		grecs_tree_free(parse_tree);
		parse_tree = NULL;
	}
	return parse_tree;
}

static struct grecs_value *
node_to_value(struct grecs_node *node, struct grecs_txtacc *acc)
{
	struct grecs_value *val = grecs_malloc(sizeof(*val));
	int delim = 0;
	
	if (node->ident) {
		grecs_txtacc_grow(acc, node->ident, strlen(node->ident));
		delim = 1;
	}
	
	if (node->v.value) {
		if (delim)
			grecs_txtacc_grow_char(acc, ' ');
		grecs_txtacc_format_value(node->v.value, 0, acc);
		delim = 1;
	}
	if (node->type == grecs_node_stmt) {
		val->type = GRECS_TYPE_STRING;
		grecs_txtacc_grow_char(acc, 0);
		val->v.string = grecs_txtacc_finish(acc, 1);
	} else if (node->down) {
		struct grecs_list *list = grecs_value_list_create();
		struct grecs_node *np;

		if (delim)
			grecs_txtacc_grow_char(acc, ' ');
		for (np = node->down; np; np = np->next)
			grecs_list_append(list, node_to_value(np, acc));
		val->type = GRECS_TYPE_LIST;
		val->v.list = list;
	}
	return val;
}

static struct grecs_value *
stmtlist_to_value(struct grecs_node *node)
{
	struct grecs_txtacc *acc = grecs_txtacc_create();
	struct grecs_value *val;
	struct grecs_node parent;

	memset(&parent, 0, sizeof(parent));
	parent.type = grecs_node_block;
	parent.down = node;
	val = node_to_value(&parent, acc);
	grecs_txtacc_free(acc);
	return val;
}
