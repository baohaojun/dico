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
#include <xdico.h>
#include <stdio.h>
#include <ctype.h>
#include <libi18n.h>

int
stream_writez(dico_stream_t str, const char *buf)
{
    return dico_stream_write(str, buf, strlen(buf));
}

int
stream_printf(dico_stream_t str, const char *fmt, ...)
{
    int len;
    char *buf;
    
    va_list ap;
    va_start(ap, fmt);
    len = vasprintf(&buf, fmt, ap);
    va_end(ap);
    if (len < 0) {
	dico_log(L_CRIT, 0,
	       _("not enough memory while formatting reply message"));
	exit(1);
    }
    len = dico_stream_write(str, buf, len);
    free(buf);
    return len;
}

void
stream_write_multiline(dico_stream_t str, const char *text)
{
    struct utf8_iterator itr;
    size_t len = 0;
    
    for (utf8_iter_first(&itr, (char*)text);
	 !utf8_iter_end_p(&itr);
	 utf8_iter_next(&itr)) {
	if (utf8_iter_isascii(itr) && *itr.curptr == '\n') {
	    dico_stream_writeln(str, itr.curptr - len, len);
	    len = 0;
	} else
	    len += itr.curwidth;
    }
    if (len)
	dico_stream_writeln(str, itr.curptr - len, len);
}

