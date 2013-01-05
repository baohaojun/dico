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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct hdr_buf
{
    char *base;
    size_t size;
    size_t level;
};

#define HDR_BUF_SIZE 128
#define HDR_BUF_INIT { NULL, 0, 0 }

#define ISWS(c) ((c) == ' ' || (c) == '\t')

static int
hdr_buf_append(struct hdr_buf *buf, const char *str, size_t len)
{
    if (len == 0)
	return 0;
    
    if (buf->size == 0) {
	size_t size = HDR_BUF_SIZE * ((len + HDR_BUF_SIZE - 1) / HDR_BUF_SIZE);
	buf->base = malloc(size);
	if (!buf->base)
	    return 1;
	buf->size = size;
    } else if (buf->level + len > buf->size) {
	char *p;
	size_t diff = buf->level + len - buf->size;
	size_t size = buf->size * ((diff + buf->size - 1) / buf->size + 1);
	if (size < buf->size) {
	    errno = ENOMEM;
	    return 1;
	}
	p = realloc(buf->base, size);
	if (!p)
	    return 1;
	buf->base = p;
	buf->size = size;
    }
    memcpy(buf->base + buf->level, str, len);
    buf->level += len;
    return 0;
}

#define hdr_buf_clear(b) do { (b)->level = 0; } while (0)
#define hdr_buf_free(b) do { free((b)->base); } while (0)

static int
collect_line(const char **ptext, dico_assoc_list_t asc, struct hdr_buf *hbuf)
{
    const char *text = *ptext;
    char c, *p;
    size_t n;

    hdr_buf_clear(hbuf);
    do {
	if (ISWS(*text)) {
	    while (*text && ISWS(*text))
		text++;
	    text--;
	}
	n = strcspn(text, "\n");
	
	if (n == 0) {
	    text += strlen(text);
	    break;
	}
	    
	if (hdr_buf_append(hbuf, text, n)) 
	    return 1;

	text += n;
	if (!*text)
	    break;
	text++;
    } while (ISWS(*text));
    
    c = 0;
    if (hdr_buf_append(hbuf, &c, 1))
	return 1;
    p = strchr(hbuf->base, ':');
    if (!p) {
	errno = EINVAL;
	return 1;
    }
    *p++ = 0;
    while (*p && ISWS(*p))
	p++;
    if (dico_assoc_append(asc, hbuf->base, p))
	return 1;
    *ptext = text;
    return 0;
}    

int
dico_header_parse(dico_assoc_list_t *pasc, const char *text)
{
    int rc = 0;
    struct hdr_buf hbuf = HDR_BUF_INIT;
    dico_assoc_list_t asc = dico_assoc_create(DICO_ASSOC_CI|DICO_ASSOC_MULT);
    
    if (!asc)
	return 1;

    if (text) {
	while (*text && *text != '\n'
	       && (rc = collect_line(&text, asc, &hbuf)) == 0)
	    ;
	hdr_buf_free(&hbuf);
    }

    if (rc) {
	int ec = errno;
	dico_assoc_destroy(&asc);
	errno = ec;
    } else
	*pasc = asc;
    return rc;
}
	    
