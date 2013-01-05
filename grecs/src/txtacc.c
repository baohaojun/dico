/* wydawca - automatic release submission daemon
   Copyright (C) 2007, 2009-2012 Sergey Poznyakoff

   Wydawca is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   Wydawca is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with wydawca. If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include "grecs.h"

struct grecs_txtacc_entry
{
  char *buf;              /* Text buffer */
  size_t size;            /* Buffer size */  
  size_t len;             /* Actual number of bytes in buffer */ 
};
#define TXTACC_BUFSIZE 1024
#define grecs_txtacc_entry_freesize(e) ((e)->size - (e)->len)

struct grecs_txtacc
{
	struct grecs_list *cur;  /* Current build list */
	struct grecs_list *mem;  /* List of already allocated elements */
};

static struct grecs_txtacc_entry *
grecs_txtacc_alloc_entry(struct grecs_list *list, size_t size)
{
	struct grecs_txtacc_entry *p = grecs_malloc(sizeof (*p));
	p->buf = grecs_malloc(size);
	p->size = size;
	p->len = 0;
	grecs_list_append(list, p);
	return p;
}

static struct grecs_txtacc_entry *
grecs_txtacc_cur_entry(struct grecs_txtacc *acc)
{
	struct grecs_txtacc_entry *ent;

	if (grecs_list_size(acc->cur) == 0)
		return grecs_txtacc_alloc_entry(acc->cur,
						GRECS_TXTACC_BUFSIZE);
	ent = acc->cur->tail->data;
	if (grecs_txtacc_entry_freesize(ent) == 0)
		ent = grecs_txtacc_alloc_entry(acc->cur,
					       GRECS_TXTACC_BUFSIZE);
	return ent;
}

static void
grecs_txtacc_entry_append(struct grecs_txtacc_entry *ent,
			  const char *p, size_t size)
{
	memcpy(ent->buf + ent->len, p, size);
	ent->len += size;
} 

static void
grecs_txtacc_entry_tailor(struct grecs_txtacc_entry *ent)
{
	if (ent->size > ent->len) {
		char *p = realloc(ent->buf, ent->len);
		if (!p)
			return;
		ent->buf = p;
		ent->size = ent->len;
	}
}

static void
grecs_txtacc_entry_free(void *p)
{
	if (p) {
		struct grecs_txtacc_entry *ent = p;
		free(ent->buf);
		free(ent);
	}
}

struct grecs_txtacc *
grecs_txtacc_create()
{
	struct grecs_txtacc *acc = grecs_malloc(sizeof (*acc));
	acc->cur = grecs_list_create();
	acc->cur->free_entry = grecs_txtacc_entry_free;
	acc->mem = grecs_list_create();
	acc->mem->free_entry = grecs_txtacc_entry_free;
	return acc;
}

void
grecs_txtacc_free(struct grecs_txtacc *acc)
{
	if (acc) {
		grecs_list_free (acc->cur);
		grecs_list_free (acc->mem);
		free (acc);
	}
}

void
grecs_txtacc_grow(struct grecs_txtacc *acc, const char *buf, size_t size)
{
	while (size) {
		struct grecs_txtacc_entry *ent = grecs_txtacc_cur_entry(acc);
		size_t rest = grecs_txtacc_entry_freesize(ent);
		if (rest > size)
			rest = size;
		grecs_txtacc_entry_append(ent, buf, rest);
		buf += rest;
		size -= rest;
	}
}

char *
grecs_txtacc_finish(struct grecs_txtacc *acc, int steal)
{
	struct grecs_list_entry *ep;
	struct grecs_txtacc_entry *txtent;
	size_t size;
	char *p;
	
	switch (grecs_list_size(acc->cur)) {
	case 0:
		return NULL;

	case 1:
		txtent = acc->cur->head->data;
		acc->cur->head->data = NULL;
		grecs_txtacc_entry_tailor(txtent);
		grecs_list_append(acc->mem, txtent);
		break;

	default:
		size = 0;
		for (ep = acc->cur->head; ep; ep = ep->next) {
			txtent = ep->data;
			size += txtent->len;
		}
      
		txtent = grecs_txtacc_alloc_entry(acc->mem, size);
		for (ep = acc->cur->head; ep; ep = ep->next) {
			struct grecs_txtacc_entry *tp = ep->data;
			grecs_txtacc_entry_append(txtent, tp->buf, tp->len);
		}
	}
  
	grecs_list_clear(acc->cur);
	p = txtent->buf;
	if (steal) {
		grecs_list_remove_tail(acc->mem);
		free(txtent);
	}
	return p;
}

void
grecs_txtacc_free_string(struct grecs_txtacc *acc, char *str)
{
	struct grecs_list_entry *ep;
	for (ep = acc->mem->head; ep; ep = ep->next) {
		struct grecs_txtacc_entry *tp = ep->data;
		if (tp->buf == str) {
			grecs_list_remove_entry(acc->mem, ep);
			grecs_free(tp->buf);
			return;
		}
	}
}

void
grecs_txtacc_clear(struct grecs_txtacc *acc)
{
	grecs_list_clear(acc->cur);
}
