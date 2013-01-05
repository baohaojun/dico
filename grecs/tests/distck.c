#include <config.h>
#include "grecs.h"

int
main(int argc, char **argv)
{
	int res;
	struct grecs_version_info *packver, *libver;

	packver = grecs_version_split(PACKAGE_STRING);
	libver  = grecs_version();
	if (!(grecs_version_info_cmp(packver, libver, &res) == 0 &&
	      res == 0)) {
		    fprintf(stderr, "grecs.h does not match package number\n");
		    return 1;
	}
	return 0;
}
