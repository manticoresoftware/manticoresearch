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

#ifndef _searchnode_
#define _searchnode_

#include "sphinxquery.h"


enum
{
	MAX_BLOCK_DOCS = 32
};

/// query debugging printouts
#define QDEBUG 0

struct TermPos_t
{
	WORD m_uQueryPos;
	WORD m_uAtomPos;
};

/// word in the query
struct ExtQword_t
{
	CSphString	m_sWord;		///< word
	CSphString	m_sDictWord;	///< word as processed by dict
	int			m_iDocs;		///< matching documents
	int			m_iHits;		///< matching hits
	float		m_fIDF;			///< IDF value
	float		m_fBoost;		///< IDF multiplier
	int			m_iQueryPos;	///< position in the query
	bool		m_bExpanded;	///< added by prefix expansion
	bool		m_bExcluded;	///< excluded by the query (eg. bb in (aa AND NOT bb))
};


struct QwordsHash_fn
{
	static inline int Hash ( const CSphString & sKey )
	{
		return sphCRC32 ( sKey.cstr() );
	}
};


/// query words set
using ExtQwordsHash_t = CSphOrderedHash<ExtQword_t, CSphString, QwordsHash_fn, 256>;

struct ExtHit_t;
struct ExtDoc_t;

/// generic match streamer
class ExtNode_i
{
public:
	virtual						~ExtNode_i () {}

	static ExtNode_i *			Create ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 );
	static ExtNode_i *			Create ( const XQKeyword_t & tWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 );
	static ExtNode_i *			Create ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 );
	static ExtNode_i *			Create ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup, DictRefPtr_c pZonesDict, bool bUseBM25 );

	virtual void				Reset ( const ISphQwordSetup & tSetup ) = 0;
	virtual void				HintRowID ( RowID_t tRowID ) = 0;
	virtual const ExtDoc_t *	GetDocsChunk() = 0;
	virtual const ExtHit_t *	GetHits ( const ExtDoc_t * pDocs ) = 0;

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords ) = 0;
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) = 0;
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const = 0;
	virtual bool				GotHitless () = 0;
	virtual int					GetDocsCount () { return INT_MAX; }
	virtual int					GetHitsCount () { return 0; }
	virtual uint64_t			GetWordID () const = 0;			///< for now, only used for duplicate keyword checks in quorum operator
	virtual void 				SetAtomPos ( int iPos ) = 0;
	virtual int					GetAtomPos() const = 0;
	virtual void				SetCollectHits() {}				// call this if ranker needs hits
	virtual NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const = 0;

	virtual void				DebugDump ( int iLevel ) = 0;
};

struct RowIdBoundaries_t;
class RowidIterator_i;
std::unique_ptr<ExtNode_i> CreateRowIdFilterNode ( ExtNode_i * pNode, const RowIdBoundaries_t & tBoundaries, bool bClearOnReset );
std::unique_ptr<ExtNode_i> CreatePseudoFTNode ( ExtNode_i * pNode, RowidIterator_i * pIterator, bool bClearOnReset );

class NodeCacheContainer_c;

/// intra-batch node cache
class CSphQueryNodeCache
{
	friend class NodeCacheContainer_c;

public:
									CSphQueryNodeCache ( int iCells, int MaxCachedDocs, int MaxCachedHits );
									~CSphQueryNodeCache();

	ExtNode_i *						CreateProxy ( ExtNode_i * pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup );

protected:
	NodeCacheContainer_c *			m_pPool {nullptr};
	int								m_iMaxCachedDocs {0};
	int								m_iMaxCachedHits {0};
};

#endif // _searchnode_
