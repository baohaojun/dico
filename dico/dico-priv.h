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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <xalloc.h>
#include <ctype.h>
#include <syslog.h>
#include <inttypes.h>
#include <limits.h>
#include <size_max.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <ltdl.h>
#include "wordsplit.h"

#include <xdico.h>
#include <inttostr.h>
#include <c-strcase.h>
#include <appi18n.h>
#define obstack_chunk_alloc malloc
#define obstack_chunk_free free
#include <obstack.h>
#include <quotearg.h>
#if defined(WITH_READLINE) && defined(HAVE_READLINE_READLINE_H)
# include <readline/readline.h>
# include <readline/history.h>
#endif

enum dico_client_mode {
    mode_define,
    mode_match,
    mode_dbs,
    mode_strats,
    mode_help,
    mode_info,
    mode_server
};

struct match_result {
    char *database;
    char *word;
};

struct define_result {
    char *word;
    char *database;
    char *descr;
    char *defn;
    size_t nlines;
};

enum dict_result_type {
    dict_result_define,
    dict_result_match,
    dict_result_text
};

struct dict_result {
    struct dict_result *prev;          /* Previous result */
    struct dict_connection *conn;      /* Associated connection */
    enum dict_result_type  type;       /* Result type */
    size_t count;                      /* Number of elements in the set */ 
    char *base;                        /* Base data pointer */
    union {                             
	struct define_result *def;
	struct match_result *mat;
    } set;                             /* Result set */ 
};

struct dict_connection {
    int fd;
    dico_stream_t str;    /* Communication stream */
    int transcript;       /* True if transcript is enabled */ 
    int capac;            /* Number of reported server capabilities */
    char **capav;         /* Array of capabilities */
    char *msgid;          /* msg-id */
    char *buf;            /* Input buffer */
    size_t size;          /* Buffer size */
    size_t level;         /* Buffer fill level */
    size_t levdist;       /* Configured Levenshtein distance */
    struct obstack stk;   /* Obstack for input data */
    struct dict_result *last_result; /* Last result */
    struct dict_result *db_result;
    struct dict_result *strat_result;
    struct dict_result *match_result;
};

struct auth_cred {
    char *user;
    char *pass;
    int sasl;
    dico_list_t mech;
    char *service;
    char *realm;
    char *hostname;
};

struct funtab {
    char *name;        /* Keyword */
    int argmin;        /* Minimal number of arguments */
    int argmax;        /* Maximal number of arguments */
    char *argdoc;      /* Argument docstring */
    char *docstring;   /* Documentation string */
    void (*fun) (int argc, char **argv); /* Handler */
    char **(*compl) (int argc, char **argv, int ws); 
};

#define DICO_CLIENT_ID PACKAGE_STRING 

extern struct dico_url dico_url;
extern struct auth_cred default_cred;
extern char *client;
extern enum dico_client_mode mode;
extern int transcript;
extern IPADDR source_addr;
extern int noauth_option;
extern unsigned levenshtein_threshold;
extern char *autologin_file;
extern int quiet_option;

void get_options (int argc, char *argv[], int *index);

/* connect.c */
void dict_transcript(struct dict_connection *conn, int state);
int dict_connect(struct dict_connection **pconn, dico_url_t url);
void dict_conn_close(struct dict_connection *conn);
int dict_read_reply(struct dict_connection *conn);
int dict_status_p(struct dict_connection *conn, const char *status);
int dict_capa(struct dict_connection *conn, char *capa);
int dict_multiline_reply(struct dict_connection *conn);
struct dict_result *dict_result_create(struct dict_connection *conn,
				       enum dict_result_type type,
				       size_t count, char *base);
void dict_result_free(struct dict_result *res);
#define dict_conn_last_result(c) ((c)->last_result)
int dict_define(struct dict_connection *conn, char *database, char *word);
int dict_match(struct dict_connection *conn, char *database, char *strategy,
	       char *word);

char *get_homedir(void);
int ds_tilde_expand(const char *str, char **output);

#define GETCRED_OK     0
#define GETCRED_FAIL   1
#define GETCRED_NOAUTH 2

int auth_cred_get(char *host, struct auth_cred *cred);
void auth_cred_free(struct auth_cred *cred);

/* lookup.c */
int dict_lookup_url(dico_url_t url);
int dict_word(char *word);
int dict_lookup(struct dict_connection *conn, dico_url_t url);
int dict_single_command(char *cmd, char *arg, char *code);
void dict_run_single_command(struct dict_connection *conn,
			     const char *cmd, const char *arg,
			     const char *code);
void print_result(struct dict_result *res);
void print_match_result(struct dict_result *res);
void print_reply(struct dict_connection *conn);

/* netrc.c */
char *skipws(char *buf);
#define AUTOLOGIN_USERNAME 0x1
#define AUTOLOGIN_PASSWORD 0x2
#define AUTOLOGIN_NOAUTH   0x4
#define AUTOLOGIN_MECH     0x8
int parse_autologin(const char *filename, char *host, struct auth_cred *pcred,
		    int *pflags);

/* shell.c */
struct funtab *find_funtab(const char *name);
void parse_init_scripts(void);
void dico_shell(void);
void script_warning(const char *fmt, ...);
void script_error(const char *fmt, ...);
char **dict_completion_matches(int argc, char **argv, int ws,
			       char *(*generator)(const char *, int));

/* func.c */
int ensure_connection(void);
int set_bool(int *pval, char *str);
void ds_silent_close(void);
void ds_open(int argc, char **argv);
void ds_close(int argc, char **argv);
void ds_autologin(int argc, char **argv);
void ds_database(int argc, char **argv);
void ds_define_nth(size_t num);
void ds_strategy(int argc, char **argv);
void ds_transcript(int argc, char **argv);
void ds_define(int argc, char **argv);
void ds_match(int argc, char **argv);
void ds_distance(int argc, char **argv);
void ds_version(int argc, char **argv);
void ds_warranty(int argc, char **argv);
void ds_show_db(int argc, char **argv);
void ds_show_strat(int argc, char **argv);
void ds_show_info(int argc, char **argv);
void ds_sasl(int argc, char **argv);
void ds_verbose(int argc, char **argv);

char **ds_compl_database(int argc, char **argv, int ws);
char **ds_compl_strategy(int argc, char **argv, int ws);

/* pager.c */
void ds_pager(int argc, char **argv);
dico_stream_t create_pager_stream(size_t nlines);

/* saslauth.c */
#define AUTH_OK   0
#define AUTH_FAIL 1
#define AUTH_CONT 2

int saslauth(struct dict_connection *conn, dico_url_t url);
void sasl_enable(int val);
int sasl_enabled_p(void);

/* cmdline.c */
void print_version(const char *program_version, FILE *stream);
void print_version_only(const char *program_version, FILE *stream);
