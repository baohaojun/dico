/* grecs - Gray's Extensible Configuration System
   Copyright (C) 2007-2012 Sergey Poznyakoff

   Grecs is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   Grecs is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with Grecs. If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "grecs.h"

int
grecs_vasprintf(char **pbuf, size_t *psize, const char *fmt, va_list ap)
{
	char *buf = *pbuf;
	size_t buflen = *psize;
	int rc = 0;
  
	if (!buf) {
		if (buflen == 0)
			buflen = 512; /* Initial allocation */
      
		buf = calloc(1, buflen);
		if (buf == NULL)
			return ENOMEM;
	}
  
	for (;;) {
		va_list aq;
		ssize_t n;

		va_copy(aq, ap);
		n = vsnprintf(buf, buflen, fmt, aq);
		va_end(aq);
		if (n < 0 || n >= buflen || !memchr(buf, '\0', n + 1)) {
			char *newbuf;
			size_t newlen = buflen * 2;
			if (newlen < buflen) {
				rc = ENOMEM;
				break;
			}
			newbuf = realloc(buf, newlen);
			if (newbuf == NULL) {
				rc = ENOMEM;
				break;
			}
			buflen = newlen;
			buf = newbuf;
		} else
			break;
	}

	if (rc) {
		if (!*pbuf) {
			/* We made first allocation, now free it */
			free(buf);
			buf = NULL;
			buflen = 0;
		}
	}
	
	*pbuf = buf;
	*psize = buflen;
	return rc;
}  

int
grecs_asprintf(char **pbuf, size_t *psize, const char *fmt, ...)
{
	int rc;
	va_list ap;
	
	va_start(ap, fmt);
	rc = grecs_vasprintf(pbuf, psize, fmt, ap);
	va_end(ap);
	return rc;
}
