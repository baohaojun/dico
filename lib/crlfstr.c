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

enum crlf_state
{
  state_init,
  state_cr
};

struct _crlfstr {
    dico_stream_t transport;
    int noclose;
    enum crlf_state state;
    char cb;
};

static int
_crlfstr_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct _crlfstr *s = data;
    const char *p, *q;

    for (p = buf, q = buf + size; p < q; p++) {
	switch (*p) {
	case '\n':
	    if (s->state == state_cr)
		s->state = state_init;
	    else {
		if (p > buf)
		    dico_stream_write(s->transport, buf, p - buf);
		dico_stream_write(s->transport, "\r\n", 2);
		buf = p + 1;
	    }
	    break;
	case '\r':
	    s->state = state_cr;
	    break;
	default:
	    s->state = state_init;
	}
    }

    if (p > buf)
	dico_stream_write(s->transport, buf, p - buf);
    *pret = size;

    return 0;
}

static int
_crlfstr_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct _crlfstr *s = data;
    size_t sz = 0;
    size_t total = 0;
    char *p;

    if (s->cb) {
	buf[sz++] = s->cb;
	s->cb = 0;
    }
    while (total < size) {
	int i;
	if (sz == 0) {
	    int rc = dico_stream_read(s->transport, buf + total, size - total,
				      &sz);
	    if (rc)
		return rc;
	    if (sz == 0)
		break;
	}
	p = buf + total;
	for (i = 0; i < sz; i++, p++) {
	    switch (*p) {
	    case '\r':
		if (s->cb)
		    buf[total++] = s->cb;
		s->cb = '\r';
		break;
	    case '\n':
		s->cb = 0;
		buf[total++] = *p;
		break;
		
	    default:
		if (s->cb) {
		    buf[total++] = s->cb;
		    s->cb = 0;
		}
		if (total == size)
		    s->cb = *p;
		else
		    buf[total++] = *p;
	    }
	}
	sz = 0;
    }
    *pret = total;
    return 0;
}

static int
_crlfstr_destroy(void *data)
{ 
    struct _crlfstr *s = data;
    if (!s->noclose)
	dico_stream_destroy(&s->transport);
    return 0;
}

static int
_crlfstr_flush(void *data)
{
    struct _crlfstr *s = data;
    return dico_stream_flush(s->transport);
}

static int
_crlfstr_close(void *data)
{
    struct _crlfstr *s = data;
    if (s->noclose)
	return 0;
    return dico_stream_close(s->transport);
}

static int
_crlfstr_ioctl(void *data, int code, void *call_data)
{
    struct _crlfstr *s = data;
    
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

    default:
	errno = EINVAL;
	return -1;
    }
    return 0;
}

dico_stream_t
dico_crlf_stream(dico_stream_t transport, int flags, int noclose)
{
    int rc;
    struct _crlfstr *s;
    dico_stream_t str;

    s = malloc(sizeof(*s));
    if (!s)
	return NULL;
    memset(s, 0, sizeof(*s));
    rc = dico_stream_create(&str,
			    flags & (DICO_STREAM_READ|DICO_STREAM_WRITE), s);
    if (rc) {
	free(s);
	return NULL;
    }
    s->transport = transport;
    s->noclose = noclose;
    dico_stream_set_write(str, _crlfstr_write);
    dico_stream_set_read(str, _crlfstr_read);
    dico_stream_set_flush(str, _crlfstr_flush);
    dico_stream_set_close(str, _crlfstr_close);
    dico_stream_set_destroy(str, _crlfstr_destroy);
    dico_stream_set_ioctl(str, _crlfstr_ioctl);
    dico_stream_set_buffer(str, dico_buffer_line, 1024);
    return str;
}
