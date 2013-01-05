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

#ifndef __dico_udb_h
#define __dico_udb_h

#include <dico/types.h>

/* user db */
struct dico_udb_def {
    const char *proto;
    int (*_db_open) (void **, dico_url_t, const char *);
    int (*_db_close) (void *);
    int (*_db_get_password) (void *, const char *, const char *, char **);
    int (*_db_get_groups) (void *, const char *, const char *, dico_list_t *);
    int (*_db_check_password) (void *, const char *, const char *,
			       const char *);
};

typedef struct dico_udb *dico_udb_t;

void dico_udb_init(void);
int dico_udb_create(dico_udb_t *pdb,
		    const char *urlstr, const char *qpw, const char *qgrp,
		    const char *options);

int dico_udb_open(dico_udb_t db);
int dico_udb_close(dico_udb_t db);
int dico_udb_check_password(dico_udb_t db, const char *key, const char *pass);
int dico_udb_get_password(dico_udb_t db, const char *key, char **pass);
int dico_udb_get_groups(dico_udb_t db, const char *key, dico_list_t *groups);
int dico_udb_define(struct dico_udb_def *dptr);

#endif
