/* This file is part of Dico.
   Copyright (C) 2008, 2010, 2012 Sergey Poznyakoff

   Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Dico.  If not, see <http://www.gnu.org/licenses/>. */

#include <dicod.h>

dico_list_t dicod_lang_lazy_prefs;
dico_list_t dicod_lang_prefs[2];

static int
cmp_string_ci(const void *a, void *b)
{
    return c_strcasecmp(a, b);
}

int
dicod_lang_check(dico_list_t list[2])
{
    if (!list[0] && !list[1])
	return 1;
    
    if (dicod_lang_lazy_prefs) {
	if (list[0] && dico_list_intersect_p(dicod_lang_lazy_prefs, list[0],
					     cmp_string_ci))
	    return 1;
	if (list[1] && dico_list_intersect_p(dicod_lang_lazy_prefs, list[1],
					     cmp_string_ci))
	    return 1;
	return 0;
    } 

    if (dicod_lang_prefs[0] && list[0]
	&& !dico_list_intersect_p(dicod_lang_prefs[0], list[0], cmp_string_ci))
	return 0;
    if (dicod_lang_prefs[1] && list[1]
	&& !dico_list_intersect_p(dicod_lang_prefs[1], list[1], cmp_string_ci))
	return 0;
    return 1;
}

void
dicod_lang(dico_stream_t str, int argc, char **argv)
{
    dico_list_destroy(&dicod_lang_lazy_prefs);
    dico_list_destroy(&dicod_lang_prefs[0]);
    dico_list_destroy(&dicod_lang_prefs[1]);
    if (argc > 2) {
	int n = 0;
	int i;
	
	for (i = 2; i < argc; i++) {
	    if (n == 0 && strcmp(argv[i], ":") == 0) 
		n = 1;
	    else {
		if (!dicod_lang_prefs[n]) {
		    dicod_lang_prefs[n] = xdico_list_create();
		    dico_list_set_free_item(dicod_lang_prefs[n],
					    dicod_free_item, NULL);
		}
		xdico_list_append(dicod_lang_prefs[n], xstrdup(argv[i]));
	    }
	}
	if (n == 0) {
	    dicod_lang_lazy_prefs = dicod_lang_prefs[0];
	    dicod_lang_prefs[0] = NULL;
	}
    }
    check_db_visibility();
    stream_writez(str, "250 ok - set language preferences\n");
}

static int
_display_pref(void *item, void *data)
{
    dico_stream_t str = data;
    stream_writez(str, " ");
    stream_writez(str, item);
    return 0;
}

static void
show_lang_lists(dico_stream_t str, dico_list_t list[2])
{
    dico_list_iterate(list[0], _display_pref, str);
    dico_stream_write(str, " :", 2);
    dico_list_iterate(list[1], _display_pref, str);
    dico_stream_write(str, "\n", 1);
}    

void
dicod_show_lang_pref(dico_stream_t str, int argc, char **argv)
{
    stream_writez(str, "280");
    if (dicod_lang_lazy_prefs) {
	dico_list_iterate(dicod_lang_lazy_prefs, _display_pref, str);
	dico_stream_write(str, "\n", 1);
    } else 
	show_lang_lists(str, dicod_lang_prefs);
}

void
dicod_show_lang_info(dico_stream_t str, int argc, char **argv)
{
    dicod_database_t *db = find_database(argv[3]);
    if (!db) {
	stream_writez(str,
		      "550 invalid database, use SHOW DB for a list\n");
    } else {
	dico_list_t langlist[2];

	dicod_get_database_languages(db, langlist);
	stream_writez(str, "280");
	show_lang_lists(str, langlist);
    }
}

static int
_show_database_lang(void *item, void *data)
{
    dicod_database_t *db = item;
    dico_stream_t str = data;
    dico_list_t langlist[2];
    dicod_get_database_languages(db, langlist);
    stream_printf(str, "%s", db->name);
    show_lang_lists(str, langlist);
    return 0;
}

void
dicod_show_lang_db(dico_stream_t str, int argc, char **argv)
{
    size_t count = database_count();
    if (count == 0) 
	stream_printf(str, "554 No databases present\n");
    else {
	dico_stream_t ostr;
	
	stream_printf(str, "110 %lu databases present\n",
		      (unsigned long) count);
	ostr = dicod_ostream_create(str, NULL);
	database_iterate(_show_database_lang, ostr);
	dico_stream_close(ostr);
	dico_stream_destroy(&ostr);
	stream_writez(str, ".\n");
	stream_writez(str, "250 ok\n");
    }
}

void
register_lang()
{
    static struct dicod_command cmd[] = {
	{ "OPTION LANG", 2, DICOD_MAXPARAM_INF, "[list]",
	  "define language preferences",
	  dicod_lang },
	{ "SHOW LANG DB", 3, 3, NULL,
	  "show databases with their language preferences",
	  dicod_show_lang_db },
	{ "SHOW LANG DATABASES", 3, 3, NULL,
	  "show databases with their language preferences",
	  dicod_show_lang_db },
	{ "SHOW LANG INFO", 4, 4, "database",
	  "show language preferences of a database",
	  dicod_show_lang_info },
	{ "SHOW LANG PREF", 3, 3, NULL,
	  "show server language preferences",
	  dicod_show_lang_pref },
	{ NULL }
    };
    dicod_capa_register("lang", cmd, NULL, NULL);
}
