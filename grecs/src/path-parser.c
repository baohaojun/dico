/* Path-style configuration file parser for Grecs.
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

static int
next_char(FILE *infile)
{
	int c = fgetc(infile);
	if (c == '\n')
		grecs_locus_point_advance_line(grecs_current_locus_point);
	else {
		grecs_current_locus_point.col++;
		if (c == '\\') {
			int nc = fgetc(infile);
			if (nc == '\n') {
				grecs_locus_point_advance_line(grecs_current_locus_point);
				c = fgetc(infile);
				grecs_current_locus_point.col++;
			} else
				ungetc(nc, infile);
		}
	}
	return c;
}

struct grecs_node *
grecs_path_parser(const char *name, int traceflags)
{
	struct grecs_node *root, *subtree = NULL, *node;
	FILE *infile;
	struct grecs_txtacc *acc = NULL;
	char *kw, *val;
	grecs_locus_t kwloc, valloc, rootloc;
	int inquote;
	int lookahead;
	int err = 0;
	unsigned prev_col;
	
	infile = fopen(name, "r");
	if (!infile) {
		grecs_error(NULL, errno, _("cannot open `%s'"), name);
		return NULL;
	}
	grecs_current_locus_point.file = grecs_install_text(name);
	grecs_current_locus_point.line = 1;
	grecs_current_locus_point.col = 0;
	rootloc.beg = grecs_current_locus_point;
	rootloc.beg.col++;
	
	acc = grecs_txtacc_create();
	
	while ((lookahead = next_char(infile)) > 0) {
		while (1) {
			while (lookahead == ' ' || lookahead == '\t')
				lookahead = next_char(infile);

			if (lookahead == '#') {
				while ((lookahead = next_char(infile)) &&
				       lookahead != '\n')
					;
				continue;
			}
			break;
		}
		
		if (lookahead <= 0)
			break;

		kwloc.beg = grecs_current_locus_point;

		inquote = 0;
		for (; lookahead > 0 && lookahead != ':';
		     lookahead = next_char(infile)) {
			if (inquote) {
				if (inquote == '"' && lookahead == '\\') {
					lookahead = next_char(infile);
					if (lookahead <= 0)
						break;
				} else if (lookahead == inquote)
					inquote = 0;
			} else if (lookahead == '\'' || lookahead == '"')
				inquote = lookahead;
			grecs_txtacc_grow_char(acc, lookahead);
		}
		
		if (lookahead <= 0) {
			grecs_error(&kwloc, 0, _("unexpected end of file"));
			err = 1;
			break;
		}

		grecs_txtacc_grow_char(acc, 0);
		kw = grecs_txtacc_finish(acc, 0);

		kwloc.end = grecs_current_locus_point;
		kwloc.end.col--;
		
		while ((lookahead = next_char(infile)) > 0 &&
		       (lookahead == ' ' || lookahead == '\t'));

		if (lookahead <= 0) {
			grecs_error(&kwloc, 0, _("unexpected end of file"));
			err = 1;
			break;
		}

		valloc.beg = grecs_current_locus_point;
		do {
			grecs_txtacc_grow_char(acc, lookahead);
			prev_col = grecs_current_locus_point.col;
		} while ((lookahead = next_char(infile)) > 0 &&
			 lookahead != '\n');
		valloc.end = grecs_current_locus_point;
		valloc.end.line--;
		valloc.end.col = prev_col;
		
		grecs_txtacc_grow_char(acc, 0);
		val = grecs_txtacc_finish(acc, 0);

		node = grecs_node_from_path_locus(kw, val, &kwloc, &valloc);
		if (!node) {
			grecs_error(&kwloc, 0, _("parse error"));
			err = 1;
			break;
		}
		node->locus.end = valloc.end;
		node->idloc = kwloc;
		
		if (!subtree)
			subtree = node;
		else
			grecs_node_bind(subtree, node, 0);
		grecs_txtacc_free_string(acc, kw);
		grecs_txtacc_free_string(acc, val);
	}

	fclose(infile);
	grecs_txtacc_free(acc);

	if (err) {
		grecs_tree_free(subtree);
		root = NULL;
	} else {
		rootloc.end = grecs_current_locus_point;
		root = grecs_node_create(grecs_node_root, &rootloc);
		root->v.texttab = grecs_text_table();
		grecs_node_bind(root, subtree, 1);
		grecs_tree_reduce(root, NULL, 0);
	}
	
	return root;
}

