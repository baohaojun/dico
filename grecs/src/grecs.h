/* grecs - Gray's Extensible Configuration System
   Copyright (C) 2007-2012 Sergey Poznyakoff

   Grecs is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   Grecs is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with Grecs. If not, see <http://www.gnu.org/licenses/>. */

#ifndef _GRECS_H
#define _GRECS_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#if ENABLE_NLS
# include "gettext.h"
#else
# ifndef gettext
#  define gettext(msgid) msgid
# endif
#endif
#ifndef _
# define _(msgid) gettext(msgid)
#endif
#ifndef N_
# define N_(s) s
#endif

#define GRECS_VERSION_MAJOR 1
#define GRECS_VERSION_MINOR 0

struct grecs_version_info {
	const char *package;
	const char *version;
	const char *id;
	int major;
	int minor;
	int patch;
	char *suffix;
	char *buffer;
};

struct grecs_locus_point {
	char *file;
	unsigned line;
	unsigned col;
};

#define grecs_locus_point_advance_line(loc) do {	\
		(loc).line++;				\
		(loc).col = 0;				\
	} while (0)

#define GRECS_LOCUS_POINT_EQ(a,b) \
	((strcmp((a)->file, (b)->file) == 0) && ((a)->line == (b)->line))

typedef struct grecs_locus {
	struct grecs_locus_point beg;
	struct grecs_locus_point end;
} grecs_locus_t;

extern grecs_locus_t grecs_locus;
extern int grecs_adjust_string_locations;

enum grecs_data_type {
	grecs_type_void,
	grecs_type_string,
	grecs_type_short,
	grecs_type_ushort,
	grecs_type_int,
	grecs_type_uint,
	grecs_type_long,
	grecs_type_ulong,
	grecs_type_size,
/*    grecs_type_off,*/
	grecs_type_time,
	grecs_type_bool,
	grecs_type_ipv4,
	grecs_type_cidr,
	grecs_type_host,
	grecs_type_sockaddr,
	grecs_type_section
};

#define GRECS_DFLT 0x00
#define GRECS_AGGR 0x01
#define GRECS_MULT 0x02
#define GRECS_INAC 0x04
#define GRECS_LIST 0x08

enum grecs_callback_command {
	grecs_callback_section_begin,
	grecs_callback_section_end,
	grecs_callback_set_value
};

#define GRECS_TYPE_STRING 0
#define GRECS_TYPE_LIST   1
#define GRECS_TYPE_ARRAY  2

struct grecs_list_entry {
	struct grecs_list_entry *next, *prev;
	void *data;
};

struct grecs_list {
	struct grecs_list_entry *head, *tail;
	size_t count;
	int (*cmp)(const void *, const void *);
	void (*free_entry)(void *);
};

typedef struct grecs_value {
	int type;
	grecs_locus_t locus;
	union {
		struct grecs_list *list;
		char *string;
		struct {
			size_t c;
			struct grecs_value **v;
		} arg;
	} v;
} grecs_value_t;

#define GRECS_VALUE_EMPTY_P(val) \
	(!(val) ||						\
	 ((val)->type == GRECS_TYPE_STRING && (val)->v.string == NULL))

enum grecs_node_type {
	grecs_node_root,
	grecs_node_stmt,
	grecs_node_block
};

typedef struct grecs_node {
	enum grecs_node_type type;
	grecs_locus_t locus;
	struct grecs_node *up;
	struct grecs_node *down;
	struct grecs_node *next;
	struct grecs_node *prev;
	char *ident;
	grecs_locus_t idloc;
	union {
		struct grecs_value *value;
		struct grecs_symtab *texttab;
	} v;
} grecs_node_t;

typedef int (*grecs_callback_fn)(
	enum grecs_callback_command cmd,
	grecs_locus_t *    /* locus */,
	void *             /* varptr */,
	grecs_value_t *    /* value */,
	void *             /* cb_data */
	);

struct grecs_keyword {
	const char *ident;
	const char *argname;
	const char *docstring;
	enum grecs_data_type type;
	int flags;
	void *varptr;
	size_t offset;
	grecs_callback_fn callback;
	void *callback_data;
	struct grecs_keyword *kwd;
};

struct grecs_sockaddr {
	int len;
	struct sockaddr *sa;
};

struct grecs_version_info *grecs_version(void);
int grecs_version_cmp(const char *vstr1, const char *vstr2, int *pres);
int grecs_version_ok(const char *vstr);
void grecs_version_info_free(struct grecs_version_info *pv);
struct grecs_version_info *grecs_version_split(const char *vstr);
int grecs_version_info_cmp(struct grecs_version_info *vx,
			   struct grecs_version_info *vy,
			   int *pres);

extern void *(*grecs_malloc_fun)(size_t size);
extern void *(*grecs_realloc_fun)(void *ptr, size_t size);
extern void (*grecs_alloc_die_fun)(void);
extern void (*grecs_free_fun)(void *ptr);

void *grecs_malloc(size_t size);
void *grecs_zalloc(size_t size);
void *grecs_calloc(size_t nmemb, size_t size);
void *grecs_realloc(void *ptr, size_t size);
void grecs_alloc_die(void);
char *grecs_strdup(const char *str);
void grecs_free(void *ptr);

grecs_value_t *grecs_value_ptr_from_static(grecs_value_t *input);

extern void (*grecs_print_diag_fun)(grecs_locus_t const *, int, int, const char*);

void grecs_warning(grecs_locus_t const *locus,
		   int errcode, const char *fmt, ...)
	__attribute__ ((__format__ (__printf__, 3, 4)));
void grecs_error(grecs_locus_t const *locus, int errcode,
		 const char *fmt, ...)
	__attribute__ ((__format__ (__printf__, 3, 4)));


extern int grecs_trace_flags;

#define GRECS_TRACE_GRAM 0x01
#define GRECS_TRACE_LEX  0x02
void grecs_gram_trace(int n);
void grecs_lex_trace(int n);

void grecs_parse_line_directive(char *text, grecs_locus_t *ploc,
				struct grecs_locus_point *ppoint, 
				size_t *pxlines);
void grecs_parse_line_directive_cpp(char *text, grecs_locus_t *ploc,
				    struct grecs_locus_point *ppoint, 
				    size_t *pxlines);



int grecs_lex_begin(const char*, int);  
void grecs_lex_end(int err);
struct grecs_node *grecs_parse(const char *name);

typedef struct grecs_node *(*grecs_parser_t)(const char *name, int trace);

extern grecs_parser_t grecs_parser_fun;

/* Parsers: */
struct grecs_node *grecs_grecs_parser(const char *name, int traceflags);
struct grecs_node *grecs_meta1_parser(const char *name, int traceflags);
struct grecs_node *grecs_bind_parser(const char *name, int traceflags);
struct grecs_node *grecs_git_parser(const char *name, int traceflags);
struct grecs_node *grecs_path_parser(const char *name, int traceflags);


/* Parser database */
int grecs_enumerate_parsers(int (*fun)(const char *, grecs_parser_t, void *),
			    void *);
grecs_parser_t grecs_get_parser_by_type(const char *type);


struct grecs_list *_grecs_simple_list_create(int dispose);
struct grecs_list *grecs_value_list_create(void);

void grecs_line_acc_create(void);
void grecs_line_acc_free(void);
void grecs_line_acc_grow_char(int c);
void grecs_line_acc_grow_char_unescape(int c);
void grecs_line_acc_grow(const char *text, size_t len);
void grecs_line_acc_grow_unescape_last(char *text, size_t len);

void grecs_line_begin(void);
#define grecs_line_add grecs_line_acc_grow
char *grecs_line_finish(void);

extern int grecs_string_convert(void *target, enum grecs_data_type type,
				const char *string,
				grecs_locus_t const *locus);
extern void grecs_process_ident(struct grecs_keyword *kwp,
				grecs_value_t *value,
				void *base,
				grecs_locus_t *locus);

struct grecs_node *grecs_node_create(enum grecs_node_type type,
				     grecs_locus_t *loc);
struct grecs_node *grecs_node_create_points(enum grecs_node_type type,
					    struct grecs_locus_point beg,
					    struct grecs_locus_point end);
void grecs_node_bind(struct grecs_node *master, struct grecs_node *node,
		     int dn);
int grecs_node_eq(struct grecs_node *a, struct grecs_node *b);


extern struct grecs_locus_point grecs_current_locus_point;
extern int grecs_error_count;    
extern int grecs_default_port;

extern const char *grecs_preprocessor;
extern int grecs_log_to_stderr;
extern void (*grecs_log_setup_hook)();

size_t grecs_preproc_fill_buffer(char *buf, size_t size);
void grecs_preproc_add_include_dir(char *dir);
int grecs_preproc_init(const char *name);
void grecs_preproc_done(void);
int grecs_preproc_run(const char *config_file, const char *extpp);

#define GRECS_STD_INCLUDE 0x01
#define GRECS_USR_INCLUDE 0x02

size_t grecs_include_path_count(int flag);
int grecs_foreach_include_dir(int flag, int (*fun)(int, const char *, void *),
			      void *data);

char *grecs_find_include_file(const char *name, int allow_cwd);

FILE *grecs_preproc_extrn_start(const char *file, pid_t *ppid);
void grecs_preproc_extrn_shutdown(pid_t pid);

char *grecs_install_text(const char *str);
void grecs_destroy_text(void);
struct grecs_symtab *grecs_text_table(void);

void grecs_include_path_clear(void);
void grecs_include_path_setup(const char *dir, ...);
void grecs_include_path_setup_v(char **dirs);

ssize_t grecs_getline(char **pbuf, size_t *psize, FILE *fp);

const char *grecs_data_type_string(enum grecs_data_type type);
void grecs_print_docstring(const char *docstring, unsigned level,
			   FILE *stream);
void grecs_print_simple_statement(struct grecs_keyword *kwp,
				  unsigned level, FILE *stream);
void grecs_print_block_statement(struct grecs_keyword *kwp,
				 unsigned level, FILE *stream);
void grecs_print_statement_array(struct grecs_keyword *kwp,
				 unsigned n,
				 unsigned level, FILE *stream);

struct grecs_format_closure
{
	int (*fmtfun)(const char *, void *);
	void *data;
};

void grecs_format_locus(grecs_locus_t *locus, struct grecs_format_closure *fp);
void grecs_format_node_path(struct grecs_node *node, int flag,
			    struct grecs_format_closure *fp);
void grecs_format_value(struct grecs_value *val, int flags,
			struct grecs_format_closure *fp);

#define GRECS_NODE_FLAG_PATH      0x00100
#define GRECS_NODE_FLAG_VALUE     0x00200
#define GRECS_NODE_FLAG_DESCEND   0x01000
#define GRECS_NODE_FLAG_LOCUS     0x02000
#define GRECS_NODE_FLAG_QUOTE     0x04000
#define GRECS_NODE_FLAG_NOQUOTE   0x08000
#define GRECS_NODE_FLAG_QUOTE_HEX 0x10000

#define _GRECS_NODE_MASK_DELIM    0x000ff
#define _GRECS_NODE_MASK_OUTPUT   0x00f00

#define GRECS_NODE_FLAG_DEFAULT \
	(GRECS_NODE_FLAG_PATH|GRECS_NODE_FLAG_VALUE|\
	 GRECS_NODE_FLAG_DESCEND|GRECS_NODE_FLAG_QUOTE)
int grecs_format_node(struct grecs_node *node, int flags,
		      struct grecs_format_closure *fp);

void grecs_print_locus(grecs_locus_t *locus, FILE *fp);
void grecs_print_node_path(struct grecs_node *node, int flag, FILE *fp);
void grecs_print_value(struct grecs_value *val, int flags, FILE *fp);

int grecs_print_node(struct grecs_node *node, int flags, FILE *fp);

struct grecs_txtacc;
void grecs_txtacc_format_value(struct grecs_value *val, int flags,
			       struct grecs_txtacc *acc);



struct grecs_list *grecs_list_create(void);
size_t grecs_list_size(struct grecs_list *lp);
void grecs_list_append(struct grecs_list *lp, void *val);
void grecs_list_push(struct grecs_list *lp, void *val);
void *grecs_list_pop(struct grecs_list *lp);
void *grecs_list_locate(struct grecs_list *lp, void *data);
void *grecs_list_index(struct grecs_list *lp, size_t idx);
void *grecs_list_remove_tail(struct grecs_list *lp);
void grecs_list_remove_entry(struct grecs_list *lp,
			     struct grecs_list_entry *ent);
void grecs_list_clear(struct grecs_list *lp);
void grecs_list_free(struct grecs_list *lp);
void grecs_list_add(struct grecs_list *dst, struct grecs_list *src);

int grecs_vasprintf(char **pbuf, size_t *psize, const char *fmt, va_list ap);
int grecs_asprintf(char **pbuf, size_t *psize, const char *fmt, ...);

#define GRECS_TXTACC_BUFSIZE 1024
struct grecs_txtacc *grecs_txtacc_create(void);
void grecs_txtacc_free(struct grecs_txtacc *acc);
void grecs_txtacc_grow(struct grecs_txtacc *acc, const char *buf, size_t size);
#define grecs_txtacc_grow_char(acc,c)		\
	do {					\
		char __ch = c;			\
		grecs_txtacc_grow(acc,&__ch,1);	\
	} while (0)
char *grecs_txtacc_finish(struct grecs_txtacc *acc, int steal);
void grecs_txtacc_free_string(struct grecs_txtacc *acc, char *str);

struct grecs_symtab;

struct grecs_syment {
	char *name;
};

typedef int (*grecs_symtab_enumerator_t)(void *sym, void *data);

const char *grecs_symtab_strerror(int rc);

void *grecs_symtab_lookup_or_install(struct grecs_symtab *st,
				     void *key, int *install);
void grecs_symtab_clear(struct grecs_symtab *st);
struct grecs_symtab *grecs_symtab_create(size_t elsize, 
			     unsigned (*hash_fun)(void *, unsigned long),
			     int (*cmp_fun)(const void *, const void *),
			     int (*copy_fun)(void *, void *),
			     void *(*alloc_fun)(size_t),
			     void (*free_fun)(void *));
struct grecs_symtab *grecs_symtab_create_default(size_t elsize);

void grecs_symtab_free(struct grecs_symtab *pst);
int grecs_symtab_remove(struct grecs_symtab *st, void *elt);
int grecs_symtab_replace(struct grecs_symtab *st, void *ent, void **old_ent);
int grecs_symtab_enumerate(struct grecs_symtab *st,
			   grecs_symtab_enumerator_t fun, void *data);

size_t grecs_symtab_count_entries(struct grecs_symtab *st);

unsigned grecs_hash_string(const char *name, unsigned long hashsize);


void grecs_value_free(struct grecs_value *val);
void grecs_node_free(struct grecs_node *node);
int grecs_tree_free(struct grecs_node *node);

enum grecs_tree_recurse_op {
	grecs_tree_recurse_set,
	grecs_tree_recurse_pre,
	grecs_tree_recurse_post
};

enum grecs_tree_recurse_res {
	grecs_tree_recurse_ok,
	grecs_tree_recurse_fail,
	grecs_tree_recurse_skip,
	grecs_tree_recurse_stop
};
	
typedef enum grecs_tree_recurse_res
       (*grecs_tree_recursor_t)(enum grecs_tree_recurse_op,
				struct grecs_node *, void *);

int grecs_tree_recurse(struct grecs_node *node, grecs_tree_recursor_t recfun,
		       void *data);
int grecs_tree_join(struct grecs_node *dst, struct grecs_node *src);


int grecs_tree_process(struct grecs_node *node, struct grecs_keyword *kwd);

typedef struct grecs_match_buf *grecs_match_buf_t;
struct grecs_node *grecs_match_first(struct grecs_node *tree,
				     const char *pattern,
				     grecs_match_buf_t *buf);
struct grecs_node *grecs_match_next(struct grecs_match_buf *buf);
void grecs_match_buf_free(struct grecs_match_buf *buf);

int grecs_value_eq(struct grecs_value *a, struct grecs_value *b);
int grecs_value_match(struct grecs_value *pat, struct grecs_value *b,
		      int flags);

struct grecs_node *grecs_find_node(struct grecs_node *node, const char *path);

struct grecs_node *grecs_node_from_path(const char *path, const char *value);
struct grecs_node *grecs_node_from_path_locus(const char *path,
					      const char *value,
					      grecs_locus_t *locus,
					      grecs_locus_t *vallocus);
int grecs_tree_reduce(struct grecs_node *node, struct grecs_keyword *kwd,
		      int flags);

void  grecs_tree_sort(struct grecs_node *node,
		      int (*compare)(struct grecs_node const *,
				     struct grecs_node const *));

struct grecs_node *grecs_tree_first_node(struct grecs_node *tree);
struct grecs_node *grecs_next_node(struct grecs_node *node);

#endif
