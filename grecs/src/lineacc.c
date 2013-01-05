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
#include <wordsplit.h>

static struct grecs_txtacc *line_acc;
    
void
grecs_line_acc_create()
{
	line_acc = grecs_txtacc_create();
}

void
grecs_line_acc_free()
{
	grecs_txtacc_free(line_acc);
	line_acc = NULL;
}

void
grecs_line_acc_grow_char(int c)
{
	char t = c;
	grecs_txtacc_grow(line_acc, &t, 1);
}

void
grecs_line_acc_grow_char_unescape(int c)
{
	if (c != '\n')
		grecs_line_acc_grow_char(wordsplit_c_unquote_char(c));
}

void
grecs_line_acc_grow(const char *text, size_t len)
{
	grecs_txtacc_grow(line_acc, text, len);
}

/* Same, but unescapes the last character from text */
void
grecs_line_acc_grow_unescape_last(char *text, size_t len)
{
	grecs_txtacc_grow(line_acc, text, len - 2);
	grecs_line_acc_grow_char_unescape(text[len - 1]);
}

void
grecs_line_begin()
{
	if (!line_acc)
		grecs_line_acc_create();
}

char *
grecs_line_finish()
{
	grecs_line_acc_grow_char(0);
	return grecs_txtacc_finish(line_acc, 1);
}




