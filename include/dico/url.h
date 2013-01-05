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

#ifndef __dico_url_h
#define __dico_url_h

/* URLs */

#define DICO_REQUEST_DEFINE 0
#define DICO_REQUEST_MATCH 1

struct dico_request {
    int type;
    char *word;
    char *database;
    char *strategy;
    unsigned long n;
};

struct dico_url {
    char *string;
    char *proto;
    char *host;
    int port;
    char *path;
    char *user;
    char *passwd;
    dico_assoc_list_t args;
    struct dico_request req;
};

typedef struct dico_url *dico_url_t;
int dico_url_parse(dico_url_t *purl, const char *str);
void dico_url_destroy(dico_url_t *purl);
const char *dico_url_get_arg(dico_url_t url, const char *argname);
char *dico_url_full_path(dico_url_t url);

#endif
