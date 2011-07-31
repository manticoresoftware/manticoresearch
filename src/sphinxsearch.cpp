//
// $Id$
//

//
// Copyright (c) 2001-2011, Andrew Aksyonoff
// Copyright (c) 2008-2011, Sphinx Technologies Inc
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
#include "sphinxint.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////////
// EXTENDED MATCHING V2
//////////////////////////////////////////////////////////////////////////

typedef Hitman_c<8> HITMAN;

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
	CSphSmallBitvec m_dFields;
	float			m_fTFIDF;
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
	bool		m_bExpanded;	///< added by prefix expansion
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
	static ExtNode_i *			Create ( const XQKeyword_t & tWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup );
	static ExtNode_i *			Create ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup );

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
class ExtTerm_c : public ExtNode_i, ISphNoncopyable
{
public:
								ExtTerm_c ( ISphQword * pQword, const CSphSmallBitvec& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted );
								ExtTerm_c ( ISphQword * pQword, const ISphQwordSetup & tSetup );
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
		if ( m_dFields.TestAll(true) )
		{
			printf ( "(all)\n" );
		} else
		{
			bool bFirst = true;
			printf ( "in: " );
			for ( int iField = 0; iField < CSphSmallBitvec::iTOTALBITS; iField++ )
			{
				if ( m_dFields.Test ( iField ) )
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
	mutable CSphSmallBitvec		m_dFields;		///< accepted fields mask
	bool						m_bLongMask;	///< if we work with >32bit mask
	float						m_fIDF;			///< IDF for this term (might be 0.0f for non-1st occurences in query)
	int64_t						m_iMaxTimer;	///< work until this timestamp
	CSphString *				m_pWarning;
	const bool					m_bNotWeighted;

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
									ExtTermHitless_c ( ISphQword * pQword, const CSphSmallBitvec& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
									: ExtTerm_c ( pQword, uFields, tSetup, bNotWeighted )
									, m_uFieldPos ( 0 )

								{
									m_dFieldMask.Unset();
								}
	virtual void				Reset ( const ISphQwordSetup & )
	{
		m_dFieldMask.Unset();
		m_uFieldPos = 0;
	}
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual bool				GotHitless () { return true; }

protected:
	CSphSmallBitvec	m_dFieldMask;
	DWORD	m_uFieldPos;
};


/// single keyword streamer, with term position filtering
template < TermPosFilter_e T >
class ExtTermPos_c : public ExtTerm_c
{
public:
								ExtTermPos_c ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual bool				GotHitless () { return false; }

protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * pHit ) const;

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

	ISphZoneCheck *				m_pZoneChecker;					///< zone-limited searches query ranker about zones
	mutable CSphVector<int>		m_dZones;					///< zone ids for this particular term
	mutable SphDocID_t			m_uLastZonedId;
	mutable int					m_iCheckFrom;
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

	void SetNodePos ( WORD uPosLeft, WORD uPosRight )
	{
		m_dNodePos[0] = uPosLeft;
		m_dNodePos[1] = uPosRight;
		m_bPosAware = true;
	}

protected:
	ExtNode_i *					m_pChildren[2];
	const ExtDoc_t *			m_pCurDoc[2];
	const ExtHit_t *			m_pCurHit[2];
	WORD						m_dNodePos[2];
	bool						m_bPosAware;
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


/// generic operator over N nodes
class ExtNWayT : public ExtNode_i
{
public:
								ExtNWayT ( const CSphVector<ExtNode_i *> & dNodes, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
								~ExtNWayT ();
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				GotHitless () { return false; }

protected:
	ExtNode_i *					m_pNode;				///< my and-node for all the terms
	const ExtDoc_t *			m_pDocs;				///< current docs chunk from and-node
	SphDocID_t					m_uDocsMaxID;			///< max id in current docs chunk
	const ExtHit_t *			m_pHits;				///< current hits chunk from and-node
	const ExtDoc_t *			m_pDoc;					///< current doc from and-node
	const ExtHit_t *			m_pHit;					///< current hit from and-node
	const ExtDoc_t *			m_pMyDoc;				///< current doc for hits getter
	const ExtHit_t *			m_pMyHit;				///< current hit for hits getter
	SphDocID_t					m_uLastDocID;			///< last emitted hit
	ExtHit_t					m_dMyHits[MAX_HITS];	///< buffer for all my phrase hits; inherited m_dHits will receive filtered results
	SphDocID_t					m_uMatchedDocid;		///< doc currently in process
	SphDocID_t					m_uHitsOverFor;			///< there are no more hits for matches block starting with this ID

protected:
	inline void					ConstructNode ( const CSphVector<ExtNode_i *> & dNodes, const CSphVector<WORD> & dPositions, const ISphQwordSetup & tSetup )
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
			pCur = pCurEx = new ExtAnd_c ( pCur, dNodes[uRPos++], tSetup ); // ++ for zero-based to 1-based
			pCurEx->SetNodePos ( uLPos, uRPos );
			uLPos = 0;
		}
		m_pNode = pCur;
	}
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

template < class FSM >
class ExtNWay_c : public ExtNWayT, private FSM
{
public:
	ExtNWay_c ( const CSphVector<ExtNode_i *> & dNodes, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
		: ExtNWayT ( dNodes, uDupeMask, tNode, tSetup )
		, FSM ( dNodes, uDupeMask, tNode, tSetup )
	{
		bool bTerms = FSM::bTermsTree; // workaround MSVC const condition warning
		CSphVector<WORD> dPositions ( dNodes.GetLength() );
		ARRAY_FOREACH ( i, dPositions )
			dPositions[i] = (WORD) i;
		if ( bTerms )
			dPositions.Sort ( ExtNodeTFExt_fn ( dNodes ) );
		ConstructNode ( dNodes, dPositions, tSetup );
	}

public:
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual void DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "%s\n", FSM::GetName() );
		m_pNode->DebugDump ( iLevel+1 );
	}
private:
	bool						EmitTail ( int & iHit );	///< the "trickiest part" extracted in order to process the proximity also
};

class FSMphrase
{
	protected:
									FSMphrase ( const CSphVector<ExtNode_i *> & dQwords, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
		inline void ResetFSM()
		{
			m_uExpPos = 0;
			m_uExpQpos = 0;
		}
		bool						HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget );
		inline static const char*	GetName() { return "ExtPhrase"; }
		static const bool			bTermsTree = true;		///< we work with ExtTerm nodes

	protected:
		DWORD						m_uExpQpos;
		CSphVector<int>				m_dQposDelta;			///< next expected qpos delta for each existing qpos (for skipped stopwords case)
		DWORD						m_uMinQpos;
		DWORD						m_uMaxQpos;
		DWORD						m_uExpPos;
		DWORD						m_uLeaves;				///< number of keywords (might be different from qpos delta because of stops and overshorts)
};
/// exact phrase streamer
typedef ExtNWay_c < FSMphrase > ExtPhrase_c;

/// proximity streamer
class FSMproximity
{
protected:
								FSMproximity ( const CSphVector<ExtNode_i *> & dQwords, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	inline void ResetFSM()
	{
		m_uExpPos = 0;
		m_uWords = 0;
		m_iMinQindex = -1;
		ARRAY_FOREACH ( i, m_dProx )
			m_dProx[i] = UINT_MAX;
	}
	bool						HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget );
	inline static const char*	GetName() { return "ExtProximity"; }
	static const bool			bTermsTree = true;		///< we work with ExtTerm nodes
protected:
	int							m_iMaxDistance;
	DWORD						m_uWordsExpected;
	DWORD						m_uMinQpos;
	DWORD						m_uQLen;
	DWORD						m_uExpPos;
	CSphVector<DWORD>			m_dProx; // proximity hit position for i-th word
	CSphVector<int> 			m_dDeltas; // used for weight calculation
	DWORD						m_uWords;
	int							m_iMinQindex;
};
/// exact phrase streamer
typedef ExtNWay_c<FSMproximity> ExtProximity_c;

/// proximity streamer
class FSMmultinear
{
protected:
								FSMmultinear ( const CSphVector<ExtNode_i *> & dNodes, DWORD uDupeMask, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	inline void ResetFSM()
	{
		m_iRing = m_uLastP = m_uPrelastP = 0;
	}
	bool						HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget );
	inline static const char*	GetName() { return "ExtMultinear"; }
	static const bool			bTermsTree = true;	///< we work with generic (not just ExtTerm) nodes
protected:
	int							m_iNear;			///< the NEAR distance
	DWORD						m_uPrelastP;
	DWORD						m_uPrelastML;
	DWORD						m_uPrelastSL;
	DWORD						m_uPrelastW;
	DWORD						m_uLastP;			///< position of the last hit
	DWORD						m_uLastML;			///< the length of the previous hit
	DWORD						m_uLastSL;			///< the length of the previous hit in Query
	DWORD						m_uLastW;			///< last weight
	DWORD						m_uWordsExpected;	///< now many hits we're expect
	DWORD						m_uWeight;			///< weight accum
	DWORD						m_uFirstHit;		///< hitpos of the beginning of the match chain
	WORD						m_uFirstNpos;		///< N-position of the head of the chain
	WORD						m_uFirstQpos;		///< Q-position of the head of the chain (for twofers)
	CSphVector<WORD>			m_dNpos;			///< query positions for multinear
	CSphVector<ExtHit_t>		m_dRing;			///< ring buffer for multihit data
	int							m_iRing;			///< the head of the ring
	bool						m_bTwofer;			///< if we have 2- or N-way NEAR
private:
	inline int RingTail() const
	{
		return ( m_iRing + m_dNpos.GetLength() - 1 ) % m_uWordsExpected;
	}
	inline void Add2Ring ( const ExtHit_t* pHit )
	{
		if ( !m_bTwofer )
			m_dRing [ RingTail() ] = *pHit;
	}
	inline void ShiftRing()
	{
		if ( ++m_iRing==(int)m_uWordsExpected )
			m_iRing=0;
	}
};
/// exact phrase streamer
typedef ExtNWay_c<FSMmultinear> ExtMultinear_c;

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
	CSphVector<const ExtHit_t*>	m_pCurHit;			///< current positions into children hitlists
	DWORD						m_uMask;			///< mask of nodes that count toward threshold
	DWORD						m_uMaskEnd;			///< index of the last bit in mask
	bool						m_bDone;			///< am i done
	SphDocID_t					m_uMatchedDocid;	///< current docid for hitlist emission

private:
	DWORD						m_uInitialMask;		///< backup mask for Reset()
	CSphVector<ExtNode_i*>		m_dInitialChildren;	///< my children nodes (simply ExtTerm_c for now)
};


/// A-B-C-in-this-order streamer
class ExtOrder_c : public ExtNode_i
{
public:
								ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup );
								~ExtOrder_c ();

	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t );
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

protected:
	int							GetNextHit ( SphDocID_t uDocid );										///< get next hit within given document, and return its child-id
	int							GetMatchingHits ( SphDocID_t uDocid, ExtHit_t * pHitbuf, int iLimit );	///< process candidate hits and stores actual matches while we can
};


/// same-text-unit streamer
/// (aka, A and B within same sentence, or same paragraph)
class ExtUnit_c : public ExtNode_i
{
public:
	ExtUnit_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const CSphSmallBitvec& dFields, const ISphQwordSetup & tSetup, const char * sUnit );
	~ExtUnit_c ();

	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual void				GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

public:
	virtual bool GotHitless ()
	{
		return false;
	}

	virtual void DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "ExtSentence\n" );
		m_pArg1->DebugDump ( iLevel+1 );
		m_pArg2->DebugDump ( iLevel+1 );
	}

protected:
	inline const ExtDoc_t * ReturnDocsChunk ( int iDocs, int iMyHit, SphDocID_t * pMaxID )
	{
		assert ( iMyHit<MAX_HITS );
		m_dMyHits[iMyHit].m_uDocid = DOCID_MAX;
		m_uHitsOverFor = 0;
		return ExtNode_i::ReturnDocsChunk ( iDocs, pMaxID );
	}

protected:
	int					FilterHits ( int iMyHit, DWORD uSentenceEnd, SphDocID_t uDocid, int * pDoc );
	void				SkipTailHits ();

private:
	ExtNode_i *			m_pArg1;				///< left arg
	ExtNode_i *			m_pArg2;				///< right arg
	ExtTerm_c *			m_pDot;					///< dot positions

	ExtHit_t			m_dMyHits[MAX_HITS];	///< matching hits buffer (inherited m_dHits will receive filtered results)
	SphDocID_t			m_uHitsOverFor;			///< no more hits for matches block starting with this ID
	SphDocID_t			m_uTailDocid;			///< trailing docid
	DWORD				m_uTailSentenceEnd;		///< trailing hits filtering state

	const ExtDoc_t *	m_pDocs1;		///< last chunk start
	const ExtDoc_t *	m_pDocs2;		///< last chunk start
	const ExtDoc_t *	m_pDotDocs;		///< last chunk start
	const ExtDoc_t *	m_pDoc1;		///< current in-chunk ptr
	const ExtDoc_t *	m_pDoc2;		///< current in-chunk ptr
	const ExtDoc_t *	m_pDotDoc;		///< current in-chunk ptr
	const ExtHit_t *	m_pHit1;		///< current in-chunk ptr
	const ExtHit_t *	m_pHit2;		///< current in-chunk ptr
	const ExtHit_t *	m_pDotHit;		///< current in-chunk ptr
};

//////////////////////////////////////////////////////////////////////////

/// per-document zone information (span start/end positions)
struct ZoneInfo_t
{
	CSphVector<Hitpos_t> m_dStarts;
	CSphVector<Hitpos_t> m_dEnds;
};


/// zone hash key, zoneid+docid
struct ZoneKey_t
{
	int				m_iZone;
	SphDocID_t		m_uDocid;

	bool operator == ( const ZoneKey_t & rhs ) const
	{
		return m_iZone==rhs.m_iZone && m_uDocid==rhs.m_uDocid;
	}
};


/// zone hashing function
struct ZoneHash_fn
{
	static inline int Hash ( const ZoneKey_t & tKey )
	{
		return (DWORD)tKey.m_uDocid ^ ( tKey.m_iZone<<16 );
	}
};


/// zone hash
typedef CSphOrderedHash < ZoneInfo_t, ZoneKey_t, ZoneHash_fn, 4096, 117 > ZoneHash_c;


/// ranker interface
/// ranker folds incoming hitstream into simple match chunks, and computes relevance rank
class ExtRanker_c : public ISphRanker, public ISphZoneCheck
{
public:
								ExtRanker_c ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup );
	virtual						~ExtRanker_c ();
	virtual void				Reset ( const ISphQwordSetup & tSetup );

	virtual CSphMatch *			GetMatchesBuffer () { return m_dMatches; }
	virtual const ExtDoc_t *	GetFilteredDocs ();

	void						GetQwords ( ExtQwordsHash_t & hQwords )					{ if ( m_pRoot ) m_pRoot->GetQwords ( hQwords ); }
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				InitState ( const CSphQueryContext &, CSphString & )	{ return true; }

public:
	// FIXME? hide and friend?
	virtual SphZoneHit_e		IsInZone ( int iZone, const ExtHit_t * pHit );

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

protected:
	CSphVector<CSphString>		m_dZones;
	CSphVector<ExtTerm_c*>		m_dZoneStartTerm;
	CSphVector<ExtTerm_c*>		m_dZoneEndTerm;
	CSphVector<const ExtDoc_t*>	m_dZoneStart;
	CSphVector<const ExtDoc_t*>	m_dZoneEnd;
	CSphVector<SphDocID_t>		m_dZoneMax;				///< last docid we (tried) to cache
	CSphVector<SphDocID_t>		m_dZoneMin;				///< first docid we (tried) to cache
	ZoneHash_c					m_hZoneInfo;
};


STATIC_ASSERT ( ( 8*8*sizeof(DWORD) )>=SPH_MAX_FIELDS, PAYLOAD_MASK_OVERFLOW );

static const bool WITH_BM25 = true;

template < bool USE_BM25 = false >
class ExtRanker_WeightSum_c : public ExtRanker_c
{
protected:
	int				m_iWeights;
	const int *		m_pWeights;

public:
					ExtRanker_WeightSum_c ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup ) : ExtRanker_c ( tXQ, tSetup ) {}
	virtual int		GetMatches ();

	virtual bool InitState ( const CSphQueryContext & tCtx, CSphString & )
	{
		m_iWeights = tCtx.m_iWeights;
		m_pWeights = tCtx.m_dWeights;
		return true;
	}
};


class ExtRanker_None_c : public ExtRanker_c
{
public:
					ExtRanker_None_c ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup ) : ExtRanker_c ( tXQ, tSetup ) {}
	virtual int		GetMatches ();
};


template < typename STATE >
class ExtRanker_T : public ExtRanker_c
{
protected:
	STATE			m_tState;

public:
					ExtRanker_T<STATE> ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup ) : ExtRanker_c ( tXQ, tSetup ) {}
	virtual int		GetMatches ();

	virtual bool InitState ( const CSphQueryContext & tCtx, CSphString & sError )
	{
		return m_tState.Init ( tCtx.m_iWeights, &tCtx.m_dWeights[0], this, sError );
}
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
	, m_uMaxID(0)
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
	pWord->m_bExpanded = tWord.m_bExpanded;
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


static DWORD CalcDupeMask ( const CSphVector<XQNode_t *> & dChildren )
{
	DWORD uDupeMask = 0;
	ARRAY_FOREACH ( i, dChildren )
	{
		int iValue = 1;
		for ( int j = i+1; j<dChildren.GetLength(); j++ )
		{
			if ( KeywordsEqual ( dChildren[i], dChildren[j] ) )
			{
				iValue = 0;
				break;
			}
		}
		uDupeMask |= iValue << i;
	}
	return uDupeMask;
}


static DWORD CalcDupeMask ( const CSphVector<ISphQword *> & dQwordsHit )
{
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
	return uDupeMask;
}


template < typename T, bool NEED_MASK >
static ExtNode_i * CreateMultiNode ( const XQNode_t * pQueryNode, const ISphQwordSetup & tSetup, bool bNeedsHitlist )
{
	///////////////////////////////////
	// virtually plain (expanded) node
	///////////////////////////////////

	if ( pQueryNode->m_dChildren.GetLength() )
	{
		CSphVector<ExtNode_i *> dNodes;
		ARRAY_FOREACH ( i, pQueryNode->m_dChildren )
		{
			dNodes.Add ( ExtNode_i::Create ( pQueryNode->m_dChildren[i], tSetup ) );
			assert ( dNodes.Last()->m_iAtomPos>=0 );
		}

		// compute dupe mask (needed for quorum only)
		// FIXME! this check will fail with wordforms and stuff; sorry, no wordforms vs expand vs quorum support for now!
		DWORD uDupeMask = NEED_MASK
			? CalcDupeMask ( pQueryNode->m_dChildren )
			: 0;
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
			dNodes.Add ( ExtNode_i::Create ( dQwordsHit[i], pQueryNode, tSetup ) );
			dNodes.Last()->m_iAtomPos = dQwordsHit[i]->m_iAtomPos;
		}

		// compute dupe mask (needed for quorum only)
		DWORD uDupeMask = NEED_MASK
			? CalcDupeMask ( dQwordsHit )
			: 0;
		pResult = new T ( dNodes, uDupeMask, *pQueryNode, tSetup );
	}

	// AND result with the words that had no hitlist
	if ( dQwords.GetLength() )
	{
		ExtNode_i * pNode = ExtNode_i::Create ( dQwords[0], pQueryNode, tSetup );
		for ( int i=1; i<dQwords.GetLength(); i++ )
			pNode = new ExtAnd_c ( pNode, ExtNode_i::Create ( dQwords[i], pQueryNode, tSetup ), tSetup );
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

ExtNode_i * ExtNode_i::Create ( const XQKeyword_t & tWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
{
	return Create ( CreateQueryWord ( tWord, tSetup ), pNode, tSetup );
};

ExtNode_i * ExtNode_i::Create ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
{
	assert ( pQword );

	if ( pNode->m_iFieldMaxPos )
		pQword->m_iTermPos = TERM_POS_FIELD_LIMIT;

	if ( pNode->m_dZones.GetLength() )
		pQword->m_iTermPos = TERM_POS_ZONES;

	if ( !pQword->m_bHasHitlist )
	{
		if ( tSetup.m_pWarning && pQword->m_iTermPos )
			tSetup.m_pWarning->SetSprintf ( "hitlist unavailable, position limit ignored" );
		return new ExtTermHitless_c ( pQword, pNode->m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
	switch ( pQword->m_iTermPos )
	{
		case TERM_POS_FIELD_STARTEND:	return new ExtTermPos_c<TERM_POS_FIELD_STARTEND> ( pQword, pNode, tSetup );
		case TERM_POS_FIELD_START:		return new ExtTermPos_c<TERM_POS_FIELD_START> ( pQword, pNode, tSetup );
		case TERM_POS_FIELD_END:		return new ExtTermPos_c<TERM_POS_FIELD_END> ( pQword, pNode, tSetup );
		case TERM_POS_FIELD_LIMIT:		return new ExtTermPos_c<TERM_POS_FIELD_LIMIT> ( pQword, pNode, tSetup );
		case TERM_POS_ZONES:			return new ExtTermPos_c<TERM_POS_ZONES> ( pQword, pNode, tSetup );
		default:						return new ExtTerm_c ( pQword, pNode->m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
}

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
				return Create ( pNode->m_dWords[0], pNode, tSetup );
		}

		switch ( pNode->GetOp() )
		{
			case SPH_QUERY_PHRASE:
				return CreateMultiNode<ExtPhrase_c,false> ( pNode, tSetup, true );

			case SPH_QUERY_PROXIMITY:
				return CreateMultiNode<ExtProximity_c,false> ( pNode, tSetup, true );

			case SPH_QUERY_NEAR:
				return CreateMultiNode<ExtMultinear_c,false> ( pNode, tSetup, true );

			case SPH_QUERY_QUORUM:
			{
				assert ( pNode->m_dWords.GetLength()==0 || pNode->m_dChildren.GetLength()==0 );
				int iQuorumCount = pNode->m_dWords.GetLength()+pNode->m_dChildren.GetLength();
				if ( pNode->m_iOpArg>=iQuorumCount )
				{
					// threshold is too high
					if ( tSetup.m_pWarning )
						tSetup.m_pWarning->SetSprintf ( "quorum threshold too high (words=%d, thresh=%d); replacing quorum operator with AND operator",
							iQuorumCount, pNode->m_iOpArg );

				} else if ( iQuorumCount>32 )
				{
					// right now quorum can only handle 32 words
					if ( tSetup.m_pWarning )
						tSetup.m_pWarning->SetSprintf ( "too many words (%d) for quorum; replacing with an AND", iQuorumCount );

				} else // everything is ok; create quorum node
					return CreateMultiNode<ExtQuorum_c,true> ( pNode, tSetup, false );

				// couldn't create quorum, make an AND node instead
				CSphVector<ExtNode_i*> dTerms;
				dTerms.Reserve ( iQuorumCount );

				ARRAY_FOREACH ( i, pNode->m_dWords )
					dTerms.Add ( Create ( pNode->m_dWords[i], pNode, tSetup ) );

				ARRAY_FOREACH ( i, pNode->m_dChildren )
					dTerms.Add ( Create ( pNode->m_dChildren[i], tSetup ) );

				// make not simple, but optimized AND node.
				dTerms.Sort ( ExtNodeTF_fn() );

				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<dTerms.GetLength(); i++ )
					pCur = new ExtAnd_c ( pCur, dTerms[i], tSetup );

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
				dTerms.Add ( ExtNode_i::Create ( pChild, tSetup ) );
			}

			dTerms.Sort ( ExtNodeTF_fn() );

			ExtNode_i * pCur = dTerms[0];
			for ( int i=1; i<iChildren; i++ )
				pCur = new ExtAnd_c ( pCur, dTerms[i], tSetup );

			if ( pNode->GetCount() )
				return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );

			return pCur;
		}

		// Multinear could be also non-plain, so here is the second entry for it.
		if ( pNode->GetOp()==SPH_QUERY_NEAR )
			return CreateMultiNode<ExtMultinear_c,false> ( pNode, tSetup, true );

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
				case SPH_QUERY_OR:			pCur = new ExtOr_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_AND:			pCur = new ExtAnd_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_ANDNOT:		pCur = new ExtAndNot_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_SENTENCE:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dFieldMask, tSetup, MAGIC_WORD_SENTENCE ); break;
				case SPH_QUERY_PARAGRAPH:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dFieldMask, tSetup, MAGIC_WORD_PARAGRAPH ); break;
				default:					assert ( 0 && "internal error: unhandled op in ExtNode_i::Create()" ); break;
			}
		}
		if ( pNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
		return pCur;
	}
}

//////////////////////////////////////////////////////////////////////////

ExtTerm_c::ExtTerm_c ( ISphQword * pQword, const CSphSmallBitvec& dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
	: m_pQword ( pQword )
	, m_pWarning ( tSetup.m_pWarning )
	, m_bNotWeighted ( bNotWeighted )
{
	m_iAtomPos = pQword->m_iAtomPos;
	m_pHitDoc = NULL;
	m_uHitsOverFor = 0;
	m_dFields = dFields;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	if ( tSetup.m_pIndex )
		m_bLongMask = tSetup.m_pIndex->GetMatchSchema().m_dFields.GetLength()>32;
	else
		m_bLongMask = false;

	AllocDocinfo ( tSetup );
}

ExtTerm_c::ExtTerm_c ( ISphQword * pQword, const ISphQwordSetup & tSetup )
	: m_pQword ( pQword )
	, m_pWarning ( tSetup.m_pWarning )
	, m_bNotWeighted ( true )
{
	m_iAtomPos = pQword->m_iAtomPos;
	m_pHitDoc = NULL;
	m_uHitsOverFor = 0;
	m_dFields.Set();
	m_iMaxTimer = tSetup.m_iMaxTimer;
	if ( tSetup.m_pIndex )
		m_bLongMask = tSetup.m_pIndex->GetMatchSchema().m_dFields.GetLength()>32;
	else
		m_bLongMask = false;

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
		if (!( m_pQword->m_dFields.Test ( m_dFields ) ))
		{
			if ( (!m_bLongMask) || m_pQword->m_bAllFieldsKnown )
				continue;
			m_pQword->CollectHitMask();
			if (!( m_pQword->m_dFields.Test ( m_dFields ) ))
				continue;
		}

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_uDocid = tMatch.m_iDocID;
		tDoc.m_pDocinfo = pDocinfo;
		tDoc.m_uHitlistOffset = m_pQword->m_iHitlistPos;
		tDoc.m_dFields = m_pQword->m_dFields & m_dFields; // OPTIMIZE: only needed for phrase node
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
		Hitpos_t uHit = m_pQword->GetNextHit();
		if ( uHit==EMPTY_HIT )
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

		if (!( m_dFields.Test ( HITMAN::GetField ( uHit ) ) ))
			continue;

		ExtHit_t & tHit = m_dHits[iHit++];
		tHit.m_uDocid = pDoc->m_uDocid;
		tHit.m_uHitpos = uHit;
		tHit.m_uQuerypos = (WORD) m_iAtomPos; // assume less that 64K words per query
		tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
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
	if ( m_bNotWeighted || hQwords.Exists ( m_pQword->m_sWord ) )
		return;

	m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = m_pQword->m_sWord;
	tInfo.m_sDictWord = m_pQword->m_sDictWord;
	tInfo.m_iDocs = m_pQword->m_iDocs;
	tInfo.m_iHits = m_pQword->m_iHits;
	tInfo.m_iQueryPos = m_pQword->m_iAtomPos;
	tInfo.m_fIDF = -1.0f; // suppress gcc 4.2.3 warning
	tInfo.m_bExpanded = m_pQword->m_bExpanded;
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

		m_dFieldMask = pDoc->m_dFields;
		m_uFieldPos = 0;
	}

	// hit emission
	int iHit = 0;
	for ( ;; )
	{
		if ( m_dFieldMask.Test ( m_uFieldPos ) )
		{
			if ( m_dFields.Test ( m_uFieldPos ) )
			{
				// emit hit
				ExtHit_t & tHit = m_dHits[iHit++];
				tHit.m_uDocid = pDoc->m_uDocid;
				tHit.m_uHitpos = HITMAN::Create ( m_uFieldPos, -1 );
				tHit.m_uQuerypos = (WORD) m_iAtomPos;
				tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;

				if ( iHit==MAX_HITS-1 )
					break;
			}
		}

		if ( m_uFieldPos < CSphSmallBitvec::iTOTALBITS-1 )
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

		m_dFieldMask = pDoc->m_dFields;
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
ExtTermPos_c<T>::ExtTermPos_c ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: ExtTerm_c ( pQword, pNode->m_dFieldMask, tSetup, pNode->m_bNotWeighted )
	, m_iMaxFieldPos ( pNode->m_iFieldMaxPos )
	, m_uTermMaxID ( 0 )
	, m_pRawDocs ( NULL )
	, m_pRawDoc ( NULL )
	, m_pRawHit ( NULL )
	, m_uLastID ( 0 )
	, m_eState ( COPY_DONE )
	, m_uDoneFor ( 0 )
	, m_pZoneChecker ( tSetup.m_pZoneChecker )
	, m_dZones ( pNode->m_dZones )
	, m_uLastZonedId ( 0 )
	, m_iCheckFrom ( 0 )
{
	m_dMyDocs[0].m_uDocid = DOCID_MAX;
	m_dMyHits[0].m_uDocid = DOCID_MAX;
	m_dFilteredHits[0].m_uDocid = DOCID_MAX;

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
	m_eState = COPY_DONE;
	m_uDoneFor = 0;
	m_uLastZonedId = 0;
	m_iCheckFrom = 0;
	m_dMyDocs[0].m_uDocid = DOCID_MAX;
	m_dMyHits[0].m_uDocid = DOCID_MAX;
	m_dFilteredHits[0].m_uDocid = DOCID_MAX;
}

template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_LIMIT>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )<=m_iMaxFieldPos;
}


template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_START>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1;
}


template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_END>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::IsEnd ( pHit->m_uHitpos );
}


template<>
inline bool ExtTermPos_c<TERM_POS_FIELD_STARTEND>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1 && HITMAN::IsEnd ( pHit->m_uHitpos );
}


template<>
inline bool ExtTermPos_c<TERM_POS_ZONES>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	assert ( m_pZoneChecker );

	if ( m_uLastZonedId!=pHit->m_uDocid )
		m_iCheckFrom = 0;
	m_uLastZonedId = pHit->m_uDocid;

	// only check zones that actually match this document
	for ( int i=m_iCheckFrom; i<m_dZones.GetLength(); i++ )
	{
		SphZoneHit_e eState = m_pZoneChecker->IsInZone ( m_dZones[i], pHit );
		switch ( eState )
		{
		case SPH_ZONE_FOUND:
			return true;
		case SPH_ZONE_NO_DOCUMENT:
			Swap ( m_dZones[i], m_dZones[m_iCheckFrom] );
			m_iCheckFrom++;
			break;
		}
	}
	return false;
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
		m_uLastID = 0;
	}

	// filter the hits, and build the documents list
	int iMyDoc = 0;
	int iMyHit = 0;

	const ExtDoc_t * pDoc = m_pRawDoc; // just a shortcut
	const ExtHit_t * pHit = m_pRawHit;
	const SphDocID_t uSkipID = m_uLastID;
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

		// skip all tail hits hits from documents below or same ID as uSkipID

		// scan until next acceptable hit
		while ( pHit->m_uDocid < pDoc->m_uDocid || ( uSkipID && pHit->m_uDocid<=uSkipID ) ) // skip leftovers
			pHit++;

		while ( ( pHit->m_uDocid!=DOCID_MAX || ( uSkipID && pHit->m_uDocid<=uSkipID ) ) && !IsAcceptableHit ( pHit ) ) // skip unneeded hits
			pHit++;

		if ( pHit->m_uDocid==DOCID_MAX || ( uSkipID && pHit->m_uDocid<=uSkipID ) ) // check for eof
			continue;

		// find and emit new document
		while ( pDoc->m_uDocid<pHit->m_uDocid ) pDoc++; // FIXME? unsafe in broken cases
		assert ( pDoc->m_uDocid==pHit->m_uDocid );
		assert ( iMyDoc<MAX_DOCS-1 );

		if ( uLastID!=pDoc->m_uDocid )
			CopyExtDoc ( m_dMyDocs[iMyDoc++], *pDoc, &pDocinfo, m_iStride );
		uLastID = pDoc->m_uDocid;

		// copy acceptable hits for this document
		while ( iMyHit<MAX_HITS-1 && pHit->m_uDocid==uLastID )
		{
			if ( IsAcceptableHit ( pHit ) )
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
const ExtHit_t * ExtTermPos_c<T>::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
{
	if ( m_eState==COPY_DONE )
	{
		// this request completed in full
		if ( m_uDoneFor==pDocs->m_uDocid || !m_uDoneFor )
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
			m_pRawHit = ExtTerm_c::GetHitsChunk ( m_pRawDocs, Min ( uMaxID, m_uTermMaxID ) );

		// no more hits for current chunk
		if ( !m_pRawHit )
		{
			m_eState = COPY_DONE;
			break;
		}

		// copy while we can
		while ( m_pRawHit->m_uDocid==m_uLastID && iFilteredHits<MAX_HITS-1 )
		{
			if ( IsAcceptableHit ( m_pRawHit ) )
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
	m_dNodePos[0] = 0;
	m_dNodePos[1] = 0;
	m_bPosAware = false;
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
			tDoc.m_dFields = pCur0->m_dFields | pCur1->m_dFields;
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
	WORD uNodePos0 = m_dNodePos[0];
	WORD uNodePos1 = m_dNodePos[1];
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
				if ( ( pCur0->m_uHitpos < pCur1->m_uHitpos )
					|| ( pCur0->m_uHitpos==pCur1->m_uHitpos && pCur0->m_uQuerypos>pCur1->m_uQuerypos ) )
				{
					m_dHits[iHit] = *pCur0++;
					if ( uNodePos0!=0 )
						m_dHits[iHit++].m_uNodepos = uNodePos0;
					else
						iHit++;
					if ( pCur0->m_uDocid!=m_uMatchedDocid )
						break;
				} else
				{
					m_dHits[iHit] = *pCur1++;
					if ( uNodePos1!=0 )
						m_dHits[iHit++].m_uNodepos = uNodePos1;
					else
						iHit++;
					if ( pCur1->m_uDocid!=m_uMatchedDocid )
						break;
				}
			}

			// copy tail, while possible, unless the other child is at the end of a hit block
			if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid && !( pCur1 && pCur1->m_uDocid==DOCID_MAX ) )
			{
				while ( pCur0->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
				{
					m_dHits[iHit] = *pCur0++;
					if ( uNodePos0!=0 )
						m_dHits[iHit++].m_uNodepos = uNodePos0;
					else
						iHit++;
				}
			}
			if ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid && !( pCur0 && pCur0->m_uDocid==DOCID_MAX ) )
			{
				while ( pCur1->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
				{
					m_dHits[iHit] = *pCur1++;
					if ( uNodePos1!=0 )
						m_dHits[iHit++].m_uNodepos = uNodePos1;
					else
						iHit++;
				}
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

inline int BispanLen ( const ExtHit_t * pLeft, const ExtHit_t * pRight )
{
	return pRight->m_uSpanlen + pRight->m_uHitpos - pLeft->m_uHitpos;
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
					m_dDocs[iDoc].m_dFields = pCur0->m_dFields | pCur1->m_dFields;
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

			// a pretty tricky bit
			// one of the nodes might have run out of current hits chunk (rather hits at all)
			// so we need to get the next hits chunk NOW, check for that condition, and keep merging
			// simply going to tail hits copying is incorrect, it could copy in wrong order
			// example, word A, pos 1, 2, 3, hit chunk ends, 4, 5, 6, word B, pos 7, 8, 9
			if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX )
			{
				pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs, uMaxID );
				if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid )
					continue;
			}
			if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
			{
				pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs, uMaxID );
				if ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid )
					continue;
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

ExtNWayT::ExtNWayT ( const CSphVector<ExtNode_i *> & dNodes, DWORD, const XQNode_t &, const ISphQwordSetup & tSetup )
	: m_pNode ( NULL )
	, m_pDocs ( NULL )
	, m_pHits ( NULL )
	, m_pDoc ( NULL )
	, m_pHit ( NULL )
	, m_pMyDoc ( NULL )
	, m_pMyHit ( NULL )
	, m_uLastDocID ( 0 )
	, m_uMatchedDocid ( 0 )
	, m_uHitsOverFor ( 0 )
{
	assert ( dNodes.GetLength()>1 );
	m_iAtomPos = dNodes[0]->m_iAtomPos;
	m_dMyHits[0].m_uDocid = DOCID_MAX;
	AllocDocinfo ( tSetup );
}

ExtNWayT::~ExtNWayT ()
{
	SafeDelete ( m_pNode );
}

void ExtNWayT::Reset ( const ISphQwordSetup & tSetup )
{
	m_pNode->Reset ( tSetup );
	m_pDocs = NULL;
	m_pHits = NULL;
	m_pDoc = NULL;
	m_pHit = NULL;
	m_pMyDoc = NULL;
	m_pMyHit = NULL;
	m_uLastDocID = 0;
	m_uMatchedDocid = 0;
	m_uHitsOverFor = 0;
	m_dMyHits[0].m_uDocid = DOCID_MAX;
}

void ExtNWayT::GetQwords ( ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->GetQwords ( hQwords );
}

void ExtNWayT::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->SetQwordsIDF ( hQwords );
}

template < class FSM >
const ExtDoc_t * ExtNWay_c<FSM>::GetDocsChunk ( SphDocID_t * pMaxID )
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

	FSM::ResetFSM();

	// skip leftover hits
	while ( m_uLastDocID )
	{
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
		{
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, m_uDocsMaxID );
			if ( !pHit )
				break;
		}

		while ( pHit->m_uDocid==m_uLastDocID )
			pHit++;

		if ( pHit->m_uDocid!=DOCID_MAX && pHit->m_uDocid!=m_uLastDocID )
			m_uLastDocID = 0;
	}

	// search for matches
	int iDoc = 0;
	int iHit = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	while ( iHit<MAX_HITS-1 )
	{
		// out of hits?
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
		{
			// grab more hits
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, m_uDocsMaxID );
			if ( m_pHits ) continue;

			m_uMatchedDocid = 0;

			// no more hits for current docs chunk; grab more docs
			pDoc = m_pDocs = m_pNode->GetDocsChunk ( &m_uDocsMaxID );
			if ( !m_pDocs ) break;

			// we got docs, there must be hits
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs, m_uDocsMaxID );
			assert ( pHit );
			continue;
		}

		// check if the incoming hit is out of bounds, or affects min pos
		if ( pHit->m_uDocid!=m_uMatchedDocid )
		{
			m_uMatchedDocid = pHit->m_uDocid;
			FSM::ResetFSM();
			continue;
		}

		if ( FSM::HitFSM ( pHit, &m_dMyHits[iHit] ) )
		{
			// emit document, if it's new
			if ( pHit->m_uDocid!=m_uLastDocID )
			{
				assert ( pDoc->m_uDocid<=pHit->m_uDocid );
				while ( pDoc->m_uDocid < pHit->m_uDocid ) pDoc++;
				assert ( pDoc->m_uDocid==pHit->m_uDocid );

				m_dDocs[iDoc].m_uDocid = pHit->m_uDocid;
				m_dDocs[iDoc].m_dFields.Unset();
				m_dDocs[iDoc].m_dFields.Set ( HITMAN::GetField ( pHit->m_uHitpos ) );
				m_dDocs[iDoc].m_uHitlistOffset = -1;
				m_dDocs[iDoc].m_fTFIDF = pDoc->m_fTFIDF;
				CopyExtDocinfo ( m_dDocs[iDoc], *pDoc, &pDocinfo, m_iStride );
				iDoc++;
				m_uLastDocID = pHit->m_uDocid;
			}
			iHit++;
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

template < class FSM >
const ExtHit_t * ExtNWay_c<FSM>::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
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
			// the trickiest part; handle the end of my hitlist chunk
			// The doclist chunk was built from it; so it must be the end of doclist as well.
			// Covered by test 114.
			assert ( pMyDoc[1].m_uDocid==DOCID_MAX );

			// keep scanning and-node hits while there are hits for the last matched document
			assert ( m_uMatchedDocid==pMyDoc->m_uDocid );
			assert ( m_uMatchedDocid==m_uLastDocID );
			assert ( !m_pDoc || m_uMatchedDocid==m_pDoc->m_uDocid );
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

template < class FSM >
bool ExtNWay_c<FSM>::EmitTail ( int & iHit )
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
		if ( pHit->m_uDocid!=m_uMatchedDocid )
		{
			// reset hits getter; this docs chunk from above is finally over
			bTailFinished = true;
			m_uMatchedDocid = 0;
			pMyDoc++;
			break;
		}

		if ( FSM::HitFSM ( pHit, &m_dHits[iHit] ) )
			iHit++;
		pHit++;
	}
	// save shortcut
	m_pHit = pHit;
	m_pMyDoc = pMyDoc;
	return bTailFinished;
}

//////////////////////////////////////////////////////////////////////////

FSMphrase::FSMphrase ( const CSphVector<ExtNode_i *> & dQwords, DWORD, const XQNode_t & , const ISphQwordSetup & )
	: m_uExpQpos ( 0 )
	, m_uExpPos ( 0 )
	, m_uLeaves ( dQwords.GetLength() )
{
	m_uMinQpos = dQwords[0]->m_iAtomPos;
	m_uMaxQpos = dQwords.Last()->m_iAtomPos;
	m_dQposDelta.Resize ( m_uMaxQpos-m_uMinQpos+1 );
	ARRAY_FOREACH ( i, m_dQposDelta )
		m_dQposDelta[i] = -INT_MAX;

	for ( int i=1; i<(int)m_uLeaves; i++ )
		m_dQposDelta [ dQwords[i-1]->m_iAtomPos - dQwords[0]->m_iAtomPos ] = dQwords[i]->m_iAtomPos - dQwords[i-1]->m_iAtomPos;
}

inline bool FSMphrase::HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget )
{
	// unexpected too-low position? must be duplicate keywords for the previous one ("aaa bbb aaa ccc" case); just skip them
	if ( HITMAN::GetLCS ( pHit->m_uHitpos )<m_uExpPos )
		return false;
	// unexpected position? reset and continue
	if ( HITMAN::GetLCS ( pHit->m_uHitpos )!=m_uExpPos )
	{
		// stream position out of sequence; reset expected positions
		if ( pHit->m_uQuerypos==m_uMinQpos )
		{
			m_uExpPos = HITMAN::GetLCS ( pHit->m_uHitpos ) + m_dQposDelta[0];
			m_uExpQpos = pHit->m_uQuerypos + m_dQposDelta[0];
		} else
			m_uExpPos = m_uExpQpos = 0;
		return false;
	}

	// scan all hits with matching stream position
	// duplicate stream positions occur when there are duplicate query words
	// stream position is as expected; let's check query position
	if ( pHit->m_uQuerypos!=m_uExpQpos )
	{
		// unexpected query position
		// do nothing; there might be other words in same (!) expected position following, with proper query positions
		// (eg. if the query words are repeated)
		if ( pHit->m_uQuerypos==m_uMinQpos )
		{
			m_uExpPos = pHit->m_uHitpos + m_dQposDelta[0];
			m_uExpQpos = pHit->m_uQuerypos + m_dQposDelta[0];
		}
		return false;
	}

	if ( m_uExpQpos!=m_uMaxQpos )
	{
		// intermediate expected position; keep looking
		assert ( pHit->m_uQuerypos==m_uExpQpos );
		int iDelta = m_dQposDelta [ pHit->m_uQuerypos - m_uMinQpos ];
		m_uExpPos += iDelta;
		m_uExpQpos += iDelta;
		// FIXME! what if there *more* hits with current pos following?
		return false;
	}

	// expected position which concludes the phrase; emit next match
	assert ( pHit->m_uQuerypos==m_uExpQpos );

	DWORD uSpanlen = m_uMaxQpos - m_uMinQpos;

	// emit directly into m_dHits, this is no need to disturb m_dMyHits here.
	dTarget->m_uDocid = pHit->m_uDocid;
	dTarget->m_uHitpos = HITMAN::GetLCS ( pHit->m_uHitpos ) - uSpanlen;
	dTarget->m_uQuerypos = (WORD) m_uMinQpos;
	dTarget->m_uMatchlen = dTarget->m_uSpanlen = (WORD)( uSpanlen + 1 );
	dTarget->m_uWeight = m_uLeaves;
	m_uExpPos = m_uExpQpos = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////

FSMproximity::FSMproximity ( const CSphVector<ExtNode_i *> & dQwords, DWORD, const XQNode_t & tNode, const ISphQwordSetup & )
	: m_iMaxDistance ( tNode.m_iOpArg )
	, m_uWordsExpected ( dQwords.GetLength() )
	, m_uExpPos ( 0 )
{
	assert ( m_iMaxDistance>0 );
	m_uMinQpos = dQwords[0]->m_iAtomPos;
	m_uQLen = dQwords.Last()->m_iAtomPos - m_uMinQpos;
	m_dProx.Resize ( m_uQLen+1 );
	m_dDeltas.Resize ( m_uQLen+1 );
}

inline bool FSMproximity::HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget )
{
	// walk through the hitlist and update context
	int iQindex = pHit->m_uQuerypos - m_uMinQpos;
	DWORD uHitpos = HITMAN::GetLCS ( pHit->m_uHitpos );

	// check if the word is new
	if ( m_dProx[iQindex]==UINT_MAX )
		m_uWords++;

	// update the context
	m_dProx[iQindex] = uHitpos;

	// check if the incoming hit is out of bounds, or affects min pos
	if ( uHitpos>=m_uExpPos // out of expected bounds
		|| iQindex==m_iMinQindex ) // or simply affects min pos
	{
		m_iMinQindex = iQindex;
		int iMinPos = uHitpos - m_uQLen - m_iMaxDistance;

		ARRAY_FOREACH ( i, m_dProx )
			if ( m_dProx[i]!=UINT_MAX )
			{
				if ( (int)m_dProx[i]<=iMinPos )
				{
					m_dProx[i] = UINT_MAX;
					m_uWords--;
					continue;
				}
				if ( m_dProx[i]<uHitpos )
				{
					m_iMinQindex = i;
					uHitpos = m_dProx[i];
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
	{
		m_dDeltas[i] = m_dProx[i] - i;
		uMax = Max ( uMax, m_dProx[i] );
	}
	m_dDeltas.Sort ();

	DWORD uWeight = 0;
	int iLast = -INT_MAX;
	ARRAY_FOREACH ( i, m_dDeltas )
	{
		if ( m_dDeltas[i]==iLast )
			uWeight++;
		else
			uWeight = 1;
		iLast = m_dDeltas[i];
	}

	// emit hit
	dTarget->m_uDocid = pHit->m_uDocid;
	dTarget->m_uHitpos = Hitpos_t ( m_dProx[m_iMinQindex] ); // !COMMIT strictly speaking this is creation from LCS not value
	dTarget->m_uQuerypos = (WORD) m_uMinQpos;
	dTarget->m_uSpanlen = dTarget->m_uMatchlen = (WORD)( uMax-m_dProx[m_iMinQindex]+1 );
	dTarget->m_uWeight = uWeight;

	// remove current min, and force recompue
	m_dProx[m_iMinQindex] = UINT_MAX;
	m_iMinQindex = -1;
	m_uWords--;
	m_uExpPos = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////

FSMmultinear::FSMmultinear ( const CSphVector<ExtNode_i *> & dNodes, DWORD, const XQNode_t & tNode, const ISphQwordSetup & )
	: m_iNear ( tNode.m_iOpArg )
	, m_uWordsExpected ( dNodes.GetLength() )
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

inline bool FSMmultinear::HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget )
{
	// walk through the hitlist and update context
	DWORD uHitpos = HITMAN::GetLCS ( pHit->m_uHitpos );
	WORD uNpos = pHit->m_uNodepos;
	WORD uQpos = pHit->m_uQuerypos;

	// skip dupe hit (may be emitted by OR node, for example)
	if ( m_uLastP==uHitpos )
	{
		// check if the hit is subset of another one
		if ( m_uPrelastP && m_uLastML < pHit->m_uMatchlen )
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
	if ( m_uLastP==0 || ( m_uLastP + m_uLastML + m_iNear )<=uHitpos )
	{
		m_uFirstHit = m_uLastP = uHitpos;
		m_uLastML = pHit->m_uMatchlen;
		m_uLastSL = pHit->m_uSpanlen;
		m_uWeight = m_uLastW = pHit->m_uWeight;
		if ( m_bTwofer )
		{
			m_uFirstQpos = uQpos;
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
		if ( ( m_uFirstHit + m_uLastML )>uHitpos
			&& ( m_uFirstHit + m_uLastML )<( uHitpos + pHit->m_uMatchlen )
			&& m_uLastML!=pHit->m_uMatchlen )
		{
			m_uFirstHit = m_uLastP = uHitpos;
			m_uLastML = pHit->m_uMatchlen;
			m_uLastSL = pHit->m_uSpanlen;
			m_uWeight = m_uLastW = pHit->m_uWeight;
			m_uFirstQpos = uQpos;
			m_uFirstNpos = uNpos;
			return false;
		}
		if ( uNpos==m_uFirstNpos )
		{
			if ( m_uLastP < uHitpos )
			{
				m_uPrelastML = m_uLastML;
				m_uPrelastSL = m_uLastSL;
				m_uPrelastP = m_uLastP;
				m_uPrelastW = pHit->m_uWeight;

				m_uFirstHit = m_uLastP = uHitpos;
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
						m_uFirstHit = HITMAN::GetLCS ( dHit.m_uHitpos );
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
			m_uFirstHit = HITMAN::GetLCS ( m_dRing[m_iRing].m_uHitpos );
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
		dTarget->m_uDocid = pHit->m_uDocid;
		dTarget->m_uHitpos = Hitpos_t ( m_uFirstHit ); // !COMMIT strictly speaking this is creation from LCS not value
		dTarget->m_uMatchlen = (WORD)( uHitpos - m_uFirstHit + m_uLastML );
		dTarget->m_uWeight = m_uWeight;
		m_uPrelastP = 0;

		if ( m_bTwofer ) // for exactly 2 words allow overlapping - so, just shift the chain, not reset it
		{
			dTarget->m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			dTarget->m_uSpanlen = 2;
			m_uFirstHit = m_uLastP = uHitpos;
			m_uWeight = pHit->m_uWeight;
			m_uFirstQpos = pHit->m_uQuerypos;
		} else
		{
			dTarget->m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			dTarget->m_uSpanlen = (WORD) m_dNpos.GetLength();
			m_uLastP = 0;
		}
		return true;
	}

	m_uLastP = uHitpos;
	return false;
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
		m_dInitialChildren.Add ( dQwords[i] );
		m_pCurDoc.Add ( NULL );
		m_pCurHit.Add ( NULL );
	}

	m_dChildren = m_dInitialChildren;

	m_uMask = m_uInitialMask = uDupeMask;
	m_uMaskEnd = dQwords.GetLength() - 1;
	m_uMatchedDocid = 0;
}

ExtQuorum_c::~ExtQuorum_c ()
{
	ARRAY_FOREACH ( i, m_dInitialChildren )
		SafeDelete ( m_dInitialChildren[i] );
}

void ExtQuorum_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bDone = false;

	m_pCurDoc.Resize ( m_dInitialChildren.GetLength() );
	m_pCurHit.Resize ( m_dInitialChildren.GetLength() );
	m_dChildren.Resize ( m_dInitialChildren.GetLength() );
	ARRAY_FOREACH ( i, m_dInitialChildren )
	{
		m_dChildren[i] = m_dInitialChildren[i];
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
		tCand.m_dFields.Unset();
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
				tCand.m_dFields |= m_pCurDoc[i]->m_dFields;
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

			uTouched &= ~(1UL<<i);
			uTouched |= ( ( uTouched >> (m_dChildren.GetLength()-1) ) & 1UL ) << i;

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
				if ( HITMAN::GetLCS ( m_pCurHit[i]->m_uHitpos ) < uMinPos )
		{
			uMinPos = HITMAN::GetLCS ( m_pCurHit[i]->m_uHitpos ); // !COMMIT bench/fix, is LCS right here?
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
	m_dMyHits[0].m_uDocid = DOCID_MAX;

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
	m_uHitsOverFor = 0;
	m_dMyHits[0].m_uDocid = DOCID_MAX;

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
			if ( HITMAN::GetLCS ( m_pHits[i]->m_uHitpos ) < uMinPos )
			{
				uMinPos = HITMAN::GetLCS ( m_pHits[i]->m_uHitpos );
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

	const ExtHit_t * pMyHits = m_dMyHits;
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
		if ( pDocs->m_uDocid!=DOCID_MAX )
		{
			iHit = GetMatchingHits ( pDocs->m_uDocid, m_dHits, MAX_HITS-1 );
		}
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

ExtUnit_c::ExtUnit_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const CSphSmallBitvec& uFields, const ISphQwordSetup & tSetup, const char * sUnit )
{
	m_pArg1 = pFirst;
	m_pArg2 = pSecond;

	XQKeyword_t tDot;
	tDot.m_sWord = sUnit;
	m_pDot = new ExtTerm_c ( CreateQueryWord ( tDot, tSetup ), uFields, tSetup, true );

	m_uHitsOverFor = 0;
	m_uTailDocid = 0;
	m_uTailSentenceEnd = 0;

	m_pDocs1 = NULL;
	m_pDocs2 = NULL;
	m_pDotDocs = NULL;
	m_pDoc1 = NULL;
	m_pDoc2 = NULL;
	m_pDotDoc = NULL;
	m_pHit1 = NULL;
	m_pHit2 = NULL;
	m_pDotHit = NULL;
	m_dMyHits[0].m_uDocid = DOCID_MAX;
}


ExtUnit_c::~ExtUnit_c ()
{
	SafeDelete ( m_pArg1 );
	SafeDelete ( m_pArg2 );
}


void ExtUnit_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pArg1->Reset ( tSetup );
	m_pArg2->Reset ( tSetup );
	m_pDot->Reset ( tSetup );
	m_dMyHits[0].m_uDocid = DOCID_MAX;
}


void ExtUnit_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_pArg1->GetQwords ( hQwords );
	m_pArg2->GetQwords ( hQwords );
}


void ExtUnit_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pArg1->SetQwordsIDF ( hQwords );
	m_pArg2->SetQwordsIDF ( hQwords );
}


/// skips hits until their docids are less than the given limit
static inline void SkipHitsLtDocid ( const ExtHit_t * (*ppHits), SphDocID_t uMatch, ExtNode_i * pNode, const ExtDoc_t * pDocs )
{
	for ( ;; )
	{
		const ExtHit_t * pHit = *ppHits;
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
		{
			pHit = *ppHits = pNode->GetHitsChunk ( pDocs, DOCID_MAX ); // OPTIMIZE? use that max?
			if ( !pHit )
				return;
		}

		while ( pHit->m_uDocid < uMatch )
			pHit++;

		*ppHits = pHit;
		if ( pHit->m_uDocid!=DOCID_MAX )
			return;
	}
}


/// skips hits within current document while their position is less or equal than the given limit
/// returns true if a matching hit (with big enough position, and in current document) was found
/// returns false otherwise
static inline bool SkipHitsLtePos ( const ExtHit_t * (*ppHits), Hitpos_t uPos, ExtNode_i * pNode, const ExtDoc_t * pDocs )
{
	SphDocID_t uDocid = (*ppHits)->m_uDocid;
	for ( ;; )
	{
		const ExtHit_t * pHit = *ppHits;
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
		{
			pHit = *ppHits = pNode->GetHitsChunk ( pDocs, DOCID_MAX ); // OPTIMIZE? use that max?
			if ( !pHit )
				return false;
		}

		while ( pHit->m_uDocid==uDocid && pHit->m_uHitpos<=uPos )
			pHit++;

		*ppHits = pHit;
		if ( pHit->m_uDocid!=DOCID_MAX )
			return ( pHit->m_uDocid==uDocid );
	}
}


int ExtUnit_c::FilterHits ( int iMyHit, DWORD uSentenceEnd, SphDocID_t uDocid, int * pDoc )
{
	while ( iMyHit<MAX_HITS-1 )
	{
		if ( uSentenceEnd )
		{
			// we're in a matched sentence state
			// copy hits until next dot
			bool bValid1 = m_pHit1 && m_pHit1->m_uDocid==uDocid && m_pHit1->m_uHitpos<uSentenceEnd;
			bool bValid2 = m_pHit2 && m_pHit2->m_uDocid==uDocid && m_pHit2->m_uHitpos<uSentenceEnd;

			if ( !bValid1 && !bValid2 )
			{
				// no more hits in this sentence
				uSentenceEnd = 0;
				if ( m_pHit1 && m_pHit2 && m_pHit1->m_uDocid==uDocid && m_pHit2->m_uDocid==uDocid )
					continue; // no more in-sentence hits, but perhaps more sentences in this document
				else
					break; // document is over
			}

			// register document as matching
			if ( pDoc )
			{
				ExtDoc_t & tDoc = m_dDocs [ (*pDoc)++ ];
				tDoc.m_uDocid = m_pDoc1->m_uDocid;
				tDoc.m_dFields = m_pDoc1->m_dFields | m_pDoc2->m_dFields;
				tDoc.m_uHitlistOffset = -1;
				tDoc.m_fTFIDF = m_pDoc1->m_fTFIDF + m_pDoc2->m_fTFIDF;
				tDoc.m_pDocinfo = NULL; // no inline support, sorry
				pDoc = NULL; // just once
			}

			if ( bValid1 && ( !bValid2 || m_pHit1->m_uHitpos < m_pHit2->m_uHitpos ) )
			{
				m_dMyHits[iMyHit++] = *m_pHit1++;
				if ( m_pHit1->m_uDocid==DOCID_MAX )
					m_pHit1 = m_pArg1->GetHitsChunk ( m_pDocs1, 0 );

			} else
			{
				m_dMyHits[iMyHit++] = *m_pHit2++;
				if ( m_pHit2->m_uDocid==DOCID_MAX )
					m_pHit2 = m_pArg2->GetHitsChunk ( m_pDocs2, 0 );
			}

		} else
		{
			// no sentence matched yet
			// let's check the next hit pair
			assert ( m_pHit1->m_uDocid==uDocid );
			assert ( m_pHit2->m_uDocid==uDocid );
			assert ( m_pDotHit->m_uDocid==uDocid );

			// our current hit pair locations
			DWORD uMin = Min ( m_pHit1->m_uHitpos, m_pHit2->m_uHitpos );
			DWORD uMax = Max ( m_pHit1->m_uHitpos, m_pHit2->m_uHitpos );

			// skip all dots beyond the min location
			if ( !SkipHitsLtePos ( &m_pDotHit, uMin, m_pDot, m_pDotDocs ) )
			{
				// we have a match!
				// moreover, no more dots past min location in current document
				// copy hits until next document
				uSentenceEnd = UINT_MAX;
				continue;
			}

			// does the first post-pair-start dot separate our hit pair?
			if ( m_pDotHit->m_uHitpos < uMax )
			{
				// yes, got an "A dot B" case
				// rewind candidate hits past this dot, break if current document is over
				if ( !SkipHitsLtePos ( &m_pHit1, m_pDotHit->m_uHitpos, m_pArg1, m_pDocs1 ) )
					break;
				if ( !SkipHitsLtePos ( &m_pHit2, m_pDotHit->m_uHitpos, m_pArg2, m_pDocs2 ) )
					break;
				continue;

			} else
			{
				// we have a match!
				// copy hits until next dot
				if ( !SkipHitsLtePos ( &m_pDotHit, m_pDotHit->m_uHitpos, m_pDot, m_pDotDocs ) )
					uSentenceEnd = UINT_MAX; // correction, no next dot, so make it "next document"
				else
					uSentenceEnd = m_pDotHit->m_uHitpos;
				assert ( uSentenceEnd );
			}
		}
	}

	m_uTailSentenceEnd = uSentenceEnd; // just in case tail hits loop will happen
	return iMyHit;
}

void ExtUnit_c::SkipTailHits ()
{
	m_uTailDocid = 0;
	m_pDoc1++;
	m_pDoc2++;
}


const ExtDoc_t * ExtUnit_c::GetDocsChunk ( SphDocID_t * pMaxID )
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
	int iMyHit = 0;

	if ( m_uTailDocid )
		SkipTailHits();

	while ( iMyHit<MAX_HITS-1 )
	{
		// fetch more candidate docs, if needed
		if ( !m_pDoc1 || m_pDoc1->m_uDocid==DOCID_MAX )
		{
			m_pDoc1 = m_pDocs1 = m_pArg1->GetDocsChunk ( NULL );
			if ( !m_pDoc1 )
				break; // node is over
		}

		if ( !m_pDoc2 || m_pDoc2->m_uDocid==DOCID_MAX )
		{
			m_pDoc2 = m_pDocs2 = m_pArg2->GetDocsChunk ( NULL );
			if ( !m_pDoc2 )
				break; // node is over
		}

		// find next candidate match
		while ( m_pDoc1->m_uDocid!=m_pDoc2->m_uDocid && m_pDoc1->m_uDocid!=DOCID_MAX && m_pDoc2->m_uDocid!=DOCID_MAX )
		{
			while ( m_pDoc1->m_uDocid < m_pDoc2->m_uDocid && m_pDoc2->m_uDocid!=DOCID_MAX )
				m_pDoc1++;
			while ( m_pDoc1->m_uDocid > m_pDoc2->m_uDocid && m_pDoc1->m_uDocid!=DOCID_MAX )
				m_pDoc2++;
		}

		// got our candidate that matches AND?
		SphDocID_t uDocid = m_pDoc1->m_uDocid;
		if ( m_pDoc1->m_uDocid==DOCID_MAX || m_pDoc2->m_uDocid==DOCID_MAX )
			continue;

		// yes, now fetch more dots docs, if needed
		// note how NULL is accepted here, "A and B but no dots" case is valid!
		if ( !m_pDotDoc || m_pDotDoc->m_uDocid==DOCID_MAX )
			m_pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk ( NULL );

		// skip preceding docs
		while ( m_pDotDoc && m_pDotDoc->m_uDocid < uDocid )
		{
			while ( m_pDotDoc->m_uDocid < uDocid )
				m_pDotDoc++;

			if ( m_pDotDoc->m_uDocid==DOCID_MAX )
				m_pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk ( NULL );
		}

		// we will need document hits on both routes below
		SkipHitsLtDocid ( &m_pHit1, uDocid, m_pArg1, m_pDocs1 );
		SkipHitsLtDocid ( &m_pHit2, uDocid, m_pArg2, m_pDocs2 );
		assert ( m_pHit1->m_uDocid==uDocid );
		assert ( m_pHit2->m_uDocid==uDocid );

		DWORD uSentenceEnd = 0;
		if ( !m_pDotDoc || m_pDotDoc->m_uDocid!=uDocid )
		{
			// no dots in current document?
			// just copy all hits until next document
			uSentenceEnd = UINT_MAX;

		} else
		{
			// got both hits and dots
			// rewind to relevant dots hits, then do sentence boundary detection
			SkipHitsLtDocid ( &m_pDotHit, uDocid, m_pDot, m_pDotDocs );
		}

		// do those hits
		iMyHit = FilterHits ( iMyHit, uSentenceEnd, uDocid, &iDoc );

		// out of matching hits buffer? gotta return docs chunk now, then
		if ( iMyHit==MAX_HITS-1 )
		{
			// mark a possibility of some trailing hits for current dot, if any
			if ( ( m_pHit1 && m_pHit1->m_uDocid==uDocid ) || ( m_pHit2 && m_pHit2->m_uDocid==uDocid ) )
			{
				m_uTailDocid = uDocid; // yep, do check that tail
			} else
			{
				SkipTailHits(); // nope, both hit lists are definitely over
			}

			return ReturnDocsChunk ( iDoc, iMyHit, pMaxID );
		}

		// all hits copied; do the next candidate
		m_pDoc1++;
		m_pDoc2++;
	}

	return ReturnDocsChunk ( iDoc, iMyHit, pMaxID );
}


const ExtHit_t * ExtUnit_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t )
{
	SphDocID_t uFirstMatch = pDocs->m_uDocid;

	// current hits chunk already returned
	if ( m_uHitsOverFor==uFirstMatch )
	{
		// and there are no trailing hits? bail
		if ( !m_uTailDocid )
			return NULL;

		// and there might be trailing hits for the last document? try and loop them
		int iMyHit = FilterHits ( 0, m_uTailSentenceEnd, m_uTailDocid, NULL );
		if ( !iMyHit )
		{
			// no trailing hits were there actually
			m_uTailDocid = 0;
			m_pDoc1++;
			m_pDoc2++;
			return NULL;
		}

		// ok, we got some trailing hits!
		// check whether we might have even more
		if (!( iMyHit==MAX_HITS-1 && m_pHit1 && m_pHit1->m_uDocid==m_uTailDocid && m_pHit2 && m_pHit2->m_uDocid==m_uTailDocid ))
		{
			// nope, both hit lists are definitely over now
			m_uTailDocid = 0;
			m_pDoc1++;
			m_pDoc2++;
		}

		// return those trailing hits
		assert ( iMyHit<MAX_HITS );
		m_dMyHits[iMyHit].m_uDocid = DOCID_MAX;
		return m_dMyHits;
	}

	// no hits returned yet; do return them
	int iHit = 0;
	ExtHit_t * pMyHit = m_dMyHits;

	for ( ;; )
	{
		// skip filtered hits until next requested document
		while ( pMyHit->m_uDocid!=pDocs->m_uDocid )
		{
			while ( pDocs->m_uDocid < pMyHit->m_uDocid && pDocs->m_uDocid!=DOCID_MAX )
				pDocs++;
			if ( pDocs->m_uDocid==DOCID_MAX )
				break;
			while ( pMyHit->m_uDocid < pDocs->m_uDocid )
				pMyHit++;
		}

		// out of hits
		if ( pMyHit->m_uDocid==DOCID_MAX || pDocs->m_uDocid==DOCID_MAX )
		{
			// there still might be trailing hits
			// if so, they will be handled on next entry
			m_uHitsOverFor = uFirstMatch;
			if ( pDocs->m_uDocid==DOCID_MAX && m_uTailDocid )
				SkipTailHits();

			break;
		}

		// copy
		while ( pMyHit->m_uDocid==pDocs->m_uDocid )
			m_dHits[iHit++] = *pMyHit++;

		if ( pMyHit->m_uDocid==DOCID_MAX )
		{
			m_uHitsOverFor = uFirstMatch;
			break;
		}
	}

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtRanker_c::ExtRanker_c ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup )
{
	assert ( tSetup.m_pCtx );

	m_iInlineRowitems = tSetup.m_iInlineRowitems;
	for ( int i=0; i<ExtNode_i::MAX_DOCS; i++ )
	{
		m_dMatches[i].Reset ( tSetup.m_iDynamicRowitems );
		m_dMyMatches[i].Reset ( tSetup.m_iDynamicRowitems );
	}
	m_tTestMatch.Reset ( tSetup.m_iDynamicRowitems );

	assert ( tXQ.m_pRoot );
	tSetup.m_pZoneChecker = this;
	m_pRoot = ExtNode_i::Create ( tXQ.m_pRoot, tSetup );
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

	m_dZones = tXQ.m_dZones;
	m_dZoneStart.Resize ( m_dZones.GetLength() );
	m_dZoneEnd.Resize ( m_dZones.GetLength() );
	m_dZoneMax.Resize ( m_dZones.GetLength() );
	m_dZoneMin.Resize ( m_dZones.GetLength() );
	m_dZoneMax.Fill ( 0 );
	m_dZoneMin.Fill	( DOCID_MAX );

	ARRAY_FOREACH ( i, m_dZones )
	{
		XQKeyword_t tDot;

		tDot.m_sWord.SetSprintf ( "%c%s", MAGIC_CODE_ZONE, m_dZones[i].cstr() );
		m_dZoneStartTerm.Add ( new ExtTerm_c ( CreateQueryWord ( tDot, tSetup ), tSetup ) );
		m_dZoneStart[i] = NULL;

		tDot.m_sWord.SetSprintf ( "%c/%s", MAGIC_CODE_ZONE, m_dZones[i].cstr() );
		m_dZoneEndTerm.Add ( new ExtTerm_c ( CreateQueryWord ( tDot, tSetup ), tSetup ) );
		m_dZoneEnd[i] = NULL;
	}
}


ExtRanker_c::~ExtRanker_c ()
{
	SafeDelete ( m_pRoot );
	ARRAY_FOREACH ( i, m_dZones )
	{
		SafeDelete ( m_dZoneStartTerm[i] );
		SafeDelete ( m_dZoneEndTerm[i] );
	}
}

void ExtRanker_c::Reset ( const ISphQwordSetup & tSetup )
{
	if ( m_pRoot )
		m_pRoot->Reset ( tSetup );
	ARRAY_FOREACH ( i, m_dZones )
	{
		m_dZoneStartTerm[i]->Reset ( tSetup );
		m_dZoneEndTerm[i]->Reset ( tSetup );

		m_dZoneStart[i] = NULL;
		m_dZoneEnd[i] = NULL;
	}

	m_dZoneMax.Fill ( 0 );
	m_dZoneMin.Fill ( DOCID_MAX );
	m_hZoneInfo.Reset();
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

		// clean up zone hash
		if ( m_uMaxID!=DOCID_MAX )
		{
			ARRAY_FOREACH ( i, m_dZoneMin )
			{
				SphDocID_t uMinDocid = m_dZoneMin[i];
				if ( uMinDocid==DOCID_MAX )
					continue;

				ZoneKey_t tZoneStart;
				tZoneStart.m_iZone = i;
				tZoneStart.m_uDocid = uMinDocid;
				Verify ( m_hZoneInfo.IterateStart ( tZoneStart ) );
				uMinDocid = DOCID_MAX;
				do
				{
					ZoneKey_t tKey = m_hZoneInfo.IterateGetKey();
					if ( tKey.m_iZone!=i || tKey.m_uDocid>m_uMaxID )
					{
						uMinDocid = ( tKey.m_iZone==i ) ? tKey.m_uDocid : DOCID_MAX;
						break;
					}

					m_hZoneInfo.Delete ( tKey );
				} while ( m_hZoneInfo.IterateNext() );

				m_dZoneMin[i] = uMinDocid;
			}
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


SphZoneHit_e ExtRanker_c::IsInZone ( int iZone, const ExtHit_t * pHit )
{
	// quick route, we have current docid cached
	ZoneKey_t tKey; // OPTIMIZE? allow 2-component hash keys maybe?
	tKey.m_uDocid = pHit->m_uDocid;
	tKey.m_iZone = iZone;

	ZoneInfo_t * pZone = m_hZoneInfo ( tKey );
	if ( pZone )
	{
		// remove end markers that might mess up ordering
		Hitpos_t uPos = HITMAN::GetLCS ( pHit->m_uHitpos );
		int iSpan = FindSpan ( pZone->m_dStarts, uPos );
		return ( iSpan>=0 && uPos<=pZone->m_dEnds[iSpan] ) ? SPH_ZONE_FOUND : SPH_ZONE_NO_SPAN;
	}

	// is there any zone info for this document at all?
	if ( pHit->m_uDocid<=m_dZoneMax[iZone] )
		return SPH_ZONE_NO_DOCUMENT;

	// long route, read in zone info for all (!) the documents until next requested
	// that's because we might be queried out of order

	// current chunk
	const ExtDoc_t * pStart = m_dZoneStart[iZone];
	const ExtDoc_t * pEnd = m_dZoneEnd[iZone];

	// now keep caching spans until we see current id
	while ( pHit->m_uDocid > m_dZoneMax[iZone] )
	{
		// get more docs if needed
		if ( ( !pStart && m_dZoneMax[iZone]!=DOCID_MAX ) || pStart->m_uDocid==DOCID_MAX )
		{
			pStart = m_dZoneStartTerm[iZone]->GetDocsChunk ( NULL );
			if ( !pStart )
			{
				m_dZoneMax[iZone] = DOCID_MAX;
				return SPH_ZONE_NO_DOCUMENT;
			}
		}

		if ( ( !pEnd && m_dZoneMax[iZone]!=DOCID_MAX ) || pEnd->m_uDocid==DOCID_MAX )
		{
			pEnd = m_dZoneEndTerm[iZone]->GetDocsChunk ( NULL );
			if ( !pEnd )
			{
				m_dZoneMax[iZone] = DOCID_MAX;
				return SPH_ZONE_NO_DOCUMENT;
			}
		}

		assert ( pStart && pEnd );

		// skip zone starts past already cached stuff
		while ( pStart->m_uDocid<=m_dZoneMax[iZone] )
			pStart++;
		if ( pStart->m_uDocid==DOCID_MAX )
			continue;

		// skip zone ends until a match with start
		while ( pEnd->m_uDocid<pStart->m_uDocid )
			pEnd++;
		if ( pEnd->m_uDocid==DOCID_MAX )
			continue;

		// handle mismatching start/end ids
		// (this must never happen normally, but who knows what data we're fed)
		assert ( pStart->m_uDocid!=DOCID_MAX );
		assert ( pEnd->m_uDocid!=DOCID_MAX );
		assert ( pStart->m_uDocid<=pEnd->m_uDocid );

		if ( pStart->m_uDocid!=pEnd->m_uDocid )
		{
			while ( pStart->m_uDocid < pEnd->m_uDocid )
				pStart++;
			if ( pStart->m_uDocid==DOCID_MAX )
				continue;
		}

		// first matching uncached docid found!
		assert ( pStart->m_uDocid==pEnd->m_uDocid );
		assert ( pStart->m_uDocid > m_dZoneMax[iZone] );

		// but maybe we don't need docid this big just yet?
		if ( pStart->m_uDocid > pHit->m_uDocid )
		{
			// store current in-chunk positions
			m_dZoneStart[iZone] = pStart;
			m_dZoneEnd[iZone] = pEnd;

			// no zone info for all those precending documents (including requested one)
			m_dZoneMax[iZone] = pStart->m_uDocid-1;
			return SPH_ZONE_NO_DOCUMENT;
		}

		// cache all matching docs from current chunks below requested docid
		// (there might be more matching docs, but we are lazy and won't cache them upfront)
		ExtDoc_t dCache [ ExtNode_i::MAX_DOCS ];
		int iCache = 0;

		while ( pStart->m_uDocid<=pHit->m_uDocid )
		{
			// match
			if ( pStart->m_uDocid==pEnd->m_uDocid )
			{
				dCache[iCache++] = *pStart;
				pStart++;
				pEnd++;
				continue;
			}

			// mismatch!
			// this must not really happen, starts/ends must be in sync
			// but let's be graceful anyway, and just skip to next match
			if ( pStart->m_uDocid==DOCID_MAX || pEnd->m_uDocid==DOCID_MAX )
				break;

			while ( pStart->m_uDocid < pEnd->m_uDocid )
				pStart++;
			if ( pStart->m_uDocid==DOCID_MAX )
				break;

			while ( pEnd->m_uDocid < pStart->m_uDocid )
				pEnd++;
			if ( pEnd->m_uDocid==DOCID_MAX )
				break;
		}

		// should have found at least one id to cache
		assert ( iCache );
		assert ( iCache < ExtNode_i::MAX_DOCS );
		dCache[iCache].m_uDocid = DOCID_MAX;

		// do caching
		const ExtHit_t * pStartHits = m_dZoneStartTerm[iZone]->GetHitsChunk ( dCache, DOCID_MAX );
		const ExtHit_t * pEndHits = m_dZoneEndTerm[iZone]->GetHitsChunk ( dCache, DOCID_MAX );

		// loop documents one by one
		while ( pStartHits && pEndHits )
		{
			// load all hits for current document
			SphDocID_t uCur = pStartHits->m_uDocid;

			tKey.m_uDocid = uCur;
			m_hZoneInfo.Add ( ZoneInfo_t(), tKey );
			pZone = m_hZoneInfo ( tKey ); // OPTIMIZE? return pointer from Add()?

			// load all the start hits for it
			while ( pStartHits )
			{
				while ( pStartHits->m_uDocid==uCur )
				{
					pZone->m_dStarts.Add ( pStartHits->m_uHitpos );
					pStartHits++;
				}
				if ( pStartHits->m_uDocid!=DOCID_MAX )
					break;
				pStartHits = m_dZoneStartTerm[iZone]->GetHitsChunk ( dCache, DOCID_MAX );
			}

			// load all the end hits for it
			assert ( pEndHits->m_uDocid==uCur );
			while ( pEndHits )
			{
				while ( pEndHits->m_uDocid==uCur )
				{
					pZone->m_dEnds.Add ( pEndHits->m_uHitpos );
					pEndHits++;
				}
				if ( pEndHits->m_uDocid!=DOCID_MAX )
					break;
				pEndHits = m_dZoneEndTerm[iZone]->GetHitsChunk ( dCache, DOCID_MAX );
			}

			// data sanity checks
			assert ( pZone->m_dStarts.GetLength()==pZone->m_dEnds.GetLength() );

			// update cache status
			m_dZoneMax[iZone] = uCur;
			m_dZoneMin[iZone] = Min ( m_dZoneMin[iZone], uCur );
		}
	}

	// store current in-chunk positions
	m_dZoneStart[iZone] = pStart;
	m_dZoneEnd[iZone] = pEnd;

	// cached a bunch of spans, try our check again
	tKey.m_uDocid = pHit->m_uDocid;
	pZone = m_hZoneInfo ( tKey );
	if ( pZone )
	{
		// remove end markers that might mess up ordering
		Hitpos_t uPos = HITMAN::GetLCS ( pHit->m_uHitpos );
		int iSpan = FindSpan ( pZone->m_dStarts, uPos );
		return ( iSpan>=0 && uPos<=pZone->m_dEnds[iSpan] ) ? SPH_ZONE_FOUND : SPH_ZONE_NO_SPAN;
	}

	return SPH_ZONE_NO_DOCUMENT;
}

//////////////////////////////////////////////////////////////////////////

template < bool USE_BM25 >
int ExtRanker_WeightSum_c<USE_BM25>::GetMatches ()
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
		for ( int i=0; i<m_iWeights; i++ )
			if ( pDoc->m_dFields.Test(i) )
				uRank += m_pWeights[i];

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

int ExtRanker_None_c::GetMatches ()
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
int ExtRanker_T<STATE>::GetMatches ()
{
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
			m_tState.Update ( pHlist++ );

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
			m_dMatches[iMatches].m_iWeight = m_tState.Finalize ( m_dMatches[iMatches] );
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

	bool Init ( int iFields, const int * pWeights, ExtRanker_c *, CSphString & )
	{
		memset ( m_uLCS, 0, sizeof(m_uLCS) );
		m_uCurLCS = 0;
		m_iExpDelta = -INT_MAX;
		m_iFields = iFields;
		m_pWeights = pWeights;
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		int iDelta = HITMAN::GetLCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
		if ( iDelta==m_iExpDelta )
			m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
		else
			m_uCurLCS = BYTE(pHlist->m_uWeight);

		DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
		if ( m_uCurLCS>m_uLCS[uField] )
			m_uLCS[uField] = m_uCurLCS;

		m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1; // !COMMIT why spanlen??
	}

	DWORD Finalize ( const CSphMatch & tMatch )
	{
		m_uCurLCS = 0;
		m_iExpDelta = -1;

		DWORD uRank = 0;
		for ( int i=0; i<m_iFields; i++ )
		{
			uRank += m_uLCS[i]*m_pWeights[i];
			m_uLCS[i] = 0;
		}

		return USE_BM25 ? tMatch.m_iWeight + uRank*SPH_BM25_SCALE : uRank;
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

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & )
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
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		// upd LCS
		DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
		int iDelta = HITMAN::GetLCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
		if ( iDelta==m_iExpDelta && HITMAN::GetLCS ( pHlist->m_uHitpos )>=m_uMinExpPos )
		{
			m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
			if ( HITMAN::IsEnd ( pHlist->m_uHitpos ) && (int)pHlist->m_uQuerypos==m_iMaxQuerypos && HITMAN::GetPos ( pHlist->m_uHitpos )==m_iMaxQuerypos )
				m_uExactHit |= ( 1UL << HITMAN::GetField ( pHlist->m_uHitpos ) );
		} else
		{
			m_uCurLCS = BYTE(pHlist->m_uWeight);
			if ( HITMAN::GetPos ( pHlist->m_uHitpos )==1 )
			{
				m_uHeadHit |= ( 1UL << HITMAN::GetField ( pHlist->m_uHitpos ) );
				if ( HITMAN::IsEnd ( pHlist->m_uHitpos ) && m_iMaxQuerypos==1 )
					m_uExactHit |= ( 1UL << HITMAN::GetField ( pHlist->m_uHitpos ) );
			}
		}

		if ( m_uCurLCS>m_uLCS[uField] )
			m_uLCS[uField] = m_uCurLCS;

		m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1;
		m_uMinExpPos = HITMAN::GetLCS ( pHlist->m_uHitpos ) + 1;
	}

	DWORD Finalize ( const CSphMatch & tMatch )
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

		return tMatch.m_iWeight + uRank*SPH_BM25_SCALE;
	}
};


template < bool USE_BM25 >
struct RankerState_ProximityPayload_fn : public RankerState_Proximity_fn<USE_BM25>
{
	DWORD m_uPayloadRank;
	DWORD m_uPayloadMask;

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError )
	{
		RankerState_Proximity_fn<USE_BM25>::Init ( iFields, pWeights, pRanker, sError );
		m_uPayloadRank = 0;
		m_uPayloadMask = pRanker->m_uPayloadMask;
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
		if ( ( 1<<uField ) & m_uPayloadMask )
			this->m_uPayloadRank += HITMAN::GetPos ( pHlist->m_uHitpos ) * this->m_pWeights[uField];
		else
			RankerState_Proximity_fn<USE_BM25>::Update ( pHlist );
	}

	DWORD Finalize ( const CSphMatch & tMatch )
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
		return USE_BM25 ? tMatch.m_iWeight + uRank*SPH_BM25_SCALE : uRank;
	}
};

//////////////////////////////////////////////////////////////////////////

struct RankerState_MatchAny_fn : public RankerState_Proximity_fn<false>
{
	int m_iPhraseK;
	BYTE m_uMatchMask[SPH_MAX_FIELDS];

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError )
	{
		RankerState_Proximity_fn<false>::Init ( iFields, pWeights, pRanker, sError );
		m_iPhraseK = 0;
		for ( int i=0; i<iFields; i++ )
			m_iPhraseK += pWeights[i] * pRanker->m_iQwords;
		memset ( m_uMatchMask, 0, sizeof(m_uMatchMask) );
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		RankerState_Proximity_fn<false>::Update ( pHlist );
		m_uMatchMask [ HITMAN::GetField ( pHlist->m_uHitpos ) ] |= ( 1<<(pHlist->m_uQuerypos-1) );
	}

	DWORD Finalize ( const CSphMatch & )
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

	bool Init ( int iFields, const int * pWeights, ExtRanker_c *, CSphString & )
	{
		m_uRank = 0;
		m_iFields = iFields;
		m_pWeights = pWeights;
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		m_uRank += m_pWeights [ HITMAN::GetField ( pHlist->m_uHitpos ) ];
	}

	DWORD Finalize ( const CSphMatch & )
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

	bool Init ( int, const int *, ExtRanker_c *, CSphString & )
	{
		m_uRank = 0;
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		m_uRank |= 1UL << HITMAN::GetField ( pHlist->m_uHitpos );
	}

	DWORD Finalize ( const CSphMatch & )
	{
		DWORD uRes = m_uRank;
		m_uRank = 0;
		return uRes;
	}
};

//////////////////////////////////////////////////////////////////////////
// EXPRESSION RANKER
//////////////////////////////////////////////////////////////////////////

/// ranker state that computes weight dynamically based on user supplied expression (formula)
struct RankerState_Expr_fn
{
public:
	BYTE				m_uLCS[SPH_MAX_FIELDS];
	BYTE				m_uMatchMask[SPH_MAX_FIELDS];
	BYTE				m_uCurLCS;
	int					m_iExpDelta;
	int					m_iFields;
	const int *			m_pWeights;
	DWORD				m_uDocBM25;
	DWORD				m_uMatchedFields;
	int					m_iCurrentField;
	DWORD				m_uHitCount[SPH_MAX_FIELDS];
	DWORD				m_uWordCount[SPH_MAX_FIELDS];
	CSphVector<float>	m_dIDF;
	float				m_dTFIDF[SPH_MAX_FIELDS];
	int					m_iMinHitPos[SPH_MAX_FIELDS];
	int					m_iMinBestSpanPos[SPH_MAX_FIELDS];
	int					m_iMaxQuerypos;
	DWORD				m_uExactHit;

	const char *		m_sExpr;
	ISphExpr *			m_pExpr;
	ESphAttr			m_eExprType;
	const CSphSchema *	m_pSchema;

	int					m_iMaxLCS;

public:
						RankerState_Expr_fn ();
						~RankerState_Expr_fn ();

	bool				Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError );
	void				Update ( const ExtHit_t * pHlist );
	DWORD				Finalize ( const CSphMatch & tMatch );
};


/// extra expression ranker node types
enum ExprRankerNode_e
{
	// field level factors
	XRANK_LCS,
	XRANK_USER_WEIGHT,
	XRANK_HIT_COUNT,
	XRANK_WORD_COUNT,
	XRANK_TF_IDF,
	XRANK_MIN_HIT_POS,
	XRANK_MIN_BEST_SPAN_POS,
	XRANK_EXACT_HIT,

	// document level factors
	XRANK_BM25,
	XRANK_MAX_LCS,
	XRANK_FIELD_MASK,

	// field aggregation functions
	XRANK_SUM
};


/// generic field factor
template < typename T >
struct Expr_FieldFactor_c : public ISphExpr
{
	const int *		m_pIndex;
	const T *		m_pData;

	Expr_FieldFactor_c ( const int * pIndex, const T * pData )
		: m_pIndex ( pIndex )
		, m_pData ( pData )
	{}

	float Eval ( const CSphMatch & ) const
	{
		return (float) m_pData [ *m_pIndex ];
	}

	int IntEval ( const CSphMatch & ) const
	{
		return (int) m_pData [ *m_pIndex ];
	}
};


/// bitmask field factor specialization
template<>
struct Expr_FieldFactor_c<bool> : public ISphExpr
{
	const int *		m_pIndex;
	const DWORD *	m_pData;

	Expr_FieldFactor_c ( const int * pIndex, const DWORD * pData )
		: m_pIndex ( pIndex )
		, m_pData ( pData )
	{}

	float Eval ( const CSphMatch & ) const
	{
		return (float)( (*m_pData) >> (*m_pIndex) );
	}

	int IntEval ( const CSphMatch & ) const
	{
		return (int)( (*m_pData) >> (*m_pIndex) );
	}
};



/// generic per-document int factor
struct Expr_IntPtr_c : public ISphExpr
{
	DWORD * m_pVal;

	explicit Expr_IntPtr_c ( DWORD * pVal )
		: m_pVal ( pVal )
	{}

	float Eval ( const CSphMatch & ) const
	{
		return (float)*m_pVal;
	}

	int IntEval ( const CSphMatch & ) const
	{
		return (int)*m_pVal;
	}
};


/// function that sums sub-expressions over matched fields
struct Expr_Sum_c : public ISphExpr
{
	RankerState_Expr_fn *	m_pState;
	ISphExpr *				m_pArg;

	Expr_Sum_c ( RankerState_Expr_fn * pState, ISphExpr * pArg )
		: m_pState ( pState )
		, m_pArg ( pArg )
	{}

	float Eval ( const CSphMatch & tMatch ) const
	{
		m_pState->m_iCurrentField = 0;
		float fRes = 0;
		DWORD uMask = m_pState->m_uMatchedFields;
		while ( uMask )
		{
			if ( uMask & 1 )
				fRes += m_pArg->Eval ( tMatch );
			uMask >>= 1;
			m_pState->m_iCurrentField++;
		}
		return fRes;
	}

	int IntEval ( const CSphMatch & tMatch ) const
	{
		m_pState->m_iCurrentField = 0;
		int iRes = 0;
		DWORD uMask = m_pState->m_uMatchedFields;
		while ( uMask )
		{
			if ( uMask & 1 )
				iRes += m_pArg->IntEval ( tMatch );
			uMask >>= 1;
			m_pState->m_iCurrentField++;
		}
		return iRes;
	}
};


// FIXME! cut/pasted from sphinxexpr; remove dupe
struct Expr_GetIntConst_c : public ISphExpr
{
	int m_iValue;
	explicit Expr_GetIntConst_c ( int iValue ) : m_iValue ( iValue ) {}
	virtual float Eval ( const CSphMatch & ) const { return (float) m_iValue; } // no assert() here cause generic float Eval() needs to work even on int-evaluator tree
	virtual int IntEval ( const CSphMatch & ) const { return m_iValue; }
	virtual int64_t Int64Eval ( const CSphMatch & ) const { return m_iValue; }
};


/// hook that exposes field-level factors, document-level factors, and matched field SUM() function to generic expressions
class ExprRankerHook_c : public ISphExprHook
{
public:
	RankerState_Expr_fn *	m_pState;
	const char *			m_sCheckError;
	bool					m_bCheckInFieldAggr;

public:
	explicit ExprRankerHook_c ( RankerState_Expr_fn * pState )
		: m_pState ( pState )
		, m_sCheckError ( NULL )
		, m_bCheckInFieldAggr ( false )
	{}

	int IsKnownIdent ( const char * sIdent )
	{
		// OPTIMIZE? hash this some nice long winter night?
		if ( !strcasecmp ( sIdent, "lcs" ) )
			return XRANK_LCS;
		if ( !strcasecmp ( sIdent, "user_weight" ) )
			return XRANK_USER_WEIGHT;
		if ( !strcasecmp ( sIdent, "hit_count" ) )
			return XRANK_HIT_COUNT;
		if ( !strcasecmp ( sIdent, "word_count" ) )
			return XRANK_WORD_COUNT;
		if ( !strcasecmp ( sIdent, "tf_idf" ) )
			return XRANK_TF_IDF;
		if ( !strcasecmp ( sIdent, "min_hit_pos" ) )
			return XRANK_MIN_HIT_POS;
		if ( !strcasecmp ( sIdent, "min_best_span_pos" ) )
			return XRANK_MIN_BEST_SPAN_POS;
		if ( !strcasecmp ( sIdent, "exact_hit" ) )
			return XRANK_EXACT_HIT;

		if ( !strcasecmp ( sIdent, "bm25" ) )
			return XRANK_BM25;
		if ( !strcasecmp ( sIdent, "max_lcs" ) )
			return XRANK_MAX_LCS;
		if ( !strcasecmp ( sIdent, "field_mask" ) )
			return XRANK_FIELD_MASK;
		return -1;
	}

	int IsKnownFunc ( const char * sFunc )
	{
		if ( !strcasecmp ( sFunc, "sum" ) )
			return XRANK_SUM;
		return -1;
	}

	ISphExpr * CreateNode ( int iID, ISphExpr * pLeft )
	{
		int * pCF = &m_pState->m_iCurrentField; // just a shortcut
		switch ( iID )
		{
			case XRANK_LCS:					return new Expr_FieldFactor_c<BYTE> ( pCF, m_pState->m_uLCS );
			case XRANK_USER_WEIGHT:			return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_pWeights );
			case XRANK_HIT_COUNT:			return new Expr_FieldFactor_c<DWORD> ( pCF, m_pState->m_uHitCount );
			case XRANK_WORD_COUNT:			return new Expr_FieldFactor_c<DWORD> ( pCF, m_pState->m_uWordCount );
			case XRANK_TF_IDF:				return new Expr_FieldFactor_c<float> ( pCF, m_pState->m_dTFIDF );
			case XRANK_MIN_HIT_POS:			return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_iMinHitPos );
			case XRANK_MIN_BEST_SPAN_POS:	return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_iMinBestSpanPos );
			case XRANK_EXACT_HIT:			return new Expr_FieldFactor_c<bool> ( pCF, &m_pState->m_uExactHit );
			case XRANK_BM25:				return new Expr_IntPtr_c ( &m_pState->m_uDocBM25 );
			case XRANK_MAX_LCS:				return new Expr_GetIntConst_c ( m_pState->m_iMaxLCS );
			case XRANK_FIELD_MASK:			return new Expr_IntPtr_c ( &m_pState->m_uMatchedFields );
			case XRANK_SUM:					return new Expr_Sum_c ( m_pState, pLeft );
			default:						return NULL;
		}
	}

	ESphAttr GetIdentType ( int iID )
	{
		switch ( iID )
		{
			case XRANK_LCS:
			case XRANK_USER_WEIGHT:
			case XRANK_HIT_COUNT:
			case XRANK_WORD_COUNT:
			case XRANK_MIN_HIT_POS:
			case XRANK_MIN_BEST_SPAN_POS:
			case XRANK_EXACT_HIT:
			case XRANK_BM25:
			case XRANK_MAX_LCS:
			case XRANK_FIELD_MASK:
				return SPH_ATTR_INTEGER;
			case XRANK_TF_IDF:
				return SPH_ATTR_FLOAT;
			default:
				assert ( 0 );
				return SPH_ATTR_INTEGER;
		}
	}

	ESphAttr GetFuncType ( int iID, ESphAttr eArgType )
	{
		switch ( iID )
		{
			case XRANK_SUM:		return eArgType;
			default:			return SPH_ATTR_INTEGER;
		}
	}

	int GetExpectedArgc ( int iID )
	{
		switch ( iID )
		{
			case XRANK_SUM:		return 1;
			default:			return -1;
		}
	}

	const char * GetFuncName ( int iID )
	{
		switch ( iID )
		{
			case XRANK_SUM:		return "SUM";
			default:			return "???";
		}
	}

	void CheckEnter ( int iID )
	{
		if ( !m_sCheckError )
			switch ( iID )
		{
			case XRANK_LCS:
			case XRANK_USER_WEIGHT:
			case XRANK_HIT_COUNT:
			case XRANK_WORD_COUNT:
			case XRANK_TF_IDF:
			case XRANK_MIN_HIT_POS:
			case XRANK_MIN_BEST_SPAN_POS:
			case XRANK_EXACT_HIT:
				if ( !m_bCheckInFieldAggr )
					m_sCheckError = "field factors must only occur withing field aggregates in ranking expression";
				break;

			case XRANK_SUM:
				if ( m_bCheckInFieldAggr )
					m_sCheckError = "field aggregates can not be nested in ranking expression";
				else
					m_bCheckInFieldAggr = true;
				break;

			default:
				assert ( iID>=0 );
				return;
		}
	}

	void CheckExit ( int iID )
	{
		if ( !m_sCheckError && iID==XRANK_SUM )
		{
			assert ( m_bCheckInFieldAggr );
			m_bCheckInFieldAggr = false;
		}
	}
};


/// ctor
RankerState_Expr_fn::RankerState_Expr_fn ()
	: m_pWeights ( NULL )
	, m_sExpr ( NULL )
	, m_pExpr ( NULL )
	, m_iMaxLCS ( 0 )
{}


/// dtor
RankerState_Expr_fn::~RankerState_Expr_fn ()
{
	SafeRelease ( m_pExpr );
}


/// initialize ranker state
bool RankerState_Expr_fn::Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError )
{
	// cleanup factors
	memset ( m_uLCS, 0, sizeof(m_uLCS) );
	memset ( m_uMatchMask, 0, sizeof(m_uMatchMask) );
	m_uCurLCS = 0;
	m_iExpDelta = -INT_MAX;
	m_iFields = iFields;
	m_pWeights = pWeights;
	m_uDocBM25 = 0;
	m_uMatchedFields = 0;
	m_iCurrentField = 0;
	memset ( m_uHitCount, 0, sizeof(m_uHitCount) );
	memset ( m_uWordCount, 0, sizeof(m_uWordCount) );
	memset ( m_dTFIDF, 0, sizeof(m_dTFIDF) );
	memset ( m_iMinHitPos, 0, sizeof(m_iMinHitPos) );
	memset ( m_iMinBestSpanPos, 0, sizeof(m_iMinBestSpanPos) );
	m_iMaxQuerypos = pRanker->m_iMaxQuerypos;
	m_uExactHit = 0;

	// compute query level constants (max_lcs, for matchany ranker emulation)
	m_iMaxLCS = 0;
	for ( int i=0; i<iFields; i++ )
		m_iMaxLCS += pWeights[i] * pRanker->m_iQwords;

	// parse expression
	bool bUsesWeight;
	ExprRankerHook_c tHook ( this );
	m_pExpr = sphExprParse ( m_sExpr, *m_pSchema, &m_eExprType, &bUsesWeight, sError, NULL, &tHook );
	if ( !m_pExpr )
		return false;
	if ( m_eExprType!=SPH_ATTR_INTEGER && m_eExprType!=SPH_ATTR_FLOAT )
	{
		sError = "ranking expression must evaluate to integer or float";
		return false;
	}
	if ( bUsesWeight )
	{
		sError = "ranking expression must not refer to WEIGHT()";
		return false;
	}
	if ( tHook.m_sCheckError )
	{
		sError = tHook.m_sCheckError;
		return false;
	}

	// all seems ok
	return true;
}


/// process next hit, update factors
void RankerState_Expr_fn::Update ( const ExtHit_t * pHlist )
{
	const DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );

	// update LCS
	int iDelta = HITMAN::GetLCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
	if ( iDelta==m_iExpDelta )
	{
		m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
		if ( HITMAN::IsEnd ( pHlist->m_uHitpos ) && (int)pHlist->m_uQuerypos==m_iMaxQuerypos && HITMAN::GetPos ( pHlist->m_uHitpos )==m_iMaxQuerypos )
			m_uExactHit |= ( 1UL << uField );
	} else
	{
		m_uCurLCS = BYTE(pHlist->m_uWeight);
		if ( HITMAN::GetPos ( pHlist->m_uHitpos )==1 && HITMAN::IsEnd ( pHlist->m_uHitpos ) && m_iMaxQuerypos==1 )
			m_uExactHit |= ( 1UL << uField );
	}
	if ( m_uCurLCS>m_uLCS[uField] )
	{
		m_uLCS[uField] = m_uCurLCS;
		m_iMinBestSpanPos[uField] = HITMAN::GetPos ( pHlist->m_uHitpos ) - m_uCurLCS + 1;
	}
	m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1;

	// update other stuff
	m_uMatchMask[uField] |= ( 1<<(pHlist->m_uQuerypos-1) );
	m_uMatchedFields |= ( 1UL<<uField );
	m_uHitCount[uField]++;
	m_uWordCount[uField] |= ( 1<<pHlist->m_uQuerypos );
	m_dTFIDF[uField] += m_dIDF [ pHlist->m_uQuerypos ];
	if ( !m_iMinHitPos[uField] )
		m_iMinHitPos[uField] = HITMAN::GetPos ( pHlist->m_uHitpos );
}


/// finish document processing, compute weight from factors
DWORD RankerState_Expr_fn::Finalize ( const CSphMatch & tMatch )
{
	// finishing touches
	m_uDocBM25 = tMatch.m_iWeight;
	for ( int i=0; i<m_iFields; i++ )
		m_uWordCount[i] = sphBitCount ( m_uWordCount[i] );

	// compute expression
	DWORD uRes = ( m_eExprType==SPH_ATTR_INTEGER )
		? m_pExpr->IntEval ( tMatch )
		: (DWORD)m_pExpr->Eval ( tMatch );

	// cleanup
	// OPTIMIZE? quick full wipe? (using dwords/sse/whatever)
	m_uCurLCS = 0;
	m_iExpDelta = -1;
	for ( int i=0; i<m_iFields; i++ )
	{
		m_uMatchMask[i] = 0;
		m_uLCS[i] = 0;
		m_uHitCount[i] = 0;
		m_uWordCount[i] = 0;
		m_dTFIDF[i] = 0;
		m_iMinHitPos[i] = 0;
		m_iMinBestSpanPos[i] = 0;
	}
	m_uMatchedFields = 0;
	m_uExactHit = 0;

	// done
	return uRes;
}


/// expression ranker
class ExtRanker_Expr_c : public ExtRanker_T<RankerState_Expr_fn>
{
public:
	ExtRanker_Expr_c ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup, const char * sExpr, const CSphSchema & tSchema )
		: ExtRanker_T<RankerState_Expr_fn> ( tXQ, tSetup )
	{
		// tricky bit, we stash the pointer to expr here, but it will be parsed
		// somewhat later during InitState() call, when IDFs etc are computed
		m_tState.m_sExpr = sExpr;
		m_tState.m_pSchema = &tSchema;
	}

	void SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
	{
		ExtRanker_T<RankerState_Expr_fn>::SetQwordsIDF ( hQwords );

		m_tState.m_dIDF.Resize ( m_iMaxQuerypos+1 );
		ARRAY_FOREACH ( i, m_tState.m_dIDF )
			m_tState.m_dIDF[i] = 0.0f;
		hQwords.IterateStart();
		while ( hQwords.IterateNext() )
			m_tState.m_dIDF [ hQwords.IterateGet().m_iQueryPos ] = hQwords.IterateGet().m_fIDF;
	}
};

//////////////////////////////////////////////////////////////////////////
// RANKER FACTORY
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


struct ExtQwordOrderbyQueryPos_t
{
	bool IsLess ( const ExtQword_t * pA, const ExtQword_t * pB ) const
	{
		return pA->m_iQueryPos < pB->m_iQueryPos;
	}
};


ISphRanker * sphCreateRanker ( const XQQuery_t & tXQ, const CSphQuery * pQuery, CSphQueryResult * pResult, const ISphQwordSetup & tTermSetup, const CSphQueryContext & tCtx )
{
	// shortcut
	const CSphIndex * pIndex = tTermSetup.m_pIndex;

	// check the keywords
	CheckExtendedQuery ( tXQ.m_pRoot, pResult, pIndex->GetSettings(), pIndex->m_bEnableStar );

	// fill payload mask
	DWORD uPayloadMask = 0;
	ARRAY_FOREACH ( i, pIndex->GetMatchSchema().m_dFields )
		uPayloadMask |= pIndex->GetMatchSchema().m_dFields[i].m_bPayload << i;

	bool bSingleWord = tXQ.m_pRoot->m_dChildren.GetLength()==0 && tXQ.m_pRoot->m_dWords.GetLength()==1;

	// setup eval-tree
	ExtRanker_c * pRanker = NULL;
	switch ( pQuery->m_eRanker )
	{
		case SPH_RANK_PROXIMITY_BM25:
			if ( uPayloadMask )
				pRanker = new ExtRanker_T < RankerState_ProximityPayload_fn<true> > ( tXQ, tTermSetup );
			else if ( bSingleWord )
				pRanker = new ExtRanker_WeightSum_c<WITH_BM25> ( tXQ, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true> > ( tXQ, tTermSetup );
			break;
		case SPH_RANK_BM25:				pRanker = new ExtRanker_WeightSum_c<WITH_BM25> ( tXQ, tTermSetup ); break;
		case SPH_RANK_NONE:				pRanker = new ExtRanker_None_c ( tXQ, tTermSetup ); break;
		case SPH_RANK_WORDCOUNT:		pRanker = new ExtRanker_T < RankerState_Wordcount_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_PROXIMITY:
			if ( bSingleWord )
				pRanker = new ExtRanker_WeightSum_c<> ( tXQ, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<false> > ( tXQ, tTermSetup );
			break;
		case SPH_RANK_MATCHANY:			pRanker = new ExtRanker_T < RankerState_MatchAny_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_FIELDMASK:		pRanker = new ExtRanker_T < RankerState_Fieldmask_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_SPH04:			pRanker = new ExtRanker_T < RankerState_ProximityBM25Exact_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_EXPR:				pRanker = new ExtRanker_Expr_c ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() ); break;
		default:
			pResult->m_sWarning.SetSprintf ( "unknown ranking mode %d; using default", (int)pQuery->m_eRanker );
			pRanker = new ExtRanker_T < RankerState_Proximity_fn<true> > ( tXQ, tTermSetup );
			break;
	}
	assert ( pRanker );
	pRanker->m_uPayloadMask = uPayloadMask;

	// setup IDFs
	ExtQwordsHash_t hQwords;
	pRanker->GetQwords ( hQwords );

	const int iQwords = hQwords.GetLength ();
	const CSphSourceStats & tSourceStats = pIndex->GetStats();

	CSphVector<const ExtQword_t *> dWords;
	dWords.Reserve ( hQwords.GetLength() );

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
		dWords.Add ( &tWord );
	}

	dWords.Sort ( ExtQwordOrderbyQueryPos_t() );
	ARRAY_FOREACH ( i, dWords )
	{
		const ExtQword_t * pWord = dWords[i];
		pResult->AddStat ( pWord->m_sDictWord, pWord->m_iDocs, pWord->m_iHits, pWord->m_bExpanded );
	}

	pRanker->SetQwordsIDF ( hQwords );
	if ( !pRanker->InitState ( tCtx, pResult->m_sError ) )
		SafeDelete ( pRanker );
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
			tMark.m_uPosition = HITMAN::GetPos ( pHits->m_uHitpos );
			tMark.m_uSpan = pHits->m_uMatchlen;

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
		m_dHits[iHit].m_uQuerypos = (WORD)( m_dHits[iHit].m_uQuerypos + m_iAtomPos - m_pNode->m_iAtomPos );
		iHit++;
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
