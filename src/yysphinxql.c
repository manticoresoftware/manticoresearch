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
     TOK_CREATE = 272,
     TOK_DELETE = 273,
     TOK_DESC = 274,
     TOK_DESCRIBE = 275,
     TOK_DISTINCT = 276,
     TOK_DIV = 277,
     TOK_DROP = 278,
     TOK_FALSE = 279,
     TOK_FLOAT = 280,
     TOK_FROM = 281,
     TOK_FUNCTION = 282,
     TOK_GLOBAL = 283,
     TOK_GROUP = 284,
     TOK_ID = 285,
     TOK_IN = 286,
     TOK_INSERT = 287,
     TOK_INT = 288,
     TOK_INTO = 289,
     TOK_LIMIT = 290,
     TOK_MATCH = 291,
     TOK_MAX = 292,
     TOK_META = 293,
     TOK_MIN = 294,
     TOK_MOD = 295,
     TOK_NULL = 296,
     TOK_OPTION = 297,
     TOK_ORDER = 298,
     TOK_REPLACE = 299,
     TOK_RETURNS = 300,
     TOK_ROLLBACK = 301,
     TOK_SELECT = 302,
     TOK_SET = 303,
     TOK_SHOW = 304,
     TOK_SONAME = 305,
     TOK_START = 306,
     TOK_STATUS = 307,
     TOK_SUM = 308,
     TOK_TABLES = 309,
     TOK_TRANSACTION = 310,
     TOK_TRUE = 311,
     TOK_UPDATE = 312,
     TOK_USERVAR = 313,
     TOK_VALUES = 314,
     TOK_VARIABLES = 315,
     TOK_WARNINGS = 316,
     TOK_WEIGHT = 317,
     TOK_WHERE = 318,
     TOK_WITHIN = 319,
     TOK_OR = 320,
     TOK_AND = 321,
     TOK_NE = 322,
     TOK_GTE = 323,
     TOK_LTE = 324,
     TOK_NOT = 325,
     TOK_NEG = 326
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
#define TOK_CREATE 272
#define TOK_DELETE 273
#define TOK_DESC 274
#define TOK_DESCRIBE 275
#define TOK_DISTINCT 276
#define TOK_DIV 277
#define TOK_DROP 278
#define TOK_FALSE 279
#define TOK_FLOAT 280
#define TOK_FROM 281
#define TOK_FUNCTION 282
#define TOK_GLOBAL 283
#define TOK_GROUP 284
#define TOK_ID 285
#define TOK_IN 286
#define TOK_INSERT 287
#define TOK_INT 288
#define TOK_INTO 289
#define TOK_LIMIT 290
#define TOK_MATCH 291
#define TOK_MAX 292
#define TOK_META 293
#define TOK_MIN 294
#define TOK_MOD 295
#define TOK_NULL 296
#define TOK_OPTION 297
#define TOK_ORDER 298
#define TOK_REPLACE 299
#define TOK_RETURNS 300
#define TOK_ROLLBACK 301
#define TOK_SELECT 302
#define TOK_SET 303
#define TOK_SHOW 304
#define TOK_SONAME 305
#define TOK_START 306
#define TOK_STATUS 307
#define TOK_SUM 308
#define TOK_TABLES 309
#define TOK_TRANSACTION 310
#define TOK_TRUE 311
#define TOK_UPDATE 312
#define TOK_USERVAR 313
#define TOK_VALUES 314
#define TOK_VARIABLES 315
#define TOK_WARNINGS 316
#define TOK_WEIGHT 317
#define TOK_WHERE 318
#define TOK_WITHIN 319
#define TOK_OR 320
#define TOK_AND 321
#define TOK_NE 322
#define TOK_GTE 323
#define TOK_LTE 324
#define TOK_NOT 325
#define TOK_NEG 326




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
#define YYFINAL  72
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   651

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  70
/* YYNRULES -- Number of rules. */
#define YYNRULES  188
/* YYNRULES -- Number of states. */
#define YYNSTATES  368

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   326

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    79,    68,     2,
      84,    85,    77,    75,    83,    76,     2,    78,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    82,
      71,    69,    72,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    67,     2,     2,     2,     2,     2,
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
      65,    66,    70,    73,    74,    80,    81
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      42,    44,    46,    57,    59,    63,    65,    69,    76,    83,
      90,    97,    99,   105,   106,   108,   110,   114,   115,   117,
     120,   122,   126,   131,   135,   139,   143,   149,   156,   160,
     165,   171,   175,   179,   183,   187,   191,   195,   199,   203,
     209,   213,   217,   219,   222,   224,   227,   229,   233,   234,
     236,   240,   241,   243,   249,   250,   252,   256,   258,   262,
     264,   267,   270,   272,   274,   275,   277,   280,   285,   286,
     288,   291,   293,   297,   301,   305,   311,   313,   317,   321,
     323,   325,   327,   329,   332,   335,   339,   343,   347,   351,
     355,   359,   363,   367,   371,   375,   379,   383,   387,   391,
     395,   399,   403,   407,   409,   414,   419,   423,   430,   437,
     439,   443,   446,   448,   450,   452,   457,   462,   467,   475,
     481,   483,   485,   487,   489,   491,   493,   495,   497,   499,
     502,   509,   511,   513,   514,   518,   520,   524,   526,   530,
     534,   536,   540,   542,   544,   546,   554,   564,   571,   572,
     575,   577,   581,   585,   587,   589,   592,   594,   596,   599,
     608,   610,   614,   618,   621,   624,   632,   634,   636
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      87,     0,    -1,    88,    -1,    89,    -1,    89,    82,    -1,
     131,    -1,   139,    -1,   125,    -1,   126,    -1,   129,    -1,
     140,    -1,   145,    -1,   147,    -1,   148,    -1,   151,    -1,
     152,    -1,   153,    -1,   155,    -1,    90,    -1,    89,    82,
      90,    -1,    91,    -1,   123,    -1,    47,    92,    26,    95,
      96,   103,   105,   107,   112,   114,    -1,    93,    -1,    92,
      83,    93,    -1,     3,    -1,   120,    94,     3,    -1,     9,
      84,   120,    85,    94,     3,    -1,    37,    84,   120,    85,
      94,     3,    -1,    39,    84,   120,    85,    94,     3,    -1,
      53,    84,   120,    85,    94,     3,    -1,    77,    -1,    16,
      84,    21,     3,    85,    -1,    -1,     7,    -1,     3,    -1,
      95,    83,     3,    -1,    -1,    97,    -1,    63,    98,    -1,
      99,    -1,    98,    66,    99,    -1,    36,    84,     6,    85,
      -1,    30,    69,   100,    -1,     3,    69,   100,    -1,     3,
      70,   100,    -1,     3,    31,    84,   102,    85,    -1,     3,
      80,    31,    84,   102,    85,    -1,     3,    31,    58,    -1,
       3,    80,    31,    58,    -1,     3,    11,   100,    66,   100,
      -1,     3,    72,   100,    -1,     3,    71,   100,    -1,     3,
      73,   100,    -1,     3,    74,   100,    -1,     3,    69,   101,
      -1,     3,    70,   101,    -1,     3,    72,   101,    -1,     3,
      71,   101,    -1,     3,    11,   101,    66,   101,    -1,     3,
      73,   101,    -1,     3,    74,   101,    -1,     4,    -1,    76,
       4,    -1,     5,    -1,    76,     5,    -1,   100,    -1,   102,
      83,   100,    -1,    -1,   104,    -1,    29,    12,     3,    -1,
      -1,   106,    -1,    64,    29,    43,    12,   109,    -1,    -1,
     108,    -1,    43,    12,   109,    -1,   110,    -1,   109,    83,
     110,    -1,   111,    -1,   111,     8,    -1,   111,    19,    -1,
      30,    -1,     3,    -1,    -1,   113,    -1,    35,     4,    -1,
      35,     4,    83,     4,    -1,    -1,   115,    -1,    42,   116,
      -1,   117,    -1,   116,    83,   117,    -1,     3,    69,     3,
      -1,     3,    69,     4,    -1,     3,    69,    84,   118,    85,
      -1,   119,    -1,   118,    83,   119,    -1,     3,    69,   100,
      -1,     3,    -1,     4,    -1,     5,    -1,    58,    -1,    76,
     120,    -1,    80,   120,    -1,   120,    75,   120,    -1,   120,
      76,   120,    -1,   120,    77,   120,    -1,   120,    78,   120,
      -1,   120,    71,   120,    -1,   120,    72,   120,    -1,   120,
      68,   120,    -1,   120,    67,   120,    -1,   120,    79,   120,
      -1,   120,    22,   120,    -1,   120,    40,   120,    -1,   120,
      74,   120,    -1,   120,    73,   120,    -1,   120,    69,   120,
      -1,   120,    70,   120,    -1,   120,    66,   120,    -1,   120,
      65,   120,    -1,    84,   120,    85,    -1,   121,    -1,     3,
      84,   122,    85,    -1,    31,    84,   122,    85,    -1,     3,
      84,    85,    -1,    39,    84,   120,    83,   120,    85,    -1,
      37,    84,   120,    83,   120,    85,    -1,   120,    -1,   122,
      83,   120,    -1,    49,   124,    -1,    61,    -1,    52,    -1,
      38,    -1,    48,     3,    69,   128,    -1,    48,     3,    69,
     127,    -1,    48,     3,    69,    41,    -1,    48,    28,    58,
      69,    84,   102,    85,    -1,    48,    28,     3,    69,   127,
      -1,     3,    -1,     6,    -1,    56,    -1,    24,    -1,   100,
      -1,    15,    -1,    46,    -1,   130,    -1,    10,    -1,    51,
      55,    -1,   132,    34,     3,   133,    59,   135,    -1,    32,
      -1,    44,    -1,    -1,    84,   134,    85,    -1,   111,    -1,
     134,    83,   111,    -1,   136,    -1,   135,    83,   136,    -1,
      84,   137,    85,    -1,   138,    -1,   137,    83,   138,    -1,
     100,    -1,   101,    -1,     6,    -1,    18,    26,     3,    63,
      30,    69,   100,    -1,    18,    26,     3,    63,    30,    31,
      84,   102,    85,    -1,    13,     3,    84,   137,   141,    85,
      -1,    -1,    83,   142,    -1,   143,    -1,   142,    83,   143,
      -1,   138,    94,   144,    -1,     3,    -1,    35,    -1,   146,
       3,    -1,    20,    -1,    19,    -1,    49,    54,    -1,    57,
       3,    48,   149,    63,    30,    69,   100,    -1,   150,    -1,
     149,    83,   150,    -1,     3,    69,   100,    -1,    49,    60,
      -1,    49,    14,    -1,    17,    27,     3,    45,   154,    50,
       6,    -1,    33,    -1,    25,    -1,    23,    27,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   150,   150,   151,   152,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   174,   175,
     179,   180,   184,   199,   200,   204,   205,   206,   207,   208,
     209,   210,   211,   226,   227,   231,   232,   235,   237,   241,
     245,   246,   250,   264,   271,   278,   286,   294,   303,   308,
     313,   317,   321,   325,   329,   333,   334,   335,   336,   341,
     345,   349,   356,   357,   361,   362,   366,   372,   378,   380,
     384,   391,   393,   397,   403,   405,   409,   416,   417,   421,
     422,   423,   427,   428,   431,   433,   437,   442,   449,   451,
     455,   459,   460,   464,   469,   474,   483,   493,   505,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,   529,   530,   531,   532,   533,   534,   535,
     536,   537,   538,   539,   543,   544,   545,   546,   547,   551,
     552,   558,   562,   563,   564,   570,   577,   584,   594,   601,
     611,   612,   616,   617,   618,   632,   633,   634,   638,   639,
     645,   653,   654,   657,   659,   663,   664,   668,   669,   673,
     677,   678,   682,   683,   684,   690,   696,   708,   715,   717,
     721,   726,   730,   738,   739,   745,   753,   754,   760,   766,
     777,   778,   782,   796,   800,   806,   817,   818,   822
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
  "TOK_COMMIT", "TOK_COUNT", "TOK_CREATE", "TOK_DELETE", "TOK_DESC", 
  "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DROP", "TOK_FALSE", 
  "TOK_FLOAT", "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", 
  "TOK_ID", "TOK_IN", "TOK_INSERT", "TOK_INT", "TOK_INTO", "TOK_LIMIT", 
  "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_NULL", 
  "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", "TOK_RETURNS", "TOK_ROLLBACK", 
  "TOK_SELECT", "TOK_SET", "TOK_SHOW", "TOK_SONAME", "TOK_START", 
  "TOK_STATUS", "TOK_SUM", "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", 
  "TOK_UPDATE", "TOK_USERVAR", "TOK_VALUES", "TOK_VARIABLES", 
  "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE", "TOK_WITHIN", "TOK_OR", 
  "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'", "'>'", "TOK_GTE", 
  "TOK_LTE", "'+'", "'-'", "'*'", "'/'", "'%'", "TOK_NOT", "TOK_NEG", 
  "';'", "','", "'('", "')'", "$accept", "request", "statement", 
  "multi_stmt_list", "multi_stmt", "select_from", "select_items_list", 
  "select_item", "opt_as", "ident_list", "opt_where_clause", 
  "where_clause", "where_expr", "where_item", "const_int", "const_float", 
  "const_list", "opt_group_clause", "group_clause", 
  "opt_group_order_clause", "group_order_clause", "opt_order_clause", 
  "order_clause", "order_items_list", "order_item", "ident_or_id", 
  "opt_limit_clause", "limit_clause", "opt_option_clause", 
  "option_clause", "option_list", "option_item", "named_const_list", 
  "named_const", "expr", "function", "arglist", "show_clause", 
  "show_variable", "set_clause", "set_global_clause", "set_string_value", 
  "boolean_value", "transact_op", "start_transaction", "insert_into", 
  "insert_or_replace", "opt_column_list", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "delete_from", "call_proc", "opt_call_opts_list", "call_opts_list", 
  "call_opt", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "update", "update_items_list", "update_item", "show_variables", 
  "show_collation", "create_function", "udf_type", "drop_function", 0
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
     315,   316,   317,   318,   319,   320,   321,   124,    38,    61,
     322,    60,    62,   323,   324,    43,    45,    42,    47,    37,
     325,   326,    59,    44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    86,    87,    87,    87,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    89,    89,
      90,    90,    91,    92,    92,    93,    93,    93,    93,    93,
      93,    93,    93,    94,    94,    95,    95,    96,    96,    97,
      98,    98,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,   100,   100,   101,   101,   102,   102,   103,   103,
     104,   105,   105,   106,   107,   107,   108,   109,   109,   110,
     110,   110,   111,   111,   112,   112,   113,   113,   114,   114,
     115,   116,   116,   117,   117,   117,   118,   118,   119,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,   120,   121,   121,   121,   121,   121,   122,
     122,   123,   124,   124,   124,   125,   125,   125,   126,   126,
     127,   127,   128,   128,   128,   129,   129,   129,   130,   130,
     131,   132,   132,   133,   133,   134,   134,   135,   135,   136,
     137,   137,   138,   138,   138,   139,   139,   140,   141,   141,
     142,   142,   143,   144,   144,   145,   146,   146,   147,   148,
     149,   149,   150,   151,   152,   153,   154,   154,   155
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,    10,     1,     3,     1,     3,     6,     6,     6,
       6,     1,     5,     0,     1,     1,     3,     0,     1,     2,
       1,     3,     4,     3,     3,     3,     5,     6,     3,     4,
       5,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       3,     3,     1,     2,     1,     2,     1,     3,     0,     1,
       3,     0,     1,     5,     0,     1,     3,     1,     3,     1,
       2,     2,     1,     1,     0,     1,     2,     4,     0,     1,
       2,     1,     3,     3,     3,     5,     1,     3,     3,     1,
       1,     1,     1,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     4,     4,     3,     6,     6,     1,
       3,     2,     1,     1,     1,     4,     4,     4,     7,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       6,     1,     1,     0,     3,     1,     3,     1,     3,     3,
       1,     3,     1,     1,     1,     7,     9,     6,     0,     2,
       1,     3,     3,     1,     1,     2,     1,     1,     2,     8,
       1,     3,     3,     2,     2,     7,     1,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   148,     0,   145,     0,     0,   177,   176,     0,   151,
     152,   146,     0,     0,     0,     0,     0,     0,     2,     3,
      18,    20,    21,     7,     8,     9,   147,     5,     0,     6,
      10,    11,     0,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,    99,   100,   101,     0,     0,     0,     0,
       0,     0,   102,     0,    31,     0,     0,     0,    23,    33,
     123,     0,     0,   184,   134,   133,   178,   183,   132,   131,
     149,     0,     1,     4,     0,   175,     0,     0,     0,   188,
       0,     0,     0,     0,     0,     0,     0,    99,     0,     0,
     103,   104,     0,     0,     0,    34,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
     153,    62,    64,   164,     0,   162,   163,   168,   160,     0,
       0,   126,   129,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   122,    35,    37,    24,   114,   115,   121,   120,
     112,   111,   118,   119,   109,   110,   117,   116,   105,   106,
     107,   108,   113,    26,   140,   141,   143,   137,   142,     0,
     144,   136,   135,     0,     0,     0,     0,   180,     0,     0,
      63,    65,     0,     0,   187,   186,     0,     0,     0,   124,
      33,     0,   125,     0,    33,     0,    33,    33,     0,     0,
       0,     0,    68,    38,   139,     0,     0,     0,     0,    83,
      82,   155,     0,     0,    33,   169,   170,   167,     0,     0,
       0,   130,     0,    32,     0,     0,     0,     0,     0,     0,
       0,     0,    39,    40,    36,     0,    71,    69,    66,     0,
     182,     0,   181,     0,   154,     0,   150,   157,     0,     0,
     185,     0,   165,    27,   128,    28,   127,    29,    30,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    74,    72,     0,   138,     0,   156,     0,
       0,   173,   174,   172,    33,   171,     0,     0,     0,    48,
       0,    44,    55,    45,    56,    52,    58,    51,    57,    53,
      60,    54,    61,     0,    43,     0,    41,    70,     0,     0,
      84,    75,    67,   179,     0,   159,   158,   166,     0,     0,
       0,    49,     0,    42,     0,     0,     0,    88,    85,   161,
      50,     0,    59,    46,     0,     0,    76,    77,    79,    86,
       0,    22,    89,    47,    73,     0,    80,    81,     0,     0,
      90,    91,    78,    87,     0,     0,    93,    94,     0,    92,
       0,     0,    96,     0,     0,    95,    98,    97
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    17,    18,    19,    20,    21,    57,    58,   248,   144,
     202,   203,   232,   233,   125,   126,   239,   236,   237,   273,
     274,   310,   311,   336,   337,   338,   327,   328,   341,   342,
     350,   351,   361,   362,    59,    60,   133,    22,    69,    23,
      24,   171,   172,    25,    26,    27,    28,   179,   212,   246,
     247,   127,   128,    29,    30,   183,   215,   216,   283,    31,
      32,    33,    34,   176,   177,    35,    36,    37,   186,    38
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -229
static const short yypact[] =
{
     594,  -229,    33,  -229,    -3,    39,  -229,  -229,    50,  -229,
    -229,  -229,   161,    14,   174,    16,   106,   113,  -229,    36,
    -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,    80,  -229,
    -229,  -229,   122,  -229,  -229,  -229,  -229,  -229,  -229,    42,
     139,   159,   168,    -8,  -229,  -229,    94,    96,   107,   112,
     118,   121,  -229,   164,  -229,   164,   164,   -10,  -229,   187,
    -229,   137,    23,  -229,  -229,  -229,  -229,  -229,  -229,  -229,
    -229,   142,  -229,    75,   207,  -229,     4,   180,   167,  -229,
      35,   164,   211,   164,   164,   164,   164,   149,   152,   162,
    -229,  -229,   281,   240,   161,  -229,   164,   164,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,   164,
     164,   164,   164,   244,    99,   181,   182,   280,   155,  -229,
     200,  -229,  -229,  -229,    15,  -229,  -229,   203,  -229,    83,
     258,  -229,   464,    61,   302,   286,    73,   202,   260,   342,
     164,   164,  -229,  -229,   -42,  -229,  -229,  -229,   322,   403,
     483,   505,   524,   524,    21,    21,    21,    21,    95,    95,
    -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,   287,
    -229,  -229,  -229,    24,   206,   223,    44,  -229,    34,   234,
    -229,  -229,     4,   209,  -229,  -229,   245,    -6,   164,  -229,
     289,   212,  -229,   164,   289,   164,   289,   289,   424,   443,
      65,   295,   270,  -229,  -229,    11,    11,   271,   280,  -229,
    -229,  -229,   104,   218,     0,   221,  -229,  -229,   299,   222,
      11,   464,   304,  -229,   363,   305,   384,   306,   307,    59,
     242,   228,   247,  -229,  -229,   303,   250,  -229,  -229,   114,
    -229,   248,  -229,    34,  -229,     4,   233,  -229,    32,     4,
    -229,    11,  -229,  -229,  -229,  -229,  -229,  -229,  -229,     8,
      20,     8,     8,     8,     8,     8,     8,   288,    11,   312,
      65,   317,   293,   297,  -229,    11,  -229,    11,  -229,   128,
     218,  -229,  -229,  -229,   289,  -229,   132,   257,   275,  -229,
      11,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,
    -229,  -229,  -229,    54,  -229,   276,  -229,  -229,   320,   353,
     348,  -229,  -229,  -229,     4,  -229,  -229,  -229,    11,     6,
     135,  -229,    11,  -229,   372,    34,   382,   360,  -229,  -229,
    -229,   399,  -229,  -229,   138,    34,   339,  -229,   102,   340,
     402,  -229,  -229,  -229,   339,    34,  -229,  -229,   422,   375,
     362,  -229,  -229,  -229,     2,   402,  -229,  -229,   444,  -229,
     397,   146,  -229,    11,   444,  -229,  -229,  -229
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -229,  -229,  -229,  -229,   394,  -229,  -229,   374,   -37,  -229,
    -229,  -229,  -229,   214,  -114,   -80,  -228,  -229,  -229,  -229,
    -229,  -229,  -229,   150,   143,  -164,  -229,  -229,  -229,  -229,
    -229,   151,  -229,   123,   -52,  -229,   441,  -229,  -229,  -229,
    -229,   352,  -229,  -229,  -229,  -229,  -229,  -229,  -229,  -229,
     264,   283,  -180,  -229,  -229,  -229,  -229,   298,  -229,  -229,
    -229,  -229,  -229,  -229,   341,  -229,  -229,  -229,  -229,  -229
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -162
static const short yytable[] =
{
     170,    90,   214,    91,    92,   356,   357,    95,   121,   122,
     123,   122,   121,   122,   211,   121,    93,    61,   -25,   180,
     181,   200,   113,   286,    40,   219,   115,   164,   132,   134,
     165,   132,   137,   138,   139,   281,    39,   209,    87,    44,
      45,   201,    62,    96,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,    97,   320,   220,   210,    41,    48,   282,   229,   284,
     259,    70,    88,    94,    89,   -25,    80,    42,   289,   278,
     124,   116,   331,  -161,   124,  -161,   358,   169,   198,   199,
     260,   238,   240,    52,   334,   230,   108,   109,   110,   111,
     112,   231,   164,   121,   290,   165,   252,   207,   184,    71,
     346,    53,   321,    72,    74,    55,   185,    96,    73,    56,
     131,   347,    12,   166,   118,    75,    76,   208,   261,   262,
     263,   264,   265,   266,   329,    97,   221,   238,   322,   267,
     167,   224,    77,   226,   188,   287,   189,   291,   293,   295,
     297,   299,   301,   222,   304,   168,   188,   225,   192,   227,
     228,   312,    78,   313,    43,    44,    45,    87,    44,    45,
      46,    79,   110,   111,   112,   169,   238,    47,    81,   288,
      82,   292,   294,   296,   298,   300,   302,   243,    63,   244,
     117,    83,    48,    64,    95,    48,    84,   275,    49,   276,
      50,    88,    85,    89,   330,    86,   114,    65,   238,    96,
     120,   314,    64,   315,    51,   275,    68,   317,   275,    52,
     333,   275,    52,   343,    96,   129,    65,    97,    66,   364,
     130,   365,   135,    80,    67,    68,   140,    53,    54,   332,
      53,    55,    97,   143,    55,    56,   141,   163,    56,   366,
     173,   174,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    96,   175,   178,   193,   182,   194,   187,   191,
     205,   180,   206,   213,   217,   218,    95,   223,   234,   235,
      97,   241,   245,    96,   249,   250,   251,   253,   255,   257,
     258,   268,   269,   270,   272,   271,   280,   277,   305,   303,
     307,    97,   308,   318,    96,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     309,   319,    97,   195,    96,   196,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   323,    97,   324,    96,   325,   142,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    97,   326,   335,    96,   339,   190,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   340,    97,   181,   349,    96,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   345,   348,    97,    96,   353,   197,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    97,   354,   355,    96,   360,   254,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    97,    96,   363,   119,   145,   256,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    97,   306,   344,    96,   367,   352,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    97,    96,   359,   193,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    97,   136,   204,   195,    96,   279,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   316,    97,    96,   285,     0,   242,
       0,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,     0,    97,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   104,   105,   106,   107,   108,
     109,   110,   111,   112,     1,     0,     0,     2,     0,     3,
       0,     4,     5,     6,     7,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     9,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
      11,    12,    13,    14,     0,    15,     0,     0,     0,     0,
       0,    16
};

static const short yycheck[] =
{
     114,    53,   182,    55,    56,     3,     4,     7,     4,     5,
       6,     5,     4,     5,   178,     4,    26,     3,    26,     4,
       5,    63,    59,   251,    27,    31,     3,     3,    80,    81,
       6,    83,    84,    85,    86,     3,     3,     3,     3,     4,
       5,    83,    28,    22,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    40,   290,    69,    30,    26,    31,    35,     3,   249,
      11,    55,    37,    83,    39,    83,    84,    27,    58,   243,
      76,    58,    76,    83,    76,    85,    84,    76,   140,   141,
      31,   205,   206,    58,   322,    30,    75,    76,    77,    78,
      79,    36,     3,     4,    84,     6,   220,    63,    25,     3,
       8,    76,    58,     0,    34,    80,    33,    22,    82,    84,
      85,    19,    47,    24,    49,     3,    84,    83,    69,    70,
      71,    72,    73,    74,   314,    40,   188,   251,    84,    80,
      41,   193,     3,   195,    83,   259,    85,   261,   262,   263,
     264,   265,   266,   190,   268,    56,    83,   194,    85,   196,
     197,   275,     3,   277,     3,     4,     5,     3,     4,     5,
       9,     3,    77,    78,    79,    76,   290,    16,    84,   259,
      84,   261,   262,   263,   264,   265,   266,    83,    14,    85,
      48,    84,    31,    38,     7,    31,    84,    83,    37,    85,
      39,    37,    84,    39,   318,    84,    69,    52,   322,    22,
       3,    83,    38,    85,    53,    83,    61,    85,    83,    58,
      85,    83,    58,    85,    22,    45,    52,    40,    54,    83,
      63,    85,    21,    84,    60,    61,    84,    76,    77,   319,
      76,    80,    40,     3,    80,    84,    84,     3,    84,   363,
      69,    69,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    22,     3,    84,    83,    83,    85,    30,     3,
      84,     4,    69,    59,    85,    50,     7,    85,     3,    29,
      40,    30,    84,    22,    83,     6,    84,     3,     3,     3,
       3,    69,    84,    66,    64,    12,    83,    69,     6,    31,
       3,    40,    29,    66,    22,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      43,    66,    40,    83,    22,    85,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    85,    40,    43,    22,    12,    85,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    40,    35,    12,    22,     4,    85,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    42,    40,     5,     3,    22,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    83,    83,    40,    22,     4,    85,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    40,    69,    83,    22,     3,    85,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    40,    22,    69,    73,    94,    85,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    40,   270,   335,    22,   364,   345,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    40,    22,   355,    83,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    40,    83,   173,    83,    22,   245,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,   280,    40,    22,   249,    -1,   208,
      -1,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    10,    -1,    -1,    13,    -1,    15,
      -1,    17,    18,    19,    20,    -1,    -1,    23,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,
      46,    47,    48,    49,    -1,    51,    -1,    -1,    -1,    -1,
      -1,    57
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    10,    13,    15,    17,    18,    19,    20,    23,    32,
      44,    46,    47,    48,    49,    51,    57,    87,    88,    89,
      90,    91,   123,   125,   126,   129,   130,   131,   132,   139,
     140,   145,   146,   147,   148,   151,   152,   153,   155,     3,
      27,    26,    27,     3,     4,     5,     9,    16,    31,    37,
      39,    53,    58,    76,    77,    80,    84,    92,    93,   120,
     121,     3,    28,    14,    38,    52,    54,    60,    61,   124,
      55,     3,     0,    82,    34,     3,    84,     3,     3,     3,
      84,    84,    84,    84,    84,    84,    84,     3,    37,    39,
     120,   120,   120,    26,    83,     7,    22,    40,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    94,    69,     3,    58,    48,    49,    90,
       3,     4,     5,     6,    76,   100,   101,   137,   138,    45,
      63,    85,   120,   122,   120,    21,   122,   120,   120,   120,
      84,    84,    85,     3,    95,    93,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   120,     3,     3,     6,    24,    41,    56,    76,
     100,   127,   128,    69,    69,     3,   149,   150,    84,   133,
       4,     5,    83,   141,    25,    33,   154,    30,    83,    85,
      85,     3,    85,    83,    85,    83,    85,    85,   120,   120,
      63,    83,    96,    97,   127,    84,    69,    63,    83,     3,
      30,   111,   134,    59,   138,   142,   143,    85,    50,    31,
      69,   120,    94,    85,   120,    94,   120,    94,    94,     3,
      30,    36,    98,    99,     3,    29,   103,   104,   100,   102,
     100,    30,   150,    83,    85,    84,   135,   136,    94,    83,
       6,    84,   100,     3,    85,     3,    85,     3,     3,    11,
      31,    69,    70,    71,    72,    73,    74,    80,    69,    84,
      66,    12,    64,   105,   106,    83,    85,    69,   111,   137,
      83,     3,    35,   144,   138,   143,   102,   100,   101,    58,
      84,   100,   101,   100,   101,   100,   101,   100,   101,   100,
     101,   100,   101,    31,   100,     6,    99,     3,    29,    43,
     107,   108,   100,   100,    83,    85,   136,    85,    66,    66,
     102,    58,    84,    85,    43,    12,    35,   112,   113,   138,
     100,    76,   101,    85,   102,    12,   109,   110,   111,     4,
      42,   114,   115,    85,   109,    83,     8,    19,    83,     3,
     116,   117,   110,     4,    69,    83,     3,     4,    84,   117,
       3,   118,   119,    69,    83,    85,   100,   119
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

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 26:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 27:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 28:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 29:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 30:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 31:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 32:

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

  case 36:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 42:

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

  case 43:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 44:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 45:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
		;}
    break;

  case 46:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = *yyvsp[-1].m_pValues.Ptr();
			tFilter.m_dValues.Sort();
		;}
    break;

  case 47:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = *yyvsp[-1].m_pValues.Ptr();
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 48:

    {
			if ( !AddUservarFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 49:

    {
			if ( !AddUservarFilter ( pParser, yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 50:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 51:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 52:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 53:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 54:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 58:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 59:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 60:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 61:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 62:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 63:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 64:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 65:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 66:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 67:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 70:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 73:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 76:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 78:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 80:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 81:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 86:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 87:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 93:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 94:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 95:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 96:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 97:

    {
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 98:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 103:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 104:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 121:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 122:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 124:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 125:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 126:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 127:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 128:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 132:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 133:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 134:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 135:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_LOCAL;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 136:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_LOCAL;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 137:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_LOCAL;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 138:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_GLOBAL_UVAR;
			pParser->m_pStmt->m_sSetName = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 139:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_eSet = SET_GLOBAL_SVAR;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 142:

    { yyval.m_iValue = 1; ;}
    break;

  case 143:

    { yyval.m_iValue = 0; ;}
    break;

  case 144:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 145:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 146:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 147:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 150:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 151:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 152:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 155:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 156:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 159:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 160:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 161:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 162:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 163:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 164:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 165:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 166:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			for ( int i=0; i<yyvsp[-1].m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*yyvsp[-1].m_pValues.Ptr())[i] );
		;}
    break;

  case 167:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 170:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 172:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 174:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 175:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 178:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 179:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_UPDATE;
			tStmt.m_sIndex = yyvsp[-6].m_sValue;	
			tStmt.m_tUpdate.m_dDocids.Add ( (SphDocID_t) yyvsp[0].m_iValue );
			tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );
		;}
    break;

  case 182:

    {
			CSphAttrUpdate & tUpd = pParser->m_pStmt->m_tUpdate;
			CSphColumnInfo & tAttr = tUpd.m_dAttrs.Add();
			tAttr.m_sName = yyvsp[-2].m_sValue;
			tAttr.m_sName.ToLower();
			tAttr.m_eAttrType = SPH_ATTR_INTEGER; // sorry, ints only for now, riding on legacy shit!
			tUpd.m_dPool.Add ( (DWORD) yyvsp[0].m_iValue );
		;}
    break;

  case 183:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 184:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 185:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = yyvsp[-4].m_sValue;
			tStmt.m_sUdfLib = yyvsp[0].m_sValue;
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 186:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 187:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 188:

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

