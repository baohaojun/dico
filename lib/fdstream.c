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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

struct _stream {
    int fd;
};

static int
fd_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct _stream *p = data;
    int n = read(p->fd, buf, size);
    if (n == -1)
	return errno;
    *pret = n;
    return 0;
}

static int
fd_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct _stream *p = data;
    int n = write(p->fd, (char*) buf, size);
    if (n == -1)
	return errno;
    *pret = n;
    return 0;
}

static int
fd_close(void *data)
{
    struct _stream *p = data;
    if (close(p->fd))
	return errno;
    return 0;
}

int
fd_seek (void *data, off_t off, int whence, off_t *presult)
{
    struct _stream *p = data;
    off = lseek(p->fd, off, whence);
    if (off < 0)
	return errno;
    *presult = off;
    return 0;
}

int
fd_size (void *data, off_t *psize)
{
    struct _stream *p = data;
    off_t size = lseek(p->fd, 0, SEEK_END);
    if (size < 0)
	return errno;
    *psize = size;
    return 0;
}
    
int
fd_destroy(void *data)
{
    free(data);
    return 0;
}

dico_stream_t
dico_fd_stream_create(int fd, int flags, int noclose)
{
    int rc;
    dico_stream_t str;
    struct _stream *s = malloc(sizeof(*s));

    if (!s)
	return NULL;
    rc = dico_stream_create(&str, flags, s);
    if (rc) {
	free(s);
	return NULL;
    }
    s->fd = fd;
    dico_stream_set_seek(str, fd_seek);
    dico_stream_set_size(str, fd_size);
    dico_stream_set_write(str, fd_write);
    dico_stream_set_read(str, fd_read);
    if (!noclose)
	dico_stream_set_close(str, fd_close);
    dico_stream_set_destroy(str, fd_destroy);
    return str;
}

    
