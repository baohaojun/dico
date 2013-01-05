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
#include <stdlib.h>
#include <errno.h>
#include "grecs.h"

static void
reset_point(struct grecs_locus_point *point, struct grecs_symtab *st)
{
	struct grecs_syment *ent, key;
	int install = 1;

	if (!point->file)
		return;
	key.name = (char*) point->file;
	ent = grecs_symtab_lookup_or_install(st, &key, &install);
	if (!ent)
		grecs_alloc_die();
	point->file = ent->name;
}

static enum grecs_tree_recurse_res
reset_locus(enum grecs_tree_recurse_op op, struct grecs_node *node, void *data)
{
	struct grecs_symtab *st = data;
	switch (op) {
	case grecs_tree_recurse_set:
	case grecs_tree_recurse_pre:
		reset_point(&node->locus.beg, st);
		reset_point(&node->locus.end, st);
		break;
	default:
		break;
	}
	return grecs_tree_recurse_ok;

}

int
grecs_tree_join(struct grecs_node *dst, struct grecs_node *src)
{
	struct grecs_node *p;
	
	if (dst->type != grecs_node_root || src->type != grecs_node_root)
		return 1;
	grecs_node_bind(dst, src->down, 1);
	for (p = src->down; p; p = p->next)
		p->up = dst;
	if (!src->v.texttab) {
		dst->v.texttab = src->v.texttab;
	} else {
		grecs_tree_recurse(src->down, reset_locus, dst->v.texttab);
		grecs_symtab_free(src->v.texttab);
	}
	src->v.texttab = NULL;
	src->down = NULL;
	return 0;
}
