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
     TOK_GLOBAL = 277,
     TOK_GROUP = 278,
     TOK_ID = 279,
     TOK_IN = 280,
     TOK_INSERT = 281,
     TOK_INTO = 282,
     TOK_LIMIT = 283,
     TOK_MATCH = 284,
     TOK_MAX = 285,
     TOK_META = 286,
     TOK_MIN = 287,
     TOK_OPTION = 288,
     TOK_ORDER = 289,
     TOK_REPLACE = 290,
     TOK_ROLLBACK = 291,
     TOK_SELECT = 292,
     TOK_SET = 293,
     TOK_SHOW = 294,
     TOK_START = 295,
     TOK_STATUS = 296,
     TOK_SUM = 297,
     TOK_TABLES = 298,
     TOK_TRANSACTION = 299,
     TOK_TRUE = 300,
     TOK_UPDATE = 301,
     TOK_USERVAR = 302,
     TOK_VALUES = 303,
     TOK_WARNINGS = 304,
     TOK_WEIGHT = 305,
     TOK_WHERE = 306,
     TOK_WITHIN = 307,
     TOK_OR = 308,
     TOK_AND = 309,
     TOK_NE = 310,
     TOK_GTE = 311,
     TOK_LTE = 312,
     TOK_NOT = 313,
     TOK_NEG = 314
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
#define TOK_GLOBAL 277
#define TOK_GROUP 278
#define TOK_ID 279
#define TOK_IN 280
#define TOK_INSERT 281
#define TOK_INTO 282
#define TOK_LIMIT 283
#define TOK_MATCH 284
#define TOK_MAX 285
#define TOK_META 286
#define TOK_MIN 287
#define TOK_OPTION 288
#define TOK_ORDER 289
#define TOK_REPLACE 290
#define TOK_ROLLBACK 291
#define TOK_SELECT 292
#define TOK_SET 293
#define TOK_SHOW 294
#define TOK_START 295
#define TOK_STATUS 296
#define TOK_SUM 297
#define TOK_TABLES 298
#define TOK_TRANSACTION 299
#define TOK_TRUE 300
#define TOK_UPDATE 301
#define TOK_USERVAR 302
#define TOK_VALUES 303
#define TOK_WARNINGS 304
#define TOK_WEIGHT 305
#define TOK_WHERE 306
#define TOK_WITHIN 307
#define TOK_OR 308
#define TOK_AND 309
#define TOK_NE 310
#define TOK_GTE 311
#define TOK_LTE 312
#define TOK_NOT 313
#define TOK_NEG 314




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
#define YYFINAL  62
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   522

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  64
/* YYNRULES -- Number of rules. */
#define YYNRULES  171
/* YYNRULES -- Number of states. */
#define YYNSTATES  338

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   314

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    56,     2,
      71,    72,    65,    63,    70,    64,     2,    66,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    69,
      59,    57,    60,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    55,     2,     2,     2,     2,     2,
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
      58,    61,    62,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    34,    36,    38,    49,
      51,    55,    57,    61,    68,    75,    82,    89,    91,    97,
      98,   100,   102,   106,   107,   109,   112,   114,   118,   123,
     127,   131,   135,   141,   148,   152,   157,   163,   167,   171,
     175,   179,   183,   187,   191,   195,   201,   205,   209,   211,
     214,   216,   219,   221,   225,   226,   228,   232,   233,   235,
     241,   242,   244,   248,   250,   254,   256,   259,   262,   264,
     266,   267,   269,   272,   277,   278,   280,   283,   285,   289,
     293,   297,   303,   305,   309,   313,   315,   317,   319,   321,
     324,   327,   331,   335,   339,   343,   347,   351,   355,   359,
     363,   367,   371,   375,   379,   383,   387,   389,   394,   399,
     403,   410,   417,   419,   423,   426,   428,   430,   432,   437,
     442,   450,   452,   454,   456,   458,   460,   462,   464,   467,
     474,   476,   478,   479,   483,   485,   489,   491,   495,   499,
     501,   505,   507,   509,   511,   519,   529,   536,   537,   540,
     542,   546,   550,   552,   554,   557,   559,   561,   564,   573,
     575,   579
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
      74,     0,    -1,    75,    -1,    76,    -1,    76,    69,    -1,
     117,    -1,   125,    -1,   112,    -1,   113,    -1,   115,    -1,
     126,    -1,   131,    -1,   133,    -1,   134,    -1,    77,    -1,
      76,    69,    77,    -1,    78,    -1,   110,    -1,    37,    79,
      21,    82,    83,    90,    92,    94,    99,   101,    -1,    80,
      -1,    79,    70,    80,    -1,     3,    -1,   107,    81,     3,
      -1,     9,    71,   107,    72,    81,     3,    -1,    30,    71,
     107,    72,    81,     3,    -1,    32,    71,   107,    72,    81,
       3,    -1,    42,    71,   107,    72,    81,     3,    -1,    65,
      -1,    15,    71,    19,     3,    72,    -1,    -1,     7,    -1,
       3,    -1,    82,    70,     3,    -1,    -1,    84,    -1,    51,
      85,    -1,    86,    -1,    85,    54,    86,    -1,    29,    71,
       6,    72,    -1,    24,    57,    87,    -1,     3,    57,    87,
      -1,     3,    58,    87,    -1,     3,    25,    71,    89,    72,
      -1,     3,    67,    25,    71,    89,    72,    -1,     3,    25,
      47,    -1,     3,    67,    25,    47,    -1,     3,    11,    87,
      54,    87,    -1,     3,    60,    87,    -1,     3,    59,    87,
      -1,     3,    61,    87,    -1,     3,    62,    87,    -1,     3,
      57,    88,    -1,     3,    58,    88,    -1,     3,    60,    88,
      -1,     3,    59,    88,    -1,     3,    11,    88,    54,    88,
      -1,     3,    61,    88,    -1,     3,    62,    88,    -1,     4,
      -1,    64,     4,    -1,     5,    -1,    64,     5,    -1,    87,
      -1,    89,    70,    87,    -1,    -1,    91,    -1,    23,    12,
       3,    -1,    -1,    93,    -1,    52,    23,    34,    12,    96,
      -1,    -1,    95,    -1,    34,    12,    96,    -1,    97,    -1,
      96,    70,    97,    -1,    98,    -1,    98,     8,    -1,    98,
      17,    -1,    24,    -1,     3,    -1,    -1,   100,    -1,    28,
       4,    -1,    28,     4,    70,     4,    -1,    -1,   102,    -1,
      33,   103,    -1,   104,    -1,   103,    70,   104,    -1,     3,
      57,     3,    -1,     3,    57,     4,    -1,     3,    57,    71,
     105,    72,    -1,   106,    -1,   105,    70,   106,    -1,     3,
      57,    87,    -1,     3,    -1,     4,    -1,     5,    -1,    47,
      -1,    64,   107,    -1,    67,   107,    -1,   107,    63,   107,
      -1,   107,    64,   107,    -1,   107,    65,   107,    -1,   107,
      66,   107,    -1,   107,    59,   107,    -1,   107,    60,   107,
      -1,   107,    56,   107,    -1,   107,    55,   107,    -1,   107,
      62,   107,    -1,   107,    61,   107,    -1,   107,    57,   107,
      -1,   107,    58,   107,    -1,   107,    54,   107,    -1,   107,
      53,   107,    -1,    71,   107,    72,    -1,   108,    -1,     3,
      71,   109,    72,    -1,    25,    71,   109,    72,    -1,     3,
      71,    72,    -1,    32,    71,   107,    70,   107,    72,    -1,
      30,    71,   107,    70,   107,    72,    -1,   107,    -1,   109,
      70,   107,    -1,    39,   111,    -1,    49,    -1,    41,    -1,
      31,    -1,    38,     3,    57,   114,    -1,    38,     3,    57,
       6,    -1,    38,    22,    47,    57,    71,    89,    72,    -1,
      45,    -1,    20,    -1,    87,    -1,    14,    -1,    36,    -1,
     116,    -1,    10,    -1,    40,    44,    -1,   118,    27,     3,
     119,    48,   121,    -1,    26,    -1,    35,    -1,    -1,    71,
     120,    72,    -1,    98,    -1,   120,    70,    98,    -1,   122,
      -1,   121,    70,   122,    -1,    71,   123,    72,    -1,   124,
      -1,   123,    70,   124,    -1,    87,    -1,    88,    -1,     6,
      -1,    16,    21,     3,    51,    24,    57,    87,    -1,    16,
      21,     3,    51,    24,    25,    71,    89,    72,    -1,    13,
       3,    71,   123,   127,    72,    -1,    -1,    70,   128,    -1,
     129,    -1,   128,    70,   129,    -1,   124,    81,   130,    -1,
       3,    -1,    28,    -1,   132,     3,    -1,    18,    -1,    17,
      -1,    39,    43,    -1,    46,     3,    38,   135,    51,    24,
      57,    87,    -1,   136,    -1,   135,    70,   136,    -1,     3,
      57,    87,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   137,   137,   138,   139,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   157,   158,   162,   163,   167,   182,
     183,   187,   188,   189,   190,   191,   192,   193,   194,   209,
     210,   214,   215,   218,   220,   224,   228,   229,   233,   247,
     254,   261,   269,   277,   286,   291,   296,   300,   304,   308,
     312,   316,   317,   318,   319,   324,   328,   332,   339,   340,
     344,   345,   349,   350,   353,   355,   359,   366,   368,   372,
     378,   380,   384,   391,   392,   396,   397,   398,   402,   403,
     406,   408,   412,   417,   424,   426,   430,   434,   435,   439,
     444,   449,   458,   468,   480,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   515,   516,   517,
     518,   519,   523,   524,   530,   534,   535,   536,   542,   549,
     559,   569,   570,   571,   585,   586,   587,   591,   592,   598,
     606,   607,   610,   612,   616,   617,   621,   622,   626,   630,
     631,   635,   636,   637,   643,   649,   661,   668,   670,   674,
     679,   683,   691,   692,   698,   706,   707,   713,   719,   730,
     731,   735
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
  "TOK_FALSE", "TOK_FROM", "TOK_GLOBAL", "TOK_GROUP", "TOK_ID", "TOK_IN", 
  "TOK_INSERT", "TOK_INTO", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", 
  "TOK_META", "TOK_MIN", "TOK_OPTION", "TOK_ORDER", "TOK_REPLACE", 
  "TOK_ROLLBACK", "TOK_SELECT", "TOK_SET", "TOK_SHOW", "TOK_START", 
  "TOK_STATUS", "TOK_SUM", "TOK_TABLES", "TOK_TRANSACTION", "TOK_TRUE", 
  "TOK_UPDATE", "TOK_USERVAR", "TOK_VALUES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "TOK_NOT", "TOK_NEG", "';'", "','", "'('", "')'", "$accept", 
  "request", "statement", "multi_stmt_list", "multi_stmt", "select_from", 
  "select_items_list", "select_item", "opt_as", "ident_list", 
  "opt_where_clause", "where_clause", "where_expr", "where_item", 
  "const_int", "const_float", "const_list", "opt_group_clause", 
  "group_clause", "opt_group_order_clause", "group_order_clause", 
  "opt_order_clause", "order_clause", "order_items_list", "order_item", 
  "ident_or_id", "opt_limit_clause", "limit_clause", "opt_option_clause", 
  "option_clause", "option_list", "option_item", "named_const_list", 
  "named_const", "expr", "function", "arglist", "show_clause", 
  "show_variable", "set_clause", "set_global_clause", "boolean_value", 
  "transact_op", "start_transaction", "insert_into", "insert_or_replace", 
  "opt_column_list", "column_list", "insert_rows_list", "insert_row", 
  "insert_vals_list", "insert_val", "delete_from", "call_proc", 
  "opt_call_opts_list", "call_opts_list", "call_opt", "call_opt_name", 
  "describe", "describe_tok", "show_tables", "update", 
  "update_items_list", "update_item", 0
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
     305,   306,   307,   308,   309,   124,    38,    61,   310,    60,
      62,   311,   312,    43,    45,    42,    47,   313,   314,    59,
      44,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    73,    74,    74,    74,    75,    75,    75,    75,    75,
      75,    75,    75,    75,    76,    76,    77,    77,    78,    79,
      79,    80,    80,    80,    80,    80,    80,    80,    80,    81,
      81,    82,    82,    83,    83,    84,    85,    85,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    87,    87,
      88,    88,    89,    89,    90,    90,    91,    92,    92,    93,
      94,    94,    95,    96,    96,    97,    97,    97,    98,    98,
      99,    99,   100,   100,   101,   101,   102,   103,   103,   104,
     104,   104,   105,   105,   106,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   107,   107,   107,   107,   108,   108,   108,
     108,   108,   109,   109,   110,   111,   111,   111,   112,   112,
     113,   114,   114,   114,   115,   115,   115,   116,   116,   117,
     118,   118,   119,   119,   120,   120,   121,   121,   122,   123,
     123,   124,   124,   124,   125,   125,   126,   127,   127,   128,
     128,   129,   130,   130,   131,   132,   132,   133,   134,   135,
     135,   136
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     1,    10,     1,
       3,     1,     3,     6,     6,     6,     6,     1,     5,     0,
       1,     1,     3,     0,     1,     2,     1,     3,     4,     3,
       3,     3,     5,     6,     3,     4,     5,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     3,     3,     1,     2,
       1,     2,     1,     3,     0,     1,     3,     0,     1,     5,
       0,     1,     3,     1,     3,     1,     2,     2,     1,     1,
       0,     1,     2,     4,     0,     1,     2,     1,     3,     3,
       3,     5,     1,     3,     3,     1,     1,     1,     1,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     4,     4,     3,
       6,     6,     1,     3,     2,     1,     1,     1,     4,     4,
       7,     1,     1,     1,     1,     1,     1,     1,     2,     6,
       1,     1,     0,     3,     1,     3,     1,     3,     3,     1,
       3,     1,     1,     1,     7,     9,     6,     0,     2,     1,
       3,     3,     1,     1,     2,     1,     1,     2,     8,     1,
       3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,   137,     0,   134,     0,   166,   165,   140,   141,   135,
       0,     0,     0,     0,     0,     0,     2,     3,    14,    16,
      17,     7,     8,     9,   136,     5,     0,     6,    10,    11,
       0,    12,    13,     0,     0,    95,    96,    97,     0,     0,
       0,     0,     0,     0,    98,     0,    27,     0,     0,     0,
      19,    29,   116,     0,     0,   127,   126,   167,   125,   124,
     138,     0,     1,     4,     0,   164,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    95,     0,     0,    99,   100,
       0,     0,     0,    30,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    15,   142,    58,    60,   153,     0,   151,
     152,   157,   149,     0,   119,   122,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   115,    31,    33,    20,   114,
     113,   108,   107,   111,   112,   105,   106,   110,   109,   101,
     102,   103,   104,    22,   129,   132,   131,     0,   133,   128,
       0,     0,     0,   169,     0,     0,    59,    61,     0,     0,
       0,     0,   117,    29,     0,   118,     0,    29,     0,    29,
      29,     0,     0,     0,     0,    64,    34,     0,     0,     0,
       0,    79,    78,   144,     0,     0,    29,   158,   159,   156,
       0,     0,   123,     0,    28,     0,     0,     0,     0,     0,
       0,     0,     0,    35,    36,    32,     0,    67,    65,    62,
       0,   171,     0,   170,     0,   143,     0,   139,   146,     0,
       0,     0,   154,    23,   121,    24,   120,    25,    26,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,    68,     0,   130,     0,   145,     0,
       0,   162,   163,   161,    29,   160,     0,     0,     0,    44,
       0,    40,    51,    41,    52,    48,    54,    47,    53,    49,
      56,    50,    57,     0,    39,     0,    37,    66,     0,     0,
      80,    71,    63,   168,     0,   148,   147,   155,     0,     0,
       0,    45,     0,    38,     0,     0,     0,    84,    81,   150,
      46,     0,    55,    42,     0,     0,    72,    73,    75,    82,
       0,    18,    85,    43,    69,     0,    76,    77,     0,     0,
      86,    87,    74,    83,     0,     0,    89,    90,     0,    88,
       0,     0,    92,     0,     0,    91,    94,    93
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    15,    16,    17,    18,    19,    49,    50,   219,   127,
     175,   176,   203,   204,   109,   110,   210,   207,   208,   243,
     244,   280,   281,   306,   307,   308,   297,   298,   311,   312,
     320,   321,   331,   332,    51,    52,   116,    20,    59,    21,
      22,   149,    23,    24,    25,    26,   155,   184,   217,   218,
     111,   112,    27,    28,   159,   187,   188,   253,    29,    30,
      31,    32,   152,   153
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -215
static const short yypact[] =
{
     240,  -215,   118,  -215,   123,  -215,  -215,  -215,  -215,  -215,
     138,    86,    70,   106,   163,   174,  -215,   109,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,  -215,   148,  -215,  -215,  -215,
     176,  -215,  -215,   111,   181,     6,  -215,  -215,   115,   116,
     117,   121,   124,   125,  -215,   151,  -215,   151,   151,   -13,
    -215,   183,  -215,   137,   150,  -215,  -215,  -215,  -215,  -215,
    -215,   161,  -215,    -5,   197,  -215,     5,   153,    35,   151,
     182,   151,   151,   151,   151,   135,   136,   139,  -215,  -215,
     274,   205,   138,  -215,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   209,    52,
     154,   210,    79,  -215,   143,  -215,  -215,  -215,   168,  -215,
    -215,   146,  -215,   193,  -215,   410,   -35,   294,   216,    28,
     234,   254,   314,   151,   151,  -215,  -215,    42,  -215,   423,
     435,   446,   456,   208,   208,    94,    94,    94,    94,    38,
      38,  -215,  -215,  -215,  -215,  -215,  -215,   217,  -215,  -215,
     149,   172,    44,  -215,    85,   184,  -215,  -215,     5,   158,
      34,   151,  -215,   224,   179,  -215,   151,   224,   151,   224,
     224,   374,   392,    61,   232,   229,  -215,    19,    19,   231,
     210,  -215,  -215,  -215,    45,   188,    -4,   190,  -215,  -215,
     191,    19,   410,   258,  -215,   334,   260,   354,   261,   262,
     166,   225,   212,   227,  -215,  -215,   272,   233,  -215,  -215,
      54,  -215,   244,  -215,    85,  -215,     5,   235,  -215,    33,
       5,    19,  -215,  -215,  -215,  -215,  -215,  -215,  -215,     9,
      15,     9,     9,     9,     9,     9,     9,   277,    19,   297,
      61,   318,   299,   289,  -215,    19,  -215,    19,  -215,    59,
     188,  -215,  -215,  -215,   224,  -215,    68,   271,   287,  -215,
      19,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,
    -215,  -215,  -215,    23,  -215,   270,  -215,  -215,   309,   332,
     317,  -215,  -215,  -215,     5,  -215,  -215,  -215,    19,     7,
      92,  -215,    19,  -215,   349,    85,   358,   330,  -215,  -215,
    -215,   359,  -215,  -215,    95,    85,   295,  -215,    88,   311,
     379,  -215,  -215,  -215,   295,    85,  -215,  -215,   380,   326,
     315,  -215,  -215,  -215,     3,   379,  -215,  -215,   398,  -215,
     345,    99,  -215,    19,   398,  -215,  -215,  -215
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -215,  -215,  -215,  -215,   340,  -215,  -215,   322,   -18,  -215,
    -215,  -215,  -215,   165,   -99,  -214,  -205,  -215,  -215,  -215,
    -215,  -215,  -215,   119,   107,  -130,  -215,  -215,  -215,  -215,
    -215,    96,  -215,    89,   -43,  -215,   370,  -215,  -215,  -215,
    -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,  -215,   175,
     226,  -157,  -215,  -215,  -215,  -215,   223,  -215,  -215,  -215,
    -215,  -215,  -215,   279
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -151
static const short yytable[] =
{
     148,   186,    78,    83,    79,    80,   326,   327,    81,   105,
     106,   107,   106,   105,   106,   258,   256,   262,   264,   266,
     268,   270,   272,   105,   183,   115,   117,   -21,   115,   120,
     121,   122,    10,    98,   102,   161,   251,   162,    75,    36,
      37,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   290,   105,    82,   144,   190,
      40,   252,   259,   254,   200,    76,  -150,    77,  -150,   108,
     291,   301,   145,   108,   328,   302,   -21,    68,   209,   211,
     171,   172,    44,   147,   248,   201,   260,   304,   181,    53,
     202,   191,   222,   173,   292,   179,   316,   146,   161,    45,
     165,    55,    47,    96,    97,   317,    48,   114,    54,   182,
      55,    56,   174,    57,   180,   214,   147,   215,   192,    58,
      56,    33,   209,   195,   245,   197,   246,   299,    58,   284,
     257,   285,   261,   263,   265,   267,   269,   271,   245,   274,
     287,    35,    36,    37,    34,   193,   282,    38,   283,   196,
      60,   198,   199,    39,    75,    36,    37,    94,    95,    96,
      97,   209,   245,    40,   303,   245,    61,   313,    41,   334,
      42,   335,   156,   157,    62,    64,    40,   229,    63,    65,
      43,    76,    66,    77,    67,    44,    69,    70,    71,   300,
      83,   230,    72,   209,    99,    73,    74,   100,    44,   101,
     104,   118,    45,    46,   113,    47,    68,   123,   126,    48,
     124,   150,   143,   151,   154,    45,   158,   160,    47,   164,
     177,   156,    48,   231,   232,   233,   234,   235,   236,   178,
     189,    83,   185,   237,   336,   205,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
       1,   194,   206,     2,     3,   212,     4,     5,     6,   216,
     220,   223,   221,   225,   227,   228,     7,    90,    91,    92,
      93,    94,    95,    96,    97,     8,     9,    10,    11,    12,
      13,   240,   238,   239,   241,   242,    14,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   247,   273,   275,   166,   250,   167,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   277,   278,   279,   168,   288,   169,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   289,   293,   294,   295,   296,   125,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   305,   309,   310,   157,   315,   163,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   318,   319,   324,   323,   325,   170,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   330,   333,   103,   128,   276,   224,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   329,   322,   337,   314,   286,   226,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,   119,   249,   255,   166,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,   213,
       0,     0,   168,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97
};

static const short yycheck[] =
{
      99,   158,    45,     7,    47,    48,     3,     4,    21,     4,
       5,     6,     5,     4,     5,   229,   221,   231,   232,   233,
     234,   235,   236,     4,   154,    68,    69,    21,    71,    72,
      73,    74,    37,    51,    39,    70,     3,    72,     3,     4,
       5,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,   260,     4,    70,     6,    25,
      25,    28,    47,   220,     3,    30,    70,    32,    72,    64,
      47,    64,    20,    64,    71,   289,    70,    71,   177,   178,
     123,   124,    47,    64,   214,    24,    71,   292,     3,     3,
      29,    57,   191,    51,    71,    51,     8,    45,    70,    64,
      72,    31,    67,    65,    66,    17,    71,    72,    22,    24,
      31,    41,    70,    43,    70,    70,    64,    72,   161,    49,
      41,     3,   221,   166,    70,   168,    72,   284,    49,    70,
     229,    72,   231,   232,   233,   234,   235,   236,    70,   238,
      72,     3,     4,     5,    21,   163,   245,     9,   247,   167,
      44,   169,   170,    15,     3,     4,     5,    63,    64,    65,
      66,   260,    70,    25,    72,    70,     3,    72,    30,    70,
      32,    72,     4,     5,     0,    27,    25,    11,    69,     3,
      42,    30,    71,    32,     3,    47,    71,    71,    71,   288,
       7,    25,    71,   292,    57,    71,    71,    47,    47,    38,
       3,    19,    64,    65,    51,    67,    71,    71,     3,    71,
      71,    57,     3,     3,    71,    64,    70,    24,    67,     3,
      71,     4,    71,    57,    58,    59,    60,    61,    62,    57,
      72,     7,    48,    67,   333,     3,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      10,    72,    23,    13,    14,    24,    16,    17,    18,    71,
      70,     3,    71,     3,     3,     3,    26,    59,    60,    61,
      62,    63,    64,    65,    66,    35,    36,    37,    38,    39,
      40,    54,    57,    71,    12,    52,    46,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    57,    25,     6,    70,    70,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,     3,    23,    34,    70,    54,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    54,    72,    34,    12,    28,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    12,     4,    33,     5,    70,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    70,     3,    57,     4,    70,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,     3,    57,    63,    82,   240,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,   325,   315,   334,   305,   250,    72,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    71,   216,   220,    70,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,   180,
      -1,    -1,    70,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    10,    13,    14,    16,    17,    18,    26,    35,    36,
      37,    38,    39,    40,    46,    74,    75,    76,    77,    78,
     110,   112,   113,   115,   116,   117,   118,   125,   126,   131,
     132,   133,   134,     3,    21,     3,     4,     5,     9,    15,
      25,    30,    32,    42,    47,    64,    65,    67,    71,    79,
      80,   107,   108,     3,    22,    31,    41,    43,    49,   111,
      44,     3,     0,    69,    27,     3,    71,     3,    71,    71,
      71,    71,    71,    71,    71,     3,    30,    32,   107,   107,
     107,    21,    70,     7,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    81,    57,
      47,    38,    39,    77,     3,     4,     5,     6,    64,    87,
      88,   123,   124,    51,    72,   107,   109,   107,    19,   109,
     107,   107,   107,    71,    71,    72,     3,    82,    80,   107,
     107,   107,   107,   107,   107,   107,   107,   107,   107,   107,
     107,   107,   107,     3,     6,    20,    45,    64,    87,   114,
      57,     3,   135,   136,    71,   119,     4,     5,    70,   127,
      24,    70,    72,    72,     3,    72,    70,    72,    70,    72,
      72,   107,   107,    51,    70,    83,    84,    71,    57,    51,
      70,     3,    24,    98,   120,    48,   124,   128,   129,    72,
      25,    57,   107,    81,    72,   107,    81,   107,    81,    81,
       3,    24,    29,    85,    86,     3,    23,    90,    91,    87,
      89,    87,    24,   136,    70,    72,    71,   121,   122,    81,
      70,    71,    87,     3,    72,     3,    72,     3,     3,    11,
      25,    57,    58,    59,    60,    61,    62,    67,    57,    71,
      54,    12,    52,    92,    93,    70,    72,    57,    98,   123,
      70,     3,    28,   130,   124,   129,    89,    87,    88,    47,
      71,    87,    88,    87,    88,    87,    88,    87,    88,    87,
      88,    87,    88,    25,    87,     6,    86,     3,    23,    34,
      94,    95,    87,    87,    70,    72,   122,    72,    54,    54,
      89,    47,    71,    72,    34,    12,    28,    99,   100,   124,
      87,    64,    88,    72,    89,    12,    96,    97,    98,     4,
      33,   101,   102,    72,    96,    70,     8,    17,    70,     3,
     103,   104,    97,     4,    57,    70,     3,     4,    71,   104,
       3,   105,   106,    57,    70,    72,    87,   106
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

  case 14:

    { pParser->PushQuery(); ;}
    break;

  case 15:

    { pParser->PushQuery(); ;}
    break;

  case 18:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+yyvsp[-6].m_iStart, yyvsp[-6].m_iEnd-yyvsp[-6].m_iStart );
		;}
    break;

  case 21:

    { pParser->SetSelect ( yyvsp[0].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 22:

    { pParser->SetSelect ( yyvsp[-2].m_iStart, yyvsp[0].m_iEnd ); pParser->AddItem ( &yyvsp[-2], &yyvsp[0] ); ;}
    break;

  case 23:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_AVG ); ;}
    break;

  case 24:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MAX ); ;}
    break;

  case 25:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_MIN ); ;}
    break;

  case 26:

    { pParser->SetSelect (yyvsp[-5].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[-3], &yyvsp[0], SPH_AGGR_SUM ); ;}
    break;

  case 27:

    { pParser->SetSelect (yyvsp[0].m_iStart, yyvsp[0].m_iEnd); pParser->AddItem ( &yyvsp[0], NULL ); ;}
    break;

  case 28:

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

  case 32:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 38:

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

  case 39:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
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
		;}
    break;

  case 41:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-2].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( yyvsp[0].m_iValue );
			tFilter.m_bExclude = true;
		;}
    break;

  case 42:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-4].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 43:

    {
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = yyvsp[-5].m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = yyvsp[-1].m_dValues;
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Sort();
		;}
    break;

  case 44:

    {
			if ( !AddUservarFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_sValue, false ) )
				YYERROR;
		;}
    break;

  case 45:

    {
			if ( !AddUservarFilter ( pParser, yyvsp[-3].m_sValue, yyvsp[-1].m_sValue, true ) )
				YYERROR;
		;}
    break;

  case 46:

    {
			AddUintRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_iValue, yyvsp[0].m_iValue );
		;}
    break;

  case 47:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue+1, UINT_MAX );
		;}
    break;

  case 48:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue-1 );
		;}
    break;

  case 49:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_iValue, UINT_MAX );
		;}
    break;

  case 50:

    {
			AddUintRangeFilter ( pParser, yyvsp[-2].m_sValue, 0, yyvsp[0].m_iValue );
		;}
    break;

  case 54:

    {
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		;}
    break;

  case 55:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-4].m_sValue, yyvsp[-2].m_fValue, yyvsp[0].m_fValue );
		;}
    break;

  case 56:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, yyvsp[0].m_fValue, FLT_MAX );
		;}
    break;

  case 57:

    {
			AddFloatRangeFilter ( pParser, yyvsp[-2].m_sValue, -FLT_MAX, yyvsp[0].m_fValue );
		;}
    break;

  case 58:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 59:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = -yyvsp[0].m_iValue; ;}
    break;

  case 60:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 61:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 62:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 63:

    { yyval.m_dValues.Add ( yyvsp[0].m_iValue ); ;}
    break;

  case 66:

    {
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = yyvsp[0].m_sValue;
		;}
    break;

  case 69:

    {
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 72:

    {
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+yyvsp[0].m_iStart, yyvsp[0].m_iEnd-yyvsp[0].m_iStart );
		;}
    break;

  case 74:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 76:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 77:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 82:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 83:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 89:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 90:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 91:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 92:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 93:

    {
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( yyval.m_iValue );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = yyvsp[0].m_sValue;
			dVec.Last().m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 94:

    {
			yyval.m_sValue = yyvsp[-2].m_sValue;
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 99:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 100:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
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

  case 117:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 118:

    { yyval = yyvsp[-3]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 119:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 120:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 121:

    { yyval = yyvsp[-5]; yyval.m_iEnd = yyvsp[0].m_iEnd ;}
    break;

  case 125:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 126:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 127:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 128:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_bSetGlobal = false;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 129:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_bSetGlobal = false;
			pParser->m_pStmt->m_sSetName = yyvsp[-2].m_sValue;
			pParser->m_pStmt->m_sSetValue = yyvsp[0].m_sValue;
		;}
    break;

  case 130:

    {
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_bSetGlobal = true;
			pParser->m_pStmt->m_sSetName = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dSetValues = yyvsp[-1].m_dValues;
		;}
    break;

  case 131:

    { yyval.m_iValue = 1; ;}
    break;

  case 132:

    { yyval.m_iValue = 0; ;}
    break;

  case 133:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 134:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 135:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 136:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 139:

    {
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = yyvsp[-3].m_sValue;
		;}
    break;

  case 140:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 141:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 144:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 145:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 148:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 149:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 150:

    { AddInsval ( pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 151:

    { yyval.m_iInstype = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 152:

    { yyval.m_iInstype = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 153:

    { yyval.m_iInstype = TOK_QUOTED_STRING; yyval.m_sValue = yyvsp[0].m_sValue; ;}
    break;

  case 154:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-4].m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 155:

    {
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = yyvsp[-6].m_sValue;
			ARRAY_FOREACH ( i, yyvsp[-1].m_dValues )
				pParser->m_pStmt->m_dDeleteIds.Add ( yyvsp[-1].m_dValues[i] );
		;}
    break;

  case 156:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = yyvsp[-4].m_sValue;
		;}
    break;

  case 159:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 161:

    {
			pParser->m_pStmt->m_dCallOptNames.Add ( yyvsp[0].m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 163:

    { yyval.m_sValue = "limit"; ;}
    break;

  case 164:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = yyvsp[0].m_sValue;
		;}
    break;

  case 167:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 168:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_UPDATE;
			tStmt.m_sIndex = yyvsp[-6].m_sValue;	
			tStmt.m_tUpdate.m_dDocids.Add ( (SphDocID_t) yyvsp[0].m_iValue );
			tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );
		;}
    break;

  case 171:

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

