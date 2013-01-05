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
#include <pwd.h>
#include <grp.h>

int foreground;     /* Run in foreground mode */
int single_process; /* Single process mode */
/* Location of the default configuration file */
char *config_file = SYSCONFIG "/dicod.conf" ;
int config_lint_option; /* Check configuration file syntax and exit. */
/* Location of the pidfile */
char *pidfile_name = LOCALSTATEDIR "/run/dicod.pid";

/* Operation mode */
int mode = MODE_DAEMON;

/* Debug verbosity level */
int debug_level;
char *debug_level_str;
int debug_source_info;
dico_stream_t debug_stream;

/* Maximum number of children in allowed in daemon mode. */
unsigned int max_children = 64;
/* Wait this number of seconds for all subprocesses to terminate. */
unsigned int shutdown_timeout = 5;
/* Inactivity timeout */
unsigned int inactivity_timeout = 0;

/* Syslog parameters: */ 
int log_to_stderr;  /* Log to stderr */
const char *log_tag; 
int log_facility = LOG_FACILITY;
int log_print_severity;
int transcript;

/* Server information (for SHOW INFO command) */
const char *server_info;

dicod_acl_t show_sys_info;

/* This host name */
char *hostname;

/* Text of initial banner. By default == program_version */
char *initial_banner_text;

/* Alternative help text. Default - see dicod_help (commands.c) */
char *help_text;

/* List of sockets to listen on for the requests */
dico_list_t /* of struct sockaddr */ listen_addr;

/* User database for AUTH */
dico_udb_t user_db;

/* Run as this user */
uid_t user_id;
gid_t group_id;
/* Retain these supplementary groups when switching to the user privileges. */
dico_list_t /* of gid_t */ group_list;

/* List of directories to search for handler modules. */
dico_list_t /* of char * */ module_load_path;
dico_list_t /* of char * */ prepend_load_path;

/* List of configured database module instances */
dico_list_t /* of dicod_module_instance_t */ modinst_list;

/* List of configured dictionaries */
dico_list_t /* of dicod_database_t */ database_list;

/* Global Dictionary ACL */
dicod_acl_t global_acl;

/* ACL for incoming connections */
dicod_acl_t connect_acl;

/* From CLIENT command: */
char *client_id;

/* In authenticated state: */
char *user_name;  /* User name */
dico_list_t /* of char * */ user_groups; /* List of groups he is member of */

/* Information from AUTH check: */
int identity_check;     /* Enable identity check */
char *identity_name;    /* Name received from AUTH query. */
char *ident_keyfile;    /* Keyfile for decrypting AUTH replies. */
long ident_timeout = 3; /* Timeout for AUTH I/O */

/* Provide timing information */
int timing_option;

/* Name of the default strategy */
const char *default_strategy_name;


typedef int (*grecs_list_iterator_t)(grecs_value_t *, void *);

void
grecs_list_iterate(struct grecs_list *list, grecs_list_iterator_t fun,
		   void *data)
{
    struct grecs_list_entry *ep;

    for (ep = list->head; ep; ep = ep->next) {
	grecs_value_t *vp = ep->data;
	if (fun(vp, data))
	    break;
    }
}
	


/* Configuration */
int
cb_dico_list(enum grecs_callback_command cmd,
	     grecs_locus_t *locus,
	     void *varptr,
	     grecs_value_t *value,
	     void *cb_data)
{
    dico_list_t *plist = varptr, list;

    if (*plist)
	list = *plist;
    else {
	list = xdico_list_create();
	dico_list_set_free_item(list, dicod_free_item, NULL);
	*plist = list;
    }
    switch (value->type) {
    case GRECS_TYPE_STRING:
	xdico_list_append(list, xstrdup(value->v.string));
	break;

    case GRECS_TYPE_LIST: {
	struct grecs_list_entry *ep;

	for (ep = value->v.list->head; ep; ep = ep->next) {
	    const grecs_value_t *vp = ep->data;

	    if (vp->type != GRECS_TYPE_STRING) {
		grecs_error(&vp->locus, 0,
			    _("list element must be a string"));
		return 1;
	    }
	    xdico_list_append(list, xstrdup(vp->v.string));
	}
	break;
    }

    case GRECS_TYPE_ARRAY:
	grecs_error(locus, 0, _("too many arguments"));
	return 1;
    }
    return 0;
}

int
cb_dico_sockaddr_list(enum grecs_callback_command cmd,
		      grecs_locus_t *locus,
		      void *varptr,
		      grecs_value_t *value,
		      void *cb_data)
{
    dico_list_t *plist = varptr, list;
    struct grecs_sockaddr *sp;
    
    if (*plist)
	list = *plist;
    else {
	list = xdico_list_create();
//FIXME	dico_list_set_free_item(list, dicod_free_item, NULL);
	*plist = list;
    }
    
    switch (value->type) {
    case GRECS_TYPE_STRING:
	sp = xmalloc(sizeof(*sp));
	if (grecs_string_convert(sp, grecs_type_sockaddr,
				 value->v.string, locus)) {
	    free(sp);
	    return 1;
	}
	xdico_list_append(list, sp);
	break;

    case GRECS_TYPE_LIST: {
	struct grecs_list_entry *ep;

	for (ep = value->v.list->head; ep; ep = ep->next) {
	    const grecs_value_t *vp = ep->data;

	    if (vp->type != GRECS_TYPE_STRING) {
		grecs_error(&vp->locus, 0,
			    _("list element must be a socket specification"));
		return 1;
	    }
	    sp = xmalloc(sizeof(*sp));
	    if (grecs_string_convert(sp, grecs_type_sockaddr,
				     vp->v.string, &vp->locus)) {
		free(sp);
		return 1;
	    }
	    xdico_list_append(list, sp);
	}
	break;
    }

    case GRECS_TYPE_ARRAY:
	grecs_error(locus, 0, _("too many arguments"));
	return 1;
    }
    return 0;
}


int
allow_cb(enum grecs_callback_command cmd,
	 grecs_locus_t *locus,
	 void *varptr,
	 grecs_value_t *value,
	 void *cb_data)
{
    dicod_acl_t acl = varptr;

    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    parse_acl_line(locus, 1, acl, value);
    return 0;
}

int
deny_cb(enum grecs_callback_command cmd,
	grecs_locus_t *locus,
	void *varptr,
	grecs_value_t *value,
	void *cb_data)
{
    dicod_acl_t acl = varptr;
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    parse_acl_line(locus, 0, acl, value);
    return 0;
}

int
acl_cb(enum grecs_callback_command cmd,
       grecs_locus_t *locus,
       void *varptr,
       grecs_value_t *value,
       void *cb_data)
{
    void **pdata = cb_data;
    dicod_acl_t acl;
    
    switch (cmd) {
    case grecs_callback_section_begin:
	if (value->type != GRECS_TYPE_STRING) 
	    grecs_error(locus, 0, _("ACL name must be a string"));
	else if (!value->v.string)
	    grecs_error(locus, 0, _("missing ACL name"));
	else {
	    grecs_locus_t defn_loc;
	    acl = dicod_acl_create(value->v.string, locus);
	    if (dicod_acl_install(acl, &defn_loc)) {
		grecs_error(locus, 0,
			     _("redefinition of ACL %s"),
			     value->v.string);
		grecs_error(&defn_loc, 0,
			     _("location of the previous definition"));
		return 1;
	    }
	    *pdata = acl;
	}
	break;

    case grecs_callback_section_end:
    case grecs_callback_set_value:
	break;
    }	
    return 0;
}

struct grecs_keyword kwd_acl[] = {
    { "allow",
      N_("[all|authenticated|group <grp: list>] [from <addr: list>]"),
      N_("Allow access"),
      grecs_type_string, GRECS_DFLT, NULL, 0,
      allow_cb },
    { "deny", N_("[all|authenticated|group <grp: list>] [from <addr: list>]"),
      N_("Deny access"),
      grecs_type_string, GRECS_DFLT, NULL, 0,
      deny_cb },
    { NULL }
};

int
apply_acl_cb(enum grecs_callback_command cmd,
	     grecs_locus_t *locus,
	     void *varptr,
	     grecs_value_t *value,
	     void *cb_data)
{
    dicod_acl_t *pacl = varptr;

    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    if (value->type != GRECS_TYPE_STRING) {
	grecs_error(locus, 0, _("expected scalar value"));
	return 1;
    }
    if ((*pacl =  dicod_acl_lookup(value->v.string)) == NULL) {
	grecs_error(locus, 0, _("no such ACL: `%s'"), value->v.string);
	return 1;
    }
    return 0;
}


int
set_user(enum grecs_callback_command cmd,
	 grecs_locus_t *locus,
	 void *varptr,
	 grecs_value_t *value,
	 void *cb_data)
{
    struct passwd *pw;
    
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }

    if (value->type != GRECS_TYPE_STRING) {
	grecs_error(locus, 0, _("expected scalar value but found list"));
	return 1;
    }
    
    pw = getpwnam(value->v.string);
    if (!pw) {
	grecs_error(locus, 0, _("%s: no such user"), value->v.string);
	return 1;
    }
    user_id = pw->pw_uid;
    group_id = pw->pw_gid;
    return 0;
}

static int set_supp_group(enum grecs_callback_command cmd,
			  grecs_locus_t *locus,
			  void *varptr,
			  grecs_value_t *value,
			  void *cb_data);

static int
set_supp_group_iter(grecs_value_t *value, void *data)
{
    return set_supp_group(grecs_callback_set_value,
			  &value->locus, NULL, value, NULL);
}
	
static int
set_supp_group(enum grecs_callback_command cmd,
	       grecs_locus_t *locus,
	       void *varptr,
	       grecs_value_t *value,
	       void *cb_data)
{
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }

    if (!group_list)
	group_list = xdico_list_create();
    
    if (value->type == GRECS_TYPE_LIST)
	grecs_list_iterate(value->v.list, set_supp_group_iter, NULL);
    else {
	struct group *group = getgrnam(value->v.string);
	if (group)
	    xdico_list_append(group_list, (void*)group->gr_gid);
	else {
	    grecs_error(locus, 0, _("%s: unknown group"), value->v.string);
	    return 1;
	}
    }
    return 0;
}

int
set_mode(enum grecs_callback_command cmd,
	 grecs_locus_t *locus,
	 void *varptr,
	 grecs_value_t *value,
	 void *cb_data)
{
    static struct xlat_tab tab[] = {
	{ "daemon", MODE_DAEMON },
	{ "inetd", MODE_INETD },
	{ NULL }
    };
	
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }

    if (value->type != GRECS_TYPE_STRING) {
	grecs_error(locus, 0, _("expected scalar value but found list"));
	return 1;
    }
    if (xlat_c_string(tab, value->v.string, 0, &mode)) {
	grecs_error(locus, 0, _("unknown mode"));
	return 1;
    }
    return 0;
}

static struct xlat_tab syslog_facility_tab[] = {
    { "USER",    LOG_USER },   
    { "DAEMON",  LOG_DAEMON },
    { "AUTH",    LOG_AUTH },
    { "AUTHPRIV",LOG_AUTHPRIV },
    { "MAIL",    LOG_MAIL },
    { "CRON",    LOG_CRON },
    { "LOCAL0",  LOG_LOCAL0 },
    { "LOCAL1",  LOG_LOCAL1 },
    { "LOCAL2",  LOG_LOCAL2 },
    { "LOCAL3",  LOG_LOCAL3 },
    { "LOCAL4",  LOG_LOCAL4 },
    { "LOCAL5",  LOG_LOCAL5 },
    { "LOCAL6",  LOG_LOCAL6 },
    { "LOCAL7",  LOG_LOCAL7 },
    { NULL }
};

int
set_log_facility(enum grecs_callback_command cmd,
		 grecs_locus_t *locus,
		 void *varptr,
		 grecs_value_t *value,
		 void *cb_data)
{
    const char *str;

    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    if (value->type != GRECS_TYPE_STRING) {
	grecs_error(locus, 0, _("expected scalar value but found list"));
	return 1;
    }
    str = value->v.string;
    if (strncasecmp (str, "LOG_", 4) == 0)
	str += 4;
    if (xlat_c_string(syslog_facility_tab, str, XLAT_ICASE, &log_facility)) {
	grecs_error(locus, 0, _("unknown syslog facility"));
	return 1;
    }
    return 0;
}

static int
cmp_modinst_ident(const void *item, void *data)
{
    const dicod_module_instance_t *inst = item;
    if (!inst->ident)
	return 1;
    return strcmp(inst->ident, (const char*)data);
}

static int
_add_simple_module(grecs_value_t *value, void *unused_data)
{
    if (value->type != GRECS_TYPE_STRING) 
	grecs_error(&value->locus, 0, _("tag must be a string"));
    else if (value->v.string == NULL) 
	grecs_error(&value->locus, 0, _("missing tag"));
    else {
	dicod_module_instance_t *inst = xzalloc(sizeof(*inst));
	inst->ident = xstrdup(value->v.string);
	inst->command = xstrdup(value->v.string);
	xdico_list_append(modinst_list, inst);
    }
    return 0;
}

int
load_module_cb(enum grecs_callback_command cmd,
	       grecs_locus_t *locus,
	       void *varptr,
	       grecs_value_t *value,
	       void *cb_data)
{
    dicod_module_instance_t *inst;
    void **pdata = cb_data;
    
    if (!modinst_list) {
	modinst_list = xdico_list_create();
	dico_list_set_comparator(modinst_list, cmp_modinst_ident);
    }
    
    switch (cmd) {
    case grecs_callback_section_begin:
	inst = xzalloc(sizeof(*inst));
	if (value->type != GRECS_TYPE_STRING) 
	    grecs_error(locus, 0, _("tag must be a string"));
	else if (value->v.string == NULL) 
	    grecs_error(locus, 0, _("missing tag"));
	else
	    inst->ident = xstrdup(value->v.string);
	*pdata = inst;
	break;
	
    case grecs_callback_section_end:
	inst = *pdata;
	xdico_list_append(modinst_list, inst);
	*pdata = NULL;
	break;
	
    case grecs_callback_set_value:
	switch (value->type) {
	case GRECS_TYPE_STRING:
	    _add_simple_module(value, NULL);
	    break;

	case GRECS_TYPE_LIST:
	    grecs_list_iterate(value->v.list, _add_simple_module, NULL);
	    break;

	case GRECS_TYPE_ARRAY:
	    grecs_error(locus, 0, _("too many arguments"));
	}
    }
    return 0;
}

static int
cmp_database_name(const void *item, void *data)
{
    const dicod_database_t *db = item;
    int rc;
    
    if (!db->name)
	return 1;
    rc = strcmp(db->name, (const char*)data);
    if (rc == 0 && !database_visible_p(db))
	rc = 1;
    return rc;
}    

static int
set_database(enum grecs_callback_command cmd,
	     grecs_locus_t *locus,
	     void *varptr,
	     grecs_value_t *value,
	     void *cb_data)
{
    dicod_database_t *dict;
    void **pdata = cb_data;
    
    switch (cmd) {
    case grecs_callback_section_begin:
	dict = xzalloc(sizeof(*dict));
	*pdata = dict;
	break;
	
    case grecs_callback_section_end:
	dict = *pdata;
	if (!dict->name) {
	    grecs_error(locus, 0, _("database name not supplied"));
	    break;
	}

	if (!database_list) {
	    database_list = xdico_list_create();
	    dico_list_set_comparator (database_list, cmp_database_name);
	}
	if (dict->langlist[0] || dict->langlist[1])
	     /* Prevent dico_db_lang from being called */
	    dict->flags |= DICOD_DBF_LANG;
	
	xdico_list_append(database_list, dict);
	*pdata = NULL;
	break;
	
    case grecs_callback_set_value:
	grecs_error(locus, 0, _("invalid use of block statement"));
    }
    return 0;
}

int
set_dict_handler(enum grecs_callback_command cmd,
		 grecs_locus_t *locus,
		 void *varptr,
		 grecs_value_t *value,
		 void *cb_data)
{
    dicod_module_instance_t *inst;
    dicod_database_t *db = varptr;
    int rc;
    struct wordsplit ws;
    
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }

    if (value->type != GRECS_TYPE_STRING) {
	grecs_error(locus, 0, _("expected scalar value but found list"));
	return 1;
    }

    if (wordsplit(value->v.string, &ws, WRDSF_DEFFLAGS)) {
	grecs_error(locus, rc, _("cannot parse command line `%s': %s"),
		    value->v.string, wordsplit_strerror (&ws));
	dicod_database_free(db); 
	return 1;
    } 

    db->argc = ws.ws_wordc;
    db->argv = ws.ws_wordv;
    db->command = db->argv[0];

    ws.ws_wordc = 0;
    ws.ws_wordv = NULL;
    wordsplit_free (&ws);

    inst = dico_list_locate(modinst_list, db->argv[0]);
    if (!inst) {
	grecs_error(locus, 0, _("%s: handler not declared"), db->argv[0]);
	/* FIXME: Free memory */
	return 1;
    }
    db->instance = inst;
    
    return 0;
}

int enable_capability(enum grecs_callback_command cmd,
		      grecs_locus_t *locus,
		      void *varptr,
		      grecs_value_t *value,
		      void *cb_data);

int
set_capability(grecs_value_t *value, void *data)
{
    return enable_capability(grecs_callback_set_value,
			     &value->locus, NULL, value, NULL);
}

int
enable_capability(enum grecs_callback_command cmd,
		  grecs_locus_t *locus,
		  void *varptr,
		  grecs_value_t *value,
		  void *cb_data)
{
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    if (value->type == GRECS_TYPE_LIST)
	grecs_list_iterate(value->v.list, set_capability, locus);
    else if (dicod_capa_add(value->v.string)) 
	grecs_error(locus, 0, _("unknown capability: %s"), value->v.string);
    return 0;
}

int
mime_headers_cb (enum grecs_callback_command cmd,
		 grecs_locus_t *locus,
		 void *varptr,
		 grecs_value_t *value,
		 void *cb_data)
{
    dico_assoc_list_t *pasc = varptr;
    const char *enc;
    
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }

    if (value->type != GRECS_TYPE_STRING) {
	grecs_error(locus, 0, _("expected scalar value"));
	return 1;
    }

    if (dico_header_parse(pasc, value->v.string)) 
	grecs_error(locus, 0, _("cannot parse headers: %s"),
		     strerror(errno));

    if (enc = dico_assoc_find(*pasc, CONTENT_TRANSFER_ENCODING_HEADER)) {
        if (!(strcmp(enc, "quoted-printable") == 0
	      || strcmp(enc, "base64") == 0
	      || strcmp(enc, "8bit") == 0))
	    grecs_error(locus, 0, _("unknown encoding type: %s"), enc);
    }
    
    return 0;
}

struct grecs_keyword kwd_load_module[] = {
    { "command", N_("arg"), N_("Command line."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(dicod_module_instance_t, command) },
    { NULL }
};

struct grecs_keyword kwd_database[] = {
    { "name", N_("word"), N_("Dictionary name (a single word)."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(dicod_database_t, name) },
    { "description", N_("arg"),
      N_("Short description, to be shown in reply to SHOW DB command."),
      grecs_type_string, GRECS_DFLT, NULL,
      offsetof(dicod_database_t, descr) },
    { "info", N_("arg"),
      N_("Full description of the database, to be shown in reply to "
	 "SHOW INFO command."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(dicod_database_t, info) },
    { "languages-from", N_("arg"),
      N_("List of languages this database translates from."),
      grecs_type_string, GRECS_LIST,
      NULL, offsetof(dicod_database_t, langlist[0]), cb_dico_list },
    { "languages-to", N_("arg"),
      N_("List of languages this database translates to."),
      grecs_type_string, GRECS_LIST,
      NULL, offsetof(dicod_database_t, langlist[1]), cb_dico_list },
    { "handler", N_("name"), N_("Name of the handler for this database."),
      grecs_type_string, GRECS_DFLT,
      NULL, 0, set_dict_handler },
    { "visibility-acl", N_("arg: acl"),
      N_("ACL controlling visibility of this database"),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(dicod_database_t, acl), apply_acl_cb },
    { "mime-headers", N_("text"),
      N_("Additional MIME headers"),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(dicod_database_t, mime_headers),
      mime_headers_cb },
    { NULL }
};


struct user_db_conf {
    grecs_locus_t locus;
    char *url;
    char *get_pw;
    char *get_groups;
    char *options;
};

struct user_db_conf user_db_cfg;

struct grecs_keyword kwd_user_db[] = {
    { "password-resource", N_("arg"), N_("Password file or query."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct user_db_conf, get_pw) },
    { "group-resource", N_("arg"),
      N_("File containing user group information or a query to retrieve it."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct user_db_conf, get_groups) },
    { "options", N_("arg"),
      N_("Implementation-dependent options"),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct user_db_conf, options) },
    { NULL }
};

int
user_db_config(enum grecs_callback_command cmd,
	       grecs_locus_t *locus,
	       void *varptr,
	       grecs_value_t *value,
	       void *cb_data)
{
    struct user_db_conf *cfg = varptr;
    void **pdata = cb_data;
    
    switch (cmd) {
    case grecs_callback_section_begin:
	cfg->locus = *locus;
	cfg->locus.beg.file = xstrdup(cfg->locus.beg.file);
	cfg->locus.end.file = xstrdup(cfg->locus.end.file);
	if (value->type != GRECS_TYPE_STRING) 
	    grecs_error(locus, 0, _("URL must be a string"));
	else if (!value->v.string)
	    grecs_error(locus, 0, _("empty URL"));
	else
	    cfg->url = xstrdup(value->v.string);
	*pdata = cfg;
	break;
	
    case grecs_callback_section_end:
	break;
	
    case grecs_callback_set_value:
	cfg->locus = *locus;
	cfg->locus.beg.file = xstrdup(cfg->locus.beg.file);
	cfg->locus.end.file = xstrdup(cfg->locus.end.file);
	if (value->type != GRECS_TYPE_STRING) 
	    grecs_error(locus, 0, _("URL must be a string"));
	else if (!value->v.string)
	    grecs_error(locus, 0, _("empty URL"));
	cfg->url = xstrdup(value->v.string);
    }
    return 0;
}

static void
init_user_db()
{
    if (!user_db_cfg.url)
	sasl_enable = 0;
    else if (dico_udb_create(&user_db,
			     user_db_cfg.url, user_db_cfg.get_pw,
			     user_db_cfg.get_groups, user_db_cfg.options)) {
	grecs_error(&user_db_cfg.locus, errno,
		    _("cannot create user database"));
	sasl_enable = 0;
    }
}


int
alias_cb(enum grecs_callback_command cmd,
	 grecs_locus_t *locus,
	 void *varptr,
	 grecs_value_t *value,
	 void *cb_data)
{
    char **argv;
    int argc;
    int i;

    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    if (value->type != GRECS_TYPE_ARRAY) {
	grecs_error(locus, 0, _("Not enough arguments for alias"));
	return 1;
    }
    argc = value->v.arg.c - 1;
    argv = xcalloc(argc + 1, sizeof(argv[0]));
    for (i = 0; i < argc; i++) {
	if (value->v.arg.v[i+1]->type != GRECS_TYPE_STRING) {
	    grecs_error(locus, 0, _("argument %d has wrong type"), i+1);
	    return 1;
	}
	argv[i] = xstrdup(value->v.arg.v[i+1]->v.string);
    }
    argv[i] = NULL;
    return alias_install(value->v.arg.v[0]->v.string, argc, argv, locus);
}


#ifdef WITH_GSASL
int
sasl_cb(enum grecs_callback_command cmd,
	grecs_locus_t *locus,
	void *varptr,
	grecs_value_t *value,
	void *cb_data)
{
    if (cmd == grecs_callback_set_value) {
	if (value->type != GRECS_TYPE_STRING) 
	    grecs_error(locus, 0, _("expected boolean value but found list"));
	else
	    grecs_string_convert(&sasl_enable, grecs_type_bool,
				 value->v.string, locus);
    }
    return 0;
}

struct grecs_keyword kwd_sasl[] = {
    { "disable-mechanism", N_("mech: list"),
      N_("Disable SASL mechanisms listed in <mech>."),
      grecs_type_string, GRECS_LIST, &sasl_disabled_mech, 0, cb_dico_list},
    { "enable-mechanism", N_("mech: list"),
      N_("Enable SASL mechanisms listed in <mech>."),
      grecs_type_string, GRECS_LIST, &sasl_enabled_mech, 0, cb_dico_list},
    { "service", N_("name"),
      N_("Set service name for GSSAPI and Kerberos."),
      grecs_type_string, GRECS_DFLT, &sasl_service },
    { "realm", N_("name"),
      N_("Set realm name for GSSAPI and Kerberos."),
      grecs_type_string, GRECS_DFLT, &sasl_realm },
    { "anon-group", N_("arg"),
      N_("Define groups for anonymous users."),
      grecs_type_string, GRECS_LIST, &sasl_anon_groups, 0, cb_dico_list },
    { NULL }
};
#endif

static dico_list_t strat_forward;

static int
flush_strat_forward_fn(void *item, void *data)
{
    dico_strategy_t strat = item;
    dico_strategy_t p = dico_list_locate(strat_forward, strat->name);
    if (p && p->stratcl) {
	strat->stratcl = p->stratcl;
	p->stratcl = NULL;
    }
    return 0;
}

void
flush_strat_forward()
{
    dico_strategy_iterate(flush_strat_forward_fn, NULL);
    dico_list_destroy(&strat_forward);
}

int
strategy_cb(enum grecs_callback_command cmd,
	    grecs_locus_t *locus,
	    void *varptr,
	    grecs_value_t *value,
	    void *cb_data)
{
    void **pdata = cb_data;

    switch (cmd) {
    case grecs_callback_section_begin:
	if (value->type != GRECS_TYPE_STRING) 
	    grecs_error(locus, 0, _("Section name must be a string"));
	else if (!value->v.string)
	    grecs_error(locus, 0, _("missing section name"));
	else {
	    dico_strategy_t strat = dico_list_locate(strat_forward,
						     (void*)value->v.string);
	    if (!strat) {
		strat = dico_strategy_create(value->v.string, "");
		strat->stratcl = xdico_list_create();
		if (!strat_forward) {
		    strat_forward = xdico_list_create();
		    dico_list_set_comparator (strat_forward,
					      dico_strat_name_cmp);
		    dico_list_set_free_item(strat_forward,
					    dico_strat_free, NULL);
		}
		xdico_list_append(strat_forward, strat);
	    } 
	    *pdata = strat;
	}
	break;
		
    case grecs_callback_section_end:
	break;
	
    case grecs_callback_set_value:
	grecs_error(locus, 0, _("Unexpected statement"));
    }
    return 0;
}

int
strategy_deny_all_cb(enum grecs_callback_command cmd,
		     grecs_locus_t *locus,
		     void *varptr,
		     grecs_value_t *value,
		     void *cb_data)
{
    int bool;
    
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    if (value->type == GRECS_TYPE_STRING
	&& grecs_string_convert(&bool, grecs_type_bool,
				value->v.string, locus) == 0) {
	if (bool)
	    stratcl_add_disable(*(dico_list_t*) varptr);
    } else
	grecs_error(locus, 0, _("Expected boolean value"));
    return 0;
}

struct compile_pattern_closure {
    dico_list_t list;
};

static int
add_deny_word(grecs_value_t *val, void *data)
{
    struct compile_pattern_closure *cpc = data;
    if (val->type != GRECS_TYPE_STRING) {
	grecs_error(&val->locus, 0, _("Expected string value"));
	return 1;
    }
    stratcl_add_word(cpc->list, val->v.string);
    return 0;
}

int
strategy_deny_word_cb(enum grecs_callback_command cmd,
		      grecs_locus_t *locus,
		      void *varptr,
		      grecs_value_t *value,
		      void *cb_data)
{
    struct compile_pattern_closure cpc;

    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 1;
    }
    cpc.list = *(dico_list_t*) varptr;
    if (value->type == GRECS_TYPE_LIST)
	grecs_list_iterate(value->v.list, add_deny_word, &cpc);
    else if (value->type == GRECS_TYPE_STRING)
	add_deny_word(value, &cpc);
    else
	grecs_error(locus, 0, _("expected list or string"));
    return 0;
}

int
strategy_deny_length(enum grecs_callback_command cmd,
		     grecs_locus_t *locus,
		     dico_list_t list,
		     grecs_value_t *value,
		     enum cmp_op op)
{
    if (cmd != grecs_callback_set_value) {
	grecs_error(locus, 0, _("Unexpected block statement"));
	return 0;
    }
    if (value->type == GRECS_TYPE_STRING) {
	size_t val;
	
	if (grecs_string_convert(&val, grecs_type_size,
				 value->v.string, locus))
	    return 0;
	stratcl_add_cmp(list, op, val);
    } else
	grecs_error(locus, 0, _("Expected number"));
    return 0;
}

int
strategy_deny_length_lt_cb(enum grecs_callback_command cmd,
			   grecs_locus_t *locus,
			   void *varptr,
			   grecs_value_t *value,
			   void *cb_data)
{
    return strategy_deny_length(cmd, locus, *(dico_list_t*) varptr,
				value, cmp_lt);
}

int
strategy_deny_length_le_cb(enum grecs_callback_command cmd,
			   grecs_locus_t *locus,
			   void *varptr,
			   grecs_value_t *value,
			   void *cb_data)
{
    return strategy_deny_length(cmd, locus, *(dico_list_t*) varptr,
				value, cmp_le);
}

int
strategy_deny_length_gt_cb(enum grecs_callback_command cmd,
			   grecs_locus_t *locus,
			   void *varptr,
			   grecs_value_t *value,
			   void *cb_data)
{
    return strategy_deny_length(cmd, locus, *(dico_list_t*) varptr,
				value, cmp_gt);
}

int
strategy_deny_length_ge_cb(enum grecs_callback_command cmd,
			   grecs_locus_t *locus,
			   void *varptr,
			   grecs_value_t *value,
			   void *cb_data)
{
    return strategy_deny_length(cmd, locus, *(dico_list_t*) varptr,
				value, cmp_ge);
}

int
strategy_deny_length_eq_cb(enum grecs_callback_command cmd,
			   grecs_locus_t *locus,
			   void *varptr,
			   grecs_value_t *value,
			   void *cb_data)
{
    return strategy_deny_length(cmd, locus, *(dico_list_t*) varptr,
				value, cmp_eq);
}

int
strategy_deny_length_ne_cb(enum grecs_callback_command cmd,
			   grecs_locus_t *locus,
			   void *varptr,
			   grecs_value_t *value,
			   void *cb_data)
{
    return strategy_deny_length(cmd, locus, *(dico_list_t*) varptr,
				value, cmp_ne);
}

struct grecs_keyword kwd_strategy[] = {
    { "deny-all", N_("arg: bool"), N_("Deny all * and ! look ups"),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_all_cb },
    { "deny-word", N_("cond"), N_("Deny * and ! look-ups on these words."),
      grecs_type_string, GRECS_LIST,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_word_cb, },
    { "deny-length-lt",
      N_("len: number"),
      N_("Deny * and ! look-ups on words with length < <len>."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_length_lt_cb, },
    { "deny-length-le",
      N_("len: number"),
      N_("Deny * and ! look-ups on words with length <= <len>."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_length_le_cb, },
    { "deny-length-gt",
      N_("len: number"),
      N_("Deny * and ! look-ups on words with length > <len>."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_length_gt_cb, },
    { "deny-length-ge",
      N_("len: number"),
      N_("Deny * and ! look-ups on words with length >= <len>."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_length_ge_cb, },
    { "deny-length-eq",
      N_("len: number"),
      N_("Deny * and ! look-ups on words with length == <len>."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_length_eq_cb, },
    { "deny-length-ne",
      N_("len: number"),
      N_("Deny * and ! look-ups on words with length != <len>."),
      grecs_type_string, GRECS_DFLT,
      NULL, offsetof(struct dico_strategy, stratcl),
      strategy_deny_length_ne_cb, },
    { NULL }
};


struct grecs_keyword keywords[] = {
    { "user", N_("name"), N_("Run with these user privileges."),
      grecs_type_string, GRECS_DFLT,
      NULL, 0, set_user  },
    { "group", N_("name"),
      N_("Supplementary group to retain with the user privileges."),
      grecs_type_string, GRECS_DFLT,
      NULL, 0, set_supp_group },
    { "mode", N_("arg: [daemon|inetd]"), N_("Operation mode."),
      grecs_type_string, GRECS_DFLT,
      NULL, 0, set_mode },
    { "server-info", N_("text"),
      N_("Server description to be shown in reply to SHOW SERVER command."),
      grecs_type_string, GRECS_DFLT, &server_info,  },
    { "show-sys-info", N_("arg: acl"),
      N_("Show system information if arg matches."),
      grecs_type_string, GRECS_DFLT, &show_sys_info, 0, apply_acl_cb },
    { "identity-check", N_("arg"),
      N_("Enable identification check using AUTH protocol (RFC 1413)"),
      grecs_type_bool, GRECS_DFLT, &identity_check },
    { "ident-timeout", N_("val"),
      N_("Set timeout for AUTH I/O operations."),
      grecs_type_long, GRECS_DFLT, &ident_timeout },
    { "ident-keyfile", N_("name"),
      N_("Name of the file containing the keys for decrypting AUTH replies."),
      grecs_type_string, GRECS_DFLT, &ident_keyfile },
    { "max-children", N_("arg"),
      N_("Maximum number of children running simultaneously."),
      grecs_type_uint, GRECS_DFLT, &max_children, 0 },
    { "log-tag", N_("arg"),  N_("Tag syslog diagnostics with this tag."),
      grecs_type_string, GRECS_DFLT, &log_tag, 0 },
    { "log-facility", N_("arg"),
      N_("Set syslog facility. Arg is one of the following: user, daemon, "
	 "auth, authpriv, mail, cron, local0 through local7 "
	 "(case-insensitive), or a facility number."),
      grecs_type_string, GRECS_DFLT, NULL, 0, set_log_facility },
    { "log-print-severity", N_("arg"),
      N_("Prefix diagnostics messages with their severity."),
      grecs_type_bool, GRECS_DFLT, &log_print_severity, 0 },
    { "access-log-format", N_("fmt"),
      N_("Set format string for access log file."),
      grecs_type_string, GRECS_DFLT, &access_log_format, },
    { "access-log-file", N_("name"),
      N_("Set access log file name."),
      grecs_type_string, GRECS_DFLT, &access_log_file },
    { "transcript", N_("arg"), N_("Log session transcript."),
      grecs_type_bool, GRECS_DFLT, &transcript },
    { "pidfile", N_("name"),
      N_("Store PID of the master process in this file."),
      grecs_type_string, GRECS_DFLT, &pidfile_name, },
    { "shutdown-timeout", N_("seconds"),
      N_("Wait this number of seconds for all children to terminate."),
      grecs_type_uint, GRECS_DFLT, &shutdown_timeout },
    { "inactivity-timeout", N_("seconds"),
      N_("Set inactivity timeout."),
      grecs_type_uint, GRECS_DFLT, &inactivity_timeout },
    { "listen", N_("addr"), N_("Listen on these addresses."),
      grecs_type_sockaddr, GRECS_LIST, &listen_addr, 0, cb_dico_sockaddr_list },
    { "initial-banner-text", N_("text"),
      N_("Display this text in the initial 220 banner"),
      grecs_type_string, GRECS_DFLT, &initial_banner_text },
    { "help-text", N_("text"),
      N_("Display this text in reply to the HELP command. If text "
	 "begins with a +, usual command summary is displayed before it."),
      grecs_type_string, GRECS_DFLT, &help_text },
    { "hostname", N_("name"), N_("Override the host name."),
      grecs_type_string, GRECS_DFLT, &hostname },
    { "capability", N_("arg"), N_("Request additional capabilities."),
      grecs_type_string, GRECS_LIST,
      NULL, 0, enable_capability },
    { "module-load-path", N_("path"),
      N_("List of directories searched for database modules."),
      grecs_type_string, GRECS_LIST,
      &module_load_path, 0, cb_dico_list },
    { "prepend-load-path", N_("path"),
      N_("List of directories searched for database modules prior to "
	 "the default module directory"),
      grecs_type_string, GRECS_LIST,
      &prepend_load_path, 0, cb_dico_list },
    { "default-strategy", N_("name"),
      N_("Set the name of the default matching strategy."),
      grecs_type_string, GRECS_DFLT, &default_strategy_name },
    { "timing", N_("arg"),
      N_("Provide timing information after successful completion of an "
	 "operation."),
      grecs_type_bool, GRECS_DFLT, &timing_option },
    { "visibility-acl", N_("arg: acl"),
      N_("Set ACL to control visibility of all databases."),
      grecs_type_string, GRECS_DFLT, &global_acl, 0, apply_acl_cb },
    { "connection-acl", N_("arg: acl"),
      N_("Apply this ACL to incoming connections."),
      grecs_type_string, GRECS_DFLT, &connect_acl, 0, apply_acl_cb },
    { "database", NULL, N_("Define a dictionary database."),
      grecs_type_section, GRECS_DFLT, NULL, 0, set_database, NULL,
      kwd_database },
    { "load-module",
      N_("name: string"),
      N_("Load a module instance.\n"
	 "If <name> is the same as <arg> in command, it can be simplified as:\n"
	 "  load-module <name: string>;\n"
	 "Furthermore, several simplified forms can be combined into one\n"
	 "by using a list of names as its argument, as in:\n"
	 "  load-module (stratall,word);\n"),
      grecs_type_section, GRECS_DFLT, NULL, 0, load_module_cb, NULL,
      kwd_load_module },
    { "acl", N_("name: string"), N_("Define an ACL."),
      grecs_type_section, GRECS_DFLT, NULL, 0, acl_cb, NULL, kwd_acl },
    { "user-db", N_("url: string"),
      N_("Define user database for authentication."),
      grecs_type_section, GRECS_DFLT, &user_db_cfg, 0, user_db_config, NULL,
      kwd_user_db },
    { "alias", N_("name: string"), N_("Define a command alias."),
      grecs_type_string, GRECS_DFLT, NULL, 0, alias_cb, },
#ifdef WITH_GSASL
    { "sasl", NULL,
      N_("Control SASL authentication."),
      grecs_type_section, GRECS_DFLT, NULL, 0, sasl_cb, NULL, kwd_sasl },
#endif
    { "strategy", N_("name: string"),
      N_("Additional configuration for strategy <name>"),
      grecs_type_section, GRECS_DFLT,
      NULL, 0, strategy_cb, NULL, kwd_strategy },
    { NULL }
};

void
config_init()
{
    grecs_include_path_setup (DEFAULT_VERSION_INCLUDE_DIR,
			      DEFAULT_INCLUDE_DIR, NULL);
    grecs_preprocessor = DEFAULT_PREPROCESSOR;
    grecs_log_to_stderr = 1;
    grecs_default_port = htons(DICO_DICT_PORT);
}

void
config_parse()
{
    struct grecs_node *tree;

    tree = grecs_parse (config_file);
    if (!tree)
	exit (EX_CONFIG);
    if (grecs_tree_process (tree, keywords) || grecs_error_count)
	exit (EX_CONFIG);
    grecs_tree_free (tree);
}

void
config_help()
{
    static char docstring[] =
	N_("Configuration file structure for dicod.\n"
	   "For more information, use `info dico configuration'.");
    grecs_print_docstring (docstring, 0, stdout);
    grecs_print_statement_array (keywords, 1, 0, stdout);
}


int
show_sys_info_p()
{
    if (!show_sys_info)
	return 1;
    return dicod_acl_check(show_sys_info, 1);
}

void
reset_db_visibility()
{
    dicod_database_t *db;
    dico_iterator_t itr;

    itr = xdico_list_iterator(database_list);
    for (db = dico_iterator_first(itr); db; db = dico_iterator_next(itr)) 
	db->visible = 1;
    dico_iterator_destroy(&itr);
}

void
check_db_visibility()
{
    dicod_database_t *db;
    dico_iterator_t itr;
    int global = dicod_acl_check(global_acl, 1);
    
    itr = xdico_list_iterator(database_list);
    for (db = dico_iterator_first(itr); db; db = dico_iterator_next(itr)) {
	if (!dicod_acl_check(db->acl, global))
	    db->visible = 0;
	else {
	    dico_list_t list[2];
	    dicod_get_database_languages(db, list);
	    db->visible = dicod_lang_check(list);
	}
    }
    dico_iterator_destroy(&itr);
}


dicod_database_t *
find_database(const char *name)
{
    return dico_list_locate(database_list, (void*) name);
}

static int
_count_databases(void *item, void *data)
{
    const dicod_database_t *db = item;
    size_t *pcount = data;
    if (database_visible_p(db))
	++*pcount;
    return 0;
}

size_t
database_count()
{
    size_t count = 0;
    dico_list_iterate(database_list, _count_databases, &count);
    return count;
}

int
database_iterate(dico_list_iterator_t fun, void *data)
{
    dico_iterator_t itr = xdico_list_iterator(database_list);
    dicod_database_t *db;
    int rc = 0;
    
    for (db = dico_iterator_first(itr); rc == 0 && db;
	 db = dico_iterator_next(itr)) {
	if (database_visible_p(db)) 
	    rc = fun(db, data);
    }
    dico_iterator_destroy(&itr);
    return rc;
}

/* Remove all dictionaries that depend on the given handler */
void
database_remove_dependent(dicod_module_instance_t *inst)
{
    dico_iterator_t itr = xdico_list_iterator(database_list);
    dicod_database_t *dp;

    for (dp = dico_iterator_first(itr); dp; dp = dico_iterator_next(itr)) {
	if (dp->instance == inst) {
	    dico_log(L_NOTICE, 0, _("removing database %s"), dp->name);
	    dico_iterator_remove_current(itr, NULL);
	    dicod_database_free(dp); 
	}
    }
    dico_iterator_destroy(&itr);
}

void
dicod_database_free(dicod_database_t *dp)
{
    dico_list_destroy(&dp->langlist[0]);
    dico_list_destroy(&dp->langlist[1]);
    dico_argcv_free(dp->argc, dp->argv);
    /* FIXME: Command can point either to argv[0] or to an allocated
       string. In the latter case it is not freed. */
    free(dp);
}


void
syslog_log_printer(int lvl, int exitcode, int errcode,
                   const char *fmt, va_list ap)
{
    char *s;
    int prio = LOG_INFO;
    static struct {
        char *prefix;
        int priority;
    } loglevels[] = {
        { "Debug",  LOG_DEBUG },
        { "Info",   LOG_INFO },
        { "Notice", LOG_NOTICE },
        { "Warning",LOG_WARNING },
        { "Error",  LOG_ERR },
        { "CRIT",   LOG_CRIT },
        { "ALERT",  LOG_ALERT },
        { "EMERG",  LOG_EMERG },
    };
    char buf[512];
    int level = 0;

    if (lvl & L_CONS)
        _dico_stderr_log_printer(lvl, exitcode, errcode, fmt, ap);

    s    = loglevels[lvl & L_MASK].prefix;
    prio = loglevels[lvl & L_MASK].priority;

    if (log_print_severity)
	level += snprintf(buf + level, sizeof(buf) - level, "%s: ", s);
    level += vsnprintf(buf + level, sizeof(buf) - level, fmt, ap);
    if (errcode)
        level += snprintf(buf + level, sizeof(buf) - level, ": %s",
                          strerror(errcode));
    syslog(prio, "%s", buf);
}


static void
_print_diag(grecs_locus_t const *locus, int err, int errcode, const char *msg)
{
    if (locus) {
	char *locstr = NULL;
	size_t size = 0;

	if (locus->beg.col == 0)
	    grecs_asprintf(&locstr, &size, "%s:%u",
			   locus->beg.file,
			   locus->beg.line);
	else if (strcmp(locus->beg.file, locus->end.file))
	    grecs_asprintf(&locstr, &size, "%s:%u.%u-%s:%u.%u",
			   locus->beg.file,
			   locus->beg.line, locus->beg.col,
			   locus->end.file,
			   locus->end.line, locus->end.col);
	else if (locus->beg.line != locus->end.line)
	    grecs_asprintf(&locstr, &size, "%s:%u.%u-%u.%u",
			   locus->beg.file,
			   locus->beg.line, locus->beg.col,
			   locus->end.line, locus->end.col);
	else
	    grecs_asprintf(&locstr, &size, "%s:%u.%u-%u",
			   locus->beg.file,
			   locus->beg.line, locus->beg.col,
			   locus->end.col);
	dico_log (err ? L_ERR : L_WARN, errcode,
		  "%s: %s", locstr, msg);
	free (locstr);
    } else
	dico_log (err ? L_ERR : L_WARN, errcode, "%s", msg);
}

void
dicod_log_setup()
{
    if (!log_to_stderr) {
	openlog(log_tag, LOG_PID, log_facility);
	dico_set_log_printer(syslog_log_printer);
    }
    grecs_print_diag_fun = _print_diag;
}    

/* When requested restart by a SIGHUP, the daemon first starts
   a copy of itself with the `--lint' option to verify
   configuration settings.  This subsidiary process should use
   the same logging parameters as its ancestor.  In order to ensure
   that, the logging settings are passed to the lint child using
   __DICTD_LOGGING__ environment variable. */

void
dicod_log_encode_envar()
{
    char *p;
    asprintf(&p, "%d:%d:%s", log_facility, log_print_severity, log_tag);
    setenv(DICTD_LOGGING_ENVAR, p, 1);
}

void
dicod_log_pre_setup()
{
    char *str = getenv(DICTD_LOGGING_ENVAR);
    if (str) {
	char *p;
	log_facility = strtoul(str, &p, 10);
	if (*p == ':') {
	    log_print_severity = strtoul(p + 1, &p, 10);
	    if (*p == ':') 
		log_tag = p + 1;
	}
	log_to_stderr = 0;
	dicod_log_setup();
    }
}


void
init_conf_override(struct dicod_conf_override *ovr)
{
    ovr->transcript = -1;
}

void
apply_conf_override(struct dicod_conf_override *ovr)
{
    if (ovr->transcript >= 0)
	transcript = ovr->transcript;
}


static void
udb_init()
{
    dico_udb_define(&text_udb_def);
}



int
main(int argc, char **argv)
{
    int rc = 0;
    struct dicod_conf_override ovr;

    appi18n_init();
    dico_set_program_name(argv[0]);
    set_quoting_style(NULL, escape_quoting_style);
    log_tag = dico_program_name;
    dicod_log_pre_setup();
    debug_stream = dico_dbg_stream_create();
    hostname = xdico_local_hostname();
    dicod_init_command_tab();
    dicod_init_strategies();
    udb_init();
    register_auth();
    register_mime();
    register_lang();
    register_markup();
    register_xidle();
    register_xversion();
    register_lev();
    register_regex();

    dico_argcv_quoting_style = dico_argcv_quoting_hex;

    config_init();
    init_conf_override(&ovr);

    get_options(argc, argv, &ovr);
    if (!config_lint_option)
	dicod_log_setup();
    
    if (mode == MODE_PREPROC)
	exit(grecs_preproc_run(config_file, grecs_preprocessor) ?
	      EX_CONFIG : 0);

    config_parse();
    /* Logging settings may have been changed by the config, so setup
       it again. */
    dicod_log_setup();

    apply_conf_override(&ovr);
    
    compile_access_log();

    dicod_loader_init();
    
    begin_timing("server");
    dicod_server_init();
    init_user_db();
    register_sasl();
    flush_strat_forward();
    if (default_strategy_name
	&& dico_set_default_strategy(default_strategy_name)) {
	dico_die(EX_UNAVAILABLE, L_ERR, 0, _("unknown strategy"));
    }
    if (dicod_capa_flush())
	exit(EX_SOFTWARE);
    if (config_lint_option)
	exit(EX_OK);
	
    markup_flush_capa();

    switch (mode) {
    case MODE_DAEMON:
	dicod_server(argc, argv);
	break;
	
    case MODE_INETD:
	if (dicod_inetd())
	    rc = EX_UNAVAILABLE;
	dicod_server_cleanup();
	break;
    }
    return rc;
}
