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
     TOK_GROUPBY = 293,
     TOK_ID = 294,
     TOK_IN = 295,
     TOK_INDEX = 296,
     TOK_INSERT = 297,
     TOK_INT = 298,
     TOK_INTO = 299,
     TOK_ISOLATION = 300,
     TOK_LEVEL = 301,
     TOK_LIMIT = 302,
     TOK_MATCH = 303,
     TOK_MAX = 304,
     TOK_META = 305,
     TOK_MIN = 306,
     TOK_MOD = 307,
     TOK_NAMES = 308,
     TOK_NULL = 309,
     TOK_OPTION = 310,
     TOK_ORDER = 311,
     TOK_RAND = 312,
     TOK_READ = 313,
     TOK_REPEATABLE = 314,
     TOK_REPLACE = 315,
     TOK_RETURNS = 316,
     TOK_ROLLBACK = 317,
     TOK_RTINDEX = 318,
     TOK_SELECT = 319,
     TOK_SERIALIZABLE = 320,
     TOK_SET = 321,
     TOK_SESSION = 322,
     TOK_SHOW = 323,
     TOK_SONAME = 324,
     TOK_START = 325,
     TOK_STATUS = 326,
     TOK_SUM = 327,
     TOK_TABLES = 328,
     TOK_TO = 329,
     TOK_TRANSACTION = 330,
     TOK_TRUE = 331,
     TOK_TRUNCATE = 332,
     TOK_UNCOMMITTED = 333,
     TOK_UPDATE = 334,
     TOK_VALUES = 335,
     TOK_VARIABLES = 336,
     TOK_WARNINGS = 337,
     TOK_WEIGHT = 338,
     TOK_WHERE = 339,
     TOK_WITHIN = 340,
     TOK_OR = 341,
     TOK_AND = 342,
     TOK_NE = 343,
     TOK_GTE = 344,
     TOK_LTE = 345,
     TOK_NOT = 346,
     TOK_NEG = 347
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
#define TOK_GROUPBY 293
#define TOK_ID 294
#define TOK_IN 295
#define TOK_INDEX 296
#define TOK_INSERT 297
#define TOK_INT 298
#define TOK_INTO 299
#define TOK_ISOLATION 300
#define TOK_LEVEL 301
#define TOK_LIMIT 302
#define TOK_MATCH 303
#define TOK_MAX 304
#define TOK_META 305
#define TOK_MIN 306
#define TOK_MOD 307
#define TOK_NAMES 308
#define TOK_NULL 309
#define TOK_OPTION 310
#define TOK_ORDER 311
#define TOK_RAND 312
#define TOK_READ 313
#define TOK_REPEATABLE 314
#define TOK_REPLACE 315
#define TOK_RETURNS 316
#define TOK_ROLLBACK 317
#define TOK_RTINDEX 318
#define TOK_SELECT 319
#define TOK_SERIALIZABLE 320
#define TOK_SET 321
#define TOK_SESSION 322
#define TOK_SHOW 323
#define TOK_SONAME 324
#define TOK_START 325
#define TOK_STATUS 326
#define TOK_SUM 327
#define TOK_TABLES 328
#define TOK_TO 329
#define TOK_TRANSACTION 330
#define TOK_TRUE 331
#define TOK_TRUNCATE 332
#define TOK_UNCOMMITTED 333
#define TOK_UPDATE 334
#define TOK_VALUES 335
#define TOK_VARIABLES 336
#define TOK_WARNINGS 337
#define TOK_WEIGHT 338
#define TOK_WHERE 339
#define TOK_WITHIN 340
#define TOK_OR 341
#define TOK_AND 342
#define TOK_NE 343
#define TOK_GTE 344
#define TOK_LTE 345
#define TOK_NOT 346
#define TOK_NEG 347




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
#define YYFINAL  96
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   788

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  108
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  89
/* YYNRULES -- Number of rules. */
#define YYNRULES  247
/* YYNRULES -- Number of states. */
#define YYNSTATES  461

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   347

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   100,    89,     2,
     105,   106,    98,    96,   104,    97,   107,    99,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   103,
      92,    90,    93,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    88,     2,     2,     2,     2,     2,
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
      85,    86,    87,    91,    94,    95,   101,   102
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    52,    54,    56,    67,    69,
      73,    75,    78,    79,    81,    84,    86,    91,    96,   101,
     106,   111,   115,   119,   125,   127,   131,   132,   134,   137,
     139,   143,   148,   152,   156,   162,   169,   173,   178,   184,
     188,   192,   196,   200,   204,   208,   212,   216,   222,   226,
     230,   232,   234,   239,   243,   247,   249,   251,   254,   256,
     259,   261,   265,   266,   268,   272,   273,   275,   281,   282,
     284,   288,   294,   296,   300,   302,   305,   308,   309,   311,
     314,   319,   320,   322,   325,   327,   331,   335,   339,   345,
     352,   356,   358,   362,   366,   368,   370,   372,   374,   376,
     378,   381,   384,   388,   392,   396,   400,   404,   408,   412,
     416,   420,   424,   428,   432,   436,   440,   444,   448,   452,
     456,   458,   463,   468,   472,   479,   486,   488,   492,   494,
     496,   499,   501,   503,   505,   507,   509,   511,   513,   515,
     520,   525,   530,   534,   539,   547,   553,   555,   557,   559,
     561,   563,   565,   567,   569,   571,   574,   581,   583,   585,
     586,   590,   592,   596,   598,   602,   606,   608,   612,   614,
     616,   618,   622,   625,   633,   643,   650,   652,   656,   658,
     662,   664,   668,   669,   672,   674,   678,   682,   683,   685,
     687,   689,   692,   694,   696,   699,   705,   707,   711,   715,
     719,   725,   730,   735,   736,   738,   741,   743,   747,   751,
     754,   761,   762,   764,   766,   769,   772,   775,   777,   785,
     787,   789,   793,   800,   804,   808,   810,   814
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     109,     0,    -1,   110,    -1,   111,    -1,   111,   103,    -1,
     156,    -1,   164,    -1,   150,    -1,   151,    -1,   154,    -1,
     165,    -1,   174,    -1,   176,    -1,   177,    -1,   180,    -1,
     185,    -1,   189,    -1,   191,    -1,   192,    -1,   193,    -1,
     186,    -1,   194,    -1,   196,    -1,   112,    -1,   111,   103,
     112,    -1,   113,    -1,   147,    -1,    64,   114,    34,   118,
     119,   127,   129,   131,   135,   137,    -1,   115,    -1,   114,
     104,   115,    -1,    98,    -1,   117,   116,    -1,    -1,     3,
      -1,    12,     3,    -1,   143,    -1,    15,   105,   143,   106,
      -1,    49,   105,   143,   106,    -1,    51,   105,   143,   106,
      -1,    72,   105,   143,   106,    -1,    23,   105,    98,   106,
      -1,    83,   105,   106,    -1,    38,   105,   106,    -1,    23,
     105,    28,     3,   106,    -1,     3,    -1,   118,   104,     3,
      -1,    -1,   120,    -1,    84,   121,    -1,   122,    -1,   121,
      87,   122,    -1,    48,   105,     8,   106,    -1,   123,    90,
     124,    -1,   123,    91,   124,    -1,   123,    40,   105,   126,
     106,    -1,   123,   101,    40,   105,   126,   106,    -1,   123,
      40,     9,    -1,   123,   101,    40,     9,    -1,   123,    17,
     124,    87,   124,    -1,   123,    93,   124,    -1,   123,    92,
     124,    -1,   123,    94,   124,    -1,   123,    95,   124,    -1,
     123,    90,   125,    -1,   123,    91,   125,    -1,   123,    93,
     125,    -1,   123,    92,   125,    -1,   123,    17,   125,    87,
     125,    -1,   123,    94,   125,    -1,   123,    95,   125,    -1,
       3,    -1,     4,    -1,    23,   105,    98,   106,    -1,    38,
     105,   106,    -1,    83,   105,   106,    -1,    39,    -1,     5,
      -1,    97,     5,    -1,     6,    -1,    97,     6,    -1,   124,
      -1,   126,   104,   124,    -1,    -1,   128,    -1,    37,    18,
     123,    -1,    -1,   130,    -1,    85,    37,    56,    18,   133,
      -1,    -1,   132,    -1,    56,    18,   133,    -1,    56,    18,
      57,   105,   106,    -1,   134,    -1,   133,   104,   134,    -1,
     123,    -1,   123,    13,    -1,   123,    26,    -1,    -1,   136,
      -1,    47,     5,    -1,    47,     5,   104,     5,    -1,    -1,
     138,    -1,    55,   139,    -1,   140,    -1,   139,   104,   140,
      -1,     3,    90,     3,    -1,     3,    90,     5,    -1,     3,
      90,   105,   141,   106,    -1,     3,    90,     3,   105,     8,
     106,    -1,     3,    90,     8,    -1,   142,    -1,   141,   104,
     142,    -1,     3,    90,   124,    -1,     3,    -1,     4,    -1,
      39,    -1,     5,    -1,     6,    -1,     9,    -1,    97,   143,
      -1,   101,   143,    -1,   143,    96,   143,    -1,   143,    97,
     143,    -1,   143,    98,   143,    -1,   143,    99,   143,    -1,
     143,    92,   143,    -1,   143,    93,   143,    -1,   143,    89,
     143,    -1,   143,    88,   143,    -1,   143,   100,   143,    -1,
     143,    29,   143,    -1,   143,    52,   143,    -1,   143,    95,
     143,    -1,   143,    94,   143,    -1,   143,    90,   143,    -1,
     143,    91,   143,    -1,   143,    87,   143,    -1,   143,    86,
     143,    -1,   105,   143,   106,    -1,   144,    -1,     3,   105,
     145,   106,    -1,    40,   105,   145,   106,    -1,     3,   105,
     106,    -1,    51,   105,   143,   104,   143,   106,    -1,    49,
     105,   143,   104,   143,   106,    -1,   146,    -1,   145,   104,
     146,    -1,   143,    -1,     8,    -1,    68,   148,    -1,    82,
      -1,    71,    -1,    50,    -1,     3,    -1,    54,    -1,     8,
      -1,     5,    -1,     6,    -1,    66,     3,    90,   153,    -1,
      66,     3,    90,   152,    -1,    66,     3,    90,    54,    -1,
      66,    53,   149,    -1,    66,    10,    90,   149,    -1,    66,
      36,     9,    90,   105,   126,   106,    -1,    66,    36,     3,
      90,   152,    -1,     3,    -1,     8,    -1,    76,    -1,    31,
      -1,   124,    -1,    21,    -1,    62,    -1,   155,    -1,    16,
      -1,    70,    75,    -1,   157,    44,     3,   158,    80,   160,
      -1,    42,    -1,    60,    -1,    -1,   105,   159,   106,    -1,
     123,    -1,   159,   104,   123,    -1,   161,    -1,   160,   104,
     161,    -1,   105,   162,   106,    -1,   163,    -1,   162,   104,
     163,    -1,   124,    -1,   125,    -1,     8,    -1,   105,   126,
     106,    -1,   105,   106,    -1,    25,    34,   118,    84,    39,
      90,   124,    -1,    25,    34,   118,    84,    39,    40,   105,
     126,   106,    -1,    19,     3,   105,   166,   169,   106,    -1,
     167,    -1,   166,   104,   167,    -1,   163,    -1,   105,   168,
     106,    -1,     8,    -1,   168,   104,     8,    -1,    -1,   104,
     170,    -1,   171,    -1,   170,   104,   171,    -1,   163,   172,
     173,    -1,    -1,    12,    -1,     3,    -1,    47,    -1,   175,
       3,    -1,    27,    -1,    26,    -1,    68,    73,    -1,    79,
     118,    66,   178,   120,    -1,   179,    -1,   178,   104,   179,
      -1,     3,    90,   124,    -1,     3,    90,   125,    -1,     3,
      90,   105,   126,   106,    -1,     3,    90,   105,   106,    -1,
      68,   187,    81,   181,    -1,    -1,   182,    -1,    84,   183,
      -1,   184,    -1,   183,    86,   184,    -1,     3,    90,     8,
      -1,    68,    20,    -1,    66,   187,    75,    45,    46,   188,
      -1,    -1,    36,    -1,    67,    -1,    58,    78,    -1,    58,
      22,    -1,    59,    58,    -1,    65,    -1,    24,    35,     3,
      61,   190,    69,     8,    -1,    43,    -1,    32,    -1,    30,
      35,     3,    -1,    14,    41,     3,    74,    63,     3,    -1,
      33,    63,     3,    -1,    64,   195,   135,    -1,    10,    -1,
      10,   107,     3,    -1,    77,    63,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   121,   121,   122,   123,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   150,   151,   155,   156,   160,   175,   176,
     180,   181,   184,   186,   187,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   203,   204,   207,   209,   213,   217,
     218,   222,   227,   234,   242,   250,   259,   264,   269,   274,
     279,   284,   289,   294,   295,   296,   297,   302,   307,   312,
     320,   321,   326,   332,   338,   344,   353,   354,   358,   359,
     363,   369,   375,   377,   381,   388,   390,   394,   400,   402,
     406,   410,   417,   418,   422,   423,   424,   427,   429,   433,
     438,   445,   447,   451,   455,   456,   460,   465,   470,   476,
     481,   489,   494,   501,   511,   512,   513,   514,   515,   516,
     517,   518,   519,   520,   521,   522,   523,   524,   525,   526,
     527,   528,   529,   530,   531,   532,   533,   534,   535,   536,
     537,   541,   542,   543,   544,   545,   549,   550,   554,   555,
     561,   565,   566,   567,   573,   574,   575,   576,   577,   581,
     586,   591,   596,   597,   601,   606,   614,   615,   619,   620,
     621,   635,   636,   637,   641,   642,   648,   656,   657,   660,
     662,   666,   667,   671,   672,   676,   680,   681,   685,   686,
     687,   688,   689,   695,   703,   717,   725,   729,   736,   737,
     744,   754,   760,   762,   766,   771,   775,   782,   784,   788,
     789,   795,   803,   804,   810,   816,   824,   825,   829,   833,
     837,   841,   851,   857,   858,   862,   866,   867,   871,   875,
     882,   888,   889,   890,   894,   895,   896,   897,   903,   914,
     915,   919,   930,   942,   953,   961,   962,   971
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
  "TOK_GROUP", "TOK_GROUPBY", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_ISOLATION", "TOK_LEVEL", 
  "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", 
  "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_RAND", 
  "TOK_READ", "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_SUM", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", 
  "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", "')'", "'.'", 
  "$accept", "request", "statement", "multi_stmt_list", "multi_stmt", 
  "select_from", "select_items_list", "select_item", "opt_alias", 
  "select_expr", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "where_item", "expr_ident", "const_int", "const_float", 
  "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "arg", "show_stmt", "show_what", "set_value", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "call_args_list", "call_arg", "const_string_list", "opt_call_opts_list", 
  "call_opts_list", "call_opt", "opt_as", "call_opt_name", "describe", 
  "describe_tok", "show_tables", "update", "update_items_list", 
  "update_item", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "set_transaction", 
  "opt_scope", "isolation_level", "create_function", "udf_type", 
  "drop_function", "attach_index", "flush_rtindex", "select_sysvar", 
  "sysvar_name", "truncate", 0
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
     335,   336,   337,   338,   339,   340,   341,   342,   124,    38,
      61,   343,    60,    62,   344,   345,    43,    45,    42,    47,
      37,   346,   347,    59,    44,    40,    41,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   108,   109,   109,   109,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   111,   111,   112,   112,   113,   114,   114,
     115,   115,   116,   116,   116,   117,   117,   117,   117,   117,
     117,   117,   117,   117,   118,   118,   119,   119,   120,   121,
     121,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     123,   123,   123,   123,   123,   123,   124,   124,   125,   125,
     126,   126,   127,   127,   128,   129,   129,   130,   131,   131,
     132,   132,   133,   133,   134,   134,   134,   135,   135,   136,
     136,   137,   137,   138,   139,   139,   140,   140,   140,   140,
     140,   141,   141,   142,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   144,   144,   144,   144,   144,   145,   145,   146,   146,
     147,   148,   148,   148,   149,   149,   149,   149,   149,   150,
     150,   150,   150,   150,   151,   151,   152,   152,   153,   153,
     153,   154,   154,   154,   155,   155,   156,   157,   157,   158,
     158,   159,   159,   160,   160,   161,   162,   162,   163,   163,
     163,   163,   163,   164,   164,   165,   166,   166,   167,   167,
     168,   168,   169,   169,   170,   170,   171,   172,   172,   173,
     173,   174,   175,   175,   176,   177,   178,   178,   179,   179,
     179,   179,   180,   181,   181,   182,   183,   183,   184,   185,
     186,   187,   187,   187,   188,   188,   188,   188,   189,   190,
     190,   191,   192,   193,   194,   195,   195,   196
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,    10,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     3,     3,     5,     1,     3,     0,     1,     2,     1,
       3,     4,     3,     3,     5,     6,     3,     4,     5,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     3,     3,
       1,     1,     4,     3,     3,     1,     1,     2,     1,     2,
       1,     3,     0,     1,     3,     0,     1,     5,     0,     1,
       3,     5,     1,     3,     1,     2,     2,     0,     1,     2,
       4,     0,     1,     2,     1,     3,     3,     3,     5,     6,
       3,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     4,     4,     3,     6,     6,     1,     3,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       4,     4,     3,     4,     7,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     6,     1,     1,     0,
       3,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     3,     2,     7,     9,     6,     1,     3,     1,     3,
       1,     3,     0,     2,     1,     3,     3,     0,     1,     1,
       1,     2,     1,     1,     2,     5,     1,     3,     3,     3,
       5,     4,     4,     0,     1,     2,     1,     3,     3,     2,
       6,     0,     1,     1,     2,     2,     2,     1,     7,     1,
       1,     3,     6,     3,     3,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   174,     0,   171,     0,     0,   213,   212,     0,
       0,   177,   178,   172,     0,   231,   231,     0,     0,     0,
       0,     2,     3,    23,    25,    26,     7,     8,     9,   173,
       5,     0,     6,    10,    11,     0,    12,    13,    14,    15,
      20,    16,    17,    18,    19,    21,    22,     0,     0,     0,
       0,     0,     0,   114,   115,   117,   118,   119,   245,     0,
       0,     0,   116,     0,     0,     0,     0,     0,     0,    30,
       0,     0,     0,    28,    32,    35,   140,    97,     0,     0,
     232,     0,   233,     0,   229,   232,   153,   152,   214,   151,
     150,     0,   175,     0,    44,     0,     1,     4,     0,   211,
       0,     0,     0,     0,   241,   243,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   120,   121,
       0,     0,     0,    33,     0,    31,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   244,    98,     0,     0,     0,     0,
     154,   157,   158,   156,   155,   162,     0,   223,   247,     0,
       0,     0,     0,    24,   179,     0,    76,    78,   190,     0,
       0,   188,   189,   198,   202,   196,     0,     0,   149,   143,
     148,     0,   146,   246,     0,     0,     0,    42,     0,     0,
       0,     0,    41,     0,     0,   139,    46,    29,    34,   131,
     132,   138,   137,   129,   128,   135,   136,   126,   127,   134,
     133,   122,   123,   124,   125,   130,    99,   166,   167,   169,
     161,   168,     0,   170,   160,   159,   163,     0,     0,     0,
       0,   222,   224,     0,     0,   216,    45,     0,     0,     0,
      77,    79,   200,   192,    80,     0,     0,     0,     0,   240,
     239,     0,     0,     0,   141,    36,     0,    40,   142,     0,
      37,     0,    38,    39,     0,     0,     0,    82,    47,     0,
     165,     0,     0,     0,   225,   226,     0,     0,   215,    70,
      71,     0,     0,    75,     0,   181,     0,     0,   242,     0,
     191,     0,   199,   198,   197,   203,   204,   195,     0,     0,
       0,   147,    43,     0,     0,     0,    48,    49,     0,     0,
      85,    83,   100,     0,     0,     0,   237,   230,     0,     0,
       0,   218,   219,   217,     0,     0,     0,     0,   180,     0,
     176,   183,    81,   201,   208,     0,     0,   238,     0,   193,
     145,   144,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    88,    86,   164,   235,   234,
     236,   228,   227,   221,     0,     0,    73,    74,   182,     0,
       0,   186,     0,   209,   210,   206,   207,   205,     0,     0,
      50,     0,     0,    56,     0,    52,    63,    53,    64,    60,
      66,    59,    65,    61,    68,    62,    69,     0,    84,     0,
       0,    97,    89,   220,    72,     0,   185,   184,   194,    51,
       0,     0,     0,    57,     0,     0,     0,   101,   187,    58,
       0,    67,    54,     0,     0,     0,    94,    90,    92,     0,
      27,   102,    55,    87,     0,    95,    96,     0,     0,   103,
     104,    91,    93,     0,     0,   106,   107,   110,     0,   105,
       0,     0,     0,   111,     0,     0,     0,   108,   109,   113,
     112
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    20,    21,    22,    23,    24,    72,    73,   125,    74,
      95,   267,   268,   306,   307,   426,   244,   172,   245,   310,
     311,   355,   356,   401,   402,   427,   428,   144,   145,   430,
     431,   439,   440,   452,   453,    75,    76,   181,   182,    25,
      90,   155,    26,    27,   224,   225,    28,    29,    30,    31,
     238,   286,   330,   331,   370,   173,    32,    33,   174,   175,
     246,   248,   295,   296,   335,   375,    34,    35,    36,    37,
     234,   235,    38,   231,   232,   274,   275,    39,    40,    83,
     317,    41,   251,    42,    43,    44,    45,    77,    46
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -263
static const short yypact[] =
{
     708,   -12,  -263,    40,  -263,    16,    46,  -263,  -263,    87,
      60,  -263,  -263,  -263,   128,   191,   706,     6,    72,   154,
     158,  -263,    78,  -263,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,   161,  -263,  -263,  -263,   206,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,  -263,  -263,  -263,   214,   113,   220,
     154,   228,   231,   147,  -263,  -263,  -263,  -263,   155,   162,
     168,   175,  -263,   179,   202,   215,   217,   225,   266,  -263,
     266,   266,   -13,  -263,   180,   514,  -263,   241,   195,   203,
     173,   260,  -263,   237,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,   255,  -263,   334,  -263,   -49,  -263,    29,   335,  -263,
     245,    22,   278,   -66,  -263,  -263,   101,   337,   266,    -2,
     236,   251,   266,   266,   266,   243,   239,   242,  -263,  -263,
     343,   154,   181,  -263,   340,  -263,   266,   266,   266,   266,
     266,   266,   266,   266,   266,   266,   266,   266,   266,   266,
     266,   266,   266,   345,  -263,  -263,   139,   260,   261,   263,
    -263,  -263,  -263,  -263,  -263,  -263,   310,   273,  -263,   355,
     356,   181,    89,  -263,   256,   297,  -263,  -263,  -263,    51,
      14,  -263,  -263,  -263,   258,  -263,   148,   325,  -263,  -263,
     514,    50,  -263,  -263,   367,   362,   264,  -263,   106,   294,
     316,   392,  -263,   266,   266,  -263,   -52,  -263,  -263,  -263,
    -263,   539,   563,   588,   612,   621,   621,    65,    65,    65,
      65,    74,    74,  -263,  -263,  -263,   262,  -263,  -263,  -263,
    -263,  -263,   364,  -263,  -263,  -263,  -263,   200,   268,   328,
     372,  -263,  -263,   287,   -44,  -263,  -263,   295,   299,   394,
    -263,  -263,  -263,  -263,  -263,   110,   118,    22,   311,  -263,
    -263,   330,    58,   251,  -263,  -263,   312,  -263,  -263,   266,
    -263,   266,  -263,  -263,   465,   490,   293,   386,  -263,   396,
    -263,     5,   -24,   336,   338,  -263,    48,   355,  -263,  -263,
    -263,   320,   322,  -263,   323,  -263,   133,   341,  -263,     5,
    -263,   439,  -263,   166,  -263,   344,  -263,  -263,   442,   346,
       5,  -263,  -263,   416,   441,   347,   382,  -263,   234,   453,
     387,  -263,  -263,   134,    36,   417,  -263,  -263,   466,   372,
      15,  -263,  -263,  -263,   378,   371,   389,   295,  -263,    31,
     393,  -263,  -263,  -263,  -263,    12,    31,  -263,     5,  -263,
    -263,  -263,   488,   293,     3,    -6,     3,     3,     3,     3,
       3,     3,   459,   295,   463,   445,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,   177,   412,  -263,  -263,  -263,    18,
     183,  -263,   341,  -263,  -263,  -263,   508,  -263,   188,   415,
    -263,   436,   437,  -263,     5,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,  -263,  -263,  -263,  -263,    -4,  -263,   469,
     526,   241,  -263,  -263,  -263,    31,  -263,  -263,  -263,  -263,
       5,    19,   197,  -263,     5,   527,   238,   471,  -263,  -263,
     540,  -263,  -263,   204,   295,   443,    20,   446,  -263,   546,
    -263,  -263,  -263,   446,   461,  -263,  -263,   295,   480,   467,
    -263,  -263,  -263,     8,   546,   468,  -263,  -263,   569,  -263,
     566,   485,   207,  -263,   487,     5,   569,  -263,  -263,  -263,
    -263
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -263,  -263,  -263,  -263,   498,  -263,  -263,   474,  -263,  -263,
     -26,  -263,   363,  -263,   275,  -235,  -101,  -262,  -259,  -263,
    -263,  -263,  -263,  -263,  -263,   174,   182,   198,  -263,  -263,
    -263,  -263,   172,  -263,   164,   -64,  -263,   510,   369,  -263,
    -263,   476,  -263,  -263,   397,  -263,  -263,  -263,  -263,  -263,
    -263,  -263,  -263,   253,  -263,  -246,  -263,  -263,  -263,   395,
    -263,  -263,  -263,   307,  -263,  -263,  -263,  -263,  -263,  -263,
    -263,   368,  -263,  -263,  -263,  -263,   327,  -263,  -263,   628,
    -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263,  -263
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -208
static const short yytable[] =
{
     171,   293,   285,   383,   118,   413,   119,   120,   166,   167,
     166,   445,   313,   446,   322,   373,   447,   159,   177,   166,
     166,   121,   242,   166,   103,   167,   185,   166,   167,    47,
     168,   308,   266,   435,   314,   315,   166,   167,   160,   168,
     266,   316,   180,    48,   184,   223,   436,   180,   189,   190,
     191,    49,   160,   166,   167,   160,   240,   241,   358,   374,
     277,   364,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   214,   215,   378,
      50,    92,   382,   371,   386,   388,   390,   392,   394,   396,
     376,   122,   368,   161,   126,   196,   186,   162,   299,   384,
     169,   414,   222,   126,    53,    54,    55,    56,   308,   178,
      57,   222,   222,   448,   359,   222,   420,   127,   398,   169,
     243,   363,    51,    52,   243,   412,   127,   170,   169,   264,
     265,    53,    54,    55,    56,    93,   369,    57,    58,    86,
      62,    63,   217,    59,   166,   169,   171,   218,   300,   421,
     116,    60,   117,   320,   253,   423,   254,    94,    96,   418,
      87,   138,   139,   140,   141,   142,    61,    62,    63,  -207,
     219,    89,   140,   141,   142,   321,   148,    64,   334,    65,
     249,    97,   149,   123,    53,    54,    55,    56,   332,   180,
      57,   250,   124,   220,    78,   303,    59,   304,    68,   339,
      66,    79,    70,   217,    60,    98,    71,   179,   218,    99,
     253,    67,   258,  -207,   289,   221,   290,   100,   101,    61,
      62,    63,   291,   102,   292,    68,    69,    80,   171,    70,
      64,   104,    65,    71,   105,   171,   222,   327,   289,   328,
     357,   279,   280,   381,    81,   385,   387,   389,   391,   393,
     395,   344,   106,    66,    53,    54,    55,    56,    82,   178,
      57,   281,   107,   150,    67,   151,   152,   108,   153,    53,
      54,    55,    56,   109,   345,    57,   282,   283,    68,    69,
     110,   289,    70,   403,   111,   146,    71,   405,   143,   406,
      62,    63,   289,   147,   408,   425,   279,   280,   279,   280,
     116,   289,   117,   422,   171,    62,    63,   112,   289,   419,
     432,   456,   156,   457,   154,   116,   281,   117,   281,   165,
     113,   284,   114,   126,   346,   347,   348,   349,   350,   351,
     115,   282,   283,   282,   283,   352,   157,   158,   164,   176,
     183,   305,   187,   198,   193,   126,   127,   194,    68,   192,
     216,   227,    70,   228,   459,   229,    71,   230,   233,   236,
     239,   237,   247,    68,   252,   256,   269,    70,   127,   240,
     257,    71,   126,   271,   272,   273,   284,   276,   284,   287,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   127,   126,   288,   259,   298,
     260,   312,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   297,   302,   127,
     261,   126,   262,   309,   319,   324,   318,   325,   326,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   127,   126,   329,   333,   336,   195,
     337,   338,   342,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   127,   343,
     126,   353,   354,   255,   361,   360,   365,   366,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   127,   126,   367,   379,   372,   263,   397,
     399,   400,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   127,   404,   126,
     334,   409,   340,   410,   411,   415,   429,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   127,   126,   416,   424,   241,   341,   434,   438,
     437,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   127,   441,   126,   259,
     443,   444,   451,   450,   454,   455,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   127,   126,   458,   261,   163,   197,   278,   433,   417,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   127,   449,   126,   380,   442,
     460,   188,   301,   226,   270,   407,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     127,   126,   294,   377,    91,   323,   362,     0,     0,     0,
     126,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   127,     0,     0,     0,     0,     0,
       0,     0,     0,   127,     0,     0,     0,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   134,   135,   136,   137,   138,   139,   140,
     141,   142,     1,     0,     2,     0,    84,     3,     0,     4,
       0,     0,     5,     6,     7,     8,     0,     0,     9,     0,
       0,    10,    85,     0,     0,     0,     0,     0,     0,     0,
      11,     0,     0,     0,     0,     0,    86,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
      13,     0,    14,    82,    15,     0,    16,    87,    17,    88,
       0,     0,     0,     0,     0,    18,     0,    19,    89
};

static const short yycheck[] =
{
     101,   247,   237,     9,    68,     9,    70,    71,     5,     6,
       5,     3,   271,     5,   276,     3,     8,    66,    84,     5,
       5,    34,     8,     5,    50,     6,    28,     5,     6,    41,
       8,   266,    84,    13,    58,    59,     5,     6,   104,     8,
      84,    65,   106,     3,   108,   146,    26,   111,   112,   113,
     114,    35,   104,     5,     6,   104,     5,     6,    22,    47,
     104,   320,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   338,
      34,    75,   344,   329,   346,   347,   348,   349,   350,   351,
     336,   104,   327,    64,    29,   121,    98,    68,    40,   105,
      97,   105,    97,    29,     3,     4,     5,     6,   343,     8,
       9,    97,    97,   105,    78,    97,    97,    52,   353,    97,
     106,   106,    35,    63,   106,   384,    52,   105,    97,   193,
     194,     3,     4,     5,     6,    63,   105,     9,    10,    50,
      39,    40,     3,    15,     5,    97,   247,     8,    90,   411,
      49,    23,    51,   105,   104,   414,   106,     3,     0,   405,
      71,    96,    97,    98,    99,   100,    38,    39,    40,     3,
      31,    82,    98,    99,   100,   276,     3,    49,    12,    51,
      32,   103,     9,     3,     3,     4,     5,     6,   289,   253,
       9,    43,    12,    54,     3,   259,    15,   261,    97,   300,
      72,    10,   101,     3,    23,    44,   105,   106,     8,     3,
     104,    83,   106,    47,   104,    76,   106,     3,   105,    38,
      39,    40,   104,     3,   106,    97,    98,    36,   329,   101,
      49,     3,    51,   105,     3,   336,    97,   104,   104,   106,
     106,     3,     4,   344,    53,   346,   347,   348,   349,   350,
     351,    17,   105,    72,     3,     4,     5,     6,    67,     8,
       9,    23,   107,     3,    83,     5,     6,   105,     8,     3,
       4,     5,     6,   105,    40,     9,    38,    39,    97,    98,
     105,   104,   101,   106,   105,    90,   105,   104,    47,   106,
      39,    40,   104,    90,   106,    57,     3,     4,     3,     4,
      49,   104,    51,   106,   405,    39,    40,   105,   104,   410,
     106,   104,    75,   106,    54,    49,    23,    51,    23,    74,
     105,    83,   105,    29,    90,    91,    92,    93,    94,    95,
     105,    38,    39,    38,    39,   101,    81,     3,     3,    61,
       3,    48,   106,     3,   105,    29,    52,   105,    97,   106,
       5,    90,   101,    90,   455,    45,   105,    84,     3,     3,
      63,   105,   104,    97,    39,     3,   104,   101,    52,     5,
     106,   105,    29,   105,    46,     3,    83,    90,    83,    80,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,    52,    29,     3,   104,    69,
     106,     5,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   106,   106,    52,
     104,    29,   106,    37,    86,   105,    90,   105,   105,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,    52,    29,   105,     8,   104,   106,
       8,   105,   105,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,    52,    87,
      29,    18,    85,   106,     8,    58,    98,   106,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,    52,    29,   106,     8,   104,   106,    40,
      37,    56,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,    52,   106,    29,
      12,   106,   106,    87,    87,    56,    55,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    52,    29,    18,    18,     6,   106,   105,     3,
     104,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,    52,   106,    29,   104,
      90,   104,     3,   105,     8,    90,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    52,    29,   106,   104,    97,   122,   234,   424,   401,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,    52,   444,    29,   343,   437,
     456,   111,   253,   147,   227,   372,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
      52,    29,   247,   336,    16,   277,   319,    -1,    -1,    -1,
      29,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,    52,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    14,    -1,    16,    -1,    20,    19,    -1,    21,
      -1,    -1,    24,    25,    26,    27,    -1,    -1,    30,    -1,
      -1,    33,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    -1,
      62,    -1,    64,    67,    66,    -1,    68,    71,    70,    73,
      -1,    -1,    -1,    -1,    -1,    77,    -1,    79,    82
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    14,    16,    19,    21,    24,    25,    26,    27,    30,
      33,    42,    60,    62,    64,    66,    68,    70,    77,    79,
     109,   110,   111,   112,   113,   147,   150,   151,   154,   155,
     156,   157,   164,   165,   174,   175,   176,   177,   180,   185,
     186,   189,   191,   192,   193,   194,   196,    41,     3,    35,
      34,    35,    63,     3,     4,     5,     6,     9,    10,    15,
      23,    38,    39,    40,    49,    51,    72,    83,    97,    98,
     101,   105,   114,   115,   117,   143,   144,   195,     3,    10,
      36,    53,    67,   187,    20,    36,    50,    71,    73,    82,
     148,   187,    75,    63,     3,   118,     0,   103,    44,     3,
       3,   105,     3,   118,     3,     3,   105,   107,   105,   105,
     105,   105,   105,   105,   105,   105,    49,    51,   143,   143,
     143,    34,   104,     3,    12,   116,    29,    52,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,    47,   135,   136,    90,    90,     3,     9,
       3,     5,     6,     8,    54,   149,    75,    81,     3,    66,
     104,    64,    68,   112,     3,    74,     5,     6,     8,    97,
     105,   124,   125,   163,   166,   167,    61,    84,     8,   106,
     143,   145,   146,     3,   143,    28,    98,   106,   145,   143,
     143,   143,   106,   105,   105,   106,   118,   115,     3,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,     5,     3,     8,    31,
      54,    76,    97,   124,   152,   153,   149,    90,    90,    45,
      84,   181,   182,     3,   178,   179,     3,   105,   158,    63,
       5,     6,     8,   106,   124,   126,   168,   104,   169,    32,
      43,   190,    39,   104,   106,   106,     3,   106,   106,   104,
     106,   104,   106,   106,   143,   143,    84,   119,   120,   104,
     152,   105,    46,     3,   183,   184,    90,   104,   120,     3,
       4,    23,    38,    39,    83,   123,   159,    80,     3,   104,
     106,   104,   106,   163,   167,   170,   171,   106,    69,    40,
      90,   146,   106,   143,   143,    48,   121,   122,   123,    37,
     127,   128,     5,   126,    58,    59,    65,   188,    90,    86,
     105,   124,   125,   179,   105,   105,   105,   104,   106,   105,
     160,   161,   124,     8,    12,   172,   104,     8,   105,   124,
     106,   106,   105,    87,    17,    40,    90,    91,    92,    93,
      94,    95,   101,    18,    85,   129,   130,   106,    22,    78,
      58,     8,   184,   106,   126,    98,   106,   106,   123,   105,
     162,   163,   104,     3,    47,   173,   163,   171,   126,     8,
     122,   124,   125,     9,   105,   124,   125,   124,   125,   124,
     125,   124,   125,   124,   125,   124,   125,    40,   123,    37,
      56,   131,   132,   106,   106,   104,   106,   161,   106,   106,
      87,    87,   126,     9,   105,    56,    18,   135,   163,   124,
      97,   125,   106,   126,    18,    57,   123,   133,   134,    55,
     137,   138,   106,   133,   105,    13,    26,   104,     3,   139,
     140,   106,   134,    90,   104,     3,     5,     8,   105,   140,
     105,     3,   141,   142,     8,    90,   104,   106,   106,   124,
     142
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

  case 23:

    { pParser->PushQuery(); ;}
    break;

  case 24:

    { pParser->PushQuery(); ;}
    break;

  case 27:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 30:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 33:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 34:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 36:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 37:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 38:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 39:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 40:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 41:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 42:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 43:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 45:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 51:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 52:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 53:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 54:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 55:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 56:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 57:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 58:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 59:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 60:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 61:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 62:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 66:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 67:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 68:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 69:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 72:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 73:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 74:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 75:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 76:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 77:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 78:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 79:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 80:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 81:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 84:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 87:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 90:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 91:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 93:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 95:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 96:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 99:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 100:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 108:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 109:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
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
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 112:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 113:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 115:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 116:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 120:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 121:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 141:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 142:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 143:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 144:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 145:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 151:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 152:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 153:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 159:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 160:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 161:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 162:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 163:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 164:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 165:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 168:

    { yyval.m_iValue = 1; ;}
    break;

  case 169:

    { yyval.m_iValue = 0; ;}
    break;

  case 170:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 176:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 177:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 178:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 181:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 182:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 185:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 186:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 187:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 188:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 189:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 190:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 191:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 192:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 193:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 194:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 195:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 196:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 197:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 199:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 200:

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

  case 201:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 204:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 206:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 210:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 211:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 214:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 215:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 218:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 219:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 220:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 221:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 222:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 229:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 230:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 238:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 239:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 240:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 241:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 242:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 243:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 244:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 246:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 247:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
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

