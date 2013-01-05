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
#include <regex.h>

struct regex_flags {
    int flags;
};
    
struct regex_data {
    regex_t reg;
};

static int
regex_sel(int cmd, dico_key_t key, const char *dict_word)
{
    char const *word = key->word;
    struct regex_flags *fp = key->strat->closure;
    struct regex_data *rp = key->call_data;
    int rc;

    switch (cmd) {
    case DICO_SELECT_BEGIN:
	rp = malloc(sizeof(*rp));
	if (!rp)
	    return 1;
	key->call_data = rp;
	rc = regcomp(&rp->reg, word, fp->flags);
	if (rc) {
	    char errbuf[512];
	    regerror(rc, &rp->reg, errbuf, sizeof(errbuf));
	    dico_log(L_ERR, 0, _("Regex error: %s"), errbuf);
	}
	break;

    case DICO_SELECT_RUN:
	rc = regexec(&rp->reg, dict_word, 0, NULL, 0) == 0;
	break;
	
    case DICO_SELECT_END:
	rc = 0;
	regfree(&rp->reg);
	free(rp);
	break;
    }
    return rc;
}

static struct regex_flags ext_flags = {
    REG_EXTENDED|REG_ICASE
};

static struct regex_flags basic_flags = {
    REG_ICASE
};

static struct dico_strategy re_strat = {
    "re",
    "POSIX 1003.2 (modern) regular expressions",
    regex_sel,
    &ext_flags
};

static struct dico_strategy regex_strat = {
    "regexp",
    "Old (basic) regular expressions",
    regex_sel,
    &basic_flags
};

void
register_regex()
{
    dico_strategy_add(&re_strat);
    dico_strategy_add(&regex_strat);
}

    
