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

int option_mime;

void
dicod_mime(dico_stream_t str, int argc, char **argv)
{
    option_mime = 1;
    stream_writez(str, "250 ok - using MIME headers\n");
}

void
register_mime()
{
    static struct dicod_command cmd[] = {
	{ "OPTION MIME", 2, 2, NULL, "use MIME headers",
	  dicod_mime },
	{ NULL }
    };
    dicod_capa_register("mime", cmd, NULL, NULL);
}
