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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <size_max.h>

#define _STR_DIRTY         0x1000    /* Buffer dirty */
#define _STR_ERR           0x2000    /* Permanent error state */
#define _STR_EOF           0x4000    /* EOF encountered */

struct dico_stream {
    enum dico_buffer_type buftype;
    size_t bufsize;
    char *buffer;
    size_t level;
    char *cur;

    int flags;
    off_t bytes_in, bytes_out;
    
    int last_err;
    int (*read) (void *, char *, size_t, size_t *);
    int (*write) (void *, const char *, size_t, size_t *);
    int (*flush) (void *);
    int (*open) (void *, int);
    int (*close) (void *);
    int (*destroy) (void *);
    int (*seek) (void *, off_t, int, off_t *);
    int (*size) (void *, off_t *);
    int (*ctl) (void *, int, void *);
    const char *(*error_string) (void *, int);
    void *data;
};    

static int
_stream_seterror(dico_stream_t stream, int code, int perm)
{
    stream->last_err = code;
    if (perm)
	stream->flags |= _STR_ERR;
    return code;
}

int
dico_stream_create(dico_stream_t *pstream, int flags, void *data)
{
    dico_stream_t stream = malloc(sizeof(*stream));
    if (stream == NULL)
	return ENOMEM;
    memset(stream, 0, sizeof(*stream));
    stream->flags = flags;
    stream->data = data;
    *pstream = stream;
    return 0;
}

int
dico_stream_open(dico_stream_t stream)
{
    int rc;
    if (stream->open && (rc = stream->open(stream->data, stream->flags))) 
	return _stream_seterror(stream, rc, 1);
    stream->bytes_in = stream->bytes_out = 0;
    return 0;
}

void
dico_stream_set_error_string(dico_stream_t stream,
			     const char *(*error_string) (void *, int))
{
    stream->error_string = error_string;
}

void
dico_stream_set_open(dico_stream_t stream, int (*openfn) (void *, int))
{
    stream->open = openfn;
}

void
dico_stream_set_seek(dico_stream_t stream,
		     int (*fun_seek) (void *, off_t, int, off_t *))
{
    stream->seek = fun_seek;
}

void
dico_stream_set_read(dico_stream_t stream,
		     int (*readfn) (void *, char *, size_t, size_t *))
{
    stream->read = readfn;
}

void
dico_stream_set_write(dico_stream_t stream,    
		      int (*writefn) (void *, const char *, size_t, size_t *))
{
    stream->write = writefn;
}

void
dico_stream_set_flush(dico_stream_t stream, int (*flushfn) (void *))
{
    stream->flush = flushfn;
}

void
dico_stream_set_close(dico_stream_t stream, int (*closefn) (void *))
{
    stream->close = closefn;
}

void
dico_stream_set_destroy(dico_stream_t stream, int (*destroyfn) (void *))
{
    stream->destroy = destroyfn;
}

void
dico_stream_set_size(dico_stream_t stream, int (*sizefn) (void *, off_t *))
{
    stream->size = sizefn;
}

void
dico_stream_set_ioctl(dico_stream_t stream, int (*ctl) (void *, int, void *))
{
    stream->ctl = ctl;
}


const char *
dico_stream_strerror(dico_stream_t stream, int rc)
{
    const char *str;
    if (stream->error_string)
	str = stream->error_string(stream->data, rc);
    else
	str =  strerror(rc);
    return str;
}

int
dico_stream_last_error(dico_stream_t stream)
{
    return stream->last_err;
}

void
dico_stream_clearerr(dico_stream_t stream)
{
    stream->last_err = 0;
}

int
dico_stream_eof(dico_stream_t stream)
{
    return stream->flags & _STR_EOF;
}

#define _stream_cleareof(s) ((s)->flags &= ~_STR_EOF)
#define _stream_advance_buffer(s,n) ((s)->cur += n, (s)->level -= n)
#define _stream_buffer_offset(s) ((s)->cur - (s)->buffer)
#define _stream_orig_level(s) ((s)->level + _stream_buffer_offset(s))

off_t
dico_stream_seek(dico_stream_t stream, off_t offset, int whence)
{
    int rc;
    off_t res;
    size_t bpos;
    
    if (stream->flags & _STR_ERR)
	return -1;
    if (!stream->seek) {
	_stream_seterror(stream, ENOSYS, 0);
	return -1;
    }
    if (!(stream->flags & DICO_STREAM_SEEK)) {
	_stream_seterror(stream, EACCES, 1);
	return -1;
    }

    switch (whence) {
    case DICO_SEEK_SET:
	break;

    case DICO_SEEK_CUR:
	break;

    case DICO_SEEK_END: 
	bpos = _stream_buffer_offset(stream);
	if (bpos + offset >= 0 && bpos + offset < _stream_orig_level(stream)) {
	    if ((rc = stream->seek(stream->data, offset, whence, &res))) {
		_stream_seterror(stream, rc, 1);
		return -1;
	    }
	    offset -= bpos;
	    _stream_advance_buffer(stream, offset);
	    _stream_cleareof(stream);
	    return res - stream->level;
	}
	break;

    default:
	_stream_seterror(stream, EINVAL, 1);
	return -1;
    }

    if (dico_stream_flush(stream))
	return -1;
    rc = stream->seek(stream->data, offset, whence, &res);
    if (rc) {
	_stream_seterror(stream, rc, 1);
	return -1;
    }
    _stream_cleareof(stream);
    return res;
}

int
dico_stream_set_buffer(dico_stream_t stream, enum dico_buffer_type type,
		       size_t size)
{
    if (size == 0)
	type = dico_buffer_none;

    if (stream->buffer) {
	dico_stream_flush(stream);
	free(stream->buffer);
    }

    stream->buftype = type;
    if (type == dico_buffer_none) {
	stream->buffer = NULL;
	return 0;
    }

    stream->buffer = malloc(size);
    if (stream->buffer == NULL) {
	stream->buftype = dico_buffer_none;
	return _stream_seterror(stream, ENOMEM, 1);
    }
    stream->bufsize = size;
    stream->cur = stream->buffer;
    stream->level = 0;
    
    return 0;
}

int
dico_stream_read_unbuffered(dico_stream_t stream, void *buf, size_t size,
			    size_t *pread)
{
    int rc = 0;

    if (!stream->read) 
	return _stream_seterror(stream, ENOSYS, 0);

    if (!(stream->flags & DICO_STREAM_READ)) 
	return _stream_seterror(stream, EACCES, 1);
    
    if (stream->flags & _STR_ERR)
	return stream->last_err;
    
    if ((stream->flags & _STR_EOF) || size == 0) {
       if (pread) {
	   *pread = 0;
	   return 0;
       } else {
	   _stream_seterror(stream, EIO, 0);
	   return EIO;
       }
    }
    
    if (pread == NULL) {
	size_t rdbytes;

	while (size > 0
	       && (rc = stream->read(stream->data, buf, size, &rdbytes)) == 0) {
	    if (rdbytes == 0) {
		stream->flags |= _STR_EOF;
		break;
	    }
	    buf += rdbytes;
	    size -= rdbytes;
	    stream->bytes_in += rdbytes;
	}
	if (size) {
	    _stream_seterror(stream, EIO, 0);
	    return EIO;
	}
    } else {
	rc = stream->read(stream->data, buf, size, pread);
	if (rc == 0) {
	    if (*pread == 0)
		stream->flags |= _STR_EOF;
	    stream->bytes_in += *pread;
	}
    }
    _stream_seterror(stream, rc, rc != 0);
    return rc;
}

int
dico_stream_write_unbuffered(dico_stream_t stream,
			     const void *buf, size_t size,
			     size_t *pwrite)
{
    int rc;
    
    if (!stream->write) 
	return _stream_seterror(stream, ENOSYS, 0);

    if (!(stream->flags & DICO_STREAM_WRITE)) 
	return _stream_seterror(stream, EACCES, 1);

    if (stream->flags & _STR_ERR)
	return stream->last_err;

    if (size == 0) {
	if (pwrite)
	    *pwrite = 0;
	return 0;
    }
    
    if (pwrite == NULL) {
	size_t wrbytes;
	const char *bufp = buf;
	
	while (size > 0
	       && (rc = stream->write(stream->data, bufp, size, &wrbytes))
	             == 0) {
	    if (wrbytes == 0) {
		rc = EIO;
		break;
	    }
	    bufp += wrbytes;
	    size -= wrbytes;
	    stream->bytes_out += wrbytes;
	}
    } else {
	rc = stream->write(stream->data, buf, size, pwrite);
	if (rc == 0)
	    stream->bytes_out += *pwrite;
    }
    _stream_seterror(stream, rc, rc != 0);
    return rc;
}

static int
_stream_fill_buffer(dico_stream_t stream)
{
    size_t n;
    int rc = 0;
    char c;
    
    switch (stream->buftype) {
    case dico_buffer_none:
	return 0;
	
    case dico_buffer_full:
	if (dico_stream_read_unbuffered(stream,
					stream->buffer, stream->bufsize,
					&stream->level))
	    return 1;
	break;
	
    case dico_buffer_line:
	for (n = 0;
	     n < stream->bufsize
		 && (rc = dico_stream_read_unbuffered(stream,
						      &c, 1, NULL)) == 0;) {
	    stream->buffer[n++] = c;
	    if (c == '\n')
		break;
	}
	stream->level = n;
	break;
    }
    stream->cur = stream->buffer;
    return rc;
}

#define BUFFER_FULL_P(s) \
    ((s)->cur + (s)->level == (s)->buffer + (s)->bufsize)

static int
_stream_buffer_full_p(dico_stream_t stream)
{
    switch (stream->buftype) {
    case dico_buffer_none:
	break;

    case dico_buffer_line:
	return BUFFER_FULL_P(stream)
	       || memchr(stream->cur, '\n', stream->level) != NULL;

    case dico_buffer_full:
	return BUFFER_FULL_P(stream);
    }
    return 0;
}

static int
_stream_flush_buffer(dico_stream_t stream, int all)
{
    char *end;
		  
    if (stream->flags & _STR_DIRTY) {
	if ((stream->flags & DICO_STREAM_SEEK)
	    && dico_stream_seek(stream,
				- _stream_orig_level(stream),
				DICO_SEEK_CUR) < 0)
	    return 1;

	switch (stream->buftype) {
	case dico_buffer_none:
	    abort(); /* should not happen */
	    
	case dico_buffer_full:
	    if (dico_stream_write_unbuffered(stream, stream->cur,
					     stream->level, NULL))
		return 1;
	    _stream_advance_buffer(stream, stream->level);
	    break;
	    
	case dico_buffer_line:
	    if (stream->level == 0)
		break;
	    for (end = memchr(stream->cur, '\n', stream->level);
		 end;
		 end = memchr(stream->cur, '\n', stream->level)) {
		size_t size = end - stream->cur + 1;
		int rc = dico_stream_write_unbuffered(stream, stream->cur,
						      size, NULL);
		if (rc)
		    return rc;
		_stream_advance_buffer(stream, size);
	    }
	    if ((all && stream->level) || BUFFER_FULL_P(stream)) {
		int rc = dico_stream_write_unbuffered(stream, stream->cur,
						      stream->level, NULL);
		if (rc)
		    return rc;
		_stream_advance_buffer(stream, stream->level);
	    }
	}
    }
    if (stream->level) {
	if (stream->cur > stream->buffer)
	    memmove(stream->buffer, stream->cur, stream->level);
    } else {
	stream->flags &= ~_STR_DIRTY;
	stream->level = 0;
    }
    stream->cur = stream->buffer;
    return 0;
}

int
dico_stream_read(dico_stream_t stream, void *buf, size_t size, size_t *pread)
{
    if (stream->buftype == dico_buffer_none)
	return dico_stream_read_unbuffered(stream, buf, size, pread);
    else {
	char *bufp = buf;
	size_t nbytes = 0;

	if (stream->flags & _STR_ERR)
	    return stream->last_err;
	
	while (size) {
	    size_t n;
	    
	    if (stream->level == 0) {
		if (_stream_fill_buffer(stream)) {
		    if (nbytes)
			break;
		    return 1;
		}
		if (stream->level == 0)
		    break;
	    }

	    n = size;
	    if (n > stream->level)
		n = stream->level;
	    memcpy(bufp, stream->cur, n);
	    _stream_advance_buffer(stream, n);
	    nbytes += n;
	    bufp += n;
	    size -= n;
	    if (stream->buftype == dico_buffer_line && bufp[-1] == '\n')
		break;
	}

	if (pread)
	    *pread = nbytes;
	else if (size) 
	    return _stream_seterror(stream, EIO, 1);
    }
    return 0;
}

int
dico_stream_readln(dico_stream_t stream, char *buf, size_t size, size_t *pread)
{
    int rc;
    char c;
    size_t n = 0;

    if (size == 0)
	return EIO;
    
    size--;
    for (n = 0; n < size && (rc = dico_stream_read(stream, &c, 1, NULL)) == 0;
	 n++) {
	*buf++ = c;
	if (c == '\n')
	    break;
    }
    *buf = 0;
    if (pread)
	*pread = n;
    return rc;
}

int
dico_stream_getdelim(dico_stream_t stream, char **pbuf, size_t *psize,
		     int delim, size_t *pread)
{
    int rc;
    char *lineptr = *pbuf;
    size_t n = *psize;
    size_t cur_len = 0;
    
    if (lineptr == NULL || n == 0) {
	char *new_lineptr;
	n = 120;
	new_lineptr = realloc(lineptr, n);
	if (new_lineptr == NULL) 
	    return ENOMEM;
	lineptr = new_lineptr;
    }
    
    for (;;) {
	char c;

	rc = dico_stream_read(stream, &c, 1, NULL);
	if (rc)
	    break;
	
	/* Make enough space for len+1 (for final NUL) bytes.  */
	if (cur_len + 1 >= n) {
	    size_t needed_max =
		SSIZE_MAX < SIZE_MAX ? (size_t) SSIZE_MAX + 1 : SIZE_MAX;
	    size_t needed = 2 * n + 1;   /* Be generous. */
	    char *new_lineptr;

	    if (needed_max < needed)
		needed = needed_max;
	    if (cur_len + 1 >= needed) {
		rc = EOVERFLOW;
		break;
	    }
	    
	    new_lineptr = realloc(lineptr, needed);
	    if (new_lineptr == NULL) {
		rc = ENOMEM;
		break;
	    }
	    
	    lineptr = new_lineptr;
	    n = needed;
	}

	lineptr[cur_len] = c;
	cur_len++;

	if (c == delim)
	    break;
    }
    lineptr[cur_len] = '\0';
    
    *pbuf = lineptr;
    *psize = n;

    if (pread)
	*pread = cur_len;
    return rc;
}

int
dico_stream_getline(dico_stream_t stream, char **pbuf, size_t *psize,
		    size_t *pread)
{
    return dico_stream_getdelim(stream, pbuf, psize, '\n', pread);
}

int
dico_stream_write(dico_stream_t stream, const void *buf, size_t size)
{
    if (stream->buftype == dico_buffer_none)
	return dico_stream_write_unbuffered(stream, buf, size, NULL);
    else {
	size_t nbytes = 0;
	const char *bufp = buf;
	
	if (stream->flags & _STR_ERR)
	    return stream->last_err;
	
	while (1) {
	    size_t n;

	    if (_stream_buffer_full_p(stream)
		&& _stream_flush_buffer(stream, 0))
		return 1;

	    if (size == 0)
		break;
	    
	    n = stream->bufsize - stream->level;
	    if (n > size)
		n = size;
	    memcpy(stream->cur + stream->level, bufp, n);
	    stream->level += n;
	    nbytes += n;
	    bufp += n;
	    size -= n;
	    stream->flags |= _STR_DIRTY;
	}	    
    }
    return 0;
}

int
dico_stream_writeln(dico_stream_t stream, const char *buf, size_t size)
{
    int rc;
    if ((rc = dico_stream_write(stream, buf, size)) == 0)
	rc = dico_stream_write(stream, "\n", 1);
    return rc;
}

int
dico_stream_flush(dico_stream_t stream)
{
    if (!stream) {
	errno = EINVAL;
	return 1;
    }
    if (_stream_flush_buffer(stream, 1))
	return 1;
    if (stream->flush)
	return stream->flush(stream->data);
    return 0;
}

int
dico_stream_close(dico_stream_t stream)
{
    int rc = 0;
    if (!stream)
        return EINVAL;
    dico_stream_flush(stream);
    if (stream->close)
	rc = stream->close(stream->data);
    return rc;
}

int
dico_stream_size(dico_stream_t stream, off_t *psize)
{
    int rc;
    if (!stream->size)
	return _stream_seterror(stream, ENOSYS, 0);
    rc = stream->size(stream->data, psize);
    return _stream_seterror(stream, rc, rc != 0);
}

void
dico_stream_destroy(dico_stream_t *stream)
{
    if (!stream || !*stream)
        return;
    if ((*stream)->destroy)
	(*stream)->destroy((*stream)->data);
    free(*stream);
    *stream = NULL;
}

off_t
dico_stream_bytes_in(dico_stream_t stream)
{
    off_t val;
    if (dico_stream_ioctl(stream, DICO_IOCTL_BYTES_IN, &val) == 0)
	return val;
    return stream->bytes_in;
}

off_t
dico_stream_bytes_out(dico_stream_t stream)
{
    off_t val;
    if (dico_stream_ioctl(stream, DICO_IOCTL_BYTES_OUT, &val) == 0)
	return val;
    return stream->bytes_out;
}

int
dico_stream_ioctl(dico_stream_t stream, int code, void *ptr)
{
    if (stream->ctl == NULL) {
	errno = ENOSYS;
	return -1;
    }
    return stream->ctl(stream->data, code, ptr);
}
