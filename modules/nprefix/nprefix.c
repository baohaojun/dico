/* This file is part of GNU Dico.
   Copyright (C) 2012 Sergey Poznyakoff

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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* MATCH <dict> nprefix <skip>#<count>#<string>

   Returns at most <count> headwords whose prefix matches <string>,
   skipping <skip> first unique matches.
*/

struct nprefix {
    char *prefix;           /* Prefix being looked for */
    size_t pfxlen;          /* Its length in characters */
    int lim;                /* If true, use the limits below */
    size_t skip;            /* Skip this number of matches */
    size_t count;           /* Display at most this number of matches */
    size_t n;               /* Number of the current match */
    char *last_match_str;   /* Last matched string */    
    size_t last_match_len;  /* Size of last_match_str */
};

static int
nprefix_sel(int cmd, dico_key_t key, const char *dict_word)
{
    char const *key_word = key->word;
    struct nprefix *np;
    size_t wordlen;
    
    switch (cmd) {
    case DICO_SELECT_BEGIN: {
	char *p;
	size_t skip, count;
	
	np = calloc(1, sizeof(np[0]));
	if (!np) {
	    dico_log(L_ERR, errno, "nprefix_sel");
	    return 1;
	}
	np->prefix = (char*)key_word;
	np->lim = 0;
	skip = strtoul(key_word, &p, 10);
	if (*p == '#') {
	    count = strtoul(p + 1, &p, 10);
	    if (*p == '#') {
		np->prefix = p + 1;
		np->skip = skip;
		np->count = count;
		np->lim = 1;
	    }
	}
	np->pfxlen = utf8_strlen(np->prefix);
	key->call_data = np;
	break;
    }
	
    case DICO_SELECT_RUN:
	np = key->call_data;
	if (np->last_match_str &&
	    utf8_strcasecmp((char*)dict_word, np->last_match_str) == 0)
	    return 0;
	if (np->lim && np->n > np->skip + np->count)
	    return 0;
	wordlen = utf8_strlen(dict_word);
	if (wordlen >= np->pfxlen &&
	    utf8_strncasecmp((char*)dict_word, np->prefix, np->pfxlen) == 0) {
	    size_t s = strlen(dict_word) + 1;
	    if (np->last_match_len < s) {
		char *p = realloc(np->last_match_str, s);
		if (!p)
		    return 0;
		
		np->last_match_str = p;
		np->last_match_len = s;
	    }
	    strcpy(np->last_match_str, dict_word);
	    if (!np->lim)
		return 1;
	    np->n++;
	    return np->n > np->skip && np->n <= np->skip + np->count;
	}
	break;

    case DICO_SELECT_END:
	np = key->call_data;
	free(np->last_match_str);
	free(np);
	break;
    }
    return 0;
}

static struct dico_strategy nprefix_strat[] = {
    { "nprefix", "Match prefixes, [skip#count#]prefix", nprefix_sel },
    { NULL }
};

static int
nprefix_init(int argc, char **argv)
{
    dico_strategy_add(nprefix_strat);
    return 0;
}

struct dico_database_module DICO_EXPORT(nprefix, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NODB,
    nprefix_init,
};

	
