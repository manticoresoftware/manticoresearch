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
     TOK_COLLATION = 269,
     TOK_COMMIT = 270,
     TOK_COUNT = 271,
     TOK_DELETE = 272,
     TOK_DESC = 273,
     TOK_DESCRIBE = 274,
     TOK_DISTINCT = 275,
     TOK_DIV = 276,
     TOK_FALSE = 277,
     TOK_FROM = 278,
     TOK_GLOBAL = 279,
     TOK_GROUP = 280,
     TOK_ID = 281,
     TOK_IN = 282,
     TOK_INSERT = 283,
     TOK_INTO = 284,
     TOK_LIMIT = 285,
     TOK_MATCH = 286,
     TOK_MAX = 287,
     TOK_META = 288,
     TOK_MIN = 289,
     TOK_MOD = 290,
     TOK_NULL = 291,
     TOK_OPTION = 292,
     TOK_ORDER = 293,
     TOK_REPLACE = 294,
     TOK_ROLLBACK = 295,
     TOK_SELECT = 296,
     TOK_SET = 297,
     TOK_SHOW = 298,
     TOK_START = 299,
     TOK_STATUS = 300,
     TOK_SUM = 301,
     TOK_TABLES = 302,
     TOK_TRANSACTION = 303,
     TOK_TRUE = 304,
     TOK_UPDATE = 305,
     TOK_USERVAR = 306,
     TOK_VALUES = 307,
     TOK_VARIABLES = 308,
     TOK_WARNINGS = 309,
     TOK_WEIGHT = 310,
     TOK_WHERE = 311,
     TOK_WITHIN = 312,
     TOK_OR = 313,
     TOK_AND = 314,
     TOK_NE = 315,
     TOK_GTE = 316,
     TOK_LTE = 317,
     TOK_NOT = 318,
     TOK_NEG = 319
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
#define TOK_COLLATION 269
#define TOK_COMMIT 270
#define TOK_COUNT 271
#define TOK_DELETE 272
#define TOK_DESC 273
#define TOK_DESCRIBE 274
#define TOK_DISTINCT 275
#define TOK_DIV 276
#define TOK_FALSE 277
#define TOK_FROM 278
#define TOK_GLOBAL 279
#define TOK_GROUP 280
#define TOK_ID 281
#define TOK_IN 282
#define TOK_INSERT 283
#define TOK_INTO 284
#define TOK_LIMIT 285
#define TOK_MATCH 286
#define TOK_MAX 287
#define TOK_META 288
#define TOK_MIN 289
#define TOK_MOD 290
#define TOK_NULL 291
#define TOK_OPTION 292
#define TOK_ORDER 293
#define TOK_REPLACE 294
#define TOK_ROLLBACK 295
#define TOK_SELECT 296
#define TOK_SET 297
#define TOK_SHOW 298
#define TOK_START 299
#define TOK_STATUS 300
#define TOK_SUM 301
#define TOK_TABLES 302
#define TOK_TRANSACTION 303
#define TOK_TRUE 304
#define TOK_UPDATE 305
#define TOK_USERVAR 306
#define TOK_VALUES 307
#define TOK_VARIABLES 308
#define TOK_WARNINGS 309
#define TOK_WEIGHT 310
#define TOK_WHERE 311
#define TOK_WITHIN 312
#define TOK_OR 313
#define TOK_AND 314
#define TOK_NE 315
#define TOK_GTE 316
#define TOK_LTE 317
#define TOK_NOT 318
#define TOK_NEG 319




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

static bool AddUservarFilter ( SqlParser_c * pParser, CSphString & sCol, CSphString & sVar, bool bExclude )
{
	g_tUservarsMutex.Lock();
	Uservar_t * pVar = g_hUservars ( sVar );
	if ( !pVar )
	{
		g_tUservarsMutex.Unlock();
		yyerror ( pParser, "undefined global variable in IN clause" );
		return false;
	}

	assert ( pVar->m_eType==USERVAR_INT_SET );
	CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
	tFilter.m_sAttrName = sCol;
	tFilter.m_eType = SPH_FILTER_VALUES;
	tFilter.m_bExclude = bExclude;

	// INT_SET uservars must get sorted on SET once
	// FIXME? maybe we should do a readlock instead of copying?
	tFilter.m_dValues = *pVar->m_pVal;
	g_tUservarsMutex.Unlock();
	return true;
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
#define YYFINAL  66
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   804

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  79
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  67
/* YYNRULES -- Number of rules. */
#define YYNRULES  182
/* YYNRULES -- Number of states. */
#define YYNSTATES  354

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    72,    61,     2,
      77,    78,    70,    68,    76,    69,     2,    71,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    75,
      64,    62,    65,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    60,     2,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59,    63,    66,    67,    73,    74
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    38,    40,
      42,    53,    55,    59,    61,    65,    72,    79,    86,    93,
      95,   101,   102,   104,   106,   110,   111,   113,   116,   118,
     122,   127,   131,   135,   139,   145,   152,   156,   161,   167,
     171,   175,   179,   183,   187,   191,   195,   199,   205,   209,
     213,   215,   218,   220,   223,   225,   229,   230,   232,   236,
     237,   239,   245,   246,   248,   252,   254,   258,   260,   263,
     266,   268,   270,   271,   273,   276,   281,   282,   284,   287,
     289,   293,   297,   301,   307,   309,   313,   317,   319,   321,
     323,   325,   328,   331,   335,   339,   343,   347,   351,   355,
     359,   363,   367,   371,   375,   379,   383,   387,   391,   395,
     399,   403,   405,   410,   415,   419,   426,   433,   435,   439,
     442,   444,   446,   448,   453,   458,   463,   471,   477,   479,
     481,   483,   485,   487,   489,   491,   493,   495,   498,   505,
     507,   509,   510,   514,   516,   520,   522,   526,   530,   532,
     536,   538,   540,   542,   550,   560,   567,   568,   571,   573,
     577,   581,   583,   585,   588,   590,   592,   595,   604,   606,
     610,   614,   617
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      80,     0,    -1,    81,    -1,    82,    -1,    82,    75,    -1,
     124,    -1,   132,    -1,   118,    -1,   119,    -1,   122,    -1,
     133,    -1,   138,    -1,   140,    -1,   141,    -1,   144,    -1,
     145,    -1,    83,    -1,    82,    75,    83,    -1,    84,    -1,
     116,    -1,    41,    85,    23,    88,    89,    96,    98,   100,
     105,   107,    -1,    86,    -1,    85,    76,    86,    -1,     3,
      -1,   113,    87,     3,    -1,     9,    77,   113,    78,    87,
       3,    -1,    32,    77,   113,    78,    87,     3,    -1,    34,
      77,   113,    78,    87,     3,    -1,    46,    77,   113,    78,
      87,     3,    -1,    70,    -1,    16,    77,    20,     3,    78,
      -1,    -1,     7,    -1,     3,    -1,    88,    76,     3,    -1,
      -1,    90,    -1,    56,    91,    -1,    92,    -1,    91,    59,
      92,    -1,    31,    77,     6,    78,    -1,    26,    62,    93,
      -1,     3,    62,    93,    -1,     3,    63,    93,    -1,     3,
      27,    77,    95,    78,    -1,     3,    73,    27,    77,    95,
      78,    -1,     3,    27,    51,    -1,     3,    73,    27,    51,
      -1,     3,    11,    93,    59,    93,    -1,     3,    65,    93,
      -1,     3,    64,    93,    -1,     3,    66,    93,    -1,     3,
      67,    93,    -1,     3,    62,    94,    -1,     3,    63,    94,
      -1,     3,    65,    94,    -1,     3,    64,    94,    -1,     3,
      11,    94,    59,    94,    -1,     3,    66,    94,    -1,     3,
      67,    94,    -1,     4,    -1,    69,     4,    -1,     5,    -1,
      69,     5,    -1,    93,    -1,    95,    76,    93,    -1,    -1,
      97,    -1,    25,    12,     3,    -1,    -1,    99,    -1,    57,
      25,    38,    12,   102,    -1,    -1,   101,    -1,    38,    12,
     102,    -1,   103,    -1,   102,    76,   103,    -1,   104,    -1,
     104,     8,    -1,   104,    18,    -1,    26,    -1,     3,    -1,
      -1,   106,    -1,    30,     4,    -1,    30,     4,    76,     4,
      -1,    -1,   108,    -1,    37,   109,    -1,   110,    -1,   109,
      76,   110,    -1,     3,    62,     3,    -1,     3,    62,     4,
      -1,     3,    62,    77,   111,    78,    -1,   112,    -1,   111,
      76,   112,    -1,     3,    62,    93,    -1,     3,    -1,     4,
      -1,     5,    -1,    51,    -1,    69,   113,    -1,    73,   113,
      -1,   113,    68,   113,    -1,   113,    69,   113,    -1,   113,
      70,   113,    -1,   113,    71,   113,    -1,   113,    64,   113,
      -1,   113,    65,   113,    -1,   113,    61,   113,    -1,   113,
      60,   113,    -1,   113,    72,   113,    -1,   113,    21,   113,
      -1,   113,    35,   113,    -1,   113,    67,   113,    -1,   113,
      66,   113,    -1,   113,    62,   113,    -1,   113,    63,   113,
      -1,   113,    59,   113,    -1,   113,    58,   113,    -1,    77,
     113,    78,    -1,   114,    -1,     3,    77,   115,    78,    -1,
      27,    77,   115,    78,    -1,     3,    77,    78,    -1,    34,
      77,   113,    76,   113,    78,    -1,    32,    77,   113,    76,
     113,    78,    -1,   113,    -1,   115,    76,   113,    -1,    43,
     117,    -1,    54,    -1,    45,    -1,    33,    -1,    42,     3,
      62,   121,    -1,    42,     3,    62,   120,    -1,    42,     3,
      62,    36,    -1,    42,    24,    51,    62,    77,    95,    78,
      -1,    42,    24,     3,    62,   120,    -1,     3,    -1,     6,
      -1,    49,    -1,    22,    -1,    93,    -1,    15,    -1,    40,
      -1,   123,    -1,    10,    -1,    44,    48,    -1,   125,    29,
       3,   126,    52,   128,    -1,    28,    -1,    39,    -1,    -1,
      77,   127,    78,    -1,   104,    -1,   127,    76,   104,    -1,
     129,    -1,   128,    76,   129,    -1,    77,   130,    78,    -1,
     131,    -1,   130,    76,   131,    -1,    93,    -1,    94,    -1,
       6,    -1,    17,    23,     3,    56,    26,    62,    93,    -1,
      17,    23,     3,    56,    26,    27,    77,    95,    78,    -1,
      13,     3,    77,   130,   134,    78,    -1,    -1,    76,   135,
      -1,   136,    -1,   135,    76,   136,    -1,   131,    87,   137,
      -1,     3,    -1,    30,    -1,   139,     3,    -1,    19,    -1,
      18,    -1,    43,    47,    -1,    50,     3,    42,   142,    56,
      26,    62,    93,    -1,   143,    -1,   142,    76,   143,    -1,
       3,    62,    93,    -1,    43,    53,    -1,    43,    14,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   142,   142,   143,   144,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,   158,   164,   165,   169,   170,
     174,   189,   190,   194,   195,   196,   197,   198,   199,   200,
     201,   216,   217,   221,   222,   225,   227,   231,   235,   236,
     240,   254,   261,   268,   276,   284,   293,   298,   303,   307,
     311,   315,   319,   323,   324,   325,   326,   331,   335,   339,
     346,   347,   351,   352,   356,   362,   368,   370,   374,   381,
     383,   387,   393,   395,   399,   406,   407,   411,   412,   413,
     417,   418,   421,   423,   427,   432,   439,   441,   445,   449,
     450,   454,   459,   464,   473,   483,   495,   505,   506,   507,
     508,   509,   510,   511,   512,   513,   514,   515,   516,   517,
     518,   519,   520,   521,   522,   523,   524,   525,   526,   527,
     528,   529,   533,   534,   535,   536,   537,   541,   542,   548,
     552,   553,   554,   560,   567,   574,   584,   591,   601,   602,
     606,   607,   608,   622,   623,   624,   628,   629,   635,   643,
     644,   647,   649,   653,   654,   658,   659,   663,   667,   668,
     672,   673,   674,   680,   686,   698,   705,   707,   711,   716,
     720,   728,   729,   735,   743,   744,   750,   756,   767,   768,
     772,   786,   790
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_CONST_INT", 
  "TOK_CONST_FLOAT", "TOK_QUOTED_STRING", "TOK_AS", "TOK_ASC", "TOK_AVG", 
  "TOK_BEGIN", "TOK_BETWEEN", "TOK_BY", "TOK_CALL", "TOK_COLLATION", 
  "TOK_COMMIT", "TOK_COUNT", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_FALSE", "TOK_FROM", "TOK_GLOBAL", 
  "TOK_GROUP", "TOK_ID", "TOK_IN", "TOK_INSERT", "TOK_INTO", "TOK_LIMIT", 
  "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", "TOK_ROLLBACK", "TOK_SELECT", 
  "TOK_SET", "TOK_SHOW", "TOK_START", "TOK_STATUS", "TOK_SUM", 
  "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", "TOK_UPDATE", 
  "TOK_USERVAR", "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", 
  "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", 
  "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", 
  "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", 
  "')'", "$accept", "request", "statement", "multi_stmt_list", 
  "multi_stmt", "select_from", "select_items_list", "select_item", 
  "opt_as", "ident_list", "opt_where_clause", "where_clause", 
  "where_expr", "where_item", "const_int", "const_float", "const_list", 
  "opt_group_clause", "group_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "ident_or_id", "opt_limit_clause", 
  "limit_clause", "opt_option_clause", "option_clause", "option_list", 
  "option_item", "named_const_list", "named_const", "expr", "function", 
  "arglist", "show_clause", "show_variable", "set_clause", 
  "set_global_clause", "set_string_value", "boolean_value", "transact_op", 
  "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "opt_call_opts_list", "call_opts_list", "call_opt", "call_opt_name", 
  "describe", "describe_tok", "show_tables", "update", 
  "update_items_list", "update_item", "show_variables", "show_collation", 0
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
     124,    38,    61,   315,    60,    62,   316,   317,    43,    45,
      42,    47,    37,   318,   319,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    79,    80,    80,    80,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    81,    82,    82,    83,    83,
      84,    85,    85,    86,    86,    86,    86,    86,    86,    86,
      86,    87,    87,    88,    88,    89,    89,    90,    91,    91,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      93,    93,    94,    94,    95,    95,    96,    96,    97,    98,
      98,    99,   100,   100,   101,   102,   102,   103,   103,   103,
     104,   104,   105,   105,   106,   106,   107,   107,   108,   109,
     109,   110,   110,   110,   111,   111,   112,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   114,   114,   114,   114,   114,   115,   115,   116,
     117,   117,   117,   118,   118,   118,   119,   119,   120,   120,
     121,   121,   121,   122,   122,   122,   123,   123,   124,   125,
     125,   126,   126,   127,   127,   128,   128,   129,   130,   130,
     131,   131,   131,   132,   132,   133,   134,   134,   135,   135,
     136,   137,   137,   138,   139,   139,   140,   141,   142,   142,
     143,   144,   145
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     1,
      10,     1,     3,     1,     3,     6,     6,     6,     6,     1,
       5,     0,     1,     1,     3,     0,     1,     2,     1,     3,
       4,     3,     3,     3,     5,     6,     3,     4,     5,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     3,     3,
       1,     2,     1,     2,     1,     3,     0,     1,     3,     0,
       1,     5,     0,     1,     3,     1,     3,     1,     2,     2,
       1,     1,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     1,     3,     3,     1,     1,     1,
       1,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     4,     4,     3,     6,     6,     1,     3,     2,
       1,     1,     1,     4,     4,     4,     7,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     6,     1,
       1,     0,     3,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     7,     9,     6,     0,     2,     1,     3,
       3,     1,     1,     2,     1,     1,     2,     8,     1,     3,
       3,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   146,     0,   143,     0,   175,   174,   149,   150,   144,
       0,     0,     0,     0,     0,     0,     2,     3,    16,    18,
      19,     7,     8,     9,   145,     5,     0,     6,    10,    11,
       0,    12,    13,    14,    15,     0,     0,    97,    98,    99,
       0,     0,     0,     0,     0,     0,   100,     0,    29,     0,
       0,     0,    21,    31,   121,     0,     0,   182,   132,   131,
     176,   181,   130,   129,   147,     0,     1,     4,     0,   173,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,   101,   102,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    17,   151,    60,    62,   162,     0,   160,   161,   166,
     158,     0,   124,   127,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   120,    33,    35,    22,   112,   113,   119,
     118,   110,   109,   116,   117,   107,   108,   115,   114,   103,
     104,   105,   106,   111,    24,   138,   139,   141,   135,   140,
       0,   142,   134,   133,     0,     0,     0,     0,   178,     0,
       0,    61,    63,     0,     0,     0,     0,   122,    31,     0,
     123,     0,    31,     0,    31,    31,     0,     0,     0,     0,
      66,    36,   137,     0,     0,     0,     0,    81,    80,   153,
       0,     0,    31,   167,   168,   165,     0,     0,   128,     0,
      30,     0,     0,     0,     0,     0,     0,     0,     0,    37,
      38,    34,     0,    69,    67,    64,     0,   180,     0,   179,
       0,   152,     0,   148,   155,     0,     0,     0,   163,    25,
     126,    26,   125,    27,    28,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    72,
      70,     0,   136,     0,   154,     0,     0,   171,   172,   170,
      31,   169,     0,     0,     0,    46,     0,    42,    53,    43,
      54,    50,    56,    49,    55,    51,    58,    52,    59,     0,
      41,     0,    39,    68,     0,     0,    82,    73,    65,   177,
       0,   157,   156,   164,     0,     0,     0,    47,     0,    40,
       0,     0,     0,    86,    83,   159,    48,     0,    57,    44,
       0,     0,    74,    75,    77,    84,     0,    20,    87,    45,
      71,     0,    78,    79,     0,     0,    88,    89,    76,    85,
       0,     0,    91,    92,     0,    90,     0,     0,    94,     0,
       0,    93,    96,    95
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    15,    16,    17,    18,    19,    51,    52,   235,   135,
     190,   191,   219,   220,   117,   118,   226,   223,   224,   259,
     260,   296,   297,   322,   323,   324,   313,   314,   327,   328,
     336,   337,   347,   348,    53,    54,   124,    20,    63,    21,
      22,   162,   163,    23,    24,    25,    26,   170,   200,   233,
     234,   119,   120,    27,    28,   174,   203,   204,   269,    29,
      30,    31,    32,   167,   168,    33,    34
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -215
static const short yypact[] =
{
     754,  -215,    21,  -215,    45,  -215,  -215,  -215,  -215,  -215,
     158,    96,   126,   -20,   103,   107,  -215,    52,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,  -215,    82,  -215,  -215,  -215,
     146,  -215,  -215,  -215,  -215,    89,   166,    36,  -215,  -215,
      98,   117,   118,   119,   120,   122,  -215,   173,  -215,   173,
     173,    -7,  -215,   194,  -215,   141,    15,  -215,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,   164,  -215,   -22,   185,  -215,
       5,   152,    87,   173,   190,   173,   173,   173,   173,   134,
     135,   136,  -215,  -215,   325,   211,   158,  -215,   173,   173,
     173,   173,   173,   173,   173,   173,   173,   173,   173,   173,
     173,   173,   173,   173,   173,   214,    67,   156,   163,   223,
      63,  -215,   153,  -215,  -215,  -215,    59,  -215,  -215,   157,
    -215,   208,  -215,   563,    24,   346,   233,    46,   252,   273,
     398,   173,   173,  -215,  -215,    39,  -215,  -215,  -215,   615,
     628,   680,   691,   638,   638,   151,   151,   151,   151,    62,
      62,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,
     234,  -215,  -215,  -215,    14,   160,   177,    42,  -215,    79,
     188,  -215,  -215,     5,   167,    -2,   173,  -215,   237,   169,
    -215,   173,   237,   173,   237,   237,   492,   544,    78,   238,
     224,  -215,  -215,    11,    11,   222,   223,  -215,  -215,  -215,
      47,   174,     0,   198,  -215,  -215,   199,    11,   563,   272,
    -215,   419,   274,   471,   276,   277,   205,   219,   206,   225,
    -215,  -215,   270,   228,  -215,  -215,    50,  -215,   226,  -215,
      79,  -215,     5,   210,  -215,    31,     5,    11,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,     8,   -10,     8,     8,     8,
       8,     8,     8,   262,    11,   284,    78,   288,   267,   255,
    -215,    11,  -215,    11,  -215,    74,   174,  -215,  -215,  -215,
     237,  -215,   105,   236,   239,  -215,    11,  -215,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,    33,
    -215,   218,  -215,  -215,   259,   287,   271,  -215,  -215,  -215,
       5,  -215,  -215,  -215,    11,     3,   106,  -215,    11,  -215,
     290,    79,   296,   266,  -215,  -215,  -215,   299,  -215,  -215,
     111,    79,   229,  -215,   150,   230,   304,  -215,  -215,  -215,
     229,    79,  -215,  -215,   305,   263,   250,  -215,  -215,  -215,
       2,   304,  -215,  -215,   324,  -215,   285,   115,  -215,    11,
     324,  -215,  -215,  -215
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -215,  -215,  -215,  -215,   281,  -215,  -215,   243,   -31,  -215,
    -215,  -215,  -215,    94,  -106,  -212,  -214,  -215,  -215,  -215,
    -215,  -215,  -215,    32,    23,  -155,  -215,  -215,  -215,  -215,
    -215,    16,  -215,     6,   -46,  -215,   280,  -215,  -215,  -215,
    -215,   195,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,
      86,   129,  -171,  -215,  -215,  -215,  -215,   127,  -215,  -215,
    -215,  -215,  -215,  -215,   162,  -215,  -215
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -160
static const short yytable[] =
{
     161,    82,   202,    83,    84,   342,   343,    87,   114,   113,
     114,   115,   113,   114,   199,   113,    85,   155,   107,    10,
     156,   110,   105,   272,    35,   206,   123,   125,    64,   123,
     128,   129,   130,   274,   267,   278,   280,   282,   284,   286,
     288,   275,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   -23,
     207,   268,   306,   171,   172,   270,   108,   276,    36,    86,
     155,   113,   317,   156,   116,   264,  -159,   116,  -159,   344,
     160,   216,   197,    88,   307,   186,   187,   225,   227,   157,
      79,    38,    39,   318,   320,   188,    58,    89,   195,    55,
     176,   238,   177,   158,   217,   198,    65,    66,    59,   218,
     308,    68,   -23,    72,    42,   189,   159,    62,   196,    80,
      56,    81,   176,   230,   180,   231,   261,    67,   262,   315,
     208,   225,   102,   103,   104,   211,   160,   213,    46,   273,
      57,   277,   279,   281,   283,   285,   287,   209,   290,    69,
     300,   212,   301,   214,   215,   298,    47,   299,   332,    58,
      49,    37,    38,    39,    50,   122,    70,    40,   333,    71,
     225,    59,    88,    60,    41,    73,    79,    38,    39,    61,
      62,   261,   261,   303,   319,    42,    89,   261,   112,   329,
      43,   350,    44,   351,    74,    75,    76,    77,   316,    78,
      42,    87,   225,   106,    45,    80,   109,    81,   121,    46,
     126,    72,   131,   132,   134,    88,   245,   154,   164,   100,
     101,   102,   103,   104,    46,   165,   166,    47,    48,    89,
     169,    49,   246,   173,   175,    50,   179,   193,   171,   194,
     201,   221,    47,   352,    87,   205,    49,   210,   228,   222,
      50,   232,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   247,   248,   249,
     250,   251,   252,    88,   236,   239,   237,   241,   253,   243,
     244,   254,   257,   255,   256,   258,   266,    89,   263,   289,
     291,   293,   294,   295,    88,   304,   309,   310,   305,   311,
     325,   312,   321,   326,   172,   331,   334,   335,    89,   339,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   340,   341,   346,   181,   136,
     182,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    88,   349,   111,   183,
     292,   184,   302,   330,   338,   127,   353,   345,   229,   192,
      89,   265,     0,   271,     0,     0,     0,    88,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    89,     0,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,     0,     0,
       0,     0,     0,   133,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    88,
       0,     0,     0,     0,   178,     0,     0,     0,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,     0,     0,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    89,     0,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,     0,     0,     0,     0,     0,   185,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,    88,     0,     0,     0,     0,   240,     0,     0,
       0,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,     0,     0,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    89,     0,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,     0,     0,     0,     0,     0,   242,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    88,     0,     0,   181,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    89,     0,
       0,     0,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,     0,     0,     0,
     183,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,    88,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    88,
      89,     0,     0,     0,     0,     0,     0,     0,     0,    88,
       0,     0,     0,    89,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    89,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,    88,    96,    97,    98,    99,   100,   101,   102,   103,
     104,     0,    88,     0,     0,    89,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,     1,     0,     0,     2,     0,     3,
       0,     4,     5,     6,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     8,     9,    10,    11,    12,    13,     0,
       0,     0,     0,     0,    14
};

static const short yycheck[] =
{
     106,    47,   173,    49,    50,     3,     4,     7,     5,     4,
       5,     6,     4,     5,   169,     4,    23,     3,     3,    41,
       6,    43,    53,   237,     3,    27,    72,    73,    48,    75,
      76,    77,    78,   245,     3,   247,   248,   249,   250,   251,
     252,    51,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,    23,
      62,    30,   276,     4,     5,   236,    51,    77,    23,    76,
       3,     4,    69,     6,    69,   230,    76,    69,    78,    77,
      69,     3,     3,    21,    51,   131,   132,   193,   194,    22,
       3,     4,     5,   305,   308,    56,    33,    35,    56,     3,
      76,   207,    78,    36,    26,    26,     3,     0,    45,    31,
      77,    29,    76,    77,    27,    76,    49,    54,    76,    32,
      24,    34,    76,    76,    78,    78,    76,    75,    78,   300,
     176,   237,    70,    71,    72,   181,    69,   183,    51,   245,
      14,   247,   248,   249,   250,   251,   252,   178,   254,     3,
      76,   182,    78,   184,   185,   261,    69,   263,     8,    33,
      73,     3,     4,     5,    77,    78,    77,     9,    18,     3,
     276,    45,    21,    47,    16,    77,     3,     4,     5,    53,
      54,    76,    76,    78,    78,    27,    35,    76,     3,    78,
      32,    76,    34,    78,    77,    77,    77,    77,   304,    77,
      27,     7,   308,    62,    46,    32,    42,    34,    56,    51,
      20,    77,    77,    77,     3,    21,    11,     3,    62,    68,
      69,    70,    71,    72,    51,    62,     3,    69,    70,    35,
      77,    73,    27,    76,    26,    77,     3,    77,     4,    62,
      52,     3,    69,   349,     7,    78,    73,    78,    26,    25,
      77,    77,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    62,    63,    64,
      65,    66,    67,    21,    76,     3,    77,     3,    73,     3,
       3,    62,    12,    77,    59,    57,    76,    35,    62,    27,
       6,     3,    25,    38,    21,    59,    78,    38,    59,    12,
       4,    30,    12,    37,     5,    76,    76,     3,    35,     4,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    62,    76,     3,    76,    86,
      78,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    21,    62,    67,    76,
     256,    78,   266,   321,   331,    75,   350,   341,   196,   164,
      35,   232,    -1,   236,    -1,    -1,    -1,    21,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    -1,    -1,    78,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    21,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    35,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    -1,    78,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    21,    -1,    -1,    -1,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,    78,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    21,    -1,    -1,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    21,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    21,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      -1,    -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    35,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    21,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    21,    -1,    -1,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    10,    -1,    -1,    13,    -1,    15,
      -1,    17,    18,    19,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    39,    40,    41,    42,    43,    44,    -1,
      -1,    -1,    -1,    -1,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    10,    13,    15,    17,    18,    19,    28,    39,    40,
      41,    42,    43,    44,    50,    80,    81,    82,    83,    84,
     116,   118,   119,   122,   123,   124,   125,   132,   133,   138,
     139,   140,   141,   144,   145,     3,    23,     3,     4,     5,
       9,    16,    27,    32,    34,    46,    51,    69,    70,    73,
      77,    85,    86,   113,   114,     3,    24,    14,    33,    45,
      47,    53,    54,   117,    48,     3,     0,    75,    29,     3,
      77,     3,    77,    77,    77,    77,    77,    77,    77,     3,
      32,    34,   113,   113,   113,    23,    76,     7,    21,    35,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    87,    62,     3,    51,    42,
      43,    83,     3,     4,     5,     6,    69,    93,    94,   130,
     131,    56,    78,   113,   115,   113,    20,   115,   113,   113,
     113,    77,    77,    78,     3,    88,    86,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,     3,     3,     6,    22,    36,    49,
      69,    93,   120,   121,    62,    62,     3,   142,   143,    77,
     126,     4,     5,    76,   134,    26,    76,    78,    78,     3,
      78,    76,    78,    76,    78,    78,   113,   113,    56,    76,
      89,    90,   120,    77,    62,    56,    76,     3,    26,   104,
     127,    52,   131,   135,   136,    78,    27,    62,   113,    87,
      78,   113,    87,   113,    87,    87,     3,    26,    31,    91,
      92,     3,    25,    96,    97,    93,    95,    93,    26,   143,
      76,    78,    77,   128,   129,    87,    76,    77,    93,     3,
      78,     3,    78,     3,     3,    11,    27,    62,    63,    64,
      65,    66,    67,    73,    62,    77,    59,    12,    57,    98,
      99,    76,    78,    62,   104,   130,    76,     3,    30,   137,
     131,   136,    95,    93,    94,    51,    77,    93,    94,    93,
      94,    93,    94,    93,    94,    93,    94,    93,    94,    27,
      93,     6,    92,     3,    25,    38,   100,   101,    93,    93,
      76,    78,   129,    78,    59,    59,    95,    51,    77,    78,
      38,    12,    30,   105,   106,   131,    93,    69,    94,    78,
      95,    12,   102,   103,   104,     4,    37,   107,   108,    78,
     102,    76,     8,    18,    76,     3,   109,   110,   103,     4,
      62,    76,     3,     4,    77,   110,     3,   111,   112,    62,
      76,    78,    93,   112
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

  case 16:

    { pParser->PushQuery(); ;}
    break;

  case 17:

    { pParser->PushQuery(); ;}
    break;

  case 20:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 23:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 24:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 25:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 26:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 27:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 28:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 29:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 30:

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

  case 34:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 40:

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

  case 41:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 42:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 43:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
		;}
    break;

  case 44:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = *yyvsp[-1].m_pValues.Ptr();
			tFilter.m_dValues.Sort();
		;}
    break;

  case 45:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = *yyvsp[-1].m_pValues.Ptr();
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 46:

    {
			if ( !AddUservarFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 47:

    {
			if ( !AddUservarFilter ( pParser, yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 48:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 49:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 50:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 51:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 52:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 56:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 57:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 58:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 59:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 60:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 61:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 62:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 63:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 64:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 65:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 68:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 71:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 74:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 76:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 78:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 79:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 84:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 85:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 92:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 94:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 95:

    {
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 96:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 101:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 102:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 122:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 123:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 125:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 126:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 130:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 131:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 132:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 133:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_LOCAL;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 134:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_LOCAL;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 135:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_LOCAL;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 136:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_GLOBAL_UVAR;
			pParser->m_pStmt->m_sSetName = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 137:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_GLOBAL_SVAR;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 140:

    { yyval.m_iValue = 1; ;}
    break;

  case 141:

    { yyval.m_iValue = 0; ;}
    break;

  case 142:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 143:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 144:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 145:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 148:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 149:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 150:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 153:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 154:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 157:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 158:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 159:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 160:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 161:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 162:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 163:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 164:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 165:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 168:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 170:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 172:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 173:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 176:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 177:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_UPDATE;
			tStmt.m_sIndex = yyvsp[-6].m_sValue;	
			tStmt.m_tUpdate.m_dDocids.Add ( (SphDocID_t) yyvsp[0].m_iValue );
			tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );
		;}
    break;

  case 180:

    {
			CSphAttrUpdate & tUpd = pParser->m_pStmt->m_tUpdate;
			CSphColumnInfo & tAttr = tUpd.m_dAttrs.Add();
			tAttr.m_sName = yyvsp[-2].m_sValue;
			tAttr.m_sName.ToLower();
			tAttr.m_eAttrType = SPH_ATTR_INTEGER; // sorry, ints only for now, riding on legacy shit!
			tUpd.m_dPool.Add ( (DWORD) yyvsp[0].m_iValue );
		;}
    break;

  case 181:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 182:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
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

