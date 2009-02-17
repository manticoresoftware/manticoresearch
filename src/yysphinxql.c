/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_IDENT = 258,
     TOK_CONST = 259,
     TOK_QUOTED_STRING = 260,
     TOK_AS = 261,
     TOK_ASC = 262,
     TOK_AVG = 263,
     TOK_BETWEEN = 264,
     TOK_BY = 265,
     TOK_COUNT = 266,
     TOK_DESC = 267,
     TOK_DISTINCT = 268,
     TOK_FROM = 269,
     TOK_GROUP = 270,
     TOK_LIMIT = 271,
     TOK_IN = 272,
     TOK_ID = 273,
     TOK_MATCH = 274,
     TOK_MAX = 275,
     TOK_META = 276,
     TOK_MIN = 277,
     TOK_OPTION = 278,
     TOK_ORDER = 279,
     TOK_SELECT = 280,
     TOK_SHOW = 281,
     TOK_STATUS = 282,
     TOK_SUM = 283,
     TOK_WARNINGS = 284,
     TOK_WEIGHT = 285,
     TOK_WITHIN = 286,
     TOK_WHERE = 287,
     TOK_OR = 288,
     TOK_AND = 289,
     TOK_NOT = 290,
     TOK_NE = 291,
     TOK_GTE = 292,
     TOK_LTE = 293,
     TOK_NEG = 294
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST 259
#define TOK_QUOTED_STRING 260
#define TOK_AS 261
#define TOK_ASC 262
#define TOK_AVG 263
#define TOK_BETWEEN 264
#define TOK_BY 265
#define TOK_COUNT 266
#define TOK_DESC 267
#define TOK_DISTINCT 268
#define TOK_FROM 269
#define TOK_GROUP 270
#define TOK_LIMIT 271
#define TOK_IN 272
#define TOK_ID 273
#define TOK_MATCH 274
#define TOK_MAX 275
#define TOK_META 276
#define TOK_MIN 277
#define TOK_OPTION 278
#define TOK_ORDER 279
#define TOK_SELECT 280
#define TOK_SHOW 281
#define TOK_STATUS 282
#define TOK_SUM 283
#define TOK_WARNINGS 284
#define TOK_WEIGHT 285
#define TOK_WITHIN 286
#define TOK_WHERE 287
#define TOK_OR 288
#define TOK_AND 289
#define TOK_NOT 290
#define TOK_NE 291
#define TOK_GTE 292
#define TOK_LTE 293
#define TOK_NEG 294




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#endif


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */


#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
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
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  27
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   330

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  31
/* YYNRULES -- Number of rules. */
#define YYNRULES  87
/* YYNRULES -- Number of states. */
#define YYNSTATES  192

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   294

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      48,    49,    44,    42,    47,    43,     2,    45,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      38,    36,    39,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    37,    40,    41,    46
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    22,    24,    28,
      30,    34,    41,    48,    55,    62,    64,    70,    72,    76,
      77,    79,    82,    84,    88,    93,    97,   101,   107,   114,
     120,   124,   128,   132,   136,   138,   142,   143,   145,   149,
     150,   152,   158,   159,   161,   165,   167,   171,   173,   176,
     179,   180,   182,   187,   188,   190,   193,   195,   199,   203,
     207,   209,   211,   214,   217,   221,   225,   229,   233,   237,
     241,   245,   249,   253,   257,   261,   265,   269,   271,   276,
     281,   285,   292,   299,   301,   305,   308,   311
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      51,     0,    -1,    52,    -1,    78,    -1,    79,    -1,    80,
      -1,    25,    53,    14,    55,    56,    61,    63,    65,    69,
      71,    -1,    54,    -1,    53,    47,    54,    -1,     3,    -1,
      75,     6,     3,    -1,     8,    48,    75,    49,     6,     3,
      -1,    20,    48,    75,    49,     6,     3,    -1,    22,    48,
      75,    49,     6,     3,    -1,    28,    48,    75,    49,     6,
       3,    -1,    44,    -1,    11,    48,    13,     3,    49,    -1,
       3,    -1,    55,    47,     3,    -1,    -1,    57,    -1,    32,
      58,    -1,    59,    -1,    58,    34,    59,    -1,    19,    48,
       5,    49,    -1,     3,    36,     4,    -1,     3,    37,     4,
      -1,     3,    17,    48,    60,    49,    -1,     3,    35,    17,
      48,    60,    49,    -1,     3,     9,     4,    34,     4,    -1,
       3,    39,     4,    -1,     3,    38,     4,    -1,     3,    40,
       4,    -1,     3,    41,     4,    -1,     4,    -1,    60,    47,
       4,    -1,    -1,    62,    -1,    15,    10,     3,    -1,    -1,
      64,    -1,    31,    15,    24,    10,    67,    -1,    -1,    66,
      -1,    24,    10,    67,    -1,    68,    -1,    67,    47,    68,
      -1,     3,    -1,     3,     7,    -1,     3,    12,    -1,    -1,
      70,    -1,    16,     4,    47,     4,    -1,    -1,    72,    -1,
      23,    73,    -1,    74,    -1,    73,    47,    74,    -1,     3,
      36,     3,    -1,     3,    36,     4,    -1,     3,    -1,     4,
      -1,    43,    75,    -1,    35,    75,    -1,    75,    42,    75,
      -1,    75,    43,    75,    -1,    75,    44,    75,    -1,    75,
      45,    75,    -1,    75,    38,    75,    -1,    75,    39,    75,
      -1,    75,    41,    75,    -1,    75,    40,    75,    -1,    75,
      36,    75,    -1,    75,    37,    75,    -1,    75,    34,    75,
      -1,    75,    33,    75,    -1,    48,    75,    49,    -1,    76,
      -1,     3,    48,    77,    49,    -1,    17,    48,    77,    49,
      -1,     3,    48,    49,    -1,    22,    48,    75,    47,    75,
      49,    -1,    20,    48,    75,    47,    75,    49,    -1,    75,
      -1,    77,    47,    75,    -1,    26,    29,    -1,    26,    27,
      -1,    26,    21,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    55,    55,    56,    57,    58,    64,    79,    80,    84,
      85,    86,    87,    88,    89,    90,    91,   106,   107,   110,
     112,   116,   120,   121,   125,   138,   146,   155,   163,   172,
     181,   190,   199,   208,   220,   221,   224,   226,   230,   237,
     239,   243,   249,   251,   255,   262,   263,   267,   268,   269,
     272,   274,   278,   285,   287,   291,   295,   296,   300,   301,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   328,   329,
     330,   331,   332,   336,   337,   343,   347,   351
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_CONST", 
  "TOK_QUOTED_STRING", "TOK_AS", "TOK_ASC", "TOK_AVG", "TOK_BETWEEN", 
  "TOK_BY", "TOK_COUNT", "TOK_DESC", "TOK_DISTINCT", "TOK_FROM", 
  "TOK_GROUP", "TOK_LIMIT", "TOK_IN", "TOK_ID", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_OPTION", "TOK_ORDER", "TOK_SELECT", 
  "TOK_SHOW", "TOK_STATUS", "TOK_SUM", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WITHIN", "TOK_WHERE", "TOK_OR", "TOK_AND", "TOK_NOT", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "TOK_NEG", "','", "'('", "')'", "$accept", "statement", 
  "select_from", "select_items_list", "select_item", "ident_list", 
  "opt_where_clause", "where_clause", "where_expr", "where_item", 
  "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "expr", "function", "arglist", "show_warnings", 
  "show_status", "show_meta", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,    61,   291,    60,    62,
     292,   293,    43,    45,    42,    47,   294,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    50,    51,    51,    51,    51,    52,    53,    53,    54,
      54,    54,    54,    54,    54,    54,    54,    55,    55,    56,
      56,    57,    58,    58,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    59,    60,    60,    61,    61,    62,    63,
      63,    64,    65,    65,    66,    67,    67,    68,    68,    68,
      69,    69,    70,    71,    71,    72,    73,    73,    74,    74,
      75,    75,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    75,    75,    75,    75,    76,    76,
      76,    76,    76,    77,    77,    78,    79,    80
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,    10,     1,     3,     1,
       3,     6,     6,     6,     6,     1,     5,     1,     3,     0,
       1,     2,     1,     3,     4,     3,     3,     5,     6,     5,
       3,     3,     3,     3,     1,     3,     0,     1,     3,     0,
       1,     5,     0,     1,     3,     1,     3,     1,     2,     2,
       0,     1,     4,     0,     1,     2,     1,     3,     3,     3,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     4,     4,
       3,     6,     6,     1,     3,     2,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     0,     2,     3,     4,     5,    60,    61,
       0,     0,     0,     0,     0,     0,     0,     0,    15,     0,
       0,     7,     0,    77,    87,    86,    85,     1,     0,     0,
       0,     0,     0,     0,     0,    60,     0,     0,    63,    62,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    80,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    76,    17,    19,
       8,    10,    75,    74,    72,    73,    68,    69,    71,    70,
      64,    65,    66,    67,     0,    78,     0,     0,    79,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,    20,
      84,     0,    16,     0,     0,     0,     0,     0,     0,     0,
      21,    22,    18,     0,    39,    37,    11,    82,    12,    81,
      13,    14,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    42,    40,     0,     0,     0,
      25,    26,    31,    30,    32,    33,     0,    23,    38,     0,
       0,    50,    43,     0,    34,     0,     0,    24,     0,     0,
       0,    53,    51,    29,     0,    27,     0,     0,    47,    44,
      45,     0,     0,     6,    54,    35,    28,    41,    48,    49,
       0,     0,     0,    55,    56,    46,    52,     0,     0,    58,
      59,    57
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     3,     4,    20,    21,    69,    98,    99,   110,   111,
     155,   114,   115,   135,   136,   151,   152,   169,   170,   161,
     162,   173,   174,   183,   184,    22,    23,    58,     5,     6,
       7
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -44
static const short yypact[] =
{
      16,    52,    49,    11,   -44,   -44,   -44,   -44,    -4,   -44,
     -43,   -29,   -25,    13,    31,    40,    42,    42,   -44,    42,
       0,   -44,    65,   -44,   -44,   -44,   -44,   -44,     5,    42,
      53,    42,    42,    42,    42,    41,    63,    73,   230,   -44,
     120,    89,    52,    94,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,   -44,   220,   -23,   134,
     135,    18,    86,   103,   148,    42,    42,   -44,   -44,   -26,
     -44,   -44,   230,   230,   238,   238,    39,    39,    39,    39,
       7,     7,   -44,   -44,    42,   -44,   143,   102,   -44,    42,
     149,    42,   160,   174,   190,   205,     1,   191,   193,   -44,
     220,   219,   -44,   162,   233,   176,   237,   248,    77,   207,
     250,   -44,   -44,   275,   255,   -44,   -44,   -44,   -44,   -44,
     -44,   -44,   283,   240,   272,   286,   287,   288,   289,   290,
     291,   292,     1,   293,   284,   274,   -44,   266,   297,   254,
     -44,   -44,   -44,   -44,   -44,   -44,   256,   -44,   -44,   279,
     294,   295,   -44,   302,   -44,    44,   297,   -44,   298,   304,
     305,   296,   -44,   -44,   306,   -44,    85,   304,    -5,   265,
     -44,   267,   310,   -44,   -44,   -44,   -44,   265,   -44,   -44,
     304,   311,   280,   270,   -44,   -44,   -44,    54,   310,   -44,
     -44,   -44
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -44,   -44,   -44,   -44,   276,   -44,   -44,   -44,   -44,   188,
     165,   -44,   -44,   -44,   -44,   -44,   -44,   155,   144,   -44,
     -44,   -44,   -44,   -44,   137,   -16,   -44,   299,   -44,   -44,
     -44
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -10
static const short yytable[] =
{
      38,    39,   178,    40,   108,    29,    96,   179,    35,     9,
      -9,    27,    57,    59,    41,    57,    62,    63,    64,    30,
     109,    97,    12,    31,    84,    36,    85,    37,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      16,     1,     2,    -9,    28,    35,     9,    42,    17,    94,
      95,    54,    55,    19,    56,     8,     9,   189,   190,    12,
      10,    32,    36,    11,    37,    84,    60,    88,   100,    12,
      24,    43,    13,   103,    14,   105,    25,    16,    26,    33,
      15,    52,    53,    54,    55,    17,   122,    16,    34,    28,
      19,   164,    68,   165,   123,    17,    18,    71,    44,    45,
      19,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    65,   124,   125,   126,   127,   128,   129,   130,    44,
      45,    66,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   164,    89,   176,    90,    44,    45,    87,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,   101,
      91,   102,    92,    44,    45,   104,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   106,    44,    45,    67,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
     107,    44,    45,    86,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,   112,    44,    45,    93,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,   113,    44,
      45,   117,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,   116,    44,    45,   119,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,   118,    89,    44,    45,
     120,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,   121,    91,    44,    45,   131,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    48,    49,    50,    51,
      52,    53,    54,    55,   132,   133,   134,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   148,   146,   150,   149,
     153,   154,   156,   158,   159,   157,   163,   168,   167,   171,
     175,   160,   180,   182,   181,   186,   187,   188,    70,   172,
     147,   166,   177,     0,   185,   191,     0,     0,     0,     0,
      61
};

static const short yycheck[] =
{
      16,    17,     7,    19,     3,    48,    32,    12,     3,     4,
      14,     0,    28,    29,    14,    31,    32,    33,    34,    48,
      19,    47,    17,    48,    47,    20,    49,    22,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      35,    25,    26,    47,    48,     3,     4,    47,    43,    65,
      66,    44,    45,    48,    49,     3,     4,     3,     4,    17,
       8,    48,    20,    11,    22,    47,    13,    49,    84,    17,
      21,     6,    20,    89,    22,    91,    27,    35,    29,    48,
      28,    42,    43,    44,    45,    43,     9,    35,    48,    48,
      48,    47,     3,    49,    17,    43,    44,     3,    33,    34,
      48,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    48,    35,    36,    37,    38,    39,    40,    41,    33,
      34,    48,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    47,    47,    49,    49,    33,    34,     3,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,     6,
      47,    49,    49,    33,    34,     6,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,     6,    33,    34,    49,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
       6,    33,    34,    49,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,     3,    33,    34,    49,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    15,    33,
      34,    49,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,     3,    33,    34,    49,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,     3,    47,    33,    34,
       3,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,     3,    47,    33,    34,    48,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    38,    39,    40,    41,
      42,    43,    44,    45,    34,    10,    31,     4,    48,    17,
       4,     4,     4,     4,     4,     4,     3,     5,    24,    15,
      34,     4,    48,    24,    10,    49,     4,     3,    10,     4,
       4,    16,    47,     3,    47,     4,    36,    47,    42,    23,
     132,   156,   167,    -1,   180,   188,    -1,    -1,    -1,    -1,
      31
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    25,    26,    51,    52,    78,    79,    80,     3,     4,
       8,    11,    17,    20,    22,    28,    35,    43,    44,    48,
      53,    54,    75,    76,    21,    27,    29,     0,    48,    48,
      48,    48,    48,    48,    48,     3,    20,    22,    75,    75,
      75,    14,    47,     6,    33,    34,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    49,    75,    77,    75,
      13,    77,    75,    75,    75,    48,    48,    49,     3,    55,
      54,     3,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    47,    49,    49,     3,    49,    47,
      49,    47,    49,    49,    75,    75,    32,    47,    56,    57,
      75,     6,    49,    75,     6,    75,     6,     6,     3,    19,
      58,    59,     3,    15,    61,    62,     3,    49,     3,    49,
       3,     3,     9,    17,    35,    36,    37,    38,    39,    40,
      41,    48,    34,    10,    31,    63,    64,     4,    48,    17,
       4,     4,     4,     4,     4,     4,     5,    59,     3,    15,
      24,    65,    66,    34,     4,    60,    48,    49,    24,    10,
      16,    69,    70,     4,    47,    49,    60,    10,     3,    67,
      68,     4,    23,    71,    72,     4,    49,    67,     7,    12,
      47,    47,     3,    73,    74,    68,     4,    36,    47,     3,
       4,    74
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror (pParser, "syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, pParser)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse ( SqlParser_t * pParser );
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse ( SqlParser_t * pParser )
#else
int
yyparse (pParser)
     SqlParser_t * pParser ;
#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:

    {
			pParser->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 9:

    { pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 10:

    { pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 11:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 12:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 13:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 14:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 15:

    { pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 16:

    {
			if ( !pParser->m_pQuery->m_sGroupDistinct.IsEmpty() )
			{
				yyerror ( pParser, "too many COUNT(DISTINCT) clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sGroupDistinct = yyvsp[-1].m_sValue;
			}
		;}
    break;

  case 18:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 24:

    {
			if ( pParser->m_bGotQuery )
			{
				yyerror ( pParser, "too many MATCH() clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sQuery = yyvsp[-1].m_sValue;
				pParser->m_bGotQuery = true;
			}
		;}
    break;

  case 25:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 26:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 27:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 28:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-2].m_dValues;
			tFilter.m_bExclude = true;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 29:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = yyvsp[-2].m_iValue;
			tFilter.m_uMaxValue = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 30:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = yyvsp[0].m_iValue + 1;
			tFilter.m_uMaxValue = UINT_MAX;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 31:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = 0;
			tFilter.m_uMaxValue = yyvsp[0].m_iValue - 1;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 32:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = yyvsp[0].m_iValue;
			tFilter.m_uMaxValue = UINT_MAX;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 33:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = 0;
			tFilter.m_uMaxValue = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 34:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 35:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 38:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 41:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 44:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 46:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 48:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 49:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 52:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 58:

    { if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 59:

    { if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 62:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 63:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 64:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 65:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 66:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 67:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 68:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 69:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 70:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 71:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 72:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 73:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 74:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 75:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 76:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 78:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 79:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 80:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 81:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 82:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 85:

    { pParser->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 86:

    { pParser->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 87:

    { pParser->m_eStmt = STMT_SHOW_META; ;}
    break;


    }

/* Line 991 of yacc.c.  */


  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (pParser, yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror (pParser, "syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (pParser, "syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__)
//  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror (pParser, "parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}





#if USE_WINDOWS
#pragma warning(pop)
#endif

