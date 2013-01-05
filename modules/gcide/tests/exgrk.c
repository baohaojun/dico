/* Extract greek transliterations from GCIDE.  This file is part of GNU Dico.
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

/* This program extracts greek transliterations from input GCIDE dictionaries.
   Sample usage:

     cat CIDE.[A-Z] | exgrk | sort | uniq
*/

#include <stdio.h>

enum {
    S_INIT,
    S_OPEN,
    S_OG,
    S_OR,
    S_OK,
    S_GRK
};

int
main(int argc, char **argv)
{
    int c;
    int state = S_INIT;
    
    while ((c = getc(stdin)) > 0) {
	switch (state) {
	case S_INIT:
	    if (c == '<')
		state = S_OPEN;
	    break;
	    
	case S_OPEN:
	    if (c == 'g')
		state = S_OG;
	    else
		state = S_INIT;
	    break;
	    
	case S_OG:
	    if (c == 'r')
		state = S_OR;
	    else
		state = S_INIT;
	    break;
	    
	case S_OR:
	    if (c == 'k')
		state = S_OK;
	    else 
		state = S_INIT;
	    break;
	    
	case S_OK:
	    if (c == '>')
		state = S_GRK;
	    else 
		state = S_INIT;
	    break;

	case S_GRK:
	    if (c == '<') {
		state = S_INIT;
		c = '\n';
	    }
	    putchar(c);
	    break;
	}
    }
    return 0;
}
