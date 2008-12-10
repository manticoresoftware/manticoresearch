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

#ifndef _sphinxquery_
#define _sphinxquery_

#include "sphinx.h"

//////////////////////////////////////////////////////////////////////////////

/// extended query word with attached position within atom
struct CSphExtendedQueryAtomWord
{
	CSphString	m_sWord;
	int			m_iAtomPos;
	int			m_iFieldPos;	///< specific position within field, INT_MAX means no limit
	bool		m_bFieldStart;	///< must occur at very start
	bool		m_bFieldEnd;	///< must occur at very end

	CSphExtendedQueryAtomWord () : m_iAtomPos ( -1 ), m_bFieldStart ( false ), m_bFieldEnd ( false ) {}
	CSphExtendedQueryAtomWord ( const char * sWord, int iPos ) : m_sWord ( sWord ), m_iAtomPos ( iPos ), m_bFieldStart ( false ), m_bFieldEnd ( false ) {}
};


/// extended query atom
/// atom is a list of required query words with field and proximity constraints
struct CSphExtendedQueryAtom
{
	CSphVector<CSphExtendedQueryAtomWord>	m_dWords;
	DWORD		m_uFields;
	int			m_iMaxFieldPos;		/// max acceptable position within field; 0 means no limit
	int			m_iMaxDistance;
	bool		m_bQuorum;

	/// default ctor
	CSphExtendedQueryAtom ()
		: m_uFields		( 0xFFFFFFFF )
		, m_iMaxFieldPos( 0 )
		, m_iMaxDistance( -1 )
		, m_bQuorum		( false )
	{}

	/// default dtor
	virtual ~CSphExtendedQueryAtom () {}

	/// clears but does NOT free everything
	/// NOTE: preserves field ID!
	virtual void Reset ()
	{
		m_dWords.Reset ();
		m_iMaxDistance = -1;
	}

	/// check if we're empty
	bool IsEmpty () const
	{
		return m_dWords.GetLength()==0;
	}
};


/// extended query operator
enum ESphExtendedQueryOperator
{
	SPH_QUERY_AND,
	SPH_QUERY_OR,
	SPH_QUERY_NOT,
	SPH_QUERY_ANDNOT,
	SPH_QUERY_BEFORE
};


/// extended query node
/// plain nodes are just an atom
/// non-plain nodes are a logical function over children nodes
struct CSphExtendedQueryNode : public ISphNoncopyable
{
	CSphExtendedQueryNode *					m_pParent;		///< my parent node (NULL for root ones)
	CSphExtendedQueryAtom					m_tAtom;		///< plain node atom
	ESphExtendedQueryOperator				m_eOp;			///< operation over childen
	CSphVector<CSphExtendedQueryNode*>		m_dChildren;	///< non-plain node children

	bool									m_bFieldSpec;	///< whether field spec was already explicitly set
	DWORD									m_uFieldMask;	///< fields mask (spec part)
	int										m_iFieldMaxPos;	///< max position within field (spec part)

public:
	/// ctor
	CSphExtendedQueryNode ()
		: m_pParent ( NULL )
		, m_eOp ( SPH_QUERY_AND )
		, m_bFieldSpec ( false )
	{}

	/// dtor
	~CSphExtendedQueryNode ()
	{
		ARRAY_FOREACH ( i, m_dChildren )
			SafeDelete ( m_dChildren[i] );
	}

	/// check if i'm empty
	bool IsEmpty () const
	{
		assert ( m_tAtom.IsEmpty() || m_dChildren.GetLength()==0 );
		return m_tAtom.IsEmpty() && ( m_dChildren.GetLength()==0 );
	}

	/// check if i'm plain
	bool IsPlain () const
	{
		return m_dChildren.GetLength()==0;
	}

	/// setup field limits
	void SetFieldSpec ( DWORD uMask, int iMaxPos );
};


/// extended query
struct CSphExtendedQuery : public ISphNoncopyable
{
	CSphString					m_sParseError;
	CSphString					m_sParseWarning;
	CSphExtendedQueryNode *		m_pRoot;

	/// ctor
	CSphExtendedQuery ()
	{
		m_pRoot = new CSphExtendedQueryNode ();
	}

	/// dtor
	~CSphExtendedQuery ()
	{
		SafeDelete ( m_pRoot );
	}
};

//////////////////////////////////////////////////////////////////////////////

/// parses the query and returns the resulting tree
/// return false and fills tQuery.m_sParseError on error
/// WARNING, parsed tree might be NULL (eg. if query was empty)
bool	sphParseExtendedQuery ( CSphExtendedQuery & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

#endif // _sphinxquery_

//
// $Id$
//
