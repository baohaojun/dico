/* This file is part of GNU Dico
   Copyright (C) 2003-2004, 2007-2008, 2010, 2012 Sergey Poznyakoff
  
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
#include <errno.h>
#include <dico.h>
#include <string.h>

#define FILTER_BSIZE 2048

struct filter_stream {
    dico_stream_t transport;
    char buf[FILTER_BSIZE];
    size_t level;
    size_t min_level;
    size_t max_line_length;
    size_t line_length;
    filter_xcode_t xcode;
    char *inbuf;
    size_t inlevel;
};

static int
filter_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct filter_stream *fs = data;
    size_t rdsize;
    int rc;

    if (fs->level < fs->min_level) {
	rc = dico_stream_read(fs->transport, fs->buf + fs->level,
			      sizeof(fs->buf) - fs->level,
			      &rdsize);
	if (rc)
	    return rc;
	fs->level = rdsize;
    }
    
    if (fs->level) {
	rc = fs->xcode(fs->buf, fs->level, buf, size, &rdsize);
	/* FIXME */
	memmove(fs->buf, fs->buf + rc, fs->level - rc);
	fs->level = rc;
	*pret = rdsize;
	rc = 0;
    } else {
	*pret = 0;
	rc = 0;
    }
    return rc;
}

static int
filter_flush(struct filter_stream *fs)
{
    if (fs->level == 0)
	return 0;
    else if (fs->max_line_length == 0) {
	int rc = dico_stream_write(fs->transport, fs->buf, fs->level);
	if (rc == 0)
	    fs->level = 0;
	return rc;
    } else {
	char *buf = fs->buf;
	size_t level = fs->level;
	while (level) {
	    int rc;
	    size_t rest = fs->max_line_length - fs->line_length;
	    size_t len;
	    int skip = 0;
	    char *p = memchr(buf, '\n', level);

	    if (rest > level)
		rest = level;
	    
	    if (p) {
		len = p - buf;
		if (len > rest)
		    len = rest;
		else
		    skip = 1;
	    } else
		len = rest;

	    rc = dico_stream_write(fs->transport, buf, len);
	    if (rc)
		return 1;
	    fs->line_length += len;
	    if (fs->line_length == fs->max_line_length) {
		fs->line_length = 0;
		rc = dico_stream_write(fs->transport, "\n", 1);
		if (rc)
		    return 1;
	    }

	    if (skip)
		len++;
	    buf += len;
	    level -= len;
	}
	fs->level = 0;
    }
    return 0;
}

static int
filter_write0(struct filter_stream *fs, const char *buf, size_t size,
	      size_t *pret)
{
    size_t wrsize;
    int rc;
    
    if (fs->level >= sizeof(fs->buf) - fs->min_level) {
	rc = filter_flush(fs);
	if (rc)
	    return rc;
	fs->level = 0;
    }

    for (;;) {
	rc = fs->xcode(buf, size, fs->buf + fs->level,
		       sizeof(fs->buf) - fs->level, &wrsize);
	if (rc == 0) {
	    rc = filter_flush(fs);
	    if (rc)
		return rc;
	    fs->level = 0;
	} else
	    break;
    }
    fs->level += wrsize;
    *pret = rc;
    return 0;
}

static int
filter_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct filter_stream *fs = data;
    size_t ret = 0;
    size_t wrs;
    int rc = 0;
    
    if (size < fs->min_level
	|| (fs->inlevel && fs->inlevel < fs->min_level)) {
	size_t rest = fs->min_level - fs->inlevel;
	if (rest > size)
	    rest = size;
	memcpy(fs->inbuf + fs->inlevel, buf, rest);
	fs->inlevel += rest;
	if (fs->inlevel < fs->min_level) {
	    *pret = rest;
	    return 0;
	}
	buf += rest;
	size -= rest;

	rc = filter_write0(fs, fs->inbuf, fs->inlevel, &wrs);
	if (rc)
	    return rc;
	if (wrs != fs->inlevel) {
	    /*FIXME: errno = EIO; */
	    return 1;
	}
	fs->inlevel = 0;
	ret = rest;
    }
    if (size) 
	rc = filter_write0(fs, buf, size, &wrs);
    else {
	rc = 0;
	wrs = 0;
    }
    if (rc == 0) {
	ret += wrs;
	*pret = ret;
    }
    return rc;
}
	

static int
filter_wr_flush(void *data)
{
    struct filter_stream *fs = data;
    int rc = 0;
    
    if (fs->level) {
	int nl = fs->buf[fs->level-1] == '\n';
	
	rc = filter_flush(fs);
	if (rc == 0) {
	    if (fs->inlevel) {
		size_t wrs;
		filter_write0(fs, fs->inbuf, fs->inlevel, &wrs);
		nl = fs->buf[fs->level-1] == '\n';
		rc = filter_flush(fs);
	    }
	    if (!nl)
		rc = dico_stream_write(fs->transport, "\n", 1);
	}
    }
    return rc;
}

static int
filter_stream_destroy(void *data)
{
    struct filter_stream *fs = data;
    free(fs->inbuf);
    return 0;
}

static int
filter_ioctl(void *data, int code, void *call_data)
{
    struct filter_stream *fs = data;

    switch (code) {
    case DICO_IOCTL_BYTES_IN:
	*(off_t*)call_data = dico_stream_bytes_in(fs->transport);
	break;

    case DICO_IOCTL_BYTES_OUT:
	*(off_t*)call_data = dico_stream_bytes_out(fs->transport);
	break;
	
    default:
	errno = EINVAL;
	return -1;
    }
    return 0;
}


dico_stream_t
filter_stream_create(dico_stream_t str,
		     size_t min_level,
		     size_t max_line_length,
		     filter_xcode_t xcode,
		     int mode)
{
    struct filter_stream *fs = malloc(sizeof(*fs));
    dico_stream_t stream;
    int rc;
    
    if (!fs)
	return NULL;

    rc = dico_stream_create(&stream,
			    mode == FILTER_ENCODE ?
			        DICO_STREAM_WRITE : DICO_STREAM_READ,
			    fs);
    if (rc) {
	free(fs);
	return NULL;
    }
    
    if (mode == FILTER_ENCODE) {
	fs->inbuf = malloc(min_level);
	if (!fs->inbuf) {
	    dico_stream_destroy(&stream);
	    return NULL;
	}
	fs->inlevel = 0;
	dico_stream_set_write(stream, filter_write);
	dico_stream_set_flush(stream, filter_wr_flush);
	dico_stream_set_destroy(stream, filter_stream_destroy);
    } else {
	dico_stream_set_read(stream, filter_read);
    }
    dico_stream_set_ioctl(stream, filter_ioctl);
    
    fs->transport = str;
    fs->level = 0;
    fs->min_level = min_level;
    fs->line_length = 0;
    fs->max_line_length = max_line_length;
    fs->xcode = xcode;

    return stream;
}

dico_stream_t
dico_codec_stream_create(const char *encoding, int mode,
			 dico_stream_t transport)
{
    dico_stream_t str = NULL;
    if (strcmp(encoding, "base64") == 0) 
	str = dico_base64_stream_create(transport, mode);
    else if (strcmp(encoding, "quoted-printable") == 0) 
	str = dico_qp_stream_create(transport, mode);
    return str;
}
