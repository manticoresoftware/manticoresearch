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
     TOK_ATIDENT = 259,
     TOK_CONST_INT = 260,
     TOK_CONST_FLOAT = 261,
     TOK_CONST_MVA = 262,
     TOK_QUOTED_STRING = 263,
     TOK_USERVAR = 264,
     TOK_SYSVAR = 265,
     TOK_AS = 266,
     TOK_ASC = 267,
     TOK_AVG = 268,
     TOK_BEGIN = 269,
     TOK_BETWEEN = 270,
     TOK_BY = 271,
     TOK_CALL = 272,
     TOK_COLLATION = 273,
     TOK_COMMIT = 274,
     TOK_COUNT = 275,
     TOK_CREATE = 276,
     TOK_DELETE = 277,
     TOK_DESC = 278,
     TOK_DESCRIBE = 279,
     TOK_DISTINCT = 280,
     TOK_DIV = 281,
     TOK_DROP = 282,
     TOK_FALSE = 283,
     TOK_FLOAT = 284,
     TOK_FROM = 285,
     TOK_FUNCTION = 286,
     TOK_GLOBAL = 287,
     TOK_GROUP = 288,
     TOK_ID = 289,
     TOK_IN = 290,
     TOK_INSERT = 291,
     TOK_INT = 292,
     TOK_INTO = 293,
     TOK_LIMIT = 294,
     TOK_MATCH = 295,
     TOK_MAX = 296,
     TOK_META = 297,
     TOK_MIN = 298,
     TOK_MOD = 299,
     TOK_NAMES = 300,
     TOK_NULL = 301,
     TOK_OPTION = 302,
     TOK_ORDER = 303,
     TOK_RAND = 304,
     TOK_REPLACE = 305,
     TOK_RETURNS = 306,
     TOK_ROLLBACK = 307,
     TOK_SELECT = 308,
     TOK_SET = 309,
     TOK_SESSION = 310,
     TOK_SHOW = 311,
     TOK_SONAME = 312,
     TOK_START = 313,
     TOK_STATUS = 314,
     TOK_SUM = 315,
     TOK_TABLES = 316,
     TOK_TRANSACTION = 317,
     TOK_TRUE = 318,
     TOK_UPDATE = 319,
     TOK_VALUES = 320,
     TOK_VARIABLES = 321,
     TOK_WARNINGS = 322,
     TOK_WEIGHT = 323,
     TOK_WHERE = 324,
     TOK_WITHIN = 325,
     TOK_OR = 326,
     TOK_AND = 327,
     TOK_NE = 328,
     TOK_GTE = 329,
     TOK_LTE = 330,
     TOK_NOT = 331,
     TOK_NEG = 332
   };
#endif
#define TOK_IDENT 258
#define TOK_ATIDENT 259
#define TOK_CONST_INT 260
#define TOK_CONST_FLOAT 261
#define TOK_CONST_MVA 262
#define TOK_QUOTED_STRING 263
#define TOK_USERVAR 264
#define TOK_SYSVAR 265
#define TOK_AS 266
#define TOK_ASC 267
#define TOK_AVG 268
#define TOK_BEGIN 269
#define TOK_BETWEEN 270
#define TOK_BY 271
#define TOK_CALL 272
#define TOK_COLLATION 273
#define TOK_COMMIT 274
#define TOK_COUNT 275
#define TOK_CREATE 276
#define TOK_DELETE 277
#define TOK_DESC 278
#define TOK_DESCRIBE 279
#define TOK_DISTINCT 280
#define TOK_DIV 281
#define TOK_DROP 282
#define TOK_FALSE 283
#define TOK_FLOAT 284
#define TOK_FROM 285
#define TOK_FUNCTION 286
#define TOK_GLOBAL 287
#define TOK_GROUP 288
#define TOK_ID 289
#define TOK_IN 290
#define TOK_INSERT 291
#define TOK_INT 292
#define TOK_INTO 293
#define TOK_LIMIT 294
#define TOK_MATCH 295
#define TOK_MAX 296
#define TOK_META 297
#define TOK_MIN 298
#define TOK_MOD 299
#define TOK_NAMES 300
#define TOK_NULL 301
#define TOK_OPTION 302
#define TOK_ORDER 303
#define TOK_RAND 304
#define TOK_REPLACE 305
#define TOK_RETURNS 306
#define TOK_ROLLBACK 307
#define TOK_SELECT 308
#define TOK_SET 309
#define TOK_SESSION 310
#define TOK_SHOW 311
#define TOK_SONAME 312
#define TOK_START 313
#define TOK_STATUS 314
#define TOK_SUM 315
#define TOK_TABLES 316
#define TOK_TRANSACTION 317
#define TOK_TRUE 318
#define TOK_UPDATE 319
#define TOK_VALUES 320
#define TOK_VARIABLES 321
#define TOK_WARNINGS 322
#define TOK_WEIGHT 323
#define TOK_WHERE 324
#define TOK_WITHIN 325
#define TOK_OR 326
#define TOK_AND 327
#define TOK_NE 328
#define TOK_GTE 329
#define TOK_LTE 330
#define TOK_NOT 331
#define TOK_NEG 332




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif


// some helpers
#include <float.h> // for FLT_MAX



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
#define YYFINAL  80
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   709

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  92
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  75
/* YYNRULES -- Number of rules. */
#define YYNRULES  213
/* YYNRULES -- Number of states. */
#define YYNSTATES  398

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   332

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    85,    74,     2,
      90,    91,    83,    81,    89,    82,     2,    84,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    88,
      77,    75,    78,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    73,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    76,    79,
      80,    86,    87
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      42,    44,    46,    57,    59,    63,    65,    68,    69,    71,
      74,    76,    81,    86,    91,    96,   101,   105,   111,   113,
     117,   118,   120,   123,   125,   129,   134,   138,   142,   148,
     155,   159,   164,   170,   174,   178,   182,   186,   190,   194,
     198,   202,   208,   212,   216,   218,   220,   225,   229,   231,
     233,   236,   238,   241,   243,   247,   248,   250,   254,   255,
     257,   263,   264,   266,   270,   276,   278,   282,   284,   287,
     290,   291,   293,   296,   301,   302,   304,   307,   309,   313,
     317,   321,   327,   334,   336,   340,   344,   346,   348,   350,
     352,   354,   356,   359,   362,   366,   370,   374,   378,   382,
     386,   390,   394,   398,   402,   406,   410,   414,   418,   422,
     426,   430,   434,   436,   441,   446,   450,   457,   464,   466,
     470,   472,   474,   477,   479,   481,   483,   485,   487,   489,
     491,   493,   498,   503,   508,   512,   517,   525,   531,   533,
     535,   537,   539,   541,   543,   545,   547,   549,   552,   559,
     561,   563,   564,   568,   570,   574,   576,   580,   584,   586,
     590,   592,   594,   596,   600,   608,   618,   625,   626,   629,
     631,   635,   639,   640,   642,   644,   646,   649,   651,   653,
     656,   662,   664,   668,   672,   678,   683,   687,   688,   690,
     693,   701,   703,   705
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      93,     0,    -1,    94,    -1,    95,    -1,    95,    88,    -1,
     140,    -1,   148,    -1,   134,    -1,   135,    -1,   138,    -1,
     149,    -1,   155,    -1,   157,    -1,   158,    -1,   161,    -1,
     163,    -1,   164,    -1,   166,    -1,    96,    -1,    95,    88,
      96,    -1,    97,    -1,   131,    -1,    53,    98,    30,   102,
     103,   111,   113,   115,   119,   121,    -1,    99,    -1,    98,
      89,    99,    -1,    83,    -1,   101,   100,    -1,    -1,     3,
      -1,    11,     3,    -1,   127,    -1,    13,    90,   127,    91,
      -1,    41,    90,   127,    91,    -1,    43,    90,   127,    91,
      -1,    60,    90,   127,    91,    -1,    20,    90,    83,    91,
      -1,    68,    90,    91,    -1,    20,    90,    25,     3,    91,
      -1,     3,    -1,   102,    89,     3,    -1,    -1,   104,    -1,
      69,   105,    -1,   106,    -1,   105,    72,   106,    -1,    40,
      90,     8,    91,    -1,   107,    75,   108,    -1,   107,    76,
     108,    -1,   107,    35,    90,   110,    91,    -1,   107,    86,
      35,    90,   110,    91,    -1,   107,    35,     9,    -1,   107,
      86,    35,     9,    -1,   107,    15,   108,    72,   108,    -1,
     107,    78,   108,    -1,   107,    77,   108,    -1,   107,    79,
     108,    -1,   107,    80,   108,    -1,   107,    75,   109,    -1,
     107,    76,   109,    -1,   107,    78,   109,    -1,   107,    77,
     109,    -1,   107,    15,   109,    72,   109,    -1,   107,    79,
     109,    -1,   107,    80,   109,    -1,     3,    -1,     4,    -1,
      20,    90,    83,    91,    -1,    68,    90,    91,    -1,    34,
      -1,     5,    -1,    82,     5,    -1,     6,    -1,    82,     6,
      -1,   108,    -1,   110,    89,   108,    -1,    -1,   112,    -1,
      33,    16,   107,    -1,    -1,   114,    -1,    70,    33,    48,
      16,   117,    -1,    -1,   116,    -1,    48,    16,   117,    -1,
      48,    16,    49,    90,    91,    -1,   118,    -1,   117,    89,
     118,    -1,   107,    -1,   107,    12,    -1,   107,    23,    -1,
      -1,   120,    -1,    39,     5,    -1,    39,     5,    89,     5,
      -1,    -1,   122,    -1,    47,   123,    -1,   124,    -1,   123,
      89,   124,    -1,     3,    75,     3,    -1,     3,    75,     5,
      -1,     3,    75,    90,   125,    91,    -1,     3,    75,     3,
      90,     8,    91,    -1,   126,    -1,   125,    89,   126,    -1,
       3,    75,   108,    -1,     3,    -1,     4,    -1,    34,    -1,
       5,    -1,     6,    -1,     9,    -1,    82,   127,    -1,    86,
     127,    -1,   127,    81,   127,    -1,   127,    82,   127,    -1,
     127,    83,   127,    -1,   127,    84,   127,    -1,   127,    77,
     127,    -1,   127,    78,   127,    -1,   127,    74,   127,    -1,
     127,    73,   127,    -1,   127,    85,   127,    -1,   127,    26,
     127,    -1,   127,    44,   127,    -1,   127,    80,   127,    -1,
     127,    79,   127,    -1,   127,    75,   127,    -1,   127,    76,
     127,    -1,   127,    72,   127,    -1,   127,    71,   127,    -1,
      90,   127,    91,    -1,   128,    -1,     3,    90,   129,    91,
      -1,    35,    90,   129,    91,    -1,     3,    90,    91,    -1,
      43,    90,   127,    89,   127,    91,    -1,    41,    90,   127,
      89,   127,    91,    -1,   130,    -1,   129,    89,   130,    -1,
     127,    -1,     8,    -1,    56,   132,    -1,    67,    -1,    59,
      -1,    42,    -1,     3,    -1,    46,    -1,     8,    -1,     5,
      -1,     6,    -1,    54,     3,    75,   137,    -1,    54,     3,
      75,   136,    -1,    54,     3,    75,    46,    -1,    54,    45,
     133,    -1,    54,    10,    75,   133,    -1,    54,    32,     9,
      75,    90,   110,    91,    -1,    54,    32,     3,    75,   136,
      -1,     3,    -1,     8,    -1,    63,    -1,    28,    -1,   108,
      -1,    19,    -1,    52,    -1,   139,    -1,    14,    -1,    58,
      62,    -1,   141,    38,     3,   142,    65,   144,    -1,    36,
      -1,    50,    -1,    -1,    90,   143,    91,    -1,   107,    -1,
     143,    89,   107,    -1,   145,    -1,   144,    89,   145,    -1,
      90,   146,    91,    -1,   147,    -1,   146,    89,   147,    -1,
     108,    -1,   109,    -1,     8,    -1,    90,   110,    91,    -1,
      22,    30,     3,    69,    34,    75,   108,    -1,    22,    30,
       3,    69,    34,    35,    90,   110,    91,    -1,    17,     3,
      90,   146,   150,    91,    -1,    -1,    89,   151,    -1,   152,
      -1,   151,    89,   152,    -1,   147,   153,   154,    -1,    -1,
      11,    -1,     3,    -1,    39,    -1,   156,     3,    -1,    24,
      -1,    23,    -1,    56,    61,    -1,    64,   102,    54,   159,
     104,    -1,   160,    -1,   159,    89,   160,    -1,     3,    75,
     108,    -1,     3,    75,    90,   110,    91,    -1,     3,    75,
      90,    91,    -1,    56,   162,    66,    -1,    -1,    55,    -1,
      56,    18,    -1,    21,    31,     3,    51,   165,    57,     8,
      -1,    37,    -1,    29,    -1,    27,    31,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   106,   106,   107,   108,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   130,   131,
     135,   136,   140,   155,   156,   160,   161,   164,   166,   167,
     171,   172,   173,   174,   175,   176,   177,   178,   182,   183,
     186,   188,   192,   196,   197,   201,   206,   213,   221,   229,
     238,   243,   248,   253,   258,   263,   268,   273,   274,   275,
     276,   281,   286,   291,   299,   300,   305,   311,   317,   326,
     327,   331,   332,   336,   342,   348,   350,   354,   361,   363,
     367,   373,   375,   379,   383,   390,   391,   395,   396,   397,
     400,   402,   406,   411,   418,   420,   424,   428,   429,   433,
     438,   443,   449,   457,   462,   469,   479,   480,   481,   482,
     483,   484,   485,   486,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   509,   510,   511,   512,   513,   517,   518,
     522,   523,   529,   533,   534,   535,   541,   542,   543,   544,
     545,   549,   554,   559,   564,   565,   569,   574,   582,   583,
     587,   588,   589,   603,   604,   605,   609,   610,   616,   624,
     625,   628,   630,   634,   635,   639,   640,   644,   648,   649,
     653,   654,   655,   656,   662,   668,   680,   687,   689,   693,
     698,   702,   709,   711,   715,   716,   722,   730,   731,   737,
     743,   751,   752,   756,   760,   764,   773,   776,   777,   781,
     787,   798,   799,   803
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_ATIDENT", 
  "TOK_CONST_INT", "TOK_CONST_FLOAT", "TOK_CONST_MVA", 
  "TOK_QUOTED_STRING", "TOK_USERVAR", "TOK_SYSVAR", "TOK_AS", "TOK_ASC", 
  "TOK_AVG", "TOK_BEGIN", "TOK_BETWEEN", "TOK_BY", "TOK_CALL", 
  "TOK_COLLATION", "TOK_COMMIT", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", 
  "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", 
  "TOK_GROUP", "TOK_ID", "TOK_IN", "TOK_INSERT", "TOK_INT", "TOK_INTO", 
  "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", 
  "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_RAND", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_SELECT", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_SUM", "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", "')'", 
  "$accept", "request", "statement", "multi_stmt_list", "multi_stmt", 
  "select_from", "select_items_list", "select_item", "opt_alias", 
  "select_expr", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "where_item", "expr_ident", "const_int", "const_float", 
  "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "show_clause", "show_variable", "set_value", 
  "set_clause", "set_global_clause", "set_string_value", "boolean_value", 
  "transact_op", "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "opt_call_opts_list", "call_opts_list", "call_opt", "opt_as", 
  "call_opt_name", "describe", "describe_tok", "show_tables", "update", 
  "update_items_list", "update_item", "show_variables", "opt_session", 
  "show_collation", "create_function", "udf_type", "drop_function", 0
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
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   124,    38,    61,   328,    60,    62,   329,
     330,    43,    45,    42,    47,    37,   331,   332,    59,    44,
      40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    92,    93,    93,    93,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    95,    95,
      96,    96,    97,    98,    98,    99,    99,   100,   100,   100,
     101,   101,   101,   101,   101,   101,   101,   101,   102,   102,
     103,   103,   104,   105,   105,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   107,   107,   107,   107,   107,   108,
     108,   109,   109,   110,   110,   111,   111,   112,   113,   113,
     114,   115,   115,   116,   116,   117,   117,   118,   118,   118,
     119,   119,   120,   120,   121,   121,   122,   123,   123,   124,
     124,   124,   124,   125,   125,   126,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   128,   128,   128,   128,   128,   129,   129,
     130,   130,   131,   132,   132,   132,   133,   133,   133,   133,
     133,   134,   134,   134,   134,   134,   135,   135,   136,   136,
     137,   137,   137,   138,   138,   138,   139,   139,   140,   141,
     141,   142,   142,   143,   143,   144,   144,   145,   146,   146,
     147,   147,   147,   147,   148,   148,   149,   150,   150,   151,
     151,   152,   153,   153,   154,   154,   155,   156,   156,   157,
     158,   159,   159,   160,   160,   160,   161,   162,   162,   163,
     164,   165,   165,   166
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,    10,     1,     3,     1,     2,     0,     1,     2,
       1,     4,     4,     4,     4,     4,     3,     5,     1,     3,
       0,     1,     2,     1,     3,     4,     3,     3,     5,     6,
       3,     4,     5,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     3,     3,     1,     1,     4,     3,     1,     1,
       2,     1,     2,     1,     3,     0,     1,     3,     0,     1,
       5,     0,     1,     3,     5,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     6,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     4,     4,     3,     6,     6,     1,     3,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     4,     4,     3,     4,     7,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     6,     1,
       1,     0,     3,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     3,     7,     9,     6,     0,     2,     1,
       3,     3,     0,     1,     1,     1,     2,     1,     1,     2,
       5,     1,     3,     3,     5,     4,     3,     0,     1,     2,
       7,     1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   166,     0,   163,     0,     0,   198,   197,     0,   169,
     170,   164,     0,     0,   207,     0,     0,     0,     2,     3,
      18,    20,    21,     7,     8,     9,   165,     5,     0,     6,
      10,    11,     0,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,   106,   107,   109,   110,   111,     0,     0,
     108,     0,     0,     0,     0,     0,     0,    25,     0,     0,
       0,    23,    27,    30,   132,     0,     0,     0,     0,   209,
     145,   208,   144,   199,   143,   142,     0,   167,    38,     0,
       1,     4,     0,   196,     0,     0,     0,   213,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   112,   113,
       0,     0,     0,    28,     0,    26,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   146,   149,   150,
     148,   147,   154,   206,     0,     0,     0,    19,   171,    69,
      71,   182,     0,     0,   180,   181,   187,   178,     0,     0,
     141,   135,   140,     0,   138,     0,     0,     0,     0,     0,
       0,     0,    36,     0,     0,   131,    40,    24,    29,   123,
     124,   130,   129,   121,   120,   127,   128,   118,   119,   126,
     125,   114,   115,   116,   117,   122,   158,   159,   161,   153,
     160,     0,   162,   152,   151,   155,     0,     0,     0,     0,
     201,    39,     0,     0,    70,    72,    73,     0,     0,     0,
     212,   211,     0,     0,     0,   133,    31,     0,    35,   134,
       0,    32,     0,    33,    34,     0,     0,     0,    75,    41,
     157,     0,     0,     0,   200,    64,    65,     0,    68,     0,
     173,     0,     0,     0,   183,   179,   188,   189,   186,     0,
       0,     0,   139,    37,     0,     0,     0,    42,    43,     0,
       0,    78,    76,     0,     0,   203,   202,     0,     0,     0,
     172,     0,   168,   175,    74,   193,     0,     0,   210,     0,
     184,   137,   136,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    81,    79,   156,   205,
       0,     0,    67,   174,     0,     0,   194,   195,   191,   192,
     190,     0,     0,    44,     0,     0,    50,     0,    46,    57,
      47,    58,    54,    60,    53,    59,    55,    62,    56,    63,
       0,    77,     0,     0,    90,    82,   204,    66,     0,   177,
     176,   185,    45,     0,     0,     0,    51,     0,     0,     0,
       0,    94,    91,   179,    52,     0,    61,    48,     0,     0,
       0,    87,    83,    85,    92,     0,    22,    95,    49,    80,
       0,    88,    89,     0,     0,     0,    96,    97,    84,    86,
      93,     0,     0,    99,   100,     0,    98,     0,     0,     0,
     103,     0,     0,     0,   101,   102,   105,   104
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    60,    61,   105,    62,
      79,   228,   229,   257,   258,   361,   206,   145,   207,   261,
     262,   296,   297,   334,   335,   362,   363,   351,   352,   366,
     367,   376,   377,   389,   390,   152,    64,   153,   154,    22,
      75,   132,    23,    24,   193,   194,    25,    26,    27,    28,
     203,   241,   272,   273,   146,   147,    29,    30,   209,   246,
     247,   276,   308,    31,    32,    33,    34,   199,   200,    35,
      76,    36,    37,   212,    38
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -269
static const short yypact[] =
{
     645,  -269,    29,  -269,    82,    96,  -269,  -269,    98,  -269,
    -269,  -269,   157,   242,   218,    81,   151,   155,  -269,    93,
    -269,  -269,  -269,  -269,  -269,  -269,  -269,  -269,   131,  -269,
    -269,  -269,   183,  -269,  -269,  -269,  -269,  -269,  -269,    99,
     187,   193,   196,   120,  -269,  -269,  -269,  -269,   128,   129,
    -269,   140,   147,   156,   160,   171,   229,  -269,   229,   229,
     -17,  -269,    22,   465,  -269,   127,   139,   176,   122,  -269,
    -269,  -269,  -269,  -269,  -269,  -269,   199,  -269,  -269,    67,
    -269,   -38,   238,  -269,    32,   211,   206,  -269,   130,   229,
      54,   141,   229,   229,   229,   190,   192,   205,  -269,  -269,
     272,   151,   157,  -269,   280,  -269,   229,   229,   229,   229,
     229,   229,   229,   229,   229,   229,   229,   229,   229,   229,
     229,   229,   229,   221,   122,   222,   225,  -269,  -269,  -269,
    -269,  -269,  -269,  -269,   283,   293,   -15,  -269,   212,  -269,
    -269,  -269,    37,    11,  -269,  -269,   215,  -269,   143,   267,
    -269,  -269,   465,   -60,  -269,   294,   302,   216,    51,   -16,
     250,   316,  -269,   229,   229,  -269,   -55,  -269,  -269,  -269,
    -269,   484,   507,   526,   549,   568,   568,   573,   573,   573,
     573,   404,   404,  -269,  -269,  -269,  -269,  -269,  -269,  -269,
    -269,   301,  -269,  -269,  -269,  -269,   175,   219,   235,   -43,
    -269,  -269,    50,   247,  -269,  -269,  -269,   106,    32,   223,
    -269,  -269,   256,    42,   141,  -269,  -269,   226,  -269,  -269,
     229,  -269,   229,  -269,  -269,   423,   442,   154,   285,  -269,
    -269,    11,    30,   283,  -269,  -269,  -269,   246,  -269,   268,
    -269,   124,   269,    11,  -269,   112,   248,  -269,  -269,   332,
     271,    11,  -269,  -269,   338,   360,   290,   309,  -269,   213,
     346,   313,  -269,   153,     3,  -269,  -269,   319,   312,    50,
    -269,    32,   295,  -269,  -269,  -269,    47,    32,  -269,    11,
    -269,  -269,  -269,   397,   154,     6,    -3,     6,     6,     6,
       6,     6,     6,   371,    50,   375,   376,  -269,  -269,  -269,
     162,   334,  -269,  -269,   166,   269,  -269,  -269,  -269,   415,
    -269,   167,   336,  -269,   356,   378,  -269,    11,  -269,  -269,
    -269,  -269,  -269,  -269,  -269,  -269,  -269,  -269,  -269,  -269,
       0,  -269,   405,   436,   416,  -269,  -269,  -269,    32,  -269,
    -269,  -269,  -269,    11,    -2,   177,  -269,    11,   438,   443,
     451,   410,  -269,  -269,  -269,   452,  -269,  -269,   180,    50,
     369,    18,   372,  -269,   373,   457,  -269,  -269,  -269,   372,
     374,  -269,  -269,    50,   459,   391,   380,  -269,  -269,  -269,
    -269,     2,   457,   381,  -269,   467,  -269,   464,   398,   189,
    -269,   383,    11,   467,  -269,  -269,  -269,  -269
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -269,  -269,  -269,  -269,   394,  -269,  -269,   377,  -269,  -269,
     379,  -269,   277,  -269,   194,  -153,   -84,  -268,  -228,  -269,
    -269,  -269,  -269,  -269,  -269,   123,   108,  -269,  -269,  -269,
    -269,  -269,   101,  -269,    91,   -11,  -269,   399,   279,  -269,
    -269,   361,  -269,  -269,   333,  -269,  -269,  -269,  -269,  -269,
    -269,  -269,  -269,   227,   259,  -206,  -269,  -269,  -269,  -269,
     257,  -269,  -269,  -269,  -269,  -269,  -269,  -269,   320,  -269,
    -269,  -269,  -269,  -269,  -269
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -193
static const short yytable[] =
{
     144,    63,   245,   263,   140,   383,   316,   384,   139,   346,
     106,   139,   140,   101,   227,    12,   139,   315,   136,   319,
     321,   323,   325,   327,   329,   103,   227,    70,   107,   214,
     371,   215,    39,   104,   135,   139,   300,   139,   140,   192,
     141,   372,   204,   205,    72,    98,   233,    99,   100,   240,
     306,   311,    74,   235,   236,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     237,   309,   102,   220,   259,   221,   356,   250,   155,   156,
     355,   159,   160,   161,   238,   191,   307,   317,   142,   345,
     347,    63,   385,   191,   299,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   191,    40,   142,  -192,   303,   251,   239,   358,
     264,   134,   143,   275,   144,   127,    41,   128,   129,    42,
     130,   259,   353,    43,    44,    45,    46,   157,   150,    47,
     214,   331,   219,    77,    43,    44,    45,    46,   265,   150,
      47,  -192,   225,   226,    78,    80,   135,   235,   236,   274,
      43,    44,    45,    46,    50,    51,    47,   280,   131,    82,
      48,    96,   210,    97,   237,    50,    51,    49,   186,   125,
     211,    81,    96,   187,    97,   126,    83,   144,   238,    84,
      85,    50,    51,   144,   256,   243,    86,   244,    52,    87,
      53,   314,   123,   318,   320,   322,   324,   326,   328,   254,
      88,   255,    56,   269,   124,   270,    58,    54,    89,    90,
      59,   151,   239,    56,   186,    55,   139,    58,   285,   187,
      91,    59,    43,    44,    45,    46,    69,    92,    47,    56,
      57,   138,   243,    58,   298,    65,    93,    59,   286,   188,
      94,   243,    66,   336,   144,   338,   243,   339,   341,   354,
      70,    95,   148,    50,    51,   133,   243,   189,   357,   243,
      96,   368,    97,    71,    67,   149,   106,    72,   393,    73,
     394,   162,   163,   168,   190,    74,   198,    68,   287,   288,
     289,   290,   291,   292,   107,   164,   201,   196,   106,   293,
     197,   213,   202,   191,   208,   217,   204,   218,   396,   231,
     232,    56,   242,   249,   248,    58,   107,   253,   260,    59,
     106,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   267,   277,   107,   222,
     278,   223,   106,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   268,   271,
     107,   279,   294,   165,   106,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     283,   284,   107,   295,   305,   216,   106,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   301,   302,   107,   312,   330,   224,   332,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   333,   337,   275,   342,   343,   281,
     106,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   235,   236,   107,   106,
     344,   282,   349,   348,   359,   350,   364,   365,   205,   370,
     375,   373,   374,   237,   380,   378,   381,   107,   106,   382,
     388,   387,   391,   392,   395,   137,   234,   238,   313,   167,
     166,   379,   369,   386,   397,   195,   107,   120,   121,   122,
     158,   106,   360,   252,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   107,
     106,   239,   220,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   107,   230,
     304,   222,   340,   106,   310,     0,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   107,   106,   266,     0,     0,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     107,     0,     0,     0,     0,   106,     0,     0,     0,     0,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   107,   106,     0,     0,     0,     0,   106,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   107,     0,     0,     0,     0,   107,     0,     0,
       0,     0,     0,     0,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   118,   119,   120,   121,   122,     1,
       0,     0,     2,     0,     3,     0,     4,     5,     6,     7,
       0,     0,     8,     0,     0,     0,     0,     0,     0,     0,
       0,     9,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    10,     0,    11,    12,    13,
       0,    14,     0,    15,     0,     0,     0,     0,     0,    16
};

static const short yycheck[] =
{
      84,    12,   208,   231,     6,     3,     9,     5,     5,     9,
      26,     5,     6,    30,    69,    53,     5,   285,    56,   287,
     288,   289,   290,   291,   292,     3,    69,    42,    44,    89,
      12,    91,     3,    11,    89,     5,   264,     5,     6,   123,
       8,    23,     5,     6,    59,    56,    89,    58,    59,   202,
       3,   279,    67,     3,     4,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      20,   277,    89,    89,   227,    91,   344,    35,    89,    25,
      82,    92,    93,    94,    34,    82,    39,    90,    82,   317,
      90,   102,    90,    82,    91,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,    82,    31,    82,     3,   269,    75,    68,   347,
      90,    54,    90,    11,   208,     3,    30,     5,     6,    31,
       8,   284,   338,     3,     4,     5,     6,    83,     8,     9,
      89,   294,    91,    62,     3,     4,     5,     6,   232,     8,
       9,    39,   163,   164,     3,     0,    89,     3,     4,   243,
       3,     4,     5,     6,    34,    35,     9,   251,    46,    38,
      13,    41,    29,    43,    20,    34,    35,    20,     3,     3,
      37,    88,    41,     8,    43,     9,     3,   271,    34,    90,
       3,    34,    35,   277,    40,    89,     3,    91,    41,     3,
      43,   285,    75,   287,   288,   289,   290,   291,   292,   220,
      90,   222,    82,    89,    75,    91,    86,    60,    90,    90,
      90,    91,    68,    82,     3,    68,     5,    86,    15,     8,
      90,    90,     3,     4,     5,     6,    18,    90,     9,    82,
      83,     3,    89,    86,    91,     3,    90,    90,    35,    28,
      90,    89,    10,    91,   338,    89,    89,    91,    91,   343,
      42,    90,    51,    34,    35,    66,    89,    46,    91,    89,
      41,    91,    43,    55,    32,    69,    26,    59,    89,    61,
      91,    91,    90,     3,    63,    67,     3,    45,    75,    76,
      77,    78,    79,    80,    44,    90,     3,    75,    26,    86,
      75,    34,    90,    82,    89,     3,     5,    91,   392,    90,
      75,    82,    65,    57,    91,    86,    44,    91,    33,    90,
      26,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    90,    89,    44,    89,
       8,    91,    26,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    90,    90,
      44,    90,    16,    91,    26,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      90,    72,    44,    70,    89,    91,    26,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    83,    91,    44,     8,    35,    91,    33,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    48,    91,    11,    91,    72,    91,
      26,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,     3,     4,    44,    26,
      72,    91,    16,    48,    16,    39,     5,    47,     6,    90,
       3,    89,    89,    20,     5,    91,    75,    44,    26,    89,
       3,    90,     8,    75,    91,    81,   199,    34,   284,   102,
     101,   373,   359,   382,   393,   124,    44,    83,    84,    85,
      91,    26,    49,   214,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    44,
      26,    68,    89,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    44,   196,
     271,    89,   305,    26,   277,    -1,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    44,    26,   233,    -1,    -1,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      44,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    44,    26,    -1,    -1,    -1,    -1,    26,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    44,    -1,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    81,    82,    83,    84,    85,    14,
      -1,    -1,    17,    -1,    19,    -1,    21,    22,    23,    24,
      -1,    -1,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    52,    53,    54,
      -1,    56,    -1,    58,    -1,    -1,    -1,    -1,    -1,    64
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    17,    19,    21,    22,    23,    24,    27,    36,
      50,    52,    53,    54,    56,    58,    64,    93,    94,    95,
      96,    97,   131,   134,   135,   138,   139,   140,   141,   148,
     149,   155,   156,   157,   158,   161,   163,   164,   166,     3,
      31,    30,    31,     3,     4,     5,     6,     9,    13,    20,
      34,    35,    41,    43,    60,    68,    82,    83,    86,    90,
      98,    99,   101,   127,   128,     3,    10,    32,    45,    18,
      42,    55,    59,    61,    67,   132,   162,    62,     3,   102,
       0,    88,    38,     3,    90,     3,     3,     3,    90,    90,
      90,    90,    90,    90,    90,    90,    41,    43,   127,   127,
     127,    30,    89,     3,    11,   100,    26,    44,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    75,    75,     3,     9,     3,     5,     6,
       8,    46,   133,    66,    54,    89,    56,    96,     3,     5,
       6,     8,    82,    90,   108,   109,   146,   147,    51,    69,
       8,    91,   127,   129,   130,   127,    25,    83,   129,   127,
     127,   127,    91,    90,    90,    91,   102,    99,     3,   127,
     127,   127,   127,   127,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   127,   127,     3,     8,    28,    46,
      63,    82,   108,   136,   137,   133,    75,    75,     3,   159,
     160,     3,    90,   142,     5,     6,   108,   110,    89,   150,
      29,    37,   165,    34,    89,    91,    91,     3,    91,    91,
      89,    91,    89,    91,    91,   127,   127,    69,   103,   104,
     136,    90,    75,    89,   104,     3,     4,    20,    34,    68,
     107,   143,    65,    89,    91,   147,   151,   152,    91,    57,
      35,    75,   130,    91,   127,   127,    40,   105,   106,   107,
      33,   111,   112,   110,    90,   108,   160,    90,    90,    89,
      91,    90,   144,   145,   108,    11,   153,    89,     8,    90,
     108,    91,    91,    90,    72,    15,    35,    75,    76,    77,
      78,    79,    80,    86,    16,    70,   113,   114,    91,    91,
     110,    83,    91,   107,   146,    89,     3,    39,   154,   147,
     152,   110,     8,   106,   108,   109,     9,    90,   108,   109,
     108,   109,   108,   109,   108,   109,   108,   109,   108,   109,
      35,   107,    33,    48,   115,   116,    91,    91,    89,    91,
     145,    91,    91,    72,    72,   110,     9,    90,    48,    16,
      39,   119,   120,   147,   108,    82,   109,    91,   110,    16,
      49,   107,   117,   118,     5,    47,   121,   122,    91,   117,
      90,    12,    23,    89,    89,     3,   123,   124,    91,   118,
       5,    75,    89,     3,     5,    90,   124,    90,     3,   125,
     126,     8,    75,    89,    91,    91,   108,   126
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
int yyparse ( SqlParser_c * pParser );
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
yyparse ( SqlParser_c * pParser )
#else
int
yyparse (pParser)
     SqlParser_c * pParser ;
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
        case 2:

    { pParser->PushQuery(); ;}
    break;

  case 18:

    { pParser->PushQuery(); ;}
    break;

  case 19:

    { pParser->PushQuery(); ;}
    break;

  case 22:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 25:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 28:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 29:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 30:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 31:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 32:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 33:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 34:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 35:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 36:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 37:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 39:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 45:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 46:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 47:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 48:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 49:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 50:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 51:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 52:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 53:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 54:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 55:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 56:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 60:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 61:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 65:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 66:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 67:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 68:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 69:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 70:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 71:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 72:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 73:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 74:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 77:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 80:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 83:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 84:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 86:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 88:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 89:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 92:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 93:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 99:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 100:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 101:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 102:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 103:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 104:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 105:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 107:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 108:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 112:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 113:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 114:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 115:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 116:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 117:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 118:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 119:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 120:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 121:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 122:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 123:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 125:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 126:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 127:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 128:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 129:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 130:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 131:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 133:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 134:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 135:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 136:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 137:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 143:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 144:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 145:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 151:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 152:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 153:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 154:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 155:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 156:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 157:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 160:

    { yyval.m_iValue = 1; ;}
    break;

  case 161:

    { yyval.m_iValue = 0; ;}
    break;

  case 162:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 163:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 164:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 165:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 168:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 169:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 173:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 174:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 177:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 178:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 179:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 180:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 181:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 182:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 183:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_iValue = yyvsp[-1].m_pValues->GetLength(); yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 184:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 185:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 186:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 189:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 191:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 195:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 199:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 200:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 203:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, (DWORD) yyvsp[0].m_iValue );
		;}
    break;

  case 204:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 205:

    {
			pParser->UpdateAttr ( yyvsp[-3].m_sValue, NULL, SPH_ATTR_UINT32SET );
		;}
    break;

  case 206:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 209:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 210:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 211:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 212:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 213:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
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

	      if (yycount < 4)
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
	      else
		{
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			snprintf (yyp, (int)(yysize - (yyp - yymsg)), ", expecting %s (or %d other tokens)", yytname[yyx], yycount - 1);
			while (*yyp++);
			break;
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

