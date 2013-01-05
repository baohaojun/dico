/* This file is part of GNU Dico.
   Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <string.h>
#include <errno.h>

const char *dico_markup_type = "none";
dico_list_t dico_markup_list;

static int
cmp_markup_name(const void *item, void *data)
{
    return strcasecmp((char*)item, (char*)data);
}

const char *
dico_markup_lookup(const char *name)
{
    return dico_list_locate(dico_markup_list, (void *)name);
}

int
dico_markup_valid_name_p(const char *name)
{
    for (; *name; name++)
	if (!(isascii(*name) && (isalnum(*name) || *name == '_')))
	    return 0;
    return 1;
}

int
dico_markup_register(const char *name)
{
    if (!dico_markup_valid_name_p(name))
	return EINVAL;
    
    if (!dico_markup_list) {
	dico_markup_list = dico_list_create();
	if (!dico_markup_list)
	    return ENOMEM;
	dico_list_set_comparator(dico_markup_list, cmp_markup_name);
    }

    if (!dico_markup_lookup(name)) {
	char *s = strdup(name);
	if (!s)
	    return ENOMEM;
	return dico_list_append(dico_markup_list, s);
    }
    return 0;
}

