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

#include <dicod.h>

static void
dicod_xidle(dico_stream_t str, int argc, char **argv)
{
    stream_printf(str, "110 %lu second(s)\n", inactivity_timeout);
}
    
void
register_xidle()
{
    static struct dicod_command cmd[] = {
	{ "XIDLE", 1, 1, NULL, "report inactivity timeout, reset idle timer",
	  dicod_xidle },
	{ NULL }
    };
    dicod_capa_register("xidle", cmd, NULL, NULL);
}

