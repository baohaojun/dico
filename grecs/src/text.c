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
#include <string.h>

static struct grecs_symtab *text_table;

/* Lookup a text. If it does not exist, create it. */
char *
grecs_install_text(const char *str)
{
	struct grecs_syment key;
	struct grecs_syment *ent;
	int install = 1;

	if (!text_table) {
		text_table = grecs_symtab_create_default(
			              sizeof(struct grecs_syment));
		if (!text_table)
			grecs_alloc_die();
	}
  
	key.name = (char*) str;
	ent = grecs_symtab_lookup_or_install(text_table, &key, &install);
	if (!ent)
		grecs_alloc_die();
	return ent->name;
}

void
grecs_destroy_text()
{
	grecs_symtab_free(text_table);
}

struct grecs_symtab *
grecs_text_table()
{
	struct grecs_symtab *tmp = text_table;
	text_table = NULL;
	return tmp;
}
	
