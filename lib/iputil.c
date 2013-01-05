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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <intprops.h>
#include <inttostr.h>

#include <xdico.h>
#include <xalloc.h>

/*
 *	Return an IP address in host long notation from a host
 *	name or address in dot notation.
 */
IPADDR
get_ipaddr(char *host)
{
    struct hostent *hp;
    struct in_addr addr;

    if (inet_aton(host, &addr))
	return ntohl(addr.s_addr);
    else if ((hp = gethostbyname(host)) == NULL)
	return 0;
    else
	return ntohl(*(UINT4 *) hp->h_addr);
}

/*
 *	Return a printable host name (or IP address in dot notation)
 *	for the supplied IP address.
 */
char *
ip_hostname(IPADDR ipaddr)
{
    struct hostent *hp;
    UINT4 n_ipaddr;

    n_ipaddr = htonl(ipaddr);
    hp = gethostbyaddr((char *) &n_ipaddr, sizeof(struct in_addr),
		       AF_INET);
    if (!hp) {
	struct in_addr in;
	in.s_addr = htonl(ipaddr);
	return inet_ntoa(in);
    }
    return (char *) hp->h_name;
}

IPADDR
getmyip()
{
    char myname[256];

    gethostname(myname, sizeof(myname));
    return get_ipaddr(myname);
}

int
str2port(char *str)
{
    struct servent *serv;
    char *p;
    int port;

    /* First try to read it from /etc/services */
    serv = getservbyname(str, "tcp");

    if (serv != NULL)
	port = ntohs(serv->s_port);
    else {
	long l;
	/* Not in services, maybe a number? */
	l = strtol(str, &p, 0);

	if (*p || l < 0 || l > USHRT_MAX)
	    return -1;

	port = l;
    }

    return port;
}

static size_t
mu_stpcpy (char **pbuf, size_t *psize, const char *src)
{
    size_t slen = strlen (src);
    if (pbuf == NULL || *pbuf == NULL)
	return slen;
    else {
	char *buf = *pbuf;
	size_t size = *psize;
	if (size > slen)
	    size = slen;
	memcpy (buf, src, size);
	*psize -= size;
	*pbuf += size;
	if (*psize)
	    **pbuf = 0;
	else
	    (*pbuf)[-1] = 0;
	return size;
    }
}

#define S_UN_NAME(sa, salen) \
  ((salen < offsetof (struct sockaddr_un,sun_path)) ? "" : (sa)->sun_path)

void
sockaddr_to_str (const struct sockaddr *sa, int salen,
		 char *bufptr, size_t buflen,
		 size_t *plen)
{
    char buf[INT_BUFSIZE_BOUND (uintmax_t)]; /* FIXME: too much */
    size_t len = 0;
    switch (sa->sa_family) {
    case AF_INET:
    {
	struct sockaddr_in s_in = *(struct sockaddr_in *)sa;
	len += mu_stpcpy (&bufptr, &buflen, inet_ntoa(s_in.sin_addr));
	len += mu_stpcpy (&bufptr, &buflen, ":");
	len += mu_stpcpy (&bufptr, &buflen, umaxtostr(ntohs (s_in.sin_port),
						      buf));
	break;
    }

    case AF_UNIX:
    {
	struct sockaddr_un *s_un = (struct sockaddr_un *)sa;
	if (S_UN_NAME(s_un, salen)[0] == 0)
	    len += mu_stpcpy (&bufptr, &buflen, "anonymous socket");
	else {
	    len += mu_stpcpy (&bufptr, &buflen, "socket ");
	    len += mu_stpcpy (&bufptr, &buflen, s_un->sun_path);
	}
	break;
    }

    default:
	len += mu_stpcpy (&bufptr, &buflen, "{Unsupported family: ");
	len += mu_stpcpy (&bufptr, &buflen, umaxtostr (sa->sa_family, buf));
	len += mu_stpcpy (&bufptr, &buflen, "}");
    }
    if (plen)
	*plen = len + 1;
}

char *
sockaddr_to_astr (const struct sockaddr *sa, int salen)
{
    size_t size;
    char *p;
    
    sockaddr_to_str(sa, salen, NULL, 0, &size);
    p = xmalloc(size);
    sockaddr_to_str(sa, salen, p, size, NULL);
    return p;
}
