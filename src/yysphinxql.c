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
     TOK_DESC = 266,
     TOK_FROM = 267,
     TOK_GROUP = 268,
     TOK_LIMIT = 269,
     TOK_IN = 270,
     TOK_ID = 271,
     TOK_MATCH = 272,
     TOK_MAX = 273,
     TOK_MIN = 274,
     TOK_OPTION = 275,
     TOK_ORDER = 276,
     TOK_SELECT = 277,
     TOK_SHOW = 278,
     TOK_STATUS = 279,
     TOK_SUM = 280,
     TOK_WARNINGS = 281,
     TOK_WEIGHT = 282,
     TOK_WITHIN = 283,
     TOK_WHERE = 284,
     TOK_OR = 285,
     TOK_AND = 286,
     TOK_NOT = 287,
     TOK_NE = 288,
     TOK_GTE = 289,
     TOK_LTE = 290,
     TOK_NEG = 291
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
#define TOK_DESC 266
#define TOK_FROM 267
#define TOK_GROUP 268
#define TOK_LIMIT 269
#define TOK_IN 270
#define TOK_ID 271
#define TOK_MATCH 272
#define TOK_MAX 273
#define TOK_MIN 274
#define TOK_OPTION 275
#define TOK_ORDER 276
#define TOK_SELECT 277
#define TOK_SHOW 278
#define TOK_STATUS 279
#define TOK_SUM 280
#define TOK_WARNINGS 281
#define TOK_WEIGHT 282
#define TOK_WITHIN 283
#define TOK_WHERE 284
#define TOK_OR 285
#define TOK_AND 286
#define TOK_NOT 287
#define TOK_NE 288
#define TOK_GTE 289
#define TOK_LTE 290
#define TOK_NEG 291




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
#define YYFINAL  24
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   303

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  47
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  30
/* YYNRULES -- Number of rules. */
#define YYNRULES  78
/* YYNRULES -- Number of states. */
#define YYNSTATES  170

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   291

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      45,    46,    41,    39,    44,    40,     2,    42,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      35,    33,    36,     2,     2,     2,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    32,    34,    37,
      38,    43
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    20,    22,    26,    28,
      32,    39,    46,    53,    60,    62,    64,    68,    69,    71,
      74,    76,    80,    85,    89,    95,   101,   103,   107,   108,
     110,   114,   115,   117,   123,   124,   126,   130,   132,   136,
     138,   141,   144,   145,   147,   152,   153,   155,   158,   160,
     164,   168,   172,   174,   176,   179,   182,   186,   190,   194,
     198,   202,   206,   210,   214,   218,   222,   226,   230,   234,
     236,   241,   246,   250,   257,   264,   266,   270,   273
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      48,     0,    -1,    49,    -1,    75,    -1,    76,    -1,    22,
      50,    12,    52,    53,    58,    60,    62,    66,    68,    -1,
      51,    -1,    50,    44,    51,    -1,     3,    -1,    72,     6,
       3,    -1,     8,    45,    72,    46,     6,     3,    -1,    18,
      45,    72,    46,     6,     3,    -1,    19,    45,    72,    46,
       6,     3,    -1,    25,    45,    72,    46,     6,     3,    -1,
      41,    -1,     3,    -1,    52,    44,     3,    -1,    -1,    54,
      -1,    29,    55,    -1,    56,    -1,    55,    31,    56,    -1,
      17,    45,     5,    46,    -1,     3,    33,     4,    -1,     3,
      15,    45,    57,    46,    -1,     3,     9,     4,    31,     4,
      -1,     4,    -1,    57,    44,     4,    -1,    -1,    59,    -1,
      13,    10,     3,    -1,    -1,    61,    -1,    28,    13,    21,
      10,    64,    -1,    -1,    63,    -1,    21,    10,    64,    -1,
      65,    -1,    64,    44,    65,    -1,     3,    -1,     3,     7,
      -1,     3,    11,    -1,    -1,    67,    -1,    14,     4,    44,
       4,    -1,    -1,    69,    -1,    20,    70,    -1,    71,    -1,
      70,    44,    71,    -1,     3,    33,     3,    -1,     3,    33,
       4,    -1,     3,    -1,     4,    -1,    40,    72,    -1,    32,
      72,    -1,    72,    39,    72,    -1,    72,    40,    72,    -1,
      72,    41,    72,    -1,    72,    42,    72,    -1,    72,    35,
      72,    -1,    72,    36,    72,    -1,    72,    38,    72,    -1,
      72,    37,    72,    -1,    72,    33,    72,    -1,    72,    34,
      72,    -1,    72,    31,    72,    -1,    72,    30,    72,    -1,
      45,    72,    46,    -1,    73,    -1,     3,    45,    74,    46,
      -1,    15,    45,    74,    46,    -1,     3,    45,    46,    -1,
      19,    45,    72,    44,    72,    46,    -1,    18,    45,    72,
      44,    72,    46,    -1,    72,    -1,    74,    44,    72,    -1,
      23,    26,    -1,    23,    24,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    52,    52,    53,    54,    60,    75,    76,    80,    81,
      82,    83,    84,    85,    86,    90,    91,    94,    96,   100,
     104,   105,   109,   113,   121,   129,   141,   142,   145,   147,
     151,   158,   160,   164,   170,   172,   176,   183,   184,   188,
     189,   190,   193,   195,   199,   206,   208,   212,   216,   217,
     221,   222,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     249,   250,   251,   252,   253,   257,   258,   264,   268
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_CONST", 
  "TOK_QUOTED_STRING", "TOK_AS", "TOK_ASC", "TOK_AVG", "TOK_BETWEEN", 
  "TOK_BY", "TOK_DESC", "TOK_FROM", "TOK_GROUP", "TOK_LIMIT", "TOK_IN", 
  "TOK_ID", "TOK_MATCH", "TOK_MAX", "TOK_MIN", "TOK_OPTION", "TOK_ORDER", 
  "TOK_SELECT", "TOK_SHOW", "TOK_STATUS", "TOK_SUM", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WITHIN", "TOK_WHERE", "TOK_OR", "TOK_AND", "TOK_NOT", 
  "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", 
  "'*'", "'/'", "TOK_NEG", "','", "'('", "')'", "$accept", "statement", 
  "select_from", "select_items_list", "select_item", "ident_list", 
  "opt_where_clause", "where_clause", "where_expr", "where_item", 
  "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "expr", "function", "arglist", "show_warnings", 
  "show_status", 0
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
     285,   286,   287,    61,   288,    60,    62,   289,   290,    43,
      45,    42,    47,   291,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    47,    48,    48,    48,    49,    50,    50,    51,    51,
      51,    51,    51,    51,    51,    52,    52,    53,    53,    54,
      55,    55,    56,    56,    56,    56,    57,    57,    58,    58,
      59,    60,    60,    61,    62,    62,    63,    64,    64,    65,
      65,    65,    66,    66,    67,    68,    68,    69,    70,    70,
      71,    71,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      73,    73,    73,    73,    73,    74,    74,    75,    76
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,    10,     1,     3,     1,     3,
       6,     6,     6,     6,     1,     1,     3,     0,     1,     2,
       1,     3,     4,     3,     5,     5,     1,     3,     0,     1,
       3,     0,     1,     5,     0,     1,     3,     1,     3,     1,
       2,     2,     0,     1,     4,     0,     1,     2,     1,     3,
       3,     3,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       4,     4,     3,     6,     6,     1,     3,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     0,     2,     3,     4,    52,    53,     0,
       0,     0,     0,     0,     0,     0,    14,     0,     0,     6,
       0,    69,    78,    77,     1,     0,     0,     0,     0,     0,
       0,    52,     0,     0,    55,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    72,    75,     0,     0,     0,     0,     0,     0,
       0,     0,    68,    15,    17,     7,     9,    67,    66,    64,
      65,    60,    61,    63,    62,    56,    57,    58,    59,     0,
      70,     0,    71,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,    18,    76,     0,     0,     0,     0,     0,
       0,     0,     0,    19,    20,    16,     0,    31,    29,    10,
      74,    11,    73,    12,    13,     0,     0,     0,     0,     0,
       0,     0,    34,    32,     0,     0,    23,     0,    21,    30,
       0,     0,    42,    35,     0,    26,     0,    22,     0,     0,
       0,    45,    43,    25,     0,    24,     0,    39,    36,    37,
       0,     0,     5,    46,    27,    33,    40,    41,     0,     0,
       0,    47,    48,    38,    44,     0,     0,    50,    51,    49
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     3,     4,    18,    19,    64,    92,    93,   103,   104,
     136,   107,   108,   122,   123,   132,   133,   148,   149,   141,
     142,   152,   153,   161,   162,    20,    21,    54,     5,     6
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -37
static const short yypact[] =
{
      22,    58,   -19,     6,   -37,   -37,   -37,    -4,   -37,   -36,
       3,     7,    27,    29,    82,    82,   -37,    82,    -2,   -37,
      98,   -37,   -37,   -37,   -37,    35,    82,    82,    82,    82,
      82,    33,    34,    37,   255,   -37,   145,    19,    58,    81,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,   -37,   245,   -23,   159,     5,   111,   128,   173,
      82,    82,   -37,   -37,   -27,   -37,   -37,   255,   255,    70,
      70,    16,    16,    16,    16,    18,    18,   -37,   -37,    82,
     -37,    83,   -37,    82,    85,    82,    86,    87,   215,   230,
       1,    84,    75,   -37,   245,    91,   187,    92,   201,    93,
      99,    10,    68,    88,   -37,   -37,   105,    89,   -37,   -37,
     -37,   -37,   -37,   -37,   -37,   112,    73,   116,   118,     1,
     121,   108,   104,   -37,    95,   126,   -37,    97,   -37,   -37,
     133,   146,   157,   -37,   156,   -37,    24,   -37,   163,   174,
     184,   182,   -37,   -37,   212,   -37,   174,    13,   186,   -37,
     200,   259,   -37,   -37,   -37,   186,   -37,   -37,   174,   254,
     240,   233,   -37,   -37,   -37,    60,   259,   -37,   -37,   -37
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -37,   -37,   -37,   -37,   260,   -37,   -37,   -37,   -37,   180,
     -37,   -37,   -37,   -37,   -37,   -37,   -37,   154,   143,   -37,
     -37,   -37,   -37,   -37,   136,   -14,   -37,   276,   -37,   -37
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -9
static const short yytable[] =
{
      34,    35,    90,    36,   101,    22,    24,    23,    -8,    26,
      37,    53,    55,    53,    57,    58,    59,    91,   102,   115,
     156,    79,    63,    80,   157,   116,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    31,     8,
      -8,    25,    38,   117,     1,     2,    88,    89,    27,    79,
      10,    82,    28,    32,    33,    48,    49,    50,    51,    50,
      51,     7,     8,   167,   168,    94,     9,    14,   144,    96,
     145,    98,    29,    10,    30,    15,    11,    12,    25,    60,
      17,    52,    61,    13,    66,    31,     8,   105,   106,    95,
      14,    97,    99,   100,   109,   111,   113,    10,    15,    16,
      32,    33,   114,    17,    39,    44,    45,    46,    47,    48,
      49,    50,    51,   118,    14,   120,   124,   121,   125,   119,
     126,   130,    15,   127,   129,   131,   134,    17,    40,    41,
     135,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    40,    41,   137,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   138,    83,   139,    84,    40,    41,
     143,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,   140,    85,   146,    86,    40,    41,   147,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   150,    40,
      41,    62,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   151,    40,    41,    81,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,   154,    40,    41,    87,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
     158,    40,    41,   110,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,   159,    40,    41,   112,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   164,    83,
      40,    41,   160,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,   165,    85,    40,    41,   166,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    65,   128,
     155,   163,   169,    56
};

static const unsigned char yycheck[] =
{
      14,    15,    29,    17,     3,    24,     0,    26,    12,    45,
      12,    25,    26,    27,    28,    29,    30,    44,    17,     9,
       7,    44,     3,    46,    11,    15,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,     3,     4,
      44,    45,    44,    33,    22,    23,    60,    61,    45,    44,
      15,    46,    45,    18,    19,    39,    40,    41,    42,    41,
      42,     3,     4,     3,     4,    79,     8,    32,    44,    83,
      46,    85,    45,    15,    45,    40,    18,    19,    45,    45,
      45,    46,    45,    25,     3,     3,     4,     3,    13,     6,
      32,     6,     6,     6,     3,     3,     3,    15,    40,    41,
      18,    19,     3,    45,     6,    35,    36,    37,    38,    39,
      40,    41,    42,    45,    32,    10,     4,    28,    45,    31,
       4,    13,    40,     5,     3,    21,    31,    45,    30,    31,
       4,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    30,    31,    46,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    21,    44,    10,    46,    30,    31,
       4,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    14,    44,    10,    46,    30,    31,     3,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     4,    30,
      31,    46,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    20,    30,    31,    46,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,     4,    30,    31,    46,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      44,    30,    31,    46,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    44,    30,    31,    46,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     4,    44,
      30,    31,     3,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    33,    44,    30,    31,    44,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    38,   119,
     146,   158,   166,    27
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    22,    23,    48,    49,    75,    76,     3,     4,     8,
      15,    18,    19,    25,    32,    40,    41,    45,    50,    51,
      72,    73,    24,    26,     0,    45,    45,    45,    45,    45,
      45,     3,    18,    19,    72,    72,    72,    12,    44,     6,
      30,    31,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    46,    72,    74,    72,    74,    72,    72,    72,
      45,    45,    46,     3,    52,    51,     3,    72,    72,    72,
      72,    72,    72,    72,    72,    72,    72,    72,    72,    44,
      46,    46,    46,    44,    46,    44,    46,    46,    72,    72,
      29,    44,    53,    54,    72,     6,    72,     6,    72,     6,
       6,     3,    17,    55,    56,     3,    13,    58,    59,     3,
      46,     3,    46,     3,     3,     9,    15,    33,    45,    31,
      10,    28,    60,    61,     4,    45,     4,     5,    56,     3,
      13,    21,    62,    63,    31,     4,    57,    46,    21,    10,
      14,    66,    67,     4,    44,    46,    10,     3,    64,    65,
       4,    20,    68,    69,     4,    64,     7,    11,    44,    44,
       3,    70,    71,    65,     4,    33,    44,     3,     4,    71
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
        case 5:

    {
			pParser->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 8:

    { pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 9:

    { pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 10:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 11:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 12:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 13:

    { pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 14:

    { pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 16:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 22:

    {
			pParser->m_pQuery->m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 23:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 24:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 25:

    {
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = yyvsp[-2].m_iValue;
			tFilter.m_uMaxValue = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		;}
    break;

  case 26:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 27:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 30:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 33:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 36:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 38:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 40:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 41:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 44:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 50:

    { if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 51:

    { if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 54:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 55:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 56:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 57:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 58:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 59:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 60:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 61:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 62:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 63:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 70:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 71:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 72:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 73:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 74:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 77:

    { pParser->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 78:

    { pParser->m_eStmt = STMT_SHOW_STATUS; ;}
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

