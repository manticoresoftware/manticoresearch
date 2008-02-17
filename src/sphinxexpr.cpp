//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxexpr.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////////

#ifndef M_LOG2E
#define M_LOG2E    1.44269504088896340736
#endif

#ifndef M_LOG10E
#define M_LOG10E   0.434294481903251827651
#endif

//////////////////////////////////////////////////////////////////////////
// EVALUATION ENGINE
//////////////////////////////////////////////////////////////////////////

struct Expr_GetInt_c : public ISphExpr
{
	int m_iRowitem;
	Expr_GetInt_c ( int iLocator ) : m_iRowitem ( iLocator ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)tMatch.m_pRowitems[m_iRowitem]; }
};


struct Expr_GetBits_c : public ISphExpr
{
	int m_iRowitem;
	int m_iShift;
	DWORD m_uMask;
	Expr_GetBits_c ( int iLocator ) : m_iRowitem ( iLocator>>16 ), m_iShift ( (iLocator>>8)&255 ), m_uMask ( (1UL<<(iLocator&255)) - 1 ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return float ( ( tMatch.m_pRowitems[m_iRowitem]>>m_iShift ) & m_uMask ); }
};


struct Expr_GetFloat_c : public ISphExpr
{
	int m_iRowitem;
	Expr_GetFloat_c ( int iLocator ) : m_iRowitem ( iLocator ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return sphDW2F(tMatch.m_pRowitems[m_iRowitem]); }
};


struct Expr_GetConst_c : public ISphExpr
{
	float m_fValue;
	Expr_GetConst_c ( float fValue ) : m_fValue ( fValue ) {}
	virtual float Eval ( const CSphMatch & ) const { return m_fValue; }
};


struct Expr_GetId_c : public ISphExpr
{
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)tMatch.m_iDocID; }
};


struct Expr_GetWeight_c : public ISphExpr
{
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)tMatch.m_iWeight; }
};

//////////////////////////////////////////////////////////////////////////

struct Expr_Arglist_c : public ISphExpr
{
	CSphVector<ISphExpr *> m_dArgs;

	Expr_Arglist_c ( ISphExpr * pLeft, ISphExpr * pRight )
	{
		AddArgs ( pLeft );
		AddArgs ( pRight );
	}

	~Expr_Arglist_c ()
	{
		ARRAY_FOREACH ( i, m_dArgs )
			SafeDelete ( m_dArgs[i] );
	}

	void AddArgs ( ISphExpr * pExpr )
	{
		// not an arglist? just add it
		if ( !pExpr->IsArglist() )
		{
			m_dArgs.Add ( pExpr );
			return;
		}

		// arglist? take ownership of its args, and dismiss it
		Expr_Arglist_c * pArgs = (Expr_Arglist_c *) pExpr;
		ARRAY_FOREACH ( i, pArgs->m_dArgs )
		{
			m_dArgs.Add ( pArgs->m_dArgs[i] );
			pArgs->m_dArgs[i] = NULL;
		}
		SafeDelete ( pExpr );
	}

	virtual bool IsArglist () const
	{
		return true;
	}

	virtual float Eval ( const CSphMatch & ) const
	{
		assert ( 0 && "internal error: Eval() must not be explicitly called on arglist" );
		return 0.0f;
	}
};

//////////////////////////////////////////////////////////////////////////

#define FIRST	m_pFirst->Eval(tMatch)
#define SECOND	m_pSecond->Eval(tMatch)
#define THIRD	m_pThird->Eval(tMatch)

#define DECLARE_UNARY(_classname,_expr) \
	struct _classname : public ISphExpr \
	{ \
		ISphExpr * m_pFirst; \
		_classname ( ISphExpr * pFirst ) : m_pFirst ( pFirst ) {}; \
		~_classname () { SafeDelete ( m_pFirst ); } \
		virtual float Eval ( const CSphMatch & tMatch ) const { return _expr; } \
	};

DECLARE_UNARY ( Expr_Neg_c,		-FIRST )
DECLARE_UNARY ( Expr_Abs_c,		fabs(FIRST) )
DECLARE_UNARY ( Expr_Ceil_c,	float(ceil(FIRST)) )
DECLARE_UNARY ( Expr_Floor_c,	float(floor(FIRST)) )
DECLARE_UNARY ( Expr_Sin_c,		float(sin(FIRST)) )
DECLARE_UNARY ( Expr_Cos_c,		float(cos(FIRST)) )
DECLARE_UNARY ( Expr_Ln_c,		float(log(FIRST)) )
DECLARE_UNARY ( Expr_Log2_c,	float(log(FIRST)*M_LOG2E) )
DECLARE_UNARY ( Expr_Log10_c,	float(log(FIRST)*M_LOG10E) )
DECLARE_UNARY ( Expr_Exp_c,		float(exp(FIRST)) )
DECLARE_UNARY ( Expr_Sqrt_c,	float(sqrt(FIRST)) )

//////////////////////////////////////////////////////////////////////////

#define DECLARE_BINARY(_classname,_expr) \
	struct _classname : public ISphExpr \
	{ \
		ISphExpr * m_pFirst; \
		ISphExpr * m_pSecond; \
		_classname ( ISphExpr * pFirst, ISphExpr * pSecond ) : m_pFirst ( pFirst ), m_pSecond ( pSecond ) {} \
		~_classname () { SafeDelete ( m_pFirst ); SafeDelete ( m_pSecond ); } \
		virtual float Eval ( const CSphMatch & tMatch ) const { return _expr; } \
	};

DECLARE_BINARY ( Expr_Add_c,	FIRST + SECOND ) 
DECLARE_BINARY ( Expr_Sub_c,	FIRST - SECOND ) 
DECLARE_BINARY ( Expr_Mul_c,	FIRST * SECOND ) 
DECLARE_BINARY ( Expr_Div_c,	FIRST / SECOND ) 
DECLARE_BINARY ( Expr_Lt_c,		(FIRST < SECOND) ? 1.0f : 0.0f ) 
DECLARE_BINARY ( Expr_Gt_c,		(FIRST > SECOND) ? 1.0f : 0.0f ) 
DECLARE_BINARY ( Expr_Lte_c,	(FIRST <= SECOND) ? 1.0f : 0.0f ) 
DECLARE_BINARY ( Expr_Gte_c,	(FIRST >= SECOND) ? 1.0f : 0.0f ) 
DECLARE_BINARY ( Expr_Eq_c,		fabs(FIRST-SECOND)<=1e-6 ? 1.0f : 0.0f ) 
DECLARE_BINARY ( Expr_Ne_c,		fabs(FIRST-SECOND)>1e-6 ? 1.0f : 0.0f ) 

DECLARE_BINARY ( Expr_Min_c,	Min(FIRST,SECOND) )
DECLARE_BINARY ( Expr_Max_c,	Max(FIRST,SECOND) )
DECLARE_BINARY ( Expr_Pow_c,	float(pow(FIRST,SECOND)) )

//////////////////////////////////////////////////////////////////////////

#define DECLARE_TERNARY(_classname,_expr) \
	struct _classname : public ISphExpr \
	{ \
		ISphExpr * m_pFirst; \
		ISphExpr * m_pSecond; \
		ISphExpr * m_pThird; \
		_classname ( ISphExpr * pFirst, ISphExpr * pSecond, ISphExpr * pThird ) : m_pFirst ( pFirst ), m_pSecond ( pSecond ), m_pThird ( pThird ) {} \
		~_classname () { SafeDelete ( m_pFirst ); SafeDelete ( m_pSecond ); SafeDelete ( m_pThird ); } \
		virtual float Eval ( const CSphMatch & tMatch ) const { return _expr; } \
	};

DECLARE_TERNARY ( Expr_If_c,	( FIRST!=0.0f ) ? SECOND : THIRD )
DECLARE_TERNARY ( Expr_Madd_c,	FIRST*SECOND+THIRD )
DECLARE_TERNARY ( Expr_Mul3_c,	FIRST*SECOND*THIRD )

//////////////////////////////////////////////////////////////////////////
// PARSER INTERNALS
//////////////////////////////////////////////////////////////////////////

/// known docinfo entries list
enum Docinfo_e
{
	DI_ID,
	DI_WEIGHT
};

#include "sphinxexpryy.hpp"

//////////////////////////////////////////////////////////////////////////

/// known functions
enum Func_e
{
	FUNC_ABS,
	FUNC_CEIL,
	FUNC_FLOOR,
	FUNC_SIN,
	FUNC_COS,
	FUNC_LN,
	FUNC_LOG2,
	FUNC_LOG10,
	FUNC_EXP,
	FUNC_SQRT,

	FUNC_MIN,
	FUNC_MAX,
	FUNC_POW,

	FUNC_IF,
	FUNC_MADD,
	FUNC_MUL3
};


struct FuncDesc_t
{
	const char *	m_sName;
	int				m_iArgs;
	Func_e			m_eFunc;
};


static FuncDesc_t g_dFuncs[] =
{
	{ "abs",	1,	FUNC_ABS },
	{ "ceil",	1,	FUNC_CEIL },
	{ "floor",	1,	FUNC_FLOOR },
	{ "sin",	1,	FUNC_SIN },
	{ "cos",	1,	FUNC_COS },
	{ "ln",		1,	FUNC_LN },
	{ "log2",	1,	FUNC_LOG2 },
	{ "log10",	1,	FUNC_LOG10 },
	{ "exp",	1,	FUNC_EXP },
	{ "sqrt",	1,	FUNC_SQRT },

	{ "min",	2,	FUNC_MIN },
	{ "max",	2,	FUNC_MAX },
	{ "pow",	2,	FUNC_POW },

	{ "if",		3,	FUNC_IF },
	{ "madd",	3,	FUNC_MADD },
	{ "mul3",	3,	FUNC_MUL3 }
};

//////////////////////////////////////////////////////////////////////////

/// expression tree node
struct ExprNode_t
{
	int				m_iToken;	///< token type, including operators
	union
	{

		int			m_iAttrLocator;	///< attribute locator, for TOK_ATTR type
		float		m_fConst;		///< constant value, for TOK_CONST type
		int			m_iFunc;		///< built-in function id, for TOK_FUNC type
		Docinfo_e	m_eDocinfo;		///< docinfo field id, for TOK_DOCINFO type
		int			m_iArgs;		///< args count, for arglist (token==',') type
	};
	int				m_iLeft;
	int				m_iRight;

	ExprNode_t () : m_iToken ( 0 ), m_iLeft ( -1 ), m_iRight ( -1 ) {}
};

/// expression parser
class ExprParser_t
{
	friend int				yylex ( YYSTYPE * lvalp, ExprParser_t * pParser );
	friend int				yyparse ( ExprParser_t * pParser );
	friend void				yyerror ( ExprParser_t * pParser, const char * sMessage );

public:	
							ExprParser_t () {}
							~ExprParser_t () {}

	ISphExpr *				Parse ( const char * sExpr, const CSphSchema & tSchema, CSphString & sError );

protected:
	int						m_iParsed;	///< filled by yyparse() at the very end
	CSphString				m_sLexerError;
	CSphString				m_sParserError;

protected:
	int						AddNodeNumber ( float fValue );
	int						AddNodeAttr ( int iTokenType, int iAttrLocator );
	int						AddNodeDocinfo ( Docinfo_e eDocinfo );
	int						AddNodeOp ( int iOp, int iLeft, int iRight );
	int						AddNodeFunc ( int iFunc, int iLeft );

private:
	const char *			m_sExpr;
	const char *			m_pCur;
	const char *			m_pLastTokenStart;
	const CSphSchema *		m_pSchema;
	CSphVector<ExprNode_t>	m_dNodes;

private:
	int						GetToken ( YYSTYPE * lvalp );
	void					Optimize ( int iNode );
	ISphExpr *				CreateTree ( int iNode );
};

//////////////////////////////////////////////////////////////////////////

/// let's build our own theme park!
static inline int sphIsAttr ( int c )
{
	// different from sphIsAlpha() in that we don't allow minus
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_';
}

/// a lexer of my own
/// returns token id and fills lvalp on success
/// returns -1 and fills sError on failure
int ExprParser_t::GetToken ( YYSTYPE * lvalp )
{
	// skip whitespace, check eof
	while ( isspace(*m_pCur) ) m_pCur++;
	m_pLastTokenStart = m_pCur;
	if ( !*m_pCur ) return 0;

	// check for constant
	if ( isdigit(*m_pCur) )
	{
		char * pEnd = NULL;
		lvalp->fNumber = (float)strtod ( m_pCur, &pEnd );
		m_pCur = pEnd;
		return TOK_NUMBER;
	}

	// check for magic names
	if ( *m_pCur=='@' && sphIsAttr(m_pCur[1]) && !isdigit(m_pCur[1]) )
	{
		// get token
		const char * pStart = ++m_pCur;
		while ( sphIsAttr(*m_pCur) ) m_pCur++;

		CSphString sTok;
		sTok.SetBinary ( pStart, m_pCur-pStart );
		sTok.ToLower ();

		if ( sTok=="id" )		{ lvalp->eDocinfo = DI_ID; return TOK_DOCINFO; }
		if ( sTok=="weight" )	{ lvalp->eDocinfo = DI_WEIGHT; return TOK_DOCINFO; }

		m_sLexerError.SetSprintf ( "unknown magic name '@%s'", sTok.cstr() );
		return -1;
	}

	// check for field or function name
	if ( sphIsAttr(*m_pCur) )
	{
		// get token
		const char * pStart = m_pCur;
		assert ( !isdigit(*m_pCur) ); // can't start with a digit, right?

		while ( sphIsAttr(*m_pCur) ) m_pCur++;
		assert ( !sphIsAttr(*m_pCur) );

		CSphString sTok;
		sTok.SetBinary ( pStart, m_pCur-pStart );

		// check for attribute
		int iAttr = m_pSchema->GetAttrIndex ( sTok.cstr() );
		if ( iAttr>=0 )
		{
			// check attribute type and width
			const CSphColumnInfo & tCol = m_pSchema->GetAttr ( iAttr );
			if ( tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP || tCol.m_eAttrType==SPH_ATTR_BOOL )
			{
				if ( tCol.m_iRowitem>=0 )
				{
					lvalp->iAttrLocator = tCol.m_iRowitem;
					return TOK_ATTR_INT;
				} else
				{
					DWORD uRowitem = tCol.m_iBitOffset / ROWITEM_BITS;
					DWORD uItemOff = tCol.m_iBitOffset % ROWITEM_BITS;

					assert ( uRowitem<=65535 );
					assert ( uItemOff<=255 );
					assert ( tCol.m_iBitCount<=255 );
					lvalp->iAttrLocator = ( uRowitem<<16 ) + ( uItemOff<<8 ) + tCol.m_iBitCount;
					return TOK_ATTR_BITS;
				}
			} else if ( tCol.m_eAttrType==SPH_ATTR_FLOAT )
			{
				lvalp->iAttrLocator = tCol.m_iRowitem;
				return TOK_ATTR_FLOAT;
			} else
			{
				if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
					m_sLexerError.SetSprintf ( "attribute '%s' is MVA, can not be used in expressions", sTok.cstr() );
				else
					m_sLexerError.SetSprintf ( "attribute '%s' is of unsupported type (type=%d)", sTok.cstr(), tCol.m_eAttrType );
				return -1;
			}
		}

		// check for function
		sTok.ToLower();
		for ( int i=0; i<int(sizeof(g_dFuncs)/sizeof(g_dFuncs[0])); i++ )
			if ( sTok==g_dFuncs[i].m_sName )
		{
			lvalp->iFunc = i;
			return TOK_FUNC;
		}

		m_sLexerError.SetSprintf ( "unknown identifier '%s' (not an attribute, not a function)", sTok.cstr() );
		return -1;
	}

	// check for known operators, then
	switch ( *m_pCur )
	{
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
		case ')':
		case ',':
			return *m_pCur++;

		case '<':
			m_pCur++;
			if ( *m_pCur=='>' ) { m_pCur++; return TOK_NE; }
			if ( *m_pCur=='=' ) { m_pCur++; return TOK_LTE; }
			return '<';

		case '>':
			m_pCur++;
			if ( *m_pCur=='=' ) { m_pCur++; return TOK_GTE; }
			return '>';

		case '=':
			m_pCur++;
			if ( *m_pCur=='=' ) m_pCur++;
			return TOK_EQ;
	}

	m_sLexerError.SetSprintf ( "unknown operator '%c' near '%s'", *m_pCur, m_pCur );
	return -1;
}

/// is arithmetic?
static inline bool IsAri ( int iTok )
{
	return iTok=='+' || iTok=='-' || iTok=='*' || iTok=='/';
}

/// optimize subtree
void ExprParser_t::Optimize ( int iNode )
{
	if ( iNode<0 )
		return;

	Optimize ( m_dNodes[iNode].m_iLeft );
	Optimize ( m_dNodes[iNode].m_iRight );

	ExprNode_t * pRoot = &m_dNodes[iNode];

	// madd, mul3
	if ( ( pRoot->m_iToken=='+' || pRoot->m_iToken=='*' )
		&& ( m_dNodes[pRoot->m_iLeft].m_iToken=='*' || m_dNodes[pRoot->m_iRight].m_iToken=='*' ) )
	{
		if ( m_dNodes[pRoot->m_iLeft].m_iToken!='*' )
			Swap ( pRoot->m_iLeft, pRoot->m_iRight );
		assert ( m_dNodes[pRoot->m_iLeft].m_iToken=='*' );

		m_dNodes.Resize ( m_dNodes.GetLength()+1 );
		pRoot = &m_dNodes[iNode];

		m_dNodes[pRoot->m_iLeft].m_iToken = ',';

		m_dNodes.Last().m_iToken = ',';
		m_dNodes.Last().m_iLeft = pRoot->m_iLeft;
		m_dNodes.Last().m_iRight = pRoot->m_iRight;

		pRoot->m_iFunc = ( pRoot->m_iToken=='+' ) ? FUNC_MADD : FUNC_MUL3;
		pRoot->m_iToken = TOK_FUNC;
		assert ( g_dFuncs[pRoot->m_iFunc].m_eFunc==pRoot->m_iFunc );

		pRoot->m_iLeft = m_dNodes.GetLength()-1;
		pRoot->m_iRight = -1;
		return;
	}

	// constant arithmetic expression
	if ( IsAri ( pRoot->m_iToken )
		&& m_dNodes[pRoot->m_iLeft].m_iToken==TOK_NUMBER
		&& m_dNodes[pRoot->m_iRight].m_iToken==TOK_NUMBER )
	{
		float fLeft = m_dNodes[pRoot->m_iLeft].m_fConst;
		float fRight = m_dNodes[pRoot->m_iRight].m_fConst;
		switch ( pRoot->m_iToken )
		{
			case '+':	pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = fLeft + fRight; break;
			case '-':	pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = fLeft - fRight; break;
			case '*':	pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = fLeft * fRight; break;
			case '/':	pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = fLeft / fRight; break;
		}
		return;
	}

	// division by a constant (replace with multiplication by inverse)
	if ( pRoot->m_iToken=='/' && m_dNodes[pRoot->m_iRight].m_iToken==TOK_NUMBER )
	{
		m_dNodes[pRoot->m_iRight].m_fConst = 1.0f / m_dNodes[pRoot->m_iRight].m_fConst;
		pRoot->m_iToken = '*';
		return;
	}

	// unary function from a constant
	if ( pRoot->m_iToken==TOK_FUNC && g_dFuncs[pRoot->m_iFunc].m_iArgs==1 && m_dNodes[pRoot->m_iLeft].m_iToken==TOK_NUMBER )
	{
		float fLeft = m_dNodes[pRoot->m_iLeft].m_fConst;
		switch ( g_dFuncs[pRoot->m_iFunc].m_eFunc )
		{
			case FUNC_ABS:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = fabs(fLeft); break;
			case FUNC_CEIL:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(ceil(fLeft)); break;
			case FUNC_FLOOR:	pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(floor(fLeft)); break;
			case FUNC_SIN:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(sin(fLeft)); break;
			case FUNC_COS:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(cos(fLeft)); break;
			case FUNC_LN:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(log(fLeft)); break;
			case FUNC_LOG2:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(log(fLeft)*M_LOG2E); break;
			case FUNC_LOG10:	pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(log(fLeft)*M_LOG10E); break;
			case FUNC_EXP:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(exp(fLeft)); break;
			case FUNC_SQRT:		pRoot->m_iToken = TOK_NUMBER; pRoot->m_fConst = float(sqrt(fLeft)); break;
			default:			break;
		}
		return;
	}
}

/// fold nodes subtree into opcodes
ISphExpr * ExprParser_t::CreateTree ( int iNode )
{
	if ( iNode<0 )
		return NULL;

	const ExprNode_t & tNode = m_dNodes[iNode];
	ISphExpr * pLeft = CreateTree ( tNode.m_iLeft );
	ISphExpr * pRight = CreateTree ( tNode.m_iRight );

	switch ( tNode.m_iToken )
	{
		case TOK_ATTR_INT:		return new Expr_GetInt_c ( tNode.m_iAttrLocator );
		case TOK_ATTR_BITS:		return new Expr_GetBits_c ( tNode.m_iAttrLocator );
		case TOK_ATTR_FLOAT:	return new Expr_GetFloat_c ( tNode.m_iAttrLocator );
		case TOK_NUMBER:		return new Expr_GetConst_c ( tNode.m_fConst );
		case TOK_DOCINFO:
			switch ( tNode.m_eDocinfo )
			{
				case DI_ID:		return new Expr_GetId_c ();
				case DI_WEIGHT:	return new Expr_GetWeight_c ();
				default:		assert ( 0 && "unhandled docinfo element id" ); break;
			}
			break;

		case '+':				return new Expr_Add_c ( pLeft, pRight ); break;
		case '-':				return new Expr_Sub_c ( pLeft, pRight ); break;
		case '*':				return new Expr_Mul_c ( pLeft, pRight ); break;
		case '/':				return new Expr_Div_c ( pLeft, pRight ); break;
		case '<':				return new Expr_Lt_c ( pLeft, pRight ); break;
		case '>':				return new Expr_Gt_c ( pLeft, pRight ); break;
		case TOK_LTE:			return new Expr_Lte_c ( pLeft, pRight ); break;
		case TOK_GTE:			return new Expr_Gte_c ( pLeft, pRight ); break;
		case TOK_EQ:			return new Expr_Eq_c ( pLeft, pRight ); break;
		case TOK_NE:			return new Expr_Ne_c ( pLeft, pRight ); break;

		case ',':				return new Expr_Arglist_c ( pLeft, pRight ); break;
		case TOK_NEG:			assert ( pRight==NULL ); return new Expr_Neg_c ( pLeft ); break;
		case TOK_FUNC:
			{
				// fold arglist to array
				CSphVector<ISphExpr *> dArgs;
				assert ( pLeft );
				if ( pLeft->IsArglist() )
				{
					assert ( !pRight );
					Expr_Arglist_c * pArgs = (Expr_Arglist_c *) pLeft;

					dArgs = pArgs->m_dArgs;
					pArgs->m_dArgs.Reset ();
					SafeDelete ( pLeft );

				} else
				{
					dArgs.Add ( pLeft );
				}

				// spawn proper function
				assert ( tNode.m_iFunc>=0 && tNode.m_iFunc<int(sizeof(g_dFuncs)/sizeof(g_dFuncs[0])) );
				assert ( g_dFuncs[tNode.m_iFunc].m_iArgs==dArgs.GetLength() );

				switch ( g_dFuncs[tNode.m_iFunc].m_eFunc )
				{
					case FUNC_ABS:		return new Expr_Abs_c ( dArgs[0] ); break;
					case FUNC_CEIL:		return new Expr_Ceil_c ( dArgs[0] ); break;
					case FUNC_FLOOR:	return new Expr_Floor_c ( dArgs[0] ); break;
					case FUNC_SIN:		return new Expr_Sin_c ( dArgs[0] ); break;
					case FUNC_COS:		return new Expr_Cos_c ( dArgs[0] ); break;
					case FUNC_LN:		return new Expr_Ln_c ( dArgs[0] ); break;
					case FUNC_LOG2:		return new Expr_Log2_c ( dArgs[0] ); break;
					case FUNC_LOG10:	return new Expr_Log10_c ( dArgs[0] ); break;
					case FUNC_EXP:		return new Expr_Exp_c ( dArgs[0] ); break;
					case FUNC_SQRT:		return new Expr_Sqrt_c ( dArgs[0] ); break;

					case FUNC_MIN:		return new Expr_Min_c ( dArgs[0], dArgs[1] ); break;
					case FUNC_MAX:		return new Expr_Max_c ( dArgs[0], dArgs[1] ); break;
					case FUNC_POW:		return new Expr_Pow_c ( dArgs[0], dArgs[1] ); break;

					case FUNC_IF:		return new Expr_If_c ( dArgs[0], dArgs[1], dArgs[2] ); break;
					case FUNC_MADD:		return new Expr_Madd_c ( dArgs[0], dArgs[1], dArgs[2] ); break;
					case FUNC_MUL3:		return new Expr_Mul3_c ( dArgs[0], dArgs[1], dArgs[2] ); break;
				}
				assert ( 0 && "unhandled function id" );
				break;
			}

		default:				assert ( 0 && "unhandled token type" ); break;
	}

	// fire exit
	SafeDelete ( pLeft );
	SafeDelete ( pRight );
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

int yylex ( YYSTYPE * lvalp, ExprParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

void yyerror ( ExprParser_t * pParser, const char * sMessage )
{
	pParser->m_sParserError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart );
}

#if USE_WINDOWS
#pragma warning(push,1)
#endif

#include "sphinxexpryy.cpp"

#if USE_WINDOWS
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////

int ExprParser_t::AddNodeNumber ( float fValue )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_NUMBER;
	tNode.m_fConst = fValue;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeAttr ( int iTokenType, int iAttrLocator )
{
	assert ( iTokenType==TOK_ATTR_INT || iTokenType==TOK_ATTR_BITS || iTokenType==TOK_ATTR_FLOAT );
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iTokenType;
	tNode.m_iAttrLocator = iAttrLocator;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeDocinfo ( Docinfo_e eDocinfo )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_DOCINFO;
	tNode.m_eDocinfo  = eDocinfo;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeOp ( int iOp, int iLeft, int iRight )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iOp;
	tNode.m_iArgs = 0;
	if ( iOp==',' )
	{
		if ( iLeft>=0 )		tNode.m_iArgs += ( m_dNodes[iLeft].m_iToken==',' ) ? m_dNodes[iLeft].m_iArgs : 1;
		if ( iRight>=0 )	tNode.m_iArgs += ( m_dNodes[iRight].m_iToken==',' ) ? m_dNodes[iRight].m_iArgs : 1;
	}
	tNode.m_iLeft = iLeft;
	tNode.m_iRight = iRight;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeFunc ( int iFunc, int iArgsNode )
{
	// check args count
	assert ( iFunc>=0 && iFunc<int(sizeof(g_dFuncs)/sizeof(g_dFuncs[0])) );
	int iExpectedArgc = g_dFuncs[iFunc].m_iArgs;
	int iArgc = ( m_dNodes[iArgsNode].m_iToken==',' ) ? m_dNodes[iArgsNode].m_iArgs : 1;
	if ( iArgc!=iExpectedArgc )
	{
		m_sParserError.SetSprintf ( "%s() called with %d args, %d args expected", g_dFuncs[iFunc].m_sName, iArgc, iExpectedArgc );
		return -1;
	}

	// do add
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = iArgsNode;
	return m_dNodes.GetLength()-1;
}

ISphExpr * ExprParser_t::Parse ( const char * sExpr, const CSphSchema & tSchema, CSphString & sError )
{
	// setup lexer
	m_sExpr = sExpr;
	m_pCur = sExpr;
	m_pSchema = &tSchema;

	// build tree
	m_iParsed = -1;
	yyparse ( this );

	// handle errors
	if ( m_iParsed<0 || !m_sLexerError.IsEmpty() || !m_sParserError.IsEmpty() )
	{
		sError = !m_sLexerError.IsEmpty() ? m_sLexerError : m_sParserError;
		if ( sError.IsEmpty() ) sError = "general parsing error";
		return NULL;
	}

	// perform optimizations, create evaluator
	Optimize ( m_iParsed );
	ISphExpr * pRes = CreateTree ( m_iParsed );
	if ( !pRes )
		sError.SetSprintf ( "empty expression" );
	return pRes;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC STUFF
//////////////////////////////////////////////////////////////////////////

/// parser entry point
ISphExpr * sphExprParse ( const char * sExpr, const CSphSchema & tSchema, CSphString & sError )
{
	// parse into opcodes
	ExprParser_t tParser;
	return tParser.Parse ( sExpr, tSchema, sError );
}

//
// $Id$
//
