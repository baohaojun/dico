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
#include <unistd.h>

dico_stream_t
dico_fd_io_stream_create(int ifd, int ofd)
{
    dico_stream_t in, out;
    dico_stream_t str;

    in = dico_fd_stream_create(ifd, DICO_STREAM_READ, 0);
    if (!in)
	return NULL;
    out = dico_fd_stream_create(ofd, DICO_STREAM_WRITE, 0);
    if (!out) {
	dico_stream_destroy(&in);
	return NULL;
    }

    str = dico_io_stream(in, out);
    if (!str) {
	dico_stream_destroy(&in);
	dico_stream_destroy(&out);
    }
    return str;
}
