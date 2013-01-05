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
		"usage: %s [-h] [-locus] [-delim=char] [-reduce] file\n",
		arg);
	exit(code);
}

int
main(int argc, char **argv)
{
	char *progname = argv[0];
	char *file = NULL;
	struct grecs_node *tree, *node;
	int flags = GRECS_NODE_FLAG_QUOTE|
		    GRECS_NODE_FLAG_PATH;
	int reduce = 0;
	
	while (--argc) {
		char *arg = *++argv;
		if (strcmp(arg, "-locus") == 0)
			flags |= GRECS_NODE_FLAG_LOCUS;
		else if (strncmp(arg, "-delim=", 7) == 0)
			flags |= arg[7];
		else if (strcmp(arg, "-reduce") == 0)
			reduce = 1;
		else if (strcmp(arg, "-h") == 0)
			usage(progname, stdout, 0);
		else if (arg[0] == '-')
			usage(progname, stderr, 1);
		else
			file = arg;
	}
	
	if (!file || argc)
		usage(progname, stderr, 1);

	tree = grecs_parse(file);
	if (!tree)
		exit(1);
	if (reduce)
		grecs_tree_reduce(tree, NULL, 0);

	for (node = grecs_tree_first_node(tree); node;
	     node = grecs_next_node(node)) {
		grecs_print_node(node, flags, stdout);
		fputc('\n', stdout);
	}
	grecs_tree_free(tree);
	exit(0);
}
	
		
