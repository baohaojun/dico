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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dico.h>

#define ISWS(c) ((c) == ' ' || (c) == '\t')
#define ISQUOTE(c) ((c) == '"' || (c) == '\'')

static char quote_transtab[] = "\\\\\"\"a\ab\bf\fn\nr\rt\t";

int
dico_unquote_char(int c)
{
    char *p;

    for (p = quote_transtab; *p; p += 2) {
	if (*p == c)
	    return p[1];
    }
    return 0;
}

int
dico_quote_char(int c)
{
    char *p;

    for (p = quote_transtab; *p; p += 2) {
	if (p[1] == c)
	    return p[0];
    }
    return 0;
}

void
dico_tokenize_begin(struct dico_tokbuf *tb)
{
    memset(tb, 0, sizeof(*tb));
}

void
dico_tokenize_end(struct dico_tokbuf *tb)
{
    free (tb->tb_base);
    free (tb->tb_tokv);
}

void
dico_tokenize_clear(struct dico_tokbuf *tb)
{
    tb->tb_level = 0;
    tb->tb_tokc = 0;
}

static int
_dico_tkn_grow(struct dico_tokbuf *tb, char *str, size_t len)
{
    if (tb->tb_level + len > tb->tb_size) {
	size_t newsize = ((tb->tb_level + len + TKNBLOCKSIZ - 1) /
			  TKNBLOCKSIZ) * TKNBLOCKSIZ;
	char *newbase = realloc(tb->tb_base, newsize);
	if (!newbase)
	    return ENOMEM;
	tb->tb_base = newbase;
	tb->tb_size = newsize;
    }
    memcpy(tb->tb_base + tb->tb_level, str, len);
    tb->tb_level += len;
    return 0;
}

static int
_dico_tkn_1grow(struct dico_tokbuf *tb, int ch)
{
    char c = ch;
    return _dico_tkn_grow(tb, &c, 1);
}

int
dico_tokenize_string(struct dico_tokbuf *tb, char *str)
{
    struct utf8_iterator itr;
    int i, argc = 0;
    int rc;
    size_t start_level;
    char *p;

    utf8_iter_first(&itr, str);

    start_level = tb->tb_level;
	
    for (rc = 0; rc == 0;) {
	int quote;
	
	for (; !utf8_iter_end_p(&itr)
		 && utf8_iter_isascii(itr) && ISWS(*itr.curptr);
	     utf8_iter_next(&itr))
	    ;

	if (utf8_iter_end_p(&itr))
	    break;

	if (utf8_iter_isascii(itr) && ISQUOTE(*itr.curptr)) {
	    quote = *itr.curptr;
	    utf8_iter_next(&itr);
	} else
	    quote = 0;

	for (; !utf8_iter_end_p(&itr)
		 && !(utf8_iter_isascii(itr) && (quote ? 0 : ISWS(*itr.curptr)));
	     utf8_iter_next(&itr)) {
	    if (utf8_iter_isascii(itr)) {
		if (*itr.curptr == quote) {
		    utf8_iter_next(&itr);
		    break;
		} else if (*itr.curptr == '\\') {
		    utf8_iter_next(&itr);
		    if (utf8_iter_isascii(itr)) {
			rc = _dico_tkn_1grow(tb, dico_quote_char(*itr.curptr));
		    } else {
			rc = _dico_tkn_1grow(tb, '\\');
			if (rc == 0)
			    rc = _dico_tkn_grow(tb, itr.curptr, itr.curwidth);
		    }
		    continue;
		}
	    }
	    rc = _dico_tkn_grow(tb, itr.curptr, itr.curwidth);
	}
	if (rc == 0)
	    rc = _dico_tkn_1grow(tb, 0);
	argc++;
    }

    if (rc)
	return rc;
    
    if (tb->tb_tokc + argc + 1 > tb->tb_tokm) {
	size_t nmax = tb->tb_tokc + argc + 1;
	char **nargv = realloc(tb->tb_tokv, sizeof(tb->tb_tokv[0]) * nmax);
	if (!nargv)
	    return ENOMEM;
	tb->tb_tokv = nargv;
	tb->tb_tokm = nmax;
    }

    for (i = 0, p = tb->tb_base + start_level; i < argc; i++) {
	tb->tb_tokv[tb->tb_tokc++] = p;
	p += strlen(p) + 1;
    }
    tb->tb_tokv[tb->tb_tokc] = NULL;
    return 0;
}

