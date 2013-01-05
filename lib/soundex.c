/* This file is part of GNU Dico
   Copyright (C) 2007-2008, 2010, 2012 Sergey Poznyakoff
 
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
#include <ctype.h>

static unsigned char soundextbl[] = {
/* A */     0,
/* B */     1,
/* C */     2,
/* D */     3,
/* E */     0,
/* F */     1,
/* G */     2,
/* H */     0,
/* I */     0,
/* J */     2,
/* K */     2,
/* L */     4,
/* M */     5,
/* N */     5,
/* O */     0,
/* P */     1,
/* Q */     2,
/* R */     6,
/* S */     2,
/* T */     3,
/* U */     0,
/* V */     1,
/* W */     0,
/* X */     2,
/* Y */     0,
/* Z */     2
};

#define soundex_code(c) soundextbl[toupper(c)-'A']

int
dico_soundex(const char *s, char codestr[DICO_SOUNDEX_SIZE])
{
    int i, prev = 0;
    
    codestr[0] = toupper(*s);
    for (i = 1, s++; i < DICO_SOUNDEX_SIZE-1 && *s; s++) {
	int n =  soundex_code(*s);
	if (n) {
	    if (n == prev)
		continue;
	    codestr[i++] = n + '0';
	    prev = n;
	}
    }
    for (; i < DICO_SOUNDEX_SIZE-1; i++)
	codestr[i] = '0';
    codestr[i] = 0;
    return 0;
}
