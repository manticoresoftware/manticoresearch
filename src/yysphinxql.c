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
     TOK_COMMITTED = 276,
     TOK_COUNT = 277,
     TOK_CREATE = 278,
     TOK_DELETE = 279,
     TOK_DESC = 280,
     TOK_DESCRIBE = 281,
     TOK_DISTINCT = 282,
     TOK_DIV = 283,
     TOK_DROP = 284,
     TOK_FALSE = 285,
     TOK_FLOAT = 286,
     TOK_FLUSH = 287,
     TOK_FROM = 288,
     TOK_FUNCTION = 289,
     TOK_GLOBAL = 290,
     TOK_GROUP = 291,
     TOK_ID = 292,
     TOK_IN = 293,
     TOK_INDEX = 294,
     TOK_INSERT = 295,
     TOK_INT = 296,
     TOK_INTO = 297,
     TOK_ISOLATION = 298,
     TOK_LEVEL = 299,
     TOK_LIMIT = 300,
     TOK_MATCH = 301,
     TOK_MAX = 302,
     TOK_META = 303,
     TOK_MIN = 304,
     TOK_MOD = 305,
     TOK_NAMES = 306,
     TOK_NULL = 307,
     TOK_OPTION = 308,
     TOK_ORDER = 309,
     TOK_RAND = 310,
     TOK_READ = 311,
     TOK_REPEATABLE = 312,
     TOK_REPLACE = 313,
     TOK_RETURNS = 314,
     TOK_ROLLBACK = 315,
     TOK_RTINDEX = 316,
     TOK_SELECT = 317,
     TOK_SERIALIZABLE = 318,
     TOK_SET = 319,
     TOK_SESSION = 320,
     TOK_SHOW = 321,
     TOK_SONAME = 322,
     TOK_START = 323,
     TOK_STATUS = 324,
     TOK_SUM = 325,
     TOK_TABLES = 326,
     TOK_TO = 327,
     TOK_TRANSACTION = 328,
     TOK_TRUE = 329,
     TOK_UNCOMMITTED = 330,
     TOK_UPDATE = 331,
     TOK_VALUES = 332,
     TOK_VARIABLES = 333,
     TOK_WARNINGS = 334,
     TOK_WEIGHT = 335,
     TOK_WHERE = 336,
     TOK_WITHIN = 337,
     TOK_OR = 338,
     TOK_AND = 339,
     TOK_NE = 340,
     TOK_GTE = 341,
     TOK_LTE = 342,
     TOK_NOT = 343,
     TOK_NEG = 344
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
#define TOK_COMMITTED 276
#define TOK_COUNT 277
#define TOK_CREATE 278
#define TOK_DELETE 279
#define TOK_DESC 280
#define TOK_DESCRIBE 281
#define TOK_DISTINCT 282
#define TOK_DIV 283
#define TOK_DROP 284
#define TOK_FALSE 285
#define TOK_FLOAT 286
#define TOK_FLUSH 287
#define TOK_FROM 288
#define TOK_FUNCTION 289
#define TOK_GLOBAL 290
#define TOK_GROUP 291
#define TOK_ID 292
#define TOK_IN 293
#define TOK_INDEX 294
#define TOK_INSERT 295
#define TOK_INT 296
#define TOK_INTO 297
#define TOK_ISOLATION 298
#define TOK_LEVEL 299
#define TOK_LIMIT 300
#define TOK_MATCH 301
#define TOK_MAX 302
#define TOK_META 303
#define TOK_MIN 304
#define TOK_MOD 305
#define TOK_NAMES 306
#define TOK_NULL 307
#define TOK_OPTION 308
#define TOK_ORDER 309
#define TOK_RAND 310
#define TOK_READ 311
#define TOK_REPEATABLE 312
#define TOK_REPLACE 313
#define TOK_RETURNS 314
#define TOK_ROLLBACK 315
#define TOK_RTINDEX 316
#define TOK_SELECT 317
#define TOK_SERIALIZABLE 318
#define TOK_SET 319
#define TOK_SESSION 320
#define TOK_SHOW 321
#define TOK_SONAME 322
#define TOK_START 323
#define TOK_STATUS 324
#define TOK_SUM 325
#define TOK_TABLES 326
#define TOK_TO 327
#define TOK_TRANSACTION 328
#define TOK_TRUE 329
#define TOK_UNCOMMITTED 330
#define TOK_UPDATE 331
#define TOK_VALUES 332
#define TOK_VARIABLES 333
#define TOK_WARNINGS 334
#define TOK_WEIGHT 335
#define TOK_WHERE 336
#define TOK_WITHIN 337
#define TOK_OR 338
#define TOK_AND 339
#define TOK_NE 340
#define TOK_GTE 341
#define TOK_LTE 342
#define TOK_NOT 343
#define TOK_NEG 344




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
#define YYFINAL  89
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   759

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  104
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  79
/* YYNRULES -- Number of rules. */
#define YYNRULES  225
/* YYNRULES -- Number of states. */
#define YYNSTATES  423

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   344

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    97,    86,     2,
     102,   103,    95,    93,   101,    94,     2,    96,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   100,
      89,    87,    90,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    85,     2,     2,     2,     2,     2,
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
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      88,    91,    92,    98,    99
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    48,    50,    52,    63,    65,    69,    71,
      74,    75,    77,    80,    82,    87,    92,    97,   102,   107,
     111,   117,   119,   123,   124,   126,   129,   131,   135,   140,
     144,   148,   154,   161,   165,   170,   176,   180,   184,   188,
     192,   196,   200,   204,   208,   214,   218,   222,   224,   226,
     231,   235,   237,   239,   242,   244,   247,   249,   253,   254,
     256,   260,   261,   263,   269,   270,   272,   276,   282,   284,
     288,   290,   293,   296,   297,   299,   302,   307,   308,   310,
     313,   315,   319,   323,   327,   333,   340,   342,   346,   350,
     352,   354,   356,   358,   360,   362,   365,   368,   372,   376,
     380,   384,   388,   392,   396,   400,   404,   408,   412,   416,
     420,   424,   428,   432,   436,   440,   442,   447,   452,   456,
     463,   470,   472,   476,   478,   480,   483,   485,   487,   489,
     491,   493,   495,   497,   499,   504,   509,   514,   518,   523,
     531,   537,   539,   541,   543,   545,   547,   549,   551,   553,
     555,   558,   565,   567,   569,   570,   574,   576,   580,   582,
     586,   590,   592,   596,   598,   600,   602,   606,   614,   624,
     631,   632,   635,   637,   641,   645,   646,   648,   650,   652,
     655,   657,   659,   662,   668,   670,   674,   678,   682,   688,
     693,   697,   700,   707,   708,   710,   712,   715,   718,   721,
     723,   731,   733,   735,   739,   746
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     105,     0,    -1,   106,    -1,   107,    -1,   107,   100,    -1,
     152,    -1,   160,    -1,   146,    -1,   147,    -1,   150,    -1,
     161,    -1,   167,    -1,   169,    -1,   170,    -1,   173,    -1,
     174,    -1,   178,    -1,   180,    -1,   181,    -1,   182,    -1,
     175,    -1,   108,    -1,   107,   100,   108,    -1,   109,    -1,
     143,    -1,    62,   110,    33,   114,   115,   123,   125,   127,
     131,   133,    -1,   111,    -1,   110,   101,   111,    -1,    95,
      -1,   113,   112,    -1,    -1,     3,    -1,    11,     3,    -1,
     139,    -1,    14,   102,   139,   103,    -1,    47,   102,   139,
     103,    -1,    49,   102,   139,   103,    -1,    70,   102,   139,
     103,    -1,    22,   102,    95,   103,    -1,    80,   102,   103,
      -1,    22,   102,    27,     3,   103,    -1,     3,    -1,   114,
     101,     3,    -1,    -1,   116,    -1,    81,   117,    -1,   118,
      -1,   117,    84,   118,    -1,    46,   102,     8,   103,    -1,
     119,    87,   120,    -1,   119,    88,   120,    -1,   119,    38,
     102,   122,   103,    -1,   119,    98,    38,   102,   122,   103,
      -1,   119,    38,     9,    -1,   119,    98,    38,     9,    -1,
     119,    16,   120,    84,   120,    -1,   119,    90,   120,    -1,
     119,    89,   120,    -1,   119,    91,   120,    -1,   119,    92,
     120,    -1,   119,    87,   121,    -1,   119,    88,   121,    -1,
     119,    90,   121,    -1,   119,    89,   121,    -1,   119,    16,
     121,    84,   121,    -1,   119,    91,   121,    -1,   119,    92,
     121,    -1,     3,    -1,     4,    -1,    22,   102,    95,   103,
      -1,    80,   102,   103,    -1,    37,    -1,     5,    -1,    94,
       5,    -1,     6,    -1,    94,     6,    -1,   120,    -1,   122,
     101,   120,    -1,    -1,   124,    -1,    36,    17,   119,    -1,
      -1,   126,    -1,    82,    36,    54,    17,   129,    -1,    -1,
     128,    -1,    54,    17,   129,    -1,    54,    17,    55,   102,
     103,    -1,   130,    -1,   129,   101,   130,    -1,   119,    -1,
     119,    12,    -1,   119,    25,    -1,    -1,   132,    -1,    45,
       5,    -1,    45,     5,   101,     5,    -1,    -1,   134,    -1,
      53,   135,    -1,   136,    -1,   135,   101,   136,    -1,     3,
      87,     3,    -1,     3,    87,     5,    -1,     3,    87,   102,
     137,   103,    -1,     3,    87,     3,   102,     8,   103,    -1,
     138,    -1,   137,   101,   138,    -1,     3,    87,   120,    -1,
       3,    -1,     4,    -1,    37,    -1,     5,    -1,     6,    -1,
       9,    -1,    94,   139,    -1,    98,   139,    -1,   139,    93,
     139,    -1,   139,    94,   139,    -1,   139,    95,   139,    -1,
     139,    96,   139,    -1,   139,    89,   139,    -1,   139,    90,
     139,    -1,   139,    86,   139,    -1,   139,    85,   139,    -1,
     139,    97,   139,    -1,   139,    28,   139,    -1,   139,    50,
     139,    -1,   139,    92,   139,    -1,   139,    91,   139,    -1,
     139,    87,   139,    -1,   139,    88,   139,    -1,   139,    84,
     139,    -1,   139,    83,   139,    -1,   102,   139,   103,    -1,
     140,    -1,     3,   102,   141,   103,    -1,    38,   102,   141,
     103,    -1,     3,   102,   103,    -1,    49,   102,   139,   101,
     139,   103,    -1,    47,   102,   139,   101,   139,   103,    -1,
     142,    -1,   141,   101,   142,    -1,   139,    -1,     8,    -1,
      66,   144,    -1,    79,    -1,    69,    -1,    48,    -1,     3,
      -1,    52,    -1,     8,    -1,     5,    -1,     6,    -1,    64,
       3,    87,   149,    -1,    64,     3,    87,   148,    -1,    64,
       3,    87,    52,    -1,    64,    51,   145,    -1,    64,    10,
      87,   145,    -1,    64,    35,     9,    87,   102,   122,   103,
      -1,    64,    35,     3,    87,   148,    -1,     3,    -1,     8,
      -1,    74,    -1,    30,    -1,   120,    -1,    20,    -1,    60,
      -1,   151,    -1,    15,    -1,    68,    73,    -1,   153,    42,
       3,   154,    77,   156,    -1,    40,    -1,    58,    -1,    -1,
     102,   155,   103,    -1,   119,    -1,   155,   101,   119,    -1,
     157,    -1,   156,   101,   157,    -1,   102,   158,   103,    -1,
     159,    -1,   158,   101,   159,    -1,   120,    -1,   121,    -1,
       8,    -1,   102,   122,   103,    -1,    24,    33,     3,    81,
      37,    87,   120,    -1,    24,    33,     3,    81,    37,    38,
     102,   122,   103,    -1,    18,     3,   102,   158,   162,   103,
      -1,    -1,   101,   163,    -1,   164,    -1,   163,   101,   164,
      -1,   159,   165,   166,    -1,    -1,    11,    -1,     3,    -1,
      45,    -1,   168,     3,    -1,    26,    -1,    25,    -1,    66,
      71,    -1,    76,   114,    64,   171,   116,    -1,   172,    -1,
     171,   101,   172,    -1,     3,    87,   120,    -1,     3,    87,
     121,    -1,     3,    87,   102,   122,   103,    -1,     3,    87,
     102,   103,    -1,    66,   176,    78,    -1,    66,    19,    -1,
      64,   176,    73,    43,    44,   177,    -1,    -1,    35,    -1,
      65,    -1,    56,    75,    -1,    56,    21,    -1,    57,    56,
      -1,    63,    -1,    23,    34,     3,    59,   179,    67,     8,
      -1,    41,    -1,    31,    -1,    29,    34,     3,    -1,    13,
      39,     3,    72,    61,     3,    -1,    32,    61,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   118,   118,   119,   120,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   145,   146,   150,   151,   155,   170,   171,   175,   176,
     179,   181,   182,   186,   187,   188,   189,   190,   191,   192,
     193,   197,   198,   201,   203,   207,   211,   212,   216,   221,
     228,   236,   244,   253,   258,   263,   268,   273,   278,   283,
     288,   289,   290,   291,   296,   301,   306,   314,   315,   320,
     326,   332,   341,   342,   346,   347,   351,   357,   363,   365,
     369,   376,   378,   382,   388,   390,   394,   398,   405,   406,
     410,   411,   412,   415,   417,   421,   426,   433,   435,   439,
     443,   444,   448,   453,   458,   464,   472,   477,   484,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   524,   525,   526,   527,
     528,   532,   533,   537,   538,   544,   548,   549,   550,   556,
     557,   558,   559,   560,   564,   569,   574,   579,   580,   584,
     589,   597,   598,   602,   603,   604,   618,   619,   620,   624,
     625,   631,   639,   640,   643,   645,   649,   650,   654,   655,
     659,   663,   664,   668,   669,   670,   671,   677,   683,   695,
     702,   704,   708,   713,   717,   724,   726,   730,   731,   737,
     745,   746,   752,   758,   766,   767,   771,   775,   779,   783,
     792,   799,   806,   812,   813,   814,   818,   819,   820,   821,
     827,   838,   839,   843,   854,   866
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
  "TOK_CALL", "TOK_COLLATION", "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", 
  "TOK_CREATE", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", 
  "TOK_DIV", "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", 
  "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTO", 
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", 
  "TOK_ORDER", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", 
  "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", 
  "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
  "TOK_START", "TOK_STATUS", "TOK_SUM", "TOK_TABLES", "TOK_TO", 
  "TOK_TRANSACTION", "TOK_TRUE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
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
  "update_items_list", "update_item", "show_variables", "show_collation", 
  "set_transaction", "opt_scope", "isolation_level", "create_function", 
  "udf_type", "drop_function", "attach_index", "flush_rtindex", 0
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
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   124,    38,    61,   340,    60,
      62,   341,   342,    43,    45,    42,    47,    37,   343,   344,
      59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   104,   105,   105,   105,   106,   106,   106,   106,   106,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   107,   107,   108,   108,   109,   110,   110,   111,   111,
     112,   112,   112,   113,   113,   113,   113,   113,   113,   113,
     113,   114,   114,   115,   115,   116,   117,   117,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   119,   119,   119,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     124,   125,   125,   126,   127,   127,   128,   128,   129,   129,
     130,   130,   130,   131,   131,   132,   132,   133,   133,   134,
     135,   135,   136,   136,   136,   136,   137,   137,   138,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   140,   140,   140,   140,
     140,   141,   141,   142,   142,   143,   144,   144,   144,   145,
     145,   145,   145,   145,   146,   146,   146,   146,   146,   147,
     147,   148,   148,   149,   149,   149,   150,   150,   150,   151,
     151,   152,   153,   153,   154,   154,   155,   155,   156,   156,
     157,   158,   158,   159,   159,   159,   159,   160,   160,   161,
     162,   162,   163,   163,   164,   165,   165,   166,   166,   167,
     168,   168,   169,   170,   171,   171,   172,   172,   172,   172,
     173,   174,   175,   176,   176,   176,   177,   177,   177,   177,
     178,   179,   179,   180,   181,   182
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,    10,     1,     3,     1,     2,
       0,     1,     2,     1,     4,     4,     4,     4,     4,     3,
       5,     1,     3,     0,     1,     2,     1,     3,     4,     3,
       3,     5,     6,     3,     4,     5,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     3,     3,     1,     1,     4,
       3,     1,     1,     2,     1,     2,     1,     3,     0,     1,
       3,     0,     1,     5,     0,     1,     3,     5,     1,     3,
       1,     2,     2,     0,     1,     2,     4,     0,     1,     2,
       1,     3,     3,     3,     5,     6,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     4,     4,     3,     6,
       6,     1,     3,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     4,     4,     3,     4,     7,
       5,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     6,     1,     1,     0,     3,     1,     3,     1,     3,
       3,     1,     3,     1,     1,     1,     3,     7,     9,     6,
       0,     2,     1,     3,     3,     0,     1,     1,     1,     2,
       1,     1,     2,     5,     1,     3,     3,     3,     5,     4,
       3,     2,     6,     0,     1,     1,     2,     2,     2,     1,
       7,     1,     1,     3,     6,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   169,     0,   166,     0,     0,   201,   200,     0,
       0,   172,   173,   167,     0,   213,   213,     0,     0,     0,
       2,     3,    21,    23,    24,     7,     8,     9,   168,     5,
       0,     6,    10,    11,     0,    12,    13,    14,    15,    20,
      16,    17,    18,    19,     0,     0,     0,     0,     0,     0,
     109,   110,   112,   113,   114,     0,     0,   111,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,    26,    30,
      33,   135,     0,     0,   214,     0,   215,     0,   211,   214,
     148,   147,   202,   146,   145,     0,   170,    41,     0,     1,
       4,     0,   199,     0,     0,     0,     0,   223,   225,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   115,
     116,     0,     0,     0,    31,     0,    29,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   149,   152,
     153,   151,   150,   157,     0,   210,     0,     0,     0,    22,
     174,     0,    72,    74,   185,     0,     0,   183,   184,   190,
     181,     0,     0,   144,   138,   143,     0,   141,     0,     0,
       0,     0,     0,     0,     0,    39,     0,     0,   134,    43,
      27,    32,   126,   127,   133,   132,   124,   123,   130,   131,
     121,   122,   129,   128,   117,   118,   119,   120,   125,   161,
     162,   164,   156,   163,     0,   165,   155,   154,   158,     0,
       0,     0,     0,     0,   204,    42,     0,     0,     0,    73,
      75,    76,     0,     0,     0,   222,   221,     0,     0,     0,
     136,    34,     0,    38,   137,     0,    35,     0,    36,    37,
       0,     0,     0,    78,    44,   160,     0,     0,     0,     0,
     203,    67,    68,     0,    71,     0,   176,     0,     0,   224,
       0,   186,   182,   191,   192,   189,     0,     0,     0,   142,
      40,     0,     0,     0,    45,    46,     0,     0,    81,    79,
       0,     0,     0,   219,   212,     0,   206,   207,   205,     0,
       0,     0,   175,     0,   171,   178,    77,   196,     0,     0,
     220,     0,   187,   140,   139,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,    82,
     159,   217,   216,   218,   209,     0,     0,    70,   177,     0,
       0,   197,   198,   194,   195,   193,     0,     0,    47,     0,
       0,    53,     0,    49,    60,    50,    61,    57,    63,    56,
      62,    58,    65,    59,    66,     0,    80,     0,     0,    93,
      85,   208,    69,     0,   180,   179,   188,    48,     0,     0,
       0,    54,     0,     0,     0,     0,    97,    94,   182,    55,
       0,    64,    51,     0,     0,     0,    90,    86,    88,    95,
       0,    25,    98,    52,    83,     0,    91,    92,     0,     0,
       0,    99,   100,    87,    89,    96,     0,     0,   102,   103,
       0,   101,     0,     0,     0,   106,     0,     0,     0,   104,
     105,   108,   107
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    19,    20,    21,    22,    23,    67,    68,   116,    69,
      88,   243,   244,   274,   275,   386,   221,   158,   222,   278,
     279,   318,   319,   359,   360,   387,   388,   376,   377,   391,
     392,   401,   402,   414,   415,   165,    71,   166,   167,    24,
      84,   143,    25,    26,   206,   207,    27,    28,    29,    30,
     217,   257,   294,   295,   159,   160,    31,    32,   224,   263,
     264,   298,   333,    33,    34,    35,    36,   213,   214,    37,
      38,    39,    77,   284,    40,   227,    41,    42,    43
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -246
static const short yypact[] =
{
     683,   -20,  -246,    58,  -246,    39,    41,  -246,  -246,    43,
      25,  -246,  -246,  -246,   153,   138,   247,    61,   137,   149,
    -246,    50,  -246,  -246,  -246,  -246,  -246,  -246,  -246,  -246,
     111,  -246,  -246,  -246,   162,  -246,  -246,  -246,  -246,  -246,
    -246,  -246,  -246,  -246,   169,    96,   183,   198,   201,   207,
     112,  -246,  -246,  -246,  -246,   119,   123,  -246,   128,   129,
     141,   142,   143,   223,  -246,   223,   223,   -26,  -246,    73,
     478,  -246,   159,   167,    23,    19,  -246,   176,  -246,  -246,
    -246,  -246,  -246,  -246,  -246,   178,  -246,  -246,   -44,  -246,
     -45,   254,  -246,   187,    31,   206,   186,  -246,  -246,    90,
     223,   -12,   203,   223,   223,   223,   168,   171,   173,  -246,
    -246,   308,   137,   153,  -246,   273,  -246,   223,   223,   223,
     223,   223,   223,   223,   223,   223,   223,   223,   223,   223,
     223,   223,   223,   223,   234,    19,   191,   192,  -246,  -246,
    -246,  -246,  -246,  -246,   240,  -246,   281,   282,    10,  -246,
     185,   228,  -246,  -246,  -246,   155,     3,  -246,  -246,   189,
    -246,    -8,   255,  -246,  -246,   478,   -54,  -246,   334,   288,
     190,    67,   260,   285,   357,  -246,   223,   223,  -246,   -53,
    -246,  -246,  -246,  -246,   501,   527,   550,   575,   598,   598,
     556,   556,   556,   556,   -15,   -15,  -246,  -246,  -246,  -246,
    -246,  -246,  -246,  -246,   289,  -246,  -246,  -246,  -246,   144,
     196,   256,   212,    54,  -246,  -246,   259,   225,   300,  -246,
    -246,  -246,    68,    31,   204,  -246,  -246,   237,   -28,   203,
    -246,  -246,   211,  -246,  -246,   223,  -246,   223,  -246,  -246,
     430,   453,   231,   270,  -246,  -246,     3,    88,    36,   281,
    -246,  -246,  -246,   213,  -246,   217,  -246,    75,   218,  -246,
       3,  -246,    40,   208,  -246,  -246,   314,   222,     3,  -246,
    -246,   382,   405,   235,   243,  -246,   242,   321,   277,  -246,
      76,    -5,   286,  -246,  -246,    29,  -246,  -246,  -246,   246,
     257,   259,  -246,    31,   263,  -246,  -246,  -246,    11,    31,
    -246,     3,  -246,  -246,  -246,   358,   231,    49,     0,    49,
      49,    49,    49,    49,    49,   327,   259,   331,   329,  -246,
    -246,  -246,  -246,  -246,  -246,    79,   284,  -246,  -246,    80,
     218,  -246,  -246,  -246,   378,  -246,    84,   287,  -246,   322,
     324,  -246,     3,  -246,  -246,  -246,  -246,  -246,  -246,  -246,
    -246,  -246,  -246,  -246,  -246,    20,  -246,   355,   395,   368,
    -246,  -246,  -246,    31,  -246,  -246,  -246,  -246,     3,    32,
      93,  -246,     3,   397,     8,   410,   363,  -246,  -246,  -246,
     428,  -246,  -246,    94,   259,   333,     6,   335,  -246,   337,
     436,  -246,  -246,  -246,   335,   353,  -246,  -246,   259,   452,
     372,   360,  -246,  -246,  -246,  -246,     1,   436,   361,  -246,
     459,  -246,   456,   396,   135,  -246,   379,     3,   459,  -246,
    -246,  -246,  -246
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -246,  -246,  -246,  -246,   394,  -246,  -246,   373,  -246,  -246,
     375,  -246,   291,  -246,   199,  -170,   -94,  -245,  -241,  -246,
    -246,  -246,  -246,  -246,  -246,   125,   109,  -246,  -246,  -246,
    -246,  -246,   103,  -246,   114,   -13,  -246,   409,   283,  -246,
    -246,   398,  -246,  -246,   325,  -246,  -246,  -246,  -246,  -246,
    -246,  -246,  -246,   200,   264,  -221,  -246,  -246,  -246,  -246,
     236,  -246,  -246,  -246,  -246,  -246,  -246,  -246,   303,  -246,
    -246,  -246,   537,  -246,  -246,  -246,  -246,  -246,  -246
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -196
static const short yytable[] =
{
     157,    70,   262,   287,   408,   280,   409,   112,   152,   341,
     267,   251,   252,   117,   331,   169,   321,    14,   396,    44,
     146,   148,   138,   225,   139,   140,   136,   141,   242,   371,
     253,   397,   137,   226,   152,   118,   152,   153,   153,   154,
     205,   152,   153,  -195,   325,   254,   256,   229,   147,   230,
     109,   297,   110,   111,   152,   153,   332,   147,    80,   268,
     336,    45,   340,   385,   344,   346,   348,   350,   352,   354,
     322,   142,   276,    46,    47,   113,   114,    48,   334,    81,
     131,   132,   133,   170,   115,  -195,    49,   168,   255,    83,
     172,   173,   174,    50,    51,    52,    53,   204,   163,    54,
      70,   370,   342,   410,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   328,   372,   204,   381,   155,   380,    57,    58,   157,
     155,   383,   324,   156,    86,   242,   276,   107,   285,   108,
      87,    72,   378,   155,   281,   282,   356,   199,    73,    89,
      90,   283,   200,    91,   286,   249,    50,    51,    52,    53,
     219,   220,    54,   240,   241,    92,   296,    55,   229,   260,
     234,   261,    93,    74,   302,    56,   291,   260,   292,   320,
     260,   363,   361,   364,    63,   260,    95,   366,    65,    75,
      57,    58,    66,   164,   260,   260,   382,   393,    94,   157,
      59,    96,    60,    76,    97,   157,    50,    51,    52,    53,
      98,   163,    54,   339,    99,   343,   345,   347,   349,   351,
     353,   100,   271,    61,   272,   101,    50,    51,    52,    53,
     102,   103,    54,    62,   251,   252,   418,   199,   419,   152,
      57,    58,   200,   104,   105,   106,   134,    63,    64,   144,
     107,    65,   108,   253,   135,    66,   145,   150,   307,   151,
      57,    58,   251,   252,   201,   161,    78,   162,   254,   157,
     107,   175,   108,   176,   379,   177,   181,   273,   209,   210,
     308,   253,    79,   211,   212,   215,   202,   216,   117,   218,
     223,   232,   228,   233,   219,    80,   254,    63,   246,   248,
     247,    65,   258,   259,   266,    66,   277,   265,   203,   299,
     118,   255,    76,   117,   270,   289,    81,    63,    82,   290,
     293,    65,   300,   421,   301,    66,    83,   306,   204,   309,
     310,   311,   312,   313,   314,   118,   117,   305,   316,   255,
     315,   326,   323,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   118,   317,
     327,   235,   117,   236,   330,   355,   337,   357,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   358,   118,   117,   237,   362,   238,   297,
     367,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   368,   118,   369,   373,
     117,   178,   374,   375,   384,   389,   390,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   118,   117,   220,   395,   398,   231,   399,   400,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   118,   403,   405,   117,   406,
     239,   407,   413,   412,   416,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     118,   117,   420,   417,   149,   303,   180,   179,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   118,   250,   338,   117,   404,   304,   394,
     411,   171,   269,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   118,   117,
     365,   235,   422,   208,   245,   335,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   118,   288,    85,   237,   117,     0,   329,     0,     0,
       0,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   118,   117,     0,
       0,     0,     0,     0,   117,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,     0,
     118,     0,     0,   117,     0,     0,   118,     0,     0,     0,
       0,     0,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   118,   117,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   118,   129,
     130,   131,   132,   133,     0,     0,     0,     0,     0,     0,
       0,     0,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     1,     0,     2,     0,
       0,     3,     0,     4,     0,     0,     5,     6,     7,     8,
       0,     0,     9,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,    11,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,     0,    13,     0,    14,     0,    15,     0,    16,
       0,    17,     0,     0,     0,     0,     0,     0,     0,    18
};

static const short yycheck[] =
{
      94,    14,   223,   248,     3,   246,     5,    33,     5,     9,
      38,     3,     4,    28,     3,    27,    21,    62,    12,    39,
      64,    66,     3,    31,     5,     6,     3,     8,    81,     9,
      22,    25,     9,    41,     5,    50,     5,     6,     6,     8,
     134,     5,     6,     3,   285,    37,   216,   101,   101,   103,
      63,    11,    65,    66,     5,     6,    45,   101,    48,    87,
     301,     3,   307,    55,   309,   310,   311,   312,   313,   314,
      75,    52,   242,    34,    33,   101,     3,    34,   299,    69,
      95,    96,    97,    95,    11,    45,    61,   100,    80,    79,
     103,   104,   105,     3,     4,     5,     6,    94,     8,     9,
     113,   342,   102,   102,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   291,   102,    94,   369,    94,    94,    37,    38,   223,
      94,   372,   103,   102,    73,    81,   306,    47,   102,    49,
       3,     3,   363,    94,    56,    57,   316,     3,    10,     0,
     100,    63,     8,    42,   248,   101,     3,     4,     5,     6,
       5,     6,     9,   176,   177,     3,   260,    14,   101,   101,
     103,   103,     3,    35,   268,    22,   101,   101,   103,   103,
     101,   101,   103,   103,    94,   101,     3,   103,    98,    51,
      37,    38,   102,   103,   101,   101,   103,   103,   102,   293,
      47,     3,    49,    65,     3,   299,     3,     4,     5,     6,
       3,     8,     9,   307,   102,   309,   310,   311,   312,   313,
     314,   102,   235,    70,   237,   102,     3,     4,     5,     6,
     102,   102,     9,    80,     3,     4,   101,     3,   103,     5,
      37,    38,     8,   102,   102,   102,    87,    94,    95,    73,
      47,    98,    49,    22,    87,   102,    78,     3,    16,    72,
      37,    38,     3,     4,    30,    59,    19,    81,    37,   363,
      47,   103,    49,   102,   368,   102,     3,    46,    87,    87,
      38,    22,    35,    43,     3,     3,    52,   102,    28,    61,
     101,     3,    37,   103,     5,    48,    37,    94,   102,    87,
      44,    98,    77,     3,    67,   102,    36,   103,    74,   101,
      50,    80,    65,    28,   103,   102,    69,    94,    71,   102,
     102,    98,     8,   417,   102,   102,    79,    84,    94,    87,
      88,    89,    90,    91,    92,    50,    28,   102,    17,    80,
      98,    95,    56,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    50,    82,
     103,   101,    28,   103,   101,    38,     8,    36,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    54,    50,    28,   101,   103,   103,    11,
     103,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    84,    50,    84,    54,
      28,   103,    17,    45,    17,     5,    53,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    50,    28,     6,   102,   101,   103,   101,     3,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    50,   103,     5,    28,    87,
     103,   101,     3,   102,     8,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      50,    28,   103,    87,    90,   103,   113,   112,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    50,   213,   306,    28,   398,   103,   384,
     407,   102,   229,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    50,    28,
     330,   101,   418,   135,   209,   299,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    50,   249,    16,   101,    28,    -1,   293,    -1,    -1,
      -1,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    -1,    50,    28,    -1,
      -1,    -1,    -1,    -1,    28,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    -1,
      50,    -1,    -1,    28,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    50,    28,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    50,    93,
      94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    13,    -1,    15,    -1,
      -1,    18,    -1,    20,    -1,    -1,    23,    24,    25,    26,
      -1,    -1,    29,    -1,    -1,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    62,    -1,    64,    -1,    66,
      -1,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    13,    15,    18,    20,    23,    24,    25,    26,    29,
      32,    40,    58,    60,    62,    64,    66,    68,    76,   105,
     106,   107,   108,   109,   143,   146,   147,   150,   151,   152,
     153,   160,   161,   167,   168,   169,   170,   173,   174,   175,
     178,   180,   181,   182,    39,     3,    34,    33,    34,    61,
       3,     4,     5,     6,     9,    14,    22,    37,    38,    47,
      49,    70,    80,    94,    95,    98,   102,   110,   111,   113,
     139,   140,     3,    10,    35,    51,    65,   176,    19,    35,
      48,    69,    71,    79,   144,   176,    73,     3,   114,     0,
     100,    42,     3,     3,   102,     3,     3,     3,     3,   102,
     102,   102,   102,   102,   102,   102,   102,    47,    49,   139,
     139,   139,    33,   101,     3,    11,   112,    28,    50,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    87,    87,     3,     9,     3,     5,
       6,     8,    52,   145,    73,    78,    64,   101,    66,   108,
       3,    72,     5,     6,     8,    94,   102,   120,   121,   158,
     159,    59,    81,     8,   103,   139,   141,   142,   139,    27,
      95,   141,   139,   139,   139,   103,   102,   102,   103,   114,
     111,     3,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,     3,
       8,    30,    52,    74,    94,   120,   148,   149,   145,    87,
      87,    43,     3,   171,   172,     3,   102,   154,    61,     5,
       6,   120,   122,   101,   162,    31,    41,   179,    37,   101,
     103,   103,     3,   103,   103,   101,   103,   101,   103,   103,
     139,   139,    81,   115,   116,   148,   102,    44,    87,   101,
     116,     3,     4,    22,    37,    80,   119,   155,    77,     3,
     101,   103,   159,   163,   164,   103,    67,    38,    87,   142,
     103,   139,   139,    46,   117,   118,   119,    36,   123,   124,
     122,    56,    57,    63,   177,   102,   120,   121,   172,   102,
     102,   101,   103,   102,   156,   157,   120,    11,   165,   101,
       8,   102,   120,   103,   103,   102,    84,    16,    38,    87,
      88,    89,    90,    91,    92,    98,    17,    82,   125,   126,
     103,    21,    75,    56,   103,   122,    95,   103,   119,   158,
     101,     3,    45,   166,   159,   164,   122,     8,   118,   120,
     121,     9,   102,   120,   121,   120,   121,   120,   121,   120,
     121,   120,   121,   120,   121,    38,   119,    36,    54,   127,
     128,   103,   103,   101,   103,   157,   103,   103,    84,    84,
     122,     9,   102,    54,    17,    45,   131,   132,   159,   120,
      94,   121,   103,   122,    17,    55,   119,   129,   130,     5,
      53,   133,   134,   103,   129,   102,    12,    25,   101,   101,
       3,   135,   136,   103,   130,     5,    87,   101,     3,     5,
     102,   136,   102,     3,   137,   138,     8,    87,   101,   103,
     103,   120,   138
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

  case 21:

    { pParser->PushQuery(); ;}
    break;

  case 22:

    { pParser->PushQuery(); ;}
    break;

  case 25:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 28:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 31:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 32:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 33:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 34:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 36:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 37:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 38:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 39:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 40:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 42:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 48:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 49:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 50:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 51:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 52:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 53:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 54:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 55:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 56:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 57:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 58:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 59:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 64:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 65:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 66:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 69:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 70:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 71:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 72:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 73:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 74:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 75:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 76:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 77:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 80:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 83:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 86:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 87:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 89:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 91:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 92:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 95:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 96:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 102:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 103:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 104:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 105:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 106:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 107:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 108:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 110:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 111:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 115:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 116:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 133:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 134:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 137:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 138:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 139:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 140:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 146:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 147:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 148:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 154:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 155:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 156:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 157:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 158:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 159:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 160:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 163:

    { yyval.m_iValue = 1; ;}
    break;

  case 164:

    { yyval.m_iValue = 0; ;}
    break;

  case 165:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 166:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 168:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 171:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 176:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 177:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 180:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 181:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 182:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 183:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 184:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 185:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 186:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_iValue = yyvsp[-1].m_pValues->GetLength(); yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 187:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 188:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 189:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 192:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 194:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 198:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 199:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 203:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 206:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 207:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 208:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 209:

    {
			pParser->UpdateAttr ( yyvsp[-3].m_sValue, NULL, SPH_ATTR_UINT32SET );
		;}
    break;

  case 210:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 211:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 212:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 220:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 221:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 222:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 223:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 224:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 225:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
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

