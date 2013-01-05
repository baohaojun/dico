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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>

#ifndef MAP_FAILED
# define MAP_FAILED (void*)-1
#endif

struct _mapfile_stream {
    char *filename;
    int fd;
    int flags;
    char *start;
    size_t size;
    off_t offset;
};

static int
_mapfile_close(void *data)
{
    struct _mapfile_stream *mfs = data;
    if (mfs->start) {
	munmap(mfs->start, mfs->size);
	mfs->start = NULL;
	mfs->size = 0;
    }
    close(mfs->fd);
    mfs->fd = -1;
    return 0;
}

int
_mapfile_destroy(void *data)
{
    struct _mapfile_stream *str = data;
    free(str->filename);
    free(str);
    return 0;
}

int
_mapfile_open(void *data, int flags)
{
    struct _mapfile_stream *mfs = data;
    struct stat st;
    int mflags = 0, oflags = 0;
    
    /* Close any previous file.  */
    _mapfile_close(data);

    /* Map the flags to the system equivalent */
    if (flags & DICO_STREAM_READ) 
	mflags |= PROT_READ;
    if (flags & DICO_STREAM_WRITE) 
	mflags |= PROT_WRITE;

    if ((flags & (DICO_STREAM_READ|DICO_STREAM_WRITE))
	== (DICO_STREAM_READ|DICO_STREAM_WRITE))
	oflags = O_RDWR;
    else if (flags & DICO_STREAM_READ)
	oflags = O_RDONLY;
    else
	oflags = O_WRONLY;

    mfs->fd = open(mfs->filename, oflags);
    if (mfs->fd < 0)
	return errno;
    if (fstat (mfs->fd, &st)) {
	int err = errno;
	close(mfs->fd);
	return err;
    }
    mfs->size = st.st_size;
    if (mfs->size) {
	mfs->start = mmap (NULL, mfs->size, mflags, MAP_SHARED, mfs->fd, 0);
	if (mfs->start == MAP_FAILED) {
	    int err = errno;
	    close(mfs->fd);
	    mfs->start = NULL;
	    return err;
	}
    } else
	mfs->start = NULL;
    mfs->flags = mflags;
    return 0;
}

static int
_mapfile_seek(void *data, off_t needle, int whence, off_t *presult)
{
    struct _mapfile_stream *str = data;
    off_t offset;
    
    switch (whence) {
    case DICO_SEEK_SET:
	offset = needle;
	break;

    case DICO_SEEK_CUR:
	offset = str->offset + needle;
	break;

    case DICO_SEEK_END:
	offset = str->size + needle;
	break;

    default:
	return EINVAL;
    }

    if (offset < 0 || offset > str->size) 
	return EINVAL;
    str->offset = offset;
    *presult = offset;
    return 0;
}

static int
_mapfile_size(void *data, off_t *presult)
{
    struct _mapfile_stream *mfs = data;
    
    if (mfs->start == NULL)
	return EINVAL;
    *presult = mfs->size;
    return 0;
}

static int
_mapfile_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct _mapfile_stream *mfs = data;
    size_t n;
    
    if (mfs->start == NULL)
	return EINVAL;

    n = (mfs->offset + size > mfs->size) ? mfs->size - mfs->offset : size;
    memcpy(buf, mfs->start + mfs->offset, n);
    mfs->offset += n;
    
    *pret = n;
    return 0;
}
    
dico_stream_t
dico_mapfile_stream_create(const char *filename, int flags)
{
    int rc;
    dico_stream_t str;
    struct _mapfile_stream *s = malloc(sizeof(*s));

    /* FIXME: Remove this when read-write mapped streams are implemented */
    if (flags & DICO_STREAM_WRITE) {
	errno = EINVAL;
	return NULL; 
    }
    
    if (!s)
	return NULL;
    memset(s, 0, sizeof(*s));
    s->fd = -1;
    s->filename = strdup(filename);
    if (!s->filename) {
	free(s);
	return NULL;
    }
    
    rc = dico_stream_create(&str, flags, s);
    if (rc) {
	free(s->filename);
	free(s);
	return NULL;
    }
    dico_stream_set_open(str, _mapfile_open);
    dico_stream_set_seek(str, _mapfile_seek);
    dico_stream_set_size(str, _mapfile_size);
    dico_stream_set_read(str, _mapfile_read);
    dico_stream_set_close(str, _mapfile_close);
    dico_stream_set_destroy(str, _mapfile_destroy);
    return str;
}


