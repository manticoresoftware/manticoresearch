//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchnode.h"
#include "sphinxquery.h"
#include "sphinxint.h"
#include "sphinxplugin.h"
#include "sphinxqcache.h"
#include "attribute.h"
#include "mini_timer.h"
#include "coroutine.h"
#include "secondaryindex.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////////

#if QDEBUG
#define QDEBUGARG(_arg) _arg
#else
#define QDEBUGARG(_arg)
#endif

//////////////////////////////////////////////////////////////////////////

/// costs for max_predicted_time estimations, in nanoseconds
/// YMMV, defaults were estimated in a very specific environment, and then rounded off
int g_iPredictorCostDoc		= 64;
int g_iPredictorCostHit		= 48;
int g_iPredictorCostSkip	= 2048;
int g_iPredictorCostMatch	= 64;

//////////////////////////////////////////////////////////////////////////
// EXTENDED MATCHING V2
//////////////////////////////////////////////////////////////////////////

#define SPH_BM25_K1				1.2f

static const float COST_SCALE = 1.0f/1000000.0f;

static volatile bool g_bInterruptNow = false;


static void PrintDocsChunk ( int QDEBUGARG(iCount), int QDEBUGARG(iAtomPos), const ExtDoc_t * QDEBUGARG(pDocs), const char * QDEBUGARG(sNode), void * QDEBUGARG(pNode), const char * sTerm=nullptr )
{
#if QDEBUG
	StringBuilder_c tRes;
	tRes.Appendf ( "node %s 0x%x:%p getdocs (%d)(%s) = ", sNode ? sNode : "???", iAtomPos, pNode, iCount, ( sTerm ? sTerm : "" ) );
	tRes.StartBlock (", ","[","]\n");
	for ( int i=0; i<iCount; ++i )
		tRes.Appendf ( "0x%x", DWORD ( pDocs[i].m_tRowID ) );
	tRes.FinishBlock ( false );
	printf ( "%s", tRes.cstr() );
#endif

}

static void PrintHitsChunk ( int QDEBUGARG(iCount), int QDEBUGARG(iAtomPos), const ExtHit_t * QDEBUGARG(pHits), void * QDEBUGARG(pNode) )
{
#if QDEBUG
	StringBuilder_c tRes;
	tRes.Appendf ( "node 0x%x:%p gethits (%d) = ", iAtomPos, pNode, iCount );
	tRes.StartBlock ( ", ", "[", "]\n" );
	for ( int i=0; i<iCount; ++i )
		tRes.Appendf ( "0x%x:0x%x", DWORD ( pHits[i].m_tRowID ), DWORD ( pHits[i].m_uHitpos ) );
	tRes.FinishBlock ( false );
	printf ( "%s\n", tRes.cstr() );
#endif

}


static void DebugIndent ( int iLevel )
{
	while ( iLevel-- )
		printf ( "    " );
}


static inline bool HasDocs ( const ExtDoc_t * pDoc )
{
	return pDoc && pDoc->m_tRowID!=INVALID_ROWID;
}


static inline bool HasHits ( const ExtHit_t * pHit )
{
	assert ( pHit );
	return pHit->m_tRowID!=INVALID_ROWID;
}


static inline bool WarmupDocs ( const ExtDoc_t * & pDoc, ExtNode_i * pNode )
{
	assert(pNode);

	if ( HasDocs(pDoc) )
		return true;

	pDoc = pNode->GetDocsChunk();
	return HasDocs(pDoc);
}


static inline bool WarmupDocs ( const ExtDoc_t * & pDoc, const ExtHit_t * & pHit, ExtNode_i * pNode )
{
	assert(pNode);

	if ( HasDocs(pDoc) )
		return true;

	pDoc = pNode->GetDocsChunk();
	if ( !HasDocs(pDoc) )
		return false;

	pHit = pNode->GetHits(pDoc);
	return true;
}


static inline bool WarmupDocs ( const ExtDoc_t * & pDocL, const ExtDoc_t * pDocR, ExtNode_i * pLeft )
{
	assert(pLeft);

	if ( HasDocs(pDocL) )
		return true;

	if ( HasDocs(pDocR)  )
		pLeft->HintRowID ( pDocR->m_tRowID );

	pDocL = pLeft->GetDocsChunk();
	return HasDocs(pDocL);
}

//////////////////////////////////////////////////////////////////////////

class ExtNode_c : public ExtNode_i
{
public:
	static const int		MAX_HITS = 512;

							ExtNode_c();

	const ExtHit_t *		GetHits ( const ExtDoc_t * pDocs ) override;
	void					DebugDump ( int iLevel ) override;
	void 					SetAtomPos ( int iPos ) override;
	int						GetAtomPos() const override;
	void					SetQPosReverse();

protected:
	ExtDoc_t				m_dDocs[MAX_BLOCK_DOCS];
	CSphVector<ExtHit_t>	m_dHits;
	bool					m_bQPosReverse {false};
	int						m_iAtomPos {0};		///< we now need it on this level for tricks like expanded keywords within phrases
	const int64_t&			m_iCheckTimePoint { Threads::Coro::GetNextTimePointUS() };

	virtual void			CollectHits ( const ExtDoc_t * pDocs ) = 0;

	inline const ExtDoc_t *	ReturnDocsChunk ( int iCount, const char * sNode, const char * sTerm=nullptr );
	inline const ExtHit_t *	ReturnHitsChunk ( int iCount, const char * sNode, bool bReverse );
	inline const ExtHit_t *	ReturnHits ( bool bReverse );
};

//////////////////////////////////////////////////////////////////////////

class ExtRowIdRange_c : public ExtNode_c
{
public:
						ExtRowIdRange_c ( ExtNode_i * pNode, const RowIdBoundaries_t & tBoundaries, bool bClearOnReset );
						~ExtRowIdRange_c() = default;

	const ExtDoc_t *	GetDocsChunk() override;
	const ExtHit_t *	GetHits ( const ExtDoc_t * pDocs ) final		{ return m_pNode->GetHits(pDocs); }
	void				Reset ( const ISphQwordSetup & tSetup ) final;
	void				HintRowID ( RowID_t tRowID ) final				{ m_pNode->HintRowID(tRowID); }
	int					GetQwords ( ExtQwordsHash_t & hQwords ) final	{ return m_pNode->GetQwords(hQwords); }
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) final { m_pNode->SetQwordsIDF(hQwords); }
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const final { m_pNode->GetTerms ( hQwords, dTermDupes ); }
	bool				GotHitless() final								{ return m_pNode->GotHitless(); }
	uint64_t			GetWordID() const final							{ return m_pNode->GetWordID(); }
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const final		{ return m_pNode->Estimate(iTotalDocs); }

protected:
	void				CollectHits ( const ExtDoc_t * pDocs ) final	{ assert ( 0 && "ExtRowIdRange_c doesn't collect hits" ); }

private:
	std::unique_ptr<ExtNode_i>	m_pNode;
	RowIdBoundaries_t			m_tBoundaries;
	bool						m_bClearOnReset = false;

	inline const ExtDoc_t * FilterHead ( const ExtDoc_t * pDocStart );
	inline const ExtDoc_t * FilterTail ( const ExtDoc_t * pDocStart );
};


ExtRowIdRange_c::ExtRowIdRange_c ( ExtNode_i * pNode, const RowIdBoundaries_t & tBoundaries, bool bClearOnReset )
	: m_pNode ( pNode )
	, m_tBoundaries ( tBoundaries )
	, m_bClearOnReset ( bClearOnReset )
{
	assert ( m_pNode );
	pNode->HintRowID ( m_tBoundaries.m_tMinRowID );
}


const ExtDoc_t * ExtRowIdRange_c::GetDocsChunk()
{
	const ExtDoc_t * pDocs = m_pNode->GetDocsChunk();
	if ( !pDocs )
		return nullptr;

	if (pDocs->m_tRowID<m_tBoundaries.m_tMinRowID )
		return FilterHead(pDocs);

	return FilterTail(pDocs);
}


void ExtRowIdRange_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pNode->Reset(tSetup);
	if ( m_bClearOnReset )
		m_pNode.release();
}


const ExtDoc_t * ExtRowIdRange_c::FilterHead ( const ExtDoc_t * pDocStart )
{
	const ExtDoc_t * pDoc = pDocStart;

	// filter the whole chunk
	while (true)
	{
		while ( pDoc->m_tRowID<m_tBoundaries.m_tMinRowID && pDoc->m_tRowID!=INVALID_ROWID )
			pDoc++;

		if ( pDoc->m_tRowID!=INVALID_ROWID )
			break;

		pDoc = m_pNode->GetDocsChunk();
		if ( !pDoc )
			return nullptr;
	}

	ExtDoc_t * pDstDoc = &m_dDocs[0];
	while ( pDoc->m_tRowID<=m_tBoundaries.m_tMaxRowID && pDoc->m_tRowID!=INVALID_ROWID )
		*pDstDoc++ = *pDoc++;

	pDstDoc->m_tRowID = INVALID_ROWID;
	return m_dDocs;
}


const ExtDoc_t * ExtRowIdRange_c::FilterTail ( const ExtDoc_t * pDocStart )
{
	const ExtDoc_t * pDoc = pDocStart;

	// cut the tail
	while ( pDoc->m_tRowID!=INVALID_ROWID )
	{
		if ( pDoc->m_tRowID>m_tBoundaries.m_tMaxRowID )
		{
			const_cast<ExtDoc_t *>(pDoc)->m_tRowID = INVALID_ROWID;	// hack to avoid copying the doclist
			break;
		}

		pDoc++;
	}

	// signal the end if we stopped at the first doc
	return pDoc==pDocStart ? nullptr : pDocStart;
}

//////////////////////////////////////////////////////////////////////////

// outputs docids returned by rowid iterators
class ExtIterator_c : public ExtNode_c
{
public:
						ExtIterator_c ( RowidIterator_i * pIterator ) : m_pIterator ( pIterator ) { assert ( pIterator ); }

	int					GetQwords ( ExtQwordsHash_t & hQwords )	override { return -1; }
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override {}
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override {}
	uint64_t			GetWordID () const override { return 0; }
	const ExtDoc_t *	GetDocsChunk() override;
	const ExtHit_t *	GetHits ( const ExtDoc_t * pDocs ) final { return nullptr; }
	void				Reset ( const ISphQwordSetup & tSetup ) override {}
	void				HintRowID ( RowID_t tRowID ) override { m_pIterator->HintRowID(tRowID); }
	bool				GotHitless() override { return false; }
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { return { 0.0f, 0, 0 }; }

protected:
	RowidIterator_i *	m_pIterator = nullptr;	// not owned by the node
	bool				m_bWarmup = true;
	RowIdBlock_t		m_dIteratorRowIDs;
	int					m_iDocOffset = 0;

	void				CollectHits ( const ExtDoc_t * pDocs ) final	{ assert ( 0 && "ExtRowIdRange_c doesn't collect hits" ); }
};


const ExtDoc_t * ExtIterator_c::GetDocsChunk()
{
	if ( m_bWarmup )
	{
		if ( !m_pIterator->GetNextRowIdBlock(m_dIteratorRowIDs) )
			return nullptr;

		m_iDocOffset = 0;
		m_bWarmup = false;
	}

	RowID_t * pStart = m_dIteratorRowIDs.Begin()+m_iDocOffset;
	RowID_t * pEnd = pStart + Min ( MAX_BLOCK_DOCS-1, m_dIteratorRowIDs.GetLength()-m_iDocOffset );
	ExtDoc_t * pDocStart = m_dDocs;
	while ( pStart < pEnd )
		*pDocStart++ = { *pStart++, 0, 0.0f };

	if ( pEnd==m_dIteratorRowIDs.End() )
		m_bWarmup = true;
	else
		m_iDocOffset = pEnd-m_dIteratorRowIDs.Begin();

	return ReturnDocsChunk ( pDocStart-m_dDocs, "filter" );
}

//////////////////////////////////////////////////////////////////////////

/// single keyword streamer
template<bool USE_BM25>
class ExtTerm_T : public ExtNode_c, ISphNoncopyable
{
public:
						ExtTerm_T ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted );
						ExtTerm_T ( ISphQword * pQword, const ISphQwordSetup & tSetup );
						ExtTerm_T ();
						~ExtTerm_T () override;

	void				Init ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted );
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pMatched ) override;

	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override { return false; }
	int					GetDocsCount () override { return m_pQword->m_iDocs; }
	int					GetHitsCount () override{ return m_pQword->m_iHits; }
	uint64_t			GetWordID () const override;
	void				HintRowID ( RowID_t tRowID ) override;
	void				SetCollectHits() override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;

	void				DebugDump ( int iLevel ) override;

protected:
	struct StoredHit_t
	{
		SphOffset_t	m_tHitlistOffset;
		RowID_t		m_tRowID;
	};

	ISphQword *			m_pQword = nullptr;
	FieldMask_t			m_dQueriedFields;	///< accepted fields mask
	bool				m_bHasWideFields = false;	///< whether fields mask for this term refer to fields 32+
	float				m_fIDF = 0.0f;		///< IDF for this term (might be 0.0f for non-1st occurences in query)
	int64_t				m_iMaxTimer = 0;	///< work until this timestamp
	CSphString *		m_pWarning = nullptr;
	bool				m_bNotWeighted = true;
	CSphQueryStats *	m_pStats = nullptr;
	int64_t *			m_pNanoBudget = nullptr;
	bool				m_bCollectHits = false;

	CSphVector<StoredHit_t> m_dStoredHits;
};


/// single keyword streamer with artificial hitlist
template<bool USE_BM25>
class ExtTermHitless_T : public ExtTerm_T<USE_BM25>
{
public:
						ExtTermHitless_T ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted );

	void				CollectHits ( const ExtDoc_t * pMatched ) final;
	bool				GotHitless () final { return true; }
};

//////////////////////////////////////////////////////////////////////////

/// position filter policy
template < TermPosFilter_e T >
class TermAcceptor_T
{
public:
								TermAcceptor_T ( ISphQword *, const XQNode_t *, const ISphQwordSetup & ) {}
protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const { return true; }
	inline void					Reset() {}
};

template<>
class TermAcceptor_T<TERM_POS_FIELD_LIMIT> : public ISphNoncopyable
{
public:
								TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & );

protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const;
	inline void					Reset() {}

private:
	const int					m_iMaxFieldPos;
};

template<>
class TermAcceptor_T<TERM_POS_ZONES> : public ISphNoncopyable
{
public:
								TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & tSetup );

protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * pHit ) const;
	inline void					Reset();

	ISphZoneCheck *				m_pZoneChecker;				///< zone-limited searches query ranker about zones
	mutable CSphVector<int>		m_dZones;					///< zone ids for this particular term
	mutable RowID_t				m_tLastZoneRowID {INVALID_ROWID};
	mutable int					m_iCheckFrom {0};
};

//////////////////////////////////////////////////////////////////////////

class BufferedNode_c
{
protected:
	const ExtDoc_t *	m_pRawDocs = nullptr;				///< chunk start as returned by raw GetDocsChunk() (need to store it for raw CollectHits() calls)
	const ExtDoc_t *	m_pRawDoc = nullptr;				///< current position in raw docs chunk
	const ExtHit_t *	m_pRawHit = nullptr;				///< current position in raw hits chunk
	ExtDoc_t			m_dMyDocs[MAX_BLOCK_DOCS];			///< all documents within the required pos range
	CSphVector<ExtHit_t> m_dMyHits;							///< all hits within the required pos range


						BufferedNode_c();

	void				Reset();
	void				CopyMatchingHits ( CSphVector<ExtHit_t> & dHits, const ExtDoc_t * pDocs );
};


/// single keyword streamer, with term position filtering
template < TermPosFilter_e T, class NODE >
class ExtConditional_T : public ExtNode_c, public BufferedNode_c, protected TermAcceptor_T<T>
{
	using Acceptor_c = TermAcceptor_T<T>;

public:
	void				Reset ( const ISphQwordSetup & tSetup ) final;
	const ExtDoc_t *	GetDocsChunk() final;
	void				CollectHits ( const ExtDoc_t * pDocs ) final;
	void				HintRowID ( RowID_t tRowID ) override;
	bool				GotHitless () final { return false; }
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID() const override;
	void 				SetAtomPos ( int iPos ) override;
	int					GetAtomPos() const override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { return m_tNode.Estimate(iTotalDocs); }

protected:
	NODE				m_tNode;

						ExtConditional_T ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup );
};

/// single keyword streamer, with term position filtering
template < TermPosFilter_e T, bool USE_BM25 >
class ExtTermPos_T : public ExtConditional_T<T,ExtTerm_T<USE_BM25>>
{
public:
	ExtTermPos_T( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
		: ExtConditional_T<T,ExtTerm_T<USE_BM25>> ( pQword, pNode, tSetup )
	{
		this->m_tNode.Init ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
};


/// multi-node binary-operation streamer traits
class ExtTwofer_c : public ExtNode_c
{
public:
						ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond );
						ExtTwofer_c () {} ///< to be used in pair with Init();

	void				Init ( ExtNode_i * pLeft, ExtNode_i * pRight );
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override;
	void				HintRowID ( RowID_t tRowID ) override;
	uint64_t			GetWordID() const override;
	void				SetCollectHits() override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;

	void				SetNodePos ( WORD uPosLeft, WORD uPosRight );

protected:
	std::unique_ptr<ExtNode_i> m_pLeft;
	std::unique_ptr<ExtNode_i> m_pRight;
	const ExtDoc_t *	m_pDocL = nullptr;
	const ExtDoc_t *	m_pDocR = nullptr;
	WORD				m_uNodePosL = 0;
	WORD				m_uNodePosR = 0;

	void				DebugDumpT ( const char * sName, int iLevel );
};

/// A-and-B streamer
class ExtAnd_c : public ExtTwofer_c
{
public:
						ExtAnd_c ( ExtNode_i * pLeft, ExtNode_i * pRight ) : ExtTwofer_c ( pLeft, pRight ) {}
						ExtAnd_c() {} ///< to be used with Init()

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
	void				DebugDump ( int iLevel ) override;
};

// AND that returns hits from right node only
class ExtAndRightHits_c : public ExtAnd_c
{
public:
						ExtAndRightHits_c ( ExtNode_i * pLeft, ExtNode_i * pRight, bool bClearOnReset );

	int					GetQwords ( ExtQwordsHash_t & hQwords ) override	{ assert(m_pRight); return m_pRight->GetQwords(hQwords); }
	const ExtHit_t *	GetHits ( const ExtDoc_t * pDocs ) override			{ assert(m_pRight); return m_pRight->GetHits(pDocs); }
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				DebugDump ( int iLevel ) override					{ DebugDumpT ( "ExtAndRightHits", iLevel ); }

private:
	bool	m_bClearOnReset = false;
};


ExtAndRightHits_c::ExtAndRightHits_c ( ExtNode_i * pLeft, ExtNode_i * pRight, bool bClearOnReset )
	: ExtAnd_c ( pLeft, pRight )
	, m_bClearOnReset ( bClearOnReset )
{}


void ExtAndRightHits_c::Reset ( const ISphQwordSetup & tSetup )
{
	assert(m_pRight);
	m_pRight->Reset(tSetup);
	if ( m_bClearOnReset )
		m_pRight.release();
}


template < bool USE_BM25, bool TEST_FIELDS >
class ExtMultiAnd_T : public ExtNode_c
{
public:
						ExtMultiAnd_T ( const VecTraits_T<XQNode_t*> & dXQNodes, const ISphQwordSetup & tSetup );
						~ExtMultiAnd_T() override;

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID () const override;
	bool				GotHitless () override { return false; }
	void				HintRowID ( RowID_t tRowID ) override;
	void				SetCollectHits() override;
	void				DebugDump ( int iLevel ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;

private:
	struct NodeInfo_t
	{
		ISphQword *		m_pQword {nullptr};
		RowID_t			m_tRowID {INVALID_ROWID};
		bool			m_bHitsOver {false};

		float			m_fIDF {0.0f};
		WORD			m_uNodepos {0};
		int				m_iAtomPos {0};
		FieldMask_t		m_dQueriedFields;
		bool			m_bHasWideFields {false};
		bool			m_bNotWeighted {true};

		void			UpdateWideFieldFlag ( const ISphQwordSetup & tSetup );
		bool			FitsFields() const;
	};

	struct StoredMultiHit_t
	{
		CSphFixedVector<SphOffset_t>	m_dHitlistOffsets;
		RowID_t							m_tRowID;

		// don't change to default ctr, centos6 will fail to compile!
		StoredMultiHit_t () : m_dHitlistOffsets ( 0 ) {}
	};

	struct HitInfo_t
	{
		ISphQword * m_pQword;
		WORD		m_uNodePos;
		WORD		m_uQueryPos;
		Hitpos_t	m_uHitpos;
	};

	struct SelectivitySorter_t
	{
		bool IsLess ( const NodeInfo_t & tA, const NodeInfo_t & tB ) const;
	};

	struct HitWithQpos_t
	{
		int			m_iNode;
		Hitpos_t	m_uHit;
		WORD		m_uQueryPos;

					HitWithQpos_t() = default;
					HitWithQpos_t ( int iNode, Hitpos_t uHit, WORD uQueryPos );

		static bool	IsLess ( const HitWithQpos_t & a, const HitWithQpos_t & b );
	};


	bool							m_bFirstChunk {true};
	bool							m_bCollectHits {false};
	CSphVector<NodeInfo_t>			m_dNodes;
	CSphVector<StoredMultiHit_t>	m_dStoredHits;
	int								m_iNodesSet {0};

	int64_t							m_iMaxTimer {0};
	CSphString *					m_pWarning {nullptr};
	CSphQueryStats *				m_pStats {nullptr};
	int64_t *						m_pNanoBudget {nullptr};

	CSphFixedVector<uint64_t>		m_dWordIds;
	CSphQueue<HitWithQpos_t, HitWithQpos_t > m_tQueue;

	bool				AdvanceQwords();
	RowID_t				Advance ( int iNode );
	RowID_t				Advance ( int iNode, RowID_t tRowID );
	bool				FitsFields ( const NodeInfo_t & tNode ) const;
	DWORD				GetDocFieldsMask() const;
	float				GetTFIDF() const;
	int					GetQword ( NodeInfo_t & tNode, ExtQwordsHash_t & hQwords );
	void				PushNextHit ( int iNode );
	void				MergeHitsN ( const StoredMultiHit_t & tStoredHit );
	void				MergeHits2 ( const StoredMultiHit_t & tStoredHit );
	void				MergeHits3 ( const StoredMultiHit_t & tStoredHit );

	void				InitHitMerge ( HitInfo_t & tHitInfo, int iNode, const StoredMultiHit_t & tStoredHit );
	void				DoHitMerge ( RowID_t tRowID, HitInfo_t & tLeft, HitInfo_t & tRight );
	void				DoHitMerge ( RowID_t tRowID, HitInfo_t & tHit1, HitInfo_t & tHit2, HitInfo_t & tHit3 );
	void				CopyHits ( RowID_t tRowID, HitInfo_t & tHitInfo, int iNode );
	void				AddHit ( RowID_t tRowID, HitInfo_t & tHit, int iNode );

	static bool			IsHitLess ( const HitInfo_t & tLeft, const HitInfo_t & tRight );
};


class ExtAndZonespanned_c : public ExtAnd_c
{
	friend class ExtAndZonespan_c;

public:
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				DebugDump ( int iLevel ) override;

protected:
	bool				IsSameZonespan ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 ) const;

	ISphZoneCheck *		m_pZoneChecker = nullptr;
	CSphVector<int>		m_dZones;
};


class ExtAndZonespan_c : public ExtConditional_T < TERM_POS_NONE, ExtAndZonespanned_c >
{
public:
	ExtAndZonespan_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup, const XQNode_t * pNode )
		: ExtConditional_T<TERM_POS_NONE,ExtAndZonespanned_c> ( NULL, pNode, tSetup )
	{
		m_tNode.Init ( pFirst, pSecond );
		m_tNode.m_pZoneChecker = tSetup.m_pZoneChecker;
		m_tNode.m_dZones = pNode->m_dSpec.m_dZones;
	}
};


/// A-or-B streamer
class ExtOr_c : public ExtTwofer_c
{
public:
						ExtOr_c ( ExtNode_i * pLeft, ExtNode_i * pRight );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				DebugDump ( int iLevel ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;
};


/// A-maybe-B streamer
class ExtMaybe_c : public ExtOr_c
{
public:
						ExtMaybe_c ( ExtNode_i * pLeft, ExtNode_i * pRight );

	const ExtDoc_t *	GetDocsChunk() override;
	void				DebugDump ( int iLevel ) override;
};


/// A-and-not-B streamer
class ExtAndNot_c : public ExtTwofer_c
{
public:
						ExtAndNot_c ( ExtNode_i * pLeft, ExtNode_i * pRight );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				SetCollectHits() override;
	void				DebugDump ( int iLevel ) override;

protected:
	bool				m_bPassthrough {false};
};


/// generic operator over N nodes
class ExtNWay_c : public ExtNode_c, public BufferedNode_c
{
public:
						ExtNWay_c ( const CSphVector<ExtNode_i *> & dNodes, const ISphQwordSetup & tSetup );
						~ExtNWay_c() override;

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override { return false; }
	void				HintRowID ( RowID_t tRowID ) override;
	uint64_t			GetWordID() const override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { assert(m_pNode); return m_pNode->Estimate(iTotalDocs); }

protected:
	ExtNode_i *			m_pNode	{nullptr};					///< my and-node for all the terms
	const ExtDoc_t *	m_pDocs	{nullptr};					///< current docs chunk from and-node
	const ExtHit_t *	m_pHits	{nullptr};					///< current hits chunk from and-node

	inline void			ConstructNode ( const CSphVector<ExtNode_i *> & dNodes, const CSphVector<WORD> & dPositions, const ISphQwordSetup & tSetup );
};


struct ExtNodeTF_fn
{
	bool IsLess ( ExtNode_i * pA, ExtNode_i * pB ) const
	{
		return pA->GetDocsCount() < pB->GetDocsCount();
	}
};


struct ExtNodeTFExt_fn
{
	const CSphVector<ExtNode_i *> & m_dNodes;

	explicit ExtNodeTFExt_fn ( const CSphVector<ExtNode_i *> & dNodes )
		: m_dNodes ( dNodes )
	{}

	ExtNodeTFExt_fn ( const ExtNodeTFExt_fn & rhs )
		: m_dNodes ( rhs.m_dNodes )
	{}

	bool IsLess ( WORD uA, WORD uB ) const
	{
		return m_dNodes[uA]->GetDocsCount() < m_dNodes[uB]->GetDocsCount();
	}

private:
	const ExtNodeTFExt_fn & operator = ( const ExtNodeTFExt_fn & )
	{
		return *this;
	}
};

/// FSM is Finite State Machine
template < class FSM >
class ExtNWay_T : public ExtNWay_c, private FSM
{
public:
						ExtNWay_T ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				DebugDump ( int iLevel ) override;
};


class FSMphrase_c
{
protected:
	struct State_t
	{
		int m_iTagQword;
		DWORD m_uExpHitposWithField;
	};

	CSphVector<int>			m_dQposDelta;			///< next expected qpos delta for each existing qpos (for skipped stopwords case)
	CSphVector<int>			m_dAtomPos;				///< lets use it as finite automata states and keep references on it
	CSphVector<State_t>		m_dStates;				///< pointers to states of finite automata
	DWORD					m_uQposMask {0};


							FSMphrase_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	inline bool				HitFSM ( const ExtHit_t* pHit, CSphVector<ExtHit_t> & dHits );
	inline void				ResetFSM();

	static const char *		GetName() { return "ExtPhrase"; }
};

/// exact phrase streamer
using ExtPhrase_c = ExtNWay_T<FSMphrase_c>;

/// proximity streamer
class FSMproximity_c
{
protected:
	int						m_iMaxDistance;
	DWORD					m_uWordsExpected;
	DWORD					m_uMinQpos;
	DWORD					m_uQLen;
	DWORD					m_uExpPos = 0;
	CSphVector<DWORD>		m_dProx; // proximity hit position for i-th word
	CSphVector<int> 		m_dDeltas; // used for weight calculation
	DWORD					m_uWords = 0;
	int						m_iMinQindex = 65535;
	DWORD					m_uQposMask = 0;


							FSMproximity_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	inline bool				HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits );
	inline void				ResetFSM();

	static const char *		GetName() { return "ExtProximity"; }
};

/// exact phrase streamer
using ExtProximity_c = ExtNWay_T<FSMproximity_c>;

/// proximity streamer
class FSMmultinear_c
{
protected:
	int						m_iNear;			///< the NEAR distance
	DWORD					m_uPrelastP = 0;
	DWORD					m_uPrelastML = 0;
	DWORD					m_uPrelastSL = 0;
	DWORD					m_uPrelastW = 0;
	DWORD					m_uLastP = 0;		///< position of the last hit
	DWORD					m_uLastML = 0;		///< the length of the previous hit
	DWORD					m_uLastSL = 0;		///< the length of the previous hit in Query
	DWORD					m_uLastW = 0;		///< last weight
	DWORD					m_uWordsExpected;	///< now many hits we're expect
	DWORD					m_uWeight = 0;		///< weight accum
	DWORD					m_uFirstHit = 0;	///< hitpos of the beginning of the match chain
	WORD					m_uFirstNpos = 0;	///< N-position of the head of the chain
	WORD					m_uFirstQpos = 65535;		///< Q-position of the head of the chain (for twofers)
	CSphVector<WORD>		m_dNpos;			///< query positions for multinear
	CSphVector<ExtHit_t>	m_dRing;			///< ring buffer for multihit data
	int						m_iRing = 0;		///< the head of the ring
	bool					m_bTwofer;			///< if we have 2- or N-way NEAR
	bool					m_bQposMask;


							FSMmultinear_c ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup );

	inline bool				HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits );
	inline void				ResetFSM();

	static const char *		GetName() { return "ExtMultinear"; }

private:
	inline int				RingTail() const;
	inline void				Add2Ring ( const ExtHit_t * pHit );
	inline void				ShiftRing();
};

/// exact phrase streamer
using ExtMultinear_c = ExtNWay_T<FSMmultinear_c>;

/// quorum streamer
class ExtQuorum_c : public ExtNode_c, public BufferedNode_c
{
	friend struct QuorumNodeAtomPos_fn;

public:
						ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
						~ExtQuorum_c() override;

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID () const override;
	bool				GotHitless () override { return false; }
	void				HintRowID ( RowID_t tRowID ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;

	static int			GetThreshold ( const XQNode_t & tNode, int iQwords );

private:
	struct TermTuple_t
	{
		ExtNode_i *			m_pTerm;		///< my children nodes (simply ExtTerm_c for now, not true anymore)
		const ExtDoc_t *	m_pCurDoc;		///< current positions into children doclists
		const ExtHit_t *	m_pCurHit;		///< current positions into children hitlists
		int					m_iCount;		///< terms count in case of dupes
	};

	CSphVector<TermTuple_t>	m_dInitialChildren;			///< my children nodes (simply ExtTerm_c for now)
	CSphVector<TermTuple_t>	m_dChildren;
	int						m_iThresh;					///< keyword count threshold
	// FIXME!!! also skip hits processing for children w\o constrains ( zones or field limit )
	bool					m_bHasDupes;				///< should we analyze hits on docs collecting

	// check for hits that matches and return flag that docs might be advanced
	bool				CollectMatchingHits ( RowID_t tRowID, int iQuorum );
	int					CountQuorum ( bool bFixDupes );
};


/// A-B-C-in-this-order streamer
class ExtOrder_c : public ExtNode_c, public BufferedNode_c
{
public:
								ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup );
								~ExtOrder_c() override;

	void						Reset ( const ISphQwordSetup & tSetup ) override;
	const ExtDoc_t *			GetDocsChunk() override;
	void						CollectHits ( const ExtDoc_t * pDocs ) override;
	int							GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void						SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void						GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool						GotHitless () override { return false; }
	uint64_t					GetWordID () const override;
	void						HintRowID ( RowID_t tRowID ) override;
	NodeEstimate_t				Estimate ( int64_t iTotalDocs ) const override;

protected:
	CSphVector<ExtNode_i *>		m_dChildren;
	CSphVector<const ExtDoc_t*>	m_dChildDocsChunk;	///< last document chunk (for hit fetching)
	CSphVector<const ExtDoc_t*>	m_dChildDoc;		///< current position in document chunk
	CSphVector<const ExtHit_t*>	m_dChildHit;		///< current position in hits chunk
	bool						m_bDone;

	int							GetChildIdWithNextHit ( RowID_t tRowID );	///< get next hit within given document, and return its child-id
	bool						GetMatchingHits ( RowID_t tRowID );			///< process candidate hits and stores actual matches while we can
};


/// same-text-unit streamer
/// (aka, A and B within same sentence, or same paragraph)
class ExtUnit_c : public ExtNode_c, public BufferedNode_c
{
public:
						ExtUnit_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const FieldMask_t& dFields, const ISphQwordSetup & tSetup, const char * sUnit );
						~ExtUnit_c() override;

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	uint64_t			GetWordID () const override;
	bool				GotHitless () override { return false; }
	void				HintRowID ( RowID_t tRowID ) override;
	void				DebugDump ( int iLevel ) override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override;

protected:
	void				FilterHits ( const ExtDoc_t * pDoc1, const ExtDoc_t * pDoc2, const ExtHit_t * & pHit1, const ExtHit_t * & pHit2, const ExtHit_t * & pDotHit, DWORD uSentenceEnd, RowID_t tRowID, int & iDoc );

private:
	ExtNode_i *			m_pArg1 {nullptr};		///< left arg
	ExtNode_i *			m_pArg2 {nullptr};		///< right arg
	ExtTerm_T<false> *	m_pDot {nullptr};		///< dot positions

	const ExtDoc_t *	m_pDocs1 {nullptr};		///< last chunk start
	const ExtDoc_t *	m_pDocs2 {nullptr};		///< last chunk start
	const ExtDoc_t *	m_pDotDocs {nullptr};	///< last chunk start
	const ExtDoc_t *	m_pDoc1 {nullptr};		///< current in-chunk ptr
	const ExtDoc_t *	m_pDoc2 {nullptr};		///< current in-chunk ptr
	const ExtDoc_t *	m_pDotDoc {nullptr};	///< current in-chunk ptr
	const ExtHit_t *	m_pHit1 {nullptr};		///< current in-chunk ptr
	const ExtHit_t *	m_pHit2 {nullptr};		///< current in-chunk ptr
	const ExtHit_t *	m_pDotHit {nullptr};	///< current in-chunk ptr

	// need to keep this between GetDocsChunk
	// as one call of GetDocsChunk might fetch m_pDotDocs
	// but fetch m_pDotHit many calls later
	bool m_bNeedDotHits = false;
};


class ExtNotNear_c : public ExtTwofer_c, public BufferedNode_c
{
public:
						ExtNotNear_c ( ExtNode_i * pMust, ExtNode_i * pNot, int iDist );

	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				DebugDump ( int iLevel ) override;

private:
	const int			m_iDist = 1;
	CSphString			m_sNodeName;
	const ExtHit_t *	m_pHitL = nullptr;
	const ExtHit_t *	m_pHitR = nullptr;

	bool				FilterHits ( RowID_t tRowID, const ExtHit_t * & pMust, const ExtHit_t * & pNot ); // returns true if doc has matched hits
};


//////////////////////////////////////////////////////////////////////////


static ISphQword * CreateQueryWord ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup, DictRefPtr_c pZonesDict = nullptr )
{
	BYTE sTmp [ 3*SPH_MAX_WORD_LEN + 16 ];
	strncpy ( (char*)sTmp, tWord.m_sWord.cstr(), sizeof(sTmp) );
	sTmp[sizeof(sTmp)-1] = '\0';

	ISphQword * pWord = tSetup.QwordSpawn ( tWord );
	pWord->m_sWord = tWord.m_sWord;
	if (!pZonesDict)
		pZonesDict = tSetup.Dict();
	pWord->m_uWordID = tWord.m_bMorphed
		? pZonesDict->GetWordIDNonStemmed ( sTmp )
		: pZonesDict->GetWordID ( sTmp );
	pWord->m_sDictWord = (char*)sTmp;
	pWord->m_bExpanded = tWord.m_bExpanded;
	tSetup.QwordSetup ( pWord );

	if ( tWord.m_bFieldStart && tWord.m_bFieldEnd )	pWord->m_iTermPos = TERM_POS_FIELD_STARTEND;
	else if ( tWord.m_bFieldStart )					pWord->m_iTermPos = TERM_POS_FIELD_START;
	else if ( tWord.m_bFieldEnd )					pWord->m_iTermPos = TERM_POS_FIELD_END;
	else											pWord->m_iTermPos = TERM_POS_NONE;

	pWord->m_fBoost = tWord.m_fBoost;
	pWord->m_iAtomPos = tWord.m_iAtomPos;
	return pWord;
}

struct AtomPosQWord_fn
{
	bool operator () ( ISphQword * ) const { return true; }
};

struct AtomPosExtNode_fn
{
	bool operator () ( ExtNode_i * pNode ) const { return !pNode->GotHitless(); }
};

template <typename T, typename NODE_CHECK>
int CountAtomPos ( const CSphVector<T *> & dNodes, const NODE_CHECK & fnCheck )
{
	if ( dNodes.GetLength()<2 )
		return dNodes.GetLength();

	int iMinPos = INT_MAX;
	int iMaxPos = 0;
	ARRAY_FOREACH ( i, dNodes )
	{
		T * pNode = dNodes[i];
		if ( fnCheck ( pNode ) )
		{
			iMinPos = Min ( pNode->GetAtomPos(), iMinPos );
			iMaxPos = Max ( pNode->GetAtomPos(), iMaxPos );
		}
	}
	if ( iMinPos==INT_MAX )
		return 0;

	CSphBitvec dAtomPos ( iMaxPos - iMinPos + 1 );
	ARRAY_FOREACH ( i, dNodes )
	{
		if ( fnCheck ( dNodes[i] ) )
			dAtomPos.BitSet ( dNodes[i]->GetAtomPos() - iMinPos );
	}

	return dAtomPos.BitCount();
}


template < typename T >
static ExtNode_i * CreateMultiNode ( const XQNode_t * pQueryNode, const ISphQwordSetup & tSetup, bool bNeedsHitlist, bool bUseBM25 )
{
	///////////////////////////////////
	// virtually plain (expanded) node
	///////////////////////////////////
	assert ( pQueryNode );
	if ( pQueryNode->m_dChildren.GetLength() )
	{
		CSphVector<ExtNode_i *> dNodes;
		CSphVector<ExtNode_i *> dTerms;
		ARRAY_FOREACH ( i, pQueryNode->m_dChildren )
		{
			ExtNode_i * pTerm = ExtNode_i::Create ( pQueryNode->m_dChildren[i], tSetup, bUseBM25 );
			assert ( !pTerm || pTerm->GetAtomPos()>=0 );
			if ( pTerm )
			{
				if ( !pTerm->GotHitless() )
					dNodes.Add ( pTerm );
				else
					dTerms.Add ( pTerm );
			}
		}

		int iAtoms = CountAtomPos ( dNodes, AtomPosExtNode_fn() );
		if ( iAtoms<2 )
		{
			ARRAY_FOREACH ( i, dNodes )
				SafeDelete ( dNodes[i] );
			ARRAY_FOREACH ( i, dTerms )
				SafeDelete ( dTerms[i] );
			if ( tSetup.m_pWarning )
				tSetup.m_pWarning->SetSprintf ( "can't create phrase node, hitlists unavailable (hitlists=%d, nodes=%d)", iAtoms, pQueryNode->m_dChildren.GetLength() );
			return NULL;
		}

		// FIXME! tricky combo again
		// quorum+expand used KeywordsEqual() path to drill down until actual nodes
		ExtNode_i * pResult = new T ( dNodes, *pQueryNode, tSetup );

		// AND result with the words that had no hitlist
		if ( dTerms.GetLength () )
		{
			pResult = new ExtAnd_c ( pResult, dTerms[0] );
			for ( int i=1; i<dTerms.GetLength (); i++ )
				pResult = new ExtAnd_c ( pResult, dTerms[i] );
		}

		if ( pQueryNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pResult, pQueryNode, tSetup );

		return pResult;
	}

	//////////////////////
	// regular plain node
	//////////////////////

	ExtNode_i * pResult = NULL;
	CSphVector<ISphQword *> dQwordsHit;	// have hits
	CSphVector<ISphQword *> dQwords;	// don't have hits

	// partition phrase words
	const CSphVector<XQKeyword_t> & dWords = pQueryNode->m_dWords;
	ARRAY_FOREACH ( i, dWords )
	{
		ISphQword * pWord = CreateQueryWord ( dWords[i], tSetup );
		if ( pWord->m_bHasHitlist || !bNeedsHitlist )
			dQwordsHit.Add ( pWord );
		else
			dQwords.Add ( pWord );
	}

	// see if we can create the node
	int iAtoms = CountAtomPos ( dQwordsHit, AtomPosQWord_fn() );
	if ( iAtoms<2 )
	{
		ARRAY_FOREACH ( i, dQwords )
			SafeDelete ( dQwords[i] );
		ARRAY_FOREACH ( i, dQwordsHit )
			SafeDelete ( dQwordsHit[i] );
		if ( tSetup.m_pWarning )
			tSetup.m_pWarning->SetSprintf ( "can't create phrase node, hitlists unavailable (hitlists=%d, nodes=%d)",
				iAtoms, dWords.GetLength() );
		return NULL;

	} else
	{
		// at least two words have hitlists, creating phrase node
		assert ( pQueryNode->m_dWords.GetLength() );
		assert ( pQueryNode->GetOp()==SPH_QUERY_PHRASE || pQueryNode->GetOp()==SPH_QUERY_PROXIMITY || pQueryNode->GetOp()==SPH_QUERY_QUORUM );

		// create nodes
		CSphVector<ExtNode_i *> dNodes;
		ARRAY_FOREACH ( i, dQwordsHit )
		{
			dNodes.Add ( ExtNode_i::Create ( dQwordsHit[i], pQueryNode, tSetup, bUseBM25 ) );
			dNodes.Last()->SetAtomPos ( dQwordsHit[i]->m_iAtomPos );
		}

		pResult = new T ( dNodes, *pQueryNode, tSetup );
	}

	// AND result with the words that had no hitlist
	if ( dQwords.GetLength() )
	{
		ExtNode_i * pNode = ExtNode_i::Create ( dQwords[0], pQueryNode, tSetup, bUseBM25 );
		for ( int i=1; i<dQwords.GetLength(); i++ )
			pNode = new ExtAnd_c ( pNode, ExtNode_i::Create ( dQwords[i], pQueryNode, tSetup, bUseBM25 ) );

		pResult = new ExtAnd_c ( pResult, pNode );
	}

	if ( pQueryNode->GetCount() )
		return tSetup.m_pNodeCache->CreateProxy ( pResult, pQueryNode, tSetup );

	return pResult;
}

static ExtNode_i * CreateOrderNode ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	if ( pNode->m_dChildren.GetLength()<2 )
	{
		if ( tSetup.m_pWarning )
			tSetup.m_pWarning->SetSprintf ( "order node requires at least two children" );
		return NULL;
	}

	CSphVector<ExtNode_i *> dChildren;
	ARRAY_FOREACH ( i, pNode->m_dChildren )
	{
		ExtNode_i * pChild = ExtNode_i::Create ( pNode->m_dChildren[i], tSetup, bUseBM25 );
		if ( !pChild || pChild->GotHitless() )
		{
			if ( tSetup.m_pWarning )
				tSetup.m_pWarning->SetSprintf ( "failed to create order node, hitlist unavailable" );
			ARRAY_FOREACH ( j, dChildren )
				SafeDelete ( dChildren[j] );
			return NULL;
		}
		dChildren.Add ( pChild );
	}
	ExtNode_i * pResult = new ExtOrder_c ( dChildren, tSetup );

	if ( pNode->GetCount() )
		return tSetup.m_pNodeCache->CreateProxy ( pResult, pNode, tSetup );

	return pResult;
}

ExtNode_i * ExtNode_i::Create ( const XQKeyword_t & tWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	return Create ( CreateQueryWord ( tWord, tSetup ), pNode, tSetup, bUseBM25 );
}


template <TermPosFilter_e TERMPOS>
static ExtNode_i * CreateTermposNode ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	if ( bUseBM25 )
		return new ExtTermPos_T<TERMPOS,true> ( pQword, pNode, tSetup );
	else
		return new ExtTermPos_T<TERMPOS,false> ( pQword, pNode, tSetup );
}


static ExtNode_i * CreateHitlessNode ( ISphQword * pQword, const FieldMask_t & tFieldMask, const ISphQwordSetup & tSetup, bool bNotWeighted, bool bUseBM25 )
{
	if ( bUseBM25 )
		return new ExtTermHitless_T<true> ( pQword, tFieldMask, tSetup, bNotWeighted );
	else
		return new ExtTermHitless_T<false> ( pQword, tFieldMask, tSetup, bNotWeighted );
}


static ExtNode_i * CreateTermNode ( ISphQword * pQword, const FieldMask_t & tFieldMask, const ISphQwordSetup & tSetup, bool bNotWeighted, bool bUseBM25 )
{
	if ( bUseBM25 )
		return new ExtTerm_T<true> ( pQword, tFieldMask, tSetup, bNotWeighted );
	else
		return new ExtTerm_T<false> ( pQword, tFieldMask, tSetup, bNotWeighted );
}


static ExtNode_i * CreateTermNode ( ISphQword * pQword, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	if ( bUseBM25 )
		return new ExtTerm_T<true> ( pQword, tSetup );
	else
		return new ExtTerm_T<false> ( pQword, tSetup );
}


static ExtNode_i * CreateMultiAndNode ( const VecTraits_T<XQNode_t*> & dXQNodes, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	bool bNeedFieldSpec = false;
	for ( const auto & i : dXQNodes )
		bNeedFieldSpec |= !i->m_dSpec.m_dFieldMask.TestAll(true);

	if ( bUseBM25 )
	{
		if ( bNeedFieldSpec )
			return new ExtMultiAnd_T<true,true> ( dXQNodes, tSetup );
		else
			return new ExtMultiAnd_T<true,false> ( dXQNodes, tSetup );
	}
	else
	{
		if ( bNeedFieldSpec )
			return new ExtMultiAnd_T<false,true> ( dXQNodes, tSetup );
		else
			return new ExtMultiAnd_T<false,false> ( dXQNodes, tSetup );
	}
}


ExtNode_i * ExtNode_i::Create ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	assert ( pQword );

	if ( pNode->m_dSpec.m_iFieldMaxPos )
		pQword->m_iTermPos = TERM_POS_FIELD_LIMIT;

	if ( pNode->m_dSpec.m_dZones.GetLength() )
		pQword->m_iTermPos = TERM_POS_ZONES;

	if ( !pQword->m_bHasHitlist )
	{
		if ( tSetup.m_pWarning && pQword->m_iTermPos!=TERM_POS_NONE )
			tSetup.m_pWarning->SetSprintf ( "hitlist unavailable, position limit ignored" );

		return CreateHitlessNode ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted, bUseBM25 );
	}

	switch ( pQword->m_iTermPos )
	{
		case TERM_POS_FIELD_STARTEND:	return CreateTermposNode<TERM_POS_FIELD_STARTEND> ( pQword, pNode, tSetup, bUseBM25 );
		case TERM_POS_FIELD_START:		return CreateTermposNode<TERM_POS_FIELD_START> ( pQword, pNode, tSetup, bUseBM25 );
		case TERM_POS_FIELD_END:		return CreateTermposNode<TERM_POS_FIELD_END> ( pQword, pNode, tSetup, bUseBM25 );
		case TERM_POS_FIELD_LIMIT:		return CreateTermposNode<TERM_POS_FIELD_LIMIT> ( pQword, pNode, tSetup, bUseBM25 );
		case TERM_POS_ZONES:			return CreateTermposNode<TERM_POS_ZONES> ( pQword, pNode, tSetup, bUseBM25 );
		default:						return CreateTermNode ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted, bUseBM25 );
	}
}

ExtNode_i * ExtNode_i::Create ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup, DictRefPtr_c pZonesDict, bool bUseBM25 )
{
	return CreateTermNode ( CreateQueryWord ( tWord, tSetup, std::move (pZonesDict) ), tSetup, bUseBM25 );
}

//////////////////////////////////////////////////////////////////////////

ExtNode_c::ExtNode_c()
{
	m_dDocs[0].m_tRowID = INVALID_ROWID;
	m_dHits.Reserve ( MAX_BLOCK_DOCS );
}


void ExtNode_c::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtNode\n" );
}


void ExtNode_c::SetAtomPos ( int iPos )
{
	m_iAtomPos = iPos;
}


int ExtNode_c::GetAtomPos() const
{
	return m_iAtomPos;
}


void ExtNode_c::SetQPosReverse ()
{
	m_bQPosReverse = true;
}


const ExtHit_t * ExtNode_c::GetHits ( const ExtDoc_t * pDocs )
{
	m_dHits.Resize(0);
	CollectHits ( pDocs );

	return ReturnHits ( m_bQPosReverse );
}


inline const ExtDoc_t * ExtNode_c::ReturnDocsChunk ( int iCount, const char * sNode, const char * sTerm )
{
	assert ( iCount>=0 && iCount<MAX_BLOCK_DOCS );
	m_dDocs[iCount].m_tRowID = INVALID_ROWID;

	PrintDocsChunk ( iCount, m_iAtomPos, m_dDocs, sNode, this, sTerm );
	return iCount ? m_dDocs : nullptr;
}


inline const ExtHit_t * ExtNode_c::ReturnHits ( bool bReverse )
{
	int iCount = m_dHits.GetLength();
	m_dHits.Add().m_tRowID = INVALID_ROWID;

#ifndef NDEBUG
	for ( int i=1; i<iCount; i++ )
	{
		bool bQPosPassed = ( ( bReverse && m_dHits[i-1].m_uQuerypos>=m_dHits[i].m_uQuerypos ) || ( !bReverse && m_dHits[i-1].m_uQuerypos<=m_dHits[i].m_uQuerypos ) );
		assert ( m_dHits[i-1].m_tRowID!=m_dHits[i].m_tRowID ||
			( m_dHits[i-1].m_uHitpos<m_dHits[i].m_uHitpos || ( m_dHits[i-1].m_uHitpos==m_dHits[i].m_uHitpos && bQPosPassed ) ) );

		assert ( m_dHits[i-1].m_tRowID <= m_dHits[i].m_tRowID );
	}
#endif

	PrintHitsChunk ( iCount, m_iAtomPos, iCount ? m_dHits.Begin() : nullptr, this );
	return m_dHits.Begin();
}


inline const ExtHit_t * ExtNode_c::ReturnHitsChunk ( int iCount, const char * sNode, bool bReverse )
{
	assert ( iCount>=0 && iCount<MAX_HITS );
	m_dHits[iCount].m_tRowID = INVALID_ROWID;

#ifndef NDEBUG
	for ( int i=1; i<iCount; i++ )
	{
		bool bQPosPassed = ( ( bReverse && m_dHits[i-1].m_uQuerypos>=m_dHits[i].m_uQuerypos ) || ( !bReverse && m_dHits[i-1].m_uQuerypos<=m_dHits[i].m_uQuerypos ) );
		assert ( m_dHits[i-1].m_tRowID!=m_dHits[i].m_tRowID ||
			( m_dHits[i-1].m_uHitpos<m_dHits[i].m_uHitpos || ( m_dHits[i-1].m_uHitpos==m_dHits[i].m_uHitpos && bQPosPassed ) ) );
	}
#endif

	const ExtHit_t * pHits = iCount ? m_dHits.Begin() : nullptr;
	PrintHitsChunk ( iCount, m_iAtomPos, pHits, this );
	return pHits;
}


//////////////////////////////////////////////////////////////////////////

struct ExtPayloadEntry_t
{
	RowID_t		m_tRowID;
	Hitpos_t	m_uHitpos;

	bool operator < ( const ExtPayloadEntry_t & rhs ) const
	{
		if ( m_tRowID!=rhs.m_tRowID )
			return ( m_tRowID<rhs.m_tRowID );
		return ( m_uHitpos<rhs.m_uHitpos );
	}
};

struct ExtPayloadKeyword_t : public XQKeyword_t
{
	CSphString	m_sDictWord;
	SphWordID_t	m_uWordID;
	float		m_fIDF;
	int			m_iDocs;
	int			m_iHits;
};

/// simple in-memory multi-term cache
class ExtPayload_c : public ExtNode_c
{
public:
						ExtPayload_c ( const XQNode_t * pNode, const ISphQwordSetup & tSetup );

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				HintRowID ( RowID_t ) override {} // FIXME!!! implement with tree
	void				CollectHits ( const ExtDoc_t * pDocs ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t &, CSphVector<TermPos_t> & ) const override;
	bool				GotHitless () override { return false; }
	int					GetDocsCount () override { return m_tWord.m_iDocs; }
	uint64_t			GetWordID () const override { return m_tWord.m_uWordID; }

protected:
	CSphVector<ExtPayloadEntry_t> m_dCache;
	ExtPayloadKeyword_t	m_tWord;

	int					m_iCurDocsEnd;		///< end of the last docs chunk returned, exclusive, ie [begin,end)
	int					m_iCurHit;			///< end of the last hits chunk (within the last docs chunk) returned, exclusive

	int64_t				m_iMaxTimer;		///< work until this timestamp
	CSphString *		m_pWarning;

private:
	FieldMask_t			m_dFieldMask;

	void				PopulateCache ( const ISphQwordSetup & tSetup, bool bFillStat );
};

template<bool USE_BM25>
class ExtPayload_T : public ExtPayload_c
{
public:
						ExtPayload_T ( const XQNode_t * pNode, const ISphQwordSetup & tSetup );

	const ExtDoc_t *	GetDocsChunk() final;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const final { return { 0.0f, 0, 0 }; }
};


ExtPayload_c::ExtPayload_c ( const XQNode_t * pNode, const ISphQwordSetup & tSetup )
{
	// sanity checks
	// this node must be only created for a huge OR of tiny expansions
	assert ( pNode->m_dWords.GetLength()==1 );
	assert ( pNode->m_dWords.Begin()->m_pPayload );
	assert ( pNode->m_dSpec.m_dZones.GetLength()==0 && !pNode->m_dSpec.m_bZoneSpan );

	(XQKeyword_t &)m_tWord = *pNode->m_dWords.Begin();
	m_dFieldMask = pNode->m_dSpec.m_dFieldMask;
	m_iAtomPos = m_tWord.m_iAtomPos;

	BYTE sTmpWord [ 3*SPH_MAX_WORD_LEN + 4 ];
	// our little stemming buffer (morphology aware dictionary might need to change the keyword)
	strncpy ( (char*)sTmpWord, m_tWord.m_sWord.cstr(), sizeof(sTmpWord) );
	sTmpWord[sizeof(sTmpWord)-1] = '\0';

	// setup keyword disk reader
	m_tWord.m_uWordID = tSetup.m_pDict->GetWordID ( sTmpWord );
	m_tWord.m_sDictWord = (const char *)sTmpWord;
	m_tWord.m_fIDF = -1.0f;
	m_tWord.m_iDocs = 0;
	m_tWord.m_iHits = 0;

	m_pWarning = tSetup.m_pWarning;
	m_iMaxTimer = tSetup.m_iMaxTimer;

	PopulateCache ( tSetup, true );
}


void ExtPayload_c::PopulateCache ( const ISphQwordSetup & tSetup, bool bFillStat )
{
	ISphQword * pQword = tSetup.QwordSpawn ( m_tWord );
	pQword->m_sWord = m_tWord.m_sWord;
	pQword->m_uWordID = m_tWord.m_uWordID;
	pQword->m_sDictWord = m_tWord.m_sDictWord;
	pQword->m_bExpanded = true;

	bool bOk = tSetup.QwordSetup ( pQword );

	// setup keyword idf and stats
	if ( bFillStat )
	{
		m_tWord.m_iDocs = pQword->m_iDocs;
		m_tWord.m_iHits = pQword->m_iHits;
	}
	m_dCache.Reserve ( Max ( pQword->m_iHits, pQword->m_iDocs ) );

	// read and cache all docs and hits
	if ( bOk )
		while (true)
	{
		const CSphMatch & tMatch = pQword->GetNextDoc();
		if ( tMatch.m_tRowID==INVALID_ROWID )
			break;

		pQword->SeekHitlist ( pQword->m_iHitlistPos );
		for ( Hitpos_t uHit = pQword->GetNextHit(); uHit!=EMPTY_HIT; uHit = pQword->GetNextHit() )
		{
			// apply field limits
			if ( !m_dFieldMask.Test ( HITMAN::GetField(uHit) ) )
				continue;

			// FIXME!!! apply zone limits too

			// apply field-start/field-end modifiers
			if ( m_tWord.m_bFieldStart && HITMAN::GetPos(uHit)!=1 )
				continue;
			if ( m_tWord.m_bFieldEnd && !HITMAN::IsEnd(uHit) )
				continue;

			// ok, this hit works, copy it
			ExtPayloadEntry_t & tEntry = m_dCache.Add ();
			tEntry.m_tRowID = tMatch.m_tRowID;
			tEntry.m_uHitpos = uHit;
		}
	}

	m_dCache.Sort();
	if ( bFillStat && m_dCache.GetLength() )
	{
		// there might be duplicate documents, but not hits, lets recalculate docs count
		// FIXME!!! that not work for RT index - get rid of ExtPayload_c and move PopulateCache code to index specific QWord
		RowID_t tLastRowID = m_dCache.Begin()->m_tRowID;
		const ExtPayloadEntry_t * pCur = m_dCache.Begin() + 1;
		const ExtPayloadEntry_t * pEnd = m_dCache.Begin() + m_dCache.GetLength();
		int iDocsTotal = 1;
		while ( pCur!=pEnd )
		{
			iDocsTotal += ( tLastRowID!=pCur->m_tRowID );
			tLastRowID = pCur->m_tRowID;
			pCur++;
		}
		m_tWord.m_iDocs = iDocsTotal;
	}

	// reset iterators
	m_iCurDocsEnd = 0;
	m_iCurHit = 0;

	// dismissed
	SafeDelete ( pQword );
}


void ExtPayload_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_dCache.Resize ( 0 );
	PopulateCache ( tSetup, false );
}


void ExtPayload_c::CollectHits ( const ExtDoc_t * pDocs )
{
	int iCurHit = m_iCurHit;
	const int iCurDocsEnd = m_iCurDocsEnd;

	while ( HasDocs(pDocs) && iCurHit<iCurDocsEnd )
	{
		// skip rejected documents
		while ( iCurHit<iCurDocsEnd && m_dCache[iCurHit].m_tRowID<pDocs->m_tRowID )
			iCurHit++;
		if ( iCurHit>=iCurDocsEnd )
			break;

		// skip non-matching documents
		RowID_t tRowID = m_dCache[iCurHit].m_tRowID;
		if ( pDocs->m_tRowID<tRowID )
		{
			while ( pDocs->m_tRowID<tRowID )
				pDocs++;
			if ( pDocs->m_tRowID!=tRowID )
				continue;
		}

		// copy accepted documents
		while ( iCurHit<iCurDocsEnd && m_dCache[iCurHit].m_tRowID==pDocs->m_tRowID )
		{
			ExtHit_t & tHit = m_dHits.Add();
			tHit.m_tRowID = m_dCache[iCurHit].m_tRowID;
			tHit.m_uHitpos = m_dCache[iCurHit].m_uHitpos;
			tHit.m_uQuerypos = (WORD) m_tWord.m_iAtomPos;
			tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
			iCurHit++;
		}
	}

	m_iCurHit = iCurHit;
}


int ExtPayload_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ExtQword_t tQword;
	tQword.m_sWord = m_tWord.m_sWord;
	tQword.m_sDictWord = m_tWord.m_sDictWord;
	tQword.m_iDocs = m_tWord.m_iDocs;
	tQword.m_iHits = m_tWord.m_iHits;
	tQword.m_fIDF = -1.0f;
	tQword.m_fBoost = m_tWord.m_fBoost;
	tQword.m_iQueryPos = m_tWord.m_iAtomPos;
	tQword.m_bExpanded = true;
	tQword.m_bExcluded = m_tWord.m_bExcluded;

	hQwords.Add ( tQword, m_tWord.m_sWord );
	if ( !m_tWord.m_bExcluded )
		iMax = Max ( iMax, m_tWord.m_iAtomPos );

	return iMax;
}


void ExtPayload_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	// pull idfs
	if ( m_tWord.m_fIDF<0.0f )
	{
		assert ( hQwords ( m_tWord.m_sWord ) );
		m_tWord.m_fIDF = hQwords ( m_tWord.m_sWord )->m_fIDF;
	}
}

void ExtPayload_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	if ( m_tWord.m_bExcluded )
		return;

	ExtQword_t & tQword = hQwords[ m_tWord.m_sWord ];

	TermPos_t & tPos = dTermDupes.Add();
	tPos.m_uAtomPos = (WORD)m_tWord.m_iAtomPos;
	tPos.m_uQueryPos = (WORD)tQword.m_iQueryPos;
}

//////////////////////////////////////////////////////////////////////////

template <bool USE_BM25>
ExtPayload_T<USE_BM25>::ExtPayload_T ( const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: ExtPayload_c ( pNode, tSetup )
{}


template <bool USE_BM25>
const ExtDoc_t * ExtPayload_T<USE_BM25>::GetDocsChunk()
{
	// max_query_time
	if ( sph::TimeExceeded ( m_iMaxTimer ) )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	// interrupt by sitgerm
	if ( sph::TimeExceeded ( m_iCheckTimePoint ) )
	{
		if ( g_bInterruptNow )
		{
			if ( m_pWarning )
				*m_pWarning = "Server shutdown in progress";
			return nullptr;
		}

		if ( session::GetKilled() )
		{
			if ( m_pWarning )
				*m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	int iDoc = 0;
	int iEnd = m_iCurDocsEnd;
	while ( iDoc<MAX_BLOCK_DOCS-1 && iEnd<m_dCache.GetLength() )
	{
		RowID_t tRowID = m_dCache[iEnd].m_tRowID;

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_tRowID = tRowID;
		tDoc.m_uDocFields = 0;

		int iHitStart = iEnd;
		while ( iEnd<m_dCache.GetLength() && m_dCache[iEnd].m_tRowID==tRowID )
		{
			tDoc.m_uDocFields |= 1<< ( HITMAN::GetField ( m_dCache[iEnd].m_uHitpos ) );
			iEnd++;
		}

		if_const(USE_BM25)
		{
			int iHits = iEnd - iHitStart;
			tDoc.m_fTFIDF = float(iHits) / float(SPH_BM25_K1+iHits) * m_tWord.m_fIDF;
		}
	}
	m_iCurDocsEnd = iEnd;

	return ReturnDocsChunk ( iDoc, "payload", m_tWord.m_sDictWord.cstr() );
}


//////////////////////////////////////////////////////////////////////////

static ExtNode_i * CreatePayloadNode ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	if ( bUseBM25 )
		return new ExtPayload_T<true> ( pNode, tSetup );
	else
		return new ExtPayload_T<false> ( pNode, tSetup );
}


ExtNode_i * ExtNode_i::Create ( const XQNode_t * pNode, const ISphQwordSetup & tSetup, bool bUseBM25 )
{
	// empty node?
	if ( pNode->IsEmpty() && pNode->GetOp()!=SPH_QUERY_SCAN )
		return NULL;

	if ( pNode->GetOp()==SPH_QUERY_SCAN )
	{
		return CreateHitlessNode ( tSetup.ScanSpawn(), pNode->m_dSpec.m_dFieldMask, tSetup, true, bUseBM25 );
	}

	if ( pNode->m_dWords.GetLength() || pNode->m_bVirtuallyPlain )
	{
		const int iWords = pNode->m_bVirtuallyPlain
			? pNode->m_dChildren.GetLength()
			: pNode->m_dWords.GetLength();

		if ( iWords==1 )
		{
			if ( pNode->m_dWords.Begin()->m_bExpanded && pNode->m_dWords.Begin()->m_pPayload )
				return CreatePayloadNode ( pNode, tSetup, bUseBM25 );

			if ( pNode->m_bVirtuallyPlain )
				return Create ( pNode->m_dChildren[0], tSetup, bUseBM25 );
			else
				return Create ( pNode->m_dWords[0], pNode, tSetup, bUseBM25 );
		}

		switch ( pNode->GetOp() )
		{
			case SPH_QUERY_PHRASE:
				return CreateMultiNode<ExtPhrase_c> ( pNode, tSetup, true, bUseBM25 );

			case SPH_QUERY_PROXIMITY:
				return CreateMultiNode<ExtProximity_c> ( pNode, tSetup, true, bUseBM25 );

			case SPH_QUERY_NEAR:
				return CreateMultiNode<ExtMultinear_c> ( pNode, tSetup, true, bUseBM25 );

			case SPH_QUERY_QUORUM:
			{
				assert ( pNode->m_dWords.GetLength()==0 || pNode->m_dChildren.GetLength()==0 );
				int iQuorumCount = pNode->m_dWords.GetLength()+pNode->m_dChildren.GetLength();
				int iThr = ExtQuorum_c::GetThreshold ( *pNode, iQuorumCount );
				bool bOrOperator = false;
				if ( iThr>=iQuorumCount )
				{
					// threshold is too high
					if ( tSetup.m_pWarning && !pNode->m_bPercentOp )
						tSetup.m_pWarning->SetSprintf ( "quorum threshold too high (words=%d, thresh=%d); replacing quorum operator with AND operator",
							iQuorumCount, pNode->m_iOpArg );

				} else if ( iQuorumCount>256 )
				{
					// right now quorum can only handle 256 words
					if ( tSetup.m_pWarning )
						tSetup.m_pWarning->SetSprintf ( "too many words (%d) for quorum; replacing with an AND", iQuorumCount );
				} else if ( iThr==1 )
				{
					bOrOperator = true;
				} else // everything is ok; create quorum node
				{
					return CreateMultiNode<ExtQuorum_c> ( pNode, tSetup, false, bUseBM25 );
				}

				// couldn't create quorum, make an AND node instead
				CSphVector<ExtNode_i*> dTerms;
				dTerms.Reserve ( iQuorumCount );

				for ( const XQKeyword_t& tWord: pNode->m_dWords )
					dTerms.Add ( Create ( tWord, pNode, tSetup, bUseBM25 ) );

				for ( const XQNode_t* tNode: pNode->m_dChildren )
					dTerms.Add ( Create ( tNode, tSetup, bUseBM25 ) );

				// make not simple, but optimized AND node.
				dTerms.Sort ( ExtNodeTF_fn() );

				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<dTerms.GetLength(); i++ )
				{
					if ( !bOrOperator )
						pCur = new ExtAnd_c ( pCur, dTerms[i] );
					else
						pCur = new ExtOr_c ( pCur, dTerms[i] );
				}

				if ( pNode->GetCount() )
					return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
				return pCur;
			}
			default:
				assert ( 0 && "unexpected plain node type" );
				return NULL;
		}

	} else
	{
		int iChildren = pNode->m_dChildren.GetLength ();
		assert ( iChildren>0 );

		// special case, operator BEFORE
		if ( pNode->GetOp ()==SPH_QUERY_BEFORE )
		{
			// before operator can not handle ZONESPAN
			if ( tSetup.m_pWarning
				&& pNode->m_dChildren.any_of ( [] ( XQNode_t * pChild ) { return pChild->m_dSpec.m_bZoneSpan; } ) )
				tSetup.m_pWarning->SetSprintf ( "BEFORE operator is incompatible with ZONESPAN, ZONESPAN ignored" );
			return CreateOrderNode ( pNode, tSetup, bUseBM25 );
		}

		// special case, AND over terms (internally reordered for speed)
		bool bAndTerms = ( pNode->GetOp()==SPH_QUERY_AND );
		for ( int i=0; i<iChildren && bAndTerms; i++ )
		{
			const XQNode_t * pChildren = pNode->m_dChildren[i];
			bAndTerms = ( pChildren->m_dWords.GetLength()==1 );
		}

		bool bZonespan = bAndTerms;
		for ( int i=0; i<iChildren && bZonespan; i++ )
			bZonespan &= pNode->m_dChildren[i]->m_dSpec.m_bZoneSpan;

		if ( bAndTerms )
		{
			// check if we can create multi-and node
			bool bMultiAnd = !bZonespan && pNode->m_dChildren.GetLength()>1;
			for ( int i=0; i<iChildren && bMultiAnd; i++ )
			{
				const XQNode_t * pChild = pNode->m_dChildren[i];
				const XQKeyword_t & tWord = pChild->m_dWords[0];
				if ( tWord.m_bFieldStart || tWord.m_bFieldEnd || tWord.m_pPayload || pChild->m_dSpec.m_iFieldMaxPos || pChild->m_dSpec.m_dZones.GetLength() )
				{
					bMultiAnd = false;
					break;
				}
			}

			ESphHitless eMode = tSetup.m_pIndex ? tSetup.m_pIndex->GetSettings().m_eHitless : SPH_HITLESS_NONE;
			if ( eMode==SPH_HITLESS_SOME || eMode==SPH_HITLESS_ALL )
				bMultiAnd = false;

			if ( !bMultiAnd )
			{
				// create eval-tree terms from query-tree nodes
				CSphVector<ExtNode_i*> dTerms;
				for ( int i=0; i<iChildren; i++ )
				{
					const XQNode_t * pChild = pNode->m_dChildren[i];
					ExtNode_i * pTerm = ExtNode_i::Create ( pChild, tSetup, bUseBM25 );
					if ( pTerm )
						dTerms.Add ( pTerm );
				}

				// sort them by frequency, to speed up AND matching
				dTerms.Sort ( ExtNodeTF_fn() );

				// create the right eval-tree node
				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<dTerms.GetLength(); i++ )
					if ( bZonespan )
						pCur = new ExtAndZonespan_c ( pCur, dTerms[i], tSetup, pNode->m_dChildren[0] );
					else
						pCur = new ExtAnd_c ( pCur, dTerms[i] );

				// zonespan has Extra data which is not (yet?) covered by common-node optimizations,
				// so we need to avoid those for zonespan
				if ( !bZonespan && pNode->GetCount() )
					return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );

				return pCur;
			}
			else
			{
				// create multi-and node over terms
				return CreateMultiAndNode ( pNode->m_dChildren, tSetup, bUseBM25 );
			}
		}

		// Multinear and phrase could be also non-plain, so here is the second entry for it.
		if ( pNode->GetOp()==SPH_QUERY_NEAR )
			return CreateMultiNode<ExtMultinear_c> ( pNode, tSetup, true, bUseBM25 );
		if ( pNode->GetOp()==SPH_QUERY_PHRASE )
			return CreateMultiNode<ExtPhrase_c> ( pNode, tSetup, true, bUseBM25 );

		// generic create
		ExtNode_i * pCur = NULL;
		for ( int i=0; i<iChildren; i++ )
		{
			ExtNode_i * pNext = ExtNode_i::Create ( pNode->m_dChildren[i], tSetup, bUseBM25 );
			if ( !pNext ) continue;
			if ( !pCur )
			{
				pCur = pNext;
				continue;
			}
			switch ( pNode->GetOp() )
			{
				case SPH_QUERY_OR:			pCur = new ExtOr_c ( pCur, pNext ); break;
				case SPH_QUERY_MAYBE:		pCur = new ExtMaybe_c ( pCur, pNext ); break;
				case SPH_QUERY_AND:			pCur = new ExtAnd_c ( pCur, pNext ); break;
				case SPH_QUERY_ANDNOT:		pCur = new ExtAndNot_c ( pCur, pNext ); break;
				case SPH_QUERY_SENTENCE:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, MAGIC_WORD_SENTENCE ); break;
				case SPH_QUERY_PARAGRAPH:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, MAGIC_WORD_PARAGRAPH ); break;
				case SPH_QUERY_NOTNEAR:		pCur = new ExtNotNear_c ( pCur, pNext, pNode->m_iOpArg ); break;
				default:					assert ( 0 && "internal error: unhandled op in ExtNode_i::Create()" ); break;
			}
		}
		if ( pCur && pNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
		return pCur;
	}
}

//////////////////////////////////////////////////////////////////////////

template<bool USE_BM25>
ExtTerm_T<USE_BM25>::ExtTerm_T()
{
	m_dQueriedFields.UnsetAll ();
}


template<bool USE_BM25>
ExtTerm_T<USE_BM25>::~ExtTerm_T()
{
	SafeDelete ( m_pQword );
}


template<bool USE_BM25>
inline void ExtTerm_T<USE_BM25>::Init ( ISphQword * pQword, const FieldMask_t & tFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
{
	m_pQword = pQword;
	m_pWarning = tSetup.m_pWarning;
	m_bNotWeighted = bNotWeighted;
	m_iAtomPos = pQword->m_iAtomPos;
	m_dQueriedFields = tFields;
	m_bHasWideFields = false;
	if ( tSetup.m_bHasWideFields )
		for ( int i=1; i<FieldMask_t::SIZE && !m_bHasWideFields; i++ )
			if ( m_dQueriedFields[i] )
				m_bHasWideFields = true;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pStats = tSetup.m_pStats;
	m_pNanoBudget = m_pStats ? m_pStats->m_pNanoBudget : NULL;
}

template<bool USE_BM25>
ExtTerm_T<USE_BM25>::ExtTerm_T ( ISphQword * pQword, const ISphQwordSetup & tSetup )
	: m_pQword ( pQword )
	, m_pWarning ( tSetup.m_pWarning )
{
	m_iAtomPos = pQword->m_iAtomPos;
	m_dQueriedFields.SetAll();
	m_bHasWideFields = tSetup.m_bHasWideFields;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pStats = tSetup.m_pStats;
	m_pNanoBudget = m_pStats ? m_pStats->m_pNanoBudget : nullptr;
}

template<bool USE_BM25>
ExtTerm_T<USE_BM25>::ExtTerm_T ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
{
	Init ( pQword, dFields, tSetup, bNotWeighted );
}

template<bool USE_BM25>
void ExtTerm_T<USE_BM25>::Reset ( const ISphQwordSetup & tSetup )
{
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pQword->Reset ();
	tSetup.QwordSetup ( m_pQword );
	m_dStoredHits.Resize(0);
}

template<bool USE_BM25>
const ExtDoc_t * ExtTerm_T<USE_BM25>::GetDocsChunk()
{
	if ( !m_pQword->m_iDocs )
		return NULL;

	// max_query_time
	if ( sph::TimeExceeded ( m_iMaxTimer ) )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	// max_predicted_time
	if ( m_pNanoBudget && *m_pNanoBudget<0 )
	{
		if ( m_pWarning )
			*m_pWarning = "predicted query time exceeded max_predicted_time";
		return nullptr;
	}

	if ( sph::TimeExceeded ( m_iCheckTimePoint ) )
	{
		// interrupt by sitgerm
		if ( g_bInterruptNow )
		{
			if ( m_pWarning )
				*m_pWarning = "Server shutdown in progress";
			return nullptr;
		}

		if ( session::GetKilled() )
		{
			if ( m_pWarning )
				*m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	StoredHit_t * pStoredHit = nullptr;
	StoredHit_t * pFirstHit = nullptr;
	if ( m_bCollectHits )
	{
		int iLength = m_dStoredHits.GetLength();
		m_dStoredHits.Reserve ( iLength+MAX_BLOCK_DOCS );
		pStoredHit = m_dStoredHits.End();
		pFirstHit = pStoredHit-iLength;	// hack to get to m_pData
	}

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		const CSphMatch & tMatch = m_pQword->GetNextDoc();
		if ( tMatch.m_tRowID==INVALID_ROWID )
		{
			m_pQword->m_iDocs = 0;
			break;
		}

		if ( !m_bHasWideFields )
		{
			// fields 0-31 can be quickly checked right here, right now
			if (!( m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32() ))
				continue;
		} else
		{
			// fields 32+ need to be checked with CollectHitMask() and stuff
			m_pQword->CollectHitMask();
			bool bHasSameFields = false;
			for ( int i=0; i<FieldMask_t::SIZE && !bHasSameFields; i++ )
				bHasSameFields = ( m_pQword->m_dQwordFields[i] & m_dQueriedFields[i] )!=0;
			if ( !bHasSameFields )
				continue;
		}

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_tRowID = tMatch.m_tRowID;
		tDoc.m_uDocFields = m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32(); // OPTIMIZE: only needed for phrase node

		if_const ( USE_BM25 )
			tDoc.m_fTFIDF = float(m_pQword->m_uMatchHits) / float(m_pQword->m_uMatchHits+SPH_BM25_K1) * m_fIDF;

		// store some hit info here, we can't reuse m_dDocs in CollectHits
		// but only if the ranker uses hits
		if ( m_bCollectHits )
		{
			pStoredHit->m_tHitlistOffset = m_pQword->m_iHitlistPos;
			pStoredHit->m_tRowID = tDoc.m_tRowID;
			pStoredHit++;
		}
	}

	if ( m_bCollectHits )
		m_dStoredHits.Resize ( pStoredHit-pFirstHit );

	if ( m_pStats )
		m_pStats->m_iFetchedDocs += iDoc;
	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostDoc*iDoc;

	return ReturnDocsChunk ( iDoc, "term", m_pQword->m_sDictWord.cstr() );
}


template<bool USE_BM25>
void ExtTerm_T<USE_BM25>::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return;

	m_dStoredHits.Add().m_tRowID = INVALID_ROWID;
	StoredHit_t * pStoredHit = m_dStoredHits.Begin();

	for ( ; HasDocs(pMatched); pMatched++ )
	{
		while ( pStoredHit->m_tRowID < pMatched->m_tRowID )
			pStoredHit++;

		if ( pStoredHit->m_tRowID!=pMatched->m_tRowID )
			continue;

		// setup hitlist reader
		m_pQword->SeekHitlist ( pStoredHit->m_tHitlistOffset );

		while (true)
		{
			// get next hit
			Hitpos_t uHit = m_pQword->GetNextHit();
			if ( uHit==EMPTY_HIT )
			{
				// no more hits; get next acceptable document
				pStoredHit++;
				break;
			}

			if ( !( m_dQueriedFields.Test ( HITMAN::GetField(uHit) ) ) )
				continue;

			ExtHit_t & tHit = m_dHits.Add();
			tHit.m_tRowID = pStoredHit->m_tRowID;
			tHit.m_uHitpos = uHit;
			tHit.m_uQuerypos = (WORD) m_iAtomPos; // assume less that 64K words per query
			tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
		}
	}

	int nHits = m_dHits.GetLength();
	if ( m_pStats )
		m_pStats->m_iFetchedHits += nHits;

	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostHit*nHits;

	// we assume that GetHits doesn't get called multiple times for the same docids in pMatched
	// so let's drop the stored hits that we already used
	// so that we won't need to loop through them the next time GetHits gets called for the same docs chunk
	// however, remove only the hits that we've processed. others will be processed in the next GetDocsChunk() call
	int nProcessed = int ( pStoredHit-m_dStoredHits.Begin() );
	m_dStoredHits.Pop();	// end marker
	m_dStoredHits.Remove ( 0, nProcessed );
}


template<bool USE_BM25>
int ExtTerm_T<USE_BM25>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_fIDF = 0.0f;

	ExtQword_t * pQword = hQwords ( m_pQword->m_sWord );
	if ( !m_bNotWeighted && pQword && !pQword->m_bExcluded )
		pQword->m_iQueryPos = Min ( pQword->m_iQueryPos, m_pQword->m_iAtomPos );

	if ( m_bNotWeighted || pQword )
		return m_pQword->m_bExcluded ? -1 : m_pQword->m_iAtomPos;

	m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = m_pQword->m_sWord;
	tInfo.m_sDictWord = m_pQword->m_sDictWord;
	tInfo.m_iDocs = m_pQword->m_iDocs;
	tInfo.m_iHits = m_pQword->m_iHits;
	tInfo.m_iQueryPos = m_pQword->m_iAtomPos;
	tInfo.m_fIDF = -1.0f; // suppress gcc 4.2.3 warning
	tInfo.m_fBoost = m_pQword->m_fBoost;
	tInfo.m_bExpanded = m_pQword->m_bExpanded;
	tInfo.m_bExcluded = m_pQword->m_bExcluded;
	hQwords.Add ( tInfo, m_pQword->m_sWord );
	return m_pQword->m_bExcluded ? -1 : m_pQword->m_iAtomPos;
}

template<bool USE_BM25>
void ExtTerm_T<USE_BM25>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	if ( m_fIDF<0.0f )
	{
		assert ( hQwords ( m_pQword->m_sWord ) );
		m_fIDF = hQwords ( m_pQword->m_sWord )->m_fIDF;
	}
}

template<bool USE_BM25>
void ExtTerm_T<USE_BM25>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	if ( m_bNotWeighted || m_pQword->m_bExcluded )
		return;

	ExtQword_t & tQword = hQwords[ m_pQword->m_sWord ];

	TermPos_t & tPos = dTermDupes.Add ();
	tPos.m_uAtomPos = (WORD)m_pQword->m_iAtomPos;
	tPos.m_uQueryPos = (WORD)tQword.m_iQueryPos;
}

template<bool USE_BM25>
uint64_t ExtTerm_T<USE_BM25>::GetWordID () const
{
	if ( m_pQword->m_uWordID )
		return m_pQword->m_uWordID;

	return sphFNV64 ( m_pQword->m_sDictWord.cstr() );
}

template<bool USE_BM25>
void ExtTerm_T<USE_BM25>::HintRowID ( RowID_t tRowID )
{
	m_pQword->HintRowID ( tRowID );

	if ( m_pStats )
		m_pStats->m_iSkips++;

	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostSkip;
}

template<bool USE_BM25>
void ExtTerm_T<USE_BM25>::SetCollectHits()
{
	m_bCollectHits = true;
}

template<bool USE_BM25>
NodeEstimate_t ExtTerm_T<USE_BM25>::Estimate ( int64_t iTotalDocs ) const
{
	return { float(m_pQword->m_iDocs)*COST_SCALE*60.0f, m_pQword->m_iDocs, 1 };
}

template<bool USE_BM25>
void ExtTerm_T<USE_BM25>::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtTerm: %s at: %d ", m_pQword->m_sWord.cstr(), m_pQword->m_iAtomPos );

	if ( m_dQueriedFields.TestAll(true) )
		printf ( "(all)\n" );
	else
	{
		bool bFirst = true;
		printf ( "in: " );
		for ( int iField=0; iField<SPH_MAX_FIELDS; iField++ )
		{
			if ( m_dQueriedFields.Test ( iField ) )
			{
				if ( !bFirst )
					printf ( ", " );
				printf ( "%d", iField );
				bFirst = false;
			}
		}
		printf ( "\n" );
	}
}


//////////////////////////////////////////////////////////////////////////

template<bool USE_BM25>
ExtTermHitless_T<USE_BM25>::ExtTermHitless_T ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
	: ExtTerm_T<USE_BM25> ( pQword, dFields, tSetup, bNotWeighted )
{}


template<bool USE_BM25>
void ExtTermHitless_T<USE_BM25>::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return;

	this->m_dStoredHits.Add().m_tRowID = INVALID_ROWID;
	typename ExtTerm_T<USE_BM25>::StoredHit_t * pStoredHit = this->m_dStoredHits.Begin();

	for ( ; HasDocs(pMatched); pMatched++ )
	{
		while ( pStoredHit->m_tRowID < pMatched->m_tRowID )
			pStoredHit++;

		if ( pStoredHit->m_tRowID!=pMatched->m_tRowID )
			continue;

		DWORD uMaxFields = SPH_MAX_FIELDS;
		if ( !this->m_bHasWideFields )
		{
			uMaxFields = 0;
			DWORD uFields = pMatched->m_uDocFields;
			while ( uFields ) // count up to highest bit, max value is 32
			{
				uFields >>= 1;
				uMaxFields++;
			}
		}

		for ( DWORD uFieldPos=0; uFieldPos<uMaxFields; uFieldPos++ )
			if ( ( pMatched->m_uDocFields & ( 1 << uFieldPos ) ) && this->m_dQueriedFields.Test ( uFieldPos ) )
			{
				// emit hit
				ExtHit_t & tHit = this->m_dHits.Add();
				tHit.m_tRowID = pMatched->m_tRowID;
				tHit.m_uHitpos = HITMAN::Create ( uFieldPos, -1 );
				tHit.m_uQuerypos = (WORD) (this->m_iAtomPos);
				tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
			}
	}

	int nHits = this->m_dHits.GetLength();
	if ( this->m_pStats )
		this->m_pStats->m_iFetchedHits += nHits;

	if ( this->m_pNanoBudget )
		*(this->m_pNanoBudget) -= g_iPredictorCostHit*nHits;

	// same logic as in ExtTerm_T::CollectHits
	int nProcessed = int ( pStoredHit-this->m_dStoredHits.Begin() );
	this->m_dStoredHits.Pop();	// end marker
	this->m_dStoredHits.Remove ( 0, nProcessed );
}

//////////////////////////////////////////////////////////////////////////

BufferedNode_c::BufferedNode_c()
{
	Reset();
}


void BufferedNode_c::Reset()
{
	m_pRawDocs = nullptr;
	m_pRawDoc = nullptr;
	m_pRawHit = nullptr;
	m_dMyDocs[0].m_tRowID = INVALID_ROWID;
	m_dMyHits.Resize(0);
}


void BufferedNode_c::CopyMatchingHits ( CSphVector<ExtHit_t> & dHits, const ExtDoc_t * pDocs )
{
	m_dMyHits.Add().m_tRowID = INVALID_ROWID;

	dHits.Resize(0);
	const ExtHit_t * pMyHit = m_dMyHits.Begin();

	while ( HasDocs(pDocs) )
	{
		while ( pMyHit->m_tRowID < pDocs->m_tRowID )
			pMyHit++;

		while ( pMyHit->m_tRowID==pDocs->m_tRowID )
			dHits.Add ( *pMyHit++ );

		pDocs++;
	}

	// remove only the hits that we've processed. others will be processed in the next GetDocsChunk() call
	int nProcessed = int ( pMyHit-m_dMyHits.Begin() );
	m_dMyHits.Pop();	// end marker
	m_dMyHits.Remove ( 0, nProcessed );
}

//////////////////////////////////////////////////////////////////////////

template < TermPosFilter_e T, class NODE >
ExtConditional_T<T,NODE>::ExtConditional_T ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: BufferedNode_c ()
	, Acceptor_c ( pQword, pNode, tSetup )
{
	// we still need those hits even if the ranker hints us that we can ignore them
	m_tNode.SetCollectHits();
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T,NODE>::Reset ( const ISphQwordSetup & tSetup )
{
	BufferedNode_c::Reset();
	TermAcceptor_T<T>::Reset();
	m_tNode.Reset(tSetup);
}

//////////////////////////////////////////////////////////////////////////

TermAcceptor_T<TERM_POS_FIELD_LIMIT>::TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & )
	: m_iMaxFieldPos ( pNode->m_dSpec.m_iFieldMaxPos )
{}


inline bool TermAcceptor_T<TERM_POS_FIELD_LIMIT>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )<=m_iMaxFieldPos;
}

template<>
inline bool TermAcceptor_T<TERM_POS_FIELD_START>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1;
}

template<>
inline bool TermAcceptor_T<TERM_POS_FIELD_END>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::IsEnd ( pHit->m_uHitpos );
}

template<>
inline bool TermAcceptor_T<TERM_POS_FIELD_STARTEND>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1 && HITMAN::IsEnd ( pHit->m_uHitpos );
}


TermAcceptor_T<TERM_POS_ZONES>::TermAcceptor_T ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: m_pZoneChecker ( tSetup.m_pZoneChecker )
	, m_dZones ( pNode->m_dSpec.m_dZones )
{}


inline bool TermAcceptor_T<TERM_POS_ZONES>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	assert ( m_pZoneChecker );

	if ( m_tLastZoneRowID!=pHit->m_tRowID )
		m_iCheckFrom = 0;

	m_tLastZoneRowID = pHit->m_tRowID;

	// only check zones that actually match this document
	for ( int i=m_iCheckFrom; i<m_dZones.GetLength(); i++ )
	{
		SphZoneHit_e eState = m_pZoneChecker->IsInZone ( m_dZones[i], pHit, NULL );
		switch ( eState )
		{
			case SPH_ZONE_FOUND:
				return true;
			case SPH_ZONE_NO_DOCUMENT:
				Swap ( m_dZones[i], m_dZones[m_iCheckFrom] );
				m_iCheckFrom++;
				break;
			default:
				break;
		}
	}
	return false;
}


inline void TermAcceptor_T<TERM_POS_ZONES>::Reset()
{
	m_tLastZoneRowID = INVALID_ROWID;
	m_iCheckFrom = 0;
}

//////////////////////////////////////////////////////////////////////////

template < TermPosFilter_e T, class NODE >
const ExtDoc_t * ExtConditional_T<T,NODE>::GetDocsChunk()
{
	// fetch more docs if needed
	if ( !HasDocs(m_pRawDocs) )
	{
		m_pRawDocs = m_tNode.GetDocsChunk();
		if ( !HasDocs(m_pRawDocs) )
			return nullptr;

		m_pRawDoc = m_pRawDocs;
		m_pRawHit = m_tNode.GetHits(m_pRawDoc);
	}

	// filter the hits, and build the documents list
	int iMyDoc = 0;

	const ExtDoc_t * pDoc = m_pRawDoc;
	const ExtHit_t * pHit = m_pRawHit;

	while (true)
	{
		if ( iMyDoc==MAX_BLOCK_DOCS-1 )
			break;

		// did we touch all the hits we had? if so, we're fully done with
		// current raw docs block, and should start a new one
		if ( !HasHits(pHit) )
		{
			m_pRawDocs = m_tNode.GetDocsChunk();
			if ( !HasDocs(m_pRawDocs) )
				break;

			pDoc = m_pRawDocs;
			pHit = m_tNode.GetHits(pDoc);
			continue;
		}

		// scan until next acceptable hit
		while ( pHit->m_tRowID < pDoc->m_tRowID ) // skip leftovers
			pHit++;

		while ( HasHits(pHit) && !Acceptor_c::IsAcceptableHit(pHit) ) // skip unneeded hits
			pHit++;

		if ( !HasHits(pHit) ) // check for eof
			continue;

		// find and emit new document
		while ( pDoc->m_tRowID<pHit->m_tRowID )
			pDoc++; // FIXME? unsafe in broken cases

		assert ( pDoc->m_tRowID==pHit->m_tRowID );
		assert ( iMyDoc<MAX_BLOCK_DOCS-1 );

		m_dMyDocs[iMyDoc++] = *pDoc;
		m_dMyHits.Add ( *(pHit++) );

		// copy acceptable hits for this document
		for ( ; pHit->m_tRowID==pDoc->m_tRowID; pHit++ )
		{
			if ( Acceptor_c::IsAcceptableHit ( pHit ) )
				m_dMyHits.Add ( *pHit );
		}
	}

	m_pRawDoc = pDoc;
	m_pRawHit = pHit;

	assert ( iMyDoc>=0 && iMyDoc<MAX_BLOCK_DOCS );
	m_dMyDocs[iMyDoc].m_tRowID = INVALID_ROWID;

	PrintDocsChunk ( iMyDoc, m_tNode.GetAtomPos(), m_dMyDocs, "cond", this );

	return iMyDoc ? m_dMyDocs : nullptr;
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_tNode.GetAtomPos(), m_dHits.Begin(), this );
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::HintRowID ( RowID_t tRowID )
{
	m_tNode.HintRowID ( tRowID );
}


template < TermPosFilter_e T, class NODE >
int ExtConditional_T<T, NODE>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	return m_tNode.GetQwords ( hQwords );
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	return m_tNode.SetQwordsIDF ( hQwords );
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	return m_tNode.GetTerms ( hQwords, dTermDupes );
}


template < TermPosFilter_e T, class NODE >
uint64_t ExtConditional_T<T, NODE>::GetWordID() const
{
	return m_tNode.GetWordID();
}


template < TermPosFilter_e T, class NODE >
void ExtConditional_T<T, NODE>::SetAtomPos ( int iPos )
{
	m_tNode.SetAtomPos(iPos);
}


template < TermPosFilter_e T, class NODE >
int ExtConditional_T<T, NODE>::GetAtomPos() const
{
	return m_tNode.GetAtomPos();
}


//////////////////////////////////////////////////////////////////////////

ExtTwofer_c::ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond )
{
	Init ( pFirst, pSecond );
}

inline void	ExtTwofer_c::Init ( ExtNode_i * pLeft, ExtNode_i * pRight )
{
	m_pLeft = std::unique_ptr<ExtNode_i>(pLeft);
	m_pRight = std::unique_ptr<ExtNode_i>(pRight);
	m_pDocL = nullptr;
	m_pDocR = nullptr;
	m_uNodePosL = 0;
	m_uNodePosR = 0;
	m_iAtomPos = ( pLeft && pLeft->GetAtomPos() ) ? pLeft->GetAtomPos() : 0;
	if ( pRight && pRight->GetAtomPos() && pRight->GetAtomPos()<m_iAtomPos && m_iAtomPos!=0 )
		m_iAtomPos = pRight->GetAtomPos();
}

void ExtTwofer_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pLeft->Reset ( tSetup );
	m_pRight->Reset ( tSetup );
	m_pDocL = nullptr;
	m_pDocR = nullptr;
}

int ExtTwofer_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax1 = m_pLeft->GetQwords ( hQwords );
	int iMax2 = m_pRight->GetQwords ( hQwords );
	return Max ( iMax1, iMax2 );
}

void ExtTwofer_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pLeft->SetQwordsIDF ( hQwords );
	m_pRight->SetQwordsIDF ( hQwords );
}


void ExtTwofer_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	m_pLeft->GetTerms ( hQwords, dTermDupes );
	m_pRight->GetTerms ( hQwords, dTermDupes );
}


bool ExtTwofer_c::GotHitless ()
{
	return m_pLeft->GotHitless() || m_pRight->GotHitless();
}


void ExtTwofer_c::DebugDumpT ( const char * sName, int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "%s:\n", sName );
	m_pLeft->DebugDump ( iLevel+1 );
	m_pRight->DebugDump ( iLevel+1 );
}


void ExtTwofer_c::SetNodePos ( WORD uPosLeft, WORD uPosRight )
{
	m_uNodePosL = uPosLeft;
	m_uNodePosR = uPosRight;
}


void ExtTwofer_c::HintRowID ( RowID_t tRowID )
{
	m_pLeft->HintRowID ( tRowID );
	m_pRight->HintRowID ( tRowID );
}


uint64_t ExtTwofer_c::GetWordID () const
{
	uint64_t dHash[2];
	dHash[0] = m_pLeft->GetWordID();
	dHash[1] = m_pRight->GetWordID();
	return sphFNV64 ( dHash, sizeof(dHash) );
}


void ExtTwofer_c::SetCollectHits()
{
	if ( m_pLeft )
		m_pLeft->SetCollectHits();

	if ( m_pRight )
		m_pRight->SetCollectHits();
}


NodeEstimate_t ExtTwofer_c::Estimate ( int64_t iTotalDocs ) const
{
	NodeEstimate_t tLeft = { 0.0f, 0, 0 };
	if ( m_pLeft )
		tLeft = m_pLeft->Estimate(iTotalDocs);

	NodeEstimate_t tRight = { 0.0f, 0, 0 };
	if ( m_pRight )
		tRight = m_pRight->Estimate(iTotalDocs);

	tLeft += tRight;
	return tLeft;
}

//////////////////////////////////////////////////////////////////////////

const ExtDoc_t * ExtAnd_c::GetDocsChunk()
{
	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, pDocR, m_pLeft.get() ) )
			break;

		if ( !WarmupDocs ( pDocR, pDocL, m_pRight.get() ) )
			break;

		assert ( pDocL && pDocR );
		
		if ( pDocL->m_tRowID==pDocR->m_tRowID )
		{
			// emit it
			ExtDoc_t & tDoc = m_dDocs[iDoc++];
			tDoc.m_tRowID = pDocL->m_tRowID;
			tDoc.m_uDocFields = pDocL->m_uDocFields | pDocR->m_uDocFields; // not necessary
			tDoc.m_fTFIDF = pDocL->m_fTFIDF + pDocR->m_fTFIDF;

			// skip it
			pDocL++;
			pDocR++;
		}
		else if ( pDocL->m_tRowID<pDocR->m_tRowID )
			pDocL++;
		else
			pDocR++;
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "and" );
}


static inline bool IsHitLess ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 )
{
	assert ( pHit1 && pHit2 );
	return ( pHit1->m_uHitpos<pHit2->m_uHitpos ) || ( pHit1->m_uHitpos==pHit2->m_uHitpos && pHit1->m_uQuerypos<=pHit2->m_uQuerypos );
}


struct CmpAndHitReverse_fn
{
	inline bool IsLess ( const ExtHit_t & a, const ExtHit_t & b ) const
	{
		return ( a.m_tRowID<b.m_tRowID || ( a.m_tRowID==b.m_tRowID && a.m_uHitpos<b.m_uHitpos ) || ( a.m_tRowID==b.m_tRowID && a.m_uHitpos==b.m_uHitpos && a.m_uQuerypos>b.m_uQuerypos ) );
	}
};


void ExtAnd_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pCurL = m_pLeft->GetHits(pDocs);
	const ExtHit_t * pCurR = m_pRight->GetHits(pDocs);
	const WORD uNodePosL = m_uNodePosL;
	const WORD uNodePosR = m_uNodePosR;

	RowID_t tMatchedRowID = INVALID_ROWID;

	while ( HasHits(pCurL) && HasHits(pCurR) )
	{
		bool bLeft = false;

		if ( pCurL->m_tRowID < pCurR->m_tRowID )
		{
			if ( pCurL->m_tRowID==tMatchedRowID )
				m_dHits.Add ( *pCurL++ );
			else
			{
				pCurL++;
				continue;
			}

			bLeft = true;
		}
		else if ( pCurL->m_tRowID > pCurR->m_tRowID )
		{
			if ( pCurR->m_tRowID==tMatchedRowID )
				m_dHits.Add ( *pCurR++ );
			else
			{
				pCurR++;
				continue;
			}
		}
		else
		{
			tMatchedRowID = pCurL->m_tRowID;

			if ( IsHitLess ( pCurL, pCurR ) )
			{
				m_dHits.Add ( *pCurL++ );
				bLeft = true;
			}
			else
				m_dHits.Add ( *pCurR++ );
		}

		if ( bLeft )
		{
			if ( uNodePosL!=0 )
				m_dHits.Last().m_uNodepos = uNodePosL;
		}
		else
		{
			if ( uNodePosR!=0 )
				m_dHits.Last().m_uNodepos = uNodePosR;
		}
	}

	while ( HasHits(pCurL) && pCurL->m_tRowID==tMatchedRowID )
	{
		m_dHits.Add ( *pCurL++ );
		if ( uNodePosL!=0 )
			m_dHits.Last().m_uNodepos = uNodePosL;
	}

	while ( HasHits(pCurR) && pCurR->m_tRowID==tMatchedRowID )
	{
		m_dHits.Add ( *pCurR++ );
		if ( uNodePosR!=0 )
			m_dHits.Last().m_uNodepos = uNodePosR;
	}

	if ( m_bQPosReverse )
		m_dHits.Sort ( CmpAndHitReverse_fn() );
}


NodeEstimate_t ExtAnd_c::Estimate ( int64_t iTotalDocs ) const
{
	assert ( m_pLeft && m_pRight );

	auto tLeftEstimate = m_pLeft->Estimate(iTotalDocs);
	auto tRightEstimate = m_pRight->Estimate(iTotalDocs);

	float fRatio = float(tLeftEstimate.m_fCost)/iTotalDocs*float(tRightEstimate.m_fCost)/iTotalDocs;
	float fCost = CalcFTIntersectCost ( tLeftEstimate, tRightEstimate, iTotalDocs, MAX_BLOCK_DOCS, MAX_BLOCK_DOCS );
	return { fCost, int64_t(fRatio*iTotalDocs), tLeftEstimate.m_iTerms+tRightEstimate.m_iTerms };
}


void ExtAnd_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtAnd", iLevel );
}

//////////////////////////////////////////////////////////////////////////

template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::NodeInfo_t::UpdateWideFieldFlag ( const ISphQwordSetup & tSetup )
{
	m_bHasWideFields = false;
	if ( tSetup.m_bHasWideFields )
		for ( int i=1; i<FieldMask_t::SIZE && !m_bHasWideFields; i++ )
			if ( m_dQueriedFields[i] )
				m_bHasWideFields = true;
}

template <bool USE_BM25,bool TEST_FIELDS>
inline bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::NodeInfo_t::FitsFields() const
{
	if ( !m_bHasWideFields )
	{
		// fields 0-31 can be quickly checked right here, right now
		if (!( m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32() ))
			return false;
	} else
	{
		// fields 32+ need to be checked with CollectHitMask() and stuff
		m_pQword->CollectHitMask();
		bool bHasSameFields = false;
		for ( int i=0; i<FieldMask_t::SIZE && !bHasSameFields; i++ )
			bHasSameFields = ( m_pQword->m_dQwordFields[i] & m_dQueriedFields[i] )!=0;

		if ( !bHasSameFields )
			return false;
	}

	return true;
}


template <bool USE_BM25,bool TEST_FIELDS>
bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::SelectivitySorter_t::IsLess ( const NodeInfo_t & tA, const NodeInfo_t & tB ) const
{
	return tA.m_pQword->m_iDocs < tB.m_pQword->m_iDocs;
}

template <bool USE_BM25,bool TEST_FIELDS>
ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::HitWithQpos_t::HitWithQpos_t ( int iNode, Hitpos_t uHit, WORD uQueryPos )
	: m_iNode ( iNode )
	, m_uHit ( uHit )
	, m_uQueryPos ( uQueryPos )
{}


template <bool USE_BM25,bool TEST_FIELDS>
inline bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::HitWithQpos_t::IsLess ( const HitWithQpos_t & a, const HitWithQpos_t & b )
{
	return ( a.m_uHit<b.m_uHit ) || ( a.m_uHit==b.m_uHit && a.m_uQueryPos<=b.m_uQueryPos );
}


template <bool USE_BM25,bool TEST_FIELDS>
ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::ExtMultiAnd_T ( const VecTraits_T<XQNode_t*> & dXQNodes, const ISphQwordSetup & tSetup )
	: m_dWordIds ( dXQNodes.GetLength() )
	, m_tQueue ( dXQNodes.GetLength() )
{
	m_dNodes.Resize ( dXQNodes.GetLength() );
	ARRAY_FOREACH ( i, m_dNodes )
	{
		NodeInfo_t & tNode = m_dNodes[i];
		const XQNode_t & tXQNode = *dXQNodes[i];

		tNode.m_pQword = CreateQueryWord ( tXQNode.m_dWords[0], tSetup );
		assert ( tNode.m_pQword );
		tNode.m_iAtomPos = tNode.m_pQword->m_iAtomPos;
		tNode.m_uNodepos = (WORD)i;
		tNode.m_bNotWeighted = tXQNode.m_bNotWeighted;
		tNode.m_dQueriedFields = tXQNode.m_dSpec.m_dFieldMask;
		tNode.UpdateWideFieldFlag(tSetup);
	}

	m_dNodes.Sort ( SelectivitySorter_t() );
	m_iNodesSet = m_dNodes.GetLength();

	m_pWarning = tSetup.m_pWarning;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pStats = tSetup.m_pStats;
	m_pNanoBudget = m_pStats ? m_pStats->m_pNanoBudget : NULL;
}


template <bool USE_BM25,bool TEST_FIELDS>
ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::~ExtMultiAnd_T()
{
	for ( auto & i : m_dNodes )
		SafeDelete ( i.m_pQword );
}


template <bool USE_BM25,bool TEST_FIELDS>
inline DWORD ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::GetDocFieldsMask() const
{
	DWORD uMask = 0;
	for ( const auto & i : m_dNodes )
		uMask |= i.m_pQword->m_dQwordFields.GetMask32() & i.m_dQueriedFields.GetMask32();

	return uMask;
}


template <bool USE_BM25,bool TEST_FIELDS>
inline float ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::GetTFIDF() const
{
	float fTFIDF = 0.0f;

	if_const ( USE_BM25 )
	{
		for ( const auto & i : m_dNodes )	
			fTFIDF += float(i.m_pQword->m_uMatchHits) / float(i.m_pQword->m_uMatchHits+SPH_BM25_K1) * i.m_fIDF;
	}

	return fTFIDF;
}


template <bool USE_BM25,bool TEST_FIELDS>
inline RowID_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::Advance ( int iNode )
{
	NodeInfo_t & tNode = m_dNodes[iNode];
	do
	{
		tNode.m_tRowID = tNode.m_pQword->GetNextDoc().m_tRowID;
	}
	while ( tNode.m_tRowID!=INVALID_ROWID && !tNode.FitsFields() );

	return tNode.m_tRowID;
}


template <bool USE_BM25,bool TEST_FIELDS>
inline RowID_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::Advance ( int iNode, RowID_t tRowID )
{
	NodeInfo_t & tNode = m_dNodes[iNode];
	if ( tRowID==tNode.m_tRowID )
		return tRowID;

	tNode.m_tRowID = tNode.m_pQword->AdvanceTo ( tRowID );
	while ( tNode.m_tRowID!=INVALID_ROWID && !tNode.FitsFields() )
		tNode.m_tRowID = tNode.m_pQword->GetNextDoc().m_tRowID;

	return tNode.m_tRowID;
}


template <bool USE_BM25,bool TEST_FIELDS>
inline bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::AdvanceQwords()
{
	RowID_t tMaxRowID = m_dNodes[0].m_tRowID;
	for ( int i=1; i < m_dNodes.GetLength(); i++ )
	{
		NodeInfo_t & tCurNode = m_dNodes[i];
		if ( tCurNode.m_tRowID==tMaxRowID )
			continue;
		
		Advance ( i, tMaxRowID );
		
		if ( tCurNode.m_tRowID==INVALID_ROWID )
			return false;
		else if ( tCurNode.m_tRowID>tMaxRowID )
		{
			if ( Advance ( 0, tCurNode.m_tRowID )==INVALID_ROWID )
				return false;

			tMaxRowID = m_dNodes[0].m_tRowID;
			i = 0;
		}
	}

	return true;
}


template <bool USE_BM25,bool TEST_FIELDS>
const ExtDoc_t * ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::GetDocsChunk()
{
	// since we're working directly with qwords, we need to check all those things here and not in ExtTerm
	// max_query_time
	if ( sph::TimeExceeded ( m_iMaxTimer ) )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	// max_predicted_time
	if ( m_pNanoBudget && *m_pNanoBudget<0 )
	{
		if ( m_pWarning )
			*m_pWarning = "predicted query time exceeded max_predicted_time";
		return nullptr;
	}

	if ( sph::TimeExceeded ( m_iCheckTimePoint ) )
	{
		// interrupt by sitgerm
		if ( g_bInterruptNow )
		{
			if ( m_pWarning )
				*m_pWarning = "Server shutdown in progress";
			return nullptr;
		}

		if ( session::GetKilled() )
		{
			if ( m_pWarning )
				*m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	if ( m_bFirstChunk )
	{
		if ( m_iNodesSet!=m_dNodes.GetLength() || !m_dNodes[0].m_pQword->m_iDocs )
			return nullptr;

		Advance(0);
		m_bFirstChunk = false;
	}

	StoredMultiHit_t * pStoredHit = nullptr;
	StoredMultiHit_t * pFirstHit = nullptr;
	if ( m_bCollectHits )
	{
		int iLength = m_dStoredHits.GetLength();
		m_dStoredHits.Reserve ( iLength+MAX_BLOCK_DOCS );
		pStoredHit = m_dStoredHits.End();
		pFirstHit = pStoredHit-iLength;	// hack to get to m_pData
	}

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( m_dNodes[0].m_tRowID==INVALID_ROWID )
			break;

		if ( !AdvanceQwords() )
		{
			m_dNodes[0].m_tRowID=INVALID_ROWID;
			break;
		}

		RowID_t tMatchedRowID = m_dNodes[0].m_tRowID;

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_tRowID = tMatchedRowID;
		tDoc.m_uDocFields = GetDocFieldsMask();
		tDoc.m_fTFIDF = GetTFIDF();

		if ( m_bCollectHits )
		{
			pStoredHit->m_tRowID = tMatchedRowID;
			pStoredHit->m_dHitlistOffsets.Reset(m_dNodes.GetLength());
			ARRAY_FOREACH ( i, m_dNodes )
				pStoredHit->m_dHitlistOffsets[i] = m_dNodes[i].m_pQword->m_iHitlistPos;

			pStoredHit++;
		}

		// we assume that the 1st node returns the least docs
		Advance(0);
	}

	if ( m_bCollectHits )
		m_dStoredHits.Resize ( pStoredHit-pFirstHit );

	if ( m_pStats )
		m_pStats->m_iFetchedDocs += iDoc;
	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostDoc*iDoc;

	return ReturnDocsChunk ( iDoc, "multiand" );
}


static inline bool IsHitLess ( Hitpos_t uHitposL, WORD uQueryPosL, Hitpos_t uHitposR, WORD uQueryPosR )
{
	return uHitposL<uHitposR || ( uHitposL==uHitposR && uQueryPosL<=uQueryPosR );
}


struct HitWithQpos_t
{
	int			m_iNode;
	Hitpos_t	m_uHit;
	WORD		m_uQueryPos;

	HitWithQpos_t ( int iNode, Hitpos_t uHit, WORD uQueryPos )
		: m_iNode ( iNode )
		, m_uHit ( uHit )
		, m_uQueryPos ( uQueryPos )
	{}

	
	bool operator < ( const HitWithQpos_t & rhs ) const
	{
		return IsHitLess ( rhs.m_uHit, rhs.m_uQueryPos, m_uHit, m_uQueryPos );
	}
};


template <bool USE_BM25,bool TEST_FIELDS>
inline bool ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::IsHitLess ( const HitInfo_t & tLeft, const HitInfo_t & tRight )
{
	return tLeft.m_uHitpos<tRight.m_uHitpos || ( tLeft.m_uHitpos==tRight.m_uHitpos && tLeft.m_uQueryPos<=tRight.m_uQueryPos );
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::InitHitMerge ( HitInfo_t & tHitInfo, int iNode, const StoredMultiHit_t & tStoredHit )
{
	const NodeInfo_t & tNode = m_dNodes[iNode];

	tHitInfo.m_pQword = tNode.m_pQword;
	assert ( tHitInfo.m_pQword );
	tHitInfo.m_uNodePos = tNode.m_uNodepos;
	tHitInfo.m_uQueryPos = (WORD)tNode.m_iAtomPos;
	tHitInfo.m_pQword->SeekHitlist ( tStoredHit.m_dHitlistOffsets[iNode] );
	tHitInfo.m_uHitpos = tHitInfo.m_pQword->GetNextHit();
}


template <bool USE_BM25,bool TEST_FIELDS>
inline void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::AddHit ( RowID_t tRowID, HitInfo_t & tHit, int iNode )
{
	if_const(TEST_FIELDS)
	{
		if ( m_dNodes[iNode].m_dQueriedFields.Test ( HITMAN::GetField ( tHit.m_uHitpos ) ) )
			m_dHits.Add ( ExtHit_t { tRowID, tHit.m_uHitpos, tHit.m_uQueryPos, tHit.m_uNodePos, 1, 1, 1, 0 } );
	}
	else
		m_dHits.Add ( ExtHit_t { tRowID, tHit.m_uHitpos, tHit.m_uQueryPos, tHit.m_uNodePos, 1, 1, 1, 0 } );

	tHit.m_uHitpos = tHit.m_pQword->GetNextHit();
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::DoHitMerge ( RowID_t tRowID, HitInfo_t & tLeft, HitInfo_t & tRight )
{
	while ( tLeft.m_uHitpos!=EMPTY_HIT && tRight.m_uHitpos!=EMPTY_HIT )
	{		
		if ( IsHitLess ( tLeft, tRight ) )
			AddHit ( tRowID, tLeft, 0 );
		else
			AddHit ( tRowID, tRight, 1 );
	}
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::DoHitMerge ( RowID_t tRowID, HitInfo_t & tHit1, HitInfo_t & tHit2, HitInfo_t & tHit3 )
{
	while ( tHit1.m_uHitpos!=EMPTY_HIT && tHit2.m_uHitpos!=EMPTY_HIT && tHit3.m_uHitpos!=EMPTY_HIT )
	{
		if ( IsHitLess ( tHit1, tHit2 ) && IsHitLess ( tHit1, tHit3 ) )
			AddHit ( tRowID, tHit1, 0 );
		else if ( IsHitLess ( tHit2, tHit1 ) && IsHitLess ( tHit2, tHit3 ) )
			AddHit ( tRowID, tHit2, 1 );
		else
			AddHit ( tRowID, tHit3, 2 );
	}

	if ( tHit1.m_uHitpos==EMPTY_HIT )
		DoHitMerge ( tRowID, tHit2, tHit3 );
	else if ( tHit2.m_uHitpos==EMPTY_HIT )
		DoHitMerge ( tRowID, tHit1, tHit3 );
	else
		DoHitMerge ( tRowID, tHit1, tHit2 );
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::CopyHits ( RowID_t tRowID, HitInfo_t & tHitInfo, int iNode )
{
	while ( tHitInfo.m_uHitpos!=EMPTY_HIT )
	{
		if_const(TEST_FIELDS)
		{
			if ( m_dNodes[iNode].m_dQueriedFields.Test ( HITMAN::GetField ( tHitInfo.m_uHitpos ) ) )
				m_dHits.Add ( ExtHit_t { tRowID, tHitInfo.m_uHitpos, tHitInfo.m_uQueryPos, tHitInfo.m_uNodePos, 1, 1, 1, 0 } );
		}
		else
			m_dHits.Add ( ExtHit_t { tRowID, tHitInfo.m_uHitpos, tHitInfo.m_uQueryPos, tHitInfo.m_uNodePos, 1, 1, 1, 0 } );

		tHitInfo.m_uHitpos = tHitInfo.m_pQword->GetNextHit();
	}
}

template <bool USE_BM25,bool TEST_FIELDS>
inline void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::MergeHits2 ( const StoredMultiHit_t & tStoredHit )
{
	const int NUM_STREAMS = 2;
	HitInfo_t dHits[NUM_STREAMS];
	RowID_t tRowID = tStoredHit.m_tRowID;

	for ( int i = 0; i < NUM_STREAMS; i++ )
		InitHitMerge ( dHits[i], i, tStoredHit );

	DoHitMerge ( tRowID, dHits[0], dHits[1] );

	for ( int i = 0; i < NUM_STREAMS; i++ )
		CopyHits ( tRowID, dHits[i], i );
}


template <bool USE_BM25,bool TEST_FIELDS>
inline void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::MergeHits3 ( const StoredMultiHit_t & tStoredHit )
{
	const int NUM_STREAMS = 3;
	HitInfo_t dHits[NUM_STREAMS];
	RowID_t tRowID = tStoredHit.m_tRowID;

	for ( int i = 0; i < NUM_STREAMS; i++ )
		InitHitMerge ( dHits[i], i, tStoredHit );

	DoHitMerge ( tRowID, dHits[0], dHits[1], dHits[2] );

	for ( int i = 0; i < NUM_STREAMS; i++ )
		CopyHits ( tRowID, dHits[i], i );
}


template <bool USE_BM25,bool TEST_FIELDS>
inline void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::PushNextHit ( int iNode )
{
	NodeInfo_t & tNode = m_dNodes[iNode];
	while ( !tNode.m_bHitsOver )
	{
		Hitpos_t uHit = tNode.m_pQword->GetNextHit();
		if ( uHit==EMPTY_HIT )
			tNode.m_bHitsOver = true;
		else if ( tNode.m_dQueriedFields.Test ( HITMAN::GetField(uHit) ) )
		{
			m_tQueue.Push ( HitWithQpos_t ( iNode, uHit, (WORD)tNode.m_iAtomPos ) );
			break;
		}
	}
}


template <bool USE_BM25,bool TEST_FIELDS>
inline void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::MergeHitsN ( const StoredMultiHit_t & tStoredHit )
{
	// setup hitlist reader
	ARRAY_FOREACH ( i, m_dNodes )
	{
		m_dNodes[i].m_pQword->SeekHitlist ( tStoredHit.m_dHitlistOffsets[i] );
		m_dNodes[i].m_bHitsOver = false;
	}

	// merge hitlists from all nodes for a given rowid
	assert ( !m_tQueue.GetLength() );
	ARRAY_FOREACH ( i, m_dNodes )
		PushNextHit(i);

	while ( m_tQueue.GetLength() )
	{
		const HitWithQpos_t & tHitWithQpos = m_tQueue.Root();
		int iNode = tHitWithQpos.m_iNode;
		NodeInfo_t & tNode = m_dNodes[iNode];
		ExtHit_t & tHit = m_dHits.Add();
		tHit.m_tRowID = tStoredHit.m_tRowID;
		tHit.m_uHitpos = tHitWithQpos.m_uHit;
		tHit.m_uQuerypos = tHitWithQpos.m_uQueryPos; // assume less that 64K words per query
		tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
		tHit.m_uNodepos = tNode.m_uNodepos;

		m_tQueue.Pop();

		PushNextHit(iNode);
	}
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return;

	m_dStoredHits.Add().m_tRowID = INVALID_ROWID;
	StoredMultiHit_t * pStoredHit = m_dStoredHits.Begin();

	for ( ; HasDocs(pMatched); pMatched++ )
	{
		while ( pStoredHit->m_tRowID < pMatched->m_tRowID )
			pStoredHit++;

		if ( pStoredHit->m_tRowID!=pMatched->m_tRowID )
			continue;

		switch ( m_dNodes.GetLength() )
		{
		case 2:		MergeHits2 ( *pStoredHit );	break;
		case 3:		MergeHits3 ( *pStoredHit );	break;
		default:	MergeHitsN ( *pStoredHit );	break;
		}
	}

	int nHits = m_dHits.GetLength();
	if ( m_pStats )
		m_pStats->m_iFetchedHits += nHits;

	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostHit*nHits;

	// look at ExtTerm_T for more info on this code
	int nProcessed = int ( pStoredHit-m_dStoredHits.Begin() );
	m_dStoredHits.Pop();	// end marker
	m_dStoredHits.Remove ( 0, nProcessed );

	if ( m_bQPosReverse )
		m_dHits.Sort ( CmpAndHitReverse_fn() );
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::Reset ( const ISphQwordSetup & tSetup )
{
	m_bFirstChunk = true;
	m_iNodesSet = 0;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	for ( auto & i : m_dNodes )
	{
		i.m_tRowID = INVALID_ROWID;
		i.m_bHitsOver = false;
		i.m_pQword->Reset ();
		// need to track active nodes for every segment
		// however AND requires all nodes that is why can use fast reject
		if ( tSetup.QwordSetup ( i.m_pQword ) )
			m_iNodesSet++;
	}
	
	m_dStoredHits.Resize(0);
}


template <bool USE_BM25,bool TEST_FIELDS>
int ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::GetQword ( NodeInfo_t & tNode, ExtQwordsHash_t & hQwords )
{
	tNode.m_fIDF = 0.0f;

	ExtQword_t * pQword = hQwords ( tNode.m_pQword->m_sWord );
	if ( !tNode.m_bNotWeighted && pQword && !pQword->m_bExcluded )
		pQword->m_iQueryPos = Min ( pQword->m_iQueryPos, tNode.m_pQword->m_iAtomPos );

	if ( tNode.m_bNotWeighted || pQword )
		return tNode.m_pQword->m_bExcluded ? -1 : tNode.m_pQword->m_iAtomPos;

	tNode.m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = tNode.m_pQword->m_sWord;
	tInfo.m_sDictWord = tNode.m_pQword->m_sDictWord;
	tInfo.m_iDocs = tNode.m_pQword->m_iDocs;
	tInfo.m_iHits = tNode.m_pQword->m_iHits;
	tInfo.m_iQueryPos = tNode.m_pQword->m_iAtomPos;
	tInfo.m_fIDF = -1.0f; // suppress gcc 4.2.3 warning
	tInfo.m_fBoost = tNode.m_pQword->m_fBoost;
	tInfo.m_bExpanded = tNode.m_pQword->m_bExpanded;
	tInfo.m_bExcluded = tNode.m_pQword->m_bExcluded;
	hQwords.Add ( tInfo, tNode.m_pQword->m_sWord );
	return tNode.m_pQword->m_bExcluded ? -1 : tNode.m_pQword->m_iAtomPos;
}


template <bool USE_BM25,bool TEST_FIELDS>
int ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	for ( auto & i : m_dNodes )
	{
		int iRes = GetQword ( i, hQwords );
		iMax = Max ( iRes, iMax );
	}

	return iMax;
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	for ( auto & i : m_dNodes )
		if ( i.m_fIDF<0.0f )
		{
			assert ( hQwords ( i.m_pQword->m_sWord ) );
			i.m_fIDF = hQwords ( i.m_pQword->m_sWord )->m_fIDF;
		}
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	for ( const auto & i : m_dNodes )
		if ( i.m_bNotWeighted || !i.m_pQword->m_bExcluded )
		{
			ExtQword_t & tQword = hQwords[i.m_pQword->m_sWord];

			TermPos_t & tPos = dTermDupes.Add();
			tPos.m_uAtomPos = (WORD)i.m_pQword->m_iAtomPos;
			tPos.m_uQueryPos = (WORD)tQword.m_iQueryPos;
		}
}


template <bool USE_BM25,bool TEST_FIELDS>
uint64_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::GetWordID() const
{
	ARRAY_FOREACH ( i, m_dNodes )
	{
		NodeInfo_t & tNode = m_dNodes[i];
		if ( tNode.m_pQword->m_uWordID ) 
			m_dWordIds[i] = tNode.m_pQword->m_uWordID;
		else
			m_dWordIds[i] = sphFNV64 ( tNode.m_pQword->m_sDictWord.cstr() );
	}

	return sphFNV64 ( m_dWordIds.Begin(), (int) m_dWordIds.GetLengthBytes() );
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::HintRowID ( RowID_t tRowID )
{
	if ( !m_dNodes[0].m_pQword->m_iDocs )
		return;

	if ( m_bFirstChunk || ( m_dNodes[0].m_tRowID!=INVALID_ROWID && tRowID>m_dNodes[0].m_tRowID ) )
	{
		if ( m_bFirstChunk && m_iNodesSet!=m_dNodes.GetLength() )
			return;

		Advance ( 0, tRowID );
		m_bFirstChunk = false;
	}
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::SetCollectHits()
{
	m_bCollectHits = true;
}


template <bool USE_BM25,bool TEST_FIELDS>
void ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtMultiAnd\n" );

	DebugIndent ( iLevel+1 );
	for ( const auto & i : m_dNodes )
	{
		printf ( "%s at: %d ", i.m_pQword->m_sWord.cstr(), i.m_pQword->m_iAtomPos );

		if ( i.m_dQueriedFields.TestAll(true) )
			printf ( "(all)\n" );
		else
		{
			bool bFirst = true;
			printf ( "in: " );
			for ( int iField=0; iField<SPH_MAX_FIELDS; iField++ )
			{
				if ( i.m_dQueriedFields.Test ( iField ) )
				{
					if ( !bFirst )
						printf ( ", " );
					printf ( "%d", iField );
					bFirst = false;
				}
			}
			printf ( "\n" );
		}
	}	
}


static float CalcQwordReadCost ( ISphQword * pQword )
{
	assert(pQword);
	return float(pQword->m_iDocs)*COST_SCALE*55.0f;
}

template <bool USE_BM25,bool TEST_FIELDS>
NodeEstimate_t ExtMultiAnd_T<USE_BM25,TEST_FIELDS>::Estimate ( int64_t iTotalDocs ) const
{
	float fRatio = 1.0f;
	float fCostLeft = 0.0f;
	ARRAY_FOREACH ( i, m_dNodes )
	{
		const auto & tNode = m_dNodes[i];
		assert(tNode.m_pQword);

		if (!i)
			fCostLeft = CalcQwordReadCost ( tNode.m_pQword );
		else
		{
			float fCostRight = CalcQwordReadCost ( tNode.m_pQword );
			NodeEstimate_t tEst1 = { fCostLeft, int64_t(fRatio*iTotalDocs), i };
			NodeEstimate_t tEst2 = { fCostRight, tNode.m_pQword->m_iDocs, 1 };
			fCostLeft = CalcFTIntersectCost ( tEst1, tEst2, iTotalDocs, MAX_BLOCK_DOCS, MAX_BLOCK_DOCS );
		}

		fRatio *= float(tNode.m_pQword->m_iDocs) / iTotalDocs;
	}

	return { fCostLeft, int64_t(fRatio*iTotalDocs), m_dNodes.GetLength() };
}

//////////////////////////////////////////////////////////////////////////

bool ExtAndZonespanned_c::IsSameZonespan ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 ) const
{
	for ( auto iZone : m_dZones )
	{
		int iSpan1, iSpan2;
		if ( m_pZoneChecker->IsInZone ( iZone, pHit1, &iSpan1 )==SPH_ZONE_FOUND && m_pZoneChecker->IsInZone ( iZone, pHit2, &iSpan2 )==SPH_ZONE_FOUND )
		{
			assert ( iSpan1>=0 && iSpan2>=0 );
			if ( iSpan1==iSpan2 )
				return true;
		}
	}
	return false;
}

void ExtAndZonespanned_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pCurL = m_pLeft->GetHits(pDocs);
	const ExtHit_t * pCurR = m_pRight->GetHits(pDocs);
	const WORD uNodePosL = m_uNodePosL;
	const WORD uNodePosR = m_uNodePosR;

	while ( HasHits(pCurL) && HasHits(pCurR) )
	{
		if ( pCurL->m_tRowID < pCurR->m_tRowID )
			pCurL++;
		else if ( pCurL->m_tRowID > pCurR->m_tRowID )
			pCurR++;
		else
		{
			if ( IsHitLess ( pCurL, pCurR ) )
			{
				if ( IsSameZonespan ( pCurL, pCurR ) )
				{
					m_dHits.Add ( *pCurL );
					if ( uNodePosL!=0 )
						m_dHits.Last().m_uNodepos = uNodePosL;
				}

				pCurL++;
			}
			else
			{
				if ( IsSameZonespan ( pCurL, pCurR ) )
				{
					m_dHits.Add ( *pCurR );
					if ( uNodePosR!=0 )
						m_dHits.Last().m_uNodepos = uNodePosR;	
				}

				pCurR++;
			}
		}
	}

	if ( m_bQPosReverse )
		m_dHits.Sort ( CmpAndHitReverse_fn() );
}


void ExtAndZonespanned_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtAndZonespan", iLevel );
}

//////////////////////////////////////////////////////////////////////////

ExtOr_c::ExtOr_c ( ExtNode_i * pLeft, ExtNode_i * pRight )
	: ExtTwofer_c ( pLeft, pRight )
{}

const ExtDoc_t * ExtOr_c::GetDocsChunk()
{
	int iDoc = 0;

	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !HasDocs(pDocL) )
			pDocL = m_pLeft->GetDocsChunk();

		if ( !HasDocs(pDocR) )
			pDocR = m_pRight->GetDocsChunk();

		if ( !HasDocs(pDocL) && !HasDocs(pDocR) )
			break;

		ExtDoc_t & tNewDoc = m_dDocs[iDoc];

		// merge lists while we can, copy tail while if we can not
		if ( HasDocs(pDocL) && HasDocs(pDocR) )
		{
			if ( pDocL->m_tRowID==pDocR->m_tRowID )
			{
				tNewDoc = *pDocL;
				tNewDoc.m_uDocFields = pDocL->m_uDocFields | pDocR->m_uDocFields; // not necessary
				tNewDoc.m_fTFIDF = pDocL->m_fTFIDF + pDocR->m_fTFIDF;
				pDocL++;
				pDocR++;
			}
			else if ( pDocL->m_tRowID<pDocR->m_tRowID )
				tNewDoc = *pDocL++;
			else
				tNewDoc = *pDocR++;
		}
		else if ( HasDocs(pDocL) )
			tNewDoc = *pDocL++;
		else
			tNewDoc = *pDocR++;

		iDoc++;
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "or" );
}


void ExtOr_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pCurL = m_pLeft->GetHits(pDocs);
	const ExtHit_t * pCurR = m_pRight->GetHits(pDocs);

	// merge, while possible
	while ( HasHits(pCurL) && HasHits(pCurR) )
	{
		if ( pCurL->m_tRowID < pCurR->m_tRowID )
			m_dHits.Add ( *pCurL++ );
		else if ( pCurL->m_tRowID > pCurR->m_tRowID )
			m_dHits.Add ( *pCurR++ );
		else
		{
			if ( IsHitLess ( pCurL, pCurR ) )
				m_dHits.Add ( *pCurL++ );
			else
				m_dHits.Add ( *pCurR++ );
		}
	}

	while ( HasHits(pCurL) )
		m_dHits.Add ( *pCurL++ );

	while ( HasHits(pCurR) )
		m_dHits.Add ( *pCurR++ );
}


void ExtOr_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtOr", iLevel );
}


NodeEstimate_t ExtOr_c::Estimate ( int64_t iTotalDocs ) const
{
	assert ( m_pLeft && m_pRight );

	auto tLeftEstimate = m_pLeft->Estimate(iTotalDocs);
	auto tRightEstimate = m_pRight->Estimate(iTotalDocs);

	float fIntersection = float(tLeftEstimate.m_iDocs)/iTotalDocs*float(tRightEstimate.m_iDocs)/iTotalDocs;
	int64_t iIntersectionDocs = int64_t(fIntersection*iTotalDocs);

	int64_t iResDocs = tLeftEstimate.m_iDocs+tRightEstimate.m_iDocs>=iIntersectionDocs ? tLeftEstimate.m_iDocs+tRightEstimate.m_iDocs-iIntersectionDocs : iIntersectionDocs;

	float fMergeCost = float(tLeftEstimate.m_iDocs + tRightEstimate.m_iDocs)*COST_SCALE*10.0f;
	return { tLeftEstimate.m_fCost + tRightEstimate.m_fCost + fMergeCost, iResDocs, tLeftEstimate.m_iTerms + tRightEstimate.m_iTerms };
}

//////////////////////////////////////////////////////////////////////////

ExtMaybe_c::ExtMaybe_c ( ExtNode_i * pLeft, ExtNode_i * pRight )
	: ExtOr_c ( pLeft, pRight )
{}

// returns documents from left subtree only
//
// each call returns only one document and rewinds docs in rhs to look for the
// same docID as in lhs
//
// we do this to return hits from rhs too which we need to affect match rank
const ExtDoc_t * ExtMaybe_c::GetDocsChunk()
{
	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	int iDoc = 0;
	bool bRightEmpty = false;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, m_pLeft.get() ) )
			break;

		if ( !bRightEmpty )
			bRightEmpty = !WarmupDocs ( pDocR, m_pRight.get() );

		if ( !bRightEmpty )
		{
			if ( pDocL->m_tRowID==pDocR->m_tRowID )
			{
				m_dDocs[iDoc] = *pDocL;
				m_dDocs[iDoc].m_uDocFields = pDocL->m_uDocFields | pDocR->m_uDocFields;
				m_dDocs[iDoc].m_fTFIDF = pDocL->m_fTFIDF + pDocR->m_fTFIDF;
				iDoc++;
				pDocL++;
				pDocR++;
			}
			else if ( pDocL->m_tRowID<pDocR->m_tRowID )
				m_dDocs[iDoc++] = *pDocL++;
			else
				pDocR++;
		}
		else
			m_dDocs[iDoc++] = *pDocL++;
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "maybe" );
}


void ExtMaybe_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtMaybe", iLevel );
}

//////////////////////////////////////////////////////////////////////////

ExtAndNot_c::ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond )
	: ExtTwofer_c ( pFirst, pSecond )
{}

const ExtDoc_t * ExtAndNot_c::GetDocsChunk()
{
	// if reject-list is over, simply pass through to accept-list
	if ( m_bPassthrough )
		return m_pLeft->GetDocsChunk();

	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, m_pLeft.get() ) )
			break;

		WarmupDocs ( pDocR, m_pRight.get() );

		// if there's nothing to filter against, simply copy leftovers
		if ( !HasDocs(pDocR) )
		{
			while ( HasDocs(pDocL) && iDoc<MAX_BLOCK_DOCS-1 )
				m_dDocs[iDoc++] = *pDocL++;

			m_bPassthrough = !HasDocs(pDocL);
			break;
		}

		// perform filtering
		assert ( pDocL );
		assert ( pDocR );
		while (true)
		{
			assert ( iDoc<MAX_BLOCK_DOCS-1 );
			assert ( HasDocs(pDocL) && HasDocs(pDocR) );

			// copy accepted until min rejected id
			while ( pDocL->m_tRowID < pDocR->m_tRowID && iDoc<MAX_BLOCK_DOCS-1 )
				m_dDocs[iDoc++] = *pDocL++;

			if ( !HasDocs(pDocL) || iDoc==MAX_BLOCK_DOCS-1 )
				break;

			// skip rejected until min accepted id
			while ( pDocR->m_tRowID < pDocL->m_tRowID )
				pDocR++;

			if ( !HasDocs(pDocR) )
				break;

			// skip both while ids match
			while ( pDocL->m_tRowID==pDocR->m_tRowID && HasDocs(pDocL) )
			{
				pDocL++;
				pDocR++;
			}

			if ( !HasDocs(pDocL) || !HasDocs(pDocR) )
				break;
		}
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;

	return ReturnDocsChunk ( iDoc, "andnot" );
}


void ExtAndNot_c::CollectHits ( const ExtDoc_t * pDocs )
{
	if ( !pDocs )
		return;

	const ExtHit_t * pHit = m_pLeft->GetHits(pDocs);
	while ( HasHits(pHit) )
		m_dHits.Add ( *pHit++ );
}

void ExtAndNot_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bPassthrough = false;
	ExtTwofer_c::Reset ( tSetup );
}

void ExtAndNot_c::SetCollectHits()
{
	m_pLeft->SetCollectHits();
	// m_pRight always ignores hits
}

void ExtAndNot_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtAndNot", iLevel );
}

//////////////////////////////////////////////////////////////////////////

ExtNWay_c::ExtNWay_c ( const CSphVector<ExtNode_i *> & dNodes, const ISphQwordSetup & tSetup )
{
	assert ( dNodes.GetLength()>1 );
	m_iAtomPos = dNodes[0]->GetAtomPos();
}

ExtNWay_c::~ExtNWay_c ()
{
	SafeDelete ( m_pNode );
}

void ExtNWay_c::Reset ( const ISphQwordSetup & tSetup )
{
	BufferedNode_c::Reset();

	m_pNode->Reset ( tSetup );
	m_pDocs = nullptr;
	m_pHits = nullptr;
}

int ExtNWay_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	return m_pNode->GetQwords ( hQwords );
}

void ExtNWay_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->SetQwordsIDF ( hQwords );
}

void ExtNWay_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	assert ( m_pNode );
	m_pNode->GetTerms ( hQwords, dTermDupes );
}


void ExtNWay_c::HintRowID ( RowID_t tRowID )
{
	m_pNode->HintRowID ( tRowID );
}


uint64_t ExtNWay_c::GetWordID() const
{
	assert ( m_pNode );
	return m_pNode->GetWordID();
}


inline void ExtNWay_c::ConstructNode ( const CSphVector<ExtNode_i *> & dNodes, const CSphVector<WORD> & dPositions, const ISphQwordSetup & tSetup )
{
	assert ( m_pNode==NULL );
	WORD uLPos = dPositions[0];
	ExtNode_i * pCur = dNodes[uLPos++]; // ++ for zero-based to 1-based
	ExtAnd_c * pCurEx = NULL;
	DWORD uLeaves = dNodes.GetLength();
	WORD uRPos;
	for ( DWORD i=1; i<uLeaves; i++ )
	{
		uRPos = dPositions[i];
		pCur = pCurEx = new ExtAnd_c ( pCur, dNodes[uRPos++] ); // ++ for zero-based to 1-based
		pCurEx->SetNodePos ( uLPos, uRPos );
		uLPos = 0;
	}
	if ( pCurEx )
		pCurEx->SetQPosReverse();

	pCur->SetCollectHits();
	m_pNode = pCur;
}


//////////////////////////////////////////////////////////////////////////

template < class FSM >
ExtNWay_T<FSM>::ExtNWay_T ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: ExtNWay_c ( dNodes, tSetup )
	, FSM ( dNodes, tNode, tSetup )
{
	CSphVector<WORD> dPositions ( dNodes.GetLength() );
	ARRAY_FOREACH ( i, dPositions )
		dPositions[i] = (WORD) i;
	dPositions.Sort ( ExtNodeTFExt_fn ( dNodes ) );
	ConstructNode ( dNodes, dPositions, tSetup );
}


template < class FSM >
const ExtDoc_t * ExtNWay_T<FSM>::GetDocsChunk()
{
	if ( !WarmupDocs ( m_pDocs, m_pHits, m_pNode ) )
		return nullptr;

	const ExtDoc_t * pDoc = m_pDocs;
	const ExtHit_t * pHit = m_pHits;

	FSM::ResetFSM();

	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		assert ( pHit->m_tRowID==pDoc->m_tRowID );

		FSM::ResetFSM();

		// iterate all hits for this doc
		while ( pHit->m_tRowID==pDoc->m_tRowID )
		{
			// emit document, if its new and acceptable
			if ( FSM::HitFSM ( pHit, m_dMyHits ) && ( !iDoc || pHit->m_tRowID!=m_dDocs[iDoc-1].m_tRowID ) )
			{
				m_dDocs[iDoc].m_tRowID = pHit->m_tRowID;
				m_dDocs[iDoc].m_uDocFields = 1<< ( HITMAN::GetField ( pHit->m_uHitpos ) ); // not necessary
				m_dDocs[iDoc].m_fTFIDF = pDoc->m_fTFIDF;
				iDoc++;
			}

			pHit++;
		}

		pDoc++;

		if ( !WarmupDocs ( pDoc, pHit, m_pNode ) )
			break;
	}

	m_pDocs = pDoc;
	m_pHits = pHit;

	return ReturnDocsChunk ( iDoc, "nway" );
}


template < class FSM >
void ExtNWay_T<FSM>::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
}


template < class FSM >
void ExtNWay_T<FSM>::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "%s\n", FSM::GetName() );
	m_pNode->DebugDump ( iLevel+1 );
}


//////////////////////////////////////////////////////////////////////////

static DWORD GetQposMask ( const CSphVector<ExtNode_i *> & dQwords )
{
	DWORD uQposMask = ( 1 << dQwords[0]->GetAtomPos() );
	for ( int i=1; i<dQwords.GetLength(); i++ )
	{
		int iQpos = dQwords[i]->GetAtomPos();
		assert ( iQpos<(int)sizeof(uQposMask)*8 );
		uQposMask |= ( 1 << iQpos );
	}

	return uQposMask;
}


FSMphrase_c::FSMphrase_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & , const ISphQwordSetup & tSetup )
	: m_dAtomPos ( dQwords.GetLength() )
{
	ARRAY_FOREACH ( i, dQwords )
		m_dAtomPos[i] = dQwords[i]->GetAtomPos();

	assert ( ( m_dAtomPos.Last()-m_dAtomPos[0]+1 )>0 );
	m_dQposDelta.Resize ( m_dAtomPos.Last()-m_dAtomPos[0]+1 );
	ARRAY_FOREACH ( i, m_dQposDelta )
		m_dQposDelta[i] = -INT_MAX;
	for ( int i=1; i<(int)m_dAtomPos.GetLength(); i++ )
		m_dQposDelta [ dQwords[i-1]->GetAtomPos() - dQwords[0]->GetAtomPos() ] = dQwords[i]->GetAtomPos() - dQwords[i-1]->GetAtomPos();

	if ( tSetup.m_bSetQposMask )
		m_uQposMask = GetQposMask ( dQwords );
}

inline bool FSMphrase_c::HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits )
{
	DWORD uHitposWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );

	// adding start state for start hit
	if ( pHit->m_uQuerypos==m_dAtomPos[0] )
	{
		State_t & tState = m_dStates.Add();
		tState.m_iTagQword = 0;
		tState.m_uExpHitposWithField = uHitposWithField + m_dQposDelta[0];
	}

	// updating states
	for ( int i=m_dStates.GetLength()-1; i>=0; i-- )
	{
		if ( m_dStates[i].m_uExpHitposWithField<uHitposWithField )
		{
			m_dStates.RemoveFast(i); // failed to match
			continue;
		}

		// get next state
		if ( m_dStates[i].m_uExpHitposWithField==uHitposWithField && m_dAtomPos [ m_dStates[i].m_iTagQword+1 ]==pHit->m_uQuerypos )
		{
			m_dStates[i].m_iTagQword++; // check for next elm in query
			m_dStates[i].m_uExpHitposWithField = uHitposWithField + m_dQposDelta [ pHit->m_uQuerypos - m_dAtomPos[0] ];
		}

		// checking if state successfully matched
		if ( m_dStates[i].m_iTagQword==m_dAtomPos.GetLength()-1 )
		{
			DWORD uSpanlen = m_dAtomPos.Last() - m_dAtomPos[0];

			ExtHit_t & tTarget = dHits.Add();
			tTarget.m_tRowID = pHit->m_tRowID;
			tTarget.m_uHitpos = uHitposWithField - uSpanlen;
			tTarget.m_uQuerypos = (WORD) m_dAtomPos[0];
			tTarget.m_uMatchlen = tTarget.m_uSpanlen = (WORD)( uSpanlen + 1 );
			tTarget.m_uWeight = m_dAtomPos.GetLength();
			tTarget.m_uQposMask = m_uQposMask;
			ResetFSM ();
			return true;
		}
	}

	return false;
}


inline void FSMphrase_c::ResetFSM()
{
	m_dStates.Resize(0);
}


//////////////////////////////////////////////////////////////////////////

FSMproximity_c::FSMproximity_c ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: m_iMaxDistance ( tNode.m_iOpArg )
	, m_uWordsExpected ( dQwords.GetLength() )
{
	assert ( m_iMaxDistance>0 );
	m_uMinQpos = dQwords[0]->GetAtomPos();
	m_uQLen = dQwords.Last()->GetAtomPos() - m_uMinQpos;
	m_dProx.Resize ( m_uQLen+1 );
	m_dDeltas.Resize ( m_uQLen+1 );

	if ( tSetup.m_bSetQposMask )
		m_uQposMask = GetQposMask ( dQwords );
}


inline bool FSMproximity_c::HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits )
{
	// walk through the hitlist and update context
	int iQindex = pHit->m_uQuerypos - m_uMinQpos;
	DWORD uHitposWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );

	// check if the word is new
	if ( m_dProx[iQindex]==UINT_MAX )
		m_uWords++;

	// update the context
	m_dProx[iQindex] = uHitposWithField;

	// check if the incoming hit is out of bounds, or affects min pos
	if ( uHitposWithField>=m_uExpPos // out of expected bounds
		|| iQindex==m_iMinQindex ) // or simply affects min pos
	{
		m_iMinQindex = iQindex;
		int iMinPos = uHitposWithField - m_uQLen - m_iMaxDistance;

		ARRAY_FOREACH ( i, m_dProx )
			if ( m_dProx[i]!=UINT_MAX )
			{
				if ( (int)m_dProx[i]<=iMinPos )
				{
					m_dProx[i] = UINT_MAX;
					m_uWords--;
					continue;
				}
				if ( m_dProx[i]<uHitposWithField )
				{
					m_iMinQindex = i;
					uHitposWithField = m_dProx[i];
				}
			}

		m_uExpPos = m_dProx[m_iMinQindex] + m_uQLen + m_iMaxDistance;
	}

	// all words were found within given distance?
	if ( m_uWords!=m_uWordsExpected )
		return false;

	// compute phrase weight
	//
	// FIXME! should also account for proximity factor, which is in 1 to maxdistance range:
	// m_iMaxDistance - ( pHit->m_uHitpos - m_dProx[m_iMinQindex] - m_uQLen )
	DWORD uMax = 0;
	ARRAY_FOREACH ( i, m_dProx )
		if ( m_dProx[i]!=UINT_MAX )
		{
			m_dDeltas[i] = m_dProx[i] - i;
			uMax = Max ( uMax, m_dProx[i] );
		} else
			m_dDeltas[i] = INT_MAX;

	m_dDeltas.Sort ();

	DWORD uCurWeight = 0;
	DWORD uWeight = 0;
	int iLast = -INT_MAX;
	ARRAY_FOREACH_COND ( i, m_dDeltas, m_dDeltas[i]!=INT_MAX )
	{
		if ( m_dDeltas[i]==iLast )
			uCurWeight++;
		else
		{
			uWeight += uCurWeight ? ( 1+uCurWeight ) : 0;
			uCurWeight = 0;
		}
		iLast = m_dDeltas[i];
	}

	uWeight += uCurWeight ? ( 1+uCurWeight ) : 0;
	if ( !uWeight )
		uWeight = 1;

	// emit hit
	ExtHit_t & tTarget = dHits.Add();
	tTarget.m_tRowID = pHit->m_tRowID;
	tTarget.m_uHitpos = Hitpos_t ( m_dProx[m_iMinQindex] ); // !COMMIT strictly speaking this is creation from LCS not value
	tTarget.m_uQuerypos = (WORD) m_uMinQpos;
	tTarget.m_uSpanlen = tTarget.m_uMatchlen = (WORD)( uMax-m_dProx[m_iMinQindex]+1 );
	tTarget.m_uWeight = uWeight;
	tTarget.m_uQposMask = m_uQposMask;

	// remove current min, and force recompue
	m_dProx[m_iMinQindex] = UINT_MAX;
	m_iMinQindex = -1;
	m_uWords--;
	m_uExpPos = 0;
	return true;
}


inline void FSMproximity_c::ResetFSM()
{
	m_uExpPos = 0;
	m_uWords = 0;
	m_iMinQindex = -1;
	ARRAY_FOREACH ( i, m_dProx )
		m_dProx[i] = UINT_MAX;
}


//////////////////////////////////////////////////////////////////////////

FSMmultinear_c::FSMmultinear_c ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: m_iNear ( tNode.m_iOpArg )
	, m_uWordsExpected ( dNodes.GetLength() )
	, m_bQposMask ( tSetup.m_bSetQposMask )
{
	if ( m_uWordsExpected==2 )
		m_bTwofer = true;
	else
	{
		m_dNpos.Reserve ( m_uWordsExpected );
		m_dRing.Resize ( m_uWordsExpected );
		m_bTwofer = false;
	}
	assert ( m_iNear>0 );
}

inline bool FSMmultinear_c::HitFSM ( const ExtHit_t * pHit, CSphVector<ExtHit_t> & dHits )
{
	// walk through the hitlist and update context
	DWORD uHitposWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );
	WORD uNpos = pHit->m_uNodepos;
	WORD uQpos = pHit->m_uQuerypos;

	// skip dupe hit (may be emitted by OR node, for example)
	if ( m_uLastP==uHitposWithField )
	{
		// lets choose leftmost (in query) from all dupes. 'a NEAR/2 a' case
		if ( m_bTwofer && uNpos<m_uFirstNpos )
		{
			m_uFirstQpos = uQpos;
			m_uFirstNpos = uNpos;
			return false;
		} else if ( !m_bTwofer && uNpos<m_dRing [ RingTail() ].m_uNodepos ) // 'a NEAR/2 a NEAR/2 a' case
		{
			WORD * p = const_cast<WORD *>( m_dNpos.BinarySearch ( uNpos ) );
			if ( !p )
			{
				p = const_cast<WORD *>( m_dNpos.BinarySearch ( m_dRing [ RingTail() ].m_uNodepos ) );
				*p = uNpos;
				m_dNpos.Sort();
				m_dRing [ RingTail() ].m_uNodepos = uNpos;
				m_dRing [ RingTail() ].m_uQuerypos = uQpos;
			}
			return false;
		} else if ( m_uPrelastP && m_uLastML < pHit->m_uMatchlen ) // check if the hit is subset of another one
		{
			// roll back pre-last to check agains this new hit.
			m_uLastML = m_uPrelastML;
			m_uLastSL = m_uPrelastSL;
			m_uFirstHit = m_uLastP = m_uPrelastP;
			m_uWeight = m_uWeight - m_uLastW + m_uPrelastW;
		} else
			return false;
	}

	// probably new chain
	if ( m_uLastP==0 || ( m_uLastP + m_uLastML + m_iNear )<=uHitposWithField )
	{
		m_uFirstHit = m_uLastP = uHitposWithField;
		m_uLastML = pHit->m_uMatchlen;
		m_uLastSL = pHit->m_uSpanlen;
		m_uWeight = m_uLastW = pHit->m_uWeight;
		m_uFirstQpos = uQpos;
		if ( m_bTwofer )
		{
			m_uFirstNpos = uNpos;
		} else
		{
			m_dNpos.Resize(1);
			m_dNpos[0] = uNpos;
			Add2Ring ( pHit );
		}
		return false;
	}

	// this hit (with such querypos) already was there. Skip the hit.
	if ( m_bTwofer )
	{
		// special case for twofer: hold the overlapping
		if ( ( m_uFirstHit + m_uLastML )>uHitposWithField
			&& ( m_uFirstHit + m_uLastML )<( uHitposWithField + pHit->m_uMatchlen )
			&& m_uLastML!=pHit->m_uMatchlen )
		{
			m_uFirstHit = m_uLastP = uHitposWithField;
			m_uLastML = pHit->m_uMatchlen;
			m_uLastSL = pHit->m_uSpanlen;
			m_uWeight = m_uLastW = pHit->m_uWeight;
			m_uFirstQpos = uQpos;
			m_uFirstNpos = uNpos;
			return false;
		}
		if ( uNpos==m_uFirstNpos )
		{
			if ( m_uLastP < uHitposWithField )
			{
				m_uPrelastML = m_uLastML;
				m_uPrelastSL = m_uLastSL;
				m_uPrelastP = m_uLastP;
				m_uPrelastW = pHit->m_uWeight;

				m_uFirstHit = m_uLastP = uHitposWithField;
				m_uLastML = pHit->m_uMatchlen;
				m_uLastSL = pHit->m_uSpanlen;
				m_uWeight = m_uLastW = m_uPrelastW;
				m_uFirstQpos = uQpos;
				m_uFirstNpos = uNpos;
			}
			return false;
		}
	} else
	{
		if ( uNpos < m_dNpos[0] )
		{
			m_uFirstQpos = Min ( m_uFirstQpos, uQpos );
			m_dNpos.Insert ( 0, uNpos );
		} else if ( uNpos > m_dNpos.Last() )
		{
			m_uFirstQpos = Min ( m_uFirstQpos, uQpos );
			m_dNpos.Add ( uNpos );
		} else if ( uNpos!=m_dNpos[0] && uNpos!=m_dNpos.Last() )
		{
			int iEnd = m_dNpos.GetLength();
			int iStart = 0;
			int iMid = -1;
			while ( iEnd-iStart>1 )
			{
				iMid = ( iStart + iEnd ) / 2;
				if ( uNpos==m_dNpos[iMid] )
				{
					const ExtHit_t& dHit = m_dRing[m_iRing];
					// last addition same as the first. So, we can shift
					if ( uNpos==dHit.m_uNodepos )
					{
						m_uWeight -= dHit.m_uWeight;
						m_uFirstHit = HITMAN::GetPosWithField ( dHit.m_uHitpos );
						ShiftRing();
					// last addition same as the first. So, we can shift
					} else if ( uNpos==m_dRing [ RingTail() ].m_uNodepos )
						m_uWeight -= m_dRing [ RingTail() ].m_uWeight;
					else
						return false;
				}

				if ( uNpos<m_dNpos[iMid] )
					iEnd = iMid;
				else
					iStart = iMid;
			}
			m_dNpos.Insert ( iEnd, uNpos );
			m_uFirstQpos = Min ( m_uFirstQpos, uQpos );
		// last addition same as the first. So, we can shift
		} else if ( uNpos==m_dRing[m_iRing].m_uNodepos )
		{
			m_uWeight -= m_dRing[m_iRing].m_uWeight;
			m_uFirstHit = HITMAN::GetPosWithField ( m_dRing[m_iRing].m_uHitpos );
			ShiftRing();
		// last addition same as the tail. So, we can move the tail onto it.
		} else if ( uNpos==m_dRing [ RingTail() ].m_uNodepos )
			m_uWeight -= m_dRing [ RingTail() ].m_uWeight;
		else
			return false;
	}

	m_uWeight += pHit->m_uWeight;
	m_uLastML = pHit->m_uMatchlen;
	m_uLastSL = pHit->m_uSpanlen;
	Add2Ring ( pHit );

	// finally got the whole chain - emit it!
	// warning: we don't support overlapping in generic chains.
	if ( m_bTwofer || (int)m_uWordsExpected==m_dNpos.GetLength() )
	{
		ExtHit_t & tTarget = dHits.Add();
		tTarget.m_tRowID = pHit->m_tRowID;
		tTarget.m_uHitpos = Hitpos_t ( m_uFirstHit ); // !COMMIT strictly speaking this is creation from LCS not value
		tTarget.m_uMatchlen = (WORD)( uHitposWithField - m_uFirstHit + m_uLastML );
		tTarget.m_uWeight = m_uWeight;
		m_uPrelastP = 0;

		if ( m_bTwofer ) // for exactly 2 words allow overlapping - so, just shift the chain, not reset it
		{
			tTarget.m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			tTarget.m_uSpanlen = 2;
			tTarget.m_uQposMask = ( 1 << ( Max ( m_uFirstQpos, pHit->m_uQuerypos ) - tTarget.m_uQuerypos ) );
			m_uFirstHit = m_uLastP = uHitposWithField;
			m_uWeight = pHit->m_uWeight;
			m_uFirstQpos = pHit->m_uQuerypos;
		} else
		{
			tTarget.m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			tTarget.m_uSpanlen = (WORD) m_dNpos.GetLength();
			tTarget.m_uQposMask = 0;
			m_uLastP = 0;
			if ( m_bQposMask && tTarget.m_uSpanlen>1 )
			{
				int iNpos0 = m_dNpos[0];
				ARRAY_FOREACH ( i, m_dNpos )
				{
					int iQposDelta = ( m_dNpos[i] - iNpos0 ) + tTarget.m_uQuerypos;
					assert ( iQposDelta<(int)sizeof(tTarget.m_uQposMask)*8 );
					tTarget.m_uQposMask |= ( 1 << iQposDelta );
				}
			}
		}
		return true;
	}

	m_uLastP = uHitposWithField;
	return false;
}


inline void FSMmultinear_c::ResetFSM()
{
	m_iRing = m_uLastP = m_uPrelastP = 0;
}


inline int FSMmultinear_c::RingTail() const
{
	return ( m_iRing + m_dNpos.GetLength() - 1 ) % m_uWordsExpected;
}


inline void FSMmultinear_c::Add2Ring ( const ExtHit_t* pHit )
{
	if ( !m_bTwofer )
		m_dRing [ RingTail() ] = *pHit;
}


inline void FSMmultinear_c::ShiftRing()
{
	if ( ++m_iRing==(int)m_uWordsExpected )
		m_iRing=0;
}


//////////////////////////////////////////////////////////////////////////

struct QuorumDupeNodeHash_t
{
	uint64_t m_uWordID;
	int m_iIndex;

	bool operator < ( const QuorumDupeNodeHash_t & b ) const
	{
		if ( m_uWordID==b.m_uWordID )
			return m_iIndex<b.m_iIndex;
		else
			return m_uWordID<b.m_uWordID;
	}
};

struct QuorumNodeAtomPos_fn
{
	inline bool IsLess ( const ExtQuorum_c::TermTuple_t & a, const ExtQuorum_c::TermTuple_t & b ) const
	{
		return a.m_pTerm->GetAtomPos() < b.m_pTerm->GetAtomPos();
	}
};


ExtQuorum_c::ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
{
	assert ( tNode.GetOp()==SPH_QUERY_QUORUM );
	assert ( dQwords.GetLength()<MAX_HITS );

	m_iThresh = GetThreshold ( tNode, dQwords.GetLength() );
	m_iThresh = Max ( m_iThresh, 1 );
	m_bHasDupes = false;

	assert ( dQwords.GetLength()>1 ); // use TERM instead
	assert ( dQwords.GetLength()<=256 ); // internal masks are upto 256 bits
	assert ( m_iThresh>=1 ); // 1 is also OK; it's a bit different from just OR
	assert ( m_iThresh<dQwords.GetLength() ); // use AND instead

	if ( dQwords.GetLength()>0 )
	{
		m_iAtomPos = dQwords[0]->GetAtomPos();

		// compute duplicate keywords mask (aka dupe mask)
		// FIXME! will fail with wordforms and stuff; sorry, no wordforms vs expand vs quorum support for now!
		CSphFixedVector<QuorumDupeNodeHash_t> dHashes ( dQwords.GetLength() );
		ARRAY_FOREACH ( i, dQwords )
		{
			dHashes[i].m_uWordID = dQwords[i]->GetWordID();
			dHashes[i].m_iIndex = i;
		}
		sphSort ( dHashes.Begin(), dHashes.GetLength() );

		QuorumDupeNodeHash_t tParent = *dHashes.Begin();
		m_dInitialChildren.Add().m_pTerm = dQwords[tParent.m_iIndex];
		m_dInitialChildren.Last().m_iCount = 1;
		tParent.m_iIndex = 0;

		for ( int i=1; i<dHashes.GetLength(); i++ )
		{
			QuorumDupeNodeHash_t & tElem = dHashes[i];
			if ( tParent.m_uWordID!=tElem.m_uWordID )
			{
				tParent = tElem;
				tParent.m_iIndex = m_dInitialChildren.GetLength();
				m_dInitialChildren.Add().m_pTerm = dQwords [ tElem.m_iIndex ];
				m_dInitialChildren.Last().m_iCount = 1;
			} else
			{
				m_dInitialChildren[tParent.m_iIndex].m_iCount++;
				SafeDelete ( dQwords[tElem.m_iIndex] );
				m_bHasDupes = true;
			}
		}

		// sort back to qpos order
		m_dInitialChildren.Sort ( QuorumNodeAtomPos_fn() );
	}

	ARRAY_FOREACH ( i, m_dInitialChildren )
	{
		m_dInitialChildren[i].m_pCurDoc = NULL;
		m_dInitialChildren[i].m_pCurHit = NULL;
		m_dInitialChildren[i].m_pTerm->SetCollectHits();
	}

	m_dChildren = m_dInitialChildren;
}

ExtQuorum_c::~ExtQuorum_c ()
{
	ARRAY_FOREACH ( i, m_dInitialChildren )
		SafeDelete ( m_dInitialChildren[i].m_pTerm );
}

void ExtQuorum_c::Reset ( const ISphQwordSetup & tSetup )
{
	BufferedNode_c::Reset();

	m_dChildren = m_dInitialChildren;

	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i].m_pTerm->Reset ( tSetup );
}

int ExtQuorum_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		int iKidMax = m_dChildren[i].m_pTerm->GetQwords ( hQwords );
		iMax = Max ( iMax, iKidMax );
	}
	return iMax;
}

void ExtQuorum_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i].m_pTerm->SetQwordsIDF ( hQwords );
}

void ExtQuorum_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i].m_pTerm->GetTerms ( hQwords, dTermDupes );
}

uint64_t ExtQuorum_c::GetWordID() const
{
	uint64_t uHash = SPH_FNV64_SEED;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		uint64_t uCur = m_dChildren[i].m_pTerm->GetWordID();
		uHash = sphFNV64 ( &uCur, sizeof(uCur), uHash );
	}

	return uHash;
}


void ExtQuorum_c::HintRowID ( RowID_t tRowID )
{
	for ( auto & i : m_dChildren )
		if ( i.m_pTerm )
			i.m_pTerm->HintRowID ( tRowID );
}


NodeEstimate_t ExtQuorum_c::Estimate ( int64_t iTotalDocs ) const
{
	NodeEstimate_t tEst;
	for ( auto & i : m_dChildren )
		if ( i.m_pTerm )
			tEst += i.m_pTerm->Estimate(iTotalDocs);

	return tEst;
}


const ExtDoc_t * ExtQuorum_c::GetDocsChunk()
{
	// warmup
	ARRAY_FOREACH ( i, m_dChildren )
	{
		TermTuple_t & tElem = m_dChildren[i];
		if ( HasDocs(tElem.m_pCurDoc) )
			continue;

		tElem.m_pCurDoc = tElem.m_pTerm->GetDocsChunk();
		if ( tElem.m_pCurDoc )
			tElem.m_pCurHit = tElem.m_pTerm->GetHits ( tElem.m_pCurDoc );
		else
		{
			m_dChildren.RemoveFast(i);
			i--;
		}
	}

	// main loop
	int iDoc = 0;
	int iQuorumLeft = CountQuorum ( true );
	while ( iDoc<MAX_BLOCK_DOCS-1 && iQuorumLeft>=m_iThresh )
	{
		// find min document ID, count occurrences
		ExtDoc_t tCand;

		tCand.m_tRowID = INVALID_ROWID; // current candidate id
		tCand.m_uDocFields = 0; // non necessary
		tCand.m_fTFIDF = 0.0f;

		int iQuorum = 0;
		for ( auto & tChild : m_dChildren )
		{
			assert ( HasDocs ( tChild.m_pCurDoc ) );
			if ( tChild.m_pCurDoc->m_tRowID < tCand.m_tRowID )
			{
				tCand = *tChild.m_pCurDoc;
				iQuorum = tChild.m_iCount;
			}
			else if ( tChild.m_pCurDoc->m_tRowID==tCand.m_tRowID )
			{
				tCand.m_uDocFields |= tChild.m_pCurDoc->m_uDocFields; // FIXME!!! check hits in case of dupes or field constrain
				tCand.m_fTFIDF += tChild.m_pCurDoc->m_fTFIDF;
				iQuorum += tChild.m_iCount;
			}
		}

		if ( iQuorum>=m_iThresh && CollectMatchingHits ( tCand.m_tRowID, m_iThresh ) )
			m_dDocs[iDoc++] = tCand;

		// advance children
		int iNumChildren = m_dChildren.GetLength();
		ARRAY_FOREACH ( i, m_dChildren )
		{
			TermTuple_t & tElem = m_dChildren[i];
			if ( tElem.m_pCurDoc->m_tRowID!=tCand.m_tRowID )
				continue;

			tElem.m_pCurDoc++;
			if ( HasDocs(tElem.m_pCurDoc) )
				continue;

			tElem.m_pCurDoc = tElem.m_pTerm->GetDocsChunk();
			if ( tElem.m_pCurDoc )
				tElem.m_pCurHit = tElem.m_pTerm->GetHits ( tElem.m_pCurDoc );
			else
			{
				m_dChildren.RemoveFast ( i );
				i--;
			}
		}

		if ( iNumChildren!=m_dChildren.GetLength() )
			iQuorumLeft = CountQuorum ( false );
	}

	return ReturnDocsChunk ( iDoc, "quorum" );
}


struct QuorumCmpHitPos_fn
{
	inline bool IsLess ( const ExtHit_t & a, const ExtHit_t & b ) const
	{
		if ( a.m_tRowID==b.m_tRowID )
		{
			DWORD uHitPosA = HITMAN::GetPosWithField(a.m_uHitpos);
			DWORD uHitPosB = HITMAN::GetPosWithField(b.m_uHitpos);

			if ( uHitPosA==uHitPosB )
				return a.m_uQuerypos<b.m_uQuerypos;

			return uHitPosA<uHitPosB;
		}

		return a.m_tRowID<b.m_tRowID;
	}
};


void ExtQuorum_c::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	m_dHits.Sort ( QuorumCmpHitPos_fn() );
}


int ExtQuorum_c::CountQuorum ( bool bFixDupes )
{
	if ( !m_bHasDupes )
		return m_dChildren.GetLength();

	int iSum = 0;
	bool bHasDupes = false;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		iSum += m_dChildren[i].m_iCount;
		bHasDupes |= ( m_dChildren[i].m_iCount>1 );
	}

#if QDEBUG
	if ( bFixDupes && bHasDupes!=m_bHasDupes )
		printf ( "quorum dupes %d -> %d\n", m_bHasDupes, bHasDupes );
#endif

	m_bHasDupes = bFixDupes ? bHasDupes : m_bHasDupes;
	return iSum;
}


int ExtQuorum_c::GetThreshold ( const XQNode_t & tNode, int iQwords )
{
	return ( tNode.m_bPercentOp ? (int)floor ( 1.0f / 100.0f * tNode.m_iOpArg * iQwords + 0.5f ) : tNode.m_iOpArg );
}

bool ExtQuorum_c::CollectMatchingHits ( RowID_t tRowID, int iThreshold )
{
	if ( !m_bHasDupes )
	{
		for ( auto & tChild : m_dChildren )
		{
			while ( tChild.m_pCurHit->m_tRowID < tRowID )
				tChild.m_pCurHit++;

			while ( tChild.m_pCurHit->m_tRowID==tRowID )
				m_dMyHits.Add ( *tChild.m_pCurHit++ );
		}

		return true;
	}

	int iOldHitLen = m_dMyHits.GetLength();
	int iQuorum = 0;
	for ( auto & tChild : m_dChildren )
	{
		const ExtHit_t * & pHit = tChild.m_pCurHit;
		if ( !HasHits(pHit) )
			continue;

		while ( pHit->m_tRowID<tRowID )
			pHit++;

		// collect matched hits but only up to quorum.count per-term
		for ( int iTermHits = 0; iTermHits<tChild.m_iCount && pHit->m_tRowID==tRowID; iTermHits++, iQuorum++ )
			m_dMyHits.Add ( *pHit++ );

		// got quorum - no need to check further
		if ( iQuorum>=iThreshold )
			break;
	}

	// discard collected hits in case of no quorum matched
	if ( iQuorum<iThreshold )
	{
		m_dMyHits.Resize ( iOldHitLen );
		return false;
	}

	// collect all hits to move docs/hits further
	for ( auto & tChild : m_dChildren )
	{
		while ( tChild.m_pCurHit->m_tRowID==tRowID )
			m_dMyHits.Add ( *tChild.m_pCurHit++ );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

ExtOrder_c::ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup )
	: m_dChildren ( dChildren )
	, m_bDone ( false )
{
	int iChildren = dChildren.GetLength();
	assert ( iChildren>=2 );

	m_dChildDoc.Resize ( iChildren );
	m_dChildHit.Resize ( iChildren );
	m_dChildDocsChunk.Resize ( iChildren );

	if ( dChildren.GetLength()>0 )
		m_iAtomPos = dChildren[0]->GetAtomPos();

	ARRAY_FOREACH ( i, dChildren )
	{
		assert ( m_dChildren[i] );
		m_dChildDoc[i] = nullptr;
		m_dChildHit[i] = nullptr;
		m_dChildren[i]->SetCollectHits();
	}
}


void ExtOrder_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bDone = false;
	m_dMyHits.Resize(0);

	m_dChildDoc.Fill(nullptr);
	m_dChildHit.Fill(nullptr);

	for ( auto pChild : m_dChildren )
	{
		assert(pChild);
		pChild->Reset(tSetup);
	}
}


ExtOrder_c::~ExtOrder_c()
{
	for ( auto & pChild : m_dChildren )
		SafeDelete ( pChild );
}


// rewinds all children hitlists to given row id
// returns the one with min hitpos
int ExtOrder_c::GetChildIdWithNextHit ( RowID_t tRowID )
{
	// OPTIMIZE! implement PQ instead of full-scan
	DWORD uMinPosWithField = UINT_MAX;
	int iChild = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		const ExtHit_t * & pHit = m_dChildHit[i];

		// skip until proper hit
		while ( pHit->m_tRowID<tRowID )
			pHit++;

		// is this our man at all?
		if ( pHit->m_tRowID==tRowID )
		{
			// is he the best we can get?
			if ( HITMAN::GetPosWithField ( pHit->m_uHitpos ) < uMinPosWithField )
			{
				uMinPosWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );
				iChild = i;
			}
		}
	}
	return iChild;
}


bool ExtOrder_c::GetMatchingHits ( RowID_t tRowID )
{
	// my trackers
	CSphVector<ExtHit_t> dAccLongest;
	CSphVector<ExtHit_t> dAccRecent;
	int iPosLongest = 0; // needed to handle cases such as "a b c" << a
	int iPosRecent = 0;
	int iField = -1;

	dAccLongest.Reserve ( m_dChildren.GetLength() );
	dAccRecent.Reserve ( m_dChildren.GetLength() );

	int nOldHits = m_dMyHits.GetLength();

	while ( true )
	{
		// get next hit (in hitpos ascending order)
		int iChild = GetChildIdWithNextHit ( tRowID );
		if ( iChild<0 )
			break; // OPTIMIZE? no trailing hits on this route

		const ExtHit_t * & pHit = m_dChildHit[iChild];
		assert ( pHit->m_tRowID==tRowID );

		// most recent subseq must never be longer
		assert ( dAccRecent.GetLength()<=dAccLongest.GetLength() );

		// handle that hit!
		int iHitField = HITMAN::GetField ( pHit->m_uHitpos );
		int iHitPos = HITMAN::GetPos ( pHit->m_uHitpos );

		if ( iHitField!=iField )
		{
			// new field; reset both trackers
			dAccLongest.Resize ( 0 );
			dAccRecent.Resize ( 0 );

			// initial seeding, if needed
			if ( iChild==0 )
			{
				dAccLongest.Add ( *pHit );
				iPosLongest = iHitPos + pHit->m_uSpanlen;
				iField = iHitField;
			}

		} else if ( iChild==dAccLongest.GetLength() && iHitPos>=iPosLongest )
		{
			// it fits longest tracker
			dAccLongest.Add ( *pHit );
			iPosLongest = iHitPos + pHit->m_uSpanlen;

			// fully matched subsequence
			if ( dAccLongest.GetLength()==m_dChildren.GetLength() )
			{
				// flush longest tracker into buffer, and keep it terminated
				ARRAY_FOREACH ( i, dAccLongest )
					m_dMyHits.Add ( dAccLongest[i] );

				// reset both trackers
				dAccLongest.Resize ( 0 );
				dAccRecent.Resize ( 0 );
				iPosRecent = iPosLongest;
			}

		} else if ( iChild==0 )
		{
			// it restarts most-recent tracker
			dAccRecent.Resize ( 0 );
			dAccRecent.Add ( *pHit );
			iPosRecent = iHitPos + pHit->m_uSpanlen;
			if ( !dAccLongest.GetLength() )
			{
				dAccLongest.Add	( *pHit );
				iPosLongest = iHitPos + pHit->m_uSpanlen;
			}
		} else if ( iChild==dAccRecent.GetLength() && iHitPos>=iPosRecent )
		{
			// it fits most-recent tracker
			dAccRecent.Add ( *pHit );
			iPosRecent = iHitPos + pHit->m_uSpanlen;

			// maybe most-recent just became longest too?
			if ( dAccRecent.GetLength()==dAccLongest.GetLength() )
			{
				dAccLongest.SwapData ( dAccRecent );
				dAccRecent.Resize ( 0 );
				iPosLongest = iPosRecent;
			}
		}

		// advance hit stream
		pHit++;
	}

	return nOldHits!=m_dMyHits.GetLength();
}


const ExtDoc_t * ExtOrder_c::GetDocsChunk()
{
	if ( m_bDone )
		return nullptr;

	// warm up
	ARRAY_FOREACH ( i, m_dChildren )
	{
		if ( !m_dChildDoc[i] )
		{
			m_dChildDoc[i] = m_dChildDocsChunk[i] = m_dChildren[i]->GetDocsChunk();
			m_dChildHit[i] = nullptr;
		}

		if ( !m_dChildDoc[i] )
		{
			m_bDone = true;
			return nullptr;
		}
	}

	// match while there's enough space in buffer
	int iDoc = 0;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		// find next candidate document (that has all the words)
		RowID_t tRowID = m_dChildDoc[0]->m_tRowID;
		assert ( tRowID!=INVALID_ROWID );

		int iChild = 1;
		while ( iChild < m_dChildren.GetLength() )
		{
			// skip docs with too small ids
			assert ( m_dChildDoc[iChild] );
			while ( m_dChildDoc[iChild]->m_tRowID < tRowID )
				m_dChildDoc[iChild]++;

			// block end? pull next block and keep scanning
			if ( !HasDocs ( m_dChildDoc[iChild] ) )
			{
				m_dChildDoc[iChild] = m_dChildDocsChunk[iChild] = m_dChildren[iChild]->GetDocsChunk();
				m_dChildHit[iChild] = nullptr;
				if ( !m_dChildDoc[iChild] )
				{
					m_bDone = true;
					return ReturnDocsChunk ( iDoc, "order" );
				}
				continue;
			}

			// too big id? its out next candidate
			if ( m_dChildDoc[iChild]->m_tRowID > tRowID )
			{
				tRowID = m_dChildDoc[iChild]->m_tRowID;
				iChild = 0;
				continue;
			}

			assert ( m_dChildDoc[iChild]->m_tRowID==tRowID );
			iChild++;
		}

		#ifndef NDEBUG
		assert ( tRowID!=INVALID_ROWID );
		for ( auto pChildDoc : m_dChildDoc )
		{
			assert ( pChildDoc );
			assert ( pChildDoc->m_tRowID==tRowID );
		}
		#endif

		// fetch hits
		ARRAY_FOREACH ( i, m_dChildren )
		{
			if ( !m_dChildHit[i] )
				m_dChildHit[i] = m_dChildren[i]->GetHits ( m_dChildDoc[i] );
		}

		// match and save hits
		if ( GetMatchingHits ( tRowID ) )
			m_dDocs[iDoc++] = *m_dChildDoc[0];

		// advance doc stream
		m_dChildDoc[0]++;
		if ( !HasDocs ( m_dChildDoc[0] ) )
		{
			m_dChildDoc[0] = m_dChildDocsChunk[0] = m_dChildren[0]->GetDocsChunk();
			m_dChildHit[0] = nullptr;
			if ( !m_dChildDoc[0] )
			{
				m_bDone = true;
				break;
			}
		}
	}

	return ReturnDocsChunk ( iDoc, "order" );
}


void ExtOrder_c::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_iAtomPos, m_dHits.Begin(), this );
}


int ExtOrder_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		int iKidMax = m_dChildren[i]->GetQwords ( hQwords );
		iMax = Max ( iMax, iKidMax );
	}
	return iMax;
}

void ExtOrder_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetQwordsIDF ( hQwords );
}

void ExtOrder_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->GetTerms ( hQwords, dTermDupes );
}

uint64_t ExtOrder_c::GetWordID () const
{
	uint64_t uHash = SPH_FNV64_SEED;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		uint64_t uCur = m_dChildren[i]->GetWordID();
		uHash = sphFNV64 ( &uCur, sizeof(uCur), uHash );
	}

	return uHash;
}


void ExtOrder_c::HintRowID ( RowID_t tRowID )
{
	for ( auto i : m_dChildren )
		i->HintRowID ( tRowID );
}


NodeEstimate_t ExtOrder_c::Estimate ( int64_t iTotalDocs ) const
{
	NodeEstimate_t tEst;
	for ( const auto & i : m_dChildren )
		tEst += i->Estimate(iTotalDocs);

	return tEst;
}

//////////////////////////////////////////////////////////////////////////

ExtUnit_c::ExtUnit_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const FieldMask_t& uFields, const ISphQwordSetup & tSetup, const char * sUnit )
	: m_pArg1 ( pFirst )
	, m_pArg2 ( pSecond )
{
	XQKeyword_t tDot;
	tDot.m_sWord = sUnit;
	m_pDot = new ExtTerm_T<false> ( CreateQueryWord ( tDot, tSetup ), uFields, tSetup, true );

	m_pArg1->SetCollectHits();
	m_pArg2->SetCollectHits();
	m_pDot->SetCollectHits();
}


ExtUnit_c::~ExtUnit_c ()
{
	SafeDelete ( m_pArg1 );
	SafeDelete ( m_pArg2 );
	SafeDelete ( m_pDot );
}


void ExtUnit_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pArg1->Reset ( tSetup );
	m_pArg2->Reset ( tSetup );
	m_pDot->Reset ( tSetup );

	m_pDocs1 = m_pDocs2 = m_pDotDocs = nullptr;
	m_pDoc1 = m_pDoc2 = m_pDotDoc = nullptr;
	m_bNeedDotHits = false;

	BufferedNode_c::Reset();
}


int ExtUnit_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax1 = m_pArg1->GetQwords ( hQwords );
	int iMax2 = m_pArg2->GetQwords ( hQwords );
	return Max ( iMax1, iMax2 );
}


void ExtUnit_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pArg1->SetQwordsIDF ( hQwords );
	m_pArg2->SetQwordsIDF ( hQwords );
}

void ExtUnit_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	m_pArg1->GetTerms ( hQwords, dTermDupes );
	m_pArg2->GetTerms ( hQwords, dTermDupes );
}

uint64_t ExtUnit_c::GetWordID() const
{
	uint64_t dHash[2];
	dHash[0] = m_pArg1->GetWordID();
	dHash[1] = m_pArg2->GetWordID();
	return sphFNV64 ( dHash, sizeof(dHash) );
}


void ExtUnit_c::HintRowID ( RowID_t tRowID )
{
	m_pArg1->HintRowID ( tRowID );
	m_pArg2->HintRowID ( tRowID );
}


void ExtUnit_c::DebugDump ( int iLevel )
{
	DebugIndent ( iLevel );
	printf ( "ExtSentence\n" );
	m_pArg1->DebugDump ( iLevel+1 );
	m_pArg2->DebugDump ( iLevel+1 );
}


NodeEstimate_t ExtUnit_c::Estimate ( int64_t iTotalDocs ) const
{
	assert ( m_pArg1 && m_pArg2 && m_pDot );

	NodeEstimate_t tRes;
	tRes += m_pArg1->Estimate(iTotalDocs);
	tRes += m_pArg2->Estimate(iTotalDocs);
	tRes += m_pDot->Estimate(iTotalDocs);

	return tRes;
}

/// skips hits within current document while their position is less or equal than the given limit
/// returns true if a matching hit (with big enough position, and in current document) was found
/// returns false otherwise
static inline bool SkipHitsLtePos ( const ExtHit_t * & pHits, Hitpos_t uPos )
{
	assert ( pHits );
	RowID_t tRowID = pHits->m_tRowID;
	if ( tRowID==INVALID_ROWID )
		return false;

	while ( pHits->m_tRowID==tRowID && pHits->m_uHitpos<=uPos )
		pHits++;

	return pHits->m_tRowID==tRowID;
}


void ExtUnit_c::FilterHits ( const ExtDoc_t * pDoc1, const ExtDoc_t * pDoc2, const ExtHit_t * & pHit1, const ExtHit_t * & pHit2, const ExtHit_t * & pDotHit, DWORD uSentenceEnd, RowID_t tRowID, int & iDoc )
{
	bool bRegistered = false;
	while ( true )
	{
		if ( uSentenceEnd )
		{
			// we're in a matched sentence state
			// copy hits until next dot
			bool bValid1 = pHit1->m_tRowID==tRowID && pHit1->m_uHitpos<uSentenceEnd;
			bool bValid2 = pHit2->m_tRowID==tRowID && pHit2->m_uHitpos<uSentenceEnd;

			if ( !bValid1 && !bValid2 )
			{
				// no more hits in this sentence
				uSentenceEnd = 0;
				if ( pHit1->m_tRowID==tRowID && pHit2->m_tRowID==tRowID )
					continue; // no more in-sentence hits, but perhaps more sentences in this document
				else
					break; // document is over
			}

			// register document as matching
			if ( !bRegistered )
			{
				ExtDoc_t & tDoc = m_dDocs[iDoc++];
				tDoc.m_tRowID = pDoc1->m_tRowID;
				tDoc.m_uDocFields = pDoc1->m_uDocFields | pDoc2->m_uDocFields; // non necessary
				tDoc.m_fTFIDF = pDoc1->m_fTFIDF + pDoc2->m_fTFIDF;
				bRegistered = true; // just once
			}

			if ( bValid1 && ( !bValid2 || IsHitLess ( pHit1, pHit2 ) ) )
				m_dMyHits.Add ( *pHit1++ );
			else
				m_dMyHits.Add ( *pHit2++ );
		}
		else
		{
			// no sentence matched yet
			// let's check the next hit pair
			assert ( pHit1->m_tRowID==tRowID );
			assert ( pHit2->m_tRowID==tRowID );
			assert ( pDotHit->m_tRowID==tRowID );

			// our current hit pair locations
			DWORD uMin = Min ( pHit1->m_uHitpos, pHit2->m_uHitpos );
			DWORD uMax = Max ( pHit1->m_uHitpos, pHit2->m_uHitpos );

			// skip all dots beyond the min location
			if ( !SkipHitsLtePos ( pDotHit, uMin ) )
			{
				// we have a match!
				// moreover, no more dots past min location in current document
				// copy hits until next document
				uSentenceEnd = UINT_MAX;
				continue;
			}

			// does the first post-pair-start dot separate our hit pair?
			if ( pDotHit->m_uHitpos < uMax )
			{
				// yes, got an "A dot B" case
				// rewind candidate hits past this dot, break if current document is over
				if ( !SkipHitsLtePos ( pHit1, pDotHit->m_uHitpos ) )
					break;
				if ( !SkipHitsLtePos ( pHit2, pDotHit->m_uHitpos ) )
					break;

				continue;
			}
			else
			{
				// we have a match!
				// copy hits until next dot
				if ( !SkipHitsLtePos ( pDotHit, uMax ) )
					uSentenceEnd = UINT_MAX; // correction, no next dot, so make it "next document"
				else
					uSentenceEnd = pDotHit->m_uHitpos;

				assert ( uSentenceEnd );
			}
		}
	}
}


const ExtDoc_t * ExtUnit_c::GetDocsChunk()
{
	// SENTENCE operator is essentially AND on steroids
	// that also takes relative dot positions into account
	//
	// when document matches both args but not the dot, it degenerates into AND
	// we immediately lookup and copy matching document hits anyway, though
	// this is suboptimal (because these hits might never be required at all)
	// but this is expected to be rare case, so let's keep code simple
	//
	// when document matches both args and the dot, we need to filter the hits
	// only those left/right pairs that are not (!) separated by a dot should match

	int iDoc = 0;
	const ExtHit_t * pHit1 = m_pHit1;
	const ExtHit_t * pHit2 = m_pHit2;
	const ExtHit_t * pDotHit = m_pDotHit;
	const ExtDoc_t * pDoc1 = m_pDoc1;
	const ExtDoc_t * pDoc2 = m_pDoc2;
	const ExtDoc_t * pDotDoc = m_pDotDoc;

	bool bNeedDoc1Hits = false;
	bool bNeedDoc2Hits = false;
	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		// fetch more candidate docs, if needed
		if ( !HasDocs(pDoc1) )
		{
			if ( HasDocs(pDoc2)  )
				m_pArg1->HintRowID ( pDoc2->m_tRowID );

			pDoc1 = m_pArg1->GetDocsChunk();
			if ( !HasDocs(pDoc1) )
				break;

			bNeedDoc1Hits = true;
		}

		if ( !HasDocs(pDoc2) )
		{
			if ( HasDocs(pDoc1)  )
				m_pArg2->HintRowID ( pDoc1->m_tRowID );

			pDoc2 = m_pArg2->GetDocsChunk();
			if ( !HasDocs(pDoc2) )
				break;

			bNeedDoc2Hits = true;
		}

		// find next candidate match
		while ( pDoc1->m_tRowID!=pDoc2->m_tRowID && HasDocs(pDoc1) && HasDocs(pDoc2) )
		{
			while ( pDoc1->m_tRowID < pDoc2->m_tRowID && HasDocs(pDoc2) )
				pDoc1++;
			while ( pDoc1->m_tRowID > pDoc2->m_tRowID && HasDocs(pDoc1) )
				pDoc2++;
		}

		// got our candidate that matches AND?
		RowID_t tRowID = pDoc1->m_tRowID;
		if ( !HasDocs(pDoc1) || !HasDocs(pDoc2) )
			continue;

		// yes, now fetch more dots docs, if needed
		// note how NULL is accepted here, "A and B but no dots" case is valid!
		if ( !HasDocs(pDotDoc) )
		{
			m_pDot->HintRowID(tRowID);
			pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk();
			m_bNeedDotHits = true;
		}

		// skip preceding docs
		while ( pDotDoc && pDotDoc->m_tRowID < tRowID )
		{
			while ( pDotDoc->m_tRowID < tRowID )
				pDotDoc++;

			if ( !HasDocs(pDotDoc) )
			{
				pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk();
				m_bNeedDotHits = true;
			}
		}

		// we will need document hits on both routes below
		if ( bNeedDoc1Hits )
		{
			pHit1 = m_pArg1->GetHits(pDoc1);
			bNeedDoc1Hits = false;
		}

		while ( pHit1->m_tRowID < tRowID )
			pHit1++;

		if ( bNeedDoc2Hits )
		{
			pHit2 = m_pArg2->GetHits(pDoc2);
			bNeedDoc2Hits = false;
		}

		while ( pHit2->m_tRowID < tRowID )
			pHit2++;

		assert ( pHit1->m_tRowID==tRowID );
		assert ( pHit2->m_tRowID==tRowID );

		DWORD uSentenceEnd = 0;
		if ( !pDotDoc || pDotDoc->m_tRowID!=tRowID )
		{
			// no dots in current document?
			// just copy all hits until next document
			uSentenceEnd = UINT_MAX;

		} else
		{
			// got both hits and dots
			// rewind to relevant dots hits, then do sentence boundary detection
			if ( m_bNeedDotHits )
			{
				pDotHit = m_pDot->GetHits ( pDotDoc );
				m_bNeedDotHits = false;
			}

			while ( pDotHit->m_tRowID < tRowID )
				pDotHit++;
		}

		// do those hits
		FilterHits ( pDoc1, pDoc2, pHit1, pHit2, pDotHit, uSentenceEnd, tRowID, iDoc );

		// all hits copied; do the next candidate
		pDoc1++;
		pDoc2++;
	}

	m_pDoc1 = pDoc1;
	m_pDoc2 = pDoc2;
	m_pDotDoc = pDotDoc;
	m_pHit1 = pHit1;
	m_pHit2 = pHit2;
	m_pDotHit = pDotHit;

	return ReturnDocsChunk ( iDoc, "unit" );
}


void ExtUnit_c::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_iAtomPos, m_dHits.Begin(), this );
}

//////////////////////////////////////////////////////////////////////////

ExtNotNear_c::ExtNotNear_c ( ExtNode_i * pMust, ExtNode_i * pNot, int iDist )
	: ExtTwofer_c ( pMust, pNot )
	, m_iDist ( iDist )
{
	m_sNodeName.SetSprintf ( "NOTNEAR/%d", m_iDist );

	// need hits from both nodes
	pMust->SetCollectHits();
	pNot->SetCollectHits();
}


void ExtNotNear_c::Reset ( const ISphQwordSetup & tSetup )
{
	ExtTwofer_c::Reset ( tSetup );
	BufferedNode_c::Reset();
	m_pHitL = nullptr;
	m_pHitR = nullptr;
}


void ExtNotNear_c::DebugDump ( int iLevel )
{
	DebugDumpT ( "ExtNotNear_c", iLevel );
}


bool ExtNotNear_c::FilterHits ( RowID_t tRowID, const ExtHit_t * & pMust, const ExtHit_t * & pNot )
{
	assert ( pMust && pNot && HasHits(pMust) && HasHits(pNot) );

	const int iWasHits = m_dMyHits.GetLength();
	bool bRightEmpty = false;

	// any hits stream over might have tail hits
	while ( pMust->m_tRowID==tRowID )
	{
		// get NOT next after current MUST
		while ( pNot->m_tRowID==tRowID && HITMAN::GetPosWithField ( pNot->m_uHitpos ) < HITMAN::GetPosWithField ( pMust->m_uHitpos ) )
			pNot++;

		if ( !HasHits(pNot) )
			break;

		bRightEmpty = ( pNot->m_tRowID!=tRowID );
		DWORD uPosMust = HITMAN::GetPosWithField ( pMust->m_uHitpos );

		// field is top 8 bytes that is why it safe to add distance straight to GetPosWithField and compare these without checking both fields are eq
		if ( bRightEmpty || uPosMust + pMust->m_uMatchlen - 1 + m_iDist<HITMAN::GetPosWithField ( pNot->m_uHitpos ) )
			m_dMyHits.Add ( *pMust );

		pMust++;
	}

	return ( iWasHits<m_dMyHits.GetLength() );
}


const ExtDoc_t * ExtNotNear_c::GetDocsChunk()
{
	const ExtDoc_t * pDocL = m_pDocL;
	const ExtDoc_t * pDocR = m_pDocR;
	const ExtHit_t * pHitL = m_pHitL;
	const ExtHit_t * pHitR = m_pHitR;

	int iDoc = 0;
	bool bRightEmpty = false;

	while ( iDoc<MAX_BLOCK_DOCS-1 )
	{
		if ( !WarmupDocs ( pDocL, pHitL, m_pLeft.get() ) )
			break;
		
		if ( !bRightEmpty )
		{
			if ( HasDocs(pDocL) )
				m_pRight->HintRowID ( pDocL->m_tRowID );

			bRightEmpty = !WarmupDocs ( pDocR, pHitR, m_pRight.get() );
		}

		RowID_t tNotRowID = ( bRightEmpty ? INVALID_ROWID : pDocR->m_tRowID );

		// copy none matched from MUST
		while ( pDocL->m_tRowID < tNotRowID && iDoc<MAX_BLOCK_DOCS-1 )
		{
			m_dDocs[iDoc++] = *pDocL;

			while ( pHitL->m_tRowID<pDocL->m_tRowID )
				pHitL++;

			while ( pHitL->m_tRowID==pDocL->m_tRowID )
				m_dMyHits.Add ( *pHitL++ );

			pDocL++;
		}

		if ( !HasDocs(pDocL) || iDoc==MAX_BLOCK_DOCS-1 )
			continue;

		if ( bRightEmpty )
			continue;

		// skip NOT until min accepted id
		while ( pDocR->m_tRowID<pDocL->m_tRowID ) pDocR++;
		while ( pHitR->m_tRowID<pDocR->m_tRowID ) pHitR++;
		if ( !HasHits(pHitR) || pDocL->m_tRowID!=pDocR->m_tRowID )
			continue;

		// filter must with not
		assert ( HasDocs(pDocL) );
		assert ( pDocL->m_tRowID==pDocR->m_tRowID );
		assert ( pDocL->m_tRowID==pHitL->m_tRowID && pDocL->m_tRowID==pHitR->m_tRowID );

		bool bMatched = FilterHits ( pDocL->m_tRowID, pHitL, pHitR );
		bMatched |= pHitL->m_tRowID==pDocL->m_tRowID;

		// copy MUST tail hits
		while ( pHitL->m_tRowID==pDocL->m_tRowID )
			m_dMyHits.Add ( *pHitL++ );

		if ( bMatched )
			m_dDocs[iDoc++] = *pDocL;

		pDocL++;
		pDocR++;

		if ( HasDocs(pDocL) )
		{
			while ( pHitL->m_tRowID<pDocL->m_tRowID )
				pHitL++;
		}

		if ( HasDocs(pDocR) )
		{
			while ( pHitR->m_tRowID<pDocR->m_tRowID )
				pHitR++;
		}
	}

	m_pDocL = pDocL;
	m_pDocR = pDocR;
	m_pHitL = pHitL;
	m_pHitR = pHitR;

	return ReturnDocsChunk ( iDoc, "notnear" );
}


void ExtNotNear_c::CollectHits ( const ExtDoc_t * pDocs )
{
	CopyMatchingHits ( m_dHits, pDocs );
	PrintHitsChunk ( m_dHits.GetLength(), m_iAtomPos, m_dHits.Begin(), this );
}


//////////////////////////////////////////////////////////////////////////
// INTRA-BATCH CACHING
//////////////////////////////////////////////////////////////////////////

/// container that does intra-batch query-sub-tree caching
/// actually carries the cached data, NOT to be recreated frequently (see thin wrapper below)
class NodeCacheContainer_c
{
	friend class ExtNodeCached_c;
	friend class CSphQueryNodeCache;

public:
	void						Release();
	ExtNode_i *					CreateCachedWrapper ( ExtNode_i* pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup );

private:
	int							m_iRefCount {1};
	bool						m_bStateOk {true};
	const ISphQwordSetup *		m_pSetup {nullptr};

	CSphVector<ExtDoc_t>		m_dDocs;
	CSphVector<ExtHit_t>		m_dHits;
	int							m_iAtomPos {0}; // minimal position from original donor, used for shifting

	CSphQueryNodeCache *		m_pNodeCache {nullptr};

	bool						WarmupCache ( ExtNode_i * pChild, int iQWords );
	void						Invalidate();
};


/// cached node wrapper to be injected into actual search trees
/// (special container actually carries all the data and does the work, see below)
class ExtNodeCached_c : public ExtNode_c
{
	friend class NodeCacheContainer_c;

public:
						~ExtNodeCached_c() override;

	void				Reset ( const ISphQwordSetup & tSetup ) override;
	void				HintRowID ( RowID_t tRowID ) override {}
	const ExtDoc_t *	GetDocsChunk() override;
	void				CollectHits ( const ExtDoc_t * pMatched ) override;
	int					GetQwords ( ExtQwordsHash_t & hQwords ) override;
	void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) override;
	void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const override;
	bool				GotHitless () override;
	uint64_t			GetWordID() const override;
	void				SetCollectHits() override;
	NodeEstimate_t		Estimate ( int64_t iTotalDocs ) const override { return { 0.0f, 0, 0 }; }

private:
	NodeCacheContainer_c * m_pNode;
	const ExtDoc_t *	m_pHitDoc;			///< points to entry in m_dDocs which GetHitsChunk() currently emits hits for
	CSphString *		m_pWarning;
	int64_t				m_iMaxTimer;		///< work until this timestamp
	int					m_iHitIndex;		///< store the current position in m_Hits for GetHitsChunk()
	int					m_iDocIndex;		///< store the current position in m_Docs for GetDocsChunk()
	ExtNode_i *			m_pChild;			///< pointer to donor for the sake of AtomPos procession
	int					m_iQwords;			///< number of tokens in parent query


						// creation possible ONLY via NodeCacheContainer_c
						ExtNodeCached_c ( NodeCacheContainer_c * pNode, ExtNode_i * pChild );

	void				StepForwardToHitsFor ( RowID_t tRowID );
	bool				RewindDocs ( const ExtDoc_t * & pDoc, const ExtDoc_t * & pMatched );
};

//////////////////////////////////////////////////////////////////////////

void NodeCacheContainer_c::Release()
{
	if ( --m_iRefCount<=0 )
		Invalidate();
}


ExtNode_i * NodeCacheContainer_c::CreateCachedWrapper ( ExtNode_i * pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup )
{
	if ( !m_bStateOk )
		return pChild;

	// wow! we have a virgin!
	if ( !m_dDocs.GetLength() )
	{
		m_iRefCount = pRawChild->GetCount();
		m_pSetup = &tSetup;
	}
	return new ExtNodeCached_c ( this, pChild );
}


bool NodeCacheContainer_c::WarmupCache ( ExtNode_i * pChild, int iQwords )
{
	assert ( pChild );
	assert ( m_pSetup );

	m_iAtomPos = pChild->GetAtomPos();
	const ExtDoc_t * pChunk = pChild->GetDocsChunk();

	while ( pChunk )
	{
		const ExtDoc_t * pChunkHits = pChunk;
		bool iHasDocs = false;
		for ( ; HasDocs(pChunk); pChunk++ )
		{
			m_dDocs.Add ( *pChunk );
			// exclude number or Qwords from FIDF
			m_dDocs.Last().m_fTFIDF *= iQwords;
			m_pNodeCache->m_iMaxCachedDocs--;
			iHasDocs = true;
		}

		if ( iHasDocs )
		{
			const ExtHit_t * pHit = pChild->GetHits(pChunkHits);
			while (	HasHits(pHit) )
			{
				m_dHits.Add ( *pHit++ );
				m_pNodeCache->m_iMaxCachedHits--;
			}
		}

		// too many values, stop caching
		if ( m_pNodeCache->m_iMaxCachedDocs<0 || m_pNodeCache->m_iMaxCachedHits<0 )
		{
			Invalidate ();
			pChild->Reset ( *m_pSetup );
			m_pSetup = NULL;
			return false;
		}
		pChunk = pChild->GetDocsChunk();
	}

	m_dDocs.Add().m_tRowID = INVALID_ROWID;
	m_dHits.Add().m_tRowID = INVALID_ROWID;
	pChild->Reset ( *m_pSetup );
	m_pSetup = NULL;
	return true;
}


void NodeCacheContainer_c::Invalidate()
{
	m_pNodeCache->m_iMaxCachedDocs += m_dDocs.GetLength();
	m_pNodeCache->m_iMaxCachedHits += m_dDocs.GetLength();
	m_dDocs.Reset();
	m_dHits.Reset();
	m_bStateOk = false;
}

//////////////////////////////////////////////////////////////////////////

ExtNodeCached_c::~ExtNodeCached_c ()
{
	SafeDelete ( m_pChild );
	SafeRelease ( m_pNode );
}


void ExtNodeCached_c::Reset ( const ISphQwordSetup & tSetup )
{
	if ( m_pChild )
		m_pChild->Reset ( tSetup );

	m_iHitIndex = 0;
	m_iDocIndex = 0;
	m_pHitDoc = NULL;
	m_iMaxTimer = 0;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pWarning = tSetup.m_pWarning;
}


int ExtNodeCached_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	if ( !m_pChild )
		return -1;

	int iChildAtom = m_pChild->GetQwords ( hQwords );
	if ( iChildAtom<0 )
		return -1;

	return m_iAtomPos + iChildAtom;
}


void ExtNodeCached_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_iQwords = hQwords.GetLength();
	if ( m_pNode->m_pSetup && m_pChild )
	{
		m_pChild->SetQwordsIDF ( hQwords );
		m_pNode->WarmupCache ( m_pChild, m_iQwords );
	}
}


void ExtNodeCached_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	if ( m_pChild )
		m_pChild->GetTerms ( hQwords, dTermDupes );
}


bool ExtNodeCached_c::GotHitless ()
{
	if ( m_pChild )
		return m_pChild->GotHitless();

	return false;
}


uint64_t ExtNodeCached_c::GetWordID() const
{
	if ( m_pChild )
		return m_pChild->GetWordID();

	return 0;
}


void ExtNodeCached_c::SetCollectHits()
{
	if ( m_pChild )
		m_pChild->SetCollectHits();
}


ExtNodeCached_c::ExtNodeCached_c ( NodeCacheContainer_c * pNode, ExtNode_i * pChild )
	: m_pNode ( pNode )
	, m_pHitDoc ( NULL )
	, m_pWarning ( NULL )
	, m_iMaxTimer ( 0 )
	, m_iHitIndex ( 0 )
	, m_iDocIndex ( 0 )
	, m_pChild ( pChild )
	, m_iQwords ( 0 )
{
	m_iAtomPos = pChild->GetAtomPos();
}


void ExtNodeCached_c::StepForwardToHitsFor ( RowID_t tRowID )
{
	assert ( m_pNode );
	assert ( m_pNode->m_bStateOk );

	CSphVector<ExtHit_t> & dHits = m_pNode->m_dHits;

	int iEnd = dHits.GetLength()-1;
	if ( m_iHitIndex>=iEnd )
		return;

	if ( dHits[m_iHitIndex].m_tRowID==tRowID )
		return;

	m_iHitIndex = sphBinarySearchFirst ( dHits.Begin(), m_iHitIndex, iEnd, bind ( &ExtHit_t::m_tRowID ),tRowID );
}


const ExtDoc_t * ExtNodeCached_c::GetDocsChunk()
{
	if ( !m_pNode || !m_pChild )
		return NULL;

	if ( !m_pNode->m_bStateOk )
		return m_pChild->GetDocsChunk();

	if ( m_iMaxTimer>0 && sph::TimeExceeded ( m_iMaxTimer ) )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return nullptr;
	}

	if ( sph::TimeExceeded ( m_iCheckTimePoint ) )
	{
		if ( session::GetKilled() )
		{
			if ( m_pWarning )
				*m_pWarning = "query was killed";
			return nullptr;
		}
		Threads::Coro::RescheduleAndKeepCrashQuery();
	}

	int iDoc = Min ( m_iDocIndex+MAX_BLOCK_DOCS-1, m_pNode->m_dDocs.GetLength()-1 ) - m_iDocIndex;
	memcpy ( &m_dDocs[0], &m_pNode->m_dDocs[m_iDocIndex], sizeof(ExtDoc_t)*iDoc );
	m_iDocIndex += iDoc;

	// funny trick based on the formula of FIDF calculation.
	for ( int i=0; i<iDoc; i++ )
		m_dDocs[i].m_fTFIDF /= m_iQwords;

	return ReturnDocsChunk ( iDoc, "cached" );
}


bool ExtNodeCached_c::RewindDocs ( const ExtDoc_t * & pDoc, const ExtDoc_t * & pMatched )
{
	do
	{
		while ( pDoc->m_tRowID < pMatched->m_tRowID )
			pDoc++;

		if ( !HasDocs(pDoc) )
			return false; // matched docs block is over for me, gimme another one

		while ( pMatched->m_tRowID < pDoc->m_tRowID )
			pMatched++;

		if ( !HasDocs(pMatched) )
			return false; // matched doc block did not yet begin for me, gimme another one
	}
	while ( pDoc->m_tRowID!=pMatched->m_tRowID );

	// setup hitlist reader
	StepForwardToHitsFor ( pDoc->m_tRowID );

	return true;
}


void ExtNodeCached_c::CollectHits ( const ExtDoc_t * pMatched )
{
	if ( !m_pNode || !m_pChild )
		return;

	if ( !m_pNode->m_bStateOk )
	{
		const ExtHit_t * pHit = m_pChild->GetHits(pMatched);
		while ( HasHits(pHit) )
			m_dHits.Add ( *pHit++ );

		return;
	}

	if ( !pMatched )
		return;

	// aim to the right document
	const ExtDoc_t * pDoc = m_pHitDoc;
	m_pHitDoc = NULL;

	if ( !pDoc )
	{
		// find match
		pDoc = m_dDocs;
		RewindDocs ( pDoc, pMatched );
	}

	// hit emission
	while ( true )
	{
		// get next hit
		ExtHit_t & tCachedHit = m_pNode->m_dHits[m_iHitIndex];
		if ( !HasHits(&tCachedHit) )
			break;

		if ( tCachedHit.m_tRowID==pDoc->m_tRowID )
		{
			m_iHitIndex++;
			ExtHit_t & tHit = m_dHits.Add();
			tHit = tCachedHit;
			tHit.m_uQuerypos = (WORD)( tCachedHit.m_uQuerypos + m_iAtomPos - m_pNode->m_iAtomPos );
		}
		else
		{
			// no more hits; get next acceptable document
			pDoc++;
			if ( !RewindDocs ( pDoc, pMatched ) )
			{
				pDoc = nullptr;
				break;
			}

			assert ( pDoc->m_tRowID==pMatched->m_tRowID );

			// setup hitlist reader
			StepForwardToHitsFor ( pDoc->m_tRowID );
		}
	}

	m_pHitDoc = pDoc;
}

//////////////////////////////////////////////////////////////////////////

CSphQueryNodeCache::CSphQueryNodeCache ( int iCells, int iMaxCachedDocs, int iMaxCachedHits )
{
	if ( iCells>0 && iMaxCachedHits>0 && iMaxCachedDocs>0 )
	{
		m_pPool = new NodeCacheContainer_c [ iCells ];
		for ( int i=0; i<iCells; i++ )
			m_pPool[i].m_pNodeCache = this;
	}
	m_iMaxCachedDocs = iMaxCachedDocs / sizeof(ExtDoc_t);
	m_iMaxCachedHits = iMaxCachedHits / sizeof(ExtHit_t);
}

CSphQueryNodeCache::~CSphQueryNodeCache ()
{
	SafeDeleteArray ( m_pPool );
}

ExtNode_i * CSphQueryNodeCache::CreateProxy ( ExtNode_i * pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup )
{
	// TEMPORARILY DISABLED
	return pChild;

/*	if ( m_iMaxCachedDocs<=0 || m_iMaxCachedHits<=0 )
		return pChild;

	assert ( pRawChild );
	return m_pPool [ pRawChild->GetOrder() ].CreateCachedWrapper ( pChild, pRawChild, tSetup );*/
}

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<ExtNode_i> CreateRowIdFilterNode ( ExtNode_i * pNode, const RowIdBoundaries_t & tBoundaries, bool bClearOnReset )
{
	return std::make_unique<ExtRowIdRange_c> ( pNode, tBoundaries, bClearOnReset );
}


std::unique_ptr<ExtNode_i> CreatePseudoFTNode ( ExtNode_i * pNode, RowidIterator_i * pIterator, bool bClearOnReset )
{
	return std::make_unique<ExtAndRightHits_c> ( new ExtIterator_c(pIterator), pNode, bClearOnReset );
}

/// Immediately interrupt current operation
void sphInterruptNow()
{
	g_bInterruptNow = true;
}

bool sphInterrupted()
{
	return g_bInterruptNow;
}
