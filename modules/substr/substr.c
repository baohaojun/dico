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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <appi18n.h>

static int
substr_sel(int cmd, struct dico_key *key, const char *dict_word)
{
    unsigned *sample;
    unsigned *tmp;
    int res;
    
    switch (cmd) {
    case DICO_SELECT_BEGIN:
	if (utf8_mbstr_to_wc(key->word, &sample, NULL))
	    return 1;
	utf8_wc_strupper(sample);
	key->call_data = sample;
	break;
	
    case DICO_SELECT_RUN:
	sample = key->call_data;
	if (utf8_mbstr_to_wc(dict_word, &tmp, NULL))
	    return 0;
	utf8_wc_strupper(tmp);
	res = !!utf8_wc_strstr(tmp, sample);
	free(tmp);
	return res;

    case DICO_SELECT_END:
	free(key->call_data);
	break;
    }
    return 0;
}

static struct dico_strategy substr_strat = {
    "substr",
    "Match a substring anywhere in the headword",
    substr_sel
};

static int
substr_init(int argc, char **argv)
{
    dico_strategy_add(&substr_strat);
    return 0;
}

struct dico_database_module DICO_EXPORT(substr, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NODB,
    substr_init,
};
