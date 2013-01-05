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

struct _iostr {
    dico_stream_t in;
    dico_stream_t out;
    dico_stream_t last_err;
};
    

static int
io_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct _iostr *p = data;
    if (dico_stream_read(p->in, buf, size, pret)) {
	p->last_err = p->in;
	return dico_stream_last_error(p->in);
    }
    return 0;
}

static int
io_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct _iostr *p = data;
    if (dico_stream_write(p->out, buf, size)) {
	p->last_err = p->out;
	return dico_stream_last_error(p->out);
    }
    *pret = size;
    return 0;
}

static int
io_flush(void *data)
{
    struct _iostr *p = data;
    if (dico_stream_flush(p->in)) {
	p->last_err = p->in;
	return dico_stream_last_error(p->in);
    }
    if (dico_stream_flush(p->out)) {
	p->last_err = p->out;
	return dico_stream_last_error(p->out);
    }
    return 0;
}

static int
io_close(void *data)
{
    struct _iostr *p = data;

    dico_stream_close(p->in);
    dico_stream_close(p->out);
    return 0;
}

static int
io_destroy(void *data)
{
    struct _iostr *p = data;
    dico_stream_destroy(&p->in);
    dico_stream_destroy(&p->out);
    free(p);
    return 0;
}

static const char *
io_error_string(void *data, int code)
{
    struct _iostr *p = data;
    if (!p->last_err)
	return _("No error");
    return dico_stream_strerror(p->last_err, code);
}

dico_stream_t
dico_io_stream(dico_stream_t in, dico_stream_t out)
{
    int rc;
    dico_stream_t str;
    struct _iostr *s;

    _dico_libi18n_init();
    s = malloc(sizeof(*s));
    if (!s)
	return NULL;
    rc = dico_stream_create(&str, DICO_STREAM_READ|DICO_STREAM_WRITE, s);
    if (rc) {
	free(s);
	return NULL;
    }
    s->in = in;
    s->out = out;
    s->last_err = NULL;
    dico_stream_set_write(str, io_write);
    dico_stream_set_read(str, io_read);
    dico_stream_set_flush(str, io_flush);
    dico_stream_set_close(str, io_close);
    dico_stream_set_destroy(str, io_destroy);
    dico_stream_set_error_string(str, io_error_string);
    return str;
}

