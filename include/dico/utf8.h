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

#ifndef __dico_utf8_h
#define __dico_utf8_h

#include <ctype.h>

size_t utf8_char_width(const char *p);
size_t utf8_strlen (const char *s);
size_t utf8_strbytelen (const char *s);

struct utf8_iterator {
    char *string;
    char *curptr;
    unsigned curwidth;
};

#define utf8_iter_isascii(itr) \
 ((itr).curwidth == 1 && isascii((itr).curptr[0]))

int utf8_iter_end_p(struct utf8_iterator *itr);
int utf8_iter_first(struct utf8_iterator *itr, char *ptr);
int utf8_iter_next(struct utf8_iterator *itr);

int utf8_mbtowc_internal (void *data, int (*read) (void*), unsigned int *pwc);
int utf8_wctomb (char *r, unsigned int wc);

int utf8_symcmp(char *a, char *b);
int utf8_symcasecmp(char *a, char *b);
int utf8_strcasecmp(char *a, char *b);
int utf8_strncasecmp(char *a, char *b, size_t maxlen);

unsigned utf8_wc_toupper (unsigned wc);
int utf8_toupper (char *s);
unsigned utf8_wc_tolower (unsigned wc);
int utf8_tolower (char *s);
size_t utf8_wc_strlen (const unsigned *s);
unsigned *utf8_wc_strdup (const unsigned *s);
size_t utf8_wc_hash_string (const unsigned *ws, size_t n_buckets);
int utf8_wc_strcmp (const unsigned *a, const unsigned *b);
int utf8_wc_strncmp(const unsigned *a, const unsigned *b, size_t n);
int utf8_wc_strcasecmp(const unsigned *a, const unsigned *b);
int utf8_wc_strncasecmp(const unsigned *a, const unsigned *b, size_t n);

int utf8_wc_to_mbstr(const unsigned *wordbuf, size_t wordlen, char **sptr);

int utf8_mbstr_to_wc(const char *str, unsigned **wptr, size_t *plen);
int utf8_mbstr_to_norm_wc(const char *str, unsigned **nptr, size_t *plen);

int utf8_quote (const char *str, char **sptr);
unsigned *utf8_wc_quote (const unsigned *s);

const unsigned *utf8_wc_strchr(const unsigned *str, unsigned chr);
const unsigned *utf8_wc_strchr_ci(const unsigned *str, unsigned chr);
const unsigned *utf8_wc_strstr(const unsigned *haystack,
			       const unsigned *needle);

void utf8_wc_strupper(unsigned *str);
void utf8_wc_strlower(unsigned *str);

#define DICO_LEV_NORM    0x1
#define DICO_LEV_DAMERAU 0x2

int dico_levenshtein_distance(const char *a, const char *b, int flags);
#define DICO_SOUNDEX_SIZE 5
int dico_soundex(const char *s, char codestr[DICO_SOUNDEX_SIZE]);

#endif

