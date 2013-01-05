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
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dico.h>
#include "gcide.h"
#include <errno.h>
#include <appi18n.h>

#define GCIDE_NOPR     0x01
#define GCIDE_DBGLEX   0x02

struct gcide_db {
    char *db_dir;
    char *idx_dir;
    char *tmpl_name;
    char *tmpl_letter;
    char *idxgcide;
    int flags;
    time_t latest_change;
    
    int file_letter;
    dico_stream_t file_stream;
    
    size_t idx_cache_size;
    gcide_idx_file_t idx;
};

enum result_type {
    result_match,
    result_define
};

struct gcide_result {
    enum result_type type;
    struct gcide_db *db;
    size_t compare_count;
    dico_iterator_t itr;
    dico_list_t list;
};

static char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char *idxgcide_program = LIBEXECDIR "/idxgcide";

static void
free_db(struct gcide_db *db)
{
    free(db->db_dir);
    free(db->idx_dir);
    free(db->tmpl_name);
    free(db->idxgcide);
    if (db->file_stream) {
	dico_stream_close(db->file_stream);
	dico_stream_destroy(&db->file_stream);
    }	
    gcide_idx_file_close(db->idx);
    free(db);
}

static int
gcide_check_dir(const char *dir)
{
    struct stat st;
    
    if (stat(dir, &st)) {
	dico_log(L_ERR, errno, _("gcide: cannot stat `%s'"), dir);
	return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
	dico_log(L_ERR, 0, _("gcide: `%s' is not a directory"), dir);
	return 1;
    }
    if (access(dir, R_OK)) {
	dico_log(L_ERR, 0, _("gcide: `%s' is not readable"), dir);
	return 1;
    }
    return 0;
}

static char *
gcide_template_name(struct gcide_db *db, int let)
{
    *db->tmpl_letter = let;
    return db->tmpl_name;
}

static int
run_idxgcide(char *idxname, struct gcide_db *db)
{
    pid_t pid;
    int status;
    char *idxgcide = db->idxgcide ? db->idxgcide : idxgcide_program;
    
    dico_log(L_NOTICE, 0, _("gcide_open_idx: creating index %s"),
	     idxname);
    if (access(idxgcide, X_OK)) {
	dico_log(L_ERR, errno, _("gcide_open_idx: cannot run %s"),
		 idxgcide);
	return 1;
    }
    pid = fork();
    if (pid == 0) {
	execl(idxgcide, idxgcide, db->db_dir, db->idx_dir, NULL);
    }
    if (pid == -1) {
	dico_log(L_ERR, errno, _("gcide_open_idx: fork failed"));
	return 1;
    }
    if (waitpid(pid, &status, 0) != pid) {
	dico_log(L_ERR, errno, _("gcide_open_idx: %s failed"), idxgcide);
	kill(pid, SIGKILL);
	return 1;
    }
    if (!WIFEXITED(status)) {
	dico_log(L_ERR, 0, _("gcide_open_idx: %s failed"), idxgcide);
	return 1;
    }

    status = WEXITSTATUS(status);
    if (status) {
	dico_log(L_ERR, 0,
		 _("gcide_open_idx: %s exited with status %d"),
		 idxgcide, status);
	return 1;
    }
    return 0;
}

static int
gcide_check_files(struct gcide_db *db)
{
    int i;
    time_t t = 0;
    struct stat st;
    
    for (i = 0; letters[i]; i++) {
	char *p = gcide_template_name(db, letters[i]);
	if (access(p, R_OK)) {
	    dico_log(L_ERR, 0, _("gcide: `%s' is not readable"), p);
	    return 1;
	}
	if (stat(p, &st)) {
	    dico_log(L_ERR, errno, _("gcide: can't stat `%s'"), p);
	    return 1;
	}
	if (st.st_mtime > t)
	    t = st.st_mtime;
    }
    db->latest_change = t;
    return 0;
}

/* Try to access IDXNAME.  Return 0 on success, 1 if it should be (re)created
   and -1 on error */
static int
gcide_access_idx(struct gcide_db *db, char *idxname)
{
    int rc = 1;
    struct stat st;

    if (access(idxname, R_OK) == 0) {
	if (stat(idxname, &st)) {
	    dico_log(L_ERR, errno, _("gcide: can't stat `%s'"), idxname);
	    /* try to create it, anyway */
	} else if (db->latest_change <= st.st_mtime)
	    rc = 0;
	else
	    dico_log(L_NOTICE, 0,
		     _("gcide: index file older than database, reindexing"));
    } else if (errno != ENOENT) {
	dico_log(L_ERR, errno, _("gcide_open_idx: cannot access %s"),
		 idxname);
	rc = -1;
    }
    return rc;
}
    
static int
gcide_open_idx(struct gcide_db *db)
{
    int rc = 1;
    char *idxname;
    
    idxname = dico_full_file_name(db->idx_dir, "GCIDE.IDX");
    if (!idxname) {
	dico_log(L_ERR, errno, "gcide_open_idx");
	return 1;
    }
    
    rc = gcide_access_idx(db, idxname);
    if (rc == 1)
	rc = run_idxgcide(idxname, db);
	
    if (rc == 0) {
	db->idx = gcide_idx_file_open(idxname, db->idx_cache_size);
	if (!db->idx)
	    rc = 1;
    }
    
    free(idxname);
    return rc;
}

static dico_handle_t
gcide_init_db(const char *dbname, int argc, char **argv)
{
    char *db_dir = NULL;
    char *idx_dir = NULL;
    char *idxgcide = NULL;
    long idx_cache_size = 16;
    int flags = 0;
    struct gcide_db *db;
    
    struct dico_option init_db_option[] = {
	{ DICO_OPTSTR(dbdir), dico_opt_string, &db_dir },
	{ DICO_OPTSTR(idxdir), dico_opt_string, &idx_dir },
	{ DICO_OPTSTR(index-program), dico_opt_string, &idxgcide },
	{ DICO_OPTSTR(index-cache-size), dico_opt_long, &idx_cache_size },
	{ DICO_OPTSTR(suppress-pr), dico_opt_bitmask, &flags, { value: GCIDE_NOPR } },
	{ DICO_OPTSTR(debug-lex), dico_opt_bitmask, &flags, { value: GCIDE_DBGLEX } },
	{ NULL }
    };
    
    if (dico_parseopt(init_db_option, argc, argv, 0, NULL))
	return NULL;
    if (!db_dir) {
	dico_log(L_ERR, 0,
		 _("gcide_init_db: database directory not given"));
	return NULL;
    }
    if (!idx_dir) {
	idx_dir = strdup(db_dir);
	if (!idx_dir) {
	    dico_log(L_ERR, errno, "gcide_init_db");
	    free(db_dir);
	    return NULL;
	}
    }

    db = calloc(1, sizeof(*db));
    if (!db) {
	free(db_dir);
	free(idx_dir);
	dico_log(L_ERR, ENOMEM, "gcide_init_db");
	return NULL;
    }
    db->db_dir = db_dir;
    db->idx_dir = idx_dir;
    db->idx_cache_size = idx_cache_size;
    db->flags = flags;
    
    if (gcide_check_dir(db->db_dir) || gcide_check_dir(db->idx_dir)) {
	free_db(db);
	return NULL;
    }

    db->tmpl_name = dico_full_file_name(db->db_dir, "CIDE.A");
    db->tmpl_letter = db->tmpl_name + strlen(db->tmpl_name) - 1;
    if (gcide_check_files(db)) {
	free_db(db);
	return NULL;
    }
    db->idxgcide = idxgcide; 
    if (gcide_open_idx(db)) {
	free_db(db);
	return NULL;
    }
    return (dico_handle_t)db;
}

static int
gcide_free_db(dico_handle_t hp)
{
    struct gcide_db *db = (struct gcide_db *) hp;
    free_db(db);
    return 0;
}

static int
_is_nl_or_punct(int c)
{
    return !!strchr("\r\n!,-./:;?", c);
}

static char *
read_info_file(const char *fname, int first_line)
{
    dico_stream_t stream;
    int rc;
    char *bufptr = NULL;
    size_t bufsize = 0;
    
    stream = dico_mapfile_stream_create(fname, DICO_STREAM_READ);
    if (!stream) {
	dico_log(L_NOTICE, errno, _("cannot create stream `%s'"), fname);
	return NULL;
    }

    rc = dico_stream_open(stream);
    if (rc) {
	dico_log(L_ERR, 0,
		 _("cannot open stream `%s': %s"),
		 fname, dico_stream_strerror(stream, rc));
	dico_stream_destroy(&stream);
	return NULL;
    }

    if (first_line) {
	size_t n;
	
	rc = dico_stream_getline(stream, &bufptr, &bufsize, &n);
	if (rc) {
	    dico_log(L_ERR, 0,
		     _("read error in stream `%s': %s"),
		     fname, dico_stream_strerror(stream, rc));
	} else
	    dico_string_trim(bufptr, n, _is_nl_or_punct);
    } else {
	off_t size;
	rc = dico_stream_size(stream, &size);
	if (rc) {
	    dico_log(L_ERR, 0,
		     _("cannot get size of stream `%s': %s"),
		     fname, dico_stream_strerror(stream, rc));
	} else {
	    bufsize = size;
	    bufptr = malloc (bufsize + 1);
	    if (!bufptr) {
		dico_log(L_ERR, errno,
			 _("cannot allocate dictionary information buffer"));
	    } else if ((rc = dico_stream_read(stream, bufptr, bufsize, NULL))) {
		dico_log(L_ERR, 0,
			 _("read error in stream `%s': %s"),
			 fname, dico_stream_strerror(stream, rc));
		free(bufptr);
		bufptr = NULL;
	    } else
		bufptr[bufsize] = 0;
	}
    }
    
    dico_stream_destroy(&stream);
    return bufptr;
}

static char *
read_dictionary_info(struct gcide_db *db, int first_line)
{
    char *fname = dico_full_file_name(db->db_dir, "INFO");
    char *info = read_info_file(fname, first_line);
    free(fname);
    return info;
}

char *
gcide_info(dico_handle_t hp)
{
    return read_dictionary_info((struct gcide_db *) hp, 0);
}

char *
gcide_descr(dico_handle_t hp)
{
    return read_dictionary_info((struct gcide_db *) hp, 1);
}

static gcide_iterator_t
exact_match(struct gcide_db *db, const char *hw)
{
    return gcide_idx_locate(db->idx, (char*)hw, 0);
}

static gcide_iterator_t
prefix_match(struct gcide_db *db, const char *hw)
{
    return gcide_idx_locate(db->idx, (char*)hw, utf8_strlen(hw));
}

typedef gcide_iterator_t (*matcher_t)(struct gcide_db *, const char *);

struct strategy_def {
    struct dico_strategy strat;
    matcher_t matcher;
};

static struct strategy_def strat_tab[] = {
    { { "exact", "Match words exactly" }, exact_match },
    { { "prefix", "Match word prefixes" }, prefix_match },
};

static int
gcide_init(int argc, char **argv)
{
    int i;

    for (i = 0; i < DICO_ARRAY_SIZE(strat_tab); i++) 
	dico_strategy_add(&strat_tab[i].strat);

    return 0;
}

static matcher_t
find_matcher(const char *strat)
{
    int i;
    for (i = 0; i < DICO_ARRAY_SIZE(strat_tab); i++) 
	if (strcmp(strat, strat_tab[i].strat.name) == 0)
	    return strat_tab[i].matcher;
    return NULL;
}

static int
compare_ref(const void *a, void *b)
{
    struct gcide_ref const *aref = a;
    struct gcide_ref const *bref = b;

    return utf8_strcasecmp(aref->ref_headword, bref->ref_headword);
}

static int
free_ref(void *a, void *b)
{
    struct gcide_ref *ref = a;
    free(ref->ref_headword);
    free(ref);
    return 0;
}

static dico_list_t
gcide_create_result_list(int unique)
{
    dico_list_t list;
    
    list = dico_list_create();
    if (!list) {
	dico_log(L_ERR, ENOMEM, "gcide_create_result_list");
	return NULL;
    }
    if (unique) {
	dico_list_set_comparator(list,
				 (int (*)(const void *, void *)) compare_ref);
	dico_list_set_flags(list, DICO_LIST_COMPARE_TAIL);
    }
    dico_list_set_free_item(list, free_ref, NULL);
    return list;
}

static int
gcide_result_list_append(dico_list_t list, struct gcide_ref *ref)
{
    struct gcide_ref *copy = calloc(1,sizeof(*copy));
    if (!copy) {
	dico_log(L_ERR, errno, "gcide_result_list_append");
	return -1;
    }
    *copy = *ref;
    copy->ref_headword = strdup(ref->ref_headword);
    if (!copy->ref_headword ||
	(dico_list_append(list, copy) && errno == ENOMEM)) {
	dico_log(L_ERR, errno, "gcide_result_list_append");
	free(copy);
	return -1;
    }
    return 0;
}

struct match_closure {
    dico_strategy_t strat;
    dico_list_t list;
    struct dico_key key;
};
    
static int
match_key(struct gcide_ref *ref, void *data)
{
    struct match_closure *clos = data;

    if (dico_key_match(&clos->key, ref->ref_headword)) {
	if (gcide_result_list_append(clos->list, ref))
	    return 1;
    }
    return 0;
}

static dico_result_t
gcide_match_all(struct gcide_db *db, const dico_strategy_t strat,
		const char *word)
{
    struct gcide_result *res;
    struct match_closure clos;
    
    clos.list = gcide_create_result_list(1);
    if (!clos.list)
	return NULL;
    
    if (dico_key_init(&clos.key, strat, word)) {
	dico_log(L_ERR, 0, _("gcide_match_all: key initialization failed"));
	dico_list_destroy(&clos.list);
	return NULL;
    }
    
    clos.strat = strat;
    gcide_idx_enumerate(db->idx, match_key, &clos); 
    
    dico_key_deinit(&clos.key);

    if (dico_list_count(clos.list) == 0) {
	dico_list_destroy(&clos.list);
	return NULL;
    }
    
    res = calloc(1, sizeof(*res));
    if (!res) {
	dico_log(L_ERR, errno, "gcide_match_all");
	dico_list_destroy(&clos.list);
    } else {
	res->type = result_match;
	res->db = db;
	res->list = clos.list;
	res->compare_count = gcide_idx_defs(db->idx);
    }
    
    return (dico_result_t) res;
}

static dico_result_t
gcide_match(dico_handle_t hp, const dico_strategy_t strat, const char *word)
{
    struct gcide_db *db = (struct gcide_db *) hp;
    matcher_t matcher = find_matcher(strat->name);
    gcide_iterator_t itr;
    struct gcide_result *res = NULL;
    
    if (!matcher)
	return gcide_match_all(db, strat, word);
    itr = matcher(db, word);
    if (itr) {
	res = calloc(1, sizeof(*res));
	if (!res) {
	    dico_log(L_ERR, errno, "gcide_match");
	    gcide_iterator_free(itr);
	    return NULL;
	}

	res->type = result_match;
	res->db = db;
	res->list = gcide_create_result_list(1);
	if (!res->list) {
	    free(res);
	    gcide_iterator_free(itr);
	    return NULL;
	}

	do
	    gcide_result_list_append(res->list, gcide_iterator_ref(itr));
	while (gcide_iterator_next(itr) == 0);
	res->compare_count = gcide_iterator_compare_count(itr);
	gcide_iterator_free(itr);
    }
    return (dico_result_t) res;
}

static dico_result_t
gcide_define(dico_handle_t hp, const char *word)
{
    struct gcide_db *db = (struct gcide_db *) hp;
    gcide_iterator_t itr;
    struct gcide_result *res = NULL;
    
    itr = exact_match(db, word);
    if (itr) {
	res = calloc(1, sizeof(*res));
	if (!res) {
	    dico_log(L_ERR, errno, "gcide_define");
	    gcide_iterator_free(itr);
	    return NULL;
	}

	res->type = result_define;
	res->db = db;
	res->list = gcide_create_result_list(0);
	if (!res->list) {
	    free(res);
	    gcide_iterator_free(itr);
	    return NULL;
	}

	do
	    gcide_result_list_append(res->list, gcide_iterator_ref(itr));
	while (gcide_iterator_next(itr) == 0);
	res->compare_count = gcide_iterator_compare_count(itr);
	gcide_iterator_free(itr);
    }
    return (dico_result_t) res;
}

static struct gcide_ref *
gcide_result_ref(struct gcide_result *res)
{
    struct gcide_ref *ref = NULL;
    if (!res->itr) {
	res->itr = dico_list_iterator(res->list);
	if (!res->itr)
	    return NULL;
	ref = dico_iterator_first(res->itr);
    } else
	ref = dico_iterator_next(res->itr);
    return ref;
}

#define GOF_IGNORE 0x0001000
#define GOF_AS     0x0002000

struct output_closure {
    dico_stream_t stream;
    int flags;
    int rc;
};

static int
print_text(int end, struct gcide_tag *tag, void *data)
{
    struct output_closure *clos = data;
    static char *quote[2] = { "“", "”" };
    static char *ref[2] = { "{" , "}" };

    switch (tag->tag_type) {
    case gcide_content_unspecified:
	break;
    case gcide_content_text:
	if (clos->flags & GOF_IGNORE)
	    break;
	if (clos->flags & GOF_AS) {
	    char *s = tag->tag_v.text;
	    
	    if (strncmp(s, "as", 2) == 0 &&
		(isspace(s[3]) || ispunct(s[3]))) {
		
		dico_stream_write(clos->stream, s, 3);
		for (s += 3; *s && isspace(*s); s++)
		    dico_stream_write(clos->stream, s, 1);
		dico_stream_write(clos->stream, quote[0], strlen(quote[0]));
		dico_stream_write(clos->stream, s, strlen(s));
	    } else
		dico_stream_write(clos->stream, quote[0], strlen(quote[0]));
	} else
	    dico_stream_write(clos->stream, tag->tag_v.text,
			      strlen(tag->tag_v.text));
	break;
    case gcide_content_taglist:
	if (tag->tag_parmc) {
	    clos->flags &= ~GOF_AS;
	    if (end) {
		if (strcmp(tag->tag_name, "pr") == 0 &&
			 clos->flags & GCIDE_NOPR)
		    clos->flags &= ~GOF_IGNORE;
		else if (clos->flags & GOF_IGNORE)
		    break;
		else if (strcmp(tag->tag_name, "as") == 0)
		    dico_stream_write(clos->stream, quote[1], strlen(quote[1]));
		else if (strcmp(tag->tag_name, "er") == 0)
		    dico_stream_write(clos->stream, ref[1], strlen(ref[1]));
	    } else {
		if (strcmp(tag->tag_name, "pr") == 0 &&
			 clos->flags & GCIDE_NOPR)
		    clos->flags |= GOF_IGNORE;
		else if (clos->flags & GOF_IGNORE)
		    break;
		else if (strcmp(tag->tag_name, "sn") == 0)
		    dico_stream_write(clos->stream, "\n", 1);
		else if (strcmp(tag->tag_name, "as") == 0)
		    clos->flags |= GOF_AS;
		else if (strcmp(tag->tag_name, "er") == 0)
		    dico_stream_write(clos->stream, ref[0], strlen(ref[0]));
	    }
	}
    }
    return 0;
}


static int
output_def(dico_stream_t str, struct gcide_db *db, struct gcide_ref *ref)
{
    char *buffer;
    struct gcide_parse_tree *tree;
    int rc;
    
    if (db->file_letter != ref->ref_letter) {
	int rc;
	
	if (db->file_stream) {
	    dico_stream_close(db->file_stream);
	    dico_stream_destroy(&db->file_stream);
	    db->file_letter = 0;
	}

	*db->tmpl_letter = ref->ref_letter;

	db->file_stream =
	    dico_mapfile_stream_create(db->tmpl_name,
				       DICO_STREAM_READ|DICO_STREAM_SEEK);
	if (!db->file_stream) {
	    dico_log(L_ERR, errno, _("cannot create stream `%s'"),
		     db->tmpl_name);
	    return 1;
	}
	rc = dico_stream_open(db->file_stream);
	if (rc) {
	    dico_log(L_ERR, 0,
		     _("cannot open stream `%s': %s"),
		     db->tmpl_name, dico_stream_strerror(db->file_stream, rc));
	    dico_stream_destroy(&db->file_stream);
	    return 1;
	}
	db->file_letter = ref->ref_letter;
    }

    if (dico_stream_seek(db->file_stream, ref->ref_offset, SEEK_SET) < 0) {
	dico_log(L_ERR, errno,
		 _("seek error on `%s' while positioning to %lu: %s"),
		 db->tmpl_name, ref->ref_offset,
		 dico_stream_strerror(db->file_stream,
				      dico_stream_last_error(db->file_stream)));
	return 1;
    }

    buffer = malloc(ref->ref_size);
    if (!buffer) {
	dico_log(L_ERR, errno, "output_def");
	return 1;
    }
    
    if ((rc = dico_stream_read(db->file_stream, buffer, ref->ref_size, NULL))) {
	dico_log(L_ERR, 0, _("%s: read error: %s"),
		 db->tmpl_name,
		 dico_stream_strerror(db->file_stream, rc));
	free(buffer);
	return 1;
    }

    tree = gcide_markup_parse(buffer, ref->ref_size, db->flags & GCIDE_DBGLEX);
    if (!tree)
	rc = dico_stream_write(str, buffer, ref->ref_size);
    else {
	struct output_closure clos;
	clos.stream = str;
	clos.flags = db->flags;
	clos.rc = 0;
	gcide_parse_tree_inorder(tree, print_text, &clos);
	gcide_parse_tree_free(tree);
	rc = clos.rc;
	
    }
    free(buffer);
    return rc;
}

static int
gcide_output_result(dico_result_t rp, size_t n, dico_stream_t str)
{
    struct gcide_result *res = (struct gcide_result *) rp;
    struct gcide_ref *ref;
    
    ref = gcide_result_ref(res);
    if (!ref)
	return 1;
    switch (res->type) {
    case result_match:
	dico_stream_write(str, ref->ref_headword, ref->ref_hwbytelen - 1);
	break;

    case result_define:
	return output_def(str, res->db, ref);
    }
    return 0;
}

static size_t
gcide_result_count(dico_result_t rp)
{
    struct gcide_result *res = (struct gcide_result *) rp;
    return dico_list_count(res->list);
}

static size_t
gcide_compare_count(dico_result_t rp)
{
    struct gcide_result *res = (struct gcide_result *) rp;
    return res->compare_count;
}

static void
gcide_free_result(dico_result_t rp)
{
    struct gcide_result *res = (struct gcide_result *) rp;
    dico_iterator_destroy(&res->itr);
    dico_list_destroy(&res->list);
}

static int
gcide_result_headers(dico_result_t rp, dico_assoc_list_t hdr)
{  
    /* FIXME */
    return 0;
}

struct dico_database_module DICO_EXPORT(gcide, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NONE,
    gcide_init,
    gcide_init_db,
    gcide_free_db,
    NULL,
    NULL,
    gcide_info,
    gcide_descr,
    NULL,
    gcide_match,
    gcide_define,
    gcide_output_result,
    gcide_result_count,
    gcide_compare_count,
    gcide_free_result,
    gcide_result_headers
};
