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

#include "dico.h"

/* GCIDE-specific definitions. */
#define GCIDE_IDX_MAGIC "GCIDEIDX"
#define GCIDE_IDX_MAGIC_LEN (sizeof(GCIDE_IDX_MAGIC)-1)

#define GCIDE_IDX_HEADER_PAGESIZE 10240

struct gcide_idx_header {
    char ihdr_magic[GCIDE_IDX_MAGIC_LEN];
    unsigned long ihdr_pagesize;
    unsigned long ihdr_maxpageref;
    unsigned long ihdr_num_pages;
    unsigned long ihdr_num_headwords;
    unsigned long ihdr_num_defs;
};

struct gcide_ref {
    unsigned long ref_hwoff;
    unsigned long ref_hwlen;
    unsigned long ref_hwbytelen;
    int ref_letter;
    unsigned long ref_offset;
    unsigned long ref_size;
    char *ref_headword;
};

struct gcide_page_header {
    size_t phdr_numentries;
    size_t phdr_text_offset;
};
    
struct gcide_idx_page {
    union {
	struct gcide_page_header hdr;
	struct gcide_ref align;
    } ipg_header;
    struct gcide_ref ipg_ref[1];
};

typedef struct gcide_idx_file *gcide_idx_file_t;
typedef struct gcide_iterator *gcide_iterator_t;

gcide_idx_file_t gcide_idx_file_open(const char *name, size_t cachesize);
void gcide_idx_file_close(gcide_idx_file_t file);
size_t gcide_idx_headwords(struct gcide_idx_file *file);
size_t gcide_idx_defs(struct gcide_idx_file *file);

int gcide_idx_enumerate(struct gcide_idx_file *file,
			int (*fun)(struct gcide_ref *, void *),
			void *data);

gcide_iterator_t gcide_idx_locate(struct gcide_idx_file *file, char *headword,
				  size_t len);
void gcide_iterator_free(gcide_iterator_t itr);
int gcide_iterator_next(gcide_iterator_t itr);
int gcide_iterator_rewind(gcide_iterator_t itr);
struct gcide_ref *gcide_iterator_ref(gcide_iterator_t itr);
size_t gcide_iterator_count(gcide_iterator_t itr);
size_t gcide_iterator_compare_count(gcide_iterator_t itr);
void gcide_iterator_store_flags(gcide_iterator_t itr, int flags);
int gcide_iterator_flags(gcide_iterator_t itr);

extern char gcide_webchr[256][4];
char const *gcide_escape_to_utf8(const char *esc);
char const *gcide_entity_to_utf8(const char *str);

const char *gcide_grk_to_utf8(const char *input, size_t *prd);


enum gcide_content_type
{
    gcide_content_unspecified,
    gcide_content_text,
    gcide_content_taglist
};

struct gcide_tag {
    size_t tag_parmc;
    char **tag_parmv;
#define tag_name tag_parmv[0]
    enum gcide_content_type tag_type;
    struct gcide_tag *tag_next;
    union {
	char *text;
	size_t textpos;
	dico_list_t taglist;
    } tag_v;
};

#define TAG_HAS_NAME(t) ((t) && (t)->tag_parmv && (t)->tag_name)

struct gcide_parse_tree {
    char *textspace;
    size_t textsize;
    struct gcide_tag *root;
};

struct gcide_parse_tree *gcide_markup_parse(char const *text, size_t len,
					    int dbg);
void gcide_parse_tree_free(struct gcide_parse_tree *tp);
int gcide_parse_tree_inorder(struct gcide_parse_tree *tp,
			     int (*fun)(int, struct gcide_tag *, void *),
			     void *data);



