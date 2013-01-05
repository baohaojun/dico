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
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include "grecs.h"

static struct parser_tab {
	const char *name;
	grecs_parser_t parser;
} parser_tab[] = {
	{ "GRECS", grecs_grecs_parser },
	{ "PATH", grecs_path_parser },
#ifdef ENABLE_META1_PARSER
	{ "META1", grecs_meta1_parser },
#endif
#ifdef ENABLE_BIND_PARSER
	{ "BIND", grecs_bind_parser },
#endif
#ifdef ENABLE_GIT_PARSER
	{ "GIT", grecs_git_parser },
#endif
	{ NULL }
};

int
grecs_enumerate_parsers(int (*fun)(const char *, grecs_parser_t, void *),
			void *data)
{
	struct parser_tab *pt;
	int rc = 0;
	
	for (pt = parser_tab; rc == 0 && pt->name; pt++)
		rc = fun(pt->name, pt->parser, data);
	return rc;
}

grecs_parser_t
grecs_get_parser_by_type(const char *type)
{
	struct parser_tab *pt;

	for (pt = parser_tab; pt->name; pt++) {
		if (strcasecmp(pt->name, type) == 0)
			return pt->parser;
	}
	return NULL;
}

