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
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dico.h>
#include <errno.h>
#include <appi18n.h>
#include <wn.h>

#define WNDB_MERGE_DEFS 0x01

struct wndb {
    char *dbname;
    int flags;
    int pos;
    int optc;
    struct wn_option **optv;
};

static int
_dico_display_message(char *msg)
{
    dico_log(L_ERR, 0, "WordNet: %s", msg);
    return -1;
}

static int
wn_free_db(dico_handle_t hp)
{
    struct wndb *wndb = (struct wndb *)hp;
    free(wndb->dbname);
    free(wndb->optv);
    free(wndb);
    return 0;
}

static void wn_register_strategies(void);
static void wn_free_result(dico_result_t rp);

static int
wn_init(int argc, char **argv)
{
    char *wnsearchdir = NULL;
    char *wnhome = NULL;
    
    struct dico_option init_option[] = {
	{ DICO_OPTSTR(wnsearchdir), dico_opt_string, &wnsearchdir },
	{ DICO_OPTSTR(wnhome), dico_opt_string, &wnhome },
	{ NULL }
    };

    if (dico_parseopt(init_option, argc, argv, 0, NULL))
	return 1;

    display_message = _dico_display_message;
    /* FIXME: Control lengths! libWN does not bother to check boundaries */
    if (wnhome)
	setenv("WNHOME", wnhome, 1);
    if (wnsearchdir)
	setenv("WNSEARCHDIR", wnsearchdir, 1);
    if (wninit()) {
	dico_log(L_ERR, 0, _("cannot open wordnet database"));
	return 1;
    }

    wn_register_strategies();

    return 0;
}

static const char *pos_choice[] = {
    "all",
    "noun",
    "verb",
    "adj",
    "adjective",
    "adv",
    "adverb",
    "satellite",
    "adjsat",
    NULL
};
static int pos_trans[] = {
    ALL_POS,
    NOUN,
    VERB,
    ADJ,
    ADJ,
    ADV,
    ADV,
    ADJSAT,
    ADJSAT
};

/* Forward declarations */
struct result;
static void _wn_print_overview(struct wn_option *, SynsetPtr,
			       struct result *res, dico_stream_t);
static void _wn_print_definition(struct wn_option *opt, SynsetPtr sp,
				 struct result *res, dico_stream_t str);
static void _wn_print_hypernym(struct wn_option *, SynsetPtr,
			       struct result *res, dico_stream_t);

struct wn_option {
    char *option;	 /* user's search request */
    int search;		 /* search to pass findtheinfo() */
    int posmask;	 /* for what parts of speech it is defined */
    char *label;	 /* text for search header message */
    void (*tracer) (SynsetPtr);
    void (*printer) (struct wn_option *, SynsetPtr, struct result *,
		     dico_stream_t);
};

#define POS_MASK(n) bit(n)
#define PM_ALL 0xffffffff

static struct wn_option wn_optlist[] = {
    { "hypernym",   HYPERPTR, POS_MASK(NOUN)|POS_MASK(VERB),
      "Synonyms/Hypernyms (Ordered by Estimated Frequency)",
      NULL, _wn_print_hypernym },
    { "synonym",   },
#if 0
    /* These are some thoughts only */
    { "similar",   SIMPTR,    POS_MASK(ADJ), "Similarity" },
    { "synonym",   SYNS,      POS_MASK(ADV), "Synonyms" }, 
    { "antonym",   ANTPTR,    PM_ALL, "Antonyms" },
    { "pertainym", PERTPTR,   POS_MASK(ADV)|POS_MASK(ADV), "Pertainyms" },
    { "attribute", ATTRIBUTE, POS_MASK(NOUN)|POS_MASK(ADJ), "Attributes" },
    { "domain",    CLASSIFICATION,  PM_ALL, "Domain" },
    { "class",     CLASS,     PM_ALL,  "Domain Terms" },
    { "familarity", FREQ,     PM_ALL,  "Familiarity" },
    { "hypernym",   HYPERPTR, POS_MASK(NOUN)|POS_MASK(VERB),
      "Synonyms/Hypernyms (Ordered by Estimated Frequency)" },
    { "hypernym/rec", -HYPERPTR, POS_MASK(NOUN)|POS_MASK(VERB),
      "Synonyms/Hypernyms (Ordered by Estimated Frequency)" },
    { "hyponym",     HYPOPTR,  POS_MASK(NOUN)|POS_MASK(VERB), "Hyponyms" },
    { "hyponym/rec", -HYPOPTR, POS_MASK(NOUN), "Hyponyms" },
    { "holonym",     HOLONYM,  POS_MASK(NOUN), "Holonyms" },
    { "holonym/rec", -HHOLONYM, POS_MASK(NOUN), "Holonyms" },
    { "holonym/part", ISPARTPTR, POS_MASK(NOUN), "Part Holonyms" },
    { "holonym/member", ISMEMBERPTR, POS_MASK(NOUN), "Member Holonyms" },
    { "holonym/substance", ISSTUFFPTR, POS_MASK(NOUN), "Substance Holonyms" },
    { "meronym",      MERONYM, POS_MASK(NOUN), "Meronyms" },
    { "meronym/rec", -HMERONYM, POS_MASK(NOUN), "Meronyms" },
    { "meronym/substance", HASSTUFFPTR, POS_MASK(NOUN), "Substance Meronyms" },
    { "meronym/part", HASPARTPTR, POS_MASK(NOUN), "Part Meronyms" },
    { "meronym/member", HASMEMBERPTR, POS_MASK(NOUN), "Member Meronyms" },
    { "derivation", DERIVATION, POS_MASK(NOUN)|POS_MASK(VERB),
      "Derived Forms" },
    { "relatives", RELATIVES, POS_MASK(VERB),
      "Synonyms (Grouped by Similarity of Meaning)" },
    { "coords", COORDS, POS_MASK(NOUN)|POS_MASK(VERB),
      "Coordinate Terms (sisters)" },
    { "entail", ENTAILPTR, POS_MASK(VERB), "Entailment" },
    { "cause",  CAUSETO, POS_MASK(VERB), "\'Cause To\'" },
    { "frames", FRAMES, POS_MASK(VERB), "Sample Sentences" },
#endif
    { NULL }
};

static struct wn_option *
find_option(const char *text)
{
    struct wn_option *p;

    for (p = wn_optlist; p->option; p++) {
	if (strcmp(p->option, text) == 0)
	    return p;
    }
    return NULL;
}

static dico_handle_t
wn_init_db(const char *dbname, int argc, char **argv)
{
    struct wndb *wndb;
    int pos = 0;
    int idx, i, j;
    struct wn_option **optv;
    int optc;
    int flags = 0;
    static struct wn_option overview[2] = {
	{ "overview", OVERVIEW, PM_ALL, "Overview", NULL, _wn_print_overview },
	{ "overview", OVERVIEW, PM_ALL, "Overview", NULL, _wn_print_definition }
    };
	
    
    struct dico_option init_db_option[] = {
	{ DICO_OPTSTR(pos), dico_opt_enum, &pos, { enumstr: pos_choice } },
	{ DICO_OPTSTR(merge-defs), dico_opt_bool, &flags,
	  { value: WNDB_MERGE_DEFS } },
	{ NULL }
    };

    if (dico_parseopt(init_db_option, argc, argv, DICO_PARSEOPT_PERMUTE,
		      &idx))
	return NULL;

    argc -= idx;
    argv += idx;

    optc = argc + 1;
    optv = calloc(optc, sizeof(optv[0]));
    if (!optv) {
	dico_log(L_ERR, ENOMEM, "wn_init_db");
	return NULL;
    }

    optv[0] = overview + ((flags & WNDB_MERGE_DEFS) ? 1 : 0);
    
    for (i = 0, j = 1; i < argc; i++) {
	struct wn_option *p;
	
	if ((p = find_option(argv[i])) == NULL) {
	    dico_log(L_ERR, 0, _("wordnet: unknown option %s"), argv[i]);
	    free(optv);
	    return NULL;
	}
	while (p->search == 0 && p > wn_optlist)
	    p--;
	if (!p->printer) {
	    dico_log(L_WARN, 0, _("wordnet: option %s is not yet supported"),
		     argv[i]);
	    continue;
	}
	optv[j] = p;
    }
    
    wndb = calloc(1, sizeof(*wndb));
    if (!wndb) {
	dico_log(L_ERR, ENOMEM, "wn_init_db");
	free(optv);
	return NULL;
    }
    wndb->dbname = strdup(dbname);
    if (!wndb->dbname) {
	dico_log(L_ERR, ENOMEM, "wn_init_db");
	free(wndb);
	return NULL;
    }
    wndb->flags = flags;
    wndb->pos = pos_trans[pos];
    wndb->optc = optc;
    wndb->optv = optv;

    return (dico_handle_t)wndb;
}
    
static char *
wn_descr(dico_handle_t hp)
{
    return strdup("WordNet dictionary");
}

static char *
wn_info(dico_handle_t hp)
{
    return strdup(license);
}

static int
wn_lang(dico_handle_t hp, dico_list_t list[2])
{
    if ((list[0] = dico_list_create()) == NULL)
	return -1;
    if ((list[1] = dico_list_create()) == NULL) {
	dico_list_destroy(&list[0]);
	return -1;
    }
    dico_list_append(list[0], strdup("en"));
    dico_list_append(list[1], strdup("en"));
    return 0;
}

struct wordbuf {
    char *word;
    size_t len;
    size_t size;
};
#define WORDBUFINC 16
#define INIT_WORDBUF { NULL, 0, 0 }
#define wordbuf_start(wb) ((wb)->len=0)

static int
wordbuf_expand(struct wordbuf *wb, size_t len)
{
    if (len >= wb->size) {
	size_t size = ((len + WORDBUFINC - 1) /  WORDBUFINC ) * WORDBUFINC;
	char *newword = realloc(wb->word, size);
	if (!newword) {
	    dico_log(L_ERR, ENOMEM, "wordbuf_expand");
	    return 1;
	}
	wb->word = newword;
	wb->size = size;
    }
    return 0;
}

static int
wordbuf_grow(struct wordbuf *wb, int c)
{
    if (wordbuf_expand(wb, wb->len + 1))
	return 1;
    wb->word[wb->len++] = c;
    return 0;
}

static int
wordbuf_finish(struct wordbuf *wb)
{
    if (wordbuf_expand(wb, wb->len + 1))
	return 1;
    wb->word[wb->len] = 0;
    return 0;
}

static void
wordbuf_reverse(struct wordbuf *wb)
{
    int i, j;

    if (wb->len == 0)
	return;
    for (i = 0, j = wb->len - 1; i < j; i++, j--) {
	int c = wb->word[j];
	wb->word[j] = wb->word[i];
	wb->word[i] = c;
    }
}


enum result_type {
    result_match,
    result_define
};

struct defn {
    int pos;           /* Part of speach */
    SynsetPtr *synset; /* wndb->optc elements, first one always being
			  the overview. */
};

struct result {
    enum result_type type;
    size_t compare_count;
    struct wndb *wndb;
    dico_list_t list;
    dico_iterator_t itr;
    /* For definitions only: */
    char *searchword;
    dico_list_t rootlist; /* List of root synsets */
};

static int
free_defn(void *item, void *data)
{
    struct defn *dp = item;
    free(dp->synset);
    free(dp);
    return 0;
}

static int
free_root_synset(void *item, void *data)
{
    free_syns(item);
    return 0;
}

static int
free_string(void *item, void *data)
{
    free(item);
    return 0;
}

static int
compare_words(const void *a, void *b)
{
    return utf8_strcasecmp((char*)a, (char*)b);
}

static struct result *
wn_create_match_result(struct wndb *wndb)
{
    struct result *res;

    res = calloc(1, sizeof(*res));
    if (!res) {
	dico_log(L_ERR, ENOMEM, "wn_create_match_result");
	return NULL;
    }
    res->type = result_match;
    res->wndb = wndb;
    res->list = dico_list_create();
    if (!res) {
	dico_log(L_ERR, ENOMEM, "wn_create_match_result");
	free(res);
	return NULL;
    }
    dico_list_set_free_item(res->list, free_string, NULL);
    dico_list_set_comparator(res->list, compare_words);
    dico_list_set_flags(res->list, DICO_LIST_COMPARE_TAIL);
    return res;
}

static struct result *
wn_create_define_result(struct wndb *wndb, const char *searchword)
{
    struct result *res;

    res = calloc(1, sizeof(*res));
    if (!res) {
	dico_log(L_ERR, ENOMEM, "wn_create_define_result");
	return NULL;
    }
    res->type = result_define;
    res->wndb = wndb;
    res->list = dico_list_create();
    if (!res) {
	dico_log(L_ERR, ENOMEM, "wn_create_match_result");
	free(res);
	return NULL;
    }
    dico_list_set_free_item(res->list, free_defn, NULL);

    res->searchword = strdup(searchword);
    if (!res->searchword) {
	dico_log(L_ERR, ENOMEM, "wn_create_match_result");
	wn_free_result((dico_result_t) res);
    }

    res->rootlist = dico_list_create();
    if (!res->rootlist) {
	dico_log(L_ERR, ENOMEM, "wn_create_match_result");
	wn_free_result((dico_result_t) res);
    }
    dico_list_set_free_item(res->rootlist, free_root_synset, NULL);
    
    return res;
}

static int
wn_match_result_add(struct result *res, const char *hw)
{
    int rc;
    char *s = strdup(hw);

    if (!s) {
	dico_log(L_ERR, ENOMEM, "wn_result_add_key");
	return -1;
    }
    rc = dico_list_insert_sorted(res->list, s);
    if (rc) {
	free(s);
	if (rc != EEXIST) {
	    dico_log(L_ERR, ENOMEM, "wn_foreach_db");
	    return -1;
	}
    }
    return 0;
}

static void
skipeol(FILE *fp)
{
    int c;
    while ((c = getc(fp)) != '\n' && c != EOF)
	;
}

static void
skipheader(FILE *fp)
{
    int c;

    while ((c = getc(fp)) == ' ')
	skipeol(fp);
    ungetc(c, fp);
}

static int
getword(FILE *fp, struct wordbuf *wb)
{
    int c;
    size_t i;

    wordbuf_start(wb);
    while ((c = getc(fp)) != EOF) {
	if (c == ' ')
	    break;
	if (wordbuf_grow(wb, c))
	    return -1;
    }
    if (wb->len == 0 && c == EOF)
	return -1;
    if (wordbuf_finish(wb))
	return -1;
    for (i = 0; wb->word[i]; i++)
	if (wb->word[i] == '_')
	    wb->word[i] = ' ';
    return 0;
}

static int
lineback(FILE *fp, struct wordbuf *wb)
{
    int c, i;
    
    wordbuf_start(wb);
    while (fseek(fp, -2, SEEK_CUR) == 0) {
	if ((c = getc(fp)) == '\n')
	    break;
	if (wordbuf_grow(wb, c))
	    return -1;
    }
    if (wordbuf_finish(wb))
	return -1;
    wordbuf_reverse(wb);
    for (i = 0; wb->word[i]; i++) {
	if (wb->word[i] == ' ')
	    break;
	else if (wb->word[i] == '_')
	    wb->word[i] = ' ';
    }
    return 0;
}

static int
wn_is_defined(struct wndb *wndb, char *searchword)
{
    int i, j;
    unsigned int search;
    
    for (i = 1; i <= NUMPARTS; i++) {
	if ((search = is_defined(searchword, i)) != 0) {
	    for (j = 0; j < wndb->optc; j++) {
		int n;

		if (!(POS_MASK(i) & wndb->optv[j]->posmask))
		    continue;

		n = wndb->optv[j]->search;
		if (n < 0)
		    n = -n;
		if (bit(n) & search)
		    return 1;
	    }
	}
    }
    return 0;
}

static void
wn_foreach_db(struct wndb *wndb, int dbn, char *searchword,
	      struct dico_key *key, struct result *res)
{
    FILE *fp = indexfps[dbn];
    struct wordbuf wb = INIT_WORDBUF;
    dico_iterator_t itr;
    char *word;
    
    fseek(fp, 0, SEEK_SET);
    for (skipheader(fp); getword(fp, &wb) == 0; skipeol(fp)) {
	res->compare_count++;
	if (dico_key_match(key, wb.word)) {
	    if (wn_match_result_add(res, wb.word))
		break;
	}
    }
    free(wb.word);

    itr = dico_list_iterator(res->list);
    for (word = dico_iterator_first(itr); word;
	 word = dico_iterator_next(itr)) {
	if (!wn_is_defined(wndb, word))
	    dico_iterator_remove_current(itr, NULL);
    }
    dico_iterator_destroy(&itr);
}

static struct result *
wn_foreach(struct wndb *wndb, const dico_strategy_t strat, const char *word)
{
    struct result *res = wn_create_match_result(wndb);
    struct dico_key key;
    char *searchword;

    searchword = strdup(word);
    strtolower(strsubst(searchword, ' ', '_'));

    if (!searchword) {
	dico_log(L_ERR, ENOMEM, "wn_foreach");
	wn_free_result((dico_result_t) res);
	return NULL;
    }
	       
    if (dico_key_init(&key, strat, word)) {
	dico_log(L_ERR, 0, _("wn_foreach: key initialization failed"));
	wn_free_result((dico_result_t) res);
	free(searchword);
	return NULL;
    }
	       
    if (wndb->pos == ALL_POS) {
	int i;

	for (i = 1; i <= NUMPARTS; i++)
	    wn_foreach_db(wndb, i, searchword, &key, res);
    } else
	wn_foreach_db(wndb, wndb->pos, searchword, &key, res);
    
    dico_key_deinit(&key);
    free(searchword);
    
    if (dico_list_count(res->list) == 0) {
	wn_free_result((dico_result_t) res);
	return NULL;
    }
    return res;
}

static off_t
wn_bsearch(FILE *fp, void *key, int (*cmp)(const char *a, void *b))
{
    long top, mid, bot, diff;
    struct wordbuf wb = INIT_WORDBUF;
    off_t last_match = -1;
    
    fseek(fp, 0L, SEEK_END);
    top = 0;
    bot = ftell(fp);
    mid = (bot - top) / 2;

    do {
	fseek(fp, mid - 1, SEEK_SET);
	if (mid != 1)
	    skipeol(fp);
	if (getword(fp, &wb))
	    break;
	if (cmp(wb.word, key) < 0) {
	    top = mid;
	    diff = (bot - top) / 2;
	    mid = top + diff;
	} else if (cmp(wb.word, key) > 0) {
	    bot = mid;
	    diff = (bot - top) / 2;
	    mid = top + diff;
	} else {
	    /* Move back until first non-matching headword is found */
	    do
		last_match = ftell(fp);
	    while (lineback(fp, &wb) == 0 && cmp(wb.word, key) == 0);
	    break;
	}
    } while (diff);

    free(wb.word);
    return last_match;
}

static struct result *
wn_exact_match(struct wndb *db, const char *hw)
{
    struct result *res;

    if (!wn_is_defined(db, hw))
	return NULL;
    res = wn_create_match_result(db);
    dico_list_append(res->list, strdup(hw));
    return res;
}

struct prefix {
    const char *str;
    size_t len;
};

static int
cmp_pref(const char *hw, void *key)
{
    struct prefix *pref = key;
    return strncasecmp(hw, pref->str, pref->len);
}

static struct result *
wn_prefix_match(struct wndb *db, const char *hw)
{
    struct result *res;
    int i;
    struct prefix pfx;
    struct wordbuf wb = INIT_WORDBUF;
    
    res = wn_create_match_result(db);
    if (!res)
	return NULL;
    pfx.str = hw;
    pfx.len = strlen(hw);

    for (i = 1; i <= NUMPARTS; i++) {
	FILE *fp = indexfps[i];
	off_t off;

	off = wn_bsearch(fp, &pfx, cmp_pref);
	if (off != -1) {
	    fseek(fp, off, SEEK_SET);
	    for (;getword(fp, &wb) == 0 && cmp_pref(wb.word, &pfx) == 0;
		 skipeol(fp)) {
		if (wn_match_result_add(res, wb.word))
		    break;
	    }
	}
    }
    free(wb.word);
    if (dico_list_count(res->list) == 0) {
	wn_free_result((dico_result_t) res);
	return NULL;
    }
    return res;
}

typedef struct result *(*wn_matcher_t)(struct wndb *db, const char *hw);

struct strategy_def {
    struct dico_strategy strat;
    wn_matcher_t matcher;
};

static struct strategy_def stratdef[] = {
    { { "exact", "Match words exactly" }, wn_exact_match },
    { { "prefix", "Match word prefixes" }, wn_prefix_match },
};

static wn_matcher_t
wn_find_matcher(const char *strat)
{
    int i;
    for (i = 0; i < DICO_ARRAY_SIZE(stratdef); i++) 
	if (strcmp(strat, stratdef[i].strat.name) == 0)
	    return stratdef[i].matcher;
    return NULL;
}

static void
wn_register_strategies(void)
{
    int i;
    for (i = 0; i < DICO_ARRAY_SIZE(stratdef); i++) 
	dico_strategy_add(&stratdef[i].strat);
}

#define ISSPACE(c) ((c) == ' ' || (c) == '\t')

static char *
nornmalize_search_word(const char *word)
{
    char *copy = NULL;
    char *p;
    const char *q;

    copy = malloc(strlen(word) + 1);
    if (!copy) {
	dico_log(L_ERR, ENOMEM, "nornmalize_search_word");
	return NULL;
    }
    for (p = copy, q = word; *q; ) {
	if (ISSPACE(*q)) {
	    *p++ = '_';
	    do
		q++;
	    while (*q && ISSPACE(*q));
	} else
	    *p++ = *q++;
    }
    *p = 0;
    strtolower(copy);
    return copy;
}

static dico_result_t
wn_match(dico_handle_t hp, const dico_strategy_t strat, const char *word)
{
    struct wndb *wndb = (struct wndb *)hp;
    wn_matcher_t match = wn_find_matcher(strat->name);

    if (match)
	return (dico_result_t) match(wndb, word);
    else if (strat->sel)
	return (dico_result_t) wn_foreach(wndb, strat, word);
    return NULL;
}

static struct defn *
create_defn(struct wndb *wndb, int pos)
{
    struct defn *p = malloc(sizeof(*p));
    if (!p) {
	dico_log(L_ERR, ENOMEM, "create_defn");
	return NULL;
    }
    p->synset = calloc(wndb->optc, sizeof(p->synset[0]));
    if (!p->synset) {
	dico_log(L_ERR, ENOMEM, "create_defn");
	free(p);
	return NULL;
    }
    p->pos = pos;
    return p;
}

static int
search_defns(struct wndb *wndb, int pos, struct result *res,
	     const char *searchword)
{
    SynsetPtr sp;
    int i;
    struct defn *dp;
    int sense = 0;
    
    sp = findtheinfo_ds((char*)searchword, pos, OVERVIEW, ALLSENSES);

    if (!sp)
	return 0;
    dico_list_append(res->rootlist, sp);

    do {
	dp = create_defn(wndb, pos);
	if (!dp)
	    return 0;
	dp->synset[0] = sp;

	++sense;
	for (i = 1; i < wndb->optc; i++) {
	    SynsetPtr ssp;
	    if (!(POS_MASK(pos) & wndb->optv[i]->posmask))
		continue;
	    ssp = findtheinfo_ds((char*)searchword, pos, wndb->optv[i]->search,
				 sense);
	    if (ssp)
		dp->synset[i] = ssp;
	}
	dico_list_append(res->list, dp);
    } while (sp = sp->nextss);

    return 1;
}

static dico_result_t
wn_define(dico_handle_t hp, const char *word)
{
    struct wndb *wndb = (struct wndb *)hp;
    struct result *res;
    int i;
    int found = 0;
    char *copy;
    
    res = wn_create_define_result(wndb, word);
    
    copy = nornmalize_search_word(word);
    if (!copy) {
	wn_free_result((dico_result_t) res);
	return NULL;
    }

    res->wndb = wndb;
    res->type = result_define;
    if (wndb->pos == ALL_POS) {
	for (i = 1; i <= NUMPARTS; i++)
	    found += search_defns(wndb, i, res, copy);
    } else
	found = search_defns(wndb, wndb->pos, res, copy);

    if (!found) {
	free(copy);
	wn_free_result((dico_result_t) res);
	return NULL;
    }
    free(copy);
    
    return (dico_result_t)res;
}

static void
format_word(const char *word, dico_stream_t str)
{
    for (;;) {
	size_t len = strcspn(word, "_");
	dico_stream_write(str, word, len);
	if (word[len] == 0)
	    break;
	dico_stream_write(str, " ", 1);
	word += len + 1;
    }
}

static void
format_defn_string(const char *defn, dico_stream_t str)
{
    size_t len = strlen(defn);

    if (len == 0)
	return;
    if (defn[0] == '(' && defn[len-1] == ')') {
	    defn++;
	    len -= 2;
    }
    dico_stream_write(str, defn, len);
    dico_stream_write(str, "\n", 1);
}

static void
_wn_print_overview(struct wn_option *opt, SynsetPtr sp, struct result *res,
		   dico_stream_t str)
{
    int i;

    /* Print words: */
    for (i = 0; i < sp->wcount; i++) {
	if (i)
	    dico_stream_write(str, ", ", 2);
	format_word(sp->words[i], str);
    }

    /* Print pos */
    dico_stream_write(str, "; ", 2);
    dico_stream_write(str, sp->pos, strlen(sp->pos));
    dico_stream_write(str, ".\n\n", 3);

    /* Print definition */
    format_defn_string(sp->defn, str);
}

static void
_wn_print_definition(struct wn_option *opt, SynsetPtr sp, struct result *res,
		     dico_stream_t str)
{
    /* Print definition */
    format_defn_string(sp->defn, str);

    if (sp->wcount > 1) {
	int i, j;
	static char const *sym = "Synonyms: ";
	
	dico_stream_write(str, sym, strlen(sym));
	for (i = j = 0; i < sp->wcount; i++) {
	    if (strcmp(sp->words[i], res->searchword) == 0)
		continue;
	    if (j)
		dico_stream_write(str, ", ", 2);
	    dico_stream_write(str, "{", 1);
	    format_word(sp->words[i], str);
	    dico_stream_write(str, "}", 1);
	    j++;
	}
	dico_stream_write(str, "\n", 1);
    }
    
}

static void
_wn_print_hypernym(struct wn_option *opt, SynsetPtr ptr, struct result *res,
		   dico_stream_t str)
{
    int i;
    SynsetPtr sp;
    
    dico_stream_write(str, opt->label, strlen(opt->label));
    dico_stream_write(str, ":\n\n", 3);

    for (sp = ptr->ptrlist; sp; sp = sp->nextss) {
	for (i = 0; i < sp->wcount; i++) {
	    if (i)
		dico_stream_write(str, ", ", 2);
	    format_word(sp->words[i], str);
	}
	dico_stream_write(str, "\n", 1);
    }
}

static int
format_defn(struct defn *defn, struct result *res, dico_stream_t str)
{
    int i;
    struct wndb *wndb = res->wndb;

    for (i = 0; i < wndb->optc; i++)
	wndb->optv[i]->printer(wndb->optv[i], defn->synset[i], res, str);
    return 0;
}

static void
print_num(dico_stream_t str, unsigned num)
{
    char buf[128];
    char *p = buf + sizeof(buf);

    *--p = 0;
    while (p > buf && num) {
	*--p = num % 10 + '0';
	num /= 10;
    }
    dico_stream_write(str, p, strlen(p));
    dico_stream_write(str, ". ", 2);
}

static void
format_all_defns(struct result *res, dico_stream_t str)
{
    struct defn *defn;
    int pos = 0;
    unsigned num;

    format_word(res->searchword, str);
    dico_stream_write(str, "\n", 1);

    for (defn = dico_iterator_first(res->itr); defn;
	 defn = dico_iterator_next(res->itr)) {
	if (defn->pos != pos) {
	    pos = defn->pos;
	    num = 1;

	    dico_stream_write(str, defn->synset[0]->pos,
			      strlen(defn->synset[0]->pos));
	    dico_stream_write(str, ". ", 2);
	} 

	print_num(str, num);
	format_defn(defn, res, str);
	num++;
    }
}

int
wn_output_result(dico_result_t rp, size_t n, dico_stream_t str)
{
    struct result *res = (struct result *) rp;
    void *item;
    
    if (!res->itr) {
	res->itr = dico_list_iterator(res->list);
	if (!res->itr)
	    return 1;
    }
    item = dico_iterator_item(res->itr, n);
    switch (res->type) {
    case result_match:
	dico_stream_write(str, item, strlen((char*)item));
	break;

    case result_define:
	if (res->wndb->flags & WNDB_MERGE_DEFS)
	    format_all_defns(res, str);
	else
	    format_defn(item, res, str);
	break;
	    
    default:
	return 1;
    }
    return 0;
}

static size_t
wn_result_count (dico_result_t rp)
{
    struct result *res = (struct result *) rp;
    if (res->type == result_define && (res->wndb->flags & WNDB_MERGE_DEFS))
	    return 1;
    return dico_list_count(res->list);
}

static size_t
wn_compare_count (dico_result_t rp)
{
    struct result *res = (struct result *) rp;
    return res->compare_count;
}

static void
wn_free_result(dico_result_t rp)
{
    struct result *res = (struct result *) rp;
    
    dico_list_destroy(&res->list);
    dico_iterator_destroy(&res->itr);
    dico_list_destroy(&res->rootlist);
    free(res->searchword);
    free(res);
}


struct dico_database_module DICO_EXPORT(wordnet, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NONE,
    wn_init,
    wn_init_db,
    wn_free_db,
    NULL,
    NULL,
    wn_info,
    wn_descr,
    wn_lang,
    wn_match,
    wn_define,
    wn_output_result,
    wn_result_count,
    wn_compare_count,
    wn_free_result
};


