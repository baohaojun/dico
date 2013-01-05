/* This file is part of GNU Dico
   Copyright (C) 2003,2004,2007,2008 Sergey Poznyakoff
  
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
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "wordsplit.h"
#include "itrsh.h"

void
next(dico_iterator_t itr, char *arg)
{
    int skip = arg ? strtoul(arg, NULL, 0) :  1;

    if (skip == 0)
	fprintf(stderr, "next arg?\n");
    while (skip--)
	dico_iterator_next(itr);
}

void
prev(dico_iterator_t itr, char *arg)
{
    int skip = arg ? strtoul(arg, NULL, 0) :  1;

    if (skip == 0)
	fprintf(stderr, "next arg?\n");
    while (skip--)
	dico_iterator_prev(itr);
}

void
print(struct itr_shell *shp)
{
    dico_iterator_t itr = shp->get_iterator(shp->object);
    void *elt;

    if (shp->count)
	printf("# items: %lu\n", (unsigned long) shp->count(shp->object));
    
    if (!itr) {
	dico_log(L_ERR, 0, "get_iterator failed");
	exit(1);
    }
    
    for (elt = dico_iterator_first(itr); elt;
	 elt = dico_iterator_next(itr)) {
	shp->print_item(elt);
	putchar('\n');
    }
    dico_iterator_destroy(&itr);
}

#define NITR 4

int
iter(int *pnum, int argc, char **argv)
{
    int n;
  
    if (argc != 2) {
	fprintf(stderr, "iter num?\n");
	return 1;
    }

    n = strtoul(argv[1], NULL, 0);
    if (n < 0 || n >= NITR) {
	fprintf(stderr, "iter [0-%d]?\n", NITR-1);
	return 1;
    }
    *pnum = n;
    return 0;
}

void
help(struct itr_shell *shp)
{
    struct itr_shell_command *cmd;
    static char format[] = "%-8.8s %-10.10s - %s\n";
    
    for (cmd = shp->cmdtab; cmd->name; cmd++) 
	printf(format,
	       cmd->name,
	       cmd->argstr ? cmd->argstr : "",
	       cmd->docstr ? cmd->docstr : "");
    printf(format,
	   "next", "[count]", "go to next item");
    printf(format,
	   "first", "", "rewind to the first item");
    printf(format,
	   "print", "", "print all items");
    printf(format,
	   "quit", "", "quit the shell");
    printf(format,
	   "iter", "num", "switch to iterator #num");
    printf(format,
	   "help", "", "print this help list");
}

int num = 0;
dico_iterator_t itr[NITR];

void
current(struct itr_shell *shp)
{
    char *elt = dico_iterator_current(itr[num]);
    printf("%lu:", (unsigned long) num);
    if (elt)
	shp->print_item(elt);
    else
	printf("%s", "NUL");
    putchar('\n');
}


dico_iterator_t
shell_iterator()
{
    return itr[num];
}

void
shell(struct itr_shell *shp)
{
    struct wordsplit ws;
    int wsflags = WRDSF_DEFFLAGS | WRDSF_SHOWERR;
    int interactive = isatty(0);
    
    memset(&itr, 0, sizeof itr);
    num = 0;
    while (1) {
	char buf[80];
	void *elt;
	
	if (!itr[num])  {
	    itr[num] = shp->get_iterator(shp->object);
	    if (!itr[num]) {
		perror("get_iterator");
		exit(1);
	    }
	    dico_iterator_first(itr[num]);
	}
      
	elt = dico_iterator_current(itr[num]);

	if (interactive) {
	    printf("%d:(", num);
	    if (elt)
		shp->print_item(elt);
	    else
		printf("NUL");
	    printf(")> ");
	    fflush(stdout);
	}
	if (fgets(buf, sizeof buf, stdin) == NULL)
	    break;

	ws.ws_comment = "#";
	if (wordsplit(buf, &ws, wsflags|WRDSF_COMMENT))
	    exit(1);
	wsflags |= WRDSF_REUSE;
	if (ws.ws_wordc > 0) {
	    if (strcmp(ws.ws_wordv[0], "next") == 0)
		next(itr[num], ws.ws_wordv[1]);
	    else if (strcmp(ws.ws_wordv[0], "prev") == 0)
		prev(itr[num], ws.ws_wordv[1]);
	    else if (strcmp(ws.ws_wordv[0], "first") == 0)
		dico_iterator_first(itr[num]);
	    else if (strcmp(ws.ws_wordv[0], "print") == 0)
		print(shp);
	    else if (strcmp(ws.ws_wordv[0], "quit") == 0)
		break;
	    else if (strcmp(ws.ws_wordv[0], "iter") == 0) {
		int n;
		if (iter(&n, ws.ws_wordc, ws.ws_wordv) == 0 && !itr[n]) {
		    itr[n] = shp->get_iterator(shp->object);
		    if (!itr[n]) {
			perror("get_iterator");
			exit(1);
		    }
		    dico_iterator_first(itr[n]);
		}
		num = n;
	    } else if (strcmp(ws.ws_wordv[0], "close") == 0) {
		int n;
		if (iter(&n, ws.ws_wordc, ws.ws_wordv) == 0) {
		    dico_iterator_destroy(&itr[n]);
		    if (n == num && ++num == NITR)
			num = 0;
		}
	    } else if (strcmp(ws.ws_wordv[0], ".") == 0 ||
		       strcmp(ws.ws_wordv[0], "cur") == 0) {
		current(shp);
	    } else if (strcmp(ws.ws_wordv[0], "help") == 0)
		help(shp);
	    else {
		struct itr_shell_command *cmd;
		
		for (cmd = shp->cmdtab; cmd->name; cmd++) 
		    if (strcmp(cmd->name, ws.ws_wordv[0]) == 0) {
			if (ws.ws_wordc < cmd->minargc) 
			    fprintf(stderr, "too few arguments\n");
			else if (cmd->maxargc && ws.ws_wordc > cmd->maxargc) 
			    fprintf(stderr, "too many arguments\n");
			else
			    cmd->fun(shp->object, ws.ws_wordc, ws.ws_wordv);
			break;
		    }
		if (cmd->name == NULL) {
		    if (cmd->fun && ws.ws_wordc >= cmd->minargc
			&& (cmd->maxargc == 0 || ws.ws_wordc <= cmd->maxargc))
			cmd->fun(shp->object, ws.ws_wordc, ws.ws_wordv);
		    else
			fprintf(stderr, "?\n");
		}
	    }
	}
    }
    if (wsflags & WRDSF_REUSE)
	wordsplit_free(&ws);
}

