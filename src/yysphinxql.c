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
     TOK_FLUSH = 286,
     TOK_FROM = 287,
     TOK_FUNCTION = 288,
     TOK_GLOBAL = 289,
     TOK_GROUP = 290,
     TOK_ID = 291,
     TOK_IN = 292,
     TOK_INDEX = 293,
     TOK_INSERT = 294,
     TOK_INT = 295,
     TOK_INTO = 296,
     TOK_LIMIT = 297,
     TOK_MATCH = 298,
     TOK_MAX = 299,
     TOK_META = 300,
     TOK_MIN = 301,
     TOK_MOD = 302,
     TOK_NAMES = 303,
     TOK_NULL = 304,
     TOK_OPTION = 305,
     TOK_ORDER = 306,
     TOK_RAND = 307,
     TOK_REPLACE = 308,
     TOK_RETURNS = 309,
     TOK_ROLLBACK = 310,
     TOK_RTINDEX = 311,
     TOK_SELECT = 312,
     TOK_SET = 313,
     TOK_SESSION = 314,
     TOK_SHOW = 315,
     TOK_SONAME = 316,
     TOK_START = 317,
     TOK_STATUS = 318,
     TOK_SUM = 319,
     TOK_TABLES = 320,
     TOK_TO = 321,
     TOK_TRANSACTION = 322,
     TOK_TRUE = 323,
     TOK_UPDATE = 324,
     TOK_VALUES = 325,
     TOK_VARIABLES = 326,
     TOK_WARNINGS = 327,
     TOK_WEIGHT = 328,
     TOK_WHERE = 329,
     TOK_WITHIN = 330,
     TOK_OR = 331,
     TOK_AND = 332,
     TOK_NE = 333,
     TOK_GTE = 334,
     TOK_LTE = 335,
     TOK_NOT = 336,
     TOK_NEG = 337
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
#define TOK_FLUSH 286
#define TOK_FROM 287
#define TOK_FUNCTION 288
#define TOK_GLOBAL 289
#define TOK_GROUP 290
#define TOK_ID 291
#define TOK_IN 292
#define TOK_INDEX 293
#define TOK_INSERT 294
#define TOK_INT 295
#define TOK_INTO 296
#define TOK_LIMIT 297
#define TOK_MATCH 298
#define TOK_MAX 299
#define TOK_META 300
#define TOK_MIN 301
#define TOK_MOD 302
#define TOK_NAMES 303
#define TOK_NULL 304
#define TOK_OPTION 305
#define TOK_ORDER 306
#define TOK_RAND 307
#define TOK_REPLACE 308
#define TOK_RETURNS 309
#define TOK_ROLLBACK 310
#define TOK_RTINDEX 311
#define TOK_SELECT 312
#define TOK_SET 313
#define TOK_SESSION 314
#define TOK_SHOW 315
#define TOK_SONAME 316
#define TOK_START 317
#define TOK_STATUS 318
#define TOK_SUM 319
#define TOK_TABLES 320
#define TOK_TO 321
#define TOK_TRANSACTION 322
#define TOK_TRUE 323
#define TOK_UPDATE 324
#define TOK_VALUES 325
#define TOK_VARIABLES 326
#define TOK_WARNINGS 327
#define TOK_WEIGHT 328
#define TOK_WHERE 329
#define TOK_WITHIN 330
#define TOK_OR 331
#define TOK_AND 332
#define TOK_NE 333
#define TOK_GTE 334
#define TOK_LTE 335
#define TOK_NOT 336
#define TOK_NEG 337




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
#define YYFINAL  86
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   726

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  97
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  77
/* YYNRULES -- Number of rules. */
#define YYNRULES  218
/* YYNRULES -- Number of states. */
#define YYNSTATES  410

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   337

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    90,    79,     2,
      95,    96,    88,    86,    94,    87,     2,    89,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    93,
      82,    80,    83,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    78,     2,     2,     2,     2,     2,
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
      75,    76,    77,    81,    84,    85,    91,    92
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    46,    48,    50,    61,    63,    67,    69,    72,
      73,    75,    78,    80,    85,    90,    95,   100,   105,   109,
     115,   117,   121,   122,   124,   127,   129,   133,   138,   142,
     146,   152,   159,   163,   168,   174,   178,   182,   186,   190,
     194,   198,   202,   206,   212,   216,   220,   222,   224,   229,
     233,   235,   237,   240,   242,   245,   247,   251,   252,   254,
     258,   259,   261,   267,   268,   270,   274,   280,   282,   286,
     288,   291,   294,   295,   297,   300,   305,   306,   308,   311,
     313,   317,   321,   325,   331,   338,   340,   344,   348,   350,
     352,   354,   356,   358,   360,   363,   366,   370,   374,   378,
     382,   386,   390,   394,   398,   402,   406,   410,   414,   418,
     422,   426,   430,   434,   438,   440,   445,   450,   454,   461,
     468,   470,   474,   476,   478,   481,   483,   485,   487,   489,
     491,   493,   495,   497,   502,   507,   512,   516,   521,   529,
     535,   537,   539,   541,   543,   545,   547,   549,   551,   553,
     556,   563,   565,   567,   568,   572,   574,   578,   580,   584,
     588,   590,   594,   596,   598,   600,   604,   612,   622,   629,
     630,   633,   635,   639,   643,   644,   646,   648,   650,   653,
     655,   657,   660,   666,   668,   672,   676,   680,   686,   691,
     695,   696,   698,   701,   709,   711,   713,   717,   724
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      98,     0,    -1,    99,    -1,   100,    -1,   100,    93,    -1,
     145,    -1,   153,    -1,   139,    -1,   140,    -1,   143,    -1,
     154,    -1,   160,    -1,   162,    -1,   163,    -1,   166,    -1,
     168,    -1,   169,    -1,   171,    -1,   172,    -1,   173,    -1,
     101,    -1,   100,    93,   101,    -1,   102,    -1,   136,    -1,
      57,   103,    32,   107,   108,   116,   118,   120,   124,   126,
      -1,   104,    -1,   103,    94,   104,    -1,    88,    -1,   106,
     105,    -1,    -1,     3,    -1,    11,     3,    -1,   132,    -1,
      14,    95,   132,    96,    -1,    44,    95,   132,    96,    -1,
      46,    95,   132,    96,    -1,    64,    95,   132,    96,    -1,
      21,    95,    88,    96,    -1,    73,    95,    96,    -1,    21,
      95,    26,     3,    96,    -1,     3,    -1,   107,    94,     3,
      -1,    -1,   109,    -1,    74,   110,    -1,   111,    -1,   110,
      77,   111,    -1,    43,    95,     8,    96,    -1,   112,    80,
     113,    -1,   112,    81,   113,    -1,   112,    37,    95,   115,
      96,    -1,   112,    91,    37,    95,   115,    96,    -1,   112,
      37,     9,    -1,   112,    91,    37,     9,    -1,   112,    16,
     113,    77,   113,    -1,   112,    83,   113,    -1,   112,    82,
     113,    -1,   112,    84,   113,    -1,   112,    85,   113,    -1,
     112,    80,   114,    -1,   112,    81,   114,    -1,   112,    83,
     114,    -1,   112,    82,   114,    -1,   112,    16,   114,    77,
     114,    -1,   112,    84,   114,    -1,   112,    85,   114,    -1,
       3,    -1,     4,    -1,    21,    95,    88,    96,    -1,    73,
      95,    96,    -1,    36,    -1,     5,    -1,    87,     5,    -1,
       6,    -1,    87,     6,    -1,   113,    -1,   115,    94,   113,
      -1,    -1,   117,    -1,    35,    17,   112,    -1,    -1,   119,
      -1,    75,    35,    51,    17,   122,    -1,    -1,   121,    -1,
      51,    17,   122,    -1,    51,    17,    52,    95,    96,    -1,
     123,    -1,   122,    94,   123,    -1,   112,    -1,   112,    12,
      -1,   112,    24,    -1,    -1,   125,    -1,    42,     5,    -1,
      42,     5,    94,     5,    -1,    -1,   127,    -1,    50,   128,
      -1,   129,    -1,   128,    94,   129,    -1,     3,    80,     3,
      -1,     3,    80,     5,    -1,     3,    80,    95,   130,    96,
      -1,     3,    80,     3,    95,     8,    96,    -1,   131,    -1,
     130,    94,   131,    -1,     3,    80,   113,    -1,     3,    -1,
       4,    -1,    36,    -1,     5,    -1,     6,    -1,     9,    -1,
      87,   132,    -1,    91,   132,    -1,   132,    86,   132,    -1,
     132,    87,   132,    -1,   132,    88,   132,    -1,   132,    89,
     132,    -1,   132,    82,   132,    -1,   132,    83,   132,    -1,
     132,    79,   132,    -1,   132,    78,   132,    -1,   132,    90,
     132,    -1,   132,    27,   132,    -1,   132,    47,   132,    -1,
     132,    85,   132,    -1,   132,    84,   132,    -1,   132,    80,
     132,    -1,   132,    81,   132,    -1,   132,    77,   132,    -1,
     132,    76,   132,    -1,    95,   132,    96,    -1,   133,    -1,
       3,    95,   134,    96,    -1,    37,    95,   134,    96,    -1,
       3,    95,    96,    -1,    46,    95,   132,    94,   132,    96,
      -1,    44,    95,   132,    94,   132,    96,    -1,   135,    -1,
     134,    94,   135,    -1,   132,    -1,     8,    -1,    60,   137,
      -1,    72,    -1,    63,    -1,    45,    -1,     3,    -1,    49,
      -1,     8,    -1,     5,    -1,     6,    -1,    58,     3,    80,
     142,    -1,    58,     3,    80,   141,    -1,    58,     3,    80,
      49,    -1,    58,    48,   138,    -1,    58,    10,    80,   138,
      -1,    58,    34,     9,    80,    95,   115,    96,    -1,    58,
      34,     3,    80,   141,    -1,     3,    -1,     8,    -1,    68,
      -1,    29,    -1,   113,    -1,    20,    -1,    55,    -1,   144,
      -1,    15,    -1,    62,    67,    -1,   146,    41,     3,   147,
      70,   149,    -1,    39,    -1,    53,    -1,    -1,    95,   148,
      96,    -1,   112,    -1,   148,    94,   112,    -1,   150,    -1,
     149,    94,   150,    -1,    95,   151,    96,    -1,   152,    -1,
     151,    94,   152,    -1,   113,    -1,   114,    -1,     8,    -1,
      95,   115,    96,    -1,    23,    32,     3,    74,    36,    80,
     113,    -1,    23,    32,     3,    74,    36,    37,    95,   115,
      96,    -1,    18,     3,    95,   151,   155,    96,    -1,    -1,
      94,   156,    -1,   157,    -1,   156,    94,   157,    -1,   152,
     158,   159,    -1,    -1,    11,    -1,     3,    -1,    42,    -1,
     161,     3,    -1,    25,    -1,    24,    -1,    60,    65,    -1,
      69,   107,    58,   164,   109,    -1,   165,    -1,   164,    94,
     165,    -1,     3,    80,   113,    -1,     3,    80,   114,    -1,
       3,    80,    95,   115,    96,    -1,     3,    80,    95,    96,
      -1,    60,   167,    71,    -1,    -1,    59,    -1,    60,    19,
      -1,    22,    33,     3,    54,   170,    61,     8,    -1,    40,
      -1,    30,    -1,    28,    33,     3,    -1,    13,    38,     3,
      66,    56,     3,    -1,    31,    56,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   111,   111,   112,   113,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     137,   138,   142,   143,   147,   162,   163,   167,   168,   171,
     173,   174,   178,   179,   180,   181,   182,   183,   184,   185,
     189,   190,   193,   195,   199,   203,   204,   208,   213,   220,
     228,   236,   245,   250,   255,   260,   265,   270,   275,   280,
     281,   282,   283,   288,   293,   298,   306,   307,   312,   318,
     324,   333,   334,   338,   339,   343,   349,   355,   357,   361,
     368,   370,   374,   380,   382,   386,   390,   397,   398,   402,
     403,   404,   407,   409,   413,   418,   425,   427,   431,   435,
     436,   440,   445,   450,   456,   464,   469,   476,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   502,   503,   504,   505,   506,   507,
     508,   509,   510,   511,   512,   516,   517,   518,   519,   520,
     524,   525,   529,   530,   536,   540,   541,   542,   548,   549,
     550,   551,   552,   556,   561,   566,   571,   572,   576,   581,
     589,   590,   594,   595,   596,   610,   611,   612,   616,   617,
     623,   631,   632,   635,   637,   641,   642,   646,   647,   651,
     655,   656,   660,   661,   662,   663,   669,   675,   687,   694,
     696,   700,   705,   709,   716,   718,   722,   723,   729,   737,
     738,   744,   750,   758,   759,   763,   767,   771,   775,   784,
     787,   788,   792,   798,   809,   810,   814,   825,   837
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
  "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FROM", 
  "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_ID", "TOK_IN", 
  "TOK_INDEX", "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_LIMIT", 
  "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", 
  "TOK_NULL", "TOK_OPTION", "TOK_ORDER", "TOK_RAND", "TOK_REPLACE", 
  "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SET", 
  "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", "TOK_START", "TOK_STATUS", 
  "TOK_SUM", "TOK_TABLES", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", 
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
  "opt_call_opts_list", "call_opts_list", "call_opt", "opt_as", 
  "call_opt_name", "describe", "describe_tok", "show_tables", "update", 
  "update_items_list", "update_item", "show_variables", "opt_session", 
  "show_collation", "create_function", "udf_type", "drop_function", 
  "attach_index", "flush_rtindex", 0
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
     325,   326,   327,   328,   329,   330,   331,   332,   124,    38,
      61,   333,    60,    62,   334,   335,    43,    45,    42,    47,
      37,   336,   337,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    97,    98,    98,    98,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
     100,   100,   101,   101,   102,   103,   103,   104,   104,   105,
     105,   105,   106,   106,   106,   106,   106,   106,   106,   106,
     107,   107,   108,   108,   109,   110,   110,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
     111,   111,   111,   111,   111,   111,   112,   112,   112,   112,
     112,   113,   113,   114,   114,   115,   115,   116,   116,   117,
     118,   118,   119,   120,   120,   121,   121,   122,   122,   123,
     123,   123,   124,   124,   125,   125,   126,   126,   127,   128,
     128,   129,   129,   129,   129,   130,   130,   131,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   133,   133,   133,   133,   133,
     134,   134,   135,   135,   136,   137,   137,   137,   138,   138,
     138,   138,   138,   139,   139,   139,   139,   139,   140,   140,
     141,   141,   142,   142,   142,   143,   143,   143,   144,   144,
     145,   146,   146,   147,   147,   148,   148,   149,   149,   150,
     151,   151,   152,   152,   152,   152,   153,   153,   154,   155,
     155,   156,   156,   157,   158,   158,   159,   159,   160,   161,
     161,   162,   163,   164,   164,   165,   165,   165,   165,   166,
     167,   167,   168,   169,   170,   170,   171,   172,   173
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     1,    10,     1,     3,     1,     2,     0,
       1,     2,     1,     4,     4,     4,     4,     4,     3,     5,
       1,     3,     0,     1,     2,     1,     3,     4,     3,     3,
       5,     6,     3,     4,     5,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     3,     3,     1,     1,     4,     3,
       1,     1,     2,     1,     2,     1,     3,     0,     1,     3,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     6,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     4,     4,     3,     6,     6,
       1,     3,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     4,     4,     3,     4,     7,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       6,     1,     1,     0,     3,     1,     3,     1,     3,     3,
       1,     3,     1,     1,     1,     3,     7,     9,     6,     0,
       2,     1,     3,     3,     0,     1,     1,     1,     2,     1,
       1,     2,     5,     1,     3,     3,     3,     5,     4,     3,
       0,     1,     2,     7,     1,     1,     3,     6,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,   168,     0,   165,     0,     0,   200,   199,     0,
       0,   171,   172,   166,     0,     0,   210,     0,     0,     0,
       2,     3,    20,    22,    23,     7,     8,     9,   167,     5,
       0,     6,    10,    11,     0,    12,    13,    14,    15,    16,
      17,    18,    19,     0,     0,     0,     0,     0,     0,   108,
     109,   111,   112,   113,     0,     0,   110,     0,     0,     0,
       0,     0,     0,    27,     0,     0,     0,    25,    29,    32,
     134,     0,     0,     0,     0,   212,   147,   211,   146,   201,
     145,   144,     0,   169,    40,     0,     1,     4,     0,   198,
       0,     0,     0,     0,   216,   218,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   114,   115,     0,     0,
       0,    30,     0,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   148,   151,   152,   150,   149,
     156,   209,     0,     0,     0,    21,   173,     0,    71,    73,
     184,     0,     0,   182,   183,   189,   180,     0,     0,   143,
     137,   142,     0,   140,     0,     0,     0,     0,     0,     0,
       0,    38,     0,     0,   133,    42,    26,    31,   125,   126,
     132,   131,   123,   122,   129,   130,   120,   121,   128,   127,
     116,   117,   118,   119,   124,   160,   161,   163,   155,   162,
       0,   164,   154,   153,   157,     0,     0,     0,     0,   203,
      41,     0,     0,     0,    72,    74,    75,     0,     0,     0,
     215,   214,     0,     0,     0,   135,    33,     0,    37,   136,
       0,    34,     0,    35,    36,     0,     0,     0,    77,    43,
     159,     0,     0,     0,   202,    66,    67,     0,    70,     0,
     175,     0,     0,   217,     0,   185,   181,   190,   191,   188,
       0,     0,     0,   141,    39,     0,     0,     0,    44,    45,
       0,     0,    80,    78,     0,     0,   205,   206,   204,     0,
       0,     0,   174,     0,   170,   177,    76,   195,     0,     0,
     213,     0,   186,   139,   138,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,    81,
     158,   208,     0,     0,    69,   176,     0,     0,   196,   197,
     193,   194,   192,     0,     0,    46,     0,     0,    52,     0,
      48,    59,    49,    60,    56,    62,    55,    61,    57,    64,
      58,    65,     0,    79,     0,     0,    92,    84,   207,    68,
       0,   179,   178,   187,    47,     0,     0,     0,    53,     0,
       0,     0,     0,    96,    93,   181,    54,     0,    63,    50,
       0,     0,     0,    89,    85,    87,    94,     0,    24,    97,
      51,    82,     0,    90,    91,     0,     0,     0,    98,    99,
      86,    88,    95,     0,     0,   101,   102,     0,   100,     0,
       0,     0,   105,     0,     0,     0,   103,   104,   107,   106
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    19,    20,    21,    22,    23,    66,    67,   113,    68,
      85,   238,   239,   268,   269,   373,   216,   154,   217,   272,
     273,   308,   309,   346,   347,   374,   375,   363,   364,   378,
     379,   388,   389,   401,   402,   161,    70,   162,   163,    24,
      81,   140,    25,    26,   202,   203,    27,    28,    29,    30,
     212,   251,   284,   285,   155,   156,    31,    32,   219,   257,
     258,   288,   320,    33,    34,    35,    36,   208,   209,    37,
      82,    38,    39,   222,    40,    41,    42
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -239
static const short yypact[] =
{
     657,     8,  -239,    55,  -239,    37,     4,  -239,  -239,    42,
      30,  -239,  -239,  -239,   179,    23,   246,    57,   126,   131,
    -239,    47,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,
     108,  -239,  -239,  -239,   151,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,  -239,   152,    66,   170,   178,   183,   188,   104,
    -239,  -239,  -239,  -239,   106,   107,  -239,   109,   110,   112,
     135,   136,   244,  -239,   244,   244,   -14,  -239,    44,   467,
    -239,   123,   141,    76,    19,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,   155,  -239,  -239,   -44,  -239,   -15,   219,  -239,
     166,     5,   180,   159,  -239,  -239,   133,   244,    -7,   232,
     244,   244,   244,   146,   156,   160,  -239,  -239,   313,   126,
     179,  -239,   243,  -239,   244,   244,   244,   244,   244,   244,
     244,   244,   244,   244,   244,   244,   244,   244,   244,   244,
     244,   127,    19,   174,   176,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,   254,   255,    81,  -239,   165,   205,  -239,  -239,
    -239,    77,     7,  -239,  -239,   169,  -239,    -9,   237,  -239,
    -239,   467,    29,  -239,   334,   272,   196,    51,   269,   290,
     358,  -239,   244,   244,  -239,   -51,  -239,  -239,  -239,  -239,
     491,   512,   535,   556,   579,   579,   561,   561,   561,   561,
     -12,   -12,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,
     274,  -239,  -239,  -239,  -239,    53,   199,   215,   -40,  -239,
    -239,   241,   227,   295,  -239,  -239,  -239,    52,     5,   203,
    -239,  -239,   239,    -8,   232,  -239,  -239,   206,  -239,  -239,
     244,  -239,   244,  -239,  -239,   423,   446,   154,   266,  -239,
    -239,     7,    33,   254,  -239,  -239,  -239,   208,  -239,   211,
    -239,    56,   212,  -239,     7,  -239,    17,   214,  -239,  -239,
     302,   217,     7,  -239,  -239,   379,   402,   220,   245,  -239,
     202,   304,   249,  -239,    68,     3,  -239,  -239,  -239,   238,
     229,   241,  -239,     5,   234,  -239,  -239,  -239,    27,     5,
    -239,     7,  -239,  -239,  -239,   321,   154,    11,    -2,    11,
      11,    11,    11,    11,    11,   293,   241,   297,   282,  -239,
    -239,  -239,    71,   240,  -239,  -239,    72,   212,  -239,  -239,
    -239,   323,  -239,    78,   242,  -239,   265,   267,  -239,     7,
    -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,     0,  -239,   292,   345,   322,  -239,  -239,  -239,
       5,  -239,  -239,  -239,  -239,     7,    35,    84,  -239,     7,
     365,   268,   378,   337,  -239,  -239,  -239,   382,  -239,  -239,
      93,   241,   309,    20,   314,  -239,   331,   404,  -239,  -239,
    -239,   314,   332,  -239,  -239,   241,   422,   351,   338,  -239,
    -239,  -239,  -239,     1,   404,   356,  -239,   430,  -239,   444,
     373,   100,  -239,   375,     7,   430,  -239,  -239,  -239,  -239
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -239,  -239,  -239,  -239,   385,  -239,  -239,   364,  -239,  -239,
     367,  -239,   287,  -239,   181,  -163,   -91,  -237,  -238,  -239,
    -239,  -239,  -239,  -239,  -239,   125,   130,  -239,  -239,  -239,
    -239,  -239,   103,  -239,   111,   -13,  -239,   420,   296,  -239,
    -239,   389,  -239,  -239,   336,  -239,  -239,  -239,  -239,  -239,
    -239,  -239,  -239,   225,   275,  -216,  -239,  -239,  -239,  -239,
     248,  -239,  -239,  -239,  -239,  -239,  -239,  -239,   317,  -239,
    -239,  -239,  -239,  -239,  -239,  -239,  -239
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -195
static const short yytable[] =
{
     153,    69,   256,   274,   395,   277,   396,   328,   148,   358,
     148,   149,   148,   150,   142,   114,   148,   149,   109,   165,
    -194,   220,   135,   237,   136,   137,    71,   138,   287,   261,
     318,   221,   383,    72,   237,   115,    46,   312,   148,   149,
     201,   149,    14,   143,   384,   144,    43,   111,   250,   106,
     143,   107,   108,   323,   243,   112,   195,    73,    44,  -194,
     327,   196,   331,   333,   335,   337,   339,   341,   139,   319,
      45,    74,   262,   321,   270,    47,   128,   129,   130,   133,
     110,   166,   214,   215,   164,   134,    48,   168,   169,   170,
     200,   357,   151,   329,   200,   359,   397,    69,   151,   311,
     152,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   315,   368,
     151,   370,   367,   224,    83,   225,    76,   153,   275,    84,
     195,    86,   148,   270,   365,   196,    49,    50,    51,    52,
      87,   159,    53,   343,    78,   224,   254,   229,   255,    88,
     281,   276,   282,    80,    89,    90,   197,   245,   246,   235,
     236,    91,   254,   286,   310,   254,   350,   348,   351,    56,
      57,   292,   254,    92,   353,   247,   198,   104,   254,   105,
     369,    93,    49,    50,    51,    52,    94,   254,    53,   380,
     248,    95,   153,    54,   405,   199,   406,   267,   153,    96,
      55,    97,    98,   131,    99,   100,   326,   101,   330,   332,
     334,   336,   338,   340,   200,    56,    57,   265,   297,   266,
      62,   132,   146,    58,    64,    59,   141,   249,    65,   160,
     102,   103,   147,   158,   157,    49,    50,    51,    52,   298,
     159,    53,   171,    60,   245,   246,   177,    49,    50,    51,
      52,   172,    61,    53,   205,   173,   206,   207,   210,   153,
     211,   213,   247,   218,   366,    75,    62,    63,    56,    57,
      64,   245,   246,   223,    65,   227,   104,   248,   105,   214,
      56,    57,   299,   300,   301,   302,   303,   304,   104,   247,
     105,    76,   228,   305,   241,   242,   114,   252,   253,   259,
     260,   271,   264,   279,   248,    77,   280,   283,   289,    78,
     290,    79,   291,   408,   249,   295,   115,   114,    80,    62,
     372,   306,   296,    64,   307,   314,   313,    65,   317,   324,
     342,    62,   344,   345,   287,    64,   349,   115,   354,    65,
     114,   249,   355,   360,   356,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     115,   114,   361,   230,   362,   231,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   115,   371,   376,   232,   114,   233,   377,   215,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   382,   115,   114,   387,   385,   174,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   386,   115,   392,   390,   114,
     226,   393,   394,   400,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   115,
     114,   399,   403,   404,   234,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     115,   407,   145,   114,   176,   293,   175,   325,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   115,   114,   244,   381,   398,   294,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   115,   391,   409,   230,   114,   167,
     263,   204,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   322,   115,   114,
     232,   240,   352,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   316,   115,
     278,     0,   114,     0,     0,     0,     0,     0,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   115,   114,     0,     0,     0,     0,   114,     0,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   115,     0,     0,   114,     0,   115,     0,
       0,     0,     0,     0,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   115,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   126,   127,   128,
     129,   130,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   122,   123,   124,   125,   126,   127,   128,   129,   130,
       1,     0,     2,     0,     0,     3,     0,     4,     0,     5,
       6,     7,     8,     0,     0,     9,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,    11,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,     0,    13,     0,    14,    15,     0,    16,     0,    17,
       0,     0,     0,     0,     0,     0,    18
};

static const short yycheck[] =
{
      91,    14,   218,   241,     3,   242,     5,     9,     5,     9,
       5,     6,     5,     8,    58,    27,     5,     6,    32,    26,
       3,    30,     3,    74,     5,     6,     3,     8,    11,    37,
       3,    40,    12,    10,    74,    47,    32,   275,     5,     6,
     131,     6,    57,    94,    24,    60,    38,     3,   211,    62,
      94,    64,    65,   291,    94,    11,     3,    34,     3,    42,
     297,     8,   299,   300,   301,   302,   303,   304,    49,    42,
      33,    48,    80,   289,   237,    33,    88,    89,    90,     3,
      94,    88,     5,     6,    97,     9,    56,   100,   101,   102,
      87,   329,    87,    95,    87,    95,    95,   110,    87,    96,
      95,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   281,   356,
      87,   359,    87,    94,    67,    96,    45,   218,    95,     3,
       3,     0,     5,   296,   350,     8,     3,     4,     5,     6,
      93,     8,     9,   306,    63,    94,    94,    96,    96,    41,
      94,   242,    96,    72,     3,     3,    29,     3,     4,   172,
     173,    95,    94,   254,    96,    94,    94,    96,    96,    36,
      37,   262,    94,     3,    96,    21,    49,    44,    94,    46,
      96,     3,     3,     4,     5,     6,     3,    94,     9,    96,
      36,     3,   283,    14,    94,    68,    96,    43,   289,    95,
      21,    95,    95,    80,    95,    95,   297,    95,   299,   300,
     301,   302,   303,   304,    87,    36,    37,   230,    16,   232,
      87,    80,     3,    44,    91,    46,    71,    73,    95,    96,
      95,    95,    66,    74,    54,     3,     4,     5,     6,    37,
       8,     9,    96,    64,     3,     4,     3,     3,     4,     5,
       6,    95,    73,     9,    80,    95,    80,     3,     3,   350,
      95,    56,    21,    94,   355,    19,    87,    88,    36,    37,
      91,     3,     4,    36,    95,     3,    44,    36,    46,     5,
      36,    37,    80,    81,    82,    83,    84,    85,    44,    21,
      46,    45,    96,    91,    95,    80,    27,    70,     3,    96,
      61,    35,    96,    95,    36,    59,    95,    95,    94,    63,
       8,    65,    95,   404,    73,    95,    47,    27,    72,    87,
      52,    17,    77,    91,    75,    96,    88,    95,    94,     8,
      37,    87,    35,    51,    11,    91,    96,    47,    96,    95,
      27,    73,    77,    51,    77,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      47,    27,    17,    94,    42,    96,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    47,    17,     5,    94,    27,    96,    50,     6,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    95,    47,    27,     3,    94,    96,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    94,    47,     5,    96,    27,
      96,    80,    94,     3,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    47,
      27,    95,     8,    80,    96,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      47,    96,    87,    27,   110,    96,   109,   296,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    47,    27,   208,   371,   394,    96,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    47,   385,   405,    94,    27,    99,
     224,   132,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,   289,    47,    27,
      94,   205,   317,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,   283,    47,
     243,    -1,    27,    -1,    -1,    -1,    -1,    -1,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    47,    27,    -1,    -1,    -1,    -1,    27,    -1,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    47,    -1,    -1,    27,    -1,    47,    -1,
      -1,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    47,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    86,    87,    88,
      89,    90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      13,    -1,    15,    -1,    -1,    18,    -1,    20,    -1,    22,
      23,    24,    25,    -1,    -1,    28,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      53,    -1,    55,    -1,    57,    58,    -1,    60,    -1,    62,
      -1,    -1,    -1,    -1,    -1,    -1,    69
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    13,    15,    18,    20,    22,    23,    24,    25,    28,
      31,    39,    53,    55,    57,    58,    60,    62,    69,    98,
      99,   100,   101,   102,   136,   139,   140,   143,   144,   145,
     146,   153,   154,   160,   161,   162,   163,   166,   168,   169,
     171,   172,   173,    38,     3,    33,    32,    33,    56,     3,
       4,     5,     6,     9,    14,    21,    36,    37,    44,    46,
      64,    73,    87,    88,    91,    95,   103,   104,   106,   132,
     133,     3,    10,    34,    48,    19,    45,    59,    63,    65,
      72,   137,   167,    67,     3,   107,     0,    93,    41,     3,
       3,    95,     3,     3,     3,     3,    95,    95,    95,    95,
      95,    95,    95,    95,    44,    46,   132,   132,   132,    32,
      94,     3,    11,   105,    27,    47,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    80,    80,     3,     9,     3,     5,     6,     8,    49,
     138,    71,    58,    94,    60,   101,     3,    66,     5,     6,
       8,    87,    95,   113,   114,   151,   152,    54,    74,     8,
      96,   132,   134,   135,   132,    26,    88,   134,   132,   132,
     132,    96,    95,    95,    96,   107,   104,     3,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,     3,     8,    29,    49,    68,
      87,   113,   141,   142,   138,    80,    80,     3,   164,   165,
       3,    95,   147,    56,     5,     6,   113,   115,    94,   155,
      30,    40,   170,    36,    94,    96,    96,     3,    96,    96,
      94,    96,    94,    96,    96,   132,   132,    74,   108,   109,
     141,    95,    80,    94,   109,     3,     4,    21,    36,    73,
     112,   148,    70,     3,    94,    96,   152,   156,   157,    96,
      61,    37,    80,   135,    96,   132,   132,    43,   110,   111,
     112,    35,   116,   117,   115,    95,   113,   114,   165,    95,
      95,    94,    96,    95,   149,   150,   113,    11,   158,    94,
       8,    95,   113,    96,    96,    95,    77,    16,    37,    80,
      81,    82,    83,    84,    85,    91,    17,    75,   118,   119,
      96,    96,   115,    88,    96,   112,   151,    94,     3,    42,
     159,   152,   157,   115,     8,   111,   113,   114,     9,    95,
     113,   114,   113,   114,   113,   114,   113,   114,   113,   114,
     113,   114,    37,   112,    35,    51,   120,   121,    96,    96,
      94,    96,   150,    96,    96,    77,    77,   115,     9,    95,
      51,    17,    42,   124,   125,   152,   113,    87,   114,    96,
     115,    17,    52,   112,   122,   123,     5,    50,   126,   127,
      96,   122,    95,    12,    24,    94,    94,     3,   128,   129,
      96,   123,     5,    80,    94,     3,     5,    95,   129,    95,
       3,   130,   131,     8,    80,    94,    96,    96,   113,   131
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

  case 20:

    { pParser->PushQuery(); ;}
    break;

  case 21:

    { pParser->PushQuery(); ;}
    break;

  case 24:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 27:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 30:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 31:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 32:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 33:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 34:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 35:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 36:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 37:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 38:

    { if ( !pParser->AddItem ( "weight()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 39:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 41:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 47:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 48:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 49:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 50:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		;}
    break;

  case 51:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		;}
    break;

  case 52:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 53:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 54:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 55:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 56:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 ) )
				YYERROR;
		;}
    break;

  case 57:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX ) )
				YYERROR;
		;}
    break;

  case 58:

    {
			if ( !pParser->AddUintRangeFilter ( yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 62:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 63:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 64:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX ) )
				YYERROR;
		;}
    break;

  case 65:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue ) )
				YYERROR;
		;}
    break;

  case 67:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 68:

    {
			yyval.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 69:

    {
			yyval.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 70:

    {
			yyval.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 71:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 72:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 73:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 74:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 75:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 76:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 79:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 82:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 85:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 86:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 88:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 90:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 91:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 94:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 95:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
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
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 103:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 104:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-2], yyvsp[-1].m_sValue ) )
				YYERROR;
		;}
    break;

  case 105:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 106:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 107:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 109:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 110:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 114:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 115:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 133:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 135:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 136:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 137:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 138:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 139:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 145:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 146:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 147:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 153:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 154:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 155:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 156:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 157:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 158:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 159:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 162:

    { yyval.m_iValue = 1; ;}
    break;

  case 163:

    { yyval.m_iValue = 0; ;}
    break;

  case 164:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 165:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 166:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 170:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 171:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 172:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 175:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 176:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 179:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 180:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 181:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 182:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 183:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 184:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 185:

    { yyval.m_iInstype = TOK_CONST_MVA; yyval.m_iValue = yyvsp[-1].m_pValues->GetLength(); yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 186:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 187:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 188:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 191:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 193:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 197:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 198:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 202:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-3] ) )
				YYERROR;
		;}
    break;

  case 205:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0] );
		;}
    break;

  case 206:

    {
			pParser->UpdateAttr ( yyvsp[-2].m_sValue, &yyvsp[0], SPH_ATTR_FLOAT);
		;}
    break;

  case 207:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4].m_sValue, yyvsp[-1] );
		;}
    break;

  case 208:

    {
			pParser->UpdateAttr ( yyvsp[-3].m_sValue, NULL, SPH_ATTR_UINT32SET );
		;}
    break;

  case 209:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 212:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 213:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 214:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 215:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 216:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = yyvsp[0].m_sValue;
		;}
    break;

  case 217:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = yyvsp[-3].m_sValue;
			tStmt.m_sSetName = yyvsp[0].m_sValue;
		;}
    break;

  case 218:

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

