/* This file is part of GNU Dico.
   Copyright (C) 2010, 2012 Sergey Poznyakoff

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

static int
word_sel(int cmd, dico_key_t key, const char *dict_word)
{
    int rc = 0;
    char const *key_word = key->word;
    struct dico_tokbuf tb;
    int i;
    
    switch (cmd) {
    case DICO_SELECT_BEGIN:
	break;

    case DICO_SELECT_RUN:
	dico_tokenize_begin(&tb);
	if (dico_tokenize_string(&tb, (char*) dict_word)) {
	    dico_tokenize_end(&tb);
	    return 0;
	}
	for (i = 0; i < tb.tb_tokc; i++)
	    if (utf8_strcasecmp(tb.tb_tokv[i], (char*) key_word) == 0) {
		rc = 1;
		break;
	    }
	dico_tokenize_end(&tb);
	break;

    case DICO_SELECT_END:
	break;
    }
    return rc;
}

static int
first_sel(int cmd, dico_key_t key, const char *dict_word)
{
    int rc = 0;
    char const *key_word = key->word;
    struct dico_tokbuf tb;
    
    switch (cmd) {
    case DICO_SELECT_BEGIN:
	break;

    case DICO_SELECT_RUN:
	dico_tokenize_begin(&tb);
	if (dico_tokenize_string(&tb, (char*) dict_word) == 0 && tb.tb_tokc) 
	    rc = utf8_strcasecmp(tb.tb_tokv[0], (char*) key_word) == 0;
	dico_tokenize_end(&tb);
	break;

    case DICO_SELECT_END:
	break;
    }
    return rc;
}

static int
last_sel(int cmd, dico_key_t key, const char *dict_word)
{
    int rc = 0;
    char const *key_word = key->word;
    struct dico_tokbuf tb;
    
    switch (cmd) {
    case DICO_SELECT_BEGIN:
	break;

    case DICO_SELECT_RUN:
	dico_tokenize_begin(&tb);
	if (dico_tokenize_string(&tb, (char*) dict_word) == 0 && tb.tb_tokc)
	    rc = utf8_strcasecmp(tb.tb_tokv[tb.tb_tokc-1],
				 (char*) key_word) == 0;
	dico_tokenize_end(&tb);
	break;

    case DICO_SELECT_END:
	break;
    }
    return rc;
}

static struct dico_strategy strats[] = {
    { "word", "Match a word anywhere in the headword", word_sel },
    { "first", "Match the first word within headwords", first_sel },
    { "last", "Match the last word within headwords", last_sel },
    { NULL }
};

static struct dico_strategy *
findstrat(const char *name)
{
    struct dico_strategy *sp;
    for (sp = strats; sp->name; sp++)
	if (strcmp(sp->name, name) == 0)
	    return sp;
    dico_log(L_ERR, 0, _("unknown strategy: %s"), name);
    return NULL;
}

static int
word_init(int argc, char **argv)
{
    struct dico_strategy *sp;

    if (argc == 1) {
	for (sp = strats; sp->name; sp++)
	    dico_strategy_add(sp);
    } else {
	int i;
	
	for (i = 1; i < argc; i++)
	    if ((sp = findstrat(argv[i])))
		dico_strategy_add(sp);
    }
    return 0;
}

struct dico_database_module DICO_EXPORT(word, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NODB,
    word_init,
};

	
