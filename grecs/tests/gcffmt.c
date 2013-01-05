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
#include "grecs.h"

struct list_helper {
	FILE *file;
	int delim;
};

static int
list_parser_types(const char *name, grecs_parser_t parser, void *data)
{
	struct list_helper *p = data;
	fprintf(p->file, "%c%s", p->delim, name);
	p->delim = '|';
	return 0;
}

static void
usage(const char *arg, FILE *fp, int code)
{
	struct list_helper lh;
	
	fprintf(fp,
		"usage: %s [-h] [-list[=type]] [-locus] [-delim=char] [-reduce] [-sort]\n",
		arg);
	
	fprintf(fp, "    [-nopath] [-novalue] [-type");
	lh.file = fp;
	lh.delim = '=';
	grecs_enumerate_parsers(list_parser_types, &lh);
	fprintf(fp, "] [-Idir] [-include=dir] file [file...]\n");
	exit(code);
}

static int
node_ident_cmp(struct grecs_node const *a, struct grecs_node const *b)
{
	return strcmp(a->ident, b->ident);
}

int
list_parser(const char *name, grecs_parser_t parser, void *data)
{
	printf("%s\n", name);
	return 0;
}

int
find_parser(const char *name, grecs_parser_t parser, void *data)
{
	if (strcasecmp(name, (char*)data) == 0)
		exit(0);
	return 0;
}

int
set_parser(const char *arg)
{
	grecs_parser_t p = grecs_get_parser_by_type(arg);
	if (p) {
		grecs_parser_fun = p;
		return 0;
	}
	return 1;
}

int
main(int argc, char **argv)
{
	char *progname = argv[0];
	char *file = NULL;
	struct grecs_node *tree;
	int flags = GRECS_NODE_FLAG_DEFAULT;
	int reduce = 0;
	int sort = 0;
	
	while (--argc) {
		char *arg = *++argv;
		if (strcmp(arg, "-locus") == 0)
			flags |= GRECS_NODE_FLAG_LOCUS;
		else if (strcmp(arg, "-nopath") == 0)
			flags &= ~GRECS_NODE_FLAG_PATH;
		else if (strcmp(arg, "-novalue") == 0)
			flags &= ~GRECS_NODE_FLAG_VALUE;
		else if (strncmp(arg, "-delim=", 7) == 0)
			flags |= arg[7];
		else if (strcmp(arg, "-reduce") == 0)
			reduce = 1;
		else if (strcmp(arg, "-sort") == 0)
			sort = 1;
		else if (strcmp(arg, "-h") == 0)
			usage(progname, stdout, 0);
		else if (strcmp(arg, "-list") == 0) {
			grecs_enumerate_parsers(list_parser, NULL);
			exit(0);
		} else if (strncmp(arg, "-list=", 6) == 0) {
			grecs_enumerate_parsers(find_parser, arg + 6);
			exit(2);
		} else if (strncmp(arg, "-type=", 6) == 0) {
			if (set_parser(arg + 6))
				usage(progname, stderr, 1);
		} else if (strncmp(arg, "-I", 2) == 0)
			grecs_preproc_add_include_dir(arg+2);
		else if (strncmp(arg, "-include=", 9) == 0)
			grecs_preproc_add_include_dir(arg+9);
		else if (strcmp(arg, "-x") == 0)
			grecs_gram_trace(1);
		else if (strcmp(arg, "-X") == 0)
			grecs_lex_trace(1);
		else if (arg[0] == '-')
			usage(progname, stderr, 1);
		else {
			file = arg;
			--argc;
			break;
		}
	}

	if (!grecs_parser_fun) {
		fprintf(stderr, "%s: requested type not supported", progname);
		exit(2);
	}
	
	if (!file)
		usage(progname, stderr, 1);

	tree = grecs_parse(file);
	if (!tree)
		exit(1);

	for (; argc; argc--) {
		char *arg = *++argv;
		struct grecs_node *node = grecs_parse(arg);
		if (!node)
			exit(1);
		if (grecs_tree_join(tree, node)) {
			fprintf(stderr, "join failed\n");
			exit(1);
		}
		grecs_tree_free(node);
	}
	
	if (reduce)
		grecs_tree_reduce(tree, NULL, 0);
	if (sort)
		grecs_tree_sort(tree, node_ident_cmp);
	grecs_print_node(tree, flags, stdout);
	fputc('\n', stdout);
	grecs_tree_free(tree);
	exit(0);
}
