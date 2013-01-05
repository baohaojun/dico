/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with grecs_grecs_ or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum grecs_grecs_tokentype {
     STRING = 258,
     QSTRING = 259,
     MSTRING = 260,
     IDENT = 261
   };
#endif
/* Tokens.  */
#define STRING 258
#define QSTRING 259
#define MSTRING 260
#define IDENT 261




/* Copy the first part of user declarations.  */
#line 1 "grecs-gram.y"

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
#include <grecs-locus.h>
#include <grecs-gram.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
  
int grecs_grecs_lex(void);
int grecs_grecs_error(char *s);

static struct grecs_node *parse_tree;


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 38 "grecs-gram.y"
{
	char *string;
	grecs_value_t svalue, *pvalue;
	struct grecs_list *list;
	struct grecs_node *node;
	grecs_locus_t locus;
	struct { struct grecs_node *head, *tail; } node_list;
}
/* Line 187 of yacc.c.  */
#line 151 "grecs-gram.c"
	YYSTYPE;
# define grecs_grecs_stype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define grecs_grecs_ltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 176 "grecs-gram.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 grecs_grecs_type_uint8;
#else
typedef unsigned char grecs_grecs_type_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 grecs_grecs_type_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char grecs_grecs_type_int8;
#else
typedef short int grecs_grecs_type_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 grecs_grecs_type_uint16;
#else
typedef unsigned short int grecs_grecs_type_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 grecs_grecs_type_int16;
#else
typedef short int grecs_grecs_type_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined grecs_grecs_overflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined grecs_grecs_overflow || YYERROR_VERBOSE */


#if (! defined grecs_grecs_overflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union grecs_grecs_alloc
{
  grecs_grecs_type_int16 grecs_grecs_ss;
  YYSTYPE grecs_grecs_vs;
    YYLTYPE grecs_grecs_ls;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union grecs_grecs_alloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (grecs_grecs_type_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T grecs_grecs_i;				\
	  for (grecs_grecs_i = 0; grecs_grecs_i < (Count); grecs_grecs_i++)	\
	    (To)[grecs_grecs_i] = (From)[grecs_grecs_i];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T grecs_grecs_newbytes;						\
	YYCOPY (&grecs_grecs_ptr->Stack, Stack, grecs_grecs_size);				\
	Stack = &grecs_grecs_ptr->Stack;						\
	grecs_grecs_newbytes = grecs_grecs_stacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	grecs_grecs_ptr += grecs_grecs_newbytes / sizeof (*grecs_grecs_ptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  22
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   39

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  13
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  32
/* YYNRULES -- Number of states.  */
#define YYNSTATES  39

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   261

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? grecs_grecs_translate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const grecs_grecs_type_uint8 grecs_grecs_translate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      10,    11,     2,     2,    12,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     7,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     8,     2,     9,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const grecs_grecs_type_uint8 grecs_grecs_prhs[] =
{
       0,     0,     3,     5,     6,     8,    10,    13,    15,    17,
      21,    24,    31,    32,    34,    36,    38,    41,    43,    45,
      47,    49,    51,    53,    55,    57,    60,    63,    67,    72,
      74,    78,    79
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const grecs_grecs_type_int8 grecs_grecs_rhs[] =
{
      14,     0,    -1,    15,    -1,    -1,    16,    -1,    17,    -1,
      16,    17,    -1,    18,    -1,    19,    -1,     6,    21,     7,
      -1,     6,     7,    -1,     6,    20,     8,    16,     9,    29,
      -1,    -1,    21,    -1,    22,    -1,    23,    -1,    22,    23,
      -1,    24,    -1,    27,    -1,     5,    -1,     3,    -1,     6,
      -1,    25,    -1,    26,    -1,     4,    -1,    26,     4,    -1,
      10,    11,    -1,    10,    28,    11,    -1,    10,    28,    12,
      11,    -1,    23,    -1,    28,    12,    23,    -1,    -1,     7,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const grecs_grecs_type_uint8 grecs_grecs_rline[] =
{
       0,    58,    58,    68,    71,    77,    81,    87,    88,    91,
      99,   108,   120,   123,   126,   150,   155,   161,   167,   173,
     181,   182,   183,   186,   201,   206,   213,   217,   221,   227,
     232,   239,   240
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const grecs_grecs_tname[] =
{
  "$end", "error", "$undefined", "STRING", "QSTRING", "MSTRING", "IDENT",
  "';'", "'{'", "'}'", "'('", "')'", "','", "$accept", "input",
  "maybe_stmtlist", "stmtlist", "stmt", "simple", "block", "tag",
  "vallist", "vlist", "value", "string", "slist", "slist0", "list",
  "values", "opt_sc", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const grecs_grecs_type_uint16 grecs_grecs_toknum[] =
{
       0,   256,   257,   258,   259,   260,   261,    59,   123,   125,
      40,    41,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const grecs_grecs_type_uint8 grecs_grecs_r1[] =
{
       0,    13,    14,    15,    15,    16,    16,    17,    17,    18,
      18,    19,    20,    20,    21,    22,    22,    23,    23,    23,
      24,    24,    24,    25,    26,    26,    27,    27,    27,    28,
      28,    29,    29
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const grecs_grecs_type_uint8 grecs_grecs_r2[] =
{
       0,     2,     1,     0,     1,     1,     2,     1,     1,     3,
       2,     6,     0,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     3,     4,     1,
       3,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const grecs_grecs_type_uint8 grecs_grecs_defact[] =
{
       3,    12,     0,     2,     4,     5,     7,     8,    20,    24,
      19,    21,    10,     0,     0,    13,    14,    15,    17,    22,
      23,    18,     1,     6,    26,    29,     0,     0,     9,    16,
      25,    27,     0,     0,    28,    30,    31,    32,    11
};

/* YYDEFGOTO[NTERM-NUM].  */
static const grecs_grecs_type_int8 grecs_grecs_defgoto[] =
{
      -1,     2,     3,     4,     5,     6,     7,    14,    15,    16,
      17,    18,    19,    20,    21,    26,    38
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -13
static const grecs_grecs_type_int8 grecs_grecs_pact[] =
{
       3,    20,    10,   -13,     3,   -13,   -13,   -13,   -13,   -13,
     -13,   -13,   -13,     2,    27,     4,    28,   -13,   -13,   -13,
      14,   -13,   -13,   -13,   -13,   -13,    -9,     3,   -13,   -13,
     -13,   -13,    11,    30,   -13,   -13,    12,   -13,   -13
};

/* YYPGOTO[NTERM-NUM].  */
static const grecs_grecs_type_int8 grecs_grecs_pgoto[] =
{
     -13,   -13,   -13,     1,    -4,   -13,   -13,   -13,   -13,   -13,
     -12,   -13,   -13,   -13,   -13,   -13,   -13
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const grecs_grecs_type_uint8 grecs_grecs_table[] =
{
      23,    25,    31,    32,    29,     8,     9,    10,    11,     1,
      22,    28,    13,    24,     8,     9,    10,    11,    30,    37,
      35,    13,    34,     8,     9,    10,    11,    12,    33,    23,
      13,     8,     9,    10,    11,    27,     1,     0,    13,    36
};

static const grecs_grecs_type_int8 grecs_grecs_check[] =
{
       4,    13,    11,    12,    16,     3,     4,     5,     6,     6,
       0,     7,    10,    11,     3,     4,     5,     6,     4,     7,
      32,    10,    11,     3,     4,     5,     6,     7,    27,    33,
      10,     3,     4,     5,     6,     8,     6,    -1,    10,     9
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const grecs_grecs_type_uint8 grecs_grecs_stos[] =
{
       0,     6,    14,    15,    16,    17,    18,    19,     3,     4,
       5,     6,     7,    10,    20,    21,    22,    23,    24,    25,
      26,    27,     0,    17,    11,    23,    28,     8,     7,    23,
       4,    11,    12,    16,    11,    23,     9,     7,    29
};

#define grecs_grecs_errok		(grecs_grecs_errstatus = 0)
#define grecs_grecs_clearin	(grecs_grecs_char = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto grecs_grecs_acceptlab
#define YYABORT		goto grecs_grecs_abortlab
#define YYERROR		goto grecs_grecs_errorlab


/* Like YYERROR except do call grecs_grecs_error.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto grecs_grecs_errlab

#define YYRECOVERING()  (!!grecs_grecs_errstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (grecs_grecs_char == YYEMPTY && grecs_grecs_len == 1)				\
    {								\
      grecs_grecs_char = (Token);						\
      grecs_grecs_lval = (Value);						\
      grecs_grecs_token = YYTRANSLATE (grecs_grecs_char);				\
      YYPOPSTACK (1);						\
      goto grecs_grecs_backup;						\
    }								\
  else								\
    {								\
      grecs_grecs_error (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `grecs_grecs_lex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX grecs_grecs_lex (YYLEX_PARAM)
#else
# define YYLEX grecs_grecs_lex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (grecs_grecs_debug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (grecs_grecs_debug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      grecs_grecs__symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
grecs_grecs__symbol_value_print (FILE *grecs_grecs_output, int grecs_grecs_type, YYSTYPE const * const grecs_grecs_valuep, YYLTYPE const * const grecs_grecs_locationp)
#else
static void
grecs_grecs__symbol_value_print (grecs_grecs_output, grecs_grecs_type, grecs_grecs_valuep, grecs_grecs_locationp)
    FILE *grecs_grecs_output;
    int grecs_grecs_type;
    YYSTYPE const * const grecs_grecs_valuep;
    YYLTYPE const * const grecs_grecs_locationp;
#endif
{
  if (!grecs_grecs_valuep)
    return;
  YYUSE (grecs_grecs_locationp);
# ifdef YYPRINT
  if (grecs_grecs_type < YYNTOKENS)
    YYPRINT (grecs_grecs_output, grecs_grecs_toknum[grecs_grecs_type], *grecs_grecs_valuep);
# else
  YYUSE (grecs_grecs_output);
# endif
  switch (grecs_grecs_type)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
grecs_grecs__symbol_print (FILE *grecs_grecs_output, int grecs_grecs_type, YYSTYPE const * const grecs_grecs_valuep, YYLTYPE const * const grecs_grecs_locationp)
#else
static void
grecs_grecs__symbol_print (grecs_grecs_output, grecs_grecs_type, grecs_grecs_valuep, grecs_grecs_locationp)
    FILE *grecs_grecs_output;
    int grecs_grecs_type;
    YYSTYPE const * const grecs_grecs_valuep;
    YYLTYPE const * const grecs_grecs_locationp;
#endif
{
  if (grecs_grecs_type < YYNTOKENS)
    YYFPRINTF (grecs_grecs_output, "token %s (", grecs_grecs_tname[grecs_grecs_type]);
  else
    YYFPRINTF (grecs_grecs_output, "nterm %s (", grecs_grecs_tname[grecs_grecs_type]);

  YY_LOCATION_PRINT (grecs_grecs_output, *grecs_grecs_locationp);
  YYFPRINTF (grecs_grecs_output, ": ");
  grecs_grecs__symbol_value_print (grecs_grecs_output, grecs_grecs_type, grecs_grecs_valuep, grecs_grecs_locationp);
  YYFPRINTF (grecs_grecs_output, ")");
}

/*------------------------------------------------------------------.
| grecs_grecs__stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
grecs_grecs__stack_print (grecs_grecs_type_int16 *bottom, grecs_grecs_type_int16 *top)
#else
static void
grecs_grecs__stack_print (bottom, top)
    grecs_grecs_type_int16 *bottom;
    grecs_grecs_type_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (grecs_grecs_debug)							\
    grecs_grecs__stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
grecs_grecs__reduce_print (YYSTYPE *grecs_grecs_vsp, YYLTYPE *grecs_grecs_lsp, int grecs_grecs_rule)
#else
static void
grecs_grecs__reduce_print (grecs_grecs_vsp, grecs_grecs_lsp, grecs_grecs_rule)
    YYSTYPE *grecs_grecs_vsp;
    YYLTYPE *grecs_grecs_lsp;
    int grecs_grecs_rule;
#endif
{
  int grecs_grecs_nrhs = grecs_grecs_r2[grecs_grecs_rule];
  int grecs_grecs_i;
  unsigned long int grecs_grecs_lno = grecs_grecs_rline[grecs_grecs_rule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     grecs_grecs_rule - 1, grecs_grecs_lno);
  /* The symbols being reduced.  */
  for (grecs_grecs_i = 0; grecs_grecs_i < grecs_grecs_nrhs; grecs_grecs_i++)
    {
      fprintf (stderr, "   $%d = ", grecs_grecs_i + 1);
      grecs_grecs__symbol_print (stderr, grecs_grecs_rhs[grecs_grecs_prhs[grecs_grecs_rule] + grecs_grecs_i],
		       &(grecs_grecs_vsp[(grecs_grecs_i + 1) - (grecs_grecs_nrhs)])
		       , &(grecs_grecs_lsp[(grecs_grecs_i + 1) - (grecs_grecs_nrhs)])		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (grecs_grecs_debug)				\
    grecs_grecs__reduce_print (grecs_grecs_vsp, grecs_grecs_lsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int grecs_grecs_debug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef grecs_grecs_strlen
#  if defined __GLIBC__ && defined _STRING_H
#   define grecs_grecs_strlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
grecs_grecs_strlen (const char *grecs_grecs_str)
#else
static YYSIZE_T
grecs_grecs_strlen (grecs_grecs_str)
    const char *grecs_grecs_str;
#endif
{
  YYSIZE_T grecs_grecs_len;
  for (grecs_grecs_len = 0; grecs_grecs_str[grecs_grecs_len]; grecs_grecs_len++)
    continue;
  return grecs_grecs_len;
}
#  endif
# endif

# ifndef grecs_grecs_stpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define grecs_grecs_stpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
grecs_grecs_stpcpy (char *grecs_grecs_dest, const char *grecs_grecs_src)
#else
static char *
grecs_grecs_stpcpy (grecs_grecs_dest, grecs_grecs_src)
    char *grecs_grecs_dest;
    const char *grecs_grecs_src;
#endif
{
  char *grecs_grecs_d = grecs_grecs_dest;
  const char *grecs_grecs_s = grecs_grecs_src;

  while ((*grecs_grecs_d++ = *grecs_grecs_s++) != '\0')
    continue;

  return grecs_grecs_d - 1;
}
#  endif
# endif

# ifndef grecs_grecs_tnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for grecs_grecs_error.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from grecs_grecs_tname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
grecs_grecs_tnamerr (char *grecs_grecs_res, const char *grecs_grecs_str)
{
  if (*grecs_grecs_str == '"')
    {
      YYSIZE_T grecs_grecs_n = 0;
      char const *grecs_grecs_p = grecs_grecs_str;

      for (;;)
	switch (*++grecs_grecs_p)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++grecs_grecs_p != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (grecs_grecs_res)
	      grecs_grecs_res[grecs_grecs_n] = *grecs_grecs_p;
	    grecs_grecs_n++;
	    break;

	  case '"':
	    if (grecs_grecs_res)
	      grecs_grecs_res[grecs_grecs_n] = '\0';
	    return grecs_grecs_n;
	  }
    do_not_strip_quotes: ;
    }

  if (! grecs_grecs_res)
    return grecs_grecs_strlen (grecs_grecs_str);

  return grecs_grecs_stpcpy (grecs_grecs_res, grecs_grecs_str) - grecs_grecs_res;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
grecs_grecs_syntax_error (char *grecs_grecs_result, int grecs_grecs_state, int grecs_grecs_char)
{
  int grecs_grecs_n = grecs_grecs_pact[grecs_grecs_state];

  if (! (YYPACT_NINF < grecs_grecs_n && grecs_grecs_n <= YYLAST))
    return 0;
  else
    {
      int grecs_grecs_type = YYTRANSLATE (grecs_grecs_char);
      YYSIZE_T grecs_grecs_size0 = grecs_grecs_tnamerr (0, grecs_grecs_tname[grecs_grecs_type]);
      YYSIZE_T grecs_grecs_size = grecs_grecs_size0;
      YYSIZE_T grecs_grecs_size1;
      int grecs_grecs_size_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *grecs_grecs_arg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int grecs_grecs_x;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *grecs_grecs_fmt;
      char const *grecs_grecs_f;
      static char const grecs_grecs_unexpected[] = "syntax error, unexpected %s";
      static char const grecs_grecs_expecting[] = ", expecting %s";
      static char const grecs_grecs_or[] = " or %s";
      char grecs_grecs_format[sizeof grecs_grecs_unexpected
		    + sizeof grecs_grecs_expecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof grecs_grecs_or - 1))];
      char const *grecs_grecs_prefix = grecs_grecs_expecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int grecs_grecs_xbegin = grecs_grecs_n < 0 ? -grecs_grecs_n : 0;

      /* Stay within bounds of both grecs_grecs_check and grecs_grecs_tname.  */
      int grecs_grecs_checklim = YYLAST - grecs_grecs_n + 1;
      int grecs_grecs_xend = grecs_grecs_checklim < YYNTOKENS ? grecs_grecs_checklim : YYNTOKENS;
      int grecs_grecs_count = 1;

      grecs_grecs_arg[0] = grecs_grecs_tname[grecs_grecs_type];
      grecs_grecs_fmt = grecs_grecs_stpcpy (grecs_grecs_format, grecs_grecs_unexpected);

      for (grecs_grecs_x = grecs_grecs_xbegin; grecs_grecs_x < grecs_grecs_xend; ++grecs_grecs_x)
	if (grecs_grecs_check[grecs_grecs_x + grecs_grecs_n] == grecs_grecs_x && grecs_grecs_x != YYTERROR)
	  {
	    if (grecs_grecs_count == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		grecs_grecs_count = 1;
		grecs_grecs_size = grecs_grecs_size0;
		grecs_grecs_format[sizeof grecs_grecs_unexpected - 1] = '\0';
		break;
	      }
	    grecs_grecs_arg[grecs_grecs_count++] = grecs_grecs_tname[grecs_grecs_x];
	    grecs_grecs_size1 = grecs_grecs_size + grecs_grecs_tnamerr (0, grecs_grecs_tname[grecs_grecs_x]);
	    grecs_grecs_size_overflow |= (grecs_grecs_size1 < grecs_grecs_size);
	    grecs_grecs_size = grecs_grecs_size1;
	    grecs_grecs_fmt = grecs_grecs_stpcpy (grecs_grecs_fmt, grecs_grecs_prefix);
	    grecs_grecs_prefix = grecs_grecs_or;
	  }

      grecs_grecs_f = YY_(grecs_grecs_format);
      grecs_grecs_size1 = grecs_grecs_size + grecs_grecs_strlen (grecs_grecs_f);
      grecs_grecs_size_overflow |= (grecs_grecs_size1 < grecs_grecs_size);
      grecs_grecs_size = grecs_grecs_size1;

      if (grecs_grecs_size_overflow)
	return YYSIZE_MAXIMUM;

      if (grecs_grecs_result)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *grecs_grecs_p = grecs_grecs_result;
	  int grecs_grecs_i = 0;
	  while ((*grecs_grecs_p = *grecs_grecs_f) != '\0')
	    {
	      if (*grecs_grecs_p == '%' && grecs_grecs_f[1] == 's' && grecs_grecs_i < grecs_grecs_count)
		{
		  grecs_grecs_p += grecs_grecs_tnamerr (grecs_grecs_p, grecs_grecs_arg[grecs_grecs_i++]);
		  grecs_grecs_f += 2;
		}
	      else
		{
		  grecs_grecs_p++;
		  grecs_grecs_f++;
		}
	    }
	}
      return grecs_grecs_size;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
grecs_grecs_destruct (const char *grecs_grecs_msg, int grecs_grecs_type, YYSTYPE *grecs_grecs_valuep, YYLTYPE *grecs_grecs_locationp)
#else
static void
grecs_grecs_destruct (grecs_grecs_msg, grecs_grecs_type, grecs_grecs_valuep, grecs_grecs_locationp)
    const char *grecs_grecs_msg;
    int grecs_grecs_type;
    YYSTYPE *grecs_grecs_valuep;
    YYLTYPE *grecs_grecs_locationp;
#endif
{
  YYUSE (grecs_grecs_valuep);
  YYUSE (grecs_grecs_locationp);

  if (!grecs_grecs_msg)
    grecs_grecs_msg = "Deleting";
  YY_SYMBOL_PRINT (grecs_grecs_msg, grecs_grecs_type, grecs_grecs_valuep, grecs_grecs_locationp);

  switch (grecs_grecs_type)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int grecs_grecs_parse (void *YYPARSE_PARAM);
#else
int grecs_grecs_parse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int grecs_grecs_parse (void);
#else
int grecs_grecs_parse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int grecs_grecs_char;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE grecs_grecs_lval;

/* Number of syntax errors so far.  */
int grecs_grecs_nerrs;
/* Location data for the look-ahead symbol.  */
YYLTYPE grecs_grecs_lloc;



/*----------.
| grecs_grecs_parse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
grecs_grecs_parse (void *YYPARSE_PARAM)
#else
int
grecs_grecs_parse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
grecs_grecs_parse (void)
#else
int
grecs_grecs_parse ()

#endif
#endif
{
  
  int grecs_grecs_state;
  int grecs_grecs_n;
  int grecs_grecs_result;
  /* Number of tokens to shift before error messages enabled.  */
  int grecs_grecs_errstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int grecs_grecs_token = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char grecs_grecs_msgbuf[128];
  char *grecs_grecs_msg = grecs_grecs_msgbuf;
  YYSIZE_T grecs_grecs_msg_alloc = sizeof grecs_grecs_msgbuf;
#endif

  /* Three stacks and their tools:
     `grecs_grecs_ss': related to states,
     `grecs_grecs_vs': related to semantic values,
     `grecs_grecs_ls': related to locations.

     Refer to the stacks thru separate pointers, to allow grecs_grecs_overflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  grecs_grecs_type_int16 grecs_grecs_ssa[YYINITDEPTH];
  grecs_grecs_type_int16 *grecs_grecs_ss = grecs_grecs_ssa;
  grecs_grecs_type_int16 *grecs_grecs_ssp;

  /* The semantic value stack.  */
  YYSTYPE grecs_grecs_vsa[YYINITDEPTH];
  YYSTYPE *grecs_grecs_vs = grecs_grecs_vsa;
  YYSTYPE *grecs_grecs_vsp;

  /* The location stack.  */
  YYLTYPE grecs_grecs_lsa[YYINITDEPTH];
  YYLTYPE *grecs_grecs_ls = grecs_grecs_lsa;
  YYLTYPE *grecs_grecs_lsp;
  /* The locations where the error started and ended.  */
  YYLTYPE grecs_grecs_error_range[2];

#define YYPOPSTACK(N)   (grecs_grecs_vsp -= (N), grecs_grecs_ssp -= (N), grecs_grecs_lsp -= (N))

  YYSIZE_T grecs_grecs_stacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE grecs_grecs_val;
  YYLTYPE grecs_grecs_loc;

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int grecs_grecs_len = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  grecs_grecs_state = 0;
  grecs_grecs_errstatus = 0;
  grecs_grecs_nerrs = 0;
  grecs_grecs_char = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  grecs_grecs_ssp = grecs_grecs_ss;
  grecs_grecs_vsp = grecs_grecs_vs;
  grecs_grecs_lsp = grecs_grecs_ls;
#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  grecs_grecs_lloc.first_line   = grecs_grecs_lloc.last_line   = 1;
  grecs_grecs_lloc.first_column = grecs_grecs_lloc.last_column = 0;
#endif

  goto grecs_grecs_setstate;

/*------------------------------------------------------------.
| grecs_grecs_newstate -- Push a new state, which is found in grecs_grecs_state.  |
`------------------------------------------------------------*/
 grecs_grecs_newstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  grecs_grecs_ssp++;

 grecs_grecs_setstate:
  *grecs_grecs_ssp = grecs_grecs_state;

  if (grecs_grecs_ss + grecs_grecs_stacksize - 1 <= grecs_grecs_ssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T grecs_grecs_size = grecs_grecs_ssp - grecs_grecs_ss + 1;

#ifdef grecs_grecs_overflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *grecs_grecs_vs1 = grecs_grecs_vs;
	grecs_grecs_type_int16 *grecs_grecs_ss1 = grecs_grecs_ss;
	YYLTYPE *grecs_grecs_ls1 = grecs_grecs_ls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if grecs_grecs_overflow is a macro.  */
	grecs_grecs_overflow (YY_("memory exhausted"),
		    &grecs_grecs_ss1, grecs_grecs_size * sizeof (*grecs_grecs_ssp),
		    &grecs_grecs_vs1, grecs_grecs_size * sizeof (*grecs_grecs_vsp),
		    &grecs_grecs_ls1, grecs_grecs_size * sizeof (*grecs_grecs_lsp),
		    &grecs_grecs_stacksize);
	grecs_grecs_ls = grecs_grecs_ls1;
	grecs_grecs_ss = grecs_grecs_ss1;
	grecs_grecs_vs = grecs_grecs_vs1;
      }
#else /* no grecs_grecs_overflow */
# ifndef YYSTACK_RELOCATE
      goto grecs_grecs_exhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= grecs_grecs_stacksize)
	goto grecs_grecs_exhaustedlab;
      grecs_grecs_stacksize *= 2;
      if (YYMAXDEPTH < grecs_grecs_stacksize)
	grecs_grecs_stacksize = YYMAXDEPTH;

      {
	grecs_grecs_type_int16 *grecs_grecs_ss1 = grecs_grecs_ss;
	union grecs_grecs_alloc *grecs_grecs_ptr =
	  (union grecs_grecs_alloc *) YYSTACK_ALLOC (YYSTACK_BYTES (grecs_grecs_stacksize));
	if (! grecs_grecs_ptr)
	  goto grecs_grecs_exhaustedlab;
	YYSTACK_RELOCATE (grecs_grecs_ss);
	YYSTACK_RELOCATE (grecs_grecs_vs);
	YYSTACK_RELOCATE (grecs_grecs_ls);
#  undef YYSTACK_RELOCATE
	if (grecs_grecs_ss1 != grecs_grecs_ssa)
	  YYSTACK_FREE (grecs_grecs_ss1);
      }
# endif
#endif /* no grecs_grecs_overflow */

      grecs_grecs_ssp = grecs_grecs_ss + grecs_grecs_size - 1;
      grecs_grecs_vsp = grecs_grecs_vs + grecs_grecs_size - 1;
      grecs_grecs_lsp = grecs_grecs_ls + grecs_grecs_size - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) grecs_grecs_stacksize));

      if (grecs_grecs_ss + grecs_grecs_stacksize - 1 <= grecs_grecs_ssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", grecs_grecs_state));

  goto grecs_grecs_backup;

/*-----------.
| grecs_grecs_backup.  |
`-----------*/
grecs_grecs_backup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  grecs_grecs_n = grecs_grecs_pact[grecs_grecs_state];
  if (grecs_grecs_n == YYPACT_NINF)
    goto grecs_grecs_default;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (grecs_grecs_char == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      grecs_grecs_char = YYLEX;
    }

  if (grecs_grecs_char <= YYEOF)
    {
      grecs_grecs_char = grecs_grecs_token = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      grecs_grecs_token = YYTRANSLATE (grecs_grecs_char);
      YY_SYMBOL_PRINT ("Next token is", grecs_grecs_token, &grecs_grecs_lval, &grecs_grecs_lloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  grecs_grecs_n += grecs_grecs_token;
  if (grecs_grecs_n < 0 || YYLAST < grecs_grecs_n || grecs_grecs_check[grecs_grecs_n] != grecs_grecs_token)
    goto grecs_grecs_default;
  grecs_grecs_n = grecs_grecs_table[grecs_grecs_n];
  if (grecs_grecs_n <= 0)
    {
      if (grecs_grecs_n == 0 || grecs_grecs_n == YYTABLE_NINF)
	goto grecs_grecs_errlab;
      grecs_grecs_n = -grecs_grecs_n;
      goto grecs_grecs_reduce;
    }

  if (grecs_grecs_n == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (grecs_grecs_errstatus)
    grecs_grecs_errstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", grecs_grecs_token, &grecs_grecs_lval, &grecs_grecs_lloc);

  /* Discard the shifted token unless it is eof.  */
  if (grecs_grecs_char != YYEOF)
    grecs_grecs_char = YYEMPTY;

  grecs_grecs_state = grecs_grecs_n;
  *++grecs_grecs_vsp = grecs_grecs_lval;
  *++grecs_grecs_lsp = grecs_grecs_lloc;
  goto grecs_grecs_newstate;


/*-----------------------------------------------------------.
| grecs_grecs_default -- do the default action for the current state.  |
`-----------------------------------------------------------*/
grecs_grecs_default:
  grecs_grecs_n = grecs_grecs_defact[grecs_grecs_state];
  if (grecs_grecs_n == 0)
    goto grecs_grecs_errlab;
  goto grecs_grecs_reduce;


/*-----------------------------.
| grecs_grecs_reduce -- Do a reduction.  |
`-----------------------------*/
grecs_grecs_reduce:
  /* grecs_grecs_n is the number of a rule to reduce with.  */
  grecs_grecs_len = grecs_grecs_r2[grecs_grecs_n];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  grecs_grecs_val = grecs_grecs_vsp[1-grecs_grecs_len];

  /* Default location.  */
  YYLLOC_DEFAULT (grecs_grecs_loc, (grecs_grecs_lsp - grecs_grecs_len), grecs_grecs_len);
  YY_REDUCE_PRINT (grecs_grecs_n);
  switch (grecs_grecs_n)
    {
        case 2:
#line 59 "grecs-gram.y"
    {
		  parse_tree = grecs_node_create(grecs_node_root, &(grecs_grecs_lsp[(1) - (1)]));
		  parse_tree->v.texttab = grecs_text_table();
		  grecs_node_bind(parse_tree, (grecs_grecs_vsp[(1) - (1)].node), 1);
	  }
    break;

  case 3:
#line 68 "grecs-gram.y"
    {
		  (grecs_grecs_val.node) = NULL;
	  }
    break;

  case 4:
#line 72 "grecs-gram.y"
    {
		  (grecs_grecs_val.node) = (grecs_grecs_vsp[(1) - (1)].node_list).head;
	  }
    break;

  case 5:
#line 78 "grecs-gram.y"
    {
		  (grecs_grecs_val.node_list).head = (grecs_grecs_val.node_list).tail = (grecs_grecs_vsp[(1) - (1)].node);
	  }
    break;

  case 6:
#line 82 "grecs-gram.y"
    {
		  grecs_node_bind((grecs_grecs_vsp[(1) - (2)].node_list).tail, (grecs_grecs_vsp[(2) - (2)].node), 0);
	  }
    break;

  case 9:
#line 92 "grecs-gram.y"
    {
		  (grecs_grecs_val.node) = grecs_node_create_points(grecs_node_stmt,
						(grecs_grecs_lsp[(1) - (3)]).beg, (grecs_grecs_lsp[(2) - (3)]).end);
		  (grecs_grecs_val.node)->ident = (grecs_grecs_vsp[(1) - (3)].string);
		  (grecs_grecs_val.node)->idloc = (grecs_grecs_lsp[(1) - (3)]);
		  (grecs_grecs_val.node)->v.value = (grecs_grecs_vsp[(2) - (3)].pvalue);
	  }
    break;

  case 10:
#line 100 "grecs-gram.y"
    {
		  (grecs_grecs_val.node) = grecs_node_create(grecs_node_stmt, &(grecs_grecs_lsp[(1) - (2)]));
		  (grecs_grecs_val.node)->ident = (grecs_grecs_vsp[(1) - (2)].string);
		  (grecs_grecs_val.node)->idloc = (grecs_grecs_lsp[(1) - (2)]);
		  (grecs_grecs_val.node)->v.value = NULL;
	  }
    break;

  case 11:
#line 109 "grecs-gram.y"
    {
		  (grecs_grecs_val.node) = grecs_node_create_points(grecs_node_block,
						(grecs_grecs_lsp[(1) - (6)]).beg, (grecs_grecs_lsp[(5) - (6)]).end);
		  (grecs_grecs_val.node)->ident = (grecs_grecs_vsp[(1) - (6)].string);
		  (grecs_grecs_val.node)->idloc = (grecs_grecs_lsp[(1) - (6)]);
		  (grecs_grecs_val.node)->v.value = (grecs_grecs_vsp[(2) - (6)].pvalue);
		  grecs_node_bind((grecs_grecs_val.node), (grecs_grecs_vsp[(4) - (6)].node_list).head, 1);
	  }
    break;

  case 12:
#line 120 "grecs-gram.y"
    {
		  (grecs_grecs_val.pvalue) = NULL;
	  }
    break;

  case 14:
#line 127 "grecs-gram.y"
    {
		  size_t n;
		  
		  if ((n = grecs_list_size((grecs_grecs_vsp[(1) - (1)].list))) == 1) {
			  (grecs_grecs_val.pvalue) = grecs_list_index((grecs_grecs_vsp[(1) - (1)].list), 0);
		  } else {
			  size_t i;
			  struct grecs_list_entry *ep;
		
			  (grecs_grecs_val.pvalue) = grecs_malloc(sizeof((grecs_grecs_val.pvalue)[0]));
			  (grecs_grecs_val.pvalue)->type = GRECS_TYPE_ARRAY;
			  (grecs_grecs_val.pvalue)->locus = (grecs_grecs_lsp[(1) - (1)]);
			  (grecs_grecs_val.pvalue)->v.arg.c = n;
			  (grecs_grecs_val.pvalue)->v.arg.v = grecs_calloc(n,
						     sizeof((grecs_grecs_val.pvalue)->v.arg.v[0]));
			  for (i = 0, ep = (grecs_grecs_vsp[(1) - (1)].list)->head; ep; i++, ep = ep->next)
				  (grecs_grecs_val.pvalue)->v.arg.v[i] = ep->data;
		  }
		  (grecs_grecs_vsp[(1) - (1)].list)->free_entry = NULL;
		  grecs_list_free((grecs_grecs_vsp[(1) - (1)].list));	      
	  }
    break;

  case 15:
#line 151 "grecs-gram.y"
    {
		  (grecs_grecs_val.list) = grecs_value_list_create();
		  grecs_list_append((grecs_grecs_val.list), grecs_value_ptr_from_static(&(grecs_grecs_vsp[(1) - (1)].svalue)));
	  }
    break;

  case 16:
#line 156 "grecs-gram.y"
    {
		  grecs_list_append((grecs_grecs_vsp[(1) - (2)].list), grecs_value_ptr_from_static(&(grecs_grecs_vsp[(2) - (2)].svalue)));
	  }
    break;

  case 17:
#line 162 "grecs-gram.y"
    {
		  (grecs_grecs_val.svalue).type = GRECS_TYPE_STRING;
		  (grecs_grecs_val.svalue).locus = (grecs_grecs_lsp[(1) - (1)]);
		  (grecs_grecs_val.svalue).v.string = (grecs_grecs_vsp[(1) - (1)].string);
	  }
    break;

  case 18:
#line 168 "grecs-gram.y"
    {
		  (grecs_grecs_val.svalue).type = GRECS_TYPE_LIST;
		  (grecs_grecs_val.svalue).locus = (grecs_grecs_lsp[(1) - (1)]);
		  (grecs_grecs_val.svalue).v.list = (grecs_grecs_vsp[(1) - (1)].list);
	  }
    break;

  case 19:
#line 174 "grecs-gram.y"
    {
		  (grecs_grecs_val.svalue).type = GRECS_TYPE_STRING;
		  (grecs_grecs_val.svalue).locus = (grecs_grecs_lsp[(1) - (1)]);
		  (grecs_grecs_val.svalue).v.string = (grecs_grecs_vsp[(1) - (1)].string);
	  }
    break;

  case 23:
#line 187 "grecs-gram.y"
    {
		  struct grecs_list_entry *ep;
		  
		  grecs_line_begin();
		  for (ep = (grecs_grecs_vsp[(1) - (1)].list)->head; ep; ep = ep->next) {
			  grecs_line_add(ep->data, strlen(ep->data));
			  free(ep->data);
			  ep->data = NULL;
		  }
		  (grecs_grecs_val.string) = grecs_line_finish();
		  grecs_list_free((grecs_grecs_vsp[(1) - (1)].list));
	  }
    break;

  case 24:
#line 202 "grecs-gram.y"
    {
		  (grecs_grecs_val.list) = grecs_list_create();
		  grecs_list_append((grecs_grecs_val.list), (grecs_grecs_vsp[(1) - (1)].string));
	  }
    break;

  case 25:
#line 207 "grecs-gram.y"
    {
		  grecs_list_append((grecs_grecs_vsp[(1) - (2)].list), (grecs_grecs_vsp[(2) - (2)].string));
		  (grecs_grecs_val.list) = (grecs_grecs_vsp[(1) - (2)].list);
	  }
    break;

  case 26:
#line 214 "grecs-gram.y"
    {
		  (grecs_grecs_val.list) = NULL;
	  }
    break;

  case 27:
#line 218 "grecs-gram.y"
    {
		  (grecs_grecs_val.list) = (grecs_grecs_vsp[(2) - (3)].list);
	  }
    break;

  case 28:
#line 222 "grecs-gram.y"
    {
		  (grecs_grecs_val.list) = (grecs_grecs_vsp[(2) - (4)].list);
	  }
    break;

  case 29:
#line 228 "grecs-gram.y"
    {
		  (grecs_grecs_val.list) = grecs_value_list_create();
		  grecs_list_append((grecs_grecs_val.list), grecs_value_ptr_from_static(&(grecs_grecs_vsp[(1) - (1)].svalue)));
	  }
    break;

  case 30:
#line 233 "grecs-gram.y"
    {
		  grecs_list_append((grecs_grecs_vsp[(1) - (3)].list), grecs_value_ptr_from_static(&(grecs_grecs_vsp[(3) - (3)].svalue)));
		  (grecs_grecs_val.list) = (grecs_grecs_vsp[(1) - (3)].list);
	  }
    break;


/* Line 1267 of yacc.c.  */
#line 1634 "grecs-gram.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", grecs_grecs_r1[grecs_grecs_n], &grecs_grecs_val, &grecs_grecs_loc);

  YYPOPSTACK (grecs_grecs_len);
  grecs_grecs_len = 0;
  YY_STACK_PRINT (grecs_grecs_ss, grecs_grecs_ssp);

  *++grecs_grecs_vsp = grecs_grecs_val;
  *++grecs_grecs_lsp = grecs_grecs_loc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  grecs_grecs_n = grecs_grecs_r1[grecs_grecs_n];

  grecs_grecs_state = grecs_grecs_pgoto[grecs_grecs_n - YYNTOKENS] + *grecs_grecs_ssp;
  if (0 <= grecs_grecs_state && grecs_grecs_state <= YYLAST && grecs_grecs_check[grecs_grecs_state] == *grecs_grecs_ssp)
    grecs_grecs_state = grecs_grecs_table[grecs_grecs_state];
  else
    grecs_grecs_state = grecs_grecs_defgoto[grecs_grecs_n - YYNTOKENS];

  goto grecs_grecs_newstate;


/*------------------------------------.
| grecs_grecs_errlab -- here on detecting error |
`------------------------------------*/
grecs_grecs_errlab:
  /* If not already recovering from an error, report this error.  */
  if (!grecs_grecs_errstatus)
    {
      ++grecs_grecs_nerrs;
#if ! YYERROR_VERBOSE
      grecs_grecs_error (YY_("syntax error"));
#else
      {
	YYSIZE_T grecs_grecs_size = grecs_grecs_syntax_error (0, grecs_grecs_state, grecs_grecs_char);
	if (grecs_grecs_msg_alloc < grecs_grecs_size && grecs_grecs_msg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T grecs_grecs_alloc = 2 * grecs_grecs_size;
	    if (! (grecs_grecs_size <= grecs_grecs_alloc && grecs_grecs_alloc <= YYSTACK_ALLOC_MAXIMUM))
	      grecs_grecs_alloc = YYSTACK_ALLOC_MAXIMUM;
	    if (grecs_grecs_msg != grecs_grecs_msgbuf)
	      YYSTACK_FREE (grecs_grecs_msg);
	    grecs_grecs_msg = (char *) YYSTACK_ALLOC (grecs_grecs_alloc);
	    if (grecs_grecs_msg)
	      grecs_grecs_msg_alloc = grecs_grecs_alloc;
	    else
	      {
		grecs_grecs_msg = grecs_grecs_msgbuf;
		grecs_grecs_msg_alloc = sizeof grecs_grecs_msgbuf;
	      }
	  }

	if (0 < grecs_grecs_size && grecs_grecs_size <= grecs_grecs_msg_alloc)
	  {
	    (void) grecs_grecs_syntax_error (grecs_grecs_msg, grecs_grecs_state, grecs_grecs_char);
	    grecs_grecs_error (grecs_grecs_msg);
	  }
	else
	  {
	    grecs_grecs_error (YY_("syntax error"));
	    if (grecs_grecs_size != 0)
	      goto grecs_grecs_exhaustedlab;
	  }
      }
#endif
    }

  grecs_grecs_error_range[0] = grecs_grecs_lloc;

  if (grecs_grecs_errstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (grecs_grecs_char <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (grecs_grecs_char == YYEOF)
	    YYABORT;
	}
      else
	{
	  grecs_grecs_destruct ("Error: discarding",
		      grecs_grecs_token, &grecs_grecs_lval, &grecs_grecs_lloc);
	  grecs_grecs_char = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto grecs_grecs_errlab1;


/*---------------------------------------------------.
| grecs_grecs_errorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
grecs_grecs_errorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label grecs_grecs_errorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto grecs_grecs_errorlab;

  grecs_grecs_error_range[0] = grecs_grecs_lsp[1-grecs_grecs_len];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (grecs_grecs_len);
  grecs_grecs_len = 0;
  YY_STACK_PRINT (grecs_grecs_ss, grecs_grecs_ssp);
  grecs_grecs_state = *grecs_grecs_ssp;
  goto grecs_grecs_errlab1;


/*-------------------------------------------------------------.
| grecs_grecs_errlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
grecs_grecs_errlab1:
  grecs_grecs_errstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      grecs_grecs_n = grecs_grecs_pact[grecs_grecs_state];
      if (grecs_grecs_n != YYPACT_NINF)
	{
	  grecs_grecs_n += YYTERROR;
	  if (0 <= grecs_grecs_n && grecs_grecs_n <= YYLAST && grecs_grecs_check[grecs_grecs_n] == YYTERROR)
	    {
	      grecs_grecs_n = grecs_grecs_table[grecs_grecs_n];
	      if (0 < grecs_grecs_n)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (grecs_grecs_ssp == grecs_grecs_ss)
	YYABORT;

      grecs_grecs_error_range[0] = *grecs_grecs_lsp;
      grecs_grecs_destruct ("Error: popping",
		  grecs_grecs_stos[grecs_grecs_state], grecs_grecs_vsp, grecs_grecs_lsp);
      YYPOPSTACK (1);
      grecs_grecs_state = *grecs_grecs_ssp;
      YY_STACK_PRINT (grecs_grecs_ss, grecs_grecs_ssp);
    }

  if (grecs_grecs_n == YYFINAL)
    YYACCEPT;

  *++grecs_grecs_vsp = grecs_grecs_lval;

  grecs_grecs_error_range[1] = grecs_grecs_lloc;
  /* Using YYLLOC is tempting, but would change the location of
     the look-ahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (grecs_grecs_loc, (grecs_grecs_error_range - 1), 2);
  *++grecs_grecs_lsp = grecs_grecs_loc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", grecs_grecs_stos[grecs_grecs_n], grecs_grecs_vsp, grecs_grecs_lsp);

  grecs_grecs_state = grecs_grecs_n;
  goto grecs_grecs_newstate;


/*-------------------------------------.
| grecs_grecs_acceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
grecs_grecs_acceptlab:
  grecs_grecs_result = 0;
  goto grecs_grecs_return;

/*-----------------------------------.
| grecs_grecs_abortlab -- YYABORT comes here.  |
`-----------------------------------*/
grecs_grecs_abortlab:
  grecs_grecs_result = 1;
  goto grecs_grecs_return;

#ifndef grecs_grecs_overflow
/*-------------------------------------------------.
| grecs_grecs_exhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
grecs_grecs_exhaustedlab:
  grecs_grecs_error (YY_("memory exhausted"));
  grecs_grecs_result = 2;
  /* Fall through.  */
#endif

grecs_grecs_return:
  if (grecs_grecs_char != YYEOF && grecs_grecs_char != YYEMPTY)
     grecs_grecs_destruct ("Cleanup: discarding lookahead",
		 grecs_grecs_token, &grecs_grecs_lval, &grecs_grecs_lloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (grecs_grecs_len);
  YY_STACK_PRINT (grecs_grecs_ss, grecs_grecs_ssp);
  while (grecs_grecs_ssp != grecs_grecs_ss)
    {
      grecs_grecs_destruct ("Cleanup: popping",
		  grecs_grecs_stos[*grecs_grecs_ssp], grecs_grecs_vsp, grecs_grecs_lsp);
      YYPOPSTACK (1);
    }
#ifndef grecs_grecs_overflow
  if (grecs_grecs_ss != grecs_grecs_ssa)
    YYSTACK_FREE (grecs_grecs_ss);
#endif
#if YYERROR_VERBOSE
  if (grecs_grecs_msg != grecs_grecs_msgbuf)
    YYSTACK_FREE (grecs_grecs_msg);
#endif
  /* Make sure YYID is used.  */
  return YYID (grecs_grecs_result);
}


#line 243 "grecs-gram.y"


int
grecs_grecs_error(char *s)
{
	grecs_error(&grecs_grecs_lloc, 0, "%s", s);
	return 0;
}

struct grecs_node *
grecs_grecs_parser(const char *name, int traceflags)
{
	int rc;
	if (grecs_lex_begin(name, traceflags & GRECS_TRACE_LEX))
		return NULL;
	grecs_grecs_debug = traceflags & GRECS_TRACE_GRAM;
	parse_tree = NULL;
	rc = grecs_grecs_parse();
	if (grecs_error_count)
		rc = 1;
	grecs_lex_end(rc);
	if (rc) {
		grecs_tree_free(parse_tree);
		parse_tree = NULL;
	}
	return parse_tree;
}




    


