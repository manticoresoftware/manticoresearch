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
struct XQKeyword_t
{
	CSphString	m_sWord;
	int			m_iAtomPos;
	bool		m_bFieldStart;	///< must occur at very start
	bool		m_bFieldEnd;	///< must occur at very end

	XQKeyword_t () : m_iAtomPos ( -1 ), m_bFieldStart ( false ), m_bFieldEnd ( false ) {}
	XQKeyword_t ( const char * sWord, int iPos ) : m_sWord ( sWord ), m_iAtomPos ( iPos ), m_bFieldStart ( false ), m_bFieldEnd ( false ) {}
};


/// extended query operator
enum XQOperator_e
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
struct XQNode_t : public ISphNoncopyable
{
	XQNode_t *				m_pParent;		///< my parent node (NULL for root ones)
	XQOperator_e			m_eOp;			///< operation over childen
	CSphVector<XQNode_t*>	m_dChildren;	///< non-plain node children

	bool					m_bFieldSpec;	///< whether field spec was already explicitly set
	DWORD					m_uFieldMask;	///< fields mask (spec part)
	int						m_iFieldMaxPos;	///< max position within field (spec part)

	CSphVector<XQKeyword_t>	m_dWords;		///< query words (plain node)
	int						m_iMaxDistance;	///< proximity distance or quorum length; 0 or less if not proximity/quorum node
	bool					m_bQuorum;		///< quorum node flag

public:
	/// ctor
	XQNode_t ()
		: m_pParent ( NULL )
		, m_eOp ( SPH_QUERY_AND )
		, m_bFieldSpec ( false )
		, m_uFieldMask ( 0xFFFFFFFFUL )
		, m_iFieldMaxPos ( 0 )
		, m_iMaxDistance ( -1 )
		, m_bQuorum ( false )
	{}

	/// dtor
	~XQNode_t ()
	{
		ARRAY_FOREACH ( i, m_dChildren )
			SafeDelete ( m_dChildren[i] );
	}

	/// check if i'm empty
	bool IsEmpty () const
	{
		assert ( m_dWords.GetLength()==0 || m_dChildren.GetLength()==0 );
		return m_dWords.GetLength()==0 && m_dChildren.GetLength()==0;
	}

	/// check if i'm plain
	bool IsPlain () const
	{
		assert ( m_dWords.GetLength()==0 || m_dChildren.GetLength()==0 );
		return m_dChildren.GetLength()==0;
	}

	/// setup field limits
	void SetFieldSpec ( DWORD uMask, int iMaxPos );
};


/// extended query
struct XQQuery_t : public ISphNoncopyable
{
	CSphString	m_sParseError;
	CSphString	m_sParseWarning;
	XQNode_t *	m_pRoot;

	/// ctor
	XQQuery_t ()
	{
		m_pRoot = new XQNode_t ();
	}

	/// dtor
	~XQQuery_t ()
	{
		SafeDelete ( m_pRoot );
	}
};

//////////////////////////////////////////////////////////////////////////////

/// parses the query and returns the resulting tree
/// return false and fills tQuery.m_sParseError on error
/// WARNING, parsed tree might be NULL (eg. if query was empty)
bool	sphParseExtendedQuery ( XQQuery_t & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

#endif // _sphinxquery_

//
// $Id$
//
