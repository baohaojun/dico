/* This file is part of GNU Dico.
   Copyright (C) 2008, 2012 Wojciech Polak

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
#include <string.h>
#include <errno.h>
#include <appi18n.h>
#include <Python.h>

static char *init_script;
static char *load_path;
static char *root_class = "DicoModule";

static struct dico_option init_option[] = {
    { DICO_OPTSTR(init-script), dico_opt_string, &init_script },
    { DICO_OPTSTR(load-path), dico_opt_string, &load_path },
    { DICO_OPTSTR(root-class), dico_opt_string, &root_class },
    { NULL }
};

struct _python_database {
    const char *dbname;
    int argc;
    char **argv;
    PyThreadState *py_ths;
    PyObject *py_instance;
};


typedef struct {
    PyObject_HEAD;
    struct dico_key *key;
} PySelectionKey;

static void 
_PySelectionKey_dealloc (PyObject *self)
{
}

static PyMethodDef selection_key_methods[] = {
    /* None so far */
    { NULL, NULL, 0, NULL }
};
    
static PyObject *
_PySelectionKey_getattr (PyObject *self, char *name)
{
    PySelectionKey *py_key = (PySelectionKey *)self;

    if (strcmp (name, "word") == 0)
	return PyString_FromString (py_key->key->word);
    return Py_FindMethod (selection_key_methods, self, name);
}

static PyObject *
_PySelectionKey_repr (PyObject *self)
{
    PySelectionKey *py_key = (PySelectionKey *)self;
    char buf[80];
    snprintf (buf, sizeof buf, "<DicoSelectionKey %s>", py_key->key->word);
    return PyString_FromString (buf);
}

static PyObject *
_PySelectionKey_str (PyObject *self)
{
    PySelectionKey *py_key = (PySelectionKey *)self;
    return PyString_FromString (py_key->key->word);
}

static PyTypeObject PySelectionKeyType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "DicoSelectionKey",      /* tp_name */
    sizeof (PySelectionKey), /* tp_basicsize */
    0,                   /* tp_itemsize; */
    _PySelectionKey_dealloc, /* tp_dealloc; */
    NULL,                /* tp_print; */
    _PySelectionKey_getattr, /* tp_getattr; __getattr__ */
    NULL,                /* tp_setattr; __setattr__ */
    NULL,                /* tp_compare; __cmp__ */
    _PySelectionKey_repr,    /* tp_repr; __repr__ */
    NULL,                /* tp_as_number */
    NULL,                /* tp_as_sequence */
    NULL,                /* tp_as_mapping */
    NULL,                /* tp_hash; __hash__ */
    NULL,                /* tp_call; __call__ */
    _PySelectionKey_str,     /* tp_str; __str__ */
};


typedef struct {
    PyObject_HEAD;
    dico_strategy_t strat;
} PyStrategy;

static inline PyObject *
_ro (PyObject *obj)
{
    Py_INCREF (obj);
    return obj;
}

static PyObject *
strat_select_method (PyObject *self, PyObject *args)
{
    PyStrategy *py_strat = (PyStrategy *)self;
    PySelectionKey *py_key;
    char *word = NULL;

    if (!PyArg_ParseTuple (args, "sO!", &word, &PySelectionKeyType, &py_key))
	return _ro (Py_False);

    return _ro (py_strat->strat->sel (DICO_SELECT_RUN, py_key->key, word)
		? Py_True : Py_False);
}

static PyMethodDef strategy_methods[] = {
    { "select", strat_select_method, METH_VARARGS,
      "Return True if KEY matches WORD as per this strategy." },
    { NULL, NULL, 0, NULL }
};

static void 
_PyStrategy_dealloc (PyObject *self)
{
}

static PyObject *
_PyStrategy_getattr (PyObject *self, char *name)
{
    PyStrategy *py_strat = (PyStrategy *)self;
    dico_strategy_t strat = py_strat->strat;

    if (strcmp (name, "name") == 0) {
	/* Return the name of the strategy STRAT. */
	return PyString_FromString (strat->name);
    } else if (strcmp (name, "descr") == 0) {
	/* Return a textual description of the strategy STRAT. */
	return PyString_FromString (strat->descr);
    } else if (strcmp (name, "has_selector") == 0) {
	/* Return True if STRAT has a selector. */
	return _ro (strat->sel ? Py_True : Py_False);
    } else if (strcmp (name, "is_default") == 0) {
	/* Return True if STRAT is a default strategy. */
	return _ro (dico_strategy_is_default_p (strat) ? Py_True : Py_False);
    }
    return Py_FindMethod (strategy_methods, self, name);
}

static PyObject *
_PyStrategy_repr (PyObject *self)
{
    PyStrategy *py_strat = (PyStrategy *)self;
    char buf[80];
    snprintf (buf, sizeof buf, "<DicoStrategy %s>", py_strat->strat->name);
    return PyString_FromString (buf);
}

static PyObject *
_PyStrategy_str (PyObject *self)
{
    PyStrategy *py_strat = (PyStrategy *)self;
    return PyString_FromString (py_strat->strat->name);
}

static PyTypeObject PyStrategyType = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,
    "DicoStrategy",      /* tp_name */
    sizeof (PyStrategy), /* tp_basicsize */
    0,                   /* tp_itemsize; */
    _PyStrategy_dealloc, /* tp_dealloc; */
    NULL,                /* tp_print; */
    _PyStrategy_getattr, /* tp_getattr; __getattr__ */
    NULL,                /* tp_setattr; __setattr__ */
    NULL,                /* tp_compare; __cmp__ */
    _PyStrategy_repr,    /* tp_repr; __repr__ */
    NULL,                /* tp_as_number */
    NULL,                /* tp_as_sequence */
    NULL,                /* tp_as_mapping */
    NULL,                /* tp_hash; __hash__ */
    NULL,                /* tp_call; __call__ */
    _PyStrategy_str,     /* tp_str; __str__ */
};


static dico_stream_t dico_stream_output;
static dico_stream_t dico_stream_log_err;
static dico_stream_t dico_stream_log_info;

static PyObject *
_capture_stdout_result (PyObject *self, PyObject *args)
{
    char *buf = "";
    if (!PyArg_ParseTuple (args, "s", &buf))
	return NULL;
    if (dico_stream_output)
	dico_stream_write (dico_stream_output, buf, strlen (buf));
    return _ro (Py_None);
}

static PyObject *
_capture_stdout_info (PyObject *self, PyObject *args)
{
    char *buf = "";
    if (!PyArg_ParseTuple (args, "s", &buf))
	return NULL;
    if (dico_stream_log_info)
	dico_stream_write (dico_stream_log_info, buf, strlen (buf));
    return _ro (Py_None);
}

static PyObject *
_capture_stderr (PyObject *self, PyObject *args)
{
    char *buf = "";
    if (!PyArg_ParseTuple (args, "s", &buf))
	return NULL;
    if (dico_stream_log_err)
	dico_stream_write (dico_stream_log_err, buf, strlen (buf));
    return _ro (Py_None);
}

static PyMethodDef capture_stdout_result_method[] =
{
    { "write", _capture_stdout_result, 1 },
    { NULL, NULL, 0, NULL }
};
static PyMethodDef capture_stdout_info_method[] =
{
    { "write", _capture_stdout_info, 1 },
    { NULL, NULL, 0, NULL }
};
static PyMethodDef capture_stderr_method[] =
{
    { "write", _capture_stderr, 1 },
    { NULL, NULL, 0, NULL }
};

static int
_python_selector (int cmd, struct dico_key *key, const char *dict_word)
{
    PyObject *py_args, *py_res;
    PySelectionKey *py_key;
    void *closure = key->strat->closure;
    
    py_args = PyTuple_New (3);
    PyTuple_SetItem (py_args, 0, PyInt_FromLong (cmd));
    py_key = PyObject_NEW (PySelectionKey, &PySelectionKeyType);
    py_key->key = key;
    PyTuple_SetItem (py_args, 1, (PyObject*)py_key);
    PyTuple_SetItem (py_args, 2, PyString_FromString (dict_word));

    if (closure && PyCallable_Check (closure)) {
	py_res = PyObject_CallObject (closure, py_args);
	if (py_res) {
	    if (PyBool_Check (py_res))
		return py_res == Py_True ? 1 : 0;
	} else if (PyErr_Occurred ())
	    PyErr_Print ();
    }
    return 0;
}

static PyObject *
dico_register_strat (PyObject *self, PyObject *args)
{
    struct dico_strategy strat;
    char *name = NULL;
    char *descr = NULL;
    char *fnc = NULL;

    if (!PyArg_ParseTuple (args, "ss|s", &name, &descr, &fnc))
        return NULL;

    strat.name = name;
    strat.descr = descr;
    if (!fnc) {
	strat.sel = NULL;
	strat.closure = NULL;
    } else {
	strat.sel = _python_selector;
	strat.closure = fnc;
    }
    dico_strategy_add (&strat);

    return _ro (Py_None);
}

static PyObject *
dico_register_markup (PyObject *self, PyObject *py_obj)
{
    int rc;
    char *type;

    if (!PyString_Check (py_obj)) {
	PyErr_SetString (PyExc_TypeError, _("This parameter must be a string"));
        return NULL;
    }

    type = strdup (PyString_AsString (py_obj));
    rc = dico_markup_register (type);
    free (type);
    if (rc)
	return NULL;

    return _ro (Py_None);
}

static PyObject *
dico_current_markup (PyObject *self)
{
    return _ro (PyString_FromString (dico_markup_type));
}

static PyMethodDef dico_methods[] = {
    { "register_strat", dico_register_strat, METH_VARARGS,
      "Register a new strategy." },
    { "register_markup", dico_register_markup, METH_O,
      "Register a new markup type." },
    { "current_markup", (PyCFunction) dico_current_markup, METH_NOARGS,
      "Return current dico markup type." },
    { NULL, NULL, 0, NULL }
};


static PyObject *
_argv_to_tuple (int argc, char **argv)
{
    int i;
    PyObject *py_args = PyTuple_New (argc);
    for (i = 0; argc; argc--, argv++, i++) {
	PyTuple_SetItem (py_args, i, PyString_FromString (*argv));
    }
    return py_args;
}

static int
mod_init (int argc, char **argv)
{
    if (dico_parseopt (init_option, argc, argv, 0, NULL))
	return 1;

    if (!Py_IsInitialized ())
	Py_Initialize ();

    dico_stream_log_err = dico_log_stream_create (L_ERR);
    dico_stream_log_info = dico_log_stream_create (L_INFO);
    return 0;
}

static void
insert_load_path (const char *dir)
{
    PyObject *py_sys, *py_path, *py_dirstr;
    const char *p;

    py_sys = PyImport_ImportModule ("sys");
    py_path = PyObject_GetAttrString (py_sys, "path");
    p = dir + strlen (dir);
    do {
	size_t len;
	for (len = 0; p > dir && p[-1] != ':'; p--, len++)
	    ;
	py_dirstr = PyString_FromStringAndSize (p, len);
	if (PySequence_Index (py_path, py_dirstr) == -1) {
	    PyObject *py_list;
	    PyErr_Clear ();
	    py_list = Py_BuildValue ("[O]", py_dirstr);
	    PyList_SetSlice (py_path, 0, 0, py_list);
	    Py_DECREF (py_list);
	}
	Py_DECREF (py_dirstr);
    } while (p-- > dir);
    Py_DECREF (py_path);
    Py_DECREF (py_sys);
}

static struct {
    char *name;
    int value;
} constab[] = {
    { "DICO_SELECT_BEGIN", DICO_SELECT_BEGIN },
    { "DICO_SELECT_RUN", DICO_SELECT_RUN },
    { "DICO_SELECT_END", DICO_SELECT_END },
    { NULL }
};

static void
declare_constants(PyObject *module)
{
    int i;
    
    for (i = 0; constab[i].name; i++)
	PyModule_AddIntConstant (module, constab[i].name, constab[i].value);
}

static dico_handle_t
mod_init_db (const char *dbname, int argc, char **argv)
{
    int pindex;
    struct _python_database *db;
    PyObject *py_err, *py_out, *py_name, *py_module, *py_class;
    PyThreadState *py_ths;

    if (dico_parseopt (init_option, argc, argv, DICO_PARSEOPT_PERMUTE,
		       &pindex))
	return NULL;

    if (!init_script)
	return NULL;

    argv += pindex;
    argc -= pindex;

    db = malloc (sizeof (*db));
    if (!db) {
	dico_log (L_ERR, 0, _("%s: not enough memory"), "mod_init_db");
	return NULL;
    }
    db->dbname = dbname;
    db->argc = argc;
    db->argv = argv;

    py_ths = Py_NewInterpreter ();
    if (!py_ths) {
	dico_log (L_ERR, 0,
		  _("mod_init_db: cannot create new interpreter: %s"), 
		  init_script);
	return NULL;
    }

    PyThreadState_Swap (py_ths);
    db->py_ths = py_ths;

    declare_constants (Py_InitModule ("dico", dico_methods));
    
    PyRun_SimpleString ("import sys");
    if (load_path)
	insert_load_path (load_path);
    insert_load_path ("");

    py_err = Py_InitModule ("stderr", capture_stderr_method);
    if (py_err)
	PySys_SetObject ("stderr", py_err);
    py_out = Py_InitModule ("stdout", capture_stdout_info_method);
    if (py_out)
	PySys_SetObject ("stdout", py_out);

    py_name = PyString_FromString (init_script);
    py_module = PyImport_Import (py_name);
    Py_DECREF (py_name);

    if (!py_module) {
	dico_log (L_ERR, 0, _("mod_init_db: cannot load init script: %s"), 
		  init_script);
	if (PyErr_Occurred ())
	    PyErr_Print ();
	return NULL;
    }

    py_class = PyObject_GetAttrString (py_module, root_class);
    if (py_class && PyClass_Check (py_class)) {
	PyObject *py_instance = PyInstance_New (py_class,
						_argv_to_tuple (argc, argv),
						NULL);
	if (py_instance && PyInstance_Check (py_instance))
	    db->py_instance = py_instance;
	else if (PyErr_Occurred ()) {
	    PyErr_Print ();
	    return NULL;
	}
    } else {
	dico_log (L_ERR, 0, _("mod_init_db: cannot create class instance: %s"),
		  root_class);
	if (PyErr_Occurred ())
	    PyErr_Print ();
	return NULL;
    }
    return (dico_handle_t)db;
}

static int
mod_free_db (dico_handle_t hp)
{
    struct _python_database *db = (struct _python_database *)hp;

    PyThreadState_Swap (NULL);
    PyThreadState_Delete (db->py_ths);

    free (db);
    return 0;
}

static int
mod_open (dico_handle_t hp)
{
    PyObject *py_args, *py_value, *py_fnc, *py_res;
    struct _python_database *db = (struct _python_database *)hp;

    PyThreadState_Swap (db->py_ths);

    py_value = PyString_FromString (db->dbname);
    py_args = PyTuple_New (1);
    PyTuple_SetItem (py_args, 0, py_value);

    py_fnc = PyObject_GetAttrString (db->py_instance, "open");
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, py_args);
	Py_DECREF (py_args);
	Py_DECREF (py_fnc);
	if (py_res && PyBool_Check (py_res) && py_res == Py_False) {
	    return 1;
	} else if (PyErr_Occurred ()) {
	    PyErr_Print ();
	    return 1;
	}
    }
    return 0;
}

static int
mod_close (dico_handle_t hp)
{
    PyObject *py_fnc, *py_res;
    struct _python_database *db = (struct _python_database *)hp;

    PyThreadState_Swap (db->py_ths);

    py_fnc = PyObject_GetAttrString (db->py_instance, "close");
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, NULL);
	Py_DECREF (py_fnc);
	if (py_res && PyBool_Check (py_res) && py_res == Py_False) {
	    return 1;
	}
	else if (PyErr_Occurred ()) {
	    PyErr_Print ();
	    return 1;
	}
    }
    return 0;
}

static char *
_mod_get_text (PyObject *py_instance, const char *method)
{
    PyObject *py_fnc, *py_res;

    if (!py_instance)
	return NULL;
    else if (!PyObject_HasAttrString (py_instance, method))
	return NULL;
    
    py_fnc = PyObject_GetAttrString (py_instance, method);
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, NULL);
	Py_DECREF (py_fnc);
	if (py_res && PyString_Check (py_res)) {
	    char *text = strdup (PyString_AsString (py_res));
	    Py_DECREF (py_res);
	    return text;
	} else if (PyErr_Occurred ())
	    PyErr_Print ();
    }
    return NULL;
}

static char *
mod_info (dico_handle_t hp)
{
    struct _python_database *db = (struct _python_database *)hp;

    PyThreadState_Swap (db->py_ths);
    return _mod_get_text (db->py_instance, "info");
}

static char *
mod_descr (dico_handle_t hp)
{
    struct _python_database *db = (struct _python_database *)hp;

    PyThreadState_Swap (db->py_ths);
    return _mod_get_text (db->py_instance, "descr");
}

static dico_list_t
_tuple_to_langlist (PyObject *py_obj)
{
    dico_list_t list = NULL;

    if (!py_obj)
	return NULL;
    
    if (PyString_Check (py_obj)) {
	char *text = strdup (PyString_AsString (py_obj));
	list = dico_list_create ();
	dico_list_append (list, text);
    } else if (PyTuple_Check (py_obj) || PyList_Check (py_obj)) {
	PyObject *py_item;
	PyObject *py_iterator = PyObject_GetIter (py_obj);
	
	list = dico_list_create ();

	if (py_iterator) {
	    while (py_item = PyIter_Next (py_iterator)) {
		if (PyString_Check (py_item)) {
		    char *text = strdup (PyString_AsString (py_item));
		    dico_list_append (list, text);
		}
		Py_DECREF (py_item);
	    }
	    Py_DECREF (py_iterator);
	    if (PyErr_Occurred ()) {
		PyErr_Print ();
		return NULL;
	    }
	}
    }
    return list;
}

static int
mod_lang (dico_handle_t hp, dico_list_t list[2])
{
    PyObject *py_fnc, *py_res;
    struct _python_database *db = (struct _python_database *)hp;
    list[0] = list[1] = NULL;

    PyThreadState_Swap (db->py_ths);
    if (!PyObject_HasAttrString (db->py_instance, "lang"))
	return 1;

    py_fnc = PyObject_GetAttrString (db->py_instance, "lang");
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, NULL);
	Py_DECREF (py_fnc);
	if (py_res) {
	    if (PyString_Check (py_res)) {
		char *text = strdup (PyString_AsString (py_res));
		Py_DECREF (py_res);
		list[0] = dico_list_create ();
		dico_list_append (list[0], text);
	    } else if (PyTuple_Check (py_res)) {
		switch (PyTuple_Size (py_res)) {
		case 2:
		    list[0] = _tuple_to_langlist (PyTuple_GetItem (py_res, 0));
		    list[1] = _tuple_to_langlist (PyTuple_GetItem (py_res, 1));
		    break;
		    
		case 1:
		    list[0] = _tuple_to_langlist (PyTuple_GetItem (py_res, 0));
		    break;

		default:
		    dico_log (L_ERR, 0,
			      _("Method `lang' must return at most"
				" 2 elements"));
		    return 1;
		}
		Py_DECREF (py_res);
	    } else if (PyList_Check (py_res)) {
		switch (PyList_Size (py_res)) {
		case 2:
		    list[0] = _tuple_to_langlist (PyList_GetItem (py_res, 0));
		    list[1] = _tuple_to_langlist (PyList_GetItem (py_res, 1));
		    break;
		    
		case 1:
		    list[0] = _tuple_to_langlist (PyList_GetItem (py_res, 0));
		    break;

		default:
		    dico_log (L_ERR, 0,
			      _("Method `lang' must return at most"
				" 2 elements"));
		    return 1;
		}
		Py_DECREF (py_res);
	    } else {
		dico_log (L_ERR, 0, _("Method `lang' must return a tuple or "
				      "a list"));
		return 1;
	    }
	} else if (PyErr_Occurred ()) {
	    PyErr_Print ();
	    return 1;
	}
    }
    return 0;
}

struct python_result {
    struct _python_database *db;
    PyObject *result;
};

static dico_result_t
_make_python_result (struct _python_database *db, PyObject *res)
{
    struct python_result *rp = malloc (sizeof (*rp));
    if (rp) {
	rp->db = db;
	rp->result = res;
    }
    return (dico_result_t)rp;
}

dico_result_t
do_match(struct _python_database *db, const dico_strategy_t strat,
	 struct dico_key *key)
{
    PyStrategy *py_strat;
    PySelectionKey *py_key;
    PyObject *py_args, *py_fnc, *py_res;
    
    py_key = PyObject_NEW(PySelectionKey, &PySelectionKeyType);
    if (!py_key)
	return NULL;
    py_key->key = key;
    
    py_strat = PyObject_NEW(PyStrategy, &PyStrategyType);
    if (py_strat) {
	py_strat->strat = strat;
	
	py_args = PyTuple_New(2);
	PyTuple_SetItem(py_args, 0, (PyObject *)py_strat);
	PyTuple_SetItem(py_args, 1, (PyObject *)py_key);
	py_fnc = PyObject_GetAttrString(db->py_instance, "match_word");
	if (py_fnc && PyCallable_Check(py_fnc)) {
	    py_res = PyObject_CallObject(py_fnc, py_args);
	    Py_DECREF(py_args);
	    Py_DECREF(py_fnc);
	    if (py_res) {
		if (PyBool_Check(py_res) && py_res == Py_False)
		    return NULL;
		else
		    return _make_python_result(db, py_res);
	    } else if (PyErr_Occurred())
		PyErr_Print();
	}
    }
    return NULL;
}

static dico_result_t
mod_match (dico_handle_t hp, const dico_strategy_t strat, const char *word)
{
    struct _python_database *db = (struct _python_database *)hp;
    struct dico_key key;
    dico_result_t res;
    
    PyThreadState_Swap(db->py_ths);

    if (dico_key_init(&key, strat, word)) {
	dico_log(L_ERR, 0, _("mod_match: key initialization failed"));
	return NULL;
    }
    res = do_match(db, strat, &key);
    dico_key_deinit(&key);
    return res;
}

static dico_result_t
mod_define (dico_handle_t hp, const char *word)
{
    PyObject *py_args, *py_fnc, *py_res;
    struct _python_database *db = (struct _python_database *)hp;

    PyThreadState_Swap (db->py_ths);

    py_args = PyTuple_New (1);
    PyTuple_SetItem (py_args, 0, PyString_FromString (word));

    py_fnc = PyObject_GetAttrString (db->py_instance, "define_word");
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, py_args);
	Py_DECREF (py_args);
	Py_DECREF (py_fnc);
	if (py_res) {
	    if (PyBool_Check (py_res) && py_res == Py_False)
		return NULL;
	    else
		return _make_python_result (db, py_res);
	} else if (PyErr_Occurred ())
	    PyErr_Print ();
    }
    return NULL;
}

static int
mod_output_result (dico_result_t rp, size_t n, dico_stream_t str)
{
    PyObject *py_args, *py_fnc, *py_res, *py_out;
    struct python_result *gres = (struct python_result *)rp;
    struct _python_database *db = (struct _python_database *)gres->db;

    PyThreadState_Swap (db->py_ths);

    dico_stream_output = str;

    py_out = Py_InitModule ("stdout", capture_stdout_result_method);
    if (py_out)
	PySys_SetObject ("stdout", py_out);
    else {
	dico_log (L_ERR, 0, _("mod_output_result: cannot capture stdout"));
	return 1;
    }

    py_args = PyTuple_New (2);
    PyTuple_SetItem (py_args, 0, gres->result);
    Py_INCREF (gres->result);
    PyTuple_SetItem (py_args, 1, PyLong_FromLong (n));

    py_fnc = PyObject_GetAttrString (db->py_instance, "output");
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, py_args);
	Py_DECREF (py_args);
	Py_DECREF (py_fnc);
	if (PyErr_Occurred ())
	    PyErr_Print ();
    }

    py_out = Py_InitModule ("stdout", capture_stdout_info_method);
    if (py_out)
	PySys_SetObject ("stdout", py_out);

    dico_stream_output = NULL;
    return 0;
}

static size_t
_mod_get_size_t (PyObject *py_instance, PyObject *py_args, const char *method)
{
    PyObject *py_fnc, *py_res;

    if (!py_instance)
	return 0;

    py_fnc = PyObject_GetAttrString (py_instance, method);
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, py_args);
	Py_DECREF (py_fnc);
	if (py_res && PyInt_Check (py_res)) {
	    size_t s = (size_t)PyInt_AsSsize_t (py_res);
	    Py_DECREF (py_res);
	    return s;
	} else if (PyErr_Occurred ())
	    PyErr_Print ();
    }
    return 0;
}

static size_t
mod_result_count (dico_result_t rp)
{
    PyObject *py_args;
    struct python_result *gres = (struct python_result *)rp;
    struct _python_database *db = (struct _python_database *)gres->db;
    size_t ret;

    PyThreadState_Swap (db->py_ths);

    py_args = PyTuple_New (1);
    PyTuple_SetItem (py_args, 0, gres->result);
    Py_INCREF (gres->result);
    ret = _mod_get_size_t (db->py_instance, py_args, "result_count");
    Py_DECREF (py_args);
    return ret;
}

static size_t
mod_compare_count (dico_result_t rp)
{
    PyObject *py_args;
    struct python_result *gres = (struct python_result *)rp;
    struct _python_database *db = (struct _python_database *)gres->db;
    size_t ret;

    PyThreadState_Swap (db->py_ths);
    if (!PyObject_HasAttrString (db->py_instance, "compare_count"))
	return 0;

    py_args = PyTuple_New (1);
    PyTuple_SetItem (py_args, 0, gres->result);
    Py_INCREF (gres->result);
    ret = _mod_get_size_t (db->py_instance, py_args, "compare_count");
    Py_DECREF (py_args);
    return ret;
}

static void
mod_free_result (dico_result_t rp)
{
    PyObject *py_args, *py_fnc;
    struct python_result *gres = (struct python_result *)rp;
    struct _python_database *db = (struct _python_database *)gres->db;

    PyThreadState_Swap (db->py_ths);
    if (!PyObject_HasAttrString (db->py_instance, "free_result"))
	return;

    py_args = PyTuple_New (1);
    PyTuple_SetItem (py_args, 0, gres->result);
    Py_INCREF (gres->result);
    py_fnc = PyObject_GetAttrString (db->py_instance, "free_result");
    if (py_fnc && PyCallable_Check (py_fnc)) {
	PyObject_CallObject (py_fnc, py_args);
	Py_DECREF (py_args);
	Py_DECREF (py_fnc);
	if (PyErr_Occurred ())
	    PyErr_Print ();
    }
    Py_DECREF (gres->result);
    free (gres);
}

static PyObject *
_assoc_to_dict (dico_assoc_list_t assoc)
{
    PyObject *py_dict;
    dico_iterator_t itr;
    struct dico_assoc *p;

    py_dict = PyDict_New ();
    if (py_dict) {
	itr = dico_assoc_iterator (assoc);
	for (p = dico_iterator_first (itr); p; p = dico_iterator_next (itr)) {
	    PyDict_SetItemString (py_dict, p->key,
				  PyString_FromString (p->value));
	}
	dico_iterator_destroy (&itr);
	return _ro (py_dict);
    }
    return NULL;
}

static void
_dict_to_assoc (dico_assoc_list_t assoc, PyObject *py_dict)
{
    PyObject *py_key, *py_value;
    Py_ssize_t py_pos = 0;

    dico_assoc_clear (assoc);

    while (PyDict_Next (py_dict, &py_pos, &py_key, &py_value)) {
	char *key, *val;
	key = strdup (PyString_AsString (py_key));
	val = strdup (PyString_AsString (py_value));
	dico_assoc_append (assoc, key, val);
    }
}

static int
mod_result_headers (dico_result_t rp, dico_assoc_list_t hdr)
{
    PyObject *py_dict, *py_args, *py_fnc, *py_res;
    struct python_result *gres = (struct python_result *)rp;
    struct _python_database *db = (struct _python_database *)gres->db;

    PyThreadState_Swap (db->py_ths);
    if (!PyObject_HasAttrString (db->py_instance, "result_headers"))
	return 0;

    py_dict = _assoc_to_dict (hdr);

    py_args = PyTuple_New (2);
    PyTuple_SetItem (py_args, 0, gres->result);
    PyTuple_SetItem (py_args, 1, py_dict);
    Py_INCREF (gres->result);

    py_fnc = PyObject_GetAttrString (db->py_instance, "result_headers");
    if (py_fnc && PyCallable_Check (py_fnc)) {
	py_res = PyObject_CallObject (py_fnc, py_args);
	Py_DECREF (py_args);
	Py_DECREF (py_fnc);
	if (py_res && PyDict_Check (py_res)) {
	    _dict_to_assoc (hdr, py_res);
	    Py_DECREF (py_res);
	} else if (PyErr_Occurred ()) {
	    PyErr_Print ();
	    return 1;
	}
    }
    Py_DECREF (py_dict);
    return 0;
}

struct dico_database_module DICO_EXPORT(python, module) = {
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
