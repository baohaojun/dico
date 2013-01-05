/* This file is part of GNU Dico.
   Copyright (C) 2012 Sergey Poznyakoff

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
#include <libi18n.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct _linetrimstr {
    dico_stream_t transport;
    int noclose;
    size_t maxlen;
    size_t linelen;
};

#define ISWS(c) ((c)==' '||(c)=='\t'||(c)=='\n')

static size_t
_linetrimstr_find_end(struct _linetrimstr *s, const char *buf, size_t size,
		      size_t *psize)
{
    const char *end = buf + size;
    struct utf8_iterator itr;
    const char *wordstart = buf;

    utf8_iter_first(&itr, (char*)buf);
    do {    
	for (; utf8_iter_isascii(itr) && ISWS(*itr.curptr);
	     utf8_iter_next(&itr)) {
	    if (itr.curptr >= end) {
		*psize = itr.curptr - buf;
		return 0;
	    }
	    if (*itr.curptr == '\n')
		s->linelen = 0;
	    else if (++s->linelen >= s->maxlen) {
		*psize = wordstart > buf ? wordstart - buf : itr.curptr - buf;
		s->linelen = 0;
		return 1;
	    }
	}

	wordstart = itr.curptr;
	for (; !(utf8_iter_isascii(itr) && ISWS(*itr.curptr));
	     utf8_iter_next(&itr)) {
	    if (itr.curptr >= end) {
		*psize = itr.curptr - buf;
		return 0;
	    }
	    if (++s->linelen >= s->maxlen) {
		size_t size = itr.string == wordstart ?
		    itr.curptr - buf : wordstart - itr.string;
		s->linelen = 0;
		if (size > 0) {
		    *psize = size;
		    return 1;
		}
	    }
	}
    } while (itr.curptr < end);
    *psize = itr.curptr - buf;
    return 0;
}

static int
_linetrimstr_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct _linetrimstr *s = data;
    size_t len;
    int nl;
    
    nl = _linetrimstr_find_end(s, buf, size, &len);
    dico_stream_write(s->transport, buf, len);
    if (nl)
	dico_stream_write(s->transport, "\n", 1);
    *pret = len;

    return 0;
}
    
static int
_linetrimstr_destroy(void *data)
{ 
    struct _linetrimstr *s = data;
    if (!s->noclose)
	dico_stream_destroy(&s->transport);
    return 0;
}

static int
_linetrimstr_flush(void *data)
{
    struct _linetrimstr *s = data;
    return dico_stream_flush(s->transport);
}

static int
_linetrimstr_close(void *data)
{
    struct _linetrimstr *s = data;
    if (s->noclose)
	return 0;
    return dico_stream_close(s->transport);
}

static int
_linetrimstr_ioctl(void *data, int code, void *call_data)
{
    struct _linetrimstr *s = data;
    
    switch (code) {
    case DICO_IOCTL_GET_TRANSPORT:
	*(dico_stream_t*)call_data = s->transport;
	break;

    case DICO_IOCTL_SET_TRANSPORT:
	s->transport = call_data;
	break;

    case DICO_IOCTL_BYTES_IN:
	*(off_t*)call_data = dico_stream_bytes_in(s->transport);
	break;

    case DICO_IOCTL_BYTES_OUT:
	*(off_t*)call_data = dico_stream_bytes_out(s->transport);
	break;

    case DICO_IOCTL_SET_LINELEN:
	s->maxlen = *(size_t*)call_data;
	break;
	
    case DICO_IOCTL_GET_LINELEN:
	*(size_t*)call_data = s->maxlen;
	break;
	
    default:
	errno = EINVAL;
	return -1;
    }
    return 0;
}

dico_stream_t
dico_linetrim_stream(dico_stream_t transport, size_t maxlen, int noclose)
{
    int rc;
    struct _linetrimstr *s;
    dico_stream_t str;

    s = malloc(sizeof(*s));
    if (!s)
	return NULL;
    memset(s, 0, sizeof(*s));
    rc = dico_stream_create(&str, DICO_STREAM_WRITE, s);
    if (rc) {
	free(s);
	return NULL;
    }
    s->transport = transport;
    s->maxlen = maxlen;
    s->noclose = noclose;
    dico_stream_set_write(str, _linetrimstr_write);
    dico_stream_set_flush(str, _linetrimstr_flush);
    dico_stream_set_close(str, _linetrimstr_close);
    dico_stream_set_destroy(str, _linetrimstr_destroy);
    dico_stream_set_ioctl(str, _linetrimstr_ioctl);
    dico_stream_set_buffer(str, dico_buffer_line, 1024);
    return str;
}
