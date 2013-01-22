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
     TOK_LIKE = 305,
     TOK_LIMIT = 306,
     TOK_MATCH = 307,
     TOK_MAX = 308,
     TOK_META = 309,
     TOK_MIN = 310,
     TOK_MOD = 311,
     TOK_NAMES = 312,
     TOK_NULL = 313,
     TOK_OPTION = 314,
     TOK_ORDER = 315,
     TOK_OPTIMIZE = 316,
     TOK_RAND = 317,
     TOK_READ = 318,
     TOK_REPEATABLE = 319,
     TOK_REPLACE = 320,
     TOK_RETURNS = 321,
     TOK_ROLLBACK = 322,
     TOK_RTINDEX = 323,
     TOK_SELECT = 324,
     TOK_SERIALIZABLE = 325,
     TOK_SET = 326,
     TOK_SESSION = 327,
     TOK_SHOW = 328,
     TOK_SONAME = 329,
     TOK_START = 330,
     TOK_STATUS = 331,
     TOK_STRING = 332,
     TOK_SUM = 333,
     TOK_TABLES = 334,
     TOK_TO = 335,
     TOK_TRANSACTION = 336,
     TOK_TRUE = 337,
     TOK_TRUNCATE = 338,
     TOK_UNCOMMITTED = 339,
     TOK_UPDATE = 340,
     TOK_VALUES = 341,
     TOK_VARIABLES = 342,
     TOK_WARNINGS = 343,
     TOK_WEIGHT = 344,
     TOK_WHERE = 345,
     TOK_WITHIN = 346,
     TOK_OR = 347,
     TOK_AND = 348,
     TOK_NE = 349,
     TOK_GTE = 350,
     TOK_LTE = 351,
     TOK_NOT = 352,
     TOK_NEG = 353
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
#define TOK_LIKE 305
#define TOK_LIMIT 306
#define TOK_MATCH 307
#define TOK_MAX 308
#define TOK_META 309
#define TOK_MIN 310
#define TOK_MOD 311
#define TOK_NAMES 312
#define TOK_NULL 313
#define TOK_OPTION 314
#define TOK_ORDER 315
#define TOK_OPTIMIZE 316
#define TOK_RAND 317
#define TOK_READ 318
#define TOK_REPEATABLE 319
#define TOK_REPLACE 320
#define TOK_RETURNS 321
#define TOK_ROLLBACK 322
#define TOK_RTINDEX 323
#define TOK_SELECT 324
#define TOK_SERIALIZABLE 325
#define TOK_SET 326
#define TOK_SESSION 327
#define TOK_SHOW 328
#define TOK_SONAME 329
#define TOK_START 330
#define TOK_STATUS 331
#define TOK_STRING 332
#define TOK_SUM 333
#define TOK_TABLES 334
#define TOK_TO 335
#define TOK_TRANSACTION 336
#define TOK_TRUE 337
#define TOK_TRUNCATE 338
#define TOK_UNCOMMITTED 339
#define TOK_UPDATE 340
#define TOK_VALUES 341
#define TOK_VARIABLES 342
#define TOK_WARNINGS 343
#define TOK_WEIGHT 344
#define TOK_WHERE 345
#define TOK_WITHIN 346
#define TOK_OR 347
#define TOK_AND 348
#define TOK_NE 349
#define TOK_GTE 350
#define TOK_LTE 351
#define TOK_NOT 352
#define TOK_NEG 353




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
#define YYFINAL  106
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   887

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  99
/* YYNRULES -- Number of rules. */
#define YYNRULES  276
/* YYNRULES -- Number of states. */
#define YYNSTATES  525

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   353

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   106,    95,     2,
     110,   111,   104,   102,   112,   103,   115,   105,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   109,
      98,    96,    99,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   113,    94,   114,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    97,
     100,   101,   107,   108
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    56,    58,    60,
      62,    72,    73,    74,    78,    79,    82,    87,    98,   100,
     104,   106,   109,   110,   112,   115,   117,   122,   127,   132,
     137,   142,   147,   151,   155,   161,   163,   167,   168,   170,
     173,   175,   179,   180,   184,   188,   193,   197,   201,   207,
     214,   218,   223,   229,   233,   237,   241,   245,   247,   253,
     257,   261,   265,   269,   273,   277,   279,   281,   286,   290,
     294,   296,   298,   301,   303,   306,   308,   312,   313,   315,
     319,   320,   322,   328,   329,   331,   335,   341,   343,   347,
     349,   352,   355,   356,   358,   361,   366,   367,   369,   372,
     374,   378,   382,   386,   392,   399,   403,   405,   409,   413,
     415,   417,   419,   421,   423,   425,   428,   431,   435,   439,
     443,   447,   451,   455,   459,   463,   467,   471,   475,   479,
     483,   487,   491,   495,   499,   503,   507,   509,   514,   519,
     523,   530,   537,   539,   543,   545,   547,   551,   557,   560,
     561,   564,   566,   569,   572,   576,   581,   586,   590,   592,
     594,   596,   598,   600,   602,   606,   611,   616,   621,   625,
     630,   638,   644,   646,   648,   650,   652,   654,   656,   658,
     660,   662,   665,   672,   674,   676,   677,   681,   683,   687,
     689,   693,   697,   699,   703,   705,   707,   709,   713,   716,
     724,   734,   741,   743,   747,   749,   753,   755,   759,   760,
     763,   765,   769,   773,   774,   776,   778,   780,   784,   786,
     788,   792,   799,   801,   805,   809,   813,   819,   824,   829,
     830,   832,   835,   837,   841,   845,   848,   852,   859,   860,
     862,   864,   867,   870,   873,   875,   883,   885,   887,   889,
     893,   900,   904,   908,   910,   914,   918
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     117,     0,    -1,   118,    -1,   119,    -1,   119,   109,    -1,
     172,    -1,   180,    -1,   166,    -1,   167,    -1,   170,    -1,
     181,    -1,   190,    -1,   192,    -1,   193,    -1,   196,    -1,
     201,    -1,   202,    -1,   206,    -1,   208,    -1,   209,    -1,
     210,    -1,   203,    -1,   211,    -1,   213,    -1,   214,    -1,
     120,    -1,   119,   109,   120,    -1,   121,    -1,   161,    -1,
     125,    -1,    69,   126,    36,   110,   122,   125,   111,   123,
     124,    -1,    -1,    -1,    60,    19,   146,    -1,    -1,    51,
       5,    -1,    51,     5,   112,     5,    -1,    69,   126,    36,
     130,   131,   140,   142,   144,   148,   150,    -1,   127,    -1,
     126,   112,   127,    -1,   104,    -1,   129,   128,    -1,    -1,
       3,    -1,    13,     3,    -1,   156,    -1,    16,   110,   156,
     111,    -1,    53,   110,   156,   111,    -1,    55,   110,   156,
     111,    -1,    78,   110,   156,   111,    -1,    41,   110,   156,
     111,    -1,    25,   110,   104,   111,    -1,    89,   110,   111,
      -1,    40,   110,   111,    -1,    25,   110,    30,     3,   111,
      -1,     3,    -1,   130,   112,     3,    -1,    -1,   132,    -1,
      90,   133,    -1,   135,    -1,   133,    93,   135,    -1,    -1,
     136,    96,   138,    -1,   136,    97,   138,    -1,    52,   110,
       8,   111,    -1,   136,    96,   137,    -1,   136,    97,   137,
      -1,   136,    43,   110,   139,   111,    -1,   136,   107,    43,
     110,   139,   111,    -1,   136,    43,     9,    -1,   136,   107,
      43,     9,    -1,   136,    18,   137,    93,   137,    -1,   136,
      99,   137,    -1,   136,    98,   137,    -1,   136,   100,   137,
      -1,   136,   101,   137,    -1,   134,    -1,   136,    18,   138,
      93,   138,    -1,   136,    99,   138,    -1,   136,    98,   138,
      -1,   136,   100,   138,    -1,   136,   101,   138,    -1,   136,
      96,     8,    -1,   136,    97,     8,    -1,     3,    -1,     4,
      -1,    25,   110,   104,   111,    -1,    40,   110,   111,    -1,
      89,   110,   111,    -1,    42,    -1,     5,    -1,   103,     5,
      -1,     6,    -1,   103,     6,    -1,   137,    -1,   139,   112,
     137,    -1,    -1,   141,    -1,    39,    19,   136,    -1,    -1,
     143,    -1,    91,    39,    60,    19,   146,    -1,    -1,   145,
      -1,    60,    19,   146,    -1,    60,    19,    62,   110,   111,
      -1,   147,    -1,   146,   112,   147,    -1,   136,    -1,   136,
      14,    -1,   136,    28,    -1,    -1,   149,    -1,    51,     5,
      -1,    51,     5,   112,     5,    -1,    -1,   151,    -1,    59,
     152,    -1,   153,    -1,   152,   112,   153,    -1,     3,    96,
       3,    -1,     3,    96,     5,    -1,     3,    96,   110,   154,
     111,    -1,     3,    96,     3,   110,     8,   111,    -1,     3,
      96,     8,    -1,   155,    -1,   154,   112,   155,    -1,     3,
      96,   137,    -1,     3,    -1,     4,    -1,    42,    -1,     5,
      -1,     6,    -1,     9,    -1,   103,   156,    -1,   107,   156,
      -1,   156,   102,   156,    -1,   156,   103,   156,    -1,   156,
     104,   156,    -1,   156,   105,   156,    -1,   156,    98,   156,
      -1,   156,    99,   156,    -1,   156,    95,   156,    -1,   156,
      94,   156,    -1,   156,   106,   156,    -1,   156,    31,   156,
      -1,   156,    56,   156,    -1,   156,   101,   156,    -1,   156,
     100,   156,    -1,   156,    96,   156,    -1,   156,    97,   156,
      -1,   156,    93,   156,    -1,   156,    92,   156,    -1,   110,
     156,   111,    -1,   113,   160,   114,    -1,   157,    -1,     3,
     110,   158,   111,    -1,    43,   110,   158,   111,    -1,     3,
     110,   111,    -1,    55,   110,   156,   112,   156,   111,    -1,
      53,   110,   156,   112,   156,   111,    -1,   159,    -1,   158,
     112,   159,    -1,   156,    -1,     8,    -1,     3,    96,   137,
      -1,   160,   112,     3,    96,   137,    -1,    73,   163,    -1,
      -1,    50,     8,    -1,    88,    -1,    76,   162,    -1,    54,
     162,    -1,    12,    76,   162,    -1,    12,     8,    76,   162,
      -1,    12,     3,    76,   162,    -1,    44,     3,    76,    -1,
       3,    -1,    58,    -1,     8,    -1,     5,    -1,     6,    -1,
     164,    -1,   165,   103,   164,    -1,    71,     3,    96,   169,
      -1,    71,     3,    96,   168,    -1,    71,     3,    96,    58,
      -1,    71,    57,   165,    -1,    71,    10,    96,   165,    -1,
      71,    38,     9,    96,   110,   139,   111,    -1,    71,    38,
       3,    96,   168,    -1,     3,    -1,     8,    -1,    82,    -1,
      33,    -1,   137,    -1,    23,    -1,    67,    -1,   171,    -1,
      17,    -1,    75,    81,    -1,   173,    47,     3,   174,    86,
     176,    -1,    45,    -1,    65,    -1,    -1,   110,   175,   111,
      -1,   136,    -1,   175,   112,   136,    -1,   177,    -1,   176,
     112,   177,    -1,   110,   178,   111,    -1,   179,    -1,   178,
     112,   179,    -1,   137,    -1,   138,    -1,     8,    -1,   110,
     139,   111,    -1,   110,   111,    -1,    27,    36,   130,    90,
      42,    96,   137,    -1,    27,    36,   130,    90,    42,    43,
     110,   139,   111,    -1,    20,     3,   110,   182,   185,   111,
      -1,   183,    -1,   182,   112,   183,    -1,   179,    -1,   110,
     184,   111,    -1,     8,    -1,   184,   112,     8,    -1,    -1,
     112,   186,    -1,   187,    -1,   186,   112,   187,    -1,   179,
     188,   189,    -1,    -1,    13,    -1,     3,    -1,    51,    -1,
     191,     3,   162,    -1,    29,    -1,    28,    -1,    73,    79,
     162,    -1,    85,   130,    71,   194,   132,   150,    -1,   195,
      -1,   194,   112,   195,    -1,     3,    96,   137,    -1,     3,
      96,   138,    -1,     3,    96,   110,   139,   111,    -1,     3,
      96,   110,   111,    -1,    73,   204,    87,   197,    -1,    -1,
     198,    -1,    90,   199,    -1,   200,    -1,   199,    92,   200,
      -1,     3,    96,     8,    -1,    73,    22,    -1,    73,    21,
      71,    -1,    71,   204,    81,    48,    49,   205,    -1,    -1,
      38,    -1,    72,    -1,    63,    84,    -1,    63,    24,    -1,
      64,    63,    -1,    70,    -1,    26,    37,     3,    66,   207,
      74,     8,    -1,    46,    -1,    34,    -1,    77,    -1,    32,
      37,     3,    -1,    15,    44,     3,    80,    68,     3,    -1,
      35,    68,     3,    -1,    69,   212,   148,    -1,    10,    -1,
      10,   115,     3,    -1,    83,    68,     3,    -1,    61,    44,
       3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   127,   127,   128,   129,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   158,   159,   163,   164,   168,
     169,   177,   191,   194,   201,   203,   207,   215,   231,   232,
     236,   237,   240,   242,   243,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   260,   261,   264,   266,   270,
     274,   275,   278,   279,   280,   284,   289,   296,   304,   312,
     321,   326,   331,   336,   341,   346,   351,   356,   361,   366,
     371,   376,   381,   386,   391,   399,   400,   405,   411,   417,
     423,   432,   433,   444,   445,   449,   455,   461,   463,   467,
     473,   475,   479,   490,   492,   496,   500,   507,   508,   512,
     513,   514,   517,   519,   523,   528,   535,   537,   541,   545,
     546,   550,   555,   560,   566,   571,   579,   584,   591,   601,
     602,   603,   604,   605,   606,   607,   608,   609,   610,   611,
     612,   613,   614,   615,   616,   617,   618,   619,   620,   621,
     622,   623,   624,   625,   626,   627,   628,   632,   633,   634,
     635,   636,   640,   641,   645,   646,   650,   651,   657,   660,
     662,   666,   667,   668,   669,   670,   675,   680,   690,   691,
     692,   693,   694,   698,   699,   703,   708,   713,   718,   719,
     723,   728,   736,   737,   741,   742,   743,   757,   758,   759,
     763,   764,   770,   778,   779,   782,   784,   788,   789,   793,
     794,   798,   802,   803,   807,   808,   809,   810,   811,   817,
     825,   839,   847,   851,   858,   859,   866,   876,   882,   884,
     888,   893,   897,   904,   906,   910,   911,   917,   925,   926,
     932,   938,   946,   947,   951,   955,   959,   963,   973,   979,
     980,   984,   988,   989,   993,   997,  1004,  1011,  1017,  1018,
    1019,  1023,  1024,  1025,  1026,  1032,  1043,  1044,  1045,  1049,
    1060,  1072,  1083,  1091,  1092,  1101,  1112
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
  "TOK_ISOLATION", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_RAND", "TOK_READ", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_STRING", "TOK_SUM", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", 
  "TOK_TRUE", "TOK_TRUNCATE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", 
  "'}'", "'.'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select", "subselect_start", "opt_outer_order", 
  "opt_outer_limit", "select_from", "select_items_list", "select_item", 
  "opt_alias", "select_expr", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "expr_float_unhandled", "where_item", 
  "expr_ident", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "show_stmt", "like_filter", "show_what", 
  "simple_set_value", "set_value", "set_stmt", "set_global_stmt", 
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
     345,   346,   347,   348,   124,    38,    61,   349,    60,    62,
     350,   351,    43,    45,    42,    47,    37,   352,   353,    59,
      40,    41,    44,   123,   125,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,   116,   117,   117,   117,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   119,   119,   120,   120,   121,
     121,   122,   123,   123,   124,   124,   124,   125,   126,   126,
     127,   127,   128,   128,   128,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   130,   130,   131,   131,   132,
     133,   133,   134,   134,   134,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   135,   135,   135,
     135,   135,   135,   135,   135,   136,   136,   136,   136,   136,
     136,   137,   137,   138,   138,   139,   139,   140,   140,   141,
     142,   142,   143,   144,   144,   145,   145,   146,   146,   147,
     147,   147,   148,   148,   149,   149,   150,   150,   151,   152,
     152,   153,   153,   153,   153,   153,   154,   154,   155,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   157,   157,   157,
     157,   157,   158,   158,   159,   159,   160,   160,   161,   162,
     162,   163,   163,   163,   163,   163,   163,   163,   164,   164,
     164,   164,   164,   165,   165,   166,   166,   166,   166,   166,
     167,   167,   168,   168,   169,   169,   169,   170,   170,   170,
     171,   171,   172,   173,   173,   174,   174,   175,   175,   176,
     176,   177,   178,   178,   179,   179,   179,   179,   179,   180,
     180,   181,   182,   182,   183,   183,   184,   184,   185,   185,
     186,   186,   187,   188,   188,   189,   189,   190,   191,   191,
     192,   193,   194,   194,   195,   195,   195,   195,   196,   197,
     197,   198,   199,   199,   200,   201,   202,   203,   204,   204,
     204,   205,   205,   205,   205,   206,   207,   207,   207,   208,
     209,   210,   211,   212,   212,   213,   214
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       9,     0,     0,     3,     0,     2,     4,    10,     1,     3,
       1,     2,     0,     1,     2,     1,     4,     4,     4,     4,
       4,     4,     3,     3,     5,     1,     3,     0,     1,     2,
       1,     3,     0,     3,     3,     4,     3,     3,     5,     6,
       3,     4,     5,     3,     3,     3,     3,     1,     5,     3,
       3,     3,     3,     3,     3,     1,     1,     4,     3,     3,
       1,     1,     2,     1,     2,     1,     3,     0,     1,     3,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     4,     4,     3,
       6,     6,     1,     3,     1,     1,     3,     5,     2,     0,
       2,     1,     2,     2,     3,     4,     4,     3,     1,     1,
       1,     1,     1,     1,     3,     4,     4,     4,     3,     4,
       7,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     6,     1,     1,     0,     3,     1,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     3,     2,     7,
       9,     6,     1,     3,     1,     3,     1,     3,     0,     2,
       1,     3,     3,     0,     1,     1,     1,     3,     1,     1,
       3,     6,     1,     3,     3,     3,     5,     4,     4,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     3,
       6,     3,     3,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,   200,     0,   197,     0,     0,   239,   238,     0,
       0,   203,     0,   204,   198,     0,   258,   258,     0,     0,
       0,     0,     2,     3,    25,    27,    29,    28,     7,     8,
       9,   199,     5,     0,     6,    10,    11,     0,    12,    13,
      14,    15,    16,    21,    17,    18,    19,    20,    22,    23,
      24,     0,     0,     0,     0,     0,     0,     0,   129,   130,
     132,   133,   134,   273,     0,     0,     0,     0,   131,     0,
       0,     0,     0,     0,     0,    40,     0,     0,     0,     0,
      38,    42,    45,   156,   112,     0,     0,   259,     0,   260,
       0,     0,     0,   255,   259,     0,   169,   169,   169,   171,
     168,     0,   201,     0,    55,     0,     1,     4,     0,   169,
       0,     0,     0,     0,   269,   271,   276,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     135,   136,     0,     0,     0,     0,     0,    43,     0,    41,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   272,   113,
       0,     0,     0,     0,   178,   181,   182,   180,   179,   183,
     188,     0,     0,     0,   169,   256,     0,     0,   173,   172,
     240,   249,   275,     0,     0,     0,     0,    26,   205,   237,
       0,    91,    93,   216,     0,     0,   214,   215,   224,   228,
     222,     0,     0,   165,   159,   164,     0,   162,   274,     0,
       0,     0,    53,     0,     0,     0,     0,     0,    52,     0,
       0,   154,     0,     0,   155,    31,    57,    39,    44,   146,
     147,   153,   152,   144,   143,   150,   151,   141,   142,   149,
     148,   137,   138,   139,   140,   145,   114,   192,   193,   195,
     187,   194,     0,   196,   186,   185,   189,     0,     0,     0,
       0,   169,   169,   174,   177,   170,     0,   248,   250,     0,
       0,   242,    56,     0,     0,     0,    92,    94,   226,   218,
      95,     0,     0,     0,     0,   267,   266,   268,     0,     0,
     157,     0,    46,     0,    51,    50,   158,    47,     0,    48,
       0,    49,     0,     0,   166,     0,     0,    62,    97,    58,
       0,   191,     0,   184,     0,   176,   175,     0,   251,   252,
       0,     0,   116,    85,    86,     0,     0,    90,     0,   207,
       0,     0,   270,   217,     0,   225,     0,   224,   223,   229,
     230,   221,     0,     0,     0,   163,    54,     0,     0,     0,
       0,     0,     0,    59,    77,    60,     0,     0,   100,    98,
     115,     0,     0,     0,   264,   257,     0,     0,     0,   244,
     245,   243,     0,   241,   117,     0,     0,     0,   206,     0,
       0,   202,   209,    96,   227,   234,     0,     0,   265,     0,
     219,   161,   160,   167,     0,    32,     0,    62,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   103,
     101,   190,   262,   261,   263,   254,   253,   247,     0,     0,
     118,   119,     0,    88,    89,   208,     0,     0,   212,     0,
     235,   236,   232,   233,   231,     0,     0,     0,    34,     0,
      61,     0,     0,    70,     0,    83,    66,    63,    84,    67,
      64,    74,    80,    73,    79,    75,    81,    76,    82,     0,
      99,     0,     0,   112,   104,   246,     0,     0,    87,   211,
       0,   210,   220,     0,     0,    30,    65,     0,     0,     0,
      71,     0,     0,     0,   116,   121,   122,   125,     0,   120,
     213,   109,    33,   107,    35,    72,     0,    78,    68,     0,
       0,     0,   105,    37,     0,     0,     0,   126,   110,   111,
       0,     0,    69,   102,     0,     0,     0,   123,     0,   108,
      36,   106,   124,   128,   127
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    21,    22,    23,    24,    25,   306,   438,   475,    26,
      79,    80,   139,    81,   226,   308,   309,   353,   354,   355,
     491,   280,   197,   281,   358,   359,   409,   410,   463,   464,
     492,   493,   158,   159,   373,   374,   420,   421,   506,   507,
      82,    83,   206,   207,   134,    27,   178,   100,   169,   170,
      28,    29,   254,   255,    30,    31,    32,    33,   274,   330,
     381,   382,   427,   198,    34,    35,   199,   200,   282,   284,
     339,   340,   386,   432,    36,    37,    38,    39,   270,   271,
      40,   267,   268,   318,   319,    41,    42,    43,    90,   365,
      44,   288,    45,    46,    47,    48,    84,    49,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -378
static const short yypact[] =
{
     802,   -29,  -378,    36,  -378,    60,    23,  -378,  -378,    67,
      -1,  -378,    71,  -378,  -378,   140,   210,   618,    74,   103,
     182,   207,  -378,   105,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,   174,  -378,  -378,  -378,   232,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,   248,   152,   263,   182,   265,   267,   280,   185,  -378,
    -378,  -378,  -378,   184,   201,   213,   215,   217,  -378,   219,
     223,   234,   238,   251,   275,  -378,   275,   275,   316,   -11,
    -378,   203,   581,  -378,   291,   268,   273,   208,   191,  -378,
     292,    99,   304,  -378,  -378,   373,   327,   327,   327,  -378,
    -378,   293,  -378,   376,  -378,   113,  -378,    85,   378,   327,
     303,    18,   318,    49,  -378,  -378,  -378,    57,   383,   275,
     -17,   276,   275,   255,   275,   275,   275,   278,   281,   283,
    -378,  -378,   419,   298,   -66,     3,   199,  -378,   392,  -378,
     275,   275,   275,   275,   275,   275,   275,   275,   275,   275,
     275,   275,   275,   275,   275,   275,   275,   391,  -378,  -378,
     154,   191,   302,   305,  -378,  -378,  -378,  -378,  -378,  -378,
     296,   352,   326,   328,   327,  -378,   347,   395,  -378,  -378,
    -378,   337,  -378,   440,   441,   199,   122,  -378,   335,  -378,
     384,  -378,  -378,  -378,   240,    11,  -378,  -378,  -378,   334,
    -378,   188,   409,  -378,  -378,   581,   144,  -378,  -378,   445,
     450,   343,  -378,   471,   161,   314,   336,   497,  -378,   275,
     275,  -378,    17,   452,  -378,  -378,    88,  -378,  -378,  -378,
    -378,   632,   658,   684,   710,   693,   693,   605,   605,   605,
     605,    86,    86,  -378,  -378,  -378,   344,  -378,  -378,  -378,
    -378,  -378,   468,  -378,  -378,  -378,   296,   224,   364,   191,
     408,   327,   327,  -378,  -378,  -378,   474,  -378,  -378,   385,
      89,  -378,  -378,   332,   394,   476,  -378,  -378,  -378,  -378,
    -378,   163,   189,    18,   371,  -378,  -378,  -378,   410,    -8,
    -378,   255,  -378,   372,  -378,  -378,  -378,  -378,   275,  -378,
     275,  -378,   366,   393,  -378,   404,   434,   282,   465,  -378,
     501,  -378,    17,  -378,   167,  -378,  -378,   411,   416,  -378,
      26,   440,   451,  -378,  -378,   399,   421,  -378,   422,  -378,
     202,   423,  -378,  -378,    17,  -378,   518,   138,  -378,   417,
    -378,  -378,   526,   425,    17,  -378,  -378,   523,   549,    17,
     199,   444,   426,   459,  -378,  -378,   253,   538,   467,  -378,
    -378,   204,    79,   496,  -378,  -378,   552,   474,    13,  -378,
    -378,  -378,   558,  -378,  -378,   458,   470,   472,  -378,   332,
      22,   466,  -378,  -378,  -378,  -378,    39,    22,  -378,    17,
    -378,  -378,  -378,  -378,    21,   524,   577,   282,    50,    -2,
      28,    35,    50,    50,    50,    50,   543,   332,   548,   528,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,   209,   508,
     494,  -378,   498,  -378,  -378,  -378,    15,   220,  -378,   423,
    -378,  -378,  -378,   594,  -378,   226,   182,   591,   560,   502,
    -378,   521,   539,  -378,    17,  -378,  -378,  -378,  -378,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,    20,
    -378,   571,   614,   291,  -378,  -378,     9,   558,  -378,  -378,
      22,  -378,  -378,   332,   630,  -378,  -378,    17,    32,   228,
    -378,    17,   619,   301,   451,   547,  -378,  -378,   655,  -378,
    -378,    30,   553,  -378,   554,  -378,   653,  -378,  -378,   235,
     332,   557,   553,  -378,   656,   572,   244,  -378,  -378,  -378,
     332,   664,  -378,   553,   559,   580,    17,  -378,   655,  -378,
    -378,  -378,  -378,  -378,  -378
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -378,  -378,  -378,  -378,   564,  -378,  -378,  -378,  -378,   386,
     345,   562,  -378,  -378,   132,  -378,   429,  -378,  -378,   299,
    -270,  -111,  -309,  -304,  -378,  -378,  -378,  -378,  -378,  -378,
    -377,   183,   237,  -378,   218,  -378,  -378,   236,  -378,   186,
     -72,  -378,   578,   414,  -378,  -378,   -88,  -378,   453,   555,
    -378,  -378,   456,  -378,  -378,  -378,  -378,  -378,  -378,  -378,
    -378,   288,  -378,  -282,  -378,  -378,  -378,   435,  -378,  -378,
    -378,   333,  -378,  -378,  -378,  -378,  -378,  -378,  -378,   398,
    -378,  -378,  -378,  -378,   354,  -378,  -378,  -378,   705,  -378,
    -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378,  -378
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -234
static const short yytable[] =
{
     196,   337,   130,   329,   131,   132,   104,   443,   361,   179,
     180,   370,   485,   210,   486,    51,   191,   487,   191,   278,
     191,   189,   191,   191,   192,   135,   193,   191,   192,   480,
     193,   191,   192,   191,   192,   343,   445,   356,   192,    52,
     191,   192,   430,   448,   508,   205,   223,   209,   224,   253,
     213,   205,   215,   216,   217,   191,   192,   436,   509,    54,
      58,    59,    60,    61,   418,   203,    62,    56,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,   240,
     241,   242,   243,   244,   245,   435,   263,   211,   344,   442,
     431,   447,   450,   452,   454,   456,   458,    53,   428,    68,
      69,   136,   172,   412,    55,   433,   502,   173,   444,   425,
     128,   304,   129,   225,   252,    57,   252,   140,   252,   488,
     252,   194,   279,   513,   417,   194,   279,   356,   195,   194,
     481,   194,   426,   136,    91,   496,   368,   460,   194,   202,
     479,  -233,   141,    58,    59,    60,    61,   302,   303,    62,
      63,   385,   105,   194,   185,   102,    64,   247,   186,   191,
      74,   184,   248,   413,    76,    65,    95,    77,   204,   497,
      78,   103,   196,   315,   316,   174,    96,   499,   307,   307,
      66,    67,    68,    69,   183,   104,   113,   249,   490,  -233,
     154,   155,   156,    70,   164,    71,   165,   166,    97,   167,
     184,   321,    58,    59,    60,    61,   137,   106,    62,   369,
      99,   162,   250,    85,   107,    64,   138,   163,    72,   205,
      86,   108,   285,   383,    65,   184,   347,   247,   348,    73,
     362,   363,   248,   390,   286,   109,   251,   364,   393,    66,
      67,    68,    69,    74,    75,   276,   277,    76,    87,   168,
      77,   110,    70,    78,    71,   290,   291,   252,    58,    59,
      60,    61,   111,   203,    62,   287,   112,    88,   114,   196,
     115,   398,   296,   291,   333,   334,   196,    72,    58,    59,
      60,    61,    89,   116,    62,   323,   324,   441,    73,   446,
     449,   451,   453,   455,   457,   117,   399,    68,    69,   118,
     335,   336,    74,    75,   323,   324,    76,   325,   128,    77,
     129,   119,    78,   378,   379,   411,   334,    68,    69,   133,
     465,   334,   326,   120,   327,   121,   325,   122,   128,   123,
     129,   469,   470,   124,   352,   323,   324,   472,   334,   498,
     334,   326,   157,   327,   125,   140,   512,   334,   126,   400,
     401,   402,   403,   404,   405,   517,   518,   325,    74,   196,
     406,   127,    76,   501,   160,    77,   495,   140,    78,   161,
     141,   328,   326,   171,   327,   175,   176,   177,    74,   182,
     181,   188,    76,   190,   201,    77,   208,   212,    78,   218,
     328,   219,   141,   220,   222,   228,   246,   140,   257,   259,
     260,   258,   261,   265,   262,   523,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   328,   141,   264,   140,   297,   298,   266,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   269,   272,   273,   283,   299,   300,   141,
     140,   289,   275,   293,   294,   305,   310,   314,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   276,   312,   141,   140,   317,   298,   332,
     331,   320,   341,   346,   342,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     349,   141,   140,   350,   357,   300,   360,   366,   367,   375,
     372,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   384,   141,   140,   387,
     221,   376,   377,   380,   388,   389,   396,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   397,   141,   140,   395,   292,   407,   408,   414,
     415,   419,   422,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   429,   141,
     140,   423,   295,   424,   437,   439,   459,   461,   462,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   466,   141,   467,   385,   301,   468,
     473,   474,   140,   476,   477,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
      91,   482,   478,   483,   391,   494,   140,   141,   500,    92,
      93,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,    94,   504,   505,   277,
     392,   141,    95,   140,   515,   510,   511,   514,   516,   520,
     521,   187,    96,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   141,   140,
      89,   522,   351,   519,    97,   394,   440,    98,   227,   322,
     484,   214,   503,   489,   524,   345,    99,   152,   153,   154,
     155,   156,   313,   311,   141,   140,   256,   471,   338,   371,
     434,   416,   101,     0,   140,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,     0,
     141,   140,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,     0,   141,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   148,   149,   150,   151,   152,   153,   154,   155,   156,
       0,     0,     0,     0,     0,     0,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,     1,     0,     2,
       0,     0,     3,     0,     0,     4,     0,     0,     5,     6,
       7,     8,     0,     0,     9,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,     0,     0,     0,    13,     0,    14,
       0,    15,     0,    16,     0,    17,     0,    18,     0,     0,
       0,     0,     0,     0,     0,    19,     0,    20
};

static const short yycheck[] =
{
     111,   283,    74,   273,    76,    77,     3,     9,   312,    97,
      98,   320,     3,    30,     5,    44,     5,     8,     5,     8,
       5,   109,     5,     5,     6,    36,     8,     5,     6,     9,
       8,     5,     6,     5,     6,    43,     8,   307,     6,     3,
       5,     6,     3,     8,    14,   117,   112,   119,   114,   160,
     122,   123,   124,   125,   126,     5,     6,    36,    28,    36,
       3,     4,     5,     6,   368,     8,     9,    68,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   389,   174,   104,    96,   398,
      51,   400,   401,   402,   403,   404,   405,    37,   380,    42,
      43,   112,     3,    24,    37,   387,   483,     8,   110,   379,
      53,   222,    55,   110,   103,    44,   103,    31,   103,   110,
     103,   103,   111,   500,   111,   103,   111,   397,   110,   103,
     110,   103,   110,   112,    12,   103,   110,   407,   103,    90,
     444,     3,    56,     3,     4,     5,     6,   219,   220,     9,
      10,    13,    20,   103,    69,    81,    16,     3,    73,     5,
     103,   112,     8,    84,   107,    25,    44,   110,   111,   478,
     113,    68,   283,   261,   262,    76,    54,   481,    90,    90,
      40,    41,    42,    43,    71,     3,    54,    33,   470,    51,
     104,   105,   106,    53,     3,    55,     5,     6,    76,     8,
     112,   112,     3,     4,     5,     6,     3,     0,     9,   320,
      88,     3,    58,     3,   109,    16,    13,     9,    78,   291,
      10,    47,    34,   334,    25,   112,   298,     3,   300,    89,
      63,    64,     8,   344,    46,     3,    82,    70,   349,    40,
      41,    42,    43,   103,   104,     5,     6,   107,    38,    58,
     110,     3,    53,   113,    55,   111,   112,   103,     3,     4,
       5,     6,   110,     8,     9,    77,     3,    57,     3,   380,
       3,    18,   111,   112,   111,   112,   387,    78,     3,     4,
       5,     6,    72,     3,     9,     3,     4,   398,    89,   400,
     401,   402,   403,   404,   405,   110,    43,    42,    43,   115,
     111,   112,   103,   104,     3,     4,   107,    25,    53,   110,
      55,   110,   113,   111,   112,   111,   112,    42,    43,     3,
     111,   112,    40,   110,    42,   110,    25,   110,    53,   110,
      55,   111,   112,   110,    52,     3,     4,   111,   112,   111,
     112,    40,    51,    42,   110,    31,   111,   112,   110,    96,
      97,    98,    99,   100,   101,   111,   112,    25,   103,   470,
     107,   110,   107,    62,    96,   110,   477,    31,   113,    96,
      56,    89,    40,    81,    42,    71,     3,    50,   103,     3,
      87,     3,   107,    80,    66,   110,     3,   111,   113,   111,
      89,   110,    56,   110,    96,     3,     5,    31,    96,   103,
      48,    96,    76,     8,    76,   516,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    89,    56,    76,    31,   111,   112,    90,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,     3,     3,   110,   112,   111,   112,    56,
      31,    42,    68,     3,   111,     3,   112,    49,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,     5,   110,    56,    31,     3,   112,     3,
      86,    96,   111,   111,    74,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      96,    56,    31,    69,    39,   112,     5,    96,    92,   110,
      59,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,     8,    56,    31,   112,
     111,   110,   110,   110,     8,   110,   110,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,    93,    56,    31,   111,   111,    19,    91,    63,
       8,     3,   104,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   112,    56,
      31,   111,   111,   111,    60,     8,    43,    39,    60,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    96,    56,   112,    13,   111,   111,
      19,    51,    31,   111,    93,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      12,    60,    93,    19,   111,     5,    31,    56,    19,    21,
      22,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,    38,   110,     3,     6,
     111,    56,    44,    31,     8,   112,   112,   110,    96,     5,
     111,   107,    54,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,    56,    31,
      72,   111,   306,   510,    76,   350,   397,    79,   136,   270,
     463,   123,   484,   467,   518,   291,    88,   102,   103,   104,
     105,   106,   259,   257,    56,    31,   161,   429,   283,   321,
     387,   367,    17,    -1,    31,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,    -1,
      56,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      -1,    -1,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    98,    99,   100,   101,   102,   103,   104,   105,   106,
      -1,    -1,    -1,    -1,    -1,    -1,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    15,    -1,    17,
      -1,    -1,    20,    -1,    -1,    23,    -1,    -1,    26,    27,
      28,    29,    -1,    -1,    32,    -1,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    -1,    -1,    -1,    65,    -1,    67,
      -1,    69,    -1,    71,    -1,    73,    -1,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    83,    -1,    85
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    15,    17,    20,    23,    26,    27,    28,    29,    32,
      35,    45,    61,    65,    67,    69,    71,    73,    75,    83,
      85,   117,   118,   119,   120,   121,   125,   161,   166,   167,
     170,   171,   172,   173,   180,   181,   190,   191,   192,   193,
     196,   201,   202,   203,   206,   208,   209,   210,   211,   213,
     214,    44,     3,    37,    36,    37,    68,    44,     3,     4,
       5,     6,     9,    10,    16,    25,    40,    41,    42,    43,
      53,    55,    78,    89,   103,   104,   107,   110,   113,   126,
     127,   129,   156,   157,   212,     3,    10,    38,    57,    72,
     204,    12,    21,    22,    38,    44,    54,    76,    79,    88,
     163,   204,    81,    68,     3,   130,     0,   109,    47,     3,
       3,   110,     3,   130,     3,     3,     3,   110,   115,   110,
     110,   110,   110,   110,   110,   110,   110,   110,    53,    55,
     156,   156,   156,     3,   160,    36,   112,     3,    13,   128,
      31,    56,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,    51,   148,   149,
      96,    96,     3,     9,     3,     5,     6,     8,    58,   164,
     165,    81,     3,     8,    76,    71,     3,    50,   162,   162,
     162,    87,     3,    71,   112,    69,    73,   120,     3,   162,
      80,     5,     6,     8,   103,   110,   137,   138,   179,   182,
     183,    66,    90,     8,   111,   156,   158,   159,     3,   156,
      30,   104,   111,   156,   158,   156,   156,   156,   111,   110,
     110,   111,    96,   112,   114,   110,   130,   127,     3,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,     5,     3,     8,    33,
      58,    82,   103,   137,   168,   169,   165,    96,    96,   103,
      48,    76,    76,   162,    76,     8,    90,   197,   198,     3,
     194,   195,     3,   110,   174,    68,     5,     6,     8,   111,
     137,   139,   184,   112,   185,    34,    46,    77,   207,    42,
     111,   112,   111,     3,   111,   111,   111,   111,   112,   111,
     112,   111,   156,   156,   137,     3,   122,    90,   131,   132,
     112,   168,   110,   164,    49,   162,   162,     3,   199,   200,
      96,   112,   132,     3,     4,    25,    40,    42,    89,   136,
     175,    86,     3,   111,   112,   111,   112,   179,   183,   186,
     187,   111,    74,    43,    96,   159,   111,   156,   156,    96,
      69,   125,    52,   133,   134,   135,   136,    39,   140,   141,
       5,   139,    63,    64,    70,   205,    96,    92,   110,   137,
     138,   195,    59,   150,   151,   110,   110,   110,   111,   112,
     110,   176,   177,   137,     8,    13,   188,   112,     8,   110,
     137,   111,   111,   137,   126,   111,   110,    93,    18,    43,
      96,    97,    98,    99,   100,   101,   107,    19,    91,   142,
     143,   111,    24,    84,    63,     8,   200,   111,   139,     3,
     152,   153,   104,   111,   111,   136,   110,   178,   179,   112,
       3,    51,   189,   179,   187,   139,    36,    60,   123,     8,
     135,   137,   138,     9,   110,     8,   137,   138,     8,   137,
     138,   137,   138,   137,   138,   137,   138,   137,   138,    43,
     136,    39,    60,   144,   145,   111,    96,   112,   111,   111,
     112,   177,   111,    19,    51,   124,   111,    93,    93,   139,
       9,   110,    60,    19,   148,     3,     5,     8,   110,   153,
     179,   136,   146,   147,     5,   137,   103,   138,   111,   139,
      19,    62,   146,   150,   110,     3,   154,   155,    14,    28,
     112,   112,   111,   146,   110,     8,    96,   111,   112,   147,
       5,   111,   111,   137,   155
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

  case 30:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 31:

    {
		CSphVector<CSphQueryItem> & dItems = pParser->m_pQuery->m_dItems;
		if ( dItems.GetLength()!=1 || dItems[0].m_sExpr!="*" )
		{
			yyerror ( pParser, "outer select list must be a single star" );
			YYERROR;
		}
		dItems.Reset();
		pParser->ResetSelect();
	;}
    break;

  case 32:

    {
			pParser->m_pQuery->m_sOuterOrderBy = pParser->m_pQuery->m_sOrderBy;
		;}
    break;

  case 33:

    {
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart,
				yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 35:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 36:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 37:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart,
				yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 40:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 43:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 44:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 45:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 46:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 47:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 48:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 49:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 50:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 51:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 52:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 53:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 54:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 56:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 65:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 66:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 67:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 68:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 69:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 70:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 71:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 72:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 73:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 74:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 75:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 76:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 77:

    {
			yyerror ( pParser, "only >=, <=,<,>, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 78:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 79:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 80:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 81:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 82:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 83:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 84:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 86:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 87:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 88:

    {
			yyval.m_sValue = "@groupby";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 89:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 90:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 91:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 92:

    {
			yyval.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 93:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 94:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 95:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 96:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 99:

    {
			pParser->SetGroupBy ( yyvsp[0].m_sValue );
		;}
    break;

  case 102:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 105:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 106:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 108:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 110:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 111:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 114:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 115:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 121:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 122:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 123:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 124:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 125:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 126:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 127:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 128:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 130:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 131:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 135:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 143:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 144:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 145:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 146:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 147:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 148:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 149:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 150:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 151:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 152:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 153:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 154:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 155:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 157:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 158:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 159:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 160:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 161:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 166:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 167:

    { yyval = yyvsp[-4]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 170:

    { pParser->m_pStmt->m_sStringParam = yyvsp[0].m_sValue; ;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 173:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 174:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 175:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 176:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-2].m_sValue;
		;}
    break;

  case 177:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 185:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 186:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 187:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 188:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 189:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 190:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 191:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 194:

    { yyval.m_iValue = 1; ;}
    break;

  case 195:

    { yyval.m_iValue = 0; ;}
    break;

  case 196:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 197:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 198:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 199:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 202:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 203:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 204:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 207:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 208:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 211:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 212:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 213:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 214:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 215:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 216:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 217:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 218:

    { yyval.m_iInstype = TOK_CONST_MVA; ;}
    break;

  case 219:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-4].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-4].m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 220:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			pParser->m_pStmt->m_iListStart = yyvsp[-6].m_iStart;
			pParser->m_pStmt->m_iListEnd = yyvsp[-6].m_iEnd;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 221:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 222:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 223:

    {
			AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 225:

    {
			yyval.m_iInstype = TOK_CONST_STRINGS;
		;}
    break;

  case 226:

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

  case 227:

    {
			pParser->m_pStmt->m_dCallStrings.Add ( yyvsp[0].m_sValue );
		;}
    break;

  case 230:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 232:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 236:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 237:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[-1].m_sValue;
		;}
    break;

  case 240:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 241:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 244:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 245:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 246:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 247:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3].m_sValue, tNoValues );
		;}
    break;

  case 248:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 255:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 256:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 257:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 265:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 266:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 267:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 268:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 269:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 270:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sStringParam = yyvsp[0].m_sValue;
		;}
    break;

  case 271:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 272:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = yyvsp[-1].m_sValue;
		;}
    break;

  case 274:

    {
			yyval.m_sValue.SetSprintf ( "%s.%s", yyvsp[-2].m_sValue.cstr(), yyvsp[0].m_sValue.cstr() );
		;}
    break;

  case 275:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 276:

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

