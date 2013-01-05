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

void
print_reply(struct dict_connection *conn)
{
    if (strncmp(conn->buf, "552", 3) == 0)
	printf("%s\n", _("No match"));
    else
	printf("Error: %s\n", conn->buf);
}

static size_t
utf8_count_newlines(char *str)
{
    size_t count = 0;
    struct utf8_iterator itr;
    for (utf8_iter_first(&itr, (char *)str);
	 !utf8_iter_end_p(&itr); utf8_iter_next(&itr)) {
	if (utf8_iter_isascii(itr) && *itr.curptr == '\n')
	    count++;
    }
    return count;
}

size_t
result_count_lines(struct dict_result *res)
{
    size_t i, count = 0;
    
    switch (res->type) {
    case dict_result_define:
	for (i = 0; i < res->count; i++)
	    count += res->set.def[i].nlines + 3;
	break;

    case dict_result_match:
	count = res->count;
	break;

    case dict_result_text:
	count = utf8_count_newlines(res->base);
    }
    return count;
}

static void
format_defn(dico_stream_t str, struct define_result *def)
{
    stream_printf(str, _("From %s, %s:\n"),
		  def->database, def->descr);
    stream_writez(str, def->defn);
    dico_stream_write(str, "\n", 1);
}

static void
format_match(dico_stream_t str, struct match_result *mat)
{
    stream_printf(str, "%s \"%s\"\n", mat->database, mat->word);
}

void
print_result(struct dict_result *res)
{
    unsigned long i;
    dico_stream_t str;
    
    str = create_pager_stream(result_count_lines(res));
    switch (res->type) {
    case dict_result_define:
	for (i = 0; i < res->count; i++)
	    format_defn(str, &res->set.def[i]);
	break;

    case dict_result_match:
	for (i = 0; i < res->count; i++)
	    format_match(str, &res->set.mat[i]);
	break;

    case dict_result_text:
	stream_writez(str, res->base);
	break;
    }
    dico_stream_close(str);
    dico_stream_destroy(&str);
}


struct result_display {
    struct result_display *next;
    char *database;
    int count;
    char **matches;
};

struct result_display *
alloc_display(struct dict_result *res, size_t from, size_t to)
{
    struct result_display *disp = xmalloc(sizeof(*disp));
    size_t i;
    
    disp->next = NULL;
    disp->database = res->set.mat[from].database;
    disp->count = to - from;
    disp->matches = xcalloc(to - from, sizeof(disp->matches[0]));
    for (i = 0; from < to; i++, from++)
	disp->matches[i] = res->set.mat[from].word;
    return disp;
}

static char *
find_descr(struct dict_connection *conn, const char *name)
{
    if (conn->db_result) {
	size_t i;
	
	for (i = 0; i < conn->db_result->count; i++) {
	    if (strcmp(conn->db_result->set.mat[i].database, name) == 0)
		return conn->db_result->set.mat[i].word;
	}
    }
    return _("(no description available)");
}

void
print_match_result(struct dict_result *res)
{
    struct result_display *head = NULL, *tail = NULL;
    char *dbname = NULL;
    size_t i, j = 0;
    struct dict_connection *conn = res->conn;
    size_t ndb = 0;
    dico_stream_t str;
    struct result_display *p;
    
#define ALLOC_DISPLAY() do { 					\
	struct result_display *p = alloc_display(res, j, i);    \
	if (!tail)                                              \
	    head = p;                                           \
	else               					\
	    tail->next = p;					\
	tail = p;						\
	ndb++;							\
	j = i;							\
	dbname = res->set.mat[i].database;			\
    } while (0)

    dbname = res->set.mat[0].database;
    for (i = 1; i < res->count; i++) {
	if (strcmp(res->set.mat[i].database, dbname))
	    ALLOC_DISPLAY();
    }
    ALLOC_DISPLAY();

    str = create_pager_stream(ndb + result_count_lines(res));
    j = 0;
    for (p = head; p; ) {
	struct result_display *next = p->next;
	
	stream_printf(str, _("From %s, %s:\n"), p->database,
		      find_descr(conn, p->database));
	for (i = 0; i < p->count; i++, j++)
	    stream_printf(str, "%4d) \"%s\"\n", j, p->matches[i]);
	free(p->matches);
	free(p);
	p = next;
    }
    dico_stream_close(str);
    dico_stream_destroy(&str);
}



int
dict_lookup(struct dict_connection *conn, dico_url_t url)
{
    int rc;
    switch (url->req.type) {
    case DICO_REQUEST_DEFINE:
	rc = dict_define(conn,
			 quotearg_n (0, url->req.database),
			 quotearg_n (1, url->req.word));
	break;
	
    case DICO_REQUEST_MATCH:
	rc = dict_match(conn,
			quotearg_n (0, url->req.database),
			quotearg_n (1, url->req.strategy),
			quotearg_n (2, url->req.word));
	break;
	
    default:
	dico_log(L_CRIT, 0,
		 _("%s:%d: INTERNAL ERROR: unexpected request type"),
		 __FILE__, __LINE__);
    }

    if (rc == 0) {
	struct dict_result *res = dict_conn_last_result(conn);
	print_result(res);
	dict_result_free(res);
    } else
	print_reply(conn);
    
    return 0;
}

int
dict_lookup_url(dico_url_t url)
{
    struct dict_connection *conn;
    
    if (!url->host) {
	dico_log(L_ERR, 0, _("Server name or IP not specified"));
	return 1;
    }

    if (dict_connect(&conn, url))
	return 1;

    dict_lookup(conn, url);

    XDICO_DEBUG(1, _("Quitting\n"));
    stream_printf(conn->str, "QUIT\r\n");
    dict_read_reply(conn);
    dict_conn_close(conn);
    /* FIXME */
    return 0;
}

int
dict_word(char *word)
{
    int rc;
    dico_url_t url;

    if (memcmp (word, "dict://", 7) == 0) {
	if (dico_url_parse(&url, word))
	    return 1;
	rc = dict_lookup_url(url);
	dico_url_destroy(&url);
    } else {
	dico_url.req.word = word;
	rc = dict_lookup_url(&dico_url);
    }
	
    return rc;
}

void
dict_run_single_command(struct dict_connection *conn,
			const char *cmd, const char *arg, const char *code)
{
    if (arg)
	stream_printf(conn->str, "%s \"%s\"\r\n", cmd, arg);
    else
	stream_printf(conn->str, "%s\r\n", cmd);
    
    dict_read_reply(conn);
    if (!dict_status_p(conn, code))
	print_reply(conn);
    else {
	struct dict_result *res;
	
	dict_multiline_reply(conn);
	dict_read_reply(conn);
	dict_result_create(conn, dict_result_text, 1,
			   obstack_finish(&conn->stk));
	res = dict_conn_last_result(conn);
	print_result(res);
	dict_result_free(res);
    }
}

int
dict_single_command(char *cmd, char *arg, char *code)
{
    struct dict_connection *conn;
    
    if (!dico_url.host) {
	dico_log(L_ERR, 0, _("Server name or IP not specified"));
	return 1;
    }

    if (dict_connect(&conn, &dico_url))
	return 1;

    dict_run_single_command(conn, cmd, arg, code);
    dict_conn_close(conn);    
    return 0;
}
