/* grecs - Gray's Extensible Configuration System
   Copyright (C) 2007-2012 Sergey Poznyakoff

   Grecs is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   Grecs is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with Grecs. If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "grecs.h"
#include "wordsplit.h"
#include <fnmatch.h>

static int
_grecs_list_eq(struct grecs_value *a, struct grecs_value *b)
{
	struct grecs_list_entry *aent, *bent;
	if (grecs_list_size(a->v.list) != grecs_list_size(b->v.list))
		return 0;

	for (aent = a->v.list->head, bent = b->v.list->head;;
	     aent = aent->next, bent = bent->next) {
		if (!aent)
			return bent == NULL;
		if (!bent)
			return 0;
		if (!grecs_value_eq(aent->data, bent->data))
			return 0;
	}
	/*notreached*/
	return 1;
}

static int
_grecs_array_eq(struct grecs_value *a, struct grecs_value *b)
{
	size_t i;

	if (a->v.arg.c != b->v.arg.c)
		return 0;

	for (i = 0; i < a->v.arg.c; i++)
		if (!grecs_value_eq(a->v.arg.v[i], b->v.arg.v[i]))
			return 0;
	return 1;
}

/* Return 1 if configuration value A equals B */
int
grecs_value_eq(struct grecs_value *a, struct grecs_value *b)
{
	if (a == 0 || b == 0)
		return a == b;
	if (a->type != b->type)
		return 0;
	switch (a->type) {
	case GRECS_TYPE_STRING:
		if (a->v.string == NULL)
			return b->v.string == NULL;
		return strcmp(a->v.string, b->v.string) == 0;
      
	case GRECS_TYPE_LIST:
		return _grecs_list_eq(a, b);
      
	case GRECS_TYPE_ARRAY:
		return _grecs_array_eq(a, b);
    }
  return 0;
}

static int
_grecs_list_match(struct grecs_value *pat, struct grecs_value *b, int flags)
{
	struct grecs_list_entry *aent, *bent;
	if (grecs_list_size(pat->v.list) != grecs_list_size(b->v.list))
		return 0;

	for (aent = pat->v.list->head, bent = b->v.list->head;;
	     aent = aent->next, bent = bent->next) {
		if (!aent)
			return bent == NULL;
		if (!bent)
			return 0;
		if (!grecs_value_match(aent->data, bent->data, flags))
			return 0;
	}
	/*notreached*/
	return 1;
}

static int
_grecs_array_match(struct grecs_value *pat, struct grecs_value *b, int flags)
{
	size_t i;

	if (pat->v.arg.c > b->v.arg.c)
		return 0;

	for (i = 0; i < pat->v.arg.c; i++)
		if (!grecs_value_match(pat->v.arg.v[i], b->v.arg.v[i], flags))
			return 0;
	return 1;
}

int
grecs_value_match(struct grecs_value *pat, struct grecs_value *b, int flags)
{
	if (pat == 0 || b == 0)
		return pat == b;
	if (pat->type != b->type) {
		if (pat->type != GRECS_TYPE_STRING)
			return 0;
		switch (b->type) {
		case GRECS_TYPE_LIST:
			b = grecs_list_index(b->v.list, 0);
			break;

		case GRECS_TYPE_ARRAY:
			b = b->v.arg.v[0];
		}
	}
	
	switch (pat->type) {
	case GRECS_TYPE_STRING:
		if (pat->v.string == NULL)
			return b->v.string == NULL;
		return fnmatch(pat->v.string, b->v.string, flags) == 0;
      
	case GRECS_TYPE_LIST:
		return _grecs_list_match(pat, b, flags);
      
	case GRECS_TYPE_ARRAY:
		return _grecs_array_match(pat, b, flags);
    }
  return 0;
}


struct grecs_match_buf {
	int argc;
	char **argv;
	int argi;
	struct grecs_value **labelv;
	struct grecs_node *node;
};

static void
grecs_match_buf_free_contents(struct grecs_match_buf *buf)
{
	size_t i;
	for (i = 0; i < buf->argc; i++) {
		free(buf->argv[i]);
		grecs_value_free(buf->labelv[i]);
	}
	free(buf->argv);
	free(buf->labelv);
}

void
grecs_match_buf_free(struct grecs_match_buf *buf)
{
	if (buf) {
		grecs_match_buf_free_contents(buf);
		free(buf);
	}
}


static struct grecs_value *
parse_label(const char *str)
{
	struct grecs_value *val = NULL;
	size_t i;
	struct wordsplit ws;
	size_t len = strlen (str);
  
	if (len > 1 && str[0] == '(' && str[len-1] == ')') {
		struct grecs_list *lst;
      
		ws.ws_delim = ",";
		if (wordsplit_len (str + 1, len - 2, &ws,
				      WRDSF_DEFFLAGS|WRDSF_DELIM|
				      WRDSF_WS)) {
			return NULL;
		}

		lst = grecs_value_list_create();
		for (i = 0; i < ws.ws_wordc; i++) {
			struct grecs_value *p = grecs_zalloc(sizeof(*p));
			p->type = GRECS_TYPE_STRING;
			p->v.string = ws.ws_wordv[i];
			grecs_list_append(lst, p);
		}
		val = grecs_malloc(sizeof(*val));
		val->type = GRECS_TYPE_LIST;
		val->v.list = lst;
	} else {      
		if (wordsplit(str, &ws, WRDSF_DEFFLAGS))
			return NULL;
		val = grecs_zalloc(sizeof(*val));
		if (ws.ws_wordc == 1) {
			val->type = GRECS_TYPE_STRING;
			val->v.string = ws.ws_wordv[0];
		} else {
			val->type = GRECS_TYPE_ARRAY;
			val->v.arg.c = ws.ws_wordc;
			val->v.arg.v = grecs_calloc(ws.ws_wordc,
						    sizeof(val->v.arg.v[0]));
			for (i = 0; i < ws.ws_wordc; i++) {
				val->v.arg.v[i] =
					grecs_zalloc(sizeof(*val->v.arg.v[0]));
				val->v.arg.v[i]->type = GRECS_TYPE_STRING;
				val->v.arg.v[i]->v.string = ws.ws_wordv[i];
			}
		}
	}
	ws.ws_wordc = 0;
	wordsplit_free(&ws);
	return val;
}

static int
split_cfg_path(const char *path, int *pargc, char ***pargv,
	       grecs_value_t ***pvalv)
{
	int argc;
	char **argv;
	char *delim = ".";
	char static_delim[2] = { 0, 0 };
  
	if (path[0] == '\\') {
		argv = calloc(2, sizeof (*argv));
		if (!argv)
			return WRDSE_NOSPACE;
		argv[0] = strdup(path + 1);
		if (!argv[0]) {
			free(argv);
			return WRDSE_NOSPACE;
		}
		argv[1] = NULL;
		argc = 1;
	} else {
		int rc;
		struct wordsplit ws;
		
		if (strchr("./:;,^~", path[0])) {
			delim = static_delim;
			delim[0] = path[0];
			path++;
		}
		ws.ws_delim = delim;
      
		rc = wordsplit(path, &ws,
			       WRDSF_DELIM | WRDSF_DEFFLAGS);
		if (rc)
			return rc;
		argc = ws.ws_wordc;
		argv = ws.ws_wordv;
		ws.ws_wordc = 0;
		ws.ws_wordv = NULL;
		wordsplit_free(&ws);
	}

	*pargv = argv;
	*pargc = argc;
	if (pvalv) {
		int i;
		grecs_value_t **valv;
		
		valv = grecs_calloc(argc, sizeof(valv[0]));
		for (i = 0; i < argc; i++) {
			char *p = strchr(argv[i], '=');
			if (p) {
				*p++ = 0;
				valv[i] = parse_label(p);
			}
		}
		*pvalv = valv;
	}
	return 0;
}


static enum grecs_tree_recurse_res
node_finder(enum grecs_tree_recurse_op op, struct grecs_node *node, void *data)
{
	struct grecs_match_buf *buf = data;

	if (op == grecs_tree_recurse_post || node->type == grecs_node_root)
		return grecs_tree_recurse_ok;
	
	if (strcmp(buf->argv[buf->argi], node->ident) == 0
	    && (!buf->labelv[buf->argi] ||
		grecs_value_eq(buf->labelv[buf->argi], node->v.value))) {
		buf->argi++;
		if (buf->argi == buf->argc) {
			buf->node = node;
			return grecs_tree_recurse_stop;
		}
		return grecs_tree_recurse_ok;
	}
  
	return node->type == grecs_node_block ?
		grecs_tree_recurse_skip : grecs_tree_recurse_ok;
}

struct grecs_node *
grecs_find_node(struct grecs_node *node, const char *path)
{
	int rc;
	struct grecs_match_buf buf;
  
	if (strcmp(path, ".") == 0)
		return node;
	rc = split_cfg_path(path, &buf.argc, &buf.argv, &buf.labelv);
	if (rc || !buf.argc)
		return NULL;
	buf.argi = 0;
	buf.node = NULL;
	grecs_tree_recurse(node, node_finder, &buf);
	grecs_match_buf_free_contents(&buf);
	return buf.node;
}
	

static void
fixup_loci(struct grecs_node *node, 
	   grecs_locus_t const *plocus,
	   struct grecs_locus_point const *endp)
{
	grecs_locus_t loc = *plocus;
	
	for (; node; node = node->down) {
		node->idloc = loc;
		
		node->locus = loc;
		if (endp)
			node->locus.end = *endp;
	}
}

struct grecs_node *
grecs_node_from_path_locus(const char *path, const char *value,
			   grecs_locus_t *plocus, grecs_locus_t *vallocus)
{
	int rc;
	int i;
	int argc;
	char **argv;
	struct grecs_node *dn = NULL;
	
	rc = split_cfg_path(path, &argc, &argv, NULL);
	if (rc)
		return NULL;

	dn = grecs_node_create(grecs_node_stmt, NULL);
	dn->ident = argv[argc - 1];
	if (value) {
		struct grecs_value *gval = parse_label(value);
		if (vallocus)
			gval->locus = *vallocus;
		dn->v.value = gval;
	} else
		dn->v.value = NULL;		
	
	for (i = argc - 2; i >= 0; i--) {
		struct grecs_value *label = NULL;
		struct grecs_node *node;
		char *p, *q = argv[i];

		do {
			p = strchr(q, '=');
			if (p && p > argv[i] && p[-1] != '\\') {
				*p++ = 0;
				label = parse_label(p);
				break;
			} else if (p)
				q = p + 1;
			else
				break;
		} while (*q);
		
		node = grecs_node_create(grecs_node_block, plocus);
		node->ident = argv[i];
		if (label)
			node->v.value = label;

		node->down = dn;
		if (dn)
			dn->up = node;
		dn = node;
	}

	if (plocus)
		fixup_loci(dn, 
			   plocus, vallocus ? &vallocus->end : NULL);
		
	free(argv);
	return dn;
}

struct grecs_node *
grecs_node_from_path(const char *path, const char *value)
{
	return grecs_node_from_path_locus(path, value, NULL, NULL);
}


#define ISWC(c,w) ((c)[0] == (w) && (c)[1] == 0)

static int
grecs_match(struct grecs_match_buf *buf)
{
	struct grecs_node *node;
	int wcard = 0;
	
	buf->argi = buf->argc - 1;
	node = buf->node;

	while (buf->argi >= 0) {
		if (ISWC(buf->argv[buf->argi], '*')) {
			wcard = 1;
			if (buf->argi-- == 0)
				return 1;
			continue;
		}
		
		if ((ISWC(buf->argv[buf->argi], '%') ||
		     strcmp(buf->argv[buf->argi], node->ident) == 0)
		    /* FIXME: */
		    && (!buf->labelv[buf->argi] ||
			grecs_value_match(buf->labelv[buf->argi],
					  node->v.value, 0))) {
			wcard = 0;
			node = node->up;
			if (buf->argi-- == 0)
				return !node ||
					node->type == grecs_node_root;
		} else if (!wcard)
			return 0;
		else
			node = node->up;
		if (!node || node->type == grecs_node_root)
			return ISWC(buf->argv[buf->argi], '*');
	}
	return 0;
}

struct grecs_node *
grecs_match_next(struct grecs_match_buf *buf)
{
	if (!buf)
		return NULL;
	while (buf->node = grecs_next_node(buf->node))
		if (grecs_match(buf))
			break;
	return buf->node;
}

struct grecs_node *
grecs_match_first(struct grecs_node *tree, const char *pattern,
		  struct grecs_match_buf **pbuf)
{
	int i;
	struct grecs_node *node;
	struct grecs_match_buf *buf;

	if (tree->type != grecs_node_root) {
		errno = EINVAL;
		return NULL;
	}
	errno = 0;
	if (strcmp(pattern, ".") == 0) {
		*pbuf = NULL;
		return tree;
	}
	
	buf = grecs_zalloc(sizeof(*buf));
	if (split_cfg_path(pattern, &buf->argc, &buf->argv, &buf->labelv)) {
		free(buf);
		return NULL;
	}
	/* FIXME: Compress argv/argc by replacing contiguous sequences of *'s
	   with a single *. */
	for (i = 0; i < buf->argc; i++) {
		if (ISWC(buf->argv[i], '*')) {
			int j;

			for (j = i + 1;
			     j < buf->argc && ISWC(buf->argv[j], '*'); j++)
				free(buf->argv[j]);
			j -= i;
			if (j > 1) {
				memmove(&buf->argv[i+1], &buf->argv[i+j],
					(buf->argc - (i + j)) *
					sizeof(buf->argv[0]));
				memmove(&buf->labelv[i+1], &buf->labelv[i+j],
					(buf->argc - (i + j)) *
					sizeof(buf->labelv[0]));
				buf->argc -= j - 1;
			}
		}
	}
			
	
	buf->argi = 0;
	buf->node = grecs_tree_first_node(tree);
	*pbuf = buf;
	if (grecs_match(buf))
		node = buf->node;
	else
		node = grecs_match_next(buf);
	if (!node) {
		grecs_match_buf_free(buf);
		*pbuf = NULL;
	}
	return node;
}

