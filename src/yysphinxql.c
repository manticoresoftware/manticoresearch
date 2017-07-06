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
#define YYLAST   5229

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  154
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  145
/* YYNRULES -- Number of rules.  */
#define YYNRULES  519
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  920

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
     413,   417,   422,   426,   433,   441,   449,   458,   463,   468,
     473,   478,   483,   488,   493,   498,   503,   508,   513,   518,
     523,   528,   533,   538,   543,   548,   553,   558,   563,   568,
     575,   581,   592,   599,   608,   612,   621,   625,   629,   633,
     640,   641,   646,   652,   658,   664,   670,   671,   672,   673,
     674,   678,   679,   683,   684,   695,   696,   697,   701,   707,
     714,   720,   726,   728,   731,   733,   740,   744,   750,   752,
     758,   760,   764,   775,   777,   781,   785,   792,   793,   797,
     798,   799,   802,   804,   808,   813,   820,   822,   826,   830,
     831,   835,   840,   845,   850,   856,   861,   869,   874,   881,
     891,   892,   893,   894,   895,   896,   897,   898,   899,   901,
     902,   903,   904,   905,   906,   907,   908,   909,   910,   911,
     912,   913,   914,   915,   916,   917,   918,   919,   920,   921,
     922,   923,   924,   928,   929,   930,   931,   932,   933,   934,
     935,   936,   937,   938,   939,   940,   941,   945,   946,   950,
     951,   955,   956,   957,   961,   962,   966,   967,   971,   972,
     978,   981,   983,   987,   988,   989,   990,   991,   992,   993,
     994,   995,  1000,  1005,  1010,  1015,  1024,  1025,  1028,  1030,
    1038,  1043,  1048,  1053,  1054,  1055,  1059,  1064,  1069,  1074,
    1083,  1084,  1088,  1089,  1090,  1102,  1103,  1107,  1108,  1109,
    1110,  1111,  1118,  1119,  1120,  1124,  1125,  1131,  1139,  1140,
    1143,  1145,  1149,  1150,  1154,  1155,  1159,  1160,  1164,  1168,
    1169,  1173,  1174,  1175,  1176,  1177,  1180,  1181,  1185,  1186,
    1190,  1196,  1206,  1214,  1218,  1225,  1226,  1233,  1243,  1249,
    1251,  1255,  1260,  1264,  1271,  1273,  1277,  1278,  1284,  1292,
    1293,  1299,  1303,  1309,  1317,  1318,  1322,  1336,  1342,  1346,
    1351,  1365,  1376,  1377,  1378,  1379,  1380,  1381,  1382,  1383,
    1384,  1388,  1396,  1403,  1414,  1418,  1425,  1426,  1430,  1434,
    1435,  1439,  1443,  1450,  1457,  1463,  1464,  1465,  1469,  1470,
    1471,  1472,  1478,  1489,  1490,  1491,  1492,  1493,  1498,  1509,
    1518,  1520,  1529,  1538,  1547,  1555,  1565,  1573,  1574,  1578,
    1582,  1586,  1596,  1607,  1618,  1629,  1639,  1650,  1651,  1655,
    1658,  1659,  1663,  1664,  1665,  1666,  1670,  1671,  1675,  1680,
    1682,  1686,  1695,  1699,  1707,  1708,  1712,  1723,  1725,  1732
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

#define YYPACT_NINF -784

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-784)))

#define YYTABLE_NINF -509

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    4828,   142,   -10,  -784,  4318,  -784,   133,    38,  -784,  -784,
     168,   211,  -784,    58,   141,  -784,  -784,   889,  4444,   438,
       6,    39,  4318,   243,  -784,   -15,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,   112,  -784,  -784,  -784,
    4318,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  4318,  4318,  4318,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,    92,  4318,  4318,
    4318,  4318,  4318,  -784,  -784,  4318,  4318,  4318,  4318,   194,
     105,  -784,  -784,  -784,  -784,  -784,  -784,  -784,   109,   117,
     130,   134,   137,   196,   198,   209,   215,  -784,   244,   246,
     248,   254,   257,   272,   277,   278,   282,  1925,  -784,  1925,
    1925,  3736,     8,   -19,  -784,  3850,    14,  -784,   283,    45,
    -784,  3850,   205,   258,  -784,   317,   318,   349,  4570,  4318,
    2992,   337,   323,   341,  3976,   355,  -784,   388,  -784,  -784,
     388,  -784,  -784,  -784,  -784,   388,  -784,   388,   376,  -784,
    -784,  4318,   338,  -784,  4318,  -784,   -50,  -784,  1925,   179,
    -784,  4318,   388,   369,   207,   350,    40,   370,   348,   -53,
    -784,   351,  -784,  -784,   376,   420,   367,  1037,  1925,  2073,
      -5,  2073,  2073,   330,  1925,  2073,  2073,  1925,  1925,  1185,
    1925,  1925,   331,   335,   339,   340,  -784,  -784,  4768,  -784,
    -784,   154,   352,  -784,  -784,  2221,    37,  -784,  2613,  1333,
    4318,  -784,  -784,  1925,  1925,  1925,  1925,  1925,  1925,  1925,
    1925,  1925,  1925,  1925,  1925,  1925,  1925,  1925,  1925,  1925,
    1925,   479,   478,  -784,  -784,  -784,   123,  1925,  2992,  2992,
     357,   360,   440,  -784,  -784,  -784,  -784,  -784,   358,  -784,
    2740,   426,   394,   160,   398,  -784,   500,  -784,  -784,  -784,
    -784,  4318,  -784,  -784,   126,    62,  -784,  4318,  4318,  5020,
    -784,  3850,   -12,  1481,  4696,   387,  -784,   362,  -784,  -784,
     476,   481,   412,  -784,  -784,  -784,  -784,   303,    29,  -784,
    -784,  -784,   364,  -784,   200,   510,  2359,  -784,   511,  -784,
     513,  -784,   515,   392,  1629,  -784,  5020,   172,  -784,  4836,
     174,  4318,   380,   181,   184,  -784,  4873,   189,   191,   509,
     609,  -784,   193,   158,  4903,  -784,  1777,  1925,  1925,  -784,
    3736,  -784,  2866,   377,   -29,  -784,  -784,   -53,  -784,  5020,
    -784,  -784,  -784,  5040,  5053,  5086,   686,   275,  -784,   275,
     -14,   -14,   -14,   -14,    54,    54,  -784,  -784,  -784,  5005,
     382,  -784,  -784,   447,   275,   358,   358,   385,  3496,   524,
    2992,  -784,  -784,  -784,  -784,   529,  -784,  -784,  -784,  -784,
     465,   388,  -784,   388,  -784,   404,   390,  -784,   436,   536,
    -784,   453,   534,  4318,  -784,  -784,    66,   -11,  -784,   411,
    -784,  -784,  -784,  1925,   474,  1925,  4090,   439,  4318,  4318,
    4318,  -784,  -784,  -784,  -784,  -784,   206,   217,    40,   416,
    -784,  -784,  -784,  -784,  -784,   458,   460,  -784,   413,   423,
     424,   425,   427,   428,   429,  -784,   431,   432,   433,  2359,
      37,   443,  -784,  -784,   180,   214,  -784,  -784,  -784,  -784,
    1333,   434,  -784,  2073,  -784,  -784,   435,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  1925,  -784,  1925,  -784,  1925,  -784,
     658,   725,   451,  -784,  -784,  -784,  -784,  -784,   484,   531,
    -784,  4318,   585,  -784,    19,  -784,  -784,   459,  -784,   176,
    -784,  -784,  2486,  4318,  -784,  -784,  -784,  -784,   461,   462,
    -784,    69,  4318,   376,    72,  -784,   564,   522,  -784,   448,
    -784,  -784,  -784,   220,   449,   489,  -784,   473,  -784,    19,
    -784,   593,    52,  -784,   455,  -784,  -784,   594,   597,  4318,
    4318,  4318,   471,  4318,   466,   482,  4318,   614,   483,   103,
    2359,    72,    34,   128,    79,    83,    72,    72,    72,    72,
      55,    19,   477,    19,    19,    19,    19,    19,    19,   152,
     485,  -784,  -784,  4939,  4974,  4700,  2866,  1333,   506,   628,
     527,   595,  -784,   222,   514,    -1,   568,  -784,  -784,  -784,
    -784,  4318,   516,  -784,   656,  4318,    35,  -784,  -784,  -784,
    -784,  -784,  -784,  3118,  -784,  -784,  4090,    51,   -54,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
     547,  -784,  -784,  -784,  -784,  4204,    51,  -784,  -784,   518,
     519,    37,   521,   523,   525,  -784,  -784,   526,   528,  -784,
    -784,  -784,   540,   542,  -784,    23,  -784,   598,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,    19,    46,   545,    19,  -784,  -784,  -784,  -784,
    -784,  -784,    19,   532,  3622,   535,  -784,  -784,   538,  -784,
      -8,   604,  -784,   661,   636,   635,  -784,  4318,  -784,    19,
    -784,  -784,  -784,   561,   224,  -784,   687,  -784,  -784,  -784,
     226,   549,   203,   548,  -784,  -784,    36,   229,  -784,   695,
     449,  -784,  -784,  -784,  -784,  -784,   679,  -784,  -784,  -784,
    -784,  -784,  -784,  -784,  -784,    72,    72,  -784,   231,   233,
    -784,   570,  -784,    23,    19,   235,   571,    19,  -784,  -784,
    -784,   554,  -784,  -784,  2073,  4318,   675,   632,  3244,   622,
    3370,   474,    37,   559,  -784,   237,    19,  -784,  4318,   560,
    -784,   562,  -784,  -784,  3244,  -784,    51,   576,   563,  -784,
    -784,  -784,  -784,  -784,  -784,  -784,  -784,   705,    19,   239,
     241,  -784,  -784,    19,   249,  3622,   251,  3244,   710,  -784,
    -784,   567,   688,  -784,   522,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,   711,   695,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,   569,   548,   572,  3244,  3244,   376,  -784,  -784,
     573,   717,  -784,   548,  -784,  2073,  -784,   273,   575,  -784
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   385,     0,   382,     0,     0,   430,   429,
       0,     0,   388,     0,     0,   389,   383,     0,   465,   465,
       0,     0,     0,     0,     2,     3,   134,   137,   140,   142,
     138,   139,     7,     8,   384,     5,     0,     6,     9,    10,
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
       0,     0,     0,   484,   485,     0,     0,     0,     0,     0,
      34,   281,   283,   284,   508,   286,   490,   285,    37,    39,
      43,    46,    55,    62,    64,    71,    70,   282,     0,    75,
      78,    85,    87,    97,   100,   115,   127,     0,   159,     0,
       0,     0,   280,     0,   157,   161,   164,   308,     0,   262,
     487,   161,     0,   309,   310,     0,     0,    49,    68,    74,
       0,   107,     0,     0,     0,     0,   462,   341,   466,   356,
     341,   348,   349,   347,   467,   341,   357,   341,   266,   343,
     340,     0,     0,   386,     0,   173,     0,     1,     0,     4,
     136,     0,   341,     0,     0,     0,     0,     0,     0,     0,
     478,     0,   483,   482,   266,   517,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    34,    85,    87,   287,   288,     0,   337,
     336,     0,     0,   502,   503,     0,   499,   500,     0,     0,
       0,   162,   160,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   486,   263,   489,     0,     0,     0,     0,
       0,     0,     0,   380,   381,   379,   378,   377,   363,   375,
       0,     0,     0,   341,     0,   463,     0,   432,   345,   344,
     431,     0,   350,   267,   358,   456,   492,     0,     0,   513,
     514,   161,   509,     0,     0,     0,   135,   390,   428,   453,
       0,     0,     0,   233,   235,   403,   237,     0,     0,   401,
     402,   415,   419,   413,     0,     0,     0,   411,     0,   493,
       0,   519,     0,   330,     0,   319,   329,     0,   327,     0,
       0,     0,     0,     0,     0,   171,     0,     0,     0,     0,
       0,   325,     0,     0,     0,   322,     0,     0,     0,   306,
       0,   307,     0,   508,     0,   501,   151,   175,   158,   164,
     163,   298,   299,   305,   304,   296,   295,   302,   506,   303,
     293,   294,   300,   301,   289,   290,   291,   292,   297,     0,
     264,   488,   311,     0,   507,   364,   365,     0,     0,     0,
       0,   371,   373,   362,   372,     0,   370,   374,   361,   360,
       0,   341,   346,   341,   342,   271,   268,   269,     0,     0,
     353,     0,     0,     0,   454,   457,     0,     0,   434,     0,
     174,   512,   511,     0,   253,     0,     0,     0,     0,     0,
       0,   234,   236,   417,   405,   238,     0,     0,     0,     0,
     474,   475,   473,   477,   476,     0,     0,   221,    37,    39,
      46,    55,    62,     0,    71,   225,    78,    84,   127,     0,
     220,   177,   178,   182,     0,     0,   226,   495,   518,   496,
       0,     0,   313,     0,   165,   316,     0,   170,   318,   317,
     169,   314,   315,   166,     0,   167,     0,   326,     0,   168,
       0,     0,     0,   339,   338,   334,   505,   504,     0,   242,
     176,     0,     0,   312,     0,   368,   367,     0,   376,     0,
     351,   352,     0,     0,   355,   359,   354,   455,     0,   458,
     459,     0,     0,   266,     0,   515,     0,   262,   254,   510,
     393,   392,   394,     0,     0,     0,   452,   480,   404,     0,
     416,     0,   424,   414,   420,   421,   412,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     144,   328,   172,     0,     0,     0,     0,     0,     0,   244,
     250,     0,   265,     0,     0,     0,     0,   471,   464,   273,
     276,     0,   272,   270,     0,     0,     0,   436,   437,   435,
     433,   440,   441,     0,   516,   391,     0,     0,   406,   396,
     443,   445,   444,   450,   442,   448,   446,   447,   449,   451,
       0,   479,   239,   418,   425,     0,     0,   472,   494,     0,
       0,     0,     0,     0,     0,   230,   223,     0,     0,   224,
     180,   179,     0,     0,   189,     0,   208,     0,   206,   183,
     197,   207,   184,   198,   194,   203,   193,   202,   196,   205,
     195,   204,     0,     0,     0,     0,   210,   211,   216,   217,
     218,   219,     0,     0,     0,     0,   321,   320,     0,   335,
       0,     0,   245,     0,     0,   248,   251,     0,   366,     0,
     469,   468,   470,     0,     0,   277,     0,   461,   460,   439,
       0,    97,   259,   255,   257,   395,     0,     0,   399,     0,
       0,   387,   481,   427,   426,   423,   424,   422,   232,   231,
     229,   222,   228,   227,   181,     0,     0,   240,     0,     0,
     209,     0,   190,     0,     0,     0,     0,     0,   149,   150,
     148,   145,   146,   141,     0,     0,     0,   153,     0,     0,
       0,   253,   498,     0,   497,     0,     0,   274,     0,     0,
     438,     0,   260,   261,     0,   398,     0,     0,   407,   408,
     397,   191,   200,   201,   199,   185,   187,     0,     0,     0,
       0,   214,   212,     0,     0,     0,     0,     0,     0,   143,
     246,   243,     0,   249,   262,   323,   369,   279,   278,   275,
     256,   258,   400,     0,     0,   241,   192,   186,   188,   215,
     213,   147,     0,   152,   154,     0,     0,   266,   410,   409,
       0,     0,   247,   252,   156,     0,   155,     0,     0,   324
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -784,  -784,  -784,   -13,    -4,  -784,   467,  -784,   321,  -784,
    -784,  -148,  -784,  -784,  -784,   150,    64,   444,  -209,  -784,
      -9,  -784,  -355,  -523,  -784,  -111,  -646,  -784,   -38,  -578,
    -565,   -91,  -784,  -784,  -784,  -784,  -784,  -784,  -108,  -784,
    -783,  -120,  -598,  -784,  -272,  -784,  -784,   144,  -784,  -113,
     118,  -784,  -278,   201,  -784,  -784,   325,    90,  -784,  -220,
    -784,  -784,  -784,  -784,  -784,   291,  -784,    21,   292,  -784,
    -784,  -784,  -784,  -784,    65,  -784,  -784,   -37,  -784,  -512,
    -784,  -784,  -129,  -784,  -784,  -784,   250,  -784,  -784,  -784,
      50,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,   165,
    -784,  -784,  -784,  -784,  -784,  -784,    85,  -784,  -784,  -784,
     752,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,  -784,  -784,   442,  -784,  -784,  -784,  -784,  -784,  -784,
    -784,    -2,  -357,  -784,   470,  -784,   463,  -784,  -784,   269,
    -784,   274,  -784,  -784,  -784
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    23,    24,   166,   212,    25,    26,    27,    28,   765,
     831,   832,   578,   837,   879,    29,   213,   214,   312,   215,
     437,   579,   397,   541,   542,   543,   544,   545,   515,   390,
     516,   819,   670,   773,   881,   841,   775,   776,   607,   608,
     793,   794,   333,   334,   362,   363,   486,   487,   784,   785,
     406,   217,   407,   408,   218,   301,   302,   575,    30,   357,
     250,   251,   491,    31,    32,   478,   479,   348,   349,    33,
      34,    35,    36,   507,   612,   613,   698,   699,   797,   391,
     801,   858,   859,    37,    38,   392,   393,   517,   519,   624,
     625,   715,   805,    39,    40,    41,    42,    43,   497,   498,
     709,    44,    45,   494,   495,   599,   600,    46,    47,    48,
     233,   678,    49,   525,    50,    51,   711,    52,    53,    54,
      55,    56,   219,   220,   221,    57,    58,    59,    60,    61,
      62,   222,   223,   306,   307,   224,   225,   504,   505,   370,
     371,   372,   260,   401,    63
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     167,   410,   399,   413,   414,   232,   622,   417,   418,   694,
     499,   422,   335,   256,  -491,   313,   639,   502,   255,   673,
     358,   303,   304,   688,   383,   359,   692,   360,   383,   799,
     313,   817,   258,   308,   383,   780,   262,   513,   411,   546,
     383,   383,   378,   734,   835,   383,   384,   792,   385,   314,
     303,   304,    66,   367,   386,   822,   383,   384,   313,   385,
     263,   264,   265,   733,   314,   386,   740,   743,   745,   747,
     749,   751,   714,   396,   383,   384,  -498,   383,   384,   303,
     304,   752,   580,   386,   383,   384,   386,   738,   383,   384,
     170,   741,   314,   386,   903,   800,   368,   386,   313,   368,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   396,   753,   731,   331,   781,
     177,   790,   253,   913,   577,   325,   326,   327,   328,   329,
     309,   259,   314,   482,   492,   216,   412,   503,   602,   254,
     488,   309,   603,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   489,   475,
     305,   269,   501,   475,   267,   268,   255,   270,   271,   475,
     818,   272,   273,   274,   275,   475,   475,   514,   762,   261,
     387,   735,   546,   789,   514,   798,   168,   388,   493,   305,
     825,   387,   880,   823,   332,   327,   328,   329,   796,   601,
    -415,  -415,   313,   178,   806,   462,   641,   300,   792,   387,
     736,   311,   387,   763,   845,   341,   686,   311,   305,   387,
     169,   171,   380,   387,   852,   342,   347,   520,   389,   179,
     354,   792,   356,   640,   173,   490,   314,   862,   864,   266,
     651,   642,    64,   257,   853,   499,   276,   364,   643,   521,
     366,   730,   277,   381,    65,   172,  -332,   377,   869,   912,
     792,   590,   874,   591,  -331,   522,   523,   463,   675,  -113,
     174,   676,   737,   336,   722,   652,   724,   278,   677,   727,
     373,   279,   374,   546,   280,   375,   907,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   175,   430,   255,   431,   440,   568,   511,   512,
     524,   176,   477,   644,   645,   646,   647,   648,   649,   313,
     552,   553,   555,   553,   650,   296,  -497,   297,   298,   558,
     553,   690,   559,   553,   347,   347,   546,   561,   553,   562,
     553,   567,   553,   281,   892,   282,   476,   653,   654,   655,
     656,   657,   658,   314,   618,   619,   283,   485,   659,   465,
     466,   232,   284,   496,   500,   620,   621,   311,   695,   696,
     778,   619,   847,   848,   850,   619,   369,   855,   856,   865,
     619,   866,   867,   872,   619,   886,   619,   897,   619,   898,
     867,   285,   540,  -333,   574,   286,   409,   900,   619,   902,
     553,   287,   416,   234,   288,   419,   420,   556,   423,   424,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   289,
     844,   918,   553,   434,   290,   291,   300,   439,   573,   292,
     330,   441,   442,   443,   444,   445,   446,   447,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   239,
     337,   338,   339,  -467,   234,   464,   350,   351,   355,   361,
     356,   365,   379,   240,   476,   382,   347,   395,   394,   235,
     398,   236,   400,   241,   402,   242,   243,   237,   415,   425,
     389,   546,   426,   546,   460,   432,   427,   428,   186,   598,
     467,   439,   238,   468,   469,   480,   245,   546,   470,   246,
     239,   248,   611,   481,   615,   616,   617,   483,   484,   506,
     508,   249,   510,   518,   240,   509,   700,   701,   526,   547,
     546,   548,   298,   549,   241,  -508,   242,   243,   557,   583,
     576,   582,   584,   587,   511,   540,   589,   592,   702,   593,
     594,   595,   597,   244,   604,   570,   571,   245,   546,   546,
     246,   247,   248,   313,   703,   704,   876,   596,   606,   705,
     629,   614,   249,   687,   626,   627,   691,   628,   706,   707,
     630,   631,   632,   640,   633,   634,   635,   671,   636,   637,
     638,   712,   660,   662,   666,   667,   669,   314,   682,   485,
     672,   685,   674,   693,   684,   331,   697,   503,   496,   708,
     710,   713,   717,   732,   716,   718,   739,   742,   744,   746,
     748,   750,   723,   754,   725,   756,   757,   758,   759,   760,
     761,   369,   728,   369,   755,   719,   720,   721,   574,   721,
     726,   729,   721,   772,   764,   914,   540,   917,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   313,   771,   774,   777,   563,   564,   389,
     782,   779,   573,   786,   787,   802,   808,   809,   439,   810,
     815,   811,   816,   812,   813,   824,   814,   783,   389,   827,
     820,   598,   663,   833,   664,   834,   665,   314,   836,   540,
     838,   839,   611,   840,   846,   849,   851,   854,   857,   714,
     868,   873,   313,   875,   877,   878,   882,   885,   889,   893,
     890,   804,   894,   895,   821,   904,   905,   906,   910,   908,
     915,   911,   916,   919,   826,   551,   376,   901,   668,   883,
     313,   770,   870,   884,   891,   888,   314,   683,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   438,   661,   572,   769,   565,   566,   586,
     830,   795,   588,   860,   314,   909,   807,   689,   623,   313,
     788,   252,   605,   842,   461,   843,   435,   861,   863,   609,
       0,     0,   448,     0,     0,   439,   871,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,     0,   314,     0,     0,     0,   564,   887,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   389,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     896,   255,     0,     0,   540,   899,   540,     0,     0,     0,
       0,     0,     0,     0,   783,     0,     0,     0,     0,     0,
     540,     0,     0,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,     0,
       0,   830,     0,   540,   566,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   180,   181,   182,   183,     0,   184,   185,   186,
       0,   540,   540,   187,    68,    69,    70,   188,   189,     0,
      73,    74,    75,   190,    77,    78,   191,    80,     0,    81,
      82,    83,    84,    85,    86,    87,   192,    89,    90,    91,
      92,    93,    94,     0,   193,    96,     0,     0,   194,    98,
      99,     0,   100,   101,   102,   195,   196,   105,   106,   197,
     198,   107,   199,   109,   110,   200,   112,     0,   113,   114,
     115,   116,     0,   117,   201,   119,   202,     0,   121,   122,
     123,     0,   124,     0,   125,   126,   127,   128,   129,   203,
     131,   132,   133,     0,   134,   135,   204,   137,   138,   139,
       0,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     205,   150,   151,   152,   153,   154,     0,   155,   156,   157,
     158,   159,   160,   161,   206,   163,   164,   165,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   207,
     208,     0,     0,   209,     0,     0,   210,     0,     0,   211,
     293,   181,   182,   183,     0,   403,   185,     0,     0,     0,
       0,   187,    68,    69,    70,   188,   189,     0,    73,    74,
      75,    76,    77,    78,   191,    80,     0,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,   193,    96,     0,     0,   194,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   197,   198,   107,
     199,   109,   110,   200,   112,     0,   113,   114,   115,   116,
       0,   117,   294,   119,   295,     0,   121,   122,   123,     0,
     124,     0,   125,   126,   127,   128,   129,   203,   131,   132,
     133,     0,   134,   135,   204,   137,   138,   139,     0,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   155,   156,   157,   158,   159,
     160,   161,   206,   163,   164,   165,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   207,     0,     0,
       0,   209,     0,     0,   404,   405,     0,   211,   293,   181,
     182,   183,     0,   403,   185,     0,     0,     0,     0,   187,
      68,    69,    70,   188,   189,     0,    73,    74,    75,    76,
      77,    78,   191,    80,     0,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
     193,    96,     0,     0,   194,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   197,   198,   107,   199,   109,
     110,   200,   112,     0,   113,   114,   115,   116,     0,   117,
     294,   119,   295,     0,   121,   122,   123,     0,   124,     0,
     125,   126,   127,   128,   129,   203,   131,   132,   133,     0,
     134,   135,   204,   137,   138,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,   158,   159,   160,   161,
     206,   163,   164,   165,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   207,     0,     0,     0,   209,
       0,     0,   210,   421,     0,   211,   293,   181,   182,   183,
       0,   184,   185,     0,     0,     0,     0,   187,    68,    69,
      70,   188,   189,     0,    73,    74,    75,   190,    77,    78,
     191,    80,     0,    81,    82,    83,    84,    85,    86,    87,
     192,    89,    90,    91,    92,    93,    94,     0,   193,    96,
       0,     0,   194,    98,    99,     0,   100,   101,   102,   195,
     196,   105,   106,   197,   198,   107,   199,   109,   110,   200,
     112,     0,   113,   114,   115,   116,     0,   117,   201,   119,
     202,     0,   121,   122,   123,     0,   124,     0,   125,   126,
     127,   128,   129,   203,   131,   132,   133,     0,   134,   135,
     204,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   205,   150,   151,   152,   153,   154,
       0,   155,   156,   157,   158,   159,   160,   161,   206,   163,
     164,   165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   207,   208,     0,     0,   209,     0,     0,
     210,     0,     0,   211,   180,   181,   182,   183,     0,   184,
     185,     0,     0,     0,     0,   187,    68,    69,    70,   188,
     189,     0,    73,    74,    75,   190,    77,    78,   191,    80,
       0,    81,    82,    83,    84,    85,    86,    87,   192,    89,
      90,    91,    92,    93,    94,     0,   193,    96,     0,     0,
     194,    98,    99,     0,   100,   101,   102,   195,   196,   105,
     106,   197,   198,   107,   199,   109,   110,   200,   112,     0,
     113,   114,   115,   116,     0,   117,   201,   119,   202,     0,
     121,   122,   123,     0,   124,     0,   125,   126,   127,   128,
     129,   203,   131,   132,   133,     0,   134,   135,   204,   137,
     138,   139,     0,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   205,   150,   151,   152,   153,   154,     0,   155,
     156,   157,   158,   159,   160,   161,   206,   163,   164,   165,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   207,   208,     0,     0,   209,     0,     0,   210,     0,
       0,   211,   293,   181,   182,   183,     0,   184,   185,     0,
       0,     0,     0,   187,    68,    69,    70,   188,   189,     0,
      73,    74,    75,    76,    77,    78,   191,    80,     0,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,     0,   193,    96,     0,     0,   194,    98,
      99,     0,   100,   101,   102,   103,   104,   105,   106,   197,
     198,   107,   199,   109,   110,   200,   112,     0,   113,   114,
     115,   116,     0,   117,   294,   119,   295,     0,   121,   122,
     123,     0,   124,     0,   125,   126,   127,   128,   129,   203,
     131,   132,   133,     0,   134,   135,   204,   137,   138,   139,
     550,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,     0,   155,   156,   157,
     158,   159,   160,   161,   206,   163,   164,   165,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   207,
       0,     0,     0,   209,     0,     0,   210,     0,     0,   211,
     293,   181,   182,   183,     0,   403,   185,     0,     0,     0,
       0,   187,    68,    69,    70,   188,   189,     0,    73,    74,
      75,    76,    77,    78,   191,    80,     0,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,   193,    96,     0,     0,   194,    98,    99,     0,
     100,   101,   102,   103,   104,   105,   106,   197,   198,   107,
     199,   109,   110,   200,   112,     0,   113,   114,   115,   116,
       0,   117,   294,   119,   295,     0,   121,   122,   123,     0,
     124,     0,   125,   126,   127,   128,   129,   203,   131,   132,
     133,     0,   134,   135,   204,   137,   138,   139,     0,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,     0,   155,   156,   157,   158,   159,
     160,   161,   206,   163,   164,   165,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   207,     0,     0,
       0,   209,     0,     0,   210,   405,     0,   211,   293,   181,
     182,   183,     0,   184,   185,     0,     0,     0,     0,   187,
      68,    69,    70,   188,   189,     0,    73,    74,    75,    76,
      77,    78,   191,    80,     0,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
     193,    96,     0,     0,   194,    98,    99,     0,   100,   101,
     102,   103,   104,   105,   106,   197,   198,   107,   199,   109,
     110,   200,   112,     0,   113,   114,   115,   116,     0,   117,
     294,   119,   295,     0,   121,   122,   123,     0,   124,     0,
     125,   126,   127,   128,   129,   203,   131,   132,   133,     0,
     134,   135,   204,   137,   138,   139,     0,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,   155,   156,   157,   158,   159,   160,   161,
     206,   163,   164,   165,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   207,     0,     0,     0,   209,
       0,     0,   210,     0,     0,   211,   293,   181,   182,   183,
       0,   403,   185,     0,     0,     0,     0,   187,    68,    69,
      70,   188,   189,     0,    73,    74,    75,    76,    77,    78,
     191,    80,     0,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,   193,    96,
       0,     0,   194,    98,    99,     0,   100,   101,   102,   103,
     104,   105,   106,   197,   198,   107,   199,   109,   110,   200,
     112,     0,   113,   114,   115,   116,     0,   117,   294,   119,
     295,     0,   121,   122,   123,     0,   124,     0,   125,   126,
     127,   128,   129,   203,   131,   132,   133,     0,   134,   135,
     204,   137,   138,   139,     0,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,   155,   156,   157,   158,   159,   160,   161,   206,   163,
     164,   165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   207,     0,     0,     0,   209,     0,     0,
     210,     0,     0,   211,   293,   181,   182,   183,     0,   433,
     185,     0,     0,     0,     0,   187,    68,    69,    70,   188,
     189,     0,    73,    74,    75,    76,    77,    78,   191,    80,
       0,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,   193,    96,     0,     0,
     194,    98,    99,     0,   100,   101,   102,   103,   104,   105,
     106,   197,   198,   107,   199,   109,   110,   200,   112,     0,
     113,   114,   115,   116,     0,   117,   294,   119,   295,     0,
     121,   122,   123,     0,   124,     0,   125,   126,   127,   128,
     129,   203,   131,   132,   133,     0,   134,   135,   204,   137,
     138,   139,     0,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,   155,
     156,   157,   158,   159,   160,   161,   206,   163,   164,   165,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   207,    67,   527,     0,   209,     0,     0,   210,     0,
       0,   211,     0,     0,    68,    69,    70,   528,   529,     0,
      73,    74,    75,    76,    77,    78,   530,    80,     0,    81,
      82,    83,    84,    85,    86,    87,   531,    89,    90,    91,
      92,    93,    94,     0,   532,    96,   533,     0,    97,    98,
      99,     0,   100,   101,   102,   534,   104,   105,   106,   535,
       0,   107,   108,   109,   110,   536,   112,     0,   113,   114,
     115,   116,     0,   537,   118,   119,   120,     0,   121,   122,
     123,     0,   124,     0,   125,   126,   127,   128,   129,   130,
     131,   132,   133,     0,   134,   135,   136,   137,   138,   139,
       0,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,     0,   155,   156,   157,
     158,   159,   160,   161,   538,   163,   164,   165,     0,    67,
       0,   679,     0,     0,   680,     0,     0,     0,     0,     0,
       0,    68,    69,    70,    71,    72,   539,    73,    74,    75,
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
     161,   162,   163,   164,   165,     0,    67,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    68,    69,
      70,    71,    72,   681,    73,    74,    75,    76,    77,    78,
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
     164,   165,     0,    67,     0,   383,     0,     0,   471,     0,
       0,     0,     0,     0,     0,    68,    69,    70,    71,    72,
     436,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,   472,    97,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
       0,     0,   107,   108,   109,   110,   111,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
     122,   123,   473,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   474,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,    67,
       0,   383,     0,     0,     0,     0,     0,     0,     0,     0,
     475,    68,    69,    70,    71,    72,     0,    73,    74,    75,
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
     161,   162,   163,   164,   165,    67,     0,   343,   344,     0,
     345,     0,     0,     0,     0,     0,   475,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,     0,     0,   107,   108,   109,   110,   111,   112,
       0,   113,   114,   115,   116,     0,   117,   118,   119,   120,
       0,   121,   122,   123,   346,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   134,   135,   136,
     137,   138,   139,     0,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,    67,   527,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,    75,    76,    77,    78,   530,    80,     0,    81,    82,
      83,    84,    85,    86,    87,   531,    89,    90,    91,    92,
      93,    94,     0,   532,    96,   533,     0,    97,    98,    99,
       0,   100,   101,   102,   534,   104,   105,   106,   535,     0,
     107,   108,   109,   110,   536,   112,     0,   113,   114,   115,
     116,     0,   117,   118,   119,   120,     0,   121,   122,   123,
       0,   124,     0,   125,   126,   127,   128,   129,   791,   131,
     132,   133,     0,   134,   135,   136,   137,   138,   139,     0,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,   155,   156,   157,   158,
     159,   160,   161,   538,   163,   164,   165,    67,   527,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    68,
      69,    70,    71,    72,     0,    73,    74,    75,    76,    77,
      78,   530,    80,     0,    81,    82,    83,    84,    85,    86,
      87,   531,    89,    90,    91,    92,    93,    94,     0,   532,
      96,   533,     0,    97,    98,    99,     0,   100,   101,   102,
     534,   104,   105,   106,   535,     0,   107,   108,   109,   110,
     536,   112,     0,   113,   114,   115,   116,     0,   117,   118,
     119,   120,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   130,   131,   132,   133,     0,   134,
     135,   136,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,   155,   156,   157,   158,   159,   160,   161,   538,
     163,   164,   165,    67,   527,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    68,    69,    70,   528,   529,
       0,    73,    74,    75,    76,    77,    78,   530,    80,     0,
      81,    82,    83,    84,    85,    86,    87,   531,    89,    90,
      91,    92,    93,    94,     0,   532,    96,   533,     0,    97,
      98,    99,     0,   100,   101,   102,   534,   104,   105,   106,
     535,     0,   107,   108,   109,   110,   536,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
     122,   123,     0,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,     0,   155,   156,
     157,   158,   159,   160,   161,   538,   163,   164,   165,    67,
       0,   585,     0,     0,   471,     0,     0,     0,     0,     0,
       0,    68,    69,    70,    71,    72,     0,    73,    74,    75,
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
     161,   162,   163,   164,   165,    67,     0,   828,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    68,    69,    70,
      71,    72,     0,    73,    74,    75,    76,    77,    78,    79,
      80,     0,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,     0,    95,    96,     0,
       0,    97,    98,    99,     0,   100,   101,   102,   103,   104,
     105,   106,   829,     0,   107,   108,   109,   110,   111,   112,
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
     101,   102,   103,   104,   105,   106,     0,   299,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,    67,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,    68,    69,    70,    71,    72,
     310,    73,    74,    75,    76,    77,    78,    79,    80,     0,
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
       0,     0,     0,     0,   352,     0,     0,     0,     0,     0,
       0,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,     0,    97,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,     0,     0,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   123,     0,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   353,   148,   149,   150,   151,
     152,   153,   154,    67,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,    68,    69,    70,    71,    72,
       0,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
     610,     0,   107,   108,   109,   110,   111,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
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
     111,   112,     0,   113,   114,   115,   116,   803,   117,   118,
     119,   120,     0,   121,   122,   123,     0,   124,     0,   125,
     126,   127,   128,   129,   130,   131,   132,   133,     0,   134,
     135,   136,   137,   138,   139,     0,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,    67,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    68,    69,    70,    71,    72,     0,    73,
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
     159,   160,   161,   162,   163,   164,   165,    67,     0,     0,
       0,     0,     0,     0,   226,     0,     0,     0,     0,    68,
      69,    70,    71,    72,     0,    73,    74,    75,    76,    77,
      78,    79,    80,     0,    81,   227,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,    95,
      96,     0,     0,    97,    98,    99,     0,   100,   228,   102,
     103,   104,   105,   106,     0,     0,   229,   108,   109,   110,
     111,   112,     0,   113,   114,   115,   116,     0,   117,   118,
     119,   120,     0,   121,   122,   230,     0,   124,     0,   125,
     126,   127,   128,   129,   130,   131,   132,   133,     0,   134,
     135,   136,   137,   138,   139,     0,   140,   141,   142,   231,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
       0,     0,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,   165,    67,     0,     0,     0,     0,     0,   340,
       0,     0,     0,     0,     0,    68,    69,    70,    71,    72,
       0,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,    95,    96,     0,     0,    97,
      98,    99,     0,   100,   101,   102,   103,   104,   105,   106,
       0,     0,   107,   108,   109,   110,   111,   112,     0,   113,
     114,   115,   116,     0,   117,   118,   119,   120,     0,   121,
     122,     0,     0,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,   134,   135,   136,   137,   138,
     139,     0,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,  -466,     0,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,    67,
       0,     0,     0,     0,     0,     0,   226,     0,     0,     0,
       0,    68,    69,    70,    71,    72,     0,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,   227,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,    96,     0,   313,    97,    98,    99,     0,   100,
     101,   102,   103,   104,   105,   106,     0,     0,   107,   108,
     109,   110,   111,   112,     0,   113,   114,   115,   116,     0,
     117,   118,   119,   120,     0,   121,   122,   230,   314,   124,
       0,   125,   126,   127,   128,   129,   130,   131,   132,   133,
       0,   134,   135,   136,   137,   138,   139,     0,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   313,     0,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,     0,     0,     0,     0,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,     0,     1,   314,     0,     0,   768,
       2,     0,     0,     3,     0,     0,     0,     0,     4,     0,
       0,     0,     0,     5,     0,     0,     6,     0,     7,     8,
       9,     0,     0,     0,    10,     0,     0,     0,    11,     0,
     313,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,     0,     0,     0,     0,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,     0,    13,   314,     0,   429,   313,     0,     0,
       0,     0,    14,     0,    15,     0,     0,    16,     0,    17,
       0,    18,     0,     0,    19,     0,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    21,   313,     0,    22,
       0,   314,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   315,   316,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
       0,   314,     0,   313,   554,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   314,   313,     0,
       0,   560,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,     0,     0,   313,
       0,   569,   314,     0,     0,     0,   581,     0,     0,     0,
       0,     0,     0,     0,   313,     0,     0,     0,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   314,   313,     0,     0,   766,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   313,   314,     0,
       0,     0,     0,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   314,     0,
       0,     0,   767,     0,     0,     0,     0,     0,     0,     0,
     313,   314,     0,     0,   315,   316,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   314,     0,     0,     0,     0,     0,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329
};

static const yytype_int16 yycheck[] =
{
       4,   279,   274,   281,   282,    18,   518,   285,   286,   607,
     367,   289,   221,    22,     0,    44,   539,    29,    22,   584,
     240,    13,    14,   601,     5,   245,   604,   247,     5,    83,
      44,     8,    47,    52,     5,    36,    40,     8,    43,   396,
       5,     5,   262,     9,    52,     5,     6,   693,     8,    78,
      13,    14,    62,   103,    14,     9,     5,     6,    44,     8,
      64,    65,    66,   641,    78,    14,   644,   645,   646,   647,
     648,   649,    20,   126,     5,     6,    68,     5,     6,    13,
      14,    26,   437,    14,     5,     6,    14,     8,     5,     6,
      52,     8,    78,    14,   877,   149,   149,    14,    44,   149,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   126,    61,   640,    73,   120,
      62,   686,   116,   906,   153,   139,   140,   141,   142,   143,
     149,   146,    78,   353,    72,    17,   141,   149,   149,   100,
      14,   149,   497,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    32,   140,
     152,   170,   371,   140,   168,   169,   170,   171,   172,   140,
     735,   175,   176,   177,   178,   140,   140,   148,    26,    67,
     140,   147,   539,   148,   148,   697,    53,   147,   126,   152,
     755,   140,   838,   147,   149,   141,   142,   143,   147,   133,
     148,   149,    44,    62,   716,    82,    26,   211,   854,   140,
      82,   215,   140,    61,   779,   228,   147,   221,   152,   140,
      87,    53,    15,   140,    21,   229,   230,    27,   266,    88,
     234,   877,    72,   130,    23,   109,    78,   815,   816,   147,
      26,    61,   100,     0,    41,   602,    52,   251,    68,    49,
     254,   148,   147,    46,   112,    87,   147,   261,   823,   905,
     906,   481,   827,   483,   147,    65,    66,   144,    92,   109,
      59,    95,   144,    68,   631,    61,   633,   147,   102,   636,
     101,   147,   103,   640,   147,   106,   884,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,    91,   149,   308,   151,   310,   149,     5,     6,
     110,   100,   350,   133,   134,   135,   136,   137,   138,    44,
     148,   149,   148,   149,   144,   207,    68,   209,   210,   148,
     149,   603,   148,   149,   338,   339,   693,   148,   149,   148,
     149,   148,   149,   147,   856,   147,   350,   133,   134,   135,
     136,   137,   138,    78,   148,   149,   147,   361,   144,   338,
     339,   374,   147,   367,   368,   148,   149,   371,   148,   149,
     148,   149,   148,   149,   148,   149,   258,   148,   149,   148,
     149,   148,   149,   148,   149,   148,   149,   148,   149,   148,
     149,   147,   396,   147,   432,   147,   278,   148,   149,   148,
     149,   147,   284,    16,   147,   287,   288,   411,   290,   291,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   147,
     777,   148,   149,   305,   147,   147,   430,   309,   432,   147,
     147,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,    62,
     133,   133,   103,   116,    16,   337,   133,   116,   103,    83,
      72,   123,    93,    76,   468,   115,   470,   119,    98,    31,
     119,    33,    52,    86,   107,    88,    89,    39,   148,   148,
     518,   838,   147,   840,     5,   133,   147,   147,    10,   493,
     133,   373,    54,   133,    54,    69,   109,   854,   140,   112,
      62,   114,   506,   109,   508,   509,   510,   109,     8,   147,
      34,   124,   100,   149,    76,    34,    27,    28,     8,     8,
     877,     8,   404,     8,    86,   133,    88,    89,   148,    82,
     153,   149,   147,     9,     5,   539,    71,   133,    49,   149,
     104,     5,     8,   105,   133,   427,   428,   109,   905,   906,
     112,   113,   114,    44,    65,    66,   834,   104,    84,    70,
     147,   122,   124,   601,   148,   107,   604,   107,    79,    80,
     147,   147,   147,   130,   147,   147,   147,   581,   147,   147,
     147,   619,   148,   148,   133,   101,    55,    78,   592,   593,
       5,   129,   133,    29,   133,    73,   147,   149,   602,   110,
     127,     8,     8,   641,   149,     8,   644,   645,   646,   647,
     648,   649,   141,   651,   148,   653,   654,   655,   656,   657,
     658,   503,     8,   505,   147,   629,   630,   631,   666,   633,
     148,   148,   636,     5,   149,   907,   640,   915,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,    44,   148,   128,    61,   148,   149,   697,
      92,   147,   666,   147,     8,   118,   148,   148,   550,   148,
     130,   148,   130,   148,   148,   130,   148,   681,   716,   147,
      82,   685,   564,   148,   566,   147,   568,    78,    84,   693,
      29,    55,   696,    58,   133,     8,   147,   149,     3,    20,
     130,   130,    44,   149,    29,    73,    84,   148,   148,   133,
     148,   715,   149,     8,   752,     5,   149,    29,   149,     8,
     147,   149,     5,   148,   762,   404,   259,   875,   578,   840,
      44,   667,   823,   841,   854,   848,    78,   593,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   309,   553,   430,   666,   148,   149,   468,
     764,   696,   470,   800,    78,   894,   716,   602,   518,    44,
     685,    19,   503,   777,   332,   777,   306,   815,   816,   505,
      -1,    -1,   319,    -1,    -1,   667,   824,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,    -1,    78,    -1,    -1,    -1,   149,   846,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   856,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     868,   835,    -1,    -1,   838,   873,   840,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   848,    -1,    -1,    -1,    -1,    -1,
     854,    -1,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,    -1,
      -1,   875,    -1,   877,   149,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,     8,     9,    10,
      -1,   905,   906,    14,    15,    16,    17,    18,    19,    -1,
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
     121,   122,   123,   124,   125,   126,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
     141,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,   150,
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
     123,   124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,   148,    -1,   150,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
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
     125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,   144,
      -1,    -1,   147,   148,    -1,   150,     3,     4,     5,     6,
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
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,   141,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,   150,     3,     4,     5,     6,    -1,     8,
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
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,   141,    -1,    -1,   144,    -1,    -1,   147,    -1,
      -1,   150,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    -1,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    74,    75,    76,    77,    -1,    79,    80,
      81,    -1,    83,    -1,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    -1,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,    -1,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,
      -1,    -1,    -1,   144,    -1,    -1,   147,    -1,    -1,   150,
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
     123,   124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,
      -1,   144,    -1,    -1,   147,   148,    -1,   150,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
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
     125,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   140,    -1,    -1,    -1,   144,
      -1,    -1,   147,    -1,    -1,   150,     3,     4,     5,     6,
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
     127,   128,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   140,    -1,    -1,    -1,   144,    -1,    -1,
     147,    -1,    -1,   150,     3,     4,     5,     6,    -1,     8,
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
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   140,     3,     4,    -1,   144,    -1,    -1,   147,    -1,
      -1,   150,    -1,    -1,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    -1,    45,    46,    47,    -1,    49,    50,
      51,    -1,    53,    54,    55,    56,    57,    58,    59,    60,
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
      -1,    45,    46,    -1,    -1,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,     3,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,
      17,    18,    19,   147,    21,    22,    23,    24,    25,    26,
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
     127,   128,    -1,     3,    -1,     5,    -1,    -1,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
     147,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    48,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    81,    82,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,     3,
      -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     140,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
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
     124,   125,   126,   127,   128,     3,    -1,     5,     6,    -1,
       8,    -1,    -1,    -1,    -1,    -1,   140,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    -1,    45,    46,    -1,
      -1,    49,    50,    51,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    62,    63,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    74,    75,    76,    77,
      -1,    79,    80,    81,    82,    83,    -1,    85,    86,    87,
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
     122,   123,   124,   125,   126,   127,   128,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,
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
      -1,     5,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,
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
     114,   115,   116,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,     3,    -1,     5,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
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
      54,    55,    56,    57,    58,    59,    -1,    61,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    -1,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,     3,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
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
      -1,    -1,    -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,
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
     114,   115,   116,     3,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
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
      66,    67,    -1,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    -1,    79,    80,    81,    -1,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    -1,    95,
      96,    97,    98,    99,   100,    -1,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,     3,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,    15,    16,    17,    18,    19,    -1,    21,
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
     122,   123,   124,   125,   126,   127,   128,     3,    -1,    -1,
      -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,    -1,    15,
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
      -1,    -1,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,     3,    -1,    -1,    -1,    -1,    -1,     9,
      -1,    -1,    -1,    -1,    -1,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    -1,    45,    46,    -1,    -1,    49,
      50,    51,    -1,    53,    54,    55,    56,    57,    58,    59,
      -1,    -1,    62,    63,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    74,    75,    76,    77,    -1,    79,
      80,    -1,    -1,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    -1,    95,    96,    97,    98,    99,
     100,    -1,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,    -1,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    10,    -1,    -1,    -1,
      -1,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    -1,    44,    49,    50,    51,    -1,    53,
      54,    55,    56,    57,    58,    59,    -1,    -1,    62,    63,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      74,    75,    76,    77,    -1,    79,    80,    81,    78,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      -1,    95,    96,    97,    98,    99,   100,    -1,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    44,    -1,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,    -1,    -1,    -1,    -1,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    -1,    17,    78,    -1,    -1,   149,
      22,    -1,    -1,    25,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    35,    -1,    -1,    38,    -1,    40,    41,
      42,    -1,    -1,    -1,    46,    -1,    -1,    -1,    50,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    -1,    -1,    -1,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,    -1,    85,    78,    -1,   148,    44,    -1,    -1,
      -1,    -1,    94,    -1,    96,    -1,    -1,    99,    -1,   101,
      -1,   103,    -1,    -1,   106,    -1,   108,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   118,    44,    -1,   121,
      -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
      -1,    78,    -1,    44,   148,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,    78,    44,    -1,
      -1,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,    -1,    -1,    44,
      -1,   148,    78,    -1,    -1,    -1,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,    78,    44,    -1,    -1,   148,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    44,    78,    -1,
      -1,    -1,    -1,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,    78,    -1,
      -1,    -1,   148,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    78,    -1,    -1,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,    78,    -1,    -1,    -1,    -1,    -1,
     130,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    17,    22,    25,    30,    35,    38,    40,    41,    42,
      46,    50,    64,    85,    94,    96,    99,   101,   103,   106,
     108,   118,   121,   155,   156,   159,   160,   161,   162,   169,
     212,   217,   218,   223,   224,   225,   226,   237,   238,   247,
     248,   249,   250,   251,   255,   256,   261,   262,   263,   266,
     268,   269,   271,   272,   273,   274,   275,   279,   280,   281,
     282,   283,   284,   298,   100,   112,    62,     3,    15,    16,
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
     147,   150,   158,   170,   171,   173,   204,   205,   208,   276,
     277,   278,   285,   286,   289,   290,    10,    31,    54,    62,
      81,   105,   157,   264,    16,    31,    33,    39,    54,    62,
      76,    86,    88,    89,   105,   109,   112,   113,   114,   124,
     214,   215,   264,   116,   100,   158,   174,     0,    47,   146,
     296,    67,   158,   158,   158,   158,   147,   158,   158,   174,
     158,   158,   158,   158,   158,   158,    52,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   147,
     147,   147,   147,     3,    75,    77,   204,   204,   204,    61,
     158,   209,   210,    13,    14,   152,   287,   288,    52,   149,
      20,   158,   172,    44,    78,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     147,    73,   149,   196,   197,   172,    68,   133,   133,   103,
       9,   157,   158,     5,     6,     8,    82,   158,   221,   222,
     133,   116,     8,   109,   158,   103,    72,   213,   213,   213,
     213,    83,   198,   199,   158,   123,   158,   103,   149,   204,
     293,   294,   295,   101,   103,   106,   160,   158,   213,    93,
      15,    46,   115,     5,     6,     8,    14,   140,   147,   182,
     183,   233,   239,   240,    98,   119,   126,   176,   119,   198,
      52,   297,   107,     8,   147,   148,   204,   206,   207,   204,
     206,    43,   141,   206,   206,   148,   204,   206,   206,   204,
     204,   148,   206,   204,   204,   148,   147,   147,   147,   148,
     149,   151,   133,     8,   204,   288,   147,   174,   171,   204,
     158,   204,   204,   204,   204,   204,   204,   204,   290,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
       5,   277,    82,   144,   204,   221,   221,   133,   133,    54,
     140,     8,    48,    82,   117,   140,   158,   182,   219,   220,
      69,   109,   213,   109,     8,   158,   200,   201,    14,    32,
     109,   216,    72,   126,   257,   258,   158,   252,   253,   286,
     158,   172,    29,   149,   291,   292,   147,   227,    34,    34,
     100,     5,     6,     8,   148,   182,   184,   241,   149,   242,
      27,    49,    65,    66,   110,   267,     8,     4,    18,    19,
      27,    37,    45,    47,    56,    60,    66,    74,   125,   147,
     158,   177,   178,   179,   180,   181,   286,     8,     8,     8,
     101,   162,   148,   149,   148,   148,   158,   148,   148,   148,
     148,   148,   148,   148,   149,   148,   149,   148,   149,   148,
     204,   204,   210,   158,   182,   211,   153,   153,   166,   175,
     176,    51,   149,    82,   147,     5,   219,     9,   222,    71,
     213,   213,   133,   149,   104,     5,   104,     8,   158,   259,
     260,   133,   149,   176,   133,   293,    84,   192,   193,   295,
      60,   158,   228,   229,   122,   158,   158,   158,   148,   149,
     148,   149,   233,   240,   243,   244,   148,   107,   107,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   177,
     130,    26,    61,    68,   133,   134,   135,   136,   137,   138,
     144,    26,    61,   133,   134,   135,   136,   137,   138,   144,
     148,   207,   148,   204,   204,   204,   133,   101,   169,    55,
     186,   158,     5,   184,   133,    92,    95,   102,   265,     5,
       8,   147,   158,   201,   133,   129,   147,   182,   183,   253,
     198,   182,   183,    29,   196,   148,   149,   147,   230,   231,
      27,    28,    49,    65,    66,    70,    79,    80,   110,   254,
     127,   270,   182,     8,    20,   245,   149,     8,     8,   158,
     158,   158,   286,   141,   286,   148,   148,   286,     8,   148,
     148,   177,   182,   183,     9,   147,    82,   144,     8,   182,
     183,     8,   182,   183,   182,   183,   182,   183,   182,   183,
     182,   183,    26,    61,   182,   147,   182,   182,   182,   182,
     182,   182,    26,    61,   149,   163,   148,   148,   149,   211,
     170,   148,     5,   187,   128,   190,   191,    61,   148,   147,
      36,   120,    92,   158,   202,   203,   147,     8,   260,   148,
     184,    90,   180,   194,   195,   228,   147,   232,   233,    83,
     149,   234,   118,    73,   158,   246,   233,   244,   148,   148,
     148,   148,   148,   148,   148,   130,   130,     8,   184,   185,
      82,   182,     9,   147,   130,   184,   182,   147,     5,    60,
     158,   164,   165,   148,   147,    52,    84,   167,    29,    55,
      58,   189,   158,   285,   286,   184,   133,   148,   149,     8,
     148,   147,    21,    41,   149,   148,   149,     3,   235,   236,
     231,   182,   183,   182,   183,   148,   148,   149,   130,   184,
     185,   182,   148,   130,   184,   149,   206,    29,    73,   168,
     180,   188,    84,   179,   192,   148,   148,   182,   203,   148,
     148,   195,   233,   133,   149,     8,   182,   148,   148,   182,
     148,   165,   148,   194,     5,   149,    29,   196,     8,   236,
     149,   149,   180,   194,   198,   147,     5,   206,   148,   148
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
     177,   178,   178,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   181,   181,   182,   182,   183,   183,   183,   184,   184,
     185,   185,   186,   186,   187,   187,   188,   188,   189,   189,
     190,   190,   191,   192,   192,   193,   193,   194,   194,   195,
     195,   195,   196,   196,   197,   197,   198,   198,   199,   200,
     200,   201,   201,   201,   201,   201,   201,   202,   202,   203,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   206,   206,   207,
     207,   208,   208,   208,   209,   209,   210,   210,   211,   211,
     212,   213,   213,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   215,   215,   216,   216,
     217,   217,   217,   217,   217,   217,   218,   218,   218,   218,
     219,   219,   220,   220,   220,   221,   221,   222,   222,   222,
     222,   222,   223,   223,   223,   224,   224,   225,   226,   226,
     227,   227,   228,   228,   229,   229,   230,   230,   231,   232,
     232,   233,   233,   233,   233,   233,   234,   234,   235,   235,
     236,   237,   238,   239,   239,   240,   240,   241,   241,   242,
     242,   243,   243,   244,   245,   245,   246,   246,   247,   248,
     248,   249,   250,   251,   252,   252,   253,   253,   253,   253,
     253,   253,   254,   254,   254,   254,   254,   254,   254,   254,
     254,   255,   255,   255,   256,   256,   257,   257,   258,   259,
     259,   260,   261,   262,   263,   264,   264,   264,   265,   265,
     265,   265,   266,   267,   267,   267,   267,   267,   268,   269,
     270,   270,   271,   272,   273,   274,   275,   276,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   285,   286,
     287,   287,   288,   288,   288,   288,   289,   289,   290,   291,
     291,   292,   293,   294,   295,   295,   296,   297,   297,   298
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
       4,     3,     5,     1,     3,     0,     1,     2,     1,     3,
       3,     4,     1,     3,     3,     5,     6,     5,     6,     3,
       4,     5,     6,     3,     3,     3,     3,     3,     3,     5,
       5,     5,     3,     3,     3,     3,     3,     3,     3,     4,
       3,     3,     5,     6,     5,     6,     3,     3,     3,     3,
       1,     1,     4,     3,     3,     1,     1,     4,     4,     4,
       3,     4,     4,     1,     2,     1,     2,     1,     1,     3,
       1,     3,     0,     4,     0,     1,     1,     3,     0,     2,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     1,     3,     3,     5,     6,     3,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     3,     4,     4,     4,     4,     4,     4,     4,     3,
       6,     6,     3,     8,    14,     3,     4,     1,     3,     1,
       1,     1,     1,     1,     3,     5,     1,     1,     1,     1,
       2,     0,     2,     1,     2,     2,     3,     1,     1,     1,
       2,     4,     4,     3,     4,     4,     1,     1,     0,     2,
       4,     4,     4,     3,     4,     4,     7,     5,     5,     9,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     7,     1,     1,
       0,     3,     1,     1,     1,     3,     1,     3,     3,     1,
       3,     1,     1,     1,     3,     2,     0,     2,     1,     3,
       3,     4,     6,     1,     3,     1,     3,     1,     3,     0,
       2,     1,     3,     3,     0,     1,     1,     1,     3,     1,
       1,     3,     3,     6,     1,     3,     3,     3,     5,     4,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     7,     6,     4,     4,     5,     0,     1,     2,     1,
       3,     3,     2,     3,     6,     0,     1,     1,     2,     2,
       2,     1,     7,     1,     1,     1,     1,     1,     3,     7,
       0,     2,     3,     3,     2,     2,     3,     1,     3,     2,
       1,     2,     3,     4,     7,     5,     5,     1,     1,     2,
       1,     2,     1,     1,     3,     3,     3,     3,     1,     0,
       2,     1,     2,     1,     1,     3,     5,     0,     2,     4
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

  case 183:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 184:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-2]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( (yyvsp[0]).m_iValue );
			pFilter->m_bExclude = true;
		}

    break;

  case 185:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-4]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		}

    break;

  case 186:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( (yyvsp[-5]) );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
			pFilter->m_bExclude = true;
		}

    break;

  case 187:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-4]), (yyvsp[-1]), false ) )
				YYERROR;
		}

    break;

  case 188:

    {
			if ( !pParser->AddStringListFilter ( (yyvsp[-5]), (yyvsp[-1]), true ) )
				YYERROR;
		}

    break;

  case 189:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 190:

    {
			if ( !pParser->AddUservarFilter ( (yyvsp[-3]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 191:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 192:

    {
			if ( !pParser->AddIntRangeFilter ( (yyvsp[-5]), (yyvsp[-3]).m_iValue, (yyvsp[-1]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 193:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 194:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, false ) )
				YYERROR;
		}

    break;

  case 195:

    {
			if ( !pParser->AddIntFilterGreater ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 196:

    {
			if ( !pParser->AddIntFilterLesser ( (yyvsp[-2]), (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 197:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 198:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, (yyvsp[0]).m_fValue, true, true ) )
				YYERROR;
		}

    break;

  case 199:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 200:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 201:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-4]), (yyvsp[-2]).m_fValue, (yyvsp[0]).m_iValue, true ) )
				YYERROR;
		}

    break;

  case 202:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, false ) )
				YYERROR;
		}

    break;

  case 203:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, false ) )
				YYERROR;
		}

    break;

  case 204:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), (yyvsp[0]).m_fValue, FLT_MAX, true ) )
				YYERROR;
		}

    break;

  case 205:

    {
			if ( !pParser->AddFloatRangeFilter ( (yyvsp[-2]), -FLT_MAX, (yyvsp[0]).m_fValue, true ) )
				YYERROR;
		}

    break;

  case 206:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), false ) )
				YYERROR;
		}

    break;

  case 207:

    {
			if ( !pParser->AddStringFilter ( (yyvsp[-2]), (yyvsp[0]), true ) )
				YYERROR;
		}

    break;

  case 208:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-2]), true ) )
				YYERROR;
		}

    break;

  case 209:

    {
			if ( !pParser->AddNullFilter ( (yyvsp[-3]), false ) )
				YYERROR;
		}

    break;

  case 210:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-2]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-2]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues.Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 211:

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

  case 212:

    {
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-4]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-4]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 213:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_dValues = *(yyvsp[-1]).m_pValues.Ptr();
			f->m_dValues.Uniq();
		}

    break;

  case 214:

    {
			AddMvaRange ( pParser, (yyvsp[-4]), (yyvsp[-2]).m_iValue, (yyvsp[0]).m_iValue );
		}

    break;

  case 215:

    {
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( (yyvsp[-5]), SPH_FILTER_RANGE );
			f->m_eMvaFunc = ( (yyvsp[-5]).m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_iMinValue = (yyvsp[-2]).m_iValue;
			f->m_iMaxValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 216:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue-1 );
		}

    break;

  case 217:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue+1, INT64_MAX );
		}

    break;

  case 218:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), INT64_MIN, (yyvsp[0]).m_iValue );
		}

    break;

  case 219:

    {
			AddMvaRange ( pParser, (yyvsp[-2]), (yyvsp[0]).m_iValue, INT64_MAX );
		}

    break;

  case 221:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		}

    break;

  case 222:

    {
			(yyval).m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 223:

    {
			(yyval).m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 224:

    {
			(yyval).m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 225:

    {
			(yyval).m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}

    break;

  case 227:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 228:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 229:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 231:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ANY; }

    break;

  case 232:

    { (yyval) = (yyvsp[-1]); (yyval).m_iType = TOK_ALL; }

    break;

  case 233:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 234:

    {
			(yyval).m_iType = TOK_CONST_INT;
			if ( (uint64_t)(yyvsp[0]).m_iValue > (uint64_t)LLONG_MAX )
				(yyval).m_iValue = LLONG_MIN;
			else
				(yyval).m_iValue = -(yyvsp[0]).m_iValue;
		}

    break;

  case 235:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 236:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = -(yyvsp[0]).m_fValue; }

    break;

  case 237:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 238:

    {
			assert ( !(yyval).m_pValues.Ptr() );
			(yyval).m_pValues = new RefcountedVector_c<SphAttr_t> ();
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue ); 
		}

    break;

  case 239:

    {
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 240:

    {
			assert ( !(yyval).m_pValues.Ptr() );
			(yyval).m_pValues = new RefcountedVector_c<SphAttr_t> ();
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 241:

    {
			(yyval).m_pValues->Add ( (yyvsp[0]).m_iValue );
		}

    break;

  case 245:

    {
			pParser->SetGroupbyLimit ( (yyvsp[0]).m_iValue );
		}

    break;

  case 246:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 247:

    {
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 249:

    {
			pParser->AddHaving();
		}

    break;

  case 252:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, (yyvsp[0]) );
		}

    break;

  case 255:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, (yyvsp[0]) );
		}

    break;

  case 256:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		}

    break;

  case 258:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 260:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 261:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 264:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 265:

    {
			pParser->m_pQuery->m_iOffset = (yyvsp[-2]).m_iValue;
			pParser->m_pQuery->m_iLimit = (yyvsp[0]).m_iValue;
		}

    break;

  case 271:

    {
			if ( !pParser->AddOption ( (yyvsp[0]) ) )
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
			if ( !pParser->AddOption ( (yyvsp[-2]), (yyvsp[0]) ) )
				YYERROR;
		}

    break;

  case 274:

    {
			if ( !pParser->AddOption ( (yyvsp[-4]), pParser->GetNamedVec ( (yyvsp[-1]).m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( (yyvsp[-1]).m_iValue );
		}

    break;

  case 275:

    {
			if ( !pParser->AddOption ( (yyvsp[-5]), (yyvsp[-3]), (yyvsp[-1]) ) )
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
			(yyval).m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 278:

    {
			pParser->AddConst( (yyval).m_iValue, (yyvsp[0]) );
		}

    break;

  case 279:

    {
			(yyval) = (yyvsp[-2]);
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 281:

    { if ( !pParser->SetOldSyntax() ) YYERROR; }

    break;

  case 282:

    { if ( !pParser->SetNewSyntax() ) YYERROR; }

    break;

  case 287:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

    break;

  case 288:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-1]), (yyvsp[0]) ); }

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

  case 307:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 311:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

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

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 319:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 320:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 321:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-5]), (yyvsp[0]) ); }

    break;

  case 322:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 323:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-7]), (yyvsp[0]) ); }

    break;

  case 324:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-13]), (yyvsp[0]) ); }

    break;

  case 325:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 326:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-3]), (yyvsp[0]) ); }

    break;

  case 334:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 335:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-4]), (yyvsp[0]) ); }

    break;

  case 342:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) ); }

    break;

  case 343:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; }

    break;

  case 344:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; }

    break;

  case 345:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; }

    break;

  case 346:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; }

    break;

  case 347:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; }

    break;

  case 348:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; }

    break;

  case 349:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; }

    break;

  case 350:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; }

    break;

  case 351:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 352:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 353:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 354:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
		}

    break;

  case 355:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-2]) );
			pParser->m_pStmt->m_iIntParam = int((yyvsp[-1]).m_fValue*10);
		}

    break;

  case 359:

    {
			pParser->m_pStmt->m_iIntParam = (yyvsp[0]).m_iValue;
		}

    break;

  case 360:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 361:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) );
		}

    break;

  case 362:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		}

    break;

  case 363:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 364:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 365:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }

    break;

  case 366:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
		}

    break;

  case 367:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, (yyvsp[0]) ).Unquote();
		}

    break;

  case 368:

    {
			pParser->SetStatement ( (yyvsp[-2]), SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = (yyvsp[0]).m_iValue;
		}

    break;

  case 369:

    {
			pParser->SetStatement ( (yyvsp[-4]), SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *(yyvsp[-1]).m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-6]) );
		}

    break;

  case 372:

    { (yyval).m_iValue = 1; }

    break;

  case 373:

    { (yyval).m_iValue = 0; }

    break;

  case 374:

    {
			(yyval).m_iValue = (yyvsp[0]).m_iValue;
			if ( (yyval).m_iValue!=0 && (yyval).m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		}

    break;

  case 382:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; }

    break;

  case 383:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; }

    break;

  case 384:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; }

    break;

  case 387:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-4]) );
		}

    break;

  case 388:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; }

    break;

  case 389:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; }

    break;

  case 394:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 395:

    { if ( !pParser->AddSchemaItem ( &(yyvsp[0]) ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }

    break;

  case 398:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } }

    break;

  case 399:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 400:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) ); }

    break;

  case 401:

    { (yyval).m_iType = TOK_CONST_INT; (yyval).m_iValue = (yyvsp[0]).m_iValue; }

    break;

  case 402:

    { (yyval).m_iType = TOK_CONST_FLOAT; (yyval).m_fValue = (yyvsp[0]).m_fValue; }

    break;

  case 403:

    { (yyval).m_iType = TOK_QUOTED_STRING; (yyval).m_iStart = (yyvsp[0]).m_iStart; (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 404:

    { (yyval).m_iType = TOK_CONST_MVA; (yyval).m_pValues = (yyvsp[-1]).m_pValues; }

    break;

  case 405:

    { (yyval).m_iType = TOK_CONST_MVA; }

    break;

  case 410:

    { if ( !pParser->AddInsertOption ( (yyvsp[-2]), (yyvsp[0]) ) ) YYERROR; }

    break;

  case 411:

    {
		if ( !pParser->DeleteStatement ( &(yyvsp[-1]) ) )
			YYERROR;
	}

    break;

  case 412:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, (yyvsp[-4]) );
		}

    break;

  case 413:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 414:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, (yyvsp[0]) );
		}

    break;

  case 416:

    {
			(yyval).m_iType = TOK_CONST_STRINGS;
		}

    break;

  case 417:

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

  case 418:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), (yyvsp[0]) );
		}

    break;

  case 421:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		}

    break;

  case 423:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), (yyvsp[0]) );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, (yyvsp[-2]) );
		}

    break;

  case 428:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 431:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; }

    break;

  case 432:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; }

    break;

  case 433:

    {
			if ( !pParser->UpdateStatement ( &(yyvsp[-4]) ) )
				YYERROR;
		}

    break;

  case 436:

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

  case 437:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( (yyvsp[0]).m_fValue ) );
			pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_FLOAT );
		}

    break;

  case 438:

    {
			pParser->UpdateMVAAttr ( (yyvsp[-4]), (yyvsp[-1]) );
		}

    break;

  case 439:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( (yyvsp[-3]), tNoValues );
		}

    break;

  case 440:

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

  case 441:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( (yyvsp[0]).m_fValue ) );
			pParser->AddUpdatedAttr ( (yyvsp[-2]), SPH_ATTR_FLOAT );
		}

    break;

  case 442:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 443:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 444:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 445:

    { (yyval).m_iValue = SPH_ATTR_BOOL; }

    break;

  case 446:

    { (yyval).m_iValue = SPH_ATTR_UINT32SET; }

    break;

  case 447:

    { (yyval).m_iValue = SPH_ATTR_INT64SET; }

    break;

  case 448:

    { (yyval).m_iValue = SPH_ATTR_JSON; }

    break;

  case 449:

    { (yyval).m_iValue = SPH_ATTR_STRING; }

    break;

  case 450:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 451:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-4]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[-1]) );
			tStmt.m_eAlterColType = (ESphAttr)(yyvsp[0]).m_iValue;
		}

    break;

  case 452:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-3]) );
			pParser->ToString ( tStmt.m_sAlterAttr, (yyvsp[0]) );
		}

    break;

  case 453:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 454:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		}

    break;

  case 455:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 462:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		}

    break;

  case 463:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		}

    break;

  case 464:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		}

    break;

  case 472:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, (yyvsp[0]) );
			tStmt.m_eUdfType = (ESphAttr) (yyvsp[-2]).m_iValue;
		}

    break;

  case 473:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 474:

    { (yyval).m_iValue = SPH_ATTR_BIGINT; }

    break;

  case 475:

    { (yyval).m_iValue = SPH_ATTR_FLOAT; }

    break;

  case 476:

    { (yyval).m_iValue = SPH_ATTR_STRINGPTR; }

    break;

  case 477:

    { (yyval).m_iValue = SPH_ATTR_INTEGER; }

    break;

  case 478:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, (yyvsp[0]) );
		}

    break;

  case 479:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-4]) );
			pParser->ToString ( tStmt.m_sStringParam, (yyvsp[-1]) );
		}

    break;

  case 481:

    {
			pParser->m_pStmt->m_iIntParam = 1;
		}

    break;

  case 482:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 483:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 484:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		}

    break;

  case 485:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_HOSTNAMES;
		}

    break;

  case 486:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[-1]) );
		}

    break;

  case 490:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 491:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, (yyvsp[0]) );
		}

    break;

  case 492:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[0]) );
		}

    break;

  case 493:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, (yyvsp[-1]) );
		}

    break;

  case 494:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-4]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 495:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, (yyvsp[-2]) );
			pParser->ToStringUnescape ( s.m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 496:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			pParser->ToStringUnescape ( s.m_sUdfLib, (yyvsp[0]) );
		}

    break;

  case 499:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 501:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 502:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 503:

    { (yyval) = (yyvsp[0]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 504:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 505:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 506:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 507:

    { TRACK_BOUNDS ( (yyval), (yyvsp[-2]), (yyvsp[0]) ); }

    break;

  case 511:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		}

    break;

  case 513:

    {
			pParser->AddItem ( &(yyvsp[0]) );
			pParser->AddGroupBy ( (yyvsp[0]) );
		}

    break;

  case 516:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
		}

    break;

  case 518:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, (yyvsp[0]) );
		}

    break;

  case 519:

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
