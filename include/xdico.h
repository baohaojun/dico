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

#ifndef __xdico_h
#define __xdico_h

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <dico.h>

typedef unsigned long UINT4;
typedef UINT4 IPADDR;


char * ip_hostname(IPADDR ipaddr);
IPADDR get_ipaddr(char *host);
int str2port(char *str);


struct sockaddr;
void sockaddr_to_str(const struct sockaddr *sa, int salen,
		     char *bufptr, size_t buflen,
		     size_t *plen);
char *sockaddr_to_astr(const struct sockaddr *sa, int salen);


int switch_to_privs (uid_t uid, gid_t gid, dico_list_t retain_groups);


/* Utility functions */
char *make_full_file_name(const char *dir, const char *file);
void trimnl(char *buf, size_t len);

dico_list_t xdico_list_create(void);
dico_iterator_t xdico_list_iterator(dico_list_t list);
void xdico_list_append(struct dico_list *list, void *data);
void xdico_list_prepend(struct dico_list *list, void *data);
dico_assoc_list_t xdico_assoc_create(int);
void xdico_assoc_append(dico_assoc_list_t assoc, const char *key,
			const char *value);
int xdico_assoc_add(dico_assoc_list_t assoc, const char *key,
		    const char *value, size_t count, int replace);
char *xdico_assign_string(char **dest, char *str);

char *xdico_sasl_mech_to_capa(char *mech);
int xdico_sasl_capa_match_p(const char *mech, const char *capa);

int dicod_free_item (void *item, void *data);


/* Timer */
typedef struct timer_slot *xdico_timer_t;

xdico_timer_t timer_get(const char *name);
xdico_timer_t timer_start(const char *name);
xdico_timer_t timer_stop(const char *name);
xdico_timer_t timer_reset(const char *name);
double timer_get_real(xdico_timer_t t);
double timer_get_user(xdico_timer_t t);
double timer_get_system(xdico_timer_t t);
void timer_format_time(dico_stream_t stream, double t);

/* xstript */
dico_stream_t xdico_transcript_stream_create(dico_stream_t transport,
					     dico_stream_t logstr,
					     const char *prefix[]);

/* xstream.c */
int stream_writez(dico_stream_t str, const char *buf);
int stream_printf(dico_stream_t str, const char *fmt, ...);
void stream_write_multiline(dico_stream_t str, const char *text);

/* xtkn.c */
void xdico_tokenize_string(struct dico_tokbuf *tb, char *str);
    
/* appi18n.c */
void appi18n_init(void);

/* xhostname.c */
char *xdico_local_hostname(void);

/* xdebug.c */
extern int debug_level;
extern int debug_source_info;
extern dico_stream_t debug_stream;

#define XDICO_DEBUG(l, s)						\
    do {								\
	if (debug_level >= l) {						\
	    if (debug_source_info)					\
		DICO_DEBUG_SINFO(debug_stream);				\
	    dico_stream_write(debug_stream, s, strlen(s));		\
	}								\
    } while (0)

#define XDICO_DEBUG_F1(l,f,a1)						\
    do {								\
	if (debug_level >= l) {						\
	    if (debug_source_info)					\
		DICO_DEBUG_SINFO(debug_stream);				\
	    stream_printf(debug_stream, f, a1);				\
	}								\
    } while (0)

#define XDICO_DEBUG_F2(l,f,a1,a2)					\
    do {								\
	if (debug_level >= l) {						\
	    if (debug_source_info)					\
		DICO_DEBUG_SINFO(debug_stream);				\
	    stream_printf(debug_stream, f, a1, a2);			\
	}								\
    } while (0)

#define XDICO_DEBUG_F3(l,f,a1,a2,a3)					\
    do {								\
	if (debug_level >= l) {						\
	    if (debug_source_info)					\
		DICO_DEBUG_SINFO(debug_stream);				\
	    stream_printf(debug_stream, f, a1, a2, a3);			\
	}								\
    } while (0)

#define XDICO_DEBUG_F4(l,f,a1,a2,a3,a4)					\
    do {								\
	if (debug_level >= l) {						\
	    if (debug_source_info)					\
		DICO_DEBUG_SINFO(debug_stream);				\
	    stream_printf(debug_stream, f, a1, a2, a3, a4);		\
	}								\
    } while (0)

#endif
    
