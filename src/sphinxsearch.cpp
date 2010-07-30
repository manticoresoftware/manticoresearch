//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxsearch.h"
#include "sphinxquery.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////////
// EXTENDED MATCHING V2
//////////////////////////////////////////////////////////////////////////

#define SPH_TREE_DUMP			0

#define SPH_BM25_K1				1.2f
#define SPH_BM25_SCALE			1000


struct QwordsHash_fn
{
	static inline int Hash ( const CSphString & sKey )
	{
		return sphCRC32 ( (const BYTE *)sKey.cstr() );
	}
};


/// match in the stream
struct ExtDoc_t
{
	SphDocID_t		m_uDocid;
	CSphRowitem *	m_pDocinfo;			///< for inline storage only
	SphOffset_t		m_uHitlistOffset;
	DWORD			m_uFields;
	float			m_fTFIDF;
};


/// hit in the stream
struct ExtHit_t
{
	SphDocID_t	m_uDocid;
	DWORD		m_uHitpos;
	DWORD		m_uQuerypos;
	WORD		m_uSpanlen;
	WORD		m_uMatchlen;
	DWORD		m_uWeight;
};


/// word in the query
struct ExtQword_t
{
	CSphString	m_sWord;		///< word
	CSphString	m_sDictWord;	///< word as processed by dict
	int			m_iDocs;		///< matching documents
	int			m_iHits;		///< matching hits
	float		m_fIDF;			///< IDF value
	int			m_iQueryPos;	///< position in the query
};


/// query words set
typedef CSphOrderedHash < ExtQword_t, CSphString, QwordsHash_fn, 256, 13 > ExtQwordsHash_t;


/// generic match streamer
class ExtNode_i
{
public:
								ExtNode_i ();
	virtual						~ExtNode_i () { SafeDeleteArray ( m_pDocinfo ); }

	static ExtNode_i *			Create ( const XQNode_t * pNode, const ISphQwordSetup & tSetup );
	static ExtNode_i *			Create ( const XQKeyword_t & tWord, DWORD uFields, int iMaxFieldPos, const ISphQwordSetup & tSetup );
	static ExtNode_i *			Create ( ISphQword * pQword, DWORD uFields, int iMaxFieldPos, const ISphQwordSetup & tSetup );

	virtual void				Reset ( const ISphQwordSetup & tSetup ) = 0;
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID ) = 0;
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID ) = 0;

	virtual void				GetQwords ( ExtQwordsHash_t & hQwords ) = 0;
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) = 0;
	virtual bool				GotHitless () = 0;
	virtual int					GetDocsCount () { return INT_MAX; }

	void DebugIndent ( int iLevel )
	{
		while ( iLevel-- )
			printf ( "    " );
	}

	virtual void DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "ExtNode\n" );
	}

public:
	static const int			MAX_DOCS = 512;
	static const int			MAX_HITS = 512;

	int							m_iAtomPos;		///< we now need it on this level for tricks like expanded keywords within phrases

protected:
	ExtDoc_t					m_dDocs[MAX_DOCS];
	ExtHit_t					m_dHits[MAX_HITS];

public:
	SphDocID_t					m_uMaxID;
	int							m_iStride;		///< docinfo stride (for inline mode only)

protected:
	CSphRowitem *				m_pDocinfo;		///< docinfo storage (for inline mode only)

	void AllocDocinfo ( const ISphQwordSetup & tSetup )
	{
		if ( tSetup.m_iInlineRowitems )
		{
			m_iStride = tSetup.m_iInlineRowitems;
			m_pDocinfo = new CSphRowitem [ MAX_DOCS*m_iStride ];
		}
	}

protected:
	inline const ExtDoc_t *		ReturnDocsChunk ( int iCount, SphDocID_t * pMaxID )
	{
		assert ( iCount>=0 && iCount<MAX_DOCS );
		m_dDocs[iCount].m_uDocid = DOCID_MAX;

		m_uMaxID = iCount ? m_dDocs[iCount-1].m_uDocid : 0;
		if ( pMaxID ) *pMaxID = m_uMaxID;

		return iCount ? m_dDocs : NULL;
	}
};


/// single keyword streamer
class ExtTerm_c : public ExtNode_i
{
public:
								ExtTerm_c ( ISphQword * pQword, DWORD uFields, const ISphQwordSetup & tSetup );
								~ExtTerm_c ()
								{
									SafeDelete ( m_pQword );
								}

	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );

	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				GotHitless () { return false; }
	virtual int					GetDocsCount () { return m_pQword->m_iDocs; }

	virtual void DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "ExtTerm: %s at: %d ", m_pQword->m_sWord.cstr(), m_pQword->m_iAtomPos );
		DWORD uFields = m_uFields;
		if ( uFields==0xFFFFFFFFUL )
		{
			printf ( "(all)\n" );
		} else
		{
			bool bFirst = true;
			printf ( "in: " );
			for ( int iField = 1; uFields; uFields >>= 1, iField++ )
			{
				if ( uFields & 1 )
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

protected:
	ISphQword *					m_pQword;
	ExtDoc_t *					m_pHitDoc;		///< points to entry in m_dDocs which GetHitsChunk() currently emits hits for
	SphDocID_t					m_uHitsOverFor;	///< there are no more hits for matches block starting with this ID
	DWORD						m_uFields;		///< accepted fields mask
	float						m_fIDF;			///< IDF for this term (might be 0.0f for non-1st occurences in query)
	int64_t						m_iMaxTimer;	///< work until this timestamp
	CSphString *				m_pWarning;

public:
	static volatile bool		m_bInterruptNow; ///< may be set from outside to indicate the globally received sigterm
};

/// Immediately interrupt current operation
void sphInterruptNow()
{
	ExtTerm_c::m_bInterruptNow = true;
}

volatile bool ExtTerm_c::m_bInterruptNow = false;

/// single keyword streamer with artificial hitlist
class ExtTermHitless_c: public ExtTerm_c
{
public:
								ExtTermHitless_c ( ISphQword * pQword, DWORD uFields, const ISphQwordSetup & tSetup )
									: ExtTerm_c ( pQword, uFields, tSetup )
									, m_uFieldMask ( 0 )
									, m_uFieldPos ( 0 )
								{}
	virtual void				Reset ( const ISphQwordSetup & )
	{
		m_uFieldMask = 0;
		m_uFieldPos = 0;
	}
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual bool				GotHitless () { return true; }

protected:
	DWORD	m_uFieldMask;
	DWORD	m_uFieldPos;
};


/// single keyword streamer, with term position filtering
template < TermPosFilter_e T >
class ExtTermPos_c : public ExtTerm_c
{
public:
								ExtTermPos_c ( ISphQword * pQword, DWORD uFields, int iMaxFieldPos, const ISphQwordSetup & tSetup );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual bool				GotHitless () { return false; }

protected:
	inline bool					IsAcceptableHit ( DWORD uPos ) const;

protected:
	int							m_iMaxFieldPos;
	SphDocID_t					m_uTermMaxID;
	const ExtDoc_t *			m_pRawDocs;					///< chunk start as returned by raw GetDocsChunk() (need to store it for raw GetHitsChunk() calls)
	const ExtDoc_t *			m_pRawDoc;					///< current position in raw docs chunk
	const ExtHit_t *			m_pRawHit;					///< current position in raw hits chunk
	SphDocID_t					m_uLastID;
	enum
	{
		COPY_FILTERED,
		COPY_TRAILING,
		COPY_DONE
	}							m_eState;					///< internal GetHitsChunk() state (are we copying from my hits, or passing trailing raw hits, or done)
	ExtDoc_t					m_dMyDocs[MAX_DOCS];		///< all documents within the required pos range
	ExtHit_t					m_dMyHits[MAX_HITS];		///< all hits within the required pos range
	ExtHit_t					m_dFilteredHits[MAX_HITS];	///< hits from requested subset of the documents (for GetHitsChunk())
	SphDocID_t					m_uDoneFor;
};


/// multi-node binary-operation streamer traits
class ExtTwofer_c : public ExtNode_i
{
public:
								ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup );
								~ExtTwofer_c ();

	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

	virtual bool				GotHitless () { return m_pChildren[0]->GotHitless() || m_pChildren[1]->GotHitless(); }

	void DebugDumpT ( const char * sName, int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "%s:\n", sName );
		m_pChildren[0]->DebugDump ( iLevel+1 );
		m_pChildren[1]->DebugDump ( iLevel+1 );
	}

protected:
	ExtNode_i *					m_pChildren[2];
	const ExtDoc_t *			m_pCurDoc[2];
	const ExtHit_t *			m_pCurHit[2];
	SphDocID_t					m_uMatchedDocid;
};


/// A-and-B streamer
class ExtAnd_c : public ExtTwofer_c
{
public:
								ExtAnd_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup ) : ExtTwofer_c ( pFirst, pSecond, tSetup ) {}
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtAnd", iLevel ); }
};


/// A-or-B streamer
class ExtOr_c : public ExtTwofer_c
{
public:
								ExtOr_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup ) : ExtTwofer_c ( pFirst, pSecond, tSetup ) {}
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtOr", iLevel ); }
};


/// A-and-not-B streamer
class ExtAndNot_c : public ExtTwofer_c
{
public:
								ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual void				Reset ( const ISphQwordSetup & tSetup );

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtAndNot", iLevel ); }

protected:
	bool						m_bPassthrough;
};


/// exact phrase streamer
class ExtPhrase_c : public ExtNode_i
{
public:
								ExtPhrase_c ( CSphVector<ExtNode_i *> & dQwords, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
								~ExtPhrase_c ();
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

	virtual bool				GotHitless () { return false; }

	virtual void DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "ExtPhrase\n" );
		m_pNode->DebugDump ( iLevel+1 );
	}

protected:
	ExtNode_i *					m_pNode;				///< my and-node for all the terms
	const ExtDoc_t *			m_pDocs;				///< current docs chunk from and-node
	SphDocID_t					m_uDocsMaxID;			///< max id in current docs chunk
	const ExtHit_t *			m_pHits;				///< current hits chunk from and-node
	const ExtDoc_t *			m_pDoc;					///< current doc from and-node
	const ExtHit_t *			m_pHit;					///< current hit from and-node
	const ExtDoc_t *			m_pMyDoc;				///< current doc for hits getter
	const ExtHit_t *			m_pMyHit;				///< current hit for hits getter
	DWORD						m_uFields;				///< what fields is the search restricted to
	SphDocID_t					m_uLastDocID;			///< last emitted hit
	SphDocID_t					m_uExpID;
	DWORD						m_uExpPos;
	DWORD						m_uExpQpos;
	DWORD						m_uMinQpos;				///< min qpos through my keywords
	DWORD						m_uMaxQpos;				///< max qpos through my keywords
	CSphVector<int>				m_dQposDelta;			///< next expected qpos delta for each existing qpos (for skipped stopwords case)
	ExtHit_t					m_dMyHits[MAX_HITS];	///< buffer for all my phrase hits; inherited m_dHits will receive filtered results
	SphDocID_t					m_uMatchedDocid;
	SphDocID_t					m_uHitsOverFor;			///< there are no more hits for matches block starting with this ID
	DWORD						m_uWords;				///< number of keywords (might be different from qpos delta because of stops and overshorts)
private:
	virtual bool				EmitTail ( int & iHit );	///< the "trickiest part" extracted in order to process the proximity also
};


/// phrase A-near-phrase B streamer
class ExtNear_c : public ExtNode_i
{
public:
	ExtNear_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup, int iMaxDistance );
	~ExtNear_c();
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				GotHitless () { return false; }
	virtual void				DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "ExtNear\n" );
		m_pNode->DebugDump ( iLevel+1 );
	}

	// the main logical condition
	inline bool IsAppropriateHit ( const ExtHit_t * pLeft, const ExtHit_t * pRight )
	{
		return pLeft && pRight															///< both members exist
			&& pLeft->m_uDocid!=DOCID_MAX && pRight->m_uDocid!=DOCID_MAX				///< both members are valid
			&& pLeft->m_uDocid==pRight->m_uDocid										///< both are about the same doc
			/// and the main NEAR condition-filter
			&& ( ( ( HIT2LCS ( pLeft->m_uHitpos )<HIT2LCS ( pRight->m_uHitpos ) ) && ( HIT2LCS ( pLeft->m_uHitpos ) + pLeft->m_uMatchlen + m_iMaxDistance>HIT2LCS ( pRight->m_uHitpos ) ) )
				| ( ( HIT2LCS ( pLeft->m_uHitpos )>=HIT2LCS ( pRight->m_uHitpos ) ) && ( HIT2LCS ( pRight->m_uHitpos ) + pRight->m_uMatchlen + m_iMaxDistance>HIT2LCS ( pLeft->m_uHitpos ) ) ) );
	}

private:
	int					m_iMaxDistance;			///< the main param of the NEAR
	ExtAnd_c *			m_pNode;				///< the basic filter for the near
	const ExtDoc_t *	m_pDoc;					///< current doc from and-node
	const ExtHit_t *	m_pHitsLeft;			///< the hits from the left part of the expression
	const ExtHit_t *	m_pHitsRight;			///< the hits from the left part of the expression
	ExtNode_i *			m_pNodeLeft;			///< left part (branch) of the expression
	ExtNode_i *			m_pNodeRight;			///< right part (branch) of the expression
	SphDocID_t			m_uMatchedDocid;		///< the current doc for hits extraction
	ExtHit_t			m_dMyHits[MAX_HITS];	///< buffer for all my phrase hits; inherited m_dHits will receive filtered results
	SphDocID_t			m_uHitsOverFor;			///< there are no more hits for matches block starting with this ID
	const ExtDoc_t *	m_pMyDoc;				///< current doc for hits getter
	const ExtHit_t *	m_pMyHit;				///< current hit for hits getter
	SphDocID_t			m_uDocsMaxID;			///< max id in current docs chunk
	SphDocID_t			m_uExpID;
};




/// proximity streamer
class ExtProximity_c : public ExtPhrase_c
{
public:
								ExtProximity_c ( CSphVector<ExtNode_i *> & dQwords, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );

protected:
	int							m_iMaxDistance;
	int							m_iNumWords;
private:
	virtual bool				EmitTail ( int & iHit );	///< proximity-specific
};


/// quorum streamer
class ExtQuorum_c : public ExtNode_i
{
public:
								ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	virtual						~ExtQuorum_c ();

	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );

	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

	virtual bool				GotHitless () { return false; }

protected:
	int							m_iThresh;			///< keyword count threshold
	CSphVector<ExtNode_i*>		m_dChildren;		///< my children nodes (simply ExtTerm_c for now)
	CSphVector<const ExtDoc_t*>	m_pCurDoc;			///< current positions into children doclists
	CSphVector<const ExtHit_t*>	m_pCurHit;			///< current positions into children doclists
	DWORD						m_uMask;			///< mask of nodes that count toward threshold
	DWORD						m_uMaskEnd;			///< index of the last bit in mask
	bool						m_bDone;			///< am i done
	SphDocID_t					m_uMatchedDocid;	///< current docid for hitlist emission

private:
	DWORD						m_uInitialMask;		///< backup mask for Reset()
};


class ExtOrder_c : public ExtNode_i
{
public:
								ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup );
								~ExtOrder_c ();

	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				GotHitless () { return false; }

protected:
	CSphVector<ExtNode_i *>		m_dChildren;
	CSphVector<const ExtDoc_t*>	m_pDocsChunk;	///< last document chunk (for hit fetching)
	CSphVector<const ExtDoc_t*>	m_pDocs;		///< current position in document chunk
	CSphVector<const ExtHit_t*>	m_pHits;		///< current position in hits chunk
	CSphVector<SphDocID_t>		m_dMaxID;		///< max DOCID from the last chunk
	ExtHit_t					m_dMyHits[MAX_HITS];	///< buffer for all my phrase hits; inherited m_dHits will receive filtered results
	bool						m_bDone;
	SphDocID_t					m_uHitsOverFor;
	SphDocID_t					m_uLastMatchID;

protected:
	int							GetNextHit ( SphDocID_t uDocid );										///< get next hit within given document, and return its child-id
	int							GetMatchingHits ( SphDocID_t uDocid, ExtHit_t * pHitbuf, int iLimit );	///< process candidate hits and stores actual matches while we can
};


/// ranker interface
/// ranker folds incoming hitstream into simple match chunks, and computes relevance rank
class ExtRanker_c : public ISphRanker
{
public:
								ExtRanker_c ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup );
	virtual						~ExtRanker_c () { SafeDelete ( m_pRoot ); }
	virtual void				Reset ( const ISphQwordSetup & tSetup );

	virtual CSphMatch *			GetMatchesBuffer () { return m_dMatches; }
	virtual const ExtDoc_t *	GetFilteredDocs ();

	void						GetQwords ( ExtQwordsHash_t & hQwords )				{ if ( m_pRoot ) m_pRoot->GetQwords ( hQwords ); }
	void						SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

public:
	CSphMatch					m_dMatches[ExtNode_i::MAX_DOCS];	///< exposed for caller
	DWORD						m_uPayloadMask;						///< exposed for ranker state functors
	int							m_iQwords;							///< exposed for ranker state functors
	int							m_iMaxQuerypos;						///< exposed for ranker state functors

protected:
	int							m_iInlineRowitems;
	ExtNode_i *					m_pRoot;
	const ExtDoc_t *			m_pDoclist;
	const ExtHit_t *			m_pHitlist;
	SphDocID_t					m_uMaxID;
	ExtDoc_t					m_dMyDocs[ExtNode_i::MAX_DOCS];		///< my local documents pool; for filtering
	CSphMatch					m_dMyMatches[ExtNode_i::MAX_DOCS];	///< my local matches pool; for filtering
	CSphMatch					m_tTestMatch;
	const CSphIndex *			m_pIndex;							///< this is he who'll do my filtering!
	CSphQueryContext *			m_pCtx;
};


STATIC_ASSERT ( ( sizeof(DWORD)*8 )>=SPH_MAX_FIELDS, PAYLOAD_MASK_OVERFLOW );

static const bool WITH_BM25 = true;

template < bool USE_BM25 = false >
class ExtRanker_WeightSum_c : public ExtRanker_c
{
public:
					ExtRanker_WeightSum_c ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup ) : ExtRanker_c ( pRoot, tSetup ) {}
	virtual int		GetMatches ( int iFields, const int * pWeights );
};


class ExtRanker_None_c : public ExtRanker_c
{
public:
					ExtRanker_None_c ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup ) : ExtRanker_c ( pRoot, tSetup ) {}
	virtual int		GetMatches ( int iFields, const int * pWeights );
};


template < typename STATE >
class ExtRanker_T : public ExtRanker_c
{
public:
					ExtRanker_T<STATE> ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup ) : ExtRanker_c ( pRoot, tSetup ) {}
	virtual int		GetMatches ( int iFields, const int * pWeights );
};

//////////////////////////////////////////////////////////////////////////

static inline void CopyExtDocinfo ( ExtDoc_t & tDst, const ExtDoc_t & tSrc, CSphRowitem ** ppRow, int iStride )
{
	if ( tSrc.m_pDocinfo )
	{
		assert ( ppRow && *ppRow );
		memcpy ( *ppRow, tSrc.m_pDocinfo, iStride*sizeof(CSphRowitem) );
		tDst.m_pDocinfo = *ppRow;
		*ppRow += iStride;
	} else
		tDst.m_pDocinfo = NULL;
}

static inline void CopyExtDoc ( ExtDoc_t & tDst, const ExtDoc_t & tSrc, CSphRowitem ** ppRow, int iStride )
{
	tDst = tSrc;
	CopyExtDocinfo ( tDst, tSrc, ppRow, iStride );
}

ExtNode_i::ExtNode_i ()
	: m_iAtomPos(0)
	, m_iStride(0)
	, m_pDocinfo(NULL)
{
	m_dDocs[0].m_uDocid = DOCID_MAX;
	m_dHits[0].m_uDocid = DOCID_MAX;
}


static ISphQword * CreateQueryWord ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup )
{
	BYTE sTmp [ 3*SPH_MAX_WORD_LEN + 16 ];
	strncpy ( (char*)sTmp, tWord.m_sWord.cstr(), sizeof(sTmp) );
	sTmp[sizeof(sTmp)-1] = '\0';

	ISphQword * pWord = tSetup.QwordSpawn ( tWord );
	pWord->m_sWord = tWord.m_sWord;
	pWord->m_iWordID = tSetup.m_pDict->GetWordID ( sTmp );
	pWord->m_sDictWord = (char*)sTmp;
	tSetup.QwordSetup ( pWord );

	if ( tWord.m_bFieldStart && tWord.m_bFieldEnd )	pWord->m_iTermPos = TERM_POS_FIELD_STARTEND;
	else if ( tWord.m_bFieldStart )					pWord->m_iTermPos = TERM_POS_FIELD_START;
	else if ( tWord.m_bFieldEnd )					pWord->m_iTermPos = TERM_POS_FIELD_END;
	else											pWord->m_iTermPos = 0;

	pWord->m_iAtomPos = tWord.m_iAtomPos;
	return pWord;
}


static bool KeywordsEqual ( const XQNode_t * pA, const XQNode_t * pB )
{
	// we expected a keyword here but got composite node; lets drill down until first real keyword
	while ( pA->m_dChildren.GetLength() )
		pA = pA->m_dChildren[0];

	while ( pB->m_dChildren.GetLength() )
		pB = pB->m_dChildren[0];

	// actually check keywords
	assert ( pA->m_dWords.GetLength() );
	assert ( pB->m_dWords.GetLength() );
	return pA->m_dWords[0].m_sWord==pB->m_dWords[0].m_sWord;
}


template < typename T >
static ExtNode_i * CreatePhraseNode ( const XQNode_t * pQueryNode, const ISphQwordSetup & tSetup, bool bNeedsHitlist )
{
	///////////////////////////////////
	// virtually plain (expanded) node
	///////////////////////////////////

	if ( pQueryNode->m_dChildren.GetLength() )
	{
		CSphVector<ExtNode_i *> dNodes;
		CSphVector<DWORD> dWordids;
		ARRAY_FOREACH ( i, pQueryNode->m_dChildren )
		{
			dNodes.Add ( ExtNode_i::Create ( pQueryNode->m_dChildren[i], tSetup ) );
			dNodes.Last()->m_iAtomPos = pQueryNode->m_dChildren[i]->m_iAtomPos;
			assert ( dNodes.Last()->m_iAtomPos>=0 );
		}

		// compute dupe mask (needed for quorum only)
		// FIXME! this check will fail with wordforms and stuff; sorry, no wordforms vs expand vs quorum support for now!
		DWORD uDupeMask = 0;
		ARRAY_FOREACH ( i, pQueryNode->m_dChildren )
		{
			int iValue = 1;
			for ( int j = i+1; j<pQueryNode->m_dChildren.GetLength(); j++ )
				if ( KeywordsEqual ( pQueryNode->m_dChildren[i], pQueryNode->m_dChildren[j] ) )
				{
					iValue = 0;
					break;
				}
				uDupeMask |= iValue << i;
		}

		ExtNode_i * pResult = new T ( dNodes, uDupeMask, *pQueryNode, tSetup );
		if ( pQueryNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pResult, pQueryNode, tSetup );
		return pResult; // FIXME! sorry, no hitless vs expand vs phrase support for now!
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
	if ( dQwordsHit.GetLength()<2 )
	{
		ARRAY_FOREACH ( i, dQwords )
			SafeDelete ( dQwords[i] );
		ARRAY_FOREACH ( i, dQwordsHit )
			SafeDelete ( dQwordsHit[i] );
		if ( tSetup.m_pWarning )
			tSetup.m_pWarning->SetSprintf ( "can't create phrase node, hitlists unavailable (hitlists=%d, nodes=%d)",
				dQwordsHit.GetLength(), dWords.GetLength() );
		return NULL;

	} else
	{
		// at least two words have hitlists, creating phrase node
		assert ( pQueryNode );
		assert ( pQueryNode->m_dWords.GetLength() );
		assert ( pQueryNode->GetOp()==SPH_QUERY_PHRASE || pQueryNode->GetOp()==SPH_QUERY_PROXIMITY || pQueryNode->GetOp()==SPH_QUERY_QUORUM );

		// create nodes
		CSphVector<ExtNode_i *> dNodes;
		ARRAY_FOREACH ( i, dQwordsHit )
		{
			dNodes.Add ( ExtNode_i::Create ( dQwordsHit[i], pQueryNode->m_uFieldMask, pQueryNode->m_iFieldMaxPos, tSetup ) );
			dNodes.Last()->m_iAtomPos = dQwordsHit[i]->m_iAtomPos;
		}

		// compute dupe mask (needed for quorum only)
		DWORD uDupeMask = 0;
		ARRAY_FOREACH ( i, dQwordsHit )
		{
			int iValue = 1;
			for ( int j = i + 1; j < dQwordsHit.GetLength(); j++ )
				if ( dQwordsHit[i]->m_iWordID==dQwordsHit[j]->m_iWordID )
			{
				iValue = 0;
				break;
			}
			uDupeMask |= iValue << i;
		}

		pResult = new T ( dNodes, uDupeMask, *pQueryNode, tSetup );
	}

	// AND result with the words that had no hitlist
	if ( dQwords.GetLength() )
	{
		int iMaxPos = pQueryNode->m_iFieldMaxPos;
		DWORD uFields = pQueryNode->m_uFieldMask;
		ExtNode_i * pNode = ExtNode_i::Create ( dQwords[0], uFields, iMaxPos, tSetup );
		for ( int i=1; i<dQwords.GetLength(); i++ )
			pNode = new ExtAnd_c ( pNode, ExtNode_i::Create ( dQwords[i], uFields, iMaxPos, tSetup ), tSetup );
		pResult = new ExtAnd_c ( pResult, pNode, tSetup );
	}

	if ( pQueryNode->GetCount() )
		return tSetup.m_pNodeCache->CreateProxy ( pResult, pQueryNode, tSetup );

	return pResult;
}

static ExtNode_i * CreateOrderNode ( const XQNode_t * pNode, const ISphQwordSetup & tSetup )
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
		ExtNode_i * pChild = ExtNode_i::Create ( pNode->m_dChildren[i], tSetup );
		if ( pChild->GotHitless() )
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

ExtNode_i * ExtNode_i::Create ( const XQKeyword_t & tWord, DWORD uFields, int iMaxFieldPos, const ISphQwordSetup & tSetup )
{
	return Create ( CreateQueryWord ( tWord, tSetup ), uFields, iMaxFieldPos, tSetup );
};

ExtNode_i * ExtNode_i::Create ( ISphQword * pQword, DWORD uFields, int iMaxFieldPos, const ISphQwordSetup & tSetup )
{
	assert ( pQword );

	if ( iMaxFieldPos )
		pQword->m_iTermPos = TERM_POS_FIELD_LIMIT;

	if ( !pQword->m_bHasHitlist )
	{
		if ( tSetup.m_pWarning && pQword->m_iTermPos )
			tSetup.m_pWarning->SetSprintf ( "hitlist unavailable, position limit ignored" );
		return new ExtTermHitless_c ( pQword, uFields, tSetup );
	}
	switch ( pQword->m_iTermPos )
	{
		case TERM_POS_FIELD_STARTEND:	return new ExtTermPos_c<TERM_POS_FIELD_STARTEND> ( pQword, uFields, 0, tSetup );
		case TERM_POS_FIELD_START:		return new ExtTermPos_c<TERM_POS_FIELD_START> ( pQword, uFields, 0, tSetup );
		case TERM_POS_FIELD_END:		return new ExtTermPos_c<TERM_POS_FIELD_END> ( pQword, uFields, 0, tSetup );
		case TERM_POS_FIELD_LIMIT:		return new ExtTermPos_c<TERM_POS_FIELD_LIMIT> ( pQword, uFields, iMaxFieldPos, tSetup );
		default:						return new ExtTerm_c ( pQword, uFields, tSetup );
	}
}

struct ExtNodeTF_fn
{
	bool IsLess ( ExtNode_i * pA, ExtNode_i * pB ) const
	{
		return pA->GetDocsCount() < pB->GetDocsCount();
	}
};

ExtNode_i * ExtNode_i::Create ( const XQNode_t * pNode, const ISphQwordSetup & tSetup )
{
	// empty node?
	if ( pNode->IsEmpty() )
		return NULL;

	if ( pNode->m_dWords.GetLength() || pNode->m_bVirtuallyPlain )
	{
		const int iWords = pNode->m_bVirtuallyPlain
			? pNode->m_dChildren.GetLength()
			: pNode->m_dWords.GetLength();

		if ( iWords==1 )
		{
			if ( pNode->m_bVirtuallyPlain )
				return Create ( pNode->m_dChildren[0], tSetup );
			else
				return Create ( pNode->m_dWords[0], pNode->m_uFieldMask, pNode->m_iFieldMaxPos, tSetup );
		}

		switch ( pNode->GetOp() )
		{
			case SPH_QUERY_PHRASE:
				return CreatePhraseNode<ExtPhrase_c> ( pNode, tSetup, true );

			case SPH_QUERY_PROXIMITY:
				return CreatePhraseNode<ExtProximity_c> ( pNode, tSetup, true );

			case SPH_QUERY_QUORUM:
			{
				if ( pNode->m_iOpArg>=pNode->m_dWords.GetLength() )
				{
					// threshold is too high
					if ( tSetup.m_pWarning && pNode->m_iOpArg>pNode->m_dWords.GetLength() )
						tSetup.m_pWarning->SetSprintf ( "quorum threshold too high (words=%d, thresh=%d); replacing quorum operator with AND operator",
							pNode->m_dWords.GetLength(), pNode->m_iOpArg );

				} else if ( pNode->m_dWords.GetLength()>32 )
				{
					// right now quorum can only handle 32 words
					if ( tSetup.m_pWarning )
						tSetup.m_pWarning->SetSprintf ( "too many words (%d) for quorum; replacing with an AND", pNode->m_dWords.GetLength() );

				} else // everything is ok; create quorum node
					return CreatePhraseNode<ExtQuorum_c> ( pNode, tSetup, false );

				// couldn't create quorum, make an AND node instead
				const CSphVector<XQKeyword_t> & dWords = pNode->m_dWords;
				ExtNode_i * pCur = Create ( dWords[0], pNode->m_uFieldMask, pNode->m_iFieldMaxPos, tSetup );
				for ( int i=1; i<dWords.GetLength(); i++ )
					pCur = new ExtAnd_c ( pCur, Create ( dWords[i], pNode->m_uFieldMask, pNode->m_iFieldMaxPos, tSetup ), tSetup );
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
		if ( pNode->GetOp()==SPH_QUERY_BEFORE )
			return CreateOrderNode ( pNode, tSetup );

		// special case, AND over terms (internally reordered for speed)
		bool bAndTerms = ( pNode->GetOp()==SPH_QUERY_AND );
		for ( int i=0; i<iChildren && bAndTerms; i++ )
		{
			const XQNode_t * pChildren = pNode->m_dChildren[i];
			bAndTerms = ( pChildren->m_dWords.GetLength()==1 );
		}
		if ( bAndTerms )
		{
			CSphVector<ExtNode_i*> dTerms;
			for ( int i=0; i<iChildren; i++ )
			{
				const XQNode_t * pChild = pNode->m_dChildren[i];
				dTerms.Add ( Create ( pChild->m_dWords[0], pChild->m_uFieldMask, pChild->m_iFieldMaxPos, tSetup ) );
			}

			dTerms.Sort ( ExtNodeTF_fn() );

			ExtNode_i * pCur = dTerms[0];
			for ( int i=1; i<iChildren; i++ )
				pCur = new ExtAnd_c ( pCur, dTerms[i], tSetup );
			return pCur;
		}

		// generic create
		ExtNode_i * pCur = NULL;
		for ( int i=0; i<iChildren; i++ )
		{
			ExtNode_i * pNext = ExtNode_i::Create ( pNode->m_dChildren[i], tSetup );
			if ( !pNext ) continue;
			if ( !pCur )
			{
				pCur = pNext;
				continue;
			}
			switch ( pNode->GetOp() )
			{
				case SPH_QUERY_OR:		pCur = new ExtOr_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_AND:		pCur = new ExtAnd_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_NEAR:	pCur = new ExtNear_c ( pCur, pNext, tSetup, pNode->m_iOpArg ); break;
				case SPH_QUERY_ANDNOT:	pCur = new ExtAndNot_c ( pCur, pNext, tSetup ); break;
				default:				assert ( 0 && "internal error: unhandled op in ExtNode_i::Create()" ); break;
			}
		}
		if ( pNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
		return pCur;
	}
}

//////////////////////////////////////////////////////////////////////////

ExtTerm_c::ExtTerm_c ( ISphQword * pQword, DWORD uFields, const ISphQwordSetup & tSetup )
	: m_pQword ( pQword )
	, m_pWarning ( tSetup.m_pWarning )
{
	m_iAtomPos = pQword->m_iAtomPos;
	m_pHitDoc = NULL;
	m_uHitsOverFor = 0;
	m_uFields = uFields;
	m_iMaxTimer = tSetup.m_iMaxTimer;

	AllocDocinfo ( tSetup );
}

void ExtTerm_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pHitDoc = NULL;
	m_uHitsOverFor = 0;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pQword->Reset ();
	tSetup.QwordSetup ( m_pQword );
}

const ExtDoc_t * ExtTerm_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	if ( !m_pQword->m_iDocs )
		return NULL;

	m_uMaxID = 0;

	// max_query_time
	if ( m_iMaxTimer>0 && sphMicroTimer()>=m_iMaxTimer )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	// interrupt by sitgerm
	if ( m_bInterruptNow )
	{
		if ( m_pWarning )
			*m_pWarning = "Server shutdown in progress";
		return NULL;
	}

	int iDoc = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	while ( iDoc<MAX_DOCS-1 )
	{
		const CSphMatch & tMatch = m_pQword->GetNextDoc ( pDocinfo );
		if ( !tMatch.m_iDocID )
		{
			m_pQword->m_iDocs = 0;
			break;
		}
		if (!( m_pQword->m_uFields & m_uFields ))
			continue;

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_uDocid = tMatch.m_iDocID;
		tDoc.m_pDocinfo = pDocinfo;
		tDoc.m_uHitlistOffset = m_pQword->m_iHitlistPos;
		tDoc.m_uFields = m_pQword->m_uFields & m_uFields; // OPTIMIZE: only needed for phrase node
		tDoc.m_fTFIDF = float(m_pQword->m_uMatchHits) / float(m_pQword->m_uMatchHits+SPH_BM25_K1) * m_fIDF;
		pDocinfo += m_iStride;
	}

	m_pHitDoc = NULL;

	return ReturnDocsChunk ( iDoc, pMaxID );
}

const ExtHit_t * ExtTerm_c::GetHitsChunk ( const ExtDoc_t * pMatched, SphDocID_t uMaxID )
{
	if ( !pMatched )
		return NULL;
	SphDocID_t uFirstMatch = pMatched->m_uDocid;

	// aim to the right document
	ExtDoc_t * pDoc = m_pHitDoc;
	m_pHitDoc = NULL;

	if ( !pDoc )
	{
		// if we already emitted hits for this matches block, do not do that again
		if ( uFirstMatch==m_uHitsOverFor )
			return NULL;

		// early reject whole block
		if ( pMatched->m_uDocid > m_uMaxID ) return NULL;
		if ( m_uMaxID && m_dDocs[0].m_uDocid > uMaxID ) return NULL;

		// find match
		pDoc = m_dDocs;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			if ( pDoc->m_uDocid==DOCID_MAX )
			{
				m_uHitsOverFor = uFirstMatch;
				return NULL; // matched docs block is over for me, gimme another one
			}

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX )
			{
				m_uHitsOverFor = uFirstMatch;
				return NULL; // matched doc block did not yet begin for me, gimme another one
			}
		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		// setup hitlist reader
		m_pQword->SeekHitlist ( pDoc->m_uHitlistOffset );
	}

	// hit emission
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// get next hit
		DWORD uHit = m_pQword->GetNextHit();
		if ( !uHit )
		{
			// no more hits; get next acceptable document
			pDoc++;
			do
			{
				while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
				if ( pDoc->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched docs block is over for me, gimme another one

				while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
				if ( pMatched->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched doc block did not yet begin for me, gimme another one
			} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

			if ( !pDoc )
				break;
			assert ( pDoc->m_uDocid==pMatched->m_uDocid );

			// setup hitlist reader
			m_pQword->SeekHitlist ( pDoc->m_uHitlistOffset );
			continue;
		}

		if (!( m_uFields & ( 1UL<<HIT2FIELD(uHit) ) ))
			continue;

		ExtHit_t & tHit = m_dHits[iHit++];
		tHit.m_uDocid = pDoc->m_uDocid;
		tHit.m_uHitpos = uHit;
		tHit.m_uQuerypos = m_iAtomPos;
		tHit.m_uMatchlen = tHit.m_uSpanlen = tHit.m_uWeight = 1;
	}

	m_pHitDoc = pDoc;
	if ( iHit==0 || iHit<MAX_HITS-1 )
		m_uHitsOverFor = uFirstMatch;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

void ExtTerm_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_fIDF = 0.0f;
	if ( hQwords.Exists ( m_pQword->m_sWord ) )
		return;

	m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = m_pQword->m_sWord;
	tInfo.m_sDictWord = m_pQword->m_sDictWord;
	tInfo.m_iDocs = m_pQword->m_iDocs;
	tInfo.m_iHits = m_pQword->m_iHits;
	tInfo.m_iQueryPos = m_pQword->m_iAtomPos;
	tInfo.m_fIDF = -1.0f; // suppress gcc 4.2.3 warning
	hQwords.Add ( tInfo, m_pQword->m_sWord );
}

void ExtTerm_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	if ( m_fIDF<0.0f )
	{
		assert ( hQwords ( m_pQword->m_sWord ) );
		m_fIDF = hQwords ( m_pQword->m_sWord )->m_fIDF;
	}
}

//////////////////////////////////////////////////////////////////////////

const ExtHit_t * ExtTermHitless_c::GetHitsChunk ( const ExtDoc_t * pMatched, SphDocID_t uMaxID )
{
	if ( !pMatched )
		return NULL;
	SphDocID_t uFirstMatch = pMatched->m_uDocid;

	// aim to the right document
	ExtDoc_t * pDoc = m_pHitDoc;
	m_pHitDoc = NULL;

	if ( !pDoc )
	{
		// if we already emitted hits for this matches block, do not do that again
		if ( uFirstMatch==m_uHitsOverFor )
			return NULL;

		// early reject whole block
		if ( pMatched->m_uDocid > m_uMaxID ) return NULL;
		if ( m_uMaxID && m_dDocs[0].m_uDocid > uMaxID ) return NULL;

		// find match
		pDoc = m_dDocs;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			if ( pDoc->m_uDocid==DOCID_MAX )
			{
				m_uHitsOverFor = uFirstMatch;
				return NULL; // matched docs block is over for me, gimme another one
			}

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX )
			{
				m_uHitsOverFor = uFirstMatch;
				return NULL; // matched doc block did not yet begin for me, gimme another one
			}
		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		m_uFieldMask = pDoc->m_uFields;
		m_uFieldPos = 0;
	}

	// hit emission
	int iHit = 0;
	for ( ;; )
	{
		if ( m_uFieldMask & 1 )
		{
			if ( m_uFields & ( 1UL<<m_uFieldPos ) )
			{
				// emit hit
				ExtHit_t & tHit = m_dHits[iHit++];
				tHit.m_uDocid = pDoc->m_uDocid;
				tHit.m_uHitpos = HIT_PACK ( m_uFieldPos, -1 );
				tHit.m_uQuerypos = m_iAtomPos;
				tHit.m_uMatchlen = tHit.m_uSpanlen = tHit.m_uWeight = 1;

				if ( iHit==MAX_HITS-1 )
					break;
			}
		}

		if ( m_uFieldMask >>= 1 )
		{
			m_uFieldPos++;
			continue;
		}

		// field mask is empty, get next document
		pDoc++;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			if ( pDoc->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched docs block is over for me, gimme another one

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched doc block did not yet begin for me, gimme another one
		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		if ( !pDoc )
			break;

		m_uFieldMask = pDoc->m_uFields;
		m_uFieldPos = 0;
	}

	m_pHitDoc = pDoc;
	if ( iHit==0 || iHit<MAX_HITS-1 )
		m_uHitsOverFor = uFirstMatch;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

template < TermPosFilter_e T >
ExtTermPos_c<T>::ExtTermPos_c ( ISphQword * pQword, DWORD uFields, int iMaxFieldPos, const ISphQwordSetup & tSetup )
	: ExtTerm_c			( pQword, uFields, tSetup )
	, m_iMaxFieldPos	( iMaxFieldPos )
	, m_uTermMaxID		( 0 )
	, m_pRawDocs		( NULL )
	, m_pRawDoc		( NULL )
	, m_pRawHit		( NULL )
	, m_uLastID			( 0 )
	, m_uDoneFor	( 0 )
{
	AllocDocinfo ( tSetup );
}

template < TermPosFilter_e T >
void ExtTermPos_c<T>::Reset ( const ISphQwordSetup & tSetup )
{
	ExtTerm_c::Reset(tSetup);
	m_uTermMaxID = 0;
	m_pRawDocs = NULL;
	m_pRawDoc = NULL;
	m_pRawHit = NULL;
	m_uLastID = 0;
	m_uDoneFor = 0;
}

template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_LIMIT>::IsAcceptableHit ( DWORD uPos ) const
{
	return (int)HIT2POS(uPos)<=m_iMaxFieldPos;
}


template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_START>::IsAcceptableHit ( DWORD uPos ) const
{
	return (int)HIT2POS(uPos)==1;
}


template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_END>::IsAcceptableHit ( DWORD uPos ) const
{
	return ( uPos & HIT_FIELD_END )!=0;
}


template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_STARTEND>::IsAcceptableHit ( DWORD uPos ) const
{
	return (int)HIT2POS(uPos)==1 && ( uPos & HIT_FIELD_END )!=0;
}


template < TermPosFilter_e T >
const ExtDoc_t * ExtTermPos_c<T>::GetDocsChunk ( SphDocID_t * pMaxID )
{
	// fetch more docs if needed
	if ( !m_pRawDocs )
	{
		m_pRawDocs = ExtTerm_c::GetDocsChunk ( &m_uTermMaxID );
		if ( !m_pRawDocs )
			return NULL;

		m_pRawDoc = m_pRawDocs;
		m_pRawHit = NULL;
	}

	// filter the hits, and build the documents list
	int iMyDoc = 0;
	int iMyHit = 0;

	const ExtDoc_t * pDoc = m_pRawDoc; // just a shortcut
	const ExtHit_t * pHit = m_pRawHit;
	SphDocID_t uLastID = m_uLastID = 0;

	CSphRowitem * pDocinfo = m_pDocinfo;
	for ( ;; )
	{
		// try to fetch more hits for current raw docs block if we're out
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
			pHit = ExtTerm_c::GetHitsChunk ( m_pRawDocs, m_uTermMaxID );

		// did we touch all the hits we had? if so, we're fully done with
		// current raw docs block, and should start a new one
		if ( !pHit )
		{
			m_pRawDocs = ExtTerm_c::GetDocsChunk ( &m_uTermMaxID );
			if ( !m_pRawDocs ) // no more incoming documents? bail
				break;

			pDoc = m_pRawDocs;
			pHit = NULL;
			continue;
		}

		// scan until next acceptable hit
		while ( pHit->m_uDocid < pDoc->m_uDocid ) // skip leftovers
			pHit++;

		while ( pHit->m_uDocid!=DOCID_MAX && !IsAcceptableHit ( pHit->m_uHitpos ) ) // skip unneeded hits
			pHit++;

		if ( pHit->m_uDocid==DOCID_MAX ) // check for eof
			continue;

		// find and emit new document
		while ( pDoc->m_uDocid<pHit->m_uDocid ) pDoc++; // FIXME? unsafe in broken cases
		assert ( pDoc->m_uDocid==pHit->m_uDocid );

		if ( uLastID!=pDoc->m_uDocid )
			CopyExtDoc ( m_dMyDocs[iMyDoc++], *pDoc, &pDocinfo, m_iStride );
		uLastID = pDoc->m_uDocid;

		// copy acceptable hits for this document
		while ( iMyHit<MAX_HITS-1 && pHit->m_uDocid==uLastID )
		{
			if ( IsAcceptableHit ( pHit->m_uHitpos ) )
				m_dMyHits[iMyHit++] = *pHit;
			pHit++;
		}

		if ( iMyHit==MAX_HITS-1 )
		{
			// there is no more space for acceptable hits; but further calls to GetHits() *might* produce some
			// we need to memorize the trailing document id
			m_uLastID = uLastID;
			break;
		}
	}

	m_pRawDoc = pDoc;
	m_pRawHit = pHit;

	assert ( iMyDoc>=0 && iMyDoc<MAX_DOCS );
	assert ( iMyHit>=0 && iMyHit<MAX_DOCS );

	m_dMyDocs[iMyDoc].m_uDocid = DOCID_MAX;
	m_dMyHits[iMyHit].m_uDocid = DOCID_MAX;
	m_eState = COPY_FILTERED;

	m_uMaxID = iMyDoc ? m_dMyDocs[iMyDoc-1].m_uDocid : 0;
	if ( pMaxID ) *pMaxID = m_uMaxID;

	return iMyDoc ? m_dMyDocs : NULL;
}


template < TermPosFilter_e T >
const ExtHit_t * ExtTermPos_c<T>::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t ) // OPTIMIZE: could possibly use uMaxID
{
	if ( m_eState==COPY_DONE )
	{
		// this request completed in full
		if ( m_uDoneFor==pDocs->m_uDocid )
			return NULL;

		// old request completed in full, but we have a new hits subchunk request now
		// even though there were no new docs requests in the meantime!
		m_eState = COPY_FILTERED;
	}
	m_uDoneFor = pDocs->m_uDocid;

	// regular case
	// copy hits for requested docs from my hits to filtered hits, and return those
	int iFilteredHits = 0;

	if ( m_eState==COPY_FILTERED )
	{
		const ExtHit_t * pMyHit = m_dMyHits;
		for ( ;; )
		{
			// skip hits that the caller is not interested in
			while ( pMyHit->m_uDocid < pDocs->m_uDocid )
				pMyHit++;

			// out of acceptable hits?
			if ( pMyHit->m_uDocid==DOCID_MAX )
			{
				// do we have a trailing document? if yes, we should also copy trailing hits
				m_eState = m_uLastID ? COPY_TRAILING : COPY_DONE;
				break;
			}

			// skip docs that i do not have
			while ( pDocs->m_uDocid < pMyHit->m_uDocid )
				pDocs++;

			// out of requested docs? over and out
			if ( pDocs->m_uDocid==DOCID_MAX )
			{
				m_eState = COPY_DONE;
				break;
			}

			// copy matching hits
			while ( iFilteredHits<MAX_HITS-1 && pDocs->m_uDocid==pMyHit->m_uDocid )
				m_dFilteredHits[iFilteredHits++] = *pMyHit++;

			// paranoid check that we're not out of bounds
			assert ( iFilteredHits<=MAX_HITS-1 && pDocs->m_uDocid!=pMyHit->m_uDocid );
		}
	}

	// trailing hits case
	// my hits did not have enough space, so we should pass raw hits for the last doc
	while ( m_eState==COPY_TRAILING && m_uLastID && iFilteredHits<MAX_HITS-1 )
	{
		// where do we stand?
		if ( !m_pRawHit || m_pRawHit->m_uDocid==DOCID_MAX )
			m_pRawHit = ExtTerm_c::GetHitsChunk ( m_pRawDocs, m_uTermMaxID );

		// no more hits for current chunk
		if ( !m_pRawHit )
		{
			m_eState = COPY_DONE;
			break;
		}

		// copy while we can
		while ( m_pRawHit->m_uDocid==m_uLastID && iFilteredHits<MAX_HITS-1 )
		{
			if ( IsAcceptableHit ( m_pRawHit->m_uHitpos ) )
				m_dFilteredHits[iFilteredHits++] = *m_pRawHit;
			m_pRawHit++;
		}

		// raise the flag for future calls if trailing hits are over
		if ( m_pRawHit->m_uDocid!=m_uLastID && m_pRawHit->m_uDocid!=DOCID_MAX )
			m_eState = COPY_DONE;

		// in any case, this chunk is over
		break;
	}

	m_dFilteredHits[iFilteredHits].m_uDocid = DOCID_MAX;
	return iFilteredHits ? m_dFilteredHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtTwofer_c::ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup )
{
	m_pChildren[0] = pFirst;
	m_pChildren[1] = pSecond;
	m_pCurHit[0] = NULL;
	m_pCurHit[1] = NULL;
	m_pCurDoc[0] = NULL;
	m_pCurDoc[1] = NULL;
	m_uMatchedDocid = 0;
	m_iAtomPos = ( pFirst && pFirst->m_iAtomPos ) ? pFirst->m_iAtomPos : 0;
	if ( pSecond && pSecond->m_iAtomPos && pSecond->m_iAtomPos<m_iAtomPos && m_iAtomPos!=0 )
		m_iAtomPos = pSecond->m_iAtomPos;
	AllocDocinfo ( tSetup );
}

ExtTwofer_c::~ExtTwofer_c ()
{
	SafeDelete ( m_pChildren[0] );
	SafeDelete ( m_pChildren[1] );
}

void ExtTwofer_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pChildren[0]->Reset ( tSetup );
	m_pChildren[1]->Reset ( tSetup );
	m_pCurHit[0] = NULL;
	m_pCurHit[1] = NULL;
	m_pCurDoc[0] = NULL;
	m_pCurDoc[1] = NULL;
	m_uMatchedDocid = 0;
}

void ExtTwofer_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_pChildren[0]->GetQwords ( hQwords );
	m_pChildren[1]->GetQwords ( hQwords );
}

void ExtTwofer_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pChildren[0]->SetQwordsIDF ( hQwords );
	m_pChildren[1]->SetQwordsIDF ( hQwords );
}

//////////////////////////////////////////////////////////////////////////

const ExtDoc_t * ExtAnd_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	m_uMaxID = 0;
	const ExtDoc_t * pCur0 = m_pCurDoc[0];
	const ExtDoc_t * pCur1 = m_pCurDoc[1];

	int iDoc = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	for ( ;; )
	{
		// if any of the pointers is empty, *and* there is no data yet, process next child chunk
		// if there is data, we can't advance, because child hitlist offsets would be lost
		if ( !pCur0 || !pCur1 )
		{
			if ( iDoc!=0 )
				break;

			if ( !pCur0 ) pCur0 = m_pChildren[0]->GetDocsChunk ( NULL );
			if ( !pCur1 ) pCur1 = m_pChildren[1]->GetDocsChunk ( NULL );
			if ( !pCur0 || !pCur1 )
			{
				m_pCurDoc[0] = NULL;
				m_pCurDoc[1] = NULL;
				return NULL;
			}
		}

		// find common matches
		assert ( pCur0 && pCur1 );
		while ( iDoc<MAX_DOCS-1 )
		{
			// find next matching docid
			while ( pCur0->m_uDocid < pCur1->m_uDocid ) pCur0++;
			if ( pCur0->m_uDocid==DOCID_MAX ) { pCur0 = NULL; break; }

			while ( pCur1->m_uDocid < pCur0->m_uDocid ) pCur1++;
			if ( pCur1->m_uDocid==DOCID_MAX ) { pCur1 = NULL; break; }

			if ( pCur0->m_uDocid!=pCur1->m_uDocid ) continue;

			// emit it
			ExtDoc_t & tDoc = m_dDocs[iDoc++];
			tDoc.m_uDocid = pCur0->m_uDocid;
			tDoc.m_uFields = pCur0->m_uFields | pCur1->m_uFields;
			tDoc.m_uHitlistOffset = -1;
			tDoc.m_fTFIDF = pCur0->m_fTFIDF + pCur1->m_fTFIDF;
			CopyExtDocinfo ( tDoc, *pCur0, &pDocinfo, m_iStride );

			// skip it
			pCur0++; if ( pCur0->m_uDocid==DOCID_MAX ) pCur0 = NULL;
			pCur1++; if ( pCur1->m_uDocid==DOCID_MAX ) pCur1 = NULL;
			if ( !pCur0 || !pCur1 ) break;
		}
	}

	m_pCurDoc[0] = pCur0;
	m_pCurDoc[1] = pCur1;

	return ReturnDocsChunk ( iDoc, pMaxID );
}

const ExtHit_t * ExtAnd_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
{
	const ExtHit_t * pCur0 = m_pCurHit[0];
	const ExtHit_t * pCur1 = m_pCurHit[1];

	if ( m_uMatchedDocid < pDocs->m_uDocid )
		m_uMatchedDocid = 0;

	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// emit hits, while possible
		if ( m_uMatchedDocid!=0
			&& m_uMatchedDocid!=DOCID_MAX
			&& ( ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid ) || ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid ) ) )
		{
			// merge, while possible
			if ( pCur0 && pCur1 && pCur0->m_uDocid==m_uMatchedDocid && pCur1->m_uDocid==m_uMatchedDocid )
				while ( iHit<MAX_HITS-1 )
			{
				if ( pCur0->m_uHitpos < pCur1->m_uHitpos )
				{
					m_dHits[iHit++] = *pCur0++;
					if ( pCur0->m_uDocid!=m_uMatchedDocid )
						break;
				} else
				{
					m_dHits[iHit++] = *pCur1++;
					if ( pCur1->m_uDocid!=m_uMatchedDocid )
						break;
				}
			}

			// copy tail, while possible, unless the other child is at the end of a hit block
			if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid && !( pCur1 && pCur1->m_uDocid==DOCID_MAX ) )
			{
				while ( pCur0->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur0++;
			}
			if ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid && !( pCur0 && pCur0->m_uDocid==DOCID_MAX ) )
			{
				while ( pCur1->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur1++;
			}
		}

		// move on
		if ( ( pCur0 && pCur0->m_uDocid!=m_uMatchedDocid && pCur0->m_uDocid!=DOCID_MAX )
			&& ( pCur1 && pCur1->m_uDocid!=m_uMatchedDocid && pCur1->m_uDocid!=DOCID_MAX ) )
				m_uMatchedDocid = 0;

		// warmup if needed
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX ) pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs, uMaxID );
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX ) pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs, uMaxID );

		// one of the hitlists is over
		if ( !pCur0 || !pCur1 )
		{
			if ( !pCur0 && !pCur1 ) break; // both are over, we're done

			// one is over, but we still need to copy the other one
			m_uMatchedDocid = pCur0 ? pCur0->m_uDocid : pCur1->m_uDocid;
			assert ( m_uMatchedDocid!=DOCID_MAX );
			continue;
		}

		// find matching doc
		assert ( pCur1 && pCur0 );
		while ( !m_uMatchedDocid )
		{
			while ( pCur0->m_uDocid < pCur1->m_uDocid ) pCur0++;
			if ( pCur0->m_uDocid==DOCID_MAX ) break;

			while ( pCur1->m_uDocid < pCur0->m_uDocid ) pCur1++;
			if ( pCur1->m_uDocid==DOCID_MAX ) break;

			if ( pCur0->m_uDocid==pCur1->m_uDocid ) m_uMatchedDocid = pCur0->m_uDocid;
		}
	}

	m_pCurHit[0] = pCur0;
	m_pCurHit[1] = pCur1;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return iHit ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtNear_c::ExtNear_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup, int iMaxDistance ) :
	m_iMaxDistance ( iMaxDistance ) ,
	m_pDoc ( NULL ),
	m_pHitsLeft ( NULL ),
	m_pHitsRight ( NULL ),
	m_pNodeLeft ( pFirst ),
	m_pNodeRight ( pSecond ),
	m_uMatchedDocid ( 0 ),
	m_pMyDoc ( NULL ),
	m_pMyHit ( NULL ),
	m_uDocsMaxID ( 0 ),
	m_uExpID ( 0 )
{
	assert ( pFirst );
	assert ( pSecond );
	m_pNode = new ExtAnd_c ( pFirst, pSecond, tSetup );
}

ExtNear_c::~ExtNear_c()
{
	SafeDelete ( m_pNode );
}

const ExtDoc_t * ExtNear_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	m_uMaxID = 0;

	// the shortcuts
	const ExtDoc_t * pDoc = m_pDoc;
	const ExtHit_t * pHitLeft = m_pHitsLeft;
	const ExtHit_t * pHitRight = m_pHitsRight;

	CSphRowitem * pDocinfo = m_pDocinfo;
	SphDocID_t uCurDocID = DOCID_MAX;
	enum State_e { ST_ENTERED, ST_NODOCS, ST_EXAMINEDOC, ST_LHITSSEEK, ST_RHITSSEEK,
			ST_LHITSEMIT, ST_RHITSEMIT, ST_EMITHITS, ST_STEPNEXTDOC, ST_STEPNEXTHIT,
			ST_DOSEEKDOC, ST_DOEMITHITS, ST_DOCFINISHED, ST_STEPFINALIZE, ST_FINALIZE, ST_FINISHED };

	int iHit = 0;
	int iDoc = 0;
	State_e eState = ST_ENTERED;

	while ( eState!=ST_FINISHED )
		switch ( eState )
		{
		// the initial state
		case ST_ENTERED:
			if ( !pDoc || pDoc->m_uDocid==DOCID_MAX )
			{
				eState = ST_NODOCS;
				break;
			}
			eState = ST_EXAMINEDOC;
			// break? No, no break, just keep going!
		// Check if the current doc is appropriate, or collect the hits from already approved doc
		case ST_EXAMINEDOC:
		case ST_EMITHITS:
			if ( !pHitLeft || pHitLeft->m_uDocid==DOCID_MAX )
			{
				eState = ( eState==ST_EXAMINEDOC ) ? ST_LHITSSEEK : ST_LHITSEMIT;
				break;
			}
			if ( !pHitRight || pHitRight->m_uDocid==DOCID_MAX )
			{
				eState = ( eState==ST_EXAMINEDOC ) ? ST_RHITSSEEK : ST_RHITSEMIT;
				break;
			}
			if ( pHitLeft->m_uDocid==pHitRight->m_uDocid )
				eState = ( eState==ST_EXAMINEDOC ) ? ST_DOSEEKDOC : ST_DOEMITHITS;
			else if ( pHitRight->m_uDocid > pHitLeft->m_uDocid )
				pHitLeft++;
			else if ( pHitLeft->m_uDocid > pHitRight->m_uDocid )
				pHitRight++;
			break;
		// Actually examine, and then accept or decline the found doc
		case ST_DOSEEKDOC:
			if ( IsAppropriateHit ( pHitLeft, pHitRight ) )
			{
				ExtDoc_t & tDoc = m_dDocs[iDoc++];
				tDoc.m_uDocid = uCurDocID = pHitLeft->m_uDocid;
				tDoc.m_uFields = pDoc->m_uFields;
				tDoc.m_uHitlistOffset = -1;
				tDoc.m_fTFIDF = pDoc->m_fTFIDF;
				CopyExtDocinfo ( tDoc, *pDoc, &pDocinfo, m_iStride );
				eState = ST_DOEMITHITS;
				break;
			}
			eState = ST_STEPNEXTDOC;
			break;
		// Actually examine, and then emit or decline the found hit
		case ST_DOEMITHITS:
			if ( IsAppropriateHit ( pHitLeft, pHitRight ) )
			{
				if ( pHitLeft->m_uDocid!=uCurDocID )
				{
					eState = ST_DOCFINISHED;
					break;
				}
				int iDistance = HIT2LCS ( pHitRight->m_uHitpos ) - HIT2LCS ( pHitLeft->m_uHitpos );
				const ExtHit_t *&pL = (iDistance>0)?pHitLeft:pHitRight;
				const ExtHit_t *&pR = (iDistance>0)?pHitRight:pHitLeft;
				iDistance = (iDistance>0)?iDistance:-iDistance;
				m_dMyHits[iHit].m_uDocid = uCurDocID;
				m_dMyHits[iHit].m_uHitpos = pL->m_uHitpos;
				m_dMyHits[iHit].m_uQuerypos = pL->m_uQuerypos;
				m_dMyHits[iHit].m_uMatchlen = iDistance + pR->m_uMatchlen;
				m_dMyHits[iHit++].m_uWeight = pHitLeft->m_uWeight + pHitRight->m_uWeight;
				if ( iHit==MAX_HITS-1 )
				{
					eState = ST_STEPFINALIZE;
					break;
				}
			}
			eState = ST_STEPNEXTHIT;
			// break? No, no break, just keep going!
		// The pair of hits processed; step to the next one
		case ST_STEPNEXTDOC:
		case ST_STEPNEXTHIT:
		case ST_STEPFINALIZE:
			if ( pHitLeft->m_uHitpos > pHitRight->m_uHitpos )
				pHitRight++;
			else
				pHitLeft++;
			if ( eState==ST_STEPFINALIZE )
				eState = ST_FINALIZE;
			else if ( eState==ST_STEPNEXTDOC )
				eState = ST_EXAMINEDOC;
			else
				eState = ST_EMITHITS;
			break;
		// The left branch of the hist is over; Give me more
		case ST_LHITSSEEK:
		case ST_LHITSEMIT:
			pHitLeft = m_pNodeLeft->GetHitsChunk ( pDoc, m_uDocsMaxID );
			if ( !pHitLeft )
				eState = ST_FINALIZE;
			else
				eState = ( eState==ST_LHITSSEEK ) ? ST_EXAMINEDOC : ST_EMITHITS;
			break;
		// The right branch of the hits is over; Give me more
		case ST_RHITSSEEK:
		case ST_RHITSEMIT:
			pHitRight = m_pNodeRight->GetHitsChunk ( pDoc, m_uDocsMaxID );
			if ( !pHitRight )
				eState = ST_FINALIZE;
			else
				eState = ( eState==ST_RHITSSEEK ) ? ST_EXAMINEDOC : ST_EMITHITS;
			break;
		// The docs chunk is over; give me more
		case ST_NODOCS:
			pDoc = m_pNode->GetDocsChunk ( &m_uDocsMaxID );
			eState = pDoc ? ST_ENTERED : ST_FINALIZE;
			break;
		// The last doc is processed. Step forward and start all again
		case ST_DOCFINISHED:
			pDoc++;
			eState = ST_ENTERED;
			break;
		// We are finishing. Emit the end marker and say Good bye!
		case ST_FINALIZE:
			assert ( iHit>=0 && iHit<MAX_HITS );
			m_dMyHits[iHit].m_uDocid = DOCID_MAX; // end marker
			eState = ST_FINISHED;
			break;
		// Shit happens..
		default:
			/// unknown/unhandled state
			assert ( 0 && "Unknown or unhandled state in GetDocsChunk" );
		}

	// store the shortcuts
	m_pDoc = pDoc;
	m_pHitsLeft = pHitLeft;
	m_pHitsRight = pHitRight;
	m_uMatchedDocid = 0;

	// reset current positions for hits chunk getter
	m_pMyDoc = m_dDocs;
	m_pMyHit = m_dMyHits;
	return ReturnDocsChunk ( iDoc, pMaxID );
}


const ExtHit_t * ExtNear_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
{
	// if we already emitted hits for this matches block, do not do that again
	SphDocID_t uFirstMatch = pDocs->m_uDocid;
	if ( uFirstMatch==m_uHitsOverFor )
		return NULL;

	// early reject whole block
	if ( pDocs->m_uDocid > m_uMaxID ) return NULL;
	if ( m_uMaxID && m_dDocs[0].m_uDocid > uMaxID ) return NULL;

	// shortcuts
	const ExtDoc_t * pMyDoc = m_pMyDoc;
	const ExtHit_t * pMyHit = m_pMyHit;
	assert ( pMyDoc );
	assert ( pMyHit );
	const ExtHit_t * pHitLeft = m_pHitsLeft;
	const ExtHit_t * pHitRight = m_pHitsRight;
	SphDocID_t uMatchedDocid = m_uMatchedDocid;
	const ExtDoc_t * pDoc = m_pDoc;

	// filter and copy hits from m_dMyHits
	int iHit = 0;

	enum State_e { ST_ENTERED, ST_STEPHIT, ST_EMITHIT, ST_GETNEXTDOC, ST_GETMATCHEDDOC, ST_MYHITSOVER,
		ST_LHITSSEEK, ST_RHITSSEEK, ST_STEPNEXTHIT, ST_HITSOVER, ST_STEPFINALIZE, ST_FINALIZE, ST_FINISHED };
	State_e eState = ST_ENTERED;

	while ( eState!=ST_FINISHED )
		switch ( eState )
	{
		case ST_ENTERED:
			if ( !uMatchedDocid )
			{
				eState = ST_GETMATCHEDDOC;
				break;
			}
			eState = ST_STEPHIT;
		case ST_STEPHIT:
			while ( pMyHit->m_uDocid < uMatchedDocid ) pMyHit++;
			if ( pMyHit->m_uDocid==DOCID_MAX )
			{
				eState = ST_MYHITSOVER;
				break;
			}
			eState = ST_EMITHIT;
			// no break, keep going here
		case ST_EMITHIT:
			assert ( uMatchedDocid!=0 && uMatchedDocid!=DOCID_MAX );
			if ( pMyHit->m_uDocid==uMatchedDocid )
				m_dHits[iHit++] = *pMyHit++;
			else
				eState = ST_GETNEXTDOC;
			if ( iHit==MAX_HITS-1 )
					eState = ST_FINALIZE;
			break;
		case ST_GETNEXTDOC:
			uMatchedDocid = 0;
			pMyDoc++;
			eState = ST_GETMATCHEDDOC;
			// no break; keep going.
		case ST_GETMATCHEDDOC:
			do
			{
				while ( pMyDoc->m_uDocid < pDocs->m_uDocid ) pMyDoc++;
				if ( pMyDoc->m_uDocid==DOCID_MAX ) break;

				while ( pDocs->m_uDocid < pMyDoc->m_uDocid ) pDocs++;
				if ( pDocs->m_uDocid==DOCID_MAX ) break;
			} while ( pDocs->m_uDocid!=pMyDoc->m_uDocid );
			if ( pDocs->m_uDocid!=pMyDoc->m_uDocid )
			{
				assert ( pMyDoc->m_uDocid==DOCID_MAX || pDocs->m_uDocid==DOCID_MAX );
				eState = ST_HITSOVER;
				break;
			}
			assert ( pDocs->m_uDocid==pMyDoc->m_uDocid );
			assert ( pDocs->m_uDocid!=0 );
			assert ( pDocs->m_uDocid!=DOCID_MAX );

			uMatchedDocid = pDocs->m_uDocid;
			eState = ST_STEPHIT;
			break;
		case ST_MYHITSOVER:
			if ( !pDoc )
			{
				eState = ST_FINALIZE;
				break;
			}
			if ( !pHitLeft || pHitLeft->m_uDocid==DOCID_MAX )
			{
				eState = ST_LHITSSEEK;
				break;
			}
			if ( !pHitRight || pHitRight->m_uDocid==DOCID_MAX )
			{
				eState = ST_RHITSSEEK;
				break;
			}
			if ( pHitLeft->m_uDocid!=pHitRight->m_uDocid )
			{
				eState = ST_HITSOVER;
				break;
			}
			if ( IsAppropriateHit ( pHitLeft, pHitRight ) )
			{
				int iDistance = HIT2LCS ( pHitRight->m_uHitpos ) - HIT2LCS ( pHitLeft->m_uHitpos );
				const ExtHit_t *&pL = (iDistance>0)?pHitLeft:pHitRight;
				const ExtHit_t *&pR = (iDistance>0)?pHitRight:pHitLeft;
				iDistance = (iDistance>0)?iDistance:-iDistance;
				m_dHits[iHit].m_uDocid = uMatchedDocid;
				m_dHits[iHit].m_uHitpos = pL->m_uHitpos;
				m_dHits[iHit].m_uQuerypos = pL->m_uQuerypos;
				m_dHits[iHit].m_uMatchlen = iDistance + pR->m_uMatchlen;
				m_dHits[iHit++].m_uWeight = pHitLeft->m_uWeight + pHitRight->m_uWeight;
				if ( iHit==MAX_HITS-1 )
				{
					eState = ST_STEPFINALIZE;
					break;
				}
			}
			eState = ST_STEPNEXTHIT;
			// no break;
		case ST_STEPNEXTHIT:
		case ST_STEPFINALIZE:
			if ( pHitLeft->m_uHitpos > pHitRight->m_uHitpos )
				pHitRight++;
			else
				pHitLeft++;
			eState = ( eState==ST_STEPFINALIZE ) ? ST_FINALIZE : ST_MYHITSOVER;
			break;
		case ST_LHITSSEEK:
			pHitLeft = m_pNodeLeft->GetHitsChunk ( pDoc, m_uDocsMaxID );
			eState = pHitLeft ? ST_MYHITSOVER : ST_HITSOVER;
			break;
		case ST_RHITSSEEK:
			pHitRight = m_pNodeRight->GetHitsChunk ( pDoc, m_uDocsMaxID );
			eState = pHitRight ? ST_MYHITSOVER : ST_HITSOVER;
			break;
		case ST_HITSOVER:
			pDoc = NULL;
			// no break;
		case ST_FINALIZE:
			m_dHits[iHit].m_uDocid = DOCID_MAX;
			eState = ST_FINISHED;
			break;
		default:
			/// unknown/unhandled state
			assert ( 0 && "Unknown or unhandled state in GetHitsChunk" );
	}

	// save shortcuts
	m_pMyDoc = pMyDoc;
	m_pMyHit = pMyHit;
	m_pHitsLeft = pHitLeft;
	m_pHitsRight = pHitRight;
	m_uMatchedDocid = uMatchedDocid;
	m_pDoc = pDoc;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX; // end marker
	return iHit ? m_dHits : NULL;
}

void ExtNear_c::Reset ( const ISphQwordSetup & tSetup )
{
	assert ( m_pNode );
	m_pNode->Reset ( tSetup );
	m_pHitsLeft = NULL;
	m_pHitsRight = NULL;
	m_uMatchedDocid = 0;
	m_pDoc = NULL;
	m_pMyDoc = NULL;
	m_pMyHit = NULL;
	m_uDocsMaxID = 0;
	m_uExpID = 0;
}

void ExtNear_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->GetQwords ( hQwords );
}

void ExtNear_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->SetQwordsIDF ( hQwords );
}

//////////////////////////////////////////////////////////////////////////

const ExtDoc_t * ExtOr_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	m_uMaxID = 0;
	const ExtDoc_t * pCur0 = m_pCurDoc[0];
	const ExtDoc_t * pCur1 = m_pCurDoc[1];

	DWORD uTouched = 0;
	int iDoc = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	while ( iDoc<MAX_DOCS-1 )
	{
		// if any of the pointers is empty, and not touched yet, advance
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX )
		{
			if ( uTouched & 1 ) break; // it was touched, so we can't advance, because child hitlist offsets would be lost
			pCur0 = m_pChildren[0]->GetDocsChunk ( NULL );
		}
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
		{
			if ( uTouched & 2 ) break; // it was touched, so we can't advance, because child hitlist offsets would be lost
			pCur1 = m_pChildren[1]->GetDocsChunk ( NULL );
		}

		// check if we're over
		if ( !pCur0 && !pCur1 ) break;

		// merge lists while we can, copy tail while if we can not
		if ( pCur0 && pCur1 )
		{
			// merge lists if we have both of them
			while ( iDoc<MAX_DOCS-1 )
			{
				// copy min docids from 1st child
				while ( pCur0->m_uDocid < pCur1->m_uDocid && iDoc<MAX_DOCS-1 )
				{
					CopyExtDoc ( m_dDocs[iDoc++], *pCur0++, &pDocinfo, m_iStride );
					uTouched |= 1;
				}
				if ( pCur0->m_uDocid==DOCID_MAX ) { pCur0 = NULL; break; }

				// copy min docids from 2nd child
				while ( pCur1->m_uDocid < pCur0->m_uDocid && iDoc<MAX_DOCS-1 )
				{
					CopyExtDoc ( m_dDocs[iDoc++], *pCur1++, &pDocinfo, m_iStride );
					uTouched |= 2;
				}
				if ( pCur1->m_uDocid==DOCID_MAX ) { pCur1 = NULL; break; }

				// copy min docids from both children
				assert ( pCur0->m_uDocid && pCur0->m_uDocid!=DOCID_MAX );
				assert ( pCur1->m_uDocid && pCur1->m_uDocid!=DOCID_MAX );

				while ( pCur0->m_uDocid==pCur1->m_uDocid && pCur0->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
				{
					m_dDocs[iDoc] = *pCur0;
					m_dDocs[iDoc].m_uFields = pCur0->m_uFields | pCur1->m_uFields;
					m_dDocs[iDoc].m_fTFIDF = pCur0->m_fTFIDF + pCur1->m_fTFIDF;
					CopyExtDocinfo ( m_dDocs[iDoc], *pCur0, &pDocinfo, m_iStride );
					iDoc++;
					pCur0++;
					pCur1++;
					uTouched |= 3;
				}
				if ( pCur0->m_uDocid==DOCID_MAX ) { pCur0 = NULL; break; }
				if ( pCur1->m_uDocid==DOCID_MAX ) { pCur1 = NULL; break; }
			}
		} else
		{
			// copy tail if we don't have both lists
			const ExtDoc_t * pList = pCur0 ? pCur0 : pCur1;
			if ( pList->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
			{
				while ( pList->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
					CopyExtDoc ( m_dDocs[iDoc++], *pList++, &pDocinfo, m_iStride );
				uTouched |= pCur0 ? 1 : 2;
			}

			if ( pList->m_uDocid==DOCID_MAX ) pList = NULL;
			if ( pCur0 )
				pCur0 = pList;
			else
				pCur1 = pList;
		}
	}

	m_pCurDoc[0] = pCur0;
	m_pCurDoc[1] = pCur1;

	return ReturnDocsChunk ( iDoc, pMaxID );
}

const ExtHit_t * ExtOr_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
{
	const ExtHit_t * pCur0 = m_pCurHit[0];
	const ExtHit_t * pCur1 = m_pCurHit[1];

	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// emit hits, while possible
		if ( m_uMatchedDocid!=0
			&& m_uMatchedDocid!=DOCID_MAX
			&& ( ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid ) || ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid ) ) )
		{
			// merge, while possible
			if ( pCur0 && pCur1 && pCur0->m_uDocid==m_uMatchedDocid && pCur1->m_uDocid==m_uMatchedDocid )
				while ( iHit<MAX_HITS-1 )
			{
				if ( pCur0->m_uHitpos < pCur1->m_uHitpos )
				{
					m_dHits[iHit++] = *pCur0++;
					if ( pCur0->m_uDocid!=m_uMatchedDocid )
						break;
				} else
				{
					m_dHits[iHit++] = *pCur1++;
					if ( pCur1->m_uDocid!=m_uMatchedDocid )
						break;
				}
			}

			// copy tail, while possible
			if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid )
			{
				while ( pCur0->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur0++;
			} else
			{
				assert ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid );
				while ( pCur1->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					m_dHits[iHit++] = *pCur1++;
			}
		}

		// move on
		if ( ( pCur0 && pCur0->m_uDocid!=m_uMatchedDocid ) && ( pCur1 && pCur1->m_uDocid!=m_uMatchedDocid ) )
			m_uMatchedDocid = 0;

		// warmup if needed
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX ) pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs, uMaxID );
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX ) pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs, uMaxID );

		if ( !pCur0 && !pCur1 ) break;
		m_uMatchedDocid = ( pCur0 && pCur1 )
			? Min ( pCur0->m_uDocid, pCur1->m_uDocid )
			: ( pCur0 ? pCur0->m_uDocid : pCur1->m_uDocid );
	}

	m_pCurHit[0] = pCur0;
	m_pCurHit[1] = pCur1;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtAndNot_c::ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup )
	: ExtTwofer_c ( pFirst, pSecond, tSetup )
	, m_bPassthrough ( false )
{
}

const ExtDoc_t * ExtAndNot_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	// if reject-list is over, simply pass through to accept-list
	if ( m_bPassthrough )
		return m_pChildren[0]->GetDocsChunk ( pMaxID );

	// otherwise, do some removals
	m_uMaxID = 0;
	const ExtDoc_t * pCur0 = m_pCurDoc[0];
	const ExtDoc_t * pCur1 = m_pCurDoc[1];

	int iDoc = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	while ( iDoc<MAX_DOCS-1 )
	{
		// pull more docs from accept, if needed
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX )
		{
			// there were matches; we can not pull more because that'd fuckup hitlists
			if ( iDoc )
				break;

			// no matches so far; go pull
			pCur0 = m_pChildren[0]->GetDocsChunk ( NULL );
			if ( !pCur0 )
				break;
		}

		// pull more docs from reject, if nedeed
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
			pCur1 = m_pChildren[1]->GetDocsChunk ( NULL );

		// if there's nothing to filter against, simply copy leftovers
		if ( !pCur1 )
		{
			assert ( pCur0 );
			while ( pCur0->m_uDocid!=DOCID_MAX && iDoc<MAX_DOCS-1 )
				CopyExtDoc ( m_dDocs[iDoc++], *pCur0++, &pDocinfo, m_iStride );

			if ( pCur0->m_uDocid==DOCID_MAX )
				m_bPassthrough = true;

			break;
		}

		// perform filtering
		assert ( pCur0 );
		assert ( pCur1 );
		for ( ;; )
		{
			assert ( iDoc<MAX_DOCS-1 );
			assert ( pCur0->m_uDocid!=DOCID_MAX );
			assert ( pCur1->m_uDocid!=DOCID_MAX );

			// copy accepted until min rejected id
			while ( pCur0->m_uDocid < pCur1->m_uDocid && iDoc<MAX_DOCS-1 )
				CopyExtDoc ( m_dDocs[iDoc++], *pCur0++, &pDocinfo, m_iStride );
			if ( pCur0->m_uDocid==DOCID_MAX || iDoc==MAX_DOCS-1 ) break;

			// skip rejected until min accepted id
			while ( pCur1->m_uDocid < pCur0->m_uDocid ) pCur1++;
			if ( pCur1->m_uDocid==DOCID_MAX ) break;

			// skip both while ids match
			while ( pCur0->m_uDocid==pCur1->m_uDocid && pCur0->m_uDocid!=DOCID_MAX )
			{
				pCur0++;
				pCur1++;
			}
			if ( pCur0->m_uDocid==DOCID_MAX || pCur1->m_uDocid==DOCID_MAX ) break;
		}
	}

	m_pCurDoc[0] = pCur0;
	m_pCurDoc[1] = pCur1;

	return ReturnDocsChunk ( iDoc, pMaxID );
}

const ExtHit_t * ExtAndNot_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
{
	return m_pChildren[0]->GetHitsChunk ( pDocs, uMaxID );
};

void ExtAndNot_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bPassthrough = false;
	ExtTwofer_c::Reset ( tSetup );
}

//////////////////////////////////////////////////////////////////////////

ExtPhrase_c::ExtPhrase_c ( CSphVector<ExtNode_i *> & dQwords, DWORD, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: m_pDocs ( NULL )
	, m_pHits ( NULL )
	, m_pDoc ( NULL )
	, m_pHit ( NULL )
	, m_pMyDoc ( NULL )
	, m_pMyHit ( NULL )
	, m_uFields ( tNode.m_uFieldMask )
	, m_uLastDocID ( 0 )
	, m_uExpID ( 0 )
	, m_uExpPos ( 0 )
	, m_uExpQpos ( 0 )
	, m_uMatchedDocid ( 0 )
	, m_uHitsOverFor ( 0 )
	, m_uWords ( 0 )
{
	m_uWords = dQwords.GetLength();
	assert ( m_uWords>1 );

	m_uMinQpos = dQwords[0]->m_iAtomPos;
	m_uMaxQpos = dQwords.Last()->m_iAtomPos;
	m_iAtomPos = m_uMinQpos;

	m_dQposDelta.Resize ( m_uMaxQpos-m_uMinQpos+1 );
	ARRAY_FOREACH ( i, m_dQposDelta )
		m_dQposDelta[i] = -INT_MAX;

	ExtNode_i * pCur = dQwords[0];
	for ( int i=1; i<(int)m_uWords; i++ )
	{
		pCur = new ExtAnd_c ( pCur, dQwords[i], tSetup );
		m_dQposDelta [ dQwords[i-1]->m_iAtomPos - dQwords[0]->m_iAtomPos ] = dQwords[i]->m_iAtomPos - dQwords[i-1]->m_iAtomPos;
	}
	m_pNode = pCur;

	AllocDocinfo ( tSetup );
}

ExtPhrase_c::~ExtPhrase_c ()
{
	SafeDelete ( m_pNode );
}

void ExtPhrase_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pNode->Reset ( tSetup );
	m_pDocs = NULL;
	m_pHits = NULL;
	m_pDoc = NULL;
	m_pHit = NULL;
	m_pMyDoc = NULL;
	m_pMyHit = NULL;
	m_uLastDocID = 0;
	m_uExpID = 0;
	m_uExpPos = 0;
	m_uExpQpos = 0;
	m_uMatchedDocid = 0;
	m_uHitsOverFor = 0;
}

const ExtDoc_t * ExtPhrase_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	m_uMaxID = 0;

	// initial warmup
	if ( !m_pDoc )
	{
		if ( !m_pDocs ) m_pDocs = m_pNode->GetDocsChunk ( &m_uDocsMaxID );
		if ( !m_pDocs ) return NULL; // no more docs
		m_pDoc = m_pDocs;
	}

	// shortcuts
	const ExtDoc_t * pDoc = m_pDoc;
	const ExtHit_t * pHit = m_pHit;

	// search for phrase matches
	int iDoc = 0;
	int iMyHit = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	while ( iMyHit<MAX_HITS-1 )
	{
		// out of hits?
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
		{
			// grab more hits
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, m_uDocsMaxID );
			if ( m_pHits ) continue;

			// no more hits for current docs chunk; grab more docs
			pDoc = m_pDocs = m_pNode->GetDocsChunk ( &m_uDocsMaxID );
			if ( !m_pDocs ) break;

			// we got docs, there must be hits
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, m_uDocsMaxID );
			assert ( pHit );
			continue;
		}

		// unexpected too-low position? must be duplicate keywords for the previous one ("aaa bbb aaa ccc" case); just skip them
		if ( pHit->m_uDocid==m_uExpID && HIT2LCS ( pHit->m_uHitpos )<m_uExpPos )
		{
			pHit++;
			continue;
		}

		// unexpected position? reset and continue
		if ( pHit->m_uDocid!=m_uExpID || HIT2LCS ( pHit->m_uHitpos )!=m_uExpPos )
		{
			// stream position out of sequence; reset expected positions
			if ( pHit->m_uQuerypos==m_uMinQpos )
			{
				m_uExpID = pHit->m_uDocid;
				m_uExpPos = HIT2LCS ( pHit->m_uHitpos ) + m_dQposDelta[0];
				m_uExpQpos = pHit->m_uQuerypos + m_dQposDelta[0];
			} else
			{
				m_uExpID = m_uExpPos = m_uExpQpos = 0;
			}
			pHit++;
			continue;
		}

		// scan all hits with matching stream position
		// duplicate stream positions occur when there are duplicate query words
		const ExtHit_t * pStart = NULL;
		for ( ; pHit->m_uDocid==m_uExpID && HIT2LCS ( pHit->m_uHitpos )==m_uExpPos; pHit++ )
		{
			// stream position is as expected; let's check query position
			if ( pHit->m_uQuerypos!=m_uExpQpos )
			{
				// unexpected query position
				// do nothing; there might be other words in same (!) expected position following, with proper query positions
				// (eg. if the query words are repeated)
				if ( pHit->m_uQuerypos==m_uMinQpos )
					pStart = pHit;
				continue;

			} else if ( m_uExpQpos==m_uMaxQpos )
			{
				// expected position which concludes the phrase; emit next match
				assert ( pHit->m_uQuerypos==m_uExpQpos );

				if ( pHit->m_uDocid!=m_uLastDocID )
				{
					assert ( pDoc->m_uDocid<=pHit->m_uDocid );
					while ( pDoc->m_uDocid < pHit->m_uDocid ) pDoc++;
					assert ( pDoc->m_uDocid==pHit->m_uDocid );

					m_dDocs[iDoc].m_uDocid = pHit->m_uDocid;
					m_dDocs[iDoc].m_uFields = ( 1UL<<HIT2FIELD ( pHit->m_uHitpos ) );
					m_dDocs[iDoc].m_uHitlistOffset = -1;
					m_dDocs[iDoc].m_fTFIDF = pDoc->m_fTFIDF;
					CopyExtDocinfo ( m_dDocs[iDoc], *pDoc, &pDocinfo, m_iStride );
					iDoc++;
					m_uLastDocID = pHit->m_uDocid;
				}

				DWORD uSpanlen = m_uMaxQpos - m_uMinQpos;
				m_dMyHits[iMyHit].m_uDocid = pHit->m_uDocid;
				m_dMyHits[iMyHit].m_uHitpos = HIT2LCS ( pHit->m_uHitpos ) - uSpanlen;
				m_dMyHits[iMyHit].m_uQuerypos = m_uMinQpos;
				m_dMyHits[iMyHit].m_uSpanlen = m_dMyHits[iMyHit].m_uMatchlen = uSpanlen + 1; ///< since it is exact phrase, the hitlen will be same as queried spanlen.
				m_dMyHits[iMyHit].m_uWeight = m_uWords;
				iMyHit++;

				m_uExpID = m_uExpPos = m_uExpQpos = 0;

			} else
			{
				// intermediate expected position; keep looking
				assert ( pHit->m_uQuerypos==m_uExpQpos );
				int iDelta = m_dQposDelta [ pHit->m_uQuerypos - m_uMinQpos ];
				m_uExpPos += iDelta;
				m_uExpQpos += iDelta;
				// FIXME! what if there *more* hits with current pos following?
			}

			pHit++;
			pStart = NULL;
			break;
		}

		// there was a phrase start we now need to handle
		if ( pStart )
		{
			m_uExpID = pStart->m_uDocid;
			m_uExpPos = pStart->m_uHitpos + m_dQposDelta[0];
			m_uExpQpos = pStart->m_uQuerypos + m_dQposDelta[0];
		}
	}

	// save shortcuts
	m_pDoc = pDoc;
	m_pHit = pHit;

	// reset current positions for hits chunk getter
	m_pMyDoc = m_dDocs;
	m_pMyHit = m_dMyHits;

	// emit markes and return found matches
	assert ( iMyHit>=0 && iMyHit<MAX_HITS );
	m_dMyHits[iMyHit].m_uDocid = DOCID_MAX; // end marker
	m_uMatchedDocid = 0;

	return ReturnDocsChunk ( iDoc, pMaxID );
}

const ExtHit_t * ExtPhrase_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
{
	// if we already emitted hits for this matches block, do not do that again
	SphDocID_t uFirstMatch = pDocs->m_uDocid;
	if ( uFirstMatch==m_uHitsOverFor )
		return NULL;

	// early reject whole block
	if ( pDocs->m_uDocid > m_uMaxID ) return NULL;
	if ( m_uMaxID && m_dDocs[0].m_uDocid > uMaxID ) return NULL;

	// shortcuts
	const ExtDoc_t * pMyDoc = m_pMyDoc;
	const ExtHit_t * pMyHit = m_pMyHit;
	assert ( pMyDoc );
	assert ( pMyHit );

	// filter and copy hits from m_dMyHits
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// pull next doc if needed
		if ( !m_uMatchedDocid )
		{
			do
			{
				while ( pMyDoc->m_uDocid < pDocs->m_uDocid ) pMyDoc++;
				if ( pMyDoc->m_uDocid==DOCID_MAX ) break;

				while ( pDocs->m_uDocid < pMyDoc->m_uDocid ) pDocs++;
				if ( pDocs->m_uDocid==DOCID_MAX ) break;
			} while ( pDocs->m_uDocid!=pMyDoc->m_uDocid );

			if ( pDocs->m_uDocid!=pMyDoc->m_uDocid )
			{
				assert ( pMyDoc->m_uDocid==DOCID_MAX || pDocs->m_uDocid==DOCID_MAX );
				break;
			}

			assert ( pDocs->m_uDocid==pMyDoc->m_uDocid );
			assert ( pDocs->m_uDocid!=0 );
			assert ( pDocs->m_uDocid!=DOCID_MAX );

			m_uMatchedDocid = pDocs->m_uDocid;
		}

		// skip until we have to
		while ( pMyHit->m_uDocid < m_uMatchedDocid ) pMyHit++;

		// copy while we can
		if ( pMyHit->m_uDocid!=DOCID_MAX )
		{
			assert ( pMyHit->m_uDocid==m_uMatchedDocid );
			assert ( m_uMatchedDocid!=0 && m_uMatchedDocid!=DOCID_MAX );

			while ( pMyHit->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
				m_dHits[iHit++] = *pMyHit++;
		}

		// handle different end conditions
		if ( pMyHit->m_uDocid!=m_uMatchedDocid && pMyHit->m_uDocid!=DOCID_MAX )
		{
			// it's simply next document in the line; switch to it
			m_uMatchedDocid = 0;
			pMyDoc++;

		} else if ( pMyHit->m_uDocid==DOCID_MAX && !m_pHit )
		{
			// it's the end
			break;

		} else if ( pMyHit->m_uDocid==DOCID_MAX && m_pHit && iHit<MAX_HITS-1 )
		{
			// the trickiest part; handle the end of my (phrase) hitlist chunk
			// phrase doclist chunk was built from it; so it must be the end of doclist as well.
			// Covered by test 114.
			assert ( pMyDoc[1].m_uDocid==DOCID_MAX );

			// keep scanning and-node hits while there are hits for the last matched document
			assert ( m_uMatchedDocid==pMyDoc->m_uDocid );
			assert ( m_uMatchedDocid==m_uLastDocID );
			assert ( !m_pDoc || m_uMatchedDocid==m_pDoc->m_uDocid );
			m_uExpID = m_uMatchedDocid;
			m_pMyDoc = pMyDoc;
			if ( EmitTail(iHit) )
				m_uHitsOverFor = uFirstMatch;
			pMyDoc = m_pMyDoc;
		}
	}

	// save shortcuts
	m_pMyDoc = pMyDoc;
	m_pMyHit = pMyHit;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX; // end marker
	return iHit ? m_dHits : NULL;
}


bool ExtPhrase_c::EmitTail ( int & iHit )
{
	const ExtHit_t * pHit = m_pHit;
	const ExtDoc_t * pMyDoc = m_pMyDoc;
	bool bTailFinished = false;

	while ( iHit<MAX_HITS-1 )
	{
		// and-node hits chunk end reached? get some more
		if ( pHit->m_uDocid==DOCID_MAX )
		{
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, m_uDocsMaxID );
			if ( !pHit )
			{
				m_uMatchedDocid = 0;
				pMyDoc++;
				break;
			}
		}

		// stop and finish on the first new id
		if ( pHit->m_uDocid!=m_uExpID )
		{
			// reset phrase ranking
			m_uExpID = m_uExpPos = m_uExpQpos = 0;

			// reset hits getter; this docs chunk from above is finally over
			bTailFinished = true;
			m_uMatchedDocid = 0;
			pMyDoc++;
			break;
		}

		// unexpected position? reset and continue
		if ( HIT2LCS ( pHit->m_uHitpos )!=m_uExpPos )
		{
			// stream position out of sequence; reset expected positions
			if ( pHit->m_uQuerypos==m_uMinQpos )
			{
				m_uExpID = pHit->m_uDocid;
				m_uExpPos = HIT2LCS ( pHit->m_uHitpos ) + m_dQposDelta[0];
				m_uExpQpos = pHit->m_uQuerypos + m_dQposDelta[0];
			} else
			{
				m_uExpID = m_uExpPos = m_uExpQpos = 0;
			}
			pHit++;
			continue;
		}

		// scan all hits with matching stream position
		// duplicate stream positions occur when there are duplicate query words
		const ExtHit_t * pStart = NULL;
		for ( ; pHit->m_uDocid==m_uExpID && HIT2LCS ( pHit->m_uHitpos )==m_uExpPos; pHit++ )
		{
			// stream position is as expected; let's check query position
			if ( pHit->m_uQuerypos!=m_uExpQpos )
			{
				// unexpected query position
				// do nothing; there might be other words in same (!) expected position following, with proper query positions
				// (eg. if the query words are repeated)
				if ( pHit->m_uQuerypos==m_uMinQpos )
					pStart = pHit;
				continue;

			} else if ( m_uExpQpos==m_uMaxQpos )
			{
				// expected position which concludes the phrase; emit next match
				assert ( pHit->m_uQuerypos==m_uExpQpos );

				DWORD uSpanlen = m_uMaxQpos - m_uMinQpos;

				// emit directly into m_dHits, this is no need to disturb m_dMyHits here.
				m_dHits[iHit].m_uDocid = pHit->m_uDocid;
				m_dHits[iHit].m_uHitpos = HIT2LCS ( pHit->m_uHitpos ) - uSpanlen;
				m_dHits[iHit].m_uQuerypos = m_uMinQpos;
				m_dHits[iHit].m_uSpanlen = m_dHits[iHit].m_uMatchlen = uSpanlen + 1;
				m_dHits[iHit++].m_uWeight = m_uWords;
				m_uExpPos = m_uExpQpos = 0;

			} else
			{
				// intermediate expected position; keep looking
				assert ( pHit->m_uQuerypos==m_uExpQpos );
				int iDelta = m_dQposDelta [ pHit->m_uQuerypos - m_uMinQpos ];
				m_uExpPos += iDelta;
				m_uExpQpos += iDelta;
				// FIXME! what if there *more* hits with current pos following?
			}

			pHit++;
			pStart = NULL;
			break;
		}

		// there was a phrase start we now need to handle
		if ( pStart )
		{
			m_uExpID = pStart->m_uDocid;
			m_uExpPos = pStart->m_uHitpos + m_dQposDelta[0];
			m_uExpQpos = pStart->m_uQuerypos + m_dQposDelta[0];
		}
	}
	// save shortcut
	m_pHit = pHit;
	m_pMyDoc = pMyDoc;
	return bTailFinished;
}

void ExtPhrase_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->GetQwords ( hQwords );
}

void ExtPhrase_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->SetQwordsIDF ( hQwords );
}

//////////////////////////////////////////////////////////////////////////

ExtProximity_c::ExtProximity_c ( CSphVector<ExtNode_i *> & dQwords, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: ExtPhrase_c ( dQwords, uDupeMask, tNode, tSetup )
	, m_iMaxDistance ( tNode.m_iOpArg )
	, m_iNumWords ( dQwords.GetLength() )
{
	assert ( m_iMaxDistance>0 );
}

const ExtDoc_t * ExtProximity_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	m_uMaxID = 0;
	SphDocID_t uMaxID = 0;

	// shortcuts
	const ExtDoc_t * pDoc = m_pDoc;
	const ExtHit_t * pHit = m_pHit;

	// warmup
	if ( !pDoc )
	{
		if ( !m_pDocs ) pDoc = m_pDocs = m_pNode->GetDocsChunk ( &uMaxID );
		if ( !pDoc ) return NULL;
	}

	CSphVector<DWORD> dProx; // proximity hit position for i-th word
	int iProxWords = 0;
	int iProxMinEntry = -1;

	dProx.Resize ( m_uMaxQpos-m_uMinQpos+1 );
	ARRAY_FOREACH ( i, dProx )
		dProx[i] = UINT_MAX;

	int iDoc = 0;
	int iHit = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	while ( iHit<MAX_HITS-1 )
	{
		// update hitlist
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
		{
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, uMaxID );
			if ( pHit )
				continue;

			// no more hits for current document? *now* we can reset
			m_uExpID = m_uExpPos = m_uExpQpos = 0;

			pDoc = m_pDocs = m_pNode->GetDocsChunk ( &uMaxID );
			if ( !pDoc )
				break;

			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, uMaxID );
			assert ( pHit );
			continue;
		}

		// walk through the hitlist and update context
		int iEntry = pHit->m_uQuerypos - m_uMinQpos;

		// check if the incoming hit is out of bounds, or affects min pos
		if (
			!( pHit->m_uDocid==m_uExpID && HIT2LCS ( pHit->m_uHitpos )<m_uExpPos ) // out of expected bounds
			|| iEntry==iProxMinEntry ) // or simply affects min pos
		{
			if ( pHit->m_uDocid!=m_uExpID )
			{
				ARRAY_FOREACH ( i, dProx ) dProx[i] = UINT_MAX;
				m_uExpID = pHit->m_uDocid;

				dProx[iEntry] = HIT2LCS ( pHit->m_uHitpos );
				iProxMinEntry = iEntry;
				iProxWords = 1;

			} else
			{
				// update and recompute for old document
				if ( dProx[iEntry]==UINT_MAX )
					iProxWords++;
				dProx[iEntry] = HIT2LCS ( pHit->m_uHitpos );
				iProxMinEntry = iEntry;

				int iMinPos = HIT2LCS ( pHit->m_uHitpos ) - ( m_uMaxQpos-m_uMinQpos ) - m_iMaxDistance;
				DWORD uMin = HIT2LCS ( pHit->m_uHitpos );
				ARRAY_FOREACH ( i, dProx )
					if ( dProx[i]!=UINT_MAX )
				{
					if ( (int)dProx[i]<=iMinPos )
					{
						dProx[i] = UINT_MAX;
						iProxWords--;
						continue;
					}
					if ( dProx[i]<uMin )
					{
						iProxMinEntry = i;
						uMin = dProx[i];
					}
				}
			}

			m_uExpPos = dProx[iProxMinEntry] + ( m_uMaxQpos-m_uMinQpos ) + m_iMaxDistance;

		} else
		{
			// incoming hit within context bounds; check if the word is new
			if ( dProx[iEntry]==UINT_MAX )
				iProxWords++;

			// update the context
			dProx[iEntry] = HIT2LCS ( pHit->m_uHitpos );
		}

		// all words were found within given distance?
		if ( iProxWords==m_iNumWords )
		{
			// emit document, if it's new
			if ( pHit->m_uDocid!=m_uLastDocID )
			{
				assert ( pDoc->m_uDocid<=pHit->m_uDocid );
				while ( pDoc->m_uDocid < pHit->m_uDocid ) pDoc++;
				assert ( pDoc->m_uDocid==pHit->m_uDocid );

				m_dDocs[iDoc].m_uDocid = pHit->m_uDocid;
				m_dDocs[iDoc].m_uFields = ( 1UL<<HIT2FIELD ( pHit->m_uHitpos ) );
				m_dDocs[iDoc].m_uHitlistOffset = -1;
				m_dDocs[iDoc].m_fTFIDF = pDoc->m_fTFIDF;
				CopyExtDocinfo ( m_dDocs[iDoc], *pDoc, &pDocinfo, m_iStride );
				iDoc++;
				m_uLastDocID = pHit->m_uDocid;
			}

			// compute phrase weight
			//
			// FIXME! should also account for proximity factor, which is in 1 to maxdistance range:
			// m_iMaxDistance - ( pHit->m_uHitpos - dProx[iProxMinEntry] - ( m_uMaxQpos-m_uMinQpos ) )
			CSphVector<int> dDeltas;
			dDeltas.Resize ( m_uMaxQpos-m_uMinQpos+1 );

			DWORD uMax = 0;
			ARRAY_FOREACH ( i, dProx )
			{
				dDeltas[i] = dProx[i] - i;
				uMax = Max ( uMax, dProx[i] );
			}
			dDeltas.Sort ();

			DWORD uWeight = 0;
			int iLast = -INT_MAX;
			ARRAY_FOREACH ( i, dDeltas )
			{
				if ( dDeltas[i]==iLast )
					uWeight++;
				else
					uWeight = 1;
				iLast = dDeltas[i];
			}

			// emit hit
			m_dMyHits[iHit].m_uDocid = pHit->m_uDocid;
			m_dMyHits[iHit].m_uHitpos = dProx[iProxMinEntry];
			m_dMyHits[iHit].m_uQuerypos = m_uMinQpos;
			m_dMyHits[iHit].m_uSpanlen = m_dMyHits[iHit].m_uMatchlen = uMax-dProx[iProxMinEntry]+1;
			m_dMyHits[iHit].m_uWeight = uWeight;
			iHit++;

			// remove current min, and force recompue
			dProx[iProxMinEntry] = UINT_MAX;
			iProxMinEntry = -1;
			iProxWords--;
			m_uExpPos = 0;
		}

		// go on
		pHit++;
	}

	// reset current positions for hits chunk getter
	m_pMyDoc = m_dDocs;
	m_pMyHit = m_dMyHits;

	// save shortcuts
	m_pDoc = pDoc;
	m_pHit = pHit;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dMyHits[iHit].m_uDocid = DOCID_MAX; // end marker
	m_uMatchedDocid = 0;

	return ReturnDocsChunk ( iDoc, pMaxID );
}


bool ExtProximity_c::EmitTail ( int & iHit )
{
	const ExtHit_t * pHit = m_pHit;
	const ExtDoc_t * pMyDoc = m_pMyDoc;
	bool bTailFinished = false;
	CSphVector<DWORD> dProx; // proximity hit position for i-th word
	int iProxWords = 0;
	int iProxMinEntry = -1;

	dProx.Resize ( m_uMaxQpos-m_uMinQpos+1 );
	ARRAY_FOREACH ( i, dProx )
		dProx[i] = UINT_MAX;

	while ( iHit<MAX_HITS-1 )
	{
		// and-node hits chunk end reached? get some more
		if ( pHit->m_uDocid==DOCID_MAX )
		{
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, m_uDocsMaxID );
			if ( !pHit )
			{
				m_uMatchedDocid = 0;
				pMyDoc++;
				break;
			}
		}

		// stop and finish on the first new id
		if ( pHit->m_uDocid!=m_uExpID )
		{
			// reset phrase ranking
			m_uExpID = m_uExpPos = m_uExpQpos = 0;

			// reset hits getter; this docs chunk from above is finally over
			bTailFinished = true;
			m_uMatchedDocid = 0;
			pMyDoc++;
			break;
		}

		// walk through the hitlist and update context
		int iEntry = pHit->m_uQuerypos - m_uMinQpos;

		// check if the incoming hit is out of bounds, or affects min pos
		if (
			!( pHit->m_uDocid==m_uExpID && HIT2LCS ( pHit->m_uHitpos )<m_uExpPos ) // out of expected bounds
			|| iEntry==iProxMinEntry ) // or simply affects min pos
		{
			if ( pHit->m_uDocid!=m_uExpID )
			{
				ARRAY_FOREACH ( i, dProx ) dProx[i] = UINT_MAX;
				m_uExpID = pHit->m_uDocid;

				dProx[iEntry] = HIT2LCS ( pHit->m_uHitpos );
				iProxMinEntry = iEntry;
				iProxWords = 1;

			} else
			{
				// update and recompute for old document
				if ( dProx[iEntry]==UINT_MAX )
					iProxWords++;
				dProx[iEntry] = HIT2LCS ( pHit->m_uHitpos );
				iProxMinEntry = iEntry;

				int iMinPos = HIT2LCS ( pHit->m_uHitpos ) - ( m_uMaxQpos-m_uMinQpos ) - m_iMaxDistance;
				DWORD uMin = HIT2LCS ( pHit->m_uHitpos );
				ARRAY_FOREACH ( i, dProx )
					if ( dProx[i]!=UINT_MAX )
					{
						if ( (int)dProx[i]<=iMinPos )
						{
							dProx[i] = UINT_MAX;
							iProxWords--;
							continue;
						}
						if ( dProx[i]<uMin )
						{
							iProxMinEntry = i;
							uMin = dProx[i];
						}
					}
			}

			m_uExpPos = dProx[iProxMinEntry] + ( m_uMaxQpos-m_uMinQpos ) + m_iMaxDistance;

		} else
		{
			// incoming hit within context bounds; check if the word is new
			if ( dProx[iEntry]==UINT_MAX )
				iProxWords++;

			// update the context
			dProx[iEntry] = HIT2LCS ( pHit->m_uHitpos );
		}

		// all words were found within given distance?
		if ( iProxWords==m_iNumWords )
		{
			// compute phrase weight
			//
			// FIXME! should also account for proximity factor, which is in 1 to maxdistance range:
			// m_iMaxDistance - ( pHit->m_uHitpos - dProx[iProxMinEntry] - ( m_uMaxQpos-m_uMinQpos ) )
			CSphVector<int> dDeltas;
			dDeltas.Resize ( m_uMaxQpos-m_uMinQpos+1 );

			DWORD uMax = 0;
			ARRAY_FOREACH ( i, dProx )
			{
				dDeltas[i] = dProx[i] - i;
				uMax = Max ( uMax, dProx[i] );
			}
			dDeltas.Sort ();

			DWORD uWeight = 0;
			int iLast = -INT_MAX;
			ARRAY_FOREACH ( i, dDeltas )
			{
				if ( dDeltas[i]==iLast )
					uWeight++;
				else
					uWeight = 1;
				iLast = dDeltas[i];
			}

			// emit hit
			m_dHits[iHit].m_uDocid = pHit->m_uDocid;
			m_dHits[iHit].m_uHitpos = dProx[iProxMinEntry];
			m_dHits[iHit].m_uQuerypos = m_uMinQpos;
			m_dHits[iHit].m_uSpanlen = m_dHits[iHit].m_uMatchlen = uMax-dProx[iProxMinEntry]+1;
			m_dHits[iHit].m_uWeight = uWeight;
			iHit++;

			// remove current min, and force recompue
			dProx[iProxMinEntry] = UINT_MAX;
			iProxMinEntry = -1;
			iProxWords--;
			m_uExpPos = 0;
		}

		// go on
		pHit++;
	}
	// save shortcut
	m_pHit = pHit;
	m_pMyDoc = pMyDoc;
	return bTailFinished;
}
//////////////////////////////////////////////////////////////////////////

ExtQuorum_c::ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & )
{
	assert ( tNode.GetOp()==SPH_QUERY_QUORUM );

	m_iThresh = tNode.m_iOpArg;
	m_bDone = false;

	assert ( dQwords.GetLength()>1 ); // use TERM instead
	assert ( dQwords.GetLength()<=32 ); // internal masks are 32 bits
	assert ( m_iThresh>=1 ); // 1 is also OK; it's a bit different from just OR
	assert ( m_iThresh<dQwords.GetLength() ); // use AND instead

	if ( dQwords.GetLength()>0 )
		m_iAtomPos = dQwords[0]->m_iAtomPos;

	ARRAY_FOREACH ( i, dQwords )
	{
		m_dChildren.Add ( dQwords[i] );
		m_pCurDoc.Add ( NULL );
		m_pCurHit.Add ( NULL );
	}

	m_uMask = m_uInitialMask = uDupeMask;
	m_uMaskEnd = dQwords.GetLength() - 1;
	m_uMatchedDocid = 0;
}

ExtQuorum_c::~ExtQuorum_c ()
{
	ARRAY_FOREACH ( i, m_dChildren )
		SafeDelete ( m_dChildren[i] );
}

void ExtQuorum_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bDone = false;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		m_pCurDoc[i] = NULL;
		m_pCurHit[i] = NULL;
	}

	m_uMask = m_uInitialMask;
	m_uMaskEnd = m_dChildren.GetLength() - 1;
	m_uMatchedDocid = 0;
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->Reset ( tSetup );
}

void ExtQuorum_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->GetQwords ( hQwords );
}

void ExtQuorum_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetQwordsIDF ( hQwords );
}

const ExtDoc_t * ExtQuorum_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	// warmup
	ARRAY_FOREACH ( i, m_pCurDoc )
		if ( !m_pCurDoc[i] || m_pCurDoc[i]->m_uDocid==DOCID_MAX )
	{
		m_pCurDoc[i] = m_dChildren[i]->GetDocsChunk ( NULL );
		if ( m_pCurDoc[i] )
			continue;

		if ( m_dChildren.GetLength()==m_iThresh )
		{
			m_bDone = true;
			break;
		}

		// replace i-th bit with the last one
		m_uMask &= ~( 1UL<<i ); // clear i-th bit
		m_uMask |= ( ( m_uMask >> m_uMaskEnd ) & 1 ) << i; // set i-th bit to end bit
		m_uMaskEnd--;

		SafeDelete ( m_dChildren[i] );
		m_dChildren.RemoveFast ( i );
		m_pCurDoc.RemoveFast ( i );
		m_pCurHit.RemoveFast ( i );
		i--;
	}

	// early out
	if ( m_bDone )
		return NULL;

	// main loop
	DWORD uTouched = 0; // bitmask of children that actually produced matches this time
	int iDoc = 0;
	bool bDone = false;
	CSphRowitem * pDocinfo = m_pDocinfo;
	while ( iDoc<MAX_DOCS-1 && !bDone )
	{
		// find min document ID, count occurences
		ExtDoc_t tCand;

		tCand.m_uDocid = DOCID_MAX; // current candidate id
		tCand.m_uHitlistOffset = 0; // suppress gcc warnings
		tCand.m_pDocinfo = NULL;
		tCand.m_uFields = 0;
		tCand.m_fTFIDF = 0.0f;

		int iCandMatches = 0; // amount of children that match current candidate
		ARRAY_FOREACH ( i, m_pCurDoc )
		{
			assert ( m_pCurDoc[i]->m_uDocid && m_pCurDoc[i]->m_uDocid!=DOCID_MAX );
			if ( m_pCurDoc[i]->m_uDocid < tCand.m_uDocid )
			{
				tCand = *m_pCurDoc[i];
				iCandMatches = (m_uMask >> i) & 1;

			} else if ( m_pCurDoc[i]->m_uDocid==tCand.m_uDocid )
			{
				tCand.m_uFields |= m_pCurDoc[i]->m_uFields;
				tCand.m_fTFIDF += m_pCurDoc[i]->m_fTFIDF;
				iCandMatches += (m_uMask >> i) & 1;
			}
		}

		// submit match
		if ( iCandMatches>=m_iThresh )
			CopyExtDoc ( m_dDocs[iDoc++], tCand, &pDocinfo, m_iStride );

		// advance children
		ARRAY_FOREACH ( i, m_pCurDoc )
			if ( m_pCurDoc[i]->m_uDocid==tCand.m_uDocid )
		{
			if ( iCandMatches>=m_iThresh )
				uTouched |= ( 1UL<<i );

			m_pCurDoc[i]++;
			if ( m_pCurDoc[i]->m_uDocid!=DOCID_MAX )
				continue;

			if ( uTouched & ( 1UL<<i) )
			{
				bDone = true;
				continue; // NOT break. because we still need to advance some further children!
			}

			m_pCurDoc[i] = m_dChildren[i]->GetDocsChunk ( NULL );
			if ( m_pCurDoc[i] )
				continue;

			if ( m_dChildren.GetLength()==m_iThresh )
			{
				bDone = m_bDone = true;
				break;
			}

			// replace i-th bit with the last one
			m_uMask &= ~( 1UL<<i ); // clear i-th bit
			m_uMask |= ( ( m_uMask >> m_uMaskEnd ) & 1 ) << i; // set i-th bit to end bit
			m_uMaskEnd--;

			m_dChildren.RemoveFast ( i );
			m_pCurDoc.RemoveFast ( i );
			m_pCurHit.RemoveFast ( i );
			i--;
		}
	}

	return ReturnDocsChunk ( iDoc, pMaxID );
}

const ExtHit_t * ExtQuorum_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
{
	// warmup
	ARRAY_FOREACH ( i, m_pCurHit )
		if ( !m_pCurHit[i] || m_pCurHit[i]->m_uDocid==DOCID_MAX )
			m_pCurHit[i] = m_dChildren[i]->GetHitsChunk ( pDocs, uMaxID );

	// main loop
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		// get min id
		if ( !m_uMatchedDocid )
		{
			m_uMatchedDocid = DOCID_MAX;
			ARRAY_FOREACH ( i, m_pCurHit )
				if ( m_pCurHit[i] )
			{
				assert ( m_pCurHit[i]->m_uDocid!=DOCID_MAX );
				m_uMatchedDocid = Min ( m_uMatchedDocid, m_pCurHit[i]->m_uDocid );
			}
			if ( m_uMatchedDocid==DOCID_MAX )
				break;
		}

		// emit that id while possible
		// OPTIMIZE: full linear scan for min pos and emission, eww
		int iMinChild = -1;
		DWORD uMinPos = UINT_MAX;
		ARRAY_FOREACH ( i, m_pCurHit )
			if ( m_pCurHit[i] && m_pCurHit[i]->m_uDocid==m_uMatchedDocid )
				if ( m_pCurHit[i]->m_uHitpos < uMinPos )
		{
			uMinPos = m_pCurHit[i]->m_uHitpos;
			iMinChild = i;
		}

		if ( iMinChild<0 )
		{
			m_uMatchedDocid = 0;
			continue;
		}

		m_dHits[iHit++] = *m_pCurHit[iMinChild];
		m_pCurHit[iMinChild]++;

		if ( m_pCurHit[iMinChild]->m_uDocid==DOCID_MAX )
			m_pCurHit[iMinChild] = m_dChildren[iMinChild]->GetHitsChunk ( pDocs, uMaxID );
	}

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtOrder_c::ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup )
	: m_dChildren ( dChildren )
	, m_bDone ( false )
	, m_uHitsOverFor ( 0 )
{
	int iChildren = dChildren.GetLength();
	assert ( iChildren>=2 );

	m_pDocs.Resize ( iChildren );
	m_pHits.Resize ( iChildren );
	m_pDocsChunk.Resize ( iChildren );
	m_dMaxID.Resize ( iChildren );

	if ( dChildren.GetLength()>0 )
		m_iAtomPos = dChildren[0]->m_iAtomPos;

	ARRAY_FOREACH ( i, dChildren )
	{
		assert ( m_dChildren[i] );
		m_pDocs[i] = NULL;
		m_pHits[i] = NULL;
	}

	AllocDocinfo ( tSetup );
}

void ExtOrder_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bDone = false;

	ARRAY_FOREACH ( i, m_dChildren )
	{
		assert ( m_dChildren[i] );
		m_dChildren[i]->Reset ( tSetup );
		m_pDocs[i] = NULL;
		m_pHits[i] = NULL;
	}
}

ExtOrder_c::~ExtOrder_c ()
{
	ARRAY_FOREACH ( i, m_dChildren )
		SafeDelete ( m_dChildren[i] );
}


int ExtOrder_c::GetNextHit ( SphDocID_t uDocid )
{
	// OPTIMIZE! implement PQ instead of full-scan
	DWORD uMinPos = UINT_MAX;
	int iChild = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		// is this child over?
		if ( !m_pHits[i] )
			continue;

		// skip until proper hit
		while ( m_pHits[i]->m_uDocid < uDocid )
			m_pHits[i]++;

		// hit-chunk over? request next one, and rescan
		if ( m_pHits[i]->m_uDocid==DOCID_MAX )
		{
			m_pHits[i] = m_dChildren[i]->GetHitsChunk ( m_pDocsChunk[i], m_dMaxID[i] );
			i--;
			continue;
		}

		// is this our man at all?
		if ( m_pHits[i]->m_uDocid==uDocid )
		{
			// is he the best we can get?
			if ( m_pHits[i]->m_uHitpos < uMinPos )
			{
				uMinPos = m_pHits[i]->m_uHitpos;
				iChild = i;
			}
		}
	}
	return iChild;
}


int ExtOrder_c::GetMatchingHits ( SphDocID_t uDocid, ExtHit_t * pHitbuf, int iLimit )
{
	// my trackers
	CSphVector<ExtHit_t> dAccLongest;
	CSphVector<ExtHit_t> dAccRecent;
	int iPosLongest = 0; // needed to handle cases such as "a b c" << a
	int iPosRecent = 0;
	int iField = -1;

	dAccLongest.Reserve ( m_dChildren.GetLength() );
	dAccRecent.Reserve ( m_dChildren.GetLength() );

	// while there's enough space in the buffer
	int iMyHit = 0;
	while ( iMyHit+m_dChildren.GetLength()<iLimit )
	{
		// get next hit (in hitpos ascending order)
		int iChild = GetNextHit ( uDocid );
		if ( iChild<0 )
			break; // OPTIMIZE? no trailing hits on this route

		const ExtHit_t * pHit = m_pHits[iChild];
		assert ( pHit->m_uDocid==uDocid );

		// most recent subseq must never be longer
		assert ( dAccRecent.GetLength()<=dAccLongest.GetLength() );

		// handle that hit!
		int iHitField = HIT2FIELD ( pHit->m_uHitpos );
		int iHitPos = HIT2POS ( pHit->m_uHitpos );

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
					pHitbuf[iMyHit++] = dAccLongest[i];

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
		m_pHits[iChild]++;
	}

	assert ( iMyHit>=0 && iMyHit<iLimit );
	pHitbuf[iMyHit].m_uDocid = DOCID_MAX;
	return iMyHit;
}


const ExtDoc_t * ExtOrder_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	if ( m_bDone )
		return NULL;

	// warm up
	ARRAY_FOREACH ( i, m_dChildren )
	{
		if ( !m_pDocs[i] ) m_pDocs[i] = m_pDocsChunk[i] = m_dChildren[i]->GetDocsChunk ( &m_dMaxID[i] );
		if ( !m_pDocs[i] )
		{
			m_bDone = true;
			return NULL;
		}
	}

	// match, while there's enough space in buffers
	CSphRowitem * pDocinfo = m_pDocinfo;
	int iDoc = 0;
	int iMyHit = 0;
	while ( iDoc<MAX_DOCS-1 && iMyHit+m_dChildren.GetLength()<MAX_HITS-1 )
	{
		// find next candidate document (that has all the words)
		SphDocID_t uDocid = m_pDocs[0]->m_uDocid;
		assert ( uDocid!=DOCID_MAX );

		for ( int i=1; i<m_dChildren.GetLength(); )
		{
			// skip docs with too small ids
			while ( m_pDocs[i]->m_uDocid < uDocid )
				m_pDocs[i]++;

			// block end marker? pull next block and keep scanning
			if ( m_pDocs[i]->m_uDocid==DOCID_MAX )
			{
				m_pDocs[i] = m_pDocsChunk[i] = m_dChildren[i]->GetDocsChunk ( &m_dMaxID[i] );
				if ( !m_pDocs[i] )
				{
					m_bDone = true;
					return ReturnDocsChunk ( iDoc, pMaxID );
				}
				continue;
			}

			// too big id? its out next candidate
			if ( m_pDocs[i]->m_uDocid > uDocid )
			{
				uDocid = m_pDocs[i]->m_uDocid;
				i = 0;
				continue;
			}

			assert ( m_pDocs[i]->m_uDocid==uDocid );
			i++;
		}

		#ifndef NDEBUG
		assert ( uDocid!=DOCID_MAX );
		ARRAY_FOREACH ( i, m_dChildren )
		{
			assert ( m_pDocs[i] );
			assert ( m_pDocs[i]->m_uDocid==uDocid );
		}
		#endif

		// prefetch hits
		ARRAY_FOREACH ( i, m_dChildren )
		{
			if ( !m_pHits[i] )
				m_pHits[i] = m_dChildren[i]->GetHitsChunk ( m_pDocsChunk[i], m_dMaxID[i] );

			// every document comes with at least one hit
			// and we did not yet process current candidate's hits
			// so we MUST have hits at this point no matter what
			assert ( m_pHits[i] );
		}

		// match and save hits
		int iGotHits = GetMatchingHits ( uDocid, m_dMyHits+iMyHit, MAX_HITS-1-iMyHit );
		if ( iGotHits )
		{
			m_uLastMatchID = uDocid;
			CopyExtDoc ( m_dDocs[iDoc++], *m_pDocs[0], &pDocinfo, m_iStride );
			iMyHit += iGotHits;
		}

		// advance doc stream
		m_pDocs[0]++;
		if ( m_pDocs[0]->m_uDocid==DOCID_MAX )
		{
			m_pDocs[0] = m_pDocsChunk[0] = m_dChildren[0]->GetDocsChunk ( &m_dMaxID[0] );
			if ( !m_pDocs[0] )
			{
				m_bDone = true;
				break;
			}
		}
	}

	return ReturnDocsChunk ( iDoc, pMaxID );
}


const ExtHit_t * ExtOrder_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t )
{
	if ( pDocs->m_uDocid==m_uHitsOverFor )
		return NULL;

	// copy accumulated hits while we can
	SphDocID_t uFirstMatch = pDocs->m_uDocid;

	const ExtHit_t * pMyHits = &m_dMyHits[0];
	int iHit = 0;

	for ( ;; )
	{
		while ( pDocs->m_uDocid!=pMyHits->m_uDocid )
		{
			while ( pDocs->m_uDocid < pMyHits->m_uDocid ) pDocs++;
			if ( pDocs->m_uDocid==DOCID_MAX ) break;

			while ( pMyHits->m_uDocid < pDocs->m_uDocid ) pMyHits++;
			if ( pMyHits->m_uDocid==DOCID_MAX ) break;
		}
		if ( pDocs->m_uDocid==DOCID_MAX || pMyHits->m_uDocid==DOCID_MAX )
			break;

		assert ( pDocs->m_uDocid==pMyHits->m_uDocid );
		while ( pDocs->m_uDocid==pMyHits->m_uDocid )
			m_dHits[iHit++] = *pMyHits++;
		assert ( iHit<MAX_HITS-1 ); // we're copying at most our internal buffer; can't go above limit
	}

	// handling trailing hits border case
	if ( iHit )
	{
		// we've been able to copy some accumulated hits...
		if ( pMyHits->m_uDocid==DOCID_MAX )
		{
			// ...all of them! setup the next run to check for trailing hits
			m_dMyHits[0].m_uDocid = DOCID_MAX;
		} else
		{
			// ...but not all of them! we ran out of docs earlier; hence, trailing hits are of no interest
			m_uHitsOverFor = uFirstMatch;
		}
	} else
	{
		// we did not copy any hits; check for trailing ones as the last resort
		iHit = GetMatchingHits ( m_uLastMatchID, m_dHits, MAX_HITS-1 );
		if ( !iHit )
		{
			// actually, not *only* in this case, also in partial buffer case
			// but for simplicity, lets just run one extra GetHitsChunk() iteration
			m_uHitsOverFor = uFirstMatch;
		}
	}

	// all done
	assert ( iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return iHit ? m_dHits : NULL;
}


void ExtOrder_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->GetQwords ( hQwords );
}


void ExtOrder_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetQwordsIDF ( hQwords );
}

//////////////////////////////////////////////////////////////////////////

ExtRanker_c::ExtRanker_c ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup )
{
	assert ( tSetup.m_pCtx );

	m_iInlineRowitems = tSetup.m_iInlineRowitems;
	for ( int i=0; i<ExtNode_i::MAX_DOCS; i++ )
	{
		m_dMatches[i].Reset ( tSetup.m_iDynamicRowitems );
		m_dMyMatches[i].Reset ( tSetup.m_iDynamicRowitems );
	}
	m_tTestMatch.Reset ( tSetup.m_iDynamicRowitems );

	assert ( pRoot );
	m_pRoot = ExtNode_i::Create ( pRoot, tSetup );
#if SPH_TREE_DUMP
	if ( m_pRoot )
		m_pRoot->DebugDump(0);
#endif

	m_pDoclist = NULL;
	m_pHitlist = NULL;
	m_uMaxID = 0;
	m_uPayloadMask = 0;
	m_iQwords = 0;
	m_pIndex = tSetup.m_pIndex;
	m_pCtx = tSetup.m_pCtx;
}

void ExtRanker_c::Reset ( const ISphQwordSetup & tSetup )
{
	if ( m_pRoot )
		m_pRoot->Reset ( tSetup );
}

const ExtDoc_t * ExtRanker_c::GetFilteredDocs ()
{
	for ( ;; )
	{
		// get another chunk
		m_uMaxID = 0;
		const ExtDoc_t * pCand = m_pRoot->GetDocsChunk ( &m_uMaxID );
		if ( !pCand )
			return NULL;

		// create matches, and filter them
		int iDocs = 0;
		while ( pCand->m_uDocid!=DOCID_MAX )
		{
			m_tTestMatch.m_iDocID = pCand->m_uDocid;
			if ( pCand->m_pDocinfo )
				memcpy ( m_tTestMatch.m_pDynamic, pCand->m_pDocinfo, m_iInlineRowitems*sizeof(CSphRowitem) );

			if ( m_pIndex->EarlyReject ( m_pCtx, m_tTestMatch ) )
			{
				pCand++;
				continue;
			}

			m_dMyDocs[iDocs] = *pCand;
			m_tTestMatch.m_iWeight = (int)( (pCand->m_fTFIDF+0.5f)*SPH_BM25_SCALE ); // FIXME! bench bNeedBM25
			Swap ( m_tTestMatch, m_dMyMatches[iDocs] );
			iDocs++;
			pCand++;
		}

		if ( iDocs )
		{
			m_dMyDocs[iDocs].m_uDocid = DOCID_MAX;
			return m_dMyDocs;
		}
	}
}


void ExtRanker_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_iQwords = hQwords.GetLength ();

	m_iMaxQuerypos = 0;
	hQwords.IterateStart();
	while ( hQwords.IterateNext() )
		m_iMaxQuerypos = Max ( m_iMaxQuerypos, hQwords.IterateGet().m_iQueryPos );

	if ( m_pRoot )
		m_pRoot->SetQwordsIDF ( hQwords );
}

//////////////////////////////////////////////////////////////////////////

template < bool USE_BM25 >
int ExtRanker_WeightSum_c<USE_BM25>::GetMatches ( int iFields, const int * pWeights )
{
	if ( !m_pRoot )
		return 0;

	const ExtDoc_t * pDoc = m_pDoclist;
	int iMatches = 0;

	while ( iMatches<ExtNode_i::MAX_DOCS )
	{
		if ( !pDoc || pDoc->m_uDocid==DOCID_MAX ) pDoc = GetFilteredDocs ();
		if ( !pDoc ) { m_pDoclist = NULL; return iMatches; }

		DWORD uRank = 0;
		for ( int i=0; i<iFields; i++ )
			uRank += ( (pDoc->m_uFields>>i)&1 )*pWeights[i];

		Swap ( m_dMatches[iMatches], m_dMyMatches[pDoc-m_dMyDocs] ); // OPTIMIZE? can avoid this swap and simply return m_dMyMatches (though in lesser chunks)
		m_dMatches[iMatches].m_iWeight = USE_BM25
			? ( m_dMatches[iMatches].m_iWeight + uRank*SPH_BM25_SCALE )
			: uRank;

		iMatches++;
		pDoc++;
	}

	m_pDoclist = pDoc;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

int ExtRanker_None_c::GetMatches ( int, const int * )
{
	if ( !m_pRoot )
		return 0;

	const ExtDoc_t * pDoc = m_pDoclist;
	int iMatches = 0;

	while ( iMatches<ExtNode_i::MAX_DOCS )
	{
		if ( !pDoc || pDoc->m_uDocid==DOCID_MAX ) pDoc = GetFilteredDocs ();
		if ( !pDoc ) { m_pDoclist = NULL; return iMatches; }

		Swap ( m_dMatches[iMatches], m_dMyMatches[pDoc-m_dMyDocs] ); // OPTIMIZE? can avoid this swap and simply return m_dMyMatches (though in lesser chunks)
		m_dMatches[iMatches].m_iWeight = 1;
		iMatches++;
		pDoc++;
	}

	m_pDoclist = pDoc;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

template < typename STATE >
int ExtRanker_T<STATE>::GetMatches ( int iFields, const int * pWeights )
{
	STATE tState ( iFields, pWeights, this );

	if ( !m_pRoot )
		return 0;

	int iMatches = 0;
	const ExtHit_t * pHlist = m_pHitlist;
	const ExtDoc_t * pDocs = m_pDoclist;

	// warmup if necessary
	if ( !pHlist )
	{
		if ( !pDocs ) pDocs = GetFilteredDocs ();
		if ( !pDocs ) return iMatches;

		pHlist = m_pRoot->GetHitsChunk ( pDocs, m_uMaxID );
		if ( !pHlist ) return iMatches;
	}

	// main matching loop
	const ExtDoc_t * pDoc = pDocs;
	for ( SphDocID_t uCurDocid=0; iMatches<ExtNode_i::MAX_DOCS; )
	{
		// keep ranking
		while ( pHlist->m_uDocid==uCurDocid )
			tState.Update ( pHlist++ );

		// if hits block is over, get next block, but do *not* flush current doc
		if ( pHlist->m_uDocid==DOCID_MAX )
		{
			assert ( pDocs );
			pHlist = m_pRoot->GetHitsChunk ( pDocs, m_uMaxID );
			if ( pHlist )
				continue;
		}

		// otherwise (new match or no next hits block), flush current doc
		if ( uCurDocid )
		{
			assert ( uCurDocid==pDoc->m_uDocid );
			Swap ( m_dMatches[iMatches], m_dMyMatches[pDoc-m_dMyDocs] );
			m_dMatches[iMatches].m_iWeight = tState.Finalize ( m_dMatches[iMatches].m_iWeight );
			iMatches++;
		}

		// boundary checks
		if ( !pHlist )
		{
			// there are no more hits for current docs block; do we have a next one?
			assert ( pDocs );
			pDoc = pDocs = GetFilteredDocs ();

			// we don't, so bail out
			if ( !pDocs )
				break;

			// we do, get some hits
			pHlist = m_pRoot->GetHitsChunk ( pDocs, m_uMaxID );
			assert ( pHlist ); // fresh docs block, must have hits
		}

		// skip until next good doc/hit pair
		assert ( pDoc->m_uDocid<=pHlist->m_uDocid );
		while ( pDoc->m_uDocid<pHlist->m_uDocid ) pDoc++;
		assert ( pDoc->m_uDocid==pHlist->m_uDocid );

		uCurDocid = pHlist->m_uDocid;
	}

	m_pDoclist = pDocs;
	m_pHitlist = pHlist;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

template < bool USE_BM25 >
struct RankerState_Proximity_fn
{
	BYTE m_uLCS[SPH_MAX_FIELDS];
	BYTE m_uCurLCS;
	int m_iExpDelta;
	int m_iFields;
	const int * m_pWeights;

	RankerState_Proximity_fn ( int iFields, const int * pWeights, ExtRanker_c * )
	{
		memset ( m_uLCS, 0, sizeof(m_uLCS) );
		m_uCurLCS = 0;
		m_iExpDelta = -INT_MAX;
		m_iFields = iFields;
		m_pWeights = pWeights;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		int iDelta = HIT2LCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
		if ( iDelta==m_iExpDelta )
			m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
		else
			m_uCurLCS = BYTE(pHlist->m_uWeight);

		DWORD uField = HIT2FIELD ( pHlist->m_uHitpos );
		if ( m_uCurLCS>m_uLCS[uField] )
			m_uLCS[uField] = m_uCurLCS;

		m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1;
	}

	DWORD Finalize ( DWORD uWeight )
	{
		m_uCurLCS = 0;
		m_iExpDelta = -1;

		DWORD uRank = 0;
		for ( int i=0; i<m_iFields; i++ )
		{
			uRank += m_uLCS[i]*m_pWeights[i];
			m_uLCS[i] = 0;
		}

		return USE_BM25 ? uWeight + uRank*SPH_BM25_SCALE : uRank;
	}
};

//////////////////////////////////////////////////////////////////////////

// sph04, proximity + exact boost
struct RankerState_ProximityBM25Exact_fn
{
	BYTE m_uLCS[SPH_MAX_FIELDS];
	BYTE m_uCurLCS;
	int m_iExpDelta;
	DWORD m_uMinExpPos;
	int m_iFields;
	const int * m_pWeights;
	DWORD m_uHeadHit;
	DWORD m_uExactHit;
	int m_iMaxQuerypos;

	RankerState_ProximityBM25Exact_fn ( int iFields, const int * pWeights, ExtRanker_c * pRanker )
	{
		memset ( m_uLCS, 0, sizeof(m_uLCS) );
		m_uCurLCS = 0;
		m_iExpDelta = -INT_MAX;
		m_uMinExpPos = 0;
		m_iFields = iFields;
		m_pWeights = pWeights;
		m_uHeadHit = 0;
		m_uExactHit = 0;
		m_iMaxQuerypos = pRanker->m_iMaxQuerypos;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		// upd LCS
		DWORD uField = HIT2FIELD ( pHlist->m_uHitpos );
		int iDelta = HIT2LCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
		if ( iDelta==m_iExpDelta && pHlist->m_uHitpos > m_uMinExpPos )
		{
			m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
			if ( ( pHlist->m_uHitpos & HIT_FIELD_END ) && (int)pHlist->m_uQuerypos==m_iMaxQuerypos )
				m_uExactHit |= ( 1UL<<HIT2FIELD ( pHlist->m_uHitpos ) );
		} else
		{
			m_uCurLCS = BYTE(pHlist->m_uWeight);
			if ( HIT2POS ( pHlist->m_uHitpos )==1 )
			{
				m_uHeadHit |= ( 1UL<<HIT2FIELD ( pHlist->m_uHitpos ) );
				if ( ( pHlist->m_uHitpos & HIT_FIELD_END ) && m_iMaxQuerypos==1 )
					m_uExactHit |= ( 1UL<<HIT2FIELD ( pHlist->m_uHitpos ) );
			}
		}

		if ( m_uCurLCS>m_uLCS[uField] )
			m_uLCS[uField] = m_uCurLCS;

		m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1;
		m_uMinExpPos = pHlist->m_uHitpos + 1;
	}

	DWORD Finalize ( DWORD uWeight )
	{
		m_uCurLCS = 0;
		m_iExpDelta = -1;

		DWORD uRank = 0;
		for ( int i=0; i<m_iFields; i++ )
		{
			uRank += ( 4*m_uLCS[i] + 2*((m_uHeadHit>>i)&1) + ((m_uExactHit>>i)&1) )*m_pWeights[i];
			m_uLCS[i] = 0;
		}
		m_uHeadHit = 0;
		m_uExactHit = 0;

		return uWeight + uRank*SPH_BM25_SCALE;
	}
};


template < bool USE_BM25 >
struct RankerState_ProximityPayload_fn : public RankerState_Proximity_fn<USE_BM25>
{
	DWORD m_uPayloadRank;
	DWORD m_uPayloadMask;

	RankerState_ProximityPayload_fn<USE_BM25> ( int iFields, const int * pWeights, ExtRanker_c * pRanker )
		: RankerState_Proximity_fn<USE_BM25> ( iFields, pWeights, pRanker )
	{
		m_uPayloadRank = 0;
		m_uPayloadMask = pRanker->m_uPayloadMask;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		DWORD uField = HIT2FIELD ( pHlist->m_uHitpos );
		if ( ( 1<<uField ) & m_uPayloadMask )
			this->m_uPayloadRank += HIT2POS ( pHlist->m_uHitpos ) * this->m_pWeights[uField];
		else
			RankerState_Proximity_fn<USE_BM25>::Update ( pHlist );
	}

	DWORD Finalize ( DWORD uWeight )
	{
		// as usual, redundant 'this' is just because gcc is stupid
		this->m_uCurLCS = 0;
		this->m_iExpDelta = -1;

		DWORD uRank = m_uPayloadRank;
		for ( int i=0; i<this->m_iFields; i++ )
		{
			// no special care for payload fields as their LCS will be 0 anyway
			uRank += this->m_uLCS[i]*this->m_pWeights[i];
			this->m_uLCS[i] = 0;
		}

		m_uPayloadRank = 0;
		return USE_BM25 ? uWeight + uRank*SPH_BM25_SCALE : uRank;
	}
};

//////////////////////////////////////////////////////////////////////////

struct RankerState_MatchAny_fn : public RankerState_Proximity_fn<false>
{
	int m_iPhraseK;
	BYTE m_uMatchMask[SPH_MAX_FIELDS];

	RankerState_MatchAny_fn ( int iFields, const int * pWeights, ExtRanker_c * pRanker )
		: RankerState_Proximity_fn<false> ( iFields, pWeights, pRanker )
	{
		m_iPhraseK = 0;
		for ( int i=0; i<iFields; i++ )
			m_iPhraseK += pWeights[i] * pRanker->m_iQwords;

		memset ( m_uMatchMask, 0, sizeof(m_uMatchMask) );
	}

	void Update ( const ExtHit_t * pHlist )
	{
		RankerState_Proximity_fn<false>::Update ( pHlist );
		m_uMatchMask [ HIT2FIELD ( pHlist->m_uHitpos ) ] |= ( 1<<(pHlist->m_uQuerypos-1) );
	}

	DWORD Finalize ( DWORD )
	{
		m_uCurLCS = 0;
		m_iExpDelta = -1;

		DWORD uRank = 0;
		for ( int i=0; i<m_iFields; i++ )
		{
			if ( m_uMatchMask[i] )
				uRank += ( sphBitCount ( m_uMatchMask[i] ) + ( m_uLCS[i]-1 )*m_iPhraseK )*m_pWeights[i];
			m_uMatchMask[i] = 0;
			m_uLCS[i] = 0;
		}

		return uRank;
	}
};

//////////////////////////////////////////////////////////////////////////

struct RankerState_Wordcount_fn
{
	DWORD m_uRank;
	int m_iFields;
	const int * m_pWeights;

	RankerState_Wordcount_fn ( int iFields, const int * pWeights, ExtRanker_c * )
		: m_uRank ( 0 )
		, m_iFields ( iFields )
		, m_pWeights ( pWeights )
	{}

	void Update ( const ExtHit_t * pHlist )
	{
		m_uRank += m_pWeights [ HIT2FIELD ( pHlist->m_uHitpos ) ];
	}

	DWORD Finalize ( DWORD )
	{
		DWORD uRes = m_uRank;
		m_uRank = 0;
		return uRes;
	}
};

//////////////////////////////////////////////////////////////////////////

struct RankerState_Fieldmask_fn
{
	DWORD m_uRank;

	RankerState_Fieldmask_fn ( int, const int *, ExtRanker_c * )
		: m_uRank ( 0 )
	{}

	void Update ( const ExtHit_t * pHlist )
	{
		m_uRank |= 1UL << HIT2FIELD ( pHlist->m_uHitpos );
	}

	DWORD Finalize ( DWORD )
	{
		DWORD uRes = m_uRank;
		m_uRank = 0;
		return uRes;
	}
};

//////////////////////////////////////////////////////////////////////////

static void CheckQueryWord ( const char * szWord, CSphQueryResult * pResult, const CSphIndexSettings & tSettings, bool bStar )
{
	if ( ( !tSettings.m_iMinPrefixLen && !tSettings.m_iMinInfixLen ) || !bStar || !szWord )
		return;

	int iLen = strlen ( szWord );
	bool bHeadStar = szWord[0]=='*';
	bool bTailStar = szWord[iLen-1]=='*';
	int iLenWOStars = iLen - ( bHeadStar ? 1 : 0 ) - ( bTailStar ? 1 : 0 );
	if ( bHeadStar || bTailStar )
	{
		if ( tSettings.m_iMinInfixLen > 0 && iLenWOStars < tSettings.m_iMinInfixLen )
			pResult->m_sWarning.SetSprintf ( "Query word length is less than min infix length. word: '%s' ", szWord );
		else
			if ( tSettings.m_iMinPrefixLen > 0 && iLenWOStars < tSettings.m_iMinPrefixLen )
				pResult->m_sWarning.SetSprintf ( "Query word length is less than min prefix length. word: '%s' ", szWord );
	}
}


static void CheckExtendedQuery ( const XQNode_t * pNode, CSphQueryResult * pResult, const CSphIndexSettings & tSettings, bool bStar )
{
	ARRAY_FOREACH ( i, pNode->m_dWords )
		CheckQueryWord ( pNode->m_dWords[i].m_sWord.cstr(), pResult, tSettings, bStar );

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		CheckExtendedQuery ( pNode->m_dChildren[i], pResult, tSettings, bStar );
}


ISphRanker * sphCreateRanker ( const XQNode_t * pRoot, ESphRankMode eRankMode, CSphQueryResult * pResult, const ISphQwordSetup & tTermSetup )
{
	// shortcut
	const CSphIndex * pIndex = tTermSetup.m_pIndex;

	// check the keywords
	CheckExtendedQuery ( pRoot, pResult, pIndex->GetSettings(), pIndex->m_bEnableStar );

	// fill payload mask
	DWORD uPayloadMask = 0;
	ARRAY_FOREACH ( i, pIndex->GetMatchSchema().m_dFields )
		uPayloadMask |= pIndex->GetMatchSchema().m_dFields[i].m_bPayload << i;

	bool bSingleWord = pRoot->m_dChildren.GetLength()==0 && pRoot->m_dWords.GetLength()==1;

	// setup eval-tree
	ExtRanker_c * pRanker = NULL;
	switch ( eRankMode )
	{
		case SPH_RANK_PROXIMITY_BM25:
			if ( uPayloadMask )
				pRanker = new ExtRanker_T < RankerState_ProximityPayload_fn<true> > ( pRoot, tTermSetup );
			else if ( bSingleWord )
				pRanker = new ExtRanker_WeightSum_c<WITH_BM25> ( pRoot, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true> > ( pRoot, tTermSetup );
			break;
		case SPH_RANK_BM25:				pRanker = new ExtRanker_WeightSum_c<WITH_BM25> ( pRoot, tTermSetup ); break;
		case SPH_RANK_NONE:				pRanker = new ExtRanker_None_c ( pRoot, tTermSetup ); break;
		case SPH_RANK_WORDCOUNT:		pRanker = new ExtRanker_T < RankerState_Wordcount_fn > ( pRoot, tTermSetup ); break;
		case SPH_RANK_PROXIMITY:
			if ( bSingleWord )
				pRanker = new ExtRanker_WeightSum_c<> ( pRoot, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<false> > ( pRoot, tTermSetup );
			break;
		case SPH_RANK_MATCHANY:			pRanker = new ExtRanker_T < RankerState_MatchAny_fn > ( pRoot, tTermSetup ); break;
		case SPH_RANK_FIELDMASK:		pRanker = new ExtRanker_T < RankerState_Fieldmask_fn > ( pRoot, tTermSetup ); break;
		case SPH_RANK_SPH04:			pRanker = new ExtRanker_T < RankerState_ProximityBM25Exact_fn > ( pRoot, tTermSetup ); break;
		default:
			pResult->m_sWarning.SetSprintf ( "unknown ranking mode %d; using default", (int)eRankMode );
			pRanker = new ExtRanker_T < RankerState_Proximity_fn<true> > ( pRoot, tTermSetup );
			break;
	}
	assert ( pRanker );
	pRanker->m_uPayloadMask = uPayloadMask;

	// setup word stats and IDFs
	ExtQwordsHash_t hQwords;
	pRanker->GetQwords ( hQwords );

	const int iQwords = hQwords.GetLength ();
	const CSphSourceStats & tSourceStats = pIndex->GetStats();

	hQwords.IterateStart ();
	while ( hQwords.IterateNext() )
	{
		ExtQword_t & tWord = hQwords.IterateGet ();

		// build IDF
		float fIDF = 0.0f;
		if ( tWord.m_iDocs )
		{
			const int iTotalClamped = Max ( tSourceStats.m_iTotalDocuments, tWord.m_iDocs );
			float fLogTotal = logf ( float ( 1+iTotalClamped ) );
			fIDF = logf ( float ( iTotalClamped-tWord.m_iDocs+1 )
				/ float ( tWord.m_iDocs ) )
				/ ( 2*iQwords*fLogTotal );
		}
		tWord.m_fIDF = fIDF;

		// update word stats
		pResult->AddStat ( tWord.m_sDictWord, tWord.m_iDocs, tWord.m_iHits );
	}

	pRanker->SetQwordsIDF ( hQwords );
	return pRanker;
}

//////////////////////////////////////////////////////////////////////////
/// HIT MARKER
//////////////////////////////////////////////////////////////////////////

void CSphHitMarker::Mark ( CSphVector<SphHitMark_t> & dMarked )
{
	const ExtHit_t * pHits = NULL;
	const ExtDoc_t * pDocs = NULL;

	SphDocID_t uMaxID = 0;
	pDocs = m_pRoot->GetDocsChunk ( &uMaxID );
	if ( !pDocs )
		return;

	for ( ;; )
	{
		pHits = m_pRoot->GetHitsChunk ( pDocs, uMaxID );
		if ( !pHits )
			break;

		for ( ; pHits->m_uDocid!=DOCID_MAX; pHits++ )
		{
			SphHitMark_t tMark;
			tMark.m_uPosition = HIT2POS ( pHits->m_uHitpos );
			tMark.m_uSpan = pHits->m_uSpanlen;

			dMarked.Add ( tMark );
		}
	}
}


CSphHitMarker::~CSphHitMarker ()
{
	SafeDelete ( m_pRoot );
}


CSphHitMarker * CSphHitMarker::Create ( const XQNode_t * pRoot, const ISphQwordSetup & tSetup )
{
	ExtNode_i * pNode = ExtNode_i::Create ( pRoot, tSetup );
	if ( pNode )
	{
		CSphHitMarker * pMarker = new CSphHitMarker;
		pMarker->m_pRoot = pNode;
		return pMarker;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// INTRA-BATCH CACHING
//////////////////////////////////////////////////////////////////////////


/// container that does intra-batch query-sub-tree caching
/// actually carries the cached data, NOT to be recreated frequently (see thin wrapper below)
class NodeCacheContainer_t
{
private:
	friend class ExtNodeCached_t;
	friend class CSphQueryNodeCache;

private:
	int								m_iRefCount;
	bool							m_StateOk;
	const ISphQwordSetup *			m_pSetup;

	CSphVector<ExtDoc_t>			m_Docs;
	CSphVector<ExtHit_t>			m_Hits;
	CSphVector<CSphRowitem>			m_InlineAttrs;
	int								m_iAtomPos; // minimal position from original donor, used for shifting

	CSphQueryNodeCache *			m_pNodeCache;

public:
	NodeCacheContainer_t ()
		: m_iRefCount ( 1 )
		, m_StateOk ( true )
		, m_pSetup ( NULL )
		, m_iAtomPos ( 0 )
		, m_pNodeCache ( NULL )
	{}

public:
	void Release()
	{
		if ( --m_iRefCount<=0 )
			Invalidate();
	}

	ExtNode_i *						CreateCachedWrapper ( ExtNode_i* pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup );

private:
	bool							WarmupCache ( ExtNode_i * pChild, int iQWords );
	void							Invalidate();
};


/// cached node wrapper to be injected into actual search trees
/// (special container actually carries all the data and does the work, see blow)
class ExtNodeCached_t : public ExtNode_i
{
	friend class NodeCacheContainer_t;
	NodeCacheContainer_t *		m_pNode;
	ExtDoc_t *					m_pHitDoc;			///< points to entry in m_dDocs which GetHitsChunk() currently emits hits for
	SphDocID_t					m_uHitsOverFor;		///< there are no more hits for matches block starting with this ID
	CSphString *				m_pWarning;
	int64_t						m_iMaxTimer;		///< work until this timestamp
	int							m_iHitIndex;		///< store the current position in m_Hits for GetHitsChunk()
	int							m_iDocIndex;		///< store the current position in m_Docs for GetDocsChunk()
	ExtNode_i *					m_pChild;			///< pointer to donor for the sake of AtomPos procession
	int							m_iQwords;			///< number of tokens in parent query

	void StepForwardToHitsFor ( SphDocID_t uDocId );

	// creation possible ONLY via NodeCacheContainer_t
	explicit ExtNodeCached_t ( NodeCacheContainer_t * pNode, ExtNode_i * pChild )
		: m_pNode ( pNode )
		, m_pHitDoc ( NULL )
		, m_uHitsOverFor ( 0 )
		, m_pWarning ( NULL )
		, m_iMaxTimer ( 0 )
		, m_iHitIndex ( 0 )
		, m_iDocIndex ( 0 )
		, m_pChild ( pChild )
		, m_iQwords ( 0 )
	{
		m_iAtomPos = pChild->m_iAtomPos;
	}

public:
	virtual ~ExtNodeCached_t ()
	{
		SafeDelete ( m_pChild );
		SafeRelease ( m_pNode );
	}

	virtual void Reset ( const ISphQwordSetup & tSetup )
	{
		if ( m_pChild )
			m_pChild->Reset ( tSetup );

		m_iHitIndex = 0;
		m_iDocIndex = 0;
		m_uHitsOverFor = 0;
		m_pHitDoc = NULL;
		m_iMaxTimer = 0;
		m_iMaxTimer = tSetup.m_iMaxTimer;
		m_pWarning = tSetup.m_pWarning;
	}

	virtual const ExtDoc_t * GetDocsChunk ( SphDocID_t * pMaxID );

	virtual const ExtHit_t * GetHitsChunk ( const ExtDoc_t * pMatched, SphDocID_t uMaxID );

	virtual void GetQwords ( ExtQwordsHash_t & hQwords )
	{
		if ( m_pChild )
			m_pChild->GetQwords ( hQwords );
	}

	virtual void SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
	{
		m_iQwords = hQwords.GetLength();
		if ( m_pNode->m_pSetup )
		{
			if ( m_pChild )
				m_pChild->SetQwordsIDF ( hQwords );

			m_pNode->WarmupCache ( m_pChild, m_iQwords );
		}
	}

	virtual bool GotHitless ()
	{
		return ( m_pChild )
			? m_pChild->GotHitless()
			: false;
	}
};

//////////////////////////////////////////////////////////////////////////

ExtNode_i * NodeCacheContainer_t::CreateCachedWrapper ( ExtNode_i * pChild, const XQNode_t * pRawChild, const ISphQwordSetup & tSetup )
{
	if ( !m_StateOk )
		return pChild;

	// wow! we have a virgin!
	if ( !m_Docs.GetLength() )
	{
		m_iRefCount = pRawChild->GetCount();
		m_pSetup = &tSetup;
	}
	return new ExtNodeCached_t ( this, pChild );
}


bool NodeCacheContainer_t::WarmupCache ( ExtNode_i * pChild, int iQwords )
{
	SphDocID_t pMaxID = 0;
	m_iAtomPos = pChild->m_iAtomPos;
	const ExtDoc_t * pChunk = pChild->GetDocsChunk ( &pMaxID );
	int iStride = 0;
	assert ( m_pSetup );

	if ( pChunk && pChunk->m_pDocinfo )
		iStride = pChild->m_iStride;

	while ( pChunk )
	{
		const ExtDoc_t * pChunkHits = pChunk;
		bool iHasDocs = false;
		for ( ; pChunk->m_uDocid!=DOCID_MAX; pChunk++ )
		{
			m_Docs.Add ( *pChunk );
			// exclude number or Qwords from FIDF
			m_Docs.Last().m_fTFIDF *= iQwords;
			m_pNodeCache->m_iMaxCachedDocs--;
			if ( iStride>0 )
			{
				// since vector will relocate the data on resize, do NOT fill new m_pDocinfo right now
				int iLen = m_InlineAttrs.GetLength();
				m_InlineAttrs.Resize ( iLen+iStride );
				memcpy ( &m_InlineAttrs[iLen], pChunk->m_pDocinfo, iStride*sizeof(CSphRowitem) );
			}
			iHasDocs = true;
		}

		const ExtHit_t * pHits = NULL;
		if ( iHasDocs )
			while (	( pHits = pChild->GetHitsChunk ( pChunkHits, pChild->m_uMaxID ) )!=NULL )
			{
				for ( ; pHits->m_uDocid!=DOCID_MAX; pHits++ )
				{
					m_Hits.Add ( *pHits );
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
		pChunk = pChild->GetDocsChunk ( &pMaxID );
	}

	if ( iStride )
		ARRAY_FOREACH ( i, m_Docs )
		m_Docs[i].m_pDocinfo = &m_InlineAttrs[i*iStride];

	m_Docs.Add().m_uDocid = DOCID_MAX;
	m_Hits.Add().m_uDocid = DOCID_MAX;
	pChild->Reset ( *m_pSetup );
	m_pSetup = NULL;
	return true;
}


void NodeCacheContainer_t::Invalidate()
{
	m_pNodeCache->m_iMaxCachedDocs += m_Docs.GetLength();
	m_pNodeCache->m_iMaxCachedHits += m_Docs.GetLength();
	m_Docs.Reset();
	m_InlineAttrs.Reset();
	m_Hits.Reset();
	m_StateOk = false;
}

//////////////////////////////////////////////////////////////////////////

void ExtNodeCached_t::StepForwardToHitsFor ( SphDocID_t uDocId )
{
	assert ( m_pNode );
	assert ( m_pNode->m_StateOk );

	CSphVector<ExtHit_t> & dHits = m_pNode->m_Hits;

	int iEnd = dHits.GetLength()-1;
	if ( m_iHitIndex>=iEnd )
		return;
	if ( dHits[m_iHitIndex].m_uDocid==uDocId )
		return;

	// binary search for lower (most left) bound of the subset of values
	int iHitIndex = m_iHitIndex; // http://blog.gamedeff.com/?p=12
	while ( iEnd-iHitIndex>1 )
	{
		if ( uDocId<dHits[iHitIndex].m_uDocid || uDocId>dHits[iEnd].m_uDocid )
		{
			m_iHitIndex = -1;
			return;
		}
		int iMid = iHitIndex + (iEnd-iHitIndex)/2;
		if ( dHits[iMid].m_uDocid>=uDocId )
			iEnd = iMid;
		else
			iHitIndex = iMid;
	}
	m_iHitIndex = iEnd;
}

const ExtDoc_t * ExtNodeCached_t::GetDocsChunk ( SphDocID_t * pMaxID )
{
	if ( !m_pNode || !m_pChild )
		return NULL;

	if ( !m_pNode->m_StateOk )
		return m_pChild->GetDocsChunk ( pMaxID );

	if ( m_iMaxTimer>0 && sphMicroTimer()>=m_iMaxTimer )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	m_uMaxID = 0;

	int iDoc = Min ( m_iDocIndex+MAX_DOCS-1, m_pNode->m_Docs.GetLength()-1 ) - m_iDocIndex;
	memcpy ( &m_dDocs[0], &m_pNode->m_Docs[m_iDocIndex], sizeof(ExtDoc_t)*iDoc );
	m_iDocIndex += iDoc;

	// funny trick based on the formula of FIDF calculation.
	for ( int i=0; i<iDoc; i++ )
		m_dDocs[i].m_fTFIDF /= m_iQwords;

	return ReturnDocsChunk ( iDoc, pMaxID );
}

const ExtHit_t * ExtNodeCached_t::GetHitsChunk ( const ExtDoc_t * pMatched, SphDocID_t uMaxID )
{
	if ( !m_pNode || !m_pChild )
		return NULL;

	if ( !m_pNode->m_StateOk )
		return m_pChild->GetHitsChunk ( pMatched, uMaxID );

	if ( !pMatched )
		return NULL;

	SphDocID_t uFirstMatch = pMatched->m_uDocid;

	// aim to the right document
	ExtDoc_t * pDoc = m_pHitDoc;
	m_pHitDoc = NULL;

	if ( !pDoc )
	{
		// if we already emitted hits for this matches block, do not do that again
		if ( uFirstMatch==m_uHitsOverFor )
			return NULL;

		// early reject whole block
		if ( pMatched->m_uDocid > m_uMaxID ) return NULL;
		if ( m_uMaxID && m_dDocs[0].m_uDocid > uMaxID ) return NULL;

		// find match
		pDoc = m_dDocs;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			if ( pDoc->m_uDocid==DOCID_MAX )
			{
				m_uHitsOverFor = uFirstMatch;
				return NULL; // matched docs block is over for me, gimme another one
			}

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX )
			{
				m_uHitsOverFor = uFirstMatch;
				return NULL; // matched doc block did not yet begin for me, gimme another one
			}
		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		// setup hitlist reader
		StepForwardToHitsFor ( pDoc->m_uDocid );
	}

	// hit emission
	int iHit = 0;
	while ( iHit<ExtNode_i::MAX_HITS-1 )
	{
		// get next hit
		ExtHit_t & tCachedHit = m_pNode->m_Hits[m_iHitIndex];
		if ( tCachedHit.m_uDocid==DOCID_MAX )
			break;
		if ( tCachedHit.m_uDocid!=pDoc->m_uDocid )
		{
			// no more hits; get next acceptable document
			pDoc++;
			do
			{
				while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
				if ( pDoc->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched docs block is over for me, gimme another one

				while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
				if ( pMatched->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched doc block did not yet begin for me, gimme another one
			} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

			if ( !pDoc )
				break;
			assert ( pDoc->m_uDocid==pMatched->m_uDocid );

			// setup hitlist reader
			StepForwardToHitsFor ( pDoc->m_uDocid );
			continue;
		}
		m_iHitIndex++;
		m_dHits[iHit] = tCachedHit;
		m_dHits[iHit++].m_uQuerypos += m_iAtomPos - m_pNode->m_iAtomPos;
	}

	m_pHitDoc = pDoc;
	if ( iHit==0 || iHit<MAX_HITS-1 )
		m_uHitsOverFor = uFirstMatch;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

CSphQueryNodeCache::CSphQueryNodeCache ( int iCells, int iMaxCachedDocs, int iMaxCachedHits )
{
	m_pPool = NULL;
	if ( iCells>0 && iMaxCachedHits>0 && iMaxCachedDocs>0 )
	{
		m_pPool = new NodeCacheContainer_t [ iCells ];
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
	if ( m_iMaxCachedDocs<=0 || m_iMaxCachedHits<=0 )
		return pChild;

	assert ( pRawChild );
	assert ( pRawChild->GetOrder()>=0 );
	return m_pPool [ pRawChild->GetOrder() ].CreateCachedWrapper ( pChild, pRawChild, tSetup );
}

//
// $Id$
//
