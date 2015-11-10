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
   by #include "yysphinxexpr.h".  */
#ifndef YY_YY_YYSPHINXEXPR_H_INCLUDED
# define YY_YY_YYSPHINXEXPR_H_INCLUDED
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
    TOK_CONST_INT = 258,
    TOK_CONST_FLOAT = 259,
    TOK_CONST_STRING = 260,
    TOK_SUBKEY = 261,
    TOK_DOT_NUMBER = 262,
    TOK_ATTR_INT = 263,
    TOK_ATTR_BITS = 264,
    TOK_ATTR_FLOAT = 265,
    TOK_ATTR_MVA32 = 266,
    TOK_ATTR_MVA64 = 267,
    TOK_ATTR_STRING = 268,
    TOK_ATTR_FACTORS = 269,
    TOK_FUNC = 270,
    TOK_FUNC_IN = 271,
    TOK_FUNC_REMAP = 272,
    TOK_FUNC_PF = 273,
    TOK_USERVAR = 274,
    TOK_UDF = 275,
    TOK_HOOK_IDENT = 276,
    TOK_HOOK_FUNC = 277,
    TOK_IDENT = 278,
    TOK_ATTR_JSON = 279,
    TOK_ATID = 280,
    TOK_ATWEIGHT = 281,
    TOK_ID = 282,
    TOK_GROUPBY = 283,
    TOK_WEIGHT = 284,
    TOK_COUNT = 285,
    TOK_DISTINCT = 286,
    TOK_CONST_LIST = 287,
    TOK_ATTR_SINT = 288,
    TOK_MAP_ARG = 289,
    TOK_FOR = 290,
    TOK_ITERATOR = 291,
    TOK_IS = 292,
    TOK_NULL = 293,
    TOK_IS_NULL = 294,
    TOK_IS_NOT_NULL = 295,
    TOK_OR = 296,
    TOK_AND = 297,
    TOK_EQ = 298,
    TOK_NE = 299,
    TOK_LTE = 300,
    TOK_GTE = 301,
    TOK_DIV = 302,
    TOK_MOD = 303,
    TOK_NOT = 304,
    TOK_NEG = 305
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{


	int64_t			iConst;			// constant value
	float			fConst;			// constant value
	uint64_t		iAttrLocator;	// attribute locator (rowitem for int/float; offset+size for bits)
	int				iFunc;			// function id
	int				iNode;			// node, or uservar, or udf index
	const char *	sIdent;			// generic identifier (token does NOT own ident storage; ie values are managed by parser)


};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (ExprParser_t * pParser);

#endif /* !YY_YY_YYSPHINXEXPR_H_INCLUDED  */

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
#define YYFINAL  53
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   522

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  68
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  21
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  185

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   305

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    55,    44,     2,
      61,    62,    53,    51,    63,    52,     2,    54,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      47,     2,    48,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,    67,     2,     2,    60,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,    43,    65,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    45,    46,
      49,    50,    56,    57,    58,    59
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    93,    93,    97,    98,    99,   100,   101,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   143,   144,   145,   146,   150,   151,   152,
     156,   157,   158,   159,   160,   161,   162,   166,   167,   171,
     172,   173,   174,   175,   176,   177,   178,   182,   183,   187,
     188,   189,   193,   194,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   211,   212,   216,   219,   220,   224,
     225,   226,   227,   228,   232,   236,   237,   241,   242,   243,
     247,   248
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_CONST_INT", "TOK_CONST_FLOAT",
  "TOK_CONST_STRING", "TOK_SUBKEY", "TOK_DOT_NUMBER", "TOK_ATTR_INT",
  "TOK_ATTR_BITS", "TOK_ATTR_FLOAT", "TOK_ATTR_MVA32", "TOK_ATTR_MVA64",
  "TOK_ATTR_STRING", "TOK_ATTR_FACTORS", "TOK_FUNC", "TOK_FUNC_IN",
  "TOK_FUNC_REMAP", "TOK_FUNC_PF", "TOK_USERVAR", "TOK_UDF",
  "TOK_HOOK_IDENT", "TOK_HOOK_FUNC", "TOK_IDENT", "TOK_ATTR_JSON",
  "TOK_ATID", "TOK_ATWEIGHT", "TOK_ID", "TOK_GROUPBY", "TOK_WEIGHT",
  "TOK_COUNT", "TOK_DISTINCT", "TOK_CONST_LIST", "TOK_ATTR_SINT",
  "TOK_MAP_ARG", "TOK_FOR", "TOK_ITERATOR", "TOK_IS", "TOK_NULL",
  "TOK_IS_NULL", "TOK_IS_NOT_NULL", "TOK_OR", "TOK_AND", "'|'", "'&'",
  "TOK_EQ", "TOK_NE", "'<'", "'>'", "TOK_LTE", "TOK_GTE", "'+'", "'-'",
  "'*'", "'/'", "'%'", "TOK_DIV", "TOK_MOD", "TOK_NOT", "TOK_NEG", "'`'",
  "'('", "')'", "','", "'{'", "'}'", "'['", "']'", "$accept", "exprline",
  "attr", "expr", "maparg", "map_key", "arg", "arglist", "constlist",
  "stringlist", "constlist_or_uservar", "ident", "function", "json_field",
  "json_expr", "subscript", "subkey", "for_loop", "iterator", "streq",
  "strval", YY_NULLPTR
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
     295,   296,   297,   124,    38,   298,   299,    60,    62,   300,
     301,    43,    45,    42,    47,    37,   302,   303,   304,   305,
      96,    40,    41,    44,   123,   125,    91,    93
};
# endif

#define YYPACT_NINF -165

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-165)))

#define YYTABLE_NINF -102

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     325,  -165,  -165,  -165,  -165,  -165,  -165,  -165,  -165,   -45,
     -42,   -19,   -13,    17,  -165,    44,     6,     6,  -165,  -165,
    -165,    75,   325,   325,    19,   325,    37,    95,   405,  -165,
     103,   108,  -165,  -165,   113,    77,   263,   325,   139,   201,
     263,  -165,  -165,   384,     6,  -165,     6,   105,  -165,  -165,
    -165,   134,   373,  -165,   325,   325,   325,   325,   325,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   325,
     325,   -28,   325,   145,  -165,  -165,   151,  -165,  -165,    -5,
     194,  -165,   -22,   405,   135,   253,  -165,   140,  -165,   -17,
      68,   153,   166,   128,  -165,  -165,  -165,  -165,    65,   420,
     434,   447,   458,  -165,   458,   465,   465,   465,   465,    -4,
      -4,  -165,  -165,  -165,  -165,  -165,  -165,   169,   458,  -165,
    -165,  -165,  -165,  -165,   -43,   189,  -165,   229,   193,  -165,
     263,     2,   325,  -165,  -165,  -165,  -165,  -165,  -165,  -165,
      -5,  -165,    11,   238,  -165,  -165,  -165,  -165,  -165,  -165,
     124,   195,   197,   202,   315,   211,  -165,  -165,    23,  -165,
    -165,    71,   252,  -165,   208,    12,  -165,  -165,  -165,  -165,
    -165,   183,  -165,    73,  -165,  -165,  -165,  -165,   126,   219,
     230,    73,   130,   231,  -165
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    10,    11,   100,    12,     3,     4,     5,   101,     0,
       0,     0,     0,     0,    17,     0,    95,     6,    13,    14,
      15,     0,     0,     0,     0,     0,     0,     8,     2,     9,
       0,    38,    39,    40,     0,     0,     0,     0,     0,     0,
       0,    90,    91,     0,    96,    87,    86,     0,    18,    19,
       6,     0,     0,     1,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    56,    53,    54,    52,    55,    75,     0,
      50,    57,     0,    50,     0,     0,    82,     0,    77,     0,
       0,   100,   101,     0,    88,    16,     7,    37,    36,    35,
      27,    26,    33,    97,    34,    24,    25,    31,    32,    20,
      21,    22,    23,    28,    29,    30,    41,     0,    98,    99,
      72,    48,    49,    73,     0,     0,    47,     0,     0,    74,
       0,     0,     0,    83,    76,    79,    92,    93,    89,    42,
       0,    51,     0,     0,    80,    58,    59,    61,    67,    71,
       0,    69,    70,     0,     0,     0,    43,    44,     0,    60,
      62,     0,     0,    78,     0,     0,    85,    94,    84,    63,
      65,     0,    68,     0,    45,    46,    64,    66,     0,     0,
       0,     0,     0,     0,    81
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -165,  -165,   -24,     1,  -165,   171,   -34,   192,  -164,  -165,
    -165,  -165,  -165,   154,   155,   297,    80,  -165,  -165,  -165,
     -33
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    26,    27,    83,   124,   125,    81,    82,   151,   152,
     153,   126,    29,    30,    31,    44,    45,   128,    32,    33,
      34
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      51,    28,    84,   120,    87,   146,   147,   148,   121,   178,
     116,   122,    41,    42,   156,   174,    35,   182,   123,    36,
     140,   149,   141,    48,    49,   103,    52,     5,     6,     7,
     117,     5,     6,     7,   157,   175,    80,    53,    85,   119,
     129,   130,    37,    50,    93,   134,   130,    17,    38,    66,
      67,    68,    69,    70,   150,    98,    99,   100,   101,   102,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,    43,   118,   169,   170,   146,   147,    39,    24,
       1,     2,    73,    24,     4,     5,     6,     7,    74,    75,
      76,    77,     9,    10,    11,    12,   145,    13,    14,    15,
      16,    17,    18,    19,    20,    40,    21,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,   171,    94,   150,    94,   159,   160,    22,
     135,   130,   -85,   154,   166,    23,    47,    24,    25,    78,
      71,    79,     1,     2,    73,   -84,     4,     5,     6,     7,
      74,    75,    76,    77,     9,    10,    11,    12,    72,    13,
      14,    15,    16,    17,    18,    19,    20,    95,    21,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,   176,   177,   179,   161,
    -100,    22,   183,   161,    96,   138,  -101,    23,   131,    24,
      25,    86,   133,    79,     1,     2,    73,   139,     4,     5,
       6,     7,    74,    75,    76,    77,     9,    10,    11,    12,
     136,    13,    14,    15,    16,    17,    18,    19,    20,   127,
      21,    89,    90,   137,   142,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,   143,    22,   158,   144,   165,   172,   161,    23,
     162,    24,    25,    88,   163,    79,     1,     2,    73,   173,
       4,     5,     6,     7,    74,    75,    76,    77,     9,    10,
      11,    12,   180,    13,    14,    15,    16,    17,    18,    19,
      20,   181,    21,   184,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,   155,   167,   168,    46,    22,   132,     0,     0,     0,
       0,    23,     0,    24,    25,     0,     0,    79,     1,     2,
       3,     0,     4,     5,     6,     7,     0,     0,     8,     0,
       9,    10,    11,    12,     0,    13,    14,    15,    16,    17,
      18,    19,    20,     0,    21,     0,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,    22,   164,     0,
       0,     0,     0,    23,     0,    24,    25,     1,     2,    91,
       0,     4,     5,     6,     7,     0,     0,    92,     0,     9,
      10,    11,    12,     0,    13,    14,    15,    16,    17,    18,
      19,    20,     0,    21,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,    97,    22,     0,     0,     0,
       0,     0,    23,     0,    24,    25,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    64,    65,    66,    67,
      68,    69,    70
};

static const yytype_int16 yycheck[] =
{
      24,     0,    36,     8,    38,     3,     4,     5,    13,   173,
      38,    16,     6,     7,     3,     3,    61,   181,    23,    61,
      63,    19,    65,    22,    23,    58,    25,     8,     9,    10,
      58,     8,     9,    10,    23,    23,    35,     0,    37,    72,
      62,    63,    61,    24,    43,    62,    63,    24,    61,    53,
      54,    55,    56,    57,    52,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    66,    72,     3,     4,     3,     4,    61,    60,
       3,     4,     5,    60,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,   130,    20,    21,    22,
      23,    24,    25,    26,    27,    61,    29,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    52,    44,    52,    46,     3,     4,    52,
      62,    63,    37,   132,   158,    58,    61,    60,    61,    62,
      37,    64,     3,     4,     5,    37,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    45,    20,
      21,    22,    23,    24,    25,    26,    27,    62,    29,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,     3,     4,    62,    63,
      45,    52,    62,    63,    60,    67,    45,    58,    63,    60,
      61,    62,    62,    64,     3,     4,     5,    38,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      67,    20,    21,    22,    23,    24,    25,    26,    27,    35,
      29,    39,    40,    67,    45,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    23,    52,    16,    62,    45,     5,    63,    58,
      63,    60,    61,    62,    62,    64,     3,     4,     5,    61,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    63,    20,    21,    22,    23,    24,    25,    26,
      27,    61,    29,    62,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,   140,   158,   158,    17,    52,    63,    -1,    -1,    -1,
      -1,    58,    -1,    60,    61,    -1,    -1,    64,     3,     4,
       5,    -1,     7,     8,     9,    10,    -1,    -1,    13,    -1,
      15,    16,    17,    18,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    -1,    29,    -1,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    -1,    52,    63,    -1,
      -1,    -1,    -1,    58,    -1,    60,    61,     3,     4,     5,
      -1,     7,     8,     9,    10,    -1,    -1,    13,    -1,    15,
      16,    17,    18,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    29,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    -1,    62,    52,    -1,    -1,    -1,
      -1,    -1,    58,    -1,    60,    61,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    51,    52,    53,    54,
      55,    56,    57
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     7,     8,     9,    10,    13,    15,
      16,    17,    18,    20,    21,    22,    23,    24,    25,    26,
      27,    29,    52,    58,    60,    61,    69,    70,    71,    80,
      81,    82,    86,    87,    88,    61,    61,    61,    61,    61,
      61,     6,     7,    66,    83,    84,    83,    61,    71,    71,
      24,    70,    71,     0,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    37,    45,     5,    11,    12,    13,    14,    62,    64,
      71,    74,    75,    71,    74,    71,    62,    74,    62,    75,
      75,     5,    13,    71,    84,    62,    60,    62,    71,    71,
      71,    71,    71,    88,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    38,    58,    71,    88,
       8,    13,    16,    23,    72,    73,    79,    35,    85,    62,
      63,    63,    63,    62,    62,    62,    67,    67,    67,    38,
      63,    65,    45,    23,    62,    74,     3,     4,     5,    19,
      52,    76,    77,    78,    71,    73,     3,    23,    16,     3,
       4,    63,    63,    62,    63,    45,    70,    81,    82,     3,
       4,    52,     5,    61,     3,    23,     3,     4,    76,    62,
      63,    61,    76,    62,    62
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    68,    69,    70,    70,    70,    70,    70,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    71,    71,
      71,    71,    71,    72,    72,    72,    72,    73,    73,    73,
      74,    74,    74,    74,    74,    74,    74,    75,    75,    76,
      76,    76,    76,    76,    76,    76,    76,    77,    77,    78,
      78,    78,    79,    79,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    81,    81,    82,    83,    83,    84,
      84,    84,    84,    84,    85,    86,    86,    87,    87,    87,
      88,    88
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     2,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     1,     1,
       1,     3,     4,     3,     3,     5,     5,     1,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     3,     1,
       2,     1,     2,     3,     4,     3,     4,     1,     3,     1,
       1,     1,     1,     1,     4,     3,     4,     3,     6,     4,
       5,    14,     3,     4,     1,     1,     2,     1,     2,     3,
       1,     1,     3,     3,     4,     1,     2,     3,     3,     3,
       1,     1
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, ExprParser_t * pParser)
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, ExprParser_t * pParser)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, ExprParser_t * pParser)
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, ExprParser_t * pParser)
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
yyparse (ExprParser_t * pParser)
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

    { pParser->m_iParsed = (yyvsp[0].iNode); }

    break;

  case 3:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_INT, (yyvsp[0].iAttrLocator) ); }

    break;

  case 4:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_BITS, (yyvsp[0].iAttrLocator) ); }

    break;

  case 5:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_FLOAT, (yyvsp[0].iAttrLocator) ); }

    break;

  case 6:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_JSON, (yyvsp[0].iAttrLocator) ); }

    break;

  case 7:

    { (yyval.iNode) = (yyvsp[-1].iNode); }

    break;

  case 10:

    { (yyval.iNode) = pParser->AddNodeInt ( (yyvsp[0].iConst) ); }

    break;

  case 11:

    { (yyval.iNode) = pParser->AddNodeFloat ( (yyvsp[0].fConst) ); }

    break;

  case 12:

    { (yyval.iNode) = pParser->AddNodeDotNumber ( (yyvsp[0].iConst) ); }

    break;

  case 13:

    { (yyval.iNode) = pParser->AddNodeID(); }

    break;

  case 14:

    { (yyval.iNode) = pParser->AddNodeWeight(); }

    break;

  case 15:

    { (yyval.iNode) = pParser->AddNodeID(); }

    break;

  case 16:

    { (yyval.iNode) = pParser->AddNodeWeight(); }

    break;

  case 17:

    { (yyval.iNode) = pParser->AddNodeHookIdent ( (yyvsp[0].iNode) ); }

    break;

  case 18:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_NEG, (yyvsp[0].iNode), -1 ); }

    break;

  case 19:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_NOT, (yyvsp[0].iNode), -1 ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 20:

    { (yyval.iNode) = pParser->AddNodeOp ( '+', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 21:

    { (yyval.iNode) = pParser->AddNodeOp ( '-', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 22:

    { (yyval.iNode) = pParser->AddNodeOp ( '*', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 23:

    { (yyval.iNode) = pParser->AddNodeOp ( '/', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 24:

    { (yyval.iNode) = pParser->AddNodeOp ( '<', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 25:

    { (yyval.iNode) = pParser->AddNodeOp ( '>', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 26:

    { (yyval.iNode) = pParser->AddNodeOp ( '&', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 27:

    { (yyval.iNode) = pParser->AddNodeOp ( '|', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 28:

    { (yyval.iNode) = pParser->AddNodeOp ( '%', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 29:

    { (yyval.iNode) = pParser->AddNodeFunc ( FUNC_IDIV, pParser->AddNodeOp ( ',', (yyvsp[-2].iNode), (yyvsp[0].iNode) ) ); }

    break;

  case 30:

    { (yyval.iNode) = pParser->AddNodeOp ( '%', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 31:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_LTE, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 32:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_GTE, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 33:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_EQ, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 34:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_NE, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 35:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_AND, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 36:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_OR, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 37:

    { (yyval.iNode) = (yyvsp[-1].iNode); }

    break;

  case 41:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_IS_NULL, (yyvsp[-2].iNode), -1); }

    break;

  case 42:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_IS_NOT_NULL, (yyvsp[-3].iNode), -1); }

    break;

  case 43:

    { (yyval.iNode) = pParser->AddNodeMapArg ( (yyvsp[-2].sIdent), NULL, (yyvsp[0].iConst) ); }

    break;

  case 44:

    { (yyval.iNode) = pParser->AddNodeMapArg ( (yyvsp[-2].sIdent), (yyvsp[0].sIdent), 0 ); }

    break;

  case 45:

    { pParser->AppendToMapArg ( (yyval.iNode), (yyvsp[-2].sIdent), NULL, (yyvsp[0].iConst) ); }

    break;

  case 46:

    { pParser->AppendToMapArg ( (yyval.iNode), (yyvsp[-2].sIdent), (yyvsp[0].sIdent), 0 ); }

    break;

  case 47:

    { (yyval.sIdent) = (yyvsp[0].sIdent); }

    break;

  case 48:

    { (yyval.sIdent) = pParser->Attr2Ident((yyvsp[0].iAttrLocator)); }

    break;

  case 49:

    { (yyval.sIdent) = strdup("in"); }

    break;

  case 51:

    { (yyval.iNode) = (yyvsp[-1].iNode); }

    break;

  case 52:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_STRING, (yyvsp[0].iAttrLocator) ); }

    break;

  case 53:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_MVA32, (yyvsp[0].iAttrLocator) ); }

    break;

  case 54:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_MVA64, (yyvsp[0].iAttrLocator) ); }

    break;

  case 55:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_FACTORS, (yyvsp[0].iAttrLocator) ); }

    break;

  case 56:

    { (yyval.iNode) = pParser->AddNodeString ( (yyvsp[0].iConst) ); }

    break;

  case 57:

    { (yyval.iNode) = (yyvsp[0].iNode); }

    break;

  case 58:

    { (yyval.iNode) = pParser->AddNodeOp ( ',', (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 59:

    { (yyval.iNode) = pParser->AddNodeConstlist ( (yyvsp[0].iConst) ); }

    break;

  case 60:

    { (yyval.iNode) = pParser->AddNodeConstlist ( -(yyvsp[0].iConst) );}

    break;

  case 61:

    { (yyval.iNode) = pParser->AddNodeConstlist ( (yyvsp[0].fConst) ); }

    break;

  case 62:

    { (yyval.iNode) = pParser->AddNodeConstlist ( -(yyvsp[0].fConst) );}

    break;

  case 63:

    { pParser->AppendToConstlist ( (yyval.iNode), (yyvsp[0].iConst) ); }

    break;

  case 64:

    { pParser->AppendToConstlist ( (yyval.iNode), -(yyvsp[0].iConst) );}

    break;

  case 65:

    { pParser->AppendToConstlist ( (yyval.iNode), (yyvsp[0].fConst) ); }

    break;

  case 66:

    { pParser->AppendToConstlist ( (yyval.iNode), -(yyvsp[0].fConst) );}

    break;

  case 67:

    { (yyval.iNode) = pParser->AddNodeConstlist ( (yyvsp[0].iConst) ); }

    break;

  case 68:

    { pParser->AppendToConstlist ( (yyval.iNode), (yyvsp[0].iConst) ); }

    break;

  case 71:

    { (yyval.iNode) = pParser->AddNodeUservar ( (yyvsp[0].iNode) ); }

    break;

  case 72:

    { (yyval.sIdent) = pParser->Attr2Ident ( (yyvsp[0].iAttrLocator) ); }

    break;

  case 74:

    { (yyval.iNode) = pParser->AddNodeFunc ( (yyvsp[-3].iFunc), (yyvsp[-1].iNode) ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 75:

    { (yyval.iNode) = pParser->AddNodeFunc ( (yyvsp[-2].iFunc), -1 ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 76:

    { (yyval.iNode) = pParser->AddNodeUdf ( (yyvsp[-3].iNode), (yyvsp[-1].iNode) ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 77:

    { (yyval.iNode) = pParser->AddNodeUdf ( (yyvsp[-2].iNode), -1 ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 78:

    { (yyval.iNode) = pParser->AddNodeFunc ( (yyvsp[-5].iFunc), (yyvsp[-3].iNode), (yyvsp[-1].iNode) ); }

    break;

  case 79:

    { (yyval.iNode) = pParser->AddNodeHookFunc ( (yyvsp[-3].iNode), (yyvsp[-1].iNode) ); if ( (yyval.iNode)<0 ) YYERROR; }

    break;

  case 80:

    { (yyval.iNode) = pParser->AddNodeFunc ( (yyvsp[-4].iFunc), (yyvsp[-2].iNode), (yyvsp[-1].iNode) ); }

    break;

  case 81:

    { (yyval.iNode) = pParser->AddNodeFunc ( (yyvsp[-13].iFunc), (yyvsp[-11].iNode), (yyvsp[-9].iNode), (yyvsp[-6].iNode), (yyvsp[-2].iNode) ); }

    break;

  case 82:

    { (yyval.iNode) = pParser->AddNodePF ( (yyvsp[-2].iNode), -1 ); }

    break;

  case 83:

    { (yyval.iNode) = pParser->AddNodePF ( (yyvsp[-3].iNode), (yyvsp[-1].iNode) ); }

    break;

  case 86:

    { (yyval.iNode) = pParser->AddNodeJsonField ( (yyvsp[-1].iAttrLocator), (yyvsp[0].iNode) ); }

    break;

  case 88:

    { (yyval.iNode) = pParser->AddNodeOp ( ',', (yyvsp[-1].iNode), (yyvsp[0].iNode) ); }

    break;

  case 89:

    { (yyval.iNode) = (yyvsp[-1].iNode); }

    break;

  case 90:

    { (yyval.iNode) = pParser->AddNodeJsonSubkey ( (yyvsp[0].iConst) ); }

    break;

  case 91:

    { (yyval.iNode) = pParser->AddNodeJsonSubkey ( (yyvsp[0].iConst) ); }

    break;

  case 92:

    { (yyval.iNode) = pParser->AddNodeString ( (yyvsp[-1].iConst) ); }

    break;

  case 93:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_STRING, (yyvsp[-1].iAttrLocator) ); }

    break;

  case 94:

    { (yyval.iNode) = pParser->AddNodeIdent ( (yyvsp[-2].sIdent), (yyvsp[0].iNode) ); }

    break;

  case 95:

    { (yyval.iNode) = pParser->AddNodeIdent ( (yyvsp[0].sIdent), -1 ); }

    break;

  case 96:

    { (yyval.iNode) = pParser->AddNodeIdent ( (yyvsp[-1].sIdent), (yyvsp[0].iNode) ); }

    break;

  case 97:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_EQ, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 98:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_EQ, (yyvsp[0].iNode), (yyvsp[-2].iNode) ); }

    break;

  case 99:

    { (yyval.iNode) = pParser->AddNodeOp ( TOK_EQ, (yyvsp[-2].iNode), (yyvsp[0].iNode) ); }

    break;

  case 100:

    { (yyval.iNode) = pParser->AddNodeString ( (yyvsp[0].iConst) ); }

    break;

  case 101:

    { (yyval.iNode) = pParser->AddNodeAttr ( TOK_ATTR_STRING, (yyvsp[0].iAttrLocator) ); }

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


