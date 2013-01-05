/* This file is part of GNU Dico
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dico.h>

#define CRLF_IN  1
#define CRLF_OUT 2

int
main(int argc, char **argv)
{
    int rc;
    char *filename = NULL;
    size_t sz;
    dico_stream_t in, out, s;
    char buf[512];
    int flags = 0;
    size_t in_bufsize = 1024, out_bufsize = 1024;
    int buffered;
    
    dico_set_program_name(argv[0]);

    while (--argc) {
	char *arg = *++argv;
	if (strncmp(arg, "-file=", 6) == 0)
	    filename = arg + 6;
	else if (strcmp(arg, "-in") == 0)
	    flags |= CRLF_IN;
	else if (strcmp(arg, "-out") == 0)
	    flags |= CRLF_OUT;
	else if (strncmp(arg, "-bufsize=", 9) == 0) {
	    in_bufsize = out_bufsize = atoi(arg + 9);
	    buffered = CRLF_IN | CRLF_OUT;
	} else if (strncmp(arg, "-outbufsize=", 12) == 0) {
	    out_bufsize = atoi(arg + 12);
	    buffered |= CRLF_OUT;
	} else if (strncmp(arg, "-inbufsize=", 11) == 0)
	    in_bufsize |= atoi(arg + 11);
	else if (strcmp(arg, "--") == 0) {
	    --argc;
	    ++argv;
	} else if (arg[0] == '-') {
	    dico_log(L_ERR, 0, "unknown option '%s'", arg);
	    return 1;
	} else
	    break;
    }
    if (argc) {
	fprintf(stderr, "Usage: %s [-file=S]\n", dico_program_name);
	return 1;
    }

    if (filename) {
	in = dico_mapfile_stream_create(filename, DICO_STREAM_READ);
	if (!in) {
	    dico_log(L_ERR, errno, "cannot create stream `%s'", filename);
	    return 1;
	}
    } else {
	in = dico_fd_stream_create(0, DICO_STREAM_READ, 1);
	if (!in) {
	    dico_log(L_ERR, errno, "cannot create stdin stream");
	    return 1;
	}
    }
	
    rc = dico_stream_open(in);
    if (rc) {
	dico_log(L_ERR, 0,
		 "cannot open stream `%s': %s",
		 filename ? filename : "<stdin>",
		 dico_stream_strerror(in, rc));
	return 2;
    }

    if (flags & CRLF_IN) {
	s = dico_crlf_stream(in, DICO_STREAM_READ, 0);
	if (!s) {
	    dico_log(L_ERR, errno, "cannot create filter stream");
	    return 2;
	}
	if (filename || (buffered & CRLF_IN))
	    dico_stream_set_buffer(s, dico_buffer_full, in_bufsize);

	in = s;
    }
    
    out = dico_fd_stream_create(1, DICO_STREAM_WRITE, 1);
    if (!out) {
	dico_log(L_ERR, errno, "cannot create stdout stream");
	return 2;
    }

    rc = dico_stream_open(out);
    if (rc) {
	dico_log(L_ERR, 0,
		 "cannot open stream `%s': %s",
		 "<stdout>",
		 dico_stream_strerror(out, rc));
	return 2;
    }

    if (flags & CRLF_OUT) {
	s = dico_crlf_stream(out, DICO_STREAM_WRITE, 0);
	if (!s) {
	    dico_log(L_ERR, errno, "cannot create filter stream");
	    return 2;
	}
	if (filename || (buffered & CRLF_OUT))
	    dico_stream_set_buffer(s, dico_buffer_full, out_bufsize);
	
	out = s;
    }

    while ((rc = dico_stream_read(in, buf, sizeof(buf), &sz)) == 0 && sz) {
	rc = dico_stream_write(out, buf, sz);
	if (rc) {
	    dico_log(L_ERR, 0, "write error: %s",
		     dico_stream_strerror(out, rc));
	    return 2;
	}
    }

    dico_stream_close(out);
    return 0;
}
