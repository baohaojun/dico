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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <dico.h>
#include <xdico.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include "xalloc.h"
#include "md5.h"

dico_stream_t iostr;    
char *in_buffer;
size_t in_bufsize;
size_t in_level;
char *msgid;

int
dict_status_p(const char *status)
{
    return in_level > 3
	&& memcmp(in_buffer, status, 3) == 0
	&& (in_buffer[3] == ' '
	    || (in_level == 5 && memcmp(in_buffer+3,"\r\n",2) == 0));
}

void
assert_status(const char *status)
{
    if (!dict_status_p(status)) {
	dico_log(L_ERR, 0, "expected status %s but got %s",
		 status, in_buffer);
	exit(3);
    }
}

static void
dict_read_reply()
{
    int rc;
    if (in_buffer)
	in_buffer[0] = 0;
    rc = dico_stream_getline(iostr, &in_buffer, &in_bufsize, &in_level);
    if (rc == 0)
	in_level = dico_trim_nl(in_buffer);
    else {
	rc = dico_stream_last_error(iostr);
	if (rc == EIO && in_level == 0)
	    exit(0);
	dico_log(L_ERR, 0, "error reading input: %s",
		 dico_stream_strerror(iostr, rc));
	exit(2);
    }
}

static void
get_msgid()
{
    if (in_buffer[in_level-1] == '>') {
	char *p = strrchr(in_buffer, '<');
	if (p) {
	    msgid = xstrdup(p);
	    return;
	}
    }
    dico_log(L_ERR, 0, "server does not support authentication");
    dico_log(L_ERR, 0, "last reply was: %s", in_buffer);
}

static void
apop_auth(const char *user, const char *pass)
{
    int i;
    struct md5_ctx md5context;
    unsigned char md5digest[16];
    char buf[sizeof(md5digest) * 2 + 1];
    char *p;

    md5_init_ctx(&md5context);
    md5_process_bytes(msgid, strlen(msgid), &md5context);
    md5_process_bytes(pass, strlen(pass), &md5context);
    md5_finish_ctx(&md5context, md5digest);

    for (i = 0, p = buf; i < 16; i++, p += 2)
	sprintf(p, "%02x", md5digest[i]);
    *p = 0;
    stream_printf(iostr, "AUTH %s %s\r\n", user, buf);
    dict_read_reply();
    assert_status("230");
}

dico_stream_t
run_command(int argc, char **argv, pid_t *ppid)
{
    pid_t pid;
    int oup[2], inp[2];

    assert(pipe(oup) == 0);
    assert(pipe(inp) == 0);
    pid = fork();
    if (pid == (pid_t) -1) {
	dico_log(L_ERR, errno, "fork");
	exit(2);
    }

    if (pid == 0) {
	assert(dup2(oup[0], 0) >= 0);
	close(oup[1]);
	assert(dup2(inp[1], 1) >= 0);
	close(inp[0]);
	execvp(argv[0], argv);
	dico_log(L_ERR, errno, "failed to run command");
	exit(127);
    }

    /* Master */
    close(oup[0]);
    close(inp[1]);
    *ppid = pid;
    
    return dico_fd_io_stream_create(inp[0], oup[1]);
}

void
printer()
{
    for (;;) {
	dict_read_reply();
	printf("%s\n", in_buffer);
    }
}

void
writer(FILE *fp)
{
    pid_t pid = fork();
    if (pid == (pid_t) -1) {
	dico_log(L_ERR, errno, "fork");
	exit(2);
    }
    if (pid == 0) {
	char buf[512];
	size_t n;
    
	while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
	    dico_stream_write(iostr, buf, n);
	exit(0);
    }
    fclose(fp);
}

int
main(int argc, char **argv)
{
    char *user, *pass;
    pid_t pid;
    char *script = NULL;
    
    dico_set_program_name(argv[0]);

    while (--argc) {
	char *arg = *++argv;
	if (strncmp(arg, "-script=", 8) == 0) 
	    script = arg + 8;
	else if (strcmp(arg, "--") == 0) {
	    --argc;
	    ++argv;
	    break;
	} else if (arg[0] == '-') {
	    dico_log(L_ERR, 0, "unknown option %s", arg);
	    return 1;
	} else
	    break;
    }
    
    if (argc < 3) {
	dico_log(L_ERR, 0, "usage: %s user pass command [args...]",
		 dico_program_name);
	return 1;
    }
    user = argv[0];
    pass = argv[1];

    argc -= 2;
    argv += 2;

    iostr = run_command(argc, argv, &pid);

    if (!iostr)
        return 1;
    dico_stream_set_buffer(iostr, dico_buffer_line, DICO_MAX_BUFFER);

    dict_read_reply(); 
    assert_status("220");
    get_msgid();
    apop_auth(user, pass);

    if (script) {
	FILE *fp = fopen(script, "r");
	if (!fp) {
	    dico_log(L_ERR, errno, "cannot open %s", script);
	    exit(1);
	}
	writer(fp);
	printer();
    }

    return 0;
}
