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
     TOK_AGENT = 267,
     TOK_AS = 268,
     TOK_ASC = 269,
     TOK_ATTACH = 270,
     TOK_AVG = 271,
     TOK_BEGIN = 272,
     TOK_BETWEEN = 273,
     TOK_BY = 274,
     TOK_CALL = 275,
     TOK_CHARACTER = 276,
     TOK_COLLATION = 277,
     TOK_COMMIT = 278,
     TOK_COMMITTED = 279,
     TOK_COUNT = 280,
     TOK_CREATE = 281,
     TOK_DELETE = 282,
     TOK_DESC = 283,
     TOK_DESCRIBE = 284,
     TOK_DISTINCT = 285,
     TOK_DIV = 286,
     TOK_DROP = 287,
     TOK_FALSE = 288,
     TOK_FLOAT = 289,
     TOK_FLUSH = 290,
     TOK_FROM = 291,
     TOK_FUNCTION = 292,
     TOK_GLOBAL = 293,
     TOK_GROUP = 294,
     TOK_GROUPBY = 295,
     TOK_GROUP_CONCAT = 296,
     TOK_ID = 297,
     TOK_IN = 298,
     TOK_INDEX = 299,
     TOK_INSERT = 300,
     TOK_INT = 301,
     TOK_INTO = 302,
     TOK_ISOLATION = 303,
     TOK_LEVEL = 304,
     TOK_LIMIT = 305,
     TOK_MATCH = 306,
     TOK_MAX = 307,
     TOK_META = 308,
     TOK_MIN = 309,
     TOK_MOD = 310,
     TOK_NAMES = 311,
     TOK_NULL = 312,
     TOK_OPTION = 313,
     TOK_ORDER = 314,
     TOK_OPTIMIZE = 315,
     TOK_RAND = 316,
     TOK_READ = 317,
     TOK_REPEATABLE = 318,
     TOK_REPLACE = 319,
     TOK_RETURNS = 320,
     TOK_ROLLBACK = 321,
     TOK_RTINDEX = 322,
     TOK_SELECT = 323,
     TOK_SERIALIZABLE = 324,
     TOK_SET = 325,
     TOK_SESSION = 326,
     TOK_SHOW = 327,
     TOK_SONAME = 328,
     TOK_START = 329,
     TOK_STATUS = 330,
     TOK_SUM = 331,
     TOK_TABLES = 332,
     TOK_TO = 333,
     TOK_TRANSACTION = 334,
     TOK_TRUE = 335,
     TOK_TRUNCATE = 336,
     TOK_UNCOMMITTED = 337,
     TOK_UPDATE = 338,
     TOK_VALUES = 339,
     TOK_VARIABLES = 340,
     TOK_WARNINGS = 341,
     TOK_WEIGHT = 342,
     TOK_WHERE = 343,
     TOK_WITHIN = 344,
     TOK_OR = 345,
     TOK_AND = 346,
     TOK_NE = 347,
     TOK_GTE = 348,
     TOK_LTE = 349,
     TOK_NOT = 350,
     TOK_NEG = 351
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
#define TOK_AGENT 267
#define TOK_AS 268
#define TOK_ASC 269
#define TOK_ATTACH 270
#define TOK_AVG 271
#define TOK_BEGIN 272
#define TOK_BETWEEN 273
#define TOK_BY 274
#define TOK_CALL 275
#define TOK_CHARACTER 276
#define TOK_COLLATION 277
#define TOK_COMMIT 278
#define TOK_COMMITTED 279
#define TOK_COUNT 280
#define TOK_CREATE 281
#define TOK_DELETE 282
#define TOK_DESC 283
#define TOK_DESCRIBE 284
#define TOK_DISTINCT 285
#define TOK_DIV 286
#define TOK_DROP 287
#define TOK_FALSE 288
#define TOK_FLOAT 289
#define TOK_FLUSH 290
#define TOK_FROM 291
#define TOK_FUNCTION 292
#define TOK_GLOBAL 293
#define TOK_GROUP 294
#define TOK_GROUPBY 295
#define TOK_GROUP_CONCAT 296
#define TOK_ID 297
#define TOK_IN 298
#define TOK_INDEX 299
#define TOK_INSERT 300
#define TOK_INT 301
#define TOK_INTO 302
#define TOK_ISOLATION 303
#define TOK_LEVEL 304
#define TOK_LIMIT 305
#define TOK_MATCH 306
#define TOK_MAX 307
#define TOK_META 308
#define TOK_MIN 309
#define TOK_MOD 310
#define TOK_NAMES 311
#define TOK_NULL 312
#define TOK_OPTION 313
#define TOK_ORDER 314
#define TOK_OPTIMIZE 315
#define TOK_RAND 316
#define TOK_READ 317
#define TOK_REPEATABLE 318
#define TOK_REPLACE 319
#define TOK_RETURNS 320
#define TOK_ROLLBACK 321
#define TOK_RTINDEX 322
#define TOK_SELECT 323
#define TOK_SERIALIZABLE 324
#define TOK_SET 325
#define TOK_SESSION 326
#define TOK_SHOW 327
#define TOK_SONAME 328
#define TOK_START 329
#define TOK_STATUS 330
#define TOK_SUM 331
#define TOK_TABLES 332
#define TOK_TO 333
#define TOK_TRANSACTION 334
#define TOK_TRUE 335
#define TOK_TRUNCATE 336
#define TOK_UNCOMMITTED 337
#define TOK_UPDATE 338
#define TOK_VALUES 339
#define TOK_VARIABLES 340
#define TOK_WARNINGS 341
#define TOK_WEIGHT 342
#define TOK_WHERE 343
#define TOK_WITHIN 344
#define TOK_OR 345
#define TOK_AND 346
#define TOK_NE 347
#define TOK_GTE 348
#define TOK_LTE 349
#define TOK_NOT 350
#define TOK_NEG 351




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
#define YYFINAL  103
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   863

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  112
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  91
/* YYNRULES -- Number of rules. */
#define YYNRULES  253
/* YYNRULES -- Number of states. */
#define YYNSTATES  475

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   104,    93,     2,
     109,   110,   102,   100,   108,   101,   111,   103,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   107,
      96,    94,    97,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    92,     2,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    95,    98,    99,
     105,   106
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
     497,   501,   503,   505,   508,   510,   512,   514,   517,   519,
     521,   523,   525,   527,   532,   537,   542,   546,   551,   559,
     565,   567,   569,   571,   573,   575,   577,   579,   581,   583,
     586,   593,   595,   597,   598,   602,   604,   608,   610,   614,
     618,   620,   624,   626,   628,   630,   634,   637,   645,   655,
     662,   664,   668,   670,   674,   676,   680,   681,   684,   686,
     690,   694,   695,   697,   699,   701,   704,   706,   708,   711,
     718,   720,   724,   728,   732,   738,   743,   748,   749,   751,
     754,   756,   760,   764,   767,   771,   778,   779,   781,   783,
     786,   789,   792,   794,   802,   804,   806,   810,   817,   821,
     825,   827,   831,   835
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     113,     0,    -1,   114,    -1,   115,    -1,   115,   107,    -1,
     160,    -1,   168,    -1,   154,    -1,   155,    -1,   158,    -1,
     169,    -1,   178,    -1,   180,    -1,   181,    -1,   184,    -1,
     189,    -1,   190,    -1,   194,    -1,   196,    -1,   197,    -1,
     198,    -1,   191,    -1,   199,    -1,   201,    -1,   202,    -1,
     116,    -1,   115,   107,   116,    -1,   117,    -1,   151,    -1,
      68,   118,    36,   122,   123,   131,   133,   135,   139,   141,
      -1,   119,    -1,   118,   108,   119,    -1,   102,    -1,   121,
     120,    -1,    -1,     3,    -1,    13,     3,    -1,   147,    -1,
      16,   109,   147,   110,    -1,    52,   109,   147,   110,    -1,
      54,   109,   147,   110,    -1,    76,   109,   147,   110,    -1,
      41,   109,   147,   110,    -1,    25,   109,   102,   110,    -1,
      87,   109,   110,    -1,    40,   109,   110,    -1,    25,   109,
      30,     3,   110,    -1,     3,    -1,   122,   108,     3,    -1,
      -1,   124,    -1,    88,   125,    -1,   126,    -1,   125,    91,
     126,    -1,    51,   109,     8,   110,    -1,   127,    94,   128,
      -1,   127,    95,   128,    -1,   127,    43,   109,   130,   110,
      -1,   127,   105,    43,   109,   130,   110,    -1,   127,    43,
       9,    -1,   127,   105,    43,     9,    -1,   127,    18,   128,
      91,   128,    -1,   127,    97,   128,    -1,   127,    96,   128,
      -1,   127,    98,   128,    -1,   127,    99,   128,    -1,   127,
      94,   129,    -1,   127,    95,   129,    -1,   127,    97,   129,
      -1,   127,    96,   129,    -1,   127,    18,   129,    91,   129,
      -1,   127,    98,   129,    -1,   127,    99,   129,    -1,     3,
      -1,     4,    -1,    25,   109,   102,   110,    -1,    40,   109,
     110,    -1,    87,   109,   110,    -1,    42,    -1,     5,    -1,
     101,     5,    -1,     6,    -1,   101,     6,    -1,   128,    -1,
     130,   108,   128,    -1,    -1,   132,    -1,    39,    19,   127,
      -1,    -1,   134,    -1,    89,    39,    59,    19,   137,    -1,
      -1,   136,    -1,    59,    19,   137,    -1,    59,    19,    61,
     109,   110,    -1,   138,    -1,   137,   108,   138,    -1,   127,
      -1,   127,    14,    -1,   127,    28,    -1,    -1,   140,    -1,
      50,     5,    -1,    50,     5,   108,     5,    -1,    -1,   142,
      -1,    58,   143,    -1,   144,    -1,   143,   108,   144,    -1,
       3,    94,     3,    -1,     3,    94,     5,    -1,     3,    94,
     109,   145,   110,    -1,     3,    94,     3,   109,     8,   110,
      -1,     3,    94,     8,    -1,   146,    -1,   145,   108,   146,
      -1,     3,    94,   128,    -1,     3,    -1,     4,    -1,    42,
      -1,     5,    -1,     6,    -1,     9,    -1,   101,   147,    -1,
     105,   147,    -1,   147,   100,   147,    -1,   147,   101,   147,
      -1,   147,   102,   147,    -1,   147,   103,   147,    -1,   147,
      96,   147,    -1,   147,    97,   147,    -1,   147,    93,   147,
      -1,   147,    92,   147,    -1,   147,   104,   147,    -1,   147,
      31,   147,    -1,   147,    55,   147,    -1,   147,    99,   147,
      -1,   147,    98,   147,    -1,   147,    94,   147,    -1,   147,
      95,   147,    -1,   147,    91,   147,    -1,   147,    90,   147,
      -1,   109,   147,   110,    -1,   148,    -1,     3,   109,   149,
     110,    -1,    43,   109,   149,   110,    -1,     3,   109,   110,
      -1,    54,   109,   147,   108,   147,   110,    -1,    52,   109,
     147,   108,   147,   110,    -1,   150,    -1,   149,   108,   150,
      -1,   147,    -1,     8,    -1,    72,   152,    -1,    86,    -1,
      75,    -1,    53,    -1,    12,    75,    -1,     3,    -1,    57,
      -1,     8,    -1,     5,    -1,     6,    -1,    70,     3,    94,
     157,    -1,    70,     3,    94,   156,    -1,    70,     3,    94,
      57,    -1,    70,    56,   153,    -1,    70,    10,    94,   153,
      -1,    70,    38,     9,    94,   109,   130,   110,    -1,    70,
      38,     3,    94,   156,    -1,     3,    -1,     8,    -1,    80,
      -1,    33,    -1,   128,    -1,    23,    -1,    66,    -1,   159,
      -1,    17,    -1,    74,    79,    -1,   161,    47,     3,   162,
      84,   164,    -1,    45,    -1,    64,    -1,    -1,   109,   163,
     110,    -1,   127,    -1,   163,   108,   127,    -1,   165,    -1,
     164,   108,   165,    -1,   109,   166,   110,    -1,   167,    -1,
     166,   108,   167,    -1,   128,    -1,   129,    -1,     8,    -1,
     109,   130,   110,    -1,   109,   110,    -1,    27,    36,   122,
      88,    42,    94,   128,    -1,    27,    36,   122,    88,    42,
      43,   109,   130,   110,    -1,    20,     3,   109,   170,   173,
     110,    -1,   171,    -1,   170,   108,   171,    -1,   167,    -1,
     109,   172,   110,    -1,     8,    -1,   172,   108,     8,    -1,
      -1,   108,   174,    -1,   175,    -1,   174,   108,   175,    -1,
     167,   176,   177,    -1,    -1,    13,    -1,     3,    -1,    50,
      -1,   179,     3,    -1,    29,    -1,    28,    -1,    72,    77,
      -1,    83,   122,    70,   182,   124,   141,    -1,   183,    -1,
     182,   108,   183,    -1,     3,    94,   128,    -1,     3,    94,
     129,    -1,     3,    94,   109,   130,   110,    -1,     3,    94,
     109,   110,    -1,    72,   192,    85,   185,    -1,    -1,   186,
      -1,    88,   187,    -1,   188,    -1,   187,    90,   188,    -1,
       3,    94,     8,    -1,    72,    22,    -1,    72,    21,    70,
      -1,    70,   192,    79,    48,    49,   193,    -1,    -1,    38,
      -1,    71,    -1,    62,    82,    -1,    62,    24,    -1,    63,
      62,    -1,    69,    -1,    26,    37,     3,    65,   195,    73,
       8,    -1,    46,    -1,    34,    -1,    32,    37,     3,    -1,
      15,    44,     3,    78,    67,     3,    -1,    35,    67,     3,
      -1,    68,   200,   139,    -1,    10,    -1,    10,   111,     3,
      -1,    81,    67,     3,    -1,    60,    44,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   125,   125,   126,   127,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   156,   157,   161,   162,   166,
     181,   182,   186,   187,   190,   192,   193,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   210,   211,   214,
     216,   220,   224,   225,   229,   234,   241,   249,   257,   266,
     271,   276,   281,   286,   291,   296,   301,   302,   303,   304,
     309,   314,   319,   327,   328,   333,   339,   345,   351,   360,
     361,   365,   366,   370,   376,   382,   384,   388,   395,   397,
     401,   407,   409,   413,   417,   424,   425,   429,   430,   431,
     434,   436,   440,   445,   452,   454,   458,   462,   463,   467,
     472,   477,   483,   488,   496,   501,   508,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   530,
     531,   532,   533,   534,   535,   536,   537,   538,   539,   540,
     541,   542,   543,   544,   548,   549,   550,   551,   552,   556,
     557,   561,   562,   568,   572,   573,   574,   575,   581,   582,
     583,   584,   585,   589,   594,   599,   604,   605,   609,   614,
     622,   623,   627,   628,   629,   643,   644,   645,   649,   650,
     656,   664,   665,   668,   670,   674,   675,   679,   680,   684,
     688,   689,   693,   694,   695,   696,   697,   703,   711,   725,
     733,   737,   744,   745,   752,   762,   768,   770,   774,   779,
     783,   790,   792,   796,   797,   803,   811,   812,   818,   824,
     832,   833,   837,   841,   845,   849,   859,   865,   866,   870,
     874,   875,   879,   883,   890,   897,   903,   904,   905,   909,
     910,   911,   912,   918,   929,   930,   934,   945,   957,   968,
     976,   977,   986,   997
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
  "TOK_AGENT", "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", "TOK_BEGIN", 
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
     345,   346,   124,    38,    61,   347,    60,    62,   348,   349,
      43,    45,    42,    47,    37,   350,   351,    59,    44,    40,
      41,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   112,   113,   113,   113,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   115,   115,   116,   116,   117,
     118,   118,   119,   119,   120,   120,   120,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   122,   122,   123,
     123,   124,   125,   125,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   127,   127,   127,   127,   127,   127,   128,
     128,   129,   129,   130,   130,   131,   131,   132,   133,   133,
     134,   135,   135,   136,   136,   137,   137,   138,   138,   138,
     139,   139,   140,   140,   141,   141,   142,   143,   143,   144,
     144,   144,   144,   144,   145,   145,   146,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   148,   148,   148,   148,   148,   149,
     149,   150,   150,   151,   152,   152,   152,   152,   153,   153,
     153,   153,   153,   154,   154,   154,   154,   154,   155,   155,
     156,   156,   157,   157,   157,   158,   158,   158,   159,   159,
     160,   161,   161,   162,   162,   163,   163,   164,   164,   165,
     166,   166,   167,   167,   167,   167,   167,   168,   168,   169,
     170,   170,   171,   171,   172,   172,   173,   173,   174,   174,
     175,   176,   176,   177,   177,   178,   179,   179,   180,   181,
     182,   182,   183,   183,   183,   183,   184,   185,   185,   186,
     187,   187,   188,   189,   190,   191,   192,   192,   192,   193,
     193,   193,   193,   194,   195,   195,   196,   197,   198,   199,
     200,   200,   201,   202
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
       3,     1,     1,     2,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     4,     4,     4,     3,     4,     7,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       6,     1,     1,     0,     3,     1,     3,     1,     3,     3,
       1,     3,     1,     1,     1,     3,     2,     7,     9,     6,
       1,     3,     1,     3,     1,     3,     0,     2,     1,     3,
       3,     0,     1,     1,     1,     2,     1,     1,     2,     6,
       1,     3,     3,     3,     5,     4,     4,     0,     1,     2,
       1,     3,     3,     2,     3,     6,     0,     1,     1,     2,
       2,     2,     1,     7,     1,     1,     3,     6,     3,     3,
       1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   178,     0,   175,     0,     0,   217,   216,     0,
       0,   181,     0,   182,   176,     0,   236,   236,     0,     0,
       0,     0,     2,     3,    25,    27,    28,     7,     8,     9,
     177,     5,     0,     6,    10,    11,     0,    12,    13,    14,
      15,    16,    21,    17,    18,    19,    20,    22,    23,    24,
       0,     0,     0,     0,     0,     0,     0,   117,   118,   120,
     121,   122,   250,     0,     0,     0,     0,   119,     0,     0,
       0,     0,     0,     0,    32,     0,     0,     0,    30,    34,
      37,   143,   100,     0,     0,   237,     0,   238,     0,     0,
       0,   233,   237,   156,   155,   218,   154,   153,     0,   179,
       0,    47,     0,     1,     4,     0,   215,     0,     0,     0,
       0,   246,   248,   253,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   123,   124,     0,
       0,     0,    35,     0,    33,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   249,   101,     0,     0,     0,     0,   158,
     161,   162,   160,   159,   166,     0,   157,   234,   227,   252,
       0,     0,     0,     0,    26,   183,     0,    79,    81,   194,
       0,     0,   192,   193,   202,   206,   200,     0,     0,   152,
     146,   151,     0,   149,   251,     0,     0,     0,    45,     0,
       0,     0,     0,     0,    44,     0,     0,   142,    49,    31,
      36,   134,   135,   141,   140,   132,   131,   138,   139,   129,
     130,   137,   136,   125,   126,   127,   128,   133,   102,   170,
     171,   173,   165,   172,     0,   174,   164,   163,   167,     0,
       0,     0,     0,   226,   228,     0,     0,   220,    48,     0,
       0,     0,    80,    82,   204,   196,    83,     0,     0,     0,
       0,   245,   244,     0,     0,     0,   144,    38,     0,    43,
      42,   145,     0,    39,     0,    40,    41,     0,     0,     0,
      85,    50,     0,   169,     0,     0,     0,   229,   230,     0,
       0,   104,    73,    74,     0,     0,    78,     0,   185,     0,
       0,   247,     0,   195,     0,   203,   202,   201,   207,   208,
     199,     0,     0,     0,   150,    46,     0,     0,     0,    51,
      52,     0,     0,    88,    86,   103,     0,     0,     0,   242,
     235,     0,     0,     0,   222,   223,   221,     0,   219,   105,
       0,     0,     0,     0,   184,     0,   180,   187,    84,   205,
     212,     0,     0,   243,     0,   197,   148,   147,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    91,    89,   168,   240,   239,   241,   232,   231,   225,
       0,     0,   106,   107,     0,    76,    77,   186,     0,     0,
     190,     0,   213,   214,   210,   211,   209,     0,     0,    53,
       0,     0,    59,     0,    55,    66,    56,    67,    63,    69,
      62,    68,    64,    71,    65,    72,     0,    87,     0,     0,
     100,    92,   224,     0,     0,    75,     0,   189,   188,   198,
      54,     0,     0,     0,    60,     0,     0,     0,   104,   109,
     110,   113,     0,   108,   191,    61,     0,    70,    57,     0,
       0,     0,    97,    93,    95,    29,     0,     0,     0,   114,
      58,    90,     0,    98,    99,     0,     0,     0,     0,   111,
      94,    96,   112,   116,   115
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,    77,    78,   134,    79,
     102,   280,   281,   319,   320,   452,   256,   183,   257,   323,
     324,   371,   372,   420,   421,   453,   454,   153,   154,   338,
     339,   382,   383,   458,   459,    80,    81,   192,   193,    26,
      97,   164,    27,    28,   236,   237,    29,    30,    31,    32,
     250,   299,   346,   347,   389,   184,    33,    34,   185,   186,
     258,   260,   308,   309,   351,   394,    35,    36,    37,    38,
     246,   247,    39,   243,   244,   287,   288,    40,    41,    42,
      88,   330,    43,   263,    44,    45,    46,    47,    82,    48,
      49
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -275
static const short yypact[] =
{
     780,    -7,  -275,    39,  -275,     8,    94,  -275,  -275,   133,
      95,  -275,   117,  -275,  -275,   173,   129,   627,    86,   106,
     171,   187,  -275,    99,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,   145,  -275,  -275,  -275,   198,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
     229,   100,   236,   171,   239,   243,   250,   172,  -275,  -275,
    -275,  -275,   169,   178,   179,   188,   191,  -275,   199,   202,
     213,   215,   216,   267,  -275,   267,   267,    21,  -275,   156,
     560,  -275,   246,   221,   238,   181,   140,  -275,   254,   261,
     271,  -275,  -275,  -275,  -275,  -275,  -275,  -275,   257,  -275,
     340,  -275,    29,  -275,    -8,   341,  -275,   268,    12,   280,
     -53,  -275,  -275,  -275,    98,   344,   267,    -4,   240,   267,
     286,   267,   267,   267,   241,   245,   248,  -275,  -275,   358,
     171,   225,  -275,   345,  -275,   267,   267,   267,   267,   267,
     267,   267,   267,   267,   267,   267,   267,   267,   267,   267,
     267,   267,   347,  -275,  -275,   139,   140,   255,   259,  -275,
    -275,  -275,  -275,  -275,  -275,   307,  -275,  -275,   270,  -275,
     359,   364,   225,    85,  -275,   252,   302,  -275,  -275,  -275,
      57,    -1,  -275,  -275,  -275,   263,  -275,    -5,   328,  -275,
    -275,   560,    78,  -275,  -275,   383,   370,   264,  -275,   409,
      83,   308,   329,   434,  -275,   267,   267,  -275,   -52,  -275,
    -275,  -275,  -275,   590,   640,   665,   676,   690,   690,    53,
      53,    53,    53,   262,   262,  -275,  -275,  -275,   269,  -275,
    -275,  -275,  -275,  -275,   373,  -275,  -275,  -275,  -275,    30,
     266,   330,   377,  -275,  -275,   287,   -49,  -275,  -275,   244,
     299,   382,  -275,  -275,  -275,  -275,  -275,   102,   112,    12,
     276,  -275,  -275,   315,    -9,   286,  -275,  -275,   282,  -275,
    -275,  -275,   267,  -275,   267,  -275,  -275,   509,   534,   124,
     351,  -275,   388,  -275,     4,   155,   300,   306,  -275,    25,
     359,   339,  -275,  -275,   325,   326,  -275,   327,  -275,   113,
     332,  -275,     4,  -275,   407,  -275,    45,  -275,   309,  -275,
    -275,   435,   333,     4,  -275,  -275,   459,   484,   335,   354,
    -275,   208,   427,   374,  -275,  -275,   125,     1,   385,  -275,
    -275,   458,   377,     9,  -275,  -275,  -275,   464,  -275,  -275,
     367,   360,   361,   244,  -275,    16,   380,  -275,  -275,  -275,
    -275,   130,    16,  -275,     4,  -275,  -275,  -275,   483,   124,
      22,     2,    22,    22,    22,    22,    22,    22,   429,   244,
     453,   436,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
     128,   400,   389,  -275,   386,  -275,  -275,  -275,    14,   135,
    -275,   332,  -275,  -275,  -275,   485,  -275,   153,   406,  -275,
     426,   430,  -275,     4,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,     7,  -275,   461,   499,
     246,  -275,  -275,     5,   464,  -275,    16,  -275,  -275,  -275,
    -275,     4,    17,   154,  -275,     4,   503,   295,   339,   414,
    -275,  -275,   538,  -275,  -275,  -275,   536,  -275,  -275,   175,
     244,   437,    26,   439,  -275,  -275,   535,   451,   206,  -275,
    -275,   439,   438,  -275,  -275,   244,   456,     4,   538,  -275,
    -275,  -275,  -275,  -275,  -275
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -275,  -275,  -275,  -275,   463,  -275,  -275,   440,  -275,  -275,
     -10,  -275,   322,  -275,   211,  -247,  -108,  -274,  -272,  -275,
    -275,  -275,  -275,  -275,  -275,   122,   108,   170,  -275,   157,
    -275,  -275,   168,  -275,   146,   -70,  -275,   473,   331,  -275,
    -275,   441,  -275,  -275,   379,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,   207,  -275,  -258,  -275,  -275,  -275,   357,
    -275,  -275,  -275,   288,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,   353,  -275,  -275,  -275,  -275,   290,  -275,  -275,  -275,
     602,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -212
static const short yytable[] =
{
     182,   306,   298,   127,   177,   128,   129,   254,   439,   177,
     440,   402,   326,   441,   177,   335,   434,   177,   178,   177,
     179,   177,   178,   178,   179,   374,   196,   177,   178,   261,
     177,   178,   321,   229,   312,   188,   279,    50,   230,   279,
     463,   262,    51,   110,   191,    52,   195,   235,  -211,   199,
     191,   201,   202,   203,   464,   171,   171,   130,   350,   290,
     172,   380,   252,   253,   173,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   397,   375,   135,   313,   401,   390,   405,   407,
     409,   411,   413,   415,   395,  -211,   387,    89,   197,   170,
     234,    57,    58,    59,    60,   234,   189,    61,   136,   255,
     234,   403,   321,   180,   442,   234,   435,   180,   446,   379,
     208,   181,   417,   180,   255,   388,   180,   292,   293,   131,
      53,   433,    83,   392,   333,   277,   278,   171,    93,    84,
      67,    68,   229,   159,   177,   160,   161,   230,   162,   294,
     125,   182,   126,   147,   148,   149,   150,   151,   447,   132,
      94,    56,    55,   449,   295,    99,   296,    85,   444,   133,
      54,    96,   231,   100,   101,   318,    57,    58,    59,    60,
     393,   334,    61,    62,   157,    86,   265,   103,   266,    63,
     158,   265,   105,   271,   348,   191,   232,   163,    64,    73,
      87,   106,   316,    75,   317,   355,   104,    76,   190,   108,
     302,   297,   303,    65,    66,    67,    68,   327,   328,   233,
     304,   343,   305,   344,   329,    69,   360,    70,    57,    58,
      59,    60,   107,   302,    61,   373,   302,   182,   422,   109,
     234,    63,   111,   426,   182,   427,   112,   292,   293,    71,
      64,   361,   400,   113,   404,   406,   408,   410,   412,   414,
      72,   302,   302,   429,   448,    65,    66,    67,    68,   294,
      57,    58,    59,    60,    73,    74,    61,    69,    75,    70,
     115,   114,    76,   302,   295,   460,   296,   116,   117,    57,
      58,    59,    60,   135,   189,    61,   152,   118,   292,   293,
     119,    71,   362,   363,   364,   365,   366,   367,   120,    67,
      68,   121,    72,   368,   468,   155,   469,   136,   182,   125,
     294,   126,   122,   445,   123,   124,    73,    74,    67,    68,
      75,   297,   156,   165,    76,   295,   166,   296,   125,   135,
     126,   167,   168,   169,   175,   187,   176,   194,   210,   239,
     198,   204,   228,   240,   205,   241,   451,   206,   242,   473,
     135,   249,   245,   136,   149,   150,   151,   248,    73,   251,
     264,   259,    75,   268,   269,   284,    76,   282,   252,   285,
     286,   289,   297,   300,   136,   301,   310,    73,   311,   135,
     322,    75,   315,   325,   331,    76,   332,   337,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   136,   135,   349,   272,   352,   273,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   340,   341,   342,   274,   136,   275,
     135,   345,   354,   353,   358,   359,   369,   376,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   370,   136,   135,   377,   381,   207,   384,
     385,   386,   416,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   391,   136,
     135,   398,   418,   267,   423,   419,   425,   424,   350,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   136,   135,   430,   431,   437,   270,
     436,   432,   450,   456,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   136,
     135,   457,   253,   466,   276,   467,   462,   465,   470,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   136,   135,   472,   174,   291,   356,
     399,   209,   461,   471,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   136,
     438,   135,   443,   200,   357,   455,   314,   238,   428,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   474,   136,   307,   272,   283,    98,
       0,   135,   378,     0,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,    89,
     396,     0,   274,   336,     0,   136,     0,     0,    90,    91,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,    92,     0,     0,     0,     0,
       0,   135,     0,     0,     0,     0,     0,     0,     0,     0,
      93,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   136,   135,     0,    87,     0,
       0,     0,    94,     0,    95,     0,     0,   135,     0,     0,
       0,     0,     0,    96,     0,     0,     0,     0,     0,     0,
     136,   135,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   136,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   136,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,     0,     0,     0,     0,     0,   143,   144,   145,   146,
     147,   148,   149,   150,   151,     1,     0,     2,     0,     0,
       3,     0,     0,     4,     0,     0,     5,     6,     7,     8,
       0,     0,     9,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,     0,     0,    13,     0,    14,     0,    15,     0,
      16,     0,    17,     0,    18,     0,     0,     0,     0,     0,
       0,    19,     0,    20
};

static const short yycheck[] =
{
     108,   259,   249,    73,     5,    75,    76,     8,     3,     5,
       5,     9,   284,     8,     5,   289,     9,     5,     6,     5,
       8,     5,     6,     6,     8,    24,    30,     5,     6,    34,
       5,     6,   279,     3,    43,    88,    88,    44,     8,    88,
      14,    46,     3,    53,   114,    37,   116,   155,     3,   119,
     120,   121,   122,   123,    28,   108,   108,    36,    13,   108,
      68,   333,     5,     6,    72,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   354,    82,    31,    94,   360,   345,   362,   363,
     364,   365,   366,   367,   352,    50,   343,    12,   102,    70,
     101,     3,     4,     5,     6,   101,     8,     9,    55,   110,
     101,   109,   359,   101,   109,   101,   109,   101,   101,   110,
     130,   109,   369,   101,   110,   109,   101,     3,     4,   108,
      36,   403,     3,     3,   109,   205,   206,   108,    53,    10,
      42,    43,     3,     3,     5,     5,     6,     8,     8,    25,
      52,   259,    54,   100,   101,   102,   103,   104,   432,     3,
      75,    44,    67,   435,    40,    79,    42,    38,   426,    13,
      37,    86,    33,    67,     3,    51,     3,     4,     5,     6,
      50,   289,     9,    10,     3,    56,   108,     0,   110,    16,
       9,   108,    47,   110,   302,   265,    57,    57,    25,   101,
      71,     3,   272,   105,   274,   313,   107,   109,   110,   109,
     108,    87,   110,    40,    41,    42,    43,    62,    63,    80,
     108,   108,   110,   110,    69,    52,    18,    54,     3,     4,
       5,     6,     3,   108,     9,   110,   108,   345,   110,     3,
     101,    16,     3,   108,   352,   110,     3,     3,     4,    76,
      25,    43,   360,     3,   362,   363,   364,   365,   366,   367,
      87,   108,   108,   110,   110,    40,    41,    42,    43,    25,
       3,     4,     5,     6,   101,   102,     9,    52,   105,    54,
     111,   109,   109,   108,    40,   110,    42,   109,   109,     3,
       4,     5,     6,    31,     8,     9,    50,   109,     3,     4,
     109,    76,    94,    95,    96,    97,    98,    99,   109,    42,
      43,   109,    87,   105,   108,    94,   110,    55,   426,    52,
      25,    54,   109,   431,   109,   109,   101,   102,    42,    43,
     105,    87,    94,    79,   109,    40,    75,    42,    52,    31,
      54,    70,    85,     3,     3,    65,    78,     3,     3,    94,
     110,   110,     5,    94,   109,    48,    61,   109,    88,   467,
      31,   109,     3,    55,   102,   103,   104,     3,   101,    67,
      42,   108,   105,     3,   110,   109,   109,   108,     5,    49,
       3,    94,    87,    84,    55,     3,   110,   101,    73,    31,
      39,   105,   110,     5,    94,   109,    90,    58,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,    55,    31,     8,   108,   108,   110,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   109,   109,   109,   108,    55,   110,
      31,   109,   109,     8,   109,    91,    19,    62,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,    89,    55,    31,     8,     3,   110,   102,
     110,   110,    43,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   108,    55,
      31,     8,    39,   110,    94,    59,   110,   108,    13,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    55,    31,   110,    91,    19,   110,
      59,    91,    19,   109,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    55,
      31,     3,     6,     8,   110,    94,   109,   108,   110,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    55,    31,   110,   104,   246,   110,
     359,   131,   450,   465,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    55,
     420,    31,   424,   120,   110,   438,   265,   156,   391,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   468,    55,   259,   108,   239,    17,
      -1,    31,   332,    -1,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    12,
     352,    -1,   108,   290,    -1,    55,    -1,    -1,    21,    22,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    38,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    55,    31,    -1,    71,    -1,
      -1,    -1,    75,    -1,    77,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      55,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    55,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    -1,    -1,    -1,    -1,    -1,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    15,    -1,    17,    -1,    -1,
      20,    -1,    -1,    23,    -1,    -1,    26,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      60,    -1,    -1,    -1,    64,    -1,    66,    -1,    68,    -1,
      70,    -1,    72,    -1,    74,    -1,    -1,    -1,    -1,    -1,
      -1,    81,    -1,    83
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    60,    64,    66,    68,    70,    72,    74,    81,
      83,   113,   114,   115,   116,   117,   151,   154,   155,   158,
     159,   160,   161,   168,   169,   178,   179,   180,   181,   184,
     189,   190,   191,   194,   196,   197,   198,   199,   201,   202,
      44,     3,    37,    36,    37,    67,    44,     3,     4,     5,
       6,     9,    10,    16,    25,    40,    41,    42,    43,    52,
      54,    76,    87,   101,   102,   105,   109,   118,   119,   121,
     147,   148,   200,     3,    10,    38,    56,    71,   192,    12,
      21,    22,    38,    53,    75,    77,    86,   152,   192,    79,
      67,     3,   122,     0,   107,    47,     3,     3,   109,     3,
     122,     3,     3,     3,   109,   111,   109,   109,   109,   109,
     109,   109,   109,   109,   109,    52,    54,   147,   147,   147,
      36,   108,     3,    13,   120,    31,    55,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,    50,   139,   140,    94,    94,     3,     9,     3,
       5,     6,     8,    57,   153,    79,    75,    70,    85,     3,
      70,   108,    68,    72,   116,     3,    78,     5,     6,     8,
     101,   109,   128,   129,   167,   170,   171,    65,    88,     8,
     110,   147,   149,   150,     3,   147,    30,   102,   110,   147,
     149,   147,   147,   147,   110,   109,   109,   110,   122,   119,
       3,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,     5,     3,
       8,    33,    57,    80,   101,   128,   156,   157,   153,    94,
      94,    48,    88,   185,   186,     3,   182,   183,     3,   109,
     162,    67,     5,     6,     8,   110,   128,   130,   172,   108,
     173,    34,    46,   195,    42,   108,   110,   110,     3,   110,
     110,   110,   108,   110,   108,   110,   110,   147,   147,    88,
     123,   124,   108,   156,   109,    49,     3,   187,   188,    94,
     108,   124,     3,     4,    25,    40,    42,    87,   127,   163,
      84,     3,   108,   110,   108,   110,   167,   171,   174,   175,
     110,    73,    43,    94,   150,   110,   147,   147,    51,   125,
     126,   127,    39,   131,   132,     5,   130,    62,    63,    69,
     193,    94,    90,   109,   128,   129,   183,    58,   141,   142,
     109,   109,   109,   108,   110,   109,   164,   165,   128,     8,
      13,   176,   108,     8,   109,   128,   110,   110,   109,    91,
      18,    43,    94,    95,    96,    97,    98,    99,   105,    19,
      89,   133,   134,   110,    24,    82,    62,     8,   188,   110,
     130,     3,   143,   144,   102,   110,   110,   127,   109,   166,
     167,   108,     3,    50,   177,   167,   175,   130,     8,   126,
     128,   129,     9,   109,   128,   129,   128,   129,   128,   129,
     128,   129,   128,   129,   128,   129,    43,   127,    39,    59,
     135,   136,   110,    94,   108,   110,   108,   110,   165,   110,
     110,    91,    91,   130,     9,   109,    59,    19,   139,     3,
       5,     8,   109,   144,   167,   128,   101,   129,   110,   130,
      19,    61,   127,   137,   138,   141,   109,     3,   145,   146,
     110,   137,   109,    14,    28,   108,     8,    94,   108,   110,
     110,   138,   110,   128,   146
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

  case 157:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENTSTATUS; ;}
    break;

  case 163:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 164:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 165:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 166:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 168:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 169:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 172:

    { yyval.m_iValue = 1; ;}
    break;

  case 173:

    { yyval.m_iValue = 0; ;}
    break;

  case 174:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 175:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 176:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 177:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 180:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 181:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 182:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 185:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 186:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 189:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 190:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 191:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 192:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 193:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 194:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 195:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 196:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 197:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 198:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 199:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 200:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 201:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 203:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 204:

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

  case 205:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 208:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 210:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 214:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 215:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 218:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 219:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 222:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 223:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 224:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 225:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 226:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 233:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 234:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 235:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 243:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 244:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 245:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 246:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 247:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 248:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 249:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 251:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 252:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 253:

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

