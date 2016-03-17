/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

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
    TOK_WITHIN = 382,
    TOK_OR = 383,
    TOK_AND = 384,
    TOK_NE = 385,
    TOK_LTE = 386,
    TOK_GTE = 387,
    TOK_NOT = 388,
    TOK_NEG = 389
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
#define YYFINAL  256
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5204

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  153
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  144
/* YYNRULES -- Number of rules.  */
#define YYNRULES  516
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  915

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   389

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   142,   131,     2,
     146,   147,   140,   138,   148,   139,     2,   141,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   145,
     134,   132,   135,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   151,     2,   152,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   149,   130,   150,     2,     2,     2,     2,
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
     125,   126,   127,   128,   129,   133,   136,   137,   143,   144
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   188,   188,   189,   190,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   237,   238,   238,   238,   238,   238,
     238,   238,   238,   239,   239,   239,   239,   239,   239,   240,
     240,   240,   240,   240,   241,   241,   241,   241,   241,   242,
     242,   242,   242,   242,   243,   243,   243,   243,   243,   243,
     244,   244,   244,   244,   244,   244,   244,   245,   245,   245,
     245,   245,   245,   246,   246,   246,   246,   246,   246,   247,
     247,   247,   247,   247,   248,   248,   248,   248,   248,   249,
     249,   249,   249,   249,   250,   250,   250,   250,   250,   251,
     251,   251,   251,   251,   251,   252,   252,   252,   252,   252,
     252,   253,   253,   253,   253,   253,   254,   254,   254,   254,
     258,   258,   258,   264,   265,   266,   270,   271,   272,   276,
     277,   285,   286,   293,   295,   299,   303,   310,   311,   312,
     316,   329,   336,   338,   343,   352,   368,   369,   373,   374,
     377,   379,   380,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   396,   397,   400,   402,   406,   410,   411,   412,
     416,   421,   425,   432,   440,   448,   457,   462,   467,   472,
     477,   482,   487,   492,   497,   502,   507,   512,   517,   522,
     527,   532,   537,   542,   547,   552,   557,   562,   567,   574,
     580,   591,   598,   607,   611,   620,   624,   628,   632,   639,
     640,   645,   651,   657,   663,   669,   670,   671,   672,   673,
     677,   678,   682,   683,   694,   695,   696,   700,   706,   713,
     719,   725,   727,   730,   732,   739,   743,   749,   751,   757,
     759,   763,   774,   776,   780,   784,   791,   792,   796,   797,
     798,   801,   803,   807,   812,   819,   821,   825,   829,   830,
     834,   839,   844,   849,   855,   860,   868,   873,   880,   890,
     891,   892,   893,   894,   895,   896,   897,   898,   900,   901,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     922,   923,   927,   928,   929,   930,   931,   932,   933,   934,
     935,   936,   937,   938,   939,   940,   944,   945,   949,   950,
     954,   955,   956,   960,   961,   965,   966,   970,   971,   977,
     980,   982,   986,   987,   988,   989,   990,   991,   992,   993,
     994,   999,  1004,  1009,  1014,  1023,  1024,  1027,  1029,  1037,
    1042,  1047,  1052,  1053,  1054,  1058,  1063,  1068,  1073,  1082,
    1083,  1087,  1088,  1089,  1101,  1102,  1106,  1107,  1108,  1109,
    1110,  1117,  1118,  1119,  1123,  1124,  1130,  1138,  1139,  1142,
    1144,  1148,  1149,  1153,  1154,  1158,  1159,  1163,  1167,  1168,
    1172,  1173,  1174,  1175,  1176,  1179,  1180,  1184,  1185,  1189,
    1195,  1205,  1213,  1217,  1224,  1225,  1232,  1242,  1248,  1250,
    1254,  1259,  1263,  1270,  1272,  1276,  1277,  1283,  1291,  1292,
    1298,  1302,  1308,  1316,  1317,  1321,  1335,  1341,  1345,  1350,
    1364,  1375,  1376,  1377,  1378,  1379,  1380,  1381,  1382,  1383,
    1387,  1395,  1402,  1413,  1417,  1424,  1425,  1429,  1433,  1434,
    1438,  1442,  1449,  1456,  1462,  1463,  1464,  1468,  1469,  1470,
    1471,  1477,  1488,  1489,  1490,  1491,  1492,  1497,  1508,  1520,
    1529,  1538,  1546,  1556,  1564,  1565,  1569,  1573,  1577,  1587,
    1598,  1609,  1620,  1630,  1641,  1642,  1646,  1649,  1650,  1654,
    1655,  1656,  1657,  1661,  1662,  1666,  1671,  1673,  1677,  1686,
    1690,  1698,  1699,  1703,  1714,  1716,  1723
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
  "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", "TOK_NE", "'<'",
  "'>'", "TOK_LTE", "TOK_GTE", "'+'", "'-'", "'*'", "'/'", "'%'",
  "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", "'}'", "'['",
  "']'", "$accept", "request", "statement", "ident_set", "ident",
  "multi_stmt_list", "multi_stmt", "select", "select1",
  "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg",
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from",
  "select_items_list", "select_item", "opt_alias", "select_expr",
  "ident_list", "opt_where_clause", "where_clause", "where_expr",
  "where_item", "filter_item", "expr_ident", "mva_aggr", "const_int",
  "const_float", "const_list", "string_list", "opt_group_clause",
  "opt_int", "group_items_list", "opt_having_clause",
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
  "attach_index", "flush_rtindex", "flush_ramchunk", "flush_index",
  "flush_hostnames", "select_sysvar", "sysvar_list", "sysvar_item",
  "sysvar_name", "select_dual", "truncate", "optimize_index",
  "create_plugin", "drop_plugin", "reload_plugins", "json_field",
  "json_expr", "subscript", "subkey", "streq", "strval",
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
     124,    38,    61,   385,    60,    62,   386,   387,    43,    45,
      42,    47,    37,   388,   389,    59,    40,    41,    44,   123,
     125,    91,    93
};
# endif

#define YYPACT_NINF -789

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-789)))

#define YYTABLE_NINF -506

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     683,    87,    14,  -789,  4293,  -789,    39,    52,  -789,  -789,
      99,    15,  -789,    45,   120,  -789,  -789,   885,  4418,   322,
       6,    51,  4293,   119,  -789,   -12,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,   124,  -789,  -789,  -789,
    4293,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  4293,  4293,  4293,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,    82,  4293,  4293,  4293,
    4293,  4293,  -789,  -789,  4293,  4293,  4293,  4293,   109,    84,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,    95,    97,   113,
     115,   135,   150,   154,   160,   180,  -789,   184,   186,   189,
     202,   208,   212,   214,   218,   223,  1914,  -789,  1914,  1914,
    3712,    30,   -21,  -789,  3826,   134,  -789,   228,   -28,  -789,
    3826,   127,   130,  -789,   102,   155,   100,  4543,  4293,  2973,
     267,   258,   279,  3951,   294,  -789,   330,  -789,  -789,   330,
    -789,  -789,  -789,  -789,   330,  -789,   330,   321,  -789,  -789,
    4293,   286,  -789,  4293,  -789,   -25,  -789,  1914,   221,  -789,
    4293,   330,   332,    68,   315,    50,   335,   324,   -39,  -789,
     329,  -789,  -789,  -789,   388,   350,  1032,  1914,  2061,   -11,
    2061,  2061,   313,  1914,  2061,  2061,  1914,  1914,  1179,  1914,
    1914,   314,   317,   319,   320,  -789,  -789,  4818,  -789,  -789,
     -47,   336,  -789,  -789,  2208,    34,  -789,  2597,  1326,  4293,
    -789,  -789,  1914,  1914,  1914,  1914,  1914,  1914,  1914,  1914,
    1914,  1914,  1914,  1914,  1914,  1914,  1914,  1914,  1914,  1914,
     462,   460,  -789,  -789,  -789,     3,  1914,  2973,  2973,   339,
     346,   426,  -789,  -789,  -789,  -789,  -789,   349,  -789,  2723,
     422,   383,    19,   384,  -789,   487,  -789,  -789,  -789,  -789,
    4293,  -789,  -789,    49,    -2,  -789,  4293,  4293,  5048,  -789,
    3826,    -3,  1473,  4668,   261,  -789,   351,  -789,  -789,   464,
     465,   396,  -789,  -789,  -789,  -789,   106,     9,  -789,  -789,
    -789,   353,  -789,   187,   497,  2345,  -789,   498,   499,  -789,
     500,   377,  1620,  -789,  5048,   -31,  -789,  4848,    91,  4293,
     364,   107,   132,  -789,  4883,   141,   151,   722,  4672,  -789,
     181,   682,  4918,  -789,  1767,  1914,  1914,  -789,  3712,  -789,
    2848,   360,   179,  -789,  -789,   -39,  -789,  5048,  -789,  -789,
    -789,  5062,  5031,   281,  4865,   604,  -789,   604,   144,   144,
     144,   144,    -5,    -5,  -789,  -789,  -789,  5018,   365,  -789,
    -789,   432,   604,   349,   349,   369,  3473,   508,  2973,  -789,
    -789,  -789,  -789,   513,  -789,  -789,  -789,  -789,   450,   330,
    -789,   330,  -789,   392,   378,  -789,   423,   523,  -789,   425,
     522,  4293,  -789,  -789,    58,    89,  -789,   399,  -789,  -789,
    -789,  1914,   448,  1914,  4065,   412,  4293,  4293,  4293,  -789,
    -789,  -789,  -789,  -789,   192,   194,    50,   389,  -789,  -789,
    -789,  -789,  -789,   428,   433,  -789,   393,   397,   398,   417,
     419,   420,   421,  -789,   424,   427,   430,  2345,    34,   439,
    -789,  -789,   158,     7,  -789,  -789,  -789,  -789,  1326,   434,
    -789,  2061,  -789,  -789,   435,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  1914,  -789,  1914,  -789,  1914,  -789,  4718,  4740,
     440,  -789,  -789,  -789,  -789,  -789,   470,   519,  -789,  4293,
     572,  -789,    54,  -789,  -789,   447,  -789,   140,  -789,  -789,
    2471,  4293,  -789,  -789,  -789,  -789,   451,   452,  -789,    16,
    4293,   321,    94,  -789,   555,   512,  -789,   441,  -789,  -789,
    -789,   196,   444,   379,  -789,  -789,  -789,    54,  -789,   583,
      57,  -789,   445,  -789,  -789,   584,   586,  4293,  4293,  4293,
     455,  4293,   454,   456,  4293,   589,   457,   111,  2345,    94,
      48,     4,    74,    88,    94,    94,    94,    94,    64,    54,
     453,    54,    54,    54,    54,    54,    54,    71,   458,  -789,
    -789,  4948,  4983,  4783,  2848,  1326,   461,   597,   480,   548,
    -789,   204,   466,     1,   518,  -789,  -789,  -789,  -789,  4293,
     467,  -789,   603,  4293,    29,  -789,  -789,  -789,  -789,  -789,
    -789,  3098,  -789,  -789,  4065,    61,   -43,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  4179,    61,  -789,  -789,   468,   469,    34,   471,   472,
     473,  -789,  -789,   474,   475,  -789,  -789,  -789,   488,   502,
    -789,    20,  -789,   544,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,    54,    56,
     503,    54,  -789,  -789,  -789,  -789,  -789,  -789,    54,   482,
    3598,   486,  -789,  -789,   483,  -789,   -10,   552,  -789,   609,
     587,   581,  -789,  4293,  -789,    54,  -789,  -789,  -789,   511,
     220,  -789,   636,  -789,  -789,  -789,   224,   501,   210,   504,
    -789,  -789,    36,   230,  -789,   643,   444,  -789,  -789,  -789,
    -789,   630,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
      94,    94,  -789,   232,   234,  -789,   524,  -789,    20,    54,
     239,   525,    54,  -789,  -789,  -789,   507,  -789,  -789,  2061,
    4293,   622,   588,  3223,   573,  3348,   448,    34,   509,  -789,
     241,    54,  -789,  4293,   515,  -789,   516,  -789,  -789,  3223,
    -789,    61,   532,   520,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,   659,    54,   245,   253,  -789,  -789,    54,   290,
    3598,   303,  3223,   664,  -789,  -789,   526,   641,  -789,   512,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,   665,   643,
    -789,  -789,  -789,  -789,  -789,  -789,  -789,   528,   504,   529,
    3223,  3223,   321,  -789,  -789,   534,   667,  -789,   504,  -789,
    2061,  -789,   305,   531,  -789
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   384,     0,   381,     0,     0,   429,   428,
       0,     0,   387,     0,     0,   388,   382,     0,   464,   464,
       0,     0,     0,     0,     2,     3,   133,   136,   139,   141,
     137,   138,     7,     8,   383,     5,     0,     6,     9,    10,
       0,    11,    12,    13,    28,    14,    15,    16,    23,    17,
      18,    19,    20,    21,    22,    33,    24,    25,    26,    27,
      29,    30,    31,    32,     0,     0,     0,    34,    35,    36,
      38,    37,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    71,    70,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,   131,    91,    90,    92,    93,    94,    95,
      97,    96,    98,    99,   101,   102,   100,   103,   104,   105,
     106,   108,   109,   107,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   132,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,     0,     0,     0,     0,
       0,     0,   481,   482,     0,     0,     0,     0,     0,    34,
     280,   282,   283,   505,   285,   487,   284,    37,    39,    43,
      46,    55,    62,    64,    71,    70,   281,     0,    75,    78,
      85,    87,    97,   100,   115,   127,     0,   158,     0,     0,
       0,   279,     0,   156,   160,   163,   307,     0,   261,   484,
     160,     0,   308,   309,     0,     0,    49,    68,    74,     0,
     107,     0,     0,     0,     0,   461,   340,   465,   355,   340,
     347,   348,   346,   466,   340,   356,   340,   265,   342,   339,
       0,     0,   385,     0,   172,     0,     1,     0,     4,   135,
       0,   340,     0,     0,     0,     0,     0,     0,     0,   477,
       0,   480,   479,   490,   514,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    34,    85,    87,   286,   287,     0,   336,   335,
       0,     0,   499,   500,     0,   496,   497,     0,     0,     0,
     161,   159,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   483,   262,   486,     0,     0,     0,     0,     0,
       0,     0,   379,   380,   378,   377,   376,   362,   374,     0,
       0,     0,   340,     0,   462,     0,   431,   344,   343,   430,
       0,   349,   266,   357,   455,   489,     0,     0,   510,   511,
     160,   506,     0,     0,     0,   134,   389,   427,   452,     0,
       0,     0,   232,   234,   402,   236,     0,     0,   400,   401,
     414,   418,   412,     0,     0,     0,   410,     0,     0,   516,
       0,   329,     0,   318,   328,     0,   326,     0,     0,     0,
       0,     0,     0,   170,     0,     0,     0,     0,     0,   324,
       0,     0,     0,   321,     0,     0,     0,   305,     0,   306,
       0,   505,     0,   498,   150,   174,   157,   163,   162,   297,
     298,   304,   303,   295,   294,   301,   503,   302,   292,   293,
     299,   300,   288,   289,   290,   291,   296,     0,   263,   485,
     310,     0,   504,   363,   364,     0,     0,     0,     0,   370,
     372,   361,   371,     0,   369,   373,   360,   359,     0,   340,
     345,   340,   341,   270,   267,   268,     0,     0,   352,     0,
       0,     0,   453,   456,     0,     0,   433,     0,   173,   509,
     508,     0,   252,     0,     0,     0,     0,     0,     0,   233,
     235,   416,   404,   237,     0,     0,     0,     0,   473,   474,
     472,   476,   475,     0,     0,   220,    37,    39,    46,    55,
      62,     0,    71,   224,    78,    84,   127,     0,   219,   176,
     177,   181,     0,     0,   225,   492,   515,   493,     0,     0,
     312,     0,   164,   315,     0,   169,   317,   316,   168,   313,
     314,   165,     0,   166,     0,   325,     0,   167,     0,     0,
       0,   338,   337,   333,   502,   501,     0,   241,   175,     0,
       0,   311,     0,   367,   366,     0,   375,     0,   350,   351,
       0,     0,   354,   358,   353,   454,     0,   457,   458,     0,
       0,   265,     0,   512,     0,   261,   253,   507,   392,   391,
     393,     0,     0,     0,   451,   478,   403,     0,   415,     0,
     423,   413,   419,   420,   411,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   143,   327,
     171,     0,     0,     0,     0,     0,     0,   243,   249,     0,
     264,     0,     0,     0,     0,   470,   463,   272,   275,     0,
     271,   269,     0,     0,     0,   435,   436,   434,   432,   439,
     440,     0,   513,   390,     0,     0,   405,   395,   442,   444,
     443,   449,   441,   447,   445,   446,   448,   450,   238,   417,
     424,     0,     0,   471,   491,     0,     0,     0,     0,     0,
       0,   229,   222,     0,     0,   223,   179,   178,     0,     0,
     188,     0,   207,     0,   205,   182,   196,   206,   183,   197,
     193,   202,   192,   201,   195,   204,   194,   203,     0,     0,
       0,     0,   209,   210,   215,   216,   217,   218,     0,     0,
       0,     0,   320,   319,     0,   334,     0,     0,   244,     0,
       0,   247,   250,     0,   365,     0,   468,   467,   469,     0,
       0,   276,     0,   460,   459,   438,     0,    97,   258,   254,
     256,   394,     0,     0,   398,     0,     0,   386,   426,   425,
     422,   423,   421,   231,   230,   228,   221,   227,   226,   180,
       0,     0,   239,     0,     0,   208,     0,   189,     0,     0,
       0,     0,     0,   148,   149,   147,   144,   145,   140,     0,
       0,     0,   152,     0,     0,     0,   252,   495,     0,   494,
       0,     0,   273,     0,     0,   437,     0,   259,   260,     0,
     397,     0,     0,   406,   407,   396,   190,   199,   200,   198,
     184,   186,     0,     0,     0,     0,   213,   211,     0,     0,
       0,     0,     0,     0,   142,   245,   242,     0,   248,   261,
     322,   368,   278,   277,   274,   255,   257,   399,     0,     0,
     240,   191,   185,   187,   214,   212,   146,     0,   151,   153,
       0,     0,   265,   409,   408,     0,     0,   246,   251,   155,
       0,   154,     0,     0,   323
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -789,  -789,  -789,   -16,    -4,  -789,   431,  -789,   282,  -789,
    -789,  -187,  -789,  -789,  -789,   110,    23,   386,  -196,  -789,
     -15,  -789,  -346,  -508,  -789,  -150,  -680,  -789,  -170,  -593,
    -574,  -127,  -789,  -789,  -789,  -789,  -789,  -789,  -144,  -789,
    -788,  -154,  -601,  -789,  -598,  -789,  -789,   105,  -789,  -146,
     233,  -789,  -265,   148,  -789,  -789,   273,    38,  -789,  -234,
    -789,  -789,  -789,  -789,  -789,   237,  -789,   117,   236,  -789,
    -789,  -789,  -789,  -789,    12,  -789,  -789,   -87,  -789,  -515,
    -789,  -789,  -179,  -789,  -789,  -789,   195,  -789,  -789,  -789,
       0,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,   114,
    -789,  -789,  -789,  -789,  -789,  -789,    32,  -789,  -789,  -789,
     697,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
    -789,  -789,   391,  -789,  -789,  -789,  -789,  -789,  -789,  -789,
     -56,  -235,  -789,   414,  -789,   402,  -789,  -789,   226,  -789,
     225,  -789,  -789,  -789
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    23,    24,   165,   211,    25,    26,    27,    28,   761,
     826,   827,   576,   832,   874,    29,   212,   213,   311,   214,
     435,   577,   396,   539,   540,   541,   542,   543,   513,   389,
     514,   814,   668,   769,   876,   836,   771,   772,   605,   606,
     789,   790,   332,   333,   361,   362,   484,   485,   780,   781,
     404,   216,   405,   406,   217,   300,   301,   573,    30,   356,
     249,   250,   489,    31,    32,   476,   477,   347,   348,    33,
      34,    35,    36,   505,   610,   611,   696,   697,   793,   390,
     797,   853,   854,    37,    38,   391,   392,   515,   517,   622,
     623,   711,   800,    39,    40,    41,    42,    43,   495,   496,
     707,    44,    45,   492,   493,   597,   598,    46,    47,    48,
     232,   676,    49,   523,    50,    51,    52,    53,    54,    55,
      56,   218,   219,   220,    57,    58,    59,    60,    61,    62,
     221,   222,   305,   306,   223,   224,   502,   503,   369,   370,
     371,   259,   399,    63
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     166,   620,   231,   688,   692,   357,   686,   255,   671,   690,
     358,   788,   359,   408,   382,   411,   412,   511,   254,   415,
     416,   382,   383,   420,   334,   382,   500,   377,   812,   637,
     385,   307,   409,   649,   382,   257,   261,   776,   172,   312,
     795,   382,   830,   302,   303,   330,   729,   302,   303,   736,
     739,   741,   743,   745,   747,   382,   383,   730,   384,   382,
     262,   263,   264,   486,   385,   817,   382,   383,   650,   384,
     490,   302,   303,   313,   173,   385,    66,   710,   366,   382,
     383,   487,   734,   379,   898,   460,   732,   395,   385,   578,
     748,   355,   167,   382,   383,   388,   737,   758,  -495,   382,
     383,   428,   385,   429,   169,   796,   174,   176,   385,   367,
     786,   509,   510,   908,   380,   175,   550,   551,   480,   256,
     331,   777,   252,   367,   491,   749,   168,   308,  -113,   410,
     727,   497,   759,   258,  -488,   326,   327,   328,   308,   651,
     652,   653,   654,   655,   656,   501,   461,   733,   473,   601,
     657,   253,   170,   875,   268,   386,   512,   813,   488,   473,
     544,   275,   684,   266,   267,   254,   269,   270,   473,   788,
     271,   272,   273,   274,   499,   473,   785,   820,   312,   475,
     794,   304,   177,   512,   639,   304,   171,    64,   312,   386,
     599,   260,   788,   473,   731,   335,   387,   801,  -494,    65,
     386,   840,   818,   338,  -414,  -414,   299,   792,   178,   304,
     310,   340,   313,   386,   518,   395,   310,   857,   859,   640,
     907,   788,   313,   312,   341,   346,   641,   386,   265,   353,
     276,   847,   673,   386,   336,   674,   519,   600,   553,   551,
     638,  -331,   675,  -330,   864,   588,   363,   589,   869,   365,
     215,   848,   520,   521,   556,   551,   376,   313,   726,   277,
     572,   278,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   233,   902,   557,
     551,   279,   324,   325,   326,   327,   328,   337,   559,   551,
     642,   643,   644,   645,   646,   647,   280,   522,   560,   551,
     281,   648,   544,   254,   909,   438,   282,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   372,   238,   373,   312,   283,   374,   565,   551,
     284,   575,  -332,   346,   346,   285,   887,   239,   233,   616,
     617,   618,   619,   693,   694,   474,   388,   240,   286,   241,
     242,   774,   617,   234,   287,   235,   483,   231,   288,   313,
     289,   236,   494,   498,   290,   497,   310,   842,   843,   291,
     244,   845,   617,   245,   329,   247,   237,   850,   851,   860,
     617,   861,   862,  -466,   238,   248,   867,   617,   881,   617,
     349,   538,   892,   617,   718,   350,   720,   354,   239,   723,
     893,   862,   355,   544,   360,   554,   698,   699,   240,   364,
     241,   242,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   299,   378,   571,   243,   700,   685,
     381,   244,   689,   393,   245,   246,   247,   895,   617,   295,
     398,   296,   297,   394,   701,   702,   248,   708,   397,   703,
     897,   551,   913,   551,   463,   464,   544,   400,   704,   705,
     413,   423,   474,   424,   346,   425,   426,   458,   430,   728,
     185,   465,   735,   738,   740,   742,   744,   746,   466,   750,
     467,   752,   753,   754,   755,   756,   757,   596,   468,   706,
     368,   478,   479,   481,   572,   482,   508,   504,   506,   507,
     609,   516,   613,   614,   615,   524,   545,   546,   547,  -505,
     407,   555,   574,   580,   581,   582,   414,   585,   509,   417,
     418,   587,   421,   422,   590,   388,   591,   592,   593,   594,
     595,   602,   604,   538,   612,   625,   624,   432,   839,   627,
     626,   437,   388,   628,   629,   439,   440,   441,   442,   443,
     444,   445,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   630,   871,   631,   632,   633,   638,   462,
     634,   665,   664,   635,   667,   669,   636,   670,   816,   672,
     683,   658,   660,   682,   691,   330,   680,   483,   821,   501,
     695,   709,   713,   712,   714,   719,   494,   724,   544,   751,
     544,   721,   768,   722,   725,   437,   760,   770,   767,   773,
     778,   783,   775,   782,   544,   803,   804,   810,   805,   806,
     807,   808,   809,   715,   716,   717,   815,   717,   822,   829,
     717,   811,   819,   828,   538,   297,   831,   544,   833,   835,
     856,   858,   834,   841,   844,   912,   852,   846,   312,   866,
     710,   872,   849,   863,   868,   870,   880,   877,   568,   569,
     571,   873,   884,   885,   888,   544,   544,   890,   889,   899,
     901,   882,   911,   903,   900,   779,   905,   906,   914,   596,
     910,   388,   313,   896,   549,   878,   666,   538,   766,   375,
     609,   865,   879,   891,   436,   886,   681,   883,   894,   659,
       1,   570,   765,   584,   586,     2,   791,   799,     3,   855,
     904,   621,   802,     4,   687,   784,   251,   838,     5,   433,
     446,     6,   459,     7,     8,     9,   312,   603,   607,    10,
       0,     0,     0,    11,   368,     0,   368,     0,   320,   321,
     322,   323,   324,   325,   326,   327,   328,    12,     0,     0,
       0,     0,     0,     0,     0,     0,   825,     0,     0,     0,
     313,     0,     0,     0,     0,     0,   312,     0,    13,   837,
       0,     0,     0,     0,     0,     0,     0,    14,     0,    15,
       0,   437,    16,     0,    17,     0,    18,     0,     0,    19,
       0,    20,     0,     0,     0,   661,     0,   662,     0,   663,
     313,    21,     0,     0,    22,     0,     0,     0,     0,     0,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,     0,   254,     0,     0,   538,
     566,   538,     0,     0,     0,     0,     0,     0,     0,   779,
       0,     0,     0,     0,     0,   538,     0,     0,     0,     0,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,     0,   825,     0,   538,   561,
     562,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   179,   180,
     181,   182,     0,   183,   184,   185,   538,   538,   437,   186,
      68,    69,    70,   187,   188,     0,    73,    74,    75,   189,
      77,    78,   190,    80,     0,    81,    82,    83,    84,    85,
      86,    87,   191,    89,    90,    91,    92,    93,    94,     0,
     192,    96,     0,     0,   193,    98,    99,     0,   100,   101,
     102,   194,   195,   105,   106,   196,   197,   107,   198,   109,
     110,   199,   112,     0,   113,   114,   115,   116,     0,   117,
     200,   119,   201,     0,   121,   122,   123,     0,   124,     0,
     125,   126,   127,   128,   129,   202,   131,   132,   133,     0,
     134,   135,   203,   137,   138,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   204,   150,   151,   152,
     153,   154,     0,   155,   156,   157,   158,   159,   160,   161,
     205,   163,   164,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   206,   207,     0,     0,   208,     0,
       0,   209,     0,     0,   210,   292,   180,   181,   182,     0,
     401,   184,     0,     0,     0,     0,   186,    68,    69,    70,
     187,   188,     0,    73,    74,    75,    76,    77,    78,   190,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,   192,    96,     0,
       0,   193,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   196,   197,   107,   198,   109,   110,   199,   112,
       0,   113,   114,   115,   116,     0,   117,   293,   119,   294,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   202,   131,   132,   133,     0,   134,   135,   203,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,   158,   159,   160,   161,   205,   163,   164,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   206,     0,     0,     0,   208,     0,     0,   402,   403,
       0,   210,   292,   180,   181,   182,     0,   401,   184,     0,
       0,     0,     0,   186,    68,    69,    70,   187,   188,     0,
      73,    74,    75,    76,    77,    78,   190,    80,     0,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,     0,   192,    96,     0,     0,   193,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   196,
     197,   107,   198,   109,   110,   199,   112,     0,   113,   114,
     115,   116,     0,   117,   293,   119,   294,     0,   121,   122,
     123,     0,   124,     0,   125,   126,   127,   128,   129,   202,
     131,   132,   133,     0,   134,   135,   203,   137,   138,   139,
       0,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,     0,   155,   156,   157,
     158,   159,   160,   161,   205,   163,   164,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   206,     0,
       0,     0,   208,     0,     0,   209,   419,     0,   210,   292,
     180,   181,   182,     0,   183,   184,     0,     0,     0,     0,
     186,    68,    69,    70,   187,   188,     0,    73,    74,    75,
     189,    77,    78,   190,    80,     0,    81,    82,    83,    84,
      85,    86,    87,   191,    89,    90,    91,    92,    93,    94,
       0,   192,    96,     0,     0,   193,    98,    99,     0,   100,
     101,   102,   194,   195,   105,   106,   196,   197,   107,   198,
     109,   110,   199,   112,     0,   113,   114,   115,   116,     0,
     117,   200,   119,   201,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   202,   131,   132,   133,
       0,   134,   135,   203,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   204,   150,   151,
     152,   153,   154,     0,   155,   156,   157,   158,   159,   160,
     161,   205,   163,   164,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   206,   207,     0,     0,   208,
       0,     0,   209,     0,     0,   210,   179,   180,   181,   182,
       0,   183,   184,     0,     0,     0,     0,   186,    68,    69,
      70,   187,   188,     0,    73,    74,    75,   189,    77,    78,
     190,    80,     0,    81,    82,    83,    84,    85,    86,    87,
     191,    89,    90,    91,    92,    93,    94,     0,   192,    96,
       0,     0,   193,    98,    99,     0,   100,   101,   102,   194,
     195,   105,   106,   196,   197,   107,   198,   109,   110,   199,
     112,     0,   113,   114,   115,   116,     0,   117,   200,   119,
     201,     0,   121,   122,   123,     0,   124,     0,   125,   126,
     127,   128,   129,   202,   131,   132,   133,     0,   134,   135,
     203,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   204,   150,   151,   152,   153,   154,
       0,   155,   156,   157,   158,   159,   160,   161,   205,   163,
     164,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   206,   207,     0,     0,   208,     0,     0,   209,
       0,     0,   210,   292,   180,   181,   182,     0,   183,   184,
       0,     0,     0,     0,   186,    68,    69,    70,   187,   188,
       0,    73,    74,    75,    76,    77,    78,   190,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,   192,    96,     0,     0,   193,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     196,   197,   107,   198,   109,   110,   199,   112,     0,   113,
     114,   115,   116,     0,   117,   293,   119,   294,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     202,   131,   132,   133,     0,   134,   135,   203,   137,   138,
     139,   548,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   205,   163,   164,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   206,
       0,     0,     0,   208,     0,     0,   209,     0,     0,   210,
     292,   180,   181,   182,     0,   401,   184,     0,     0,     0,
       0,   186,    68,    69,    70,   187,   188,     0,    73,    74,
      75,    76,    77,    78,   190,    80,     0,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,   192,    96,     0,     0,   193,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   196,   197,   107,
     198,   109,   110,   199,   112,     0,   113,   114,   115,   116,
       0,   117,   293,   119,   294,     0,   121,   122,   123,     0,
     124,     0,   125,   126,   127,   128,   129,   202,   131,   132,
     133,     0,   134,   135,   203,   137,   138,   139,     0,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   155,   156,   157,   158,   159,
     160,   161,   205,   163,   164,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   206,     0,     0,     0,
     208,     0,     0,   209,   403,     0,   210,   292,   180,   181,
     182,     0,   183,   184,     0,     0,     0,     0,   186,    68,
      69,    70,   187,   188,     0,    73,    74,    75,    76,    77,
      78,   190,    80,     0,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,   192,
      96,     0,     0,   193,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   196,   197,   107,   198,   109,   110,
     199,   112,     0,   113,   114,   115,   116,     0,   117,   293,
     119,   294,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   202,   131,   132,   133,     0,   134,
     135,   203,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   205,
     163,   164,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   206,     0,     0,     0,   208,     0,     0,
     209,     0,     0,   210,   292,   180,   181,   182,     0,   401,
     184,     0,     0,     0,     0,   186,    68,    69,    70,   187,
     188,     0,    73,    74,    75,    76,    77,    78,   190,    80,
       0,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,   192,    96,     0,     0,
     193,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   196,   197,   107,   198,   109,   110,   199,   112,     0,
     113,   114,   115,   116,     0,   117,   293,   119,   294,     0,
     121,   122,   123,     0,   124,     0,   125,   126,   127,   128,
     129,   202,   131,   132,   133,     0,   134,   135,   203,   137,
     138,   139,     0,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,   155,
     156,   157,   158,   159,   160,   161,   205,   163,   164,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     206,     0,     0,     0,   208,     0,     0,   209,     0,     0,
     210,   292,   180,   181,   182,     0,   431,   184,     0,     0,
       0,     0,   186,    68,    69,    70,   187,   188,     0,    73,
      74,    75,    76,    77,    78,   190,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,   192,    96,     0,     0,   193,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   196,   197,
     107,   198,   109,   110,   199,   112,     0,   113,   114,   115,
     116,     0,   117,   293,   119,   294,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   202,   131,
     132,   133,     0,   134,   135,   203,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   205,   163,   164,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   206,    67,   525,
       0,   208,     0,     0,   209,     0,     0,   210,     0,     0,
      68,    69,    70,   526,   527,     0,    73,    74,    75,    76,
      77,    78,   528,    80,     0,    81,    82,    83,    84,    85,
      86,    87,   529,    89,    90,    91,    92,    93,    94,     0,
     530,    96,   531,     0,    97,    98,    99,     0,   100,   101,
     102,   532,   104,   105,   106,   533,     0,   107,   108,   109,
     110,   534,   112,     0,   113,   114,   115,   116,     0,   535,
     118,   119,   120,     0,   121,   122,   123,     0,   124,     0,
     125,   126,   127,   128,   129,   130,   131,   132,   133,     0,
     134,   135,   136,   137,   138,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,   158,   159,   160,   161,
     536,   163,   164,     0,    67,     0,   677,     0,     0,   678,
       0,     0,     0,     0,     0,     0,    68,    69,    70,    71,
      72,   537,    73,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,    95,    96,     0,     0,
      97,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,     0,     0,   107,   108,   109,   110,   111,   112,     0,
     113,   114,   115,   116,     0,   117,   118,   119,   120,     0,
     121,   122,   123,     0,   124,     0,   125,   126,   127,   128,
     129,   130,   131,   132,   133,     0,   134,   135,   136,   137,
     138,   139,     0,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,     0,
      67,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    68,    69,    70,    71,    72,   679,    73,    74,
      75,    76,    77,    78,    79,    80,     0,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,    95,    96,     0,     0,    97,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,     0,     0,   107,
     108,   109,   110,   111,   112,     0,   113,   114,   115,   116,
       0,   117,   118,   119,   120,     0,   121,   122,   123,     0,
     124,     0,   125,   126,   127,   128,   129,   130,   131,   132,
     133,     0,   134,   135,   136,   137,   138,   139,     0,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,     0,    67,     0,   382,     0,
       0,   469,     0,     0,     0,     0,     0,     0,    68,    69,
      70,    71,    72,   434,    73,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,    95,    96,
       0,   470,    97,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,     0,     0,   107,   108,   109,   110,   111,
     112,     0,   113,   114,   115,   116,     0,   117,   118,   119,
     120,     0,   121,   122,   123,   471,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   133,     0,   134,   135,
     136,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     472,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    67,     0,   382,     0,     0,     0,     0,     0,     0,
       0,     0,   473,    68,    69,    70,    71,    72,     0,    73,
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
     159,   160,   161,   162,   163,   164,    67,     0,   342,   343,
       0,   344,     0,     0,     0,     0,     0,   473,    68,    69,
      70,    71,    72,     0,    73,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,    95,    96,
       0,     0,    97,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,     0,     0,   107,   108,   109,   110,   111,
     112,     0,   113,   114,   115,   116,     0,   117,   118,   119,
     120,     0,   121,   122,   123,   345,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   133,     0,   134,   135,
     136,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    67,   525,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,   528,    80,     0,    81,    82,
      83,    84,    85,    86,    87,   529,    89,    90,    91,    92,
      93,    94,     0,   530,    96,   531,     0,    97,    98,    99,
       0,   100,   101,   102,   532,   104,   105,   106,   533,     0,
     107,   108,   109,   110,   534,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   787,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   536,   163,   164,    67,   525,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    68,    69,
      70,    71,    72,     0,    73,    74,    75,    76,    77,    78,
     528,    80,     0,    81,    82,    83,    84,    85,    86,    87,
     529,    89,    90,    91,    92,    93,    94,     0,   530,    96,
     531,     0,    97,    98,    99,     0,   100,   101,   102,   532,
     104,   105,   106,   533,     0,   107,   108,   109,   110,   534,
     112,     0,   113,   114,   115,   116,     0,   117,   118,   119,
     120,     0,   121,   122,   123,     0,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   133,     0,   134,   135,
     136,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   155,   156,   157,   158,   159,   160,   161,   536,   163,
     164,    67,   525,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    68,    69,    70,   526,   527,     0,    73,
      74,    75,    76,    77,    78,   528,    80,     0,    81,    82,
      83,    84,    85,    86,    87,   529,    89,    90,    91,    92,
      93,    94,     0,   530,    96,   531,     0,    97,    98,    99,
       0,   100,   101,   102,   532,   104,   105,   106,   533,     0,
     107,   108,   109,   110,   534,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   536,   163,   164,    67,     0,   583,     0,
       0,   469,     0,     0,     0,     0,     0,     0,    68,    69,
      70,    71,    72,     0,    73,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,    95,    96,
       0,     0,    97,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,     0,     0,   107,   108,   109,   110,   111,
     112,     0,   113,   114,   115,   116,     0,   117,   118,   119,
     120,     0,   121,   122,   123,     0,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   133,     0,   134,   135,
     136,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    67,     0,   823,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   824,     0,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,    67,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,     0,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,   298,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,    67,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
       0,    68,    69,    70,    71,    72,   309,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,     0,     0,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,    67,     0,     0,     0,     0,   351,
       0,     0,     0,     0,     0,     0,    68,    69,    70,    71,
      72,     0,    73,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,    95,    96,     0,     0,
      97,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,     0,     0,   107,   108,   109,   110,   111,   112,     0,
     113,   114,   115,   116,     0,   117,   118,   119,   120,     0,
     121,   122,   123,     0,   124,     0,   125,   126,   127,   128,
     129,   130,   131,   132,   133,     0,   134,   135,   136,   137,
     138,   139,     0,   140,   141,   142,   143,   144,   145,   146,
     352,   148,   149,   150,   151,   152,   153,   154,    67,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,     0,
      68,    69,    70,    71,    72,     0,    73,    74,    75,    76,
      77,    78,    79,    80,     0,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
      95,    96,     0,     0,    97,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   608,     0,   107,   108,   109,
     110,   111,   112,     0,   113,   114,   115,   116,     0,   117,
     118,   119,   120,     0,   121,   122,   123,     0,   124,     0,
     125,   126,   127,   128,   129,   130,   131,   132,   133,     0,
     134,   135,   136,   137,   138,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,    67,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,     0,    68,    69,    70,    71,    72,     0,
      73,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,     0,    95,    96,     0,     0,    97,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,     0,
       0,   107,   108,   109,   110,   111,   112,     0,   113,   114,
     115,   116,   798,   117,   118,   119,   120,     0,   121,   122,
     123,     0,   124,     0,   125,   126,   127,   128,   129,   130,
     131,   132,   133,     0,   134,   135,   136,   137,   138,   139,
       0,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,    67,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,     0,    68,    69,
      70,    71,    72,     0,    73,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,    95,    96,
       0,     0,    97,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,     0,     0,   107,   108,   109,   110,   111,
     112,     0,   113,   114,   115,   116,     0,   117,   118,   119,
     120,     0,   121,   122,   123,     0,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   133,     0,   134,   135,
     136,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    67,     0,     0,     0,     0,     0,     0,   225,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,   226,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,   227,   102,   103,   104,   105,   106,     0,     0,
     228,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   229,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   230,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,     0,     0,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    67,     0,     0,     0,
       0,     0,   339,     0,     0,     0,     0,     0,    68,    69,
      70,    71,    72,     0,    73,    74,    75,    76,    77,    78,
      79,    80,     0,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,    95,    96,
       0,     0,    97,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,     0,     0,   107,   108,   109,   110,   111,
     112,     0,   113,   114,   115,   116,     0,   117,   118,   119,
     120,     0,   121,   122,     0,     0,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   133,     0,   134,   135,
     136,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,  -465,
       0,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    67,     0,     0,     0,     0,     0,     0,   225,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,   226,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,   312,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,     0,     0,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   229,
     313,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   312,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   312,     0,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   313,     0,     0,     0,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,     0,     0,     0,   313,   563,
     564,     0,     0,     0,     0,     0,     0,   312,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   313,   312,     0,     0,     0,   562,     0,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,     0,     0,     0,     0,     0,   564,     0,
       0,     0,   312,     0,     0,     0,   313,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   312,
       0,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   313,   312,     0,     0,
       0,   764,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   313,     0,     0,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   313,   312,     0,     0,   427,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,     0,   312,     0,     0,   552,   313,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,     0,     0,
       0,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   313,   312,     0,     0,
     558,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   313,   312,     0,     0,   567,     0,     0,     0,   579,
       0,     0,     0,     0,     0,   312,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,     0,   312,     0,     0,   762,   313,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   312,     0,     0,   313,
       0,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   313,     0,     0,     0,
     763,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     313,     0,     0,     0,     0,     0,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,     0,     0,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328
};

static const yytype_int16 yycheck[] =
{
       4,   516,    18,   601,   605,   239,   599,    22,   582,   602,
     244,   691,   246,   278,     5,   280,   281,     8,    22,   284,
     285,     5,     6,   288,   220,     5,    29,   261,     8,   537,
      14,    52,    43,    26,     5,    47,    40,    36,    23,    44,
      83,     5,    52,    13,    14,    73,   639,    13,    14,   642,
     643,   644,   645,   646,   647,     5,     6,     9,     8,     5,
      64,    65,    66,    14,    14,     9,     5,     6,    61,     8,
      72,    13,    14,    78,    59,    14,    62,    20,   103,     5,
       6,    32,     8,    15,   872,    82,    82,   126,    14,   435,
      26,    72,    53,     5,     6,   265,     8,    26,    68,     5,
       6,   148,    14,   150,    52,   148,    91,    62,    14,   148,
     684,     5,     6,   901,    46,   100,   147,   148,   352,     0,
     148,   120,   116,   148,   126,    61,    87,   148,   109,   140,
     638,   366,    61,   145,     0,   140,   141,   142,   148,   132,
     133,   134,   135,   136,   137,   148,   143,   143,   139,   495,
     143,   100,    53,   833,   169,   139,   147,   731,   109,   139,
     395,    52,   146,   167,   168,   169,   170,   171,   139,   849,
     174,   175,   176,   177,   370,   139,   147,   751,    44,   349,
     695,   151,    62,   147,    26,   151,    87,   100,    44,   139,
     132,    67,   872,   139,   146,    68,   146,   712,    68,   112,
     139,   775,   146,   103,   147,   148,   210,   146,    88,   151,
     214,   227,    78,   139,    27,   126,   220,   810,   811,    61,
     900,   901,    78,    44,   228,   229,    68,   139,   146,   233,
     146,    21,    92,   139,   132,    95,    49,   148,   147,   148,
     129,   146,   102,   146,   818,   479,   250,   481,   822,   253,
      17,    41,    65,    66,   147,   148,   260,    78,   147,   146,
     430,   146,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,    16,   879,   147,
     148,   146,   138,   139,   140,   141,   142,   132,   147,   148,
     132,   133,   134,   135,   136,   137,   146,   110,   147,   148,
     146,   143,   537,   307,   902,   309,   146,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   101,    62,   103,    44,   146,   106,   147,   148,
     146,   152,   146,   337,   338,   146,   851,    76,    16,   147,
     148,   147,   148,   147,   148,   349,   516,    86,   146,    88,
      89,   147,   148,    31,   146,    33,   360,   373,   146,    78,
     146,    39,   366,   367,   146,   600,   370,   147,   148,   146,
     109,   147,   148,   112,   146,   114,    54,   147,   148,   147,
     148,   147,   148,   116,    62,   124,   147,   148,   147,   148,
     132,   395,   147,   148,   629,   116,   631,   103,    76,   634,
     147,   148,    72,   638,    83,   409,    27,    28,    86,   123,
      88,    89,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   428,    93,   430,   105,    49,   599,
     115,   109,   602,    98,   112,   113,   114,   147,   148,   206,
      52,   208,   209,   119,    65,    66,   124,   617,   119,    70,
     147,   148,   147,   148,   337,   338,   691,   107,    79,    80,
     147,   147,   466,   146,   468,   146,   146,     5,   132,   639,
      10,   132,   642,   643,   644,   645,   646,   647,   132,   649,
      54,   651,   652,   653,   654,   655,   656,   491,   139,   110,
     257,    69,   109,   109,   664,     8,   100,   146,    34,    34,
     504,   148,   506,   507,   508,     8,     8,     8,     8,   132,
     277,   147,   152,   148,    82,   146,   283,     9,     5,   286,
     287,    71,   289,   290,   132,   695,   148,   104,     5,   104,
       8,   132,    84,   537,   122,   107,   147,   304,   773,   146,
     107,   308,   712,   146,   146,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   146,   829,   146,   146,   146,   129,   336,
     146,   101,   132,   146,    55,   579,   146,     5,   748,   132,
     128,   147,   147,   132,    29,    73,   590,   591,   758,   148,
     146,     8,     8,   148,     8,   140,   600,     8,   833,   146,
     835,   147,     5,   147,   147,   372,   148,   127,   147,    61,
      92,     8,   146,   146,   849,   147,   147,   129,   147,   147,
     147,   147,   147,   627,   628,   629,    82,   631,   146,   146,
     634,   129,   129,   147,   638,   402,    84,   872,    29,    58,
     810,   811,    55,   132,     8,   910,     3,   146,    44,   819,
      20,    29,   148,   129,   129,   148,   147,    84,   425,   426,
     664,    73,   147,   147,   132,   900,   901,     8,   148,     5,
      29,   841,     5,     8,   148,   679,   148,   148,   147,   683,
     146,   851,    78,   870,   402,   835,   576,   691,   665,   258,
     694,   818,   836,   863,   308,   849,   591,   843,   868,   551,
      17,   428,   664,   466,   468,    22,   694,   711,    25,   796,
     889,   516,   712,    30,   600,   683,    19,   773,    35,   305,
     318,    38,   331,    40,    41,    42,    44,   501,   503,    46,
      -1,    -1,    -1,    50,   501,    -1,   503,    -1,   134,   135,
     136,   137,   138,   139,   140,   141,   142,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   760,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,    44,    -1,    85,   773,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    94,    -1,    96,
      -1,   548,    99,    -1,   101,    -1,   103,    -1,    -1,   106,
      -1,   108,    -1,    -1,    -1,   562,    -1,   564,    -1,   566,
      78,   118,    -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,    -1,   830,    -1,    -1,   833,
     148,   835,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   843,
      -1,    -1,    -1,    -1,    -1,   849,    -1,    -1,    -1,    -1,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,    -1,   870,    -1,   872,   147,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    10,   900,   901,   665,    14,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    -1,    74,
      75,    76,    77,    -1,    79,    80,    81,    -1,    83,    -1,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    99,   100,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   139,   140,    -1,    -1,   143,    -1,
      -1,   146,    -1,    -1,   149,     3,     4,     5,     6,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    -1,   143,    -1,    -1,   146,   147,
      -1,   149,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    74,    75,    76,    77,    -1,    79,    80,
      81,    -1,    83,    -1,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    99,   100,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,
      -1,    -1,   143,    -1,    -1,   146,   147,    -1,   149,     3,
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
     124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   139,   140,    -1,    -1,   143,
      -1,    -1,   146,    -1,    -1,   149,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    74,    75,    76,
      77,    -1,    79,    80,    81,    -1,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,   140,    -1,    -1,   143,    -1,    -1,   146,
      -1,    -1,   149,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    -1,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,
      -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,   149,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      -1,    74,    75,    76,    77,    -1,    79,    80,    81,    -1,
      83,    -1,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,   100,    -1,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,    -1,
     143,    -1,    -1,   146,   147,    -1,   149,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,    15,
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
     126,   127,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   139,    -1,    -1,    -1,   143,    -1,    -1,
     146,    -1,    -1,   149,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    -1,    74,    75,    76,    77,    -1,
      79,    80,    81,    -1,    83,    -1,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    -1,    95,    96,    97,    98,
      99,   100,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     139,    -1,    -1,    -1,   143,    -1,    -1,   146,    -1,    -1,
     149,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
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
     122,   123,   124,   125,   126,   127,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   139,     3,     4,
      -1,   143,    -1,    -1,   146,    -1,    -1,   149,    -1,    -1,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    47,    -1,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    -1,    74,
      75,    76,    77,    -1,    79,    80,    81,    -1,    83,    -1,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    99,   100,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,    -1,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,    -1,     3,    -1,     5,    -1,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,
      19,   146,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    -1,    -1,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    -1,    74,    75,    76,    77,    -1,
      79,    80,    81,    -1,    83,    -1,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    -1,    95,    96,    97,    98,
      99,   100,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,    -1,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    -1,
       3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    16,    17,    18,    19,   146,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,
      53,    54,    55,    56,    57,    58,    59,    -1,    -1,    62,
      63,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      -1,    74,    75,    76,    77,    -1,    79,    80,    81,    -1,
      83,    -1,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    -1,    95,    96,    97,    98,    99,   100,    -1,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,    -1,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,    -1,     3,    -1,     5,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,   146,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    48,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    74,    75,    76,
      77,    -1,    79,    80,    81,    82,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   139,    15,    16,    17,    18,    19,    -1,    21,
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
     122,   123,   124,   125,   126,   127,     3,    -1,     5,     6,
      -1,     8,    -1,    -1,    -1,    -1,    -1,   139,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    74,    75,    76,
      77,    -1,    79,    80,    81,    82,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     122,   123,   124,   125,   126,   127,     3,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      47,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    74,    75,    76,
      77,    -1,    79,    80,    81,    -1,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     122,   123,   124,   125,   126,   127,     3,    -1,     5,    -1,
      -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    74,    75,    76,
      77,    -1,    79,    80,    81,    -1,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,     3,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    -1,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    61,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    -1,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,     3,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
      -1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,     3,    -1,    -1,    -1,    -1,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      -1,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    -1,    45,    46,    -1,    -1,
      49,    50,    51,    -1,    53,    54,    55,    56,    57,    58,
      59,    -1,    -1,    62,    63,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    -1,    74,    75,    76,    77,    -1,
      79,    80,    81,    -1,    83,    -1,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    -1,    95,    96,    97,    98,
      99,   100,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,     3,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,    -1,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    -1,
      45,    46,    -1,    -1,    49,    50,    51,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    -1,    74,
      75,    76,    77,    -1,    79,    80,    81,    -1,    83,    -1,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    -1,
      95,    96,    97,    98,    99,   100,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,     3,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,    -1,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    -1,    79,    80,
      81,    -1,    83,    -1,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    99,   100,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,     3,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,    -1,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    74,    75,    76,
      77,    -1,    79,    80,    81,    -1,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     3,    -1,    -1,    -1,    -1,    -1,    -1,    10,    -1,
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
     112,   113,   114,   115,    -1,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,     3,    -1,    -1,    -1,
      -1,    -1,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    -1,    45,    46,
      -1,    -1,    49,    50,    51,    -1,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    62,    63,    64,    65,    66,
      67,    -1,    69,    70,    71,    72,    -1,    74,    75,    76,
      77,    -1,    79,    80,    -1,    -1,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    -1,    95,    96,
      97,    98,    99,   100,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
      -1,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     3,    -1,    -1,    -1,    -1,    -1,    -1,    10,    -1,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    44,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      78,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    44,    95,    96,    97,    98,    99,   100,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,    44,    -1,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,    78,    -1,    -1,    -1,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,    -1,    -1,    -1,    78,   147,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,    78,    44,    -1,    -1,    -1,   148,    -1,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,    -1,    -1,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,
      -1,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    78,    44,    -1,    -1,
      -1,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    78,    -1,    -1,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,    78,    44,    -1,    -1,   147,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,    -1,    44,    -1,    -1,   147,    78,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,    -1,    -1,
      -1,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    78,    44,    -1,    -1,
     147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,    78,    44,    -1,    -1,   147,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    44,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,    -1,    44,    -1,    -1,   147,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    -1,    -1,    78,
      -1,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    78,    -1,    -1,    -1,
     147,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      78,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,    -1,    -1,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    17,    22,    25,    30,    35,    38,    40,    41,    42,
      46,    50,    64,    85,    94,    96,    99,   101,   103,   106,
     108,   118,   121,   154,   155,   158,   159,   160,   161,   168,
     211,   216,   217,   222,   223,   224,   225,   236,   237,   246,
     247,   248,   249,   250,   254,   255,   260,   261,   262,   265,
     267,   268,   269,   270,   271,   272,   273,   277,   278,   279,
     280,   281,   282,   296,   100,   112,    62,     3,    15,    16,
      17,    18,    19,    21,    22,    23,    24,    25,    26,    27,
      28,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    45,    46,    49,    50,    51,
      53,    54,    55,    56,    57,    58,    59,    62,    63,    64,
      65,    66,    67,    69,    70,    71,    72,    74,    75,    76,
      77,    79,    80,    81,    83,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    95,    96,    97,    98,    99,   100,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   156,   157,    53,    87,    52,
      53,    87,    23,    59,    91,   100,    62,    62,    88,     3,
       4,     5,     6,     8,     9,    10,    14,    18,    19,    24,
      27,    37,    45,    49,    56,    57,    60,    61,    63,    66,
      75,    77,    90,    97,   111,   125,   139,   140,   143,   146,
     149,   157,   169,   170,   172,   203,   204,   207,   274,   275,
     276,   283,   284,   287,   288,    10,    31,    54,    62,    81,
     105,   156,   263,    16,    31,    33,    39,    54,    62,    76,
      86,    88,    89,   105,   109,   112,   113,   114,   124,   213,
     214,   263,   116,   100,   157,   173,     0,    47,   145,   294,
      67,   157,   157,   157,   157,   146,   157,   157,   173,   157,
     157,   157,   157,   157,   157,    52,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   146,   146,   146,
     146,   146,     3,    75,    77,   203,   203,   203,    61,   157,
     208,   209,    13,    14,   151,   285,   286,    52,   148,    20,
     157,   171,    44,    78,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   146,
      73,   148,   195,   196,   171,    68,   132,   132,   103,     9,
     156,   157,     5,     6,     8,    82,   157,   220,   221,   132,
     116,     8,   109,   157,   103,    72,   212,   212,   212,   212,
      83,   197,   198,   157,   123,   157,   103,   148,   203,   291,
     292,   293,   101,   103,   106,   159,   157,   212,    93,    15,
      46,   115,     5,     6,     8,    14,   139,   146,   181,   182,
     232,   238,   239,    98,   119,   126,   175,   119,    52,   295,
     107,     8,   146,   147,   203,   205,   206,   203,   205,    43,
     140,   205,   205,   147,   203,   205,   205,   203,   203,   147,
     205,   203,   203,   147,   146,   146,   146,   147,   148,   150,
     132,     8,   203,   286,   146,   173,   170,   203,   157,   203,
     203,   203,   203,   203,   203,   203,   288,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,     5,   275,
      82,   143,   203,   220,   220,   132,   132,    54,   139,     8,
      48,    82,   117,   139,   157,   181,   218,   219,    69,   109,
     212,   109,     8,   157,   199,   200,    14,    32,   109,   215,
      72,   126,   256,   257,   157,   251,   252,   284,   157,   171,
      29,   148,   289,   290,   146,   226,    34,    34,   100,     5,
       6,     8,   147,   181,   183,   240,   148,   241,    27,    49,
      65,    66,   110,   266,     8,     4,    18,    19,    27,    37,
      45,    47,    56,    60,    66,    74,   125,   146,   157,   176,
     177,   178,   179,   180,   284,     8,     8,     8,   101,   161,
     147,   148,   147,   147,   157,   147,   147,   147,   147,   147,
     147,   147,   148,   147,   148,   147,   148,   147,   203,   203,
     209,   157,   181,   210,   152,   152,   165,   174,   175,    51,
     148,    82,   146,     5,   218,     9,   221,    71,   212,   212,
     132,   148,   104,     5,   104,     8,   157,   258,   259,   132,
     148,   175,   132,   291,    84,   191,   192,   293,    60,   157,
     227,   228,   122,   157,   157,   157,   147,   148,   147,   148,
     232,   239,   242,   243,   147,   107,   107,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   176,   129,    26,
      61,    68,   132,   133,   134,   135,   136,   137,   143,    26,
      61,   132,   133,   134,   135,   136,   137,   143,   147,   206,
     147,   203,   203,   203,   132,   101,   168,    55,   185,   157,
       5,   183,   132,    92,    95,   102,   264,     5,     8,   146,
     157,   200,   132,   128,   146,   181,   182,   252,   197,   181,
     182,    29,   195,   147,   148,   146,   229,   230,    27,    28,
      49,    65,    66,    70,    79,    80,   110,   253,   181,     8,
      20,   244,   148,     8,     8,   157,   157,   157,   284,   140,
     284,   147,   147,   284,     8,   147,   147,   176,   181,   182,
       9,   146,    82,   143,     8,   181,   182,     8,   181,   182,
     181,   182,   181,   182,   181,   182,   181,   182,    26,    61,
     181,   146,   181,   181,   181,   181,   181,   181,    26,    61,
     148,   162,   147,   147,   148,   210,   169,   147,     5,   186,
     127,   189,   190,    61,   147,   146,    36,   120,    92,   157,
     201,   202,   146,     8,   259,   147,   183,    90,   179,   193,
     194,   227,   146,   231,   232,    83,   148,   233,    73,   157,
     245,   232,   243,   147,   147,   147,   147,   147,   147,   147,
     129,   129,     8,   183,   184,    82,   181,     9,   146,   129,
     183,   181,   146,     5,    60,   157,   163,   164,   147,   146,
      52,    84,   166,    29,    55,    58,   188,   157,   283,   284,
     183,   132,   147,   148,     8,   147,   146,    21,    41,   148,
     147,   148,     3,   234,   235,   230,   181,   182,   181,   182,
     147,   147,   148,   129,   183,   184,   181,   147,   129,   183,
     148,   205,    29,    73,   167,   179,   187,    84,   178,   191,
     147,   147,   181,   202,   147,   147,   194,   232,   132,   148,
       8,   181,   147,   147,   181,   147,   164,   147,   193,     5,
     148,    29,   195,     8,   235,   148,   148,   179,   193,   197,
     146,     5,   205,   147,   147
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   153,   154,   154,   154,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   155,   155,   155,   155,   155,   155,
     155,   155,   155,   155,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     157,   157,   157,   158,   158,   158,   159,   159,   159,   160,
     160,   161,   161,   162,   162,   163,   163,   164,   164,   164,
     165,   166,   167,   167,   167,   168,   169,   169,   170,   170,
     171,   171,   171,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   173,   173,   174,   174,   175,   176,   176,   176,
     177,   177,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     180,   180,   181,   181,   182,   182,   182,   183,   183,   184,
     184,   185,   185,   186,   186,   187,   187,   188,   188,   189,
     189,   190,   191,   191,   192,   192,   193,   193,   194,   194,
     194,   195,   195,   196,   196,   197,   197,   198,   199,   199,
     200,   200,   200,   200,   200,   200,   201,   201,   202,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   205,   205,   206,   206,
     207,   207,   207,   208,   208,   209,   209,   210,   210,   211,
     212,   212,   213,   213,   213,   213,   213,   213,   213,   213,
     213,   213,   213,   213,   213,   214,   214,   215,   215,   216,
     216,   216,   216,   216,   216,   217,   217,   217,   217,   218,
     218,   219,   219,   219,   220,   220,   221,   221,   221,   221,
     221,   222,   222,   222,   223,   223,   224,   225,   225,   226,
     226,   227,   227,   228,   228,   229,   229,   230,   231,   231,
     232,   232,   232,   232,   232,   233,   233,   234,   234,   235,
     236,   237,   238,   238,   239,   239,   240,   240,   241,   241,
     242,   242,   243,   244,   244,   245,   245,   246,   247,   247,
     248,   249,   250,   251,   251,   252,   252,   252,   252,   252,
     252,   253,   253,   253,   253,   253,   253,   253,   253,   253,
     254,   254,   254,   255,   255,   256,   256,   257,   258,   258,
     259,   260,   261,   262,   263,   263,   263,   264,   264,   264,
     264,   265,   266,   266,   266,   266,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   283,   284,   285,   285,   286,
     286,   286,   286,   287,   287,   288,   289,   289,   290,   291,
     292,   293,   293,   294,   295,   295,   296
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
       1,     1,     1,     1,     3,     2,     1,     1,     1,     1,
       8,     1,     9,     0,     2,     1,     3,     1,     1,     1,
       0,     3,     0,     2,     4,    11,     1,     3,     1,     2,
       0,     1,     2,     1,     4,     4,     4,     4,     4,     4,
       3,     5,     1,     3,     0,     1,     2,     1,     3,     3,
       4,     1,     3,     3,     5,     6,     5,     6,     3,     4,
       5,     6,     3,     3,     3,     3,     3,     3,     5,     5,
       5,     3,     3,     3,     3,     3,     3,     3,     4,     3,
       3,     5,     6,     5,     6,     3,     3,     3,     3,     1,
       1,     4,     3,     3,     1,     1,     4,     4,     4,     3,
       4,     4,     1,     2,     1,     2,     1,     1,     3,     1,
       3,     0,     4,     0,     1,     1,     3,     0,     2,     0,
       1,     5,     0,     1,     3,     5,     1,     3,     1,     2,
       2,     0,     1,     2,     4,     0,     1,     2,     1,     3,
       1,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       3,     4,     4,     4,     4,     4,     4,     4,     3,     6,
       6,     3,     8,    14,     3,     4,     1,     3,     1,     1,
       1,     1,     1,     3,     5,     1,     1,     1,     1,     2,
       0,     2,     1,     2,     2,     3,     1,     1,     1,     2,
       4,     4,     3,     4,     4,     1,     1,     0,     2,     4,
       4,     4,     3,     4,     4,     7,     5,     5,     9,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     7,     1,     1,     0,
       3,     1,     1,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     1,     3,     2,     0,     2,     1,     3,     3,
       4,     6,     1,     3,     1,     3,     1,     3,     0,     2,
       1,     3,     3,     0,     1,     1,     1,     3,     1,     1,
       3,     3,     6,     1,     3,     3,     3,     5,     4,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       7,     6,     4,     4,     5,     0,     1,     2,     1,     3,
       3,     2,     3,     6,     0,     1,     1,     2,     2,     2,
       1,     7,     1,     1,     1,     1,     1,     3,     6,     3,
       3,     2,     2,     3,     1,     3,     2,     1,     2,     3,
       3,     7,     5,     5,     1,     1,     2,     1,     2,     1,
       1,     3,     3,     3,     3,     1,     0,     2,     1,     2,
       1,     1,     3,     5,     0,     2,     4
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

  case 133:

    { pParser->PushQuery(); }

    break;

  case 134:

    { pParser->PushQuery(); }

    break;

  case 135:

    { pParser->PushQuery(); }

    break;

  case 140:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, (yyvsp[-6]) );
		}

    break;

  case 142:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		}

    break;

  case 145:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), (yyvsp[0]) );
		}

    break;

  case 146:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), (yyvsp[0]) );
		}

    break;

  case 150:

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

  case 151:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, (yyvsp[0]) );
			pParser->m_pQuery->m_bHasOuter = true;
		}

    break;

  case 153:

    {
			pParser->m_pQuery->m_iOuterLimit = (yyvsp[0]).m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		}

    break;

  case 154:

    {
			pParser->m_pQuery->m_iOuterOffset = (yyvsp[-2]).m_iValue;
			pParser->m_pQuery->m_iOuterLimit = (yyvsp[0]).m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		}

    break;

  case 155:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, (yyvsp[-7]) );
		}

    break;

  case 158:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 161:

    { pParser->AliasLastItem ( &(yyvsp[0]) ); }

    break;

  case 162:

    { pParser->AliasLastItem ( &(yyvsp[0]) ); }

    break;

  case 163:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 164:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_AVG, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 165:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_MAX, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 166:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_MIN, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 167:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_SUM, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 168:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_CAT, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 169:

    { if ( !pParser->AddItem ( "count(*)", &(yyvsp[-3]), &(yyvsp[0]) ) ) YYERROR; }

    break;

  case 170:

    { if ( !pParser->AddItem ( "groupby()", &(yyvsp[-2]), &(yyvsp[0]) ) ) YYERROR; }

    break;

  case 171:

    { if ( !pParser->AddDistinct ( &(yyvsp[-1]), &(yyvsp[-4]), &(yyvsp[0]) ) ) YYERROR; }

    break;

  case 173:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 180:

    {
			if ( !pParser->SetMatch((yyvsp[-1])) )
				YYERROR;
		}

    break;

  case 182:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 183:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
			pFilter->m_bExclude = true;
		}

    break;

  case 184:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-4]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		}

    break;

  case 185:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-5]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_bExclude = true;
		}

    break;

  case 186:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-4]), (yyvsp[-1]), false ) )
				YYERROR;
		}

    break;

  case 187:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-5]), (yyvsp[-1]), true ) )
				YYERROR;
		}

    break;

  case 188:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 189:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-3]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 190:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 191:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-5]), (yyvsp[-3]).m_iValue, (yyvsp[-1]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 192:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 193:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 194:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 195:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 196:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 197:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true, true ) )
				YYERROR;
		}

    break;

  case 198:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 199:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 200:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 201:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, false ) )
				YYERROR;
		}

    break;

  case 202:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, false ) )
				YYERROR;
		}

    break;

  case 203:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, true ) )
				YYERROR;
		}

    break;

  case 204:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 205:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 206:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 207:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-2]), true ) )
				YYERROR;
		}

    break;

  case 208:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-3]), false ) )
				YYERROR;
		}

    break;

  case 209:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-2]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-2]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 210:

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

  case 211:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-4]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-4]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 212:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 213:

    {
			AddMvaRange ( pParser, (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue );
		}

    break;

  case 214:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_RANGE );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_iMinValue = (yyvsp[-2]).m_iValue;
			f->m_iMaxValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 215:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue-1 );
		}

    break;

  case 216:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue+1, INT64_MAX );
		}

    break;

  case 217:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue );
		}

    break;

  case 218:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue, INT64_MAX );
		}

    break;

  case 220:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		}

    break;

  case 221:

    {
			(yyval).m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 222:

    {
			(yyval).m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 223:

    {
			(yyval).m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 224:

    {
			(yyval).m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 230:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ANY; }

    break;

  case 231:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ALL; }

    break;

  case 232:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 233:

    {
			(yyval).m_iType = TOK_CONST_INT;
			if ( (uint64_t)(yyvsp[0]).m_iValue > (uint64_t)LLONG_MAX )
				(yyval).m_iValue = LLONG_MIN;
			else
				(yyval).m_iValue = -(yyvsp[0]).m_iValue;
		}

    break;

  case 234:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 235:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = -(yyvsp[0]).m_fValue; }

    break;

  case 236:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 237:

    {
			assert ( !(yyval).m_pValues.Ptr() );
			(yyval).m_pValues = new RefcountedVector_c<SphAttr_t> ();
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue ); 
		}

    break;

  case 238:

    {
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 239:

    {
			assert ( !(yyval).m_pValues.Ptr() );
			(yyval).m_pValues = new RefcountedVector_c<SphAttr_t> ();
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 240:

    {
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 244:

    {
			pParser->SetGroupbyLimit ( (yyvsp[0]).m_iValue );
		}

    break;

  case 245:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 246:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 248:

    {
			pParser->AddHaving();
		}

    break;

  case 251:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, (yyvsp[0]) );
		}

    break;

  case 254:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, (yyvsp[0]) );
		}

    break;

  case 255:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		}

    break;

  case 257:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 259:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 260:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 263:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 264:

    {
			pParser->m_pQuery->m_iOffset = (yyvsp[-2]).m_iValue;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 270:

    {
			if ( !pParser->AddOption ( (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 271:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 272:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 273:

    {
			if ( !pParser->AddOption ( (yyvsp[-4]), pParser->GetNamedVec ( (yyvsp[-1]).m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( (yyvsp[-1]).m_iValue );
		}

    break;

  case 274:

    {
			if ( !pParser->AddOption ( (yyvsp[-5]), (yyvsp[-3]), (yyvsp[-1]) ) )
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
			(yyval).m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 277:

    {
			pParser->AddConst( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 278:

    {
			(yyval) = (yyvsp[-2]);
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 280:

    { if ( !pParser->SetOldSyntax() ) YYERROR; }

    break;

  case 281:

    { if ( !pParser->SetNewSyntax() ) YYERROR; }

    break;

  case 286:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 287:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 288:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 289:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 290:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 291:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

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

  case 310:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 311:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 312:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 313:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 314:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

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

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 319:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 320:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 321:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 322:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-7]), (yyvsp[0]) ); }

    break;

  case 323:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-13]), (yyvsp[0]) ); }

    break;

  case 324:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 325:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 333:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 334:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-4]), (yyvsp[0]) ); }

    break;

  case 341:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) ); }

    break;

  case 342:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; }

    break;

  case 343:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; }

    break;

  case 344:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; }

    break;

  case 345:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; }

    break;

  case 346:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; }

    break;

  case 347:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; }

    break;

  case 348:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; }

    break;

  case 349:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; }

    break;

  case 350:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 351:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 352:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 353:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 354:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
			pParser->m_pStmt->m_iIntParam = int((yyvsp[-1]).m_fValue*10);
		}

    break;

  case 358:

    {
			pParser->m_pStmt->m_iIntParam = (yyvsp[0]).m_iValue;
		}

    break;

  case 359:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 360:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) );
		}

    break;

  case 361:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		}

    break;

  case 362:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 363:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 364:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 365:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
		}

    break;

  case 366:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) ).Unquote();
		}

    break;

  case 367:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 368:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-6]) );
		}

    break;

  case 371:

    { (yyval).m_iValue = 1; }

    break;

  case 372:

    { (yyval).m_iValue = 0; }

    break;

  case 373:

    {
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
			if ( (yyval).m_iValue!=0 && (yyval).m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		}

    break;

  case 381:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; }

    break;

  case 382:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; }

    break;

  case 383:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; }

    break;

  case 386:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-4]) );
		}

    break;

  case 387:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; }

    break;

  case 388:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; }

    break;

  case 393:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 394:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 397:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } }

    break;

  case 398:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 399:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 400:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 401:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 402:

    { (yyval).m_iType = TOK_QUOTED_STRING; (yyval).m_iStart = (yyvsp[0]).m_iStart; (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 403:

    { (yyval).m_iType = TOK_CONST_MVA; (yyval).m_pValues = (yyvsp[-1]).m_pValues; }

    break;

  case 404:

    { (yyval).m_iType = TOK_CONST_MVA; }

    break;

  case 409:

    { if ( !pParser->AddInsertOption ( (yyvsp[-2]), (yyvsp[0]) ) ) YYERROR; }

    break;

  case 410:

    {
		if ( !pParser->DeleteStatement ( &(yyvsp[-1]) ) )
			YYERROR;
	}

    break;

  case 411:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, (yyvsp[-4]) );
		}

    break;

  case 412:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 413:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 415:

    {
			(yyval).m_iType = TOK_CONST_STRINGS;
		}

    break;

  case 416:

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

  case 417:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), (yyvsp[0]) );
		}

    break;

  case 420:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		}

    break;

  case 422:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), (yyvsp[0]) );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, (yyvsp[-2]) );
		}

    break;

  case 427:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 430:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; }

    break;

  case 431:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; }

    break;

  case 432:

    {
			if ( !pParser->UpdateStatement ( &(yyvsp[-4]) ) )
				YYERROR;
		}

    break;

  case 435:

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

  case 436:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( (yyvsp[0]).m_fValue ) );
			pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_FLOAT );
		}

    break;

  case 437:

    {
			pParser->UpdateMVAAttr ( (yyvsp[-4]), (yyvsp[-1]) );
		}

    break;

  case 438:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( (yyvsp[-3]), tNoValues );
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

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 442:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 443:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 444:

    { (yyval).m_iValue = SPH_ATTR_BOOL; }

    break;

  case 445:

    { (yyval).m_iValue = SPH_ATTR_UINT32SET; }

    break;

  case 446:

    { (yyval).m_iValue = SPH_ATTR_INT64SET; }

    break;

  case 447:

    { (yyval).m_iValue = SPH_ATTR_JSON; }

    break;

  case 448:

    { (yyval).m_iValue = SPH_ATTR_STRING; }

    break;

  case 449:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 450:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-4]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[-1]) );
			tStmt.m_eAlterColType = (ESphAttr)(yyvsp[0]).m_iValue;
		}

    break;

  case 451:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-3]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[0]) );
		}

    break;

  case 452:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 453:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		}

    break;

  case 454:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 461:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		}

    break;

  case 462:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		}

    break;

  case 463:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		}

    break;

  case 471:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, (yyvsp[0]) );
			tStmt.m_eUdfType = (ESphAttr) (yyvsp[-2]).m_iValue;
		}

    break;

  case 472:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 473:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 474:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 475:

    { (yyval).m_iValue = SPH_ATTR_STRINGPTR; }

    break;

  case 476:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 477:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[0]) );
		}

    break;

  case 478:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-3]) );
			pParser->ToString ( tStmt.m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 479:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 480:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 481:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		}

    break;

  case 482:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_HOSTNAMES;
		}

    break;

  case 483:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[-1]) );
		}

    break;

  case 487:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 488:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[0]) );
		}

    break;

  case 489:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 490:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 491:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 492:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 493:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 496:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 498:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 499:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 500:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 501:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 502:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 503:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 504:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 508:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		}

    break;

  case 510:

    {
			pParser->AddItem ( &(yyvsp[0]) );
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 513:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
		}

    break;

  case 515:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 516:

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
