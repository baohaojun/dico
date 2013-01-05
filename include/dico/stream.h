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

#ifndef __dico_stream_h
#define __dico_stream_h


/* Streams */

enum dico_buffer_type {
    dico_buffer_none,
    dico_buffer_line,
    dico_buffer_full
};

#define DICO_STREAM_READ   0x01
#define DICO_STREAM_WRITE  0x02
#define DICO_STREAM_SEEK   0x04

#define DICO_SEEK_SET      0
#define DICO_SEEK_CUR      1
#define DICO_SEEK_END      2

int dico_stream_create(dico_stream_t *pstream, int flags, void *data);
int dico_stream_open(dico_stream_t stream);
void dico_stream_set_open(dico_stream_t stream, int (*openfn) (void *, int));
void dico_stream_set_seek(dico_stream_t stream,
			  int (*fun_seek) (void *, off_t, int, off_t *));
void dico_stream_set_size(dico_stream_t stream,
			  int (*sizefn) (void *, off_t *));
void dico_stream_set_read(dico_stream_t stream,
			  int (*readfn) (void *, char *, size_t, size_t *));
void dico_stream_set_write(dico_stream_t stream,    
			   int (*writefn) (void *, const char *, size_t,
					   size_t *));
void dico_stream_set_flush(dico_stream_t stream, int (*flushfn) (void *));
void dico_stream_set_close(dico_stream_t stream, int (*closefn) (void *));
void dico_stream_set_destroy(dico_stream_t stream, int (*destroyfn) (void *));

void dico_stream_set_error_string(dico_stream_t stream,
				  const char *(*error_string) (void *, int));

int dico_stream_set_buffer(dico_stream_t stream,
			   enum dico_buffer_type type,
			   size_t size);

off_t dico_stream_seek(dico_stream_t stream, off_t offset, int whence);
int dico_stream_size(dico_stream_t stream, off_t *psize);


int dico_stream_read_unbuffered(dico_stream_t stream, void *buf, size_t size,
				size_t *pread);
int dico_stream_write_unbuffered(dico_stream_t stream,
				 const void *buf, size_t size,
				 size_t *pwrite);

int dico_stream_read(dico_stream_t stream, void *buf, size_t size,
		     size_t *pread);
int dico_stream_readln(dico_stream_t stream, char *buf, size_t size,
		       size_t *pread);
int dico_stream_getdelim(dico_stream_t stream, char **pbuf, size_t *psize,
			 int delim, size_t *pread);
int dico_stream_getline(dico_stream_t stream, char **pbuf, size_t *psize,
			size_t *pread);
int dico_stream_write(dico_stream_t stream, const void *buf, size_t size);
int dico_stream_writeln(dico_stream_t stream, const char *buf, size_t size);

const char *dico_stream_strerror(dico_stream_t stream, int rc);
int dico_stream_last_error(dico_stream_t stream);
void dico_stream_clearerr(dico_stream_t stream);
int dico_stream_eof(dico_stream_t stream);

int dico_stream_flush(dico_stream_t stream);
int dico_stream_close(dico_stream_t stream);
void dico_stream_destroy(dico_stream_t *stream);

off_t dico_stream_bytes_in(dico_stream_t stream);
off_t dico_stream_bytes_out(dico_stream_t stream);

void dico_stream_set_ioctl(dico_stream_t stream,
			   int (*ctl) (void *, int, void *));
int dico_stream_ioctl(dico_stream_t stream, int code, void *ptr);


/* IOCTLS */
#define DICO_IOCTL_GET_TRANSPORT 0
#define DICO_IOCTL_SET_TRANSPORT 1
#define DICO_DBG_CTL_SET_FILE    2
#define DICO_DBG_CTL_SET_LINE    3
#define DICO_DBG_CTL_SET_TS      4
#define DICO_IOCTL_BYTES_IN      5
#define DICO_IOCTL_BYTES_OUT     6
#define DICO_IOCTL_SET_LINELEN   7
#define DICO_IOCTL_GET_LINELEN   8


/* FD streams */
dico_stream_t dico_fd_stream_create(int fd, int flags, int noclose);
dico_stream_t dico_fd_io_stream_create(int ifd, int ofd);

dico_stream_t dico_io_stream(dico_stream_t in, dico_stream_t out);
dico_stream_t dico_mapfile_stream_create(const char *filename, int flags);

dico_stream_t dico_linetrim_stream(dico_stream_t transport, size_t maxlen,
				   int noclose);
dico_stream_t dico_crlf_stream(dico_stream_t transport, int flags, int noclose);

#endif
