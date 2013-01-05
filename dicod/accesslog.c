/* This file is part of GNU Dico.
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

#include <dicod.h>
#include <fprintftime.h>
#include <xgethostname.h>

static char status[2][4];

void
access_log_status(const char *first, const char *last)
{
    memcpy(status[0], first, 3);
    status[0][3] = 0;
    if (last[0] == '.')
	last += 2;
    memcpy(status[1], last, 3);
    status[1][3] = 0;
}


/*
  access-log-format "%h %l %u %t \"%r\" %>s %b"

  %%          The percent sign
  %a          Remote IP-address
  %A          Local IP-address
  %B          Size of response in bytes.
  %b          Size of response in bytes in CLF format, i.e. a '-' rather
              than a 0 when no bytes are sent.
  %C          *Remote client (from CLIENT command).
  %d          *Abbreviated request command verb ('d' or 'm').
  %D          The time taken to serve the request, in microseconds.
  %h          Remote host
  %H          *Request command verb (DEFINE or MATCH)
  %l          Remote logname (from identd, if supplied). This will return a
              dash unless identity-check is set to true.
  %m          *The search strategy
  %p          The canonical port of the server serving the request
  %P          The process ID of the child that serviced the request.
  %q          *The database from the request.
  %r          Full request
  %{N}R       *Nth token from the request. N is 0-based.
  %s          Status. %>s is also accepted.
  %t          Time the request was received (standard english format)
  %{format}t  The time, in the form given by format, which should be in
              strftime(3) format. (potentially localized)
  %T          The time taken to serve the request, in seconds.
  %u          Remote user from AUTH.
  %v          The host name of the server serving the request.
  %V          *Actual host name of the server (in case it was overridden
              in conffile).
  %I          Bytes received, including request and headers. Cannot be zero.
  %O          Bytes sent, including headers. Cannot be zero. 
  %W          *The word from the request
*/
char *access_log_format = "%h %l %u %t \"%r\" %>s %b";
char *access_log_file;

struct alog_instr;

typedef void (*alog_printer_fn) (FILE *fp, struct alog_instr *instr,
				 int argc, char **argv);

struct alog_instr {
    alog_printer_fn prt;
    char *arg;
    char *cache;
};

dico_list_t access_log_prog;

struct alog_tab {
    int ch;
    alog_printer_fn prt;
    int allow_fmt;
};

static void
add_instr(alog_printer_fn prt, const char *fmt, size_t fmtsize)
{
    struct alog_instr *p;
    if (fmt == NULL)
	fmtsize = 0;
    p = xzalloc(sizeof(*p) + (fmtsize ? (fmtsize + 1) : 0));
    p->prt = prt;
    if (fmtsize) {
	p->arg = (char*) (p + 1);
	memcpy(p->arg, fmt, fmtsize);
	p->arg[fmtsize] = 0;
    } else
	p->arg = NULL;
    xdico_list_append(access_log_prog, p);
}
    
static void
print_str(FILE *fp, const char *arg)
{
    size_t len;
    if (!arg) {
	arg = "-";
	len = 1;
    } else
	len = strlen(arg);
    fwrite(arg, len, 1, fp); 
}

static void
alog_print(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, instr->arg);
}

#define _S_UN_NAME(sa, salen) \
    ((salen < offsetof (struct sockaddr_un,sun_path)) ? "" : (sa)->sun_path)

static char *
sockaddr_to_hostname(struct sockaddr *sa, int resolve)
{
    struct sockaddr_in *s_in;
    char *ret;
    
    switch (sa->sa_family) {
    case AF_INET:
	s_in = (struct sockaddr_in*)sa;
	if (resolve) {
	    struct hostent *hp;
	    hp = gethostbyaddr((char*) &s_in->sin_addr,
			       sizeof(s_in->sin_addr),
			       AF_INET);
	    if (hp)
		return xstrdup(hp->h_name);
	}
	ret = xstrdup(inet_ntoa(s_in->sin_addr));
	break;
	
    case AF_UNIX:
	ret = xstrdup("localhost");
	break;
		
    default:
	ret = xstrdup("{unsupported family}");
    }

    return ret;
}

static char *
sockaddr_to_portname(struct sockaddr *sa, int salen)
{
    struct sockaddr_in *s_in;
    struct sockaddr_un *s_un;
    char buf[UINTMAX_STRSIZE_BOUND];
    char *ret;
    
    switch (sa->sa_family) {
    case AF_UNIX:
	s_un = (struct sockaddr_un*)sa;
	if (_S_UN_NAME(s_un, salen)[0] == 0)
	    ret = xstrdup("{AF_UNIX}");
	else
	    ret = xstrdup(s_un->sun_path);
	break;

    case AF_INET:
	s_in = (struct sockaddr_in*)sa;
	ret = xstrdup(umaxtostr(ntohs(s_in->sin_port), buf));
	break;

    default:
	ret = xstrdup("{unsupported family}");
    }
    return ret;
}
	
static void
alog_remote_ip(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    if (!instr->cache) {
	if (client_addrlen == 0) 
	    instr->cache = xstrdup("stdin");
	else 
	    instr->cache = sockaddr_to_hostname(&client_addr, 0);
    }
    print_str(fp, instr->cache);
}

static void
alog_local_ip(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    if (!instr->cache) {
	if (server_addrlen == 0) 
	    instr->cache = xstrdup("stdin");
	else 
	    instr->cache = sockaddr_to_hostname(&server_addr, 0);
    }
    print_str(fp, instr->cache);
}

static void
alog_response_size(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    char buf[UINTMAX_STRSIZE_BOUND];
    print_str(fp, umaxtostr(total_bytes_out, buf));
}

static void
alog_response_size_clf(FILE *fp, struct alog_instr *instr,
		       int argc, char **argv)
{
    char buf[UINTMAX_STRSIZE_BOUND];
    if (total_bytes_out)
	print_str(fp, umaxtostr(total_bytes_out, buf));
    else
	print_str(fp, "-");
}

static void
alog_client(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, client_id);
}

static void
alog_time_ms(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    char buf[UINTMAX_STRSIZE_BOUND];
    xdico_timer_t t = timer_get(argv[0]);
    double s = timer_get_real(t);
    print_str(fp, umaxtostr((uintmax_t)(s * 1e6), buf));
}

static void
alog_remote_host(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    if (!instr->cache) {
	if (client_addrlen == 0) 
	    instr->cache = xstrdup("stdin");
	else
	    instr->cache = sockaddr_to_hostname(&client_addr, 1);
    }
    print_str(fp, instr->cache);
}

static void
alog_command_verb(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, argv[0]);
}

static void
alog_command_verb_abbr(FILE *fp, struct alog_instr *instr,
		       int argc, char **argv)
{
    print_str(fp,
	      strcasecmp(argv[0], "DEFINE") == 0 ? "d" :
	      strcasecmp(argv[0], "MATCH") == 0 ? "m" : "?");
}

static void
alog_logname(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, identity_name);
}

static void
alog_strategy(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    if (argc == 4) /* A match request */
	print_str(fp, argv[2]);
    else
	print_str(fp, "-");
}

static void
alog_server_port(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    if (server_addrlen == 0)
	print_str(fp, "-");
    else if (!instr->cache)
	instr->cache = sockaddr_to_portname(&server_addr, server_addrlen);
    print_str(fp, instr->cache);
}

static void
alog_pid(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    fprintf(fp, "%lu", (unsigned long) getpid());
}

static void
alog_database(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, argv[1]);    
}

static void
alog_request(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    char *str;
    if (dico_argcv_string(argc, (const char**)argv, &str))
	print_str(fp, "-");
    else {
	print_str(fp, str);
	free(str);
    }
}

static void
alog_token(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    unsigned n = instr->arg ? atoi(instr->arg) : 0;
    if (n > argc)
	n = 0;
    print_str(fp, argv[n]);
}

static void
alog_status(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    unsigned n = instr->arg ? atoi(instr->arg) : 0;
    if (n > argc)
	n = 0;
    print_str(fp, status[n]);
}

static void
alog_time(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    struct tm tm;
    time_t t = time(NULL);
    localtime_r(&t, &tm);
    fprintftime(fp, instr->arg ? instr->arg : "[%d/%b/%Y:%H:%M:%S %z]",
		&tm, 0, 0);    
}

static void
alog_process_time(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    char buf[UINTMAX_STRSIZE_BOUND];
    xdico_timer_t t = timer_get(argv[0]);
    print_str(fp, umaxtostr((uintmax_t)timer_get_real(t), buf));
}

static void
alog_remote_user(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, user_name);
}

static void
alog_conf_hostname(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, hostname);    
}

static void
alog_hostname(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    if (!instr->cache) {
	if (server_addrlen == 0) 
	    instr->cache = xstrdup("stdin");
	else 
	    instr->cache = sockaddr_to_hostname(&server_addr, 0);
    }
    print_str(fp, instr->cache);
}

static void
alog_word(FILE *fp, struct alog_instr *instr, int argc, char **argv)
{
    print_str(fp, argv[argc-1]);
}

static struct alog_tab alog_tab[] = {
    /* The percent sign */
    { '%', alog_print, 0 },
    /* Remote IP-address */
    { 'a', alog_remote_ip },
    /* Local IP-address */
    { 'A', alog_local_ip },
    /* Size of response in bytes. */
    { 'B', alog_response_size },
    /* Size of response in bytes in CLF format, i.e. a '-' rather
       than a 0 when no bytes are sent. */
    { 'b', alog_response_size_clf },
    /* * Remote client (from CLIENT command). */
    { 'C', alog_client },
    /* * Abbreviated request command verb ('d' or 'm'). */
    { 'd', alog_command_verb_abbr },
    /* The time taken to serve the request, in microseconds. */
    { 'D', alog_time_ms },
    /* Remote host */
    { 'h', alog_remote_host },
    /* * Request command verb (DEFINE or MATCH) */
    { 'H', alog_command_verb },
    /* Remote logname (from identd, if supplied). This will return a
       dash unless identity-check is set to true. */
    { 'l', alog_logname },
    /* * The search strategy */
    { 'm', alog_strategy },
    /* The canonical port of the server serving the request */
    { 'p', alog_server_port },
    /* The process ID of the child that serviced the request. */
    { 'P', alog_pid },
    /* * The database from the request. */
    { 'q', alog_database },
    /* Full request */
    { 'r', alog_request },
    /* * Nth token from the request. N is 0-based. */
    { 'R', alog_token, 1 },
    /* Status. %>s is also accepted. */
    { 's', alog_status, 1 },
    /* %t          Time the request was received (standard english format)
       %{format}t  The time, in the form given by format, which should be in
                   strftime(3) format. (potentially localized) */
    { 't', alog_time, 1 },
    /* The time taken to serve the request, in seconds. */
    { 'T', alog_process_time },
    /* Remote user from AUTH. */
    { 'u', alog_remote_user },
    /*  The configured host name of the server serving the request. */
    { 'v', alog_conf_hostname },
    /*  The host name of the server serving the request. */
    { 'V', alog_hostname },
#if 0
    /* FIXME: */
    { 'I', alog_inbytes },
    { 'O', alog_outbytes },
#endif
    /* *The word from the request */
    { 'W', alog_word },
    { 0 }
};

static struct alog_tab *
find_alog_entry(int c)
{
    struct alog_tab *p;

    for (p = alog_tab; p->ch; p++)
	if (p->ch == c)
	    return p;
    return NULL;
}

void
compile_access_log()
{
    char *p;
    const char *fmt = access_log_format;
    size_t len;
    
    access_log_prog = xdico_list_create();
    
    while ((p = strchr(fmt, '%'))) {
	char *arg = NULL;
	size_t arglen;
	struct alog_tab *tptr;
	
	len = p - fmt;
	if (len)
	    add_instr(alog_print, fmt, len);
	p++;
	if (*p == '>' && p[1] == 's') {
	    arg = "1";
	    arglen = 1;
	    p++;
	} else if (*p == '{') {
	    char *q = strchr(p + 1, '}');
	    
	    if (!q) {
		dico_log(L_ERR, 0,
			 _("log format error (near char %d): "
                           "missing terminating `}'"),
			 p - access_log_format);
		add_instr(alog_print, p - 1, 2);
		fmt = p + 1;
		continue;
	    } 
	    arglen = q - p - 1;
	    arg = p + 1;
	    p = q + 1;
	}

	tptr = find_alog_entry(*p);
	if (!tptr) {
	    dico_log(L_ERR, 0,
		     _("log format error (near char %d): "
		       "unknown format char `%c'"),
		     p - access_log_format,
		     *p);
	    add_instr(alog_print, fmt, p - fmt + 1);
	} else {
	    if (arg && !tptr->allow_fmt) {
		dico_log(L_ERR, 0,
			 _("log format warning (near char %d): "
			   "format char `%c' does not "
			   "take arguments"),
			 p - access_log_format,
			 *p);
		arg = NULL;
	    }
	    if (tptr->ch == '%') {
		/* Special case */
		arg = "%";
		arglen = 1;
	    }
	    add_instr(tptr->prt, arg, arglen);
	}
	fmt = p + 1;
    }
    len = strlen(fmt);
    if (len)
	add_instr(alog_print, fmt, len);
}

void
format_access_log(FILE *fp, int argc, char **argv)
{
    dico_iterator_t itr = xdico_list_iterator(access_log_prog);
    struct alog_instr *p;

    for (p = dico_iterator_first(itr); p;
	 p = dico_iterator_next(itr)) 
	p->prt(fp, p, argc, argv);
    fputc('\n', fp);
    dico_iterator_destroy(&itr);
}

void
access_log(int argc, char **argv)
{
    FILE *fp;

    if (!access_log_file)
	return;
    fp = fopen(access_log_file, "a");
    if (!fp) {
	dico_log(L_ERR, errno, _("cannot open access log file `%s'"),
		 access_log_file);
	return;
    }
    format_access_log(fp, argc, argv);
    fclose(fp);
}

static int
free_cache(void *item, void *data)
{
    struct alog_instr *p = item;
    if (p->cache) {
	free(p->cache);
	p->cache = NULL;
    }
    return 0;
}

void
access_log_free_cache()
{
    dico_list_iterate(access_log_prog, free_cache, NULL);
}
