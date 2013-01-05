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
#include <string.h>
#include <errno.h>
#include "grecs.h"

int grecs_error_count;    
int grecs_default_port = 0;

int grecs_trace_flags = 0;
int grecs_adjust_string_locations;

#ifndef GRECS_DEFAULT_PARSER
# define GRECS_DEFAULT_PARSER grecs_grecs_parser
#endif

struct grecs_node *(*grecs_parser_fun)(const char *name, int trace) =
	GRECS_DEFAULT_PARSER;

void
grecs_gram_trace(int n)
{
	if (n)
		grecs_trace_flags |= GRECS_TRACE_GRAM;
	else
		grecs_trace_flags &= ~GRECS_TRACE_GRAM;
}

void
grecs_lex_trace(int n)
{
	if (n)
		grecs_trace_flags |= GRECS_TRACE_LEX;
	else
		grecs_trace_flags &= ~GRECS_TRACE_LEX;
}

struct grecs_node *
grecs_parse(const char *name)
{
	grecs_error_count = 0;
	grecs_current_locus_point.file = grecs_install_text(name);
	grecs_current_locus_point.line = 1;
	grecs_current_locus_point.col = 0;
	return grecs_parser_fun(name, grecs_trace_flags);
}
