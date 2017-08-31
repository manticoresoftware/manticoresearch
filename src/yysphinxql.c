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
#define YYLAST   5465

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  154
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  146
/* YYNRULES -- Number of rules.  */
#define YYNRULES  524
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  929

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
     413,   414,   415,   419,   427,   428,   429,   430,   435,   442,
     450,   458,   467,   472,   477,   482,   487,   492,   497,   502,
     507,   512,   517,   522,   527,   532,   537,   542,   547,   552,
     557,   562,   567,   572,   577,   584,   590,   601,   608,   617,
     621,   630,   634,   638,   642,   649,   650,   655,   661,   667,
     673,   679,   680,   681,   682,   683,   687,   688,   692,   693,
     704,   705,   706,   710,   716,   723,   729,   735,   737,   740,
     742,   749,   753,   759,   761,   767,   769,   773,   784,   786,
     790,   794,   801,   802,   806,   807,   808,   811,   813,   817,
     822,   829,   831,   835,   839,   840,   844,   849,   854,   859,
     865,   870,   878,   883,   890,   900,   901,   902,   903,   904,
     905,   906,   907,   908,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   921,   922,   923,   924,   925,
     926,   927,   928,   929,   930,   931,   932,   933,   937,   938,
     939,   940,   941,   942,   943,   944,   945,   946,   947,   948,
     949,   950,   954,   955,   959,   960,   964,   965,   966,   970,
     971,   975,   976,   980,   981,   987,   990,   992,   996,   997,
     998,   999,  1000,  1001,  1002,  1003,  1004,  1009,  1014,  1019,
    1024,  1033,  1034,  1037,  1039,  1047,  1052,  1057,  1062,  1063,
    1064,  1068,  1073,  1078,  1083,  1092,  1093,  1097,  1098,  1099,
    1111,  1112,  1116,  1117,  1118,  1119,  1120,  1127,  1128,  1129,
    1133,  1134,  1140,  1148,  1149,  1152,  1154,  1158,  1159,  1163,
    1164,  1168,  1169,  1173,  1177,  1178,  1182,  1183,  1184,  1185,
    1186,  1189,  1190,  1194,  1195,  1199,  1205,  1215,  1223,  1227,
    1234,  1235,  1242,  1252,  1258,  1260,  1264,  1269,  1273,  1280,
    1282,  1286,  1287,  1293,  1301,  1302,  1308,  1312,  1318,  1326,
    1327,  1331,  1345,  1351,  1355,  1360,  1374,  1385,  1386,  1387,
    1388,  1389,  1390,  1391,  1392,  1393,  1397,  1405,  1412,  1423,
    1427,  1434,  1435,  1439,  1443,  1444,  1448,  1452,  1459,  1466,
    1472,  1473,  1474,  1478,  1479,  1480,  1481,  1487,  1498,  1499,
    1500,  1501,  1502,  1507,  1518,  1527,  1529,  1538,  1547,  1556,
    1564,  1574,  1582,  1583,  1587,  1591,  1595,  1605,  1616,  1627,
    1638,  1648,  1659,  1660,  1664,  1667,  1668,  1672,  1673,  1674,
    1675,  1679,  1680,  1684,  1689,  1691,  1695,  1704,  1708,  1716,
    1717,  1721,  1732,  1734,  1741
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

#define YYPACT_NINF -796

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-796)))

#define YYTABLE_NINF -514

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    5064,   122,    -3,  -796,  4454,  -796,    31,    29,  -796,  -796,
      53,   177,  -796,    38,   130,  -796,  -796,   898,  4580,   627,
      -5,    22,  4454,   114,  -796,   -31,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,    57,  -796,  -796,  -796,
    4454,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  4454,  4454,  4454,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,    11,  4454,  4454,
    4454,  4454,  4454,  -796,  -796,  4454,  4454,  4454,  4454,   143,
      61,  -796,  -796,  -796,  -796,  -796,  -796,  -796,    65,    81,
      86,    94,   101,   108,   116,   120,   123,  -796,   126,   128,
     137,   139,   154,   158,   179,   186,   198,  1934,  -796,  1934,
    1934,  3872,    34,   -23,  -796,  3986,    12,  -796,   207,   -42,
    -796,  3986,    71,   253,  -796,   191,   218,    95,  4706,  4454,
    3254,   240,   227,   257,  4112,   187,  -796,   305,  -796,  -796,
     305,  -796,  -796,  -796,  -796,   305,  -796,   305,   295,  -796,
    -796,  4454,   266,  -796,  4454,  -796,   -64,  -796,  1934,   173,
    -796,  4454,   305,   304,    63,   284,    35,   303,   300,   -18,
    -796,   310,  -796,  -796,   295,   397,   346,  1046,  1934,  2082,
     -16,  2082,  2082,   308,  1934,  2082,  2082,  1934,  1934,  1194,
    1934,  1934,   309,   311,   312,   313,  -796,  -796,  5004,  -796,
    -796,   -62,   328,  -796,  -796,  2230,    37,  -796,  2749,  1342,
    4454,  -796,  -796,  1934,  1934,  1934,  1934,  1934,  1934,  1934,
    1934,  1934,  1934,  1934,  1934,  1934,  1934,  1934,  1934,  1934,
    1934,   457,   453,  -796,  -796,  -796,   -25,  1934,  3254,  3254,
     332,   334,   415,  -796,  -796,  -796,  -796,  -796,   330,  -796,
    2876,   403,   379,     1,   381,  -796,   485,  -796,  -796,  -796,
    -796,  4454,  -796,  -796,    20,     4,  -796,  4454,  4454,  5256,
    -796,  3986,   -12,  1490,  4832,   286,  -796,   348,  -796,  -796,
     462,   466,   401,  -796,  -796,  -796,  -796,   234,    30,  -796,
    -796,  -796,   354,  -796,    67,   499,  2368,  -796,   513,  -796,
     515,  -796,   516,   392,  1638,  -796,  5256,    97,  -796,  5072,
     105,  4454,   378,   111,   113,  -796,  5109,   133,   159,   623,
     660,  -796,   167,   734,  5139,  -796,  1786,  1934,  1934,  -796,
    3872,  -796,  3128,   374,   344,  -796,  -796,   -18,  -796,  5256,
    -796,  -796,  -796,  5276,  5289,  5322,   376,   157,  -796,   157,
     241,   241,   241,   241,   188,   188,  -796,  -796,  -796,  5241,
     380,  -796,  -796,   446,   157,   330,   330,   383,  3002,   522,
    3254,  -796,  -796,  -796,  -796,   528,  -796,  -796,  -796,  -796,
     463,   305,  -796,   305,  -796,   405,   390,  -796,   436,   537,
    -796,   439,   536,  4454,  -796,  -796,    51,    10,  -796,   414,
    -796,  -796,  -796,  1934,   464,  1934,  4226,   427,  4454,  4454,
    4454,  -796,  -796,  -796,  -796,  -796,   169,   174,    35,   404,
    -796,  -796,  -796,  -796,  -796,   447,   449,  -796,   406,   410,
     412,   416,   418,   419,   420,  -796,   421,   422,   423,  2495,
      37,  -796,   432,   214,  -796,   176,   203,  -796,  -796,  -796,
    -796,  1342,   424,  -796,  2082,  -796,  -796,   426,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  1934,  -796,  1934,  -796,  1934,
    -796,  4836,  4904,   438,  -796,  -796,  -796,  -796,  -796,   474,
     521,  -796,  4454,   572,  -796,    41,  -796,  -796,   450,  -796,
     102,  -796,  -796,  2622,  4454,  -796,  -796,  -796,  -796,   451,
     456,  -796,    74,  4454,   295,    98,  -796,   550,   507,  -796,
     433,  -796,  -796,  -796,   201,   440,   471,  -796,   459,  -796,
      41,  -796,   580,     8,  -796,   442,  -796,  -796,   584,   585,
    4454,  4454,  4454,   454,  4454,   448,   458,  4454,   586,   460,
      76,  2495,  2495,  2368,    98,    14,   -10,    87,    91,    98,
      98,    98,    98,    56,    41,   468,    41,    41,    41,    41,
      41,    41,    60,   467,  -796,  -796,  5175,  5210,  4936,  3128,
    1342,   469,   593,   490,   558,  -796,   204,   473,    18,   532,
    -796,  -796,  -796,  -796,  4454,   478,  -796,   621,  4454,    39,
    -796,  -796,  -796,  -796,  -796,  -796,  3380,  -796,  -796,  4226,
      69,   -51,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,   514,  -796,  -796,  -796,  -796,  4340,    69,
    -796,  -796,   483,   486,    37,   487,   488,   492,  -796,  -796,
     493,   494,  -796,  2495,  -796,   229,   517,   518,  -796,   519,
     520,  -796,    50,  -796,   563,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,    41,
      16,   524,    41,  -796,  -796,  -796,  -796,  -796,  -796,    41,
     504,  3758,   508,  -796,  -796,   505,  -796,   -22,   571,  -796,
     628,   604,   603,  -796,  4454,  -796,    41,  -796,  -796,  -796,
     529,   217,  -796,   655,  -796,  -796,  -796,   220,   523,   172,
     525,  -796,  -796,    40,   222,  -796,   661,   440,  -796,  -796,
    -796,  -796,  -796,   648,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  2495,    98,    98,  -796,   238,   242,  -796,   541,
    -796,    50,    41,   245,   543,    41,  -796,  -796,  -796,   526,
    -796,  -796,  2082,  4454,   647,   605,  3506,   595,  3632,   464,
      37,   534,  -796,   255,    41,  -796,  4454,   538,  -796,   540,
    -796,  -796,  3506,  -796,    69,   544,   542,  -796,  -796,   229,
    -796,  -796,  -796,  -796,  -796,  -796,   682,    41,   263,   265,
    -796,  -796,    41,   267,  3758,   269,  3506,   688,  -796,  -796,
     545,   667,  -796,   507,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,   689,   661,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,   549,   525,   551,  3506,  3506,   295,  -796,  -796,   552,
     697,  -796,   525,  -796,  2082,  -796,   276,   557,  -796
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   390,     0,   387,     0,     0,   435,   434,
       0,     0,   393,     0,     0,   394,   388,     0,   470,   470,
       0,     0,     0,     0,     2,     3,   134,   137,   140,   142,
     138,   139,     7,     8,   389,     5,     0,     6,     9,    10,
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
       0,     0,     0,   489,   490,     0,     0,     0,     0,     0,
      34,   286,   288,   289,   513,   291,   495,   290,    37,    39,
      43,    46,    55,    62,    64,    71,    70,   287,     0,    75,
      78,    85,    87,    97,   100,   115,   127,     0,   159,     0,
       0,     0,   285,     0,   157,   161,   164,   313,     0,   267,
     492,   161,     0,   314,   315,     0,     0,    49,    68,    74,
       0,   107,     0,     0,     0,     0,   467,   346,   471,   361,
     346,   353,   354,   352,   472,   346,   362,   346,   271,   348,
     345,     0,     0,   391,     0,   173,     0,     1,     0,     4,
     136,     0,   346,     0,     0,     0,     0,     0,     0,     0,
     483,     0,   488,   487,   271,   522,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    34,    85,    87,   292,   293,     0,   342,
     341,     0,     0,   507,   508,     0,   504,   505,     0,     0,
       0,   162,   160,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   491,   268,   494,     0,     0,     0,     0,
       0,     0,     0,   385,   386,   384,   383,   382,   368,   380,
       0,     0,     0,   346,     0,   468,     0,   437,   350,   349,
     436,     0,   355,   272,   363,   461,   497,     0,     0,   518,
     519,   161,   514,     0,     0,     0,   135,   395,   433,   458,
       0,     0,     0,   238,   240,   408,   242,     0,     0,   406,
     407,   420,   424,   418,     0,     0,     0,   416,     0,   498,
       0,   524,     0,   335,     0,   324,   334,     0,   332,     0,
       0,     0,     0,     0,     0,   171,     0,     0,     0,     0,
       0,   330,     0,     0,     0,   327,     0,     0,     0,   311,
       0,   312,     0,   513,     0,   506,   151,   175,   158,   164,
     163,   303,   304,   310,   309,   301,   300,   307,   511,   308,
     298,   299,   305,   306,   294,   295,   296,   297,   302,     0,
     269,   493,   316,     0,   512,   369,   370,     0,     0,     0,
       0,   376,   378,   367,   377,     0,   375,   379,   366,   365,
       0,   346,   351,   346,   347,   276,   273,   274,     0,     0,
     358,     0,     0,     0,   459,   462,     0,     0,   439,     0,
     174,   517,   516,     0,   258,     0,     0,     0,     0,     0,
       0,   239,   241,   422,   410,   243,     0,     0,     0,     0,
     479,   480,   478,   482,   481,     0,     0,   226,    37,    39,
      46,    55,    62,     0,    71,   230,    78,    84,   127,     0,
     225,   177,   178,   179,   184,     0,     0,   231,   500,   523,
     501,     0,     0,   318,     0,   165,   321,     0,   170,   323,
     322,   169,   319,   320,   166,     0,   167,     0,   331,     0,
     168,     0,     0,     0,   344,   343,   339,   510,   509,     0,
     247,   176,     0,     0,   317,     0,   373,   372,     0,   381,
       0,   356,   357,     0,     0,   360,   364,   359,   460,     0,
     463,   464,     0,     0,   271,     0,   520,     0,   267,   259,
     515,   398,   397,   399,     0,     0,     0,   457,   485,   409,
       0,   421,     0,   429,   419,   425,   426,   417,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   144,   333,   172,     0,     0,     0,     0,
       0,     0,   249,   255,     0,   270,     0,     0,     0,     0,
     476,   469,   278,   281,     0,   277,   275,     0,     0,     0,
     441,   442,   440,   438,   445,   446,     0,   521,   396,     0,
       0,   411,   401,   448,   450,   449,   455,   447,   453,   451,
     452,   454,   456,     0,   484,   244,   423,   430,     0,     0,
     477,   499,     0,     0,     0,     0,     0,     0,   235,   228,
       0,     0,   229,     0,   187,   180,   186,   181,   185,     0,
       0,   194,     0,   213,     0,   211,   188,   202,   212,   189,
     203,   199,   208,   198,   207,   201,   210,   200,   209,     0,
       0,     0,     0,   215,   216,   221,   222,   223,   224,     0,
       0,     0,     0,   326,   325,     0,   340,     0,     0,   250,
       0,     0,   253,   256,     0,   371,     0,   474,   473,   475,
       0,     0,   282,     0,   466,   465,   444,     0,    97,   264,
     260,   262,   400,     0,     0,   404,     0,     0,   392,   486,
     432,   431,   428,   429,   427,   237,   236,   234,   227,   233,
     232,   183,     0,     0,     0,   245,     0,     0,   214,     0,
     195,     0,     0,     0,     0,     0,   149,   150,   148,   145,
     146,   141,     0,     0,     0,   153,     0,     0,     0,   258,
     503,     0,   502,     0,     0,   279,     0,     0,   443,     0,
     265,   266,     0,   403,     0,     0,   412,   413,   402,   182,
     196,   205,   206,   204,   190,   192,     0,     0,     0,     0,
     219,   217,     0,     0,     0,     0,     0,     0,   143,   251,
     248,     0,   254,   267,   328,   374,   284,   283,   280,   261,
     263,   405,     0,     0,   246,   197,   191,   193,   220,   218,
     147,     0,   152,   154,     0,     0,   271,   415,   414,     0,
       0,   252,   257,   156,     0,   155,     0,     0,   329
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -796,  -796,  -796,   -13,    -4,  -796,   452,  -796,   302,  -796,
    -796,  -177,  -796,  -796,  -796,   129,    47,   400,  -211,  -796,
      -8,  -796,  -384,  -796,    75,  -370,  -138,  -663,  -796,   -47,
    -581,  -566,  -112,  -796,  -796,  -796,  -796,  -796,  -796,  -129,
    -796,  -795,  -141,  -602,  -796,  -272,  -796,  -796,   131,  -796,
    -133,   118,  -796,  -278,   170,  -796,  -796,   296,    58,  -796,
    -225,  -796,  -796,  -796,  -796,  -796,   260,  -796,   112,   261,
    -796,  -796,  -796,  -796,  -796,    36,  -796,  -796,   -77,  -796,
    -509,  -796,  -796,  -170,  -796,  -796,  -796,   216,  -796,  -796,
    -796,    23,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
     134,  -796,  -796,  -796,  -796,  -796,  -796,    55,  -796,  -796,
    -796,   725,  -796,  -796,  -796,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,  -796,  -796,   413,  -796,  -796,  -796,  -796,  -796,
    -796,  -796,   -38,  -354,  -796,   441,  -796,   429,  -796,  -796,
     246,  -796,   264,  -796,  -796,  -796
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    23,    24,   166,   212,    25,    26,    27,    28,   772,
     839,   840,   579,   845,   888,    29,   213,   214,   312,   215,
     437,   580,   397,   541,   542,   738,   544,   545,   546,   515,
     390,   516,   827,   673,   780,   890,   849,   782,   783,   608,
     609,   800,   801,   333,   334,   362,   363,   486,   487,   791,
     792,   406,   217,   407,   408,   218,   301,   302,   576,    30,
     357,   250,   251,   491,    31,    32,   478,   479,   348,   349,
      33,    34,    35,    36,   507,   613,   614,   701,   702,   804,
     391,   808,   866,   867,    37,    38,   392,   393,   517,   519,
     625,   626,   718,   812,    39,    40,    41,    42,    43,   497,
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
     167,   410,   399,   413,   414,   232,   697,   417,   418,   623,
     335,   422,  -496,   499,   256,   358,   258,   502,   255,   676,
     359,   691,   360,   741,   695,   830,   543,   411,   717,   308,
     843,   331,   806,   799,   488,   383,   262,   378,   513,   367,
     383,   384,   547,   385,   383,   383,   383,   303,   304,   386,
     303,   304,   489,   581,   787,   383,   313,   462,   825,    66,
     263,   264,   265,   740,   303,   304,   747,   750,   752,   754,
     756,   758,   743,   356,   383,   384,   492,   385,   380,   383,
     384,   170,   759,   386,   168,   368,   769,   430,   386,   431,
     314,   912,   383,   384,   520,   745,   383,   384,   807,   748,
     177,   386,  -503,   383,   384,   386,   171,   332,   396,   381,
    -113,   253,   386,   604,   257,   259,   521,   760,   169,   463,
     922,   770,   254,   797,   261,   412,   309,   309,   482,   490,
     493,   368,   522,   523,   744,   216,   396,   503,   788,   336,
     172,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,  -420,  -420,   266,   603,
     501,   742,   269,   831,   267,   268,   255,   270,   271,   640,
     475,   272,   273,   274,   275,   387,   826,   524,   514,   475,
     475,   475,   388,   889,   602,   547,   305,   796,   514,   305,
     475,   805,   178,   860,   678,   276,   833,   679,   339,   799,
     173,   313,   644,   305,   680,   642,   733,   300,   277,   387,
     813,   311,  -337,   861,   387,   341,   803,   311,   179,   389,
     853,   689,    64,   799,   734,   342,   347,   387,  -336,   654,
     354,   387,   313,   278,    65,   314,   174,   645,   387,   511,
     512,   279,   871,   873,   646,   553,   554,   364,   280,   499,
     366,   921,   799,   556,   554,   281,   591,   377,   592,   559,
     554,   560,   554,   282,   655,   878,   314,   283,   175,   883,
     284,   735,   736,   285,   373,  -338,   374,   176,   725,   375,
     727,   562,   554,   730,   286,   313,   287,   547,   547,   547,
     355,   916,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   288,   234,   477,   255,   289,   440,   563,   554,   647,
     648,   649,   650,   651,   652,   568,   554,   619,   620,   314,
     653,  -502,   621,   622,   337,   296,   290,   297,   298,   327,
     328,   329,   693,   291,   347,   347,   656,   657,   658,   659,
     660,   661,   547,   642,   643,   292,   476,   662,   239,   698,
     699,   338,   785,   620,   330,   901,  -472,   485,   642,   733,
     350,   232,   240,   496,   500,   855,   856,   311,   858,   620,
     863,   864,   241,   351,   242,   243,   369,   356,   361,   547,
     325,   326,   327,   328,   329,   575,   874,   620,   313,   365,
     875,   876,   540,   881,   620,   245,   409,   379,   246,   382,
     248,   394,   416,   895,   620,   419,   420,   557,   423,   424,
     249,   906,   620,   907,   876,   909,   620,   911,   554,   395,
     313,   586,   314,   434,   927,   554,   300,   439,   574,   398,
     852,   441,   442,   443,   444,   445,   446,   447,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   400,
     465,   466,   869,   402,   314,   464,   415,   425,   426,   427,
     428,   432,   460,   186,   476,   467,   347,   468,   547,   469,
     470,   389,   480,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   481,   599,
     483,   439,   547,   484,   547,   506,   508,   578,   703,   704,
     509,   510,   612,   518,   616,   617,   618,   526,   547,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     705,   548,   298,   549,   550,  -513,   558,   577,   584,   583,
     585,   588,   547,   511,   590,   540,   706,   707,   593,   594,
     595,   708,   596,   597,   598,   571,   572,   605,   607,   615,
     709,   710,   627,   630,   628,   690,   629,   631,   694,   632,
     547,   547,   641,   633,   885,   634,   635,   636,   637,   638,
     639,   669,   663,   715,   665,   670,   672,   675,   674,   696,
     331,   711,   503,   677,   687,   688,   713,   700,   716,   685,
     485,   719,   720,   721,   731,   726,   728,   739,   779,   496,
     746,   749,   751,   753,   755,   757,   729,   761,   732,   763,
     764,   765,   766,   767,   768,   762,   771,   778,   781,   784,
     786,   369,   575,   369,   789,   793,   722,   723,   724,   794,
     724,   815,   809,   724,   816,   817,   818,   540,   540,   540,
     819,   820,   821,   234,   923,   828,   926,   733,   822,   823,
     824,   835,   842,   389,   832,   844,   841,   846,   235,   847,
     236,   848,   854,   857,   865,   574,   237,   313,   717,   439,
     859,   877,   389,   882,   862,   884,   886,   902,   887,   891,
     790,   238,   894,   666,   599,   667,   898,   668,   899,   239,
     904,   903,   540,   913,   914,   612,   915,   917,   919,   924,
     920,   314,   925,   240,   313,   928,   552,   910,   671,   438,
     892,   376,   829,   241,   811,   242,   243,   777,   737,   879,
     893,   900,   834,   897,   664,   686,   573,   776,   587,   540,
     868,   589,   244,   918,   624,   802,   245,   692,   314,   246,
     247,   248,   814,   795,   252,   461,   851,   435,   448,   606,
       0,   249,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   838,     0,   610,
       0,   564,   565,     0,     0,     0,   870,   872,   313,     0,
     850,     0,     0,     0,     0,   880,     0,     0,   439,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,     0,     0,     0,   896,   566,   567,
       0,     0,   314,     0,     0,     0,     0,   389,   540,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     905,     0,     0,     0,     0,   908,     0,     0,     0,   255,
       0,     0,   540,     0,   540,     0,     0,     0,     0,     0,
       0,     0,   790,     0,     0,     0,     0,     0,   540,     0,
       0,     0,     0,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,     0,     0,
     838,     0,   540,   569,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   180,   181,   182,   183,     0,   184,   185,   186,     0,
     540,   540,   187,    68,    69,    70,   188,   189,     0,    73,
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
       0,     0,   209,     0,     0,   210,     0,     0,   211,   293,
     181,   182,   183,     0,   403,   185,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,   207,     0,     0,     0,
     209,     0,     0,   404,   405,     0,   211,   293,   181,   182,
     183,     0,   403,   185,     0,     0,     0,     0,   187,    68,
      69,    70,   188,   189,     0,    73,    74,    75,    76,    77,
      78,   191,    80,     0,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,   193,
      96,     0,     0,   194,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   197,   198,   107,   199,   109,   110,
     200,   112,     0,   113,   114,   115,   116,     0,   117,   294,
     119,   295,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   203,   131,   132,   133,     0,   134,
     135,   204,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   206,
     163,   164,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   207,     0,     0,     0,   209,     0,
       0,   210,   421,     0,   211,   293,   181,   182,   183,     0,
     184,   185,     0,     0,     0,     0,   187,    68,    69,    70,
     188,   189,     0,    73,    74,    75,   190,    77,    78,   191,
      80,     0,    81,    82,    83,    84,    85,    86,    87,   192,
      89,    90,    91,    92,    93,    94,     0,   193,    96,     0,
       0,   194,    98,    99,     0,   100,   101,   102,   195,   196,
     105,   106,   197,   198,   107,   199,   109,   110,   200,   112,
       0,   113,   114,   115,   116,     0,   117,   201,   119,   202,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   203,   131,   132,   133,     0,   134,   135,   204,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   205,   150,   151,   152,   153,   154,     0,
     155,   156,   157,   158,   159,   160,   161,   206,   163,   164,
     165,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   207,   208,     0,     0,   209,     0,     0,   210,
       0,     0,   211,   180,   181,   182,   183,     0,   184,   185,
       0,     0,     0,     0,   187,    68,    69,    70,   188,   189,
       0,    73,    74,    75,   190,    77,    78,   191,    80,     0,
      81,    82,    83,    84,    85,    86,    87,   192,    89,    90,
      91,    92,    93,    94,     0,   193,    96,     0,     0,   194,
      98,    99,     0,   100,   101,   102,   195,   196,   105,   106,
     197,   198,   107,   199,   109,   110,   200,   112,     0,   113,
     114,   115,   116,     0,   117,   201,   119,   202,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     203,   131,   132,   133,     0,   134,   135,   204,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   205,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   206,   163,   164,   165,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     207,   208,     0,     0,   209,     0,     0,   210,     0,     0,
     211,   293,   181,   182,   183,     0,   184,   185,     0,     0,
       0,     0,   187,    68,    69,    70,   188,   189,     0,    73,
      74,    75,    76,    77,    78,   191,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,   193,    96,     0,     0,   194,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   197,   198,
     107,   199,   109,   110,   200,   112,     0,   113,   114,   115,
     116,     0,   117,   294,   119,   295,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   203,   131,
     132,   133,     0,   134,   135,   204,   137,   138,   139,   551,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   206,   163,   164,   165,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   207,     0,
       0,     0,   209,     0,     0,   210,     0,     0,   211,   293,
     181,   182,   183,     0,   403,   185,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,   207,     0,     0,     0,
     209,     0,     0,   210,   405,     0,   211,   293,   181,   182,
     183,     0,   184,   185,     0,     0,     0,     0,   187,    68,
      69,    70,   188,   189,     0,    73,    74,    75,    76,    77,
      78,   191,    80,     0,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,   193,
      96,     0,     0,   194,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,   197,   198,   107,   199,   109,   110,
     200,   112,     0,   113,   114,   115,   116,     0,   117,   294,
     119,   295,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   203,   131,   132,   133,     0,   134,
     135,   204,   137,   138,   139,     0,   140,   141,   142,   143,
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
       0,     0,   211,   293,   181,   182,   183,     0,   433,   185,
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
     207,    67,   527,     0,   209,     0,     0,   210,     0,     0,
     211,     0,     0,    68,    69,    70,   528,   529,     0,    73,
      74,    75,    76,    77,    78,   530,    80,     0,    81,    82,
      83,    84,    85,    86,    87,   531,    89,    90,    91,    92,
      93,    94,     0,   532,    96,   533,     0,    97,    98,    99,
       0,   100,   101,   102,   534,   104,   105,   106,   535,     0,
     107,   108,   109,   110,   536,   112,     0,   113,   114,   115,
     116,     0,   537,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   538,   163,   164,   165,     0,    67,   527,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      68,    69,    70,   528,   529,   539,    73,    74,    75,    76,
      77,    78,   530,    80,     0,    81,    82,    83,    84,    85,
      86,    87,   531,    89,    90,    91,    92,    93,    94,     0,
     532,    96,   533,     0,    97,    98,    99,     0,   100,   101,
     102,   534,   104,   105,   106,   535,     0,   107,   108,   109,
     110,   536,   112,     0,   113,   114,   115,   116,     0,   117,
     118,   119,   120,     0,   121,   122,   123,     0,   124,     0,
     125,   126,   127,   128,   129,   130,   131,   132,   133,     0,
     134,   135,   136,   137,   138,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,   158,   159,   160,   161,
     538,   163,   164,   165,     0,    67,     0,   682,     0,     0,
     683,     0,     0,     0,     0,     0,     0,    68,    69,    70,
      71,    72,   539,    73,    74,    75,    76,    77,    78,    79,
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
     165,     0,    67,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    68,    69,    70,    71,    72,   684,
      73,    74,    75,    76,    77,    78,    79,    80,     0,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,     0,    95,    96,     0,     0,    97,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,     0,
       0,   107,   108,   109,   110,   111,   112,     0,   113,   114,
     115,   116,     0,   117,   118,   119,   120,     0,   121,   122,
     123,     0,   124,     0,   125,   126,   127,   128,   129,   130,
     131,   132,   133,     0,   134,   135,   136,   137,   138,   139,
       0,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,     0,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,     0,    67,
       0,   383,     0,     0,   471,     0,     0,     0,     0,     0,
       0,    68,    69,    70,    71,    72,   436,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,   472,    97,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,     0,     0,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   123,   473,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   474,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,    67,     0,   383,     0,     0,
     471,     0,     0,     0,     0,     0,   475,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
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
     165,    67,     0,   383,     0,     0,     0,     0,     0,     0,
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
     159,   160,   161,   162,   163,   164,   165,    67,     0,   343,
     344,     0,   345,     0,     0,     0,     0,     0,   475,    68,
      69,    70,    71,    72,     0,    73,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,    95,
      96,     0,     0,    97,    98,    99,     0,   100,   101,   102,
     103,   104,   105,   106,     0,     0,   107,   108,   109,   110,
     111,   112,     0,   113,   114,   115,   116,     0,   117,   118,
     119,   120,     0,   121,   122,   123,   346,   124,     0,   125,
     126,   127,   128,   129,   130,   131,   132,   133,     0,   134,
     135,   136,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    67,   527,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    68,    69,    70,    71,    72,
       0,    73,    74,    75,    76,    77,    78,   530,    80,     0,
      81,    82,    83,    84,    85,    86,    87,   531,    89,    90,
      91,    92,    93,    94,     0,   532,    96,   533,     0,    97,
      98,    99,     0,   100,   101,   102,   534,   104,   105,   106,
     535,     0,   107,   108,   109,   110,   536,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     798,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   538,   163,   164,   165,    67,
     527,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,   530,    80,     0,    81,    82,    83,    84,
      85,    86,    87,   531,    89,    90,    91,    92,    93,    94,
       0,   532,    96,   533,     0,    97,    98,    99,     0,   100,
     101,   102,   534,   104,   105,   106,   535,     0,   107,   108,
     109,   110,   536,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,   155,   156,   157,   158,   159,   160,
     161,   538,   163,   164,   165,    67,   527,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    68,    69,    70,
     528,   529,     0,    73,    74,    75,    76,    77,    78,   530,
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
     165,    67,     0,   836,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,    79,    80,     0,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,    96,     0,     0,    97,    98,    99,
       0,   100,   101,   102,   103,   104,   105,   106,   837,     0,
     107,   108,   109,   110,   111,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,    67,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,   165,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,   299,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,    67,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    68,    69,    70,    71,    72,   310,    73,    74,    75,
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
     161,   162,   163,   164,   165,    67,     0,     0,     0,     0,
     352,     0,     0,     0,     0,     0,     0,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,     0,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,     0,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   353,   148,   149,   150,   151,   152,   153,   154,    67,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,   611,     0,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
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
     114,   115,   116,   810,   117,   118,   119,   120,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,    67,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,    68,
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
     163,   164,   165,    67,     0,     0,     0,     0,     0,     0,
     226,     0,     0,     0,     0,    68,    69,    70,    71,    72,
       0,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,   227,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,   228,   102,   103,   104,   105,   106,
       0,     0,   229,   108,   109,   110,   111,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
     122,   230,     0,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   231,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,     0,     0,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,    67,
       0,     0,     0,     0,     0,   340,     0,     0,     0,     0,
       0,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,     0,     0,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,     0,     0,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,  -471,     0,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,    67,     0,     0,     0,     0,
       0,     0,   226,     0,     0,     0,     0,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,   227,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
     313,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,     0,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   230,   314,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   313,     0,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,     0,     0,     0,     0,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     313,     0,   314,     0,     0,   565,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   313,     0,
       0,     0,     0,   567,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
       0,     1,   314,     0,     0,   775,     2,     0,     0,     3,
       0,     0,     0,     0,     4,     0,     0,     0,     0,     5,
       0,     0,     6,     0,     7,     8,     9,     0,     0,     0,
      10,     0,     0,     0,    11,     0,   313,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,     0,
       0,     0,     0,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,     0,    13,
     314,     0,   429,   313,     0,     0,     0,     0,    14,     0,
      15,     0,     0,    16,     0,    17,     0,    18,     0,     0,
      19,     0,    20,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    21,   313,     0,    22,     0,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,     0,   314,     0,   313,
     555,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   314,   313,     0,     0,   561,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,     0,     0,   313,     0,   570,   314,     0,
       0,     0,   582,     0,     0,     0,     0,     0,     0,     0,
     313,     0,     0,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   314,
     313,     0,     0,   773,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   313,   314,     0,     0,     0,     0,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   314,     0,     0,     0,   774,     0,
       0,     0,     0,     0,     0,     0,   313,   314,     0,     0,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     314,     0,     0,     0,     0,     0,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329
};

static const yytype_int16 yycheck[] =
{
       4,   279,   274,   281,   282,    18,   608,   285,   286,   518,
     221,   289,     0,   367,    22,   240,    47,    29,    22,   585,
     245,   602,   247,     9,   605,     9,   396,    43,    20,    52,
      52,    73,    83,   696,    14,     5,    40,   262,     8,   103,
       5,     6,   396,     8,     5,     5,     5,    13,    14,    14,
      13,    14,    32,   437,    36,     5,    44,    82,     8,    62,
      64,    65,    66,   644,    13,    14,   647,   648,   649,   650,
     651,   652,    82,    72,     5,     6,    72,     8,    15,     5,
       6,    52,    26,    14,    53,   149,    26,   149,    14,   151,
      78,   886,     5,     6,    27,     8,     5,     6,   149,     8,
      62,    14,    68,     5,     6,    14,    53,   149,   126,    46,
     109,   116,    14,   497,     0,   146,    49,    61,    87,   144,
     915,    61,   100,   689,    67,   141,   149,   149,   353,   109,
     126,   149,    65,    66,   144,    17,   126,   149,   120,    68,
      87,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   148,   149,   147,   149,
     371,   147,   170,   147,   168,   169,   170,   171,   172,   539,
     140,   175,   176,   177,   178,   140,   742,   110,   148,   140,
     140,   140,   147,   846,   133,   539,   152,   148,   148,   152,
     140,   700,    62,    21,    92,    52,   762,    95,   103,   862,
      23,    44,    26,   152,   102,   129,   130,   211,   147,   140,
     719,   215,   147,    41,   140,   228,   147,   221,    88,   266,
     786,   147,   100,   886,   148,   229,   230,   140,   147,    26,
     234,   140,    44,   147,   112,    78,    59,    61,   140,     5,
       6,   147,   823,   824,    68,   148,   149,   251,   147,   603,
     254,   914,   915,   148,   149,   147,   481,   261,   483,   148,
     149,   148,   149,   147,    61,   831,    78,   147,    91,   835,
     147,   641,   642,   147,   101,   147,   103,   100,   632,   106,
     634,   148,   149,   637,   147,    44,   147,   641,   642,   643,
     103,   893,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   147,    16,   350,   308,   147,   310,   148,   149,   133,
     134,   135,   136,   137,   138,   148,   149,   148,   149,    78,
     144,    68,   148,   149,   133,   207,   147,   209,   210,   141,
     142,   143,   604,   147,   338,   339,   133,   134,   135,   136,
     137,   138,   696,   129,   130,   147,   350,   144,    62,   148,
     149,   133,   148,   149,   147,   864,   116,   361,   129,   130,
     133,   374,    76,   367,   368,   148,   149,   371,   148,   149,
     148,   149,    86,   116,    88,    89,   258,    72,    83,   733,
     139,   140,   141,   142,   143,   432,   148,   149,    44,   123,
     148,   149,   396,   148,   149,   109,   278,    93,   112,   115,
     114,    98,   284,   148,   149,   287,   288,   411,   290,   291,
     124,   148,   149,   148,   149,   148,   149,   148,   149,   119,
      44,   468,    78,   305,   148,   149,   430,   309,   432,   119,
     784,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,    52,
     338,   339,   822,   107,    78,   337,   148,   148,   147,   147,
     147,   133,     5,    10,   468,   133,   470,   133,   822,    54,
     140,   518,    69,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   109,   493,
     109,   373,   846,     8,   848,   147,    34,   153,    27,    28,
      34,   100,   506,   149,   508,   509,   510,     8,   862,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      49,     8,   404,     8,     8,   133,   148,   153,    82,   149,
     147,     9,   886,     5,    71,   539,    65,    66,   133,   149,
     104,    70,     5,   104,     8,   427,   428,   133,    84,   122,
      79,    80,   148,   147,   107,   602,   107,   147,   605,   147,
     914,   915,   130,   147,   842,   147,   147,   147,   147,   147,
     147,   133,   148,   620,   148,   101,    55,     5,   582,    29,
      73,   110,   149,   133,   133,   129,   127,   147,     8,   593,
     594,   149,     8,     8,     8,   141,   148,   644,     5,   603,
     647,   648,   649,   650,   651,   652,   148,   654,   148,   656,
     657,   658,   659,   660,   661,   147,   149,   148,   128,    61,
     147,   503,   669,   505,    92,   147,   630,   631,   632,     8,
     634,   148,   118,   637,   148,   148,   148,   641,   642,   643,
     148,   148,   148,    16,   916,    82,   924,   130,   130,   130,
     130,   147,   147,   700,   130,    84,   148,    29,    31,    55,
      33,    58,   133,     8,     3,   669,    39,    44,    20,   551,
     147,   130,   719,   130,   149,   149,    29,   133,    73,    84,
     684,    54,   148,   565,   688,   567,   148,   569,   148,    62,
       8,   149,   696,     5,   149,   699,    29,     8,   149,   147,
     149,    78,     5,    76,    44,   148,   404,   884,   579,   309,
     848,   259,   759,    86,   718,    88,    89,   670,   643,   831,
     849,   862,   769,   856,   554,   594,   430,   669,   468,   733,
     807,   470,   105,   903,   518,   699,   109,   603,    78,   112,
     113,   114,   719,   688,    19,   332,   784,   306,   319,   503,
      -1,   124,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   771,    -1,   505,
      -1,   148,   149,    -1,    -1,    -1,   823,   824,    44,    -1,
     784,    -1,    -1,    -1,    -1,   832,    -1,    -1,   670,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    -1,    -1,    -1,   854,   148,   149,
      -1,    -1,    78,    -1,    -1,    -1,    -1,   864,   822,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     877,    -1,    -1,    -1,    -1,   882,    -1,    -1,    -1,   843,
      -1,    -1,   846,    -1,   848,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   856,    -1,    -1,    -1,    -1,    -1,   862,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    -1,    -1,
     884,    -1,   886,   149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,     8,     9,    10,    -1,
     914,   915,    14,    15,    16,    17,    18,    19,    -1,    21,
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
      -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,
     144,    -1,    -1,   147,   148,    -1,   150,     3,     4,     5,
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
     126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,   144,    -1,
      -1,   147,   148,    -1,   150,     3,     4,     5,     6,    -1,
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
      -1,    -1,   140,   141,    -1,    -1,   144,    -1,    -1,   147,
      -1,    -1,   150,     3,     4,     5,     6,    -1,     8,     9,
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
     140,   141,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,
     150,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    15,    16,    17,    18,    19,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    -1,    45,    46,    -1,    -1,    49,    50,    51,
      -1,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    74,    75,    76,    77,    -1,    79,    80,    81,
      -1,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    -1,    95,    96,    97,    98,    99,   100,   101,
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
      -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,
     144,    -1,    -1,   147,   148,    -1,   150,     3,     4,     5,
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
      -1,    -1,   150,     3,     4,     5,     6,    -1,     8,     9,
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
     140,     3,     4,    -1,   144,    -1,    -1,   147,    -1,    -1,
     150,    -1,    -1,    15,    16,    17,    18,    19,    -1,    21,
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
     122,   123,   124,   125,   126,   127,   128,    -1,     3,     4,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    16,    17,    18,    19,   147,    21,    22,    23,    24,
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
     125,   126,   127,   128,    -1,     3,    -1,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,   147,    21,    22,    23,    24,    25,    26,    27,
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
     128,    -1,     3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,   147,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    -1,
      -1,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    74,    75,    76,    77,    -1,    79,    80,
      81,    -1,    83,    -1,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    99,   100,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,     3,
      -1,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    16,    17,    18,    19,   147,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    48,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    82,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,     3,    -1,     5,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,   140,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
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
     128,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
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
       6,    -1,     8,    -1,    -1,    -1,    -1,    -1,   140,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    -1,    45,
      46,    -1,    -1,    49,    50,    51,    -1,    53,    54,    55,
      56,    57,    58,    59,    -1,    -1,    62,    63,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    74,    75,
      76,    77,    -1,    79,    80,    81,    82,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,     3,     4,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    47,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    -1,    83,    -1,    85,    86,    87,    88,    89,
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
     128,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
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
     122,   123,   124,   125,   126,   127,   128,    15,    16,    17,
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
     128,    15,    16,    17,    18,    19,    20,    21,    22,    23,
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
     124,   125,   126,   127,   128,     3,    -1,    -1,    -1,    -1,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
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
      54,    55,    56,    57,    58,    59,    60,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
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
      70,    71,    72,    73,    74,    75,    76,    77,    -1,    79,
      80,    81,    -1,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,     3,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,    15,
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
     126,   127,   128,     3,    -1,    -1,    -1,    -1,    -1,    -1,
      10,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    -1,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,    -1,    -1,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,     3,
      -1,    -1,    -1,    -1,    -1,     9,    -1,    -1,    -1,    -1,
      -1,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    -1,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      44,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    78,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    -1,    95,    96,    97,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,    44,    -1,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,    -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      44,    -1,    78,    -1,    -1,   149,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    44,    -1,
      -1,    -1,    -1,   149,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      -1,    17,    78,    -1,    -1,   149,    22,    -1,    -1,    25,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    -1,    35,
      -1,    -1,    38,    -1,    40,    41,    42,    -1,    -1,    -1,
      46,    -1,    -1,    -1,    50,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    -1,    85,
      78,    -1,   148,    44,    -1,    -1,    -1,    -1,    94,    -1,
      96,    -1,    -1,    99,    -1,   101,    -1,   103,    -1,    -1,
     106,    -1,   108,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   118,    44,    -1,   121,    -1,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,    -1,    78,    -1,    44,
     148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,    78,    44,    -1,    -1,   148,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,    -1,    -1,    44,    -1,   148,    78,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,    78,
      44,    -1,    -1,   148,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    44,    78,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    78,    -1,    -1,    -1,   148,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    78,    -1,    -1,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      78,    -1,    -1,    -1,    -1,    -1,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143
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
     287,     8,   148,   130,   148,   179,   179,   178,   179,   183,
     184,     9,   147,    82,   144,     8,   183,   184,     8,   183,
     184,   183,   184,   183,   184,   183,   184,   183,   184,    26,
      61,   183,   147,   183,   183,   183,   183,   183,   183,    26,
      61,   149,   163,   148,   148,   149,   212,   170,   148,     5,
     188,   128,   191,   192,    61,   148,   147,    36,   120,    92,
     158,   203,   204,   147,     8,   261,   148,   185,    90,   181,
     195,   196,   229,   147,   233,   234,    83,   149,   235,   118,
      73,   158,   247,   234,   245,   148,   148,   148,   148,   148,
     148,   148,   130,   130,   130,     8,   185,   186,    82,   183,
       9,   147,   130,   185,   183,   147,     5,    60,   158,   164,
     165,   148,   147,    52,    84,   167,    29,    55,    58,   190,
     158,   286,   287,   185,   133,   148,   149,     8,   148,   147,
      21,    41,   149,   148,   149,     3,   236,   237,   232,   179,
     183,   184,   183,   184,   148,   148,   149,   130,   185,   186,
     183,   148,   130,   185,   149,   207,    29,    73,   168,   181,
     189,    84,   180,   193,   148,   148,   183,   204,   148,   148,
     196,   234,   133,   149,     8,   183,   148,   148,   183,   148,
     165,   148,   195,     5,   149,    29,   197,     8,   237,   149,
     149,   181,   195,   199,   147,     5,   207,   148,   148
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
     177,   177,   177,   178,   179,   179,   179,   179,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   181,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   182,   182,   183,   183,
     184,   184,   184,   185,   185,   186,   186,   187,   187,   188,
     188,   189,   189,   190,   190,   191,   191,   192,   193,   193,
     194,   194,   195,   195,   196,   196,   196,   197,   197,   198,
     198,   199,   199,   200,   201,   201,   202,   202,   202,   202,
     202,   202,   203,   203,   204,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   207,   207,   208,   208,   209,   209,   209,   210,
     210,   211,   211,   212,   212,   213,   214,   214,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   216,   216,   217,   217,   218,   218,   218,   218,   218,
     218,   219,   219,   219,   219,   220,   220,   221,   221,   221,
     222,   222,   223,   223,   223,   223,   223,   224,   224,   224,
     225,   225,   226,   227,   227,   228,   228,   229,   229,   230,
     230,   231,   231,   232,   233,   233,   234,   234,   234,   234,
     234,   235,   235,   236,   236,   237,   238,   239,   240,   240,
     241,   241,   242,   242,   243,   243,   244,   244,   245,   246,
     246,   247,   247,   248,   249,   249,   250,   251,   252,   253,
     253,   254,   254,   254,   254,   254,   254,   255,   255,   255,
     255,   255,   255,   255,   255,   255,   256,   256,   256,   257,
     257,   258,   258,   259,   260,   260,   261,   262,   263,   264,
     265,   265,   265,   266,   266,   266,   266,   267,   268,   268,
     268,   268,   268,   269,   270,   271,   271,   272,   273,   274,
     275,   276,   277,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   286,   287,   288,   288,   289,   289,   289,
     289,   290,   290,   291,   292,   292,   293,   294,   295,   296,
     296,   297,   298,   298,   299
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
       3,     3,     5,     4,     1,     3,     3,     3,     3,     3,
       5,     6,     5,     6,     3,     4,     5,     6,     3,     3,
       3,     3,     3,     3,     5,     5,     5,     3,     3,     3,
       3,     3,     3,     3,     4,     3,     3,     5,     6,     5,
       6,     3,     3,     3,     3,     1,     1,     4,     3,     3,
       1,     1,     4,     4,     4,     3,     4,     4,     1,     2,
       1,     2,     1,     1,     3,     1,     3,     0,     4,     0,
       1,     1,     3,     0,     2,     0,     1,     5,     0,     1,
       3,     5,     1,     3,     1,     2,     2,     0,     1,     2,
       4,     0,     1,     2,     1,     3,     1,     3,     3,     5,
       6,     3,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     3,     4,     4,     4,
       4,     4,     4,     4,     3,     6,     6,     3,     8,    14,
       3,     4,     1,     3,     1,     1,     1,     1,     1,     3,
       5,     1,     1,     1,     1,     2,     0,     2,     1,     2,
       2,     3,     1,     1,     1,     2,     4,     4,     3,     4,
       4,     1,     1,     0,     2,     4,     4,     4,     3,     4,
       4,     7,     5,     5,     9,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     7,     1,     1,     0,     3,     1,     1,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     1,     3,
       2,     0,     2,     1,     3,     3,     4,     6,     1,     3,
       1,     3,     1,     3,     0,     2,     1,     3,     3,     0,
       1,     1,     1,     3,     1,     1,     3,     3,     6,     1,
       3,     3,     3,     5,     4,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     7,     6,     4,     4,
       5,     0,     1,     2,     1,     3,     3,     2,     3,     6,
       0,     1,     1,     2,     2,     2,     1,     7,     1,     1,
       1,     1,     1,     3,     7,     0,     2,     3,     3,     2,
       2,     3,     1,     3,     2,     1,     2,     3,     4,     7,
       5,     5,     1,     1,     2,     1,     2,     1,     1,     3,
       3,     3,     3,     1,     0,     2,     1,     2,     1,     1,
       3,     5,     0,     2,     4
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

  case 183:

    {
			if ( !pParser->SetMatch((yyvsp[-1])) )
				YYERROR;
		}

    break;

  case 184:

    { pParser->SetOp ( (yyval) ); }

    break;

  case 185:

    { pParser->FilterAnd ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 186:

    { pParser->FilterOr ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 187:

    { pParser->FilterGroup ( (yyval), (yyvsp[-1]) ); }

    break;

  case 188:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 189:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
			pFilter->m_bExclude = true;
		}

    break;

  case 190:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-4]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		}

    break;

  case 191:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-5]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_bExclude = true;
		}

    break;

  case 192:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-4]), (yyvsp[-1]), false ) )
				YYERROR;
		}

    break;

  case 193:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-5]), (yyvsp[-1]), true ) )
				YYERROR;
		}

    break;

  case 194:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 195:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-3]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 196:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 197:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-5]), (yyvsp[-3]).m_iValue, (yyvsp[-1]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 198:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 199:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 200:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 201:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 202:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 203:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true, true ) )
				YYERROR;
		}

    break;

  case 204:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 205:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 206:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 207:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, false ) )
				YYERROR;
		}

    break;

  case 208:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, false ) )
				YYERROR;
		}

    break;

  case 209:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, true ) )
				YYERROR;
		}

    break;

  case 210:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 211:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 212:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 213:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-2]), true ) )
				YYERROR;
		}

    break;

  case 214:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-3]), false ) )
				YYERROR;
		}

    break;

  case 215:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-2]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-2]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 216:

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

  case 217:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-4]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-4]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 218:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 219:

    {
			AddMvaRange ( pParser, (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue );
		}

    break;

  case 220:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_RANGE );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_iMinValue = (yyvsp[-2]).m_iValue;
			f->m_iMaxValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 221:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue-1 );
		}

    break;

  case 222:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue+1, INT64_MAX );
		}

    break;

  case 223:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue );
		}

    break;

  case 224:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue, INT64_MAX );
		}

    break;

  case 226:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		}

    break;

  case 227:

    {
			(yyval).m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 228:

    {
			(yyval).m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 229:

    {
			(yyval).m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 230:

    {
			(yyval).m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 232:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 233:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 234:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 236:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ANY; }

    break;

  case 237:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ALL; }

    break;

  case 238:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 239:

    {
			(yyval).m_iType = TOK_CONST_INT;
			if ( (uint64_t)(yyvsp[0]).m_iValue > (uint64_t)LLONG_MAX )
				(yyval).m_iValue = LLONG_MIN;
			else
				(yyval).m_iValue = -(yyvsp[0]).m_iValue;
		}

    break;

  case 240:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 241:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = -(yyvsp[0]).m_fValue; }

    break;

  case 242:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

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

  case 245:

    {
			assert ( !(yyval).m_pValues.Ptr() );
			(yyval).m_pValues = new RefcountedVector_c<SphAttr_t> ();
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 246:

    {
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 250:

    {
			pParser->SetGroupbyLimit ( (yyvsp[0]).m_iValue );
		}

    break;

  case 251:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 252:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 254:

    {
			pParser->AddHaving();
		}

    break;

  case 257:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, (yyvsp[0]) );
		}

    break;

  case 260:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, (yyvsp[0]) );
		}

    break;

  case 261:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		}

    break;

  case 263:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 265:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 266:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 269:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 270:

    {
			pParser->m_pQuery->m_iOffset = (yyvsp[-2]).m_iValue;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 276:

    {
			if ( !pParser->AddOption ( (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 277:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 278:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 279:

    {
			if ( !pParser->AddOption ( (yyvsp[-4]), pParser->GetNamedVec ( (yyvsp[-1]).m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( (yyvsp[-1]).m_iValue );
		}

    break;

  case 280:

    {
			if ( !pParser->AddOption ( (yyvsp[-5]), (yyvsp[-3]), (yyvsp[-1]) ) )
				YYERROR;
		}

    break;

  case 281:

    {
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 282:

    {
			(yyval).m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 283:

    {
			pParser->AddConst( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 284:

    {
			(yyval) = (yyvsp[-2]);
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 286:

    { if ( !pParser->SetOldSyntax() ) YYERROR; }

    break;

  case 287:

    { if ( !pParser->SetNewSyntax() ) YYERROR; }

    break;

  case 292:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 293:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

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

  case 311:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 312:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 316:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

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

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 323:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 324:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 325:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 326:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 327:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 328:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-7]), (yyvsp[0]) ); }

    break;

  case 329:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-13]), (yyvsp[0]) ); }

    break;

  case 330:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 331:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 339:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 340:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-4]), (yyvsp[0]) ); }

    break;

  case 347:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) ); }

    break;

  case 348:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; }

    break;

  case 349:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; }

    break;

  case 350:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; }

    break;

  case 351:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; }

    break;

  case 352:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; }

    break;

  case 353:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; }

    break;

  case 354:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; }

    break;

  case 355:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; }

    break;

  case 356:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 357:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 358:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 359:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 360:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
			pParser->m_pStmt->m_iIntParam = int((yyvsp[-1]).m_fValue*10);
		}

    break;

  case 364:

    {
			pParser->m_pStmt->m_iIntParam = (yyvsp[0]).m_iValue;
		}

    break;

  case 365:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 366:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) );
		}

    break;

  case 367:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		}

    break;

  case 368:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 369:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 370:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 371:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
		}

    break;

  case 372:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) ).Unquote();
		}

    break;

  case 373:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 374:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-6]) );
		}

    break;

  case 377:

    { (yyval).m_iValue = 1; }

    break;

  case 378:

    { (yyval).m_iValue = 0; }

    break;

  case 379:

    {
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
			if ( (yyval).m_iValue!=0 && (yyval).m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		}

    break;

  case 387:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; }

    break;

  case 388:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; }

    break;

  case 389:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; }

    break;

  case 392:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-4]) );
		}

    break;

  case 393:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; }

    break;

  case 394:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; }

    break;

  case 399:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 400:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 403:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } }

    break;

  case 404:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 405:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 406:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 407:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 408:

    { (yyval).m_iType = TOK_QUOTED_STRING; (yyval).m_iStart = (yyvsp[0]).m_iStart; (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 409:

    { (yyval).m_iType = TOK_CONST_MVA; (yyval).m_pValues = (yyvsp[-1]).m_pValues; }

    break;

  case 410:

    { (yyval).m_iType = TOK_CONST_MVA; }

    break;

  case 415:

    { if ( !pParser->AddInsertOption ( (yyvsp[-2]), (yyvsp[0]) ) ) YYERROR; }

    break;

  case 416:

    {
		if ( !pParser->DeleteStatement ( &(yyvsp[-1]) ) )
			YYERROR;
	}

    break;

  case 417:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, (yyvsp[-4]) );
		}

    break;

  case 418:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 419:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 421:

    {
			(yyval).m_iType = TOK_CONST_STRINGS;
		}

    break;

  case 422:

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

  case 423:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), (yyvsp[0]) );
		}

    break;

  case 426:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		}

    break;

  case 428:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), (yyvsp[0]) );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, (yyvsp[-2]) );
		}

    break;

  case 433:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 436:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; }

    break;

  case 437:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; }

    break;

  case 438:

    {
			if ( !pParser->UpdateStatement ( &(yyvsp[-4]) ) )
				YYERROR;
		}

    break;

  case 441:

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

  case 442:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( (yyvsp[0]).m_fValue ) );
			pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_FLOAT );
		}

    break;

  case 443:

    {
			pParser->UpdateMVAAttr ( (yyvsp[-4]), (yyvsp[-1]) );
		}

    break;

  case 444:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( (yyvsp[-3]), tNoValues );
		}

    break;

  case 445:

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

  case 446:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( (yyvsp[0]).m_fValue ) );
			pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_FLOAT );
		}

    break;

  case 447:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 448:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 449:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 450:

    { (yyval).m_iValue = SPH_ATTR_BOOL; }

    break;

  case 451:

    { (yyval).m_iValue = SPH_ATTR_UINT32SET; }

    break;

  case 452:

    { (yyval).m_iValue = SPH_ATTR_INT64SET; }

    break;

  case 453:

    { (yyval).m_iValue = SPH_ATTR_JSON; }

    break;

  case 454:

    { (yyval).m_iValue = SPH_ATTR_STRING; }

    break;

  case 455:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 456:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-4]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[-1]) );
			tStmt.m_eAlterColType = (ESphAttr)(yyvsp[0]).m_iValue;
		}

    break;

  case 457:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-3]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[0]) );
		}

    break;

  case 458:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 459:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		}

    break;

  case 460:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 467:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		}

    break;

  case 468:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		}

    break;

  case 469:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		}

    break;

  case 477:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, (yyvsp[0]) );
			tStmt.m_eUdfType = (ESphAttr) (yyvsp[-2]).m_iValue;
		}

    break;

  case 478:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 479:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 480:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 481:

    { (yyval).m_iValue = SPH_ATTR_STRINGPTR; }

    break;

  case 482:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 483:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[0]) );
		}

    break;

  case 484:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-4]) );
			pParser->ToString ( tStmt.m_sStringParam, (yyvsp[-1]) );
		}

    break;

  case 486:

    {
			pParser->m_pStmt->m_iIntParam = 1;
		}

    break;

  case 487:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 488:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 489:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		}

    break;

  case 490:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_HOSTNAMES;
		}

    break;

  case 491:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[-1]) );
		}

    break;

  case 495:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 496:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[0]) );
		}

    break;

  case 497:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 498:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 499:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 500:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 501:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 504:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 506:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 507:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 508:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 509:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 510:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 511:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 512:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 516:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		}

    break;

  case 518:

    {
			pParser->AddItem ( &(yyvsp[0]) );
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 521:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
		}

    break;

  case 523:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 524:

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
