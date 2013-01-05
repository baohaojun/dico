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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include <libguile.h>
#include <appi18n.h>
#include <wordsplit.h>

#ifndef HAVE_SCM_T_OFF
typedef off_t scm_t_off;
#endif


/* General-purpose eval handlers */

struct apply_data {
	SCM proc;
	SCM arg;
};

SCM
apply_catch_body(void *data)
{
    struct apply_data *xp = data;
    return scm_apply_0(xp->proc, xp->arg);
}

static SCM
eval_catch_handler (void *data, SCM tag, SCM throw_args)
{
    scm_handle_by_message_noexit("dico", tag, throw_args);
    longjmp(*(jmp_buf*)data, 1);
}

struct scheme_exec_data {
    SCM (*handler) (void *data);
    void *data;
    SCM result;
};

static SCM
scheme_safe_exec_body (void *data)
{
    struct scheme_exec_data *ed = data;
    ed->result = ed->handler (ed->data);
    return SCM_BOOL_F;
}

static int
guile_safe_exec(SCM (*handler) (void *data), void *data, SCM *result)
{
    jmp_buf jmp_env;
    struct scheme_exec_data ed;

    if (setjmp(jmp_env))
	return 1;
    ed.handler = handler;
    ed.data = data;
    scm_internal_lazy_catch(SCM_BOOL_T,
			    scheme_safe_exec_body, (void*)&ed,
			    eval_catch_handler, &jmp_env);
    if (result)
	*result = ed.result;
    return 0;
}

struct load_closure {
    char *filename;
    int argc;
    char **argv;
};

static SCM
load_path_handler(void *data)
{
    struct load_closure *lp = data;

    scm_set_program_arguments(lp->argc, lp->argv, lp->filename);
    scm_primitive_load_path(scm_from_locale_string(lp->filename));
    return SCM_UNDEFINED;
}

static int
guile_load(char *filename, char *args)
{
    struct load_closure lc;
    if (args) {
	struct wordsplit ws;

	if (wordsplit(args, &ws, WRDSF_DEFFLAGS)) {
	    dico_log(L_ERR, 0, "wordsplit: %s", wordsplit_strerror(&ws));
	    return 1;
	}
	lc.argc = ws.ws_wordc;
	lc.argv = ws.ws_wordv;
	wordsplit_free(&ws);
    } else {
	lc.argc = 0;
	lc.argv = NULL;
    }
    lc.filename = filename;
    return guile_safe_exec(load_path_handler, &lc, NULL);
}

static void
_add_load_path(char *path)
{
    SCM scm, path_scm;
    SCM *pscm;

    path_scm = SCM_VARIABLE_REF(scm_c_lookup("%load-path"));
    for (scm = path_scm; scm != SCM_EOL; scm = SCM_CDR(scm)) {
	SCM val = SCM_CAR(scm);
	if (scm_is_string(val)) {
	    char *s = scm_to_locale_string(val);
	    int res = strcmp(s, path);
	    free(s);
	    if (res == 0)
		return;
	}
    }

    pscm = SCM_VARIABLE_LOC(scm_c_lookup("%load-path"));
    *pscm = scm_append(scm_list_3(path_scm,
				  scm_list_1(scm_from_locale_string(path)),
				  SCM_EOL));
}

static void
memerr(const char *fname)
{
    dico_log(L_ERR, 0, _("%s: not enough memory"), fname);
}

static char *
proc_name(SCM proc)
{
    return scm_to_locale_string(
		  scm_symbol_to_string(scm_procedure_name(proc)));
}

static void
str_rettype_error(const char *name)
{
    dico_log(L_ERR, 0, _("%s: invalid return type"), name);
}

static void
rettype_error(SCM proc)
{
    char *name = proc_name(proc);
    str_rettype_error(name);
    free(name);
}

static int
guile_call_proc(SCM *result, SCM proc, SCM arglist)
{
    jmp_buf jmp_env;
    struct apply_data adata;

    if (setjmp(jmp_env)) {
	char *name = proc_name(proc);
	dico_log(L_NOTICE, 0,
		 _("procedure `%s' failed: see error output for details"),
		 name);
	free(name);
	return 1;
    }
    adata.proc = proc;
    adata.arg = arglist;
    *result = scm_internal_lazy_catch(SCM_BOOL_T,
				      apply_catch_body, &adata,
				      eval_catch_handler, &jmp_env);
    return 0;
}


scm_t_bits _guile_dico_key_tag;

static SCM
dico_new_scm_key(struct dico_key **pkey)
{
    struct dico_key *kptr;

    kptr = scm_gc_malloc (sizeof (*kptr), "Dico key");
    *pkey = kptr;
    SCM_RETURN_NEWSMOB(_guile_dico_key_tag, kptr);
}

static scm_sizet
_guile_dico_key_free(SCM message_smob)
{
    struct dico_key *kp = (struct dico_key *) SCM_CDR (message_smob);
    dico_key_deinit(kp);
    free(kp);
    return 0;
}

static int
_guile_dico_key_print(SCM message_smob, SCM port, scm_print_state *pstate)
{
    struct dico_key *kp = (struct dico_key *) SCM_CDR (message_smob);
    scm_puts("#<key ", port);
    scm_puts(kp->strat->name, port);
    scm_puts(" (", port);
    scm_puts(kp->word, port);
    scm_puts(")>", port);
    return 1;
}

static void
_guile_init_dico_key()
{
    _guile_dico_key_tag =
	scm_make_smob_type("Dico key", sizeof (struct dico_key));
    scm_set_smob_free(_guile_dico_key_tag, _guile_dico_key_free);
    scm_set_smob_print(_guile_dico_key_tag, _guile_dico_key_print);
}

#define CELL_IS_KEY(s) \
    (!SCM_IMP(s) && SCM_CELL_TYPE(s) == _guile_dico_key_tag)

SCM_DEFINE_PUBLIC(scm_dico_key_p, "dico-key?",
		  1, 0, 0,
		  (SCM obj),
		  "Return @samp{#t} if @var{obj} is a selection key.")
#define FUNC_NAME s_scm_dico_key_p
{
    return CELL_IS_KEY(obj) ? SCM_BOOL_T : SCM_BOOL_F;
}
#undef FUNC_NAME

SCM_DEFINE_PUBLIC(scm_dico_key__word, "dico-key->word",
		  1, 0, 0,
		  (SCM key),
		  "Return search word from the @var{key}.")
#define FUNC_NAME s_scm_dico_key__word
{
    struct dico_key *kp;
    SCM_ASSERT(CELL_IS_KEY(key), key, SCM_ARG1, FUNC_NAME);
    kp = (struct dico_key *) SCM_CDR(key);
    return scm_from_locale_string(kp->word);
}
#undef FUNC_NAME


scm_t_bits _guile_strategy_tag;

struct _guile_strategy
{
    dico_strategy_t strat;
};

static SCM
_make_strategy(const dico_strategy_t strat)
{
    struct _guile_strategy *sp;

    sp = scm_gc_malloc (sizeof (struct _guile_strategy), "strategy");
    sp->strat = strat;
    SCM_RETURN_NEWSMOB(_guile_strategy_tag, sp);
}

static scm_sizet
_guile_strategy_free(SCM message_smob)
{
    struct _guile_strategy *sp =
	(struct _guile_strategy *) SCM_CDR (message_smob);
    free(sp);
    return 0;
}

static int
_guile_strategy_print(SCM message_smob, SCM port, scm_print_state * pstate)
{
    struct _guile_strategy *sp =
	(struct _guile_strategy *) SCM_CDR (message_smob);
    scm_puts("#<strategy ", port);
    scm_puts(sp->strat->name, port);
    scm_puts(" [", port);
    scm_puts(sp->strat->descr, port);
    scm_puts("]>", port);
    return 1;
}

static void
_guile_init_strategy()
{
    _guile_strategy_tag = scm_make_smob_type("strategy",
					     sizeof (struct _guile_strategy));
    scm_set_smob_free(_guile_strategy_tag, _guile_strategy_free);
    scm_set_smob_print(_guile_strategy_tag, _guile_strategy_print);
}

#define CELL_IS_STRAT(s) \
    (!SCM_IMP(s) && SCM_CELL_TYPE(s) == _guile_strategy_tag)

SCM_DEFINE_PUBLIC(scm_dico_strat_selector_p, "dico-strat-selector?", 1, 0, 0,
		  (SCM strat),
		  "Return true if @var{strat} has a selector.")
#define FUNC_NAME s_scm_dico_strat_selector_p
{
    struct _guile_strategy *sp;

    SCM_ASSERT(CELL_IS_STRAT(strat), strat, SCM_ARG1, FUNC_NAME);
    sp = (struct _guile_strategy *) SCM_CDR(strat);
    return sp->strat->sel ? SCM_BOOL_T : SCM_BOOL_F;
}
#undef FUNC_NAME

SCM_DEFINE_PUBLIC(scm_dico_strat_select_p, "dico-strat-select?", 3, 0, 0,
		  (SCM strat, SCM word, SCM key),
		  "Return true if @var{key} matches @var{word} as per strategy selector @var{strat}.")
#define FUNC_NAME s_scm_dico_strat_select_p
{
    struct _guile_strategy *sp;
    struct dico_strategy *stratp;
    char *wordstr;
    int rc;

    SCM_ASSERT(CELL_IS_STRAT(strat), strat, SCM_ARG1, FUNC_NAME);
    SCM_ASSERT(scm_is_string(word), word, SCM_ARG2, FUNC_NAME);

    sp = (struct _guile_strategy *) SCM_CDR(strat);
    stratp = sp->strat;
    
    if (scm_is_string(key)) {
	char *keystr = scm_to_locale_string(key);
	struct dico_key skey;

	if (dico_key_init(&skey, stratp, wordstr)) {
	    free(keystr);
	    scm_misc_error(FUNC_NAME,
			   "key initialization failed: ~S",
			   scm_list_1(key));
	}
	rc = dico_key_match(&skey, wordstr);
	dico_key_deinit(&skey);
	free(wordstr);
	free(keystr);
    } else {
	struct dico_key *kptr;

	SCM_ASSERT(CELL_IS_KEY(key), key, SCM_ARG3, FUNC_NAME);
    
	kptr = (struct dico_key *) SCM_CDR(key);
	wordstr = scm_to_locale_string(word);
	rc = dico_key_match(kptr, wordstr);
	free(wordstr);
    }
    return rc ? SCM_BOOL_T : SCM_BOOL_F;
}
#undef FUNC_NAME

SCM_DEFINE_PUBLIC(scm_dico_strat_name, "dico-strat-name", 1, 0, 0,
		  (SCM strat),
		  "Return the name of the strategy @var{strat}.")
#define FUNC_NAME s_scm_dico_strat_name
{
    struct _guile_strategy *sp;

    SCM_ASSERT(CELL_IS_STRAT(strat), strat, SCM_ARG1, FUNC_NAME);
    sp = (struct _guile_strategy *) SCM_CDR(strat);
    return scm_from_locale_string(sp->strat->name);
}
#undef FUNC_NAME

SCM_DEFINE_PUBLIC(scm_dico_strat_description, "dico-strat-description",
		  1, 0, 0,
		  (SCM strat),
		  "Return a textual description of the strategy @var{strat}.")
#define FUNC_NAME s_scm_dico_strat_description
{
    struct _guile_strategy *sp;

    SCM_ASSERT(CELL_IS_STRAT(strat), strat, SCM_ARG1, FUNC_NAME);
    sp = (struct _guile_strategy *) SCM_CDR(strat);
    return scm_from_locale_string(sp->strat->descr);
}
#undef FUNC_NAME

SCM_DEFINE_PUBLIC(scm_dico_strat_default_p, "dico-strat-default?", 1, 0, 0,
		  (SCM strat),
		  "Return true if @var{strat} is a default strategy.")
#define FUNC_NAME s_scm_dico_strat_default_p
{
    struct _guile_strategy *sp;

    SCM_ASSERT(CELL_IS_STRAT(strat), strat, SCM_ARG1, FUNC_NAME);
    sp = (struct _guile_strategy *) SCM_CDR(strat);
    return dico_strategy_is_default_p(sp->strat) ? SCM_BOOL_T : SCM_BOOL_F;
}
#undef FUNC_NAME


SCM_DEFINE_PUBLIC(scm_dico_make_key, "dico-make-key",
		  2, 0, 0,
		  (SCM strat, SCM word),
		  "Make a key for given @var{word} and strategy @var{strat}.")
#define FUNC_NAME s_scm_dico_make_key
{
    SCM ret;
    struct dico_key *key;
    struct _guile_strategy *sp;
    char *wordstr;
    int rc;
    
    SCM_ASSERT(CELL_IS_STRAT(strat), strat, SCM_ARG1, FUNC_NAME);
    SCM_ASSERT(scm_is_string(word), word, SCM_ARG2, FUNC_NAME);
    sp = (struct _guile_strategy *) SCM_CDR(strat);
    wordstr = scm_to_locale_string(word);
    ret = dico_new_scm_key(&key);
    rc = dico_key_init(key, sp->strat, wordstr);
    free(wordstr);
    if (rc)
	scm_misc_error(FUNC_NAME,
		       "key initialization failed: ~S",
		       scm_list_1(ret));
    return ret;
}
#undef FUNC_NAME


static int
_guile_selector(int cmd, struct dico_key *key, const char *dict_word)
{
    SCM result;
    SCM list = scm_list_4((SCM)key->strat->closure,
			  scm_from_int (cmd),
			  scm_from_locale_string(key->word),
			  scm_from_locale_string(dict_word));
    if (guile_safe_exec(apply_catch_body, list, &result))
	return 0;
    return result != SCM_BOOL_F;
}

SCM_DEFINE_PUBLIC(scm_dico_register_strat, "dico-register-strat", 2, 1, 0,
		  (SCM strat, SCM descr, SCM fun),
		  "Register a new strategy.")
#define FUNC_NAME s_scm_dico_register_strat
{
    struct dico_strategy strategy;

    SCM_ASSERT(scm_is_string(strat), strat, SCM_ARG1, FUNC_NAME);
    SCM_ASSERT(scm_is_string(descr), descr, SCM_ARG2, FUNC_NAME);

    if (!SCM_UNBNDP(fun))
	SCM_ASSERT(scm_procedure_p(fun), fun, SCM_ARG3, FUNC_NAME);

    strategy.name = scm_to_locale_string(strat);
    strategy.descr = scm_to_locale_string(descr);
    if (SCM_UNBNDP(fun)) {
	strategy.sel = NULL;
	strategy.closure = NULL;
    } else {
	strategy.sel = _guile_selector;
	strategy.closure = fun;
    }
    dico_strategy_add(&strategy);
    free(strategy.name);
    free(strategy.descr);
    return SCM_UNSPECIFIED;
}
#undef FUNC_NAME


SCM_DEFINE_PUBLIC(scm_dico_register_markup, "dico-register-markup", 1, 0, 0,
		  (SCM type),
		  "Register new markup type.")
#define FUNC_NAME s_scm_dico_register_markup
{
    int rc;
    char *str;
    SCM_ASSERT(scm_is_string(type), type, SCM_ARG1, FUNC_NAME);
    str = scm_to_locale_string(type);
    rc = dico_markup_register(str);
    free(str);
    switch (rc) {
    case 0:
	break;

    case ENOMEM:
	scm_memory_error(FUNC_NAME);

    case EINVAL:
	scm_misc_error(FUNC_NAME,
		       "Invalid markup name: ~S",
		       scm_list_1(type));

    default:
	scm_misc_error(FUNC_NAME,
		       "Unexpected error: ~S",
		       scm_list_1(scm_from_int(rc)));
    }
    return SCM_UNSPECIFIED;
}
#undef FUNC_NAME

SCM_DEFINE_PUBLIC(scm_dico_current_markup, "dico-current-markup", 0, 0, 0,
		  (),
		  "Return current dico markup type.")
#define FUNC_NAME s_scm_dico_current_markup
{
    return scm_from_locale_string(dico_markup_type);
}
#undef FUNC_NAME


static scm_t_bits scm_tc16_dico_port;
struct _guile_dico_port {
    dico_stream_t str;
};

static SCM
_make_dico_port(dico_stream_t str)
{
    struct _guile_dico_port *dp;
    SCM port;
    scm_port *pt;

    dp = scm_gc_malloc (sizeof (struct _guile_dico_port), "dico-port");
    dp->str = str;

    port = scm_new_port_table_entry(scm_tc16_dico_port);
    pt = SCM_PTAB_ENTRY(port);
    pt->rw_random = 0;
    SCM_SET_CELL_TYPE(port,
		      (scm_tc16_dico_port | SCM_OPN | SCM_WRTNG | SCM_BUF0));
    SCM_SETSTREAM(port, dp);
    return port;
}

#define DICO_PORT(x) ((struct _guile_dico_port *) SCM_STREAM (x))

static SCM
_dico_port_mark(SCM port)
{
    return SCM_BOOL_F;
}

static void
_dico_port_flush(SCM port)
{
    struct _guile_dico_port *dp = DICO_PORT(port);
    if (dp && dp->str)
	dico_stream_flush(dp->str);
}

static int
_dico_port_close(SCM port)
{
    struct _guile_dico_port *dp = DICO_PORT(port);

    if (dp) {
	_dico_port_flush(port);
	SCM_SETSTREAM(port, NULL);
	scm_gc_free(dp, sizeof(struct _guile_dico_port), "dico-port");
    }
    return 0;
}

static scm_sizet
_dico_port_free(SCM port)
{
    _dico_port_close(port);
    return 0;
}

static int
_dico_port_fill_input(SCM port)
{
    return EOF;
}

static void
_dico_port_write(SCM port, const void *data, size_t size)
{
    struct _guile_dico_port *dp = DICO_PORT(port);
    dico_stream_write(dp->str, data, size);
}

static scm_t_off
_dico_port_seek (SCM port, scm_t_off offset, int whence)
{
    struct _guile_dico_port *dp = DICO_PORT(port);
    return (scm_t_off) dico_stream_seek(dp->str, (off_t) offset, whence);
}

static int
_dico_port_print(SCM exp, SCM port, scm_print_state *pstate)
{
    scm_puts ("#<Dico port>", port);
    return 1;
}

static void
_guile_init_dico_port()
{
    scm_tc16_dico_port = scm_make_port_type("dico-port",
					    _dico_port_fill_input,
					    _dico_port_write);
    scm_set_port_mark (scm_tc16_dico_port, _dico_port_mark);
    scm_set_port_free (scm_tc16_dico_port, _dico_port_free);
    scm_set_port_print (scm_tc16_dico_port, _dico_port_print);
    scm_set_port_flush (scm_tc16_dico_port, _dico_port_flush);
    scm_set_port_close (scm_tc16_dico_port, _dico_port_close);
    scm_set_port_seek (scm_tc16_dico_port, _dico_port_seek);
}



static scm_t_bits scm_tc16_dico_log_port;

static SCM
_make_dico_log_port(int level)
{
    dico_stream_t str = dico_log_stream_create(level);
    return str ? _make_dico_port(str) : SCM_BOOL_F;
}

static int
_dico_log_port_print(SCM exp, SCM port, scm_print_state *pstate)
{
    scm_puts ("#<Dico log port>", port);
    return 1;
}

static void
_guile_init_dico_log_port()
{
    scm_tc16_dico_log_port = scm_make_port_type("dico-log-port",
						_dico_port_fill_input,
						_dico_port_write);
    scm_set_port_mark (scm_tc16_dico_log_port, _dico_port_mark);
    scm_set_port_free (scm_tc16_dico_log_port, _dico_port_free);
    scm_set_port_print (scm_tc16_dico_log_port, _dico_log_port_print);
    scm_set_port_flush (scm_tc16_dico_log_port, _dico_port_flush);
    scm_set_port_close (scm_tc16_dico_log_port, _dico_port_close);
    scm_set_port_seek (scm_tc16_dico_log_port, _dico_port_seek);
}


static void
_guile_init_funcs (void)
{
#include <guile.x>
}


static int guile_debug;

static char *guile_init_script;
static char *guile_init_args;
static char *guile_init_fun;

enum guile_proc_ind {
    open_proc,
    close_proc,
    info_proc,
    descr_proc,
    lang_proc,
    match_proc,
    define_proc,
    output_proc,
    result_count_proc,
    compare_count_proc,
    free_result_proc,
    result_headers_proc,
};

#define MAX_PROC (result_headers_proc+1)

static char *guile_proc_name[] = {
    "open",
    "close",
    "info",
    "descr",
    "lang",
    "match",
    "define",
    "output",
    "result-count",
    "compare-count",
    "free-result",
    "result-headers"
};

typedef SCM guile_vtab[MAX_PROC];

static guile_vtab global_vtab;

struct _guile_database {
    const char *dbname;
    guile_vtab vtab;
    int argc;
    char **argv;
    SCM handle;
};

static int
proc_name_to_index(const char *name)
{
    int i;
    for (i = 0; i < MAX_PROC; i++)
	if (strcmp(guile_proc_name[i], name) == 0)
	    break;
    return i;
}

struct init_struct {
    const char *init_fun;
    const char *db_name;
};

static SCM
call_init_handler(void *data)
{
    struct init_struct *p = (struct init_struct *)data;
    SCM procsym = SCM_VARIABLE_REF(scm_c_lookup(p->init_fun));
    SCM arg;
    if (p->db_name)
	arg = scm_from_locale_string(p->db_name);
    else
	arg = SCM_BOOL_F;
    return scm_apply_0(procsym, scm_list_1(arg));
}

static int
init_vtab(const char *init_fun, const char *dbname, guile_vtab vtab)
{
    SCM res;
    struct init_struct istr;

    istr.init_fun = init_fun;
    istr.db_name = dbname;
    if (guile_safe_exec(call_init_handler, &istr, &res))
	return 1;

    if (!scm_is_pair(res) && res != SCM_EOL) {
	str_rettype_error(init_fun);
	return 1;
    }
    for (; res != SCM_EOL; res = SCM_CDR(res)) {
	int idx;
	char *ident;
	SCM name, proc;
	SCM car = SCM_CAR(res);
	if (!scm_is_pair(res)
	    || !scm_is_string(name = SCM_CAR(car))
	    || !scm_procedure_p(proc = SCM_CDR(car)))  {
	    str_rettype_error(init_fun);
	    return 1;
	}
	ident = scm_to_locale_string(name);
	idx = proc_name_to_index(ident);
	if (idx == MAX_PROC) {
	    dico_log(L_ERR, 0, _("%s: %s: unknown virtual function"),
		     init_fun, ident);
	    free(ident);
	    return 1;
	}
	free(ident);
	vtab[idx] = proc;
    }
    return 0;
}

static int
set_load_path(struct dico_option *opt, const char *val)
{
    char *p;
    char *tmp = strdup(val);
    if (!tmp)
	return 1;
    for (p = strtok(tmp, ":"); p; p = strtok(NULL, ":"))
	_add_load_path(p);
    free(tmp);
    return 0;
}

static struct dico_option init_option[] = {
    { DICO_OPTSTR(debug), dico_opt_bool, &guile_debug },
    { DICO_OPTSTR(init-script), dico_opt_string, &guile_init_script },
    { DICO_OPTSTR(init-args), dico_opt_string, &guile_init_args },
    { DICO_OPTSTR(load-path), dico_opt_null, NULL, { 0 }, set_load_path },
    { DICO_OPTSTR(init-fun), dico_opt_string, &guile_init_fun },
    { NULL }
};

static int
mod_init(int argc, char **argv)
{
    SCM port;

    scm_init_guile();
    scm_load_goops();

    if (dico_parseopt(init_option, argc, argv, 0, NULL))
	return 1;

    _guile_init_strategy();
    _guile_init_dico_key();
    _guile_init_dico_port();
    _guile_init_dico_log_port();
    _guile_init_funcs();
#ifdef GUILE_DEBUG_MACROS
    if (guile_debug) {
	SCM_DEVAL_P = 1;
	SCM_BACKTRACE_P = 1;
	SCM_RECORD_POSITIONS_P = 1;
	SCM_RESET_DEBUG_MODE;
    }
#endif
    port = _make_dico_log_port(L_ERR);
    if (port == SCM_BOOL_F) {
	dico_log(L_ERR, 0, _("mod_init: cannot initialize error port"));
	return 1;
    }
    scm_set_current_output_port(port);
    scm_set_current_error_port(port);

    if (guile_init_script
	&& guile_load(guile_init_script, guile_init_args)) {
	dico_log(L_ERR, 0, _("mod_init: cannot load init script %s"),
		 guile_init_script);
	return 1;
    }

    if (guile_init_fun && init_vtab(guile_init_fun, NULL, global_vtab))
	return 1;

    return 0;
}

static dico_handle_t
mod_init_db(const char *dbname, int argc, char **argv)
{
    struct _guile_database *db;
    int i;
    int err = 0;
    char *init_script = NULL;
    char *init_args = NULL;
    char *init_fun = guile_init_fun;

    struct dico_option db_option[] = {
	{ DICO_OPTSTR(init-script), dico_opt_string, &init_script },
	{ DICO_OPTSTR(init-args), dico_opt_string, &init_args },
	{ DICO_OPTSTR(init-fun), dico_opt_string, &init_fun },
	{ NULL }
    };

    if (dico_parseopt(db_option, argc, argv, DICO_PARSEOPT_PERMUTE, &i))
	return NULL;
    argc -= i;
    argv += i;

    if (init_script && guile_load(init_script, init_args)) {
	dico_log(L_ERR, 0, _("mod_init: cannot load init script %s"),
		 init_script);
	return NULL;
    }

    db = malloc(sizeof(*db));
    if (!db) {
	memerr("mod_init_db");
	return NULL;
    }
    db->dbname = dbname;
    memcpy(db->vtab, global_vtab, sizeof(db->vtab));
    if (init_fun && init_vtab(init_fun, dbname, db->vtab)) {
	free(db);
	return NULL;
    }

    for (i = 0; i < MAX_PROC; i++) {
	if (!db->vtab[i]) {
	    switch (i) {
	    case open_proc:
	    case match_proc:
	    case define_proc:
	    case output_proc:
	    case result_count_proc:
		dico_log(L_ERR, 0,
			 _("%s: %s: void virtual function"),
			 argv[0], guile_proc_name[i]);
		err++;
	    default:
		break;
	    }
	}
    }

    if (err) {
	free(db);
	return NULL;
    }

    db->argc = argc;
    db->argv = argv;
    return (dico_handle_t)db;
}

static int
mod_free_db(dico_handle_t hp)
{
    struct _guile_database *db = (struct _guile_database *)hp;
    free(db);
    return 0;
}

static int
mod_close(dico_handle_t hp)
{
    struct _guile_database *db = (struct _guile_database *)hp;
    SCM res;

    if (db->vtab[close_proc])
	if (guile_call_proc(&res, db->vtab[close_proc],
			    scm_list_1(db->handle)))
	    return 1;
    scm_gc_unprotect_object(db->handle);

    return 0;
}

static SCM
argv_to_scm(int argc, char **argv)
{
    SCM scm_first = SCM_EOL, scm_last;

    for (; argc; argc--, argv++) {
	SCM new = scm_cons(scm_from_locale_string(*argv), SCM_EOL);
	if (scm_first == SCM_EOL)
	    scm_last = scm_first = new;
	else {
	    SCM_SETCDR(scm_last, new);
	    scm_last = new;
	}
    }
    return scm_first;
}

static SCM
assoc_to_scm(dico_assoc_list_t assoc)
{
    SCM scm_first = SCM_EOL, scm_last;
    dico_iterator_t itr;
    struct dico_assoc *p;

    itr = dico_assoc_iterator(assoc);
    for (p = dico_iterator_first(itr); p; p = dico_iterator_next(itr)) {
	SCM new = scm_cons(scm_cons(scm_from_locale_string(p->key),
				    scm_from_locale_string(p->value)),
			   SCM_EOL);
	if (scm_first == SCM_EOL)
	    scm_last = scm_first = new;
	else {
	    SCM_SETCDR(scm_last, new);
	    scm_last = new;
	}
    }
    dico_iterator_destroy(&itr);
    return scm_first;
}

static void
scm_to_assoc(dico_assoc_list_t assoc, SCM scm)
{
    dico_assoc_clear(assoc);

    for (; scm != SCM_EOL && scm_is_pair(scm); scm = SCM_CDR(scm)) {
	SCM elt = SCM_CAR(scm);

	if (!scm_is_pair(elt)) {
	    scm_misc_error(NULL, "Wrong element type: ~S", scm_list_1(elt));
	}
	dico_assoc_append(assoc, scm_to_locale_string(SCM_CAR(elt)),
			  scm_to_locale_string(SCM_CDR(elt)));
    }
}


static int
mod_open(dico_handle_t dp)
{
    struct _guile_database *db = (struct _guile_database *)dp;
    if (guile_call_proc(&db->handle, db->vtab[open_proc],
			scm_cons(scm_from_locale_string(db->dbname),
				 argv_to_scm(db->argc, db->argv))))
	return 1;
    if (db->handle == SCM_EOL || db->handle == SCM_BOOL_F)
	return 1;
    scm_gc_protect_object(db->handle);
    return 0;
}

static char *
mod_get_text(struct _guile_database *db, int n)
{
    if (db->vtab[n]) {
	SCM res;

	if (guile_call_proc(&res, db->vtab[n], scm_list_1(db->handle)))
	    return NULL;
	if (scm_is_string(res))
	    return scm_to_locale_string(res);
	else {
	    rettype_error(db->vtab[n]);
	    return NULL;
	}
    }
    return NULL;
}

static char *
mod_info(dico_handle_t hp)
{
    struct _guile_database *db = (struct _guile_database *)hp;
    return mod_get_text(db, info_proc);
}

static char *
mod_descr(dico_handle_t hp)
{
    struct _guile_database *db = (struct _guile_database *)hp;
    return mod_get_text(db, descr_proc);
}

static dico_list_t
scm_to_langlist(SCM scm, SCM procsym)
{
    dico_list_t list = NULL;

    if (scm == SCM_EOL)
	return NULL;
    else if (scm_is_string(scm)) {
	list = dico_list_create();
	dico_list_append(list, scm_to_locale_string(scm));
    } else if (scm_is_pair(scm)) {
	list = dico_list_create();
	for (; scm != SCM_EOL && scm_is_pair(scm); scm = SCM_CDR(scm))
	    dico_list_append(list, scm_to_locale_string(SCM_CAR(scm)));
    } else
	rettype_error(procsym);
    return list;
}

static int
mod_lang(dico_handle_t hp, dico_list_t list[2])
{
    struct _guile_database *db = (struct _guile_database *)hp;
    SCM proc = db->vtab[lang_proc];
    list[0] = list[1] = NULL;
    if (proc) {
	SCM res;

	if (guile_call_proc(&res, proc, scm_list_1(db->handle)))
	    return 1;
	if (res == SCM_EOL)
	    /* ok, nothing */;
	else if (scm_is_string(res)) {
	    list[0] = dico_list_create();
	    dico_list_append(list[0], scm_to_locale_string(res));
	} else if (scm_is_pair(res)) {
	    list[0] = scm_to_langlist(SCM_CAR(res), proc);
	    list[1] = scm_to_langlist(SCM_CDR(res), proc);
	} else {
	    rettype_error(proc);
	    return 1;
	}
    }

    return 0;
}



struct guile_result {
    struct _guile_database *db;
    SCM result;
};

static dico_result_t
make_guile_result(struct _guile_database *db, SCM res)
{
    struct guile_result *rp = malloc(sizeof(*rp));
    if (rp) {
	rp->db = db;
	rp->result = res;
    }
    return (dico_result_t) rp;
}

static dico_result_t
mod_match(dico_handle_t hp, const dico_strategy_t strat, const char *word)
{
    struct _guile_database *db = (struct _guile_database *)hp;
    SCM scm_strat = _make_strategy(strat);
    SCM res;
    struct dico_key *key;
    SCM scm_key;

    scm_key = dico_new_scm_key(&key);
    
    if (dico_key_init(key, strat, word)) {
	dico_log(L_ERR, 0, _("mod_match: key initialization failed"));
	return NULL;
    }

    if (guile_call_proc(&res, db->vtab[match_proc],
			scm_list_3(db->handle, scm_strat, scm_key)))
	return NULL;

    dico_key_deinit(key);

    if (res == SCM_BOOL_F || res == SCM_EOL)
	return NULL;
    scm_gc_protect_object(res);
    return make_guile_result(db, res);
}

static dico_result_t
mod_define(dico_handle_t hp, const char *word)
{
    struct _guile_database *db = (struct _guile_database *)hp;
    SCM res;

    if (guile_call_proc(&res, db->vtab[define_proc],
			scm_list_2(db->handle,
				   scm_from_locale_string(word))))
	return NULL;

    if (res == SCM_BOOL_F || res == SCM_EOL)
	return NULL;
    scm_gc_protect_object(res);
    return make_guile_result(db, res);
}

static int
mod_output_result (dico_result_t rp, size_t n, dico_stream_t str)
{
    int rc;
    struct guile_result *gres = (struct guile_result *)rp;
    SCM res;
    SCM oport = scm_current_output_port();
    SCM port = _make_dico_port(str);

    scm_set_current_output_port(port);

    rc = guile_call_proc(&res, gres->db->vtab[output_proc],
			 scm_list_2(gres->result, scm_from_int(n)));
    scm_set_current_output_port(oport);
    _dico_port_close(port);
    if (rc)
	return 1;
    return 0;
}

static size_t
mod_result_count (dico_result_t rp)
{
    struct guile_result *gres = (struct guile_result *)rp;
    SCM res;

    if (guile_call_proc(&res, gres->db->vtab[result_count_proc],
			scm_list_1(gres->result)))
	return 0;
    if (scm_is_integer(res))
	return scm_to_int32(res);
    else
	rettype_error(gres->db->vtab[result_count_proc]);
    return 0;
}

static size_t
mod_compare_count (dico_result_t rp)
{
    struct guile_result *gres = (struct guile_result *)rp;

    if (gres->db->vtab[compare_count_proc]) {
	SCM res;

	if (guile_call_proc(&res, gres->db->vtab[compare_count_proc],
			    scm_list_1(gres->result)))
	    return 0;
	if (scm_is_integer(res))
	    return scm_to_int32(res);
	else
	    rettype_error(gres->db->vtab[compare_count_proc]);
    }
    return 0;
}

static void
mod_free_result(dico_result_t rp)
{
    struct guile_result *gres = (struct guile_result *)rp;

    if (gres->db->vtab[free_result_proc]) {
	SCM res;

	guile_call_proc(&res, gres->db->vtab[free_result_proc],
			scm_list_1(gres->result));
    }
    scm_gc_unprotect_object(gres->result);
    free(gres);
}

static int
mod_result_headers (dico_result_t rp, dico_assoc_list_t hdr)
{
    struct guile_result *gres = (struct guile_result *)rp;
    SCM proc = gres->db->vtab[result_headers_proc];

    if (proc) {
	SCM res;

	if (guile_call_proc(&res, proc,
			    scm_list_2(gres->result, assoc_to_scm(hdr))))
	    return 1;
	if (!scm_is_pair(res)) {
	    rettype_error(proc);
	    return 1;
	}
	scm_to_assoc(hdr, res);
    }
    return 0;
}

struct dico_database_module DICO_EXPORT(guile, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NONE,
    mod_init,
    mod_init_db,
    mod_free_db,
    mod_open,
    mod_close,
    mod_info,
    mod_descr,
    mod_lang,
    mod_match,
    mod_define,
    mod_output_result,
    mod_result_count,
    mod_compare_count,
    mod_free_result,
    mod_result_headers
};
