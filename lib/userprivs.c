/* This file is part of GNU Dico.
   Copyright (C) 2007-2008, 2010, 2012 Sergey Poznyakoff

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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>
#include <errno.h>
#include <xalloc.h>
#include <xdico.h>
#include <libi18n.h>

/* Switch to the given UID/GID */
int
switch_to_privs (uid_t uid, gid_t gid, dico_list_t retain_groups)
{
    int rc = 0;
    gid_t *emptygidset;
    size_t size = 1, j = 1;
    dico_iterator_t itr;
    void *gp;

    _dico_libi18n_init();
    
    if (uid == 0) {
	dico_log(L_ERR, 0, _("Refusing to run as root"));
	return 1;
    }

    /* Create a list of supplementary groups */
    size = dico_list_count(retain_groups);
    size++;
    emptygidset = xcalloc(size, sizeof emptygidset[0]);
    emptygidset[0] = gid ? gid : getegid();

    itr = dico_list_iterator(retain_groups);
    for (gp = dico_iterator_first(itr); gp;
	 gp = dico_iterator_next(itr)) 
	emptygidset[j++] = (gid_t) gp;
    dico_iterator_destroy(&itr);
    
    /* Reset group permissions */
    if (geteuid() == 0 && setgroups(j, emptygidset)) {
	dico_log(L_ERR, errno, _("setgroups(1, %lu) failed"),
		 (unsigned long) emptygidset[0]);
	rc = 1;
    }
    free(emptygidset);
	
    /* Switch to the user's gid. On some OSes the effective gid must
       be reset first */

#if defined(HAVE_SETEGID)
    if ((rc = setegid(gid)) < 0)
	dico_log(L_ERR, errno, _("setegid(%lu) failed"),
		 (unsigned long) gid);
#elif defined(HAVE_SETREGID)
    if ((rc = setregid(gid, gid)) < 0)
	dico_log(L_ERR, errno, _("setregid(%lu,%lu) failed"),
		 (unsigned long) gid, (unsigned long) gid);
#elif defined(HAVE_SETRESGID)
    if ((rc = setresgid(gid, gid, gid)) < 0)
	dico_log(L_ERR, errno, _("setresgid(%lu,%lu,%lu) failed"),
		 (unsigned long) gid,
		 (unsigned long) gid,
		 (unsigned long) gid);
#endif

    if (rc == 0 && gid != 0) {
	if ((rc = setgid(gid)) < 0 && getegid() != gid) 
	    dico_log(L_ERR, errno, _("setgid(%lu) failed"),
		     (unsigned long) gid);
	if (rc == 0 && getegid() != gid) {
	    dico_log(L_ERR, errno, _("Cannot set effective gid to %lu"),
		     (unsigned long) gid);
	    rc = 1;
	}
    }

    /* Now reset uid */
    if (rc == 0 && uid != 0) {
	uid_t euid;
	
	if (setuid(uid)
	    || geteuid() != uid
	    || (getuid() != uid
		&& (geteuid() == 0 || getuid() == 0))) {
			
#if defined(HAVE_SETREUID)
	    if (geteuid() != uid) {
		if (setreuid(uid, -1) < 0) { 
		    dico_log(L_ERR, errno, _("setreuid(%lu,-1) failed"),
			     (unsigned long) uid);
		    rc = 1;
		}
		if (setuid(uid) < 0) {
		    dico_log(L_ERR, errno, _("second setuid(%lu) failed"),
			     (unsigned long) uid);
		    rc = 1;
		}
	    } else
#endif
		{
		    dico_log(L_ERR, errno, _("setuid(%lu) failed"),
			     (unsigned long) uid);
		    rc = 1;
		}
	}
	
	euid = geteuid();
	if (uid != 0 && setuid(0) == 0) {
	    dico_log(L_ERR, 0, _("seteuid(0) succeeded when it should not"));
	    rc = 1;
	} else if (uid != euid && setuid(euid) == 0) {
	    dico_log(L_ERR, 0, _("Cannot drop non-root setuid privileges"));
	    rc = 1;
	}
	
    }

    return rc;
}


