/* This file is part of Dico.
   Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff

   Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Dico.  If not, see <http://www.gnu.org/licenses/>. */

#include "dictorg.h"

#define DE_UNKNOWN_FORMAT -1
#define DE_UNSUPPORTED_FORMAT -2
#define DE_UNSUPPORTED_VERSION -3
#define DE_BAD_HEADER -4
#define DE_GZIP_SEEK -5
#define DE_INFLATE -6

struct _dict_chunk {
    unsigned length;             /* Chunk length */
    unsigned offset;             /* Chunk offset */ 
};

struct _dict_chunk_cache {
    int num;                     /* Chunk number */
    char *buffer;                /* Cunk contents */
    size_t size;                 /* Size of data in buffer */
    unsigned refcount;           /* Reference count */
};

struct _dict_stream {
    int type;                    /* Stream type (see DICTORG_ constants) */
    dico_stream_t transport;     /* Underlying transport stream */
    int transport_error;         /* Last error on transport stream */
#ifdef USE_LIBZ    
    /* Gzip/dict.org header */ 
    size_t header_length;        /* Header length. */ 
    int method;                  /* Compression method */
    int flags;
    time_t mtime;                /* Mtime of the original file */
    int extra_flags;
    int os;                      /* OS (see GZ_OS_ constants) */
    int version;                 /* Format version (must be 1) */
    unsigned chunk_size;         /* Chunk size */
    unsigned chunk_count;        /* Number of chunks in the file */
    struct _dict_chunk *chunk;   /* Chunk descriptions */
    char *orig_name;             /* Original file name */ 
    char *comment;               /* Comment */
    unsigned long crc;
    unsigned long size;          /* Original file size */
    unsigned long compressed_size; /* Compressed file size */
    size_t offset;               /* Offset in stream */
    char *buffer;                /* Decompression buffer (chunk_size bytes) */
    /* Z stream */
    z_stream      zstream;
    int           zstr_ready;
    
    /* Chunk cache */
    size_t cache_size;                /* Max. number of elements in cache */
    size_t cache_used;                /* Actual number of elements in cache */
    struct _dict_chunk_cache **cache; /**/
#endif
};


#ifdef USE_LIBZ    
static struct _dict_chunk_cache *
cache_create_chunk(struct _dict_stream *str)
{
    struct _dict_chunk_cache *cp = malloc(sizeof(*cp) + str->chunk_size);
    if (cp) {
	cp->buffer = (char*) (cp + 1);
	cp->num = -1;
	cp->refcount = 0;
	cp->size = 0;
    }
    return cp;
}

static int
cache_alloc(struct _dict_stream *str)
{
    int n;
    struct _dict_chunk_cache *cp;
    
    if (!str->cache) {
	str->cache = calloc(str->cache_size, sizeof(str->cache[0]));
	if (!str->cache)
	    return 1;
    }
    if (str->cache_used < str->cache_size) {
	cp = cache_create_chunk(str);
	if (!cp)
	    return 1;
	n = str->cache_used++;
	str->cache[n] = cp;
    } else {
	n = str->cache_used - 1;
	cp = str->cache[n];
	cp->refcount = 0;
	cp->num = -1;
    }
    return n;
}

static void
cache_destroy(struct _dict_stream *str)
{
    if (str->cache) {
	size_t i;
	for (i = 0; i < str->cache_used; i++) {
	    if (!str->cache[i])
		break;
	    free(str->cache[i]);
	}
	free(str->cache);
	str->cache = NULL;
    }
}

static void
cache_promote(struct _dict_stream *str, int n)
{
    int i;
    unsigned refcount = ++str->cache[n]->refcount;

    for (i = n - 1; i >= 0; i--)
	if (str->cache[i]->refcount >= refcount)
	    break;
    i++;
    if (i != n) {
	struct _dict_chunk_cache *tmp = str->cache[n];
	str->cache[n] = str->cache[i];
	str->cache[i] = tmp;
    }
}
	
static int
cache_get(struct _dict_stream *str, int chunk_num,
	  struct _dict_chunk_cache **retptr)
{
    int i;
    struct _dict_chunk_cache *cp;
    size_t rdbytes;
    
    for (i = 0; i < str->cache_used; i++) {
	if (str->cache[i]->num == chunk_num) {
	    cp = str->cache[i];
	    cache_promote(str, i);
	    *retptr = cp;
	    return 0;
	}
    }
    
    i = cache_alloc(str);
    cp = str->cache[i];
    cp->num = chunk_num;
    cache_promote(str, i);
    if (dico_stream_seek(str->transport, str->chunk[chunk_num].offset,
			 DICO_SEEK_SET) < 0)
	return str->transport_error = dico_stream_last_error(str->transport);

    if (!str->zstr_ready) {
	str->zstream.zalloc    = NULL;
	str->zstream.zfree     = NULL;
	str->zstream.opaque    = NULL;
	str->zstream.next_in   = 0;
	str->zstream.avail_in  = 0;
	str->zstream.next_out  = NULL;
	str->zstream.avail_out = 0;
	if (inflateInit2(&str->zstream, -15 ) != Z_OK) {
	    dico_log(L_ERR, 0, _("cannot initialize inflation engine: %s"),
		     str->zstream.msg);
	    return DE_INFLATE;
	}
	str->zstr_ready = 1;
    }

    if (dico_stream_read(str->transport, str->buffer,
			 str->chunk[chunk_num].length, &rdbytes) < 0)
	return str->transport_error = dico_stream_last_error(str->transport);
    
    str->zstream.next_in   = (unsigned char*) str->buffer;
    str->zstream.avail_in  = rdbytes;
    str->zstream.next_out  = (unsigned char*) cp->buffer;
    str->zstream.avail_out = str->chunk_size;

    if (inflate(&str->zstream, Z_PARTIAL_FLUSH) != Z_OK) {
	dico_log(L_ERR, 0, "inflate: %s", str->zstream.msg);
	return DE_INFLATE;
    }

    if (str->zstream.avail_in) {
	dico_log(L_ERR, 0, _("%s:%d: INTERNAL ERROR: "
			     "inflate did not flush (%d pending, %d avail)"),
		 __FILE__, __LINE__,
		 str->zstream.avail_in, str->zstream.avail_out );
	return DE_INFLATE;
    }
    
    cp->size = str->chunk_size - str->zstream.avail_out;
	
    *retptr = cp;
    return 0;
}

static int
_dict_read_dzip(struct _dict_stream *str, char *buf, size_t size, size_t *pret)
{
    int chunk_num = str->offset / str->chunk_size;
    size_t chunk_off = str->offset - chunk_num * str->chunk_size;
    size_t rdbytes = 0;
    int rc;
    
    while (size) {
	struct _dict_chunk_cache *cp;
	size_t n;
	
	rc = cache_get(str, chunk_num, &cp);
	if (rc)
	    break;
	n = cp->size - chunk_off; /* FIXME = str->chunk[chunk_num].length;? */
	if (n > size)
	    n = size;
	memcpy(buf, cp->buffer + chunk_off, n);
	size -= n;
	buf += n;
	rdbytes += n;
	str->offset += n;
	chunk_num++;
	chunk_off = 0;
    }
    *pret = rdbytes;
    return rc;
}

static int
_dict_seek_dzip(struct _dict_stream *str, off_t needle, int whence,
		off_t *presult)
{
    off_t offset;
    
    switch (whence) {
    case DICO_SEEK_SET:
	offset = needle;
	break;

    case DICO_SEEK_CUR:
	offset = str->offset + needle;
	break;

    case DICO_SEEK_END:
	offset = str->size + needle;
	break;

    default:
	return EINVAL;
    }

    if (offset < 0 || offset > str->size) 
	return EINVAL;

    *presult = str->offset = offset;
    return 0;
}

#else
# define cache_destroy(s)
static int
_dict_read_dzip(struct _dict_stream *str, char *buf, size_t size, size_t *pret)
{
    return DE_UNSUPPORTED_FORMAT;
}

static int
_dict_seek_dzip(struct _dict_stream *str, off_t needle, int whence,
		off_t *presult)
{
    return DE_UNSUPPORTED_FORMAT;
}

#endif    


static int
_dict_close(void *data)
{
    struct _dict_stream *str = data;
    return dico_stream_close(str->transport);
}

static int
_dict_destroy(void *data)
{
    struct _dict_stream *str = data;

    if (str->zstr_ready) {
	if (inflateEnd(&str->zstream))
	    dico_log(L_ERR, 0, _("%s:%d: INTERNAL ERROR: "
				 "cannot shut down inflation engine: %s"),
		     __FILE__, __LINE__, str->zstream.msg);
	    /* Continue anyway */
    }
    
    cache_destroy(str);
    free(str->buffer);
    dico_stream_destroy(&str->transport);
    free(str);
    return 0;
}

static int
stream_get8(dico_stream_t str, unsigned char *p)
{
    return dico_stream_read(str, p, 1, NULL);
}

static int
stream_get16(dico_stream_t str, uint16_t *pres)
{
    unsigned char buf[2];
    int rc = dico_stream_read(str, buf, sizeof buf, NULL);
    if (rc == 0) 
	*pres = buf[0] + (((uint16_t) buf[1]) << 8);
    return rc;
}

static int
stream_get32(dico_stream_t str, uint32_t *pres)
{
    unsigned char buf[4];
    int rc = dico_stream_read(str, buf, sizeof buf, NULL);
    if (rc == 0) 
	*pres = (((uint16_t) buf[3]) << 24) +
	         (((uint16_t) buf[2]) << 16) +
		 (((uint16_t) buf[1]) << 8) +
	         buf[0]; 
    return rc;
}    
    
static int
_dict_open(void *data, int flags)
{
    struct _dict_stream *str = data;
    unsigned char id[2];
    int rc;
    unsigned char buf8;
    uint16_t buf16;
    uint32_t buf32;
    unsigned i;
    unsigned offset;
    off_t pos;
    
    if (dico_stream_open(str->transport)) 
	return str->transport_error = dico_stream_last_error(str->transport);

    str->header_length = GZ_XLEN - 1;
    str->type = DICTORG_UNKNOWN;

    rc = dico_stream_read(str->transport, id, 2, NULL);
    if (rc) {
	dico_stream_close(str->transport);
	return str->transport_error = dico_stream_last_error(str->transport);
    }

    if (id[0] != GZ_MAGIC1 || id[1] != GZ_MAGIC2) {
	str->type = DICTORG_TEXT;
	dico_stream_size(str->transport, &pos);
	str->compressed_size = str->size = pos;
	return 0;
    }

    str->type = DICTORG_GZIP;

    stream_get8(str->transport, &buf8);
    str->method = buf8;
    stream_get8(str->transport, &buf8);
    str->flags  = buf8;
    
    stream_get32(str->transport, &buf32);
    str->mtime = buf32;

    stream_get8(str->transport, &buf8);
    str->extra_flags  = buf8;
    stream_get8(str->transport, &buf8);
    str->os = buf8;
   
    if (str->flags & GZ_FEXTRA) {
	stream_get16(str->transport, &buf16);
	str->header_length += buf16 + 2;
	dico_stream_read(str->transport, id, 2, NULL);
      
	if (id[0] == GZ_RND_S1 && id[1] == GZ_RND_S2) {
	    dico_stream_seek(str->transport, 2, DICO_SEEK_CUR);

	    stream_get16(str->transport, &buf16);
	    str->version = buf16;
	 
	    if (str->version != 1)
		return DE_UNSUPPORTED_VERSION;

	    stream_get16(str->transport, &buf16);
	    str->chunk_size = buf16;
	    str->buffer = malloc(buf16);
	    if (!str->buffer)
		return ENOMEM;
	    
	    stream_get16(str->transport, &buf16);
	    str->chunk_count = buf16;

	    if (str->chunk_count == 0) 
		return DE_BAD_HEADER;

	    str->chunk = calloc(str->chunk_count, sizeof(str->chunk[0]));

	    for (i = 0; i < str->chunk_count; i++) {
		stream_get16(str->transport, &buf16);
		str->chunk[i].length = buf16;
	    }
	    str->type = DICTORG_DZIP;
	} else 
	    dico_stream_seek(str->transport, str->header_length,
			     DICO_SEEK_SET);
    }
   
    str->orig_name = NULL;
    if (str->flags & GZ_FNAME) {
	size_t size = 0, rdbytes;
	dico_stream_getdelim(str->transport, &str->orig_name, &size,
			     0, &rdbytes);
	str->header_length += rdbytes;
    } else 
	str->orig_name = NULL;
   
    str->comment = NULL;
    if (str->flags & GZ_COMMENT) {
	size_t size = 0, rdbytes;
	dico_stream_getdelim(str->transport, &str->comment, &size,
			     0, &rdbytes);
	str->header_length += rdbytes;
    } 
	
    if (str->flags & GZ_FHCRC) {
	dico_stream_seek(str->transport, 2, DICO_SEEK_CUR);
	str->header_length += 2;
    }

    pos = dico_stream_seek(str->transport, 0, DICO_SEEK_CUR);
    if (pos != str->header_length + 1) {
	dico_log(L_ERR, 0, _("file position (%lu) != header length + 1 (%lu)"),
		 (unsigned long) pos,
		 (unsigned long) (str->header_length + 1));
	return DE_BAD_HEADER;
    }

    offset = str->header_length + 1;
    for (i = 0; i < str->chunk_count; i++) {
	str->chunk[i].offset = offset;
	offset += str->chunk[i].length;
    }
    
    dico_stream_seek(str->transport, -8, DICO_SEEK_END);
    
    stream_get32(str->transport, &buf32);
    str->crc = buf32;

    stream_get32(str->transport, &buf32);
    str->size = buf32;
    str->compressed_size = dico_stream_seek(str->transport, 0, DICO_SEEK_CUR);

    if (dico_stream_last_error(str->transport))
	return DE_BAD_HEADER;
    return 0;
}

static const char *
_dict_strerror(void *data, int rc)
{
    struct _dict_stream *str = data;

    if (str->transport_error) {
	str->transport_error = 0;
	return dico_stream_strerror(str->transport, rc);
    }

    switch (rc) {
    case DE_UNKNOWN_FORMAT:
	return _("unknown dictionary format");

    case DE_UNSUPPORTED_FORMAT:
	return _("unsupported dictionary format");

    case DE_UNSUPPORTED_VERSION:
	return _("unsupported dictionary version");

    case DE_BAD_HEADER:
	return _("malformed header");
	
    case DE_GZIP_SEEK:
	return _("cannot seek on pure gzip format files");

    case DE_INFLATE:
	return _("error decompressing stream");

    default:
	return strerror(rc);
    }
}

static int
_dict_read_text(struct _dict_stream *str, char *buf, size_t size, size_t *pret)
{
    if (dico_stream_read(str->transport, buf, size, pret))
	return str->transport_error = dico_stream_last_error(str->transport);
    return 0;
}

static int
_dict_read(void *data, char *buf, size_t size, size_t *pret)
{
    struct _dict_stream *str = data;

    switch (str->type) {
    case DICTORG_UNKNOWN:
	return DE_UNKNOWN_FORMAT;
	
    case DICTORG_TEXT:
	return _dict_read_text(str, buf, size, pret);
	
    case DICTORG_GZIP:
	return DE_GZIP_SEEK;
	
    case DICTORG_DZIP:
	return _dict_read_dzip(str, buf, size, pret);
    }
    return DE_UNSUPPORTED_FORMAT;
}

static int
_dict_seek_text(struct _dict_stream *str, off_t needle, int whence,
		off_t *presult)
{
    off_t off = dico_stream_seek(str->transport, needle, whence);
    if (off < 0) 
	return str->transport_error = dico_stream_last_error(str->transport);
    *presult = off;
    return 0;
}

static int
_dict_seek(void *data, off_t needle, int whence, off_t *presult)
{
    struct _dict_stream *str = data;
    switch (str->type) {
    case DICTORG_UNKNOWN:
	return DE_UNKNOWN_FORMAT;
	
    case DICTORG_TEXT:
	return _dict_seek_text(str, needle, whence, presult);
	
    case DICTORG_GZIP:
	return DE_GZIP_SEEK;

    case DICTORG_DZIP:
	return _dict_seek_dzip(str, needle, whence, presult);
    }
    return DE_UNSUPPORTED_FORMAT;
}

dico_stream_t
dict_stream_create(const char *filename, size_t cache_size)
{
    int rc;
    dico_stream_t str;
    struct _dict_stream *s = malloc(sizeof(*s));

    if (!s)
	return NULL;
    rc = dico_stream_create(&str, DICO_STREAM_READ|DICO_STREAM_SEEK, s);
    if (rc) {
	free(s);
	return NULL;
    }

    memset(s, 0, sizeof(*s));
    s->type = DICTORG_UNKNOWN;
    s->cache_size = cache_size ? cache_size : 10;
    s->transport = dico_mapfile_stream_create(filename,
					     DICO_STREAM_READ|DICO_STREAM_SEEK);
    dico_stream_set_open(str, _dict_open);
    dico_stream_set_read(str, _dict_read);
    dico_stream_set_seek(str, _dict_seek);
    dico_stream_set_close(str, _dict_close);
    dico_stream_set_destroy(str, _dict_destroy);
    dico_stream_set_error_string(str, _dict_strerror);
    
    return str;
}

