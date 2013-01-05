/* This file is part of GNU Dico.
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
#include <dico.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>    
#include <errno.h>
#include <sysexits.h>
#include <string.h>
#include "gcide.h"

static void
usage(FILE *fp)
{
    fprintf(fp, "usage: %s [-debug] [-struct] [-nopr] [-help] FILE [OFF SIZE]\n", dico_program_name);
}    

struct output_closure {
    FILE *stream;
    int flags;
    int level;
};

static int
print_tag(int end, struct gcide_tag *tag, void *data)
{
    size_t i;
    struct output_closure *clos = data;

    if (end) {
	clos->level--;
	printf("%*.*s", 2*clos->level,2*clos->level, "");
	printf("END");
	if (tag->tag_parmc)
	    printf(" %s", tag->tag_name);
	putchar('\n');
	return 0;
    }
    printf("%*.*s", 2*clos->level,2*clos->level, "");
    switch (tag->tag_type) {
    case gcide_content_unspecified:
	break;
    case gcide_content_text:
	printf("TEXT");
	for (i = 0; i < tag->tag_parmc; i++)
	    printf(" %s", tag->tag_parmv[i]);
	printf(":\n%s\n%*.*sENDTEXT", tag->tag_v.text,
	       2*clos->level, 2*clos->level, "");
	if (tag->tag_parmc)
	    printf(" %s", tag->tag_name);
	putchar('\n');
	break;
    case gcide_content_taglist:
	printf("BEGIN");
	for (i = 0; i < tag->tag_parmc; i++)
	    printf(" %s", tag->tag_parmv[i]);
	putchar('\n');
	clos->level++;
	break;
    }
    return 0;
}

#define GCIDE_NOPR 0x0000001
#define GOF_IGNORE 0x0001000
#define GOF_AS     0x0002000

static int
print_text(int end, struct gcide_tag *tag, void *data)
{
    struct output_closure *clos = data;
    static char *quote[2] = { "``", "''" };
    static char *ref[2] = { "{" , "}" };

    switch (tag->tag_type) {
    case gcide_content_unspecified:
	break;
    case gcide_content_text:
	if (clos->flags & GOF_IGNORE)
	    break;
	if (clos->flags & GOF_AS) {
	    char *s = tag->tag_v.text;
	    
	    if (strncmp(s, "as", 2) == 0 &&
		(isspace(s[3]) || ispunct(s[3]))) {
		fwrite(s, 3, 1, clos->stream);
		for (s += 3; *s && isspace(*s); s++)
		    fputc(*s, clos->stream);
		fprintf(clos->stream, "%s%s", quote[0], s);
	    } else
		fprintf(clos->stream, "%s", quote[0]);
	} else
	    fprintf(clos->stream, "%s", tag->tag_v.text);
	break;
    case gcide_content_taglist:
	if (tag->tag_parmc) {
	    clos->flags &= ~GOF_AS;
	    if (end) {
		if (strcmp(tag->tag_name, "pr") == 0 &&
			 clos->flags & GCIDE_NOPR)
		    clos->flags &= ~GOF_IGNORE;
		else if (clos->flags & GOF_IGNORE)
		    break;
		else if (strcmp(tag->tag_name, "as") == 0)
		    fprintf(clos->stream, "%s", quote[1]);
		else if (strcmp(tag->tag_name, "er") == 0)
		    fprintf(clos->stream, "%s", ref[1]);
	    } else {
		if (strcmp(tag->tag_name, "pr") == 0 &&
			 clos->flags & GCIDE_NOPR)
		    clos->flags |= GOF_IGNORE;
		else if (clos->flags & GOF_IGNORE)
		    break;
		else if (strcmp(tag->tag_name, "sn") == 0)
		    fputc('\n', clos->stream);
		else if (strcmp(tag->tag_name, "as") == 0)
		    clos->flags |= GOF_AS;
		else if (strcmp(tag->tag_name, "er") == 0)
		    fprintf(clos->stream, "%s", ref[0]);
	    }
	}
    }
    return 0;
}

int
main(int argc, char **argv)
{
    char *file;
    unsigned long offset = 0;
    unsigned long size = 0;
    FILE *fp;
    char *textbuf;
    struct gcide_parse_tree *tree;
    struct output_closure clos;
    int show_struct = 0;
    int dbglex = 0;
    
    dico_set_program_name(argv[0]);
    clos.flags = 0;
    clos.stream = stdout;

    while (--argc) {
	char *arg = *++argv;

	if (strcmp(arg, "-debug") == 0)
	    dbglex = 1;
	else if (strcmp(arg, "-h") == 0 || strcmp(arg, "-help") == 0) {
	    usage(stdout);
	    exit(0);
	} else if (strcmp(arg, "-struct") == 0)
	    show_struct = 1;
	else if (strcmp(arg, "-nopr") == 0)
	    clos.flags = GCIDE_NOPR;
	else if (strcmp(arg, "--") == 0) {
	    --argc;
	    ++argv;
	    break;
	} else if (arg[0] == '-') {
	    usage(stderr);
	    exit(EX_USAGE);
	} else
	    break;
    }

    if (argc == 0 || argc > 3) {
	usage(stderr);
	exit(EX_USAGE);
    }

    file = argv[0];
    if (argc > 1) {
	offset = atoi(argv[1]);
	if (argc == 3)
	    size = atoi(argv[2]);
    }

    if (size == 0) {
	struct stat st;
	if (stat(file, &st)) {
	    dico_log(L_ERR, errno, "stat");
	    exit(EX_UNAVAILABLE);
	}
	if (st.st_size < offset) {
	    dico_log(L_ERR, 0, "invalid offset");
	    exit(EX_UNAVAILABLE);
	}   
	size = st.st_size - offset;
    }
    
    textbuf = malloc(size);
    if (!textbuf) {
	dico_log(L_ERR, 0, "not enough memory");
	exit(EX_UNAVAILABLE);
    }
    
    fp = fopen(file, "r");
    if (!fp) {
	dico_log(L_ERR, errno, "cannot open file %s", file);
	exit(EX_UNAVAILABLE);
    }

    if (fseek(fp, offset, SEEK_SET)) {
	dico_log(L_ERR, errno, "%s", file);
	exit(EX_UNAVAILABLE);
    }

    if (fread(textbuf, size, 1, fp) != 1) {
	dico_log(L_ERR, errno, "%s: read error", file);
	exit(EX_UNAVAILABLE);
    }
	
    tree = gcide_markup_parse(textbuf, size, dbglex);
    if (!tree)
	exit(EX_UNAVAILABLE);

    clos.level = 0;
    gcide_parse_tree_inorder(tree, show_struct ? print_tag : print_text, &clos);
    
    gcide_parse_tree_free(tree);
    exit(0);
}
