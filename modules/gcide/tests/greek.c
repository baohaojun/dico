/* This file is part of GNU Dico.
   Copyright (C) 2012 Sergey Poznyakoff

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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "gcide.h"

static void
print_greek(const char *arg)
{
    while (*arg) {
	size_t rd;
	const char *greek = gcide_grk_to_utf8(arg, &rd);
	
	if (greek) {
	    printf("%s", greek);
	    arg += rd;
	} else {
	    if (!(*arg == '-' || isspace(*arg)))
		printf("<!>");
	    putchar(*arg);
	    arg++;
	}
    }
    putchar('\n');
}    

int
main(int argc, char **argv)
{
    if (argc > 1) {
	while (--argc)
	    print_greek(*++argv);
    } else {
	char buf[80];

	while (fgets(buf, sizeof(buf), stdin)) {
	    buf[strlen(buf)-1] = 0;
	    print_greek(buf);
	}
    }
    return 0;
}
	
	
