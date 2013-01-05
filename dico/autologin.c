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

#include "dico-priv.h"

char *
skipws(char *buf)
{
    while (*buf && isascii(*buf) && isspace(*buf))
	buf++;
    return buf;
}

/* Compare two hostnames. Return 0 if they have the same address type,
   address length *and* at least one of the addresses of A matches
   B */
int
hostcmp(const char *a, const char *b)
{
    struct hostent *hp = gethostbyname(a);
    char **addrlist;
    char *dptr;
    char **addr;
    size_t i, count;
    size_t entry_length;
    int entry_type;

    if (!hp)
	return 1;

    for (count = 1, addr = hp->h_addr_list; *addr; addr++)
	count++;
    addrlist = xmalloc (count * (sizeof *addrlist + hp->h_length)
			- hp->h_length);
    dptr = (char *) (addrlist + count);
    for (i = 0; i < count - 1; i++) {
	memcpy(dptr, hp->h_addr_list[i], hp->h_length);
	addrlist[i] = dptr;
	dptr += hp->h_length;
    }
    addrlist[i] = NULL;
    entry_length = hp->h_length;
    entry_type = hp->h_addrtype;

    hp = gethostbyname(b);
    if (!hp || entry_length != hp->h_length || entry_type != hp->h_addrtype) {
	free(addrlist);
	return 1;
    }

    for (addr = addrlist; *addr; addr++) {
	char **p;

	for (p = hp->h_addr_list; *p; p++) {
	    if (memcmp(*addr, *p, entry_length) == 0) {
		free(addrlist);
		return 0;
	    }
	}
    }
    free(addrlist);
    return 1;
}

static int
begins_with(const char *str, const char *prefix)
{
    size_t len = strlen (prefix);

    return strlen (str) >= len && strncmp (str, prefix, len) == 0
	   && (str[len] == 0 || str[len] == ' ' || str[len] == '\t');
}

enum kw_tok {
    kw_login,
    kw_password,
    kw_noauth,
    kw_nosasl,
    kw_sasl,
    kw_mechanism,
    kw_realm,
    kw_service,
    kw_host
};
    
struct keyword {
    char *name;
    int arg;
    enum kw_tok tok;
};

static struct keyword kwtab[] = {
    { "login", 1, kw_login },
    { "password", 1, kw_password },
    { "noauth", 0, kw_noauth },
    { "nosasl", 0, kw_nosasl },
    { "sasl", 0, kw_sasl },
    { "mechanism", 1, kw_mechanism },
    { "realm", 1, kw_realm },
    { "service", 1, kw_service },
    { "host", 1, kw_host },
    { NULL }
};

static struct keyword *
findkw(const char *name)
{
    struct keyword *p;
    for (p = kwtab; p->name; p++)
	if (strcmp(p->name, name) == 0)
	    return p;
    return NULL;
}

static int
_cred_free(void *item, void *data)
{
    free(item);
    return 0;
}

struct matches {
    const char *host;
    int def_line;
    int def_argc;
    char **def_argv;
    int host_argc;
    char **host_argv;
};

static int
match_line(struct wordsplit *ws, struct matches *matches, int line)
{
    int rc = 0;
    
    if (strcmp(ws->ws_wordv[0], "machine") == 0) {
	if (hostcmp(ws->ws_wordv[1], matches->host) == 0) {
	    XDICO_DEBUG_F1(1, _("Found matching line %d\n"), line);
	    if (matches->host_argv)
		dico_argcv_free(matches->host_argc, matches->host_argv);
	    matches->host_argc = ws->ws_wordc;
	    matches->host_argv = ws->ws_wordv;
	    matches->def_line = line;
	    rc = 1;
	}
    } else if (strcmp(ws->ws_wordv[0], "default") == 0) {
	XDICO_DEBUG_F1(1, _("Found default line %d\n"), line);
	if (matches->def_argv)
	    dico_argcv_free(matches->def_argc, matches->def_argv);
	matches->def_argc = ws->ws_wordc;
	matches->def_argv = ws->ws_wordv;
	matches->def_line = line;
    }
    ws->ws_wordc = 0;
    ws->ws_wordv = NULL;
    return rc;
}


/* Parse netrc-like autologin file and set up user and key accordingly. */
int
parse_autologin(const char *filename, char *host, struct auth_cred *pcred,
		int *pflags)
{
    FILE *fp;
    char *buf = NULL;
    size_t n = 0;
    struct matches matches;
    char **p_argv = NULL;
    int line = 0;
    int flags = 0;
    struct wordsplit ws;
    int wsflags = 0;
    int beg_line = 0;
    
    fp = fopen (filename, "r");
    if (!fp) {
	if (errno != ENOENT) {
	    dico_log(L_ERR, errno, _("Cannot open autologin file %s"),
		     filename);
	}
	return 1;
    } else
	XDICO_DEBUG_F1(1, _("Reading autologin file %s...\n"), filename);

    memset(&matches, 0, sizeof matches);
    matches.host = host;
    while (getline (&buf, &n, fp) > 0 && n > 0) {
	char *p;

	line++;
	p = skipws(buf);

	if ((wsflags & WRDSF_APPEND) &&
	    (begins_with(p, "machine") || begins_with(p, "default"))) {
	    wsflags &= ~WRDSF_APPEND;
	    if (match_line(&ws, &matches, beg_line))
		break;
	    beg_line = line;
	} else if (!(wsflags & WRDSF_REUSE))
	    beg_line = line;

	ws.ws_comment = "#";
	if (wordsplit(p, &ws, WRDSF_DEFFLAGS|WRDSF_COMMENT|wsflags)) {
	    dico_log(L_ERR, 0, _("failed to parse command `%s': %s"),
		     p, wordsplit_strerror(&ws));
	    fclose(fp);
	    free(buf);
	    if (wsflags & WRDSF_REUSE)
		wordsplit_free(&ws);
	    return 1;
	}
	wsflags |= WRDSF_REUSE | WRDSF_APPEND;
	/* Add newline marker */
	if (wordsplit("#", &ws, WRDSF_DEFFLAGS|WRDSF_NOSPLIT|wsflags)) {
	    dico_log(L_ERR, 0, _("failed to add line marker: %s"),
		     wordsplit_strerror(&ws));
	    fclose(fp);
	    free(buf);
	    if (wsflags & WRDSF_REUSE)
		wordsplit_free(&ws);
	    return 1;
	}
    }
    if (wsflags & WRDSF_APPEND)
	match_line(&ws, &matches, line);
    if (wsflags & WRDSF_REUSE)
	wordsplit_free(&ws);

    fclose(fp);
    free(buf);

    if (matches.host_argv) 
	p_argv = matches.host_argv + 2;
    else if (matches.def_argv) 
	p_argv = matches.def_argv + 1;
    else {
	XDICO_DEBUG(1, _("No matching line found\n"));
	p_argv = NULL;
    }

    if (p_argv) {
	line = matches.def_line;

	pcred->sasl = sasl_enabled_p();
	while (*p_argv) {
	    if (**p_argv == '#') {
		line++;
		p_argv++;
	    } else {
		struct keyword *kw = findkw(*p_argv);
		char *arg;
		
		if (!kw) {
		    dico_log(L_ERR, 0,
			     _("%s:%d: unknown keyword"), filename, line);
		    p_argv++;
		    continue;
		}

		if (kw->arg) {
		    if (!p_argv[1]) {
			dico_log(L_ERR, 0,
				 _("%s:%d: %s without argument"),
				 filename, line, p_argv[0]);
			break;
		    }
		    arg = p_argv[1];
		    p_argv += 2;
		} else
		    p_argv++;
		
		switch (kw->tok) {
		case kw_login:
		    pcred->user = xstrdup(arg);
		    flags |= AUTOLOGIN_USERNAME;
		    break;

		case kw_password:
		    pcred->pass = xstrdup(arg);
		    flags |= AUTOLOGIN_PASSWORD;
		    break;

		case kw_service:
		    pcred->service = xstrdup(arg);
		    break;

		case kw_realm:
		    pcred->realm = xstrdup(arg);
		    break;

		case kw_host:
		    pcred->hostname = xstrdup(arg);
		    break;
		    
		case kw_noauth:
		    flags |= AUTOLOGIN_NOAUTH;
		    break;
		    
		case kw_nosasl:
		    pcred->sasl = 0;
		    break;

		case kw_sasl:
		    pcred->sasl = 1;
		    break;
		    
		case kw_mechanism: {
		    int i;
		    struct wordsplit mechws;
		    
		    if (!(flags & AUTOLOGIN_MECH)) {
			pcred->mech = xdico_list_create();
			dico_list_set_free_item(pcred->mech, _cred_free, NULL);
			flags |= AUTOLOGIN_MECH;
		    }
		    mechws.ws_delim = ",";
		    if (wordsplit(arg, &mechws,
				  WRDSF_NOVAR | WRDSF_NOCMD | WRDSF_DELIM |
				  WRDSF_WS)) {
			dico_log(L_ERR, 0,
				 _("%s:%d: failed to parse line: %s"),
				 filename, line, wordsplit_strerror (&mechws));
			exit(1);
		    }
		    
		    for (i = 0; i < mechws.ws_wordc; i++) 
			xdico_list_append(pcred->mech, mechws.ws_wordv[i]);
		    
		    mechws.ws_wordc = 0;
		    wordsplit_free(&mechws);
		    break;
		  }
		}
	    }
	}
    }
    dico_argcv_free(matches.def_argc, matches.def_argv);
    dico_argcv_free(matches.host_argc, matches.host_argv);

    if (pflags)
	*pflags = flags;
    return 0;
}
