/* This file is part of GNU Dico.
   Copyright (C) 1998-2000, 2008, 2010, 2012 Sergey Poznyakoff

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
#include <dico/udb.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct dico_udb {
    void *handle;
    dico_url_t url;
    const char *qpw;
    const char *qgrp;
    const char *options;
    int (*_db_open) (void **, dico_url_t, const char *);
    int (*_db_close) (void *);
    int (*_db_check_password) (void *, const char *, const char *,
			       const char *);
    int (*_db_get_password) (void *, const char *, const char *, char **);
    int (*_db_get_groups) (void *, const char *, const char *, dico_list_t *);
};

int
dico_udb_open(dico_udb_t db)
{
    if (!db)
	return 1;
    if (!db->_db_open)
	return 0;
    return db->_db_open(&db->handle, db->url, db->options);
}

int
dico_udb_close(dico_udb_t db)
{
    int rc;

    if (!db->_db_close)
	rc = 0;
    else 
	rc = db->_db_close(db->handle);
    db->handle = NULL;
    return rc;
}

int
dico_udb_check_password(dico_udb_t db, const char *key, const char *pass)
{
    if (!db->_db_check_password)
	return ENOSYS;
    return db->_db_check_password(db->handle, db->qpw, key, pass);
}

int
dico_udb_get_password(dico_udb_t db, const char *key, char **pass)
{
    return db->_db_get_password(db->handle, db->qpw, key, pass);
}

int
dico_udb_get_groups(dico_udb_t db, const char *key, dico_list_t *groups)
{
    return db->_db_get_groups(db->handle, db->qgrp, key, groups);
}

dico_list_t /* of struct dico_udb_def */ dico_udb_def_list;

static int
udb_def_cmp(const void *item, void *data)
{
    const struct dico_udb_def *def = item;
    const char *proto = data;
    return strcmp(def->proto, proto);
}

int
dico_udb_define(struct dico_udb_def *dptr)
{
    if (!dico_udb_def_list) {
	dico_udb_def_list = dico_list_create();
	if (!dico_udb_def_list) {
	    errno = ENOMEM;
	    return 1;
	}
	dico_list_set_comparator(dico_udb_def_list, udb_def_cmp);
    }
    return dico_list_append(dico_udb_def_list, dptr);
}

int
dico_udb_create(dico_udb_t *pdb,
		const char *urlstr, const char *qpw, const char *qgrp,
		const char *options)
{
    dico_url_t url;
    int rc;
    struct dico_udb_def *def;
    struct dico_udb *uptr;
    
    rc = dico_url_parse(&url, urlstr);
    if (rc) {
	errno = EINVAL;
	return 1;
    }

    def = dico_list_locate(dico_udb_def_list, url->proto);
    if (!def) {
	errno = EINVAL;
	dico_url_destroy(&url);
	return 1;
    }

    uptr = calloc(1, sizeof(*uptr));
    if (!uptr)
	return 1;
    
    uptr->url = url;
    uptr->qpw = qpw;
    uptr->qgrp = qgrp;
    uptr->options = options;
    uptr->_db_open = def->_db_open;
    uptr->_db_close = def->_db_close;
    uptr->_db_check_password = def->_db_check_password;
    uptr->_db_get_password = def->_db_get_password;
    uptr->_db_get_groups = def->_db_get_groups;

    *pdb = uptr;
    return 0;
}

