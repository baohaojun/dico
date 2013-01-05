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

#ifndef __dico_xlat_h
#define __dico_xlat_h

/* Simple translation tables */
struct xlat_tab {
    char *string;
    int num;
};

#define XLAT_ICASE 0x01

int xlat_string(struct xlat_tab *tab, const char *string, size_t len,
		int flags, int *result);
int xlat_c_string(struct xlat_tab *tab, const char *string, int flags,
		  int *result);

#endif
