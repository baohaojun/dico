/* This file is part of GNU Dico
   Copyright (C) 2012 Sergey Poznyakoff

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
#include <errno.h>
#include <dico.h>

typedef int (*opfn) (int argc, char **argv);

struct optab {
    char *name;
    opfn fun;
};

static unsigned *
strtowc(const char *str)
{
    unsigned *buf;
    if (utf8_mbstr_to_wc(str, &buf, NULL)) {
	dico_log(L_ERR, errno, "cannot convert \"%s\"", str);
	abort();
    }
    return buf;
}


static int
help(int argc, char **argv)
{
    printf("No more help, sorry");
    return 0;
}

static int
op_strlen(int argc, char **argv)
{
    char *opname = *argv++;
    argc--;
    if (argc != 1) {
	dico_log(L_ERR, 0, "%s requires one argument", opname);
	return 1;
    }
    printf("%lu\n", (unsigned long) utf8_strlen(argv[0]));
    return 0;
}

static int
op_strcasecmp(int argc, char **argv)
{
    char *opname = *argv++;
    argc--;
    if (argc != 2) {
	dico_log(L_ERR, 0, "%s requires two arguments", opname);
	return 1;
    }
    printf("%d\n", utf8_strcasecmp(argv[0], argv[1]));
    return 0;
}

static int
op_strncasecmp(int argc, char **argv)
{
    char *opname = *argv++;
    argc--;
    if (argc != 3) {
	dico_log(L_ERR, 0, "%s requires three arguments", opname);
	return 1;
    }
    printf("%d\n", utf8_strncasecmp(argv[0], argv[1], atoi(argv[2])));
    return 0;
}

static int
op_wc_strcmp(int argc, char **argv)
{
    unsigned *wa, *wb;
    char *opname = *argv++;
    argc--;
    if (argc != 2) {
	dico_log(L_ERR, 0, "%s requires two arguments", opname);
	return 1;
    }

    wa = strtowc(argv[0]);
    wb = strtowc(argv[1]);

    printf("%d\n", utf8_wc_strcmp(wa, wb));
    return 0;
}

static int
op_wc_strncmp(int argc, char **argv)
{
    unsigned *wa, *wb;
    char *opname = *argv++;
    argc--;
    if (argc != 3) {
	dico_log(L_ERR, 0, "%s requires three arguments", opname);
	return 1;
    }

    wa = strtowc(argv[0]);
    wb = strtowc(argv[1]);

    printf("%d\n", utf8_wc_strncmp(wa, wb, atoi(argv[2])));
    return 0;
}

static int
op_wc_strcasecmp(int argc, char **argv)
{
    unsigned *wa, *wb;
    char *opname = *argv++;
    argc--;
    if (argc != 2) {
	dico_log(L_ERR, 0, "%s requires two arguments", opname);
	return 1;
    }

    wa = strtowc(argv[0]);
    wb = strtowc(argv[1]);

    printf("%d\n", utf8_wc_strcasecmp(wa, wb));
    return 0;
}

static int
op_wc_strncasecmp(int argc, char **argv)
{
    unsigned *wa, *wb;
    char *opname = *argv++;
    argc--;
    if (argc != 3) {
	dico_log(L_ERR, 0, "%s requires three arguments", opname);
	return 1;
    }

    wa = strtowc(argv[0]);
    wb = strtowc(argv[1]);

    printf("%d\n", utf8_wc_strncasecmp(wa, wb, atoi(argv[2])));
    return 0;
}

static int
op_toupper(int argc, char **argv)
{
    char *opname = *argv++;
    argc--;
    if (argc != 1) {
	dico_log(L_ERR, 0, "%s requires one arguments", opname);
	return 1;
    }
    if (utf8_toupper(argv[0]))
	abort();
    printf("%s\n", argv[0]);
    return 0;
}

static int
op_tolower(int argc, char **argv)
{
    char *opname = *argv++;
    argc--;
    if (argc != 1) {
	dico_log(L_ERR, 0, "%s requires one argument", opname);
	return 1;
    }
    if (utf8_tolower(argv[0]))
	abort();
    printf("%s\n", argv[0]);
    return 0;
}

static int
op_wc_strchr(int argc, char **argv)
{
    unsigned *wa, *wb;
    const unsigned *p;
    char *opname = *argv++;
    argc--;
    if (argc != 2) {
	dico_log(L_ERR, 0, "%s requires two arguments", opname);
	return 1;
    }

    wa = strtowc(argv[0]);
    wb = strtowc(argv[1]);
    p = utf8_wc_strchr(wa, wb[0]);
    if (!p)
	return 2;
    printf("%d\n", p - wa);
    return 0;
}

static int
op_wc_strchr_ci(int argc, char **argv)
{
    unsigned *wa, *wb;
    const unsigned *p;
    char *opname = *argv++;
    argc--;
    if (argc != 2) {
	dico_log(L_ERR, 0, "%s requires two arguments", opname);
	return 1;
    }

    wa = strtowc(argv[0]);
    wb = strtowc(argv[1]);
    p = utf8_wc_strchr_ci(wa, wb[0]);
    if (!p)
	return 2;
    printf("%d\n", p - wa);
    return 0;
}

static int
op_wc_strstr(int argc, char **argv)
{
    unsigned *wa, *wb;
    const unsigned *p;
    char *opname = *argv++;
    argc--;
    if (argc != 2) {
	dico_log(L_ERR, 0, "%s requires two arguments", opname);
	return 1;
    }

    wa = strtowc(argv[0]);
    wb = strtowc(argv[1]);
    p = utf8_wc_strstr(wa, wb);
    if (!p)
	return 2;
    printf("%d\n", p - wa);
    return 0;
}

struct optab optab[] = {
    { "help", help },
    { "strlen", op_strlen },
    { "strcasecmp", op_strcasecmp },
    { "strncasecmp", op_strncasecmp },
    { "wc_strcmp", op_wc_strcmp },
    { "wc_strncmp", op_wc_strncmp },
    { "wc_strcasecmp", op_wc_strcasecmp },
    { "wc_strncasecmp", op_wc_strncasecmp },
    { "toupper", op_toupper },
    { "tolower", op_tolower },
    { "wc_strchr", op_wc_strchr },
    { "wc_strchr_ci", op_wc_strchr_ci },
    { "wc_strstr", op_wc_strstr },
    { NULL }
};

void
usage(FILE *fp)
{
    fprintf(fp, "usage: %s [op] [args]\n", dico_program_name);
}

int
main(int argc, char **argv)
{
    struct optab *op;
    
    dico_set_program_name(argv[0]);
    argc--;
    argv++;
    if (!argc) {
	usage(stderr);
	return 1;
    }
    for (op = optab; op->name; op++)
	if (strcmp(*argv, op->name) == 0)
	    return op->fun(argc, argv);
    dico_log(L_ERR, 0, "unknown operation");
    return 1;
}
