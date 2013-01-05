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
#include <grecs.h>
#include <string.h>
#include <ctype.h>
#include "gitid.h"

const char *grecs_vcs_id = "$Id: " GRECS_GIT_ID " $";

struct grecs_version_info *
grecs_version_split(const char *vstr)
{
	char *p;
	size_t len;
	struct grecs_version_info *pv = grecs_zalloc(sizeof(*pv));
	
	pv->buffer = grecs_strdup(vstr);
	len = strcspn(pv->buffer, " \t");
	if (pv->buffer[len]) {
		pv->package = pv->buffer;
		pv->buffer[len++] = 0;
		for (; pv->buffer[len] && isspace(pv->buffer[len]); len++)
			;
	} else {
		pv->package = NULL;
		len = 0;
	}
	if (!pv->buffer[len]) {
		free(pv->buffer);
		free(pv);
		return NULL;
	}
	pv->version = pv->buffer + len;
	pv->major = strtoul(pv->version, &p, 10);
	if (*p && *p == '.') {
		pv->minor = strtoul(p + 1, &p, 10);
		if (*p && *p == '.') {
			char *q;
			
			pv->patch = strtoul(p + 1, &q, 10);
			if (q == p + 1)
				pv->patch = 0;
			else
				p = q;
		}
	}
	pv->suffix = p;
	return pv;
}

void
grecs_version_info_free(struct grecs_version_info *pv)
{
	if (pv) {
		if (pv->buffer)
			grecs_free(pv->buffer);
		free(pv);
	}
}

struct grecs_version_info *
grecs_version(void)
{
	struct grecs_version_info *pv = grecs_zalloc(sizeof(*pv));
	size_t size = 0;
	
	pv->package = PACKAGE_NAME;
#ifdef GRECS_VERSION_PATCHLEVEL
# ifdef GRECS_VERSION_SUFFIX
	grecs_asprintf(&pv->buffer, &size,
		       "%d.%d.%d%s",
		       GRECS_VERSION_MAJOR, GRECS_VERSION_MINOR,
		       GRECS_VERSION_PATCHLEVEL, GRECS_VERSION_SUFFIX);
# else
	grecs_asprintf(&pv->buffer, &size,
		       "%d.%d.%d",
		       GRECS_VERSION_MAJOR, GRECS_VERSION_MINOR,
		       GRECS_VERSION_PATCHLEVEL);
# endif	
#else
# ifdef GRECS_VERSION_SUFFIX
	grecs_asprintf(&pv->buffer, &size,
		       "%d.%d%s",
		       GRECS_VERSION_MAJOR, GRECS_VERSION_MINOR,
		       GRECS_VERSION_SUFFIX);
# else
	grecs_asprintf(&pv->buffer, &size,
		       "%d.%d",
		       GRECS_VERSION_MAJOR, GRECS_VERSION_MINOR);
# endif
#endif
	pv->version = pv->buffer;
	pv->major = GRECS_VERSION_MAJOR;
	pv->minor = GRECS_VERSION_MINOR;
#ifdef GRECS_VERSION_PATCHLEVEL
	pv->patch = GRECS_VERSION_PATCHLEVEL;
#endif
#ifdef GRECS_VERSION_SUFFIX
	pv->suffix = GRECS_VERSION_SUFFIX;
#endif
	pv->id = GRECS_GIT_ID;
	return pv;
}

int
grecs_version_info_cmp(struct grecs_version_info *vx,
		       struct grecs_version_info *vy,
		       int *pres)
{
	if (vx->package && vy->package && strcmp(vx->package, vy->package))
		return 1;
	else if (vx->major > vy->major)
		*pres = 1;
	else if (vx->major < vy->major)
		*pres = -1;
	else if (vx->minor > vy->minor)
		*pres = 1;
	else if (vx->minor < vy->minor)
		*pres = -1;
	else if (vx->patch > vy->patch)
		*pres = 1;
	else if (vx->patch < vy->patch)
		*pres = -1;
	else if (vx->suffix && vy->suffix)
		*pres = strcmp(vx->suffix, vy->suffix);
	else
		*pres = 0;
	return 0;
}

int
grecs_version_cmp(const char *vstr1, const char *vstr2, int *pres)
{
	struct grecs_version_info *v1, *v2;
	int rc;
	
	if (!vstr1 || !vstr2)
		return -1;
	v1 = grecs_version_split(vstr1);
	if (!v1)
		return -1;
	v2 = grecs_version_split(vstr2);
	if (!v2) {
		grecs_version_info_free(v1);
		return -1;
	}
	rc = grecs_version_info_cmp(v1, v2, pres);
	grecs_version_info_free(v1);
	grecs_version_info_free(v2);
	return rc;
}

int
grecs_version_ok(const char *vstr)
{
	struct grecs_version_info *vmy, *vreq;
	int rc, res;
	
	if (!vstr)
		return -1;
	vreq = grecs_version_split(vstr);
	if (!vreq)
		return -1;
	vmy = grecs_version();
	if (vreq->suffix && !vmy->suffix)
		vmy->suffix = "";
	rc = grecs_version_info_cmp(vmy, vreq, &res);
	grecs_version_info_free(vmy);
	grecs_version_info_free(vreq);
	return rc == 0 && res >= 0;
}
