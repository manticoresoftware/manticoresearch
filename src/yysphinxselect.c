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
   by #include "yysphinxselect.h".  */
#ifndef YY_YY_YYSPHINXSELECT_H_INCLUDED
# define YY_YY_YYSPHINXSELECT_H_INCLUDED
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
    SEL_TOKEN = 258,
    SEL_ID = 259,
    SEL_AS = 260,
    SEL_AVG = 261,
    SEL_MAX = 262,
    SEL_MIN = 263,
    SEL_SUM = 264,
    SEL_GROUP_CONCAT = 265,
    SEL_GROUPBY = 266,
    SEL_COUNT = 267,
    SEL_WEIGHT = 268,
    SEL_DISTINCT = 269,
    SEL_OPTION = 270,
    SEL_COMMENT_OPEN = 271,
    SEL_COMMENT_CLOSE = 272,
    TOK_DIV = 273,
    TOK_MOD = 274,
    TOK_NEG = 275,
    TOK_LTE = 276,
    TOK_GTE = 277,
    TOK_EQ = 278,
    TOK_NE = 279,
    TOK_CONST_STRING = 280,
    TOK_OR = 281,
    TOK_AND = 282,
    TOK_NOT = 283,
    TOK_IS = 284,
    TOK_NULL = 285,
    TOK_FOR = 286,
    TOK_FUNC_IN = 287,
    TOK_FUNC_RAND = 288
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (SelectParser_t * pParser);

#endif /* !YY_YY_YYSPHINXSELECT_H_INCLUDED  */

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
#define YYFINAL  77
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   867

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  52
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  198

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   288

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    36,    29,     2,
      44,    45,    34,    32,    43,    33,    49,    35,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      30,     2,    31,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    50,     2,    51,     2,     2,    46,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    47,    28,    48,     2,     2,     2,     2,
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
      25,    26,    27,    37,    38,    39,    40,    41,    42
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    58,    58,    59,    60,    64,    65,    69,    70,    72,
      74,    75,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   124,
     125,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     141,   142,   146,   147,   151,   152,   156,   157,   161,   168,
     169,   173,   177,   178,   182,   183,   183,   183,   183,   183,
     183,   183,   184,   184,   184,   184,   184,   184,   185,   185,
     185,   185,   185,   185,   185,   185,   186,   186,   190,   191,
     192,   196
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SEL_TOKEN", "SEL_ID", "SEL_AS",
  "SEL_AVG", "SEL_MAX", "SEL_MIN", "SEL_SUM", "SEL_GROUP_CONCAT",
  "SEL_GROUPBY", "SEL_COUNT", "SEL_WEIGHT", "SEL_DISTINCT", "SEL_OPTION",
  "SEL_COMMENT_OPEN", "SEL_COMMENT_CLOSE", "TOK_DIV", "TOK_MOD", "TOK_NEG",
  "TOK_LTE", "TOK_GTE", "TOK_EQ", "TOK_NE", "TOK_CONST_STRING", "TOK_OR",
  "TOK_AND", "'|'", "'&'", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'", "'%'",
  "TOK_NOT", "TOK_IS", "TOK_NULL", "TOK_FOR", "TOK_FUNC_IN",
  "TOK_FUNC_RAND", "','", "'('", "')'", "'`'", "'{'", "'}'", "'.'", "'['",
  "']'", "$accept", "select", "select_list", "select_item", "opt_alias",
  "select_expr", "expr", "select_atom", "function", "arglist", "consthash",
  "hash_key", "arg", "comment", "json_field", "json_expr", "subscript",
  "ident", "subkey", "for_loop", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   124,    38,
      60,    62,    43,    45,    42,    47,    37,   283,   284,   285,
     286,   287,   288,    44,    40,    41,    96,   123,   125,    46,
      91,    93
};
# endif

#define YYPACT_NINF -45

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-45)))

#define YYTABLE_NINF -76

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      98,   817,   -28,   -45,     1,    12,    17,    31,    38,    56,
      79,    84,   -45,   -45,    19,   -45,   -45,   -45,   -45,   -45,
     -45,   -45,   -45,   -45,   436,   -45,   436,   -45,    85,    86,
     436,    44,    -1,    80,   -11,   -45,    22,   128,   -45,   -45,
     -45,    95,    96,   -45,   166,   481,   256,    24,   -45,   436,
     436,   436,   436,   436,    -7,     8,    91,   135,   -45,    97,
      99,   -45,   -45,   -45,   -45,   -45,   -45,   301,   211,   568,
     -45,   -45,   102,   -45,   -45,   -20,   130,   -45,   346,   -45,
     -45,   162,   -45,   436,   436,   436,   436,   391,   436,   436,
     436,   436,   436,   436,   436,   436,   436,   436,   436,   436,
      -8,   -45,   -45,   734,   -10,   -45,   -45,   -45,   -45,   -45,
     -45,   -45,   -45,   116,   -15,   -45,   588,    36,   540,   608,
     628,   -45,   179,   138,   -45,   171,   436,   436,   128,    -6,
     -45,    -2,   -45,   -45,    -1,   -45,   192,   -45,   -45,   -45,
     -45,   479,   479,   -45,   818,   818,   754,   773,   792,   811,
     479,   479,   516,   516,   -45,   -45,   -45,   157,   -45,   481,
     152,   301,   -45,   -45,   -45,   -45,   436,   -45,   436,   -45,
     -45,   -45,   153,   -45,   197,   688,   711,   -45,   -45,   178,
     -45,   -45,   161,   -45,   -45,   648,   668,   -45,   187,   203,
     518,   -45,   -45,   -45,   -45,    24,   -45,   -45
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    50,    49,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,     0,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     0,     7,    96,    97,     0,     0,
       0,     0,     0,     0,     2,     5,     9,    12,    21,    44,
       4,     0,    45,    70,     0,     0,     0,    71,    72,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    77,    78,
      79,    80,    81,    82,    83,    23,    24,     0,     0,     0,
      50,    49,     0,    64,    65,     0,     0,     1,     0,     3,
      10,     0,     8,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    67,    52,    66,     0,    60,    74,    75,    78,    79,
      84,    96,    98,     0,     0,    73,     0,     0,     0,     0,
       0,    18,     0,     0,    55,     0,     0,     0,    66,     0,
      58,     0,    42,    22,     0,    43,     0,     6,    11,    34,
      35,    36,    37,    48,    38,    39,    41,    40,    32,    31,
      29,    30,    25,    26,    27,    28,    33,     0,    46,     0,
       0,     0,    51,   100,    99,    13,     0,    14,     0,    15,
      16,    17,     0,    19,     0,     0,     0,    57,    59,     0,
      62,    47,     0,    56,    61,     0,     0,    20,     0,     0,
       0,    54,    53,    68,    63,    74,   101,    69
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -45,   -45,   -45,   131,   -45,   -45,     0,   196,   -45,     9,
     -45,    94,    78,   206,    51,    52,   -45,   -44,   198,   -45
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    33,    34,    35,    82,    36,   128,    38,    39,   104,
      75,    76,   105,    40,    41,    42,    47,    43,    48,   160
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,   112,    73,    83,    84,    14,    85,    86,    87,    88,
     -75,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   122,   134,    65,    80,    66,    81,   135,   157,
      69,   158,    78,   161,    57,   162,   164,   161,   121,   177,
      74,   161,   123,   178,   103,    49,   114,    70,    71,   116,
     117,   118,   119,   120,    83,    84,    50,    85,    86,    87,
      88,    51,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,    45,    46,    52,   129,   131,    37,   166,
      77,   167,    53,   139,   140,   141,   142,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
      54,     1,     2,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,   182,    15,    16,    17,    18,
      19,    20,    21,    55,    22,    23,   175,   176,    56,    67,
      68,    24,    25,   100,   -69,    26,   124,    27,   125,    28,
      29,   126,    30,   127,    31,    32,    83,    84,   133,    85,
      86,    87,    88,   136,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   138,   185,   163,   186,     1,
       2,     3,    58,    59,    60,    61,    62,    63,    64,    11,
      12,    13,   172,   173,    15,    16,    17,    18,    19,    20,
      21,   101,    22,    23,   174,   180,   181,   183,   187,    24,
     188,   189,   190,    26,   193,    27,   194,    28,    29,   137,
      30,   102,    31,    32,     1,     2,     3,    58,    59,    60,
      61,    62,    63,    64,    11,    12,    13,    72,   179,    15,
      16,    17,    18,    19,    20,    21,   101,    22,    23,   184,
      79,   196,   197,     0,    24,   115,     0,     0,    26,     0,
      27,     0,    28,    29,     0,    30,   130,    31,    32,     1,
       2,     3,    58,    59,    60,    61,    62,    63,    64,    11,
      12,    13,     0,     0,    15,    16,    17,    18,    19,    20,
      21,   113,    22,    23,     0,     0,     0,     0,     0,    24,
       0,     0,     0,    26,     0,    27,     0,    28,    29,     0,
      30,     0,    31,    32,     1,     2,     3,    58,    59,    60,
      61,    62,    63,    64,    11,    12,    13,     0,     0,    15,
      16,    17,    18,    19,    20,    21,   101,    22,    23,     0,
       0,     0,     0,     0,    24,     0,     0,     0,    26,     0,
      27,     0,    28,    29,     0,    30,     0,    31,    32,     1,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,     0,     0,    15,    16,    17,    18,    19,    20,
      21,     0,    22,    23,     0,     0,     0,     0,     0,    24,
      25,     0,     0,    26,     0,    27,     0,    28,    29,     0,
      30,     0,    31,    32,     1,     2,     3,    58,    59,    60,
      61,    62,    63,    64,    11,    12,    13,     0,     0,    15,
      16,    17,    18,    19,    20,    21,   143,    22,    23,     0,
       0,     0,     0,     0,    24,     0,     0,     0,    26,     0,
      27,     0,    28,    29,     0,    30,     0,    31,    32,     1,
       2,     3,    58,    59,    60,    61,    62,    63,    64,    11,
      12,    13,     0,     0,    15,    16,    17,    18,    19,    20,
      21,     0,    22,    23,     0,     0,     0,     0,     0,    24,
       0,     0,     0,    26,     0,    27,     0,    28,    29,     0,
      30,     0,    31,    32,   106,   107,     3,    58,   108,   109,
      61,    62,    63,    64,   110,    12,    13,    83,    84,    15,
      16,    17,    18,    19,    20,    21,     0,    22,    23,     0,
       0,    95,    96,    97,    98,    99,     0,     0,   111,     0,
      27,   195,   107,     3,    58,   108,   109,    61,    62,    63,
      64,   110,    12,    13,    83,    84,    15,    16,    17,    18,
      19,    20,    21,     0,    22,    23,     0,     0,     0,     0,
      97,    98,    99,     0,     0,   111,     0,    27,    83,    84,
       0,    85,    86,    87,    88,     0,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,     0,     0,     0,
       0,     0,     0,   168,     0,   169,    83,    84,     0,    85,
      86,    87,    88,     0,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     0,    83,    84,     0,    85,
      86,    87,    88,   132,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     0,    83,    84,     0,    85,
      86,    87,    88,   165,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     0,    83,    84,     0,    85,
      86,    87,    88,   170,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     0,    83,    84,     0,    85,
      86,    87,    88,   171,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     0,    83,    84,     0,    85,
      86,    87,    88,   191,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     0,    83,    84,     0,    85,
      86,    87,    88,   192,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,     0,     0,     0,     0,    83,
      84,   166,    85,    86,    87,    88,     0,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,     0,     0,
       0,     0,    83,    84,   168,    85,    86,    87,    88,     0,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,     0,    83,    84,   159,    85,    86,    87,    88,     0,
       0,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,    83,    84,     0,    85,    86,    87,    88,     0,     0,
       0,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      83,    84,     0,    85,    86,    87,    88,     0,     0,     0,
       0,    92,    93,    94,    95,    96,    97,    98,    99,    83,
      84,     0,    85,    86,    87,    88,    83,    84,     0,    85,
      86,    93,    94,    95,    96,    97,    98,    99,    93,    94,
      95,    96,    97,    98,    99,   -74,     0,     0,     0,     0,
       0,    44,     0,     0,     0,     0,    45,    46
};

static const yytype_int16 yycheck[] =
{
       0,    45,     3,    18,    19,    16,    21,    22,    23,    24,
      38,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    14,    43,    24,     3,    26,     5,    48,    37,
      30,    39,    43,    43,    15,    45,    51,    43,    45,    45,
      41,    43,    34,    45,    44,    44,    46,     3,     4,    49,
      50,    51,    52,    53,    18,    19,    44,    21,    22,    23,
      24,    44,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    49,    50,    44,    67,    68,    78,    43,
       0,    45,    44,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
      44,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,   159,    18,    19,    20,    21,
      22,    23,    24,    44,    26,    27,   126,   127,    44,    44,
      44,    33,    34,    38,    38,    37,    45,    39,     3,    41,
      42,    44,    44,    44,    46,    47,    18,    19,    46,    21,
      22,    23,    24,    23,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,     3,   166,    51,   168,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,     3,    45,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    23,     3,    39,    45,    45,    33,
       3,    23,    41,    37,    17,    39,     3,    41,    42,    78,
      44,    45,    46,    47,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    31,   134,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,   161,
      34,   190,   190,    -1,    33,    47,    -1,    -1,    37,    -1,
      39,    -1,    41,    42,    -1,    44,    45,    46,    47,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    -1,    -1,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    37,    -1,    39,    -1,    41,    42,    -1,
      44,    -1,    46,    47,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    -1,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    -1,
      39,    -1,    41,    42,    -1,    44,    -1,    46,    47,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    -1,    -1,    -1,    -1,    -1,    33,
      34,    -1,    -1,    37,    -1,    39,    -1,    41,    42,    -1,
      44,    -1,    46,    47,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    -1,    -1,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    -1,
      -1,    -1,    -1,    -1,    33,    -1,    -1,    -1,    37,    -1,
      39,    -1,    41,    42,    -1,    44,    -1,    46,    47,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      24,    -1,    26,    27,    -1,    -1,    -1,    -1,    -1,    33,
      -1,    -1,    -1,    37,    -1,    39,    -1,    41,    42,    -1,
      44,    -1,    46,    47,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    18,    19,    18,
      19,    20,    21,    22,    23,    24,    -1,    26,    27,    -1,
      -1,    32,    33,    34,    35,    36,    -1,    -1,    37,    -1,
      39,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    18,    19,    18,    19,    20,    21,
      22,    23,    24,    -1,    26,    27,    -1,    -1,    -1,    -1,
      34,    35,    36,    -1,    -1,    37,    -1,    39,    18,    19,
      -1,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    -1,    -1,    -1,
      -1,    -1,    -1,    43,    -1,    45,    18,    19,    -1,    21,
      22,    23,    24,    -1,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    18,    19,    -1,    21,
      22,    23,    24,    45,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    18,    19,    -1,    21,
      22,    23,    24,    45,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    18,    19,    -1,    21,
      22,    23,    24,    45,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    18,    19,    -1,    21,
      22,    23,    24,    45,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    18,    19,    -1,    21,
      22,    23,    24,    45,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    18,    19,    -1,    21,
      22,    23,    24,    45,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    -1,    -1,    -1,    -1,    18,
      19,    43,    21,    22,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    -1,    -1,
      -1,    -1,    18,    19,    43,    21,    22,    23,    24,    -1,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    -1,    18,    19,    40,    21,    22,    23,    24,    -1,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    18,    19,    -1,    21,    22,    23,    24,    -1,    -1,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      18,    19,    -1,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,    33,    34,    35,    36,    18,
      19,    -1,    21,    22,    23,    24,    18,    19,    -1,    21,
      22,    30,    31,    32,    33,    34,    35,    36,    30,    31,
      32,    33,    34,    35,    36,    38,    -1,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    49,    50
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    18,    19,    20,    21,    22,
      23,    24,    26,    27,    33,    34,    37,    39,    41,    42,
      44,    46,    47,    53,    54,    55,    57,    58,    59,    60,
      65,    66,    67,    69,    44,    49,    50,    68,    70,    44,
      44,    44,    44,    44,    44,    44,    44,    15,     6,     7,
       8,     9,    10,    11,    12,    58,    58,    44,    44,    58,
       3,     4,    59,     3,    41,    62,    63,     0,    43,    65,
       3,     5,    56,    18,    19,    21,    22,    23,    24,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      38,    25,    45,    58,    61,    64,     3,     4,     7,     8,
      13,    37,    69,    25,    58,    70,    58,    58,    58,    58,
      58,    45,    14,    34,    45,     3,    44,    44,    58,    61,
      45,    61,    45,    46,    43,    48,    23,    55,     3,    58,
      58,    58,    58,    25,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    37,    39,    40,
      71,    43,    45,    51,    51,    45,    43,    45,    43,    45,
      45,    45,     3,    45,    23,    58,    58,    45,    45,    63,
       3,    39,    69,    45,    64,    58,    58,    45,     3,    23,
      41,    45,    45,    17,     3,     3,    66,    67
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    52,    53,    53,    53,    54,    54,    55,    55,    56,
      56,    56,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    59,
      59,    60,    60,    60,    60,    60,    60,    60,    60,    60,
      61,    61,    62,    62,    63,    63,    64,    64,    65,    66,
      66,    67,    68,    68,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    70,    70,
      70,    71
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     3,     1,     2,     0,
       1,     2,     1,     4,     4,     4,     4,     4,     3,     4,
       5,     1,     3,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     3,     4,     3,     1,
       1,     4,     3,     6,     6,     3,     5,     4,     3,     4,
       1,     3,     3,     5,     1,     1,     1,     1,     6,     1,
       1,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       3,     4
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, SelectParser_t * pParser)
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, SelectParser_t * pParser)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, SelectParser_t * pParser)
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, SelectParser_t * pParser)
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
yyparse (SelectParser_t * pParser)
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
        case 7:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 10:

    { pParser->AliasLastItem ( &(yyvsp[0]) ); }

    break;

  case 11:

    { pParser->AliasLastItem ( &(yyvsp[0]) ); }

    break;

  case 12:

    { pParser->AddItem ( &(yyvsp[0]) ); }

    break;

  case 13:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_AVG, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 14:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_MAX, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 15:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_MIN, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 16:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_SUM, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 17:

    { pParser->AddItem ( &(yyvsp[-1]), SPH_AGGR_CAT, &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 18:

    { pParser->AddItem ( "groupby()", &(yyvsp[-2]), &(yyvsp[0]) ); }

    break;

  case 19:

    { pParser->AddItem ( "count(*)", &(yyvsp[-3]), &(yyvsp[0]) ); }

    break;

  case 20:

    { pParser->AddItem ( "@distinct", &(yyvsp[-4]), &(yyvsp[0]) ); }

    break;

  case 22:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[-1]).m_iEnd; }

    break;

  case 23:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 24:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 25:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 26:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 27:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 28:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 29:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 30:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 31:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 32:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 33:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 34:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 35:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 36:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 37:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 38:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 39:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 40:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 41:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 42:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 43:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 46:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 47:

    { (yyval) = (yyvsp[-3]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 48:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 51:

    { (yyval) = (yyvsp[-3]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 52:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 53:

    { (yyval) = (yyvsp[-5]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 54:

    { (yyval) = (yyvsp[-5]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 55:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 56:

    { (yyval) = (yyvsp[-4]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 57:

    { (yyval) = (yyvsp[-3]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 58:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 59:

    { (yyval) = (yyvsp[-3]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 68:

    {
			pParser->AddOption ( &(yyvsp[-3]), &(yyvsp[-1]) );
		}

    break;

  case 71:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 73:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 98:

    { (yyval) = (yyvsp[-1]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 99:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 100:

    { (yyval) = (yyvsp[-2]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

    break;

  case 101:

    { (yyval) = (yyvsp[-3]); (yyval).m_iEnd = (yyvsp[0]).m_iEnd; }

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
