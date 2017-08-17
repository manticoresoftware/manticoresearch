/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */


#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif



// some helpers
#include <float.h> // for FLT_MAX

static void AddInsval ( SqlParser_c * pParser, CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iType;
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	if ( tIns.m_iType==TOK_QUOTED_STRING )
		pParser->ToStringUnescape ( tIns.m_sVal, tNode );
	tIns.m_pVals = tNode.m_pValues;
}

static CSphFilterSettings * AddMvaRange ( SqlParser_c * pParser, const SqlNode_t & tNode, int64_t iMin, int64_t iMax )
{
	CSphFilterSettings * f = pParser->AddFilter ( tNode, SPH_FILTER_RANGE );
	f->m_eMvaFunc = ( tNode.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
	f->m_iMinValue = iMin;
	f->m_iMaxValue = iMax;
	return f;
}

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	if ( _res.m_iStart>0 && pParser->m_pBuf[_res.m_iStart-1]=='`' ) \
		_res.m_iStart--; \
	_res.m_iEnd = _right.m_iEnd; \
	_res.m_iType = 0;




# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "yysphinxql.h".  */
#ifndef YY_YY_YYSPHINXQL_H_INCLUDED
# define YY_YY_YYSPHINXQL_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_IDENT = 258,
    TOK_ATIDENT = 259,
    TOK_CONST_INT = 260,
    TOK_CONST_FLOAT = 261,
    TOK_CONST_MVA = 262,
    TOK_QUOTED_STRING = 263,
    TOK_USERVAR = 264,
    TOK_SYSVAR = 265,
    TOK_CONST_STRINGS = 266,
    TOK_BAD_NUMERIC = 267,
    TOK_SUBKEY = 268,
    TOK_DOT_NUMBER = 269,
    TOK_ADD = 270,
    TOK_AGENT = 271,
    TOK_ALTER = 272,
    TOK_ALL = 273,
    TOK_ANY = 274,
    TOK_AS = 275,
    TOK_ASC = 276,
    TOK_ATTACH = 277,
    TOK_ATTRIBUTES = 278,
    TOK_AVG = 279,
    TOK_BEGIN = 280,
    TOK_BETWEEN = 281,
    TOK_BIGINT = 282,
    TOK_BOOL = 283,
    TOK_BY = 284,
    TOK_CALL = 285,
    TOK_CHARACTER = 286,
    TOK_CHUNK = 287,
    TOK_COLLATION = 288,
    TOK_COLUMN = 289,
    TOK_COMMIT = 290,
    TOK_COMMITTED = 291,
    TOK_COUNT = 292,
    TOK_CREATE = 293,
    TOK_DATABASES = 294,
    TOK_DELETE = 295,
    TOK_DESC = 296,
    TOK_DESCRIBE = 297,
    TOK_DISTINCT = 298,
    TOK_DIV = 299,
    TOK_DOUBLE = 300,
    TOK_DROP = 301,
    TOK_FACET = 302,
    TOK_FALSE = 303,
    TOK_FLOAT = 304,
    TOK_FLUSH = 305,
    TOK_FOR = 306,
    TOK_FROM = 307,
    TOK_FUNCTION = 308,
    TOK_GLOBAL = 309,
    TOK_GROUP = 310,
    TOK_GROUPBY = 311,
    TOK_GROUP_CONCAT = 312,
    TOK_HAVING = 313,
    TOK_HOSTNAMES = 314,
    TOK_ID = 315,
    TOK_IN = 316,
    TOK_INDEX = 317,
    TOK_INDEXOF = 318,
    TOK_INSERT = 319,
    TOK_INT = 320,
    TOK_INTEGER = 321,
    TOK_INTO = 322,
    TOK_IS = 323,
    TOK_ISOLATION = 324,
    TOK_JSON = 325,
    TOK_LEVEL = 326,
    TOK_LIKE = 327,
    TOK_LIMIT = 328,
    TOK_MATCH = 329,
    TOK_MAX = 330,
    TOK_META = 331,
    TOK_MIN = 332,
    TOK_MOD = 333,
    TOK_MULTI = 334,
    TOK_MULTI64 = 335,
    TOK_NAMES = 336,
    TOK_NULL = 337,
    TOK_OPTION = 338,
    TOK_ORDER = 339,
    TOK_OPTIMIZE = 340,
    TOK_PLAN = 341,
    TOK_PLUGIN = 342,
    TOK_PLUGINS = 343,
    TOK_PROFILE = 344,
    TOK_RAND = 345,
    TOK_RAMCHUNK = 346,
    TOK_READ = 347,
    TOK_RECONFIGURE = 348,
    TOK_RELOAD = 349,
    TOK_REPEATABLE = 350,
    TOK_REPLACE = 351,
    TOK_REMAP = 352,
    TOK_RETURNS = 353,
    TOK_ROLLBACK = 354,
    TOK_RTINDEX = 355,
    TOK_SELECT = 356,
    TOK_SERIALIZABLE = 357,
    TOK_SET = 358,
    TOK_SETTINGS = 359,
    TOK_SESSION = 360,
    TOK_SHOW = 361,
    TOK_SONAME = 362,
    TOK_START = 363,
    TOK_STATUS = 364,
    TOK_STRING = 365,
    TOK_SUM = 366,
    TOK_TABLE = 367,
    TOK_TABLES = 368,
    TOK_THREADS = 369,
    TOK_TO = 370,
    TOK_TRANSACTION = 371,
    TOK_TRUE = 372,
    TOK_TRUNCATE = 373,
    TOK_TYPE = 374,
    TOK_UNCOMMITTED = 375,
    TOK_UPDATE = 376,
    TOK_VALUES = 377,
    TOK_VARIABLES = 378,
    TOK_WARNINGS = 379,
    TOK_WEIGHT = 380,
    TOK_WHERE = 381,
    TOK_WITH = 382,
    TOK_WITHIN = 383,
    TOK_OR = 384,
    TOK_AND = 385,
    TOK_NE = 386,
    TOK_LTE = 387,
    TOK_GTE = 388,
    TOK_NOT = 389,
    TOK_NEG = 390
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (SqlParser_c * pParser);

#endif /* !YY_YY_YYSPHINXQL_H_INCLUDED  */

/* Copy the second part of user declarations.  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  257
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5428

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  154
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  146
/* YYNRULES -- Number of rules.  */
#define YYNRULES  522
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  925

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   390

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   143,   132,     2,
     147,   148,   141,   139,   149,   140,     2,   142,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   146,
     135,   133,   136,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   152,     2,   153,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   150,   131,   151,     2,     2,     2,     2,
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
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   134,   137,   138,   144,
     145
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   189,   189,   190,   191,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
     210,   211,   212,   213,   214,   215,   216,   217,   218,   219,
     220,   221,   222,   223,   238,   239,   239,   239,   239,   239,
     239,   239,   239,   240,   240,   240,   240,   240,   240,   241,
     241,   241,   241,   241,   242,   242,   242,   242,   242,   243,
     243,   243,   243,   243,   244,   244,   244,   244,   244,   244,
     245,   245,   245,   245,   245,   245,   245,   246,   246,   246,
     246,   246,   246,   247,   247,   247,   247,   247,   247,   248,
     248,   248,   248,   248,   249,   249,   249,   249,   249,   250,
     250,   250,   250,   250,   251,   251,   251,   251,   251,   252,
     252,   252,   252,   252,   252,   253,   253,   253,   253,   253,
     253,   254,   254,   254,   254,   254,   255,   255,   255,   255,
     255,   259,   259,   259,   265,   266,   267,   271,   272,   273,
     277,   278,   286,   287,   294,   296,   300,   304,   311,   312,
     313,   317,   330,   337,   339,   344,   353,   369,   370,   374,
     375,   378,   380,   381,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   397,   398,   401,   403,   407,   411,   412,
     413,   417,   425,   426,   427,   428,   433,   440,   448,   456,
     465,   470,   475,   480,   485,   490,   495,   500,   505,   510,
     515,   520,   525,   530,   535,   540,   545,   550,   555,   560,
     565,   570,   575,   582,   588,   599,   606,   615,   619,   628,
     632,   636,   640,   647,   648,   653,   659,   665,   671,   677,
     678,   679,   680,   681,   685,   686,   690,   691,   702,   703,
     704,   708,   714,   721,   727,   733,   735,   738,   740,   747,
     751,   757,   759,   765,   767,   771,   782,   784,   788,   792,
     799,   800,   804,   805,   806,   809,   811,   815,   820,   827,
     829,   833,   837,   838,   842,   847,   852,   857,   863,   868,
     876,   881,   888,   898,   899,   900,   901,   902,   903,   904,
     905,   906,   908,   909,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   921,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   935,   936,   937,   938,
     939,   940,   941,   942,   943,   944,   945,   946,   947,   948,
     952,   953,   957,   958,   962,   963,   964,   968,   969,   973,
     974,   978,   979,   985,   988,   990,   994,   995,   996,   997,
     998,   999,  1000,  1001,  1002,  1007,  1012,  1017,  1022,  1031,
    1032,  1035,  1037,  1045,  1050,  1055,  1060,  1061,  1062,  1066,
    1071,  1076,  1081,  1090,  1091,  1095,  1096,  1097,  1109,  1110,
    1114,  1115,  1116,  1117,  1118,  1125,  1126,  1127,  1131,  1132,
    1138,  1146,  1147,  1150,  1152,  1156,  1157,  1161,  1162,  1166,
    1167,  1171,  1175,  1176,  1180,  1181,  1182,  1183,  1184,  1187,
    1188,  1192,  1193,  1197,  1203,  1213,  1221,  1225,  1232,  1233,
    1240,  1250,  1256,  1258,  1262,  1267,  1271,  1278,  1280,  1284,
    1285,  1291,  1299,  1300,  1306,  1310,  1316,  1324,  1325,  1329,
    1343,  1349,  1353,  1358,  1372,  1383,  1384,  1385,  1386,  1387,
    1388,  1389,  1390,  1391,  1395,  1403,  1410,  1421,  1425,  1432,
    1433,  1437,  1441,  1442,  1446,  1450,  1457,  1464,  1470,  1471,
    1472,  1476,  1477,  1478,  1479,  1485,  1496,  1497,  1498,  1499,
    1500,  1505,  1516,  1525,  1527,  1536,  1545,  1554,  1562,  1572,
    1580,  1581,  1585,  1589,  1593,  1603,  1614,  1625,  1636,  1646,
    1657,  1658,  1662,  1665,  1666,  1670,  1671,  1672,  1673,  1677,
    1678,  1682,  1687,  1689,  1693,  1702,  1706,  1714,  1715,  1719,
    1730,  1732,  1739
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_ATIDENT",
  "TOK_CONST_INT", "TOK_CONST_FLOAT", "TOK_CONST_MVA", "TOK_QUOTED_STRING",
  "TOK_USERVAR", "TOK_SYSVAR", "TOK_CONST_STRINGS", "TOK_BAD_NUMERIC",
  "TOK_SUBKEY", "TOK_DOT_NUMBER", "TOK_ADD", "TOK_AGENT", "TOK_ALTER",
  "TOK_ALL", "TOK_ANY", "TOK_AS", "TOK_ASC", "TOK_ATTACH",
  "TOK_ATTRIBUTES", "TOK_AVG", "TOK_BEGIN", "TOK_BETWEEN", "TOK_BIGINT",
  "TOK_BOOL", "TOK_BY", "TOK_CALL", "TOK_CHARACTER", "TOK_CHUNK",
  "TOK_COLLATION", "TOK_COLUMN", "TOK_COMMIT", "TOK_COMMITTED",
  "TOK_COUNT", "TOK_CREATE", "TOK_DATABASES", "TOK_DELETE", "TOK_DESC",
  "TOK_DESCRIBE", "TOK_DISTINCT", "TOK_DIV", "TOK_DOUBLE", "TOK_DROP",
  "TOK_FACET", "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FOR",
  "TOK_FROM", "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY",
  "TOK_GROUP_CONCAT", "TOK_HAVING", "TOK_HOSTNAMES", "TOK_ID", "TOK_IN",
  "TOK_INDEX", "TOK_INDEXOF", "TOK_INSERT", "TOK_INT", "TOK_INTEGER",
  "TOK_INTO", "TOK_IS", "TOK_ISOLATION", "TOK_JSON", "TOK_LEVEL",
  "TOK_LIKE", "TOK_LIMIT", "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN",
  "TOK_MOD", "TOK_MULTI", "TOK_MULTI64", "TOK_NAMES", "TOK_NULL",
  "TOK_OPTION", "TOK_ORDER", "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PLUGIN",
  "TOK_PLUGINS", "TOK_PROFILE", "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ",
  "TOK_RECONFIGURE", "TOK_RELOAD", "TOK_REPEATABLE", "TOK_REPLACE",
  "TOK_REMAP", "TOK_RETURNS", "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT",
  "TOK_SERIALIZABLE", "TOK_SET", "TOK_SETTINGS", "TOK_SESSION", "TOK_SHOW",
  "TOK_SONAME", "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM",
  "TOK_TABLE", "TOK_TABLES", "TOK_THREADS", "TOK_TO", "TOK_TRANSACTION",
  "TOK_TRUE", "TOK_TRUNCATE", "TOK_TYPE", "TOK_UNCOMMITTED", "TOK_UPDATE",
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", "TOK_WHERE",
  "TOK_WITH", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='",
  "TOK_NE", "'<'", "'>'", "TOK_LTE", "TOK_GTE", "'+'", "'-'", "'*'", "'/'",
  "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", "'}'",
  "'['", "']'", "$accept", "request", "statement", "ident_set", "ident",
  "multi_stmt_list", "multi_stmt", "select", "select1",
  "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg",
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from",
  "select_items_list", "select_item", "opt_alias", "select_expr",
  "ident_list", "opt_where_clause", "where_clause", "where_expr",
  "where_item", "filter_expr", "filter_item", "expr_ident", "mva_aggr",
  "const_int", "const_float", "const_list", "string_list",
  "opt_group_clause", "opt_int", "group_items_list", "opt_having_clause",
  "opt_group_order_clause", "group_order_clause", "opt_order_clause",
  "order_clause", "order_items_list", "order_item", "opt_limit_clause",
  "limit_clause", "opt_option_clause", "option_clause", "option_list",
  "option_item", "named_const_list", "named_const", "expr", "function",
  "arglist", "arg", "json_aggr", "consthash", "hash_key", "hash_val",
  "show_stmt", "like_filter", "show_what", "index_or_table", "opt_chunk",
  "set_stmt", "set_global_stmt", "set_string_value", "boolean_value",
  "set_value", "simple_set_value", "transact_op", "start_transaction",
  "insert_into", "insert_or_replace", "opt_column_list", "column_ident",
  "column_list", "insert_rows_list", "insert_row", "insert_vals_list",
  "insert_val", "opt_insert_options", "insert_options_list",
  "insert_option", "delete_from", "call_proc", "call_args_list",
  "call_arg", "const_string_list", "opt_call_opts_list", "call_opts_list",
  "call_opt", "opt_as", "call_opt_name", "describe", "describe_tok",
  "show_tables", "show_databases", "update", "update_items_list",
  "update_item", "alter_col_type", "alter", "show_variables",
  "opt_show_variables_where", "show_variables_where",
  "show_variables_where_list", "show_variables_where_entry",
  "show_collation", "show_character_set", "set_transaction", "opt_scope",
  "isolation_level", "create_function", "udf_type", "drop_function",
  "attach_index", "opt_with_truncate", "flush_rtindex", "flush_ramchunk",
  "flush_index", "flush_hostnames", "select_sysvar", "sysvar_list",
  "sysvar_item", "sysvar_name", "select_dual", "truncate",
  "optimize_index", "create_plugin", "drop_plugin", "reload_plugins",
  "json_field", "json_expr", "subscript", "subkey", "streq", "strval",
  "opt_facet_by_items_list", "facet_by", "facet_item", "facet_expr",
  "facet_items_list", "facet_stmt", "opt_reload_index_from",
  "reload_index", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
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
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   124,    38,    61,   386,    60,    62,   387,   388,    43,
      45,    42,    47,    37,   389,   390,    59,    40,    41,    44,
     123,   125,    91,    93
};
# endif

#define YYPACT_NINF -802

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-802)))

#define YYTABLE_NINF -512

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    5060,    20,    18,  -802,  4450,  -802,    36,    43,  -802,  -802,
      50,    34,  -802,    62,   130,  -802,  -802,   894,  4576,   613,
      -3,    78,  4450,   140,  -802,   -31,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,   124,  -802,  -802,  -802,
    4450,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  4450,  4450,  4450,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,    38,  4450,  4450,
    4450,  4450,  4450,  -802,  -802,  4450,  4450,  4450,  4450,   158,
      96,  -802,  -802,  -802,  -802,  -802,  -802,  -802,   126,   128,
     139,   154,   168,   172,   179,   182,   186,  -802,   192,   200,
     209,   213,   244,   250,   252,   254,   256,  1930,  -802,  1930,
    1930,  3868,    41,   -35,  -802,  3982,    14,  -802,   257,   -41,
    -802,  3982,   178,   237,  -802,   253,   278,   309,  4702,  4450,
    3250,   298,   282,   300,  4108,   314,  -802,   347,  -802,  -802,
     347,  -802,  -802,  -802,  -802,   347,  -802,   347,   337,  -802,
    -802,  4450,   302,  -802,  4450,  -802,   -50,  -802,  1930,   148,
    -802,  4450,   347,   336,    75,   315,    42,   351,   331,    13,
    -802,   332,  -802,  -802,   337,   400,   346,  1042,  1930,  2078,
     -10,  2078,  2078,   306,  1930,  2078,  2078,  1930,  1930,  1190,
    1930,  1930,   310,   312,   313,   316,  -802,  -802,  5000,  -802,
    -802,    85,   324,  -802,  -802,  2226,    51,  -802,  2745,  1338,
    4450,  -802,  -802,  1930,  1930,  1930,  1930,  1930,  1930,  1930,
    1930,  1930,  1930,  1930,  1930,  1930,  1930,  1930,  1930,  1930,
    1930,   456,   452,  -802,  -802,  -802,   -33,  1930,  3250,  3250,
     334,   335,   411,  -802,  -802,  -802,  -802,  -802,   329,  -802,
    2872,   401,   375,     7,   377,  -802,   480,  -802,  -802,  -802,
    -802,  4450,  -802,  -802,    27,    12,  -802,  4450,  4450,  5252,
    -802,  3982,    49,  1486,  4828,   286,  -802,   343,  -802,  -802,
     458,   459,   394,  -802,  -802,  -802,  -802,    99,    35,  -802,
    -802,  -802,   348,  -802,   476,   487,  2364,  -802,   488,  -802,
     490,  -802,   491,   367,  1634,  -802,  5252,    47,  -802,  5068,
     104,  4450,   379,   107,   111,  -802,  5105,   116,   121,   610,
     660,  -802,   123,   728,  5135,  -802,  1782,  1930,  1930,  -802,
    3868,  -802,  3124,   370,   378,  -802,  -802,    13,  -802,  5252,
    -802,  -802,  -802,  5272,  5285,   340,  4980,   153,  -802,   153,
     238,   238,   238,   238,    63,    63,  -802,  -802,  -802,  5237,
     380,  -802,  -802,   444,   153,   329,   329,   381,  2998,   521,
    3250,  -802,  -802,  -802,  -802,   527,  -802,  -802,  -802,  -802,
     462,   347,  -802,   347,  -802,   403,   385,  -802,   433,   533,
    -802,   435,   532,  4450,  -802,  -802,     9,    86,  -802,   410,
    -802,  -802,  -802,  1930,   460,  1930,  4222,   425,  4450,  4450,
    4450,  -802,  -802,  -802,  -802,  -802,   129,   132,    42,   402,
    -802,  -802,  -802,  -802,  -802,   441,   442,  -802,   404,   407,
     409,   412,   413,   415,   416,  -802,   417,   418,   419,  2491,
      51,  -802,   427,   137,  -802,   176,   207,  -802,  -802,  -802,
    -802,  1338,   420,  -802,  2078,  -802,  -802,   421,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  1930,  -802,  1930,  -802,  1930,
    -802,  4832,  4900,   434,  -802,  -802,  -802,  -802,  -802,   469,
     516,  -802,  4450,   567,  -802,    46,  -802,  -802,   443,  -802,
     146,  -802,  -802,  2618,  4450,  -802,  -802,  -802,  -802,   446,
     445,  -802,    82,  4450,   337,    80,  -802,   546,   504,  -802,
     431,  -802,  -802,  -802,   149,   436,   303,  -802,   454,  -802,
      46,  -802,   574,    52,  -802,   438,  -802,  -802,   576,   577,
    4450,  4450,  4450,   447,  4450,   448,   450,  4450,   583,   467,
      94,  2491,  2491,  2491,    80,    30,    77,    23,    92,    80,
      80,    80,    80,    56,    46,   461,    46,    46,    46,    46,
      46,    46,    65,   457,  -802,  -802,  5171,  5206,  4932,  3124,
    1338,   468,   587,   465,   534,  -802,   151,   470,    -2,   502,
    -802,  -802,  -802,  -802,  4450,   471,  -802,   611,  4450,    39,
    -802,  -802,  -802,  -802,  -802,  -802,  3376,  -802,  -802,  4222,
      69,   -48,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,   506,  -802,  -802,  -802,  -802,  4336,    69,
    -802,  -802,   472,   477,    51,   483,   484,   486,  -802,  -802,
     493,   495,  -802,  -802,   137,   505,  -802,   515,   517,  -802,
      68,  -802,   554,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,    46,    37,   518,
      46,  -802,  -802,  -802,  -802,  -802,  -802,    46,   503,  3754,
     501,  -802,  -802,   508,  -802,   -22,   572,  -802,   622,   602,
     600,  -802,  4450,  -802,    46,  -802,  -802,  -802,   526,   159,
    -802,   652,  -802,  -802,  -802,   169,   514,    81,   513,  -802,
    -802,    40,   173,  -802,   661,   436,  -802,  -802,  -802,  -802,
    -802,   643,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
      80,    80,  -802,   175,   188,  -802,   536,  -802,    68,    46,
     201,   538,    46,  -802,  -802,  -802,   522,  -802,  -802,  2078,
    4450,   641,   601,  3502,   589,  3628,   460,    51,   528,  -802,
     205,    46,  -802,  4450,   529,  -802,   530,  -802,  -802,  3502,
    -802,    69,   548,   537,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,   671,    46,   210,   217,  -802,  -802,    46,   222,
    3754,   239,  3502,   677,  -802,  -802,   541,   662,  -802,   504,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,   685,   661,
    -802,  -802,  -802,  -802,  -802,  -802,  -802,   545,   513,   547,
    3502,  3502,   337,  -802,  -802,   550,   693,  -802,   513,  -802,
    2078,  -802,   241,   552,  -802
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   388,     0,   385,     0,     0,   433,   432,
       0,     0,   391,     0,     0,   392,   386,     0,   468,   468,
       0,     0,     0,     0,     2,     3,   134,   137,   140,   142,
     138,   139,     7,     8,   387,     5,     0,     6,     9,    10,
       0,    11,    12,    13,    28,    14,    15,    16,    23,    17,
      18,    19,    20,    21,    22,    33,    24,    25,    26,    27,
      29,    30,    31,    32,     0,     0,     0,    34,    35,    36,
      38,    37,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    71,    70,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,   132,    91,    90,    92,    93,    94,    95,
      97,    96,    98,    99,   101,   102,   100,   103,   104,   105,
     106,   108,   109,   107,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   133,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,     0,     0,     0,
       0,     0,     0,   487,   488,     0,     0,     0,     0,     0,
      34,   284,   286,   287,   511,   289,   493,   288,    37,    39,
      43,    46,    55,    62,    64,    71,    70,   285,     0,    75,
      78,    85,    87,    97,   100,   115,   127,     0,   159,     0,
       0,     0,   283,     0,   157,   161,   164,   311,     0,   265,
     490,   161,     0,   312,   313,     0,     0,    49,    68,    74,
       0,   107,     0,     0,     0,     0,   465,   344,   469,   359,
     344,   351,   352,   350,   470,   344,   360,   344,   269,   346,
     343,     0,     0,   389,     0,   173,     0,     1,     0,     4,
     136,     0,   344,     0,     0,     0,     0,     0,     0,     0,
     481,     0,   486,   485,   269,   520,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    34,    85,    87,   290,   291,     0,   340,
     339,     0,     0,   505,   506,     0,   502,   503,     0,     0,
       0,   162,   160,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   489,   266,   492,     0,     0,     0,     0,
       0,     0,     0,   383,   384,   382,   381,   380,   366,   378,
       0,     0,     0,   344,     0,   466,     0,   435,   348,   347,
     434,     0,   353,   270,   361,   459,   495,     0,     0,   516,
     517,   161,   512,     0,     0,     0,   135,   393,   431,   456,
       0,     0,     0,   236,   238,   406,   240,     0,     0,   404,
     405,   418,   422,   416,     0,     0,     0,   414,     0,   496,
       0,   522,     0,   333,     0,   322,   332,     0,   330,     0,
       0,     0,     0,     0,     0,   171,     0,     0,     0,     0,
       0,   328,     0,     0,     0,   325,     0,     0,     0,   309,
       0,   310,     0,   511,     0,   504,   151,   175,   158,   164,
     163,   301,   302,   308,   307,   299,   298,   305,   509,   306,
     296,   297,   303,   304,   292,   293,   294,   295,   300,     0,
     267,   491,   314,     0,   510,   367,   368,     0,     0,     0,
       0,   374,   376,   365,   375,     0,   373,   377,   364,   363,
       0,   344,   349,   344,   345,   274,   271,   272,     0,     0,
     356,     0,     0,     0,   457,   460,     0,     0,   437,     0,
     174,   515,   514,     0,   256,     0,     0,     0,     0,     0,
       0,   237,   239,   420,   408,   241,     0,     0,     0,     0,
     477,   478,   476,   480,   479,     0,     0,   224,    37,    39,
      46,    55,    62,     0,    71,   228,    78,    84,   127,     0,
     223,   177,   178,   179,   182,     0,     0,   229,   498,   521,
     499,     0,     0,   316,     0,   165,   319,     0,   170,   321,
     320,   169,   317,   318,   166,     0,   167,     0,   329,     0,
     168,     0,     0,     0,   342,   341,   337,   508,   507,     0,
     245,   176,     0,     0,   315,     0,   371,   370,     0,   379,
       0,   354,   355,     0,     0,   358,   362,   357,   458,     0,
     461,   462,     0,     0,   269,     0,   518,     0,   265,   257,
     513,   396,   395,   397,     0,     0,     0,   455,   483,   407,
       0,   419,     0,   427,   417,   423,   424,   415,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   144,   331,   172,     0,     0,     0,     0,
       0,     0,   247,   253,     0,   268,     0,     0,     0,     0,
     474,   467,   276,   279,     0,   275,   273,     0,     0,     0,
     439,   440,   438,   436,   443,   444,     0,   519,   394,     0,
       0,   409,   399,   446,   448,   447,   453,   445,   451,   449,
     450,   452,   454,     0,   482,   242,   421,   428,     0,     0,
     475,   497,     0,     0,     0,     0,     0,     0,   233,   226,
       0,     0,   227,   185,   180,   184,   183,     0,     0,   192,
       0,   211,     0,   209,   186,   200,   210,   187,   201,   197,
     206,   196,   205,   199,   208,   198,   207,     0,     0,     0,
       0,   213,   214,   219,   220,   221,   222,     0,     0,     0,
       0,   324,   323,     0,   338,     0,     0,   248,     0,     0,
     251,   254,     0,   369,     0,   472,   471,   473,     0,     0,
     280,     0,   464,   463,   442,     0,    97,   262,   258,   260,
     398,     0,     0,   402,     0,     0,   390,   484,   430,   429,
     426,   427,   425,   235,   234,   232,   225,   231,   230,   181,
       0,     0,   243,     0,     0,   212,     0,   193,     0,     0,
       0,     0,     0,   149,   150,   148,   145,   146,   141,     0,
       0,     0,   153,     0,     0,     0,   256,   501,     0,   500,
       0,     0,   277,     0,     0,   441,     0,   263,   264,     0,
     401,     0,     0,   410,   411,   400,   194,   203,   204,   202,
     188,   190,     0,     0,     0,     0,   217,   215,     0,     0,
       0,     0,     0,     0,   143,   249,   246,     0,   252,   265,
     326,   372,   282,   281,   278,   259,   261,   403,     0,     0,
     244,   195,   189,   191,   218,   216,   147,     0,   152,   154,
       0,     0,   269,   413,   412,     0,     0,   250,   255,   156,
       0,   155,     0,     0,   327
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -802,  -802,  -802,   -13,    -4,  -802,   449,  -802,   299,  -802,
    -802,  -175,  -802,  -802,  -802,   127,    45,   398,  -202,  -802,
     -12,  -802,  -385,  -802,  -802,  -513,  -136,  -683,  -802,   -47,
    -581,  -570,  -117,  -802,  -802,  -802,  -802,  -802,  -802,  -134,
    -802,  -801,  -146,  -602,  -802,  -272,  -802,  -802,   122,  -802,
    -132,   118,  -802,  -278,   163,  -802,  -802,   289,    54,  -802,
    -220,  -802,  -802,  -802,  -802,  -802,   260,  -802,    55,   259,
    -802,  -802,  -802,  -802,  -802,    25,  -802,  -802,   -75,  -802,
    -506,  -802,  -802,  -168,  -802,  -802,  -802,   214,  -802,  -802,
    -802,    15,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
     133,  -802,  -802,  -802,  -802,  -802,  -802,    66,  -802,  -802,
    -802,   714,  -802,  -802,  -802,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,  -802,  -802,   423,  -802,  -802,  -802,  -802,  -802,
    -802,  -802,   -26,  -358,  -802,   429,  -802,   451,  -802,  -802,
     258,  -802,   255,  -802,  -802,  -802
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    23,    24,   166,   212,    25,    26,    27,    28,   770,
     836,   837,   579,   842,   884,    29,   213,   214,   312,   215,
     437,   580,   397,   541,   542,   543,   544,   545,   546,   515,
     390,   516,   824,   673,   778,   886,   846,   780,   781,   608,
     609,   798,   799,   333,   334,   362,   363,   486,   487,   789,
     790,   406,   217,   407,   408,   218,   301,   302,   576,    30,
     357,   250,   251,   491,    31,    32,   478,   479,   348,   349,
      33,    34,    35,    36,   507,   613,   614,   701,   702,   802,
     391,   806,   863,   864,    37,    38,   392,   393,   517,   519,
     625,   626,   718,   810,    39,    40,    41,    42,    43,   497,
     498,   712,    44,    45,   494,   495,   600,   601,    46,    47,
      48,   233,   681,    49,   525,    50,    51,   714,    52,    53,
      54,    55,    56,   219,   220,   221,    57,    58,    59,    60,
      61,    62,   222,   223,   306,   307,   224,   225,   504,   505,
     370,   371,   372,   260,   401,    63
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     167,   410,   399,   413,   414,   232,   697,   417,   418,   499,
     256,   422,   623,   797,  -494,   676,   258,   308,   255,   335,
     358,   691,   303,   304,   695,   359,   640,   360,   383,   384,
     840,   743,   331,   411,   785,   804,   262,   386,   547,   739,
     383,   488,   378,   513,   383,   383,   827,   383,   384,   462,
     385,   383,   581,   367,   303,   304,   386,   173,   313,   489,
     263,   264,   265,   738,   303,   304,   745,   748,   750,   752,
     754,   756,   717,   383,   383,   384,   822,   385,   502,   356,
      66,   908,   757,   386,   492,   383,   384,   383,   384,   168,
     380,   767,   314,   174,   386,   170,   386,   383,   384,   368,
     746,   805,   857,   171,   511,   512,   386,   313,   332,  -501,
     918,   463,   604,   253,   309,   259,  -113,   758,   786,   795,
      64,   381,   858,   169,   177,   175,   768,   309,   734,   735,
     736,   412,    65,   482,   176,   216,   490,   172,   493,   396,
     257,   314,   602,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   269,   741,
     885,   305,   368,   387,   267,   268,   255,   270,   271,   501,
     823,   272,   273,   274,   275,   475,   797,   740,   254,   475,
     475,   547,   387,   514,   828,   266,   475,   794,   514,   388,
     830,   261,   178,   305,   803,   553,   554,   313,   503,   797,
    -418,  -418,   644,   305,   327,   328,   329,   300,   475,   387,
     276,   311,   396,   811,   850,   341,   801,   311,   179,   389,
     387,   742,   387,   642,   643,   342,   347,   917,   797,   689,
     354,   314,   387,   654,   430,   603,   431,   645,   678,   867,
     869,   679,   733,   277,   646,   499,   336,   364,   680,   373,
     366,   374,   556,   554,   375,   559,   554,   377,   874,   560,
     554,   591,   879,   592,   562,   554,   642,   643,   655,   563,
     554,   568,   554,  -335,   725,  -334,   727,   619,   620,   730,
     621,   622,   313,   547,   547,   547,   278,   912,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   698,   699,   783,
     620,   279,   234,   477,   255,  -500,   440,   852,   853,   647,
     648,   649,   650,   651,   652,   280,   314,   855,   620,   281,
     653,   860,   861,   870,   620,   296,   282,   297,   298,   283,
     703,   704,   693,   284,   347,   347,   871,   872,   547,   285,
     656,   657,   658,   659,   660,   661,   476,  -336,   239,   877,
     620,   662,   705,   891,   620,   897,   286,   485,   902,   620,
     287,   232,   240,   496,   500,   903,   872,   311,   706,   707,
     905,   620,   241,   708,   242,   243,   369,   325,   326,   327,
     328,   329,   709,   710,   313,   575,   337,   907,   554,   923,
     554,   288,   540,   465,   466,   245,   409,   289,   246,   290,
     248,   291,   416,   292,   330,   419,   420,   557,   423,   424,
     249,   338,   339,   711,  -470,   350,   351,   355,   314,   356,
     361,   586,   313,   434,   849,   365,   300,   439,   574,   379,
     382,   441,   442,   443,   444,   445,   446,   447,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   394,
     395,   398,   400,   402,   415,   464,   314,   432,   425,   426,
     427,   460,   186,   428,   476,   469,   347,   467,   468,   470,
     480,   389,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   481,   547,   483,   547,   484,   599,
     506,   439,   508,   509,   510,   526,   548,   518,   549,   550,
    -511,   547,   612,   520,   616,   617,   618,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   298,   577,   547,   521,   584,   558,   585,   583,
     588,   578,   511,   590,   594,   540,   593,   595,   596,   597,
     598,   522,   523,   605,   607,   571,   572,   615,   628,   629,
     627,   630,   547,   547,   631,   690,   632,   641,   694,   633,
     634,   881,   635,   636,   637,   638,   639,   669,   663,   665,
     670,   672,   675,   715,   688,   696,   677,   331,   674,   687,
     503,   713,   716,   700,   720,   721,   524,   719,   726,   685,
     485,   731,   777,   779,   787,   782,   728,   737,   729,   496,
     744,   747,   749,   751,   753,   755,   769,   759,   760,   761,
     762,   763,   764,   765,   766,   732,   776,   784,   791,   792,
     813,   369,   575,   369,   807,   814,   722,   723,   724,   234,
     724,   815,   816,   724,   817,   643,   825,   540,   540,   540,
     919,   818,   922,   819,   235,   820,   236,   821,   829,   838,
     832,   843,   237,   389,   313,   839,   841,   844,   845,   851,
     854,   856,   859,   717,   862,   574,   873,   238,   878,   439,
     882,   880,   389,   887,   883,   239,   890,   894,   895,   900,
     788,   898,   909,   666,   599,   667,   899,   668,   314,   240,
     910,   911,   540,   913,   915,   612,   916,   920,   921,   241,
     924,   242,   243,   552,   313,   906,   671,   438,   376,   888,
     826,   875,   889,   896,   809,   775,   686,   664,   244,   573,
     831,   893,   245,   774,   800,   246,   247,   248,   587,   589,
     865,   914,   624,   252,   812,   435,   692,   249,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   793,   461,   848,     0,   564,   565,
     610,   606,     0,     0,     0,   835,     0,     0,     0,     0,
     448,     0,   313,   866,   868,     0,     0,     0,   847,     0,
       0,     0,   876,     0,     0,     0,     0,     0,   439,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   892,     0,   314,     0,   566,   567,
       0,     0,     0,     0,   389,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   901,     0,     0,     0,
       0,   904,     0,     0,     0,     0,   255,     0,     0,   540,
       0,   540,     0,     0,     0,     0,     0,     0,     0,   788,
       0,     0,     0,     0,     0,   540,     0,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,     0,     0,     0,     0,   835,   569,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   180,   181,   182,
     183,     0,   184,   185,   186,     0,   540,   540,   187,    68,
      69,    70,   188,   189,     0,    73,    74,    75,   190,    77,
      78,   191,    80,     0,    81,    82,    83,    84,    85,    86,
      87,   192,    89,    90,    91,    92,    93,    94,     0,   193,
      96,     0,     0,   194,    98,    99,     0,   100,   101,   102,
     195,   196,   105,   106,   197,   198,   107,   199,   109,   110,
     200,   112,     0,   113,   114,   115,   116,     0,   117,   201,
     119,   202,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   203,   131,   132,   133,     0,   134,
     135,   204,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   205,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   206,
     163,   164,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   207,   208,     0,     0,   209,     0,
       0,   210,     0,     0,   211,   293,   181,   182,   183,     0,
     403,   185,     0,     0,     0,     0,   187,    68,    69,    70,
     188,   189,     0,    73,    74,    75,    76,    77,    78,   191,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,   193,    96,     0,
       0,   194,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   197,   198,   107,   199,   109,   110,   200,   112,
       0,   113,   114,   115,   116,     0,   117,   294,   119,   295,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   203,   131,   132,   133,     0,   134,   135,   204,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,   158,   159,   160,   161,   206,   163,   164,
     165,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   207,     0,     0,     0,   209,     0,     0,   404,
     405,     0,   211,   293,   181,   182,   183,     0,   403,   185,
       0,     0,     0,     0,   187,    68,    69,    70,   188,   189,
       0,    73,    74,    75,    76,    77,    78,   191,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,   193,    96,     0,     0,   194,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     197,   198,   107,   199,   109,   110,   200,   112,     0,   113,
     114,   115,   116,     0,   117,   294,   119,   295,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     203,   131,   132,   133,     0,   134,   135,   204,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   206,   163,   164,   165,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     207,     0,     0,     0,   209,     0,     0,   210,   421,     0,
     211,   293,   181,   182,   183,     0,   184,   185,     0,     0,
       0,     0,   187,    68,    69,    70,   188,   189,     0,    73,
      74,    75,   190,    77,    78,   191,    80,     0,    81,    82,
      83,    84,    85,    86,    87,   192,    89,    90,    91,    92,
      93,    94,     0,   193,    96,     0,     0,   194,    98,    99,
       0,   100,   101,   102,   195,   196,   105,   106,   197,   198,
     107,   199,   109,   110,   200,   112,     0,   113,   114,   115,
     116,     0,   117,   201,   119,   202,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   203,   131,
     132,   133,     0,   134,   135,   204,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   205,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   206,   163,   164,   165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   207,   208,
       0,     0,   209,     0,     0,   210,     0,     0,   211,   180,
     181,   182,   183,     0,   184,   185,     0,     0,     0,     0,
     187,    68,    69,    70,   188,   189,     0,    73,    74,    75,
     190,    77,    78,   191,    80,     0,    81,    82,    83,    84,
      85,    86,    87,   192,    89,    90,    91,    92,    93,    94,
       0,   193,    96,     0,     0,   194,    98,    99,     0,   100,
     101,   102,   195,   196,   105,   106,   197,   198,   107,   199,
     109,   110,   200,   112,     0,   113,   114,   115,   116,     0,
     117,   201,   119,   202,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   203,   131,   132,   133,
       0,   134,   135,   204,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   205,   150,   151,
     152,   153,   154,     0,   155,   156,   157,   158,   159,   160,
     161,   206,   163,   164,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   207,   208,     0,     0,
     209,     0,     0,   210,     0,     0,   211,   293,   181,   182,
     183,     0,   184,   185,     0,     0,     0,     0,   187,    68,
      69,    70,   188,   189,     0,    73,    74,    75,    76,    77,
      78,   191,    80,     0,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,   193,
      96,     0,     0,   194,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   197,   198,   107,   199,   109,   110,
     200,   112,     0,   113,   114,   115,   116,     0,   117,   294,
     119,   295,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   203,   131,   132,   133,     0,   134,
     135,   204,   137,   138,   139,   551,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   206,
     163,   164,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   207,     0,     0,     0,   209,     0,
       0,   210,     0,     0,   211,   293,   181,   182,   183,     0,
     403,   185,     0,     0,     0,     0,   187,    68,    69,    70,
     188,   189,     0,    73,    74,    75,    76,    77,    78,   191,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,   193,    96,     0,
       0,   194,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   197,   198,   107,   199,   109,   110,   200,   112,
       0,   113,   114,   115,   116,     0,   117,   294,   119,   295,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   203,   131,   132,   133,     0,   134,   135,   204,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,   158,   159,   160,   161,   206,   163,   164,
     165,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   207,     0,     0,     0,   209,     0,     0,   210,
     405,     0,   211,   293,   181,   182,   183,     0,   184,   185,
       0,     0,     0,     0,   187,    68,    69,    70,   188,   189,
       0,    73,    74,    75,    76,    77,    78,   191,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,   193,    96,     0,     0,   194,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     197,   198,   107,   199,   109,   110,   200,   112,     0,   113,
     114,   115,   116,     0,   117,   294,   119,   295,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     203,   131,   132,   133,     0,   134,   135,   204,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   206,   163,   164,   165,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     207,     0,     0,     0,   209,     0,     0,   210,     0,     0,
     211,   293,   181,   182,   183,     0,   403,   185,     0,     0,
       0,     0,   187,    68,    69,    70,   188,   189,     0,    73,
      74,    75,    76,    77,    78,   191,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,   193,    96,     0,     0,   194,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   197,   198,
     107,   199,   109,   110,   200,   112,     0,   113,   114,   115,
     116,     0,   117,   294,   119,   295,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   203,   131,
     132,   133,     0,   134,   135,   204,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   206,   163,   164,   165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   207,     0,
       0,     0,   209,     0,     0,   210,     0,     0,   211,   293,
     181,   182,   183,     0,   433,   185,     0,     0,     0,     0,
     187,    68,    69,    70,   188,   189,     0,    73,    74,    75,
      76,    77,    78,   191,    80,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,   193,    96,     0,     0,   194,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   197,   198,   107,   199,
     109,   110,   200,   112,     0,   113,   114,   115,   116,     0,
     117,   294,   119,   295,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   203,   131,   132,   133,
       0,   134,   135,   204,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,   155,   156,   157,   158,   159,   160,
     161,   206,   163,   164,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   207,    67,   527,     0,
     209,     0,     0,   210,     0,     0,   211,     0,     0,    68,
      69,    70,   528,   529,     0,    73,    74,    75,    76,    77,
      78,   530,    80,     0,    81,    82,    83,    84,    85,    86,
      87,   531,    89,    90,    91,    92,    93,    94,     0,   532,
      96,   533,     0,    97,    98,    99,     0,   100,   101,   102,
     534,   104,   105,   106,   535,     0,   107,   108,   109,   110,
     536,   112,     0,   113,   114,   115,   116,     0,   537,   118,
     119,   120,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   130,   131,   132,   133,     0,   134,
     135,   136,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   538,
     163,   164,   165,     0,    67,   527,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    68,    69,    70,   528,
     529,   539,    73,    74,    75,    76,    77,    78,   530,    80,
       0,    81,    82,    83,    84,    85,    86,    87,   531,    89,
      90,    91,    92,    93,    94,     0,   532,    96,   533,     0,
      97,    98,    99,     0,   100,   101,   102,   534,   104,   105,
     106,   535,     0,   107,   108,   109,   110,   536,   112,     0,
     113,   114,   115,   116,     0,   117,   118,   119,   120,     0,
     121,   122,   123,     0,   124,     0,   125,   126,   127,   128,
     129,   130,   131,   132,   133,     0,   134,   135,   136,   137,
     138,   139,     0,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,   155,
     156,   157,   158,   159,   160,   161,   538,   163,   164,   165,
       0,    67,     0,   682,     0,     0,   683,     0,     0,     0,
       0,     0,     0,    68,    69,    70,    71,    72,   539,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,     0,     0,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,     0,    67,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      68,    69,    70,    71,    72,   684,    73,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
      95,    96,     0,     0,    97,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,     0,     0,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,     0,   117,
     118,   119,   120,     0,   121,   122,   123,     0,   124,     0,
     125,   126,   127,   128,   129,   130,   131,   132,   133,     0,
     134,   135,   136,   137,   138,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,     0,    67,     0,   383,     0,     0,
     471,     0,     0,     0,     0,     0,     0,    68,    69,    70,
      71,    72,   436,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
     472,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,     0,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,   473,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   474,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    67,     0,   383,     0,     0,   471,     0,     0,     0,
       0,     0,   475,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,     0,     0,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,    67,     0,   383,
       0,     0,     0,     0,     0,     0,     0,     0,   475,    68,
      69,    70,    71,    72,     0,    73,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,    95,
      96,     0,     0,    97,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,     0,     0,   107,   108,   109,   110,
     111,   112,     0,   113,   114,   115,   116,     0,   117,   118,
     119,   120,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   130,   131,   132,   133,     0,   134,
     135,   136,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    67,     0,   343,   344,     0,   345,     0,
       0,     0,     0,     0,   475,    68,    69,    70,    71,    72,
       0,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
       0,     0,   107,   108,   109,   110,   111,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
     122,   123,   346,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,    67,
     527,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,   530,    80,     0,    81,    82,    83,    84,
      85,    86,    87,   531,    89,    90,    91,    92,    93,    94,
       0,   532,    96,   533,     0,    97,    98,    99,     0,   100,
     101,   102,   534,   104,   105,   106,   535,     0,   107,   108,
     109,   110,   536,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   796,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,   155,   156,   157,   158,   159,   160,
     161,   538,   163,   164,   165,    67,   527,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,   530,
      80,     0,    81,    82,    83,    84,    85,    86,    87,   531,
      89,    90,    91,    92,    93,    94,     0,   532,    96,   533,
       0,    97,    98,    99,     0,   100,   101,   102,   534,   104,
     105,   106,   535,     0,   107,   108,   109,   110,   536,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,   158,   159,   160,   161,   538,   163,   164,
     165,    67,   527,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    68,    69,    70,   528,   529,     0,    73,
      74,    75,    76,    77,    78,   530,    80,     0,    81,    82,
      83,    84,    85,    86,    87,   531,    89,    90,    91,    92,
      93,    94,     0,   532,    96,   533,     0,    97,    98,    99,
       0,   100,   101,   102,   534,   104,   105,   106,   535,     0,
     107,   108,   109,   110,   536,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   538,   163,   164,   165,    67,     0,   833,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    68,
      69,    70,    71,    72,     0,    73,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,    95,
      96,     0,     0,    97,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   834,     0,   107,   108,   109,   110,
     111,   112,     0,   113,   114,   115,   116,     0,   117,   118,
     119,   120,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   130,   131,   132,   133,     0,   134,
     135,   136,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    67,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,     0,   299,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,    67,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,    68,    69,    70,
      71,    72,   310,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,     0,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    67,     0,     0,     0,     0,   352,     0,     0,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,     0,     0,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   353,   148,   149,
     150,   151,   152,   153,   154,    67,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   611,     0,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,    67,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,     0,     0,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,   808,
     117,   118,   119,   120,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,    67,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,    68,    69,    70,    71,    72,
       0,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
       0,     0,   107,   108,   109,   110,   111,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,    67,
       0,     0,     0,     0,     0,     0,   226,     0,     0,     0,
       0,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,   227,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
     228,   102,   103,   104,   105,   106,     0,     0,   229,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   230,     0,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   231,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,     0,     0,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,    67,     0,     0,     0,     0,
       0,   340,     0,     0,     0,     0,     0,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,     0,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,     0,     0,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,  -469,     0,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    67,     0,     0,     0,     0,     0,     0,   226,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,   227,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,   313,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,     0,     0,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   230,
     314,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   313,     0,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,     0,     0,     0,
       0,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   313,     0,   314,     0,
       0,   565,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     314,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   313,     0,     0,     0,     0,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   313,     0,     0,     0,     0,   567,
       0,     0,     0,     0,     0,     0,     0,     0,   314,     0,
       0,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,     0,     1,   314,     0,
       0,   773,     2,     0,     0,     3,     0,     0,     0,     0,
       4,     0,     0,     0,     0,     5,     0,     0,     6,     0,
       7,     8,     9,     0,     0,     0,    10,     0,     0,     0,
      11,     0,   313,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,    12,     0,     0,     0,     0,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,     0,    13,   314,     0,   429,   313,
       0,     0,     0,     0,    14,     0,    15,     0,     0,    16,
       0,    17,     0,    18,     0,     0,    19,     0,    20,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    21,   313,
       0,    22,     0,   314,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,     0,   314,     0,   313,   555,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   314,
     313,     0,     0,   561,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,     0,
       0,   313,     0,   570,   314,     0,     0,     0,   582,     0,
       0,     0,     0,     0,     0,     0,   313,     0,     0,     0,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   314,   313,     0,     0,   771,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   313,
     314,     0,     0,     0,     0,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     314,     0,     0,     0,   772,     0,     0,     0,     0,     0,
       0,     0,     0,   314,     0,     0,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,     0,     0,     0,     0,
       0,     0,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329
};

static const yytype_int16 yycheck[] =
{
       4,   279,   274,   281,   282,    18,   608,   285,   286,   367,
      22,   289,   518,   696,     0,   585,    47,    52,    22,   221,
     240,   602,    13,    14,   605,   245,   539,   247,     5,     6,
      52,     8,    73,    43,    36,    83,    40,    14,   396,     9,
       5,    14,   262,     8,     5,     5,     9,     5,     6,    82,
       8,     5,   437,   103,    13,    14,    14,    23,    44,    32,
      64,    65,    66,   644,    13,    14,   647,   648,   649,   650,
     651,   652,    20,     5,     5,     6,     8,     8,    29,    72,
      62,   882,    26,    14,    72,     5,     6,     5,     6,    53,
      15,    26,    78,    59,    14,    52,    14,     5,     6,   149,
       8,   149,    21,    53,     5,     6,    14,    44,   149,    68,
     911,   144,   497,   116,   149,   146,   109,    61,   120,   689,
     100,    46,    41,    87,    62,    91,    61,   149,   641,   642,
     643,   141,   112,   353,   100,    17,   109,    87,   126,   126,
       0,    78,   133,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   170,    82,
     843,   152,   149,   140,   168,   169,   170,   171,   172,   371,
     740,   175,   176,   177,   178,   140,   859,   147,   100,   140,
     140,   539,   140,   148,   147,   147,   140,   148,   148,   147,
     760,    67,    62,   152,   700,   148,   149,    44,   149,   882,
     148,   149,    26,   152,   141,   142,   143,   211,   140,   140,
      52,   215,   126,   719,   784,   228,   147,   221,    88,   266,
     140,   144,   140,   129,   130,   229,   230,   910,   911,   147,
     234,    78,   140,    26,   149,   149,   151,    61,    92,   820,
     821,    95,   148,   147,    68,   603,    68,   251,   102,   101,
     254,   103,   148,   149,   106,   148,   149,   261,   828,   148,
     149,   481,   832,   483,   148,   149,   129,   130,    61,   148,
     149,   148,   149,   147,   632,   147,   634,   148,   149,   637,
     148,   149,    44,   641,   642,   643,   147,   889,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   148,   149,   148,
     149,   147,    16,   350,   308,    68,   310,   148,   149,   133,
     134,   135,   136,   137,   138,   147,    78,   148,   149,   147,
     144,   148,   149,   148,   149,   207,   147,   209,   210,   147,
      27,    28,   604,   147,   338,   339,   148,   149,   696,   147,
     133,   134,   135,   136,   137,   138,   350,   147,    62,   148,
     149,   144,    49,   148,   149,   861,   147,   361,   148,   149,
     147,   374,    76,   367,   368,   148,   149,   371,    65,    66,
     148,   149,    86,    70,    88,    89,   258,   139,   140,   141,
     142,   143,    79,    80,    44,   432,   133,   148,   149,   148,
     149,   147,   396,   338,   339,   109,   278,   147,   112,   147,
     114,   147,   284,   147,   147,   287,   288,   411,   290,   291,
     124,   133,   103,   110,   116,   133,   116,   103,    78,    72,
      83,   468,    44,   305,   782,   123,   430,   309,   432,    93,
     115,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,    98,
     119,   119,    52,   107,   148,   337,    78,   133,   148,   147,
     147,     5,    10,   147,   468,    54,   470,   133,   133,   140,
      69,   518,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   109,   843,   109,   845,     8,   493,
     147,   373,    34,    34,   100,     8,     8,   149,     8,     8,
     133,   859,   506,    27,   508,   509,   510,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   404,   153,   882,    49,    82,   148,   147,   149,
       9,   153,     5,    71,   149,   539,   133,   104,     5,   104,
       8,    65,    66,   133,    84,   427,   428,   122,   107,   107,
     148,   147,   910,   911,   147,   602,   147,   130,   605,   147,
     147,   839,   147,   147,   147,   147,   147,   133,   148,   148,
     101,    55,     5,   620,   129,    29,   133,    73,   582,   133,
     149,   127,     8,   147,     8,     8,   110,   149,   141,   593,
     594,     8,     5,   128,    92,    61,   148,   644,   148,   603,
     647,   648,   649,   650,   651,   652,   149,   654,   147,   656,
     657,   658,   659,   660,   661,   148,   148,   147,   147,     8,
     148,   503,   669,   505,   118,   148,   630,   631,   632,    16,
     634,   148,   148,   637,   148,   130,    82,   641,   642,   643,
     912,   148,   920,   148,    31,   130,    33,   130,   130,   148,
     147,    29,    39,   700,    44,   147,    84,    55,    58,   133,
       8,   147,   149,    20,     3,   669,   130,    54,   130,   551,
      29,   149,   719,    84,    73,    62,   148,   148,   148,     8,
     684,   133,     5,   565,   688,   567,   149,   569,    78,    76,
     149,    29,   696,     8,   149,   699,   149,   147,     5,    86,
     148,    88,    89,   404,    44,   880,   579,   309,   259,   845,
     757,   828,   846,   859,   718,   670,   594,   554,   105,   430,
     767,   853,   109,   669,   699,   112,   113,   114,   468,   470,
     805,   899,   518,    19,   719,   306,   603,   124,    78,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   688,   332,   782,    -1,   148,   149,
     505,   503,    -1,    -1,    -1,   769,    -1,    -1,    -1,    -1,
     319,    -1,    44,   820,   821,    -1,    -1,    -1,   782,    -1,
      -1,    -1,   829,    -1,    -1,    -1,    -1,    -1,   670,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   851,    -1,    78,    -1,   148,   149,
      -1,    -1,    -1,    -1,   861,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   873,    -1,    -1,    -1,
      -1,   878,    -1,    -1,    -1,    -1,   840,    -1,    -1,   843,
      -1,   845,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   853,
      -1,    -1,    -1,    -1,    -1,   859,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,    -1,    -1,    -1,    -1,   880,   149,   882,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    10,    -1,   910,   911,    14,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    -1,    -1,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    74,    75,
      76,    77,    -1,    79,    80,    81,    -1,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,   141,    -1,    -1,   144,    -1,
      -1,   147,    -1,    -1,   150,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    -1,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,
     148,    -1,   150,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    -1,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,   148,    -1,
     150,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,   141,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,   150,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,   141,    -1,    -1,
     144,    -1,    -1,   147,    -1,    -1,   150,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    -1,    -1,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    74,    75,
      76,    77,    -1,    79,    80,    81,    -1,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,   144,    -1,
      -1,   147,    -1,    -1,   150,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    -1,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,    -1,    -1,    -1,   144,    -1,    -1,   147,
     148,    -1,   150,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    -1,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,    -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
     150,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,
      -1,    -1,   144,    -1,    -1,   147,    -1,    -1,   150,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   140,     3,     4,    -1,
     144,    -1,    -1,   147,    -1,    -1,   150,    -1,    -1,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    47,    -1,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    74,    75,
      76,    77,    -1,    79,    80,    81,    -1,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    -1,     3,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,
      19,   147,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    47,    -1,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    -1,    74,    75,    76,    77,    -1,
      79,    80,    81,    -1,    83,    -1,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    -1,    95,    96,    97,    98,
      99,   100,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,     3,    -1,     5,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,   147,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,     3,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    18,    19,   147,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    -1,    -1,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    -1,    74,
      75,    76,    77,    -1,    79,    80,    81,    -1,    83,    -1,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    99,   100,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,    -1,     3,    -1,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,   147,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      48,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    82,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,     3,    -1,     5,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    -1,   140,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,     3,    -1,     5,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    -1,    -1,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    -1,    -1,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    74,    75,
      76,    77,    -1,    79,    80,    81,    -1,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,     3,    -1,     5,     6,    -1,     8,    -1,
      -1,    -1,    -1,    -1,   140,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    82,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,     3,
       4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    47,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    60,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,     3,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    47,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    -1,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    47,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,     3,    -1,     5,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    -1,    -1,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    74,    75,
      76,    77,    -1,    79,    80,    81,    -1,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,     3,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,     3,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    -1,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,     3,    -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,     3,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    -1,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,     3,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,     3,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    -1,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    -1,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,     3,    -1,    -1,    -1,    -1,
      -1,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    -1,    -1,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,     3,    -1,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    44,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      78,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    44,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,    -1,    -1,    -1,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,    44,    -1,    78,    -1,
      -1,   149,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    44,    -1,    -1,    -1,    -1,   149,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    78,    -1,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,    -1,    17,    78,    -1,
      -1,   149,    22,    -1,    -1,    25,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    -1,    35,    -1,    -1,    38,    -1,
      40,    41,    42,    -1,    -1,    -1,    46,    -1,    -1,    -1,
      50,    -1,    44,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    64,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    -1,    85,    78,    -1,   148,    44,
      -1,    -1,    -1,    -1,    94,    -1,    96,    -1,    -1,    99,
      -1,   101,    -1,   103,    -1,    -1,   106,    -1,   108,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   118,    44,
      -1,   121,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,    -1,    78,    -1,    44,   148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,    78,
      44,    -1,    -1,   148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,    -1,
      -1,    44,    -1,   148,    78,    -1,    -1,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,    78,    44,    -1,    -1,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      78,    -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      78,    -1,    -1,    -1,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    -1,    -1,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    17,    22,    25,    30,    35,    38,    40,    41,    42,
      46,    50,    64,    85,    94,    96,    99,   101,   103,   106,
     108,   118,   121,   155,   156,   159,   160,   161,   162,   169,
     213,   218,   219,   224,   225,   226,   227,   238,   239,   248,
     249,   250,   251,   252,   256,   257,   262,   263,   264,   267,
     269,   270,   272,   273,   274,   275,   276,   280,   281,   282,
     283,   284,   285,   299,   100,   112,    62,     3,    15,    16,
      17,    18,    19,    21,    22,    23,    24,    25,    26,    27,
      28,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    45,    46,    49,    50,    51,
      53,    54,    55,    56,    57,    58,    59,    62,    63,    64,
      65,    66,    67,    69,    70,    71,    72,    74,    75,    76,
      77,    79,    80,    81,    83,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    95,    96,    97,    98,    99,   100,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   157,   158,    53,    87,
      52,    53,    87,    23,    59,    91,   100,    62,    62,    88,
       3,     4,     5,     6,     8,     9,    10,    14,    18,    19,
      24,    27,    37,    45,    49,    56,    57,    60,    61,    63,
      66,    75,    77,    90,    97,   111,   125,   140,   141,   144,
     147,   150,   158,   170,   171,   173,   205,   206,   209,   277,
     278,   279,   286,   287,   290,   291,    10,    31,    54,    62,
      81,   105,   157,   265,    16,    31,    33,    39,    54,    62,
      76,    86,    88,    89,   105,   109,   112,   113,   114,   124,
     215,   216,   265,   116,   100,   158,   174,     0,    47,   146,
     297,    67,   158,   158,   158,   158,   147,   158,   158,   174,
     158,   158,   158,   158,   158,   158,    52,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,     3,    75,    77,   205,   205,   205,    61,
     158,   210,   211,    13,    14,   152,   288,   289,    52,   149,
      20,   158,   172,    44,    78,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     147,    73,   149,   197,   198,   172,    68,   133,   133,   103,
       9,   157,   158,     5,     6,     8,    82,   158,   222,   223,
     133,   116,     8,   109,   158,   103,    72,   214,   214,   214,
     214,    83,   199,   200,   158,   123,   158,   103,   149,   205,
     294,   295,   296,   101,   103,   106,   160,   158,   214,    93,
      15,    46,   115,     5,     6,     8,    14,   140,   147,   183,
     184,   234,   240,   241,    98,   119,   126,   176,   119,   199,
      52,   298,   107,     8,   147,   148,   205,   207,   208,   205,
     207,    43,   141,   207,   207,   148,   205,   207,   207,   205,
     205,   148,   207,   205,   205,   148,   147,   147,   147,   148,
     149,   151,   133,     8,   205,   289,   147,   174,   171,   205,
     158,   205,   205,   205,   205,   205,   205,   205,   291,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
       5,   278,    82,   144,   205,   222,   222,   133,   133,    54,
     140,     8,    48,    82,   117,   140,   158,   183,   220,   221,
      69,   109,   214,   109,     8,   158,   201,   202,    14,    32,
     109,   217,    72,   126,   258,   259,   158,   253,   254,   287,
     158,   172,    29,   149,   292,   293,   147,   228,    34,    34,
     100,     5,     6,     8,   148,   183,   185,   242,   149,   243,
      27,    49,    65,    66,   110,   268,     8,     4,    18,    19,
      27,    37,    45,    47,    56,    60,    66,    74,   125,   147,
     158,   177,   178,   179,   180,   181,   182,   287,     8,     8,
       8,   101,   162,   148,   149,   148,   148,   158,   148,   148,
     148,   148,   148,   148,   148,   149,   148,   149,   148,   149,
     148,   205,   205,   211,   158,   183,   212,   153,   153,   166,
     175,   176,    51,   149,    82,   147,   183,   220,     9,   223,
      71,   214,   214,   133,   149,   104,     5,   104,     8,   158,
     260,   261,   133,   149,   176,   133,   294,    84,   193,   194,
     296,    60,   158,   229,   230,   122,   158,   158,   158,   148,
     149,   148,   149,   234,   241,   244,   245,   148,   107,   107,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     179,   130,   129,   130,    26,    61,    68,   133,   134,   135,
     136,   137,   138,   144,    26,    61,   133,   134,   135,   136,
     137,   138,   144,   148,   208,   148,   205,   205,   205,   133,
     101,   169,    55,   187,   158,     5,   185,   133,    92,    95,
     102,   266,     5,     8,   147,   158,   202,   133,   129,   147,
     183,   184,   254,   199,   183,   184,    29,   197,   148,   149,
     147,   231,   232,    27,    28,    49,    65,    66,    70,    79,
      80,   110,   255,   127,   271,   183,     8,    20,   246,   149,
       8,     8,   158,   158,   158,   287,   141,   287,   148,   148,
     287,     8,   148,   148,   179,   179,   179,   183,   184,     9,
     147,    82,   144,     8,   183,   184,     8,   183,   184,   183,
     184,   183,   184,   183,   184,   183,   184,    26,    61,   183,
     147,   183,   183,   183,   183,   183,   183,    26,    61,   149,
     163,   148,   148,   149,   212,   170,   148,     5,   188,   128,
     191,   192,    61,   148,   147,    36,   120,    92,   158,   203,
     204,   147,     8,   261,   148,   185,    90,   181,   195,   196,
     229,   147,   233,   234,    83,   149,   235,   118,    73,   158,
     247,   234,   245,   148,   148,   148,   148,   148,   148,   148,
     130,   130,     8,   185,   186,    82,   183,     9,   147,   130,
     185,   183,   147,     5,    60,   158,   164,   165,   148,   147,
      52,    84,   167,    29,    55,    58,   190,   158,   286,   287,
     185,   133,   148,   149,     8,   148,   147,    21,    41,   149,
     148,   149,     3,   236,   237,   232,   183,   184,   183,   184,
     148,   148,   149,   130,   185,   186,   183,   148,   130,   185,
     149,   207,    29,    73,   168,   181,   189,    84,   180,   193,
     148,   148,   183,   204,   148,   148,   196,   234,   133,   149,
       8,   183,   148,   148,   183,   148,   165,   148,   195,     5,
     149,    29,   197,     8,   237,   149,   149,   181,   195,   199,
     147,     5,   207,   148,   148
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   154,   155,   155,   155,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   157,   157,   157,   157,   157,   157,
     157,   158,   158,   158,   159,   159,   159,   160,   160,   160,
     161,   161,   162,   162,   163,   163,   164,   164,   165,   165,
     165,   166,   167,   168,   168,   168,   169,   170,   170,   171,
     171,   172,   172,   172,   173,   173,   173,   173,   173,   173,
     173,   173,   173,   174,   174,   175,   175,   176,   177,   177,
     177,   178,   179,   179,   179,   179,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   182,   182,   183,   183,   184,   184,
     184,   185,   185,   186,   186,   187,   187,   188,   188,   189,
     189,   190,   190,   191,   191,   192,   193,   193,   194,   194,
     195,   195,   196,   196,   196,   197,   197,   198,   198,   199,
     199,   200,   201,   201,   202,   202,   202,   202,   202,   202,
     203,   203,   204,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     207,   207,   208,   208,   209,   209,   209,   210,   210,   211,
     211,   212,   212,   213,   214,   214,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   216,
     216,   217,   217,   218,   218,   218,   218,   218,   218,   219,
     219,   219,   219,   220,   220,   221,   221,   221,   222,   222,
     223,   223,   223,   223,   223,   224,   224,   224,   225,   225,
     226,   227,   227,   228,   228,   229,   229,   230,   230,   231,
     231,   232,   233,   233,   234,   234,   234,   234,   234,   235,
     235,   236,   236,   237,   238,   239,   240,   240,   241,   241,
     242,   242,   243,   243,   244,   244,   245,   246,   246,   247,
     247,   248,   249,   249,   250,   251,   252,   253,   253,   254,
     254,   254,   254,   254,   254,   255,   255,   255,   255,   255,
     255,   255,   255,   255,   256,   256,   256,   257,   257,   258,
     258,   259,   260,   260,   261,   262,   263,   264,   265,   265,
     265,   266,   266,   266,   266,   267,   268,   268,   268,   268,
     268,   269,   270,   271,   271,   272,   273,   274,   275,   276,
     277,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   286,   287,   288,   288,   289,   289,   289,   289,   290,
     290,   291,   292,   292,   293,   294,   295,   296,   296,   297,
     298,   298,   299
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     1,
       1,     8,     1,     9,     0,     2,     1,     3,     1,     1,
       1,     0,     3,     0,     2,     4,    11,     1,     3,     1,
       2,     0,     1,     2,     1,     4,     4,     4,     4,     4,
       4,     3,     5,     1,     3,     0,     1,     2,     1,     1,
       3,     4,     1,     3,     3,     3,     3,     3,     5,     6,
       5,     6,     3,     4,     5,     6,     3,     3,     3,     3,
       3,     3,     5,     5,     5,     3,     3,     3,     3,     3,
       3,     3,     4,     3,     3,     5,     6,     5,     6,     3,
       3,     3,     3,     1,     1,     4,     3,     3,     1,     1,
       4,     4,     4,     3,     4,     4,     1,     2,     1,     2,
       1,     1,     3,     1,     3,     0,     4,     0,     1,     1,
       3,     0,     2,     0,     1,     5,     0,     1,     3,     5,
       1,     3,     1,     2,     2,     0,     1,     2,     4,     0,
       1,     2,     1,     3,     1,     3,     3,     5,     6,     3,
       1,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     3,     4,     4,     4,     4,     4,
       4,     4,     3,     6,     6,     3,     8,    14,     3,     4,
       1,     3,     1,     1,     1,     1,     1,     3,     5,     1,
       1,     1,     1,     2,     0,     2,     1,     2,     2,     3,
       1,     1,     1,     2,     4,     4,     3,     4,     4,     1,
       1,     0,     2,     4,     4,     4,     3,     4,     4,     7,
       5,     5,     9,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       7,     1,     1,     0,     3,     1,     1,     1,     3,     1,
       3,     3,     1,     3,     1,     1,     1,     3,     2,     0,
       2,     1,     3,     3,     4,     6,     1,     3,     1,     3,
       1,     3,     0,     2,     1,     3,     3,     0,     1,     1,
       1,     3,     1,     1,     3,     3,     6,     1,     3,     3,
       3,     5,     4,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     7,     6,     4,     4,     5,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     1,
       1,     3,     7,     0,     2,     3,     3,     2,     2,     3,
       1,     3,     2,     1,     2,     3,     4,     7,     5,     5,
       1,     1,     2,     1,     2,     1,     1,     3,     3,     3,
       3,     1,     0,     2,     1,     2,     1,     1,     3,     5,
       0,     2,     4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (pParser, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, pParser); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, SqlParser_c * pParser)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (pParser);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, SqlParser_c * pParser)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, pParser);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, SqlParser_c * pParser)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , pParser);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, pParser); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, SqlParser_c * pParser)
{
  YYUSE (yyvaluep);
  YYUSE (pParser);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (SqlParser_c * pParser)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, pParser);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

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

    { pParser->PushQuery(); }

    break;

  case 134:

    { pParser->PushQuery(); }

    break;

  case 135:

    { pParser->PushQuery(); }

    break;

  case 136:

    { pParser->PushQuery(); }

    break;

  case 141:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, (yyvsp[-6]) );
		}

    break;

  case 143:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		}

    break;

  case 146:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), (yyvsp[0]) );
		}

    break;

  case 147:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), (yyvsp[0]) );
		}

    break;

  case 151:

    {
		CSphVector<CSphQueryItem> & dItems = pParser->m_pQuery->m_dItems;
		if ( dItems.GetLength()!=1 || dItems[0].m_sExpr!="*" )
		{
			yyerror ( pParser, "outer select list must be a single star" );
			YYERROR;
		}
		dItems.Reset();
		pParser->ResetSelect();
	}

    break;

  case 152:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, (yyvsp[0]) );
			pParser->m_pQuery->m_bHasOuter = true;
		}

    break;

  case 154:

    {
			pParser->m_pQuery->m_iOuterLimit = (yyvsp[0]).m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		}

    break;

  case 155:

    {
			pParser->m_pQuery->m_iOuterOffset = (yyvsp[-2]).m_iValue;
			pParser->m_pQuery->m_iOuterLimit = (yyvsp[0]).m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		}

    break;

  case 156:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, (yyvsp[-7]) );
		}

    break;

  case 159:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 162:

    { pParser->AliasLastItem ( &(yyvsp[0]) ); }

    break;

  case 163:

    { pParser->AliasLastItem ( &(yyvsp[0]) ); }

    break;

  case 164:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 165:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_AVG, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 166:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_MAX, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 167:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_MIN, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 168:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_SUM, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 169:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_CAT, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 170:

    { if ( !pParser->AddItem ( "count(*)", &(yyvsp[-3]), &(yyvsp[0]) ) ) YYERROR; }

    break;

  case 171:

    { if ( !pParser->AddItem ( "groupby()", &(yyvsp[-2]), &(yyvsp[0]) ) ) YYERROR; }

    break;

  case 172:

    { if ( !pParser->AddDistinct ( &(yyvsp[-1]), &(yyvsp[-4]), &(yyvsp[0]) ) ) YYERROR; }

    break;

  case 174:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 181:

    {
			if ( !pParser->SetMatch((yyvsp[-1])) )
				YYERROR;
		}

    break;

  case 182:

    { pParser->SetOp ( (yyval) ); }

    break;

  case 183:

    { pParser->FilterAnd ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 184:

    { pParser->FilterOr ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 185:

    { pParser->FilterGroup ( (yyval), (yyvsp[-1]) ); }

    break;

  case 186:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 187:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
			pFilter->m_bExclude = true;
		}

    break;

  case 188:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-4]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		}

    break;

  case 189:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-5]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_bExclude = true;
		}

    break;

  case 190:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-4]), (yyvsp[-1]), false ) )
				YYERROR;
		}

    break;

  case 191:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-5]), (yyvsp[-1]), true ) )
				YYERROR;
		}

    break;

  case 192:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 193:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-3]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 194:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 195:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-5]), (yyvsp[-3]).m_iValue, (yyvsp[-1]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 196:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 197:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 198:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 199:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 200:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 201:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true, true ) )
				YYERROR;
		}

    break;

  case 202:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 203:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 204:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 205:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, false ) )
				YYERROR;
		}

    break;

  case 206:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, false ) )
				YYERROR;
		}

    break;

  case 207:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, true ) )
				YYERROR;
		}

    break;

  case 208:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 209:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 210:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 211:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-2]), true ) )
				YYERROR;
		}

    break;

  case 212:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-3]), false ) )
				YYERROR;
		}

    break;

  case 213:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-2]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-2]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 214:

    {
			// tricky bit
			// any(tags!=val) is not equivalent to not(any(tags==val))
			// any(tags!=val) is instead equivalent to not(all(tags)==val)
			// thus, along with setting the exclude flag on, we also need to invert the function
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-2]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-2]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 215:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-4]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-4]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 216:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 217:

    {
			AddMvaRange ( pParser, (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue );
		}

    break;

  case 218:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_RANGE );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_iMinValue = (yyvsp[-2]).m_iValue;
			f->m_iMaxValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 219:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue-1 );
		}

    break;

  case 220:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue+1, INT64_MAX );
		}

    break;

  case 221:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue );
		}

    break;

  case 222:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue, INT64_MAX );
		}

    break;

  case 224:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		}

    break;

  case 225:

    {
			(yyval).m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 226:

    {
			(yyval).m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 227:

    {
			(yyval).m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 228:

    {
			(yyval).m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 230:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 231:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 232:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 234:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ANY; }

    break;

  case 235:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ALL; }

    break;

  case 236:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 237:

    {
			(yyval).m_iType = TOK_CONST_INT;
			if ( (uint64_t)(yyvsp[0]).m_iValue > (uint64_t)LLONG_MAX )
				(yyval).m_iValue = LLONG_MIN;
			else
				(yyval).m_iValue = -(yyvsp[0]).m_iValue;
		}

    break;

  case 238:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 239:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = -(yyvsp[0]).m_fValue; }

    break;

  case 240:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 241:

    {
			assert ( !(yyval).m_pValues.Ptr() );
			(yyval).m_pValues = new RefcountedVector_c<SphAttr_t> ();
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue ); 
		}

    break;

  case 242:

    {
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 243:

    {
			assert ( !(yyval).m_pValues.Ptr() );
			(yyval).m_pValues = new RefcountedVector_c<SphAttr_t> ();
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 244:

    {
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 248:

    {
			pParser->SetGroupbyLimit ( (yyvsp[0]).m_iValue );
		}

    break;

  case 249:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 250:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 252:

    {
			pParser->AddHaving();
		}

    break;

  case 255:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, (yyvsp[0]) );
		}

    break;

  case 258:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, (yyvsp[0]) );
		}

    break;

  case 259:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		}

    break;

  case 261:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 263:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 264:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 267:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 268:

    {
			pParser->m_pQuery->m_iOffset = (yyvsp[-2]).m_iValue;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 274:

    {
			if ( !pParser->AddOption ( (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 275:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 276:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 277:

    {
			if ( !pParser->AddOption ( (yyvsp[-4]), pParser->GetNamedVec ( (yyvsp[-1]).m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( (yyvsp[-1]).m_iValue );
		}

    break;

  case 278:

    {
			if ( !pParser->AddOption ( (yyvsp[-5]), (yyvsp[-3]), (yyvsp[-1]) ) )
				YYERROR;
		}

    break;

  case 279:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 280:

    {
			(yyval).m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 281:

    {
			pParser->AddConst( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 282:

    {
			(yyval) = (yyvsp[-2]);
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 284:

    { if ( !pParser->SetOldSyntax() ) YYERROR; }

    break;

  case 285:

    { if ( !pParser->SetNewSyntax() ) YYERROR; }

    break;

  case 290:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 291:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 292:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 293:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 294:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 295:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 296:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 297:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 298:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 299:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 300:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 301:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 302:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 303:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 304:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 305:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 306:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 307:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 308:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 309:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 310:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 314:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 315:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 316:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 317:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 318:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 319:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 320:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 321:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 322:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 323:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 324:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 325:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 326:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-7]), (yyvsp[0]) ); }

    break;

  case 327:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-13]), (yyvsp[0]) ); }

    break;

  case 328:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 329:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 337:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 338:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-4]), (yyvsp[0]) ); }

    break;

  case 345:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) ); }

    break;

  case 346:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; }

    break;

  case 347:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; }

    break;

  case 348:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; }

    break;

  case 349:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; }

    break;

  case 350:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; }

    break;

  case 351:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; }

    break;

  case 352:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; }

    break;

  case 353:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; }

    break;

  case 354:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 355:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 356:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 357:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 358:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
			pParser->m_pStmt->m_iIntParam = int((yyvsp[-1]).m_fValue*10);
		}

    break;

  case 362:

    {
			pParser->m_pStmt->m_iIntParam = (yyvsp[0]).m_iValue;
		}

    break;

  case 363:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 364:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) );
		}

    break;

  case 365:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		}

    break;

  case 366:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 367:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 368:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 369:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
		}

    break;

  case 370:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) ).Unquote();
		}

    break;

  case 371:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 372:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-6]) );
		}

    break;

  case 375:

    { (yyval).m_iValue = 1; }

    break;

  case 376:

    { (yyval).m_iValue = 0; }

    break;

  case 377:

    {
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
			if ( (yyval).m_iValue!=0 && (yyval).m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		}

    break;

  case 385:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; }

    break;

  case 386:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; }

    break;

  case 387:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; }

    break;

  case 390:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-4]) );
		}

    break;

  case 391:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; }

    break;

  case 392:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; }

    break;

  case 397:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 398:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 401:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } }

    break;

  case 402:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 403:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 404:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 405:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 406:

    { (yyval).m_iType = TOK_QUOTED_STRING; (yyval).m_iStart = (yyvsp[0]).m_iStart; (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 407:

    { (yyval).m_iType = TOK_CONST_MVA; (yyval).m_pValues = (yyvsp[-1]).m_pValues; }

    break;

  case 408:

    { (yyval).m_iType = TOK_CONST_MVA; }

    break;

  case 413:

    { if ( !pParser->AddInsertOption ( (yyvsp[-2]), (yyvsp[0]) ) ) YYERROR; }

    break;

  case 414:

    {
		if ( !pParser->DeleteStatement ( &(yyvsp[-1]) ) )
			YYERROR;
	}

    break;

  case 415:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, (yyvsp[-4]) );
		}

    break;

  case 416:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 417:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 419:

    {
			(yyval).m_iType = TOK_CONST_STRINGS;
		}

    break;

  case 420:

    {
			// FIXME? for now, one such array per CALL statement, tops
			if ( pParser->m_pStmt->m_dCallStrings.GetLength() )
			{
				yyerror ( pParser, "unexpected constant string list" );
				YYERROR;
			}
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), (yyvsp[0]) );
		}

    break;

  case 421:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), (yyvsp[0]) );
		}

    break;

  case 424:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		}

    break;

  case 426:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), (yyvsp[0]) );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, (yyvsp[-2]) );
		}

    break;

  case 431:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 434:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; }

    break;

  case 435:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; }

    break;

  case 436:

    {
			if ( !pParser->UpdateStatement ( &(yyvsp[-4]) ) )
				YYERROR;
		}

    break;

  case 439:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)(yyvsp[0]).m_iValue );
			DWORD uHi = (DWORD)( (yyvsp[0]).m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_INTEGER );
			}
		}

    break;

  case 440:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( (yyvsp[0]).m_fValue ) );
			pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_FLOAT );
		}

    break;

  case 441:

    {
			pParser->UpdateMVAAttr ( (yyvsp[-4]), (yyvsp[-1]) );
		}

    break;

  case 442:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( (yyvsp[-3]), tNoValues );
		}

    break;

  case 443:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)(yyvsp[0]).m_iValue );
			DWORD uHi = (DWORD)( (yyvsp[0]).m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_INTEGER );
			}
		}

    break;

  case 444:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( (yyvsp[0]).m_fValue ) );
			pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_FLOAT );
		}

    break;

  case 445:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 446:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 447:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 448:

    { (yyval).m_iValue = SPH_ATTR_BOOL; }

    break;

  case 449:

    { (yyval).m_iValue = SPH_ATTR_UINT32SET; }

    break;

  case 450:

    { (yyval).m_iValue = SPH_ATTR_INT64SET; }

    break;

  case 451:

    { (yyval).m_iValue = SPH_ATTR_JSON; }

    break;

  case 452:

    { (yyval).m_iValue = SPH_ATTR_STRING; }

    break;

  case 453:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 454:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-4]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[-1]) );
			tStmt.m_eAlterColType = (ESphAttr)(yyvsp[0]).m_iValue;
		}

    break;

  case 455:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-3]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[0]) );
		}

    break;

  case 456:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 457:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		}

    break;

  case 458:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 465:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		}

    break;

  case 466:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		}

    break;

  case 467:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		}

    break;

  case 475:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, (yyvsp[0]) );
			tStmt.m_eUdfType = (ESphAttr) (yyvsp[-2]).m_iValue;
		}

    break;

  case 476:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 477:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 478:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 479:

    { (yyval).m_iValue = SPH_ATTR_STRINGPTR; }

    break;

  case 480:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 481:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[0]) );
		}

    break;

  case 482:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-4]) );
			pParser->ToString ( tStmt.m_sStringParam, (yyvsp[-1]) );
		}

    break;

  case 484:

    {
			pParser->m_pStmt->m_iIntParam = 1;
		}

    break;

  case 485:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 486:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 487:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		}

    break;

  case 488:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_HOSTNAMES;
		}

    break;

  case 489:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[-1]) );
		}

    break;

  case 493:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 494:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[0]) );
		}

    break;

  case 495:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 496:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 497:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 498:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 499:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 502:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 504:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 505:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 506:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 507:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 508:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 509:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 510:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 514:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		}

    break;

  case 516:

    {
			pParser->AddItem ( &(yyvsp[0]) );
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 519:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
		}

    break;

  case 521:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 522:

    {
			pParser->m_pStmt->m_eStmt = STMT_RELOAD_INDEX;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]));
		}

    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (pParser, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (pParser, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, pParser);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, pParser);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (pParser, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, pParser);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, pParser);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}



#if USE_WINDOWS
#pragma warning(pop)
#endif
