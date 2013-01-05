/* This file is part of GNU Dico
   Copyright (C) 2003-2004, 2007-2008, 2010, 2012 Sergey Poznyakoff
  
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
#include <dico.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* proto://[user[:password]@][host[:port]/]path[;arg=str[;arg=str...] 
   dict://[user[:password]@]host[:port]/{d|m}:[word]:[database]:[strat]:[n]
*/

static int
alloc_string_len(char **sptr, const char *start, size_t len)
{
    *sptr = malloc(len + 1);
    if (!*sptr)
	return 1;
    memcpy(*sptr, start, len);
    (*sptr)[len] = 0;
    return 0;
}

static int
alloc_string(char **sptr, const char *start, const char *end)
{
    size_t len = end ? end - start : strlen(start);
    return alloc_string_len(sptr, start, len);
}

static int
alloc_string_def(char **sptr, const char *start, const char *end,
		 const char *def)
{
    if (end == start)
	return alloc_string(sptr, def, NULL);
    else
	return alloc_string(sptr, start, end);
}

static int
url_parse_arg(dico_url_t url, char *p, char *q)
{
    char *s;
    char *key, *value = NULL;
    
    for (s = p; s < q && *s != '='; s++)
	;

    if (alloc_string(&key, p, s))
	return 1;
    if (s != q) {
	if (alloc_string(&value, s + 1, q))
	    return 1;
    }
    dico_assoc_append(url->args, key, value);
    free(key);
    free(value);
    return 0;
}

static int
url_get_args(dico_url_t url, char **str)
{
    int rc;
    char *p;

    if (!**str)
	return 0;

    url->args = dico_assoc_create(DICO_ASSOC_MULT);
    if (!url->args)
	return 1;
    for (p = *str, rc = 0; !rc;) {
	char *q = strchr (p, ';');
	if (q) {
	    rc = url_parse_arg(url, p, q);
	    p = q + 1;
	} else {
	    rc = url_parse_arg(url, p, p + strlen(p));
	    break;
	}
    }
    return rc;
}

static int
url_get_path(dico_url_t url, char **str)
{
    char *p;
    
    p = strchr(*str, ';');
    if (!p)
	p = *str + strlen(*str);
    if (alloc_string(&url->path, *str, p))
	return 1;
    *str = p;
    if (*p)
	++ * str;
    return url_get_args(url, str);

}

/* On input str points at the beginning of host part */
static int
url_get_host(dico_url_t url, char **str)
{
    char *s = *str;
    size_t len = strcspn(s, "/:");

    if (s[len] == ':') {
	char *q;
	unsigned long n = strtoul(s + len + 1, &q, 10);
	if ((*q && !strchr("/;:", *q)) || n > USHRT_MAX)
	    return 1;
	url->port = n;
	*str = q + strcspn(q, "/");
    } else
	*str = s + len;
    if (alloc_string_len(&url->host, s, len))
	return 1;
    if (**str) {
	++*str;
	return url_get_path(url, str);
    }
    return 0;
}

/* On input str points past the mech:// part */
static int
url_get_user (dico_url_t url, char **str)
{
    size_t len = strcspn(*str, ":;@/");
    char *p = *str + len;
    
    switch (*p) {
    case ';':
	if (strcmp(url->proto, "dict"))
	    break;
	/* else fall through */
    case ':':
	len = strcspn(p + 1, "@/:");
	if (p[len+1] == '@') {
	    if (alloc_string_len(&url->passwd, p + 1, len))
		return 1;
	    if (alloc_string (&url->user, *str, p))
		return 1;
	    *str = p + len + 2;
	}
	break;
	
    case '@':
	if (alloc_string(&url->user, *str, p))
	    return 1;
	url->passwd = NULL;
	*str = p + 1;
    }
    return url_get_host(url, str);
}

static int
url_get_proto(dico_url_t url, const char *str)
{
    char *p;
    
    if (!str) {
	errno = EINVAL;
	return 1;
    }
    
    p = strchr (str, ':');
    if (!p) {
	errno = EINVAL;
	return 1;
    }
    
    alloc_string(&url->proto, str, p);

    /* Skip slashes */
    for (p++; *p == '/'; p++)
	;
    return url_get_user(url, &p);
}

static int
url_parse_dico_request(dico_url_t url)
{
    char *p, *q;
    
    if (!url->path)
	return 0;
    p = url->path;
    if (p[1] != ':')
	return 1;
    switch (*p) {
    case 'm':
	url->req.type = DICO_REQUEST_MATCH;
	break;

    case 'd':
	url->req.type = DICO_REQUEST_DEFINE;
	break;

    default:
	return 1;
    }

    p += 2;
    q = strchr(p, ':');
    if (alloc_string(&url->req.word, p, q))
	return 1;
    if (!q) 
	return alloc_string_len(&url->req.database, "!", 1)
	        || alloc_string_len(&url->req.strategy, ".", 1);

    p = q + 1;
    q = strchr(p, ':');
    if (alloc_string_def(&url->req.database, p, q, "*"))
	return 1;

    if (url->req.type == DICO_REQUEST_MATCH) {
	if (!q)
	    return alloc_string_len(&url->req.strategy, ".", 1);
    
	p = q + 1;
	q = strchr(p, ':');
	if (alloc_string_def(&url->req.strategy, p, q, "."))
	    return 1;
    }
    
    if (q) {
	p = q + 1;
	url->req.n = strtoul(p, &q, 10);
	if (*q)
	    return 1;
    }
    
    return 0;
}

void
dico_url_destroy(dico_url_t *purl)
{
    dico_url_t url = *purl;

    free(url->string);
    free(url->proto);
    free(url->host);
    free(url->path);
    free(url->user);
    free(url->passwd);
    dico_assoc_destroy(&url->args);
    free(url->req.word);
    free(url->req.database);
    free(url->req.strategy);
    free(url);
    *purl = NULL;
}

int
dico_url_parse(dico_url_t *purl, const char *str)
{
    int rc;
    dico_url_t url;
    
    url = malloc(sizeof (*url));
    if (!url)
	return 1;
    memset(url, 0, sizeof(*url));
    rc = url_get_proto(url, str);
    if (rc)
	dico_url_destroy(&url);
    else {
	url->string = strdup(str);

	if (memcmp(url->proto, "dict", 4) == 0
	    && url_parse_dico_request(url)) {
	    dico_url_destroy(&url);
	    return 1;
	}
	
	*purl = url;
    }
    return rc;
}

char *
dico_url_full_path(dico_url_t url)
{
    char *path;
    size_t size = 1;

    if (url->host)
	size += strlen(url->host);
    if (url->path)
	size += strlen(url->path) + 1;
    path = malloc(size + 1);
    if (path) {
	if (url->host) {
	    strcpy(path, "/");
	    strcat(path, url->host);
	}
	if (url->path) {
	    if (path[0])
		strcat(path, "/");
	    strcat(path, url->path);
	}
    }
    return path;
}

const char *
dico_url_get_arg(dico_url_t url, const char *argname)
{
    return dico_assoc_find(url->args, argname);
}
