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
#include <sys/types.h>
#include <pwd.h>
#include <md5.h>
#include "getpass.h"

const char *xscript_prefix[] = { "S:", "C:" };

static int
parse_initial_reply(struct dict_connection *conn)
{
    char *p;
    size_t n = 0;
    size_t len;
    
    if (!dict_status_p(conn, "220"))
	return 1;
    p = strchr(conn->buf, '<');
    if (!p)
	return 1;
    p++;
    
    while ((len = strcspn(p, ".>"))) {
	char *s;
	if (conn->capac == n) {
	    if (n == 0)
		n = 2;
	    conn->capav = x2nrealloc(conn->capav, &n, sizeof(conn->capav[0]));
	}

	s = xmalloc(len+1);
	memcpy(s, p, len);
	s[len] = 0;
	conn->capav[conn->capac++] = s;
	p += len + 1;
	if (p[-1] == '>')
	    break;
    }

    p = strchr(p, '<');
    if (!p)
	return 1;
    len = strcspn(p, ">");
    if (p[len] != '>')
	return 1;
    len++;
    conn->msgid = xmalloc(len + 1);
    memcpy(conn->msgid, p, len);
    conn->msgid[len] = 0;
    
    return 0;
}

static int
apop_auth(struct dict_connection *conn, struct auth_cred *cred)
{
    int i;
    struct md5_ctx md5context;
    unsigned char md5digest[16];
    char buf[sizeof(md5digest) * 2 + 1];
    char *p;

    md5_init_ctx(&md5context);
    md5_process_bytes(conn->msgid, strlen(conn->msgid), &md5context);
    md5_process_bytes(cred->pass, strlen(cred->pass), &md5context);
    md5_finish_ctx(&md5context, md5digest);

    for (i = 0, p = buf; i < 16; i++, p += 2)
	sprintf(p, "%02x", md5digest[i]);
    *p = 0;
    stream_printf(conn->str, "AUTH %s %s\r\n", cred->user, buf);
    if (dict_read_reply(conn)) {
	dico_log(L_ERR, 0, _("No reply from server"));
	return 1;
    }
    return dict_status_p(conn, "230") == 0;
}

static int
dict_auth(struct dict_connection *conn, dico_url_t url)
{
    int rc = saslauth(conn, url);

    switch (rc) {
    case AUTH_OK:
	return 0;

    case AUTH_CONT:
	if (dict_capa(conn, "auth")) {
	    struct auth_cred cred;
	    
	    switch (auth_cred_get(url->host, &cred)) {
	    case GETCRED_OK:
		XDICO_DEBUG(1, _("Attempting APOP authentication\n"));
		rc = apop_auth(conn, &cred);
		auth_cred_free(&cred);
		return rc;

	    case GETCRED_FAIL:
		dico_log(L_WARN, 0,
			 _("Not enough credentials for authentication"));
		break;

	    case GETCRED_NOAUTH:
		XDICO_DEBUG(1, _("Skipping authentication\n"));
		break;
	    }
	}
	return 0;
	
    case AUTH_FAIL:
	break;
    }
    return 1;
}

char *
get_homedir()
{
    char *homedir = getenv("HOME");
    if (!homedir) {
	struct passwd *pw = getpwuid(geteuid());
	homedir = pw->pw_dir;
    }
    return homedir;
}

int
ds_tilde_expand(const char *str, char **output)
{
    char *dir;
    
    if (str[0] != '~')
	return 0;
    if (str[1] == '/') {
	dir = get_homedir();
	str += 2;
    } else {
	char *p;
	size_t len;
	char *name;
	struct passwd *pw;

	str++;
	p = strchr(str, '/');
	if (!p)
	    return 0;
	len = p - str;
	name = xmalloc(len + 1);
	memcpy(name, str, len);
	name[len] = 0;
	pw = getpwnam(name);
	free(name);
	if (pw) {
	    dir = pw->pw_dir;
	    str = p + 1;
	}
    }
    *output = dico_full_file_name(dir, str);
    return 1;
}
    
static void
auth_cred_dup(struct auth_cred *dst, const struct auth_cred *src)
{
    dst->user = src->user ? xstrdup(src->user) : NULL;
    dst->pass = src->pass ? xstrdup(src->pass) : NULL;
}    

void
auth_cred_free(struct auth_cred *cred)
{
    free(cred->user);
    free(cred->pass);
    dico_list_destroy(&cred->mech);
    free(cred->service);
    free(cred->realm);
    free(cred->hostname);
}

int
auth_cred_get(char *host, struct auth_cred *cred)
{
    memset(cred, 0, sizeof(cred[0]));
    auth_cred_dup(cred, &default_cred);
    if (default_cred.user && default_cred.pass) {
	XDICO_DEBUG(1,
		    _("Obtained authentication credentials from the command line\n"));
	return GETCRED_OK;
    } else {
	int flags = 0;
	
	if (autologin_file) {
	    if (access(autologin_file, F_OK))
		dico_log(L_WARN, 0, _("File %s does not exist"),
			 autologin_file);
	    else
		parse_autologin(autologin_file, host, cred, &flags);
	}
	
	if (!flags && DEFAULT_AUTOLOGIN_FILE) {
	    char *home = get_homedir();
	    char *filename = dico_full_file_name(home,
						 DEFAULT_AUTOLOGIN_FILE);
	    parse_autologin(filename, host, cred, &flags);
	    free(filename);
	}
	if (flags & AUTOLOGIN_NOAUTH)
	    return GETCRED_NOAUTH;
    }
    if (cred->user && !cred->pass) {
	char *p = getpass(_("Password:"));
	cred->pass = p ? xstrdup(p) : NULL;
    }
    return (cred->user && cred->pass) ? GETCRED_OK : GETCRED_FAIL;
}

void
dict_transcript(struct dict_connection *conn, int state)
{
    if (state == conn->transcript)
	return;
    if (state == 0) {
	dico_stream_t transport;
	if (dico_stream_ioctl(conn->str, DICO_IOCTL_GET_TRANSPORT,
			      &transport)) {
	    dico_log(L_CRIT, errno,
		     _("INTERNAL ERROR at %s:%d: cannot get stream transport"),
		     __FILE__,
		     __LINE__);
	    return;
	}
	
	if (dico_stream_ioctl(conn->str, DICO_IOCTL_SET_TRANSPORT, NULL)) {
	    dico_log(L_CRIT, errno,
		     _("INTERNAL ERROR at %s:%d: cannot set stream transport"),
		     __FILE__,
		     __LINE__);
	    return;
	}

	dico_stream_close(conn->str);
	dico_stream_destroy(&conn->str);
	conn->str = transport;
	conn->transcript = state;
    } else {
	dico_stream_t logstr = dico_log_stream_create(L_DEBUG);
	if (!logstr)
	    xalloc_die();
	conn->str = xdico_transcript_stream_create(conn->str, logstr,
						   xscript_prefix);
	conn->transcript = state;
    }
}

int
dict_connect(struct dict_connection **pconn, dico_url_t url)
{
    struct sockaddr_in s;
    int fd;
    IPADDR ip;
    dico_stream_t str;
    struct dict_connection *conn;
    
    XDICO_DEBUG_F2(1, _("Connecting to %s:%d\n"), url->host,
		   url->port ? url->port : DICO_DICT_PORT);
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
	dico_log(L_ERR, errno,
		 _("cannot create dict socket"));
	return 1;
    }

    s.sin_family = AF_INET;
    s.sin_addr.s_addr = htonl(source_addr);
    s.sin_port = 0;
    if (bind(fd, (struct sockaddr*) &s, sizeof(s)) < 0) {
	dico_log(L_ERR, errno,
		 _("cannot bind AUTH socket"));
    }

    ip = get_ipaddr(url->host);
    if (ip == 0) {
	dico_log(L_ERR, 0, _("%s: Invalid IP or unknown host name"),
		 url->host);
	return 1;
    }
    s.sin_addr.s_addr = htonl(ip);
    s.sin_port = htons(url->port ? url->port : DICO_DICT_PORT);
    if (connect(fd, (struct sockaddr*) &s, sizeof(s)) == -1) {
	dico_log(L_ERR, errno,
		 _("cannot connect to DICT server %s:%d"),
		 url->host, url->port ? url->port : DICO_DICT_PORT);
	close(fd);
	return 1;
    }

    if ((str = dico_fd_io_stream_create(fd, fd)) == NULL) {
	dico_log(L_ERR, errno,
		 _("cannot create dict stream: %s"),
		 strerror(errno));
	return 1;
    }

    conn = xzalloc(sizeof(*conn));
    conn->str = str;
    conn->fd = fd;
    dict_transcript(conn, transcript);
    if (dict_read_reply(conn)) {
	dico_log(L_ERR, 0, _("No reply from server"));
	return 1;
    }
    if (parse_initial_reply(conn)) {
	dico_log(L_ERR, 0, _("Invalid reply from server"));
	dict_conn_close(conn);
	return 1;
    }
	       
    XDICO_DEBUG(1, _("Sending client information\n"));
    stream_printf(conn->str, "CLIENT \"%s\"\r\n", client);
    dict_read_reply(conn);
    if (!dict_status_p(conn, "250")) 
	dico_log(L_WARN, 0,
		 _("Unexpected reply to CLIENT command: `%s'"),
		 conn->buf);

    obstack_init(&conn->stk);
    
    if (!noauth_option && dict_auth(conn, url)) {
	dico_log(L_ERR, 0, _("Authentication failed"));
	dict_conn_close(conn);
	return 1;
    }

    *pconn = conn;
    
    return 0;
}

int
dict_read_reply(struct dict_connection *conn)
{
    int rc;
    if (conn->buf)
	conn->buf[0] = 0;
    rc = dico_stream_getline(conn->str, &conn->buf, &conn->size,
			     &conn->level);
    if (rc == 0) 
	conn->level = dico_trim_nl(conn->buf);
    return rc;
}

int
dict_status_p(struct dict_connection *conn, const char *status)
{
    return conn->level > 3
	&& memcmp(conn->buf, status, 3) == 0
	&& (isspace(conn->buf[3])
	    || (conn->level == 5 && memcmp(conn->buf+3,"\r\n",2) == 0));
}

int
dict_capa(struct dict_connection *conn, char *capa)
{
    int i;
    
    for (i = 0; i < conn->capac; i++)
	if (strcmp(conn->capav[i], capa) == 0)
	    return 1;
    return 0;
}

int
dict_multiline_reply(struct dict_connection *conn)
{
    int rc;
    size_t nlines = 0;
    
    while ((rc = dict_read_reply(conn)) == 0) {
	char *ptr = conn->buf;
	size_t len = conn->level;
	if (*ptr == '.') {
	    if (ptr[1] == 0)
		break;
	    else if (ptr[1] == '.') {
		ptr++;
		len--;
	    }
	}
	obstack_grow(&conn->stk, ptr, len);
	obstack_1grow(&conn->stk, '\n');
	nlines++;
    }
    obstack_1grow(&conn->stk, 0);
    return rc;
}

int
dict_define(struct dict_connection *conn, char *database, char *word)
{
    int rc;

    XDICO_DEBUG_F2(1, _("Sending query for word \"%s\" in database \"%s\"\n"),
		   word, database);
    stream_printf(conn->str, "DEFINE \"%s\" \"%s\"\r\n",
		  quotearg_n (0, database),
		  quotearg_n (1, word));
    dict_read_reply(conn);
    if (dict_status_p(conn, "150")) {
	unsigned long i, count;
	char *p;
	
	count = strtoul (conn->buf + 3, &p, 10);
	XDICO_DEBUG_F1(1, ngettext("Reading %lu definition\n",
				   "Reading %lu definitions\n", count),
		       count);
	for (i = 0; i < count; i++) {
	    dict_read_reply(conn);
	    if (!dict_status_p(conn, "151")) {
		dico_log(L_WARN, 0,
			 _("Unexpected reply in place of definition %lu"), i);
		break;
	    }
	    obstack_grow(&conn->stk, conn->buf, conn->level);
	    obstack_1grow(&conn->stk, 0);
	    dict_multiline_reply(conn);
	}
	dict_read_reply(conn);
	dict_result_create(conn, dict_result_define, count,
			   obstack_finish(&conn->stk));
	rc = 0;
    } else
	rc = 1;
    return rc;
}

int
dict_match(struct dict_connection *conn, char *database, char *strategy,
	   char *word)
{
    int rc;
    if (levenshtein_threshold && conn->levdist != levenshtein_threshold
	&& dict_capa(conn, "xlev")) {
	XDICO_DEBUG(1, _("Setting Levenshtein threshold\n"));
	stream_printf(conn->str, "XLEV %u\n", levenshtein_threshold);
	dict_read_reply(conn);
	if (dict_status_p(conn, "250"))
	    conn->levdist = levenshtein_threshold;
	else {
	    dico_log(L_WARN, 0, _("Server rejected XLEV command"));
	    dico_log(L_WARN, 0, _("Server reply: %s"), conn->buf);
	}
    }
    XDICO_DEBUG_F3(1, _("Sending query to match word \"%s\" in "
			"database \"%s\", "
			"using \"%s\"\n"), word, database, strategy);
    stream_printf(conn->str, "MATCH \"%s\" \"%s\" \"%s\"\r\n",
		  quotearg_n (0, database),
		  quotearg_n (1, strategy),
		  quotearg_n (2, word));
    dict_read_reply(conn);
    if (dict_status_p(conn, "152")) {	
	unsigned long count;
	char *p;
	
	count = strtoul (conn->buf + 3, &p, 10);
	XDICO_DEBUG_F1(1, ngettext("Reading %lu match\n",
				   "Reading %lu matches\n", count),
		       count);
    
	dict_multiline_reply(conn);
	dict_result_create(conn, dict_result_match, count,
			   obstack_finish(&conn->stk));
	dict_read_reply(conn);
	rc = 0;
    } else
	rc = 1;
    return rc;
}

static size_t
count_lines(char *p)
{
    size_t count = 0;
    while ((p = strchr(p, '\n'))) {
	count++;
	p++;
    }
    return count;
}

static void
_result_parse_def(struct dict_result *res)
{
    char *p;
    size_t i;
    struct define_result *def = xcalloc(res->count, sizeof(*def));
    struct dico_tokbuf tb;

    dico_tokenize_begin(&tb);
    
    res->set.def = def;
    p = res->base;
    for (i = 0; i < res->count; i++, def++) {
	/* FIXME: Provide a destructive version of xdico_tokenize_string? */
	xdico_tokenize_string(&tb, p);
	def->word = xstrdup(tb.tb_tokv[1]);
	def->database = xstrdup(tb.tb_tokv[2]);
	def->descr = xstrdup(tb.tb_tokv[3]);
	p += strlen(p) + 1;
	def->defn = p;
	def->nlines = count_lines(p);
	p += strlen(p) + 1;
    }
    dico_tokenize_end(&tb);
}

static void
_result_free_def(struct dict_result *res)
{
    size_t i;
    struct define_result *def = res->set.def;
    
    for (i = 0; i < res->count; i++, def++) {
	free(def->word);
	free(def->database);
	free(def->descr);
    }
    free(res->set.def);
}
	
static void
_result_parse_mat(struct dict_result *res)
{
    char *p;
    size_t i;
    struct match_result *mat = xcalloc(res->count, sizeof(*mat));

    res->set.mat = mat;
    for (i = 0, p = strtok(res->base, "\n"); i < res->count;
	 p = strtok(NULL, "\n"), i++, mat++) {
	size_t len;

	if (!p) {
	    dico_log(L_NOTICE, 0, _("Not enough data in the result"));
	    res->count = i;
	    break;
	}
	
	mat->database = p;
	len = strcspn(p, " \t");
	p[len] = 0;
	p += len + 1;
	p += strspn(p, " \t");
	len = strlen(p);
	if (p[0] == '"' && p[len-1] == '"') {
	    p[len-1] = 0;
	    p++;
	}
	mat->word = p;
    }
}

static void
_result_free_mat(struct dict_result *res)
{
    free(res->set.mat);
}

struct dict_result *
dict_result_create(struct dict_connection *conn, enum dict_result_type type,
		   size_t count, char *base)
{
    struct dict_result *res = xmalloc(sizeof(*res));
    res->conn = conn;
    res->prev = conn->last_result;
    conn->last_result = res;
    res->type = type;
    res->count = count;
    res->base = base;
    switch (type) {
    case dict_result_define:
	_result_parse_def(res);
	break;

    case dict_result_match:
	_result_parse_mat(res);
	break;

    case dict_result_text:
	break;
    }
    return res;
}

void
dict_result_free(struct dict_result *res)
{
    if (!res)
	return;
    /* Detach res from the list and free obstack memory, if it was the
       last obtained result,  */
    if (res == res->conn->last_result) {
	obstack_free(&res->conn->stk, res->base);
	res->conn->last_result = res->prev;
    } else {
	struct dict_result *p;
	
	for (p = res->conn->last_result; p && p->prev != res; p = p->prev)
	    ;
	if (!p) {
	    dico_log(L_CRIT, 0, _("Freeing unlinked result"));
	    abort();
	}
	p->prev = res->prev;
    }
    /* Free allocated memory */
    switch (res->type) {
    case dict_result_define:
	_result_free_def(res);
	break;

    case dict_result_match:
	_result_free_mat(res);
	break;

    case dict_result_text:
	break;
    }
    free(res);
}

/* FIXME: Split into close/destroy */
void
dict_conn_close(struct dict_connection *conn)
{
    struct dict_result *res;

    dico_stream_close(conn->str);
    dico_stream_destroy(&conn->str);
    free(conn->msgid);
    free(conn->buf);
    dico_argcv_free(conn->capac, conn->capav);
    for (res = conn->last_result; res; ) {
	struct dict_result *prev = res->prev;
	dict_result_free(res);
	res = prev;
    }
    obstack_free(&conn->stk, NULL);
    free(conn);
}

