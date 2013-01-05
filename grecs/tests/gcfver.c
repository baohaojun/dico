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
#include "grecs.h"

int
main(int argc, char **argv)
{
	struct grecs_version_info *vinfo;
	int res;
	
	if (argc > 3) {
		fprintf(stderr, "usage: %s [version [version]]\n", argv[0]);
		exit(1);
	}
	if (argc == 2)
		exit(!grecs_version_ok(argv[1]));
	if (argc == 3)
		exit(grecs_version_cmp(argv[1], argv[2], &res) ? 1 :
		     res == 0 ? 0 : res < 0 ? 2 : 3);
	/* Default action: */
	vinfo = grecs_version();
	printf("package: %s\n", vinfo->package);
	printf("version: %s\n", vinfo->version);
	printf("major: %d\n", vinfo->major);
	printf("minor: %d\n", vinfo->minor);
	printf("patch: %d\n", vinfo->patch);
	printf("suffix:");
	if (vinfo->suffix && vinfo->suffix[0])
		printf(" %s", vinfo->suffix);
	putchar('\n');
	grecs_version_info_free(vinfo);
	exit(0);
}

		
