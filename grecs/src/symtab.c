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
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/* |hash_size| defines a sequence of symbol table sizes. These are prime
   numbers, each of which is approximately twice its predecessor. */

static unsigned int hash_size[] = {
	7, 17, 37, 101, 229, 487, 1009, 2039, 4091, 8191, 16411
};

/* |max_rehash| keeps the number of entries in |hash_size| table. */
static unsigned int max_rehash = sizeof(hash_size) / sizeof(hash_size[0]);

struct grecs_symtab {
	int flags;
	unsigned int hash_num;  /* Index to hash_size table */
	size_t elsize;          /* Size of an element */
	struct grecs_syment **tab;
	unsigned (*hash_fun)(void *, unsigned long hash_num);
	int (*cmp_fun)(const void *, const void *);
	int (*copy_fun)(void *, void *);
	void *(*syment_alloc_fun)(size_t size);
	void (*syment_free_fun) (void *);
};

static void
syment_free(struct grecs_symtab *st, void *ptr)
{
	if (st->syment_free_fun)
		st->syment_free_fun(ptr);
	else
		free(ptr);
}

static struct grecs_syment *
syment_alloc(struct grecs_symtab *st, void *key)
{
	struct grecs_syment *ent;
	
	ent = st->syment_alloc_fun ?
		st->syment_alloc_fun(st->elsize) : malloc(st->elsize);
	if (ent) {
		memset(ent, 0, st->elsize);
		if (st->copy_fun(ent, key)) {
			int ec = errno;
			syment_free(st, ent);
			errno = ec;
			return NULL;
		}
	}
	return ent;
}


unsigned
grecs_hash_string(const char *name, unsigned long hashsize)
{
	unsigned i;
	
	for (i = 0; *name; name++) {
		i <<= 1;
		i ^= *(unsigned char*) name;
	}
	return i % hashsize;
}

static unsigned
def_hash(void *data, unsigned long hashsize)
{
  struct grecs_syment *sym = data;
  return grecs_hash_string(sym->name, hashsize);
}

static int
def_cmp(const void *a, const void *b)
{
	struct grecs_syment const *syma = a;
	struct grecs_syment const *symb = b;

	return strcmp(syma->name, symb->name);
}

static int
def_copy(void *a, void *b)
{
	struct grecs_syment *syma = a;
	struct grecs_syment *symb = b;

	syma->name = strdup(symb->name);
	return syma->name == NULL;
}

static void
def_free_fun(void *p)
{
	struct grecs_syment *sym = p;
	free(sym->name);
	free(sym);
}
		


static unsigned
symtab_insert_pos(struct grecs_symtab *st, void *elt)
{
	unsigned i;
	unsigned pos = st->hash_fun(elt, hash_size[st->hash_num]);
	
	for (i = pos; st->tab[i];) {
		if (++i >= hash_size[st->hash_num])
			i = 0;
		if (i == pos)
			/* FIXME: Error message? */
			abort();
	}
	return i;
}

int
grecs_symtab_replace(struct grecs_symtab *st, void *ent, void **old_ent)
{
	struct grecs_syment *entry;
	unsigned i, pos = st->hash_fun(ent, hash_size[st->hash_num]);
	for (i = pos; entry = st->tab[i];) {
		if (st->cmp_fun(entry, ent) == 0)
			break;
		if (++i >= hash_size[st->hash_num])
			i = 0;
		if (i == pos)
			return ENOENT;
	}
	if (old_ent)
		*old_ent = entry;
	st->tab[i] = ent;
	return 0;
}

static int
symtab_rehash(struct grecs_symtab *st)
{
	struct grecs_syment **old_tab = st->tab;
	struct grecs_syment **new_tab;
	unsigned int i;
	unsigned int hash_num = st->hash_num + 1;
	
	if (hash_num >= max_rehash)
		return E2BIG;

	new_tab = calloc(hash_size[hash_num], sizeof(*new_tab));
	if (!new_tab)
		return ENOMEM;
	st->tab = new_tab;
	if (old_tab) {
		st->hash_num = hash_num;
		for (i = 0; i < hash_size[hash_num-1]; i++) {
			struct grecs_syment *elt = old_tab[i];
			if (elt->name) {
				unsigned n = symtab_insert_pos(st, elt);
				new_tab[n] = elt;
			}
		}
		free(old_tab);
	}
	return 0;
}

const char *
grecs_symtab_strerror(int rc)
{
	switch (rc) {
	case ENOENT:
		return _("element not found in table");
	case E2BIG:
		return _("symbol table is full");
	case ENOMEM:
		return strerror(ENOMEM);
	}
	return strerror(rc);
}

int
grecs_symtab_remove(struct grecs_symtab *st, void *elt)
{
	unsigned int pos, i, j, r;
	struct grecs_syment *entry;
	
	pos = st->hash_fun(elt, hash_size[st->hash_num]);
	for (i = pos; entry = st->tab[i];) {
		if (st->cmp_fun(entry, elt) == 0)
			break;
		if (++i >= hash_size[st->hash_num])
			i = 0;
		if (i == pos)
			return ENOENT;
	}
	
	syment_free(st, entry);

	for (;;) {
		st->tab[i] = NULL;
		j = i;

		do {
			if (++i >= hash_size[st->hash_num])
				i = 0;
			if (!st->tab[i])
				return 0;
			r = st->hash_fun(st->tab[i], hash_size[st->hash_num]);
		}
		while ((j < r && r <= i)
		       || (i < j && j < r) || (r <= i && i < j));
		st->tab[j] = st->tab[i];
	}
	return 0;
}

int
grecs_symtab_get_index(unsigned *idx, struct grecs_symtab *st,
		       void *key, int *install)
{
	int rc;
	unsigned i, pos;
	struct grecs_syment *elem;
  
	if (!st->tab) {
		if (install) {
			rc = symtab_rehash(st);
			if (rc)
				return rc;
		} else
			return ENOENT;
	}

	pos = st->hash_fun(key, hash_size[st->hash_num]);

	for (i = pos; elem = st->tab[i];) {
		if (st->cmp_fun(elem, key) == 0) {
			if (install)
				*install = 0;
			*idx = i; 
			return 0;
		}
      
		if (++i >= hash_size[st->hash_num])
			i = 0;
		if (i == pos)
			break;
	}

	if (!install)
		return ENOENT;
  
	if (!elem) {
		*install = 1;
		*idx = i;
		return 0;
	}

	if ((rc = symtab_rehash(st)) != 0)
		return rc;

	return grecs_symtab_get_index(idx, st, key, install);
}

void *
grecs_symtab_lookup_or_install(struct grecs_symtab *st, void *key,
			       int *install)
{
	unsigned i;
	int rc = grecs_symtab_get_index(&i, st, key, install);
	if (rc == 0) {
		if (install && *install == 1) {
			struct grecs_syment *ent = syment_alloc(st, key);
			if (!ent) {
				errno = ENOMEM;
				return NULL;
			}
			st->tab[i] = ent;
			return ent;
		} else
			return st->tab[i];
	}
	errno = rc;
	return NULL;
}

void
grecs_symtab_clear(struct grecs_symtab *st)
{
	unsigned i, hs;
  
	if (!st || !st->tab)
		return;

	hs = hash_size[st->hash_num];
	for (i = 0; i < hs; i++) {
		struct grecs_syment *elem = st->tab[i];
		if (elem) {
			syment_free(st, elem);
			st->tab[i] = NULL;
		}
	}
}

struct grecs_symtab *
grecs_symtab_create(size_t elsize, 
		    unsigned (*hash_fun)(void *, unsigned long),
		    int (*cmp_fun)(const void *, const void *),
		    int (*copy_fun)(void *, void *),
		    void *(*alloc_fun)(size_t), void (*free_fun)(void *))
{
	struct grecs_symtab *st = malloc(sizeof(*st));
	if (st) {
		memset(st, 0, sizeof(*st));
		st->elsize = elsize;
		st->hash_fun = hash_fun ? hash_fun : def_hash;
		st->cmp_fun = cmp_fun ? cmp_fun : def_cmp;
		st->copy_fun = copy_fun ? copy_fun : def_copy;
		st->syment_alloc_fun = alloc_fun;
		if (free_fun)
			st->syment_free_fun = free_fun;
		else if (!copy_fun)
			st->syment_free_fun = def_free_fun;
		else
			st->syment_free_fun = NULL;
		st->tab = calloc(hash_size[st->hash_num], sizeof(*st->tab));
		if (!st->tab) {
			free(st);
			st = NULL;
		}
	}
	return st;
}

struct grecs_symtab *
grecs_symtab_create_default(size_t elsize)
{
	return grecs_symtab_create(elsize,
				   NULL, NULL, NULL, NULL, NULL);
}

void
grecs_symtab_free(struct grecs_symtab *st)
{
	if (st) {
		grecs_symtab_clear(st);
		free(st->tab);
		free(st);
	}
}

size_t
grecs_symtab_count_entries(struct grecs_symtab *st)
{
	unsigned i;
	size_t count = 0;
	
	for (i = 0; i < hash_size[st->hash_num]; i++)
		if (st->tab[i])
			count++;
	return count;
}

int
grecs_symtab_enumerate(struct grecs_symtab *st, grecs_symtab_enumerator_t fun,
		       void *data)
{
	unsigned i;

	if (!st)
		return 0;
	for (i = 0; i < hash_size[st->hash_num]; i++) {
		struct grecs_syment *ep = st->tab[i];
		if (ep) {
			int rc = fun(ep, data);
			if (rc)
				return rc;
		}
	}
	return 0;
}








   
