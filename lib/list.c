/* This file is part of GNU Dico
   Copyright (C) 2003-2004, 2007-2008, 2010, 2012 Sergey Poznyakoff
  
   GNU Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

struct list_entry {
    struct list_entry *next, *prev;
    void *data;
};

struct dico_list {
    size_t count;
    struct list_entry *head, *tail;
    int flags;
    struct iterator *itr;
    dico_list_comp_t comp;
    dico_list_iterator_t free_item;
    void *free_data;
};

struct iterator {
    struct iterator *next;
    dico_list_t list;
    struct list_entry *cur;
    int advanced;
    size_t pos;
};

static int
cmp_ptr(const void *a, void *b)
{
    return a != b;
}

struct dico_list *
dico_list_create()
{
    struct dico_list *p = malloc(sizeof(*p));
    if (p) {
	p->count = 0;
	p->head = p->tail = NULL;
	p->flags = 0;
	p->itr = NULL;
	p->comp = cmp_ptr;
	p->free_item = NULL;
	p->free_data = NULL;
    }
    return p;
}

int
dico_list_clear(struct dico_list *list)
{
    struct list_entry *p;

    if (!list) {
	errno = EINVAL;
	return 1;
    }
    
    p = list->head;
    list->head = list->tail = NULL;
    list->count = 0;
    
    while (p) {
	struct list_entry *next = p->next;
	if (list->free_item)
	    list->free_item(p->data, list->free_data);
	free(p);
	p = next;
    }
    return 0;
}

void
dico_list_destroy(struct dico_list **plist)
{
    struct dico_list *list;
    
    if (!plist || !*plist)
	return;

    list = *plist;
    *plist = NULL;

    dico_list_clear(list);
    free(list);
}

void
dico_list_free_item(struct dico_list *list, void *item)
{
    if (list->free_item)
	list->free_item(item, list->free_data);
}

void *
dico_iterator_current(dico_iterator_t ip)
{
    if (!ip)
	return NULL;
    return ip->cur ? ip->cur->data : NULL;
}

size_t
dico_iterator_position(dico_iterator_t ip)
{
    if (!ip)
	return 0;
    return ip->pos;
}

static void
dico_iterator_attach(dico_iterator_t itr, dico_list_t list)
{
    itr->list = list;
    itr->cur = list->head;
    itr->next = list->itr;
    itr->advanced = 0;
    itr->pos = 0;
    list->itr = itr;	
}

static dico_iterator_t 
dico_iterator_detach(dico_iterator_t iter)
{
    dico_iterator_t cur, prev;
    
    for (cur = iter->list->itr, prev = NULL;
	 cur;
	 prev = cur, cur = cur->next)
	if (cur == iter)
	    break;
    
    if (cur) {
	if (prev)
	    prev->next = cur->next;
	else
	    cur->list->itr = cur->next;
    }
    return cur;
}

dico_iterator_t
dico_list_iterator(dico_list_t list)
{
    dico_iterator_t itr;
    
    if (!list) {
        errno = EINVAL;    
	return NULL;
    }
    itr = malloc(sizeof(*itr));
    if (itr) 
        dico_iterator_attach(itr, list);
    return itr;
}

void
dico_iterator_destroy(dico_iterator_t *ip)
{
    dico_iterator_t itr;
    
    if (!ip || !*ip)
	return;
    itr = dico_iterator_detach(*ip);
    if (itr)
	free(itr);
    *ip = NULL;
}
		
static void
_iterator_increase_pos(dico_iterator_t ip, size_t after)
{
    for (; ip; ip = ip->next) {
	if (ip->pos > after)
	    ip->pos++;
    }
}

static void
_iterator_advance(dico_iterator_t ip, struct list_entry *e)
{
    for (; ip; ip = ip->next) {
	if (ip->cur == e) {
	    ip->cur = e->next;
	    ip->advanced++;
	}
    }
}

void *
dico_iterator_first(dico_iterator_t ip)
{
    if (!ip)
	return NULL;
    ip->cur = ip->list->head;
    ip->advanced = 0;
    ip->pos = 0;
    return dico_iterator_current(ip);
}

void *
dico_iterator_next(dico_iterator_t ip)
{
    if (!ip || !ip->cur)
	return NULL;
    if (!ip->advanced) {
	ip->cur = ip->cur->next;
	ip->pos++;
    }
    ip->advanced = 0;
    return dico_iterator_current(ip);
}	

void *
dico_iterator_prev(dico_iterator_t ip)
{
    if (!ip || !ip->cur)
	return NULL;
    ip->cur = ip->cur->prev;
    if (!ip->advanced)
	ip->pos--;
    ip->advanced = 0;
    return dico_iterator_current(ip);
}	

void *
dico_iterator_item(dico_iterator_t ip, size_t n)
{
    if (n > ip->pos) {
	if (!ip->advanced) {
	    ip->cur = ip->cur->next;
	    ip->pos++;
	}
	ip->advanced = 0;
	
	while (ip->cur && ip->pos < n) {
	    ip->cur = ip->cur->next;
	    ip->pos++;
	}
    } else if (n < ip->pos) {
	if (!ip->advanced)
	    ip->pos--;
	ip->advanced = 0;

	while (ip->cur && ip->pos > n) {
	    ip->cur = ip->cur->prev;
	    ip->pos--;
	}
    }
    return dico_iterator_current(ip);
}

static void _dico_list_remove_item(struct dico_list *list,
				   struct list_entry *p,
				   void **pptr);

void
dico_iterator_remove_current(dico_iterator_t ip, void **pptr)
{
    _dico_list_remove_item(ip->list, ip->cur, pptr);
}

void
dico_iterator_set_data(dico_iterator_t ip, void *data)
{
    ip->cur->data = data;
}

void *
dico_list_item(struct dico_list *list, size_t n)
{
    struct list_entry *p;
    if (!list || n >= list->count)
	return NULL;
    for (p = list->head; n > 0 && p; p = p->next, n--)
	;
    return p->data;
}

size_t
dico_list_count(struct dico_list *list)
{
    if (!list)
	return 0;
    return list->count;
}

int
dico_list_set_free_item(struct dico_list *list,
			dico_list_iterator_t free_item, void *data)
{
    if (!list) {
	errno = EINVAL;
	return 1;
    }
    list->free_item = free_item;
    list->free_data = data;
    return 0;
}

dico_list_comp_t
dico_list_set_comparator(struct dico_list *list, dico_list_comp_t comp)
{
    dico_list_comp_t prev;

    if (!list) {
	errno = EINVAL;
	return NULL;
    }
    prev = list->comp;
    list->comp = comp;
    return prev;
}

int
dico_list_set_flags(struct dico_list *list, int flags)
{
   if (!list) {
       errno = EINVAL;
       return 1;
   }
   list->flags = flags;
   return 0;
}

int
dico_list_get_flags(struct dico_list *list)
{
   if (list)
       return list->flags;
   return 0;
}


dico_list_comp_t
dico_list_get_comparator(struct dico_list *list)
{
    if (!list) {
	errno = EINVAL;
	return NULL;
    }
    return list->comp;
}

int
_dico_list_append(struct dico_list *list, void *data)
{
    struct list_entry *ep = malloc(sizeof(*ep));
    if (!ep)
	return 1;
    ep->next = NULL;
    ep->prev = list->tail;
    ep->data = data;
    if (list->tail)
	list->tail->next = ep;
    else
	list->head = ep;
    list->tail = ep;
    list->count++;
    return 0;
}

int
_dico_list_prepend(struct dico_list *list, void *data)
{
    struct list_entry *ep = malloc(sizeof(*ep));
    if (!ep)
	return 1;
    ep->data = data;
    ep->next = list->head;
    ep->prev = NULL;
    list->head = ep;
    if (!list->tail)
	list->tail = list->head;
    list->count++;
    _iterator_increase_pos(list->itr, 0);
    return 0;
}

int
dico_list_append(struct dico_list *list, void *data)
{
    if (!list) {
	errno = EINVAL;
	return 1;
    }
    if ((list->flags & DICO_LIST_COMPARE_TAIL) && list->comp
	&& list->tail && list->comp(list->tail->data, data) == 0) {
	errno = EEXIST;
	return 1;
    }
    return _dico_list_append(list, data);
}

int
dico_list_prepend(struct dico_list *list, void *data)
{
    if (!list) {
	errno = EINVAL;
	return 1;
    }
    if ((list->flags & DICO_LIST_COMPARE_HEAD) && list->comp
	&& list->head && list->comp(list->head->data, data) == 0) {
	errno = EEXIST;
	return 1;
    }
    return _dico_list_prepend(list, data);
}

static void
_dico_list_remove_item(struct dico_list *list, struct list_entry *p,
		       void **pptr)
{
    struct list_entry *prev;
    
    _iterator_advance(list->itr, p);
    
    prev = p->prev;
    if (prev)
	prev->next = p->next;
    else
	list->head = list->head->next;

    if (p->next)
	p->next->prev = prev;
    else
	list->tail = prev;
    
    free(p);
    list->count--;

    if (pptr)
	*pptr = p->data;
    else if (list->free_item)
	list->free_item (p->data, list->free_data);
}

int
_dico_list_remove(struct dico_list *list, void *data, dico_list_comp_t cmp,
		  void **pptr)
{
    struct list_entry *p;

    if (!list || !list->head) {
	errno = ENOENT;
	return 1;
    }

    if (!cmp)
	cmp = cmp_ptr;
    for (p = list->head; p; p = p->next)
	if (cmp(p->data, data) == 0)
	    break;
    
    if (!p) {
	errno = ENOENT;
	return 1;
    }

    _dico_list_remove_item(list, p, pptr);

    return 0;
}

int
dico_list_remove(struct dico_list *list, void *data, void **pret)
{
    if (!list) {
	errno = EINVAL;
	return 1;
    }
    return _dico_list_remove(list, data, list->comp, pret);
}

void *
dico_list_pop(struct dico_list *list)
{
    void *p;
    if (!list->tail)
	return NULL;
    _dico_list_remove_item(list, list->tail, &p);
    return p;
}

/* Note: if modifying this function, make sure it does not allocate any
   memory! */
void
dico_list_iterate(struct dico_list *list, dico_list_iterator_t func,
		  void *data)
{
    struct iterator itr;
    void *p;
	
    if (!list)
	return;
    dico_iterator_attach(&itr, list);
    for (p = dico_iterator_first(&itr); p; p = dico_iterator_next(&itr)) {
	if (func(p, data))
	    break;
    }
    dico_iterator_detach(&itr);
}

void *
_dico_list_locate(struct dico_list *list, void *data, dico_list_comp_t cmp)
{
    struct list_entry *cur;
    if (!list)
	return NULL;
    if (!cmp)
	cmp = cmp_ptr;
    for (cur = list->head; cur; cur = cur->next)
	if (cmp(cur->data, data) == 0)
	    break;
    return cur ? cur->data : NULL;
}

void *
dico_list_locate(struct dico_list *list, void *data)
{
    if (!list)
	return NULL;
    return _dico_list_locate(list, data, list->comp);
}

int
_dico_list_insert_sorted(struct dico_list *list, void *data,
			 dico_list_comp_t cmp)
{
    int rc;
    struct list_entry *cur;
    size_t i;
    
    if (!list) {
	errno = EINVAL;
	return 1;
    }

    if (!cmp)
	cmp = cmp_ptr;

    if (!list->head)
	return _dico_list_append(list, data);
    
    for (cur = list->head, i = 0; cur; cur = cur->next, i++) {
	int res = cmp(cur->data, data);
	if (res > 0)
	    break;
	else if (res == 0 && list->flags)
	    return EEXIST;
    }
    
    if (cur && !cur->prev) {
	rc = _dico_list_prepend(list, data);
    } else if (!cur) {
	rc = _dico_list_append(list, data);
    } else {
	struct list_entry *ep = malloc(sizeof(*ep));
	if (ep) {
	    struct list_entry *prev = cur->prev;
	    
	    rc = 0;
	    ep->data = data;

	    ep->next = cur;
	    cur->prev = ep;
	    
	    ep->prev = prev;
	    prev->next = ep;

	    _iterator_increase_pos(list->itr, i - 1);
	    
	    list->count++;
	} else
	    rc = 1;
    }
    return rc;
}

int
dico_list_insert_sorted(struct dico_list *list, void *data)
{
    if (!list) {
	errno = EINVAL;
	return 1;
    }
    return _dico_list_insert_sorted(list, data, list->comp);
}

/* Computes an intersection of the two lists. The resulting list
   contains elements from the list A that are also encountered
   in the list B. Elements are compared using function CMP.
   The resulting list preserves the ordering of A. */
dico_list_t 
dico_list_intersect(dico_list_t a, dico_list_t b, dico_list_comp_t cmp)
{
    dico_list_t res;
    dico_iterator_t itr = dico_list_iterator(a);
    void *p;
    
    if (!itr)
	return NULL;
    res = dico_list_create();
    if (!res)
	return NULL;
    for (p = dico_iterator_first(itr); p; p = dico_iterator_next(itr)) {
	if (_dico_list_locate(b, p, cmp))
	    _dico_list_append(res, p); /* FIXME: check return, and? */
    }
    dico_iterator_destroy (&itr);
    return res;
}

/* Return true if there exists a non-empty intersection of lists A and B. */
int
dico_list_intersect_p(dico_list_t a, dico_list_t b, dico_list_comp_t cmp)
{
    dico_iterator_t itr = dico_list_iterator(a);
    void *p;
    int rc = 0;
    
    for (p = dico_iterator_first(itr); p; p = dico_iterator_next(itr)) {
	if (_dico_list_locate(b, p, cmp)) {
	    rc = 1;
	    break;
	}
    }
    dico_iterator_destroy (&itr);
    return rc;
}
