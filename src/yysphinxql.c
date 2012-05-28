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
     TOK_CHARACTER = 275,
     TOK_COLLATION = 276,
     TOK_COMMIT = 277,
     TOK_COMMITTED = 278,
     TOK_COUNT = 279,
     TOK_CREATE = 280,
     TOK_DELETE = 281,
     TOK_DESC = 282,
     TOK_DESCRIBE = 283,
     TOK_DISTINCT = 284,
     TOK_DIV = 285,
     TOK_DROP = 286,
     TOK_FALSE = 287,
     TOK_FLOAT = 288,
     TOK_FLUSH = 289,
     TOK_FROM = 290,
     TOK_FUNCTION = 291,
     TOK_GLOBAL = 292,
     TOK_GROUP = 293,
     TOK_GROUPBY = 294,
     TOK_GROUP_CONCAT = 295,
     TOK_ID = 296,
     TOK_IN = 297,
     TOK_INDEX = 298,
     TOK_INSERT = 299,
     TOK_INT = 300,
     TOK_INTO = 301,
     TOK_ISOLATION = 302,
     TOK_LEVEL = 303,
     TOK_LIMIT = 304,
     TOK_MATCH = 305,
     TOK_MAX = 306,
     TOK_META = 307,
     TOK_MIN = 308,
     TOK_MOD = 309,
     TOK_NAMES = 310,
     TOK_NULL = 311,
     TOK_OPTION = 312,
     TOK_ORDER = 313,
     TOK_OPTIMIZE = 314,
     TOK_RAND = 315,
     TOK_READ = 316,
     TOK_REPEATABLE = 317,
     TOK_REPLACE = 318,
     TOK_RETURNS = 319,
     TOK_ROLLBACK = 320,
     TOK_RTINDEX = 321,
     TOK_SELECT = 322,
     TOK_SERIALIZABLE = 323,
     TOK_SET = 324,
     TOK_SESSION = 325,
     TOK_SHOW = 326,
     TOK_SONAME = 327,
     TOK_START = 328,
     TOK_STATUS = 329,
     TOK_SUM = 330,
     TOK_TABLES = 331,
     TOK_TO = 332,
     TOK_TRANSACTION = 333,
     TOK_TRUE = 334,
     TOK_TRUNCATE = 335,
     TOK_UNCOMMITTED = 336,
     TOK_UPDATE = 337,
     TOK_VALUES = 338,
     TOK_VARIABLES = 339,
     TOK_WARNINGS = 340,
     TOK_WEIGHT = 341,
     TOK_WHERE = 342,
     TOK_WITHIN = 343,
     TOK_OR = 344,
     TOK_AND = 345,
     TOK_NE = 346,
     TOK_GTE = 347,
     TOK_LTE = 348,
     TOK_NOT = 349,
     TOK_NEG = 350
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
#define TOK_CHARACTER 275
#define TOK_COLLATION 276
#define TOK_COMMIT 277
#define TOK_COMMITTED 278
#define TOK_COUNT 279
#define TOK_CREATE 280
#define TOK_DELETE 281
#define TOK_DESC 282
#define TOK_DESCRIBE 283
#define TOK_DISTINCT 284
#define TOK_DIV 285
#define TOK_DROP 286
#define TOK_FALSE 287
#define TOK_FLOAT 288
#define TOK_FLUSH 289
#define TOK_FROM 290
#define TOK_FUNCTION 291
#define TOK_GLOBAL 292
#define TOK_GROUP 293
#define TOK_GROUPBY 294
#define TOK_GROUP_CONCAT 295
#define TOK_ID 296
#define TOK_IN 297
#define TOK_INDEX 298
#define TOK_INSERT 299
#define TOK_INT 300
#define TOK_INTO 301
#define TOK_ISOLATION 302
#define TOK_LEVEL 303
#define TOK_LIMIT 304
#define TOK_MATCH 305
#define TOK_MAX 306
#define TOK_META 307
#define TOK_MIN 308
#define TOK_MOD 309
#define TOK_NAMES 310
#define TOK_NULL 311
#define TOK_OPTION 312
#define TOK_ORDER 313
#define TOK_OPTIMIZE 314
#define TOK_RAND 315
#define TOK_READ 316
#define TOK_REPEATABLE 317
#define TOK_REPLACE 318
#define TOK_RETURNS 319
#define TOK_ROLLBACK 320
#define TOK_RTINDEX 321
#define TOK_SELECT 322
#define TOK_SERIALIZABLE 323
#define TOK_SET 324
#define TOK_SESSION 325
#define TOK_SHOW 326
#define TOK_SONAME 327
#define TOK_START 328
#define TOK_STATUS 329
#define TOK_SUM 330
#define TOK_TABLES 331
#define TOK_TO 332
#define TOK_TRANSACTION 333
#define TOK_TRUE 334
#define TOK_TRUNCATE 335
#define TOK_UNCOMMITTED 336
#define TOK_UPDATE 337
#define TOK_VALUES 338
#define TOK_VARIABLES 339
#define TOK_WARNINGS 340
#define TOK_WEIGHT 341
#define TOK_WHERE 342
#define TOK_WITHIN 343
#define TOK_OR 344
#define TOK_AND 345
#define TOK_NE 346
#define TOK_GTE 347
#define TOK_LTE 348
#define TOK_NOT 349
#define TOK_NEG 350




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
#define YYFINAL  102
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   808

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  111
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  91
/* YYNRULES -- Number of rules. */
#define YYNRULES  252
/* YYNRULES -- Number of states. */
#define YYNSTATES  472

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   350

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   103,    92,     2,
     108,   109,   101,    99,   107,   100,   110,   102,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   106,
      95,    93,    96,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    91,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    94,    97,    98,   104,
     105
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    56,    58,    60,
      71,    73,    77,    79,    82,    83,    85,    88,    90,    95,
     100,   105,   110,   115,   120,   124,   128,   134,   136,   140,
     141,   143,   146,   148,   152,   157,   161,   165,   171,   178,
     182,   187,   193,   197,   201,   205,   209,   213,   217,   221,
     225,   231,   235,   239,   241,   243,   248,   252,   256,   258,
     260,   263,   265,   268,   270,   274,   275,   277,   281,   282,
     284,   290,   291,   293,   297,   303,   305,   309,   311,   314,
     317,   318,   320,   323,   328,   329,   331,   334,   336,   340,
     344,   348,   354,   361,   365,   367,   371,   375,   377,   379,
     381,   383,   385,   387,   390,   393,   397,   401,   405,   409,
     413,   417,   421,   425,   429,   433,   437,   441,   445,   449,
     453,   457,   461,   465,   467,   472,   477,   481,   488,   495,
     497,   501,   503,   505,   508,   510,   512,   514,   516,   518,
     520,   522,   524,   529,   534,   539,   543,   548,   556,   562,
     564,   566,   568,   570,   572,   574,   576,   578,   580,   583,
     590,   592,   594,   595,   599,   601,   605,   607,   611,   615,
     617,   621,   623,   625,   627,   631,   634,   642,   652,   659,
     661,   665,   667,   671,   673,   677,   678,   681,   683,   687,
     691,   692,   694,   696,   698,   701,   703,   705,   708,   714,
     716,   720,   724,   728,   734,   739,   744,   745,   747,   750,
     752,   756,   760,   763,   767,   774,   775,   777,   779,   782,
     785,   788,   790,   798,   800,   802,   806,   813,   817,   821,
     823,   827,   831
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     112,     0,    -1,   113,    -1,   114,    -1,   114,   106,    -1,
     159,    -1,   167,    -1,   153,    -1,   154,    -1,   157,    -1,
     168,    -1,   177,    -1,   179,    -1,   180,    -1,   183,    -1,
     188,    -1,   189,    -1,   193,    -1,   195,    -1,   196,    -1,
     197,    -1,   190,    -1,   198,    -1,   200,    -1,   201,    -1,
     115,    -1,   114,   106,   115,    -1,   116,    -1,   150,    -1,
      67,   117,    35,   121,   122,   130,   132,   134,   138,   140,
      -1,   118,    -1,   117,   107,   118,    -1,   101,    -1,   120,
     119,    -1,    -1,     3,    -1,    12,     3,    -1,   146,    -1,
      15,   108,   146,   109,    -1,    51,   108,   146,   109,    -1,
      53,   108,   146,   109,    -1,    75,   108,   146,   109,    -1,
      40,   108,   146,   109,    -1,    24,   108,   101,   109,    -1,
      86,   108,   109,    -1,    39,   108,   109,    -1,    24,   108,
      29,     3,   109,    -1,     3,    -1,   121,   107,     3,    -1,
      -1,   123,    -1,    87,   124,    -1,   125,    -1,   124,    90,
     125,    -1,    50,   108,     8,   109,    -1,   126,    93,   127,
      -1,   126,    94,   127,    -1,   126,    42,   108,   129,   109,
      -1,   126,   104,    42,   108,   129,   109,    -1,   126,    42,
       9,    -1,   126,   104,    42,     9,    -1,   126,    17,   127,
      90,   127,    -1,   126,    96,   127,    -1,   126,    95,   127,
      -1,   126,    97,   127,    -1,   126,    98,   127,    -1,   126,
      93,   128,    -1,   126,    94,   128,    -1,   126,    96,   128,
      -1,   126,    95,   128,    -1,   126,    17,   128,    90,   128,
      -1,   126,    97,   128,    -1,   126,    98,   128,    -1,     3,
      -1,     4,    -1,    24,   108,   101,   109,    -1,    39,   108,
     109,    -1,    86,   108,   109,    -1,    41,    -1,     5,    -1,
     100,     5,    -1,     6,    -1,   100,     6,    -1,   127,    -1,
     129,   107,   127,    -1,    -1,   131,    -1,    38,    18,   126,
      -1,    -1,   133,    -1,    88,    38,    58,    18,   136,    -1,
      -1,   135,    -1,    58,    18,   136,    -1,    58,    18,    60,
     108,   109,    -1,   137,    -1,   136,   107,   137,    -1,   126,
      -1,   126,    13,    -1,   126,    27,    -1,    -1,   139,    -1,
      49,     5,    -1,    49,     5,   107,     5,    -1,    -1,   141,
      -1,    57,   142,    -1,   143,    -1,   142,   107,   143,    -1,
       3,    93,     3,    -1,     3,    93,     5,    -1,     3,    93,
     108,   144,   109,    -1,     3,    93,     3,   108,     8,   109,
      -1,     3,    93,     8,    -1,   145,    -1,   144,   107,   145,
      -1,     3,    93,   127,    -1,     3,    -1,     4,    -1,    41,
      -1,     5,    -1,     6,    -1,     9,    -1,   100,   146,    -1,
     104,   146,    -1,   146,    99,   146,    -1,   146,   100,   146,
      -1,   146,   101,   146,    -1,   146,   102,   146,    -1,   146,
      95,   146,    -1,   146,    96,   146,    -1,   146,    92,   146,
      -1,   146,    91,   146,    -1,   146,   103,   146,    -1,   146,
      30,   146,    -1,   146,    54,   146,    -1,   146,    98,   146,
      -1,   146,    97,   146,    -1,   146,    93,   146,    -1,   146,
      94,   146,    -1,   146,    90,   146,    -1,   146,    89,   146,
      -1,   108,   146,   109,    -1,   147,    -1,     3,   108,   148,
     109,    -1,    42,   108,   148,   109,    -1,     3,   108,   109,
      -1,    53,   108,   146,   107,   146,   109,    -1,    51,   108,
     146,   107,   146,   109,    -1,   149,    -1,   148,   107,   149,
      -1,   146,    -1,     8,    -1,    71,   151,    -1,    85,    -1,
      74,    -1,    52,    -1,     3,    -1,    56,    -1,     8,    -1,
       5,    -1,     6,    -1,    69,     3,    93,   156,    -1,    69,
       3,    93,   155,    -1,    69,     3,    93,    56,    -1,    69,
      55,   152,    -1,    69,    10,    93,   152,    -1,    69,    37,
       9,    93,   108,   129,   109,    -1,    69,    37,     3,    93,
     155,    -1,     3,    -1,     8,    -1,    79,    -1,    32,    -1,
     127,    -1,    22,    -1,    65,    -1,   158,    -1,    16,    -1,
      73,    78,    -1,   160,    46,     3,   161,    83,   163,    -1,
      44,    -1,    63,    -1,    -1,   108,   162,   109,    -1,   126,
      -1,   162,   107,   126,    -1,   164,    -1,   163,   107,   164,
      -1,   108,   165,   109,    -1,   166,    -1,   165,   107,   166,
      -1,   127,    -1,   128,    -1,     8,    -1,   108,   129,   109,
      -1,   108,   109,    -1,    26,    35,   121,    87,    41,    93,
     127,    -1,    26,    35,   121,    87,    41,    42,   108,   129,
     109,    -1,    19,     3,   108,   169,   172,   109,    -1,   170,
      -1,   169,   107,   170,    -1,   166,    -1,   108,   171,   109,
      -1,     8,    -1,   171,   107,     8,    -1,    -1,   107,   173,
      -1,   174,    -1,   173,   107,   174,    -1,   166,   175,   176,
      -1,    -1,    12,    -1,     3,    -1,    49,    -1,   178,     3,
      -1,    28,    -1,    27,    -1,    71,    76,    -1,    82,   121,
      69,   181,   123,    -1,   182,    -1,   181,   107,   182,    -1,
       3,    93,   127,    -1,     3,    93,   128,    -1,     3,    93,
     108,   129,   109,    -1,     3,    93,   108,   109,    -1,    71,
     191,    84,   184,    -1,    -1,   185,    -1,    87,   186,    -1,
     187,    -1,   186,    89,   187,    -1,     3,    93,     8,    -1,
      71,    21,    -1,    71,    20,    69,    -1,    69,   191,    78,
      47,    48,   192,    -1,    -1,    37,    -1,    70,    -1,    61,
      81,    -1,    61,    23,    -1,    62,    61,    -1,    68,    -1,
      25,    36,     3,    64,   194,    72,     8,    -1,    45,    -1,
      33,    -1,    31,    36,     3,    -1,    14,    43,     3,    77,
      66,     3,    -1,    34,    66,     3,    -1,    67,   199,   138,
      -1,    10,    -1,    10,   110,     3,    -1,    80,    66,     3,
      -1,    59,    43,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   124,   124,   125,   126,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   155,   156,   160,   161,   165,
     180,   181,   185,   186,   189,   191,   192,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   209,   210,   213,
     215,   219,   223,   224,   228,   233,   240,   248,   256,   265,
     270,   275,   280,   285,   290,   295,   300,   301,   302,   303,
     308,   313,   318,   326,   327,   332,   338,   344,   350,   359,
     360,   364,   365,   369,   375,   381,   383,   387,   394,   396,
     400,   406,   408,   412,   416,   423,   424,   428,   429,   430,
     433,   435,   439,   444,   451,   453,   457,   461,   462,   466,
     471,   476,   482,   487,   495,   500,   507,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   527,   528,   529,
     530,   531,   532,   533,   534,   535,   536,   537,   538,   539,
     540,   541,   542,   543,   547,   548,   549,   550,   551,   555,
     556,   560,   561,   567,   571,   572,   573,   579,   580,   581,
     582,   583,   587,   592,   597,   602,   603,   607,   612,   620,
     621,   625,   626,   627,   641,   642,   643,   647,   648,   654,
     662,   663,   666,   668,   672,   673,   677,   678,   682,   686,
     687,   691,   692,   693,   694,   695,   701,   709,   723,   731,
     735,   742,   743,   750,   760,   766,   768,   772,   777,   781,
     788,   790,   794,   795,   801,   809,   810,   816,   822,   830,
     831,   835,   839,   843,   847,   857,   863,   864,   868,   872,
     873,   877,   881,   888,   895,   901,   902,   903,   907,   908,
     909,   910,   916,   927,   928,   932,   943,   955,   966,   974,
     975,   984,   995
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
  "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", "TOK_COLLATION", 
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", 
  "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", "TOK_FUNCTION", 
  "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", "TOK_GROUP_CONCAT", "TOK_ID", 
  "TOK_IN", "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTO", 
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", 
  "TOK_ORDER", "TOK_OPTIMIZE", "TOK_RAND", "TOK_READ", "TOK_REPEATABLE", 
  "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", 
  "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", "TOK_SESSION", "TOK_SHOW", 
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_SUM", "TOK_TABLES", 
  "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", "TOK_TRUNCATE", 
  "TOK_UNCOMMITTED", "TOK_UPDATE", "TOK_VALUES", "TOK_VARIABLES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", 
  "';'", "','", "'('", "')'", "'.'", "$accept", "request", "statement", 
  "multi_stmt_list", "multi_stmt", "select_from", "select_items_list", 
  "select_item", "opt_alias", "select_expr", "ident_list", 
  "opt_where_clause", "where_clause", "where_expr", "where_item", 
  "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "show_stmt", "show_what", "set_value", "set_stmt", "set_global_stmt", 
  "set_string_value", "boolean_value", "transact_op", "start_transaction", 
  "insert_into", "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", "show_variables", 
  "opt_show_variables_where", "show_variables_where", 
  "show_variables_where_list", "show_variables_where_entry", 
  "show_collation", "show_character_set", "set_transaction", "opt_scope", 
  "isolation_level", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", "select_sysvar", "sysvar_name", 
  "truncate", "optimize_index", 0
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
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   124,    38,    61,   346,    60,    62,   347,   348,    43,
      45,    42,    47,    37,   349,   350,    59,    44,    40,    41,
      46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   111,   112,   112,   112,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   114,   114,   115,   115,   116,
     117,   117,   118,   118,   119,   119,   119,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   121,   121,   122,
     122,   123,   124,   124,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   126,   126,   126,   126,   126,   126,   127,
     127,   128,   128,   129,   129,   130,   130,   131,   132,   132,
     133,   134,   134,   135,   135,   136,   136,   137,   137,   137,
     138,   138,   139,   139,   140,   140,   141,   142,   142,   143,
     143,   143,   143,   143,   144,   144,   145,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   147,   147,   147,   147,   147,   148,
     148,   149,   149,   150,   151,   151,   151,   152,   152,   152,
     152,   152,   153,   153,   153,   153,   153,   154,   154,   155,
     155,   156,   156,   156,   157,   157,   157,   158,   158,   159,
     160,   160,   161,   161,   162,   162,   163,   163,   164,   165,
     165,   166,   166,   166,   166,   166,   167,   167,   168,   169,
     169,   170,   170,   171,   171,   172,   172,   173,   173,   174,
     175,   175,   176,   176,   177,   178,   178,   179,   180,   181,
     181,   182,   182,   182,   182,   183,   184,   184,   185,   186,
     186,   187,   188,   189,   190,   191,   191,   191,   192,   192,
     192,   192,   193,   194,   194,   195,   196,   197,   198,   199,
     199,   200,   201
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,    10,
       1,     3,     1,     2,     0,     1,     2,     1,     4,     4,
       4,     4,     4,     4,     3,     3,     5,     1,     3,     0,
       1,     2,     1,     3,     4,     3,     3,     5,     6,     3,
       4,     5,     3,     3,     3,     3,     3,     3,     3,     3,
       5,     3,     3,     1,     1,     4,     3,     3,     1,     1,
       2,     1,     2,     1,     3,     0,     1,     3,     0,     1,
       5,     0,     1,     3,     5,     1,     3,     1,     2,     2,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     6,     3,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     4,     4,     3,     6,     6,     1,
       3,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     4,     4,     3,     4,     7,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     6,
       1,     1,     0,     3,     1,     3,     1,     3,     3,     1,
       3,     1,     1,     1,     3,     2,     7,     9,     6,     1,
       3,     1,     3,     1,     3,     0,     2,     1,     3,     3,
       0,     1,     1,     1,     2,     1,     1,     2,     5,     1,
       3,     3,     3,     5,     4,     4,     0,     1,     2,     1,
       3,     3,     2,     3,     6,     0,     1,     1,     2,     2,
       2,     1,     7,     1,     1,     3,     6,     3,     3,     1,
       3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   177,     0,   174,     0,     0,   216,   215,     0,
       0,   180,     0,   181,   175,     0,   235,   235,     0,     0,
       0,     0,     2,     3,    25,    27,    28,     7,     8,     9,
     176,     5,     0,     6,    10,    11,     0,    12,    13,    14,
      15,    16,    21,    17,    18,    19,    20,    22,    23,    24,
       0,     0,     0,     0,     0,     0,     0,   117,   118,   120,
     121,   122,   249,     0,     0,     0,     0,   119,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,    30,    34,
      37,   143,   100,     0,     0,   236,     0,   237,     0,     0,
     232,   236,   156,   155,   217,   154,   153,     0,   178,     0,
      47,     0,     1,     4,     0,   214,     0,     0,     0,     0,
     245,   247,   252,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   123,   124,     0,     0,
       0,    35,     0,    33,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   248,   101,     0,     0,     0,     0,   157,   160,
     161,   159,   158,   165,     0,   233,   226,   251,     0,     0,
       0,     0,    26,   182,     0,    79,    81,   193,     0,     0,
     191,   192,   201,   205,   199,     0,     0,   152,   146,   151,
       0,   149,   250,     0,     0,     0,    45,     0,     0,     0,
       0,     0,    44,     0,     0,   142,    49,    31,    36,   134,
     135,   141,   140,   132,   131,   138,   139,   129,   130,   137,
     136,   125,   126,   127,   128,   133,   102,   169,   170,   172,
     164,   171,     0,   173,   163,   162,   166,     0,     0,     0,
       0,   225,   227,     0,     0,   219,    48,     0,     0,     0,
      80,    82,   203,   195,    83,     0,     0,     0,     0,   244,
     243,     0,     0,     0,   144,    38,     0,    43,    42,   145,
       0,    39,     0,    40,    41,     0,     0,     0,    85,    50,
       0,   168,     0,     0,     0,   228,   229,     0,     0,   218,
      73,    74,     0,     0,    78,     0,   184,     0,     0,   246,
       0,   194,     0,   202,   201,   200,   206,   207,   198,     0,
       0,     0,   150,    46,     0,     0,     0,    51,    52,     0,
       0,    88,    86,   103,     0,     0,     0,   241,   234,     0,
       0,     0,   221,   222,   220,     0,     0,     0,     0,   183,
       0,   179,   186,    84,   204,   211,     0,     0,   242,     0,
     196,   148,   147,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    91,    89,   167,   239,
     238,   240,   231,   230,   224,     0,     0,    76,    77,   185,
       0,     0,   189,     0,   212,   213,   209,   210,   208,     0,
       0,    53,     0,     0,    59,     0,    55,    66,    56,    67,
      63,    69,    62,    68,    64,    71,    65,    72,     0,    87,
       0,     0,   100,    92,   223,    75,     0,   188,   187,   197,
      54,     0,     0,     0,    60,     0,     0,     0,   104,   190,
      61,     0,    70,    57,     0,     0,     0,    97,    93,    95,
       0,    29,   105,    58,    90,     0,    98,    99,     0,     0,
     106,   107,    94,    96,     0,     0,   109,   110,   113,     0,
     108,     0,     0,     0,   114,     0,     0,     0,   111,   112,
     116,   115
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,    77,    78,   133,    79,
     101,   278,   279,   317,   318,   437,   254,   181,   255,   321,
     322,   366,   367,   412,   413,   438,   439,   152,   153,   441,
     442,   450,   451,   463,   464,    80,    81,   190,   191,    26,
      96,   163,    27,    28,   234,   235,    29,    30,    31,    32,
     248,   297,   341,   342,   381,   182,    33,    34,   183,   184,
     256,   258,   306,   307,   346,   386,    35,    36,    37,    38,
     244,   245,    39,   241,   242,   285,   286,    40,    41,    42,
      88,   328,    43,   261,    44,    45,    46,    47,    82,    48,
      49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -268
static const short yypact[] =
{
     726,   -17,  -268,    58,  -268,    53,    70,  -268,  -268,    80,
      77,  -268,    93,  -268,  -268,   114,   688,   707,    68,    86,
     160,   179,  -268,    72,  -268,  -268,  -268,  -268,  -268,  -268,
    -268,  -268,   145,  -268,  -268,  -268,   192,  -268,  -268,  -268,
    -268,  -268,  -268,  -268,  -268,  -268,  -268,  -268,  -268,  -268,
     193,    97,   210,   160,   213,   214,   224,   120,  -268,  -268,
    -268,  -268,    89,   123,   126,   149,   150,  -268,   155,   157,
     161,   174,   180,   233,  -268,   233,   233,   -22,  -268,    28,
     536,  -268,   242,   199,   200,    21,   337,  -268,   230,   243,
    -268,  -268,  -268,  -268,  -268,  -268,  -268,   227,  -268,   316,
    -268,   -50,  -268,   -42,   319,  -268,   246,    37,   262,   -66,
    -268,  -268,  -268,    98,   331,   233,     7,   226,   233,   168,
     233,   233,   233,   235,   239,   247,  -268,  -268,   335,   160,
     220,  -268,   353,  -268,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   352,  -268,  -268,   238,   337,   265,   268,  -268,  -268,
    -268,  -268,  -268,  -268,   315,  -268,   276,  -268,   361,   363,
     220,   107,  -268,   259,   302,  -268,  -268,  -268,   164,    12,
    -268,  -268,  -268,   264,  -268,   -10,   328,  -268,  -268,   536,
     -70,  -268,  -268,   360,   367,   282,  -268,   386,    35,   285,
     306,   411,  -268,   233,   233,  -268,   -59,  -268,  -268,  -268,
    -268,   561,   586,   611,   636,   620,   620,   178,   178,   178,
     178,    29,    29,  -268,  -268,  -268,   266,  -268,  -268,  -268,
    -268,  -268,   405,  -268,  -268,  -268,  -268,    55,   303,   364,
     414,  -268,  -268,   325,   -53,  -268,  -268,   144,   336,   417,
    -268,  -268,  -268,  -268,  -268,    79,   140,    37,   312,  -268,
    -268,   350,    91,   168,  -268,  -268,   314,  -268,  -268,  -268,
     233,  -268,   233,  -268,  -268,   486,   511,   263,   401,  -268,
     437,  -268,    -2,    -6,   354,   356,  -268,    82,   361,  -268,
    -268,  -268,   338,   340,  -268,   359,  -268,   176,   362,  -268,
      -2,  -268,   435,  -268,   163,  -268,   357,  -268,  -268,   460,
     365,    -2,  -268,  -268,   436,   461,   366,   381,  -268,   203,
     454,   404,  -268,  -268,   196,   -21,   432,  -268,  -268,   488,
     414,     0,  -268,  -268,  -268,   393,   388,   389,   144,  -268,
     156,   392,  -268,  -268,  -268,  -268,   138,   156,  -268,    -2,
    -268,  -268,  -268,   509,   263,    27,     3,    27,    27,    27,
      27,    27,    27,   476,   144,   481,   463,  -268,  -268,  -268,
    -268,  -268,  -268,  -268,  -268,   209,   413,  -268,  -268,  -268,
      13,   222,  -268,   362,  -268,  -268,  -268,   512,  -268,   223,
     433,  -268,   453,   456,  -268,    -2,  -268,  -268,  -268,  -268,
    -268,  -268,  -268,  -268,  -268,  -268,  -268,  -268,    18,  -268,
     465,   526,   242,  -268,  -268,  -268,   156,  -268,  -268,  -268,
    -268,    -2,    10,   241,  -268,    -2,   529,   286,   491,  -268,
    -268,   543,  -268,  -268,   244,   144,   459,   184,   462,  -268,
     565,  -268,  -268,  -268,   462,   464,  -268,  -268,   144,   478,
     467,  -268,  -268,  -268,     6,   565,   484,  -268,  -268,   569,
    -268,   587,   501,   245,  -268,   487,    -2,   569,  -268,  -268,
    -268,  -268
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -268,  -268,  -268,  -268,   494,  -268,  -268,   468,  -268,  -268,
     -43,  -268,   355,  -268,   267,  -239,  -107,  -265,  -267,  -268,
    -268,  -268,  -268,  -268,  -268,   182,   171,   208,  -268,  -268,
    -268,  -268,   167,  -268,   175,   -69,  -268,   504,   380,  -268,
    -268,   469,  -268,  -268,   407,  -268,  -268,  -268,  -268,  -268,
    -268,  -268,  -268,   284,  -268,  -256,  -268,  -268,  -268,   390,
    -268,  -268,  -268,   298,  -268,  -268,  -268,  -268,  -268,  -268,
    -268,   358,  -268,  -268,  -268,  -268,   318,  -268,  -268,  -268,
     632,  -268,  -268,  -268,  -268,  -268,  -268,  -268,  -268,  -268,
    -268
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -211
static const short yytable[] =
{
     180,   304,   369,   175,   126,   175,   127,   128,   296,   456,
     109,   457,   394,   129,   458,   324,   176,   175,   175,   168,
     252,   186,   333,   259,   156,   170,    50,   424,   277,   171,
     157,   131,   175,   176,   277,   260,   194,   263,   319,   264,
     132,   169,   175,   176,   189,   177,   193,   233,   169,   197,
     189,   199,   200,   201,   288,   325,   326,   169,   227,   134,
     370,    51,   327,   228,   375,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   389,   135,   382,   130,   206,   175,   176,    52,
     393,   387,   397,   399,   401,   403,   405,   407,   232,   379,
     232,    57,    58,    59,    60,    53,   187,    61,   195,   374,
     431,   395,   232,   232,   459,   319,    54,    57,    58,    59,
      60,   253,   253,    61,    62,   409,   425,   178,   423,    63,
     148,   149,   150,   310,   275,   276,    56,   178,    64,    67,
      68,   384,   263,    55,   269,   179,    98,   290,   291,   124,
     180,   125,    99,    65,    66,    67,    68,   432,   434,    92,
     429,   175,   176,   100,   177,    69,  -210,    70,   292,   250,
     251,    57,    58,    59,    60,   345,   187,    61,   103,   102,
     332,    93,   178,   293,   311,   294,   300,   385,   301,    71,
     331,   104,    95,   343,   189,   105,   106,   446,    73,   114,
      72,   314,    75,   315,   350,   107,    76,   188,   134,    67,
      68,   447,  -210,   108,    73,    74,   110,   111,    75,   124,
     355,   125,    76,    57,    58,    59,    60,   112,   113,    61,
     295,   115,   135,   180,   116,    63,    57,    58,    59,    60,
     180,   227,    61,   175,    64,   356,   228,   302,   392,   303,
     396,   398,   400,   402,   404,   406,   178,   117,   118,    65,
      66,    67,    68,   119,   380,   120,   290,   291,    73,   121,
     229,    69,    75,    70,    67,    68,    76,   146,   147,   148,
     149,   150,   122,   338,   124,   339,   125,   292,   123,   290,
     291,   151,   154,   155,   230,    71,   357,   358,   359,   360,
     361,   362,   293,   300,   294,   368,    72,   363,   164,   180,
     292,   166,   165,   316,   430,   134,   300,   231,   414,   167,
      73,    74,   173,   174,    75,   293,   185,   294,    76,   416,
     300,   417,   419,    73,   192,   196,   134,    75,   232,   135,
     158,    76,   159,   160,   202,   161,   436,   203,   300,   295,
     433,   300,   467,   443,   468,   204,   208,   226,   237,   470,
     135,   238,   239,   240,   243,   134,   246,   247,   249,   262,
     266,   257,   295,   280,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   135,
     134,   267,   270,   162,   271,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     250,   282,   283,   272,   135,   273,   134,   284,   287,   298,
     299,   308,   309,   313,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   320,
     135,   134,   323,   344,   205,   330,   335,   329,   336,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   347,   135,   134,   337,   348,   265,
     340,   354,   364,   349,   353,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     135,   134,   365,   371,   376,   268,   372,   377,   378,   383,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   135,   134,   390,   408,   410,
     274,   411,   415,   426,   345,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     135,   134,   420,   421,   427,   351,   422,   435,   440,   251,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   135,   134,   445,   449,   448,
     352,   454,   462,   452,   455,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     135,   134,   461,   270,   466,   465,   469,   172,   207,   289,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   135,   134,   444,   272,   453,
     428,   391,   460,   198,   236,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     135,   134,   471,   312,   281,   388,   334,   305,   373,    97,
     134,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   135,   134,   418,     0,     0,
       0,     0,     0,     0,   135,     0,     0,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     135,    83,     0,     0,     0,     0,     0,     0,    84,     0,
       0,     0,     0,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   142,   143,   144,   145,   146,
     147,   148,   149,   150,     0,    85,     0,    89,    90,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
       1,     0,     2,    86,    91,     3,     0,     0,     4,     0,
       0,     5,     6,     7,     8,     0,     0,     9,    87,    92,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,     0,     0,     0,     0,     0,     0,    87,     0,     0,
       0,    93,     0,    94,     0,    12,     0,     0,     0,    13,
       0,    14,    95,    15,     0,    16,     0,    17,     0,    18,
       0,     0,     0,     0,     0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     107,   257,    23,     5,    73,     5,    75,    76,   247,     3,
      53,     5,     9,    35,     8,   282,     6,     5,     5,    69,
       8,    87,   287,    33,     3,    67,    43,     9,    87,    71,
       9,     3,     5,     6,    87,    45,    29,   107,   277,   109,
      12,   107,     5,     6,   113,     8,   115,   154,   107,   118,
     119,   120,   121,   122,   107,    61,    62,   107,     3,    30,
      81,     3,    68,     8,   331,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   349,    54,   340,   107,   129,     5,     6,    36,
     355,   347,   357,   358,   359,   360,   361,   362,   100,   338,
     100,     3,     4,     5,     6,    35,     8,     9,   101,   109,
     100,   108,   100,   100,   108,   354,    36,     3,     4,     5,
       6,   109,   109,     9,    10,   364,   108,   100,   395,    15,
     101,   102,   103,    42,   203,   204,    43,   100,    24,    41,
      42,     3,   107,    66,   109,   108,    78,     3,     4,    51,
     257,    53,    66,    39,    40,    41,    42,   422,   425,    52,
     416,     5,     6,     3,     8,    51,     3,    53,    24,     5,
       6,     3,     4,     5,     6,    12,     8,     9,   106,     0,
     287,    74,   100,    39,    93,    41,   107,    49,   109,    75,
     108,    46,    85,   300,   263,     3,     3,    13,   100,   110,
      86,   270,   104,   272,   311,   108,   108,   109,    30,    41,
      42,    27,    49,     3,   100,   101,     3,     3,   104,    51,
      17,    53,   108,     3,     4,     5,     6,     3,   108,     9,
      86,   108,    54,   340,   108,    15,     3,     4,     5,     6,
     347,     3,     9,     5,    24,    42,     8,   107,   355,   109,
     357,   358,   359,   360,   361,   362,   100,   108,   108,    39,
      40,    41,    42,   108,   108,   108,     3,     4,   100,   108,
      32,    51,   104,    53,    41,    42,   108,    99,   100,   101,
     102,   103,   108,   107,    51,   109,    53,    24,   108,     3,
       4,    49,    93,    93,    56,    75,    93,    94,    95,    96,
      97,    98,    39,   107,    41,   109,    86,   104,    78,   416,
      24,    84,    69,    50,   421,    30,   107,    79,   109,     3,
     100,   101,     3,    77,   104,    39,    64,    41,   108,   107,
     107,   109,   109,   100,     3,   109,    30,   104,   100,    54,
       3,   108,     5,     6,   109,     8,    60,   108,   107,    86,
     109,   107,   107,   109,   109,   108,     3,     5,    93,   466,
      54,    93,    47,    87,     3,    30,     3,   108,    66,    41,
       3,   107,    86,   107,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,    54,
      30,   109,   107,    56,   109,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
       5,   108,    48,   107,    54,   109,    30,     3,    93,    83,
       3,   109,    72,   109,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,    38,
      54,    30,     5,     8,   109,    89,   108,    93,   108,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   107,    54,    30,   108,     8,   109,
     108,    90,    18,   108,   108,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      54,    30,    88,    61,   101,   109,     8,   109,   109,   107,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,    54,    30,     8,    42,    38,
     109,    58,   109,    58,    12,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      54,    30,   109,    90,    18,   109,    90,    18,    57,     6,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,    54,    30,   108,     3,   107,
     109,    93,     3,   109,   107,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      54,    30,   108,   107,    93,     8,   109,   103,   130,   244,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,    54,    30,   435,   107,   448,
     412,   354,   455,   119,   155,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      54,    30,   467,   263,   237,   347,   288,   257,   330,    17,
      30,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,    54,    30,   383,    -1,    -1,
      -1,    -1,    -1,    -1,    54,    -1,    -1,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      54,     3,    -1,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,    95,    96,    97,    98,    99,
     100,   101,   102,   103,    -1,    37,    -1,    20,    21,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      14,    -1,    16,    55,    37,    19,    -1,    -1,    22,    -1,
      -1,    25,    26,    27,    28,    -1,    -1,    31,    70,    52,
      34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    74,    -1,    76,    -1,    59,    -1,    -1,    -1,    63,
      -1,    65,    85,    67,    -1,    69,    -1,    71,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    -1,    82
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    16,    19,    22,    25,    26,    27,    28,    31,
      34,    44,    59,    63,    65,    67,    69,    71,    73,    80,
      82,   112,   113,   114,   115,   116,   150,   153,   154,   157,
     158,   159,   160,   167,   168,   177,   178,   179,   180,   183,
     188,   189,   190,   193,   195,   196,   197,   198,   200,   201,
      43,     3,    36,    35,    36,    66,    43,     3,     4,     5,
       6,     9,    10,    15,    24,    39,    40,    41,    42,    51,
      53,    75,    86,   100,   101,   104,   108,   117,   118,   120,
     146,   147,   199,     3,    10,    37,    55,    70,   191,    20,
      21,    37,    52,    74,    76,    85,   151,   191,    78,    66,
       3,   121,     0,   106,    46,     3,     3,   108,     3,   121,
       3,     3,     3,   108,   110,   108,   108,   108,   108,   108,
     108,   108,   108,   108,    51,    53,   146,   146,   146,    35,
     107,     3,    12,   119,    30,    54,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,    49,   138,   139,    93,    93,     3,     9,     3,     5,
       6,     8,    56,   152,    78,    69,    84,     3,    69,   107,
      67,    71,   115,     3,    77,     5,     6,     8,   100,   108,
     127,   128,   166,   169,   170,    64,    87,     8,   109,   146,
     148,   149,     3,   146,    29,   101,   109,   146,   148,   146,
     146,   146,   109,   108,   108,   109,   121,   118,     3,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,     5,     3,     8,    32,
      56,    79,   100,   127,   155,   156,   152,    93,    93,    47,
      87,   184,   185,     3,   181,   182,     3,   108,   161,    66,
       5,     6,     8,   109,   127,   129,   171,   107,   172,    33,
      45,   194,    41,   107,   109,   109,     3,   109,   109,   109,
     107,   109,   107,   109,   109,   146,   146,    87,   122,   123,
     107,   155,   108,    48,     3,   186,   187,    93,   107,   123,
       3,     4,    24,    39,    41,    86,   126,   162,    83,     3,
     107,   109,   107,   109,   166,   170,   173,   174,   109,    72,
      42,    93,   149,   109,   146,   146,    50,   124,   125,   126,
      38,   130,   131,     5,   129,    61,    62,    68,   192,    93,
      89,   108,   127,   128,   182,   108,   108,   108,   107,   109,
     108,   163,   164,   127,     8,    12,   175,   107,     8,   108,
     127,   109,   109,   108,    90,    17,    42,    93,    94,    95,
      96,    97,    98,   104,    18,    88,   132,   133,   109,    23,
      81,    61,     8,   187,   109,   129,   101,   109,   109,   126,
     108,   165,   166,   107,     3,    49,   176,   166,   174,   129,
       8,   125,   127,   128,     9,   108,   127,   128,   127,   128,
     127,   128,   127,   128,   127,   128,   127,   128,    42,   126,
      38,    58,   134,   135,   109,   109,   107,   109,   164,   109,
     109,    90,    90,   129,     9,   108,    58,    18,   138,   166,
     127,   100,   128,   109,   129,    18,    60,   126,   136,   137,
      57,   140,   141,   109,   136,   108,    13,    27,   107,     3,
     142,   143,   109,   137,    93,   107,     3,     5,     8,   108,
     143,   108,     3,   144,   145,     8,    93,   107,   109,   109,
     127,   145
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

  case 25:

    { pParser->PushQuery(); ;}
    break;

  case 26:

    { pParser->PushQuery(); ;}
    break;

  case 29:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 32:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 36:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 37:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 38:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 39:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 40:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 41:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 42:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 43:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 44:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 45:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 46:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 48:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 54:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 55:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 56:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 57:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 58:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 59:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 60:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, LLONG_MAX ) )
				YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-2].m_sValue, LLONG_MIN, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 64:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, LLONG_MAX ) )
				YYERROR;
		;}
    break;

  case 65:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-2].m_sValue, LLONG_MIN, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 70:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 76:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 77:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 78:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 79:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 80:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 81:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 82:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 83:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 84:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 87:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 90:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 93:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 94:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 96:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 98:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 99:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 102:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 103:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 109:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 110:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 111:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 112:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 113:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 114:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 115:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 116:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 118:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 119:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 123:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 136:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 137:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 138:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 139:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 140:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 141:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 142:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 144:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 145:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 146:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 147:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 148:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 154:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 155:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 156:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 162:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 163:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 164:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 165:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 166:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 167:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 168:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 171:

    { yyval.m_iValue = 1; ;}
    break;

  case 172:

    { yyval.m_iValue = 0; ;}
    break;

  case 173:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 175:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 176:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 179:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 180:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 181:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 184:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 185:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 188:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 189:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 190:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 191:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 192:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 193:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 194:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 195:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 196:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 197:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 198:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 199:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 200:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 202:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 203:

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

  case 204:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 207:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 209:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 213:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 214:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 217:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 218:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 221:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 222:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 223:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 224:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 225:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 232:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 233:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 234:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 242:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 243:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 244:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 245:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 246:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 247:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 248:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 250:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 251:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 252:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
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

