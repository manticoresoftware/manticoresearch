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
     TOK_AS = 273,
     TOK_ASC = 274,
     TOK_ATTACH = 275,
     TOK_AVG = 276,
     TOK_BEGIN = 277,
     TOK_BETWEEN = 278,
     TOK_BIGINT = 279,
     TOK_BOOL = 280,
     TOK_BY = 281,
     TOK_CALL = 282,
     TOK_CHARACTER = 283,
     TOK_CHUNK = 284,
     TOK_COLLATION = 285,
     TOK_COLUMN = 286,
     TOK_COMMIT = 287,
     TOK_COMMITTED = 288,
     TOK_COUNT = 289,
     TOK_CREATE = 290,
     TOK_DATABASES = 291,
     TOK_DELETE = 292,
     TOK_DESC = 293,
     TOK_DESCRIBE = 294,
     TOK_DISTINCT = 295,
     TOK_DIV = 296,
     TOK_DOUBLE = 297,
     TOK_DROP = 298,
     TOK_FACET = 299,
     TOK_FALSE = 300,
     TOK_FLOAT = 301,
     TOK_FLUSH = 302,
     TOK_FOR = 303,
     TOK_FROM = 304,
     TOK_FUNCTION = 305,
     TOK_GLOBAL = 306,
     TOK_GROUP = 307,
     TOK_GROUPBY = 308,
     TOK_GROUP_CONCAT = 309,
     TOK_HAVING = 310,
     TOK_ID = 311,
     TOK_IN = 312,
     TOK_INDEX = 313,
     TOK_INSERT = 314,
     TOK_INT = 315,
     TOK_INTEGER = 316,
     TOK_INTO = 317,
     TOK_IS = 318,
     TOK_ISOLATION = 319,
     TOK_JSON = 320,
     TOK_LEVEL = 321,
     TOK_LIKE = 322,
     TOK_LIMIT = 323,
     TOK_MATCH = 324,
     TOK_MAX = 325,
     TOK_META = 326,
     TOK_MIN = 327,
     TOK_MOD = 328,
     TOK_MULTI = 329,
     TOK_MULTI64 = 330,
     TOK_NAMES = 331,
     TOK_NULL = 332,
     TOK_OPTION = 333,
     TOK_ORDER = 334,
     TOK_OPTIMIZE = 335,
     TOK_PLAN = 336,
     TOK_PLUGIN = 337,
     TOK_PLUGINS = 338,
     TOK_PROFILE = 339,
     TOK_RAND = 340,
     TOK_RAMCHUNK = 341,
     TOK_READ = 342,
     TOK_RECONFIGURE = 343,
     TOK_REPEATABLE = 344,
     TOK_REPLACE = 345,
     TOK_REMAP = 346,
     TOK_RETURNS = 347,
     TOK_ROLLBACK = 348,
     TOK_RTINDEX = 349,
     TOK_SELECT = 350,
     TOK_SERIALIZABLE = 351,
     TOK_SET = 352,
     TOK_SETTINGS = 353,
     TOK_SESSION = 354,
     TOK_SHOW = 355,
     TOK_SONAME = 356,
     TOK_START = 357,
     TOK_STATUS = 358,
     TOK_STRING = 359,
     TOK_SUM = 360,
     TOK_TABLE = 361,
     TOK_TABLES = 362,
     TOK_THREADS = 363,
     TOK_TO = 364,
     TOK_TRANSACTION = 365,
     TOK_TRUE = 366,
     TOK_TRUNCATE = 367,
     TOK_TYPE = 368,
     TOK_UNCOMMITTED = 369,
     TOK_UPDATE = 370,
     TOK_VALUES = 371,
     TOK_VARIABLES = 372,
     TOK_WARNINGS = 373,
     TOK_WEIGHT = 374,
     TOK_WHERE = 375,
     TOK_WITHIN = 376,
     TOK_OR = 377,
     TOK_AND = 378,
     TOK_NE = 379,
     TOK_GTE = 380,
     TOK_LTE = 381,
     TOK_NOT = 382,
     TOK_NEG = 383
   };
#endif
#define TOK_IDENT 258
#define TOK_ATIDENT 259
#define TOK_CONST_INT 260
#define TOK_CONST_FLOAT 261
#define TOK_CONST_MVA 262
#define TOK_QUOTED_STRING 263
#define TOK_USERVAR 264
#define TOK_SYSVAR 265
#define TOK_CONST_STRINGS 266
#define TOK_BAD_NUMERIC 267
#define TOK_SUBKEY 268
#define TOK_DOT_NUMBER 269
#define TOK_ADD 270
#define TOK_AGENT 271
#define TOK_ALTER 272
#define TOK_AS 273
#define TOK_ASC 274
#define TOK_ATTACH 275
#define TOK_AVG 276
#define TOK_BEGIN 277
#define TOK_BETWEEN 278
#define TOK_BIGINT 279
#define TOK_BOOL 280
#define TOK_BY 281
#define TOK_CALL 282
#define TOK_CHARACTER 283
#define TOK_CHUNK 284
#define TOK_COLLATION 285
#define TOK_COLUMN 286
#define TOK_COMMIT 287
#define TOK_COMMITTED 288
#define TOK_COUNT 289
#define TOK_CREATE 290
#define TOK_DATABASES 291
#define TOK_DELETE 292
#define TOK_DESC 293
#define TOK_DESCRIBE 294
#define TOK_DISTINCT 295
#define TOK_DIV 296
#define TOK_DOUBLE 297
#define TOK_DROP 298
#define TOK_FACET 299
#define TOK_FALSE 300
#define TOK_FLOAT 301
#define TOK_FLUSH 302
#define TOK_FOR 303
#define TOK_FROM 304
#define TOK_FUNCTION 305
#define TOK_GLOBAL 306
#define TOK_GROUP 307
#define TOK_GROUPBY 308
#define TOK_GROUP_CONCAT 309
#define TOK_HAVING 310
#define TOK_ID 311
#define TOK_IN 312
#define TOK_INDEX 313
#define TOK_INSERT 314
#define TOK_INT 315
#define TOK_INTEGER 316
#define TOK_INTO 317
#define TOK_IS 318
#define TOK_ISOLATION 319
#define TOK_JSON 320
#define TOK_LEVEL 321
#define TOK_LIKE 322
#define TOK_LIMIT 323
#define TOK_MATCH 324
#define TOK_MAX 325
#define TOK_META 326
#define TOK_MIN 327
#define TOK_MOD 328
#define TOK_MULTI 329
#define TOK_MULTI64 330
#define TOK_NAMES 331
#define TOK_NULL 332
#define TOK_OPTION 333
#define TOK_ORDER 334
#define TOK_OPTIMIZE 335
#define TOK_PLAN 336
#define TOK_PLUGIN 337
#define TOK_PLUGINS 338
#define TOK_PROFILE 339
#define TOK_RAND 340
#define TOK_RAMCHUNK 341
#define TOK_READ 342
#define TOK_RECONFIGURE 343
#define TOK_REPEATABLE 344
#define TOK_REPLACE 345
#define TOK_REMAP 346
#define TOK_RETURNS 347
#define TOK_ROLLBACK 348
#define TOK_RTINDEX 349
#define TOK_SELECT 350
#define TOK_SERIALIZABLE 351
#define TOK_SET 352
#define TOK_SETTINGS 353
#define TOK_SESSION 354
#define TOK_SHOW 355
#define TOK_SONAME 356
#define TOK_START 357
#define TOK_STATUS 358
#define TOK_STRING 359
#define TOK_SUM 360
#define TOK_TABLE 361
#define TOK_TABLES 362
#define TOK_THREADS 363
#define TOK_TO 364
#define TOK_TRANSACTION 365
#define TOK_TRUE 366
#define TOK_TRUNCATE 367
#define TOK_TYPE 368
#define TOK_UNCOMMITTED 369
#define TOK_UPDATE 370
#define TOK_VALUES 371
#define TOK_VARIABLES 372
#define TOK_WARNINGS 373
#define TOK_WEIGHT 374
#define TOK_WHERE 375
#define TOK_WITHIN 376
#define TOK_OR 377
#define TOK_AND 378
#define TOK_NE 379
#define TOK_GTE 380
#define TOK_LTE 381
#define TOK_NOT 382
#define TOK_NEG 383




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

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	_res.m_iEnd = _right.m_iEnd; \
	_res.m_iType = 0;



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
#define YYFINAL  190
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4472

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  148
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  134
/* YYNRULES -- Number of rules. */
#define YYNRULES  429
/* YYNRULES -- Number of states. */
#define YYNSTATES  783

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   383

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   136,   125,     2,
     140,   141,   134,   132,   142,   133,   145,   135,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   139,
     128,   126,   129,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   146,     2,   147,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   143,   124,   144,     2,     2,     2,     2,
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
     115,   116,   117,   118,   119,   120,   121,   122,   123,   127,
     130,   131,   137,   138
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,    10,    12,    14,    16,    18,
      20,    22,    24,    26,    28,    30,    32,    34,    36,    38,
      40,    42,    44,    46,    48,    50,    52,    54,    56,    58,
      60,    62,    64,    66,    68,    70,    72,    74,    76,    78,
      80,    82,    84,    86,    88,    90,    92,    94,    96,    98,
     100,   102,   104,   106,   108,   110,   112,   114,   116,   118,
     120,   122,   124,   126,   128,   130,   132,   134,   136,   138,
     140,   142,   144,   146,   148,   150,   152,   154,   156,   158,
     160,   162,   164,   168,   171,   173,   175,   177,   186,   188,
     198,   199,   202,   204,   208,   210,   212,   214,   215,   219,
     220,   223,   228,   240,   242,   246,   248,   251,   252,   254,
     257,   259,   264,   269,   274,   279,   284,   289,   293,   299,
     301,   305,   306,   308,   311,   313,   317,   321,   326,   328,
     332,   336,   342,   349,   353,   358,   364,   368,   372,   376,
     380,   384,   386,   392,   398,   404,   408,   412,   416,   420,
     424,   428,   432,   437,   441,   443,   445,   450,   454,   458,
     460,   462,   467,   472,   477,   481,   483,   486,   488,   491,
     493,   495,   499,   500,   505,   506,   508,   510,   514,   515,
     518,   519,   521,   527,   528,   530,   534,   540,   542,   546,
     548,   551,   554,   555,   557,   560,   565,   566,   568,   571,
     573,   577,   581,   585,   591,   598,   602,   604,   608,   612,
     614,   616,   618,   620,   622,   624,   626,   629,   632,   636,
     640,   644,   648,   652,   656,   660,   664,   668,   672,   676,
     680,   684,   688,   692,   696,   700,   704,   708,   710,   712,
     714,   718,   723,   728,   733,   738,   743,   748,   753,   757,
     764,   771,   775,   784,   799,   801,   805,   807,   809,   813,
     819,   821,   823,   825,   827,   830,   831,   834,   836,   839,
     842,   846,   848,   850,   852,   855,   860,   865,   869,   874,
     879,   881,   883,   884,   887,   892,   897,   902,   906,   911,
     916,   924,   930,   936,   946,   948,   950,   952,   954,   956,
     958,   962,   964,   966,   968,   970,   972,   974,   976,   978,
     980,   983,   991,   993,   995,   996,  1000,  1002,  1004,  1006,
    1010,  1012,  1016,  1020,  1022,  1026,  1028,  1030,  1032,  1036,
    1039,  1040,  1043,  1045,  1049,  1053,  1058,  1065,  1067,  1071,
    1073,  1077,  1079,  1083,  1084,  1087,  1089,  1093,  1097,  1098,
    1100,  1102,  1104,  1108,  1110,  1112,  1116,  1120,  1127,  1129,
    1133,  1137,  1141,  1147,  1152,  1156,  1160,  1162,  1164,  1166,
    1168,  1170,  1172,  1174,  1176,  1184,  1191,  1196,  1201,  1207,
    1208,  1210,  1213,  1215,  1219,  1223,  1226,  1230,  1237,  1238,
    1240,  1242,  1245,  1248,  1251,  1253,  1261,  1263,  1265,  1267,
    1269,  1273,  1280,  1284,  1288,  1292,  1294,  1298,  1301,  1305,
    1309,  1317,  1323,  1326,  1328,  1331,  1333,  1335,  1339,  1343,
    1347,  1351,  1353,  1354,  1357,  1359,  1362,  1364,  1366,  1370
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     149,     0,    -1,   150,    -1,   153,    -1,   153,   139,    -1,
     217,    -1,   229,    -1,   209,    -1,   210,    -1,   215,    -1,
     230,    -1,   239,    -1,   241,    -1,   242,    -1,   243,    -1,
     248,    -1,   253,    -1,   254,    -1,   258,    -1,   260,    -1,
     261,    -1,   262,    -1,   263,    -1,   255,    -1,   264,    -1,
     266,    -1,   267,    -1,   268,    -1,   247,    -1,   269,    -1,
     270,    -1,     3,    -1,    16,    -1,    20,    -1,    21,    -1,
      22,    -1,    25,    -1,    30,    -1,    34,    -1,    47,    -1,
      50,    -1,    51,    -1,    52,    -1,    53,    -1,    54,    -1,
      64,    -1,    66,    -1,    67,    -1,    69,    -1,    70,    -1,
      71,    -1,    81,    -1,    82,    -1,    83,    -1,    84,    -1,
      86,    -1,    85,    -1,    87,    -1,    89,    -1,    92,    -1,
      93,    -1,    94,    -1,    96,    -1,    99,    -1,   102,    -1,
     103,    -1,   104,    -1,   105,    -1,   107,    -1,   108,    -1,
     112,    -1,   113,    -1,   114,    -1,   117,    -1,   118,    -1,
     119,    -1,   121,    -1,    65,    -1,   151,    -1,    76,    -1,
     110,    -1,   154,    -1,   153,   139,   154,    -1,   153,   281,
      -1,   155,    -1,   204,    -1,   156,    -1,    95,     3,   140,
     140,   156,   141,   157,   141,    -1,   163,    -1,    95,   164,
      49,   140,   160,   163,   141,   161,   162,    -1,    -1,   142,
     158,    -1,   159,    -1,   158,   142,   159,    -1,   152,    -1,
       5,    -1,    56,    -1,    -1,    79,    26,   187,    -1,    -1,
      68,     5,    -1,    68,     5,   142,     5,    -1,    95,   164,
      49,   168,   169,   179,   183,   182,   185,   189,   191,    -1,
     165,    -1,   164,   142,   165,    -1,   134,    -1,   167,   166,
      -1,    -1,   152,    -1,    18,   152,    -1,   197,    -1,    21,
     140,   197,   141,    -1,    70,   140,   197,   141,    -1,    72,
     140,   197,   141,    -1,   105,   140,   197,   141,    -1,    54,
     140,   197,   141,    -1,    34,   140,   134,   141,    -1,    53,
     140,   141,    -1,    34,   140,    40,   152,   141,    -1,   152,
      -1,   168,   142,   152,    -1,    -1,   170,    -1,   120,   171,
      -1,   172,    -1,   171,   123,   171,    -1,   140,   171,   141,
      -1,    69,   140,     8,   141,    -1,   173,    -1,   175,   126,
     176,    -1,   175,   127,   176,    -1,   175,    57,   140,   178,
     141,    -1,   175,   137,    57,   140,   178,   141,    -1,   175,
      57,     9,    -1,   175,   137,    57,     9,    -1,   175,    23,
     176,   123,   176,    -1,   175,   129,   176,    -1,   175,   128,
     176,    -1,   175,   130,   176,    -1,   175,   131,   176,    -1,
     175,   126,   177,    -1,   174,    -1,   175,    23,   177,   123,
     177,    -1,   175,    23,   176,   123,   177,    -1,   175,    23,
     177,   123,   176,    -1,   175,   129,   177,    -1,   175,   128,
     177,    -1,   175,   130,   177,    -1,   175,   131,   177,    -1,
     175,   126,     8,    -1,   175,   127,     8,    -1,   175,    63,
      77,    -1,   175,    63,   137,    77,    -1,   175,   127,   177,
      -1,   152,    -1,     4,    -1,    34,   140,   134,   141,    -1,
      53,   140,   141,    -1,   119,   140,   141,    -1,    56,    -1,
     271,    -1,    61,   140,   271,   141,    -1,    42,   140,   271,
     141,    -1,    24,   140,   271,   141,    -1,    44,   140,   141,
      -1,     5,    -1,   133,     5,    -1,     6,    -1,   133,     6,
      -1,    14,    -1,   176,    -1,   178,   142,   176,    -1,    -1,
      52,   180,    26,   181,    -1,    -1,     5,    -1,   175,    -1,
     181,   142,   175,    -1,    -1,    55,   173,    -1,    -1,   184,
      -1,   121,    52,    79,    26,   187,    -1,    -1,   186,    -1,
      79,    26,   187,    -1,    79,    26,    85,   140,   141,    -1,
     188,    -1,   187,   142,   188,    -1,   175,    -1,   175,    19,
      -1,   175,    38,    -1,    -1,   190,    -1,    68,     5,    -1,
      68,     5,   142,     5,    -1,    -1,   192,    -1,    78,   193,
      -1,   194,    -1,   193,   142,   194,    -1,   152,   126,   152,
      -1,   152,   126,     5,    -1,   152,   126,   140,   195,   141,
      -1,   152,   126,   152,   140,     8,   141,    -1,   152,   126,
       8,    -1,   196,    -1,   195,   142,   196,    -1,   152,   126,
     176,    -1,   152,    -1,     4,    -1,    56,    -1,     5,    -1,
       6,    -1,    14,    -1,     9,    -1,   133,   197,    -1,   137,
     197,    -1,   197,   132,   197,    -1,   197,   133,   197,    -1,
     197,   134,   197,    -1,   197,   135,   197,    -1,   197,   128,
     197,    -1,   197,   129,   197,    -1,   197,   125,   197,    -1,
     197,   124,   197,    -1,   197,   136,   197,    -1,   197,    41,
     197,    -1,   197,    73,   197,    -1,   197,   131,   197,    -1,
     197,   130,   197,    -1,   197,   126,   197,    -1,   197,   127,
     197,    -1,   197,   123,   197,    -1,   197,   122,   197,    -1,
     140,   197,   141,    -1,   143,   201,   144,    -1,   198,    -1,
     271,    -1,   274,    -1,   271,    63,    77,    -1,   271,    63,
     137,    77,    -1,     3,   140,   199,   141,    -1,    57,   140,
     199,   141,    -1,    61,   140,   199,   141,    -1,    24,   140,
     199,   141,    -1,    46,   140,   199,   141,    -1,    42,   140,
     199,   141,    -1,     3,   140,   141,    -1,    72,   140,   197,
     142,   197,   141,    -1,    70,   140,   197,   142,   197,   141,
      -1,   119,   140,   141,    -1,     3,   140,   197,    48,   152,
      57,   271,   141,    -1,    91,   140,   197,   142,   197,   142,
     140,   199,   141,   142,   140,   199,   141,   141,    -1,   200,
      -1,   199,   142,   200,    -1,   197,    -1,     8,    -1,   202,
     126,   203,    -1,   201,   142,   202,   126,   203,    -1,   152,
      -1,    57,    -1,   176,    -1,   152,    -1,   100,   206,    -1,
      -1,    67,     8,    -1,   118,    -1,   103,   205,    -1,    71,
     205,    -1,    16,   103,   205,    -1,    84,    -1,    81,    -1,
      83,    -1,   108,   191,    -1,    16,     8,   103,   205,    -1,
      16,   152,   103,   205,    -1,   207,   152,   103,    -1,   207,
     152,   208,    98,    -1,   207,   152,    14,    98,    -1,    58,
      -1,   106,    -1,    -1,    29,     5,    -1,    97,   151,   126,
     212,    -1,    97,   151,   126,   211,    -1,    97,   151,   126,
      77,    -1,    97,    76,   213,    -1,    97,    10,   126,   213,
      -1,    97,    28,    97,   213,    -1,    97,    51,     9,   126,
     140,   178,   141,    -1,    97,    51,   151,   126,   211,    -1,
      97,    51,   151,   126,     5,    -1,    97,    58,   152,    51,
       9,   126,   140,   178,   141,    -1,   152,    -1,     8,    -1,
     111,    -1,    45,    -1,   176,    -1,   214,    -1,   213,   133,
     214,    -1,   152,    -1,    77,    -1,     8,    -1,     5,    -1,
       6,    -1,    32,    -1,    93,    -1,   216,    -1,    22,    -1,
     102,   110,    -1,   218,    62,   152,   219,   116,   222,   226,
      -1,    59,    -1,    90,    -1,    -1,   140,   221,   141,    -1,
     152,    -1,    56,    -1,   220,    -1,   221,   142,   220,    -1,
     223,    -1,   222,   142,   223,    -1,   140,   224,   141,    -1,
     225,    -1,   224,   142,   225,    -1,   176,    -1,   177,    -1,
       8,    -1,   140,   178,   141,    -1,   140,   141,    -1,    -1,
      78,   227,    -1,   228,    -1,   227,   142,   228,    -1,     3,
     126,     8,    -1,    37,    49,   168,   170,    -1,    27,   152,
     140,   231,   234,   141,    -1,   232,    -1,   231,   142,   232,
      -1,   225,    -1,   140,   233,   141,    -1,     8,    -1,   233,
     142,     8,    -1,    -1,   142,   235,    -1,   236,    -1,   235,
     142,   236,    -1,   225,   237,   238,    -1,    -1,    18,    -1,
     152,    -1,    68,    -1,   240,   152,   205,    -1,    39,    -1,
      38,    -1,   100,   107,   205,    -1,   100,    36,   205,    -1,
     115,   168,    97,   244,   170,   191,    -1,   245,    -1,   244,
     142,   245,    -1,   152,   126,   176,    -1,   152,   126,   177,
      -1,   152,   126,   140,   178,   141,    -1,   152,   126,   140,
     141,    -1,   271,   126,   176,    -1,   271,   126,   177,    -1,
      61,    -1,    24,    -1,    46,    -1,    25,    -1,    74,    -1,
      75,    -1,    65,    -1,   104,    -1,    17,   106,   152,    15,
      31,   152,   246,    -1,    17,   106,   152,    43,    31,   152,
      -1,    17,    94,   152,    88,    -1,   100,   256,   117,   249,
      -1,   100,   256,   117,    67,     8,    -1,    -1,   250,    -1,
     120,   251,    -1,   252,    -1,   251,   122,   252,    -1,   152,
     126,     8,    -1,   100,    30,    -1,   100,    28,    97,    -1,
      97,   256,   110,    64,    66,   257,    -1,    -1,    51,    -1,
      99,    -1,    87,   114,    -1,    87,    33,    -1,    89,    87,
      -1,    96,    -1,    35,    50,   152,    92,   259,   101,     8,
      -1,    60,    -1,    24,    -1,    46,    -1,   104,    -1,    43,
      50,   152,    -1,    20,    58,   152,   109,    94,   152,    -1,
      47,    94,   152,    -1,    47,    86,   152,    -1,    95,   265,
     189,    -1,    10,    -1,    10,   145,   152,    -1,    95,   197,
      -1,   112,    94,   152,    -1,    80,    58,   152,    -1,    35,
      82,   152,   113,     8,   101,     8,    -1,    43,    82,   152,
     113,     8,    -1,   152,   272,    -1,   273,    -1,   272,   273,
      -1,    13,    -1,    14,    -1,   146,   197,   147,    -1,   146,
       8,   147,    -1,   197,   126,   275,    -1,   275,   126,   197,
      -1,     8,    -1,    -1,   277,   280,    -1,    26,    -1,   279,
     166,    -1,   197,    -1,   278,    -1,   280,   142,   278,    -1,
      44,   280,   276,   185,   189,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   174,   174,   175,   176,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   216,   217,   217,   217,   217,   217,   218,   218,   218,
     218,   219,   219,   219,   219,   219,   220,   220,   220,   220,
     220,   221,   221,   221,   221,   221,   221,   221,   222,   222,
     222,   222,   223,   223,   223,   223,   223,   224,   224,   224,
     224,   224,   224,   225,   225,   225,   225,   226,   230,   230,
     230,   236,   237,   238,   242,   243,   247,   248,   256,   257,
     264,   266,   270,   274,   281,   282,   283,   287,   300,   307,
     309,   314,   323,   339,   340,   344,   345,   348,   350,   351,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   367,
     368,   371,   373,   377,   381,   382,   383,   387,   392,   396,
     403,   411,   419,   428,   433,   438,   443,   448,   453,   458,
     463,   468,   473,   478,   483,   488,   493,   498,   503,   508,
     513,   518,   523,   531,   535,   536,   541,   547,   553,   559,
     565,   566,   567,   568,   569,   573,   574,   585,   586,   587,
     591,   597,   603,   605,   608,   610,   617,   621,   627,   629,
     635,   637,   641,   652,   654,   658,   662,   669,   670,   674,
     675,   676,   679,   681,   685,   690,   697,   699,   703,   707,
     708,   712,   717,   722,   728,   733,   741,   746,   753,   763,
     764,   765,   766,   767,   768,   769,   770,   771,   773,   774,
     775,   776,   777,   778,   779,   780,   781,   782,   783,   784,
     785,   786,   787,   788,   789,   790,   791,   792,   793,   794,
     795,   796,   800,   801,   802,   803,   804,   805,   806,   807,
     808,   809,   810,   811,   815,   816,   820,   821,   825,   826,
     830,   831,   835,   836,   842,   845,   847,   851,   852,   853,
     854,   855,   856,   857,   858,   859,   864,   869,   874,   879,
     888,   889,   892,   894,   902,   907,   912,   917,   918,   919,
     923,   928,   933,   938,   947,   948,   952,   953,   954,   966,
     967,   971,   972,   973,   974,   975,   982,   983,   984,   988,
     989,   995,  1003,  1004,  1007,  1009,  1013,  1014,  1018,  1019,
    1023,  1024,  1028,  1032,  1033,  1037,  1038,  1039,  1040,  1041,
    1044,  1045,  1049,  1050,  1054,  1060,  1070,  1078,  1082,  1089,
    1090,  1097,  1107,  1113,  1115,  1119,  1124,  1128,  1135,  1137,
    1141,  1142,  1148,  1156,  1157,  1163,  1167,  1173,  1181,  1182,
    1186,  1200,  1206,  1210,  1215,  1229,  1240,  1241,  1242,  1243,
    1244,  1245,  1246,  1247,  1251,  1259,  1266,  1277,  1281,  1288,
    1289,  1293,  1297,  1298,  1302,  1306,  1313,  1320,  1326,  1327,
    1328,  1332,  1333,  1334,  1335,  1341,  1352,  1353,  1354,  1355,
    1360,  1371,  1383,  1392,  1403,  1411,  1412,  1416,  1426,  1437,
    1448,  1459,  1470,  1473,  1474,  1478,  1479,  1480,  1481,  1485,
    1486,  1490,  1495,  1497,  1501,  1510,  1514,  1522,  1523,  1527
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_IDENT", "TOK_ATIDENT", 
  "TOK_CONST_INT", "TOK_CONST_FLOAT", "TOK_CONST_MVA", 
  "TOK_QUOTED_STRING", "TOK_USERVAR", "TOK_SYSVAR", "TOK_CONST_STRINGS", 
  "TOK_BAD_NUMERIC", "TOK_SUBKEY", "TOK_DOT_NUMBER", "TOK_ADD", 
  "TOK_AGENT", "TOK_ALTER", "TOK_AS", "TOK_ASC", "TOK_ATTACH", "TOK_AVG", 
  "TOK_BEGIN", "TOK_BETWEEN", "TOK_BIGINT", "TOK_BOOL", "TOK_BY", 
  "TOK_CALL", "TOK_CHARACTER", "TOK_CHUNK", "TOK_COLLATION", "TOK_COLUMN", 
  "TOK_COMMIT", "TOK_COMMITTED", "TOK_COUNT", "TOK_CREATE", 
  "TOK_DATABASES", "TOK_DELETE", "TOK_DESC", "TOK_DESCRIBE", 
  "TOK_DISTINCT", "TOK_DIV", "TOK_DOUBLE", "TOK_DROP", "TOK_FACET", 
  "TOK_FALSE", "TOK_FLOAT", "TOK_FLUSH", "TOK_FOR", "TOK_FROM", 
  "TOK_FUNCTION", "TOK_GLOBAL", "TOK_GROUP", "TOK_GROUPBY", 
  "TOK_GROUP_CONCAT", "TOK_HAVING", "TOK_ID", "TOK_IN", "TOK_INDEX", 
  "TOK_INSERT", "TOK_INT", "TOK_INTEGER", "TOK_INTO", "TOK_IS", 
  "TOK_ISOLATION", "TOK_JSON", "TOK_LEVEL", "TOK_LIKE", "TOK_LIMIT", 
  "TOK_MATCH", "TOK_MAX", "TOK_META", "TOK_MIN", "TOK_MOD", "TOK_MULTI", 
  "TOK_MULTI64", "TOK_NAMES", "TOK_NULL", "TOK_OPTION", "TOK_ORDER", 
  "TOK_OPTIMIZE", "TOK_PLAN", "TOK_PLUGIN", "TOK_PLUGINS", "TOK_PROFILE", 
  "TOK_RAND", "TOK_RAMCHUNK", "TOK_READ", "TOK_RECONFIGURE", 
  "TOK_REPEATABLE", "TOK_REPLACE", "TOK_REMAP", "TOK_RETURNS", 
  "TOK_ROLLBACK", "TOK_RTINDEX", "TOK_SELECT", "TOK_SERIALIZABLE", 
  "TOK_SET", "TOK_SETTINGS", "TOK_SESSION", "TOK_SHOW", "TOK_SONAME", 
  "TOK_START", "TOK_STATUS", "TOK_STRING", "TOK_SUM", "TOK_TABLE", 
  "TOK_TABLES", "TOK_THREADS", "TOK_TO", "TOK_TRANSACTION", "TOK_TRUE", 
  "TOK_TRUNCATE", "TOK_TYPE", "TOK_UNCOMMITTED", "TOK_UPDATE", 
  "TOK_VALUES", "TOK_VARIABLES", "TOK_WARNINGS", "TOK_WEIGHT", 
  "TOK_WHERE", "TOK_WITHIN", "TOK_OR", "TOK_AND", "'|'", "'&'", "'='", 
  "TOK_NE", "'<'", "'>'", "TOK_GTE", "TOK_LTE", "'+'", "'-'", "'*'", 
  "'/'", "'%'", "TOK_NOT", "TOK_NEG", "';'", "'('", "')'", "','", "'{'", 
  "'}'", "'.'", "'['", "']'", "$accept", "request", "statement", 
  "ident_set", "ident", "multi_stmt_list", "multi_stmt", "select", 
  "select1", "opt_tablefunc_args", "tablefunc_args_list", "tablefunc_arg", 
  "subselect_start", "opt_outer_order", "opt_outer_limit", "select_from", 
  "select_items_list", "select_item", "opt_alias", "select_expr", 
  "ident_list", "opt_where_clause", "where_clause", "where_expr", 
  "where_item", "filter_item", "expr_float_unhandled", "expr_ident", 
  "const_int", "const_float", "const_list", "opt_group_clause", "opt_int", 
  "group_items_list", "opt_having_clause", "opt_group_order_clause", 
  "group_order_clause", "opt_order_clause", "order_clause", 
  "order_items_list", "order_item", "opt_limit_clause", "limit_clause", 
  "opt_option_clause", "option_clause", "option_list", "option_item", 
  "named_const_list", "named_const", "expr", "function", "arglist", "arg", 
  "consthash", "hash_key", "hash_val", "show_stmt", "like_filter", 
  "show_what", "index_or_table", "opt_chunk", "set_stmt", 
  "set_global_stmt", "set_string_value", "boolean_value", "set_value", 
  "simple_set_value", "transact_op", "start_transaction", "insert_into", 
  "insert_or_replace", "opt_column_list", "column_ident", "column_list", 
  "insert_rows_list", "insert_row", "insert_vals_list", "insert_val", 
  "opt_insert_options", "insert_options_list", "insert_option", 
  "delete_from", "call_proc", "call_args_list", "call_arg", 
  "const_string_list", "opt_call_opts_list", "call_opts_list", "call_opt", 
  "opt_as", "call_opt_name", "describe", "describe_tok", "show_tables", 
  "show_databases", "update", "update_items_list", "update_item", 
  "alter_col_type", "alter", "show_variables", "opt_show_variables_where", 
  "show_variables_where", "show_variables_where_list", 
  "show_variables_where_entry", "show_collation", "show_character_set", 
  "set_transaction", "opt_scope", "isolation_level", "create_function", 
  "udf_type", "drop_function", "attach_index", "flush_rtindex", 
  "flush_ramchunk", "select_sysvar", "sysvar_name", "select_dual", 
  "truncate", "optimize_index", "create_plugin", "drop_plugin", 
  "json_field", "subscript", "subkey", "streq", "strval", 
  "opt_facet_by_items_list", "facet_by", "facet_item", "facet_expr", 
  "facet_items_list", "facet_stmt", 0
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
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   124,    38,    61,   379,    60,    62,
     380,   381,    43,    45,    42,    47,    37,   382,   383,    59,
      40,    41,    44,   123,   125,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   148,   149,   149,   149,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   152,   152,
     152,   153,   153,   153,   154,   154,   155,   155,   156,   156,
     157,   157,   158,   158,   159,   159,   159,   160,   161,   162,
     162,   162,   163,   164,   164,   165,   165,   166,   166,   166,
     167,   167,   167,   167,   167,   167,   167,   167,   167,   168,
     168,   169,   169,   170,   171,   171,   171,   172,   172,   173,
     173,   173,   173,   173,   173,   173,   173,   173,   173,   173,
     173,   173,   173,   173,   173,   173,   173,   173,   173,   173,
     173,   173,   173,   174,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   176,   176,   177,   177,   177,
     178,   178,   179,   179,   180,   180,   181,   181,   182,   182,
     183,   183,   184,   185,   185,   186,   186,   187,   187,   188,
     188,   188,   189,   189,   190,   190,   191,   191,   192,   193,
     193,   194,   194,   194,   194,   194,   195,   195,   196,   197,
     197,   197,   197,   197,   197,   197,   197,   197,   197,   197,
     197,   197,   197,   197,   197,   197,   197,   197,   197,   197,
     197,   197,   197,   197,   197,   197,   197,   197,   197,   197,
     197,   197,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   198,   198,   199,   199,   200,   200,   201,   201,
     202,   202,   203,   203,   204,   205,   205,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     207,   207,   208,   208,   209,   209,   209,   209,   209,   209,
     210,   210,   210,   210,   211,   211,   212,   212,   212,   213,
     213,   214,   214,   214,   214,   214,   215,   215,   215,   216,
     216,   217,   218,   218,   219,   219,   220,   220,   221,   221,
     222,   222,   223,   224,   224,   225,   225,   225,   225,   225,
     226,   226,   227,   227,   228,   229,   230,   231,   231,   232,
     232,   233,   233,   234,   234,   235,   235,   236,   237,   237,
     238,   238,   239,   240,   240,   241,   242,   243,   244,   244,
     245,   245,   245,   245,   245,   245,   246,   246,   246,   246,
     246,   246,   246,   246,   247,   247,   247,   248,   248,   249,
     249,   250,   251,   251,   252,   253,   254,   255,   256,   256,
     256,   257,   257,   257,   257,   258,   259,   259,   259,   259,
     260,   261,   262,   263,   264,   265,   265,   266,   267,   268,
     269,   270,   271,   272,   272,   273,   273,   273,   273,   274,
     274,   275,   276,   276,   277,   278,   279,   280,   280,   281
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     1,     1,     8,     1,     9,
       0,     2,     1,     3,     1,     1,     1,     0,     3,     0,
       2,     4,    11,     1,     3,     1,     2,     0,     1,     2,
       1,     4,     4,     4,     4,     4,     4,     3,     5,     1,
       3,     0,     1,     2,     1,     3,     3,     4,     1,     3,
       3,     5,     6,     3,     4,     5,     3,     3,     3,     3,
       3,     1,     5,     5,     5,     3,     3,     3,     3,     3,
       3,     3,     4,     3,     1,     1,     4,     3,     3,     1,
       1,     4,     4,     4,     3,     1,     2,     1,     2,     1,
       1,     3,     0,     4,     0,     1,     1,     3,     0,     2,
       0,     1,     5,     0,     1,     3,     5,     1,     3,     1,
       2,     2,     0,     1,     2,     4,     0,     1,     2,     1,
       3,     3,     3,     5,     6,     3,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       3,     4,     4,     4,     4,     4,     4,     4,     3,     6,
       6,     3,     8,    14,     1,     3,     1,     1,     3,     5,
       1,     1,     1,     1,     2,     0,     2,     1,     2,     2,
       3,     1,     1,     1,     2,     4,     4,     3,     4,     4,
       1,     1,     0,     2,     4,     4,     4,     3,     4,     4,
       7,     5,     5,     9,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     7,     1,     1,     0,     3,     1,     1,     1,     3,
       1,     3,     3,     1,     3,     1,     1,     1,     3,     2,
       0,     2,     1,     3,     3,     4,     6,     1,     3,     1,
       3,     1,     3,     0,     2,     1,     3,     3,     0,     1,
       1,     1,     3,     1,     1,     3,     3,     6,     1,     3,
       3,     3,     5,     4,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     7,     6,     4,     4,     5,     0,
       1,     2,     1,     3,     3,     2,     3,     6,     0,     1,
       1,     2,     2,     2,     1,     7,     1,     1,     1,     1,
       3,     6,     3,     3,     3,     1,     3,     2,     3,     3,
       7,     5,     2,     1,     2,     1,     1,     3,     3,     3,
       3,     1,     0,     2,     1,     2,     1,     1,     3,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     0,     0,   309,     0,   306,     0,     0,   354,   353,
       0,     0,   312,     0,   313,   307,     0,   388,   388,     0,
       0,     0,     0,     2,     3,    81,    84,    86,    88,    85,
       7,     8,     9,   308,     5,     0,     6,    10,    11,     0,
      12,    13,    14,    28,    15,    16,    17,    23,    18,    19,
      20,    21,    22,    24,    25,    26,    27,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    77,    46,    47,
      48,    49,    50,    79,    51,    52,    53,    54,    56,    55,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    80,    70,    71,    72,    73,    74,    75,
      76,    78,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    31,   210,   212,   213,   421,   215,   405,   214,    34,
       0,    38,     0,     0,    43,    44,   211,     0,     0,    49,
       0,     0,    67,    75,     0,   105,     0,     0,     0,   209,
       0,   103,   107,   110,   237,   192,   238,   239,     0,     0,
       0,    41,     0,     0,    63,     0,     0,     0,     0,   385,
     265,   389,   280,   265,   272,   273,   271,   390,   265,   281,
     265,   196,   267,   264,     0,     0,   310,     0,   119,     0,
       1,     0,     4,    83,     0,   265,     0,     0,     0,     0,
       0,     0,     0,   400,     0,   403,   402,   409,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    31,    49,     0,   216,   217,     0,
     261,   260,     0,     0,   415,   416,     0,   412,   413,     0,
       0,     0,   108,   106,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   404,   193,     0,     0,     0,     0,     0,     0,
       0,   304,   305,   303,   302,   301,   287,   299,     0,     0,
       0,   265,     0,   386,     0,   356,   269,   268,   355,     0,
     274,   197,   282,   379,   408,     0,     0,   426,   427,   107,
     422,     0,     0,    82,   314,   352,   376,     0,     0,     0,
     165,   167,   327,   169,     0,     0,   325,   326,   339,   343,
     337,     0,     0,     0,   335,     0,   257,     0,   248,   256,
       0,   254,   406,     0,   256,     0,     0,     0,     0,     0,
     117,     0,     0,     0,     0,     0,     0,     0,   251,     0,
       0,     0,   235,     0,   236,     0,   421,     0,   414,    97,
     121,   104,   110,   109,   227,   228,   234,   233,   225,   224,
     231,   419,   232,   222,   223,   230,   229,   218,   219,   220,
     221,   226,   194,   240,     0,   420,   288,   289,     0,     0,
       0,     0,   295,   297,   286,   296,     0,   294,   298,   285,
     284,     0,   265,   270,   265,   266,     0,   198,   199,     0,
       0,   277,     0,     0,     0,   377,   380,     0,     0,   358,
       0,   120,   425,   424,     0,   183,     0,     0,     0,     0,
       0,     0,   166,   168,   341,   329,   170,     0,     0,     0,
       0,   397,   398,   396,   399,     0,     0,   155,     0,    38,
       0,     0,    43,   159,     0,    48,    75,     0,   154,   123,
     124,   128,   141,     0,   160,   411,     0,     0,     0,   242,
       0,   111,   245,     0,   116,   247,   246,   115,   243,   244,
     112,     0,   113,     0,     0,   114,     0,     0,     0,   263,
     262,   258,   418,   417,     0,   172,   122,     0,   241,     0,
     292,   291,     0,   300,     0,   275,   276,     0,     0,   279,
     283,   278,   378,     0,   381,   382,     0,     0,   196,     0,
     428,     0,   192,   184,   423,   317,   316,   318,     0,     0,
       0,   375,   401,   328,     0,   340,     0,   348,   338,   344,
     345,   336,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    90,     0,   255,   118,     0,     0,
       0,     0,     0,     0,   174,   180,   195,     0,     0,     0,
       0,   394,   387,   202,   205,     0,   201,   200,     0,     0,
       0,   360,   361,   359,   357,   364,   365,     0,   429,   315,
       0,     0,   330,   320,   367,   369,   368,   366,   372,   370,
     371,   373,   374,   171,   342,   349,     0,     0,   395,   410,
       0,     0,     0,     0,   164,   157,     0,     0,   158,   126,
     125,     0,     0,   133,     0,   151,     0,   149,   129,   140,
     150,   130,   153,   137,   146,   136,   145,   138,   147,   139,
     148,     0,     0,     0,     0,   250,   249,     0,   259,     0,
       0,   175,     0,     0,   178,   181,   290,     0,   392,   391,
     393,     0,     0,   206,     0,   384,   383,   363,     0,    56,
     189,   185,   187,   319,     0,     0,   323,     0,     0,   311,
     351,   350,   347,   348,   346,   163,   156,   162,   161,   127,
       0,     0,     0,   152,   134,     0,    95,    96,    94,    91,
      92,    87,     0,     0,     0,     0,    99,     0,     0,     0,
     183,     0,     0,   203,     0,     0,   362,     0,   190,   191,
       0,   322,     0,     0,   331,   332,   321,   135,   143,   144,
     142,   131,     0,     0,   252,     0,     0,     0,    89,   176,
     173,     0,   179,   192,   293,   208,   207,   204,   186,   188,
     324,     0,     0,   132,    93,     0,    98,   100,     0,     0,
     196,   334,   333,     0,     0,   177,   182,   102,     0,   101,
       0,     0,   253
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    22,    23,   111,   149,    24,    25,    26,    27,   653,
     709,   710,   494,   716,   748,    28,   150,   151,   243,   152,
     360,   495,   324,   459,   460,   461,   462,   463,   436,   317,
     437,   575,   662,   750,   720,   664,   665,   522,   523,   681,
     682,   262,   263,   290,   291,   407,   408,   672,   673,   334,
     154,   330,   331,   232,   233,   491,    29,   285,   183,   184,
     412,    30,    31,   399,   400,   276,   277,    32,    33,    34,
      35,   428,   527,   528,   602,   603,   685,   318,   689,   734,
     735,    36,    37,   319,   320,   438,   440,   539,   540,   616,
     692,    38,    39,    40,    41,    42,   418,   419,   612,    43,
      44,   415,   416,   514,   515,    45,    46,    47,   166,   582,
      48,   445,    49,    50,    51,    52,    53,   155,    54,    55,
      56,    57,    58,   156,   237,   238,   157,   158,   425,   426,
     298,   299,   300,   193
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -597
static const short yypact[] =
{
    4306,    80,    17,  -597,  3753,  -597,     8,    47,  -597,  -597,
      23,    44,  -597,    65,  -597,  -597,   724,  2892,  3848,    27,
      71,  3753,   141,  -597,   -21,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,   106,  -597,  -597,  -597,  3753,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  3753,
    3753,  3753,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,    30,  3753,  3753,  3753,  3753,  3753,  3753,  3753,
    3753,    32,  -597,  -597,  -597,  -597,  -597,    42,  -597,    38,
      51,    92,    96,   116,   118,   121,  -597,   126,   130,   140,
     143,   153,   156,   158,  1570,  -597,  1570,  1570,  3221,    33,
     -15,  -597,  3327,    94,  -597,   128,   186,  -597,    89,   138,
     203,   604,  3753,  2786,   204,   189,   207,  3435,   226,  -597,
     259,  -597,  -597,   259,  -597,  -597,  -597,  -597,   259,  -597,
     259,   253,  -597,  -597,  3753,   216,  -597,  3753,  -597,   -45,
    -597,  1570,    25,  -597,  3753,   259,   281,    56,   263,    31,
     284,   265,   -34,  -597,   270,  -597,  -597,  -597,   865,  3753,
    1570,  1711,   -13,  1711,  1711,   252,  1570,  1711,  1711,  1570,
    1570,  1570,  1570,   254,   256,   257,   258,  -597,  -597,  4026,
    -597,  -597,   -38,   268,  -597,  -597,  1852,    33,  -597,  2202,
    1006,  3753,  -597,  -597,  1570,  1570,  1570,  1570,  1570,  1570,
    1570,  1570,  1570,  1570,  1570,  1570,  1570,  1570,  1570,  1570,
    1570,   395,  -597,  -597,   -46,  1570,  2786,  2786,   275,   277,
     353,  -597,  -597,  -597,  -597,  -597,   272,  -597,  2319,   342,
     305,   -16,   309,  -597,   405,  -597,  -597,  -597,  -597,  3753,
    -597,  -597,    74,   -18,  -597,  3753,  3753,  4322,  -597,  3327,
      -6,  1147,   176,  -597,   274,  -597,  -597,   384,   387,   325,
    -597,  -597,  -597,  -597,   198,    16,  -597,  -597,  -597,   280,
    -597,   221,   416,  1977,  -597,   420,   303,  1288,  -597,  4307,
      69,  -597,  -597,  4047,  4322,   102,  3753,   291,   109,   127,
    -597,  4122,   131,   134,  3845,  3866,  3887,  4143,  -597,  1429,
    1570,  1570,  -597,  3221,  -597,  2438,   286,   343,  -597,  -597,
     -34,  -597,  4322,  -597,  -597,  -597,  4336,   390,  2004,  2111,
     173,  -597,   173,    66,    66,    66,    66,    59,    59,  -597,
    -597,  -597,   292,  -597,   359,   173,   272,   272,   298,  3009,
     430,  2786,  -597,  -597,  -597,  -597,   441,  -597,  -597,  -597,
    -597,   381,   259,  -597,   259,  -597,   322,   307,  -597,   354,
     446,  -597,   357,   448,  3753,  -597,  -597,    63,   -25,  -597,
     334,  -597,  -597,  -597,  1570,   382,  1570,  3541,   364,  3753,
    3753,  3753,  -597,  -597,  -597,  -597,  -597,   136,   145,    31,
     321,  -597,  -597,  -597,  -597,   380,   385,  -597,   345,   348,
     349,   351,   352,  -597,   355,   356,   358,  1977,    33,   360,
    -597,  -597,  -597,   208,  -597,  -597,  1006,   341,  3753,  -597,
    1711,  -597,  -597,   361,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  1570,  -597,  1570,  1570,  -597,  3909,  3983,   367,  -597,
    -597,  -597,  -597,  -597,   399,   445,  -597,   494,  -597,     7,
    -597,  -597,   375,  -597,   152,  -597,  -597,  2094,  3753,  -597,
    -597,  -597,  -597,   383,   386,  -597,    48,  3753,   253,    79,
    -597,   480,   128,  -597,   365,  -597,  -597,  -597,   147,   370,
     346,  -597,  -597,  -597,     7,  -597,   503,    12,  -597,   388,
    -597,  -597,   504,   519,  3753,   394,  3753,   392,   396,  3753,
     521,   397,   -62,  1977,    79,     5,    -3,    64,    75,    79,
      79,    79,    79,   474,   393,   479,  -597,  -597,  4163,  4186,
    4005,  2438,  1006,   398,   536,   422,  -597,   169,   404,    15,
     459,  -597,  -597,  -597,  -597,  3753,   407,  -597,   540,  3753,
      10,  -597,  -597,  -597,  -597,  -597,  -597,  2557,  -597,  -597,
    3541,    36,   -50,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  3647,    36,  -597,  -597,
      33,   410,   412,   413,  -597,  -597,   414,   415,  -597,  -597,
    -597,   434,   435,  -597,     7,  -597,   482,  -597,  -597,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,    20,  3115,   419,  3753,  -597,  -597,   421,  -597,   -11,
     483,  -597,   538,   513,   511,  -597,  -597,     7,  -597,  -597,
    -597,   442,   171,  -597,   561,  -597,  -597,  -597,   179,   431,
      21,   433,  -597,  -597,    14,   199,  -597,   569,   370,  -597,
    -597,  -597,  -597,   555,  -597,  -597,  -597,  -597,  -597,  -597,
      79,    79,   201,  -597,  -597,     7,  -597,  -597,  -597,   436,
    -597,  -597,   439,  1711,  3753,   550,   509,  2669,   507,  2669,
     382,   205,     7,  -597,  3753,   447,  -597,   449,  -597,  -597,
    2669,  -597,    36,   456,   452,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,   233,  3115,  -597,   239,  2669,   582,  -597,  -597,
     453,   571,  -597,   128,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,   590,   569,  -597,  -597,   457,   433,   458,  2669,  2669,
     253,  -597,  -597,   463,   599,  -597,   433,  -597,  1711,  -597,
     247,   465,  -597
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -597,  -597,  -597,    -9,    -4,  -597,   417,  -597,   283,  -597,
    -597,  -135,  -597,  -597,  -597,   117,    45,   374,   317,  -597,
      11,  -597,  -317,  -431,  -597,  -101,  -597,  -584,  -117,  -494,
    -492,  -597,  -597,  -597,  -597,  -597,  -597,   -99,  -597,  -596,
    -111,  -520,  -597,  -517,  -597,  -597,   114,  -597,   -97,   108,
    -597,  -208,   160,  -597,   278,    52,  -597,  -162,  -597,  -597,
    -597,  -597,  -597,   243,  -597,   124,   237,  -597,  -597,  -597,
    -597,  -597,    35,  -597,  -597,   -55,  -597,  -435,  -597,  -597,
    -126,  -597,  -597,  -597,   200,  -597,  -597,  -597,    24,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,   120,  -597,  -597,
    -597,  -597,  -597,  -597,    53,  -597,  -597,  -597,   622,  -597,
    -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,  -597,
    -597,  -597,  -597,  -167,  -597,   406,  -597,   402,  -597,  -597,
     220,  -597,   219,  -597
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -422
static const short yytable[] =
{
     112,   594,   598,   335,   537,   338,   339,   577,   165,   342,
     343,   286,   310,   680,   633,   310,   287,   188,   288,   310,
     423,   310,   592,   191,   434,   596,   552,   336,   687,   704,
     615,   383,   189,   305,   239,   195,   310,   311,   714,   312,
     728,   310,   311,   496,   312,   313,   234,   235,   668,   413,
     313,   284,   295,   310,   311,   196,   197,   198,   113,   729,
     632,   553,   313,   639,   642,   644,   646,   648,   650,   310,
     311,   307,   637,   116,   635,    61,   234,   235,   313,   629,
     310,   311,   316,   640,   310,   311,   323,   -65,   409,   313,
     114,   384,   688,   313,  -407,   323,   115,   296,   678,   308,
     244,   518,   414,   410,   353,   117,   354,   244,   296,   200,
     201,   188,   203,   204,   205,   206,   207,   517,   192,   403,
     301,   337,   630,   120,   153,   302,   202,   240,   420,   669,
     118,   240,   245,   749,   636,   244,   424,   186,   119,   245,
     396,   190,   702,   396,   231,   634,   680,   396,   242,   396,
     766,   677,   269,  -339,  -339,   435,   464,   435,   270,   275,
     705,   398,   680,   282,   314,   187,   686,   245,   194,   314,
     199,   315,   208,   776,    59,   721,   684,   411,   210,   236,
     292,   314,   693,   294,   775,   680,    60,   209,   590,   516,
     304,   211,   167,   258,   259,   260,   261,   314,   256,   257,
     258,   259,   260,   432,   433,   332,   738,   740,   314,   236,
     469,   470,   314,   742,   244,   265,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   554,   212,   770,   172,   188,   213,   363,   490,   579,
     505,   580,   506,   472,   470,   441,   245,   173,   581,   264,
     475,   470,   227,   777,   228,   229,   214,   174,   215,   175,
     176,   216,   275,   275,   266,   555,   217,   442,   476,   470,
     218,   556,   478,   470,   397,   479,   470,   533,   534,   178,
     219,   443,   179,   220,   181,   406,   535,   536,   599,   600,
     464,   417,   421,   221,   182,   242,   222,   760,   223,   297,
     267,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     666,   534,   723,   724,  -390,   278,   329,   279,   333,   458,
     726,   534,   316,   283,   341,   444,   284,   344,   345,   346,
     347,   289,   473,   293,   557,   558,   559,   560,   561,   562,
     731,   732,   741,   534,   357,   563,   754,   534,   362,   231,
     420,   489,   364,   365,   366,   367,   368,   369,   370,   372,
     373,   374,   375,   376,   377,   378,   379,   380,   381,   306,
     604,   605,   309,   385,   763,   534,   321,   621,   322,   623,
     765,   470,   626,   325,   244,   397,   464,   275,   781,   470,
     386,   387,   606,   340,   355,   348,   349,   350,   351,   591,
     382,   388,   595,   389,   390,   391,   401,   607,   402,   362,
     513,   608,   404,   405,   427,   429,   245,   613,   430,   431,
     609,   610,   439,   526,   446,   530,   531,   532,   465,  -421,
     464,   244,   474,   492,   497,   229,   498,   631,   499,   502,
     638,   641,   643,   645,   647,   649,   432,   504,   507,   508,
     611,   510,   509,   458,   490,   511,   512,   329,   486,   487,
     519,   521,   541,   245,   565,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     529,   542,   564,   553,   316,   544,   543,   712,   545,   546,
     493,   547,   548,   571,   572,   549,   550,   574,   551,   576,
     316,   578,   567,   586,   406,   745,   597,   424,   589,   588,
     601,   614,   618,   417,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   619,   622,   627,
     617,   651,   297,   624,   297,   652,   654,   625,   628,   660,
     620,   661,   620,   663,   667,   620,   670,   674,   675,   458,
     464,   695,   464,   696,   697,   698,   699,   700,   701,   703,
     711,   713,   715,   464,   717,   718,   719,   489,   722,   725,
     780,   727,   733,   615,   362,   730,   746,   747,   743,   464,
     744,   671,   761,   737,   739,   513,   751,   767,   757,   568,
     758,   569,   570,   458,   762,   768,   526,   769,   771,   773,
     774,   464,   464,   778,   779,   755,   782,    62,   764,   303,
     467,   573,   691,   268,   361,   316,   422,   659,   752,   759,
      63,   753,   587,   658,    64,    65,    66,   756,   503,    67,
     566,   488,   501,   736,    68,   683,   772,   593,    69,   538,
     185,   694,   676,   358,   520,   524,     0,     0,   708,     0,
     620,    70,   371,     0,    71,    72,    73,    74,    75,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    76,    77,
      78,    79,     0,    80,    81,    82,     0,     0,     0,     0,
     362,     0,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,     0,    91,     0,     0,    92,    93,    94,     0,
      95,     0,     0,    96,     0,     0,    97,    98,    99,   100,
     188,   101,   102,   458,  -389,   458,   104,   105,   106,     0,
     671,   107,   108,   109,     0,   110,   458,   121,   122,   123,
     124,     0,   125,   126,   127,     0,     0,     0,   128,   708,
      63,     0,   458,     0,    64,   129,    66,     0,   130,    67,
       0,     0,     0,     0,    68,     0,     0,     0,   131,     0,
       0,     0,     0,     0,   458,   458,   132,     0,     0,     0,
     133,    70,     0,     0,    71,    72,    73,   134,   135,     0,
     136,   137,     0,     0,     0,   138,     0,     0,    76,    77,
      78,    79,     0,    80,   139,    82,   140,     0,     0,     0,
      83,     0,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,     0,    91,     0,   141,    92,    93,    94,     0,
      95,     0,     0,    96,     0,     0,    97,    98,    99,   142,
       0,   101,   102,     0,   103,     0,   104,   105,   106,     0,
       0,   107,   108,   143,     0,   110,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   144,   145,     0,
       0,   146,     0,     0,   147,     0,     0,   148,   224,   122,
     123,   124,     0,   326,   126,     0,     0,     0,     0,   128,
       0,    63,     0,     0,     0,    64,    65,    66,     0,   130,
      67,     0,     0,     0,     0,    68,     0,     0,     0,    69,
       0,     0,     0,     0,     0,     0,     0,   132,     0,     0,
       0,   133,    70,     0,     0,    71,    72,    73,    74,    75,
       0,   136,   137,     0,     0,     0,   138,     0,     0,    76,
      77,    78,    79,     0,    80,   225,    82,   226,     0,     0,
       0,    83,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,     0,    91,     0,   141,    92,    93,    94,
       0,    95,     0,     0,    96,     0,     0,    97,    98,    99,
     100,     0,   101,   102,     0,   103,     0,   104,   105,   106,
       0,     0,   107,   108,   143,     0,   110,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   144,     0,
       0,     0,   146,     0,     0,   327,   328,     0,   148,   224,
     122,   123,   124,     0,   125,   126,     0,     0,     0,     0,
     128,     0,    63,     0,     0,     0,    64,   129,    66,     0,
     130,    67,     0,     0,     0,     0,    68,     0,     0,     0,
     131,     0,     0,     0,     0,     0,     0,     0,   132,     0,
       0,     0,   133,    70,     0,     0,    71,    72,    73,   134,
     135,     0,   136,   137,     0,     0,     0,   138,     0,     0,
      76,    77,    78,    79,     0,    80,   139,    82,   140,     0,
       0,     0,    83,     0,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,     0,    91,     0,   141,    92,    93,
      94,     0,    95,     0,     0,    96,     0,     0,    97,    98,
      99,   142,     0,   101,   102,     0,   103,     0,   104,   105,
     106,     0,     0,   107,   108,   143,     0,   110,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   144,
     145,     0,     0,   146,     0,     0,   147,     0,     0,   148,
     121,   122,   123,   124,     0,   125,   126,     0,     0,     0,
       0,   128,     0,    63,     0,     0,     0,    64,   129,    66,
       0,   130,    67,     0,     0,     0,     0,    68,     0,     0,
       0,   131,     0,     0,     0,     0,     0,     0,     0,   132,
       0,     0,     0,   133,    70,     0,     0,    71,    72,    73,
     134,   135,     0,   136,   137,     0,     0,     0,   138,     0,
       0,    76,    77,    78,    79,     0,    80,   139,    82,   140,
       0,     0,     0,    83,     0,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,     0,    91,     0,   141,    92,
      93,    94,     0,    95,     0,     0,    96,     0,     0,    97,
      98,    99,   142,     0,   101,   102,     0,   103,     0,   104,
     105,   106,     0,     0,   107,   108,   143,     0,   110,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     144,   145,     0,     0,   146,     0,     0,   147,     0,     0,
     148,   224,   122,   123,   124,     0,   125,   126,     0,     0,
       0,     0,   128,     0,    63,     0,     0,     0,    64,    65,
      66,     0,   130,    67,     0,     0,     0,     0,    68,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
     132,     0,     0,     0,   133,    70,     0,     0,    71,    72,
      73,    74,    75,     0,   136,   137,     0,     0,     0,   138,
       0,     0,    76,    77,    78,    79,     0,    80,   225,    82,
     226,     0,     0,     0,    83,     0,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,     0,    91,     0,   141,
      92,    93,    94,   466,    95,     0,     0,    96,     0,     0,
      97,    98,    99,   100,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,   143,     0,   110,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   144,     0,     0,     0,   146,     0,     0,   147,     0,
       0,   148,   224,   122,   123,   124,     0,   326,   126,     0,
       0,     0,     0,   128,     0,    63,     0,     0,     0,    64,
      65,    66,     0,   130,    67,     0,     0,     0,     0,    68,
       0,     0,     0,    69,     0,     0,     0,     0,     0,     0,
       0,   132,     0,     0,     0,   133,    70,     0,     0,    71,
      72,    73,    74,    75,     0,   136,   137,     0,     0,     0,
     138,     0,     0,    76,    77,    78,    79,     0,    80,   225,
      82,   226,     0,     0,     0,    83,     0,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,     0,    91,     0,
     141,    92,    93,    94,     0,    95,     0,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,     0,   103,
       0,   104,   105,   106,     0,     0,   107,   108,   143,     0,
     110,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   144,     0,     0,     0,   146,     0,     0,   147,
     328,     0,   148,   224,   122,   123,   124,     0,   125,   126,
       0,     0,     0,     0,   128,     0,    63,     0,     0,     0,
      64,    65,    66,     0,   130,    67,     0,     0,     0,     0,
      68,     0,     0,     0,    69,     0,     0,     0,     0,     0,
       0,     0,   132,     0,     0,     0,   133,    70,     0,     0,
      71,    72,    73,    74,    75,     0,   136,   137,     0,     0,
       0,   138,     0,     0,    76,    77,    78,    79,     0,    80,
     225,    82,   226,     0,     0,     0,    83,     0,     0,     0,
       0,    84,    85,    86,    87,    88,    89,    90,     0,    91,
       0,   141,    92,    93,    94,     0,    95,     0,     0,    96,
       0,     0,    97,    98,    99,   100,     0,   101,   102,     0,
     103,     0,   104,   105,   106,     0,     0,   107,   108,   143,
       0,   110,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   144,     0,     0,     0,   146,     0,     0,
     147,     0,     0,   148,   224,   122,   123,   124,     0,   326,
     126,     0,     0,     0,     0,   128,     0,    63,     0,     0,
       0,    64,    65,    66,     0,   130,    67,     0,     0,     0,
       0,    68,     0,     0,     0,    69,     0,     0,     0,     0,
       0,     0,     0,   132,     0,     0,     0,   133,    70,     0,
       0,    71,    72,    73,    74,    75,     0,   136,   137,     0,
       0,     0,   138,     0,     0,    76,    77,    78,    79,     0,
      80,   225,    82,   226,     0,     0,     0,    83,     0,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,     0,
      91,     0,   141,    92,    93,    94,     0,    95,     0,     0,
      96,     0,     0,    97,    98,    99,   100,     0,   101,   102,
       0,   103,     0,   104,   105,   106,     0,     0,   107,   108,
     143,     0,   110,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   144,     0,     0,     0,   146,     0,
       0,   147,     0,     0,   148,   224,   122,   123,   124,     0,
     356,   126,     0,     0,     0,     0,   128,     0,    63,     0,
       0,     0,    64,    65,    66,     0,   130,    67,     0,     0,
       0,     0,    68,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,   132,     0,     0,     0,   133,    70,
       0,     0,    71,    72,    73,    74,    75,     0,   136,   137,
       0,     0,     0,   138,     0,     0,    76,    77,    78,    79,
       0,    80,   225,    82,   226,     0,     0,     0,    83,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
       0,    91,     0,   141,    92,    93,    94,     0,    95,     0,
       0,    96,     0,     0,    97,    98,    99,   100,     0,   101,
     102,     0,   103,     0,   104,   105,   106,     0,     0,   107,
     108,   143,     0,   110,     0,     0,     0,     0,     0,     0,
      62,   447,     0,     0,     0,   144,     0,     0,     0,   146,
       0,     0,   147,    63,     0,   148,     0,    64,    65,    66,
       0,   448,    67,     0,     0,     0,     0,    68,     0,     0,
       0,   449,     0,     0,     0,     0,     0,     0,     0,   450,
       0,   451,     0,     0,    70,     0,     0,    71,    72,    73,
     452,    75,     0,   453,     0,     0,     0,     0,   454,     0,
       0,    76,    77,    78,    79,   244,   455,    81,    82,     0,
       0,     0,     0,    83,     0,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,     0,    91,     0,     0,    92,
      93,    94,     0,    95,     0,     0,    96,   245,     0,    97,
      98,    99,   100,     0,   101,   102,     0,   103,     0,   104,
     105,   106,     0,     0,   107,   108,   456,    62,   110,   583,
       0,     0,   584,     0,     0,     0,     0,     0,     0,     0,
      63,     0,     0,     0,    64,    65,    66,   457,     0,    67,
       0,     0,     0,     0,    68,     0,     0,     0,    69,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,    70,     0,     0,    71,    72,    73,    74,    75,     0,
       0,     0,   244,     0,     0,     0,     0,     0,    76,    77,
      78,    79,     0,    80,    81,    82,     0,     0,     0,     0,
      83,     0,     0,     0,     0,    84,    85,    86,    87,    88,
      89,    90,     0,    91,   245,     0,    92,    93,    94,     0,
      95,     0,     0,    96,     0,     0,    97,    98,    99,   100,
       0,   101,   102,     0,   103,    62,   104,   105,   106,     0,
       0,   107,   108,   109,     0,   110,     0,     0,    63,     0,
       0,     0,    64,    65,    66,     0,     0,    67,     0,     0,
       0,     0,    68,     0,   585,     0,    69,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,     0,    70,
       0,     0,    71,    72,    73,    74,    75,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    76,    77,    78,    79,
       0,    80,    81,    82,     0,     0,     0,     0,    83,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
       0,    91,     0,     0,    92,    93,    94,     0,    95,     0,
       0,    96,     0,     0,    97,    98,    99,   100,     0,   101,
     102,     0,   103,     0,   104,   105,   106,     0,     0,   107,
     108,   109,    62,   110,   310,     0,     0,   392,     0,     0,
       0,     0,     0,     0,     0,    63,     0,     0,     0,    64,
      65,    66,   359,     0,    67,     0,     0,     0,     0,    68,
       0,     0,     0,    69,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   393,     0,    70,     0,     0,    71,
      72,    73,    74,    75,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    76,    77,    78,    79,     0,    80,    81,
      82,     0,     0,     0,     0,    83,   394,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,     0,    91,     0,
       0,    92,    93,    94,     0,    95,     0,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,     0,   103,
     395,   104,   105,   106,     0,     0,   107,   108,   109,     0,
     110,    62,     0,   310,     0,     0,     0,     0,     0,     0,
       0,     0,   396,     0,    63,     0,     0,     0,    64,    65,
      66,     0,     0,    67,     0,     0,     0,     0,    68,     0,
       0,     0,    69,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,     0,     0,    71,    72,
      73,    74,    75,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    76,    77,    78,    79,     0,    80,    81,    82,
       0,     0,     0,     0,    83,     0,     0,     0,     0,    84,
      85,    86,    87,    88,    89,    90,     0,    91,     0,     0,
      92,    93,    94,     0,    95,     0,     0,    96,     0,     0,
      97,    98,    99,   100,     0,   101,   102,     0,   103,     0,
     104,   105,   106,     0,     0,   107,   108,   109,     0,   110,
      62,   447,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   396,     0,    63,     0,     0,     0,    64,    65,    66,
       0,   448,    67,     0,     0,     0,     0,    68,     0,     0,
       0,   449,     0,     0,     0,     0,     0,     0,     0,   450,
       0,   451,     0,     0,    70,     0,     0,    71,    72,    73,
     452,    75,     0,   453,     0,     0,     0,     0,   454,     0,
       0,    76,    77,    78,    79,     0,    80,    81,    82,     0,
       0,     0,     0,    83,     0,     0,     0,     0,    84,    85,
      86,    87,   679,    89,    90,     0,    91,     0,     0,    92,
      93,    94,     0,    95,     0,     0,    96,     0,     0,    97,
      98,    99,   100,     0,   101,   102,     0,   103,     0,   104,
     105,   106,    62,   447,   107,   108,   456,     0,   110,     0,
       0,     0,     0,     0,     0,    63,     0,     0,     0,    64,
      65,    66,     0,   448,    67,     0,     0,     0,     0,    68,
       0,     0,     0,   449,     0,     0,     0,     0,     0,     0,
       0,   450,     0,   451,     0,     0,    70,     0,     0,    71,
      72,    73,   452,    75,     0,   453,     0,     0,     0,     0,
     454,     0,     0,    76,    77,    78,    79,     0,    80,    81,
      82,     0,     0,     0,     0,    83,     0,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,     0,    91,     0,
       0,    92,    93,    94,     0,    95,     0,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,     0,   103,
       0,   104,   105,   106,     0,     0,   107,   108,   456,    62,
     110,   271,   272,     0,   273,     0,     0,     0,     0,     0,
       0,     0,    63,     0,     0,     0,    64,    65,    66,     0,
       0,    67,     0,     0,     0,     0,    68,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    70,     0,     0,    71,    72,    73,    74,
      75,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      76,    77,    78,    79,     0,    80,    81,    82,     0,     0,
       0,     0,    83,   274,     0,     0,     0,    84,    85,    86,
      87,    88,    89,    90,     0,    91,     0,     0,    92,    93,
      94,     0,    95,     0,     0,    96,     0,     0,    97,    98,
      99,   100,     0,   101,   102,    62,   103,     0,   104,   105,
     106,     0,   159,   107,   108,   109,     0,   110,    63,     0,
       0,     0,    64,    65,    66,     0,     0,    67,     0,     0,
     160,     0,    68,     0,     0,     0,    69,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
       0,     0,    71,   161,    73,    74,    75,     0,     0,     0,
     162,     0,     0,     0,     0,     0,    76,    77,    78,    79,
       0,    80,    81,    82,     0,     0,     0,     0,   163,     0,
       0,     0,     0,    84,    85,    86,    87,    88,    89,    90,
       0,    91,     0,     0,    92,    93,    94,     0,    95,     0,
       0,   164,     0,     0,    97,    98,    99,   100,     0,   101,
     102,     0,     0,     0,   104,   105,   106,     0,     0,   107,
     108,   109,    62,   110,   500,     0,     0,   392,     0,     0,
       0,     0,     0,     0,     0,    63,     0,     0,     0,    64,
      65,    66,     0,     0,    67,     0,     0,     0,     0,    68,
       0,     0,     0,    69,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    70,     0,     0,    71,
      72,    73,    74,    75,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    76,    77,    78,    79,     0,    80,    81,
      82,     0,     0,     0,     0,    83,     0,     0,     0,     0,
      84,    85,    86,    87,    88,    89,    90,     0,    91,     0,
       0,    92,    93,    94,     0,    95,     0,     0,    96,     0,
       0,    97,    98,    99,   100,     0,   101,   102,    62,   103,
     706,   104,   105,   106,     0,     0,   107,   108,   109,     0,
     110,    63,     0,     0,     0,    64,    65,    66,     0,     0,
      67,     0,     0,     0,     0,    68,     0,     0,     0,    69,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    70,     0,     0,    71,    72,    73,    74,    75,
       0,   707,     0,     0,     0,     0,     0,     0,     0,    76,
      77,    78,    79,     0,    80,    81,    82,     0,     0,     0,
       0,    83,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,     0,    91,     0,     0,    92,    93,    94,
       0,    95,     0,     0,    96,     0,     0,    97,    98,    99,
     100,     0,   101,   102,    62,   103,     0,   104,   105,   106,
       0,     0,   107,   108,   109,     0,   110,    63,     0,     0,
       0,    64,    65,    66,     0,     0,    67,     0,     0,     0,
       0,    68,     0,     0,     0,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    70,     0,
       0,    71,    72,    73,    74,    75,     0,     0,   230,     0,
       0,     0,     0,     0,     0,    76,    77,    78,    79,     0,
      80,    81,    82,     0,     0,     0,     0,    83,     0,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,     0,
      91,     0,     0,    92,    93,    94,     0,    95,     0,     0,
      96,     0,     0,    97,    98,    99,   100,     0,   101,   102,
      62,   103,     0,   104,   105,   106,     0,     0,   107,   108,
     109,     0,   110,    63,     0,   241,     0,    64,    65,    66,
       0,     0,    67,     0,     0,     0,     0,    68,     0,     0,
       0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    70,     0,     0,    71,    72,    73,
      74,    75,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    76,    77,    78,    79,     0,    80,    81,    82,     0,
       0,     0,     0,    83,     0,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,     0,    91,     0,     0,    92,
      93,    94,     0,    95,     0,     0,    96,     0,     0,    97,
      98,    99,   100,     0,   101,   102,     0,   103,    62,   104,
     105,   106,     0,   280,   107,   108,   109,     0,   110,     0,
       0,    63,     0,     0,     0,    64,    65,    66,     0,     0,
      67,     0,     0,     0,     0,    68,     0,     0,     0,    69,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    70,     0,     0,    71,    72,    73,    74,    75,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    76,
      77,    78,    79,     0,    80,    81,    82,     0,     0,     0,
       0,    83,     0,     0,     0,     0,    84,    85,    86,    87,
      88,    89,    90,     0,    91,     0,     0,    92,    93,    94,
       0,    95,     0,     0,    96,     0,     0,    97,   281,    99,
     100,     0,   101,   102,    62,   103,     0,   104,   105,   106,
       0,     0,   107,   108,   109,     0,   110,    63,     0,     0,
       0,    64,    65,    66,     0,     0,    67,     0,     0,     0,
       0,    68,     0,     0,     0,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    70,     0,
       0,    71,    72,    73,    74,    75,     0,   525,     0,     0,
       0,     0,     0,     0,     0,    76,    77,    78,    79,     0,
      80,    81,    82,     0,     0,     0,     0,    83,     0,     0,
       0,     0,    84,    85,    86,    87,    88,    89,    90,     0,
      91,     0,     0,    92,    93,    94,     0,    95,     0,     0,
      96,     0,     0,    97,    98,    99,   100,     0,   101,   102,
      62,   103,     0,   104,   105,   106,     0,     0,   107,   108,
     109,     0,   110,    63,     0,     0,     0,    64,    65,    66,
       0,     0,    67,     0,     0,     0,     0,    68,     0,     0,
       0,    69,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    70,     0,     0,    71,    72,    73,
      74,    75,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    76,    77,    78,    79,   690,    80,    81,    82,     0,
       0,     0,     0,    83,     0,     0,     0,     0,    84,    85,
      86,    87,    88,    89,    90,     0,    91,     0,     0,    92,
      93,    94,     0,    95,     0,     0,    96,     0,     0,    97,
      98,    99,   100,     0,   101,   102,    62,   103,     0,   104,
     105,   106,     0,     0,   107,   108,   109,     0,   110,    63,
       0,     0,     0,    64,    65,    66,     0,     0,    67,     0,
       0,     0,     0,    68,     0,     0,     0,    69,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      70,     0,     0,    71,    72,    73,    74,    75,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    76,    77,    78,
      79,     0,    80,    81,    82,     0,     0,     0,     0,    83,
       0,     0,     0,     0,    84,    85,    86,    87,    88,    89,
      90,     0,    91,     0,     0,    92,    93,    94,     0,    95,
       0,     0,    96,     0,     0,    97,    98,    99,   100,     0,
     101,   102,     0,   103,   167,   104,   105,   106,     0,     0,
     107,   108,   109,     0,   110,     0,   168,     0,   169,     0,
       0,     0,     0,     0,   170,     0,   244,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,     0,     0,   172,   244,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   245,   173,
       0,     0,     0,     0,     0,     0,     0,     0,   244,   174,
       0,   175,   176,     0,     0,     0,     0,     0,     0,   245,
       0,     0,     0,     0,     0,     0,     0,   177,     0,     0,
     244,   178,     0,     0,   179,   180,   181,     0,     0,     0,
     245,     0,     0,     0,     0,     0,   182,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   245,     0,     0,     0,   480,   481,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,     0,     0,     0,     0,   482,   483,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   244,     0,     0,     0,     0,   484,
       0,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   244,     0,     0,     0,
       0,   481,     0,     0,     0,     0,   245,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   245,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   244,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   245,
       0,     0,     0,     0,     0,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     245,     0,     0,     0,     0,   483,     0,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,     0,     0,     0,     0,     0,   657,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   244,     0,     0,     0,   352,     0,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   244,     0,     0,     0,   471,     0,
       0,     0,     0,     0,     0,   245,     0,     0,     0,     0,
       0,     0,     0,     0,   244,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   245,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,     0,
       0,     0,     0,     0,     0,     0,   245,     0,     0,     0,
       0,     0,     0,     0,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   245,
       0,     0,     0,   477,     0,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
       0,     0,     0,     0,   485,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
       0,     0,     0,     0,   655,     0,     0,     0,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,     1,     0,     0,     2,   656,     3,     0,
       0,     0,     0,     4,     0,     0,     0,     0,     5,     0,
       0,     6,     0,     7,     8,     9,     0,     0,   244,    10,
       0,     0,     0,    11,     0,   468,     0,     0,     0,     0,
       0,     0,     0,   244,     0,    12,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   244,     0,     0,
     245,     0,     0,     0,     0,     0,    13,     0,     0,     0,
       0,     0,     0,     0,     0,   245,    14,     0,     0,    15,
       0,    16,     0,    17,     0,     0,    18,     0,    19,   245,
       0,     0,     0,     0,     0,     0,     0,     0,    20,     0,
       0,    21,     0,     0,     0,     0,     0,     0,     0,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260
};

static const short yycheck[] =
{
       4,   518,   522,   211,   439,   213,   214,   499,    17,   217,
     218,   173,     5,   597,     9,     5,   178,    21,   180,     5,
      26,     5,   516,    44,     8,   519,   457,    40,    78,     9,
      18,    77,    21,   195,    49,    39,     5,     6,    49,     8,
      19,     5,     6,   360,     8,    14,    13,    14,    33,    67,
      14,    67,    97,     5,     6,    59,    60,    61,    50,    38,
     554,   123,    14,   557,   558,   559,   560,   561,   562,     5,
       6,    15,     8,    50,    77,    58,    13,    14,    14,   141,
       5,     6,   199,     8,     5,     6,   120,   103,    14,    14,
      82,   137,   142,    14,     0,   120,    49,   142,   590,    43,
      41,   418,   120,    29,   142,    82,   144,    41,   142,   113,
     114,   115,   116,   117,   118,   119,   120,   142,   139,   281,
      95,   134,   553,    58,    16,   100,   115,   142,   295,   114,
      86,   142,    73,   717,   137,    41,   142,   110,    94,    73,
     133,     0,   634,   133,   148,   140,   730,   133,   152,   133,
     746,   141,   161,   141,   142,   141,   323,   141,   162,   163,
     140,   278,   746,   167,   133,    94,   601,    73,    62,   133,
     140,   140,   140,   769,    94,   667,   140,   103,   140,   146,
     184,   133,   617,   187,   768,   769,   106,   145,   140,   126,
     194,   140,    16,   134,   135,   136,    68,   133,   132,   133,
     134,   135,   136,     5,     6,   209,   700,   701,   133,   146,
     141,   142,   133,   705,    41,   126,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,    23,   140,   753,    58,   239,   140,   241,   355,    87,
     402,    89,   404,   141,   142,    24,    73,    71,    96,    63,
     141,   142,   144,   770,   146,   147,   140,    81,   140,    83,
      84,   140,   266,   267,   126,    57,   140,    46,   141,   142,
     140,    63,   141,   142,   278,   141,   142,   141,   142,   103,
     140,    60,   106,   140,   108,   289,   141,   142,   141,   142,
     457,   295,   296,   140,   118,   299,   140,   732,   140,   191,
      97,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     141,   142,   141,   142,   110,   126,   208,   110,   210,   323,
     141,   142,   439,    97,   216,   104,    67,   219,   220,   221,
     222,    78,   336,   117,   126,   127,   128,   129,   130,   131,
     141,   142,   141,   142,   236,   137,   141,   142,   240,   353,
     517,   355,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,    88,
      24,    25,   109,   265,   141,   142,    92,   544,   113,   546,
     141,   142,   549,   113,    41,   389,   553,   391,   141,   142,
     266,   267,    46,   141,   126,   141,   140,   140,   140,   516,
       5,   126,   519,   126,    51,   133,    64,    61,   103,   301,
     414,    65,   103,     8,   140,    31,    73,   534,    31,    94,
      74,    75,   142,   427,     8,   429,   430,   431,     8,   126,
     597,    41,   141,   147,   142,   327,    77,   554,   140,     9,
     557,   558,   559,   560,   561,   562,     5,    66,   126,   142,
     104,     5,    98,   457,   571,    98,     8,   349,   350,   351,
     126,    79,   141,    73,   468,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     116,   101,   141,   123,   601,   140,   101,   654,   140,   140,
     147,   140,   140,   126,    95,   140,   140,    52,   140,     5,
     617,   126,   141,   507,   508,   713,    26,   142,   122,   126,
     140,     8,     8,   517,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,     8,   134,     8,
     142,    57,   424,   141,   426,   142,    57,   141,   141,   141,
     544,     5,   546,   121,   140,   549,    87,   140,     8,   553,
     717,   141,   719,   141,   141,   141,   141,   123,   123,    77,
     141,   140,    79,   730,    26,    52,    55,   571,   126,     8,
     778,   140,     3,    18,   466,   142,    26,    68,   142,   746,
     141,   585,   126,   700,   701,   589,    79,     5,   141,   481,
     141,   483,   484,   597,   142,   142,   600,    26,     8,   142,
     142,   768,   769,   140,     5,   722,   141,     3,   743,   192,
     327,   494,   616,     9,   240,   732,   299,   572,   719,   730,
      16,   720,   508,   571,    20,    21,    22,   724,   391,    25,
     470,   353,   389,   688,    30,   600,   762,   517,    34,   439,
      18,   617,   589,   237,   424,   426,    -1,    -1,   652,    -1,
     654,    47,   250,    -1,    50,    51,    52,    53,    54,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    65,
      66,    67,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,
     572,    -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      86,    87,    -1,    89,    -1,    -1,    92,    93,    94,    -1,
      96,    -1,    -1,    99,    -1,    -1,   102,   103,   104,   105,
     714,   107,   108,   717,   110,   719,   112,   113,   114,    -1,
     724,   117,   118,   119,    -1,   121,   730,     3,     4,     5,
       6,    -1,     8,     9,    10,    -1,    -1,    -1,    14,   743,
      16,    -1,   746,    -1,    20,    21,    22,    -1,    24,    25,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    34,    -1,
      -1,    -1,    -1,    -1,   768,   769,    42,    -1,    -1,    -1,
      46,    47,    -1,    -1,    50,    51,    52,    53,    54,    -1,
      56,    57,    -1,    -1,    -1,    61,    -1,    -1,    64,    65,
      66,    67,    -1,    69,    70,    71,    72,    -1,    -1,    -1,
      76,    -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      86,    87,    -1,    89,    -1,    91,    92,    93,    94,    -1,
      96,    -1,    -1,    99,    -1,    -1,   102,   103,   104,   105,
      -1,   107,   108,    -1,   110,    -1,   112,   113,   114,    -1,
      -1,   117,   118,   119,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,    -1,
      -1,   137,    -1,    -1,   140,    -1,    -1,   143,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    14,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    24,
      25,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,    -1,
      -1,    46,    47,    -1,    -1,    50,    51,    52,    53,    54,
      -1,    56,    57,    -1,    -1,    -1,    61,    -1,    -1,    64,
      65,    66,    67,    -1,    69,    70,    71,    72,    -1,    -1,
      -1,    76,    -1,    -1,    -1,    -1,    81,    82,    83,    84,
      85,    86,    87,    -1,    89,    -1,    91,    92,    93,    94,
      -1,    96,    -1,    -1,    99,    -1,    -1,   102,   103,   104,
     105,    -1,   107,   108,    -1,   110,    -1,   112,   113,   114,
      -1,    -1,   117,   118,   119,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,
      -1,    -1,   137,    -1,    -1,   140,   141,    -1,   143,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      24,    25,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    -1,
      -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,    53,
      54,    -1,    56,    57,    -1,    -1,    -1,    61,    -1,    -1,
      64,    65,    66,    67,    -1,    69,    70,    71,    72,    -1,
      -1,    -1,    76,    -1,    -1,    -1,    -1,    81,    82,    83,
      84,    85,    86,    87,    -1,    89,    -1,    91,    92,    93,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,    -1,   110,    -1,   112,   113,
     114,    -1,    -1,   117,   118,   119,    -1,   121,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,
     134,    -1,    -1,   137,    -1,    -1,   140,    -1,    -1,   143,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,
      53,    54,    -1,    56,    57,    -1,    -1,    -1,    61,    -1,
      -1,    64,    65,    66,    67,    -1,    69,    70,    71,    72,
      -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    86,    87,    -1,    89,    -1,    91,    92,
      93,    94,    -1,    96,    -1,    -1,    99,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,    -1,    -1,   117,   118,   119,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,    -1,    -1,   137,    -1,    -1,   140,    -1,    -1,
     143,     3,     4,     5,     6,    -1,     8,     9,    -1,    -1,
      -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    24,    25,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,
      52,    53,    54,    -1,    56,    57,    -1,    -1,    -1,    61,
      -1,    -1,    64,    65,    66,    67,    -1,    69,    70,    71,
      72,    -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,    81,
      82,    83,    84,    85,    86,    87,    -1,    89,    -1,    91,
      92,    93,    94,    95,    96,    -1,    -1,    99,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,    -1,    -1,   117,   118,   119,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,    -1,    -1,   137,    -1,    -1,   140,    -1,
      -1,   143,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,
      51,    52,    53,    54,    -1,    56,    57,    -1,    -1,    -1,
      61,    -1,    -1,    64,    65,    66,    67,    -1,    69,    70,
      71,    72,    -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    86,    87,    -1,    89,    -1,
      91,    92,    93,    94,    -1,    96,    -1,    -1,    99,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,    -1,   110,
      -1,   112,   113,   114,    -1,    -1,   117,   118,   119,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,    -1,    -1,    -1,   137,    -1,    -1,   140,
     141,    -1,   143,     3,     4,     5,     6,    -1,     8,     9,
      -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,    -1,
      20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    -1,    46,    47,    -1,    -1,
      50,    51,    52,    53,    54,    -1,    56,    57,    -1,    -1,
      -1,    61,    -1,    -1,    64,    65,    66,    67,    -1,    69,
      70,    71,    72,    -1,    -1,    -1,    76,    -1,    -1,    -1,
      -1,    81,    82,    83,    84,    85,    86,    87,    -1,    89,
      -1,    91,    92,    93,    94,    -1,    96,    -1,    -1,    99,
      -1,    -1,   102,   103,   104,   105,    -1,   107,   108,    -1,
     110,    -1,   112,   113,   114,    -1,    -1,   117,   118,   119,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   133,    -1,    -1,    -1,   137,    -1,    -1,
     140,    -1,    -1,   143,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    -1,    -1,    -1,    46,    47,    -1,
      -1,    50,    51,    52,    53,    54,    -1,    56,    57,    -1,
      -1,    -1,    61,    -1,    -1,    64,    65,    66,    67,    -1,
      69,    70,    71,    72,    -1,    -1,    -1,    76,    -1,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    86,    87,    -1,
      89,    -1,    91,    92,    93,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,   102,   103,   104,   105,    -1,   107,   108,
      -1,   110,    -1,   112,   113,   114,    -1,    -1,   117,   118,
     119,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   133,    -1,    -1,    -1,   137,    -1,
      -1,   140,    -1,    -1,   143,     3,     4,     5,     6,    -1,
       8,     9,    -1,    -1,    -1,    -1,    14,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    24,    25,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    -1,    -1,    46,    47,
      -1,    -1,    50,    51,    52,    53,    54,    -1,    56,    57,
      -1,    -1,    -1,    61,    -1,    -1,    64,    65,    66,    67,
      -1,    69,    70,    71,    72,    -1,    -1,    -1,    76,    -1,
      -1,    -1,    -1,    81,    82,    83,    84,    85,    86,    87,
      -1,    89,    -1,    91,    92,    93,    94,    -1,    96,    -1,
      -1,    99,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,    -1,    -1,   117,
     118,   119,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,    -1,    -1,    -1,   133,    -1,    -1,    -1,   137,
      -1,    -1,   140,    16,    -1,   143,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      53,    54,    -1,    56,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    64,    65,    66,    67,    41,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    86,    87,    -1,    89,    -1,    -1,    92,
      93,    94,    -1,    96,    -1,    -1,    99,    73,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,    -1,    -1,   117,   118,   119,     3,   121,     5,
      -1,    -1,     8,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,    -1,    20,    21,    22,   140,    -1,    25,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    34,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,    47,    -1,    -1,    50,    51,    52,    53,    54,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,    64,    65,
      66,    67,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,
      76,    -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,
      86,    87,    -1,    89,    73,    -1,    92,    93,    94,    -1,
      96,    -1,    -1,    99,    -1,    -1,   102,   103,   104,   105,
      -1,   107,   108,    -1,   110,     3,   112,   113,   114,    -1,
      -1,   117,   118,   119,    -1,   121,    -1,    -1,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      -1,    -1,    30,    -1,   140,    -1,    34,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,    -1,    47,
      -1,    -1,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    64,    65,    66,    67,
      -1,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    -1,
      -1,    -1,    -1,    81,    82,    83,    84,    85,    86,    87,
      -1,    89,    -1,    -1,    92,    93,    94,    -1,    96,    -1,
      -1,    99,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,   110,    -1,   112,   113,   114,    -1,    -1,   117,
     118,   119,     3,   121,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,   140,    -1,    25,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    47,    -1,    -1,    50,
      51,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    65,    66,    67,    -1,    69,    70,
      71,    -1,    -1,    -1,    -1,    76,    77,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    86,    87,    -1,    89,    -1,
      -1,    92,    93,    94,    -1,    96,    -1,    -1,    99,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,    -1,   110,
     111,   112,   113,   114,    -1,    -1,   117,   118,   119,    -1,
     121,     3,    -1,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   133,    -1,    16,    -1,    -1,    -1,    20,    21,
      22,    -1,    -1,    25,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    65,    66,    67,    -1,    69,    70,    71,
      -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,    81,
      82,    83,    84,    85,    86,    87,    -1,    89,    -1,    -1,
      92,    93,    94,    -1,    96,    -1,    -1,    99,    -1,    -1,
     102,   103,   104,   105,    -1,   107,   108,    -1,   110,    -1,
     112,   113,   114,    -1,    -1,   117,   118,   119,    -1,   121,
       3,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   133,    -1,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    24,    25,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      -1,    44,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      53,    54,    -1,    56,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    64,    65,    66,    67,    -1,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    86,    87,    -1,    89,    -1,    -1,    92,
      93,    94,    -1,    96,    -1,    -1,    99,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,    -1,   112,
     113,   114,     3,     4,   117,   118,   119,    -1,   121,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    24,    25,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    -1,    44,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    53,    54,    -1,    56,    -1,    -1,    -1,    -1,
      61,    -1,    -1,    64,    65,    66,    67,    -1,    69,    70,
      71,    -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    86,    87,    -1,    89,    -1,
      -1,    92,    93,    94,    -1,    96,    -1,    -1,    99,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,    -1,   110,
      -1,   112,   113,   114,    -1,    -1,   117,   118,   119,     3,
     121,     5,     6,    -1,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,
      -1,    25,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,
      34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,    53,
      54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      64,    65,    66,    67,    -1,    69,    70,    71,    -1,    -1,
      -1,    -1,    76,    77,    -1,    -1,    -1,    81,    82,    83,
      84,    85,    86,    87,    -1,    89,    -1,    -1,    92,    93,
      94,    -1,    96,    -1,    -1,    99,    -1,    -1,   102,   103,
     104,   105,    -1,   107,   108,     3,   110,    -1,   112,   113,
     114,    -1,    10,   117,   118,   119,    -1,   121,    16,    -1,
      -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,
      28,    -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    -1,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    64,    65,    66,    67,
      -1,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    -1,
      -1,    -1,    -1,    81,    82,    83,    84,    85,    86,    87,
      -1,    89,    -1,    -1,    92,    93,    94,    -1,    96,    -1,
      -1,    99,    -1,    -1,   102,   103,   104,   105,    -1,   107,
     108,    -1,    -1,    -1,   112,   113,   114,    -1,    -1,   117,
     118,   119,     3,   121,     5,    -1,    -1,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,    20,
      21,    22,    -1,    -1,    25,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    -1,    50,
      51,    52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    65,    66,    67,    -1,    69,    70,
      71,    -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,
      81,    82,    83,    84,    85,    86,    87,    -1,    89,    -1,
      -1,    92,    93,    94,    -1,    96,    -1,    -1,    99,    -1,
      -1,   102,   103,   104,   105,    -1,   107,   108,     3,   110,
       5,   112,   113,   114,    -1,    -1,   117,   118,   119,    -1,
     121,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    53,    54,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,
      65,    66,    67,    -1,    69,    70,    71,    -1,    -1,    -1,
      -1,    76,    -1,    -1,    -1,    -1,    81,    82,    83,    84,
      85,    86,    87,    -1,    89,    -1,    -1,    92,    93,    94,
      -1,    96,    -1,    -1,    99,    -1,    -1,   102,   103,   104,
     105,    -1,   107,   108,     3,   110,    -1,   112,   113,   114,
      -1,    -1,   117,   118,   119,    -1,   121,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    65,    66,    67,    -1,
      69,    70,    71,    -1,    -1,    -1,    -1,    76,    -1,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    86,    87,    -1,
      89,    -1,    -1,    92,    93,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,   102,   103,   104,   105,    -1,   107,   108,
       3,   110,    -1,   112,   113,   114,    -1,    -1,   117,   118,
     119,    -1,   121,    16,    -1,    18,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    65,    66,    67,    -1,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    86,    87,    -1,    89,    -1,    -1,    92,
      93,    94,    -1,    96,    -1,    -1,    99,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,    -1,   110,     3,   112,
     113,   114,    -1,     8,   117,   118,   119,    -1,   121,    -1,
      -1,    16,    -1,    -1,    -1,    20,    21,    22,    -1,    -1,
      25,    -1,    -1,    -1,    -1,    30,    -1,    -1,    -1,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    47,    -1,    -1,    50,    51,    52,    53,    54,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,
      65,    66,    67,    -1,    69,    70,    71,    -1,    -1,    -1,
      -1,    76,    -1,    -1,    -1,    -1,    81,    82,    83,    84,
      85,    86,    87,    -1,    89,    -1,    -1,    92,    93,    94,
      -1,    96,    -1,    -1,    99,    -1,    -1,   102,   103,   104,
     105,    -1,   107,   108,     3,   110,    -1,   112,   113,   114,
      -1,    -1,   117,   118,   119,    -1,   121,    16,    -1,    -1,
      -1,    20,    21,    22,    -1,    -1,    25,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      -1,    50,    51,    52,    53,    54,    -1,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    64,    65,    66,    67,    -1,
      69,    70,    71,    -1,    -1,    -1,    -1,    76,    -1,    -1,
      -1,    -1,    81,    82,    83,    84,    85,    86,    87,    -1,
      89,    -1,    -1,    92,    93,    94,    -1,    96,    -1,    -1,
      99,    -1,    -1,   102,   103,   104,   105,    -1,   107,   108,
       3,   110,    -1,   112,   113,   114,    -1,    -1,   117,   118,
     119,    -1,   121,    16,    -1,    -1,    -1,    20,    21,    22,
      -1,    -1,    25,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    -1,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    64,    65,    66,    67,    68,    69,    70,    71,    -1,
      -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,    81,    82,
      83,    84,    85,    86,    87,    -1,    89,    -1,    -1,    92,
      93,    94,    -1,    96,    -1,    -1,    99,    -1,    -1,   102,
     103,   104,   105,    -1,   107,   108,     3,   110,    -1,   112,
     113,   114,    -1,    -1,   117,   118,   119,    -1,   121,    16,
      -1,    -1,    -1,    20,    21,    22,    -1,    -1,    25,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    -1,    34,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    -1,    50,    51,    52,    53,    54,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    64,    65,    66,
      67,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,    76,
      -1,    -1,    -1,    -1,    81,    82,    83,    84,    85,    86,
      87,    -1,    89,    -1,    -1,    92,    93,    94,    -1,    96,
      -1,    -1,    99,    -1,    -1,   102,   103,   104,   105,    -1,
     107,   108,    -1,   110,    16,   112,   113,   114,    -1,    -1,
     117,   118,   119,    -1,   121,    -1,    28,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    41,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    81,
      -1,    83,    84,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
      41,   103,    -1,    -1,   106,   107,   108,    -1,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,   118,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,    73,    -1,    -1,    -1,   141,   142,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,   141,   142,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,    41,    -1,    -1,    -1,    -1,   142,
      -1,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,    41,    -1,    -1,    -1,
      -1,   142,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      -1,    -1,    -1,    -1,    -1,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
      73,    -1,    -1,    -1,    -1,   142,    -1,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,    -1,    -1,    -1,    -1,    -1,   142,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,    41,    -1,    -1,    -1,   141,    -1,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,    41,    -1,    -1,    -1,   141,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,    73,
      -1,    -1,    -1,   141,    -1,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,   141,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    -1,   141,    -1,    -1,    -1,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,    17,    -1,    -1,    20,   141,    22,    -1,
      -1,    -1,    -1,    27,    -1,    -1,    -1,    -1,    32,    -1,
      -1,    35,    -1,    37,    38,    39,    -1,    -1,    41,    43,
      -1,    -1,    -1,    47,    -1,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    59,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      73,    -1,    -1,    -1,    -1,    -1,    80,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    90,    -1,    -1,    93,
      -1,    95,    -1,    97,    -1,    -1,   100,    -1,   102,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   112,    -1,
      -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,    17,    20,    22,    27,    32,    35,    37,    38,    39,
      43,    47,    59,    80,    90,    93,    95,    97,   100,   102,
     112,   115,   149,   150,   153,   154,   155,   156,   163,   204,
     209,   210,   215,   216,   217,   218,   229,   230,   239,   240,
     241,   242,   243,   247,   248,   253,   254,   255,   258,   260,
     261,   262,   263,   264,   266,   267,   268,   269,   270,    94,
     106,    58,     3,    16,    20,    21,    22,    25,    30,    34,
      47,    50,    51,    52,    53,    54,    64,    65,    66,    67,
      69,    70,    71,    76,    81,    82,    83,    84,    85,    86,
      87,    89,    92,    93,    94,    96,    99,   102,   103,   104,
     105,   107,   108,   110,   112,   113,   114,   117,   118,   119,
     121,   151,   152,    50,    82,    49,    50,    82,    86,    94,
      58,     3,     4,     5,     6,     8,     9,    10,    14,    21,
      24,    34,    42,    46,    53,    54,    56,    57,    61,    70,
      72,    91,   105,   119,   133,   134,   137,   140,   143,   152,
     164,   165,   167,   197,   198,   265,   271,   274,   275,    10,
      28,    51,    58,    76,    99,   151,   256,    16,    28,    30,
      36,    51,    58,    71,    81,    83,    84,    99,   103,   106,
     107,   108,   118,   206,   207,   256,   110,    94,   152,   168,
       0,    44,   139,   281,    62,   152,   152,   152,   152,   140,
     152,   152,   168,   152,   152,   152,   152,   152,   140,   145,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,     3,    70,    72,   197,   197,   197,
      57,   152,   201,   202,    13,    14,   146,   272,   273,    49,
     142,    18,   152,   166,    41,    73,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,    68,   189,   190,    63,   126,   126,    97,     9,   151,
     152,     5,     6,     8,    77,   152,   213,   214,   126,   110,
       8,   103,   152,    97,    67,   205,   205,   205,   205,    78,
     191,   192,   152,   117,   152,    97,   142,   197,   278,   279,
     280,    95,   100,   154,   152,   205,    88,    15,    43,   109,
       5,     6,     8,    14,   133,   140,   176,   177,   225,   231,
     232,    92,   113,   120,   170,   113,     8,   140,   141,   197,
     199,   200,   152,   197,   197,   199,    40,   134,   199,   199,
     141,   197,   199,   199,   197,   197,   197,   197,   141,   140,
     140,   140,   141,   142,   144,   126,     8,   197,   273,   140,
     168,   165,   197,   152,   197,   197,   197,   197,   197,   197,
     197,   275,   197,   197,   197,   197,   197,   197,   197,   197,
     197,   197,     5,    77,   137,   197,   213,   213,   126,   126,
      51,   133,     8,    45,    77,   111,   133,   152,   176,   211,
     212,    64,   103,   205,   103,     8,   152,   193,   194,    14,
      29,   103,   208,    67,   120,   249,   250,   152,   244,   245,
     271,   152,   166,    26,   142,   276,   277,   140,   219,    31,
      31,    94,     5,     6,     8,   141,   176,   178,   233,   142,
     234,    24,    46,    60,   104,   259,     8,     4,    24,    34,
      42,    44,    53,    56,    61,    69,   119,   140,   152,   171,
     172,   173,   174,   175,   271,     8,    95,   156,    48,   141,
     142,   141,   141,   152,   141,   141,   141,   141,   141,   141,
     141,   142,   141,   142,   142,   141,   197,   197,   202,   152,
     176,   203,   147,   147,   160,   169,   170,   142,    77,   140,
       5,   211,     9,   214,    66,   205,   205,   126,   142,    98,
       5,    98,     8,   152,   251,   252,   126,   142,   170,   126,
     278,    79,   185,   186,   280,    56,   152,   220,   221,   116,
     152,   152,   152,   141,   142,   141,   142,   225,   232,   235,
     236,   141,   101,   101,   140,   140,   140,   140,   140,   140,
     140,   140,   171,   123,    23,    57,    63,   126,   127,   128,
     129,   130,   131,   137,   141,   152,   200,   141,   197,   197,
     197,   126,    95,   163,    52,   179,     5,   178,   126,    87,
      89,    96,   257,     5,     8,   140,   152,   194,   126,   122,
     140,   176,   177,   245,   191,   176,   177,    26,   189,   141,
     142,   140,   222,   223,    24,    25,    46,    61,    65,    74,
      75,   104,   246,   176,     8,    18,   237,   142,     8,     8,
     152,   271,   134,   271,   141,   141,   271,     8,   141,   141,
     171,   176,   177,     9,   140,    77,   137,     8,   176,   177,
       8,   176,   177,   176,   177,   176,   177,   176,   177,   176,
     177,    57,   142,   157,    57,   141,   141,   142,   203,   164,
     141,     5,   180,   121,   183,   184,   141,   140,    33,   114,
      87,   152,   195,   196,   140,     8,   252,   141,   178,    85,
     175,   187,   188,   220,   140,   224,   225,    78,   142,   226,
      68,   152,   238,   225,   236,   141,   141,   141,   141,   141,
     123,   123,   178,    77,     9,   140,     5,    56,   152,   158,
     159,   141,   271,   140,    49,    79,   161,    26,    52,    55,
     182,   178,   126,   141,   142,     8,   141,   140,    19,    38,
     142,   141,   142,     3,   227,   228,   223,   176,   177,   176,
     177,   141,   178,   142,   141,   199,    26,    68,   162,   175,
     181,    79,   173,   185,   141,   176,   196,   141,   141,   188,
     225,   126,   142,   141,   159,   141,   187,     5,   142,    26,
     189,     8,   228,   142,   142,   175,   187,   191,   140,     5,
     199,   141,   141
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

  case 81:

    { pParser->PushQuery(); ;}
    break;

  case 82:

    { pParser->PushQuery(); ;}
    break;

  case 83:

    { pParser->PushQuery(); ;}
    break;

  case 87:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, yyvsp[-6] );
		;}
    break;

  case 89:

    {
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		;}
    break;

  case 92:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 93:

    {
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), yyvsp[0] );
		;}
    break;

  case 97:

    {
		CSphVector<CSphQueryItem> & dItems = pParser->m_pQuery->m_dItems;
		if ( dItems.GetLength()!=1 || dItems[0].m_sExpr!="*" )
		{
			yyerror ( pParser, "outer select list must be a single star" );
			YYERROR;
		}
		dItems.Reset();
		pParser->ResetSelect();
	;}
    break;

  case 98:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, yyvsp[0] );
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 100:

    {
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 101:

    {
			pParser->m_pQuery->m_iOuterOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iOuterLimit = yyvsp[0].m_iValue;
			pParser->m_pQuery->m_bHasOuter = true;
		;}
    break;

  case 102:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->ToString ( pParser->m_pQuery->m_sIndexes, yyvsp[-7] );
		;}
    break;

  case 105:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 108:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 109:

    { pParser->AliasLastItem ( &yyvsp[0] ); ;}
    break;

  case 110:

    { pParser->AddItem ( &yyvsp[0] ); ;}
    break;

  case 111:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_AVG, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 112:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MAX, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 113:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_MIN, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 114:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_SUM, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 115:

    { pParser->AddItem ( &yyvsp[-1], SPH_AGGR_CAT, &yyvsp[-3], &yyvsp[0] ); ;}
    break;

  case 116:

    { if ( !pParser->AddItem ( "count(*)", &yyvsp[-3], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 117:

    { if ( !pParser->AddItem ( "groupby()", &yyvsp[-2], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 118:

    { if ( !pParser->AddDistinct ( &yyvsp[-1], &yyvsp[-4], &yyvsp[0] ) ) YYERROR; ;}
    break;

  case 120:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 127:

    {
			if ( !pParser->SetMatch(yyvsp[-1]) )
				YYERROR;
		;}
    break;

  case 129:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 130:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-2] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( yyvsp[0].m_iValue );
			pFilter->m_bExclude = true;
		;}
    break;

  case 131:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-4] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 132:

    {
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( yyvsp[-5] );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *yyvsp[-1].m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Uniq();
		;}
    break;

  case 133:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 134:

    {
			if ( !pParser->AddUservarFilter ( yyvsp[-3], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 135:

    {
			if ( !pParser->AddIntRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_iValue ) )
				YYERROR;
		;}
    break;

  case 136:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 137:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, false ) )
				YYERROR;
		;}
    break;

  case 138:

    {
			if ( !pParser->AddIntFilterGreater ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 139:

    {
			if ( !pParser->AddIntFilterLesser ( yyvsp[-2], yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 140:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 141:

    {
			yyerror ( pParser, "NEQ filter on floats is not (yet?) supported" );
			YYERROR;
		;}
    break;

  case 142:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 143:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_iValue, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 144:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-4], yyvsp[-2].m_fValue, yyvsp[0].m_iValue, true ) )
				YYERROR;
		;}
    break;

  case 145:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, false ) )
				YYERROR;
		;}
    break;

  case 146:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, false ) )
				YYERROR;
		;}
    break;

  case 147:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], yyvsp[0].m_fValue, FLT_MAX, true ) )
				YYERROR;
		;}
    break;

  case 148:

    {
			if ( !pParser->AddFloatRangeFilter ( yyvsp[-2], -FLT_MAX, yyvsp[0].m_fValue, true ) )
				YYERROR;
		;}
    break;

  case 149:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], false ) )
				YYERROR;
		;}
    break;

  case 150:

    {
			if ( !pParser->AddStringFilter ( yyvsp[-2], yyvsp[0], true ) )
				YYERROR;
		;}
    break;

  case 151:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-2], true ) )
				YYERROR;
		;}
    break;

  case 152:

    {
			if ( !pParser->AddNullFilter ( yyvsp[-3], false ) )
				YYERROR;
		;}
    break;

  case 155:

    {
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		;}
    break;

  case 156:

    {
			yyval.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 157:

    {
			yyval.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 158:

    {
			yyval.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 159:

    {
			yyval.m_iType = SPHINXQL_TOK_ID;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		;}
    break;

  case 165:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 166:

    {
			yyval.m_iType = TOK_CONST_INT;
			if ( (uint64_t)yyvsp[0].m_iValue > (uint64_t)LLONG_MAX )
				yyval.m_iValue = LLONG_MIN;
			else
				yyval.m_iValue = -yyvsp[0].m_iValue;
		;}
    break;

  case 167:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 168:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = -yyvsp[0].m_fValue; ;}
    break;

  case 169:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 170:

    {
			assert ( !yyval.m_pValues.Ptr() );
			yyval.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			yyval.m_pValues->Add ( yyvsp[0].m_iValue ); 
		;}
    break;

  case 171:

    {
			yyval.m_pValues->Add ( yyvsp[0].m_iValue );
		;}
    break;

  case 175:

    {
			pParser->SetGroupbyLimit ( yyvsp[0].m_iValue );
		;}
    break;

  case 176:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 177:

    {
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 179:

    {
			pParser->AddHaving();
		;}
    break;

  case 182:

    {
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, yyvsp[0] );
		;}
    break;

  case 185:

    {
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, yyvsp[0] );
		;}
    break;

  case 186:

    {
			pParser->m_pQuery->m_sOrderBy = "@random";
		;}
    break;

  case 188:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 190:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 191:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 194:

    {
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 195:

    {
			pParser->m_pQuery->m_iOffset = yyvsp[-2].m_iValue;
			pParser->m_pQuery->m_iLimit = yyvsp[0].m_iValue;
		;}
    break;

  case 201:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 202:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 203:

    {
			if ( !pParser->AddOption ( yyvsp[-4], pParser->GetNamedVec ( yyvsp[-1].m_iValue ) ) )
				YYERROR;
			pParser->FreeNamedVec ( yyvsp[-1].m_iValue );
		;}
    break;

  case 204:

    {
			if ( !pParser->AddOption ( yyvsp[-5], yyvsp[-3], yyvsp[-1] ) )
				YYERROR;
		;}
    break;

  case 205:

    {
			if ( !pParser->AddOption ( yyvsp[-2], yyvsp[0] ) )
				YYERROR;
		;}
    break;

  case 206:

    {
			yyval.m_iValue = pParser->AllocNamedVec ();
			pParser->AddConst ( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 207:

    {
			pParser->AddConst( yyval.m_iValue, yyvsp[0] );
		;}
    break;

  case 208:

    {
			yyval = yyvsp[-2];
			yyval.m_iValue = yyvsp[0].m_iValue;
		;}
    break;

  case 210:

    { if ( !pParser->SetOldSyntax() ) YYERROR; ;}
    break;

  case 211:

    { if ( !pParser->SetNewSyntax() ) YYERROR; ;}
    break;

  case 216:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 217:

    { TRACK_BOUNDS ( yyval, yyvsp[-1], yyvsp[0] ); ;}
    break;

  case 218:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 219:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 220:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 221:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 222:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 223:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 224:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 225:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 226:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 227:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 228:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 229:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 230:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 231:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 232:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 233:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 234:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 235:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 236:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 240:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 241:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 242:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 243:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 244:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 245:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 246:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 247:

    { TRACK_BOUNDS ( yyval, yyvsp[-3], yyvsp[0] ); ;}
    break;

  case 248:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 249:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 250:

    { TRACK_BOUNDS ( yyval, yyvsp[-5], yyvsp[0] ); ;}
    break;

  case 251:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 252:

    { TRACK_BOUNDS ( yyval, yyvsp[-7], yyvsp[0] ); ;}
    break;

  case 253:

    { TRACK_BOUNDS ( yyval, yyvsp[-13], yyvsp[0] ); ;}
    break;

  case 258:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 259:

    { TRACK_BOUNDS ( yyval, yyvsp[-4], yyvsp[0] ); ;}
    break;

  case 266:

    { pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] ); ;}
    break;

  case 267:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; ;}
    break;

  case 268:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; ;}
    break;

  case 269:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_META; ;}
    break;

  case 270:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; ;}
    break;

  case 271:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; ;}
    break;

  case 272:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; ;}
    break;

  case 273:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; ;}
    break;

  case 274:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; ;}
    break;

  case 275:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 276:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 277:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 278:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
		;}
    break;

  case 279:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-2] );
			pParser->m_pStmt->m_iIntParam = int(yyvsp[-1].m_fValue*10);
		;}
    break;

  case 283:

    {
			pParser->m_pStmt->m_iIntParam = yyvsp[0].m_iValue;
		;}
    break;

  case 284:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 285:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] );
		;}
    break;

  case 286:

    {
			pParser->SetStatement ( yyvsp[-2], SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		;}
    break;

  case 287:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 288:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 289:

    { pParser->m_pStmt->m_eStmt = STMT_DUMMY; ;}
    break;

  case 290:

    {
			pParser->SetStatement ( yyvsp[-4], SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
		;}
    break;

  case 291:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, yyvsp[0] ).Unquote();
		;}
    break;

  case 292:

    {
			pParser->SetStatement ( yyvsp[-2], SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = yyvsp[0].m_iValue;
		;}
    break;

  case 293:

    {
			pParser->SetStatement ( yyvsp[-4], SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *yyvsp[-1].m_pValues.Ptr();
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-6] );
		;}
    break;

  case 296:

    { yyval.m_iValue = 1; ;}
    break;

  case 297:

    { yyval.m_iValue = 0; ;}
    break;

  case 298:

    {
			yyval.m_iValue = yyvsp[0].m_iValue;
			if ( yyval.m_iValue!=0 && yyval.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		;}
    break;

  case 306:

    { pParser->m_pStmt->m_eStmt = STMT_COMMIT; ;}
    break;

  case 307:

    { pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; ;}
    break;

  case 308:

    { pParser->m_pStmt->m_eStmt = STMT_BEGIN; ;}
    break;

  case 311:

    {
			// everything else is pushed directly into parser within the rules
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-4] );
		;}
    break;

  case 312:

    { pParser->m_pStmt->m_eStmt = STMT_INSERT; ;}
    break;

  case 313:

    { pParser->m_pStmt->m_eStmt = STMT_REPLACE; ;}
    break;

  case 318:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 319:

    { if ( !pParser->AddSchemaItem ( &yyvsp[0] ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } ;}
    break;

  case 322:

    { if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } ;}
    break;

  case 323:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 324:

    { AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] ); ;}
    break;

  case 325:

    { yyval.m_iType = TOK_CONST_INT; yyval.m_iValue = yyvsp[0].m_iValue; ;}
    break;

  case 326:

    { yyval.m_iType = TOK_CONST_FLOAT; yyval.m_fValue = yyvsp[0].m_fValue; ;}
    break;

  case 327:

    { yyval.m_iType = TOK_QUOTED_STRING; yyval.m_iStart = yyvsp[0].m_iStart; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 328:

    { yyval.m_iType = TOK_CONST_MVA; yyval.m_pValues = yyvsp[-1].m_pValues; ;}
    break;

  case 329:

    { yyval.m_iType = TOK_CONST_MVA; ;}
    break;

  case 334:

    { if ( !pParser->AddInsertOption ( yyvsp[-2], yyvsp[0] ) ) YYERROR; ;}
    break;

  case 335:

    {
		if ( !pParser->DeleteStatement ( &yyvsp[-1] ) )
			YYERROR;
	;}
    break;

  case 336:

    {
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, yyvsp[-4] );
		;}
    break;

  case 337:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 338:

    {
			AddInsval ( pParser, pParser->m_pStmt->m_dInsertValues, yyvsp[0] );
		;}
    break;

  case 340:

    {
			yyval.m_iType = TOK_CONST_STRINGS;
		;}
    break;

  case 341:

    {
			// FIXME? for now, one such array per CALL statement, tops
			if ( pParser->m_pStmt->m_dCallStrings.GetLength() )
			{
				yyerror ( pParser, "unexpected constant string list" );
				YYERROR;
			}
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 342:

    {
			pParser->ToStringUnescape ( pParser->m_pStmt->m_dCallStrings.Add(), yyvsp[0] );
		;}
    break;

  case 345:

    {
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		;}
    break;

  case 347:

    {
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), yyvsp[0] );
			AddInsval ( pParser, pParser->m_pStmt->m_dCallOptValues, yyvsp[-2] );
		;}
    break;

  case 352:

    {
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, yyvsp[-1] );
		;}
    break;

  case 355:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; ;}
    break;

  case 356:

    { pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES; ;}
    break;

  case 357:

    {
			if ( !pParser->UpdateStatement ( &yyvsp[-4] ) )
				YYERROR;
		;}
    break;

  case 360:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)yyvsp[0].m_iValue );
			DWORD uHi = (DWORD)( yyvsp[0].m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_INTEGER );
			}
		;}
    break;

  case 361:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 362:

    {
			pParser->UpdateMVAAttr ( yyvsp[-4], yyvsp[-1] );
		;}
    break;

  case 363:

    {
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( yyvsp[-3], tNoValues );
		;}
    break;

  case 364:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( (DWORD)yyvsp[0].m_iValue );
			DWORD uHi = (DWORD)( yyvsp[0].m_iValue>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->m_tUpdate.m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_INTEGER );
			}
		;}
    break;

  case 365:

    {
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->m_tUpdate.m_dPool.Add ( sphF2DW ( yyvsp[0].m_fValue ) );
			pParser->AddUpdatedAttr ( yyvsp[-2], SPH_ATTR_FLOAT );
		;}
    break;

  case 366:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 367:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 368:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 369:

    { yyval.m_iValue = SPH_ATTR_BOOL; ;}
    break;

  case 370:

    { yyval.m_iValue = SPH_ATTR_UINT32SET; ;}
    break;

  case 371:

    { yyval.m_iValue = SPH_ATTR_INT64SET; ;}
    break;

  case 372:

    { yyval.m_iValue = SPH_ATTR_JSON; ;}
    break;

  case 373:

    { yyval.m_iValue = SPH_ATTR_STRING; ;}
    break;

  case 374:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-4] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[-1] );
			tStmt.m_eAlterColType = (ESphAttr)yyvsp[0].m_iValue;
		;}
    break;

  case 375:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sAlterAttr, yyvsp[0] );
		;}
    break;

  case 376:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-1] );
		;}
    break;

  case 377:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		;}
    break;

  case 378:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
			pParser->ToStringUnescape ( pParser->m_pStmt->m_sStringParam, yyvsp[0] );
		;}
    break;

  case 385:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		;}
    break;

  case 386:

    {
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		;}
    break;

  case 387:

    {
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		;}
    break;

  case 395:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( tStmt.m_sUdfLib, yyvsp[0] );
			tStmt.m_eUdfType = (ESphAttr) yyvsp[-2].m_iValue;
		;}
    break;

  case 396:

    { yyval.m_iValue = SPH_ATTR_INTEGER; ;}
    break;

  case 397:

    { yyval.m_iValue = SPH_ATTR_BIGINT; ;}
    break;

  case 398:

    { yyval.m_iValue = SPH_ATTR_FLOAT; ;}
    break;

  case 399:

    { yyval.m_iValue = SPH_ATTR_STRINGPTR; ;}
    break;

  case 400:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, yyvsp[0] );
		;}
    break;

  case 401:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[-3] );
			pParser->ToString ( tStmt.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 402:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 403:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 404:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[-1] );
		;}
    break;

  case 407:

    {
			pParser->m_pStmt->m_eStmt = STMT_SELECT_DUAL;
			pParser->ToString ( pParser->m_pStmt->m_tQuery.m_sQuery, yyvsp[0] );
		;}
    break;

  case 408:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 409:

    {
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			pParser->ToString ( tStmt.m_sIndex, yyvsp[0] );
		;}
    break;

  case 410:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-4] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sUdfLib, yyvsp[0] );
		;}
    break;

  case 411:

    {
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, yyvsp[-2] );
			pParser->ToStringUnescape ( s.m_sStringParam, yyvsp[0] );
		;}
    break;

  case 412:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 414:

    { yyval = yyvsp[-1]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 415:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 416:

    { yyval = yyvsp[0]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 417:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 418:

    { yyval = yyvsp[-2]; yyval.m_iEnd = yyvsp[0].m_iEnd; ;}
    break;

  case 419:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 420:

    { TRACK_BOUNDS ( yyval, yyvsp[-2], yyvsp[0] ); ;}
    break;

  case 424:

    {
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		;}
    break;

  case 426:

    {
			pParser->AddItem ( &yyvsp[0] );
			pParser->AddGroupBy ( yyvsp[0] );
		;}
    break;

  case 429:

    {
			pParser->m_pStmt->m_eStmt = STMT_FACET;
			if ( pParser->m_pQuery->m_sFacetBy.IsEmpty() )
			{
				pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
				pParser->AddCount ();
			}
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
	    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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
		    if (yyx+yyn<int(sizeof(yycheck)/sizeof(yycheck[0])) && yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
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

