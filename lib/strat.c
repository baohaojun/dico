/* This file is part of GNU Dico.
   Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff

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
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* List of configured matching strategies */
static dico_list_t /* of struct dico_strategy */ strategy_list;
static dico_strategy_t default_strategy;

#define DEFSTRATNAME(s) ((s)[0] == '.' && (s)[1] == 0)

int
dico_strat_name_cmp(const void *item, void *data)
{
    dico_strategy_t strat = (dico_strategy_t) item;
    const char *name = data;
    return strcmp(strat->name, name);
}

dico_strategy_t
dico_strategy_create(const char *name, const char *descr)
{
    dico_strategy_t np;
    size_t size = sizeof(*np) + strlen(name) + strlen(descr) + 2;
    np = malloc(size);
    if (np) {
	memset(np, 0, size);
	np->name = (char*)(np + 1);
	strcpy(np->name, name);
	np->descr = np->name + strlen(np->name) + 1;
	strcpy(np->descr, descr);
    }
    return np;
}

int
dico_strat_free(void *item, void *data)
{
    dico_strategy_t strat = item;
    dico_list_destroy(&strat->stratcl);
    free(strat);
    return 0;
}

dico_strategy_t
dico_strategy_dup(const dico_strategy_t strat)
{
    dico_strategy_t np = dico_strategy_create(strat->name, strat->descr);
    if (np) {
	np->sel = strat->sel;
	np->closure = strat->closure;
    }
    return np;
}

dico_strategy_t
dico_strategy_find(const char *name)
{
    if (DEFSTRATNAME(name)) 
	return default_strategy;
    return dico_list_locate(strategy_list, (void*)name);
}

int
dico_strategy_add(const dico_strategy_t strat)
{
    if (!strategy_list) {
	strategy_list = dico_list_create();
	if (!strategy_list)
	    return 1;
	dico_list_set_comparator(strategy_list, dico_strat_name_cmp);
	dico_list_set_free_item(strategy_list, dico_strat_free, NULL);
    }
    if (!dico_strategy_find(strat->name)) {
	dico_strategy_t new_strat = dico_strategy_dup(strat);
	if (!new_strat)
	    return 1;
	dico_list_append(strategy_list, new_strat);
    }
    return 0;
}

dico_iterator_t 
dico_strategy_iterator()
{
    return dico_list_iterator(strategy_list);
}

size_t
dico_strategy_count()
{
    return dico_list_count(strategy_list);
}

void
dico_strategy_iterate(dico_list_iterator_t itr, void *data)
{
    return dico_list_iterate(strategy_list, itr, data);
}

int
dico_set_default_strategy(const char *name)
{
    dico_strategy_t sp;

    if (DEFSTRATNAME(name) || (sp = dico_strategy_find(name)) == NULL) {
	errno = EINVAL;
	return 1;
    }
    if (default_strategy)
	default_strategy->is_default = 0;
    sp->is_default = 1;
    default_strategy = sp;
    return 0;
}

dico_strategy_t
dico_get_default_strategy()
{
    return default_strategy;
}

    
