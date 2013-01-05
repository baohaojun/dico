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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define _DKF_INIT 0x01

void
dico_key_deinit(struct dico_key *key)
{
    if (key->flags & _DKF_INIT) {
	if (key->strat->sel)
	    key->strat->sel(DICO_SELECT_END, key, NULL);
	key->flags = 0;
	free(key->word);
    }
    memset(key, 0, sizeof(key[0]));
}

int
dico_key_init(struct dico_key *key, dico_strategy_t strat, const char *word)
{
    memset(key, 0, sizeof(key[0]));
    key->word = strdup (word);
    key->strat = strat;
    if (strat->sel && strat->sel(DICO_SELECT_BEGIN, key, word))
	return 1;
    key->flags |= _DKF_INIT;
    return 0;
}

int
dico_key_match(struct dico_key *key, const char *word)
{
    if (!key || !(key->flags & _DKF_INIT) || !word) {
	errno = EINVAL;
	return -1;
    }
    return key->strat->sel(DICO_SELECT_RUN, key, word);
}
