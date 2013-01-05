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

/* Implementation of a "log stream", a write-only stream that sends
   all data to the dico log channel with the given level. */

#include <dico.h>

struct log_stream {
    int level;
};

static int
log_write(void *data, const char *buf, size_t size, size_t *pret)
{
    struct log_stream *p = data;
    if (pret)
	*pret = size;
    while (size > 0 && (buf[size-1] == '\n' || buf[size-1] == '\r'))
	size--;
    if (size)
	dico_log(p->level, 0, "%.*s", size, buf);
    return 0;
}

static int
log_destroy(void *data)
{
    free(data);
    return 0;
}

dico_stream_t
dico_log_stream_create(int level)
{
    struct log_stream *p = malloc(sizeof(*p));
    dico_stream_t stream;

    if (!p || dico_stream_create(&stream, DICO_STREAM_WRITE, p))
	return NULL;
    dico_stream_set_write(stream, log_write);
    dico_stream_set_destroy(stream, log_destroy);
    dico_stream_set_buffer(stream, dico_buffer_line, 1024);
    p->level = level;
    return stream;
}
