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
     TOK_CONST_STRINGS = 266,
     TOK_AS = 267,
     TOK_ASC = 268,
     TOK_ATTACH = 269,
     TOK_AVG = 270,
     TOK_BEGIN = 271,
     TOK_BETWEEN = 272,
     TOK_BY = 273,
     TOK_CALL = 274,
     TOK_COLLATION = 275,
     TOK_COMMIT = 276,
     TOK_COMMITTED = 277,
     TOK_COUNT = 278,
     TOK_CREATE = 279,
     TOK_DELETE = 280,
     TOK_DESC = 281,
     TOK_DESCRIBE = 282,
     TOK_DISTINCT = 283,
     TOK_DIV = 284,
     TOK_DROP = 285,
     TOK_FALSE = 286,
     TOK_FLOAT = 287,
     TOK_FLUSH = 288,
     TOK_FROM = 289,
     TOK_FUNCTION = 290,
     TOK_GLOBAL = 291,
     TOK_GROUP = 292,
     TOK_ID = 293,
     TOK_IN = 294,
     TOK_INDEX = 295,
     TOK_INSERT = 296,
     TOK_INT = 297,
     TOK_INTO = 298,
     TOK_ISOLATION = 299,
     TOK_LEVEL = 300,
     TOK_LIMIT = 301,
     TOK_MATCH = 302,
     TOK_MAX = 303,
     TOK_META = 304,
     TOK_MIN = 305,
     TOK_MOD = 306,
     TOK_NAMES = 307,
     TOK_NULL = 308,
     TOK_OPTION = 309,
     TOK_ORDER = 310,
     TOK_RAND = 311,
     TOK_READ = 312,
     TOK_REPEATABLE = 313,
     TOK_REPLACE = 314,
     TOK_RETURNS = 315,
     TOK_ROLLBACK = 316,
     TOK_RTINDEX = 317,
     TOK_SELECT = 318,
     TOK_SERIALIZABLE = 319,
     TOK_SET = 320,
     TOK_SESSION = 321,
     TOK_SHOW = 322,
     TOK_SONAME = 323,
     TOK_START = 324,
     TOK_STATUS = 325,
     TOK_SUM = 326,
     TOK_TABLES = 327,
     TOK_TO = 328,
     TOK_TRANSACTION = 329,
     TOK_TRUE = 330,
     TOK_UNCOMMITTED = 331,
     TOK_UPDATE = 332,
     TOK_VALUES = 333,
     TOK_VARIABLES = 334,
     TOK_WARNINGS = 335,
     TOK_WEIGHT = 336,
     TOK_WHERE = 337,
     TOK_WITHIN = 338,
     TOK_OR = 339,
     TOK_AND = 340,
     TOK_NE = 341,
     TOK_GTE = 342,
     TOK_LTE = 343,
     TOK_NOT = 344,
     TOK_NEG = 345
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
#define TOK_CONST_STRINGS 266
#define TOK_AS 267
#define TOK_ASC 268
#define TOK_ATTACH 269
#define TOK_AVG 270
#define TOK_BEGIN 271
#define TOK_BETWEEN 272
#define TOK_BY 273
#define TOK_CALL 274
#define TOK_COLLATION 275
#define TOK_COMMIT 276
#define TOK_COMMITTED 277
#define TOK_COUNT 278
#define TOK_CREATE 279
#define TOK_DELETE 280
#define TOK_DESC 281
#define TOK_DESCRIBE 282
#define TOK_DISTINCT 283
#define TOK_DIV 284
#define TOK_DROP 285
#define TOK_FALSE 286
#define TOK_FLOAT 287
#define TOK_FLUSH 288
#define TOK_FROM 289
#define TOK_FUNCTION 290
#define TOK_GLOBAL 291
#define TOK_GROUP 292
#define TOK_ID 293
#define TOK_IN 294
#define TOK_INDEX 295
#define TOK_INSERT 296
#define TOK_INT 297
#define TOK_INTO 298
#define TOK_ISOLATION 299
#define TOK_LEVEL 300
#define TOK_LIMIT 301
#define TOK_MATCH 302
#define TOK_MAX 303
#define TOK_META 304
#define TOK_MIN 305
#define TOK_MOD 306
#define TOK_NAMES 307
#define TOK_NULL 308
#define TOK_OPTION 309
#define TOK_ORDER 310
#define TOK_RAND 311
#define TOK_READ 312
#define TOK_REPEATABLE 313
#define TOK_REPLACE 314
#define TOK_RETURNS 315
#define TOK_ROLLBACK 316
#define TOK_RTINDEX 317
#define TOK_SELECT 318
#define TOK_SERIALIZABLE 319
#define TOK_SET 320
#define TOK_SESSION 321
#define TOK_SHOW 322
#define TOK_SONAME 323
#define TOK_START 324
#define TOK_STATUS 325
#define TOK_SUM 326
#define TOK_TABLES 327
#define TOK_TO 328
#define TOK_TRANSACTION 329
#define TOK_TRUE 330
#define TOK_UNCOMMITTED 331
#define TOK_UPDATE 332
#define TOK_VALUES 333
#define TOK_VARIABLES 334
#define TOK_WARNINGS 335
#define TOK_WEIGHT 336
#define TOK_WHERE 337
#define TOK_WITHIN 338
#define TOK_OR 339
#define TOK_AND 340
#define TOK_NE 341
#define TOK_GTE 342
#define TOK_LTE 343
#define TOK_NOT 344
#define TOK_NEG 345




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
#define YYLAST   752

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  105
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  82
/* YYNRULES -- Number of rules. */
#define YYNRULES  232
/* YYNRULES -- Number of states. */
#define YYNSTATES  433

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   345

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    98,    87,     2,
     103,   104,    96,    94,   102,    95,     2,    97,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   101,
      90,    88,    91,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    86,     2,     2,     2,     2,     2,
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
      85,    89,    92,    93,    99,   100
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
     313,   315,   319,   323,   327,   333,   340,   344,   346,   350,
     354,   356,   358,   360,   362,   364,   366,   369,   372,   376,
     380,   384,   388,   392,   396,   400,   404,   408,   412,   416,
     420,   424,   428,   432,   436,   440,   444,   446,   451,   456,
     460,   467,   474,   476,   480,   482,   484,   487,   489,   491,
     493,   495,   497,   499,   501,   503,   508,   513,   518,   522,
     527,   535,   541,   543,   545,   547,   549,   551,   553,   555,
     557,   559,   562,   569,   571,   573,   574,   578,   580,   584,
     586,   590,   594,   596,   600,   602,   604,   606,   610,   618,
     628,   635,   637,   641,   643,   647,   649,   653,   654,   657,
     659,   663,   667,   668,   670,   672,   674,   677,   679,   681,
     684,   690,   692,   696,   700,   704,   710,   715,   719,   722,
     729,   730,   732,   734,   737,   740,   743,   745,   753,   755,
     757,   761,   768
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     106,     0,    -1,   107,    -1,   108,    -1,   108,   101,    -1,
     153,    -1,   161,    -1,   147,    -1,   148,    -1,   151,    -1,
     162,    -1,   171,    -1,   173,    -1,   174,    -1,   177,    -1,
     178,    -1,   182,    -1,   184,    -1,   185,    -1,   186,    -1,
     179,    -1,   109,    -1,   108,   101,   109,    -1,   110,    -1,
     144,    -1,    63,   111,    34,   115,   116,   124,   126,   128,
     132,   134,    -1,   112,    -1,   111,   102,   112,    -1,    96,
      -1,   114,   113,    -1,    -1,     3,    -1,    12,     3,    -1,
     140,    -1,    15,   103,   140,   104,    -1,    48,   103,   140,
     104,    -1,    50,   103,   140,   104,    -1,    71,   103,   140,
     104,    -1,    23,   103,    96,   104,    -1,    81,   103,   104,
      -1,    23,   103,    28,     3,   104,    -1,     3,    -1,   115,
     102,     3,    -1,    -1,   117,    -1,    82,   118,    -1,   119,
      -1,   118,    85,   119,    -1,    47,   103,     8,   104,    -1,
     120,    88,   121,    -1,   120,    89,   121,    -1,   120,    39,
     103,   123,   104,    -1,   120,    99,    39,   103,   123,   104,
      -1,   120,    39,     9,    -1,   120,    99,    39,     9,    -1,
     120,    17,   121,    85,   121,    -1,   120,    91,   121,    -1,
     120,    90,   121,    -1,   120,    92,   121,    -1,   120,    93,
     121,    -1,   120,    88,   122,    -1,   120,    89,   122,    -1,
     120,    91,   122,    -1,   120,    90,   122,    -1,   120,    17,
     122,    85,   122,    -1,   120,    92,   122,    -1,   120,    93,
     122,    -1,     3,    -1,     4,    -1,    23,   103,    96,   104,
      -1,    81,   103,   104,    -1,    38,    -1,     5,    -1,    95,
       5,    -1,     6,    -1,    95,     6,    -1,   121,    -1,   123,
     102,   121,    -1,    -1,   125,    -1,    37,    18,   120,    -1,
      -1,   127,    -1,    83,    37,    55,    18,   130,    -1,    -1,
     129,    -1,    55,    18,   130,    -1,    55,    18,    56,   103,
     104,    -1,   131,    -1,   130,   102,   131,    -1,   120,    -1,
     120,    13,    -1,   120,    26,    -1,    -1,   133,    -1,    46,
       5,    -1,    46,     5,   102,     5,    -1,    -1,   135,    -1,
      54,   136,    -1,   137,    -1,   136,   102,   137,    -1,     3,
      88,     3,    -1,     3,    88,     5,    -1,     3,    88,   103,
     138,   104,    -1,     3,    88,     3,   103,     8,   104,    -1,
       3,    88,     8,    -1,   139,    -1,   138,   102,   139,    -1,
       3,    88,   121,    -1,     3,    -1,     4,    -1,    38,    -1,
       5,    -1,     6,    -1,     9,    -1,    95,   140,    -1,    99,
     140,    -1,   140,    94,   140,    -1,   140,    95,   140,    -1,
     140,    96,   140,    -1,   140,    97,   140,    -1,   140,    90,
     140,    -1,   140,    91,   140,    -1,   140,    87,   140,    -1,
     140,    86,   140,    -1,   140,    98,   140,    -1,   140,    29,
     140,    -1,   140,    51,   140,    -1,   140,    93,   140,    -1,
     140,    92,   140,    -1,   140,    88,   140,    -1,   140,    89,
     140,    -1,   140,    85,   140,    -1,   140,    84,   140,    -1,
     103,   140,   104,    -1,   141,    -1,     3,   103,   142,   104,
      -1,    39,   103,   142,   104,    -1,     3,   103,   104,    -1,
      50,   103,   140,   102,   140,   104,    -1,    48,   103,   140,
     102,   140,   104,    -1,   143,    -1,   142,   102,   143,    -1,
     140,    -1,     8,    -1,    67,   145,    -1,    80,    -1,    70,
      -1,    49,    -1,     3,    -1,    53,    -1,     8,    -1,     5,
      -1,     6,    -1,    65,     3,    88,   150,    -1,    65,     3,
      88,   149,    -1,    65,     3,    88,    53,    -1,    65,    52,
     146,    -1,    65,    10,    88,   146,    -1,    65,    36,     9,
      88,   103,   123,   104,    -1,    65,    36,     3,    88,   149,
      -1,     3,    -1,     8,    -1,    75,    -1,    31,    -1,   121,
      -1,    21,    -1,    61,    -1,   152,    -1,    16,    -1,    69,
      74,    -1,   154,    43,     3,   155,    78,   157,    -1,    41,
      -1,    59,    -1,    -1,   103,   156,   104,    -1,   120,    -1,
     156,   102,   120,    -1,   158,    -1,   157,   102,   158,    -1,
     103,   159,   104,    -1,   160,    -1,   159,   102,   160,    -1,
     121,    -1,   122,    -1,     8,    -1,   103,   123,   104,    -1,
      25,    34,     3,    82,    38,    88,   121,    -1,    25,    34,
       3,    82,    38,    39,   103,   123,   104,    -1,    19,     3,
     103,   163,   166,   104,    -1,   164,    -1,   163,   102,   164,
      -1,   160,    -1,   103,   165,   104,    -1,     8,    -1,   165,
     102,     8,    -1,    -1,   102,   167,    -1,   168,    -1,   167,
     102,   168,    -1,   160,   169,   170,    -1,    -1,    12,    -1,
       3,    -1,    46,    -1,   172,     3,    -1,    27,    -1,    26,
      -1,    67,    72,    -1,    77,   115,    65,   175,   117,    -1,
     176,    -1,   175,   102,   176,    -1,     3,    88,   121,    -1,
       3,    88,   122,    -1,     3,    88,   103,   123,   104,    -1,
       3,    88,   103,   104,    -1,    67,   180,    79,    -1,    67,
      20,    -1,    65,   180,    74,    44,    45,   181,    -1,    -1,
      36,    -1,    66,    -1,    57,    76,    -1,    57,    22,    -1,
      58,    57,    -1,    64,    -1,    24,    35,     3,    60,   183,
      68,     8,    -1,    42,    -1,    32,    -1,    30,    35,     3,
      -1,    14,    40,     3,    73,    62,     3,    -1,    33,    62,
       3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   119,   119,   120,   121,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   146,   147,   151,   152,   156,   171,   172,   176,   177,
     180,   182,   183,   187,   188,   189,   190,   191,   192,   193,
     194,   198,   199,   202,   204,   208,   212,   213,   217,   222,
     229,   237,   245,   254,   259,   264,   269,   274,   279,   284,
     289,   290,   291,   292,   297,   302,   307,   315,   316,   321,
     327,   333,   342,   343,   347,   348,   352,   358,   364,   366,
     370,   377,   379,   383,   389,   391,   395,   399,   406,   407,
     411,   412,   413,   416,   418,   422,   427,   434,   436,   440,
     444,   445,   449,   454,   459,   465,   470,   478,   483,   490,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   509,
     510,   511,   512,   513,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   530,   531,   532,
     533,   534,   538,   539,   543,   544,   550,   554,   555,   556,
     562,   563,   564,   565,   566,   570,   575,   580,   585,   586,
     590,   595,   603,   604,   608,   609,   610,   624,   625,   626,
     630,   631,   637,   645,   646,   649,   651,   655,   656,   660,
     661,   665,   669,   670,   674,   675,   676,   677,   683,   689,
     701,   709,   713,   720,   721,   728,   738,   744,   746,   750,
     755,   759,   766,   768,   772,   773,   779,   787,   788,   794,
     800,   808,   809,   813,   817,   821,   825,   834,   841,   848,
     854,   855,   856,   860,   861,   862,   863,   869,   880,   881,
     885,   896,   908
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_ATIDENT", 
  "TOK_CONST_INT", "TOK_CONST_FLOAT", "TOK_CONST_MVA", 
  "TOK_QUOTED_STRING", "TOK_USERVAR", "TOK_SYSVAR", "TOK_CONST_STRINGS", 
  "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", "TOK_BEGIN", 
  "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_COLLATION", "TOK_COMMIT", 
  "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", "TOK_DESC", 
  "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", 
  "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", 
  "TOK_GROUP", "TOK_ID", "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", 
  "TOK_INTO", "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", 
  "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_SUM", "TOK_TABLES", 
  "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_UNCOMMITTED", 
  "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", 
  "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", 
  "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", 
  "')'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "expr_ident", "const_int", 
  "const_float", "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "show_clause", "show_variable", "set_value", 
  "set_clause", "set_global_clause", "set_string_value", "boolean_value", 
  "transact_op", "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "update", "update_items_list", 
  "update_item", "show_variables", "show_collation", "set_transaction", 
  "opt_scope", "isolation_level", "create_function", "udf_type", 
  "drop_function", "attach_index", "flush_rtindex", 0
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
     335,   336,   337,   338,   339,   340,   124,    38,    61,   341,
      60,    62,   342,   343,    43,    45,    42,    47,    37,   344,
     345,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   105,   106,   106,   106,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   108,   108,   109,   109,   110,   111,   111,   112,   112,
     113,   113,   113,   114,   114,   114,   114,   114,   114,   114,
     114,   115,   115,   116,   116,   117,   118,   118,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   120,   120,   120,
     120,   120,   121,   121,   122,   122,   123,   123,   124,   124,
     125,   126,   126,   127,   128,   128,   129,   129,   130,   130,
     131,   131,   131,   132,   132,   133,   133,   134,   134,   135,
     136,   136,   137,   137,   137,   137,   137,   138,   138,   139,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   141,   141,   141,
     141,   141,   142,   142,   143,   143,   144,   145,   145,   145,
     146,   146,   146,   146,   146,   147,   147,   147,   147,   147,
     148,   148,   149,   149,   150,   150,   150,   151,   151,   151,
     152,   152,   153,   154,   154,   155,   155,   156,   156,   157,
     157,   158,   159,   159,   160,   160,   160,   160,   161,   161,
     162,   163,   163,   164,   164,   165,   165,   166,   166,   167,
     167,   168,   169,   169,   170,   170,   171,   172,   172,   173,
     174,   175,   175,   176,   176,   176,   176,   177,   178,   179,
     180,   180,   180,   181,   181,   181,   181,   182,   183,   183,
     184,   185,   186
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
       1,     3,     3,     3,     5,     6,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     4,     4,     3,
       6,     6,     1,     3,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     4,     4,     3,     4,
       7,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     6,     1,     1,     0,     3,     1,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     3,     7,     9,
       6,     1,     3,     1,     3,     1,     3,     0,     2,     1,
       3,     3,     0,     1,     1,     1,     2,     1,     1,     2,
       5,     1,     3,     3,     3,     5,     4,     3,     2,     6,
       0,     1,     1,     2,     2,     2,     1,     7,     1,     1,
       3,     6,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   170,     0,   167,     0,     0,   208,   207,     0,
       0,   173,   174,   168,     0,   220,   220,     0,     0,     0,
       2,     3,    21,    23,    24,     7,     8,     9,   169,     5,
       0,     6,    10,    11,     0,    12,    13,    14,    15,    20,
      16,    17,    18,    19,     0,     0,     0,     0,     0,     0,
     110,   111,   113,   114,   115,     0,     0,   112,     0,     0,
       0,     0,     0,     0,    28,     0,     0,     0,    26,    30,
      33,   136,     0,     0,   221,     0,   222,     0,   218,   221,
     149,   148,   209,   147,   146,     0,   171,    41,     0,     1,
       4,     0,   206,     0,     0,     0,     0,   230,   232,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   116,
     117,     0,     0,     0,    31,     0,    29,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   150,   153,
     154,   152,   151,   158,     0,   217,     0,     0,     0,    22,
     175,     0,    72,    74,   186,     0,     0,   184,   185,   193,
     197,   191,     0,     0,   145,   139,   144,     0,   142,     0,
       0,     0,     0,     0,     0,     0,    39,     0,     0,   135,
      43,    27,    32,   127,   128,   134,   133,   125,   124,   131,
     132,   122,   123,   130,   129,   118,   119,   120,   121,   126,
     162,   163,   165,   157,   164,     0,   166,   156,   155,   159,
       0,     0,     0,     0,     0,   211,    42,     0,     0,     0,
      73,    75,   195,    76,     0,     0,     0,     0,   229,   228,
       0,     0,     0,   137,    34,     0,    38,   138,     0,    35,
       0,    36,    37,     0,     0,     0,    78,    44,   161,     0,
       0,     0,     0,   210,    67,    68,     0,    71,     0,   177,
       0,     0,   231,     0,   187,     0,   194,   193,   192,   198,
     199,   190,     0,     0,     0,   143,    40,     0,     0,     0,
      45,    46,     0,     0,    81,    79,     0,     0,     0,   226,
     219,     0,   213,   214,   212,     0,     0,     0,   176,     0,
     172,   179,    77,   196,   203,     0,     0,   227,     0,   188,
     141,   140,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    84,    82,   160,   224,   223,
     225,   216,     0,     0,    70,   178,     0,     0,   182,     0,
     204,   205,   201,   202,   200,     0,     0,    47,     0,     0,
      53,     0,    49,    60,    50,    61,    57,    63,    56,    62,
      58,    65,    59,    66,     0,    80,     0,     0,    93,    85,
     215,    69,     0,   181,   180,   189,    48,     0,     0,     0,
      54,     0,     0,     0,     0,    97,    94,   183,    55,     0,
      64,    51,     0,     0,     0,    90,    86,    88,    95,     0,
      25,    98,    52,    83,     0,    91,    92,     0,     0,     0,
      99,   100,    87,    89,    96,     0,     0,   102,   103,   106,
       0,   101,     0,     0,     0,   107,     0,     0,     0,   104,
     105,   109,   108
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    19,    20,    21,    22,    23,    67,    68,   116,    69,
      88,   246,   247,   280,   281,   395,   223,   158,   224,   284,
     285,   325,   326,   368,   369,   396,   397,   385,   386,   400,
     401,   410,   411,   424,   425,   166,    71,   167,   168,    24,
      84,   143,    25,    26,   207,   208,    27,    28,    29,    30,
     218,   260,   300,   301,   337,   159,    31,    32,   160,   161,
     225,   227,   269,   270,   305,   342,    33,    34,    35,    36,
     214,   215,    37,    38,    39,    77,   290,    40,   230,    41,
      42,    43
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -249
static const short yypact[] =
{
     675,   -19,  -249,    49,  -249,    44,    62,  -249,  -249,   102,
     101,  -249,  -249,  -249,   194,    10,   232,    74,   163,   170,
    -249,    70,  -249,  -249,  -249,  -249,  -249,  -249,  -249,  -249,
     135,  -249,  -249,  -249,   179,  -249,  -249,  -249,  -249,  -249,
    -249,  -249,  -249,  -249,   198,    99,   215,   218,   231,   240,
     134,  -249,  -249,  -249,  -249,   143,   144,  -249,   153,   154,
     158,   159,   174,   210,  -249,   210,   210,   -20,  -249,    80,
     468,  -249,   165,   185,    16,    24,  -249,   192,  -249,  -249,
    -249,  -249,  -249,  -249,  -249,   205,  -249,  -249,   -43,  -249,
      87,   284,  -249,   219,     3,   235,   206,  -249,  -249,   136,
     210,   -12,   156,   210,   210,   210,   187,   191,   193,  -249,
    -249,   300,   163,   194,  -249,   296,  -249,   210,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   210,
     210,   210,   210,   210,   233,    24,   222,   223,  -249,  -249,
    -249,  -249,  -249,  -249,   259,  -249,   303,   304,     8,  -249,
     211,   253,  -249,  -249,  -249,   150,     7,  -249,  -249,  -249,
     221,  -249,   -14,   286,  -249,  -249,   468,   -69,  -249,   321,
     322,   245,   -15,   250,   271,   348,  -249,   210,   210,  -249,
     -17,  -249,  -249,  -249,  -249,   494,   517,   542,   565,   590,
     590,   548,   548,   548,   548,   234,   234,  -249,  -249,  -249,
    -249,  -249,  -249,  -249,  -249,   365,  -249,  -249,  -249,  -249,
      83,   268,   281,   265,    47,  -249,  -249,   169,   298,   375,
    -249,  -249,  -249,  -249,    77,    81,     3,   270,  -249,  -249,
     311,    11,   156,  -249,  -249,   276,  -249,  -249,   210,  -249,
     210,  -249,  -249,   420,   445,    20,   344,  -249,  -249,    40,
      94,    43,   303,  -249,  -249,  -249,   279,  -249,   280,  -249,
      85,   299,  -249,    40,  -249,   393,  -249,    51,  -249,   301,
    -249,  -249,   412,   318,    40,  -249,  -249,   371,   397,   320,
     339,  -249,   228,   409,   345,  -249,    86,     4,   372,  -249,
    -249,    29,  -249,  -249,  -249,   334,   327,   169,  -249,    33,
     349,  -249,  -249,  -249,  -249,    14,    33,  -249,    40,  -249,
    -249,  -249,   439,    20,    31,    -3,    31,    31,    31,    31,
      31,    31,   411,   169,   416,   399,  -249,  -249,  -249,  -249,
    -249,  -249,    89,   366,  -249,  -249,    40,   106,  -249,   299,
    -249,  -249,  -249,   460,  -249,   167,   369,  -249,   391,   392,
    -249,    40,  -249,  -249,  -249,  -249,  -249,  -249,  -249,  -249,
    -249,  -249,  -249,  -249,    22,  -249,   423,   461,   434,  -249,
    -249,  -249,    33,  -249,  -249,  -249,  -249,    40,    36,   168,
    -249,    40,   480,   173,   495,   448,  -249,  -249,  -249,   493,
    -249,  -249,   172,   169,   400,    38,   418,  -249,   419,   521,
    -249,  -249,  -249,   418,   421,  -249,  -249,   169,   522,   438,
     426,  -249,  -249,  -249,  -249,     2,   521,   441,  -249,  -249,
     545,  -249,   541,   462,   178,  -249,   447,    40,   545,  -249,
    -249,  -249,  -249
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -249,  -249,  -249,  -249,   477,  -249,  -249,   456,  -249,  -249,
     458,  -249,   358,  -249,   260,  -170,   -94,  -248,  -247,  -249,
    -249,  -249,  -249,  -249,  -249,   181,   171,  -249,  -249,  -249,
    -249,  -249,   160,  -249,   147,   -10,  -249,   496,   363,  -249,
    -249,   465,  -249,  -249,   386,  -249,  -249,  -249,  -249,  -249,
    -249,  -249,  -249,   258,  -249,  -225,  -249,  -249,  -249,   376,
    -249,  -249,  -249,   295,  -249,  -249,  -249,  -249,  -249,  -249,
    -249,   368,  -249,  -249,  -249,   601,  -249,  -249,  -249,  -249,
    -249,  -249
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -203
static const short yytable[] =
{
     157,   267,   286,   293,    70,   417,   350,   418,   152,   153,
     419,   154,   152,    72,   112,   222,   170,   340,   228,   136,
      73,    44,   146,   254,   255,   137,   328,   138,   229,   139,
     140,   380,   141,   232,   152,   233,   152,   153,   152,   153,
     206,   154,   153,   256,   332,   152,    74,   259,   152,   153,
     273,   405,    45,   109,  -202,   110,   111,    80,   257,   147,
     341,   345,    75,   304,   406,   245,   349,   279,   353,   355,
     357,   359,   361,   363,   338,   282,    76,   142,    81,    46,
     329,   343,   113,   114,   171,   147,   200,   232,    83,   237,
     169,   201,   115,   173,   174,   175,    47,  -202,   155,   274,
     351,   258,   205,    70,   379,   420,   156,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   205,   381,   155,   335,   155,   245,
     390,   389,   157,   331,   392,   205,   336,    48,   155,    50,
      51,    52,    53,   282,   164,    54,   291,   387,    86,   252,
      14,   287,   288,   365,   148,   220,   221,   292,   289,    50,
      51,    52,    53,    49,   164,    54,    87,   243,   244,   302,
      89,    90,   254,   255,    57,    58,   254,   255,    91,   263,
     309,   264,    92,   265,   107,   266,   108,   297,   263,   298,
     327,   263,   256,   370,    57,    58,   256,    50,    51,    52,
      53,    93,    94,    54,   107,   157,   108,   257,   372,    55,
     373,   257,   157,    50,    51,    52,    53,    56,    95,    54,
     348,    96,   352,   354,   356,   358,   360,   362,   277,   394,
     278,    63,    57,    58,    97,    65,   200,    99,   152,    66,
     165,   201,    59,    98,    60,   314,   100,   101,    57,    58,
     258,    63,    78,   134,   258,    65,   102,   103,   107,    66,
     108,   104,   105,   117,   202,    61,   144,   315,    79,   263,
     263,   375,   391,   135,   263,    62,   402,   106,   157,   117,
     428,    80,   429,   388,   145,   118,   203,   150,   163,    63,
      64,   176,   151,    65,   177,   162,   178,    66,    76,   182,
     117,   118,    81,   212,    82,    63,   213,   216,   204,    65,
     210,   211,    83,    66,   217,   219,   316,   317,   318,   319,
     320,   321,   118,   226,   231,   235,   250,   322,   205,   117,
     131,   132,   133,   431,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   236,
     117,   118,   238,   251,   239,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     220,   249,   118,   240,   271,   241,   261,   117,   262,   272,
     276,   283,   295,   296,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   118,
     117,   303,   299,   306,   179,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     307,   308,   118,   312,   313,   234,   117,   323,   324,   330,
     333,   334,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   346,   118,   117,
     364,   339,   242,   366,   367,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     371,   118,   304,   376,   117,   310,   377,   378,   382,   383,
     384,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   118,   117,   393,   221,
     398,   311,   399,   404,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   118,
     407,   408,   238,   117,   409,   412,   415,   414,   416,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   422,   118,   117,   240,   423,   426,
     427,   430,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   149,   118,   181,
     180,   117,   253,   347,   403,   432,   421,   117,   413,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   118,   117,   275,   248,   374,   172,   118,
     209,   344,   268,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   118,    85,     0,   117,
     294,     0,     0,     0,     0,     0,     0,     0,     0,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   118,   129,   130,   131,   132,   133,     0,     0,     0,
       0,     0,     0,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     125,   126,   127,   128,   129,   130,   131,   132,   133,     1,
       0,     2,     0,     0,     3,     0,     4,     0,     0,     5,
       6,     7,     8,     0,     0,     9,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,     0,    13,     0,    14,     0,
      15,     0,    16,     0,    17,     0,     0,     0,     0,     0,
       0,     0,    18
};

static const short yycheck[] =
{
      94,   226,   249,   251,    14,     3,     9,     5,     5,     6,
       8,     8,     5,     3,    34,     8,    28,     3,    32,     3,
      10,    40,    65,     3,     4,     9,    22,     3,    42,     5,
       6,     9,     8,   102,     5,   104,     5,     6,     5,     6,
     134,     8,     6,    23,   291,     5,    36,   217,     5,     6,
      39,    13,     3,    63,     3,    65,    66,    49,    38,   102,
      46,   308,    52,    12,    26,    82,   314,    47,   316,   317,
     318,   319,   320,   321,   299,   245,    66,    53,    70,    35,
      76,   306,   102,     3,    96,   102,     3,   102,    80,   104,
     100,     8,    12,   103,   104,   105,    34,    46,    95,    88,
     103,    81,    95,   113,   351,   103,   103,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,    95,   103,    95,   297,    95,    82,
     378,    95,   226,   104,   381,    95,   103,    35,    95,     3,
       4,     5,     6,   313,     8,     9,   103,   372,    74,   102,
      63,    57,    58,   323,    67,     5,     6,   251,    64,     3,
       4,     5,     6,    62,     8,     9,     3,   177,   178,   263,
       0,   101,     3,     4,    38,    39,     3,     4,    43,   102,
     274,   104,     3,   102,    48,   104,    50,   102,   102,   104,
     104,   102,    23,   104,    38,    39,    23,     3,     4,     5,
       6,     3,   103,     9,    48,   299,    50,    38,   102,    15,
     104,    38,   306,     3,     4,     5,     6,    23,     3,     9,
     314,     3,   316,   317,   318,   319,   320,   321,   238,    56,
     240,    95,    38,    39,     3,    99,     3,   103,     5,   103,
     104,     8,    48,     3,    50,    17,   103,   103,    38,    39,
      81,    95,    20,    88,    81,    99,   103,   103,    48,   103,
      50,   103,   103,    29,    31,    71,    74,    39,    36,   102,
     102,   104,   104,    88,   102,    81,   104,   103,   372,    29,
     102,    49,   104,   377,    79,    51,    53,     3,    82,    95,
      96,   104,    73,    99,   103,    60,   103,   103,    66,     3,
      29,    51,    70,    44,    72,    95,     3,     3,    75,    99,
      88,    88,    80,   103,   103,    62,    88,    89,    90,    91,
      92,    93,    51,   102,    38,     3,    45,    99,    95,    29,
      96,    97,    98,   427,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,   104,
      29,    51,   102,    88,   104,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
       5,   103,    51,   102,   104,   104,    78,    29,     3,    68,
     104,    37,   103,   103,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    51,
      29,     8,   103,   102,   104,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
       8,   103,    51,   103,    85,   104,    29,    18,    83,    57,
      96,   104,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,     8,    51,    29,
      39,   102,   104,    37,    55,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
     104,    51,    12,   104,    29,   104,    85,    85,    55,    18,
      46,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    51,    29,    18,     6,
       5,   104,    54,   103,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    51,
     102,   102,   102,    29,     3,   104,    88,     5,   102,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,   103,    51,    29,   102,     3,     8,
      88,   104,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    90,    51,   113,
     112,    29,   214,   313,   393,   428,   416,    29,   407,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    51,    29,   232,   210,   339,   102,    51,
     135,   306,   226,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    51,    16,    -1,    29,
     252,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    51,    94,    95,    96,    97,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    14,
      -1,    16,    -1,    -1,    19,    -1,    21,    -1,    -1,    24,
      25,    26,    27,    -1,    -1,    30,    -1,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    59,    -1,    61,    -1,    63,    -1,
      65,    -1,    67,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    77
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    16,    19,    21,    24,    25,    26,    27,    30,
      33,    41,    59,    61,    63,    65,    67,    69,    77,   106,
     107,   108,   109,   110,   144,   147,   148,   151,   152,   153,
     154,   161,   162,   171,   172,   173,   174,   177,   178,   179,
     182,   184,   185,   186,    40,     3,    35,    34,    35,    62,
       3,     4,     5,     6,     9,    15,    23,    38,    39,    48,
      50,    71,    81,    95,    96,    99,   103,   111,   112,   114,
     140,   141,     3,    10,    36,    52,    66,   180,    20,    36,
      49,    70,    72,    80,   145,   180,    74,     3,   115,     0,
     101,    43,     3,     3,   103,     3,     3,     3,     3,   103,
     103,   103,   103,   103,   103,   103,   103,    48,    50,   140,
     140,   140,    34,   102,     3,    12,   113,    29,    51,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    88,    88,     3,     9,     3,     5,
       6,     8,    53,   146,    74,    79,    65,   102,    67,   109,
       3,    73,     5,     6,     8,    95,   103,   121,   122,   160,
     163,   164,    60,    82,     8,   104,   140,   142,   143,   140,
      28,    96,   142,   140,   140,   140,   104,   103,   103,   104,
     115,   112,     3,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
       3,     8,    31,    53,    75,    95,   121,   149,   150,   146,
      88,    88,    44,     3,   175,   176,     3,   103,   155,    62,
       5,     6,     8,   121,   123,   165,   102,   166,    32,    42,
     183,    38,   102,   104,   104,     3,   104,   104,   102,   104,
     102,   104,   104,   140,   140,    82,   116,   117,   149,   103,
      45,    88,   102,   117,     3,     4,    23,    38,    81,   120,
     156,    78,     3,   102,   104,   102,   104,   160,   164,   167,
     168,   104,    68,    39,    88,   143,   104,   140,   140,    47,
     118,   119,   120,    37,   124,   125,   123,    57,    58,    64,
     181,   103,   121,   122,   176,   103,   103,   102,   104,   103,
     157,   158,   121,     8,    12,   169,   102,     8,   103,   121,
     104,   104,   103,    85,    17,    39,    88,    89,    90,    91,
      92,    93,    99,    18,    83,   126,   127,   104,    22,    76,
      57,   104,   123,    96,   104,   120,   103,   159,   160,   102,
       3,    46,   170,   160,   168,   123,     8,   119,   121,   122,
       9,   103,   121,   122,   121,   122,   121,   122,   121,   122,
     121,   122,   121,   122,    39,   120,    37,    55,   128,   129,
     104,   104,   102,   104,   158,   104,   104,    85,    85,   123,
       9,   103,    55,    18,    46,   132,   133,   160,   121,    95,
     122,   104,   123,    18,    56,   120,   130,   131,     5,    54,
     134,   135,   104,   130,   103,    13,    26,   102,   102,     3,
     136,   137,   104,   131,     5,    88,   102,     3,     5,     8,
     103,   137,   103,     3,   138,   139,     8,    88,   102,   104,
     104,   121,   139
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 107:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 108:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 109:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 111:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 112:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 116:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 117:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 135:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 137:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 138:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 139:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 140:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 141:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 147:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 148:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 149:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 155:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 156:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 157:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 158:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 159:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 160:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 161:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 164:

    { yyval.m_iValue = 1; ;}
    break;

  case 165:

    { yyval.m_iValue = 0; ;}
    break;

  case 166:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 168:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 169:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 172:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 177:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 178:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 181:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 182:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 183:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 184:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 185:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 186:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 187:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_iValue = yyvsp[-1].m_pValues->GetLength(); yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 188:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 189:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 190:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 191:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 192:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 194:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 195:

    {
			// FIXME? for now, one such array per CALL statement, tops
			if ( pParser->m_pStmt->m_dCallStrings.GetLength() )
			{
				yyerror ( pParser, "unexpected constant string list" );
				YYERROR;
			}
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 199:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 201:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 205:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 206:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 209:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 210:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 213:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 214:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 215:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 216:

    {
			pParser->UpdateAttr ( yyvsp[-3].m_sValue, NULL, SPH_ATTR_UINT32SET );
		;}
    break;

  case 217:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 218:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 219:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 227:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 228:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 229:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 230:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 231:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 232:

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

