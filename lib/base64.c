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
#include <errno.h>

static char b64tab[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int b64val[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

int
dico_base64_input(unsigned n)
{
    if (n < DICO_ARRAY_SIZE(b64val))
	return b64val[n];
    return -1;
}

int
dico_base64_encode(const unsigned char *input, size_t input_len,
		   unsigned char **output, size_t *output_len)
{
    size_t olen = 4 * (input_len + 2) / 3 + 1;
    unsigned char *out = malloc(olen);

    if (!out)
	return 1;
    *output = out;
    while (input_len >= 3) {
	*out++ = b64tab[input[0] >> 2];
	*out++ = b64tab[((input[0] << 4) & 0x30) | (input[1] >> 4)];
	*out++ = b64tab[((input[1] << 2) & 0x3c) | (input[2] >> 6)];
	*out++ = b64tab[input[2] & 0x3f];
	input_len -= 3;
	input += 3;
    }

    if (input_len > 0) {
	unsigned char c = (input[0] << 4) & 0x30;
	*out++ = b64tab[input[0] >> 2];
	if (input_len > 1)
	    c |= input[1] >> 4;
	*out++ = b64tab[c];
	*out++ = (input_len < 2) ? '=' : b64tab[(input[1] << 2) & 0x3c];
	*out++ = '=';
    }
    *output_len = out - *output;
    *out = 0;
    return 0;
}

int
dico_base64_decode (const unsigned char *input, size_t input_len,
		    unsigned char **output, size_t *output_len)
{
    int olen = input_len;
    unsigned char *out = malloc(olen);
    
    if (!out)
	return -1;
    *output = out;
    do {
	if (input[0] > 127 || b64val[input[0]] == -1
	    || input[1] > 127 || b64val[input[1]] == -1
	    || input[2] > 127 || ((input[2] != '=') && (b64val[input[2]] == -1))
	    || input[3] > 127 || ((input[3] != '=')
				  && (b64val[input[3]] == -1))) {
	    errno = EINVAL;
	    return -1;
	}
	*out++ = (b64val[input[0]] << 2) | (b64val[input[1]] >> 4);
	if (input[2] != '=') {
	    *out++ = ((b64val[input[1]] << 4) & 0xf0) | (b64val[input[2]] >> 2);
	    if (input[3] != '=')
		*out++ = ((b64val[input[2]] << 6) & 0xc0) | b64val[input[3]];
	}
	input += 4;
	input_len -= 4;
    } while (input_len > 0);
    *output_len = out - *output;
    return 0;
}


static int
_flt_base64_decode(const char *ibuf, size_t isize, char *optr, size_t osize,
		   size_t *pnbytes)
{
    int i = 0, tmp = 0, pad = 0;
    size_t consumed = 0;
    unsigned char data[4];
    size_t nbytes = 0;
    const unsigned char *iptr = (const unsigned char *) ibuf;
    
    while (consumed < isize && nbytes + 3 < osize) {
	while (i < 4 && consumed < isize) {
	    tmp = b64val[*iptr++];
	    consumed++;
	    if (tmp != -1)
		data[i++] = tmp;
	    else if (*(iptr-1) == '=') {
		data[i++] = 0;
		pad++;
	    }
	}

	/* I have a entire block of data 32 bits get the output data.  */
	if (i == 4) {
	    *optr++ = (data[0] << 2) | ((data[1] & 0x30) >> 4);
	    *optr++ = ((data[1] & 0xf) << 4) | ((data[2] & 0x3c) >> 2);
	    *optr++ = ((data[2] & 0x3) << 6) | data[3];
	    nbytes += 3 - pad;
	} else {
	    /* I did not get all the data.  */
	    consumed -= i;
	    break;
	}
	i = 0;
    }
    *pnbytes = nbytes;
    return consumed;
}

static int
_flt_base64_encode (const char *iptr, size_t isize,
		    char *optr, size_t osize,
		    size_t *pnbytes)
{
    size_t consumed = 0;
    int pad = 0;
    const unsigned char* ptr = (const unsigned char*) iptr;
    size_t nbytes = 0;
  
    if (isize <= 3)
	pad = 1;
    while ((consumed + 3 <= isize && nbytes + 4 <= osize) || pad) {
	unsigned char c1 = 0, c2 = 0, x = '=', y = '=';
	
        *optr++ = b64tab[ptr[0] >> 2];
	consumed++;
	switch (isize - consumed) {
	default:
	    consumed++;
	    y = b64tab[ptr[2] & 0x3f];
	    c2 = ptr[2] >> 6;
	case 1:
	    consumed++;
	    x = b64tab[((ptr[1] << 2) + c2) & 0x3f];
	    c1 = (ptr[1] >> 4);
	case 0:
	    *optr++ = b64tab[((ptr[0] << 4) + c1) & 0x3f];
	    *optr++ = x;
	    *optr++ = y;
	}
	
	ptr += 3;
	nbytes += 4;
	pad = 0;
    }

    *pnbytes = nbytes;
    return consumed;
}

dico_stream_t
dico_base64_stream_create(dico_stream_t str, int mode)
{
    return filter_stream_create(str, 3, 76,
				mode == FILTER_ENCODE ?
				  _flt_base64_encode : _flt_base64_decode,
				mode);
}
