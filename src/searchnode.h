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

#include "sphinxquery/sphinxquery.h"
#include "postingwindow.h"


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
struct RowIdBoundaries_t;

// A transient, aligned rowid window. Only count-only roots expose this capability.
// A single contributing clause may return sorted, unique row batches instead of
// constructing a bitmap. Several batches may share a base; a base never mixes
// row batches and bitmaps. Storage is owned by the producer until NextWindow or
// Reset; hints, bounds changes and CheckContinue must preserve returned storage.
struct CountWindow_t
{
	static constexpr int ROWS = PostingWindow_t::ROWS;
	static constexpr int WORDS = ROWS/32;

	RowID_t			m_tBase = 0;
	DWORD			m_dBits[WORDS] = {};
	const RowID_t *	m_pRows = nullptr; // non-null: use rows; bitmap is undefined
	int				m_iRows = 0;
};

class CountWindowIterator_i
{
public:
	virtual ~CountWindowIterator_i() = default;
	// Mutable until the next call; null means EOF (or interruption).
	virtual CountWindow_t * NextWindow() = 0;
	// Also checked while an adapter drains an already produced window.
	virtual bool CheckContinue() = 0;
	virtual const char * GetName() const { return "WindowOrCount"; }
};

class ExtNode_i;

// Lifecycle and metadata shared by document and window producers.
class QueryNode_i
{
public:
	virtual						~QueryNode_i() = default;

	// Capabilities describe this node, never an unfiltered descendant.
	virtual ExtNode_i *			GetDocsNode() { return nullptr; }
	virtual CountWindowIterator_i * GetCountWindows() { return nullptr; }
	virtual void				Reset ( const ISphQwordSetup & tSetup ) = 0;
	virtual void				HintRowID ( RowID_t tRowID ) = 0;

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords ) = 0;
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) = 0;
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const = 0;
	virtual bool				GotHitless () = 0;
	virtual int					GetDocsCount() const { return INT_MAX; }
	virtual int					GetHitsCount() const { return 0; }
	virtual uint64_t			GetWordID () const = 0;			///< for now, only used for duplicate keyword checks in quorum operator
	virtual void 				SetAtomPos ( int iPos ) = 0;
	virtual int					GetAtomPos() const = 0;
	virtual NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const = 0;
	virtual void				SetRowidBoundaries ( const RowIdBoundaries_t & tBoundaries ) = 0;

	virtual void				DebugDump ( int iLevel ) = 0;
	virtual bool				TimeExceeded() const = 0;
	virtual int64_t				GetMaxTimeout() const = 0;
};

// Existing document/hit producers retain their document interface.
class ExtNode_i : public QueryNode_i
{
public:
	static ExtNode_i *			Create ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, const RowIdBoundaries_t * pBoundaries );
	static ExtNode_i *			Create ( const XQKeyword_t & tWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, bool bRowidLimits );
	static ExtNode_i *			Create ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25, bool bRowidLimits );
	static ExtNode_i *			Create ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup, DictRefPtr_c pZonesDict, bool bUseBM25, bool bRowidLimits );

	ExtNode_i *					GetDocsNode() final { return this; }

	virtual const ExtDoc_t *	GetDocsChunk() = 0;
	virtual const ExtHit_t *	GetHits ( const ExtDoc_t * pDocs ) = 0;
	virtual void				SetCollectHits() {}
};

// Count/membership-only producer; no document or hit iteration interface.
class WindowNode_i : public QueryNode_i, public CountWindowIterator_i
{
public:
	CountWindowIterator_i *		GetCountWindows() final { return this; }
};

class WindowDocsAdapter_i : public ExtNode_i
{
public:
	// Recover the source after Reset and detaching the adapter from its consumer.
	// Does not rewind a partially consumed window; invalidates adapter buffers.
	virtual std::unique_ptr<QueryNode_i> ReleaseSource() = 0;
};

// Caller must guarantee disk qwords and a membership-only consumer (ranker=none).
WindowNode_i * CreateWindowOrCountNode ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, const RowIdBoundaries_t * pBounds );
WindowNode_i * CreateWindowAndCountNode ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, const RowIdBoundaries_t * pBounds );

// Takes exclusive ownership of a window-only source; exposes no window capability.
std::unique_ptr<WindowDocsAdapter_i> CreateWindowDocsAdapter ( std::unique_ptr<QueryNode_i> pSource );

class RowidIterator_i;
std::unique_ptr<ExtNode_i> CreatePseudoFTNode ( ExtNode_i * pNode, RowidIterator_i * pIterator );

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
