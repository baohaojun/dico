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

#include <dicod.h>

extern int option_mime;

off_t total_bytes_out;

#define OSTREAM_INITIALIZED       0x01
#define OSTREAM_DESTROY_TRANSPORT 0x02

struct ostream {
    dico_stream_t transport;
    off_t nout;
    int flags;
    dico_assoc_list_t headers;
};

static int
print_headers(struct ostream *ostr)
{
    int rc = 0;
    const char *enc;
    
    if (ostr->headers) {
	dico_iterator_t itr;
	struct dico_assoc *p;
	
	itr = dico_assoc_iterator(ostr->headers);
	for (p = dico_iterator_first(itr); p; p = dico_iterator_next(itr)) {
	    dico_stream_write(ostr->transport, p->key, strlen(p->key));
	    dico_stream_write(ostr->transport, ": ", 2);
	    dico_stream_write(ostr->transport, p->value, strlen(p->value));
	    dico_stream_write(ostr->transport, "\n", 1);
	}
	dico_iterator_destroy(&itr);
    }
    
    rc = dico_stream_write(ostr->transport, "\n", 1);
    
    if (rc == 0) {
	dico_stream_t str;
	
	if ((enc = dico_assoc_find(ostr->headers,
				   CONTENT_TRANSFER_ENCODING_HEADER))
	    && strcmp(enc, "8bit")) {
	    str = dico_codec_stream_create(enc, FILTER_ENCODE,
					   ostr->transport);
	} else {
	    str = dico_linetrim_stream(ostr->transport, 1024, 1);
	}
	if (str) {
	    ostr->transport = str;
	    ostr->flags |= OSTREAM_DESTROY_TRANSPORT;
	}    
    }
    return rc;
}


static int
ostream_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct ostream *ostr = data;
    
    if (!(ostr->flags & OSTREAM_INITIALIZED)) {
	if (option_mime && print_headers(ostr))
	    return dico_stream_last_error(ostr->transport);
	ostr->flags |= OSTREAM_INITIALIZED;
    }
    if (buf[0] == '.' && dico_stream_write(ostr->transport, ".", 1))
	return dico_stream_last_error(ostr->transport);
    *pret = size;
    return dico_stream_write(ostr->transport, buf, size);
}

static int
ostream_flush(void *data)
{
    struct ostream *ostr = data;
    return dico_stream_flush(ostr->transport);
}

static int
ostream_destroy(void *data)
{
    struct ostream *ostr = data;
    if (ostr->flags & OSTREAM_DESTROY_TRANSPORT)
	dico_stream_destroy(&ostr->transport);
    free(data);
    return 0;
}

static int
ostream_ioctl(void *data, int code, void *call_data)
{
    struct ostream *ostr = data;
    switch (code) {
    case DICO_IOCTL_GET_TRANSPORT:
	*(dico_stream_t*)call_data = ostr->transport;
	break;

    case DICO_IOCTL_SET_TRANSPORT:
	ostr->transport = call_data;
	break;

    case DICO_IOCTL_BYTES_OUT:
	dico_stream_flush(ostr->transport);
	*(off_t*)call_data = dico_stream_bytes_out(ostr->transport) -
	                       ostr->nout;
	break;
	
    default:
	errno = EINVAL;
	return -1;
    }
    return 0;
}

dico_stream_t
dicod_ostream_create(dico_stream_t str, dico_assoc_list_t headers)
{
    struct ostream *ostr = xmalloc(sizeof(*ostr));
    dico_stream_t stream;
    
    if (dico_stream_create(&stream, DICO_STREAM_WRITE, ostr))
	xalloc_die();
    ostr->transport = str;
    ostr->nout = dico_stream_bytes_out(str);
    ostr->flags = 0;
    ostr->headers = headers;
    dico_stream_set_write(stream, ostream_write);
    dico_stream_set_flush(stream, ostream_flush);
    dico_stream_set_destroy(stream, ostream_destroy);
    dico_stream_set_ioctl(stream, ostream_ioctl);
    dico_stream_set_buffer(stream, dico_buffer_line, 1024);
    return stream;
}

    
