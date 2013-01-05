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
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <appi18n.h>
#include <pcre.h>

struct dico_pcre_flag
{
    int c;
    int flag;
};

static struct dico_pcre_flag flagtab[] = {
    { 'a', PCRE_ANCHORED }, /* Force pattern anchoring */
    { 'e', PCRE_EXTENDED }, /* Ignore whitespace and # comments */
    { 'i', PCRE_CASELESS }, /* Do caseless matching */
    { 'G', PCRE_UNGREEDY }, /* Invert greediness of quantifiers */
    { 0 },
};

static int
pcre_flag(int c, int *pflags)
{
    struct dico_pcre_flag *p;
    
    for (p = flagtab; p->c; p++) {
	if (p->c == c) {
	    *pflags |= p->flag;
	    return 0;
	} else if (p->c == tolower(c) || p->c == toupper(c)) {
	    *pflags &= ~p->flag;
	    return 0;
	}
    }
    return 1;
}
    
static pcre *
compile_pattern(const char *pattern)
{
    int cflags = PCRE_UTF8|PCRE_NEWLINE_ANY;
    const char *error;
    int error_offset;
    char *tmp = NULL;
    pcre *pre;
	
    if (pattern[0] == '/') {
	size_t len;
	char *p;

	pattern++;
	p = strrchr(pattern, '/');
	if (!p) {
	    dico_log(L_ERR, 0, _("PCRE missing terminating /: %s"),
		     pattern - 1);
	    return NULL;
	}
	len = p - pattern;

	while (*++p) {
	    if (pcre_flag(*p, &cflags)) {
		dico_log(L_ERR, 0, _("PCRE error: invalid flag %c"), *p);
		return NULL;
	    }
	}
    
	tmp = malloc(len + 1);
	if (!tmp)
	    return NULL;
	memcpy(tmp, pattern, len);
	tmp[len] = 0;
	pattern = tmp;
    }
    pre = pcre_compile(pattern, cflags, &error, &error_offset, 0);
    if (!pre) {
	dico_log(L_ERR, 0, 
		 _("pcre_compile(\"%s\") failed at offset %d: %s"),
		 pattern, error_offset, error);
    }
    free(tmp);
    return pre;
}

static int
pcre_sel(int cmd, dico_key_t key, const char *dict_word)
{
    int rc = 0;
    char const *word = key->word;
    pcre *pre = key->call_data;

    switch (cmd) {
    case DICO_SELECT_BEGIN:
	pre = compile_pattern(word);
	if (!pre)
	    return 1;
	key->call_data = pre;
	break;

    case DICO_SELECT_RUN:
	rc = pcre_exec(pre, 0, dict_word, strlen(dict_word), 0, 0,
		       NULL, 0) >= 0;
	break;
	
    case DICO_SELECT_END:
	pcre_free(pre);
	break;
    }
    return rc;
}

static struct dico_strategy pcre_strat = {
    "pcre",
    "Match using Perl-compatible regular expressions",
    pcre_sel
};

static int
pcre_init(int argc, char **argv)
{
    dico_strategy_add(&pcre_strat);
    return 0;
}

struct dico_database_module DICO_EXPORT(pcre, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NODB,
    pcre_init,
};

