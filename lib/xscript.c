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

/* A "transcript stream" transparently writes data to and reads data from
   an underlying transport stream, writing each lineful of data to a "log
   stream". Writes to log stream are prefixed with a string indicating
   direction of the data (read/write). Default prefixes are those used in
   most RFCs - "S: " for data written ("Server"), and "C: " for data read
   ("Client"). */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <xdico.h>
#include <string.h>
#include <xalloc.h>
#include <errno.h>

#define TRANS_READ 0x1
#define TRANS_WRITE 0x2
#define FLAG_TO_PFX(c) ((c) - 1)

struct transcript_stream {
    int flags;
    dico_stream_t transport;
    dico_stream_t logstr;
    char *prefix[2];
};

static void
print_transcript(struct transcript_stream *str, int flag,
		 const char *buf, size_t size)
{
    while (size) {
	const char *p;
	size_t len;
	
	if (str->flags & flag) {
	    dico_stream_write(str->logstr,
			      str->prefix[FLAG_TO_PFX(flag)],
			      strlen(str->prefix[FLAG_TO_PFX(flag)]));
	    str->flags &= ~flag;
	}
	p = memchr(buf, '\n', size);
	if (p) {
	    len = p - buf;
	    if (p > buf && p[-1] == '\r')
		len--;
	    dico_stream_write(str->logstr, buf, len);
	    dico_stream_write(str->logstr, "\n", 1);
	    str->flags |= flag;

	    len = p - buf + 1;
	    buf = p + 1;
	    size -= len;
	} else {
	    dico_stream_write(str->logstr, buf, size);
	    break;
	}
    }
}

static int
transcript_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct transcript_stream *p = data;
    size_t nbytes;

    if (dico_stream_read(p->transport, buf, size, &nbytes) == 0) {
	print_transcript(p, TRANS_READ, buf, nbytes);
	if (pret)
	    *pret = nbytes;
    } else
	return dico_stream_last_error(p->transport);
    return 0;
}

static int
transcript_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct transcript_stream *p = data;
    if (dico_stream_write(p->transport, buf, size) == 0) {
	print_transcript(p, TRANS_WRITE, buf, size);
	if (pret)
	    *pret = size;
    } else
	return dico_stream_last_error(p->transport);
    return 0;
}

static int
transcript_flush(void *data)
{
    struct transcript_stream *p = data;
    if (!p->transport)
	return 0;
    return dico_stream_flush(p->transport);
}

static int
transcript_close(void *data)
{
    struct transcript_stream *p = data;
    dico_stream_close(p->logstr);
    dico_stream_close(p->transport);
    return 0;
}
    

static int
transcript_destroy(void *data)
{
    struct transcript_stream *p = data;
    free(p->prefix[0]);
    free(p->prefix[1]);
    dico_stream_destroy(&p->transport);
    dico_stream_destroy(&p->logstr);
    free(p);
    return 0;
}

static const char *
transcript_strerror(void *data, int rc)
{
    struct transcript_stream *p = data;
    return dico_stream_strerror(p->transport, rc);
}

static int
transcript_ioctl(void *data, int code, void *call_data)
{
    struct transcript_stream *p = data;
    switch (code) {
    case DICO_IOCTL_GET_TRANSPORT:
	*(dico_stream_t*)call_data = p->transport;
	break;

    case DICO_IOCTL_SET_TRANSPORT:
	p->transport = call_data;
	break;

    case DICO_IOCTL_BYTES_IN:
	*(off_t*)call_data = dico_stream_bytes_in(p->transport);
	break;

    case DICO_IOCTL_BYTES_OUT:
	*(off_t*)call_data = dico_stream_bytes_out(p->transport);
	break;
	
    default:
	errno = EINVAL;
	return -1;
    }
    return 0;
}

const char *default_prefix[2] = {
    "C: ", "S: "
};

dico_stream_t
xdico_transcript_stream_create(dico_stream_t transport, dico_stream_t logstr,
			       const char *prefix[])
{
    struct transcript_stream *p = xmalloc(sizeof(*p));
    dico_stream_t stream;
    int rc = dico_stream_create(&stream, DICO_STREAM_READ|DICO_STREAM_WRITE,
				p);
    if (rc)
	xalloc_die();
    p->flags = TRANS_READ | TRANS_WRITE;
    if (prefix) {
	p->prefix[0] = xstrdup(prefix[0] ? prefix[0] : default_prefix[0]);
	p->prefix[1] = xstrdup(prefix[1] ? prefix[1] : default_prefix[1]);
    } else {
	p->prefix[0] = xstrdup(default_prefix[0]);
	p->prefix[1] = xstrdup(default_prefix[1]);
    }
    p->transport = transport;
    p->logstr = logstr;
    
    dico_stream_set_read(stream, transcript_read);
    dico_stream_set_write(stream, transcript_write);
    dico_stream_set_flush(stream, transcript_flush);
    dico_stream_set_close(stream, transcript_close);
    dico_stream_set_destroy(stream, transcript_destroy);
    dico_stream_set_ioctl(stream, transcript_ioctl);
    dico_stream_set_error_string(stream, transcript_strerror);
    dico_stream_set_buffer(stream, dico_buffer_line, 1024);

    return stream;
}

