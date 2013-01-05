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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "grecs.h"

void
grecs_value_free(struct grecs_value *val)
{
	int i;

	if (!val)
		return;
	switch (val->type) {
	case GRECS_TYPE_STRING:
		grecs_free(val->v.string);
		break;

	case GRECS_TYPE_LIST:
		grecs_list_free(val->v.list);
		break;

	case GRECS_TYPE_ARRAY:
		for (i = 0; i < val->v.arg.c; i++)
			grecs_value_free(val->v.arg.v[i]);
		free(val->v.arg.v);
	}
	grecs_free(val);
}

struct grecs_node *
grecs_node_create(enum grecs_node_type type, grecs_locus_t *loc)
{
	struct grecs_node *np = grecs_zalloc(sizeof(*np));
	np->type = type;
	if (loc)
		np->locus = *loc;
	return np;
}

struct grecs_node *
grecs_node_create_points(enum grecs_node_type type,
			 struct grecs_locus_point beg,
			 struct grecs_locus_point end)
{
	grecs_locus_t loc;

	loc.beg = beg;
	loc.end = end;
	return grecs_node_create(type, &loc);
}

void
grecs_node_bind(struct grecs_node *master, struct grecs_node *node, int dn)
{
	struct grecs_node *np;

	if (!node)
		return;
	if (dn) {
		if (!master->down) {
			master->down = node;
			node->prev = NULL;
		} else {
			for (np = master->down; np->next; np = np->next)
				;
			np->next = node;
			node->prev = np;
		}
		for (; node; node = node->next)
			node->up = master;
	} else {
		if (!master->next) {
			master->next = node;
			node->prev = master;
		} else {
			for (np = master->next; np->next; np = np->next)
				;
			np->next = node;
			node->prev = np;
		}
		node->up = master->up;
	}
}

int
grecs_node_unlink(struct grecs_node *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else if (node->up)
		node->up->down = node->next;
	else
		return 1;
	if (node->next)
		node->next->prev = node->prev;
	return 0;
}


static void
listel_dispose(void *el)
{
	grecs_free(el);
}
 
struct grecs_list *
_grecs_simple_list_create(int dispose)
{
	struct grecs_list *lp = grecs_list_create();
	if (dispose)
		lp->free_entry = listel_dispose;
	return lp;
}




static enum grecs_tree_recurse_res
_tree_recurse(struct grecs_node *node, grecs_tree_recursor_t recfun,
	      void *data)
{
	enum grecs_tree_recurse_res res;
#define CKRES()						\
	switch (res) {					\
        case grecs_tree_recurse_fail:			\
        case grecs_tree_recurse_stop:			\
		return res;				\
	default:					\
		break;					\
	}

	while (node) {
		struct grecs_node *next = node->next;
		if (node->type == grecs_node_stmt) {
			res = recfun(grecs_tree_recurse_set, node, data);
			CKRES();
		} else {
			switch (recfun(grecs_tree_recurse_pre, node, data)) {
			case grecs_tree_recurse_ok:
				res = _tree_recurse(node->down, recfun, data);
				CKRES();
				res = recfun(grecs_tree_recurse_post, node,
					     data);
				CKRES();
				break;
			case grecs_tree_recurse_fail:
				return grecs_tree_recurse_fail;
			case grecs_tree_recurse_stop:
				return grecs_tree_recurse_stop;
			case grecs_tree_recurse_skip:
				break;
			}
		}
		node = next;
	}
	return grecs_tree_recurse_ok;
#undef CKRES
}

int
grecs_tree_recurse(struct grecs_node *node, grecs_tree_recursor_t recfun,
		   void *data)
{
	switch (_tree_recurse(node, recfun, data)) {
	case grecs_tree_recurse_ok:
	case grecs_tree_recurse_stop:
		return 0;
	default:
		break;
	}
	return 1;
}

void
grecs_node_free(struct grecs_node *node)
{
	if (!node)
		return;
	switch (node->type) {
	case grecs_node_root:
		grecs_symtab_free(node->v.texttab);
		break;
	default:
		grecs_value_free(node->v.value);
	}
	grecs_free(node->ident);
	grecs_free(node);
}

static enum grecs_tree_recurse_res
freeproc(enum grecs_tree_recurse_op op, struct grecs_node *node, void *data)
{
	switch (op) {
	case grecs_tree_recurse_set:
	case grecs_tree_recurse_post:
		grecs_node_unlink(node);
		grecs_node_free(node);
		break;
	case grecs_tree_recurse_pre:
		/* descend into the subtree */
		break;
	}
	return grecs_tree_recurse_ok;
}

int
grecs_tree_free(struct grecs_node *node)
{
	if (!node)
		return 0;
        /* FIXME: Check if that's a node_type_root */
	if (node->type != grecs_node_root) {
		errno = EINVAL;
		return 1;
	}
	grecs_tree_recurse(node, freeproc, NULL);
	return 0;
}



static int
fake_callback(enum grecs_callback_command cmd,
	       grecs_locus_t *locus,
	       void *varptr,
	       grecs_value_t *value,
	       void *cb_data)
{
	return 0;
}

static struct grecs_keyword fake = {
	"*",
	NULL,
	NULL,
	grecs_type_void,
	GRECS_DFLT,
	NULL,
	0,
	fake_callback,
	NULL,
	&fake
};

static struct grecs_keyword *
find_keyword(struct grecs_keyword *cursect, const char *ident)
{
	struct grecs_keyword *kwp;
  
	if (cursect && cursect->kwd && cursect != &fake) {
		for (kwp = cursect->kwd; kwp->ident; kwp++)
			if (strcmp(kwp->ident, ident) == 0)
				return kwp;
	} else {
		return &fake;
	}
	return NULL;
}

static void *
target_ptr(struct grecs_keyword *kwp, char *base)
{
	if (kwp->varptr)
		base = (char*) kwp->varptr + kwp->offset;
	else if (base)
		base += kwp->offset;
	
	return base;
}

static int
string_to_bool(const char *string, int *pval, grecs_locus_t const *locus)
{
	if (strcmp(string, "yes") == 0
	    || strcmp(string, "true") == 0
	    || strcmp(string, "t") == 0
	    || strcmp(string, "1") == 0)
		*pval = 1;
	else if (strcmp(string, "no") == 0
		 || strcmp(string, "false") == 0
		 || strcmp(string, "nil") == 0
		 || strcmp(string, "0") == 0)
		*pval = 0;
	else {
		grecs_error(locus, 0,
			     _("%s: not a valid boolean value"),
			     string);
		return 1;
	}
	return 0;
}

static int
string_to_host(struct in_addr *in, const char *string,
	       grecs_locus_t const *locus)
{
	if (inet_aton(string, in) == 0) {
		struct hostent *hp;

		hp = gethostbyname(string);
		if (hp == NULL)
			return 1;
		memcpy(in, hp->h_addr, sizeof(struct in_addr));
	}
	return 0;
}

static int
string_to_sockaddr(struct grecs_sockaddr *sp, const char *string,
		   grecs_locus_t const *locus)
{
	if (string[0] == '/') {
		struct sockaddr_un s_un;
		if (strlen(string) >= sizeof(s_un.sun_path)) {
			grecs_error(locus, 0,
				     _("%s: UNIX socket name too long"),
				     string);
			return 1;
		}
		s_un.sun_family = AF_UNIX;
		strcpy(s_un.sun_path, string);
		sp->len = sizeof(s_un);
		sp->sa = grecs_malloc(sp->len);
		memcpy(sp->sa, &s_un, sp->len);
	} else {
		char *p = strchr(string, ':');
		size_t len;
		struct sockaddr_in sa;
		
		sa.sin_family = AF_INET;
		if (p) 
			len = p - string;
		else
			len = strlen(string);
		
		if (len == 0)
			sa.sin_addr.s_addr = INADDR_ANY;
		else {
			char *host = grecs_malloc(len + 1);
			memcpy(host, string, len);
			host[len] = 0;
			
			if (string_to_host(&sa.sin_addr, host, locus)) {
				grecs_error(locus, 0,
					     _("%s: not a valid IP address or hostname"),
					     host);
				grecs_free(host);
				return 1;
			}
			grecs_free(host);
		}
		
		if (p) {
			struct servent *serv;
			
			p++;
			serv = getservbyname(p, "tcp");
			if (serv != NULL)
				sa.sin_port = serv->s_port;
			else {
				unsigned long l;
				char *q;
				
				/* Not in services, maybe a number? */
				l = strtoul(p, &q, 0);
				
				if (*q || l > USHRT_MAX) {
					grecs_error(locus, 0,
						     _("%s: not a valid port number"), p);
					return 1;
				}
				sa.sin_port = htons(l);
			}
		} else if (grecs_default_port)
			sa.sin_port = grecs_default_port;
		else {
			grecs_error(locus, 0, _("missing port number"));
			return 1;
		}
		sp->len = sizeof(sa);
		sp->sa = grecs_malloc(sp->len);
		memcpy(sp->sa, &sa, sp->len);
	}
	return 0;
}


/* The TYPE_* defines come from gnulib's intprops.h */

/* True if the arithmetic type T is signed.  */
# define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The maximum and minimum values for the integer type T.  These
   macros have undefined behavior if T is signed and has padding bits. */
# define TYPE_MINIMUM(t) \
  ((t) (! TYPE_SIGNED(t) \
        ? (t) 0 \
        : TYPE_SIGNED_MAGNITUDE(t) \
        ? ~ (t) 0 \
        : ~ TYPE_MAXIMUM(t)))
# define TYPE_MAXIMUM(t) \
  ((t) (! TYPE_SIGNED(t) \
        ? (t) -1 \
        : ((((t) 1 << (sizeof(t) * CHAR_BIT - 2)) - 1) * 2 + 1)))
# define TYPE_SIGNED_MAGNITUDE(t) ((t) ~ (t) 0 < (t) -1)


#define STRTONUM(s, type, base, res, limit, loc)			\
  {									\
    type sum = 0;							\
    									\
    for (; *s; s++)							\
      {									\
	type x;								\
									\
	if ('0' <= *s && *s <= '9')					\
	  x = sum * base + *s - '0';					\
	else if (base == 16 && 'a' <= *s && *s <= 'f')			\
	  x = sum * base + *s - 'a';					\
	else if (base == 16 && 'A' <= *s && *s <= 'F')			\
	  x = sum * base + *s - 'A';					\
	else								\
	  break;							\
	if (x <= sum)							\
	  {								\
	    grecs_error(loc, 0, _("numeric overflow"));		        \
	    return 1;							\
	  }								\
	else if (limit && x > limit)					\
	  {								\
	    grecs_error(loc, 0, _("value out of allowed range"));	\
	    return 1;							\
	  }								\
	sum = x;							\
      }									\
    res = sum;								\
  }

#define STRxTONUM(s, type, res, limit, loc)				\
  {									\
    int base;								\
    if (*s == '0')							\
      {									\
	s++;								\
	if (*s == 0)							\
	  base = 10;							\
	else if (*s == 'x' || *s == 'X')				\
	  {								\
	    s++;							\
	    base = 16;							\
	  }								\
	else								\
	  base = 8;							\
      } else								\
      base = 10;							\
    STRTONUM(s, type, base, res, limit, loc);				\
  }

#define GETUNUM(str, type, res, loc)					\
  {									\
    type tmpres;							\
    const char *s = str;						\
    STRxTONUM(s, type, tmpres, 0, loc);				\
    if (*s)								\
      {									\
	grecs_error(loc, 0, _("not a number (stopped near `%s')"),	\
		     s);						\
	return 1;							\
      }									\
    res = tmpres;							\
  }

#define GETSNUM(str, type, res, loc)					\
  {									\
    unsigned type tmpres;						\
    const char *s = str;						\
    int sign;							        \
    unsigned type limit;						\
    									\
    if (*s == '-')							\
      {									\
	sign = 1;							\
	s++;								\
	limit = TYPE_MINIMUM(type);					\
	limit = - limit;						\
      }									\
    else								\
      {									\
	sign = 0;							\
	limit = TYPE_MAXIMUM(type);					\
      }									\
    									\
    STRxTONUM(s, unsigned type, tmpres, limit, loc);			\
    if (*s)								\
      {									\
	grecs_error(loc, 0, _("not a number (stopped near `%s')"), s);	\
	return 1;							\
      }									\
    res = sign ? - tmpres : tmpres;					\
  }


int
grecs_string_convert(void *target, enum grecs_data_type type,
		      const char *string, grecs_locus_t const *locus)
{
  switch (type) {
  case grecs_type_void:
	  abort();
	    
  case grecs_type_string:
	  *(const char**)target = grecs_strdup(string);
	  break;
	    
  case grecs_type_short:
	  GETUNUM(string, short, *(short*)target, locus);
	  break;
	    
  case grecs_type_ushort:
	  GETUNUM(string, unsigned short, *(unsigned short*)target, locus);
	  break;
	    
  case grecs_type_bool:
	  return string_to_bool(string, (int*)target, locus);
	    
  case grecs_type_int:
	  GETSNUM(string, int, *(int*)target, locus);
	  break;
	    
  case grecs_type_uint:
	  GETUNUM(string, unsigned int, *(unsigned int*)target, locus);
	  break;
      
  case grecs_type_long:
	  GETSNUM(string, long, *(long*)target, locus);
	  break;
	    
  case grecs_type_ulong:
	  GETUNUM(string, unsigned long, *(unsigned long*)target, locus);
	  break;
	    
  case grecs_type_size:
	  GETUNUM(string, size_t, *(size_t*)target, locus);
	  break;
	  /*FIXME	    
  case grecs_type_off:
          GETSNUM(string, off_t, *(off_t*)target, locus);
	  break;
	  */    
  case grecs_type_time:
	  /*FIXME: Use getdate */
	  GETUNUM(string, time_t, *(time_t*)target, locus);
	  break;

  case grecs_type_ipv4:
	  if (inet_aton(string, (struct in_addr *)target)) {
		  grecs_error(locus, 0, _("%s: not a valid IP address"),
			       string);
		  return 1;
	  }
	  break;
	  
  case grecs_type_host:
	  if (string_to_host((struct in_addr *)target, string, locus)) {
		  grecs_error(locus, 0,
			       _("%s: not a valid IP address or hostname"),
			       string);
		  return 1;
	  }
	  break;    
	    
  case grecs_type_sockaddr:
	  return string_to_sockaddr((struct grecs_sockaddr *)target, string,
				    locus);
      
      /* FIXME: */
  case grecs_type_cidr:
	  grecs_error(locus, 0,
		       _("INTERNAL ERROR at %s:%d"), __FILE__, __LINE__);
	  abort();
	    
  case grecs_type_section:
	  grecs_error(locus, 0,
		       _("invalid use of block statement"));
	  return 1;
  }
  return 0;
}

struct grecs_prop
{
	size_t size;
	int (*cmp)(const void *, const void *);
};

static int
string_cmp(const void *elt1, const void *elt2)
{
	return strcmp((const char *)elt1,(const char *)elt2);
}

#define __grecs_name_cat__(a,b) a ## b
#define NUMCMP(type) __grecs_name_cat__(type,_cmp)
#define __DECL_NUMCMP(type,ctype)			\
  static int						\
  NUMCMP(type)(const void *elt1, const void *elt2)	\
  {							\
    return memcmp(elt1, elt2, sizeof(ctype));		\
  }
#define DECL_NUMCMP(type) __DECL_NUMCMP(type,type)

DECL_NUMCMP(short)
DECL_NUMCMP(int)
DECL_NUMCMP(long)
DECL_NUMCMP(size_t)
/*FIXME DECL_NUMCMP(off_t)*/
DECL_NUMCMP(time_t)
__DECL_NUMCMP(in_addr, struct in_addr)
__DECL_NUMCMP(grecs_sockaddr, struct grecs_sockaddr)

struct grecs_prop grecs_prop_tab[] = {
	{ 0, NULL },                                  /* grecs_type_void */
	{ sizeof(char*), string_cmp },                /* grecs_type_string */
	{ sizeof(short), NUMCMP(short) },            /* grecs_type_short */
	{ sizeof(unsigned short), NUMCMP(short) },   /* grecs_type_ushort */
	{ sizeof(int), NUMCMP(int) },                /* grecs_type_int */
	{ sizeof(unsigned int), NUMCMP(int) },       /* grecs_type_uint */
	{ sizeof(long), NUMCMP(long) },              /* grecs_type_long */
	{ sizeof(unsigned long), NUMCMP(long) },     /* grecs_type_ulong */
	{ sizeof(size_t), NUMCMP(size_t) },          /* grecs_type_size */
#if 0
	FIXME
	{ sizeof(off_t), NUMCMP(off_t) },            /* grecs_type_off */
#endif
	{ sizeof(time_t), NUMCMP(time_t) },          /* grecs_type_time */
	{ sizeof(int), NUMCMP(int) },                /* grecs_type_bool */
	{ sizeof(struct in_addr), NUMCMP(in_addr) }, /* grecs_type_ipv4 */
	{ 0, NULL },                           /* FIXME: grecs_type_cidr */
	{ sizeof(struct in_addr), NUMCMP(in_addr) }, /* grecs_type_host */ 
	{ sizeof(struct grecs_sockaddr), NUMCMP(grecs_sockaddr) },
                                               	      /* grecs_type_sockaddr */
	{ 0, NULL }                                   /* grecs_type_section */
};
#define grecs_prop_count \
 (sizeof(grecs_prop_tab) / sizeof(grecs_prop_tab[0]))

void
grecs_process_ident(struct grecs_keyword *kwp, grecs_value_t *value,
		     void *base, grecs_locus_t *locus)
{
	void *target;

	if (!kwp)
		return;
	
	target = target_ptr(kwp, (char *) base);
	
	if (kwp->callback)
		kwp->callback(grecs_callback_set_value,
			      locus,
			      target,
			      value,
			      &kwp->callback_data);
	else if (kwp->type == grecs_type_void || target == NULL)
		return;
	else if (!value) {
		grecs_error(locus, 0, "%s has no value", kwp->ident);
		return;
	} else if (value->type == GRECS_TYPE_ARRAY) {
		grecs_error(locus, 0,
			     _("too many arguments to `%s'; missing semicolon?"),
			     kwp->ident);
		return;
	} else if (value->type == GRECS_TYPE_LIST) {
		if (kwp->flags & GRECS_LIST) {
			struct grecs_list_entry *ep;
			enum grecs_data_type type = kwp->type;
			int num = 1;
			struct grecs_list *list;
			size_t size;
			
			if (type >= grecs_prop_count
			    || (size = grecs_prop_tab[type].size) == 0) {
				grecs_error(locus, 0,
					     _("INTERNAL ERROR at %s:%d: "
					       "unhandled data type %d"),
					     __FILE__, __LINE__, type);
				abort();
			}

			list = _grecs_simple_list_create(
				     type == grecs_type_string);
			list->cmp = grecs_prop_tab[type].cmp;
				
			for (ep = value->v.list->head; ep; ep = ep->next) {
				const grecs_value_t *vp = ep->data;

				if (vp->type != GRECS_TYPE_STRING)
					grecs_error(&vp->locus, 0,
			     _("%s: incompatible data type in list item #%d"),
						     kwp->ident, num);
				else if (type == grecs_type_string)
					grecs_list_append(list,
							  grecs_strdup(vp->v.string));
				else {
					void *ptr = grecs_malloc(size);
					if (grecs_string_convert(ptr,
								 type,
								 vp->v.string,
								 &vp->locus)
					    == 0) 
						grecs_list_append(list, ptr);
					else
						grecs_free(ptr);
				}
			}
			*(struct grecs_list**)target = list;
		} else {
			grecs_error(locus, 0,
				     _("incompatible data type for `%s'"),
				     kwp->ident);
			return;
		}
	} else if (kwp->flags & GRECS_LIST) {
		struct grecs_list *list;
		enum grecs_data_type type = kwp->type;
		size_t size;
		void *ptr;
      
		if (type >= grecs_prop_count
		    || (size = grecs_prop_tab[type].size) == 0) {
			grecs_error(locus, 0,
				     _("INTERNAL ERROR at %s:%d: unhandled data type %d"),
				     __FILE__, __LINE__, type);
			abort();
		}
		
		list = _grecs_simple_list_create(1);
		list->cmp = grecs_prop_tab[type].cmp;
		if (type == grecs_type_string)
			grecs_list_append(list,
					  grecs_strdup(value->v.string));
		else {
			ptr = grecs_malloc(size);
			if (grecs_string_convert(ptr, type,
						 value->v.string,
						 &value->locus)) {
				grecs_free(ptr);
				grecs_list_free(list);
				return;
			}
			grecs_list_append(list, ptr);
		}
		*(struct grecs_list**)target = list;
	} else
		grecs_string_convert(target, kwp->type,
				     value->v.string,
				     &value->locus);
}


struct nodeproc_closure {
	struct grecs_keyword *cursect;
	struct grecs_list *sections;
	int flags;
};
#define CURRENT_BASE(clos) \
	((char*)((clos)->cursect ? (clos)->cursect->callback_data : NULL))

static void
stmt_begin(struct nodeproc_closure *clos,
	   struct grecs_keyword *kwp, struct grecs_node *node)
{
	void *target;
	
	grecs_list_push(clos->sections, clos->cursect);
	if (kwp) {
		target = target_ptr(kwp, CURRENT_BASE(clos));
		clos->cursect = kwp;
		if (kwp->callback) {
			if (kwp->callback(grecs_callback_section_begin,
					  &node->locus,
					  target,
					  node->v.value,
					  &kwp->callback_data))
				clos->cursect = &fake;
		} else
			kwp->callback_data = target;
	} else
		/* install an "ignore-all" section */
		clos->cursect = kwp;
}

static void
stmt_end(struct nodeproc_closure *clos, struct grecs_node *node)
{
	grecs_callback_fn callback = NULL;
	void *dataptr = NULL;
	struct grecs_keyword *kwp = clos->cursect;
	
	if (clos->cursect && clos->cursect->callback) {
		callback = clos->cursect->callback;
		dataptr = &clos->cursect->callback_data;
	}

	clos->cursect = (struct grecs_keyword *)grecs_list_pop(clos->sections);
	if (!clos->cursect)
		abort();
	if (callback)
		callback(grecs_callback_section_end,
			 &node->locus,
			 kwp ? target_ptr(kwp, CURRENT_BASE(clos)) : NULL,
			 NULL,
			 dataptr);
	if (kwp)
		kwp->callback_data = NULL;
}

static enum grecs_tree_recurse_res
nodeproc(enum grecs_tree_recurse_op op, struct grecs_node *node, void *data)
{
	struct nodeproc_closure *clos = data;
	struct grecs_keyword *kwp;
	
	switch (op) {
	case grecs_tree_recurse_set:
		kwp = find_keyword(clos->cursect, node->ident);
		if (!kwp) {
			grecs_error(&node->idloc, 0, _("Unknown keyword"));
			return grecs_tree_recurse_skip;
		}
		grecs_process_ident(kwp, node->v.value, CURRENT_BASE(clos),
				    &node->idloc);
		break;
		
	case grecs_tree_recurse_pre:
		kwp = find_keyword(clos->cursect, node->ident);
		if (!kwp) {
			grecs_error(&node->locus, 0, _("Unknown keyword"));
			return grecs_tree_recurse_skip;
		}
		stmt_begin(clos, kwp, node);
		break;
		
	case grecs_tree_recurse_post:
		stmt_end(clos, node);
		break;
	}
	return grecs_tree_recurse_ok;
}

int
grecs_tree_process(struct grecs_node *node, struct grecs_keyword *kwd)
{
	int rc;
	struct nodeproc_closure clos;
	struct grecs_keyword config_keywords;

	memset(&config_keywords, 0, sizeof(config_keywords));
	config_keywords.kwd = kwd;
	clos.cursect = &config_keywords;
	clos.sections = grecs_list_create();
	if (node->type == grecs_node_root)
		node = node->down;
	rc = grecs_tree_recurse(node, nodeproc, &clos);
	grecs_list_free(clos.sections);
	return rc;
}


int
grecs_node_eq(struct grecs_node *a, struct grecs_node *b)
{
	if (a->type != b->type)
		return 1;
	if (a->type == grecs_node_root)
		return 0;
	if (strcmp(a->ident, b->ident))
		return 1;
	if (a->type == grecs_node_block &&
	    !grecs_value_eq(a->v.value, b->v.value))
		return 1;
	return 0;
}

static void
free_value_entry(void *ptr)
{
	struct grecs_value *v = ptr;
	grecs_value_free(v);
}

struct grecs_list *
grecs_value_list_create()
{
	struct grecs_list *list = grecs_list_create();
	list->free_entry = free_value_entry;
	return list;
}

static void
value_to_list(struct grecs_value *val)
{
	struct grecs_list *list;
	int i;
	
	if (val->type == GRECS_TYPE_LIST)
		return;
	list = grecs_value_list_create();
	switch (val->type) {
	case GRECS_TYPE_STRING:
		grecs_list_append(list, grecs_value_ptr_from_static(val));
		break;

	case GRECS_TYPE_ARRAY:
		for (i = 0; i < val->v.arg.c; i++)
			grecs_list_append(list, val->v.arg.v[i]);
	}
	val->type = GRECS_TYPE_LIST;
	val->v.list = list;
}

static void
value_to_array(struct grecs_value *val)
{
	if (val->type == GRECS_TYPE_ARRAY)
		return;
	else {
		struct grecs_value **vp;
		vp = grecs_calloc(1, sizeof(*vp));
		vp[0] = grecs_value_ptr_from_static(val);
		val->type = GRECS_TYPE_ARRAY;
		val->v.arg.c = 1;
		val->v.arg.v = vp;
	}
}

static void
array_add(struct grecs_value *vx, struct grecs_value *vy)
{
	size_t i;
	
	vx->v.arg.v = grecs_realloc(vx->v.arg.v,
				    (vx->v.arg.c + vy->v.arg.c) *
				       sizeof(vx->v.arg.v[0]));
	for (i = 0; i < vy->v.arg.c; i++)
		vx->v.arg.v[i + vy->v.arg.c] = vy->v.arg.v[i];
	grecs_free(vy->v.arg.v);
	vy->v.arg.v = NULL;
	vy->v.arg.c = 0;
}

static void
node_aggregate_stmt(struct grecs_node *dst, struct grecs_node *src, int islist)
{
	if (islist) {
		struct grecs_list *t;
		/* Coerce both arguments to lists */
		value_to_list(dst->v.value);
		value_to_list(src->v.value);
		/* Aggregate two lists in order */
		grecs_list_add(src->v.value->v.list, dst->v.value->v.list);
		/* Swap them */
		t = dst->v.value->v.list;
		dst->v.value->v.list = src->v.value->v.list;
		src->v.value->v.list = t;
	} else {
		value_to_array(dst->v.value);
		value_to_array(src->v.value);
		array_add(dst->v.value, src->v.value);
	}
}

static void
node_merge_stmt(struct grecs_node *to_node, struct grecs_node *from_node,
		struct grecs_keyword *kwp, int flags)
{
	if (kwp &&
	    (flags & GRECS_AGGR) ^ (kwp->flags & GRECS_AGGR) &&
	    ((kwp->flags & GRECS_LIST) || kwp->callback))
		node_aggregate_stmt(to_node, from_node,
				    kwp->flags & GRECS_LIST);
	else {
		grecs_value_free(from_node->v.value);
		from_node->v.value = NULL;
	}
}

static void
node_merge_block(struct grecs_node *to_node, struct grecs_node *from_node,
		 struct grecs_keyword *kwp)
{
	struct grecs_node *sp;

	if (!from_node->down)
		return;
	for (sp = from_node->down; ; sp = sp->next) {
		sp->up = to_node;
		if (!sp->next)
			break;
	}
	sp->next = to_node->down;
	to_node->down = from_node->down;
	from_node->down = NULL;
}

static int
node_reduce(struct grecs_node *node, struct grecs_keyword *kwp, int flags)
{
	struct grecs_node *p;

	for (p = node->next; p; p = p->next)
		if (grecs_node_eq(p, node) == 0)
			break;
	if (p) {
		switch (node->type) {
		case grecs_node_root:
			return;
		case grecs_node_stmt:
			node_merge_stmt(p, node, kwp, flags);
			break;
		case grecs_node_block:
			node_merge_block(p, node, kwp);
			break;
		}
		grecs_node_unlink(node);
		grecs_node_free(node);
		return 1;
	}
	return 0;
}

static enum grecs_tree_recurse_res
reduceproc(enum grecs_tree_recurse_op op, struct grecs_node *node, void *data)
{
	struct nodeproc_closure *clos = data;
	
	if (op == grecs_tree_recurse_post) {
		if (clos->sections)
			clos->cursect = (struct grecs_keyword *)
				           grecs_list_pop(clos->sections);
	} else {
		struct grecs_keyword *kwp = NULL;
		if (clos->cursect) {
			kwp = find_keyword(clos->cursect, node->ident);
			if (!kwp) {
				grecs_error(&node->locus, 0,
					    _("%s: unknown keyword"),
					    node->ident);
				return grecs_tree_recurse_skip;
			}
			if (kwp->flags & GRECS_INAC)
				return grecs_tree_recurse_skip;
			if (!(kwp->flags & GRECS_MULT) &&
			    node_reduce(node, kwp, clos->flags))
				return grecs_tree_recurse_skip;
			if (op == grecs_tree_recurse_pre) {
				grecs_list_push(clos->sections, clos->cursect);
				clos->cursect = kwp;
			} 
		} else if (node_reduce(node, kwp, clos->flags))
			return grecs_tree_recurse_skip;
	}
	return grecs_tree_recurse_ok;
}
		
int
grecs_tree_reduce(struct grecs_node *node, struct grecs_keyword *kwd,
		  int flags)
{
	int rc;
	struct nodeproc_closure clos;
	struct grecs_keyword config_keywords;

	memset(&config_keywords, 0, sizeof(config_keywords));
	config_keywords.kwd = kwd;
	if (kwd) {
		clos.cursect = &config_keywords;
		clos.sections = grecs_list_create();
	} else {
		clos.cursect = NULL;
		clos.sections = NULL;
	}
	clos.flags = flags;
	rc = grecs_tree_recurse(node->down, reduceproc, &clos);
	grecs_list_free(clos.sections);
	return rc;
}


struct grecs_node *
grecs_tree_first_node(struct grecs_node *tree)
{
	if (tree->type != grecs_node_root) {
		errno = EINVAL;
		return NULL;
	}
	errno = 0;
	return tree->down;
}

struct grecs_node *
grecs_next_node(struct grecs_node *node)
{
	if (!node)
		return NULL;
	if (node->down)
		return node->down;
	while (!node->next) {
		node = node->up;
		if (!node || node->type == grecs_node_root)
			return NULL;
	}
	return node->next;
}
