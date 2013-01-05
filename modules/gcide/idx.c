/* This file is part of GNU Dico.
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dico.h>
#include "gcide.h"
#include <errno.h>
#include <appi18n.h>

struct gcide_idx_cache {
    size_t pageno;
    unsigned refcount;
    struct gcide_idx_page *page;
};

struct gcide_idx_file {
    char *name;
    int fd;
    struct gcide_idx_header header;
    size_t cache_size;
    size_t cache_used;
    struct gcide_idx_cache **cache;
    size_t compare_count;
};

#define REF_NOT_FOUND ((size_t)-1)

static void
_free_index(struct gcide_idx_file *file)
{
    size_t i;
    free(file->name);
    for (i = 0; i < file->cache_used; i++) {
	free(file->cache[i]->page);
	free(file->cache[i]);
    }
    free(file->cache);
    free(file);
}

static int
_idx_full_read(struct gcide_idx_file *file, void *buf, size_t size)
{
    char *p = buf;
    while (size) {
	ssize_t rc = read(file->fd, p, size);
	if (rc == -1) {
	    if (errno == EAGAIN)
		continue;
	    dico_log(L_ERR, errno, _("error reading from `%s'"), file->name);
	    return -1;
	} else if (rc == 0) {
	    dico_log(L_ERR, errno, _("short read while reading from `%s'"),
		     file->name);
	    return -1;
	}
	p += rc;
	size -= rc;
    }
    return 0;
}

static int
_open_index(struct gcide_idx_file *file)
{
    off_t total;
    struct stat st;
    
    if (_idx_full_read(file, &file->header, sizeof(file->header)))
	return 1;
    if (memcmp(file->header.ihdr_magic, GCIDE_IDX_MAGIC,
	       GCIDE_IDX_MAGIC_LEN)) {
	dico_log(L_ERR, 0, _("file `%s' is not a valid gcide index file"),
		 file->name);
	return 1;
    }
    
    if (fstat(file->fd, &st)) {
	dico_log(L_ERR, errno, "fstat `%s'", file->name);
	return 1;
    }
    
    total = (file->header.ihdr_num_pages + 1) * file->header.ihdr_pagesize;
    if (total != st.st_size) {
	dico_log(L_ERR, 0, _("index file `%s' is corrupted"), file->name);
	return 1;
    }
    return 0;
}

struct gcide_idx_file *
gcide_idx_file_open(const char *name, size_t cachesize)
{
    int fd;
    struct gcide_idx_file *file;
    
    file = calloc(1, sizeof(*file));
    if (!file) {
	dico_log(L_ERR, errno, "gcide_idx_file_open");
	return NULL;
    }
    file->name = strdup(name);
    if (!file->name) {
	dico_log(L_ERR, errno, "gcide_idx_file_open");
	free(file);
	return NULL;
    }
    fd = open(name, O_RDONLY);
    if (fd == -1) {
	dico_log(L_ERR, errno, _("cannot open index file `%s'"), name);
	free(file);
    }
    file->fd = fd;
    if (_open_index(file)) {
	_free_index(file);
	return NULL;
    }
    file->cache_size = cachesize;
    return file;
}

void
gcide_idx_file_close(struct gcide_idx_file *file)
{
    if (file) {
	close(file->fd);
	_free_index(file);
    }
}

size_t
gcide_idx_headwords(struct gcide_idx_file *file)
{
    return file->header.ihdr_num_headwords;
}

size_t
gcide_idx_defs(struct gcide_idx_file *file)
{
    return file->header.ihdr_num_defs;
}

static struct gcide_idx_cache *
_cache_alloc(struct gcide_idx_file *file)
{
    struct gcide_idx_cache *cp;
    
    if (!file->cache) {
	file->cache = calloc(file->cache_size, sizeof(file->cache[0]));
	if (!file->cache) {
	    dico_log(L_ERR, ENOMEM, "gcide _cache_alloc");
	    return NULL;
	}
    }
    if (file->cache_used < file->cache_size) {
	if (file->cache_used &&
	    file->cache[file->cache_used - 1]->refcount == 0)
	    return file->cache[file->cache_used - 1];
	cp = calloc(1, sizeof(*cp));
	if (!cp) {
	    dico_log(L_ERR, ENOMEM, "gcide _cache_alloc");
	    return NULL;
	}
	cp->page = malloc(file->header.ihdr_pagesize);
	if (!cp->page) {
	    dico_log(L_ERR, ENOMEM, "gcide _cache_alloc");
	    free(cp);
	    return NULL;
	}
	file->cache[file->cache_used++] = cp;
    } else
	cp = file->cache[file->cache_used - 1];
    cp->pageno = 0;
    cp->refcount = 0;
    return cp;
}

static void
_cache_promote(struct gcide_idx_file *file, int n)
{
    size_t i;
    unsigned refcount = ++file->cache[n]->refcount;

    if (n == 0)
	return;
    for (i = n - 1; i >= 0; i--)
	if (file->cache[i]->refcount >= refcount)
	    break;
    i++;
    if (i != n) {
	struct gcide_idx_cache *tmp = file->cache[n];
	file->cache[n] = file->cache[i];
	file->cache[i] = tmp;
    }
}

static struct gcide_idx_cache *
_cache_find_page(struct gcide_idx_file *file, size_t n)
{
    return NULL;
    size_t i;
    
    for (i = 0; i < file->cache_used; i++) {
	if (file->cache[i]->pageno == n) {
	    struct gcide_idx_cache *cp = file->cache[i];
	    _cache_promote(file, n);
	    return cp;
	}
    }
    
    return NULL;
}

struct gcide_idx_page *
_idx_get_page(struct gcide_idx_file *file, size_t n)
{
    off_t off;
    struct gcide_idx_cache *cp;
    struct gcide_idx_page *page;
    size_t i;
    
    cp = _cache_find_page(file, n);
    if (cp)
	return cp->page;
    
    off = (n + 1) * file->header.ihdr_pagesize;
    if (lseek(file->fd, off, SEEK_SET) != off) {
	dico_log(L_ERR, errno,
		 _("seek error on `%s' while positioning to %lu"),
		 file->name, (unsigned long) off);
	return NULL;
    }
    cp = _cache_alloc(file);
    if (!cp)
	return NULL;
    if (_idx_full_read(file, cp->page, file->header.ihdr_pagesize))
	return NULL;
    cp->refcount++;
    page = cp->page;
    for (i = 0; i < page->ipg_header.hdr.phdr_numentries; i++)
	page->ipg_ref[i].ref_headword =
	    (char*)page + page->ipg_ref[i].ref_hwoff;
    return page;
}


int
gcide_idx_enumerate(struct gcide_idx_file *file,
		    int (*fun)(struct gcide_ref *, void *),
		    void *data)
{
    size_t i;

    for (i = 0; i < file->header.ihdr_num_pages; i++) {
	int j;

	struct gcide_idx_page *page = _idx_get_page(file, i);
	if (!page)
	    return -1;
	for (j = 0; j < page->ipg_header.hdr.phdr_numentries; j++)
	    if (fun(&page->ipg_ref[j], data))
		break;
    }
    return 0;
}

static int
_compare(struct gcide_idx_file *file, char *hw, struct gcide_ref *ref,
	 size_t hwlen)
{
    file->compare_count++;
    if (hwlen) {
	if (hwlen > ref->ref_hwlen)
	    hwlen = ref->ref_hwlen;
	return utf8_strncasecmp(hw, ref->ref_headword, hwlen);
    }
    return utf8_strcasecmp(hw, ref->ref_headword);
}

static size_t
_idx_ref_locate(struct gcide_idx_file *file,
		struct gcide_idx_page *page, char *headword, size_t hwlen)
{
    size_t l, u, idx;
    int res;
    
    l = 0;
    u = page->ipg_header.hdr.phdr_numentries;
    while (l < u) {
	idx = (l + u) / 2;
	res = _compare(file, headword, &page->ipg_ref[idx], hwlen);
	if (res < 0)
	    u = idx;
	else if (res > 0)
	    l = idx + 1;
	else
	    return idx;
    }
    return REF_NOT_FOUND;
}

static size_t
_idx_page_locate(struct gcide_idx_file *file, char *headword, size_t hwlen)
{
    size_t l, u, idx;
    int res;
    
    l = 0;
    u = file->header.ihdr_num_pages;
    while (l < u) {
	struct gcide_idx_page *page;
	    
	idx = (l + u) / 2;
	page = _idx_get_page(file, idx);
	if (!page)
	    return REF_NOT_FOUND;

	res = _compare(file, headword, &page->ipg_ref[0], hwlen);
	if (res < 0) {
	    u = idx;
	}
	else if (res == 0)
	    return idx;
	else {
	    res = _compare(file, headword,
			   &page->ipg_ref[page->ipg_header.hdr.phdr_numentries-1],
			   hwlen);
	    if (res > 0)
		l = idx + 1;
	    else
		return idx;
	}
    }
    return REF_NOT_FOUND;
}

struct gcide_iterator {
    struct gcide_idx_file *file; /* Index file */
    char *headword;              /* Headword this iterator tracks */
    size_t hwlen;                /* Length of the headword */
    size_t start_pageno;         /* Number of start page */
    size_t start_refno;          /* Number of start reference in page */
    size_t cur_pageno;           /* Number of current page */
    size_t cur_refno;            /* Number of current reference in page */
    size_t page_numrefs;         /* Total number of references in cur. page */
    size_t compare_count;        /* Number of comparisons */
    /* */
    size_t numrefs;              /* Number of references to headword. Zero
				    if not yet determined. */
    size_t curref;               /* Position in iterator */

    char *prevbuf;               /* Previous match */ 
    size_t prevsize;
    
    int flags;                   /* User-defined flags. */
};
    
gcide_iterator_t
gcide_idx_locate(struct gcide_idx_file *file, char *headword, size_t hwlen)
{
    size_t pageno, refno;
    struct gcide_idx_page *page;
    struct gcide_iterator *itr;

    file->compare_count = 0;
    pageno = _idx_page_locate(file, headword, hwlen);
    if (pageno == REF_NOT_FOUND)
	return NULL;
    page = _idx_get_page(file, pageno);
    if (!page)
	return NULL;
    refno = _idx_ref_locate(file, page, headword, hwlen);
    if (refno == REF_NOT_FOUND)
	return NULL;

    for (;;) {
	if (_compare(file, headword, &page->ipg_ref[refno], hwlen) == 0)
	    break;
	if (_compare(file, headword, &page->ipg_ref[refno-1], hwlen) > 0)
	    break;
	if (--refno==0) {
	    if (pageno == 0)
		break;
	    page = _idx_get_page(file, --pageno);
	    if (!page)
		return NULL;
	    refno = page->ipg_header.hdr.phdr_numentries;
	}
    }
    if (refno == page->ipg_header.hdr.phdr_numentries) {
	pageno++;
	refno = 0;
    }
    
    itr = malloc(sizeof(*itr));
    if (!itr) {
	dico_log(L_ERR, errno, "gcide_idx_locate");
	return NULL;
    }
    if (hwlen) {
	itr->headword = malloc(hwlen);
	if (itr->headword)
	    memcpy(itr->headword, headword, hwlen);
    } else
	itr->headword = strdup(headword);

    if (!itr->headword) {
	dico_log(L_ERR, errno, "gcide_idx_locate");
	free(itr);
	return NULL;
    }

    itr->hwlen = hwlen;
    
    itr->file = file;
    itr->start_pageno = itr->cur_pageno = pageno;    
    itr->start_refno = itr->cur_refno = refno;
    itr->page_numrefs = page->ipg_header.hdr.phdr_numentries;
    itr->curref = itr->numrefs = 0;
    itr->compare_count = file->compare_count;
    
    return itr;
}

void
gcide_iterator_free(gcide_iterator_t itr)
{
    if (itr) {
	free(itr->headword);
	free(itr);
    }
}

int
gcide_iterator_next(gcide_iterator_t itr)
{
    struct gcide_idx_page *page;
    size_t pageno, refno;

    if (!itr)
	return -1;

    if (itr->numrefs && itr->curref == itr->numrefs-1)
	return -1;

    if (itr->cur_refno < itr->page_numrefs-1) {
	pageno = itr->cur_pageno;
	refno = itr->cur_refno + 1;
    } else if (itr->cur_pageno == itr->file->header.ihdr_num_pages) {
	if (!itr->numrefs)
	    itr->numrefs = itr->curref + 1;
	return -1;
    } else {
	pageno = itr->cur_pageno + 1;
	refno = 0;
    }

    page = _idx_get_page(itr->file, pageno);
    if (!page)
	return -1;
    if (!itr->numrefs &&
	_compare(itr->file, itr->headword, &page->ipg_ref[refno],
		 itr->hwlen)) {
	if (!itr->numrefs)
	    itr->numrefs = itr->curref + 1;
	return -1;
    }
    itr->page_numrefs = page->ipg_header.hdr.phdr_numentries;
    itr->cur_pageno = pageno;
    itr->cur_refno = refno;
    itr->curref++;
    return 0;
}

int
gcide_iterator_rewind(gcide_iterator_t itr)
{
    struct gcide_idx_page *page;

    if (!itr)
	return -1;
    itr->cur_pageno = itr->start_pageno;    
    itr->cur_refno = itr->start_refno;
    itr->curref = 0;
    page = _idx_get_page(itr->file, itr->cur_pageno);
    if (!page)
	return -1;
    itr->page_numrefs = page->ipg_header.hdr.phdr_numentries;
    return 0;
}

struct gcide_ref *
gcide_iterator_ref(gcide_iterator_t itr)
{
    struct gcide_idx_page *page;
    
    if (!itr)
	return NULL;
    page = _idx_get_page(itr->file, itr->cur_pageno);
    if (!page)
	return NULL;
    return &page->ipg_ref[itr->cur_refno];
}

size_t
gcide_iterator_count(gcide_iterator_t itr)
{
    if (!itr)
	return 0;
    if (!itr->numrefs) {
	while (gcide_iterator_next(itr) == 0)
	    ;
	gcide_iterator_rewind(itr);
    }
    return itr->numrefs;
}

size_t
gcide_iterator_compare_count(gcide_iterator_t itr)
{
    return itr->compare_count;
}

void
gcide_iterator_store_flags(gcide_iterator_t itr, int flags)
{
    itr->flags = flags;
}

int
gcide_iterator_flags(gcide_iterator_t itr)
{
    if (!itr)
	return 0;
    return itr->flags;
}
