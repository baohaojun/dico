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

#ifndef __dico_strat_h
#define __dico_strat_h

#include <dico/types.h>
#include <dico/list.h>

struct dico_strategy {
    char *name;             /* Strategy name */
    char *descr;            /* Strategy description */
    dico_select_t sel;      /* Selector function (can be NULL) */
    void *closure;          /* Additional data for SEL */ 
    int is_default;         /* True, if this is a default strategy */
    dico_list_t stratcl;    /* Strategy access control list */  
};

struct dico_key {
    char *word;
    void *call_data;
    dico_strategy_t strat;
    int flags;
};

int dico_strat_name_cmp(const void *item, void *data);
int dico_strat_free(void *item, void *data);

dico_strategy_t dico_strategy_create(const char *name, const char *descr);
dico_strategy_t dico_strategy_dup(const dico_strategy_t strat);
dico_strategy_t dico_strategy_find(const char *name);
int dico_strategy_add(const dico_strategy_t strat);
dico_iterator_t  dico_strategy_iterator(void);
void dico_strategy_iterate(dico_list_iterator_t itr, void *data);
size_t dico_strategy_count(void);

int dico_set_default_strategy(const char *name);
dico_strategy_t dico_get_default_strategy(void);
#define dico_strategy_is_default_p(s) ((s)->is_default)

void dico_key_deinit(struct dico_key *key);
int dico_key_init(struct dico_key *key, dico_strategy_t strat,
		  const char *word);
int dico_key_match(struct dico_key *key, const char *word);


#endif
