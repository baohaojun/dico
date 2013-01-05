/* This file is part of GNU Dico
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
#include <stdlib.h>
#include <string.h>
#include <netdb.h> 
#include <xdico.h>
#include <xalloc.h>
#include <xgethostname.h>
#include <xgetdomainname.h>

char *
xdico_local_hostname()
{
    struct hostent *hp;
    char *hostpart = xgethostname();
    char *ret;
	
    hp = gethostbyname(hostpart);
    if (hp) 
	ret = xstrdup(hp->h_name);
    else {
	char *domainpart = xgetdomainname();

	if (domainpart && domainpart[0] && strcmp(domainpart, "(none)")) {
	    ret = xmalloc(strlen(hostpart) + 1
			       + strlen(domainpart) + 1);
	    strcpy(ret, hostpart);
	    strcat(ret, ".");
	    strcat(ret, domainpart);
	    free(hostpart);
	} else
	    ret = hostpart;
	free(domainpart);
    }
    return ret;
}

