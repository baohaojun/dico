/* This file is part of GNU Dico.
   Copyright (C) 1998-2000, 2008, 2010, 2012 Sergey Poznyakoff

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

#include <dicod.h>

struct word_length {
    enum cmp_op op;
    size_t len;
};

enum stratcl_type {
    stratcl_all,
    stratcl_word,
    stratcl_length
};
    
struct stratcl {
    enum stratcl_type type;
    union {
	char *word;
	struct word_length wl;
    } v;
};

void
stratcl_add_word(dico_list_t list, const char *word)
{
    struct stratcl *sp = xmalloc(sizeof(*sp));
    sp->type = stratcl_word;
    sp->v.word = xstrdup(word);
    xdico_list_append(list, sp);
}

void
stratcl_add_cmp(dico_list_t list, enum cmp_op op, size_t len)
{
    struct stratcl *sp = xmalloc(sizeof(*sp));
    sp->type = stratcl_length;
    sp->v.wl.op = op;
    sp->v.wl.len = len;
    xdico_list_append(list, sp);
}

void
stratcl_add_disable(dico_list_t list)
{
    struct stratcl *sp = xmalloc(sizeof(*sp));
    sp->type = stratcl_all;
    xdico_list_append(list, sp);
}    

struct stratcl_check {
    const char *word;
    size_t len;
    int res;
};

static int
check_word(void *item, void *data)
{
    struct stratcl *sp = item;
    struct stratcl_check *cp = data;

    switch (sp->type) {
    case stratcl_all:
	cp->res = 1;
	break;

    case stratcl_word:
	cp->res = strcmp(sp->v.word, cp->word) == 0;
	break;

    case stratcl_length:
	switch (sp->v.wl.op) {
	case cmp_eq:
	    cp->res = cp->len == sp->v.wl.len;
	    break;
	    
	case cmp_ne:
	    cp->res = cp->len != sp->v.wl.len;
	    break;
	    
	case cmp_lt:
	    cp->res = cp->len < sp->v.wl.len;
	    break;
	    
	case cmp_le:
	    cp->res = cp->len <= sp->v.wl.len;
	    break;
	    
	case cmp_gt:
	    cp->res = cp->len > sp->v.wl.len;
	    break;
	    
	case cmp_ge:
	    cp->res = cp->len >= sp->v.wl.len;
	}
	break;
    }
    return cp->res;
}
	    
int
stratcl_check_word(dico_list_t list, const char *word)
{
    struct stratcl_check s;

    if (!list)
	return 0;
    s.word = word;
    s.len = strlen(word);
    s.res = 0;
    dico_list_iterate(list, check_word, &s);
    return s.res;
}
    

    
