/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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
    TOK_FUNC_RAND = 272,
    TOK_FUNC_REMAP = 273,
    TOK_FUNC_PF = 274,
    TOK_USERVAR = 275,
    TOK_UDF = 276,
    TOK_HOOK_IDENT = 277,
    TOK_HOOK_FUNC = 278,
    TOK_IDENT = 279,
    TOK_ATTR_JSON = 280,
    TOK_ATID = 281,
    TOK_ATWEIGHT = 282,
    TOK_ID = 283,
    TOK_GROUPBY = 284,
    TOK_WEIGHT = 285,
    TOK_COUNT = 286,
    TOK_DISTINCT = 287,
    TOK_CONST_LIST = 288,
    TOK_ATTR_SINT = 289,
    TOK_MAP_ARG = 290,
    TOK_FOR = 291,
    TOK_ITERATOR = 292,
    TOK_IS = 293,
    TOK_NULL = 294,
    TOK_IS_NULL = 295,
    TOK_IS_NOT_NULL = 296,
    TOK_OR = 297,
    TOK_AND = 298,
    TOK_EQ = 299,
    TOK_NE = 300,
    TOK_LTE = 301,
    TOK_GTE = 302,
    TOK_DIV = 303,
    TOK_MOD = 304,
    TOK_NOT = 305,
    TOK_NEG = 306
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


	int64_t			iConst;			// constant value
	float			fConst;			// constant value
	uint64_t		iAttrLocator;	// attribute locator (rowitem for int/float; offset+size for bits)
	int				iFunc;			// function id
	int				iNode;			// node, or uservar, or udf index
	const char *	sIdent;			// generic identifier (token does NOT own ident storage; ie values are managed by parser)


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (ExprParser_t * pParser);

#endif /* !YY_YY_YYSPHINXEXPR_H_INCLUDED  */
