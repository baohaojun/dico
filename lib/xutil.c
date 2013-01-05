/* This file is part of GNU Dico
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
#include <xdico.h>
#include <xalloc.h>
#include <string.h>
#include <errno.h>
#include <c-strcase.h>

void
trimnl(char *buf, size_t len)
{
    if (len > 1 && buf[--len] == '\n') {
	buf[len] = 0;
	if (len > 1 && buf[--len] == '\r')
	    buf[len] = 0;
    }
}


/* X-versions of dico library calls */

char *
make_full_file_name(const char *dir, const char *file)
{
    char *s = dico_full_file_name(dir, file);
    if (!s)
	xalloc_die();
    return s;
}

dico_list_t
xdico_list_create()
{
    dico_list_t p = dico_list_create();
    if (!p)
	xalloc_die();
    return p;
}

dico_iterator_t
xdico_list_iterator(dico_list_t list)
{
    dico_iterator_t p = dico_list_iterator(list); 
    if (!p && errno == ENOMEM)
	xalloc_die();
    return p;
}

void
xdico_list_append(struct dico_list *list, void *data)
{
    if (dico_list_append(list, data) && errno == ENOMEM)
	xalloc_die();
}

void
xdico_list_prepend(struct dico_list *list, void *data)
{
    if (dico_list_prepend(list, data) && errno == ENOMEM)
	xalloc_die();
}

dico_assoc_list_t
xdico_assoc_create(int flags)
{
    dico_assoc_list_t p = dico_assoc_create(flags);
    if (!p)
	xalloc_die();
    return p;
}

void
xdico_assoc_append(dico_assoc_list_t assoc, const char *key, const char *value)
{
    if (dico_assoc_append(assoc, key, value) && errno == ENOMEM)
	xalloc_die();
}

int
xdico_assoc_add(dico_assoc_list_t assoc, const char *key,
		const char *value, size_t count, int replace)
{
    int rc = dico_assoc_add(assoc, key, value, count, replace);
    if (rc && errno == ENOMEM)
	xalloc_die();
    return rc;
}

char *
xdico_assign_string(char **dest, char *str)
{
    if (*dest)
	free (*dest);
    return *dest = str ? xstrdup (str) : NULL;
}
    
static char *mech_to_capa_table[][2] = {
    { "EXTERNAL", "external" },
    { "SKEY", "skey" },
    { "GSSAPI", "gssapi" },
    { "KERBEROS_V4", "kerberos_v4" }
};

char *
xdico_sasl_mech_to_capa(char *mech)
{
    int i;
    size_t len;
    char *rets, *p;
    
    for (i = 0; i < DICO_ARRAY_SIZE(mech_to_capa_table); i++)
	if (strcmp(mech_to_capa_table[i][0], mech) == 0)
	    return xstrdup(mech_to_capa_table[i][1]);

    len = strlen(mech) + 1;
    rets = p = xmalloc(len + 1);
    *p++ = 'x';
    for (; *mech; mech++)
	*p++ = tolower(*mech);
    *p = 0;
    return rets;
}
	
int
xdico_sasl_capa_match_p(const char *mech, const char *capa)
{
    int i;
    
    for (i = 0; i < DICO_ARRAY_SIZE(mech_to_capa_table); i++)
	if (c_strcasecmp(mech_to_capa_table[i][0], mech) == 0) 
	    return c_strcasecmp(mech_to_capa_table[i][1], capa) == 0;

    if (*capa == 'x') 
	return c_strcasecmp(mech, capa + 1) == 0;
    return 0;
}
	

int
dicod_free_item (void *item, void *data DICO_ARG_UNUSED)
{
    free(item);
    return 0;
}
    

