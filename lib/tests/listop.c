/* This file is part of GNU Dico
   Copyright (C) 2003,2004,2007,2008,2012 Sergey Poznyakoff
  
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "itrsh.h"

void
usage(int code)
{
    printf("usage: listop..]\n");
    exit(code);
}

dico_iterator_t
list_iterator(void *object)
{
    return dico_list_iterator(object);
}

void
print_item(void *item)
{
    printf("%s", (char*)item);
}

void
delete(void *object, int argc, char **argv)
{
    dico_list_t list = object;

    while (--argc) {
	if (dico_list_remove(list, *++argv, NULL))
	    dico_log(L_ERR, errno, "dico_list_remove(%s)", *argv);
    }
}

void
add(void *object, int argc, char **argv)
{
    dico_list_t list = object;
    
    while (--argc) {
	if (dico_list_append(list, strdup(*++argv)))
	    dico_log(L_ERR, errno, "dico_list_append");
    }
}

void
prep(void *object, int argc, char **argv)
{
    dico_list_t list = object;
    
    while (--argc) {
	if (dico_list_prepend(list, strdup(*++argv))) 
	    dico_log(L_ERR, errno, "dico_list_prepend");
    }
}

void
find(void *object, int argc, char **argv)
{
    dico_iterator_t itr;
    char *arg = argv[1];
    char *text;
    
    itr = shell_iterator();
    for (text = dico_iterator_first(itr);
	 text;
	 text = dico_iterator_next(itr)) {
	if (strcmp(arg, text) == 0)
	    return;
    }

    dico_log(L_ERR, 0, "%s not in list", arg);
}

void
push(void *object, int argc, char **argv)
{
    dico_list_t list = object;

    while (--argc)
	if (dico_list_push(list, strdup(*++argv)))
	    dico_log(L_ERR, errno, "dico_list_push");
}

void
pop(void *object, int argc, char **argv)
{
    dico_list_t list = object;
    char *p = dico_list_pop(list);
    if (!p)
	dico_log(L_NOTICE, 0, "nothing to pop");
    else
	printf("%s\n", p);
    free(p);
}
    
void
number(void *object, int argc, char **argv)
{
    dico_list_t list = object;
    char *p;
    size_t n = strtoul(argv[0], &p, 0);
    if (*p != 0)
	fprintf(stderr, "?\n");
    else {
	char *text = dico_list_item(list, n);
	if (!text) {
	    if (errno == ENOENT)
		dico_log(L_ERR, 0, "dico_list_item: item not found");
	    else
		dico_log(L_ERR, errno, "dico_list_item");
	    return;
	}
	printf("%s\n", text);
    }
}

static int
string_comp(const void *item, void *value)
{
    return strcmp(item, value);
}

struct itr_shell_command cmdtab[] = {
    { "del", 2, 0, delete, "item...", "delete items" },
    { "add", 2, 0, add,    "item...", "add new items" },
    { "prep", 2, 0, prep,  "item...", "add new items at the beginning" },
    { "find", 2, 2, find,  "item",    "find item in the list" },
    { "push", 2, 0, push,  "item...", "push items to the list" },
    { "pop", 1, 1, pop,    "", "pop item from the list" },
    { NULL, 1, 1, number }
};

static size_t
get_count(void *data)
{
    return dico_list_count((dico_list_t)data);
}

int
main(int argc, char **argv)
{
    struct itr_shell sh;
    dico_list_t list;
  
    dico_set_program_name(argv[0]);

    while (--argc) {
	char *arg = *++argv;
	if (strcmp(arg, "-h") == 0)
	    usage(0);
	else if (strcmp(arg, "--") == 0) {
	    --argc;
	    ++argv;
	    break;
	} else if (arg[0] == '-') {
	    dico_log(L_ERR, 0, "unknown option %s", arg);
	    exit(1);
	} else
	    break;
    }

    list = dico_list_create();
    if (!list) {
	perror("dico_list_create");
	exit(1);
    }
    dico_list_set_comparator(list, string_comp);
    
    while (argc--) {
	if (dico_list_append(list, *argv++)) {
	    perror("dico_list_append");
	    exit(1);
	}
    }

    memset(&sh, 0, sizeof(sh));
    sh.object = list;
    sh.get_iterator = list_iterator;
    sh.print_item = print_item;
    sh.count = get_count;
    sh.cmdtab = cmdtab;
    shell(&sh);
  
    return 0;
}
