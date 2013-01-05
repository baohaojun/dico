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

#ifndef __dico_types_h
#define __dico_types_h

#include <sys/types.h>

#ifndef offsetof
# define offsetof(s,f) ((size_t)&((s*)0)->f)
#endif
#define DICO_ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

#ifndef DICO_ARG_UNUSED
# define DICO_ARG_UNUSED __attribute__ ((__unused__))
#endif

#ifndef DICO_PRINTFLIKE
# define DICO_PRINTFLIKE(fmt,narg) __attribute__ ((__format__ (__printf__, fmt, narg)))
#endif

#define DICO_DICT_PORT 2628
/* Maximum size of I/O buffer as per RFC2229 */
#define DICO_MAX_BUFFER 6144

#define __dico_s_cat3__(a,b,c) a ## b ## c
#define DICO_EXPORT(module,name) __dico_s_cat3__(module,_LTX_,name)

typedef struct dico_line_buffer *dico_linebuf_t;
typedef struct dico_stream *dico_stream_t;
typedef struct dico_list *dico_list_t;
typedef struct dico_assoc_list *dico_assoc_list_t;
typedef struct iterator *dico_iterator_t;

typedef struct dico_handle_struct *dico_handle_t;
typedef struct dico_result_struct *dico_result_t;

typedef struct dico_strategy *dico_strategy_t;
typedef struct dico_key *dico_key_t;

#define DICO_SELECT_BEGIN 0
#define DICO_SELECT_RUN   1
#define DICO_SELECT_END   2
typedef int (*dico_select_t) (int, dico_key_t, const char *);

#define DICO_MODULE_VERSION 1

#define DICO_CAPA_NONE 0
#define DICO_CAPA_NODB 0x0001
#define DICO_CAPA_DEFAULT DICO_CAPA_NONE

struct dico_database_module {
    unsigned dico_version;
    unsigned dico_capabilities;
    int (*dico_init) (int argc, char **argv);
    dico_handle_t (*dico_init_db) (const char *db, int argc, char **argv);
    int (*dico_free_db) (dico_handle_t hp);
    int (*dico_open) (dico_handle_t hp);
    int (*dico_close) (dico_handle_t hp);
    char *(*dico_db_info) (dico_handle_t hp);
    char *(*dico_db_descr) (dico_handle_t hp);
    int (*dico_db_lang) (dico_handle_t hp, dico_list_t list[2]);
    dico_result_t (*dico_match) (dico_handle_t hp,
				 const dico_strategy_t strat,
				 const char *word);
    dico_result_t (*dico_define) (dico_handle_t hp, const char *word);
    int (*dico_output_result) (dico_result_t rp, size_t n,
			       dico_stream_t str);
    size_t (*dico_result_count) (dico_result_t rp);
    size_t (*dico_compare_count) (dico_result_t rp);
    void (*dico_free_result) (dico_result_t rp);
    int (*dico_result_headers) (dico_result_t rp, dico_assoc_list_t hdr);
};

#endif
