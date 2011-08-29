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
     TOK_ATTACH = 268,
     TOK_AVG = 269,
     TOK_BEGIN = 270,
     TOK_BETWEEN = 271,
     TOK_BY = 272,
     TOK_CALL = 273,
     TOK_COLLATION = 274,
     TOK_COMMIT = 275,
     TOK_COUNT = 276,
     TOK_CREATE = 277,
     TOK_DELETE = 278,
     TOK_DESC = 279,
     TOK_DESCRIBE = 280,
     TOK_DISTINCT = 281,
     TOK_DIV = 282,
     TOK_DROP = 283,
     TOK_FALSE = 284,
     TOK_FLOAT = 285,
     TOK_FROM = 286,
     TOK_FUNCTION = 287,
     TOK_GLOBAL = 288,
     TOK_GROUP = 289,
     TOK_ID = 290,
     TOK_IN = 291,
     TOK_INDEX = 292,
     TOK_INSERT = 293,
     TOK_INT = 294,
     TOK_INTO = 295,
     TOK_LIMIT = 296,
     TOK_MATCH = 297,
     TOK_MAX = 298,
     TOK_META = 299,
     TOK_MIN = 300,
     TOK_MOD = 301,
     TOK_NAMES = 302,
     TOK_NULL = 303,
     TOK_OPTION = 304,
     TOK_ORDER = 305,
     TOK_RAND = 306,
     TOK_REPLACE = 307,
     TOK_RETURNS = 308,
     TOK_ROLLBACK = 309,
     TOK_RTINDEX = 310,
     TOK_SELECT = 311,
     TOK_SET = 312,
     TOK_SESSION = 313,
     TOK_SHOW = 314,
     TOK_SONAME = 315,
     TOK_START = 316,
     TOK_STATUS = 317,
     TOK_SUM = 318,
     TOK_TABLES = 319,
     TOK_TO = 320,
     TOK_TRANSACTION = 321,
     TOK_TRUE = 322,
     TOK_UPDATE = 323,
     TOK_VALUES = 324,
     TOK_VARIABLES = 325,
     TOK_WARNINGS = 326,
     TOK_WEIGHT = 327,
     TOK_WHERE = 328,
     TOK_WITHIN = 329,
     TOK_OR = 330,
     TOK_AND = 331,
     TOK_NE = 332,
     TOK_GTE = 333,
     TOK_LTE = 334,
     TOK_NOT = 335,
     TOK_NEG = 336
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
#define TOK_ATTACH 268
#define TOK_AVG 269
#define TOK_BEGIN 270
#define TOK_BETWEEN 271
#define TOK_BY 272
#define TOK_CALL 273
#define TOK_COLLATION 274
#define TOK_COMMIT 275
#define TOK_COUNT 276
#define TOK_CREATE 277
#define TOK_DELETE 278
#define TOK_DESC 279
#define TOK_DESCRIBE 280
#define TOK_DISTINCT 281
#define TOK_DIV 282
#define TOK_DROP 283
#define TOK_FALSE 284
#define TOK_FLOAT 285
#define TOK_FROM 286
#define TOK_FUNCTION 287
#define TOK_GLOBAL 288
#define TOK_GROUP 289
#define TOK_ID 290
#define TOK_IN 291
#define TOK_INDEX 292
#define TOK_INSERT 293
#define TOK_INT 294
#define TOK_INTO 295
#define TOK_LIMIT 296
#define TOK_MATCH 297
#define TOK_MAX 298
#define TOK_META 299
#define TOK_MIN 300
#define TOK_MOD 301
#define TOK_NAMES 302
#define TOK_NULL 303
#define TOK_OPTION 304
#define TOK_ORDER 305
#define TOK_RAND 306
#define TOK_REPLACE 307
#define TOK_RETURNS 308
#define TOK_ROLLBACK 309
#define TOK_RTINDEX 310
#define TOK_SELECT 311
#define TOK_SET 312
#define TOK_SESSION 313
#define TOK_SHOW 314
#define TOK_SONAME 315
#define TOK_START 316
#define TOK_STATUS 317
#define TOK_SUM 318
#define TOK_TABLES 319
#define TOK_TO 320
#define TOK_TRANSACTION 321
#define TOK_TRUE 322
#define TOK_UPDATE 323
#define TOK_VALUES 324
#define TOK_VARIABLES 325
#define TOK_WARNINGS 326
#define TOK_WEIGHT 327
#define TOK_WHERE 328
#define TOK_WITHIN 329
#define TOK_OR 330
#define TOK_AND 331
#define TOK_NE 332
#define TOK_GTE 333
#define TOK_LTE 334
#define TOK_NOT 335
#define TOK_NEG 336




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
#define YYFINAL  83
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   714

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  96
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  76
/* YYNRULES -- Number of rules. */
#define YYNRULES  216
/* YYNRULES -- Number of states. */
#define YYNSTATES  406

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   336

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    89,    78,     2,
      94,    95,    87,    85,    93,    86,     2,    88,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    92,
      81,    79,    82,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    77,     2,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    80,    83,    84,    90,    91
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    44,    46,    48,    59,    61,    65,    67,    70,    71,
      73,    76,    78,    83,    88,    93,    98,   103,   107,   113,
     115,   119,   120,   122,   125,   127,   131,   136,   140,   144,
     150,   157,   161,   166,   172,   176,   180,   184,   188,   192,
     196,   200,   204,   210,   214,   218,   220,   222,   227,   231,
     233,   235,   238,   240,   243,   245,   249,   250,   252,   256,
     257,   259,   265,   266,   268,   272,   278,   280,   284,   286,
     289,   292,   293,   295,   298,   303,   304,   306,   309,   311,
     315,   319,   323,   329,   336,   338,   342,   346,   348,   350,
     352,   354,   356,   358,   361,   364,   368,   372,   376,   380,
     384,   388,   392,   396,   400,   404,   408,   412,   416,   420,
     424,   428,   432,   436,   438,   443,   448,   452,   459,   466,
     468,   472,   474,   476,   479,   481,   483,   485,   487,   489,
     491,   493,   495,   500,   505,   510,   514,   519,   527,   533,
     535,   537,   539,   541,   543,   545,   547,   549,   551,   554,
     561,   563,   565,   566,   570,   572,   576,   578,   582,   586,
     588,   592,   594,   596,   598,   602,   610,   620,   627,   628,
     631,   633,   637,   641,   642,   644,   646,   648,   651,   653,
     655,   658,   664,   666,   670,   674,   678,   684,   689,   693,
     694,   696,   699,   707,   709,   711,   715
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      97,     0,    -1,    98,    -1,    99,    -1,    99,    92,    -1,
     144,    -1,   152,    -1,   138,    -1,   139,    -1,   142,    -1,
     153,    -1,   159,    -1,   161,    -1,   162,    -1,   165,    -1,
     167,    -1,   168,    -1,   170,    -1,   171,    -1,   100,    -1,
      99,    92,   100,    -1,   101,    -1,   135,    -1,    56,   102,
      31,   106,   107,   115,   117,   119,   123,   125,    -1,   103,
      -1,   102,    93,   103,    -1,    87,    -1,   105,   104,    -1,
      -1,     3,    -1,    11,     3,    -1,   131,    -1,    14,    94,
     131,    95,    -1,    43,    94,   131,    95,    -1,    45,    94,
     131,    95,    -1,    63,    94,   131,    95,    -1,    21,    94,
      87,    95,    -1,    72,    94,    95,    -1,    21,    94,    26,
       3,    95,    -1,     3,    -1,   106,    93,     3,    -1,    -1,
     108,    -1,    73,   109,    -1,   110,    -1,   109,    76,   110,
      -1,    42,    94,     8,    95,    -1,   111,    79,   112,    -1,
     111,    80,   112,    -1,   111,    36,    94,   114,    95,    -1,
     111,    90,    36,    94,   114,    95,    -1,   111,    36,     9,
      -1,   111,    90,    36,     9,    -1,   111,    16,   112,    76,
     112,    -1,   111,    82,   112,    -1,   111,    81,   112,    -1,
     111,    83,   112,    -1,   111,    84,   112,    -1,   111,    79,
     113,    -1,   111,    80,   113,    -1,   111,    82,   113,    -1,
     111,    81,   113,    -1,   111,    16,   113,    76,   113,    -1,
     111,    83,   113,    -1,   111,    84,   113,    -1,     3,    -1,
       4,    -1,    21,    94,    87,    95,    -1,    72,    94,    95,
      -1,    35,    -1,     5,    -1,    86,     5,    -1,     6,    -1,
      86,     6,    -1,   112,    -1,   114,    93,   112,    -1,    -1,
     116,    -1,    34,    17,   111,    -1,    -1,   118,    -1,    74,
      34,    50,    17,   121,    -1,    -1,   120,    -1,    50,    17,
     121,    -1,    50,    17,    51,    94,    95,    -1,   122,    -1,
     121,    93,   122,    -1,   111,    -1,   111,    12,    -1,   111,
      24,    -1,    -1,   124,    -1,    41,     5,    -1,    41,     5,
      93,     5,    -1,    -1,   126,    -1,    49,   127,    -1,   128,
      -1,   127,    93,   128,    -1,     3,    79,     3,    -1,     3,
      79,     5,    -1,     3,    79,    94,   129,    95,    -1,     3,
      79,     3,    94,     8,    95,    -1,   130,    -1,   129,    93,
     130,    -1,     3,    79,   112,    -1,     3,    -1,     4,    -1,
      35,    -1,     5,    -1,     6,    -1,     9,    -1,    86,   131,
      -1,    90,   131,    -1,   131,    85,   131,    -1,   131,    86,
     131,    -1,   131,    87,   131,    -1,   131,    88,   131,    -1,
     131,    81,   131,    -1,   131,    82,   131,    -1,   131,    78,
     131,    -1,   131,    77,   131,    -1,   131,    89,   131,    -1,
     131,    27,   131,    -1,   131,    46,   131,    -1,   131,    84,
     131,    -1,   131,    83,   131,    -1,   131,    79,   131,    -1,
     131,    80,   131,    -1,   131,    76,   131,    -1,   131,    75,
     131,    -1,    94,   131,    95,    -1,   132,    -1,     3,    94,
     133,    95,    -1,    36,    94,   133,    95,    -1,     3,    94,
      95,    -1,    45,    94,   131,    93,   131,    95,    -1,    43,
      94,   131,    93,   131,    95,    -1,   134,    -1,   133,    93,
     134,    -1,   131,    -1,     8,    -1,    59,   136,    -1,    71,
      -1,    62,    -1,    44,    -1,     3,    -1,    48,    -1,     8,
      -1,     5,    -1,     6,    -1,    57,     3,    79,   141,    -1,
      57,     3,    79,   140,    -1,    57,     3,    79,    48,    -1,
      57,    47,   137,    -1,    57,    10,    79,   137,    -1,    57,
      33,     9,    79,    94,   114,    95,    -1,    57,    33,     3,
      79,   140,    -1,     3,    -1,     8,    -1,    67,    -1,    29,
      -1,   112,    -1,    20,    -1,    54,    -1,   143,    -1,    15,
      -1,    61,    66,    -1,   145,    40,     3,   146,    69,   148,
      -1,    38,    -1,    52,    -1,    -1,    94,   147,    95,    -1,
     111,    -1,   147,    93,   111,    -1,   149,    -1,   148,    93,
     149,    -1,    94,   150,    95,    -1,   151,    -1,   150,    93,
     151,    -1,   112,    -1,   113,    -1,     8,    -1,    94,   114,
      95,    -1,    23,    31,     3,    73,    35,    79,   112,    -1,
      23,    31,     3,    73,    35,    36,    94,   114,    95,    -1,
      18,     3,    94,   150,   154,    95,    -1,    -1,    93,   155,
      -1,   156,    -1,   155,    93,   156,    -1,   151,   157,   158,
      -1,    -1,    11,    -1,     3,    -1,    41,    -1,   160,     3,
      -1,    25,    -1,    24,    -1,    59,    64,    -1,    68,   106,
      57,   163,   108,    -1,   164,    -1,   163,    93,   164,    -1,
       3,    79,   112,    -1,     3,    79,   113,    -1,     3,    79,
      94,   114,    95,    -1,     3,    79,    94,    95,    -1,    59,
     166,    70,    -1,    -1,    58,    -1,    59,    19,    -1,    22,
      32,     3,    53,   169,    60,     8,    -1,    39,    -1,    30,
      -1,    28,    32,     3,    -1,    13,    37,     3,    65,    55,
       3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   110,   110,   111,   112,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   135,
     136,   140,   141,   145,   160,   161,   165,   166,   169,   171,
     172,   176,   177,   178,   179,   180,   181,   182,   183,   187,
     188,   191,   193,   197,   201,   202,   206,   211,   218,   226,
     234,   243,   248,   253,   258,   263,   268,   273,   278,   279,
     280,   281,   286,   291,   296,   304,   305,   310,   316,   322,
     331,   332,   336,   337,   341,   347,   353,   355,   359,   366,
     368,   372,   378,   380,   384,   388,   395,   396,   400,   401,
     402,   405,   407,   411,   416,   423,   425,   429,   433,   434,
     438,   443,   448,   454,   462,   467,   474,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   500,   501,   502,   503,   504,   505,   506,
     507,   508,   509,   510,   514,   515,   516,   517,   518,   522,
     523,   527,   528,   534,   538,   539,   540,   546,   547,   548,
     549,   550,   554,   559,   564,   569,   570,   574,   579,   587,
     588,   592,   593,   594,   608,   609,   610,   614,   615,   621,
     629,   630,   633,   635,   639,   640,   644,   645,   649,   653,
     654,   658,   659,   660,   661,   667,   673,   685,   692,   694,
     698,   703,   707,   714,   716,   720,   721,   727,   735,   736,
     742,   748,   756,   757,   761,   765,   769,   773,   782,   785,
     786,   790,   796,   807,   808,   812,   823
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
  "TOK_ATTACH", "TOK_AVG", "TOK_BEGIN", "TOK_BETWEEN", "TOK_BY", 
  "TOK_CALL", "TOK_COLLATION", "TOK_COMMIT", "TOK_COUNT", "TOK_CREATE", 
  "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", 
  "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_RAND", "TOK_REPLACE", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SET", "TOK_SESSION", 
  "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_SUM", 
  "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_UPDATE", 
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
  "show_collation", "create_function", "udf_type", "drop_function", 
  "attach_index", 0
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
     325,   326,   327,   328,   329,   330,   331,   124,    38,    61,
     332,    60,    62,   333,   334,    43,    45,    42,    47,    37,
     335,   336,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    96,    97,    97,    97,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    99,
      99,   100,   100,   101,   102,   102,   103,   103,   104,   104,
     104,   105,   105,   105,   105,   105,   105,   105,   105,   106,
     106,   107,   107,   108,   109,   109,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   111,   111,   111,   111,   111,
     112,   112,   113,   113,   114,   114,   115,   115,   116,   117,
     117,   118,   119,   119,   120,   120,   121,   121,   122,   122,
     122,   123,   123,   124,   124,   125,   125,   126,   127,   127,
     128,   128,   128,   128,   129,   129,   130,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   132,   132,   132,   132,   132,   133,
     133,   134,   134,   135,   136,   136,   136,   137,   137,   137,
     137,   137,   138,   138,   138,   138,   138,   139,   139,   140,
     140,   141,   141,   141,   142,   142,   142,   143,   143,   144,
     145,   145,   146,   146,   147,   147,   148,   148,   149,   150,
     150,   151,   151,   151,   151,   152,   152,   153,   154,   154,
     155,   155,   156,   157,   157,   158,   158,   159,   160,   160,
     161,   162,   163,   163,   164,   164,   164,   164,   165,   166,
     166,   167,   168,   169,   169,   170,   171
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,    10,     1,     3,     1,     2,     0,     1,
       2,     1,     4,     4,     4,     4,     4,     3,     5,     1,
       3,     0,     1,     2,     1,     3,     4,     3,     3,     5,
       6,     3,     4,     5,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     3,     3,     1,     1,     4,     3,     1,
       1,     2,     1,     2,     1,     3,     0,     1,     3,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       3,     3,     5,     6,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     4,     4,     3,     6,     6,     1,
       3,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     4,     4,     3,     4,     7,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     6,
       1,     1,     0,     3,     1,     3,     1,     3,     3,     1,
       3,     1,     1,     1,     3,     7,     9,     6,     0,     2,
       1,     3,     3,     0,     1,     1,     1,     2,     1,     1,
       2,     5,     1,     3,     3,     3,     5,     4,     3,     0,
       1,     2,     7,     1,     1,     3,     6
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   167,     0,   164,     0,     0,   199,   198,     0,
     170,   171,   165,     0,     0,   209,     0,     0,     0,     2,
       3,    19,    21,    22,     7,     8,     9,   166,     5,     0,
       6,    10,    11,     0,    12,    13,    14,    15,    16,    17,
      18,     0,     0,     0,     0,     0,   107,   108,   110,   111,
     112,     0,     0,   109,     0,     0,     0,     0,     0,     0,
      26,     0,     0,     0,    24,    28,    31,   133,     0,     0,
       0,     0,   211,   146,   210,   145,   200,   144,   143,     0,
     168,    39,     0,     1,     4,     0,   197,     0,     0,     0,
       0,   215,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   113,   114,     0,     0,     0,    29,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,   150,   151,   149,   148,   155,   208,     0,     0,
       0,    20,   172,     0,    70,    72,   183,     0,     0,   181,
     182,   188,   179,     0,     0,   142,   136,   141,     0,   139,
       0,     0,     0,     0,     0,     0,     0,    37,     0,     0,
     132,    41,    25,    30,   124,   125,   131,   130,   122,   121,
     128,   129,   119,   120,   127,   126,   115,   116,   117,   118,
     123,   159,   160,   162,   154,   161,     0,   163,   153,   152,
     156,     0,     0,     0,     0,   202,    40,     0,     0,     0,
      71,    73,    74,     0,     0,     0,   214,   213,     0,     0,
       0,   134,    32,     0,    36,   135,     0,    33,     0,    34,
      35,     0,     0,     0,    76,    42,   158,     0,     0,     0,
     201,    65,    66,     0,    69,     0,   174,     0,     0,   216,
       0,   184,   180,   189,   190,   187,     0,     0,     0,   140,
      38,     0,     0,     0,    43,    44,     0,     0,    79,    77,
       0,     0,   204,   205,   203,     0,     0,     0,   173,     0,
     169,   176,    75,   194,     0,     0,   212,     0,   185,   138,
     137,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    82,    80,   157,   207,     0,     0,
      68,   175,     0,     0,   195,   196,   192,   193,   191,     0,
       0,    45,     0,     0,    51,     0,    47,    58,    48,    59,
      55,    61,    54,    60,    56,    63,    57,    64,     0,    78,
       0,     0,    91,    83,   206,    67,     0,   178,   177,   186,
      46,     0,     0,     0,    52,     0,     0,     0,     0,    95,
      92,   180,    53,     0,    62,    49,     0,     0,     0,    88,
      84,    86,    93,     0,    23,    96,    50,    81,     0,    89,
      90,     0,     0,     0,    97,    98,    85,    87,    94,     0,
       0,   100,   101,     0,    99,     0,     0,     0,   104,     0,
       0,     0,   102,   103,   106,   105
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    18,    19,    20,    21,    22,    63,    64,   109,    65,
      82,   234,   235,   264,   265,   369,   212,   150,   213,   268,
     269,   304,   305,   342,   343,   370,   371,   359,   360,   374,
     375,   384,   385,   397,   398,   157,    67,   158,   159,    23,
      78,   136,    24,    25,   198,   199,    26,    27,    28,    29,
     208,   247,   280,   281,   151,   152,    30,    31,   215,   253,
     254,   284,   316,    32,    33,    34,    35,   204,   205,    36,
      79,    37,    38,   218,    39,    40
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -235
static const short yypact[] =
{
     646,    -7,  -235,    49,  -235,    29,    39,  -235,  -235,    51,
    -235,  -235,  -235,   229,   141,   619,    52,   126,    89,  -235,
      56,  -235,  -235,  -235,  -235,  -235,  -235,  -235,  -235,    92,
    -235,  -235,  -235,   140,  -235,  -235,  -235,  -235,  -235,  -235,
    -235,   149,    61,   161,   164,   180,   108,  -235,  -235,  -235,
    -235,   112,   119,  -235,   121,   127,   130,   131,   132,   264,
    -235,   264,   264,   -16,  -235,    48,   481,  -235,   107,   120,
      45,    20,  -235,  -235,  -235,  -235,  -235,  -235,  -235,   148,
    -235,  -235,   -35,  -235,    13,   217,  -235,   165,    30,   176,
     158,  -235,   133,   264,    -9,   243,   264,   264,   264,   144,
     146,   151,  -235,  -235,   328,   126,   229,  -235,   239,  -235,
     264,   264,   264,   264,   264,   264,   264,   264,   264,   264,
     264,   264,   264,   264,   264,   264,   264,   117,    20,   174,
     175,  -235,  -235,  -235,  -235,  -235,  -235,  -235,   241,   253,
     -31,  -235,   163,   204,  -235,  -235,  -235,    74,     6,  -235,
    -235,   167,  -235,   -10,   226,  -235,  -235,   481,    40,  -235,
     349,   259,   171,    54,   284,   305,   372,  -235,   264,   264,
    -235,   -52,  -235,  -235,  -235,  -235,   504,   525,   260,   548,
     569,   569,   209,   209,   209,   209,   -13,   -13,  -235,  -235,
    -235,  -235,  -235,  -235,  -235,  -235,   266,  -235,  -235,  -235,
    -235,    79,   181,   197,   -49,  -235,  -235,   169,   208,   286,
    -235,  -235,  -235,    65,    30,   195,  -235,  -235,   233,   -24,
     243,  -235,  -235,   207,  -235,  -235,   264,  -235,   264,  -235,
    -235,   437,   460,   124,   269,  -235,  -235,     6,    37,   241,
    -235,  -235,  -235,   210,  -235,   211,  -235,    68,   214,  -235,
       6,  -235,    16,   220,  -235,  -235,   302,   223,     6,  -235,
    -235,   393,   416,   224,   238,  -235,   201,   303,   247,  -235,
      82,     2,  -235,  -235,  -235,   235,   230,   169,  -235,    30,
     231,  -235,  -235,  -235,    15,    30,  -235,     6,  -235,  -235,
    -235,   318,   124,     4,    -1,     4,     4,     4,     4,     4,
       4,   291,   169,   294,   281,  -235,  -235,  -235,    86,   240,
    -235,  -235,    87,   214,  -235,  -235,  -235,   323,  -235,    94,
     257,  -235,   277,   280,  -235,     6,  -235,  -235,  -235,  -235,
    -235,  -235,  -235,  -235,  -235,  -235,  -235,  -235,    23,  -235,
     307,   319,   334,  -235,  -235,  -235,    30,  -235,  -235,  -235,
    -235,     6,    10,    99,  -235,     6,   361,   150,   391,   348,
    -235,  -235,  -235,   395,  -235,  -235,   100,   169,   308,    22,
     326,  -235,   329,   418,  -235,  -235,  -235,   326,   345,  -235,
    -235,   169,   436,   363,   352,  -235,  -235,  -235,  -235,     1,
     418,   369,  -235,   443,  -235,   457,   387,   105,  -235,   389,
       6,   443,  -235,  -235,  -235,  -235
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -235,  -235,  -235,  -235,   401,  -235,  -235,   380,  -235,  -235,
     384,  -235,   306,  -235,   198,  -162,   -88,  -233,  -234,  -235,
    -235,  -235,  -235,  -235,  -235,   142,   147,  -235,  -235,  -235,
    -235,  -235,   139,  -235,   106,   -12,  -235,   438,   312,  -235,
    -235,   406,  -235,  -235,   350,  -235,  -235,  -235,  -235,  -235,
    -235,  -235,  -235,   242,   275,  -212,  -235,  -235,  -235,  -235,
     287,  -235,  -235,  -235,  -235,  -235,  -235,  -235,   335,  -235,
    -235,  -235,  -235,  -235,  -235,  -235
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -194
static const short yytable[] =
{
     149,    66,   252,   270,   391,   273,   392,   144,   324,   144,
     145,   144,   257,    73,   110,   105,   145,   161,   314,  -193,
     216,   233,   138,   131,   233,   132,   133,   283,   134,   217,
      41,    75,   354,   111,   379,   144,   145,   308,   146,   197,
      77,   139,   144,   145,   239,   246,   380,   102,   129,   103,
     104,   107,    42,   319,   130,   258,   315,  -193,   139,   108,
     323,    43,   327,   329,   331,   333,   335,   337,   135,    13,
      44,   266,   140,   317,   124,   125,   126,   106,   162,   210,
     211,   160,   191,    45,   164,   165,   166,   192,   196,    83,
     147,   353,   196,   325,    66,   393,   363,   307,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   311,   147,   355,    80,   364,
     191,   366,   144,   147,   148,   192,   149,   241,   242,    81,
     266,   271,    85,   220,   361,   221,    46,    47,    48,    49,
     339,   155,    50,    86,    68,   243,   193,   220,    84,   225,
     272,    69,    87,   241,   242,    88,   231,   232,   250,   244,
     251,   277,   282,   278,    89,   194,   263,    90,    53,    54,
     288,   243,   241,   242,    70,   250,   100,   306,   101,   250,
     346,   344,   347,    91,   195,   244,   127,   250,    71,   349,
     243,   149,   250,   250,   365,   376,   245,   149,   401,   128,
     402,   368,    92,   196,   244,   322,    93,   326,   328,   330,
     332,   334,   336,    94,   261,    95,   262,   293,   137,    59,
     142,    96,   245,    61,    97,    98,    99,    62,   156,   153,
     143,   154,    46,    47,    48,    49,   110,   294,    50,   167,
     168,   245,   173,    51,   203,   169,    46,    47,    48,    49,
      52,   155,    50,   201,   202,   111,   206,   207,   149,   209,
     214,   219,   223,   362,    53,    54,   224,    46,    47,    48,
      49,   210,    55,    50,    56,   237,   238,   248,    53,    54,
     295,   296,   297,   298,   299,   300,   100,   110,   101,   249,
     255,   301,    57,   256,   122,   123,   124,   125,   126,    53,
      54,    58,   260,   267,   275,   276,   111,   100,   279,   101,
     286,   110,   404,   285,   292,    59,    60,   287,   291,    61,
     302,   303,   309,    62,   313,   310,   320,   338,   340,    59,
     111,   341,   110,    61,   283,   345,   357,    62,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
      59,   111,   350,   351,    61,   110,   352,   356,    62,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   111,   358,   110,   226,   367,   227,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   111,   372,   373,   228,   110,
     229,   211,   378,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   111,   381,
     110,   383,   382,   170,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   111,
     386,   388,   389,   110,   222,   390,   396,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   111,   395,   110,   399,   400,   230,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   111,   403,   141,   172,   110,   289,   171,
     321,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   111,   405,   110,   377,
     240,   290,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   111,   387,   394,
     226,   110,   259,   163,   200,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     111,   236,   110,   228,   312,   348,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   111,   318,     0,   274,   110,     0,     0,     0,     0,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   111,     0,   110,     0,     0,     0,
       0,     0,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   111,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     118,   119,   120,   121,   122,   123,   124,   125,   126,     1,
       0,     2,     0,    73,     3,     0,     4,     0,     5,     6,
       7,     8,     0,     0,     9,     0,     0,    74,     0,     0,
       0,    75,     0,    76,    10,     0,     0,     0,     0,     0,
      77,     0,     0,     0,     0,     0,     0,     0,    11,     0,
      12,     0,    13,    14,     0,    15,     0,    16,     0,     0,
       0,     0,     0,     0,    17
};

static const short yycheck[] =
{
      88,    13,   214,   237,     3,   238,     5,     5,     9,     5,
       6,     5,    36,    44,    27,    31,     6,    26,     3,     3,
      30,    73,    57,     3,    73,     5,     6,    11,     8,    39,
      37,    62,     9,    46,    12,     5,     6,   271,     8,   127,
      71,    93,     5,     6,    93,   207,    24,    59,     3,    61,
      62,     3,     3,   287,     9,    79,    41,    41,    93,    11,
     293,    32,   295,   296,   297,   298,   299,   300,    48,    56,
      31,   233,    59,   285,    87,    88,    89,    93,    87,     5,
       6,    93,     3,    32,    96,    97,    98,     8,    86,     0,
      86,   325,    86,    94,   106,    94,    86,    95,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   277,    86,    94,    66,   352,
       3,   355,     5,    86,    94,     8,   214,     3,     4,     3,
     292,    94,    40,    93,   346,    95,     3,     4,     5,     6,
     302,     8,     9,     3,     3,    21,    29,    93,    92,    95,
     238,    10,     3,     3,     4,    94,   168,   169,    93,    35,
      95,    93,   250,    95,     3,    48,    42,     3,    35,    36,
     258,    21,     3,     4,    33,    93,    43,    95,    45,    93,
      93,    95,    95,     3,    67,    35,    79,    93,    47,    95,
      21,   279,    93,    93,    95,    95,    72,   285,    93,    79,
      95,    51,    94,    86,    35,   293,    94,   295,   296,   297,
     298,   299,   300,    94,   226,    94,   228,    16,    70,    86,
       3,    94,    72,    90,    94,    94,    94,    94,    95,    53,
      65,    73,     3,     4,     5,     6,    27,    36,     9,    95,
      94,    72,     3,    14,     3,    94,     3,     4,     5,     6,
      21,     8,     9,    79,    79,    46,     3,    94,   346,    55,
      93,    35,     3,   351,    35,    36,    95,     3,     4,     5,
       6,     5,    43,     9,    45,    94,    79,    69,    35,    36,
      79,    80,    81,    82,    83,    84,    43,    27,    45,     3,
      95,    90,    63,    60,    85,    86,    87,    88,    89,    35,
      36,    72,    95,    34,    94,    94,    46,    43,    94,    45,
       8,    27,   400,    93,    76,    86,    87,    94,    94,    90,
      17,    74,    87,    94,    93,    95,     8,    36,    34,    86,
      46,    50,    27,    90,    11,    95,    17,    94,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      86,    46,    95,    76,    90,    27,    76,    50,    94,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    46,    41,    27,    93,    17,    95,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    46,     5,    49,    93,    27,
      95,     6,    94,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    46,    93,
      27,     3,    93,    95,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    46,
      95,     5,    79,    27,    95,    93,     3,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    46,    94,    27,     8,    79,    95,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    46,    95,    84,   106,    27,    95,   105,
     292,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    46,   401,    27,   367,
     204,    95,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    46,   381,   390,
      93,    27,   220,    95,   128,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      46,   201,    27,    93,   279,   313,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    46,   285,    -1,   239,    27,    -1,    -1,    -1,    -1,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    46,    -1,    27,    -1,    -1,    -1,
      -1,    -1,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    19,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    13,
      -1,    15,    -1,    44,    18,    -1,    20,    -1,    22,    23,
      24,    25,    -1,    -1,    28,    -1,    -1,    58,    -1,    -1,
      -1,    62,    -1,    64,    38,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    -1,
      54,    -1,    56,    57,    -1,    59,    -1,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    68
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    13,    15,    18,    20,    22,    23,    24,    25,    28,
      38,    52,    54,    56,    57,    59,    61,    68,    97,    98,
      99,   100,   101,   135,   138,   139,   142,   143,   144,   145,
     152,   153,   159,   160,   161,   162,   165,   167,   168,   170,
     171,    37,     3,    32,    31,    32,     3,     4,     5,     6,
       9,    14,    21,    35,    36,    43,    45,    63,    72,    86,
      87,    90,    94,   102,   103,   105,   131,   132,     3,    10,
      33,    47,    19,    44,    58,    62,    64,    71,   136,   166,
      66,     3,   106,     0,    92,    40,     3,     3,    94,     3,
       3,     3,    94,    94,    94,    94,    94,    94,    94,    94,
      43,    45,   131,   131,   131,    31,    93,     3,    11,   104,
      27,    46,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    79,    79,     3,
       9,     3,     5,     6,     8,    48,   137,    70,    57,    93,
      59,   100,     3,    65,     5,     6,     8,    86,    94,   112,
     113,   150,   151,    53,    73,     8,    95,   131,   133,   134,
     131,    26,    87,   133,   131,   131,   131,    95,    94,    94,
      95,   106,   103,     3,   131,   131,   131,   131,   131,   131,
     131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
     131,     3,     8,    29,    48,    67,    86,   112,   140,   141,
     137,    79,    79,     3,   163,   164,     3,    94,   146,    55,
       5,     6,   112,   114,    93,   154,    30,    39,   169,    35,
      93,    95,    95,     3,    95,    95,    93,    95,    93,    95,
      95,   131,   131,    73,   107,   108,   140,    94,    79,    93,
     108,     3,     4,    21,    35,    72,   111,   147,    69,     3,
      93,    95,   151,   155,   156,    95,    60,    36,    79,   134,
      95,   131,   131,    42,   109,   110,   111,    34,   115,   116,
     114,    94,   112,   113,   164,    94,    94,    93,    95,    94,
     148,   149,   112,    11,   157,    93,     8,    94,   112,    95,
      95,    94,    76,    16,    36,    79,    80,    81,    82,    83,
      84,    90,    17,    74,   117,   118,    95,    95,   114,    87,
      95,   111,   150,    93,     3,    41,   158,   151,   156,   114,
       8,   110,   112,   113,     9,    94,   112,   113,   112,   113,
     112,   113,   112,   113,   112,   113,   112,   113,    36,   111,
      34,    50,   119,   120,    95,    95,    93,    95,   149,    95,
      95,    76,    76,   114,     9,    94,    50,    17,    41,   123,
     124,   151,   112,    86,   113,    95,   114,    17,    51,   111,
     121,   122,     5,    49,   125,   126,    95,   121,    94,    12,
      24,    93,    93,     3,   127,   128,    95,   122,     5,    79,
      93,     3,     5,    94,   128,    94,     3,   129,   130,     8,
      79,    93,    95,    95,   112,   130
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

  case 19:

    { pParser->PushQuery(); ;}
    break;

  case 20:

    { pParser->PushQuery(); ;}
    break;

  case 23:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 26:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 29:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 30:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 31:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 32:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 33:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 34:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 36:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 37:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 38:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 40:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 46:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 47:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 48:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 49:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 50:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 51:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 52:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 53:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 54:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 55:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 56:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 57:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 64:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 66:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 67:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 68:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 69:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 70:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 71:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 72:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 73:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 74:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 75:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 78:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 81:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 84:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 85:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 87:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 89:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 90:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 93:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 94:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 102:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 103:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 104:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 105:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 106:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 108:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 109:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 113:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 114:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 132:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 134:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 135:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 137:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 138:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 144:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 145:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 146:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 152:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 153:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 154:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 155:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 156:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 157:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 158:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 161:

    { yyval.m_iValue = 1; ;}
    break;

  case 162:

    { yyval.m_iValue = 0; ;}
    break;

  case 163:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 164:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 165:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 166:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 169:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 170:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 174:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 175:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 178:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 179:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 180:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 181:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 182:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 183:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 184:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_iValue = yyvsp[-1].m_pValues->GetLength(); yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 185:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 186:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 187:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 190:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 192:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 196:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 197:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 200:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 201:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 204:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 205:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 206:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 207:

    {
			pParser->UpdateAttr ( yyvsp[-3].m_sValue, NULL, SPH_ATTR_UINT32SET );
		;}
    break;

  case 208:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 211:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 212:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 213:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 214:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 215:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 216:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
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

