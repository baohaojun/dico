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
#include <stdio.h>
#include <string.h>
#include <dico.h>

int
main(int argc, char **argv)
{
    int flags = 0;

    dico_set_program_name(argv[0]);

    while (--argc) {
	char *arg = *++argv;
	if (strcmp(arg, "-d") == 0)
	    flags |= DICO_LEV_DAMERAU;
	else if (strcmp(arg, "-n") == 0)
	    flags |= DICO_LEV_NORM;
	else if (strcmp(arg, "-h") == 0) {
	    printf("Usage: %s [-dn] word word\n", dico_program_name);
	    return 0;
	} else if (strcmp(arg, "--") == 0) {
	    --argc;
	    ++argv;
	} else if (arg[0] == '-') {
	    dico_log(L_ERR, 0, "unknown option '%s'", arg);
	    return 1;
	} else
	    break;
    }
    if (argc != 2) {
	fprintf(stderr, "Usage: %s [-dn] word word\n", argv[0]);
	return 1;
    }
    printf("%d\n", dico_levenshtein_distance(argv[0], argv[1], flags));
    return 0;
}
