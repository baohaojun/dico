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
#include <grecs-gram.h>
#include <stdlib.h>
#include <string.h>

struct grecs_list *
grecs_list_create()
{
	struct grecs_list *lp = grecs_malloc(sizeof(*lp));
	memset(lp, 0, sizeof(*lp));
	return lp;
}

size_t
grecs_list_size(struct grecs_list *lp)
{
	return lp ? lp->count : 0;
}

void
grecs_list_insert_entry(struct grecs_list *lp,
			struct grecs_list_entry *anchor,
			struct grecs_list_entry *ent, int before)
{
	struct grecs_list_entry *p;
	
	if (!anchor) {
		ent->prev = NULL;
		ent->next = lp->head;
		if (lp->head)
			lp->head->prev = ent;
		else
			lp->tail = ent;
		lp->head = ent;
		lp->count++;
		return;
	}
		
	if (before) {
		grecs_list_insert_entry(lp, anchor->prev, ent, 0);
		return;
	}

	ent->prev = anchor;
	if (p = anchor->next)
		p->prev = ent;
	else
		lp->tail = ent;
	ent->next = p;
	anchor->next = ent;
	lp->count++;
}

void
grecs_list_remove_entry(struct grecs_list *lp, struct grecs_list_entry *ent)
{
	struct grecs_list_entry *p;
	if (p = ent->prev)
		p->next = ent->next;
	else
		lp->head = ent->next;
	if (p = ent->next)
		p->prev = ent->prev;
	else
		lp->tail = ent->prev;
	ent->next = ent->prev = NULL;
	lp->count--;
}

void
grecs_list_append(struct grecs_list *lp, void *val)
{
	struct grecs_list_entry *ep = grecs_malloc(sizeof(*ep));
	ep->data = val;
	grecs_list_insert_entry(lp, lp->tail, ep, 0);
}

void
grecs_list_add(struct grecs_list *dst, struct grecs_list *src)
{
	if (!src->head)
		return;

	src->head->prev = dst->tail;
	if (dst->tail)
		dst->tail->next = src->head;
	else
		dst->head = src->head;
	dst->tail = src->tail;
	dst->count += src->count;
	src->head = src->tail = NULL;
	src->count = 0;
}

void
grecs_list_push(struct grecs_list *lp, void *val)
{
	struct grecs_list_entry *ep = grecs_malloc(sizeof(*ep));
	ep->data = val;
	grecs_list_insert_entry(lp, NULL, ep, 0);
}

void *
grecs_list_pop(struct grecs_list *lp)
{
	void *data;
	struct grecs_list_entry *ep = lp->head;
	if (ep)	{
		data = ep->data;
		grecs_list_remove_entry(lp, ep);
		grecs_free(ep);
	} else
		data = NULL;
	return data;
}

void *
grecs_list_remove_tail(struct grecs_list *lp)
{
	void *data;
	struct grecs_list_entry *ep;
  
	if (!lp->tail)
		return NULL;
	ep = lp->tail;
	data = lp->tail->data;
	grecs_list_remove_entry(lp, ep);
	grecs_free(ep);
	return data;
}

void
grecs_list_clear(struct grecs_list *lp)
{
	struct grecs_list_entry *ep = lp->head;

	while (ep) {
		struct grecs_list_entry *next = ep->next;
		if (lp->free_entry)
			lp->free_entry(ep->data);
		grecs_free(ep);
		ep = next;
	}
	lp->head = lp->tail = NULL;
	lp->count = 0;
}

void
grecs_list_free(struct grecs_list *lp)
{
	if (lp) {
		grecs_list_clear(lp);
		grecs_free(lp);
	}
}

static int
_ptrcmp(const void *a, const void *b)
{
	return a != b;
}

void *
grecs_list_locate(struct grecs_list *lp, void *data)
{
	struct grecs_list_entry *ep;
	int (*cmp)(const void *, const void *) = lp->cmp ? lp->cmp : _ptrcmp;

	for (ep = lp->head; ep; ep = ep->next) {
		if (cmp(ep->data, data) == 0)
			return ep->data;
	}
	return NULL;
}

void *
grecs_list_index(struct grecs_list *lp, size_t idx)
{
	struct grecs_list_entry *ep;
	
	for (ep = lp->head; ep && idx; ep = ep->next, idx--)
		;
	return ep ? ep->data : NULL;
}

