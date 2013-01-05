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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <libi18n.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gsasl.h>

#ifdef WITH_GSASL

#define _ERR_SASL 0x1
#define _ERR_TRANSPORT 0x2

struct g_buf {
    char *buffer;
    size_t pos;
    size_t size;
    size_t level;
};

#define g_buf_ptr(p) ((p).buffer + (p).pos)
#define g_buf_level(p) ((p).level - (p).pos)
#define g_buf_drop(p) (p).level = (p).pos = 0
#define g_buf_free(p) free((p).buffer)
#define g_buf_advance(p, s)			\
    do {					\
	if (((p).pos += s) >= (p).level)	\
	    g_buf_drop(p);			\
    } while (0)

int
g_buf_grow(struct g_buf *pb, const char *ptr, size_t size)
{
    if (!pb->buffer) {
	pb->buffer = malloc(size);
	pb->size = size;
	pb->level = 0;
    } else if (pb->size - pb->level < size) {
	size_t newsize = pb->size + size;
	pb->buffer = realloc(pb->buffer, newsize);
	if (pb->buffer)
	    pb->size = newsize;
    }

    if (!pb->buffer)
	return 1;

    memcpy(pb->buffer + pb->level, ptr, size);
    pb->level += size;
    return 0;
}

struct _gsasl_str {
    Gsasl_session *sess; /* Session context */
    int last_err;        /* Last GSASL error code */
    int errflag;
    dico_stream_t transport;
    struct g_buf buf;
};

#define set_error(s, flag, code)		\
    do {					\
	(s)->last_err = code;			\
	(s)->errflag = flag;			\
    } while (0)

static int
_gsasl_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct _gsasl_str *s = data;
    int rc;
    size_t len;
    char *bufp = NULL;
  
    if ((len = g_buf_level(s->buf))) {
	if (len > size)
	    len = size;
	memcpy(buf, g_buf_ptr(s->buf), len);
	g_buf_advance(s->buf, len);
	if (pret)
	    *pret = len;
	return 0;
    }

    do {
	char tmp[80];
	size_t sz;
	int status;
	
	status = dico_stream_read(s->transport, tmp, sizeof(tmp), &sz);
	if (status == EINTR)
	    continue;
	else if (status) {
	    free (bufp);
	    return status;
	}
	rc = g_buf_grow(&s->buf, tmp, sz);
	if (rc)
	    return rc;

	rc = gsasl_decode(s->sess,
			  g_buf_ptr(s->buf),
			  g_buf_level(s->buf),
			  &bufp, &len);
    } while (rc == GSASL_NEEDS_MORE);

    if (rc != GSASL_OK) {
	set_error(s, _ERR_SASL, rc);
	free(bufp);
	errno = EIO;
	return 1;
    }
      
    g_buf_drop(s->buf);
    if (len > size) {
	memcpy(buf, bufp, size);
	g_buf_grow(&s->buf, bufp + size, len - size);
	len = size;
    } else {
	memcpy(buf, bufp, len);
    }

    if (pret)
	*pret = len;
  
    free(bufp);
    return 0;
}

static int
_gsasl_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct _gsasl_str *s = data;
    char *obuf = NULL;
    size_t olen = 0;
    
    int rc = gsasl_encode(s->sess, buf, size, &obuf, &olen);
    if (rc != GSASL_OK) {
	set_error(s, _ERR_SASL, rc);
	errno = EIO;
	return 1;
    }
    rc = dico_stream_write(s->transport, obuf, olen);
    free(obuf);
    if (rc) {
	set_error(s, _ERR_TRANSPORT, errno);
	return 1;
    }
    if (pret)
	*pret = olen;
    return 0;
}

static int
_gsasl_destroy(void *data)
{
    struct _gsasl_str *s = data;
    g_buf_free(s->buf);
    dico_stream_destroy(&s->transport);
    return 0;
}

/*
  FIXME:
static const char *
_gsasl_error_string(void *data, int code)
{
    
}
*/

static int
_gsasl_flush(void *data)
{
    struct _gsasl_str *s = data;
    return dico_stream_flush(s->transport);
}

static int
_gsasl_close(void *data)
{
    struct _gsasl_str *s = data;
    return dico_stream_close(s->transport);
}

static int
_gsasl_ioctl(void *data, int code, void *call_data)
{
    struct _gsasl_str *s = data;
    
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
dico_gsasl_stream(Gsasl_session *sess, dico_stream_t transport)
{
    int rc;
    struct _gsasl_str *s;
    dico_stream_t str;

    s = malloc(sizeof(*s));
    if (!s)
	return NULL;
    memset(s, 0, sizeof(*s));
    rc = dico_stream_create(&str, DICO_STREAM_READ|DICO_STREAM_WRITE, s);
    if (rc) {
	free(s);
	return NULL;
    }
    s->sess = sess;
    s->transport = transport;
    dico_stream_set_write(str, _gsasl_write);
    dico_stream_set_read(str, _gsasl_read);
    dico_stream_set_flush(str, _gsasl_flush);
    dico_stream_set_close(str, _gsasl_close);
    dico_stream_set_destroy(str, _gsasl_destroy);
    dico_stream_set_ioctl(str, _gsasl_ioctl);
    dico_stream_set_buffer(str, dico_buffer_line, 1024);
    return str;
}

int
insert_gsasl_stream(Gsasl_session *sess, dico_stream_t *pstr)
{
    int rc;
    dico_stream_t gsasl_str;
    dico_stream_t str = *pstr;
    dico_stream_t prev = NULL, s;
    
    while ((rc = dico_stream_ioctl(str, DICO_IOCTL_GET_TRANSPORT, &s)) == 0
	   && s) {
	prev = str;
	str = s;
    }
    if (rc && errno != EINVAL && errno != ENOSYS) {
	dico_log(L_ERR, errno, _("Cannot get transport stream"));
	return rc;
    }
    if (!str) {
	dico_log(L_ERR, 0, _("Cannot get transport stream"));
	return 1;
    }

    gsasl_str = dico_gsasl_stream(sess, str);
    if (!gsasl_str) {
	dico_log(L_ERR, errno, _("Cannot create GSASL stream"));
	return 1;
    }
    
    if (!prev)
	*pstr = gsasl_str;
    else if (dico_stream_ioctl(prev, DICO_IOCTL_SET_TRANSPORT, gsasl_str)) {
	dico_log(L_ERR, errno, _("Cannot install GSASL stream"));
	return 1;
    }
    return 0;
}

#endif
