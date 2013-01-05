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

static void
usage(const char *arg, FILE *fp, int code)
{
	fprintf(fp,
		"usage: %s [-h] [-locus] [-delim=char] [-nodesc] [-nopath] [-reduce] [-match] file path\n",
		arg);
	exit(code);
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
	char *path = NULL;
	char *file = NULL;
	struct grecs_node *tree, *node;
	int flags = GRECS_NODE_FLAG_DEFAULT;
	int rc = 2;
	int reduce = 0;
	int match = 0;
	
	while (--argc) {
		char *arg = *++argv;
		if (strcmp(arg, "-locus") == 0)
			flags |= GRECS_NODE_FLAG_LOCUS;
		else if (strncmp(arg, "-delim=", 7) == 0)
			flags |= arg[7];
		else if (strcmp(arg, "-reduce") == 0)
			reduce = 1;
		else if (strcmp(arg, "-match") == 0)
			match = 1;
		else if (strcmp(arg, "-nodesc") == 0)
			flags &= ~GRECS_NODE_FLAG_DESCEND;
		else if (strcmp(arg, "-nopath") == 0)
			flags &= ~GRECS_NODE_FLAG_PATH;
		else if (strncmp(arg, "-type=", 6) == 0) {
			if (set_parser(arg + 6))
				usage(progname, stderr, 1);
		} else if (strcmp(arg, "-h") == 0)
			usage(progname, stdout, 0);
		else if (arg[0] == '-')
			usage(progname, stderr, 1);
		else if (file) {
			if (path)
				usage(progname, stderr, 1);
			else
				path = arg;
		} else
			file = arg;
	}
	
	if (!file || !path || argc)
		usage(progname, stderr, 1);

	tree = grecs_parse(file);
	if (!tree)
		exit(1);
	if (reduce)
		grecs_tree_reduce(tree, NULL, 0);

	if (match) {
		grecs_match_buf_t match_buf;

		for (node = grecs_match_first(tree, path, &match_buf);
		     node;
		     node = grecs_match_next(match_buf)) {
			rc = 0;
			grecs_print_node(node, flags, stdout);
			fputc('\n', stdout);
		}
		grecs_match_buf_free(match_buf);
	} else {
		for (node = tree; node; node = node->next) {
			node = grecs_find_node(node, path);
			if (!node)
				break;
			rc = 0;
			grecs_print_node(node, flags, stdout);
			fputc('\n', stdout);
		}
	}
	grecs_tree_free(tree);
	exit(rc);
}
