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


static int
mod_init(int argc, char **argv)
{
    /* FIXME */
    return 1;
}
    
static dico_handle_t
mod_init_db(const char *dbname, int argc, char **argv)
{
    /* FIXME */
    return NULL;
}

static int
mod_free_db(dico_handle_t hp)
{
    /* FIXME */
    return 1;
}

static int
mod_open(dico_handle_t dp)
{
    /* FIXME */
    return 1;
}

static int
mod_close(dico_handle_t hp)
{
    /* FIXME */
    return 1;
}

static char *
mod_info(dico_handle_t hp)
{
    /* FIXME */
    return NULL;
}

static char *
mod_descr(dico_handle_t hp)
{
    /* FIXME */
    return NULL;
}

int
mod_lang(dico_handle_t hp, dico_list_t list[2])
{
    /* FIXME */
    return 1;
}

static dico_result_t
mod_match(dico_handle_t hp, const dico_strategy_t strat, const char *word)
{
    /* FIXME */
    return NULL;
}

static dico_result_t
mod_define(dico_handle_t hp, const char *word)
{
    /* FIXME */
    return NULL;
}

static int
mod_output_result (dico_result_t rp, size_t n, dico_stream_t str)
{
    /* FIXME */
    return 1;
}

static size_t
mod_result_count (dico_result_t rp)
{
    /* FIXME */
    return 0;
}

static size_t
mod_compare_count (dico_result_t rp)
{
    /* FIXME */
    return 0;
}

static void
mod_free_result(dico_result_t rp)
{
    /* FIXME */
}

static int
mod_result_headers(dico_result_t rp, dico_assoc_list_t hdr)
{  
    /* FIXME */
    return 0;
}

struct dico_database_module DICO_EXPORT(sample, module) = {
    DICO_MODULE_VERSION,
    DICO_CAPA_NONE,
    mod_init,
    mod_init_db,
    mod_free_db,
    mod_open,
    mod_close,
    mod_info,
    mod_descr,
    mod_lang,
    mod_match,
    mod_define,
    mod_output_result,
    mod_result_count,
    mod_compare_count,
    mod_free_result,
    mod_result_headers
};
