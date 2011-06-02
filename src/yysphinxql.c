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
     TOK_QUOTED_STRING = 262,
     TOK_USERVAR = 263,
     TOK_SYSVAR = 264,
     TOK_AS = 265,
     TOK_ASC = 266,
     TOK_AVG = 267,
     TOK_BEGIN = 268,
     TOK_BETWEEN = 269,
     TOK_BY = 270,
     TOK_CALL = 271,
     TOK_COLLATION = 272,
     TOK_COMMIT = 273,
     TOK_COUNT = 274,
     TOK_CREATE = 275,
     TOK_DELETE = 276,
     TOK_DESC = 277,
     TOK_DESCRIBE = 278,
     TOK_DISTINCT = 279,
     TOK_DIV = 280,
     TOK_DROP = 281,
     TOK_FALSE = 282,
     TOK_FLOAT = 283,
     TOK_FROM = 284,
     TOK_FUNCTION = 285,
     TOK_GLOBAL = 286,
     TOK_GROUP = 287,
     TOK_ID = 288,
     TOK_IN = 289,
     TOK_INSERT = 290,
     TOK_INT = 291,
     TOK_INTO = 292,
     TOK_LIMIT = 293,
     TOK_MATCH = 294,
     TOK_MAX = 295,
     TOK_META = 296,
     TOK_MIN = 297,
     TOK_MOD = 298,
     TOK_NAMES = 299,
     TOK_NULL = 300,
     TOK_OPTION = 301,
     TOK_ORDER = 302,
     TOK_REPLACE = 303,
     TOK_RETURNS = 304,
     TOK_ROLLBACK = 305,
     TOK_SELECT = 306,
     TOK_SET = 307,
     TOK_SHOW = 308,
     TOK_SONAME = 309,
     TOK_START = 310,
     TOK_STATUS = 311,
     TOK_SUM = 312,
     TOK_TABLES = 313,
     TOK_TRANSACTION = 314,
     TOK_TRUE = 315,
     TOK_UPDATE = 316,
     TOK_VALUES = 317,
     TOK_VARIABLES = 318,
     TOK_WARNINGS = 319,
     TOK_WEIGHT = 320,
     TOK_WHERE = 321,
     TOK_WITHIN = 322,
     TOK_OR = 323,
     TOK_AND = 324,
     TOK_NE = 325,
     TOK_GTE = 326,
     TOK_LTE = 327,
     TOK_NOT = 328,
     TOK_NEG = 329
   };
#endif
#define TOK_IDENT 258
#define TOK_ATIDENT 259
#define TOK_CONST_INT 260
#define TOK_CONST_FLOAT 261
#define TOK_QUOTED_STRING 262
#define TOK_USERVAR 263
#define TOK_SYSVAR 264
#define TOK_AS 265
#define TOK_ASC 266
#define TOK_AVG 267
#define TOK_BEGIN 268
#define TOK_BETWEEN 269
#define TOK_BY 270
#define TOK_CALL 271
#define TOK_COLLATION 272
#define TOK_COMMIT 273
#define TOK_COUNT 274
#define TOK_CREATE 275
#define TOK_DELETE 276
#define TOK_DESC 277
#define TOK_DESCRIBE 278
#define TOK_DISTINCT 279
#define TOK_DIV 280
#define TOK_DROP 281
#define TOK_FALSE 282
#define TOK_FLOAT 283
#define TOK_FROM 284
#define TOK_FUNCTION 285
#define TOK_GLOBAL 286
#define TOK_GROUP 287
#define TOK_ID 288
#define TOK_IN 289
#define TOK_INSERT 290
#define TOK_INT 291
#define TOK_INTO 292
#define TOK_LIMIT 293
#define TOK_MATCH 294
#define TOK_MAX 295
#define TOK_META 296
#define TOK_MIN 297
#define TOK_MOD 298
#define TOK_NAMES 299
#define TOK_NULL 300
#define TOK_OPTION 301
#define TOK_ORDER 302
#define TOK_REPLACE 303
#define TOK_RETURNS 304
#define TOK_ROLLBACK 305
#define TOK_SELECT 306
#define TOK_SET 307
#define TOK_SHOW 308
#define TOK_SONAME 309
#define TOK_START 310
#define TOK_STATUS 311
#define TOK_SUM 312
#define TOK_TABLES 313
#define TOK_TRANSACTION 314
#define TOK_TRUE 315
#define TOK_UPDATE 316
#define TOK_VALUES 317
#define TOK_VARIABLES 318
#define TOK_WARNINGS 319
#define TOK_WEIGHT 320
#define TOK_WHERE 321
#define TOK_WITHIN 322
#define TOK_OR 323
#define TOK_AND 324
#define TOK_NE 325
#define TOK_GTE 326
#define TOK_LTE 327
#define TOK_NOT 328
#define TOK_NEG 329




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
#define YYFINAL  78
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   618

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  74
/* YYNRULES -- Number of rules. */
#define YYNRULES  206
/* YYNRULES -- Number of states. */
#define YYNSTATES  386

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   329

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    82,    71,     2,
      87,    88,    80,    78,    86,    79,     2,    81,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    85,
      74,    72,    75,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    70,     2,     2,     2,     2,     2,
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
      65,    66,    67,    68,    69,    73,    76,    77,    83,    84
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
     257,   263,   264,   266,   270,   272,   276,   278,   281,   284,
     285,   287,   290,   295,   296,   298,   301,   303,   307,   311,
     315,   321,   323,   327,   331,   333,   335,   337,   339,   341,
     343,   346,   349,   353,   357,   361,   365,   369,   373,   377,
     381,   385,   389,   393,   397,   401,   405,   409,   413,   417,
     421,   423,   428,   433,   437,   444,   451,   453,   457,   459,
     461,   464,   466,   468,   470,   472,   474,   476,   478,   480,
     485,   490,   495,   499,   504,   512,   518,   520,   522,   524,
     526,   528,   530,   532,   534,   536,   539,   546,   548,   550,
     551,   555,   557,   561,   563,   567,   571,   573,   577,   579,
     581,   583,   591,   601,   608,   609,   612,   614,   618,   622,
     623,   625,   627,   629,   632,   634,   636,   639,   648,   650,
     654,   658,   661,   664,   672,   674,   676
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      90,     0,    -1,    91,    -1,    92,    -1,    92,    85,    -1,
     137,    -1,   145,    -1,   131,    -1,   132,    -1,   135,    -1,
     146,    -1,   152,    -1,   154,    -1,   155,    -1,   158,    -1,
     159,    -1,   160,    -1,   162,    -1,    93,    -1,    92,    85,
      93,    -1,    94,    -1,   128,    -1,    51,    95,    29,    99,
     100,   108,   110,   112,   116,   118,    -1,    96,    -1,    95,
      86,    96,    -1,    80,    -1,    98,    97,    -1,    -1,     3,
      -1,    10,     3,    -1,   124,    -1,    12,    87,   124,    88,
      -1,    40,    87,   124,    88,    -1,    42,    87,   124,    88,
      -1,    57,    87,   124,    88,    -1,    19,    87,    80,    88,
      -1,    65,    87,    88,    -1,    19,    87,    24,     3,    88,
      -1,     3,    -1,    99,    86,     3,    -1,    -1,   101,    -1,
      66,   102,    -1,   103,    -1,   102,    69,   103,    -1,    39,
      87,     7,    88,    -1,   104,    72,   105,    -1,   104,    73,
     105,    -1,   104,    34,    87,   107,    88,    -1,   104,    83,
      34,    87,   107,    88,    -1,   104,    34,     8,    -1,   104,
      83,    34,     8,    -1,   104,    14,   105,    69,   105,    -1,
     104,    75,   105,    -1,   104,    74,   105,    -1,   104,    76,
     105,    -1,   104,    77,   105,    -1,   104,    72,   106,    -1,
     104,    73,   106,    -1,   104,    75,   106,    -1,   104,    74,
     106,    -1,   104,    14,   106,    69,   106,    -1,   104,    76,
     106,    -1,   104,    77,   106,    -1,     3,    -1,     4,    -1,
      19,    87,    80,    88,    -1,    65,    87,    88,    -1,    33,
      -1,     5,    -1,    79,     5,    -1,     6,    -1,    79,     6,
      -1,   105,    -1,   107,    86,   105,    -1,    -1,   109,    -1,
      32,    15,   104,    -1,    -1,   111,    -1,    67,    32,    47,
      15,   114,    -1,    -1,   113,    -1,    47,    15,   114,    -1,
     115,    -1,   114,    86,   115,    -1,   104,    -1,   104,    11,
      -1,   104,    22,    -1,    -1,   117,    -1,    38,     5,    -1,
      38,     5,    86,     5,    -1,    -1,   119,    -1,    46,   120,
      -1,   121,    -1,   120,    86,   121,    -1,     3,    72,     3,
      -1,     3,    72,     5,    -1,     3,    72,    87,   122,    88,
      -1,   123,    -1,   122,    86,   123,    -1,     3,    72,   105,
      -1,     3,    -1,     4,    -1,    33,    -1,     5,    -1,     6,
      -1,     8,    -1,    79,   124,    -1,    83,   124,    -1,   124,
      78,   124,    -1,   124,    79,   124,    -1,   124,    80,   124,
      -1,   124,    81,   124,    -1,   124,    74,   124,    -1,   124,
      75,   124,    -1,   124,    71,   124,    -1,   124,    70,   124,
      -1,   124,    82,   124,    -1,   124,    25,   124,    -1,   124,
      43,   124,    -1,   124,    77,   124,    -1,   124,    76,   124,
      -1,   124,    72,   124,    -1,   124,    73,   124,    -1,   124,
      69,   124,    -1,   124,    68,   124,    -1,    87,   124,    88,
      -1,   125,    -1,     3,    87,   126,    88,    -1,    34,    87,
     126,    88,    -1,     3,    87,    88,    -1,    42,    87,   124,
      86,   124,    88,    -1,    40,    87,   124,    86,   124,    88,
      -1,   127,    -1,   126,    86,   127,    -1,   124,    -1,     7,
      -1,    53,   129,    -1,    64,    -1,    56,    -1,    41,    -1,
       3,    -1,    45,    -1,     7,    -1,     5,    -1,     6,    -1,
      52,     3,    72,   134,    -1,    52,     3,    72,   133,    -1,
      52,     3,    72,    45,    -1,    52,    44,   130,    -1,    52,
       9,    72,   130,    -1,    52,    31,     8,    72,    87,   107,
      88,    -1,    52,    31,     3,    72,   133,    -1,     3,    -1,
       7,    -1,    60,    -1,    27,    -1,   105,    -1,    18,    -1,
      50,    -1,   136,    -1,    13,    -1,    55,    59,    -1,   138,
      37,     3,   139,    62,   141,    -1,    35,    -1,    48,    -1,
      -1,    87,   140,    88,    -1,   104,    -1,   140,    86,   104,
      -1,   142,    -1,   141,    86,   142,    -1,    87,   143,    88,
      -1,   144,    -1,   143,    86,   144,    -1,   105,    -1,   106,
      -1,     7,    -1,    21,    29,     3,    66,    33,    72,   105,
      -1,    21,    29,     3,    66,    33,    34,    87,   107,    88,
      -1,    16,     3,    87,   143,   147,    88,    -1,    -1,    86,
     148,    -1,   149,    -1,   148,    86,   149,    -1,   144,   150,
     151,    -1,    -1,    10,    -1,     3,    -1,    38,    -1,   153,
       3,    -1,    23,    -1,    22,    -1,    53,    58,    -1,    61,
       3,    52,   156,    66,    33,    72,   105,    -1,   157,    -1,
     156,    86,   157,    -1,     3,    72,   105,    -1,    53,    63,
      -1,    53,    17,    -1,    20,    30,     3,    49,   161,    54,
       7,    -1,    36,    -1,    28,    -1,    26,    30,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   103,   103,   104,   105,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   127,   128,
     132,   133,   137,   152,   153,   157,   158,   161,   163,   164,
     168,   169,   170,   171,   172,   173,   174,   175,   180,   181,
     184,   186,   190,   194,   195,   199,   204,   211,   219,   227,
     236,   241,   246,   251,   256,   261,   266,   271,   272,   273,
     274,   279,   284,   289,   297,   298,   303,   309,   315,   324,
     325,   329,   330,   334,   340,   346,   348,   352,   359,   361,
     365,   371,   373,   377,   384,   385,   389,   390,   391,   394,
     396,   400,   405,   412,   414,   418,   422,   423,   427,   432,
     437,   446,   451,   458,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   498,   499,   500,   501,   502,   506,   507,   511,   512,
     518,   522,   523,   524,   530,   531,   532,   533,   534,   538,
     543,   548,   553,   554,   558,   563,   571,   572,   576,   577,
     578,   592,   593,   594,   598,   599,   605,   613,   614,   617,
     619,   623,   624,   628,   629,   633,   637,   638,   642,   643,
     644,   650,   656,   668,   675,   677,   681,   686,   690,   697,
     699,   703,   704,   710,   718,   719,   725,   731,   742,   743,
     747,   761,   765,   771,   782,   783,   787
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_ATIDENT", 
  "TOK_CONST_INT", "TOK_CONST_FLOAT", "TOK_QUOTED_STRING", "TOK_USERVAR", 
  "TOK_SYSVAR", "TOK_AS", "TOK_ASC", "TOK_AVG", "TOK_BEGIN", 
  "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_COLLATION", "TOK_COMMIT", 
  "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", "TOK_FLOAT", 
  "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_ID", 
  "TOK_IN", "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_LIMIT", "TOK_MATCH", 
  "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NAMES", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_SELECT", "TOK_SET", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_SUM", "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", 
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
  "update_items_list", "update_item", "show_variables", "show_collation", 
  "create_function", "udf_type", "drop_function", 0
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
     124,    38,    61,   325,    60,    62,   326,   327,    43,    45,
      42,    47,    37,   328,   329,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    89,    90,    90,    90,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    92,    92,
      93,    93,    94,    95,    95,    96,    96,    97,    97,    97,
      98,    98,    98,    98,    98,    98,    98,    98,    99,    99,
     100,   100,   101,   102,   102,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   104,   104,   104,   104,   104,   105,
     105,   106,   106,   107,   107,   108,   108,   109,   110,   110,
     111,   112,   112,   113,   114,   114,   115,   115,   115,   116,
     116,   117,   117,   118,   118,   119,   120,   120,   121,   121,
     121,   122,   122,   123,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   125,   125,   125,   125,   125,   126,   126,   127,   127,
     128,   129,   129,   129,   130,   130,   130,   130,   130,   131,
     131,   131,   131,   131,   132,   132,   133,   133,   134,   134,
     134,   135,   135,   135,   136,   136,   137,   138,   138,   139,
     139,   140,   140,   141,   141,   142,   143,   143,   144,   144,
     144,   145,   145,   146,   147,   147,   148,   148,   149,   150,
     150,   151,   151,   152,   153,   153,   154,   155,   156,   156,
     157,   158,   159,   160,   161,   161,   162
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
       5,     0,     1,     3,     1,     3,     1,     2,     2,     0,
       1,     2,     4,     0,     1,     2,     1,     3,     3,     3,
       5,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     4,     4,     3,     6,     6,     1,     3,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       4,     4,     3,     4,     7,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     6,     1,     1,     0,
       3,     1,     3,     1,     3,     3,     1,     3,     1,     1,
       1,     7,     9,     6,     0,     2,     1,     3,     3,     0,
       1,     1,     1,     2,     1,     1,     2,     8,     1,     3,
       3,     2,     2,     7,     1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   164,     0,   161,     0,     0,   195,   194,     0,   167,
     168,   162,     0,     0,     0,     0,     0,     0,     2,     3,
      18,    20,    21,     7,     8,     9,   163,     5,     0,     6,
      10,    11,     0,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,   104,   105,   107,   108,   109,     0,     0,
     106,     0,     0,     0,     0,     0,     0,    25,     0,     0,
       0,    23,    27,    30,   130,     0,     0,     0,     0,   202,
     143,   142,   196,   201,   141,   140,   165,     0,     1,     4,
       0,   193,     0,     0,     0,   206,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   110,   111,     0,     0,
       0,    28,     0,    26,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   144,   147,   148,   146,   145,
     152,     0,     0,    19,   169,    69,    71,   180,     0,   178,
     179,   184,   176,     0,     0,   139,   133,   138,     0,   136,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
     129,    38,    40,    24,    29,   121,   122,   128,   127,   119,
     118,   125,   126,   116,   117,   124,   123,   112,   113,   114,
     115,   120,   156,   157,   159,   151,   158,     0,   160,   150,
     149,   153,     0,     0,     0,     0,   198,     0,     0,    70,
      72,     0,     0,   205,   204,     0,     0,     0,   131,    31,
       0,    35,   132,     0,    32,     0,    33,    34,     0,     0,
       0,     0,    75,    41,   155,     0,     0,     0,     0,    64,
      65,     0,    68,     0,   171,     0,     0,   177,   185,   186,
     183,     0,     0,     0,   137,    37,     0,     0,     0,    42,
      43,     0,    39,     0,    78,    76,    73,     0,   200,     0,
     199,     0,     0,     0,   170,     0,   166,   173,   190,     0,
       0,   203,     0,   181,   135,   134,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    81,
      79,     0,   154,     0,     0,    67,   172,     0,     0,   191,
     192,   188,   189,   187,     0,     0,    44,     0,     0,    50,
       0,    46,    57,    47,    58,    54,    60,    53,    59,    55,
      62,    56,    63,     0,    77,     0,     0,    89,    82,    74,
     197,    66,     0,   175,   174,   182,    45,     0,     0,     0,
      51,     0,     0,     0,     0,    93,    90,   177,    52,     0,
      61,    48,     0,     0,    86,    83,    84,    91,     0,    22,
      94,    49,    80,    87,    88,     0,     0,     0,    95,    96,
      85,    92,     0,     0,    98,    99,     0,    97,     0,     0,
     101,     0,     0,   100,   103,   102
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    60,    61,   103,    62,
     162,   222,   223,   249,   250,   354,   139,   140,   257,   254,
     255,   289,   290,   327,   328,   355,   356,   345,   346,   359,
     360,   368,   369,   379,   380,   147,    64,   148,   149,    22,
      75,   130,    23,    24,   189,   190,    25,    26,    27,    28,
     198,   235,   266,   267,   141,   142,    29,    30,   202,   238,
     239,   269,   301,    31,    32,    33,    34,   195,   196,    35,
      36,    37,   205,    38
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -241
static const short yypact[] =
{
     557,  -241,    22,  -241,    18,    74,  -241,  -241,    83,  -241,
    -241,  -241,   179,   111,   492,    57,   137,   117,  -241,    36,
    -241,  -241,  -241,  -241,  -241,  -241,  -241,  -241,   144,  -241,
    -241,  -241,   187,  -241,  -241,  -241,  -241,  -241,  -241,   107,
     198,   208,   211,   131,  -241,  -241,  -241,  -241,   135,   136,
    -241,   150,   158,   161,   162,   164,   163,  -241,   163,   163,
       7,  -241,    39,   446,  -241,   167,   185,    66,    55,  -241,
    -241,  -241,  -241,  -241,  -241,  -241,  -241,   212,  -241,    20,
     280,  -241,    33,   240,   241,  -241,     1,   163,    28,    11,
     163,   163,   163,   218,   238,   239,  -241,  -241,   262,   326,
     179,  -241,   342,  -241,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   163,   163,   163,   163,
     163,    23,    55,   275,   276,  -241,  -241,  -241,  -241,  -241,
    -241,   348,   151,  -241,   281,  -241,  -241,  -241,   138,  -241,
    -241,   283,  -241,    27,   338,  -241,  -241,   446,    87,  -241,
     284,   383,   299,    88,   200,   222,   324,  -241,   163,   163,
    -241,  -241,   -39,  -241,  -241,  -241,  -241,   242,   303,   385,
     465,   487,   487,   174,   174,   174,   174,    72,    72,  -241,
    -241,  -241,  -241,  -241,  -241,  -241,  -241,   384,  -241,  -241,
    -241,  -241,    30,   320,   318,   -20,  -241,   176,   346,  -241,
    -241,    33,   323,  -241,  -241,   375,   -13,    11,  -241,  -241,
     344,  -241,  -241,   163,  -241,   163,  -241,  -241,   406,   425,
      53,   427,   419,  -241,  -241,    -3,    -3,   420,   348,  -241,
    -241,   365,  -241,   368,  -241,   100,   382,   155,   386,  -241,
    -241,   463,   404,    -3,  -241,  -241,   345,   366,   423,   444,
    -241,    73,  -241,   458,   462,  -241,  -241,   114,  -241,   459,
    -241,   452,   447,   176,  -241,    33,   448,  -241,  -241,    26,
      33,  -241,    -3,  -241,  -241,  -241,   529,    53,    17,    -5,
      17,    17,    17,    17,    17,    17,   515,   176,   519,   505,
    -241,    -3,  -241,    -3,   466,  -241,  -241,   118,   382,  -241,
    -241,  -241,   543,  -241,   122,   469,  -241,   489,   490,  -241,
      -3,  -241,  -241,  -241,  -241,  -241,  -241,  -241,  -241,  -241,
    -241,  -241,  -241,    12,  -241,   513,   556,   534,  -241,  -241,
    -241,  -241,    33,  -241,  -241,  -241,  -241,    -3,     6,   147,
    -241,    -3,   559,   176,   571,   535,  -241,  -241,  -241,   576,
    -241,  -241,   152,   176,    43,   498,  -241,   499,   583,  -241,
    -241,  -241,   498,  -241,  -241,   176,   582,   516,   503,  -241,
    -241,  -241,     8,   583,  -241,  -241,   587,  -241,   521,   175,
    -241,    -3,   587,  -241,  -241,  -241
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -241,  -241,  -241,  -241,   512,  -241,  -241,   494,  -241,  -241,
    -241,  -241,  -241,  -241,   319,  -196,  -121,   -54,  -240,  -241,
    -241,  -241,  -241,  -241,  -241,   244,   230,  -241,  -241,  -241,
    -241,  -241,   225,  -241,   217,    19,  -241,   511,   394,  -241,
    -241,   480,  -241,  -241,   411,  -241,  -241,  -241,  -241,  -241,
    -241,  -241,  -241,   306,   341,  -191,  -241,  -241,  -241,  -241,
     343,  -241,  -241,  -241,  -241,  -241,  -241,  -241,   387,  -241,
    -241,  -241,  -241,  -241
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -190
static const short yytable[] =
{
     188,   234,   135,   309,    43,    44,    45,    46,   145,    47,
     237,   374,   136,   375,    43,    44,    45,    46,   145,    47,
     340,   242,   135,   136,   251,    39,   182,   220,   135,   299,
     183,    63,   304,   182,    50,    51,    99,   183,   135,   136,
     137,    94,   101,    95,    50,    51,   227,   221,    40,   102,
     184,    94,   151,    95,   363,   203,   229,   230,   125,   243,
     126,   127,   128,   204,   300,   364,   228,   296,   185,   123,
     339,    12,   231,   132,   124,    96,   187,    97,    98,   302,
      56,   251,   310,   186,    58,   349,   232,   278,    59,   146,
      56,   324,   248,   100,    58,   376,   138,   104,    59,   341,
     129,   352,   187,    41,   256,   258,   150,   279,   152,   154,
     155,   156,   138,    42,    65,   105,    76,    78,   233,    63,
      66,    79,   273,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
      77,   347,    67,   199,   200,   280,   281,   282,   283,   284,
     285,   256,   118,   119,   120,    68,   286,   307,  -189,   311,
     313,   315,   317,   319,   321,   268,    43,    44,    45,    46,
     329,    47,   330,   207,   207,   208,   212,   218,   219,   229,
     230,    80,    43,    44,    45,    46,   263,    47,   264,   256,
      81,    48,    70,  -189,    82,   231,    50,    51,    49,   104,
     291,    83,   292,    94,   332,    95,   333,    71,   291,   232,
     335,    84,    50,    51,    85,    74,   348,   105,    86,    52,
     256,    53,    87,    88,   308,   104,   312,   314,   316,   318,
     320,   322,   246,   291,   247,   351,    54,    89,   291,   121,
     361,   233,    56,   105,    55,    90,    58,   104,    91,    92,
      59,    93,   116,   117,   118,   119,   120,   122,    56,    57,
     384,   382,    58,   383,   131,   105,    59,   104,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   134,   350,   105,   213,   104,   214,   143,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   105,   157,   144,   215,   104,
     216,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   158,   159,   105,   104,   161,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   164,   105,   192,   193,   104,
     160,   194,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   105,   197,   201,
     104,   206,   209,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   210,   211,   105,   199,
     226,   104,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   225,   236,   105,
     104,   240,   217,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   105,   241,
     252,   104,   245,   274,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   105,
     104,   253,   261,   259,   275,   262,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   105,   265,
     271,   104,   270,   287,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   105,
     104,   272,   213,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   105,    69,
     276,   215,   104,   277,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   288,
     105,   293,   294,    70,   298,   295,   305,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    71,   323,
      72,   325,   326,   268,   331,    73,    74,   336,   337,   338,
     342,   112,   113,   114,   115,   116,   117,   118,   119,   120,
       1,   343,   344,     2,   353,     3,   357,     4,     5,     6,
       7,   358,   200,     8,   365,   366,   367,   371,   372,   373,
     378,   133,     9,   381,   163,   370,   306,   362,   377,   385,
     153,   244,   191,   224,   334,    10,   297,    11,    12,    13,
      14,     0,    15,   303,     0,   260,     0,     0,    16
};

static const short yycheck[] =
{
     121,   197,     5,     8,     3,     4,     5,     6,     7,     8,
     201,     3,     6,     5,     3,     4,     5,     6,     7,     8,
       8,    34,     5,     6,   220,     3,     3,    66,     5,     3,
       7,    12,   272,     3,    33,    34,    29,     7,     5,     6,
       7,    40,     3,    42,    33,    34,    66,    86,    30,    10,
      27,    40,    24,    42,    11,    28,     3,     4,     3,    72,
       5,     6,     7,    36,    38,    22,    86,   263,    45,     3,
     310,    51,    19,    53,     8,    56,    79,    58,    59,   270,
      79,   277,    87,    60,    83,    79,    33,    14,    87,    88,
      79,   287,    39,    86,    83,    87,    79,    25,    87,    87,
      45,   341,    79,    29,   225,   226,    87,    34,    80,    90,
      91,    92,    79,    30,     3,    43,    59,     0,    65,   100,
       9,    85,   243,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
       3,   332,    31,     5,     6,    72,    73,    74,    75,    76,
      77,   272,    80,    81,    82,    44,    83,   278,     3,   280,
     281,   282,   283,   284,   285,    10,     3,     4,     5,     6,
     291,     8,   293,    86,    86,    88,    88,   158,   159,     3,
       4,    37,     3,     4,     5,     6,    86,     8,    88,   310,
       3,    12,    41,    38,    87,    19,    33,    34,    19,    25,
      86,     3,    88,    40,    86,    42,    88,    56,    86,    33,
      88,     3,    33,    34,     3,    64,   337,    43,    87,    40,
     341,    42,    87,    87,   278,    25,   280,   281,   282,   283,
     284,   285,   213,    86,   215,    88,    57,    87,    86,    72,
      88,    65,    79,    43,    65,    87,    83,    25,    87,    87,
      87,    87,    78,    79,    80,    81,    82,    72,    79,    80,
     381,    86,    83,    88,    52,    43,    87,    25,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     3,   338,    43,    86,    25,    88,    49,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    43,    88,    66,    86,    25,
      88,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    87,    87,    43,    25,     3,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     3,    43,    72,    72,    25,
      88,     3,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    43,    87,    86,
      25,    33,    88,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     3,    88,    43,     5,
      72,    25,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    87,    62,    43,
      25,    88,    88,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    43,    54,
       3,    25,    88,    88,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    43,
      25,    32,    87,    33,    88,    87,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    43,    87,
       7,    25,    86,    15,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    43,
      25,    87,    86,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    43,    17,
      87,    86,    25,    69,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    67,
      43,    72,    80,    41,    86,    88,     7,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    56,    34,
      58,    32,    47,    10,    88,    63,    64,    88,    69,    69,
      47,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      13,    15,    38,    16,    15,    18,     5,    20,    21,    22,
      23,    46,     6,    26,    86,    86,     3,     5,    72,    86,
       3,    79,    35,    72,   100,   365,   277,   353,   373,   382,
      89,   207,   122,   192,   298,    48,   265,    50,    51,    52,
      53,    -1,    55,   270,    -1,   228,    -1,    -1,    61
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    13,    16,    18,    20,    21,    22,    23,    26,    35,
      48,    50,    51,    52,    53,    55,    61,    90,    91,    92,
      93,    94,   128,   131,   132,   135,   136,   137,   138,   145,
     146,   152,   153,   154,   155,   158,   159,   160,   162,     3,
      30,    29,    30,     3,     4,     5,     6,     8,    12,    19,
      33,    34,    40,    42,    57,    65,    79,    80,    83,    87,
      95,    96,    98,   124,   125,     3,     9,    31,    44,    17,
      41,    56,    58,    63,    64,   129,    59,     3,     0,    85,
      37,     3,    87,     3,     3,     3,    87,    87,    87,    87,
      87,    87,    87,    87,    40,    42,   124,   124,   124,    29,
      86,     3,    10,    97,    25,    43,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    72,    72,     3,     8,     3,     5,     6,     7,    45,
     130,    52,    53,    93,     3,     5,     6,     7,    79,   105,
     106,   143,   144,    49,    66,     7,    88,   124,   126,   127,
     124,    24,    80,   126,   124,   124,   124,    88,    87,    87,
      88,     3,    99,    96,     3,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,     3,     7,    27,    45,    60,    79,   105,   133,
     134,   130,    72,    72,     3,   156,   157,    87,   139,     5,
       6,    86,   147,    28,    36,   161,    33,    86,    88,    88,
       3,    88,    88,    86,    88,    86,    88,    88,   124,   124,
      66,    86,   100,   101,   133,    87,    72,    66,    86,     3,
       4,    19,    33,    65,   104,   140,    62,   144,   148,   149,
      88,    54,    34,    72,   127,    88,   124,   124,    39,   102,
     103,   104,     3,    32,   108,   109,   105,   107,   105,    33,
     157,    87,    87,    86,    88,    87,   141,   142,    10,   150,
      86,     7,    87,   105,    88,    88,    87,    69,    14,    34,
      72,    73,    74,    75,    76,    77,    83,    15,    67,   110,
     111,    86,    88,    72,    80,    88,   104,   143,    86,     3,
      38,   151,   144,   149,   107,     7,   103,   105,   106,     8,
      87,   105,   106,   105,   106,   105,   106,   105,   106,   105,
     106,   105,   106,    34,   104,    32,    47,   112,   113,   105,
     105,    88,    86,    88,   142,    88,    88,    69,    69,   107,
       8,    87,    47,    15,    38,   116,   117,   144,   105,    79,
     106,    88,   107,    15,   104,   114,   115,     5,    46,   118,
     119,    88,   114,    11,    22,    86,    86,     3,   120,   121,
     115,     5,    72,    86,     3,     5,    87,   121,     3,   122,
     123,    72,    86,    88,   105,   123
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

  case 85:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 87:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 88:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 91:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 92:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 98:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
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
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 101:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 102:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 103:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 105:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 106:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 110:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 111:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 112:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 113:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 131:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 132:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 133:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 134:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 135:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 141:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 142:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 143:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 149:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 150:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 151:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 152:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 153:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 154:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 155:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 158:

    { yyval.m_iValue = 1; ;}
    break;

  case 159:

    { yyval.m_iValue = 0; ;}
    break;

  case 160:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 161:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 162:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 163:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 166:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 168:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 171:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 172:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 175:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 176:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 177:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 178:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 179:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 180:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 181:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 182:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 183:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 186:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 188:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 192:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 193:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 196:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 197:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_UPDATE;
			tStmt.m_sIndex = yyvsp[-6].m_sValue;	
			tStmt.m_tUpdate.m_dDocids.Add ( (SphDocID_t) yyvsp[0].m_iValue );
			tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );
		;}
    break;

  case 200:

    {
			CSphAttrUpdate & tUpd = pParser->m_pStmt->m_tUpdate;
			CSphColumnInfo & tAttr = tUpd.m_dAttrs.Add();
			tAttr.m_sName = yyvsp[-2].m_sValue;
			tAttr.m_sName.ToLower();
			tAttr.m_eAttrType = SPH_ATTR_INTEGER; // sorry, ints only for now, riding on legacy shit!
			tUpd.m_dPool.Add ( (DWORD) yyvsp[0].m_iValue );
		;}
    break;

  case 201:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 202:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 203:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 204:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 205:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 206:

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

