/* This file is part of GNU Dico.
   Copyright (C) 2012 Sergey Poznyakoff

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

#ifndef __dico_tokenize_h
#define __dico_tokenize_h

#include <dico/types.h>

struct dico_tokbuf {
    char *tb_base;
    size_t tb_size;
    size_t tb_level;
    char **tb_tokv;
    int tb_tokm;
    int tb_tokc;
};

#define TKNBLOCKSIZ 256

int dico_unquote_char(int c);
int dico_quote_char(int c);
void dico_tokenize_begin(struct dico_tokbuf *bp);
void dico_tokenize_end(struct dico_tokbuf *bp);
void dico_tokenize_clear(struct dico_tokbuf *bp);
int dico_tokenize_string(struct dico_tokbuf *bp, char *str);

#endif
