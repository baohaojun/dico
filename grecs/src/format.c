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
#include <grecs.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "wordsplit.h"

const char *
grecs_data_type_string(enum grecs_data_type type)
{
	switch (type) {
	case grecs_type_void:
		return "void";

	case grecs_type_string:
		return "string";

	case grecs_type_short:
	case grecs_type_ushort:
	case grecs_type_int:
	case grecs_type_uint:
	case grecs_type_long:
	case grecs_type_ulong:
	case grecs_type_size:
		/*FIXME case  grecs_type_off:*/
		return "number";

	case grecs_type_time:
		return "time";

	case grecs_type_bool:
		return "boolean";

	case grecs_type_ipv4:
		return "IPv4";

	case grecs_type_cidr:
		return "CIDR";

	case grecs_type_host:
		return "hostname";

	case grecs_type_sockaddr:
		return "sock-addr";

	case grecs_type_section:
		return "section";
	}
	return "UNKNOWN?";
}

static void
format_level(unsigned level, FILE *stream)
{
	while (level--)
		fprintf(stream, "  ");
}

void
grecs_print_docstring(const char *docstring, unsigned level, FILE *stream)
{
	size_t len = strlen(docstring);
	int width = 78 - level * 2;

	if (width < 0) {
		width = 78;
		level = 0;
	}

	while (len) {
		size_t seglen;
		const char *p;

		for (seglen = 0, p = docstring; p < docstring + width && *p;
		     p++) {
			if (*p == '\n') {
				seglen = p - docstring;
				break;
			}
			if (isspace(*p))
				seglen = p - docstring;
		}
		if (seglen == 0 || *p == 0)
			seglen = p - docstring;

		format_level(level, stream);
		fprintf(stream, "# ");
		fwrite(docstring, seglen, 1, stream);
		fputc('\n', stream);
		len -= seglen;
		docstring += seglen;
		if (*docstring == '\n') {
			docstring++;
			len--;
		} else
			while (*docstring && isspace(*docstring)) {
				docstring++;
				len--;
			}
	}
}

void
grecs_print_simple_statement(struct grecs_keyword *kwp, unsigned level,
			     FILE *stream)
{
	const char *argstr;

	if (kwp->flags & GRECS_INAC)
		grecs_print_docstring(N_("Disabled;"), level, stream);
	if (kwp->docstring)
		grecs_print_docstring(kwp->docstring, level, stream);
	format_level(level, stream);

	if (kwp->argname)
		argstr = kwp->argname;
	else
		argstr = N_("arg");

	if (strchr("<[", argstr[0]))
		fprintf(stream, "%s %s;\n", kwp->ident, gettext(argstr));
	else if (strchr (argstr, ':'))
		fprintf(stream, "%s <%s>;\n", kwp->ident, gettext(argstr));
	else {
		fprintf(stream, "%s <%s: ", kwp->ident, gettext(argstr));
		if (kwp->flags & GRECS_LIST)
			fprintf(stream, "list of %s",
				gettext(grecs_data_type_string(kwp->type)));
		else
			fprintf(stream, "%s",
				 gettext(grecs_data_type_string(kwp->type)));
		fprintf(stream, ">;\n");
	}
}

void
grecs_print_block_statement(struct grecs_keyword *kwp, unsigned level,
			     FILE *stream)
{
	if (kwp->docstring)
		grecs_print_docstring(kwp->docstring, level, stream);
	format_level(level, stream);
	fprintf(stream, "%s", kwp->ident);
	if (kwp->argname)
		fprintf(stream, " <%s>", gettext(kwp->argname));
	fprintf(stream, " {\n");
	grecs_print_statement_array(kwp->kwd, 0, level + 1, stream);
	format_level(level, stream);
	fprintf(stream, "}\n");
}

void
grecs_print_statement_array(struct grecs_keyword *kwp,
			    unsigned n,
			    unsigned level,
			    FILE *stream)
{
	if (!kwp) {
		return;
	}
	for (; kwp->ident; kwp++, n++) {
		if (n)
			fputc('\n', stream);
		if (kwp->type == grecs_type_section)
			grecs_print_block_statement(kwp, level, stream);
		else
			grecs_print_simple_statement(kwp, level, stream);
	}
}


void
grecs_format_locus(grecs_locus_t *locus, struct grecs_format_closure *clos)
{
	if (locus) {
		char *str = NULL;
		size_t size = 0;

		if (locus->beg.col == 0)
			grecs_asprintf(&str, &size, "%s:%u",
				       locus->beg.file,
				       locus->beg.line);
		else if (strcmp(locus->beg.file, locus->end.file))
			grecs_asprintf(&str, &size, "%s:%u.%u-%s:%u.%u",
				       locus->beg.file,
				       locus->beg.line, locus->beg.col,
				       locus->end.file,
				       locus->end.line, locus->end.col);
		else if (locus->beg.line != locus->end.line)
			grecs_asprintf(&str, &size, "%s:%u.%u-%u.%u",
				       locus->beg.file,
				       locus->beg.line, locus->beg.col,
				       locus->end.line, locus->end.col);
		else if (locus->beg.col != locus->end.col)
			grecs_asprintf(&str, &size, "%s:%u.%u-%u",
				       locus->beg.file,
				       locus->beg.line, locus->beg.col,
				       locus->end.col);
		else
			grecs_asprintf(&str, &size, "%s:%u.%u",
				       locus->beg.file,
				       locus->beg.line,
				       locus->beg.col);
		
		clos->fmtfun(str, clos->data);
		free(str);
	}
}

void
grecs_format_node_path(struct grecs_node *node, int flags,
		       struct grecs_format_closure *clos)
{
	char delim[2] = ".";

	if (node->up)
		grecs_format_node_path(node->up, flags, clos);
	if (node->type == grecs_node_root)
		return;
	if (flags & _GRECS_NODE_MASK_DELIM)
		delim[0] = flags & _GRECS_NODE_MASK_DELIM;
	clos->fmtfun(delim, clos->data);
	clos->fmtfun(node->ident, clos->data);
	if (node->type == grecs_node_block &&
	    !GRECS_VALUE_EMPTY_P(node->v.value)) {
		clos->fmtfun("=", clos->data);
		grecs_format_value(node->v.value, flags|GRECS_NODE_FLAG_QUOTE,
				   clos);
	}
}

void
grecs_format_value(struct grecs_value *val, int flags,
		   struct grecs_format_closure *clos)
{
	int i;
	struct grecs_list_entry *ep;
	size_t clen;
	int need_quote;

	if (!val)
		return;
	switch (val->type) {
	case GRECS_TYPE_STRING:
		clen = wordsplit_c_quoted_length(val->v.string,
					   flags & GRECS_NODE_FLAG_QUOTE_HEX,
						 &need_quote);
		if (flags & GRECS_NODE_FLAG_QUOTE)
			need_quote = 1;
		else if (flags & GRECS_NODE_FLAG_NOQUOTE)
			need_quote = 0;
		if (need_quote) {
			char *cbuf = grecs_malloc(clen + 1);
			wordsplit_c_quote_copy(cbuf, val->v.string,
					    flags & GRECS_NODE_FLAG_QUOTE_HEX);
			cbuf[clen] = 0;
			clos->fmtfun("\"", clos->data);
			clos->fmtfun(cbuf, clos->data);
			clos->fmtfun("\"", clos->data);
			grecs_free(cbuf);
		} else
			clos->fmtfun(val->v.string, clos->data);
		break;

	case GRECS_TYPE_LIST:
		clos->fmtfun("(", clos->data);
		for (ep = val->v.list->head; ep; ep = ep->next) {
			grecs_format_value(ep->data, flags, clos);
			if (ep->next)
				clos->fmtfun(", ", clos->data);
		}
		clos->fmtfun(")", clos->data);
		break;

	case GRECS_TYPE_ARRAY:
		for (i = 0; i < val->v.arg.c; i++) {
			if (i)
				clos->fmtfun(" ", clos->data);
			grecs_format_value(val->v.arg.v[i], flags, clos);
		}
	}
}

int
grecs_format_node(struct grecs_node *node, int flags,
		  struct grecs_format_closure *clos)
{
	const char *delim_str = NULL;

	if (!(flags & _GRECS_NODE_MASK_OUTPUT)) {
		errno = EINVAL;
		return 1;
	}

	switch (node->type) {
	case grecs_node_root:
	case grecs_node_block:
		if (flags & GRECS_NODE_FLAG_DESCEND) {
			for (node = node->down; node; node = node->next) {
				grecs_format_node(node, flags, clos);
				if (node->next)
					clos->fmtfun("\n", clos->data);
			}
			break;
		}

	case grecs_node_stmt:
		if (flags & GRECS_NODE_FLAG_LOCUS) {
			grecs_locus_t *locus;
			
			if (flags & GRECS_NODE_FLAG_PATH) {
				if (flags & GRECS_NODE_FLAG_VALUE)
					locus = &node->locus;
				else
					locus = &node->idloc;
			} else if (flags & GRECS_NODE_FLAG_VALUE)
				locus = &node->v.value->locus;
			else
				locus = &node->locus;
			grecs_format_locus(locus, clos);
			delim_str = ": ";
		}
		if (flags & GRECS_NODE_FLAG_PATH) {
			if (delim_str)
				clos->fmtfun(delim_str, clos->data);
			grecs_format_node_path(node, flags, clos);
			delim_str = ": ";
		}
		if (flags & GRECS_NODE_FLAG_VALUE) {
			if (delim_str)
				clos->fmtfun(delim_str, clos->data);
			grecs_format_value(node->v.value, flags, clos);
		}
	}
	return 0;
}


static int
txtacc_fmt(const char *str, void *data)
{
	struct grecs_txtacc *acc = data;
	grecs_txtacc_grow(acc, str, strlen(str));
	return 0;
}

void
grecs_txtacc_format_value(struct grecs_value *val, int flags,
			  struct grecs_txtacc *acc)
{
	struct grecs_format_closure clos = { txtacc_fmt, acc };
	grecs_format_value(val, flags, &clos);
}


static int
file_fmt(const char *str, void *data)
{
	fputs(str, (FILE*)data);
	return 0;
}

void
grecs_print_locus(grecs_locus_t *locus, FILE *fp)
{
	struct grecs_format_closure clos = { file_fmt, fp };
	grecs_format_locus(locus, &clos);
}

void
grecs_print_node_path(struct grecs_node *node, int flag, FILE *fp)
{
	struct grecs_format_closure clos = { file_fmt, fp };
	grecs_format_node_path(node, flag, &clos);
}

void
grecs_print_value(struct grecs_value *val, int flags, FILE *fp)
{
	struct grecs_format_closure clos = { file_fmt, fp };
	grecs_format_value(val, flags, &clos);
}

int
grecs_print_node(struct grecs_node *node, int flags, FILE *fp)
{
	struct grecs_format_closure clos = { file_fmt, fp };
	return grecs_format_node(node, flags, &clos);
}
