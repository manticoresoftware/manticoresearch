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
     SEL_TOKEN = 258,
     SEL_ID = 259,
     SEL_AS = 260,
     SEL_AVG = 261,
     SEL_MAX = 262,
     SEL_MIN = 263,
     SEL_SUM = 264,
     SEL_GROUP_CONCAT = 265,
     SEL_GROUPBY = 266,
     SEL_COUNT = 267,
     SEL_WEIGHT = 268,
     SEL_DISTINCT = 269,
     SEL_OPTION = 270,
     SEL_COMMENT_OPEN = 271,
     SEL_COMMENT_CLOSE = 272,
     TOK_DIV = 273,
     TOK_MOD = 274,
     TOK_NEG = 275,
     TOK_LTE = 276,
     TOK_GTE = 277,
     TOK_EQ = 278,
     TOK_NE = 279,
     TOK_CONST_STRING = 280,
     TOK_OR = 281,
     TOK_AND = 282,
     TOK_NOT = 283,
     TOK_IS = 284,
     TOK_NULL = 285,
     TOK_FOR = 286,
     TOK_FUNC_IN = 287
   };
#endif
#define SEL_TOKEN 258
#define SEL_ID 259
#define SEL_AS 260
#define SEL_AVG 261
#define SEL_MAX 262
#define SEL_MIN 263
#define SEL_SUM 264
#define SEL_GROUP_CONCAT 265
#define SEL_GROUPBY 266
#define SEL_COUNT 267
#define SEL_WEIGHT 268
#define SEL_DISTINCT 269
#define SEL_OPTION 270
#define SEL_COMMENT_OPEN 271
#define SEL_COMMENT_CLOSE 272
#define TOK_DIV 273
#define TOK_MOD 274
#define TOK_NEG 275
#define TOK_LTE 276
#define TOK_GTE 277
#define TOK_EQ 278
#define TOK_NE 279
#define TOK_CONST_STRING 280
#define TOK_OR 281
#define TOK_AND 282
#define TOK_NOT 283
#define TOK_IS 284
#define TOK_NULL 285
#define TOK_FOR 286
#define TOK_FUNC_IN 287




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
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
#define YYFINAL  46
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   616

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  18
/* YYNRULES -- Number of rules. */
#define YYNRULES  95
/* YYNRULES -- Number of states. */
#define YYNSTATES  184

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   287

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    36,    29,     2,
      43,    44,    34,    32,    42,    33,    47,    35,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      30,     2,    31,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    48,     2,    49,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    45,    28,    46,     2,     2,     2,     2,
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
      25,    26,    27,    37,    38,    39,    40,    41
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    16,    18,    21,
      22,    24,    27,    29,    34,    39,    44,    49,    54,    58,
      63,    69,    71,    74,    77,    81,    85,    89,    93,    97,
     101,   105,   109,   113,   117,   121,   125,   129,   133,   137,
     141,   145,   149,   151,   153,   157,   162,   166,   168,   170,
     175,   179,   186,   193,   197,   203,   208,   210,   214,   218,
     224,   226,   230,   232,   239,   242,   244,   247,   249,   251,
     253,   255,   257,   259,   261,   263,   265,   267,   269,   271,
     273,   275,   277,   279,   281,   283,   285,   287,   289,   291,
     293,   295,   297,   300,   304,   308
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      51,     0,    -1,    52,    -1,    52,    62,    -1,    62,    -1,
      53,    -1,    52,    42,    53,    -1,    34,    -1,    55,    54,
      -1,    -1,     3,    -1,     5,     3,    -1,    56,    -1,     6,
      43,    56,    44,    -1,     7,    43,    56,    44,    -1,     8,
      43,    56,    44,    -1,     9,    43,    56,    44,    -1,    10,
      43,    56,    44,    -1,    11,    43,    44,    -1,    12,    43,
      34,    44,    -1,    12,    43,    14,     3,    44,    -1,    57,
      -1,    33,    56,    -1,    37,    56,    -1,    56,    32,    56,
      -1,    56,    33,    56,    -1,    56,    34,    56,    -1,    56,
      35,    56,    -1,    56,    30,    56,    -1,    56,    31,    56,
      -1,    56,    29,    56,    -1,    56,    28,    56,    -1,    56,
      36,    56,    -1,    56,    18,    56,    -1,    56,    19,    56,
      -1,    56,    21,    56,    -1,    56,    22,    56,    -1,    56,
      23,    56,    -1,    56,    24,    56,    -1,    56,    27,    56,
      -1,    56,    26,    56,    -1,    43,    56,    44,    -1,    58,
      -1,    63,    -1,    63,    38,    39,    -1,    63,    38,    37,
      39,    -1,    56,    23,    25,    -1,     4,    -1,     3,    -1,
       3,    43,    59,    44,    -1,     3,    43,    44,    -1,     8,
      43,    56,    42,    56,    44,    -1,     7,    43,    56,    42,
      56,    44,    -1,    13,    43,    44,    -1,     3,    43,    56,
      67,    44,    -1,    41,    43,    59,    44,    -1,    61,    -1,
      59,    42,    61,    -1,     3,    23,     3,    -1,    60,    42,
       3,    23,     3,    -1,    56,    -1,    45,    60,    46,    -1,
      25,    -1,    16,    15,     3,    23,     3,    17,    -1,     3,
      64,    -1,    66,    -1,    64,    66,    -1,     3,    -1,     4,
      -1,     5,    -1,     6,    -1,     7,    -1,     8,    -1,     9,
      -1,    10,    -1,    11,    -1,    12,    -1,    13,    -1,    14,
      -1,    15,    -1,    18,    -1,    19,    -1,    20,    -1,    21,
      -1,    22,    -1,    23,    -1,    24,    -1,    26,    -1,    27,
      -1,    37,    -1,    38,    -1,    39,    -1,    47,    65,    -1,
      48,    56,    49,    -1,    48,    25,    49,    -1,    40,    65,
      41,    63,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    57,    57,    58,    59,    63,    64,    68,    69,    71,
      73,    74,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   121,   122,   126,
     127,   128,   129,   130,   131,   132,   136,   137,   141,   142,
     146,   147,   148,   152,   159,   163,   164,   168,   169,   169,
     169,   169,   169,   169,   169,   170,   170,   170,   170,   170,
     170,   171,   171,   171,   171,   171,   171,   171,   171,   172,
     172,   172,   176,   177,   178,   182
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SEL_TOKEN", "SEL_ID", "SEL_AS", "SEL_AVG", 
  "SEL_MAX", "SEL_MIN", "SEL_SUM", "SEL_GROUP_CONCAT", "SEL_GROUPBY", 
  "SEL_COUNT", "SEL_WEIGHT", "SEL_DISTINCT", "SEL_OPTION", 
  "SEL_COMMENT_OPEN", "SEL_COMMENT_CLOSE", "TOK_DIV", "TOK_MOD", 
  "TOK_NEG", "TOK_LTE", "TOK_GTE", "TOK_EQ", "TOK_NE", "TOK_CONST_STRING", 
  "TOK_OR", "TOK_AND", "'|'", "'&'", "'<'", "'>'", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_IS", "TOK_NULL", "TOK_FOR", "TOK_FUNC_IN", 
  "','", "'('", "')'", "'{'", "'}'", "'.'", "'['", "']'", "$accept", 
  "select", "select_list", "select_item", "opt_alias", "select_expr", 
  "expr", "select_atom", "function", "arglist", "consthash", "arg", 
  "comment", "json_field", "subscript", "ident", "subkey", "for_loop", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   124,    38,
      60,    62,    43,    45,    42,    47,    37,   283,   284,   285,
     286,   287,    44,    40,    41,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    50,    51,    51,    51,    52,    52,    53,    53,    54,
      54,    54,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    57,    57,    58,
      58,    58,    58,    58,    58,    58,    59,    59,    60,    60,
      61,    61,    61,    62,    63,    64,    64,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    66,    66,    66,    67
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     2,     1,     1,     3,     1,     2,     0,
       1,     2,     1,     4,     4,     4,     4,     4,     3,     4,
       5,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     3,     4,     3,     1,     1,     4,
       3,     6,     6,     3,     5,     4,     1,     3,     3,     5,
       1,     3,     1,     6,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     3,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,    48,    47,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     7,     0,     0,     0,     0,     2,     5,
       9,    12,    21,    42,     4,    43,     0,     0,     0,    64,
      65,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,    23,     0,     0,     1,     0,     3,    10,
       0,     8,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,    50,     0,    60,     0,    56,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,     0,     0,    66,     0,     0,     0,     0,     0,
      18,     0,     0,    53,     0,     0,     0,    60,     0,    41,
       6,    11,    33,    34,    35,    36,    46,    37,    38,    40,
      39,    31,    30,    28,    29,    24,    25,    26,    27,    32,
       0,    44,     0,     0,     0,     0,     0,    49,    94,    93,
      13,     0,    14,     0,    15,    16,    17,     0,    19,     0,
       0,     0,    55,    45,     0,     0,    61,     0,    54,    57,
       0,     0,    20,     0,    58,     0,     0,    52,    51,    63,
       0,     0,    95,    59
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    17,    18,    19,    51,    20,    21,    22,    23,    74,
     143,    75,    24,    25,    29,   101,    30,   145
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -42
static const short yypact[] =
{
     111,   -40,   -42,   -25,    -7,    -5,    -4,    16,    17,    20,
      21,    22,   205,   -42,   205,    23,   205,    65,   -15,   -42,
       6,   497,   -42,   -42,   -42,    32,    54,   246,   163,   -13,
     -42,   205,   205,   205,   205,   205,    29,    -9,    30,    74,
      35,    37,   -42,   -42,    68,   319,   -42,   152,   -42,   -42,
      85,   -42,   205,   205,   205,   205,   174,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   -22,
     -42,   -42,    86,   477,   -18,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,    41,   268,   -42,   338,   201,   292,   357,   376,
     -42,    89,    52,   -42,    71,   205,   205,   497,   -11,   -42,
     -42,   -42,   -42,   -42,    50,    50,   -42,   580,   580,   516,
     535,   554,   573,    50,    50,    -6,    -6,   -42,   -42,   -42,
      61,   -42,    79,   -36,   246,    62,    68,   -42,   -42,   -42,
     -42,   205,   -42,   205,   -42,   -42,   -42,    63,   -42,   105,
     433,   455,   -42,   -42,   107,   109,   -42,    75,   -42,   -42,
     395,   414,   -42,   108,   -42,   103,   125,   -42,   -42,   -42,
     126,   -13,   -42,   -42
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -42,   -42,   -42,    83,   -42,   -42,   -12,   -42,   -42,    87,
     -42,   -14,   115,   -41,   -42,    -8,   113,   -42
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      42,    11,    43,    26,    45,   111,   165,    27,    28,    49,
     166,    50,    52,    53,    73,   140,   103,   141,    31,   105,
     106,   107,   108,   109,   146,   112,   147,    47,    66,    67,
      68,   146,   117,   162,    27,    28,    32,    39,    33,    34,
     122,   123,   124,   125,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,     1,     2,    35,
      36,    40,    41,    37,    38,    46,    44,    10,    52,    53,
      69,     1,     2,   110,   113,    40,    41,   114,   115,    70,
     116,    10,    64,    65,    66,    67,    68,    12,   121,   142,
     148,    14,   157,    70,   159,    15,   158,    16,    71,    72,
     163,    12,   164,   160,   161,    14,   168,   172,   173,    15,
     174,    16,   175,    72,     1,     2,   176,     3,     4,     5,
       6,     7,     8,     9,    10,   179,   180,    11,   181,   183,
     120,   118,   169,    48,   117,   182,   167,     0,     0,   170,
       0,   171,   104,     0,    12,    13,     0,     0,    14,     0,
       0,     0,    15,     0,    16,     1,     2,     0,     3,     4,
       5,     6,     7,     8,     9,    10,     1,     2,     0,     0,
      40,    41,     0,     0,     0,     0,    10,     1,     2,     0,
       0,    40,    41,     0,     0,    12,    13,    10,   102,    14,
       0,     0,     0,    15,     0,    16,    12,     0,     0,   126,
      14,     0,     0,     0,    15,     0,    16,    12,     1,     2,
       0,    14,    40,    41,     0,    15,     0,    16,    10,    52,
      53,     0,    54,    55,    56,    57,     0,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    12,     0,
       0,     0,    14,   151,     0,   152,    15,     0,    16,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,     0,    89,    90,    91,    92,    93,    94,
      95,     0,    96,    97,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    98,    99,   100,    52,    53,     0,    54,
      55,    56,    57,     0,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,     0,     0,     0,     0,     0,
      52,    53,     0,    54,    55,    56,    57,   149,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,     0,
       0,     0,     0,     0,   153,     0,   154,    52,    53,     0,
      54,    55,    56,    57,     0,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    52,    53,     0,    54,
      55,    56,    57,   119,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    52,    53,     0,    54,    55,
      56,    57,   150,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    52,    53,     0,    54,    55,    56,
      57,   155,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    52,    53,     0,    54,    55,    56,    57,
     156,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    52,    53,     0,    54,    55,    56,    57,   177,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    52,    53,     0,    54,    55,    56,    57,   178,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
       0,     0,     0,    52,    53,   151,    54,    55,    56,    57,
       0,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,     0,     0,     0,    52,    53,   153,    54,    55,
      56,    57,     0,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,     0,    52,    53,   144,    54,    55,
      56,    57,     0,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    52,    53,     0,    54,    55,    56,
      57,     0,     0,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    52,    53,     0,    54,    55,    56,    57,
       0,     0,     0,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    52,    53,     0,    54,    55,    56,    57,     0,
       0,     0,     0,    61,    62,    63,    64,    65,    66,    67,
      68,    52,    53,     0,    54,    55,    56,    57,    52,    53,
       0,    54,    55,    62,    63,    64,    65,    66,    67,    68,
      62,    63,    64,    65,    66,    67,    68
};

static const short yycheck[] =
{
      12,    16,    14,    43,    16,    14,    42,    47,    48,     3,
      46,     5,    18,    19,    26,    37,    28,    39,    43,    31,
      32,    33,    34,    35,    42,    34,    44,    42,    34,    35,
      36,    42,    44,    44,    47,    48,    43,    15,    43,    43,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,     3,     4,    43,
      43,     7,     8,    43,    43,     0,    43,    13,    18,    19,
      38,     3,     4,    44,    44,     7,     8,     3,    43,    25,
      43,    13,    32,    33,    34,    35,    36,    33,     3,     3,
      49,    37,     3,    25,    23,    41,    44,    43,    44,    45,
      39,    33,    23,   115,   116,    37,    44,    44,     3,    41,
       3,    43,     3,    45,     3,     4,    41,     6,     7,     8,
       9,    10,    11,    12,    13,    17,    23,    16,     3,     3,
      47,    44,   146,    18,   146,   176,   144,    -1,    -1,   151,
      -1,   153,    29,    -1,    33,    34,    -1,    -1,    37,    -1,
      -1,    -1,    41,    -1,    43,     3,     4,    -1,     6,     7,
       8,     9,    10,    11,    12,    13,     3,     4,    -1,    -1,
       7,     8,    -1,    -1,    -1,    -1,    13,     3,     4,    -1,
      -1,     7,     8,    -1,    -1,    33,    34,    13,    25,    37,
      -1,    -1,    -1,    41,    -1,    43,    33,    -1,    -1,    25,
      37,    -1,    -1,    -1,    41,    -1,    43,    33,     3,     4,
      -1,    37,     7,     8,    -1,    41,    -1,    43,    13,    18,
      19,    -1,    21,    22,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    33,    -1,
      -1,    -1,    37,    42,    -1,    44,    41,    -1,    43,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    37,    38,    39,    18,    19,    -1,    21,
      22,    23,    24,    -1,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    -1,    -1,    -1,    -1,
      18,    19,    -1,    21,    22,    23,    24,    49,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    44,    18,    19,    -1,
      21,    22,    23,    24,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    18,    19,    -1,    21,
      22,    23,    24,    44,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    18,    19,    -1,    21,    22,
      23,    24,    44,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    18,    19,    -1,    21,    22,    23,
      24,    44,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    18,    19,    -1,    21,    22,    23,    24,
      44,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    18,    19,    -1,    21,    22,    23,    24,    44,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    18,    19,    -1,    21,    22,    23,    24,    44,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      -1,    -1,    -1,    18,    19,    42,    21,    22,    23,    24,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    -1,    -1,    18,    19,    42,    21,    22,
      23,    24,    -1,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    -1,    18,    19,    40,    21,    22,
      23,    24,    -1,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    18,    19,    -1,    21,    22,    23,
      24,    -1,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    18,    19,    -1,    21,    22,    23,    24,
      -1,    -1,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    18,    19,    -1,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    33,    34,    35,
      36,    18,    19,    -1,    21,    22,    23,    24,    18,    19,
      -1,    21,    22,    30,    31,    32,    33,    34,    35,    36,
      30,    31,    32,    33,    34,    35,    36
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     6,     7,     8,     9,    10,    11,    12,
      13,    16,    33,    34,    37,    41,    43,    51,    52,    53,
      55,    56,    57,    58,    62,    63,    43,    47,    48,    64,
      66,    43,    43,    43,    43,    43,    43,    43,    43,    15,
       7,     8,    56,    56,    43,    56,     0,    42,    62,     3,
       5,    54,    18,    19,    21,    22,    23,    24,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    38,
      25,    44,    45,    56,    59,    61,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    18,
      19,    20,    21,    22,    23,    24,    26,    27,    37,    38,
      39,    65,    25,    56,    66,    56,    56,    56,    56,    56,
      44,    14,    34,    44,     3,    43,    43,    56,    59,    44,
      53,     3,    56,    56,    56,    56,    25,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      37,    39,     3,    60,    40,    67,    42,    44,    49,    49,
      44,    42,    44,    42,    44,    44,    44,     3,    44,    23,
      56,    56,    44,    39,    23,    42,    46,    65,    44,    61,
      56,    56,    44,     3,     3,     3,    41,    44,    44,    17,
      23,     3,    63,     3
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
int yyparse ( SelectParser_t * pParser );
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
yyparse ( SelectParser_t * pParser )
#else
int
yyparse (pParser)
     SelectParser_t * pParser ;
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
        case 7:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 10:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 11:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 12:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 13:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 14:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 15:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 16:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 17:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 18:

    { pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 19:

    { pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 20:

    { pParser->AddItem ( "@distinct", &yyvsp[-4], &yyvsp[0] ); ;}
    break;

  case 22:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 23:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 24:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 25:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 26:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 27:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 28:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 29:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 30:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 31:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 32:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 33:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 34:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 35:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 36:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 37:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 38:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 39:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 40:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 41:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 44:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 45:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 46:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 49:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 50:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 51:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 52:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 53:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 54:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 55:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 63:

    {
			pParser->AddOption ( &yyvsp[-3], &yyvsp[-1] );
		;}
    break;

  case 64:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 66:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 92:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 93:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 94:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 95:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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
	    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
//yyerrlab1:

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

