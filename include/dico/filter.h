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

#ifndef __dico_filter_h
#define __dico_filter_h

#include <dico/stream.h>

#define FILTER_ENCODE 0
#define FILTER_DECODE 1

typedef int (*filter_xcode_t) (const char *, size_t, char *, size_t, size_t *);

dico_stream_t filter_stream_create(dico_stream_t str,
				   size_t min_level,
				   size_t max_line_length,
				   filter_xcode_t xcode,
				   int mode);
dico_stream_t dico_codec_stream_create(const char *encoding, int mode,
				       dico_stream_t transport);

dico_stream_t dico_base64_stream_create(dico_stream_t str, int mode);
dico_stream_t dico_qp_stream_create(dico_stream_t str, int mode);

int dico_base64_input(unsigned);

int dico_base64_encode(const unsigned char *input, size_t input_len,
		       unsigned char **output, size_t *output_len);
int dico_base64_decode (const unsigned char *input, size_t input_len,
			unsigned char **output, size_t *output_len);


#endif
