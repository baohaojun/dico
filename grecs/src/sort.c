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
#include "grecs.h"

struct node_list {
	struct grecs_node *head, *tail;
};

static void
node_list_init(struct node_list *list, struct grecs_node *node)
{
	if (node) {
		list->head = node;
		while (node->next)
			node = node->next;
		list->tail = node;
	} else
		list->head = list->tail = NULL;
}

static void
node_list_add(struct node_list *list, struct grecs_node *node)
{
	node->next = NULL;
	node->prev = list->tail;
	if (list->tail)
		list->tail->next = node;
	else
		list->head = node;
	list->tail = node;
}

static void
node_list_join(struct node_list *a, struct node_list *b)
{
	if (!b->head)
		return;
	b->head->prev = a->tail;
	if (a->tail)
		a->tail->next = b->head;
	else
		a->head = b->head;
	a->tail = b->tail;
}

static void
_qsort_nodelist(struct node_list *list,
		int (*compare)(struct grecs_node const *,
			       struct grecs_node const *))
{
	struct grecs_node *cur, *middle;
	struct node_list high_list, low_list;
	int rc;
	
	if (!list->head)
		return;
	cur = list->head;
	do {
		cur = cur->next;
		if (!cur)
			return;
	} while ((rc = compare(list->head, cur)) == 0);

	/* Select the lower of the two as the middle value */
	middle = (rc > 0) ? cur : list->head;

	/* Split into two sublists */
	node_list_init(&low_list, NULL);
	node_list_init(&high_list, NULL);
	for (cur = list->head; cur; ) {
		struct grecs_node *next = cur->next;
		cur->next = NULL;
		if (compare(middle, cur) < 0)
			node_list_add(&high_list, cur);
		else
			node_list_add(&low_list, cur);
		cur = next;
	}

	/* Sort both sublists recursively */
	_qsort_nodelist(&low_list, compare);
	_qsort_nodelist(&high_list, compare);

	/* Join both lists in order */
	node_list_join(&low_list, &high_list);

	/* Return the resulting list */
	list->head = low_list.head;
	list->tail = low_list.tail;
}

struct grecs_node *
grecs_nodelist_sort(struct grecs_node *node,
		    int (*compare)(struct grecs_node const *,
				   struct grecs_node const *))
{
	struct node_list list;
	node_list_init(&list, node);
	_qsort_nodelist(&list, compare);
	return list.head;
}

void
grecs_tree_sort(struct grecs_node *node,
		int (*compare)(struct grecs_node const *,
			       struct grecs_node const *))
{
	if (node && node->down) {
		node->down = grecs_nodelist_sort(node->down, compare);
		for (node = node->down; node; node = node->next)
			grecs_tree_sort(node, compare);
	}
}

