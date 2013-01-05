/* This file is part of GNU Dico.
   Copyright (C) 1998-2000, 2008, 2010, 2012 Sergey Poznyakoff

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

/* Load path */
static int
_add_load_dir (void *item, void *unused)
{
    char *str = item;
    return lt_dladdsearchdir(str);
}

void
dicod_loader_init()
{
    lt_dlinit();
    dico_list_iterate(prepend_load_path, _add_load_dir, NULL);
    lt_dladdsearchdir(DICO_MODDIR);
    dico_list_iterate(module_load_path, _add_load_dir, NULL);
}

#define MODULE_ASSERT(cond)					\
    if (!(cond)) {						\
	lt_dlclose(handle);					\
	dico_log(L_ERR, 0, _("%s: faulty module: (%s) failed"), \
		 argv[0],					\
	         #cond);					\
	return 1;						\
    }

static int
dicod_load_module0(dicod_module_instance_t *inst, int argc, char **argv)
{
    lt_dlhandle handle = NULL;
    lt_dladvise advise = NULL;
    struct dico_database_module *pmod;    

    if (inst->handle) {
	dico_log(L_ERR, 0, _("module %s already loaded"), argv[0]);
	return 1;
    }
	
    if (!lt_dladvise_init(&advise) && !lt_dladvise_ext(&advise)
        && !lt_dladvise_global(&advise))
	handle = lt_dlopenadvise(argv[0], advise);
    lt_dladvise_destroy(&advise);

    if (!handle) {
	dico_log(L_ERR, 0, _("cannot load module %s: %s"), argv[0],
		 lt_dlerror());
	return 1;
    }
    
    pmod = (struct dico_database_module *) lt_dlsym(handle, "module");
    MODULE_ASSERT(pmod);
    MODULE_ASSERT(pmod->dico_version <= DICO_MODULE_VERSION);
    if (pmod->dico_capabilities & DICO_CAPA_NODB) {
	MODULE_ASSERT(pmod->dico_init);
    } else {
	MODULE_ASSERT(pmod->dico_init_db);
	MODULE_ASSERT(pmod->dico_free_db);
	MODULE_ASSERT(pmod->dico_match);
	MODULE_ASSERT(pmod->dico_define);
	MODULE_ASSERT(pmod->dico_output_result);
	MODULE_ASSERT(pmod->dico_result_count);
	MODULE_ASSERT(pmod->dico_free_result);
	
	if (pmod->dico_open || pmod->dico_close)
	    MODULE_ASSERT(pmod->dico_open && pmod->dico_close);
    }
    
    if (pmod->dico_init && pmod->dico_init(argc, argv)) {
	lt_dlclose(handle);
	dico_log(L_ERR, 0, _("%s: initialization failed"), argv[0]);
	return 1;
    }

    inst->handle = handle;
    inst->module = pmod;
    return 0;
}

int
dicod_load_module(dicod_module_instance_t *inst)
{
    struct wordsplit ws;
    int rc;

    if (wordsplit(inst->command, &ws, WRDSF_DEFFLAGS)) {
	dico_log(L_ERR, rc, _("cannot parse command line `%s': %s"),
		 inst->command, wordsplit_strerror (&ws));
	return 1;
    }

    rc = dicod_load_module0(inst, ws.ws_wordc, ws.ws_wordv);

    wordsplit_free(&ws);
    
    return rc;
}

int
dicod_init_database(dicod_database_t *dp)
{
    dicod_module_instance_t *inst = dp->instance;

    if (inst->module->dico_capabilities & DICO_CAPA_NODB) {
	dico_log(L_ERR, 0, _("cannot initialize database `%s': "
			     "module `%s' does not support databases"),
		 dp->command, inst->ident);
	return 1;
    }
    
    if (inst->module->dico_init_db) {
	dp->mod_handle = inst->module->dico_init_db(dp->name,
						    dp->argc, dp->argv);
	if (!dp->mod_handle) {
	    dico_log(L_ERR, 0, _("cannot initialize database `%s'"),
		     dp->command);
	    return 1;
	}
    }
    return 0;
}

int
dicod_open_database(dicod_database_t *dp)
{
    dicod_module_instance_t *inst = dp->instance;

    if (inst->module->dico_open) {
	if (inst->module->dico_open(dp->mod_handle)) {
	    dico_log(L_ERR, 0, _("cannot open database `%s'"),
		     dp->command);
	    return 1;
	}
    }
    return 0;
}

int
dicod_close_database(dicod_database_t *dp)
{
    int rc = 0;
    
    if (dp->mod_handle) {
	dicod_module_instance_t *inst = dp->instance;
	if (inst->module->dico_close) 
	    rc = inst->module->dico_close(dp->mod_handle);
    }
    return rc;
}

int
dicod_free_database(dicod_database_t *dp)
{
    int rc = 0;
    
    if (dp->mod_handle) {
	dicod_module_instance_t *inst = dp->instance;
	if (inst->module->dico_free_db) {
	    rc = inst->module->dico_free_db(dp->mod_handle);
	    dp->mod_handle = NULL;
	}
    }
    return rc;
}

char *
dicod_get_database_descr(dicod_database_t *db)
{
    if (db->descr)
	return db->descr;
    else {
	dicod_module_instance_t *inst = db->instance;
	if (inst->module->dico_db_descr)
	    return inst->module->dico_db_descr(db->mod_handle);
    }
    return NULL;
}

void
dicod_free_database_descr(dicod_database_t *db, char *descr)
{
    if (descr && descr != db->descr)
	free(descr);
}

char *
dicod_get_database_info(dicod_database_t *db)
{
    if (db->info)
	return db->info;
    else {
	dicod_module_instance_t *inst = db->instance;
	if (inst->module->dico_db_info)
	    return inst->module->dico_db_info(db->mod_handle);
    }
    return NULL;
}

void
dicod_free_database_info(dicod_database_t *db, char *info)
{
    if (info && info != db->info)
	free(info);
}

static int
_dup_lang_item(void *item, void *data)
{
    char *lang = item;
    dico_list_t dst = data;
    xdico_list_append(dst, xstrdup(lang));
    return 0;
}

dico_list_t
dicod_langlist_copy(dico_list_t src)
{
    dico_list_t dst = xdico_list_create();
    dico_list_set_free_item(dst, dicod_free_item, NULL);
    dico_list_iterate(src, _dup_lang_item, dst);
    return dst;
}

int
dicod_any_lang_list_p(dico_list_t list)
{
    return list == NULL
	   || dico_list_count(list) ==  0
	   || (dico_list_count(list) == 1
	       && strcmp (dico_list_item(list, 0), "*") == 0);
}

void
dicod_get_database_languages(dicod_database_t *db, dico_list_t dlist[])
{
    if (!(db->flags & DICOD_DBF_LANG)) {
	dicod_module_instance_t *inst = db->instance;
	if (inst->module->dico_db_lang) {
	    /* FIXME: Return code? */
	    inst->module->dico_db_lang(db->mod_handle, db->langlist);
	    if (db->langlist[0] || db->langlist[1]) {		
		if (!db->langlist[0])
		    db->langlist[0] = dicod_langlist_copy(db->langlist[1]);
		else if (!db->langlist[1])
		    db->langlist[1] = dicod_langlist_copy(db->langlist[0]);
	    }
	    if (dicod_any_lang_list_p(db->langlist[0]))
		dico_list_destroy(&db->langlist[0]);
	    if (dicod_any_lang_list_p(db->langlist[1]))
		dico_list_destroy(&db->langlist[1]);
	}
	db->flags |= DICOD_DBF_LANG;
    }
    dlist[0] = db->langlist[0];
    dlist[1] = db->langlist[1];
}


static char nomatch[] = "552 No match";
static size_t nomatch_len = (sizeof(nomatch)-1);


typedef void (*outproc_t)(dicod_database_t *db, dico_result_t res,
			  const char *word, dico_stream_t stream,
			  void *data,
			  size_t count);

void
dicod_word_first(dico_stream_t stream, const char *word,
		 const dico_strategy_t strat,
		 const char *begfmt, const char *endmsg,
		 outproc_t proc, void *data, const char *tid)
{
    dicod_database_t *db;
    dico_iterator_t itr;

    begin_timing(tid);

    if (strat && stratcl_check_word(strat->stratcl, word)) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
	return;
    }

    itr = xdico_list_iterator(database_list);
    for (db = dico_iterator_first(itr); db; db = dico_iterator_next(itr)) {
	if (database_visible_p(db)) {
	    struct dico_database_module *mp = db->instance->module;
	    dico_result_t res = strat ?
		            mp->dico_match(db->mod_handle, strat, word) :
		            mp->dico_define(db->mod_handle, word);
	    size_t count;
	    
	    if (!res)
		continue;
	    count = mp->dico_result_count(res);
	    
	    if (count) {
		if (strat)
		    current_stat.matches = count;
		else
		    current_stat.defines = count;
		if (mp->dico_compare_count)
		    current_stat.compares = mp->dico_compare_count(res);
		stream_printf(stream, begfmt, (unsigned long) count);
		proc(db, res, word, stream, data, count);
		stream_writez(stream, (char*) endmsg);
		report_current_timing(stream, tid);
		dico_stream_write(stream, "\n", 1);
		access_log_status(begfmt, endmsg);
		mp->dico_free_result(res);
		break;
	    } else
		mp->dico_free_result(res);
	}
    }
    dico_iterator_destroy(&itr);
    if (!db) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
    }
}

struct dbres {
    dicod_database_t *db;
    dico_result_t res;
    size_t count;
};

void
dicod_word_all(dico_stream_t stream, const char *word,
	       const dico_strategy_t strat,
	       const char *begfmt, const char *endmsg,
	       outproc_t proc, void *data, const char *tid)
{
    dicod_database_t *db;
    dico_iterator_t itr;
    dico_list_t reslist = xdico_list_create();
    size_t total = 0;
    struct dbres *rp;
    
    begin_timing(tid);

    if (strat && stratcl_check_word(strat->stratcl, word)) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
	return;
    }

    itr = xdico_list_iterator(database_list);
    for (db = dico_iterator_first(itr); db; db = dico_iterator_next(itr)) {
	if (database_visible_p(db)) {
	    struct dico_database_module *mp = db->instance->module;
	    dico_result_t res = strat ?
		              mp->dico_match(db->mod_handle, strat, word) :
		              mp->dico_define(db->mod_handle, word);
	    size_t count;
	
	    if (!res)
		continue;
	    count = mp->dico_result_count(res);
	    if (!count) {
		mp->dico_free_result(res);
		continue;
	    }

	    total += count;
	    if (mp->dico_compare_count)
		current_stat.compares += mp->dico_compare_count(res);
	    rp = xmalloc(sizeof(*rp));
	    rp->db = db;
	    rp->res = res;
	    rp->count = count;
	    xdico_list_append(reslist, rp);
	}
    }

    dico_iterator_destroy(&itr);

    if (total == 0) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
    } else {
	itr = xdico_list_iterator(reslist);
	
	if (strat)
	    current_stat.matches = total;
	else
	    current_stat.defines = total;
	stream_printf(stream, begfmt, (unsigned long) total);
	for (rp = dico_iterator_first(itr); rp; rp = dico_iterator_next(itr)) {
	    proc(rp->db, rp->res, word, stream, data, rp->count);
	    rp->db->instance->module->dico_free_result(rp->res);
	    free(rp);
	}
	stream_writez(stream, (char*) endmsg);
	report_current_timing(stream, tid);
	dico_stream_write(stream, "\n", 1);
	dico_iterator_destroy(&itr);
	access_log_status(begfmt, endmsg);
    }
    dico_list_destroy(&reslist);
}

static void
print_matches(dicod_database_t *db, dico_result_t res,
	      const char *word,
	      dico_stream_t stream, void *data, size_t count)
{
    size_t i;
    struct dico_database_module *mp = db->instance->module;
    dico_stream_t ostr = data;
    
    for (i = 0; i < count; i++) {
	stream_writez(ostr, db->name);
	dico_stream_write(ostr, " \"", 2);
	mp->dico_output_result(res, i, ostr);
	dico_stream_write(ostr, "\"\n", 2);
    }
}

void
dicod_match_word_db(dicod_database_t *db, dico_stream_t stream,
		    const dico_strategy_t strat, const char *word)
{
    struct dico_database_module *mp = db->instance->module;
    dico_result_t res;
    size_t count;
    
    begin_timing("match");
    res = mp->dico_match(db->mod_handle, strat, word);
    
    if (!res) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
	return;
    }

    count = mp->dico_result_count(res);
    if (count == 0) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
    } else {
	dico_stream_t ostr;
	
	current_stat.matches = count;
	if (mp->dico_compare_count)
	    current_stat.compares = mp->dico_compare_count(res);
	stream_printf(stream, "152 %lu matches found: list follows\n",
		      (unsigned long) count);
	ostr = dicod_ostream_create(stream, NULL);
	print_matches(db, res, word, stream, ostr, count);
	total_bytes_out += dico_stream_bytes_out(ostr);
	dico_stream_close(ostr);
	dico_stream_destroy(&ostr);
	dico_stream_write(stream, ".\n", 2);
	stream_writez(stream, "250 Command complete");
	report_current_timing(stream, "match");
	dico_stream_write(stream, "\n", 1);
	access_log_status("152", "250");
    }
    
    mp->dico_free_result(res);
}

void
dicod_match_word_first(dico_stream_t stream,
		       const dico_strategy_t strat, const char *word)
{
    dico_stream_t ostr = dicod_ostream_create(stream, NULL);
    dicod_word_first(stream, word, strat,
		     "152 %lu matches found: list follows\n",
		     ".\n250 Command complete",
		     print_matches, ostr, "match");
    total_bytes_out += dico_stream_bytes_out(ostr);
    dico_stream_close(ostr);
    dico_stream_destroy(&ostr);
}

void
dicod_match_word_all(dico_stream_t stream,
		     const dico_strategy_t strat, const char *word)
{
    dico_stream_t ostr = dicod_ostream_create(stream, NULL);
    dicod_word_all(stream, word, strat,
		   "152 %lu matches found: list follows\n",
		   ".\n250 Command complete",
		   print_matches, ostr, "match");
    total_bytes_out += dico_stream_bytes_out(ostr);
    dico_stream_close(ostr);
    dico_stream_destroy(&ostr);
}



static void
print_definitions(dicod_database_t *db, dico_result_t res,
		  const char *word,
		  dico_stream_t stream, void *data, size_t count)
{
    size_t i;
    char *descr = dicod_get_database_descr(db);
    struct dico_database_module *mp = db->instance->module;
    for (i = 0; i < count; i++) {
	dico_stream_t ostr;
	dico_assoc_list_t hdr = NULL;
	
	stream_printf(stream, "151 \"%s\" %s \"%s\"\n",
		      word, db->name, descr ? descr : "");
	if (mp->dico_result_headers) {
	    if (db->mime_headers)
		hdr = dico_assoc_dup(db->mime_headers);
	    else
		dico_header_parse(&hdr, NULL);
	    mp->dico_result_headers(res, hdr);
	    ostr = dicod_ostream_create(stream, hdr);
	} else
	    ostr = dicod_ostream_create(stream, db->mime_headers);
	mp->dico_output_result(res, i, ostr);
	total_bytes_out += dico_stream_bytes_out(ostr);
	dico_stream_close(ostr);
	dico_stream_destroy(&ostr);
	dico_stream_write(stream, "\n.\n", 3);
	dico_assoc_destroy(&hdr);
    }
    dicod_free_database_descr(db, descr);
}

void
dicod_define_word_db(dicod_database_t *db, dico_stream_t stream,
		     const char *word)
{
    struct dico_database_module *mp = db->instance->module;
    dico_result_t res;
    size_t count;
    
    begin_timing("define");

    res = mp->dico_define(db->mod_handle, word);
    if (!res) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
	return;
    }

    count = mp->dico_result_count(res);
    if (count == 0) {
	access_log_status(nomatch, nomatch);
	dico_stream_writeln(stream, nomatch, nomatch_len);
    } else {
	current_stat.defines = count;
	if (mp->dico_compare_count)
	    current_stat.compares = mp->dico_compare_count(res);
	stream_printf(stream, "150 %lu definitions found: list follows\n",
		      (unsigned long) count);
	print_definitions(db, res, word, stream, NULL, count);
	stream_writez(stream, "250 Command complete");
	report_current_timing(stream, "define");
	dico_stream_write(stream, "\n", 1);
	access_log_status("150", "250");
    }
    
    mp->dico_free_result(res);
}

void
dicod_define_word_first(dico_stream_t stream, const char *word)
{
    dicod_word_first(stream, word, NULL,
		     "150 %lu definitions found: list follows\n",
		     "250 Command complete",
		     print_definitions, NULL, "define");
}

void
dicod_define_word_all(dico_stream_t stream, const char *word)
{
    dicod_word_all(stream, word, NULL,
		   "150 %lu definitions found: list follows\n",
		   "250 Command complete",
		   print_definitions, NULL, "define");
}
	    
