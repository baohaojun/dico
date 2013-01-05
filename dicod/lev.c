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

#include <dicod.h>

static int levenshtein_distance = 1;

static int
lev_sel(int cmd, dico_key_t key, const char *dict_word)
{
    if (cmd == DICO_SELECT_RUN) {
	int dist = dico_levenshtein_distance(key->word, dict_word,
					     (int)key->strat->closure);
	if (dist < 0)
	    return 0;
	return dist <= levenshtein_distance;
    }
    return 0;
}

static struct dico_strategy levstrat[] = {
    { "lev",
      "Match headwords within given Levenshtein distance",
      lev_sel,
      NULL },
    { "nlev",
      "Match headwords within given Levenshtein distance (normalized)",
      lev_sel,
      (void*)DICO_LEV_NORM },
    { "dlev",
      "Match headwords within given Damerau-Levenshtein distance",
      lev_sel,
      (void*)DICO_LEV_DAMERAU },
    { "ndlev",
      "Match headwords within given Damerau-Levenshtein distance (normalized)",
      lev_sel,
      (void*)(DICO_LEV_NORM|DICO_LEV_DAMERAU) }
};

static void
dicod_xlevdist(dico_stream_t str, int argc, char **argv)
{
    if (c_strcasecmp(argv[1], "tell") == 0) 
	stream_printf(str, "280 %d\n", levenshtein_distance);
    else if (isdigit(argv[1][0]) && argv[1][0] != '0' && argv[1][1] == 0) {
	levenshtein_distance = atoi(argv[1]);
	stream_printf(str, "250 ok - Levenshtein threshold set to %d\n",
		      levenshtein_distance);
    } else
	stream_writez(str, "500 invalid argument\n");
}
	
void
register_lev()
{
    int i;
    static struct dicod_command cmd[] = {
	{ "XLEV", 2, 2, "distance", "Set Levenshtein distance",
	  dicod_xlevdist },
	{ NULL }
    };
    for (i = 0; i < DICO_ARRAY_SIZE(levstrat); i++)
	dico_strategy_add(&levstrat[i]);
    dico_set_default_strategy("nlev");
    dicod_capa_register("xlev", cmd, NULL, NULL);
}

	  
