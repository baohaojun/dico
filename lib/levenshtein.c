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
#include <string.h>

#ifndef MIN
# define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#if 0
# define DEBUG(c) printf("%d ", c);
# define DEBUGNL() putchar('\n')
#else
# define DEBUG(c)
# define DEBUGNL()
#endif

int
dico_levenshtein_distance(const char *astr, const char *bstr, int flags)
{
    unsigned *a, *b;
    int alen;
    int blen;
    unsigned *rowptr;
    unsigned *row[3];
    int i, j, idx, nrows;
    int dist;
    int (*conv) (const char *, unsigned **, size_t *) =
	(flags & DICO_LEV_NORM) ? utf8_mbstr_to_norm_wc : utf8_mbstr_to_wc;
    
    if (conv(astr, &a, NULL))
	return -1;
    if (conv(bstr, &b, NULL)) {
	free(a);
	return -1;
    }
    alen = utf8_wc_strlen(a);
    blen = utf8_wc_strlen(b);
    
    rowptr = calloc(sizeof(rowptr[0]),
		    (2 + !!(flags & DICO_LEV_DAMERAU)) * (blen + 1));
    row[0] = rowptr;
    row[1] = rowptr + blen + 1;
    if (flags & DICO_LEV_DAMERAU) {
	nrows = 3;
	row[2] = row[1] + blen + 1;
    } else
	nrows = 2;

    for (i = 0; i < blen + 1; i++) {
	row[0][i] = i;
	DEBUG(row[0][i]);
    }
    DEBUGNL();
    idx = 1;
    
    for (i = 0; i < alen; i++, idx = (idx + 1) % nrows ) {
	row[idx][0] = i + 1;	
	DEBUG(row[idx][0]);
	for (j = 0; j < blen; j++) { 
	    unsigned n, cost;
	    int prev = (idx + nrows - 1) % nrows;
	    
	    cost = !(utf8_wc_toupper(a[i]) == utf8_wc_toupper(b[j]));
	    n = MIN(row[prev][j+1] + 1,   /* Deletion */
		    row[idx][j] + 1);     /* Insertion */
	    n = MIN(n, row[prev][j] + cost); /* Substitution */
	    if (flags & DICO_LEV_DAMERAU) {
		if (i > 0 && j > 0
		    && utf8_wc_toupper(a[i]) == utf8_wc_toupper(b[j-1])
		    && utf8_wc_toupper(a[i-1]) == utf8_wc_toupper(b[j]))
		    /* Transposition */
		    n = MIN(n, row[(idx + 1) % nrows][j - 1] + cost);
	    }
	    row[idx][j+1] = n;
	    DEBUG(row[idx][j+1]);
	}
	dist = row[idx][blen];
	DEBUGNL();
    }

    free(rowptr);
    free(a);
    free(b);
    return dist;
}
	
