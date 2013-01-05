/* grecs - Gray's Extensible Configuration System
   Copyright (C) 2007-2012 Sergey Poznyakoff

   Grecs is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   Grecs is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with Grecs. If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "grecs.h"

struct logging_setup {
	int use_syslog;
	int print_priority;
	char *tag;
	char *facility;
};

struct logging_setup logging_setup;
char *scalar_string;
char *mailbox_pattern;
char *mailbox_type;
struct grecs_list *listvar;

struct program {
	char *name;
	struct logging_setup logging_setup;
	char *scalar_string;
	grecs_locus_t locus;
	struct program *next;
};

struct grecs_list *proglist;

static int
cb_logging_facility(enum grecs_callback_command cmd,
		   grecs_locus_t *locus,
		   void *varptr,
		   grecs_value_t *value,
		   void *cb_data)
{
	if (cmd != grecs_callback_set_value) {
		grecs_error(locus, 0, _("Unexpected block statement"));
		return 1;
	}
	if (!value || value->type != GRECS_TYPE_STRING) {
		grecs_error(locus, 0, _("expected string argument"));
		return 1;
	}

	*(char**)varptr = grecs_strdup(value->v.string);
	return 0;
}

static struct grecs_keyword logging_kwtab[] = {
	{ "syslog", NULL, "Send to syslog",
	  grecs_type_bool, GRECS_DFLT, NULL,
	  offsetof(struct logging_setup, use_syslog) },
	{ "facility", "name", "Set logging facility",
	  grecs_type_string, GRECS_AGGR, NULL,
	  offsetof(struct logging_setup, facility), cb_logging_facility },
	{ "tag", "label", "Tag logging messages with this string",
	  grecs_type_string, GRECS_DFLT, NULL,
	  offsetof(struct logging_setup, tag) },
	{ "print-priority", NULL, "Prefix each message with its priority",
	  grecs_type_bool, GRECS_DFLT, NULL,
	  offsetof(struct logging_setup, print_priority) },
	{ NULL },
};

static struct grecs_keyword mailbox_kwtab[] = {
	{ "mailbox-pattern", NULL, "Default mailbox pattern",
	  grecs_type_string, GRECS_DFLT, &mailbox_pattern },
	{ "mailbox-type", NULL, "Default mailbox type",
	  grecs_type_string, GRECS_DFLT, &mailbox_type },
	{ NULL },
};


static struct grecs_keyword program_kwtab[] = {
	{ "scalar", "label", "Scalar string",
	  grecs_type_string, GRECS_DFLT,
	  NULL, offsetof(struct program,scalar_string) },
	{ "logging", NULL, N_("Configure logging logging"),
	  grecs_type_section, GRECS_DFLT,
	  NULL, offsetof(struct program,logging_setup),
	  NULL, NULL, logging_kwtab },
	{ NULL }
};

static int
cb_program(enum grecs_callback_command cmd,
	   grecs_locus_t *locus,
	   void *varptr,
	   grecs_value_t *value,
	   void *cb_data)
{
	struct program *prog;
	void **pdata = cb_data;

	switch (cmd) {
	case grecs_callback_section_begin:
		if (!value || value->type != GRECS_TYPE_STRING) {
			grecs_error(locus, 0, _("tag must be a string"));
			return 0;
		}
		prog = grecs_zalloc(sizeof(*prog));
		prog->name = grecs_strdup(value->v.string);
		prog->locus = *locus;
		*pdata = prog;
		break;

	case grecs_callback_section_end:
		prog = *pdata;
		if (!proglist)
			proglist = grecs_list_create();
		grecs_list_append(proglist, prog);
		break;

	case grecs_callback_set_value:
		grecs_error(locus, 0, _("invalid use of block statement"));
	}
	return 0;
}
		
static struct grecs_keyword main_kwtab[] = {
	{ "scalar", "label", "Scalar string",
	  grecs_type_string, GRECS_DFLT, &scalar_string },
	{ "logging", NULL, N_("Configure logging logging"),
	  grecs_type_section, GRECS_DFLT, &logging_setup, 0, NULL,
	  NULL, logging_kwtab },
	{ "mailbox", NULL, N_("Mailbox configuration"),
	  grecs_type_section, GRECS_DFLT, NULL, 0, NULL, NULL, mailbox_kwtab },
	{ "program", "name: string", "Subprogram configuration",
	  grecs_type_section, GRECS_DFLT,
	  NULL, 0, cb_program, NULL, program_kwtab },
	{ "listvar", NULL, "list variable",
	  grecs_type_string, GRECS_LIST, &listvar },
	{ NULL }
};


#define S(s) ((s) ? (s) : "(null)")

static void
print_logging_setup(struct logging_setup *p)
{
	printf("logging: %d/%s/%s/%d\n",
	       p->use_syslog, S(p->facility), S(p->tag), p->print_priority);
}

static void
print_program(struct program *prog)
{
	printf("Program %s:\n", prog->name);
	printf("scalar = %s\n", S(prog->scalar_string));
	print_logging_setup(&prog->logging_setup);
}

static int
node_ident_cmp(struct grecs_node const *a, struct grecs_node const *b)
{
	return strcmp(a->ident, b->ident);
}

static void
usage(const char *arg, FILE *fp, int code)
{
	fprintf(fp, "usage: %s [-h] [-cfhelp] [-reduce] [-sort] [-print] [-locus] [-noset] file\n", arg);
	exit(code);
}

int
main(int argc, char **argv)
{
	char *progname = argv[0];
	const char *file = NULL;
	struct grecs_node *tree;
	int cfhelp = 0;
	int reduce = 0;
	int print = 0;
	int sort = 0;
	int flags = GRECS_NODE_FLAG_DEFAULT;
	int dontset = 0;
	
	while (--argc) {
		char *arg = *++argv;
		if (strcmp(arg, "-cfhelp") == 0)
			cfhelp = 1;
		else if (strcmp(arg, "-h") == 0)
			usage(progname, stdout, 0);
		else if (strcmp(arg, "-reduce") == 0)
			reduce = 1;
		else if (strcmp(arg, "-print") == 0)
			print = 1;
		else if (strcmp(arg, "-locus") == 0)
			flags |= GRECS_NODE_FLAG_LOCUS;
		else if (strncmp(arg, "-delim=", 7) == 0)
			flags |= arg[7];
		else if (strcmp(arg, "-sort") == 0)
			sort = 1;
		else if (strcmp(arg, "-noset") == 0)
			dontset = 1;
		else if (arg[0] == '-')
			usage(progname, stderr, 1);
		else if (file)
			usage(progname, stderr, 1);
		else
			file = arg;
	}
				
	if ((!file && !cfhelp) || argc)
		usage(progname, stderr, 1);

	if (cfhelp) {
		static char docstring[] =
			"Sample configuration file structure.\n";
		grecs_print_docstring(docstring, 0, stdout);
		grecs_print_statement_array(main_kwtab, 1, 0, stdout);
		exit(0);
	}
	
	tree = grecs_parse(file);
	if (!tree)
		exit(2);
	if (reduce)
		grecs_tree_reduce(tree, main_kwtab, GRECS_AGGR);
	if (sort)
		grecs_tree_sort(tree, node_ident_cmp);
	if (print) {
		grecs_print_node(tree, flags, stdout);
		fputc('\n', stdout);
	}
	if (dontset)
		exit(0);
	if (grecs_tree_process(tree, main_kwtab))
		exit(2);
	grecs_tree_free(tree);
	
	printf("Global settings:\n");
	printf("scalar = %s\n", S(scalar_string));
	if (listvar) {
		struct grecs_list_entry *ep;
		printf("listvar =");
		for (ep = listvar->head; ep; ep = ep->next)
			printf(" \"%s\"", (char*)ep->data);
		putchar('\n');
	}
	
	print_logging_setup(&logging_setup);

	if (proglist) {
		struct grecs_list_entry *ep;
		
		printf("Programs configured: %d\n", grecs_list_size(proglist));
		for (ep = proglist->head; ep; ep = ep->next)
			print_program(ep->data);
	}
	
	exit(0);
}
	
