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
     TOK_CONST_INT = 259,
     TOK_CONST_FLOAT = 260,
     TOK_QUOTED_STRING = 261,
     TOK_AS = 262,
     TOK_ASC = 263,
     TOK_AVG = 264,
     TOK_BEGIN = 265,
     TOK_BETWEEN = 266,
     TOK_BY = 267,
     TOK_CALL = 268,
     TOK_COMMIT = 269,
     TOK_COUNT = 270,
     TOK_DELETE = 271,
     TOK_DESC = 272,
     TOK_DESCRIBE = 273,
     TOK_DISTINCT = 274,
     TOK_FALSE = 275,
     TOK_FROM = 276,
     TOK_GROUP = 277,
     TOK_ID = 278,
     TOK_IN = 279,
     TOK_INSERT = 280,
     TOK_INTO = 281,
     TOK_LIMIT = 282,
     TOK_MATCH = 283,
     TOK_MAX = 284,
     TOK_META = 285,
     TOK_MIN = 286,
     TOK_OPTION = 287,
     TOK_ORDER = 288,
     TOK_REPLACE = 289,
     TOK_ROLLBACK = 290,
     TOK_SELECT = 291,
     TOK_SET = 292,
     TOK_SHOW = 293,
     TOK_START = 294,
     TOK_STATUS = 295,
     TOK_SUM = 296,
     TOK_TABLES = 297,
     TOK_TRANSACTION = 298,
     TOK_TRUE = 299,
     TOK_UPDATE = 300,
     TOK_VALUES = 301,
     TOK_WARNINGS = 302,
     TOK_WEIGHT = 303,
     TOK_WHERE = 304,
     TOK_WITHIN = 305,
     TOK_OR = 306,
     TOK_AND = 307,
     TOK_NE = 308,
     TOK_GTE = 309,
     TOK_LTE = 310,
     TOK_NOT = 311,
     TOK_NEG = 312
   };
#endif
#define TOK_IDENT 258
#define TOK_CONST_INT 259
#define TOK_CONST_FLOAT 260
#define TOK_QUOTED_STRING 261
#define TOK_AS 262
#define TOK_ASC 263
#define TOK_AVG 264
#define TOK_BEGIN 265
#define TOK_BETWEEN 266
#define TOK_BY 267
#define TOK_CALL 268
#define TOK_COMMIT 269
#define TOK_COUNT 270
#define TOK_DELETE 271
#define TOK_DESC 272
#define TOK_DESCRIBE 273
#define TOK_DISTINCT 274
#define TOK_FALSE 275
#define TOK_FROM 276
#define TOK_GROUP 277
#define TOK_ID 278
#define TOK_IN 279
#define TOK_INSERT 280
#define TOK_INTO 281
#define TOK_LIMIT 282
#define TOK_MATCH 283
#define TOK_MAX 284
#define TOK_META 285
#define TOK_MIN 286
#define TOK_OPTION 287
#define TOK_ORDER 288
#define TOK_REPLACE 289
#define TOK_ROLLBACK 290
#define TOK_SELECT 291
#define TOK_SET 292
#define TOK_SHOW 293
#define TOK_START 294
#define TOK_STATUS 295
#define TOK_SUM 296
#define TOK_TABLES 297
#define TOK_TRANSACTION 298
#define TOK_TRUE 299
#define TOK_UPDATE 300
#define TOK_VALUES 301
#define TOK_WARNINGS 302
#define TOK_WEIGHT 303
#define TOK_WHERE 304
#define TOK_WITHIN 305
#define TOK_OR 306
#define TOK_AND 307
#define TOK_NE 308
#define TOK_GTE 309
#define TOK_LTE 310
#define TOK_NOT 311
#define TOK_NEG 312




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif


// some helpers
#include <float.h> // for FLT_MAX

static void AddFloatRangeFilter ( SqlParser_c * pParser, const CSphString & sAttr, float fMin, float fMax )
{
	CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_FLOATRANGE;
	tFilter.m_fMinValue = fMin;
	tFilter.m_fMaxValue = fMax;
}

static void AddUintRangeFilter ( SqlParser_c * pParser, const CSphString & sAttr, DWORD uMin, DWORD uMax )
{
	CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_RANGE;
	tFilter.m_uMinValue = uMin;
	tFilter.m_uMaxValue = uMax;
}

static void AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iInstype;
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	tIns.m_sVal = tNode.m_sValue;
}


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
#define YYFINAL  59
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   509

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  71
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  63
/* YYNRULES -- Number of rules. */
#define YYNRULES  166
/* YYNRULES -- Number of states. */
#define YYNSTATES  328

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   312

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    54,     2,
      69,    70,    63,    61,    68,    62,     2,    64,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    67,
      57,    55,    58,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    53,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    56,    59,
      60,    65,    66
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    32,    34,    36,    47,    49,
      53,    55,    59,    66,    73,    80,    87,    89,    95,    96,
      98,   100,   104,   105,   107,   110,   112,   116,   121,   125,
     129,   133,   139,   146,   152,   156,   160,   164,   168,   172,
     176,   180,   184,   190,   194,   198,   200,   203,   205,   208,
     210,   214,   215,   217,   221,   222,   224,   230,   231,   233,
     237,   239,   243,   245,   248,   251,   253,   255,   256,   258,
     261,   266,   267,   269,   272,   274,   278,   282,   286,   292,
     294,   298,   302,   304,   306,   308,   311,   314,   318,   322,
     326,   330,   334,   338,   342,   346,   350,   354,   358,   362,
     366,   370,   374,   376,   381,   386,   390,   397,   404,   406,
     410,   413,   415,   417,   419,   424,   429,   431,   433,   435,
     437,   439,   441,   443,   446,   453,   455,   457,   458,   462,
     464,   468,   470,   474,   478,   480,   484,   486,   488,   490,
     498,   508,   515,   516,   519,   521,   525,   529,   531,   533,
     536,   538,   540,   543,   552,   554,   558
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      72,     0,    -1,    73,    -1,    74,    -1,    74,    67,    -1,
     114,    -1,   122,    -1,   110,    -1,   112,    -1,   123,    -1,
     128,    -1,   130,    -1,   131,    -1,    75,    -1,    74,    67,
      75,    -1,    76,    -1,   108,    -1,    36,    77,    21,    80,
      81,    88,    90,    92,    97,    99,    -1,    78,    -1,    77,
      68,    78,    -1,     3,    -1,   105,    79,     3,    -1,     9,
      69,   105,    70,    79,     3,    -1,    29,    69,   105,    70,
      79,     3,    -1,    31,    69,   105,    70,    79,     3,    -1,
      41,    69,   105,    70,    79,     3,    -1,    63,    -1,    15,
      69,    19,     3,    70,    -1,    -1,     7,    -1,     3,    -1,
      80,    68,     3,    -1,    -1,    82,    -1,    49,    83,    -1,
      84,    -1,    83,    52,    84,    -1,    28,    69,     6,    70,
      -1,    23,    55,    85,    -1,     3,    55,    85,    -1,     3,
      56,    85,    -1,     3,    24,    69,    87,    70,    -1,     3,
      65,    24,    69,    87,    70,    -1,     3,    11,    85,    52,
      85,    -1,     3,    58,    85,    -1,     3,    57,    85,    -1,
       3,    59,    85,    -1,     3,    60,    85,    -1,     3,    55,
      86,    -1,     3,    56,    86,    -1,     3,    58,    86,    -1,
       3,    57,    86,    -1,     3,    11,    86,    52,    86,    -1,
       3,    59,    86,    -1,     3,    60,    86,    -1,     4,    -1,
      62,     4,    -1,     5,    -1,    62,     5,    -1,    85,    -1,
      87,    68,    85,    -1,    -1,    89,    -1,    22,    12,     3,
      -1,    -1,    91,    -1,    50,    22,    33,    12,    94,    -1,
      -1,    93,    -1,    33,    12,    94,    -1,    95,    -1,    94,
      68,    95,    -1,    96,    -1,    96,     8,    -1,    96,    17,
      -1,    23,    -1,     3,    -1,    -1,    98,    -1,    27,     4,
      -1,    27,     4,    68,     4,    -1,    -1,   100,    -1,    32,
     101,    -1,   102,    -1,   101,    68,   102,    -1,     3,    55,
       3,    -1,     3,    55,     4,    -1,     3,    55,    69,   103,
      70,    -1,   104,    -1,   103,    68,   104,    -1,     3,    55,
      85,    -1,     3,    -1,     4,    -1,     5,    -1,    62,   105,
      -1,    65,   105,    -1,   105,    61,   105,    -1,   105,    62,
     105,    -1,   105,    63,   105,    -1,   105,    64,   105,    -1,
     105,    57,   105,    -1,   105,    58,   105,    -1,   105,    54,
     105,    -1,   105,    53,   105,    -1,   105,    60,   105,    -1,
     105,    59,   105,    -1,   105,    55,   105,    -1,   105,    56,
     105,    -1,   105,    52,   105,    -1,   105,    51,   105,    -1,
      69,   105,    70,    -1,   106,    -1,     3,    69,   107,    70,
      -1,    24,    69,   107,    70,    -1,     3,    69,    70,    -1,
      31,    69,   105,    68,   105,    70,    -1,    29,    69,   105,
      68,   105,    70,    -1,   105,    -1,   107,    68,   105,    -1,
      38,   109,    -1,    47,    -1,    40,    -1,    30,    -1,    37,
       3,    55,   111,    -1,    37,     3,    55,     6,    -1,    44,
      -1,    20,    -1,    85,    -1,    14,    -1,    35,    -1,   113,
      -1,    10,    -1,    39,    43,    -1,   115,    26,     3,   116,
      46,   118,    -1,    25,    -1,    34,    -1,    -1,    69,   117,
      70,    -1,    96,    -1,   117,    68,    96,    -1,   119,    -1,
     118,    68,   119,    -1,    69,   120,    70,    -1,   121,    -1,
     120,    68,   121,    -1,    85,    -1,    86,    -1,     6,    -1,
      16,    21,     3,    49,    23,    55,    85,    -1,    16,    21,
       3,    49,    23,    24,    69,    87,    70,    -1,    13,     3,
      69,   120,   124,    70,    -1,    -1,    68,   125,    -1,   126,
      -1,   125,    68,   126,    -1,   121,    79,   127,    -1,     3,
      -1,    27,    -1,   129,     3,    -1,    18,    -1,    17,    -1,
      38,    42,    -1,    45,     3,    37,   132,    49,    23,    55,
      85,    -1,   133,    -1,   132,    68,   133,    -1,     3,    55,
      85,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   111,   111,   112,   113,   117,   118,   119,   120,   121,
     122,   123,   124,   130,   131,   135,   136,   140,   155,   156,
     160,   162,   164,   165,   166,   167,   168,   169,   184,   185,
     189,   190,   193,   195,   199,   203,   204,   208,   222,   229,
     236,   244,   252,   261,   265,   269,   273,   277,   281,   282,
     283,   284,   289,   293,   297,   304,   305,   309,   310,   314,
     315,   318,   320,   324,   331,   333,   337,   343,   345,   349,
     356,   357,   361,   362,   363,   367,   368,   371,   373,   377,
     382,   389,   391,   395,   399,   400,   404,   409,   414,   423,
     433,   445,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   479,   480,   481,   482,   483,   487,   488,
     494,   498,   499,   500,   506,   512,   521,   522,   523,   537,
     538,   539,   543,   544,   550,   558,   559,   562,   564,   568,
     569,   573,   574,   578,   582,   583,   587,   588,   589,   595,
     601,   613,   620,   622,   626,   631,   635,   643,   644,   650,
     658,   659,   665,   671,   682,   683,   687
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_CONST_INT", 
  "TOK_CONST_FLOAT", "TOK_QUOTED_STRING", "TOK_AS", "TOK_ASC", "TOK_AVG", 
  "TOK_BEGIN", "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_COMMIT", 
  "TOK_COUNT", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", "TOK_DISTINCT", 
  "TOK_FALSE", "TOK_FROM", "TOK_GROUP", "TOK_ID", "TOK_IN", "TOK_INSERT", 
  "TOK_INTO", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", 
  "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", "TOK_ROLLBACK", "TOK_SELECT", 
  "TOK_SET", "TOK_SHOW", "TOK_START", "TOK_STATUS", "TOK_SUM", 
  "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", "TOK_UPDATE", "TOK_VALUES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "TOK_NOT", "TOK_NEG", "';'", 
  "','", "'('", "')'", "$accept", "request", "statement", 
  "multi_stmt_list", "multi_stmt", "select_from", "select_items_list", 
  "select_item", "aass", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "where_item", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "ident_or_id", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "show_clause", "show_variable", "set_clause", 
  "boolean_value", "transact_op", "start_transaction", "insert_into", 
  "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "opt_call_opts_list", "call_opts_list", 
  "call_opt", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", 0
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
     305,   306,   307,   124,    38,    61,   308,    60,    62,   309,
     310,    43,    45,    42,    47,   311,   312,    59,    44,    40,
      41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    71,    72,    72,    72,    73,    73,    73,    73,    73,
      73,    73,    73,    74,    74,    75,    75,    76,    77,    77,
      78,    78,    78,    78,    78,    78,    78,    78,    79,    79,
      80,    80,    81,    81,    82,    83,    83,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    85,    85,    86,    86,    87,
      87,    88,    88,    89,    90,    90,    91,    92,    92,    93,
      94,    94,    95,    95,    95,    96,    96,    97,    97,    98,
      98,    99,    99,   100,   101,   101,   102,   102,   102,   103,
     103,   104,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
     105,   105,   105,   106,   106,   106,   106,   106,   107,   107,
     108,   109,   109,   109,   110,   110,   111,   111,   111,   112,
     112,   112,   113,   113,   114,   115,   115,   116,   116,   117,
     117,   118,   118,   119,   120,   120,   121,   121,   121,   122,
     122,   123,   124,   124,   125,   125,   126,   127,   127,   128,
     129,   129,   130,   131,   132,   132,   133
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,    10,     1,     3,
       1,     3,     6,     6,     6,     6,     1,     5,     0,     1,
       1,     3,     0,     1,     2,     1,     3,     4,     3,     3,
       3,     5,     6,     5,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     3,     3,     1,     2,     1,     2,     1,
       3,     0,     1,     3,     0,     1,     5,     0,     1,     3,
       1,     3,     1,     2,     2,     1,     1,     0,     1,     2,
       4,     0,     1,     2,     1,     3,     3,     3,     5,     1,
       3,     3,     1,     1,     1,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     4,     4,     3,     6,     6,     1,     3,
       2,     1,     1,     1,     4,     4,     1,     1,     1,     1,
       1,     1,     1,     2,     6,     1,     1,     0,     3,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     1,     7,
       9,     6,     0,     2,     1,     3,     3,     1,     1,     2,
       1,     1,     2,     8,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   132,     0,   129,     0,   161,   160,   135,   136,   130,
       0,     0,     0,     0,     0,     0,     2,     3,    13,    15,
      16,     7,     8,   131,     5,     0,     6,     9,    10,     0,
      11,    12,     0,     0,    92,    93,    94,     0,     0,     0,
       0,     0,     0,     0,    26,     0,     0,     0,    18,    28,
     112,     0,   123,   122,   162,   121,   120,   133,     0,     1,
       4,     0,   159,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    92,     0,     0,    95,    96,     0,     0,     0,
      29,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    14,
     137,    55,    57,   148,     0,   146,   147,   152,   144,     0,
     115,   118,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   111,    30,    32,    19,   110,   109,   104,   103,   107,
     108,   101,   102,   106,   105,    97,    98,    99,   100,    21,
     125,   127,   126,     0,   128,   124,     0,     0,   164,     0,
       0,    56,    58,     0,     0,     0,     0,   113,    28,     0,
     114,     0,    28,     0,    28,    28,     0,     0,     0,     0,
      61,    33,     0,     0,     0,    76,    75,   139,     0,     0,
      28,   153,   154,   151,     0,     0,   119,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,    34,    35,    31,
       0,    64,    62,   166,     0,   165,     0,   138,     0,   134,
     141,     0,     0,     0,   149,    22,   117,    23,   116,    24,
      25,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    67,    65,     0,   140,     0,
       0,   157,   158,   156,    28,   155,    59,     0,     0,     0,
       0,    39,    48,    40,    49,    45,    51,    44,    50,    46,
      53,    47,    54,     0,    38,     0,    36,    63,     0,     0,
      77,    68,   163,     0,   143,   142,     0,   150,     0,     0,
       0,     0,    37,     0,     0,     0,    81,    78,   145,    60,
      43,     0,    52,    41,     0,     0,    69,    70,    72,    79,
       0,    17,    82,    42,    66,     0,    73,    74,     0,     0,
      83,    84,    71,    80,     0,     0,    86,    87,     0,    85,
       0,     0,    89,     0,     0,    88,    91,    90
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    15,    16,    17,    18,    19,    47,    48,   211,   123,
     170,   171,   197,   198,   105,   106,   247,   201,   202,   235,
     236,   270,   271,   296,   297,   298,   286,   287,   301,   302,
     310,   311,   321,   322,    49,    50,   112,    20,    56,    21,
     145,    22,    23,    24,    25,   150,   178,   209,   210,   107,
     108,    26,    27,   154,   181,   182,   243,    28,    29,    30,
      31,   147,   148
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -214
static const short yypact[] =
{
     218,  -214,    34,  -214,    43,  -214,  -214,  -214,  -214,  -214,
     133,    99,   121,    64,   100,   111,  -214,    73,  -214,  -214,
    -214,  -214,  -214,  -214,  -214,   118,  -214,  -214,  -214,   115,
    -214,  -214,    83,   162,    15,  -214,  -214,    97,   104,   107,
     117,   122,   123,    51,  -214,    51,    51,     4,  -214,   152,
    -214,   129,  -214,  -214,  -214,  -214,  -214,  -214,   141,  -214,
      70,   184,  -214,    12,   140,    29,    51,   171,    51,    51,
      51,    51,   124,   125,   128,  -214,  -214,   253,   196,   133,
    -214,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    51,   197,    53,   198,   120,  -214,
     148,  -214,  -214,  -214,    91,  -214,  -214,   150,  -214,   202,
    -214,   389,    42,   273,   223,    56,   213,   233,   293,    51,
      51,  -214,  -214,    37,  -214,   402,   414,   425,   435,   187,
     187,   108,   108,   108,   108,   -44,   -44,  -214,  -214,  -214,
    -214,  -214,  -214,   226,  -214,  -214,   178,    55,  -214,    78,
     191,  -214,  -214,    12,   168,    14,    51,  -214,   232,   170,
    -214,    51,   232,    51,   232,   232,   353,   371,    62,   238,
     220,  -214,     5,   235,   198,  -214,  -214,  -214,    65,   190,
       0,   192,  -214,  -214,   193,     5,   389,   258,  -214,   313,
     275,   333,   276,   277,   164,   227,   229,   247,  -214,  -214,
     288,   252,  -214,  -214,   263,  -214,    78,  -214,    12,   251,
    -214,    60,    12,     5,  -214,  -214,  -214,  -214,  -214,  -214,
    -214,    17,   269,    17,    17,    17,    17,    17,    17,   296,
       5,   315,    62,   319,   317,   307,  -214,     5,  -214,    79,
     190,  -214,  -214,  -214,   232,  -214,  -214,    85,   289,   290,
       5,  -214,  -214,  -214,  -214,  -214,  -214,  -214,  -214,  -214,
    -214,  -214,  -214,   291,  -214,   292,  -214,  -214,   325,   347,
     334,  -214,  -214,    12,  -214,  -214,     5,  -214,     5,    26,
      88,     5,  -214,   366,    78,   375,   348,  -214,  -214,  -214,
    -214,   376,  -214,  -214,   109,    78,   314,  -214,    92,   330,
     396,  -214,  -214,  -214,   314,    78,  -214,  -214,   397,   345,
     350,  -214,  -214,  -214,     2,   396,  -214,  -214,   399,  -214,
     364,   113,  -214,     5,   399,  -214,  -214,  -214
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -214,  -214,  -214,  -214,   360,  -214,  -214,   357,   -19,  -214,
    -214,  -214,  -214,   205,   -96,  -213,  -188,  -214,  -214,  -214,
    -214,  -214,  -214,   143,   195,  -114,  -214,  -214,  -214,  -214,
    -214,   186,  -214,   179,   -42,  -214,   434,  -214,  -214,  -214,
    -214,  -214,  -214,  -214,  -214,  -214,  -214,  -214,   264,   297,
    -151,  -214,  -214,  -214,  -214,   294,  -214,  -214,  -214,  -214,
    -214,  -214,   335
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -146
static const short yytable[] =
{
     144,    75,   180,    76,    77,   316,   317,    80,   249,   101,
     252,   254,   256,   258,   260,   262,   101,   102,   103,    93,
      94,   101,   102,   111,   113,    78,   111,   116,   117,   118,
      95,   102,    72,    35,    36,   177,   -20,    32,   184,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    39,    72,    35,    36,   101,    73,   140,
      74,   244,   280,   241,    33,   194,   292,   143,  -145,   185,
    -145,   318,    79,   141,   104,    39,   203,   166,   167,   104,
      73,   175,    74,   -20,    65,   195,   168,   242,   291,   214,
     196,    43,   238,   294,    45,   151,   152,   142,    46,   110,
     306,   176,    51,    58,   173,   169,    10,    57,    98,   307,
     156,    59,   157,    43,   186,   143,    45,   246,    62,   189,
      46,   191,   288,   174,   156,   248,   160,   251,   253,   255,
     257,   259,   261,   206,   264,   207,    34,    35,    36,   187,
      60,   272,    37,   190,    61,   192,   193,   273,    38,   274,
      52,    52,    63,   276,   246,   277,   276,    39,   293,    80,
      53,    53,    40,    54,    41,    64,    66,    55,    55,    91,
      92,    93,    94,    67,    42,   221,    68,   276,    97,   303,
     289,   324,   290,   325,    96,   246,    69,   100,   222,   109,
     114,    70,    71,    65,   119,    43,    44,   120,    45,   122,
     139,   146,    46,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   149,   153,   223,
     224,   225,   226,   227,   228,   155,   159,   326,     1,   229,
     151,     2,     3,   172,     4,     5,     6,   179,   183,    80,
     188,   199,   200,     7,    87,    88,    89,    90,    91,    92,
      93,    94,     8,     9,    10,    11,    12,    13,   204,   208,
     212,   215,   213,    14,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   217,   219,
     220,   161,   230,   162,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   231,   232,
     233,   163,   234,   164,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   237,   240,
     263,   265,   267,   121,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   250,   268,
     269,   278,   279,   158,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   283,   284,
     281,   285,   282,   165,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   295,   299,
     300,   152,   305,   216,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   308,   309,
     314,   313,   320,   218,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,   315,   323,
      99,   161,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,   124,   266,   304,   163,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
     312,   319,   115,   327,   275,   239,   245,     0,     0,   205
};

static const short yycheck[] =
{
      96,    43,   153,    45,    46,     3,     4,     7,   221,     4,
     223,   224,   225,   226,   227,   228,     4,     5,     6,    63,
      64,     4,     5,    65,    66,    21,    68,    69,    70,    71,
      49,     5,     3,     4,     5,   149,    21,     3,    24,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    24,     3,     4,     5,     4,    29,     6,
      31,   212,   250,     3,    21,     3,   279,    62,    68,    55,
      70,    69,    68,    20,    62,    24,   172,   119,   120,    62,
      29,     3,    31,    68,    69,    23,    49,    27,    62,   185,
      28,    62,   206,   281,    65,     4,     5,    44,    69,    70,
       8,    23,     3,     3,    49,    68,    36,    43,    38,    17,
      68,     0,    70,    62,   156,    62,    65,   213,     3,   161,
      69,   163,   273,    68,    68,   221,    70,   223,   224,   225,
     226,   227,   228,    68,   230,    70,     3,     4,     5,   158,
      67,   237,     9,   162,    26,   164,   165,    68,    15,    70,
      30,    30,    69,    68,   250,    70,    68,    24,    70,     7,
      40,    40,    29,    42,    31,     3,    69,    47,    47,    61,
      62,    63,    64,    69,    41,    11,    69,    68,    37,    70,
     276,    68,   278,    70,    55,   281,    69,     3,    24,    49,
      19,    69,    69,    69,    69,    62,    63,    69,    65,     3,
       3,     3,    69,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    69,    68,    55,
      56,    57,    58,    59,    60,    23,     3,   323,    10,    65,
       4,    13,    14,    55,    16,    17,    18,    46,    70,     7,
      70,     3,    22,    25,    57,    58,    59,    60,    61,    62,
      63,    64,    34,    35,    36,    37,    38,    39,    23,    69,
      68,     3,    69,    45,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,     3,     3,
       3,    68,    55,    70,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    69,    52,
      12,    68,    50,    70,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    55,    68,
      24,     6,     3,    70,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    69,    22,
      33,    52,    52,    70,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    33,    12,
      69,    27,    70,    70,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    12,     4,
      32,     5,    68,    70,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    68,     3,
      55,     4,     3,    70,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    68,    55,
      60,    68,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    79,   232,   295,    68,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
     305,   315,    68,   324,   240,   208,   212,    -1,    -1,   174
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    10,    13,    14,    16,    17,    18,    25,    34,    35,
      36,    37,    38,    39,    45,    72,    73,    74,    75,    76,
     108,   110,   112,   113,   114,   115,   122,   123,   128,   129,
     130,   131,     3,    21,     3,     4,     5,     9,    15,    24,
      29,    31,    41,    62,    63,    65,    69,    77,    78,   105,
     106,     3,    30,    40,    42,    47,   109,    43,     3,     0,
      67,    26,     3,    69,     3,    69,    69,    69,    69,    69,
      69,    69,     3,    29,    31,   105,   105,   105,    21,    68,
       7,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    79,    55,    37,    38,    75,
       3,     4,     5,     6,    62,    85,    86,   120,   121,    49,
      70,   105,   107,   105,    19,   107,   105,   105,   105,    69,
      69,    70,     3,    80,    78,   105,   105,   105,   105,   105,
     105,   105,   105,   105,   105,   105,   105,   105,   105,     3,
       6,    20,    44,    62,    85,   111,     3,   132,   133,    69,
     116,     4,     5,    68,   124,    23,    68,    70,    70,     3,
      70,    68,    70,    68,    70,    70,   105,   105,    49,    68,
      81,    82,    55,    49,    68,     3,    23,    96,   117,    46,
     121,   125,   126,    70,    24,    55,   105,    79,    70,   105,
      79,   105,    79,    79,     3,    23,    28,    83,    84,     3,
      22,    88,    89,    85,    23,   133,    68,    70,    69,   118,
     119,    79,    68,    69,    85,     3,    70,     3,    70,     3,
       3,    11,    24,    55,    56,    57,    58,    59,    60,    65,
      55,    69,    52,    12,    50,    90,    91,    55,    96,   120,
      68,     3,    27,   127,   121,   126,    85,    87,    85,    86,
      69,    85,    86,    85,    86,    85,    86,    85,    86,    85,
      86,    85,    86,    24,    85,     6,    84,     3,    22,    33,
      92,    93,    85,    68,    70,   119,    68,    70,    52,    52,
      87,    69,    70,    33,    12,    27,    97,    98,   121,    85,
      85,    62,    86,    70,    87,    12,    94,    95,    96,     4,
      32,    99,   100,    70,    94,    68,     8,    17,    68,     3,
     101,   102,    95,     4,    55,    68,     3,     4,    69,   102,
       3,   103,   104,    55,    68,    70,    85,   104
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

  case 13:

    { pParser->PushQuery(); ;}
    break;

  case 14:

    { pParser->PushQuery(); ;}
    break;

  case 17:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 20:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd );
											pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 21:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd );
									pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 22:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 23:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 24:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 25:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 26:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 27:

    {
			if ( !pParser->m_pQuery->m_sGroupDistinct.IsEmpty() )
			{
				yyerror ( pParser, "too many COUNT(DISTINCT) clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sGroupDistinct = yyvsp[-1].m_sValue;
				pParser->SetSelect ( yyvsp[-1].m_iStart, yyvsp[-1].m_iEnd );
			}
		;}
    break;

  case 31:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 37:

    {
			if ( pParser->m_bGotQuery )
			{
				yyerror ( pParser, "too many MATCH() clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sQuery = yyvsp[-1].m_sValue;
				pParser->m_pQuery->m_sRawQuery = yyvsp[-1].m_sValue;
				pParser->m_bGotQuery = true;
			}
		;}
    break;

  case 38:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 39:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 40:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
		;}
    break;

  case 41:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 42:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 43:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 44:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 45:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 46:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 47:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 51:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 52:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 53:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 54:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 55:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 56:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 57:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 58:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 59:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 60:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 63:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 66:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 69:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 71:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 73:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 74:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 79:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 80:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 86:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 87:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 88:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 89:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 90:

    {
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 91:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 95:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 96:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 97:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 98:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 99:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 100:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 101:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 102:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 103:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 104:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 105:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 106:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 107:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 108:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 109:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 110:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 111:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 113:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 114:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 115:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 116:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 117:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 121:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 122:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 123:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 124:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 125:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 126:

    { yyval.m_iValue = 1; ;}
    break;

  case 127:

    { yyval.m_iValue = 0; ;}
    break;

  case 128:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 129:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 130:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 131:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 134:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 135:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 136:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 139:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 140:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 143:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 144:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 145:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 146:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 147:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 148:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 149:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 150:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			ARRAY_FOREACH ( i, yyvsp[-1].m_dValues )
				pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[-1].m_dValues[i] );
		;}
    break;

  case 151:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 154:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 156:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 158:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 159:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 162:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 163:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_UPDATE;
			tStmt.m_sIndex = yyvsp[-6].m_sValue;	
			tStmt.m_tUpdate.m_dDocids.Add ( (SphDocID_t) yyvsp[0].m_iValue );
			tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );
		;}
    break;

  case 166:

    {
			CSphAttrUpdate & tUpd = pParser->m_pStmt->m_tUpdate;
			CSphColumnInfo & tAttr = tUpd.m_dAttrs.Add();
			tAttr.m_sName = yyvsp[-2].m_sValue;
			tAttr.m_sName.ToLower();
			tAttr.m_eAttrType = SPH_ATTR_INTEGER; // sorry, ints only for now, riding on legacy shit!
			tUpd.m_dPool.Add ( (DWORD) yyvsp[0].m_iValue );
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

