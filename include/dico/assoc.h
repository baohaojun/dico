/* This file is part of Dico.
   Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff

   Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Dico.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef __dico_assoc_h
#define __dico_assoc_h

#include <dico/types.h>
#include <stdlib.h>

/* Association lists */
struct dico_assoc {
    const char *key;
    char *value;
};

#define DICO_ASSOC_CI   0x01
#define DICO_ASSOC_MULT 0x02

dico_assoc_list_t dico_assoc_create(int flags);
dico_assoc_list_t dico_assoc_dup(dico_assoc_list_t src);
void dico_assoc_destroy(dico_assoc_list_t *passoc);
int dico_assoc_clear(dico_assoc_list_t assoc);
int dico_assoc_add(dico_assoc_list_t assoc,
		   const char *key, const char *value,
		   size_t count, int replace);
int dico_assoc_append(dico_assoc_list_t assoc, const char *key,
		      const char *value);
const char *dico_assoc_find_n(dico_assoc_list_t assoc, const char *key,
			      size_t n);
const char *dico_assoc_find(dico_assoc_list_t assoc, const char *key);
void dico_assoc_remove_n(dico_assoc_list_t assoc, const char *key, size_t n);
void dico_assoc_remove(dico_assoc_list_t assoc, const char *key);

dico_iterator_t dico_assoc_iterator(dico_assoc_list_t assoc);

int dico_header_parse(dico_assoc_list_t *pasc, const char *text);

#endif
