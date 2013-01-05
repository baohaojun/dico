/* This file is part of GNU Dico
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
#include <xdico.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>
#include <hash.h>
#include <ctype.h>
#include <xalloc.h>

struct timer_slot {
    char *name;
    double real;
    double self_user;		/* user time in sec */
    double self_system;		/* system time in sec */
    double children_user;	/* user time in sec */
    double children_system;	/* system time in sec */
    struct timeval real_mark;
    struct rusage  self_mark;
    struct rusage  children_mark;
};


static Hash_table *timer_table;

static size_t
hash_string_ci (const char *string, size_t n_buckets)
{
    size_t value = 0;
    unsigned char ch;
    
    for (; (ch = *string); string++)
	value = (value * 31 + tolower(ch)) % n_buckets;
    return value;
}

/* Calculate the hash of a string.  */
static size_t
timer_hasher(void const *data, unsigned n_buckets)
{
    const struct timer_slot *t = data;
    return hash_string_ci(t->name, n_buckets);
}

/* Compare two strings for equality.  */
static bool
timer_compare(void const *data1, void const *data2)
{
    const struct timer_slot *t1 = data1;
    const struct timer_slot *t2 = data2;
    return strcasecmp(t1->name, t2->name) == 0;
}

/* Allocate a timer structure */
static xdico_timer_t
timer_alloc(const char *name)
{
    xdico_timer_t t = xmalloc(sizeof(*t) + strlen(name) + 1);
    memset(t, 0, sizeof(*t));
    t->name = (char*)(t + 1);
    strcpy(t->name, name);
    return t;
}

/* Lookup a timer by its name. If it does not exist, create it. */
xdico_timer_t
timer_get(const char *name)
{
    xdico_timer_t tp, ret;

    tp = timer_alloc(name);

    if (! ((timer_table
	    || (timer_table = hash_initialize(0, 0, 
					      timer_hasher,
					      timer_compare, 0)))
	   && (ret = hash_insert(timer_table, tp))))
	xalloc_die ();

    if (ret != tp) 
	free(tp);
    return ret;
}

xdico_timer_t
timer_start(const char *name)
{
    xdico_timer_t t = timer_get(name);

    gettimeofday(&t->real_mark, NULL);
    getrusage(RUSAGE_SELF, &t->self_mark);
    getrusage(RUSAGE_CHILDREN, &t->children_mark);
    return t;
}

#define DIFFTIME(now,then)\
   (((now).tv_sec - (then).tv_sec) \
    + ((double)((now).tv_usec - (then).tv_usec))/1000000)

static void
_timer_compute(xdico_timer_t t)
{
    struct timeval  real;
    struct rusage   rusage;

    gettimeofday(&real, NULL);
    t->real   = DIFFTIME(real, t->real_mark);
    getrusage(RUSAGE_SELF, &rusage);
    t->self_user   = DIFFTIME(rusage.ru_utime, t->self_mark.ru_utime);
    t->self_system = DIFFTIME(rusage.ru_stime, t->self_mark.ru_stime);
   
    getrusage(RUSAGE_CHILDREN, &rusage);
    t->children_user = DIFFTIME(rusage.ru_utime, t->children_mark.ru_utime);
    t->children_system = DIFFTIME(rusage.ru_stime, t->children_mark.ru_stime);
}

xdico_timer_t
timer_stop(const char *name)
{
    xdico_timer_t t = timer_get(name);
    _timer_compute(t);
    return t;
}

xdico_timer_t
timer_reset(const char *name)
{
    xdico_timer_t t = timer_get(name);
    t->real            = 0.0;
    t->self_user       = 0.0;
    t->self_system     = 0.0;
    t->children_user   = 0.0;
    t->children_system = 0.0;
    return t;
}

double
timer_get_real(xdico_timer_t t)
{
    return t->real;
}

double
timer_get_user(xdico_timer_t t)
{
    return t->self_user + t->children_user;
}

double
timer_get_system(xdico_timer_t t)
{
    return t->self_system + t->children_system;
}

void
timer_format_time(dico_stream_t stream, double t)
{
    static char buf[128];

    if (t < 600) 
	snprintf(buf, sizeof(buf), "%0.3f", t );
    else {
	long int    s, m, h, d;

	s = (long int)t;
	d = s / (3600*24);
	s -= d * 3600 * 24;
	h = s / 3600;
	s -= h * 3600;
	m = s / 60;
	s -= m * 60;

	if (d)
	    snprintf(buf, sizeof(buf), "%ld+%02ld:%02ld:%02ld", d, h, m, s);
	else if (h)
	    snprintf(buf, sizeof(buf), "%02ld:%02ld:%02ld", h, m, s);
	else
	    snprintf(buf, sizeof(buf), "%02ld:%02ld", m, s);
    }
    dico_stream_write(stream, buf, strlen(buf));
}
    
    
