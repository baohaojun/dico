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

/* Implementation of a "debug stream", a write-only stream useful
   for printing debugging diagnostics.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

struct dbg_stream {
    dico_stream_t transport;  /* Transport log-stream */
    const char *file;         /* Source file */
    unsigned line;            /* Source line */
    int ts;                   /* Print timestamps */
};

static char *
fmtline(unsigned num, char *buf, size_t bufsize)
{
    char *p = buf + bufsize;
    *--p = 0;
    while (p > buf) {
	unsigned x = num % 10;
	*--p = x + '0';
	num /= 10;
	if (num == 0)
	    break;
    }
    return p;
}

static int
dbg_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct dbg_stream *p = data;

    if (p->ts) {
	char nbuf[128], *s;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	dico_stream_write(p->transport, "[", 1);
	s = fmtline(tv.tv_sec, nbuf, sizeof(nbuf));
	dico_stream_write(p->transport, s, strlen(s));
	dico_stream_write(p->transport, ".", 1);
	s = fmtline(tv.tv_usec, nbuf, sizeof(nbuf));
	dico_stream_write(p->transport, s, strlen(s));
	dico_stream_write(p->transport, "] ", 2);
    }
    
    if (p->file) {
	char *s;
	char nbuf[128];
	
	dico_stream_write(p->transport, p->file, strlen(p->file));
	dico_stream_write(p->transport, ":", 1);
	s = fmtline(p->line, nbuf, sizeof(nbuf));
	dico_stream_write(p->transport, s, strlen(s));
	dico_stream_write(p->transport, ": ", 2);
    }
    dico_stream_write(p->transport, buf, size);

    if (pret)
	*pret = size;
    return 0;
}

static int
dbg_destroy(void *data)
{
    struct dbg_stream *p = data;
    dico_stream_destroy(&p->transport);
    free(data);
    return 0;
}

static int
dbg_ioctl(void *data, int code, void *call_data)
{
    struct dbg_stream *p = data;
    switch (code) {
    case DICO_DBG_CTL_SET_FILE:
	p->file = call_data;
	break;

    case DICO_DBG_CTL_SET_LINE:
	p->line = *(unsigned*)call_data;
	break;

    case DICO_DBG_CTL_SET_TS:
	p->ts = *(int*)call_data;
	break;

    default:
	errno = EINVAL;
	return 1;
    }
    return 0;
}
	
dico_stream_t
dico_dbg_stream_create()
{
    struct dbg_stream *p = malloc(sizeof(*p));
    dico_stream_t stream;

    if (!p || dico_stream_create(&stream, DICO_STREAM_WRITE, p))
	return NULL;
    dico_stream_set_write(stream, dbg_write);
    dico_stream_set_destroy(stream, dbg_destroy);
    dico_stream_set_ioctl(stream, dbg_ioctl);
    dico_stream_set_buffer(stream, dico_buffer_line, 1024);
    p->transport = dico_log_stream_create(L_DEBUG);
    p->file = NULL;
    p->line = 0;
    return stream;
}
