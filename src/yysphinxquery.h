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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_KEYWORD = 258,
     TOK_NEAR = 259,
     TOK_INT = 260,
     TOK_FIELDLIMIT = 261,
     TOK_BEFORE = 262
   };
#endif
#define TOK_KEYWORD 258
#define TOK_NEAR 259
#define TOK_INT 260
#define TOK_FIELDLIMIT 261
#define TOK_BEFORE 262




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)

typedef union YYSTYPE {
	XQNode_t *		pNode;			// tree node
	struct
	{
		int			iValue;
		int			iStrIndex;
	} tInt;
	struct							// field spec
	{
		DWORD		uMask;			// acceptable fields mask
		int			iMaxPos;		// max allowed position within field
	} tFieldLimit;
} YYSTYPE;
/* Line 1204 of yacc.c.  */

# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





