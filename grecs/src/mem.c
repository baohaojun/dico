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
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void *
def_malloc_fun(size_t size)
{
	return malloc(size);
}

static void *
def_realloc_fun(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

static void
def_free_fun(void *ptr)
{
	free(ptr);
}

void *(*grecs_malloc_fun)(size_t size) = def_malloc_fun;
void *(*grecs_realloc_fun)(void *ptr, size_t size) = def_realloc_fun;
void (*grecs_alloc_die_fun)(void);
void (*grecs_free_fun)(void *) = def_free_fun;

void
grecs_free(void *ptr)
{
	grecs_free_fun(ptr);
}

void *
grecs_malloc(size_t size)
{
	void *ptr = grecs_malloc_fun(size);
	if (!ptr)
		grecs_alloc_die();
	return ptr;
}

void *
grecs_zalloc(size_t size)
{
	void *ptr = grecs_malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void *
grecs_calloc(size_t nmemb, size_t size)
{
	return grecs_zalloc(nmemb * size);
}

void *
grecs_realloc(void *ptr, size_t size)
{
	void *newptr = grecs_realloc_fun(ptr, size);
	if (!newptr)
		grecs_alloc_die();
	return newptr;
}

char *
grecs_strdup(const char *str)
{
	char *newstr = grecs_malloc(strlen(str) + 1);
	return strcpy(newstr, str);
}

void
grecs_alloc_die(void)
{
	if (grecs_alloc_die_fun)
		grecs_alloc_die_fun();
	grecs_error(NULL, ENOMEM, "fatal error");
	exit(70); /* EX_SOFTWARE */
}
