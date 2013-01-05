/* This file is part of GNU Dico
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
#include <xalloc.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

size_t
dico_string_trim(char *buf, size_t len, int (*pred)(int c))
{
    while (len > 0 && pred(buf[len - 1]))
	buf[--len] = 0;
    return len;
}

static int
_is_nl(int c)
{
    return c == '\r' || c == '\n';
}

size_t
dico_trim_nl(char *buf)
{
    return dico_string_trim(buf, strlen(buf), _is_nl);
}

static int
_is_ws(int c)
{
    return isspace(c);
}

size_t
dico_trim_ws(char *buf)
{
    return dico_string_trim(buf, strlen(buf), _is_ws);
}

char *
dico_full_file_name(const char *dir, const char *file)
{
    size_t dirlen = strlen(dir);
    int need_slash = !(dirlen && dir[dirlen - 1] == '/');
    size_t size = dirlen + need_slash + 1 + strlen(file) + 1;
    char *buf = malloc(size);

    if (!buf)
	return NULL;
    
    strcpy(buf, dir);
    if (need_slash)
	strcpy(buf + dirlen++, "/");
    else {
	while (dirlen > 0 && buf[dirlen-1] == '/')
	    dirlen--;
	dirlen++;
    }
    while (*file == '/')
	file++;
    strcpy(buf + dirlen, file);
    return buf;
}

