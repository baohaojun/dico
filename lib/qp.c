/* This file is part of GNU Dico
   Copyright (C) 1999-2001, 2004-2005, 2007-2008, 2012 Free Software
   Foundation, Inc.
  
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

#define ISWS(c) ((c)==' ' || (c)=='\t')

static int
dico_qp_decode(const char *iptr, size_t isize, char *optr, size_t osize,
	       size_t *pnbytes)
{
    char c;
    int last_char = 0;
    size_t consumed = 0;
    size_t wscount = 0;
    size_t nbytes = 0;
    
    while (consumed < isize && nbytes < osize) {
	c = *iptr++;

	if (ISWS(c)) {
	    wscount++;
	    consumed++;
	}
	else {
	    /* Octets with values of 9 and 32 MAY be
	       represented as US-ASCII TAB (HT) and SPACE characters,
	       respectively, but MUST NOT be so represented at the end
	       of an encoded line.  Any TAB (HT) or SPACE characters
	       on an encoded line MUST thus be followed on that line
	       by a printable character. */
	  
	    if (wscount) {
		if (c != '\r' && c != '\n') {
		    size_t sz;
		  
		    if (consumed >= isize)
			break;

		    if (nbytes + wscount > osize)
			sz = osize - nbytes;
		    else
			sz = wscount;
		    memcpy(optr, iptr - wscount - 1, sz);
		    optr += sz;
		    nbytes += sz;
		    if (wscount > sz) {
			wscount -= sz;
			break;
		    }
		}
		wscount = 0;
		if (nbytes == osize)
		    break;
	    }
		
	    if (c == '=') {
		/* There must be 2 more characters before I consume this.  */
		if (consumed + 2 >= isize)
		    break;
		else {
		    /* you get =XX where XX are hex characters.  */
		    char chr[3];
		    int new_c;

		    chr[2] = 0;
		    chr[0] = *iptr++;
		    /* Ignore LF.  */
		    if (chr[0] != '\n') {
			chr[1] = *iptr++;
			new_c = strtoul (chr, NULL, 16);
			*optr++ = new_c;
			nbytes++;
			consumed += 3;
		    } else
			consumed += 2;
		}
	    }
	    /* CR character.  */
	    else if (c == '\r') {
		/* There must be at least 1 more character before
		   I consume this.  */
		if (consumed + 1 >= isize)
		    break;
		else {
		    iptr++; /* Skip the CR character.  */
		    *optr++ = '\n';
		    nbytes++;
		    consumed += 2;
		}
	    } else {
		*optr++ = c;
		nbytes++;
		consumed++;
	    }
	}	  
	last_char = c;
    }
    *pnbytes = nbytes;
    return consumed - wscount;
}

static int
dico_qp_encode(const char *iptr, size_t isize, char *optr, size_t osize,
	       size_t *pnbytes)
{
    unsigned int c;
    size_t consumed = 0;
    size_t nbytes = 0;
    static const char _hexdigits[] = "0123456789ABCDEF";

    nbytes = 0;

    /* Strategy: check if we have enough room in the output buffer only
       once the required size has been computed. If there is not enough,
       return and hope that the caller will free up the output buffer a
       bit. */

    while (consumed < isize) {
	int simple_char;
      
	/* candidate byte to convert */
	c = *(unsigned char*) iptr;
	simple_char = (c >= 32 && c <= 60)
	               || (c >= 62 && c <= 126)
	               || c == '\t'
	               || c == '\n';

	if (simple_char) {
	    /* a non-quoted character uses up one byte */
	    if (nbytes + 1 > osize) 
		break;

	    *optr++ = c;
	    nbytes++;
	    
	    iptr++;
	    consumed++;

	} else {
	    /* a quoted character uses up three bytes */
	    if (nbytes + 3 > osize) 
		break;

	    *optr++ = '=';
	    *optr++ = _hexdigits[(c >> 4) & 0xf];
	    *optr++ = _hexdigits[c & 0xf];

	    nbytes += 3;

	    /* we've actuall used up one byte of input */
	    iptr++;
	    consumed++;
	}
    }
    *pnbytes = nbytes;
    return consumed;
}


dico_stream_t
dico_qp_stream_create(dico_stream_t str, int mode)
{
    return filter_stream_create(str, 4, 0,
				mode == FILTER_ENCODE ?
				  dico_qp_encode : dico_qp_decode,
				mode);
}
