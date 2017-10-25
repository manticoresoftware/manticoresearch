//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxsearch_
#define _sphinxsearch_

#include "sphinx.h"
#include "sphinxquery.h"
#include "sphinxint.h"

//////////////////////////////////////////////////////////////////////////

/// term modifiers
enum TermPosFilter_e
{
	TERM_POS_NONE = 0,
	TERM_POS_FIELD_LIMIT = 1,
	TERM_POS_FIELD_START = 2,
	TERM_POS_FIELD_END = 3,
	TERM_POS_FIELD_STARTEND = 4,
	TERM_POS_ZONES = 5,
};


/// decoder state saved at a certain offset
struct SkiplistEntry_t
{
	SphDocID_t		m_iBaseDocid;		///< delta decoder docid base (aka docid infinum)
	int64_t			m_iOffset;			///< offset in the doclist file (relative to the doclist start)
	int64_t			m_iBaseHitlistPos;	///< delta decoder hitlist offset base
};


/// term, searcher view
class ISphQword
{
public:
	// setup by query parser
	CSphString		m_sWord;		///< my copy of word
	CSphString		m_sDictWord;	///< word after being processed by dict (eg. stemmed)
	SphWordID_t		m_uWordID;		///< word ID, from dictionary
	TermPosFilter_e	m_iTermPos;
	int				m_iAtomPos;		///< word position, from query
	float			m_fBoost;		///< IDF keyword boost (multiplier)
	bool			m_bExpanded;	///< added by prefix expansion
	bool			m_bExcluded;	///< excluded by the query (rval to operator NOT)

	// setup by QwordSetup()
	int				m_iDocs;		///< document count, from wordlist
	int				m_iHits;		///< hit count, from wordlist
	bool			m_bHasHitlist;	///< hitlist presence flag
	CSphVector<SkiplistEntry_t>		m_dSkiplist;	///< skiplist for quicker document list seeks

	// iterator state
	FieldMask_t m_dQwordFields;	///< current match fields
	DWORD			m_uMatchHits;	///< current match hits count
	SphOffset_t		m_iHitlistPos;	///< current position in hitlist, from doclist

protected:
	bool			m_bAllFieldsKnown; ///< whether the all match fields is known, or only low 32.

public:
	ISphQword ()
		: m_uWordID ( 0 )
		, m_iTermPos ( TERM_POS_NONE )
		, m_iAtomPos ( 0 )
		, m_fBoost ( 1.0f )
		, m_bExpanded ( false )
		, m_bExcluded ( false )
		, m_iDocs ( 0 )
		, m_iHits ( 0 )
		, m_bHasHitlist ( true )
		, m_uMatchHits ( 0 )
		, m_iHitlistPos ( 0 )
		, m_bAllFieldsKnown ( false )
	{
		m_dQwordFields.UnsetAll();
	}
	virtual ~ISphQword () {}

	virtual void				HintDocid ( SphDocID_t ) {}
	virtual const CSphMatch &	GetNextDoc ( DWORD * pInlineDocinfo ) = 0;
	virtual void				SeekHitlist ( SphOffset_t uOff ) = 0;
	virtual Hitpos_t			GetNextHit () = 0;
	virtual void				CollectHitMask ();

	virtual void Reset ()
	{
		m_iDocs = 0;
		m_iHits = 0;
		m_dQwordFields.UnsetAll();
		m_bAllFieldsKnown = false;
		m_uMatchHits = 0;
		m_iHitlistPos = 0;
	}
};


/// term setup, searcher view
class CSphQueryNodeCache;
class ISphZoneCheck;
struct CSphQueryStats;
class ISphQwordSetup : ISphNoncopyable
{
public:
	CSphDict *				m_pDict;
	const CSphIndex *		m_pIndex;
	ESphDocinfo				m_eDocinfo;
	const CSphRowitem *		m_pMinRow;
	SphDocID_t				m_uMinDocid;
	int						m_iInlineRowitems;		///< inline rowitems count
	int						m_iDynamicRowitems;		///< dynamic rowitems counts (including (!) inline)
	int64_t					m_iMaxTimer;
	CSphString *			m_pWarning;
	CSphQueryContext *		m_pCtx;
	CSphQueryNodeCache *	m_pNodeCache;
	mutable ISphZoneCheck *	m_pZoneChecker;
	CSphQueryStats *		m_pStats;
	mutable bool			m_bSetQposMask;

	ISphQwordSetup ()
		: m_pDict ( NULL )
		, m_pIndex ( NULL )
		, m_eDocinfo ( SPH_DOCINFO_NONE )
		, m_pMinRow ( NULL )
		, m_uMinDocid ( 0 )
		, m_iInlineRowitems ( 0 )
		, m_iDynamicRowitems ( 0 )
		, m_iMaxTimer ( 0 )
		, m_pWarning ( NULL )
		, m_pCtx ( NULL )
		, m_pNodeCache ( NULL )
		, m_pZoneChecker ( NULL )
		, m_pStats ( NULL )
		, m_bSetQposMask ( false )
	{}
	virtual ~ISphQwordSetup () {}

	virtual ISphQword *					QwordSpawn ( const XQKeyword_t & tWord ) const = 0;
	virtual bool						QwordSetup ( ISphQword * pQword ) const = 0;
};

/// generic ranker interface
class ISphRanker : public ISphExtra
{
public:
	virtual						~ISphRanker () {}
	virtual CSphMatch *			GetMatchesBuffer() = 0;
	virtual int					GetMatches () = 0;
	virtual void				Reset ( const ISphQwordSetup & tSetup ) = 0;
	virtual bool				IsCache() const { return false; }
	virtual void				FinalizeCache ( const ISphSchema & ) {}
};

/// factory
ISphRanker * sphCreateRanker ( const XQQuery_t & tXQ, const CSphQuery * pQuery, CSphQueryResult * pResult, const ISphQwordSetup & tTermSetup, const CSphQueryContext & tCtx, const ISphSchema & tSorterSchema );

//////////////////////////////////////////////////////////////////////////

/// hit mark, used for snippets generation
struct SphHitMark_t
{
	DWORD	m_uPosition;
	DWORD	m_uSpan;

	bool operator == ( const SphHitMark_t & rhs ) const
	{
		return m_uPosition==rhs.m_uPosition && m_uSpan==rhs.m_uSpan;
	}
};

/// hit marker, used for snippets generation
class CSphHitMarker
{
public:
	class ExtNode_i *		m_pRoot;

public:
							CSphHitMarker() : m_pRoot ( NULL ) {}
							~CSphHitMarker();

	void					Mark ( CSphVector<SphHitMark_t> & );
	static CSphHitMarker *	Create ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup );
};

//////////////////////////////////////////////////////////////////////////

/// intra-batch node cache
class CSphQueryNodeCache
{
	friend class NodeCacheContainer_t;

protected:
	class NodeCacheContainer_t *	m_pPool;
	int								m_iMaxCachedDocs;
	int								m_iMaxCachedHits;

public:
									CSphQueryNodeCache ( int iCells, int MaxCachedDocs, int MaxCachedHits );
									~CSphQueryNodeCache ();

	ExtNode_i *						CreateProxy ( ExtNode_i * pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup );
};

//////////////////////////////////////////////////////////////////////////

CSphString sphXQNodeToStr ( const XQNode_t * pNode );
CSphString sphXQNodeGetExtraStr ( const XQNode_t * pNode );
CSphString sphExplainQuery ( const XQNode_t * pNode, const CSphSchema & tSchema, const CSphVector<CSphString> & dZones );
CSphString sphExplainQueryBrief ( const XQNode_t * pNode, const CSphSchema & tSchema );

#endif // _sphinxsearch_

//
// $Id$
//
