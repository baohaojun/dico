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
#include <grecs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <wordsplit.h>

int grecs_log_to_stderr = 1;
void (*grecs_log_setup_hook) () = NULL;

struct input_file_ident {
	ino_t i_node;
	dev_t device;
};

struct buffer_ctx {
	struct buffer_ctx *prev;	/* Pointer to previous context */
	grecs_locus_t locus;		/* Current input location */
	size_t namelen;		/* Length of the file name */
	size_t xlines;		/* Number of #line directives output so far */
	struct input_file_ident id;
	FILE *infile;
};

extern int grecs_grecs__flex_debug;
static struct buffer_ctx *context_stack;
static char *linebufbase = NULL;
static size_t linebufsize = 0;

#define INFILE context_stack->infile
#define LOCUS context_stack->locus
#define POINT context_stack->locus.beg

static char *linebuf;
static size_t bufsize;
static char *putback_buffer;
static size_t putback_size;
static size_t putback_max;

static int push_source (const char *name, int once);
static int pop_source (void);
static int parse_include (const char *text, int once);

ssize_t
grecs_getline(char **pbuf, size_t *psize, FILE *fp)
{
	char *buf = *pbuf;
	size_t size = *psize;
	ssize_t off = 0;
  
	if (!buf) {
		size = 1;
		buf = grecs_malloc(size);
	}
	
	do {
		if (off == size - 1) {
			size_t nsize = 2 * size;
			if (nsize < size)
				grecs_alloc_die();
			buf = grecs_realloc(buf, nsize);
			size = nsize;
		}
		if (!fgets(buf + off, size - off, fp)) {
			if (off == 0)
				off = -1;
			break;
		}
		off += strlen(buf + off);
	} while (buf[off - 1] != '\n');

	*pbuf = buf;
	*psize = size;
	return off;
}
      
static void
putback(const char *str)
{
	size_t len;
	
	if (!*str)
		return;
	len = strlen(str) + 1;
	if (len > putback_max) {
		putback_max = len;
		putback_buffer = grecs_realloc(putback_buffer, putback_max);
	}
	strcpy(putback_buffer, str);
	putback_size = len - 1;
}

static void
pp_line_stmt()
{
	size_t ls_size;
	size_t pb_size;

	if (grecs_asprintf(&linebufbase, &linebufsize,
			   "#line %lu \"%s\" %lu\n",
			   (unsigned long) POINT.line,
			   POINT.file, (unsigned long) context_stack->xlines))
		grecs_alloc_die();

	ls_size = strlen(linebufbase);
	pb_size = putback_size + ls_size + 1;

	if (pb_size > putback_max) {
		putback_max = pb_size;
		putback_buffer = grecs_realloc(putback_buffer, putback_max);
	}

	context_stack->xlines++;
	strcpy(putback_buffer + putback_size, linebufbase);
	putback_size += ls_size;
}

#define STRMATCH(p, len, s) (len >= sizeof(s)			     \
			     && memcmp(p, s, sizeof(s) - 1) == 0     \
			     && isspace(p[sizeof(s) - 1]))

static int
next_line()
{
	ssize_t rc;

	do {
		if (putback_size) {
			if (putback_size + 1 > bufsize) {
				bufsize = putback_size + 1;
				linebuf = grecs_realloc(linebuf, bufsize);
			}
			strcpy(linebuf, putback_buffer);
			rc = putback_size;
			putback_size = 0;
		}
		else if (!context_stack)
			return 0;
		else
			rc = grecs_getline(&linebuf, &bufsize, INFILE);
	} while (rc == -1 && pop_source() == 0);
	return rc;
}

size_t
grecs_preproc_fill_buffer(char *buf, size_t size)
{
	size_t bufsize = size;

	while (next_line() > 0) {
		char *p;
		size_t len;
		int is_line = 0;
		
		for (p = linebuf; *p && isspace(*p); p++)
			;
		if (*p == '#') {
			size_t l;
			for (p++; *p && isspace(*p); p++)
				;
			l = strlen(p);
			if (STRMATCH(p, l, "include_once")) {
				if (parse_include(linebuf, 1))
					putback("/*include_once*/\n");
				continue;
			} else if (STRMATCH(p, l, "include")) {
				if (parse_include(linebuf, 0))
					putback("/*include*/\n");
				continue;
			} else if (STRMATCH(p, l, "line"))
				is_line = 1;
		}

		len = strlen(linebuf);

		if (len > size)
			len = size;
		
		memcpy(buf, linebuf, len);
		buf += len;
		size -= len;
		
		if (size == 0) {
			putback(linebuf + len);
			break;
		}

		if (!is_line && len > 0 && linebuf[len - 1] == '\n')
			POINT.line++;
	}
	return bufsize - size;
}

#define STAT_ID_EQ(st,id) ((id).i_node == (st).st_ino       \
			    && (id).device == (st).st_dev)

static struct buffer_ctx *
ctx_lookup(struct stat *st)
{
	struct buffer_ctx *ctx;

	if (!context_stack)
		return NULL;

	for (ctx = context_stack->prev; ctx; ctx = ctx->prev)
		if (STAT_ID_EQ(*st, ctx->id))
			break;
	return ctx;
}

const char *grecs_preprocessor = NULL;
static struct grecs_list *grecs_usr_include_path;
static struct grecs_list *grecs_std_include_path;

size_t
grecs_include_path_count(int flag)
{
    size_t count = 0;
    if (flag & GRECS_STD_INCLUDE)
	count += grecs_std_include_path ? grecs_std_include_path->count : 0;
    if (flag & GRECS_USR_INCLUDE)
	count += grecs_usr_include_path ? grecs_usr_include_path->count : 0;
    return 0;
}

static int
foreach_dir(struct grecs_list *list, int flag,
	    int (*fun)(int, const char *, void *), void *data)
{
    int rc;
    struct grecs_list_entry *ep;

    for (ep = list->head; rc == 0 && ep; ep = ep->next)
	rc = fun(flag, ep->data, data);
    return rc;
}

int
grecs_foreach_include_dir(int flag, int (*fun)(int, const char *, void *),
			  void *data)
{
    int rc = 0;

    if (flag & GRECS_STD_INCLUDE)
	rc = foreach_dir(grecs_std_include_path, GRECS_STD_INCLUDE, fun, data);
    if (rc == 0 && (flag & GRECS_USR_INCLUDE))
	rc = foreach_dir(grecs_usr_include_path, GRECS_USR_INCLUDE, fun, data);
    return rc;
}


struct file_data {
	const char *name;
	size_t namelen;
	char *buf;
	size_t buflen;
	int found;
};

static int
pp_list_find(struct grecs_list *list, struct file_data *dptr)
{
	struct grecs_list_entry *ep;
	
	for (ep = list->head; !dptr->found && ep; ep = ep->next) {
		const char *dir = ep->data;
		size_t size = strlen (dir) + 1 + dptr->namelen + 1;
		if (size > dptr->buflen) {
			dptr->buflen = size;
			dptr->buf = grecs_realloc(dptr->buf, dptr->buflen);
		}
		strcpy(dptr->buf, dir);
		strcat(dptr->buf, "/");
		strcat(dptr->buf, dptr->name);
		dptr->found = access(dptr->buf, F_OK) == 0;
	}
	return dptr->found;
}

static void
incl_free(void *data)
{
	grecs_free(data);
}

void
grecs_include_path_clear()
{
	if (grecs_usr_include_path)
		grecs_list_clear(grecs_usr_include_path);
	if (grecs_std_include_path)
		grecs_list_clear(grecs_std_include_path);
}

void
grecs_include_path_setup_v(char **dirs)
{
	if (!grecs_usr_include_path) {
		grecs_usr_include_path = grecs_list_create();
		grecs_usr_include_path->free_entry = incl_free;
	}
	grecs_std_include_path = grecs_list_create();
	grecs_std_include_path->free_entry = incl_free;
	if (dirs) {
		int i;
		for (i = 0; dirs[i]; i++)
			/* FIXME: Element never freed */
			grecs_list_append(grecs_std_include_path,
					  grecs_strdup(dirs[i]));
	}
}

void
grecs_include_path_setup(const char *dir, ...)
{
	const char *p;
	char **argv = NULL;
	size_t argc = 0;
	size_t argi = 0;
	va_list ap;
	
	va_start(ap, dir);
	p = dir;
	while (1) {
		if (argi == argc) {
			if (argc == 0)
				argc = 16;
			else
				argc += 16;
			argv = grecs_realloc(argv, argc * sizeof(argv[0]));
		}
		argv[argi++] = (char*) p;
		if (!p)
			break;
		p = va_arg(ap, const char*);
	}
	grecs_include_path_setup_v(argv);
	grecs_free(argv);
	va_end(ap);
}

void
grecs_preproc_add_include_dir(char *dir)
{
	if (!grecs_usr_include_path) {
		grecs_usr_include_path = grecs_list_create();
		grecs_usr_include_path->free_entry = incl_free;
	}
	grecs_list_append(grecs_usr_include_path, grecs_strdup(dir));
}

static struct grecs_symtab *incl_sources;

/* Calculate the hash of a struct input_file_ident.  */
static unsigned
incl_hasher(void *data, unsigned long n_buckets)
{
	const struct input_file_ident *id = data;
	return (id->i_node + id->device) % n_buckets;
}

/* Compare two input_file_idents for equality.  */
static int
incl_compare(void const *data1, void const *data2)
{
	const struct input_file_ident *id1 = data1;
	const struct input_file_ident *id2 = data2;
	return !(id1->device == id2->device && id1->i_node == id2->i_node);
}

static int
source_lookup(struct stat *st)
{
	struct input_file_ident key;
	int install = 1;
  
	if (!incl_sources) {
		incl_sources = grecs_symtab_create(
			sizeof(struct input_file_ident), 
			incl_hasher,
			incl_compare,
			NULL,
			NULL,/*FIXME: alloc*/
			NULL);
		if (!incl_sources)
			grecs_alloc_die();
	}
	
	key.i_node = st->st_ino;
	key.device = st->st_dev;
	if (!grecs_symtab_lookup_or_install(incl_sources, &key, &install))
		grecs_alloc_die();
	return !install;
}


static int
push_source(const char *name, int once)
{
	FILE *fp;
	struct buffer_ctx *ctx;
	struct stat st;
	int rc = stat(name, &st);

	if (context_stack) {
		if (rc) {
			grecs_error(&LOCUS, errno,
				    _("Cannot stat `%s'"), name);
			return 1;
		}

		if (POINT.file && STAT_ID_EQ(st, context_stack->id)) {
			grecs_error(&LOCUS, 0, _("Recursive inclusion"));
			return 1;
		}
		
		if ((ctx = ctx_lookup(&st))) {
			grecs_error(&LOCUS, 0, _("Recursive inclusion"));
			if (ctx->prev)
				grecs_error(&ctx->prev->locus, 0,
					    _("`%s' already included here"),
					    name);
			else
				grecs_error(&LOCUS, 0,
					    _("`%s' already included at top level"),
					    name);
			return 1;
		}
	} else if (rc) {
		grecs_error(NULL, errno, _("Cannot stat `%s'"), name);
		return 1;
	}

	if (once && source_lookup(&st))
		return -1;

	fp = fopen(name, "r");
	if (!fp) {
		grecs_error(context_stack ? &LOCUS : NULL, errno,
			    _("Cannot open `%s'"), name);
		return 1;
	}

	/* Push current context */
	ctx = grecs_malloc(sizeof(*ctx));
	ctx->locus.beg.file = grecs_install_text(name);
	ctx->locus.beg.line = 1;
	ctx->locus.beg.col = 0;
	ctx->locus.end.file = NULL;
	ctx->locus.end.line = ctx->locus.end.col = 0;
	ctx->xlines = 0;
	ctx->namelen = strlen(ctx->locus.beg.file);
	ctx->id.i_node = st.st_ino;
	ctx->id.device = st.st_dev;
	ctx->infile = fp;
	ctx->prev = context_stack;
	context_stack = ctx;

	if (grecs_grecs__flex_debug)
	  fprintf (stderr, "Processing file `%s'\n", name);

	pp_line_stmt();

	return 0;
}

static int
pop_source()
{
	struct buffer_ctx *ctx;
	
	if (!context_stack)
		return 1;
	
	fclose(INFILE);

	/* Restore previous context */
	ctx = context_stack->prev;
	grecs_free(context_stack);
	context_stack = ctx;

	if (!context_stack) {
		if (grecs_grecs__flex_debug)
			fprintf(stderr, "End of input\n");
		return 1;
	}

	POINT.line++;

	if (grecs_grecs__flex_debug)
		fprintf(stderr, "Resuming file `%s' at line %lu\n",
			POINT.file, (unsigned long) POINT.line);

	pp_line_stmt();

	return 0;
}

char *
grecs_find_include_file(const char *name, int allow_cwd)
{
	static char *cwd = ".";
	struct file_data fd;

	fd.name = name;
	fd.namelen = strlen(name);
	fd.buf = NULL;
	fd.buflen = 0;
	fd.found = 0;

	if (!grecs_usr_include_path)
		grecs_include_path_setup(NULL);
	if (allow_cwd) {
		grecs_list_append(grecs_usr_include_path, cwd);
		pp_list_find(grecs_usr_include_path, &fd);
		grecs_list_remove_tail(grecs_usr_include_path);
	} else
		pp_list_find(grecs_usr_include_path, &fd);

	if (!fd.found) {
		pp_list_find(grecs_std_include_path, &fd);
		if (!fd.found)
			return NULL;
	}
	return fd.buf;
}

static int
parse_include(const char *text, int once)
{
	struct wordsplit ws;
	char *tmp = NULL;
	char *p = NULL;
	int rc = 1;
	
	if (wordsplit(text, &ws, WRDSF_DEFFLAGS))
		grecs_error(&LOCUS, 0, _("Cannot parse include line"));
	else if (ws.ws_wordc != 2) {
		wordsplit_free(&ws);
		grecs_error(&LOCUS, 0, _("invalid include statement"));
	} else {
		size_t len;
		int allow_cwd;
		
		p = ws.ws_wordv[1];
		len = strlen (p);
		
		if (p[0] == '<' && p[len - 1] == '>') {
			allow_cwd = 0;
			p[len - 1] = 0;
			p++;
		}
		else
			allow_cwd = 1;
		
		if (p[0] != '/') {
			p = grecs_find_include_file(p, allow_cwd);
			if (!p)
				grecs_error(&LOCUS, 0,
					    _("%s: No such file or directory"),
					    p);
		}
	}
  
	if (p)
		rc = push_source(p, once);
	grecs_free(tmp);
	wordsplit_free(&ws);
	return rc;
}

int
grecs_preproc_init(const char *name)
{
	return push_source(name, 0);
}

void
grecs_preproc_done()
{
	grecs_symtab_free(incl_sources);
	grecs_free(linebuf);
	grecs_free(putback_buffer);
	free(linebufbase); /* Allocated via standard malloc/realloc */
}

int
grecs_preproc_run(const char *config_file, const char *extpp)
{
	size_t i;
	char buffer[512];
	
	if (grecs_preproc_init(config_file))
		return 1;
	if (extpp) {
		FILE *outfile;
		char *setup_file;
		char *cmd = NULL;

		setup_file = grecs_find_include_file("pp-setup", 1);
		if (setup_file) {
			size_t size = 0;
			if (grecs_asprintf(&cmd, &size,
					   "%s %s -", extpp, setup_file))
				grecs_alloc_die();
			grecs_free(setup_file);
		} else
			cmd = grecs_strdup(extpp);
		/*FIXME_DEBUG_F1 (2, "Running preprocessor: `%s'", cmd);*/
		outfile = popen(cmd, "w");
		if (!outfile) {
			grecs_error(NULL, errno,
				    _("Unable to start external preprocessor `%s'"),
				    cmd);
			grecs_free(cmd);
			return 1;
		}

		while ((i = grecs_preproc_fill_buffer(buffer, sizeof buffer)))
			fwrite(buffer, 1, i, outfile);
		pclose(outfile);
		grecs_free(cmd);
	} else {
		while ((i = grecs_preproc_fill_buffer(buffer, sizeof buffer)))
			fwrite(buffer, 1, i, stdout);
	}
	grecs_preproc_done();
	return 0;
}

FILE *
grecs_preproc_extrn_start(const char *file_name, pid_t *ppid)
{
	int pout[2];
	pid_t pid;
	int i;
	FILE *fp = NULL;
	
	/*FIXME_DEBUG_F1 (2, "Running preprocessor: `%s'", ppcmd);*/

	pipe(pout);
	switch (pid = fork()) {
		/* The child branch.  */
	case 0:
		if (pout[1] != 1) {
			close(1);
			dup2(pout[1], 1);
		}

		/* Close unneeded descripitors */
		for (i = getdtablesize(); i > 2; i--)
			close(i);

		if (!grecs_log_to_stderr) {
			int p[2];
			char *buf = NULL;
			size_t size = 0;
			FILE *fp;
	  
			signal(SIGCHLD, SIG_DFL);
			pipe(p);
			switch (pid = fork()) {
				/* Grandchild */
			case 0:
				if (p[1] != 2) {
					close(2);
					dup2(p[1], 2);
				}
				close(p[0]);

				if (grecs_preproc_run(file_name,
						      grecs_preprocessor))
					exit(127);
				exit(0);

			case -1:
				/*  Fork failed */
				if (grecs_log_setup_hook)
					grecs_log_setup_hook();
				grecs_error(NULL, errno, _("Cannot run `%s'"),
					    grecs_preprocessor);
				exit(127);
				
			default:
				/* Sub-master */
				close (p[1]);
				fp = fdopen(p[0], "r");
				if (grecs_log_setup_hook)
					grecs_log_setup_hook();
				while (grecs_getline(&buf, &size, fp) > 0)
					grecs_error(NULL, 0, "%s", buf);
			}
		} else {
			grecs_preproc_run(file_name, grecs_preprocessor);
		}
		exit (0);

	case -1:
		/*  Fork failed */
		grecs_error(NULL, errno, _("Cannot run `%s'"),
			    grecs_preprocessor);
		break;
		
	default:
		close(pout[1]);
		fp = fdopen(pout[0], "r");
		break;
	}
	*ppid = pid;
	return fp;
}

void
grecs_preproc_extrn_shutdown(pid_t pid)
{
	int status;
	waitpid(pid, &status, 0);
}

