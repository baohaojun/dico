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
#include <string.h>
#include <errno.h>

struct dico_assoc_list {
    int flags;
    dico_list_t list;
};

struct find_closure {
    size_t count;
    const char *str;
};
    
static int
assoc_key_cmp(const void *item, void *data)
{
    const struct dico_assoc *aptr = item;
    struct find_closure *clos = data;
    if (strcmp(aptr->key, clos->str) == 0 && --clos->count == 0)
	return 0;
    return 1;
}

static int
assoc_key_cmp_ci(const void *item, void *data)
{
    const struct dico_assoc *aptr = item;
    struct find_closure *clos = data;
    if (strcasecmp(aptr->key, clos->str) == 0 && --clos->count == 0)
	return 0;
    return 1;
}

static int
assoc_free(void *item, void *data)
{
    struct dico_assoc *a = item;
    free(a->value);
    free(a);
    return 0;
}

dico_assoc_list_t
dico_assoc_create(int flags)
{
    struct dico_assoc_list *assoc = malloc(sizeof(*assoc));
    if (assoc) {
	assoc->flags = flags;
	assoc->list = dico_list_create();
	if (!assoc->list) {
	    int ec = errno;
	    free(assoc);
	    assoc = NULL;
	    errno = ec;
	} else {
	    dico_list_set_comparator(assoc->list,
				     (flags & DICO_ASSOC_CI) ?
				       assoc_key_cmp_ci : assoc_key_cmp);
	    dico_list_set_free_item(assoc->list, assoc_free, NULL);
	}
    }
    return assoc;
}

dico_assoc_list_t
dico_assoc_dup(dico_assoc_list_t src)
{
    dico_iterator_t itr;
    struct dico_assoc *p;
    dico_assoc_list_t dst;


    dst = dico_assoc_create(src->flags);
    if (dst == NULL)
	return NULL;
	
    itr = dico_assoc_iterator(src);
    if (!itr) {
	int ec = errno;
	dico_assoc_destroy(&dst);
	errno = ec;
	return NULL;
    }
    
    for (p = dico_iterator_first(itr); p; p = dico_iterator_next(itr)) {
	if (dico_assoc_append(dst, p->key, p->value)) {
	    int ec = errno;
	    dico_assoc_destroy(&dst);
	    errno = ec;
	    break;
	}
    }
    dico_iterator_destroy(&itr);
    return dst;
}

    
struct dico_assoc *
_dico_assoc_find_n(dico_assoc_list_t assoc, const char *key, size_t n)
{
    struct find_closure clos;
    if (!assoc || n == 0)
	return NULL;
    clos.count = n;
    clos.str = key;
    return dico_list_locate(assoc->list, &clos);
}

const char *
dico_assoc_find_n(dico_assoc_list_t assoc, const char *key, size_t n)
{
    struct dico_assoc *kvp = _dico_assoc_find_n(assoc, key, n);
    return kvp ? kvp->value : NULL;
}

const char *
dico_assoc_find(dico_assoc_list_t assoc, const char *key)
{
    return dico_assoc_find_n(assoc, key, 1);
}

void
dico_assoc_remove_n(dico_assoc_list_t assoc, const char *key, size_t n)
{
    struct find_closure clos;
    if (n == 0)
	return;
    clos.count = n;
    clos.str = key;
    dico_list_remove(assoc->list, &clos, NULL);
}

void
dico_assoc_remove(dico_assoc_list_t assoc, const char *key)
{
    return dico_assoc_remove_n(assoc, key, 1);
}

int
dico_assoc_add(dico_assoc_list_t assoc, const char *key, const char *value,
	       size_t count, int replace)
{
    struct dico_assoc *a;
    size_t size;

    if (value == NULL) {
	dico_assoc_remove_n(assoc, key, count);
	return 0;
    }
    
    if (!(assoc->flags & DICO_ASSOC_MULT)) {
	a = _dico_assoc_find_n(assoc, key, count);
	if (a) {
	    if (replace) {
		char *s = strdup(value);
		if (!s)
		    return 1;
		free(a->value);
		a->value = s;
		return 0;
	    }
	    errno = EEXIST;
	    return 1;
	}
    }
    
    size = sizeof(*a) + strlen(key) + 1;
    a = malloc(size);
    if (!a)
	    return 1;
    a->key = (char*)(a + 1);
    strcpy((char*) a->key, key);
    a->value = strdup(value);
    if (!a->value) {
	int ec = errno;
	free(a);
	errno = ec;
	return 1;
    }
    return dico_list_append(assoc->list, a);
}

int
dico_assoc_append(dico_assoc_list_t assoc, const char *key, const char *value)
{
    return dico_assoc_add(assoc, key, value, 1, 0);
}

int
dico_assoc_clear(dico_assoc_list_t assoc)
{
    if (!assoc) {
	errno = EINVAL;
	return 1;
    }
    return dico_list_clear(assoc->list);
}

void
dico_assoc_destroy(dico_assoc_list_t *passoc)
{
    if (passoc && *passoc) {
	dico_assoc_list_t assoc = *passoc;
	dico_list_destroy(&assoc->list);
	free(assoc);
    }
}

dico_iterator_t
dico_assoc_iterator(dico_assoc_list_t assoc)
{
    if (!assoc)
	return NULL;
    return dico_list_iterator(assoc->list);
}
