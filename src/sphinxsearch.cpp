//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
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
#include "sphinxplugin.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////////

/// query debugging printouts
#define QDEBUG 0

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

#define SPH_TREE_DUMP			0

#define SPH_BM25_K1				1.2f
#define SPH_BM25_SCALE			1000


struct QwordsHash_fn
{
	static inline int Hash ( const CSphString & sKey )
	{
		return sphCRC32 ( sKey.cstr() );
	}
};

void ISphQword::CollectHitMask()
{
	if ( m_bAllFieldsKnown )
		return;
	SeekHitlist ( m_iHitlistPos );
	for ( Hitpos_t uHit = GetNextHit(); uHit!=EMPTY_HIT; uHit = GetNextHit() )
		m_dQwordFields.Set ( HITMAN::GetField ( uHit ) );
	m_bAllFieldsKnown = true;
}


/// match in the stream
struct ExtDoc_t
{
	SphDocID_t		m_uDocid;
	CSphRowitem *	m_pDocinfo;			///< for inline storage only
	SphOffset_t		m_uHitlistOffset;
	DWORD			m_uDocFields;
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
	float		m_fBoost;		///< IDF multiplier
	int			m_iQueryPos;	///< position in the query
	bool		m_bExpanded;	///< added by prefix expansion
	bool		m_bExcluded;	///< excluded by the query (eg. bb in (aa AND NOT bb))
};


/// query words set
typedef CSphOrderedHash < ExtQword_t, CSphString, QwordsHash_fn, 256 > ExtQwordsHash_t;

struct ZoneHits_t
{
	CSphVector<Hitpos_t>	m_dStarts;
	CSphVector<Hitpos_t>	m_dEnds;
};

/// per-document zone information (span start/end positions)
struct ZoneInfo_t
{
	SphDocID_t		m_uDocid;
	ZoneHits_t *	m_pHits;
};

// FindSpan vector operators
static bool operator < ( const ZoneInfo_t & tZone, SphDocID_t uDocid )
{
	return tZone.m_uDocid<uDocid;
}

static bool operator == ( const ZoneInfo_t & tZone, SphDocID_t uDocid )
{
	return tZone.m_uDocid==uDocid;
}

static bool operator < ( SphDocID_t uDocid, const ZoneInfo_t & tZone )
{
	return uDocid<tZone.m_uDocid;
}

static void PrintDocsChunk ( int QDEBUGARG(iCount), int QDEBUGARG(iAtomPos), const ExtDoc_t * QDEBUGARG(pDocs), const char * QDEBUGARG(sNode), void * QDEBUGARG(pNode) )
{
#if QDEBUG
	CSphStringBuilder tRes;
	tRes.Appendf ( "node %s 0x%x:%p getdocs (%d) = [", sNode ? sNode : "???", iAtomPos, pNode, iCount );
	for ( int i=0; i<iCount; i++ )
		tRes.Appendf ( i ? ", 0x%x" : "0x%x", DWORD ( pDocs[i].m_uDocid ) );
	tRes.Appendf ( "]" );
	printf ( "%s", tRes.cstr() );
#endif
}

static void PrintHitsChunk ( int QDEBUGARG(iCount), int QDEBUGARG(iAtomPos), const ExtHit_t * QDEBUGARG(pHits), const char * QDEBUGARG(sNode), void * QDEBUGARG(pNode) )
{
#if QDEBUG
	CSphStringBuilder tRes;
	tRes.Appendf ( "node %s 0x%x:%p gethits (%d) = [", sNode ? sNode : "???", iAtomPos, pNode, iCount );
	for ( int i=0; i<iCount; i++ )
		tRes.Appendf ( i ? ", 0x%x:0x%x" : "0x%x:0x%x", DWORD ( pHits[i].m_uDocid ), DWORD ( pHits[i].m_uHitpos ) );
	tRes.Appendf ( "]" );
	printf ( "%s", tRes.cstr() );
#endif
}


struct TermPos_t
{
	WORD m_uQueryPos;
	WORD m_uAtomPos;
};


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
	virtual void				HintDocid ( SphDocID_t uMinID ) = 0;
	virtual const ExtDoc_t *	GetDocsChunk() = 0;
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs ) = 0;

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords ) = 0;
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) = 0;
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const = 0;
	virtual bool				GotHitless () = 0;
	virtual int					GetDocsCount () { return INT_MAX; }
	virtual int					GetHitsCount () { return 0; }
	virtual uint64_t			GetWordID () const = 0;			///< for now, only used for duplicate keyword checks in quorum operator

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

	// return specific extra data may be associated with the node
	// intended to be used a bit similar to QueryInterface() in COM technology
	// but simpler due to enum instead of 128-bit GUID, and no ref. counting
	inline bool GetExtraData ( ExtraData_e eNode, void** ppData )
	{
		return ExtraDataImpl ( eNode, ppData );
	}
private:
	virtual bool ExtraDataImpl ( ExtraData_e, void** )
	{
		return false;
	}

public:
	static const int			MAX_DOCS = 512;
	static const int			MAX_HITS = 512;

	int							m_iAtomPos;		///< we now need it on this level for tricks like expanded keywords within phrases

protected:
	ExtDoc_t					m_dDocs[MAX_DOCS];
	ExtHit_t					m_dHits[MAX_HITS];

public:
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
	inline const ExtDoc_t * ReturnDocsChunk ( int iCount, const char * sNode )
	{
		assert ( iCount>=0 && iCount<MAX_DOCS );
		m_dDocs[iCount].m_uDocid = DOCID_MAX;

		PrintDocsChunk ( iCount, m_iAtomPos, m_dDocs, sNode, this );
		return iCount ? m_dDocs : NULL;
	}

	inline const ExtHit_t * ReturnHitsChunk ( int iCount, const char * sNode )
	{
		assert ( iCount>=0 && iCount<MAX_HITS );
		m_dHits[iCount].m_uDocid = DOCID_MAX;

#ifndef NDEBUG
		for ( int i=1; i<iCount; i++ )
		{
			assert ( m_dHits[i-1].m_uDocid!=m_dHits[i].m_uDocid ||
					( m_dHits[i-1].m_uHitpos<m_dHits[i].m_uHitpos || ( m_dHits[i-1].m_uHitpos==m_dHits[i].m_uHitpos && m_dHits[i-1].m_uQuerypos<=m_dHits[i].m_uQuerypos ) ) );
		}
#endif

		PrintHitsChunk ( iCount, m_iAtomPos, m_dHits, sNode, this );
		return iCount ? m_dHits : NULL;
	}
};

//////////////////////////////////////////////////////////////////////////

/// single keyword streamer
class ExtTerm_c : public ExtNode_i, ISphNoncopyable
{
public:
	ExtTerm_c ( ISphQword * pQword, const FieldMask_t& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted );
	ExtTerm_c ( ISphQword * pQword, const ISphQwordSetup & tSetup );
	ExtTerm_c () {} ///< to be used in pair with Init()
	~ExtTerm_c ()
	{
		SafeDelete ( m_pQword );
	}

	void						Init ( ISphQword * pQword, const FieldMask_t& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const;
	virtual bool				GotHitless () { return false; }
	virtual int					GetDocsCount () { return m_pQword->m_iDocs; }
	virtual int					GetHitsCount () { return m_pQword->m_iHits; }
	virtual uint64_t			GetWordID () const
	{
		if ( m_pQword->m_uWordID )
			return m_pQword->m_uWordID;
		else
			return sphFNV64 ( m_pQword->m_sDictWord.cstr() );
	}

	virtual void HintDocid ( SphDocID_t uMinID )
	{
		m_pQword->HintDocid ( uMinID );
		if ( m_pStats )
			m_pStats->m_iSkips++;
		if ( m_pNanoBudget )
			*m_pNanoBudget -= g_iPredictorCostSkip;
	}

	virtual void DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "ExtTerm: %s at: %d ", m_pQword->m_sWord.cstr(), m_pQword->m_iAtomPos );
		if ( m_dQueriedFields.TestAll(true) )
		{
			printf ( "(all)\n" );
		} else
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

protected:
	ISphQword *					m_pQword;
	FieldMask_t				m_dQueriedFields;	///< accepted fields mask
	bool						m_bHasWideFields;	///< whether fields mask for this term refer to fields 32+
	float						m_fIDF;				///< IDF for this term (might be 0.0f for non-1st occurences in query)
	int64_t						m_iMaxTimer;		///< work until this timestamp
	CSphString *				m_pWarning;
	bool						m_bNotWeighted;
	CSphQueryStats *			m_pStats;
	int64_t *					m_pNanoBudget;

	ExtDoc_t *					m_pLastChecked;		///< points to entry in m_dDocs which GetHitsChunk() currently emits hits for
	SphDocID_t					m_uMatchChecked;	///< there are no more hits for matches block starting with this ID
	bool						m_bTail;			///< should we emit more hits for current docid or proceed furthwer

public:
	static volatile bool		m_bInterruptNow; ///< may be set from outside to indicate the globally received sigterm
};

/// Immediately interrupt current operation
void sphInterruptNow()
{
	ExtTerm_c::m_bInterruptNow = true;
}

bool sphInterrupted()
{
	return ExtTerm_c::m_bInterruptNow;
}

volatile bool ExtTerm_c::m_bInterruptNow = false;

/// single keyword streamer with artificial hitlist
class ExtTermHitless_c : public ExtTerm_c
{
public:
	ExtTermHitless_c ( ISphQword * pQword, const FieldMask_t& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
		: ExtTerm_c ( pQword, uFields, tSetup, bNotWeighted )
		, m_uFieldPos ( 0 )
	{}

	virtual void				Reset ( const ISphQwordSetup & ) { m_uFieldPos = 0; }
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	virtual bool				GotHitless () { return true; }

protected:
	DWORD	m_uFieldPos;
};

//////////////////////////////////////////////////////////////////////////

/// position filter policy
template < TermPosFilter_e T >
class TermAcceptor_c
{
public:
								TermAcceptor_c ( ISphQword *, const XQNode_t *, const ISphQwordSetup & ) {}
protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const { return true; }
	inline void					Reset() {}
};

template<>
class TermAcceptor_c<TERM_POS_FIELD_LIMIT> : public ISphNoncopyable
{
public:
	TermAcceptor_c ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & )
		: m_iMaxFieldPos ( pNode->m_dSpec.m_iFieldMaxPos )
	{}
protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const;
	inline void					Reset() {}
private:
	const int					m_iMaxFieldPos;
};

template<>
class TermAcceptor_c<TERM_POS_ZONES> : public ISphNoncopyable
{
public:
								TermAcceptor_c ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
									: m_pZoneChecker ( tSetup.m_pZoneChecker )
									, m_dZones ( pNode->m_dSpec.m_dZones )
									, m_uLastZonedId ( 0 )
									, m_iCheckFrom ( 0 )
								{}
protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * pHit ) const;
	inline void					Reset()
	{
		m_uLastZonedId = 0;
		m_iCheckFrom = 0;
	}
protected:
	ISphZoneCheck *				m_pZoneChecker;				///< zone-limited searches query ranker about zones
	mutable CSphVector<int>		m_dZones;					///< zone ids for this particular term
	mutable SphDocID_t			m_uLastZonedId;
	mutable int					m_iCheckFrom;
};

///
class BufferedNode_c
{
protected:
	BufferedNode_c ()
		: m_pRawDocs ( NULL )
		, m_pRawDoc ( NULL )
		, m_pRawHit ( NULL )
		, m_uLastID ( 0 )
		, m_eState ( COPY_DONE )
		, m_uDoneFor ( 0 )
		{
			m_dMyDocs[0].m_uDocid = DOCID_MAX;
			m_dMyHits[0].m_uDocid = DOCID_MAX;
			m_dFilteredHits[0].m_uDocid = DOCID_MAX;
		}

	void Reset ()
	{
		m_pRawDocs = NULL;
		m_pRawDoc = NULL;
		m_pRawHit = NULL;
		m_uLastID = 0;
		m_eState = COPY_DONE;
		m_uDoneFor = 0;
		m_dMyDocs[0].m_uDocid = DOCID_MAX;
		m_dMyHits[0].m_uDocid = DOCID_MAX;
		m_dFilteredHits[0].m_uDocid = DOCID_MAX;
	}

protected:
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
	ExtDoc_t					m_dMyDocs[ExtNode_i::MAX_DOCS];		///< all documents within the required pos range
	ExtHit_t					m_dMyHits[ExtNode_i::MAX_HITS];		///< all hits within the required pos range
	ExtHit_t					m_dFilteredHits[ExtNode_i::MAX_HITS];	///< hits from requested subset of the documents (for GetHitsChunk())
	SphDocID_t					m_uDoneFor;
};

/// single keyword streamer, with term position filtering
template < TermPosFilter_e T, class ExtBase=ExtTerm_c >
class ExtConditional : public BufferedNode_c, public ExtBase, protected TermAcceptor_c<T>
{
	typedef TermAcceptor_c<T>	t_Acceptor;
protected:
								ExtConditional ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup );
public:
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	virtual bool				GotHitless () { return false; }

private:
	virtual bool				ExtraDataImpl ( ExtraData_e eData, void ** ppResult );
};

/// single keyword streamer, with term position filtering
template < TermPosFilter_e T >
class ExtTermPos_c : public ExtConditional<T,ExtTerm_c>
{
public:
	ExtTermPos_c ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
		: ExtConditional<T,ExtTerm_c> ( pQword, pNode, tSetup )
	{
		ExtTerm_c::Init ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
};


template<TermPosFilter_e T, class ExtBase>
bool ExtConditional<T,ExtBase>::ExtraDataImpl ( ExtraData_e, void ** )
{
	return false;
}

/// multi-node binary-operation streamer traits
class ExtTwofer_c : public ExtNode_i
{
public:
								ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup );
								ExtTwofer_c () {} ///< to be used in pair with Init();
								~ExtTwofer_c ();

			void				Init ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const;

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
	}

	virtual void HintDocid ( SphDocID_t uMinID )
	{
		m_pChildren[0]->HintDocid ( uMinID );
		m_pChildren[1]->HintDocid ( uMinID );
	}

	virtual uint64_t GetWordID () const
	{
		return m_pChildren[0]->GetWordID() ^ m_pChildren[1]->GetWordID();
	}

protected:
	ExtNode_i *					m_pChildren[2];
	const ExtDoc_t *			m_pCurDoc[2];
	const ExtHit_t *			m_pCurHit[2];
	WORD						m_dNodePos[2];
	SphDocID_t					m_uMatchedDocid;
};

/// A-and-B streamer
class ExtAnd_c : public ExtTwofer_c
{
public:
								ExtAnd_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup ) : ExtTwofer_c ( pFirst, pSecond, tSetup ) {}
								ExtAnd_c() {} ///< to be used with Init()
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtAnd", iLevel ); }
};

class ExtAndZonespanned_c : public ExtAnd_c
{
public:
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtAndZonespan", iLevel ); }

protected:
	bool IsSameZonespan ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 ) const;

	ISphZoneCheck * m_pZoneChecker;
	CSphVector<int> m_dZones;
};

class ExtAndZonespan_c : public ExtConditional < TERM_POS_NONE, ExtAndZonespanned_c >
{
public:
	ExtAndZonespan_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup, const XQNode_t * pNode )
		: ExtConditional<TERM_POS_NONE,ExtAndZonespanned_c> ( NULL, pNode, tSetup )
	{
		Init ( pFirst, pSecond, tSetup );
		m_pZoneChecker = tSetup.m_pZoneChecker;
		m_dZones = pNode->m_dSpec.m_dZones;
	}
};

/// A-or-B streamer
class ExtOr_c : public ExtTwofer_c
{
public:
								ExtOr_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup ) : ExtTwofer_c ( pFirst, pSecond, tSetup ) {}
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtOr", iLevel ); }
};


/// A-maybe-B streamer
class ExtMaybe_c : public ExtOr_c
{
public:
								ExtMaybe_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup ) : ExtOr_c ( pFirst, pSecond, tSetup ) {}
	virtual const ExtDoc_t *	GetDocsChunk();

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtMaybe", iLevel ); }
};


/// A-and-not-B streamer
class ExtAndNot_c : public ExtTwofer_c
{
public:
								ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	virtual void				Reset ( const ISphQwordSetup & tSetup );

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtAndNot", iLevel ); }

protected:
	bool						m_bPassthrough;
};


/// generic operator over N nodes
class ExtNWayT : public ExtNode_i
{
public:
								ExtNWayT ( const CSphVector<ExtNode_i *> & dNodes, const ISphQwordSetup & tSetup );
								~ExtNWayT ();
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const;
	virtual bool				GotHitless () { return false; }
	virtual void				HintDocid ( SphDocID_t uMinID ) { m_pNode->HintDocid ( uMinID ); }
	virtual uint64_t			GetWordID () const;

protected:
	ExtNode_i *					m_pNode;				///< my and-node for all the terms
	const ExtDoc_t *			m_pDocs;				///< current docs chunk from and-node
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
	inline void ConstructNode ( const CSphVector<ExtNode_i *> & dNodes, const CSphVector<WORD> & dPositions, const ISphQwordSetup & tSetup )
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

/// FSM is Finite State Machine
template < class FSM >
class ExtNWay_c : public ExtNWayT, private FSM
{
public:
	ExtNWay_c ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
		: ExtNWayT ( dNodes, tSetup )
		, FSM ( dNodes, tNode, tSetup )
	{
		CSphVector<WORD> dPositions ( dNodes.GetLength() );
		ARRAY_FOREACH ( i, dPositions )
			dPositions[i] = (WORD) i;
		dPositions.Sort ( ExtNodeTFExt_fn ( dNodes ) );
		ConstructNode ( dNodes, dPositions, tSetup );
	}

public:
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
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
	struct State_t
	{
		int m_iTagQword;
		DWORD m_uExpHitposWithField;
	};

protected:
								FSMphrase ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	bool						HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget );

	inline static const char *	GetName() { return "ExtPhrase"; }
	inline void ResetFSM()
	{
		m_dStates.Resize(0);
	}

protected:
	CSphVector<int>				m_dQposDelta;			///< next expected qpos delta for each existing qpos (for skipped stopwords case)
	CSphVector<int>				m_dAtomPos;				///< lets use it as finite automata states and keep references on it
	CSphVector<State_t>			m_dStates;				///< pointers to states of finite automata
	DWORD						m_uQposMask;
};
/// exact phrase streamer
typedef ExtNWay_c < FSMphrase > ExtPhrase_c;

/// proximity streamer
class FSMproximity
{
protected:
								FSMproximity ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	bool						HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget );

	inline static const char *	GetName() { return "ExtProximity"; }
	inline void ResetFSM()
	{
		m_uExpPos = 0;
		m_uWords = 0;
		m_iMinQindex = -1;
		ARRAY_FOREACH ( i, m_dProx )
			m_dProx[i] = UINT_MAX;
	}

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
	DWORD						m_uQposMask;
};
/// exact phrase streamer
typedef ExtNWay_c<FSMproximity> ExtProximity_c;

/// proximity streamer
class FSMmultinear
{
protected:
								FSMmultinear ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	bool						HitFSM ( const ExtHit_t * pHit, ExtHit_t * dTarget );

	inline static const char *	GetName() { return "ExtMultinear"; }
	inline void ResetFSM()
	{
		m_iRing = m_uLastP = m_uPrelastP = 0;
	}

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
	bool						m_bQposMask;
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
								ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup );
	virtual						~ExtQuorum_c ();

	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const;
	virtual uint64_t			GetWordID () const;

	virtual bool				GotHitless () { return false; }

	virtual void				HintDocid ( SphDocID_t uMinID )
	{
		ARRAY_FOREACH ( i, m_dChildren )
			if ( m_dChildren[i].m_pTerm )
				m_dChildren[i].m_pTerm->HintDocid ( uMinID );
	}

	static int					GetThreshold ( const XQNode_t & tNode, int iQwords );

private:

	struct TermTuple_t
	{
		ExtNode_i *			m_pTerm;		///< my children nodes (simply ExtTerm_c for now, not true anymore)
		const ExtDoc_t *	m_pCurDoc;		///< current positions into children doclists
		const ExtHit_t *	m_pCurHit;		///< current positions into children hitlists
		int					m_iCount;		///< terms count in case of dupes
		bool				m_bStandStill;	///< should we emit hits to proceed further
	};

	ExtHit_t					m_dQuorumHits[MAX_HITS];	///< buffer for all my quorum hits; inherited m_dHits will receive filtered results
	int							m_iMyHitCount;				///< hits collected so far
	int							m_iMyLast;					///< hits processed so far
	CSphVector<TermTuple_t>		m_dInitialChildren;			///< my children nodes (simply ExtTerm_c for now)
	CSphVector<TermTuple_t>		m_dChildren;
	SphDocID_t					m_uMatchedDocid;			///< tail docid for hitlist emission
	int							m_iThresh;					///< keyword count threshold
	// FIXME!!! also skip hits processing for children w\o constrains ( zones or field limit )
	bool						m_bHasDupes;				///< should we analyze hits on docs collecting

	// check for hits that matches and return flag that docs might be advanced
	bool						CollectMatchingHits ( SphDocID_t uDocid, int iQuorum );
	const ExtHit_t *			GetHitsChunkDupes ( const ExtDoc_t * pDocs );
	const ExtHit_t *			GetHitsChunkDupesTail ();
	const ExtHit_t *			GetHitsChunkSimple ( const ExtDoc_t * pDocs );

	int							CountQuorum ( bool bFixDupes )
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
};


/// A-B-C-in-this-order streamer
class ExtOrder_c : public ExtNode_i
{
public:
								ExtOrder_c ( const CSphVector<ExtNode_i *> & dChildren, const ISphQwordSetup & tSetup );
								~ExtOrder_c ();

	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const;
	virtual bool				GotHitless () { return false; }
	virtual uint64_t			GetWordID () const;

	virtual void HintDocid ( SphDocID_t uMinID )
	{
		ARRAY_FOREACH ( i, m_dChildren )
			m_dChildren[i]->HintDocid ( uMinID );
	}

protected:
	CSphVector<ExtNode_i *>		m_dChildren;
	CSphVector<const ExtDoc_t*>	m_pDocsChunk;	///< last document chunk (for hit fetching)
	CSphVector<const ExtDoc_t*>	m_pDocs;		///< current position in document chunk
	CSphVector<const ExtHit_t*>	m_pHits;		///< current position in hits chunk
	ExtHit_t					m_dMyHits[MAX_HITS];	///< buffer for all my phrase hits; inherited m_dHits will receive filtered results
	bool						m_bDone;
	SphDocID_t					m_uHitsOverFor;

protected:
	int							GetChildIdWithNextHit ( SphDocID_t uDocid );							///< get next hit within given document, and return its child-id
	int							GetMatchingHits ( SphDocID_t uDocid, ExtHit_t * pHitbuf, int iLimit );	///< process candidate hits and stores actual matches while we can
};


/// same-text-unit streamer
/// (aka, A and B within same sentence, or same paragraph)
class ExtUnit_c : public ExtNode_i
{
public:
	ExtUnit_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const FieldMask_t& dFields, const ISphQwordSetup & tSetup, const char * sUnit );
	~ExtUnit_c ();

	virtual const ExtDoc_t *	GetDocsChunk();
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void				GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const;
	virtual uint64_t			GetWordID () const;

public:
	virtual bool GotHitless ()
	{
		return false;
	}

	virtual void HintDocid ( SphDocID_t uMinID )
	{
		m_pArg1->HintDocid ( uMinID );
		m_pArg2->HintDocid ( uMinID );
	}

	virtual void DebugDump ( int iLevel )
	{
		DebugIndent ( iLevel );
		printf ( "ExtSentence\n" );
		m_pArg1->DebugDump ( iLevel+1 );
		m_pArg2->DebugDump ( iLevel+1 );
	}

protected:
	inline const ExtDoc_t * ReturnDocsChunk ( int iDocs, int iMyHit )
	{
		assert ( iMyHit<MAX_HITS );
		m_dMyHits[iMyHit].m_uDocid = DOCID_MAX;
		m_uHitsOverFor = 0;
		return ExtNode_i::ReturnDocsChunk ( iDocs, "unit" );
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


typedef CSphFixedVector < CSphVector < ZoneInfo_t > > ZoneVVector_t;

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

	int							GetQwords ( ExtQwordsHash_t & hQwords )					{ return m_pRoot ? m_pRoot->GetQwords ( hQwords ) : -1; }
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void				SetTermDupes ( const ExtQwordsHash_t & , int ) {}
	virtual bool				InitState ( const CSphQueryContext &, CSphString & )	{ return true; }

public:
	// FIXME? hide and friend?
	virtual SphZoneHit_e		IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan );
	virtual const CSphIndex *	GetIndex() { return m_pIndex; }

public:
	CSphMatch					m_dMatches[ExtNode_i::MAX_DOCS];	///< exposed for caller
	DWORD						m_uPayloadMask;						///< exposed for ranker state functors
	int							m_iQwords;							///< exposed for ranker state functors
	int							m_iMaxQpos;							///< max in-query pos among all keywords, including dupes; for ranker state functors

protected:
	void						CleanupZones ( SphDocID_t uMaxDocid );
	int							m_iInlineRowitems;
	ExtNode_i *					m_pRoot;
	const ExtDoc_t *			m_pDoclist;
	const ExtHit_t *			m_pHitlist;
	ExtDoc_t					m_dMyDocs[ExtNode_i::MAX_DOCS];		///< my local documents pool; for filtering
	CSphMatch					m_dMyMatches[ExtNode_i::MAX_DOCS];	///< my local matches pool; for filtering
	CSphMatch					m_tTestMatch;
	const CSphIndex *			m_pIndex;							///< this is he who'll do my filtering!
	CSphQueryContext *			m_pCtx;
	int64_t *					m_pNanoBudget;

protected:
	CSphVector<CSphString>		m_dZones;
	CSphVector<ExtTerm_c*>		m_dZoneStartTerm;
	CSphVector<ExtTerm_c*>		m_dZoneEndTerm;
	CSphVector<const ExtDoc_t*>	m_dZoneStart;
	CSphVector<const ExtDoc_t*>	m_dZoneEnd;
	CSphVector<SphDocID_t>		m_dZoneMax;				///< last docid we (tried) to cache
	CSphVector<SphDocID_t>		m_dZoneMin;				///< first docid we (tried) to cache
	ZoneVVector_t				m_dZoneInfo;
	bool						m_bZSlist;
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
	const ExtHit_t *	m_pHitBase;
	CSphVector<int>		m_dZonespans; // zonespanlists for my matches

public:
					ExtRanker_T ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup );
	virtual int		GetMatches ();

	virtual bool InitState ( const CSphQueryContext & tCtx, CSphString & sError )
	{
		return m_tState.Init ( tCtx.m_iWeights, &tCtx.m_dWeights[0], this, sError, tCtx.m_uPackedFactorFlags );
	}
private:
	virtual bool ExtraDataImpl ( ExtraData_e eType, void ** ppResult )
	{
		switch ( eType )
		{
			case EXTRA_GET_DATA_ZONESPANS:
				assert ( ppResult );
				*ppResult = &m_dZonespans;
				return true;
			default:
				return m_tState.ExtraData ( eType, ppResult );
		}
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
	: m_iAtomPos ( 0 )
	, m_iStride ( 0 )
	, m_pDocinfo ( NULL )
{
	m_dDocs[0].m_uDocid = DOCID_MAX;
	m_dHits[0].m_uDocid = DOCID_MAX;
}


static ISphQword * CreateQueryWord ( const XQKeyword_t & tWord, const ISphQwordSetup & tSetup, CSphDict * pZonesDict=NULL )
{
	BYTE sTmp [ 3*SPH_MAX_WORD_LEN + 16 ];
	strncpy ( (char*)sTmp, tWord.m_sWord.cstr(), sizeof(sTmp) );
	sTmp[sizeof(sTmp)-1] = '\0';

	ISphQword * pWord = tSetup.QwordSpawn ( tWord );
	pWord->m_sWord = tWord.m_sWord;
	CSphDict * pDict = pZonesDict ? pZonesDict : tSetup.m_pDict;
	pWord->m_uWordID = tWord.m_bMorphed
		? pDict->GetWordIDNonStemmed ( sTmp )
		: pDict->GetWordID ( sTmp );
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


template < typename T >
static ExtNode_i * CreateMultiNode ( const XQNode_t * pQueryNode, const ISphQwordSetup & tSetup, bool bNeedsHitlist )
{
	///////////////////////////////////
	// virtually plain (expanded) node
	///////////////////////////////////
	assert ( pQueryNode );
	if ( pQueryNode->m_dChildren.GetLength() )
	{
		CSphVector<ExtNode_i *> dNodes;
		ARRAY_FOREACH ( i, pQueryNode->m_dChildren )
		{
			ExtNode_i * pTerm = ExtNode_i::Create ( pQueryNode->m_dChildren[i], tSetup );
			assert ( !pTerm || pTerm->m_iAtomPos>=0 );
			if ( pTerm )
				dNodes.Add ( pTerm );
		}

		if ( dNodes.GetLength()<2 )
		{
			ARRAY_FOREACH ( i, dNodes )
				SafeDelete ( dNodes[i] );
			if ( tSetup.m_pWarning )
				tSetup.m_pWarning->SetSprintf ( "can't create phrase node, hitlists unavailable (hitlists=%d, nodes=%d)", dNodes.GetLength(), pQueryNode->m_dChildren.GetLength() );
			return NULL;
		}

		// FIXME! tricky combo again
		// quorum+expand used KeywordsEqual() path to drill down until actual nodes
		ExtNode_i * pResult = new T ( dNodes, *pQueryNode, tSetup );
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
		assert ( pQueryNode->m_dWords.GetLength() );
		assert ( pQueryNode->GetOp()==SPH_QUERY_PHRASE || pQueryNode->GetOp()==SPH_QUERY_PROXIMITY || pQueryNode->GetOp()==SPH_QUERY_QUORUM );

		// create nodes
		CSphVector<ExtNode_i *> dNodes;
		ARRAY_FOREACH ( i, dQwordsHit )
		{
			dNodes.Add ( ExtNode_i::Create ( dQwordsHit[i], pQueryNode, tSetup ) );
			dNodes.Last()->m_iAtomPos = dQwordsHit[i]->m_iAtomPos;
		}

		pResult = new T ( dNodes, *pQueryNode, tSetup );
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

ExtNode_i * ExtNode_i::Create ( const XQKeyword_t & tWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
{
	return Create ( CreateQueryWord ( tWord, tSetup ), pNode, tSetup );
}

ExtNode_i * ExtNode_i::Create ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
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
		return new ExtTermHitless_c ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
	switch ( pQword->m_iTermPos )
	{
		case TERM_POS_FIELD_STARTEND:	return new ExtTermPos_c<TERM_POS_FIELD_STARTEND> ( pQword, pNode, tSetup );
		case TERM_POS_FIELD_START:		return new ExtTermPos_c<TERM_POS_FIELD_START> ( pQword, pNode, tSetup );
		case TERM_POS_FIELD_END:		return new ExtTermPos_c<TERM_POS_FIELD_END> ( pQword, pNode, tSetup );
		case TERM_POS_FIELD_LIMIT:		return new ExtTermPos_c<TERM_POS_FIELD_LIMIT> ( pQword, pNode, tSetup );
		case TERM_POS_ZONES:			return new ExtTermPos_c<TERM_POS_ZONES> ( pQword, pNode, tSetup );
		default:						return new ExtTerm_c ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
}

//////////////////////////////////////////////////////////////////////////

struct ExtPayloadEntry_t
{
	SphDocID_t	m_uDocid;
	Hitpos_t	m_uHitpos;

	bool operator < ( const ExtPayloadEntry_t & rhs ) const
	{
		if ( m_uDocid!=rhs.m_uDocid )
			return ( m_uDocid<rhs.m_uDocid );
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
class ExtPayload_c : public ExtNode_i
{
private:
	CSphVector<ExtPayloadEntry_t>	m_dCache;
	ExtPayloadKeyword_t				m_tWord;
	FieldMask_t						m_dFieldMask;

	int		m_iCurDocsEnd;		///< end of the last docs chunk returned, exclusive, ie [begin,end)
	int		m_iCurHit;			///< end of the last hits chunk (within the last docs chunk) returned, exclusive

public:
	explicit						ExtPayload_c ( const XQNode_t * pNode, const ISphQwordSetup & tSetup );
	virtual void					Reset ( const ISphQwordSetup & tSetup );
	virtual void					HintDocid ( SphDocID_t ) {} // FIXME!!! implement with tree
	virtual const ExtDoc_t *		GetDocsChunk();
	virtual const ExtHit_t *		GetHitsChunk ( const ExtDoc_t * pDocs );

	virtual int						GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void					SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual void					GetTerms ( const ExtQwordsHash_t &, CSphVector<TermPos_t> & ) const;
	virtual bool					GotHitless () { return false; }
	virtual int						GetDocsCount () { return m_tWord.m_iDocs; }
	virtual uint64_t				GetWordID () const { return m_tWord.m_uWordID; }

private:
	void							PopulateCache ( const ISphQwordSetup & tSetup, bool bFillStat );
};


ExtPayload_c::ExtPayload_c ( const XQNode_t * pNode, const ISphQwordSetup & tSetup )
{
	// sanity checks
	// this node must be only created for a huge OR of tiny expansions
	assert ( pNode->m_dWords.GetLength()==1 );
	assert ( tSetup.m_eDocinfo!=SPH_DOCINFO_INLINE );
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
		for ( ;; )
	{
		const CSphMatch & tMatch = pQword->GetNextDoc ( NULL );
		if ( !tMatch.m_uDocID )
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
			tEntry.m_uDocid = tMatch.m_uDocID;
			tEntry.m_uHitpos = uHit;
		}
	}

	m_dCache.Sort();
	if ( bFillStat && m_dCache.GetLength() )
	{
		// there might be duplicate documents, but not hits, lets recalculate docs count
		// FIXME!!! that not work for RT index - get rid of ExtPayload_c and move PopulateCache code to index specific QWord
		SphDocID_t uLastDoc = m_dCache.Begin()->m_uDocid;
		const ExtPayloadEntry_t * pCur = m_dCache.Begin() + 1;
		const ExtPayloadEntry_t * pEnd = m_dCache.Begin() + m_dCache.GetLength();
		int iDocsTotal = 1;
		while ( pCur!=pEnd )
		{
			iDocsTotal += ( uLastDoc!=pCur->m_uDocid );
			uLastDoc = pCur->m_uDocid;
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
	m_dCache.Resize ( 0 );
	PopulateCache ( tSetup, false );
}


const ExtDoc_t * ExtPayload_c::GetDocsChunk()
{
	m_iCurHit = m_iCurDocsEnd;
	if ( m_iCurDocsEnd>=m_dCache.GetLength() )
		return NULL;

	int iDoc = 0;
	int iEnd = m_iCurDocsEnd; // shortcut, and vs2005 optimization
	while ( iDoc<MAX_DOCS-1 && iEnd<m_dCache.GetLength() )
	{
		SphDocID_t uDocid = m_dCache[iEnd].m_uDocid;

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_uDocid = uDocid;
		tDoc.m_pDocinfo = NULL; // no country for old inline men
		tDoc.m_uDocFields = 0;
		tDoc.m_uHitlistOffset = 0;

		int iHitStart = iEnd;
		while ( iEnd<m_dCache.GetLength() && m_dCache[iEnd].m_uDocid==uDocid )
		{
			tDoc.m_uDocFields |= 1<< ( HITMAN::GetField ( m_dCache[iEnd].m_uHitpos ) );
			iEnd++;
		}

		int iHits = iEnd - iHitStart;
		tDoc.m_fTFIDF = float(iHits) / float(SPH_BM25_K1+iHits) * m_tWord.m_fIDF;
	}
	m_iCurDocsEnd = iEnd;

	return ReturnDocsChunk ( iDoc, "payload" );
}


const ExtHit_t * ExtPayload_c::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	if ( m_iCurHit>=m_iCurDocsEnd )
		return NULL;

	int iHit = 0;
	while ( pDocs->m_uDocid!=DOCID_MAX )
	{
		// skip rejected documents
		while ( m_iCurHit<m_iCurDocsEnd && m_dCache[m_iCurHit].m_uDocid<pDocs->m_uDocid )
			m_iCurHit++;
		if ( m_iCurHit>=m_iCurDocsEnd )
			break;

		// skip non-matching documents
		SphDocID_t uDocid = m_dCache[m_iCurHit].m_uDocid;
		if ( pDocs->m_uDocid<uDocid )
		{
			while ( pDocs->m_uDocid<uDocid )
				pDocs++;
			if ( pDocs->m_uDocid!=uDocid )
				continue;
		}

		// copy accepted documents
		while ( m_iCurHit<m_iCurDocsEnd && m_dCache[m_iCurHit].m_uDocid==pDocs->m_uDocid && iHit<MAX_HITS-1 )
		{
			ExtHit_t & tHit = m_dHits[iHit++];
			tHit.m_uDocid = m_dCache[m_iCurHit].m_uDocid;
			tHit.m_uHitpos = m_dCache[m_iCurHit].m_uHitpos;
			tHit.m_uQuerypos = (WORD) m_tWord.m_iAtomPos;
			tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
			m_iCurHit++;
		}
		if ( m_iCurHit>=m_iCurDocsEnd || iHit>=MAX_HITS-1 )
			break;
	}

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
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
			if ( pNode->m_dWords.Begin()->m_bExpanded && pNode->m_dWords.Begin()->m_pPayload )
				return new ExtPayload_c ( pNode, tSetup );

			if ( pNode->m_bVirtuallyPlain )
				return Create ( pNode->m_dChildren[0], tSetup );
			else
				return Create ( pNode->m_dWords[0], pNode, tSetup );
		}

		switch ( pNode->GetOp() )
		{
			case SPH_QUERY_PHRASE:
				return CreateMultiNode<ExtPhrase_c> ( pNode, tSetup, true );

			case SPH_QUERY_PROXIMITY:
				return CreateMultiNode<ExtProximity_c> ( pNode, tSetup, true );

			case SPH_QUERY_NEAR:
				return CreateMultiNode<ExtMultinear_c> ( pNode, tSetup, true );

			case SPH_QUERY_QUORUM:
			{
				assert ( pNode->m_dWords.GetLength()==0 || pNode->m_dChildren.GetLength()==0 );
				int iQuorumCount = pNode->m_dWords.GetLength()+pNode->m_dChildren.GetLength();
				if ( ExtQuorum_c::GetThreshold ( *pNode, iQuorumCount )>=iQuorumCount )
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

				} else // everything is ok; create quorum node
				{
					return CreateMultiNode<ExtQuorum_c> ( pNode, tSetup, false );
				}

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
		bool bZonespan = true;
		bool bZonespanChecked = false;
		for ( int i=0; i<iChildren && bAndTerms; i++ )
		{
			const XQNode_t * pChildren = pNode->m_dChildren[i];
			bAndTerms = ( pChildren->m_dWords.GetLength()==1 );
			bZonespan &= pChildren->m_dSpec.m_bZoneSpan;
			if ( !bZonespan )
				break;
			bZonespanChecked = true;
		}
		bZonespan &= bZonespanChecked;
		if ( bAndTerms )
		{
			if ( bZonespan )
			{
				CSphVector<ExtNode_i*> dTerms;
				for ( int i=0; i<iChildren; i++ )
				{
					const XQNode_t * pChild = pNode->m_dChildren[i];
					ExtNode_i * pTerm = ExtNode_i::Create ( pChild, tSetup );
					if ( pTerm )
						dTerms.Add ( pTerm );
				}

				dTerms.Sort ( ExtNodeTF_fn() );

				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<dTerms.GetLength(); i++ )
					pCur = new ExtAndZonespan_c ( pCur, dTerms[i], tSetup, pNode->m_dChildren[0] );

// For zonespan we have also Extra data which is not (yet?) covered by common-node optimization.
// if ( pNode->GetCount() )
// return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );

				return pCur;
			} else
			{
				CSphVector<ExtNode_i*> dTerms;
				for ( int i=0; i<iChildren; i++ )
				{
					const XQNode_t * pChild = pNode->m_dChildren[i];
					ExtNode_i * pTerm = ExtNode_i::Create ( pChild, tSetup );
					if ( pTerm )
						dTerms.Add ( pTerm );
				}

				dTerms.Sort ( ExtNodeTF_fn() );

				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<dTerms.GetLength(); i++ )
					pCur = new ExtAnd_c ( pCur, dTerms[i], tSetup );

				if ( pNode->GetCount() )
					return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );

				return pCur;
			}
		}

		// Multinear and phrase could be also non-plain, so here is the second entry for it.
		if ( pNode->GetOp()==SPH_QUERY_NEAR )
			return CreateMultiNode<ExtMultinear_c> ( pNode, tSetup, true );
		if ( pNode->GetOp()==SPH_QUERY_PHRASE )
			return CreateMultiNode<ExtPhrase_c> ( pNode, tSetup, true );

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
				case SPH_QUERY_MAYBE:		pCur = new ExtMaybe_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_AND:			pCur = new ExtAnd_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_ANDNOT:		pCur = new ExtAndNot_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_SENTENCE:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, MAGIC_WORD_SENTENCE ); break;
				case SPH_QUERY_PARAGRAPH:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, MAGIC_WORD_PARAGRAPH ); break;
				default:					assert ( 0 && "internal error: unhandled op in ExtNode_i::Create()" ); break;
			}
		}
		if ( pCur && pNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
		return pCur;
	}
}

//////////////////////////////////////////////////////////////////////////

inline void ExtTerm_c::Init ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
{
	m_pQword = pQword;
	m_pWarning = tSetup.m_pWarning;
	m_bNotWeighted = bNotWeighted;
	m_iAtomPos = pQword->m_iAtomPos;
	m_pLastChecked = m_dDocs;
	m_uMatchChecked = 0;
	m_bTail = false;
	m_dQueriedFields = dFields;
	m_bHasWideFields = false;
	if ( tSetup.m_pIndex && tSetup.m_pIndex->GetMatchSchema().m_dFields.GetLength()>32 )
		for ( int i=1; i<FieldMask_t::SIZE && !m_bHasWideFields; i++ )
			if ( m_dQueriedFields[i] )
				m_bHasWideFields = true;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pStats = tSetup.m_pStats;
	m_pNanoBudget = m_pStats ? m_pStats->m_pNanoBudget : NULL;
	AllocDocinfo ( tSetup );
}

ExtTerm_c::ExtTerm_c ( ISphQword * pQword, const ISphQwordSetup & tSetup )
	: m_pQword ( pQword )
	, m_pWarning ( tSetup.m_pWarning )
	, m_bNotWeighted ( true )
{
	m_iAtomPos = pQword->m_iAtomPos;
	m_pLastChecked = m_dDocs;
	m_uMatchChecked = 0;
	m_bTail = false;
	m_dQueriedFields.SetAll();
	m_bHasWideFields = tSetup.m_pIndex && ( tSetup.m_pIndex->GetMatchSchema().m_dFields.GetLength()>32 );
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pStats = tSetup.m_pStats;
	m_pNanoBudget = m_pStats ? m_pStats->m_pNanoBudget : NULL;
	AllocDocinfo ( tSetup );
}

ExtTerm_c::ExtTerm_c ( ISphQword * pQword, const FieldMask_t & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
{
	Init ( pQword, dFields, tSetup, bNotWeighted );
}

void ExtTerm_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pLastChecked = m_dDocs;
	m_uMatchChecked = 0;
	m_bTail = false;
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pQword->Reset ();
	tSetup.QwordSetup ( m_pQword );
}

const ExtDoc_t * ExtTerm_c::GetDocsChunk()
{
	m_pLastChecked = m_dDocs;
	m_bTail = false;

	if ( !m_pQword->m_iDocs )
		return NULL;

	// max_query_time
	if ( m_iMaxTimer>0 && sphMicroTimer()>=m_iMaxTimer )
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
		if ( !tMatch.m_uDocID )
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
		tDoc.m_uDocid = tMatch.m_uDocID;
		tDoc.m_pDocinfo = pDocinfo;
		tDoc.m_uHitlistOffset = m_pQword->m_iHitlistPos;
		tDoc.m_uDocFields = m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32(); // OPTIMIZE: only needed for phrase node
		tDoc.m_fTFIDF = float(m_pQword->m_uMatchHits) / float(m_pQword->m_uMatchHits+SPH_BM25_K1) * m_fIDF;
		pDocinfo += m_iStride;
	}

	if ( m_pStats )
		m_pStats->m_iFetchedDocs += iDoc;
	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostDoc*iDoc;

	return ReturnDocsChunk ( iDoc, "term" );
}

const ExtHit_t * ExtTerm_c::GetHitsChunk ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return NULL;

	// aim to the right document
	ExtDoc_t * pDoc = m_pLastChecked;
	assert ( pDoc && pDoc>=m_dDocs && pDoc<m_dDocs+MAX_DOCS );

	if ( !m_bTail )
	{
		// if we already emitted hits for this matches block, do not do that again
		if ( pMatched->m_uDocid==m_uMatchChecked )
			return NULL;

		// find match
		m_uMatchChecked = pMatched->m_uDocid;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			m_pLastChecked = pDoc;
			if ( pDoc->m_uDocid==DOCID_MAX )
				return NULL; // matched docs block is over for me, gimme another one

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX )
				return NULL; // matched doc block did not yet begin for me, gimme another one
		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		// setup hitlist reader
		m_pQword->SeekHitlist ( pDoc->m_uHitlistOffset );
	}

	// hit emission
	int iHit = 0;
	m_bTail = false;
	while ( iHit<MAX_HITS-1 )
	{
		// get next hit
		Hitpos_t uHit = m_pQword->GetNextHit();
		if ( uHit==EMPTY_HIT )
		{
			// no more hits; get next acceptable document
			pDoc++;
			m_pLastChecked = pDoc;

			do
			{
				while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
				m_pLastChecked = pDoc;
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

		if (!( m_dQueriedFields.Test ( HITMAN::GetField ( uHit ) ) ))
			continue;

		ExtHit_t & tHit = m_dHits[iHit++];
		tHit.m_uDocid = pDoc->m_uDocid;
		tHit.m_uHitpos = uHit;
		tHit.m_uQuerypos = (WORD) m_iAtomPos; // assume less that 64K words per query
		tHit.m_uWeight = tHit.m_uMatchlen = tHit.m_uSpanlen = 1;
	}

	if ( iHit==MAX_HITS-1 )
		m_bTail = true;

	if ( m_pStats )
		m_pStats->m_iFetchedHits += iHit;
	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostHit*iHit;

	return ReturnHitsChunk ( iHit, "term" );
}

int ExtTerm_c::GetQwords ( ExtQwordsHash_t & hQwords )
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

void ExtTerm_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	if ( m_fIDF<0.0f )
	{
		assert ( hQwords ( m_pQword->m_sWord ) );
		m_fIDF = hQwords ( m_pQword->m_sWord )->m_fIDF;
	}
}

void ExtTerm_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	if ( m_bNotWeighted || m_pQword->m_bExcluded )
		return;

	ExtQword_t & tQword = hQwords[ m_pQword->m_sWord ];

	TermPos_t & tPos = dTermDupes.Add ();
	tPos.m_uAtomPos = (WORD)m_pQword->m_iAtomPos;
	tPos.m_uQueryPos = (WORD)tQword.m_iQueryPos;
}

//////////////////////////////////////////////////////////////////////////

const ExtHit_t * ExtTermHitless_c::GetHitsChunk ( const ExtDoc_t * pMatched )
{
	if ( !pMatched )
		return NULL;

	// aim to the right document
	ExtDoc_t * pDoc = m_pLastChecked;
	assert ( pDoc && pDoc>=m_dDocs && pDoc<m_dDocs+MAX_DOCS );

	if ( !m_bTail )
	{
		// if we already emitted hits for this matches block, do not do that again
		if ( pMatched->m_uDocid==m_uMatchChecked )
			return NULL;

		// find match
		m_uMatchChecked = pMatched->m_uDocid;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			m_pLastChecked = pDoc;
			if ( pDoc->m_uDocid==DOCID_MAX )
				return NULL; // matched docs block is over for me, gimme another one

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX )
				return NULL; // matched doc block did not yet begin for me, gimme another one
		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		m_uFieldPos = 0;
	}

	// hit emission
	int iHit = 0;
	m_bTail = false;
	for ( ;; )
	{
		if ( ( m_uFieldPos<32 && ( pDoc->m_uDocFields & ( 1 << m_uFieldPos ) ) ) // not necessary
			&& m_dQueriedFields.Test ( m_uFieldPos ) )
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

		if ( m_uFieldPos<SPH_MAX_FIELDS-1 )
		{
			m_uFieldPos++;
			continue;
		}

		// field mask is empty, get next document
		pDoc++;
		m_pLastChecked = pDoc;
		do
		{
			while ( pDoc->m_uDocid < pMatched->m_uDocid ) pDoc++;
			m_pLastChecked = pDoc;
			if ( pDoc->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched docs block is over for me, gimme another one

			while ( pMatched->m_uDocid < pDoc->m_uDocid ) pMatched++;
			if ( pMatched->m_uDocid==DOCID_MAX ) { pDoc = NULL; break; } // matched doc block did not yet begin for me, gimme another one
		} while ( pDoc->m_uDocid!=pMatched->m_uDocid );

		if ( !pDoc )
			break;

		m_uFieldPos = 0;
	}

	if ( iHit==MAX_HITS-1 )
		m_bTail = true;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

template < TermPosFilter_e T, class ExtBase >
ExtConditional<T,ExtBase>::ExtConditional ( ISphQword * pQword, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: BufferedNode_c ()
	, ExtBase ()
	, t_Acceptor ( pQword, pNode, tSetup )
{
	ExtBase::AllocDocinfo ( tSetup );
}

template < TermPosFilter_e T, class ExtBase >
void ExtConditional<T,ExtBase>::Reset ( const ISphQwordSetup & tSetup )
{
	BufferedNode_c::Reset();
	ExtBase::Reset(tSetup);
	TermAcceptor_c<T>::Reset();
}

inline bool TermAcceptor_c<TERM_POS_FIELD_LIMIT>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )<=m_iMaxFieldPos;
}

template<>
inline bool TermAcceptor_c<TERM_POS_FIELD_START>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1;
}

template<>
inline bool TermAcceptor_c<TERM_POS_FIELD_END>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::IsEnd ( pHit->m_uHitpos );
}

template<>
inline bool TermAcceptor_c<TERM_POS_FIELD_STARTEND>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	return HITMAN::GetPos ( pHit->m_uHitpos )==1 && HITMAN::IsEnd ( pHit->m_uHitpos );
}

inline bool TermAcceptor_c<TERM_POS_ZONES>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	assert ( m_pZoneChecker );

	if ( m_uLastZonedId!=pHit->m_uDocid )
		m_iCheckFrom = 0;
	m_uLastZonedId = pHit->m_uDocid;

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

template < TermPosFilter_e T, class ExtBase >
const ExtDoc_t * ExtConditional<T,ExtBase>::GetDocsChunk()
{
	SphDocID_t uSkipID = m_uLastID;
	// fetch more docs if needed
	if ( !m_pRawDocs )
	{
		m_pRawDocs = ExtBase::GetDocsChunk();
		if ( !m_pRawDocs )
			return NULL;

		m_pRawDoc = m_pRawDocs;
		m_pRawHit = NULL;
		uSkipID = 0;
	}

	// filter the hits, and build the documents list
	int iMyDoc = 0;
	int iMyHit = 0;

	const ExtDoc_t * pDoc = m_pRawDoc; // just a shortcut
	const ExtHit_t * pHit = m_pRawHit;
	SphDocID_t uLastID = m_uLastID = 0;

	CSphRowitem * pDocinfo = ExtBase::m_pDocinfo;
	for ( ;; )
	{
		// try to fetch more hits for current raw docs block if we're out
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
			pHit = ExtBase::GetHitsChunk ( m_pRawDocs );

		// did we touch all the hits we had? if so, we're fully done with
		// current raw docs block, and should start a new one
		if ( !pHit )
		{
			m_pRawDocs = ExtBase::GetDocsChunk();
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

		while ( ( pHit->m_uDocid!=DOCID_MAX || ( uSkipID && pHit->m_uDocid<=uSkipID ) ) && !t_Acceptor::IsAcceptableHit ( pHit ) ) // skip unneeded hits
			pHit++;

		if ( pHit->m_uDocid==DOCID_MAX || ( uSkipID && pHit->m_uDocid<=uSkipID ) ) // check for eof
			continue;

		// find and emit new document
		while ( pDoc->m_uDocid<pHit->m_uDocid ) pDoc++; // FIXME? unsafe in broken cases
		assert ( pDoc->m_uDocid==pHit->m_uDocid );
		assert ( iMyDoc<ExtBase::MAX_DOCS-1 );

		if ( uLastID!=pDoc->m_uDocid )
			CopyExtDoc ( m_dMyDocs[iMyDoc++], *pDoc, &pDocinfo, ExtBase::m_iStride );
		uLastID = pDoc->m_uDocid;


		// current hit is surely acceptable.
		m_dMyHits[iMyHit++] = *(pHit++);
		// copy acceptable hits for this document
		while ( iMyHit<ExtBase::MAX_HITS-1 && pHit->m_uDocid==uLastID )
		{
			if ( t_Acceptor::IsAcceptableHit ( pHit ) )
			{
				m_dMyHits[iMyHit++] = *pHit;
			}
			pHit++;
		}

		if ( iMyHit==ExtBase::MAX_HITS-1 )
		{
			// there is no more space for acceptable hits; but further calls to GetHits() *might* produce some
			// we need to memorize the trailing document id
			m_uLastID = uLastID;
			break;
		}
	}

	m_pRawDoc = pDoc;
	m_pRawHit = pHit;

	assert ( iMyDoc>=0 && iMyDoc<ExtBase::MAX_DOCS );
	assert ( iMyHit>=0 && iMyHit<ExtBase::MAX_DOCS );

	m_dMyDocs[iMyDoc].m_uDocid = DOCID_MAX;
	m_dMyHits[iMyHit].m_uDocid = DOCID_MAX;
	m_eState = COPY_FILTERED;

	PrintDocsChunk ( iMyDoc, ExtBase::m_iAtomPos, m_dMyDocs, "cond", this );

	return iMyDoc ? m_dMyDocs : NULL;
}


template < TermPosFilter_e T, class ExtBase >
const ExtHit_t * ExtConditional<T,ExtBase>::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	const ExtDoc_t * pStart = pDocs;
	if ( m_eState==COPY_DONE )
	{
		// this request completed in full
		if ( m_uDoneFor==pDocs->m_uDocid || !m_uDoneFor )
			return NULL;

		// old request completed in full, but we have a new hits subchunk request now
		// even though there were no new docs requests in the meantime!
		m_eState = COPY_FILTERED;
		if ( m_uDoneFor && m_uDoneFor!=DOCID_MAX )
		{
			while ( pDocs->m_uDocid!=DOCID_MAX && pDocs->m_uDocid<=m_uDoneFor )
				pDocs++;
		}
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
			while ( iFilteredHits<ExtBase::MAX_HITS-1 && pDocs->m_uDocid==pMyHit->m_uDocid )
			{
				m_dFilteredHits[iFilteredHits++] = *pMyHit++;
			}

			// paranoid check that we're not out of bounds
			assert ( iFilteredHits<=ExtBase::MAX_HITS-1 && pDocs->m_uDocid!=pMyHit->m_uDocid );
		}
	}

	// trailing hits case
	// my hits did not have enough space, so we should pass raw hits for the last doc
	while ( m_eState==COPY_TRAILING && m_uLastID && iFilteredHits<ExtBase::MAX_HITS-1 )
	{
		// where do we stand?
		if ( !m_pRawHit || m_pRawHit->m_uDocid==DOCID_MAX )
			m_pRawHit = ExtBase::GetHitsChunk ( m_pRawDocs );

		// no more hits for current chunk
		if ( !m_pRawHit )
		{
			m_eState = COPY_DONE;
			break;
		}

		// copy while we can
		while ( m_pRawHit->m_uDocid==m_uLastID && iFilteredHits<ExtBase::MAX_HITS-1 )
		{
			if ( t_Acceptor::IsAcceptableHit ( m_pRawHit ) )
			{
				m_dFilteredHits[iFilteredHits++] = *m_pRawHit;
			}
			m_pRawHit++;
		}

		// raise the flag for future calls if trailing hits are over
		if ( m_pRawHit->m_uDocid!=m_uLastID && m_pRawHit->m_uDocid!=DOCID_MAX )
			m_eState = COPY_DONE;

		// in any case, this chunk is over
		break;
	}

	m_uDoneFor = pDocs->m_uDocid;
	if ( m_uDoneFor==DOCID_MAX && pDocs-1>=pStart )
		m_uDoneFor = ( pDocs-1 )->m_uDocid;

	PrintHitsChunk ( iFilteredHits, ExtBase::m_iAtomPos, m_dFilteredHits, "cond", this );

	m_dFilteredHits[iFilteredHits].m_uDocid = DOCID_MAX;
	return iFilteredHits ? m_dFilteredHits : NULL;
}

//////////////////////////////////////////////////////////////////////////

ExtTwofer_c::ExtTwofer_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup )
{
	Init ( pFirst, pSecond, tSetup );
}

inline void	ExtTwofer_c::Init ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup )
{
	m_pChildren[0] = pFirst;
	m_pChildren[1] = pSecond;
	m_pCurHit[0] = NULL;
	m_pCurHit[1] = NULL;
	m_pCurDoc[0] = NULL;
	m_pCurDoc[1] = NULL;
	m_dNodePos[0] = 0;
	m_dNodePos[1] = 0;
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

int ExtTwofer_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax1 = m_pChildren[0]->GetQwords ( hQwords );
	int iMax2 = m_pChildren[1]->GetQwords ( hQwords );
	return Max ( iMax1, iMax2 );
}

void ExtTwofer_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_pChildren[0]->SetQwordsIDF ( hQwords );
	m_pChildren[1]->SetQwordsIDF ( hQwords );
}

void ExtTwofer_c::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	m_pChildren[0]->GetTerms ( hQwords, dTermDupes );
	m_pChildren[1]->GetTerms ( hQwords, dTermDupes );
}

//////////////////////////////////////////////////////////////////////////

const ExtDoc_t * ExtAnd_c::GetDocsChunk()
{
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

			if ( !pCur0 )
			{
				if ( pCur1 && pCur1->m_uDocid!=DOCID_MAX )
					m_pChildren[0]->HintDocid ( pCur1->m_uDocid );
				pCur0 = m_pChildren[0]->GetDocsChunk();
			}
			if ( !pCur1 )
			{
				if ( pCur0 && pCur0->m_uDocid!=DOCID_MAX )
					m_pChildren[1]->HintDocid ( pCur0->m_uDocid );
				pCur1 = m_pChildren[1]->GetDocsChunk();
			}
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
			tDoc.m_uDocFields = pCur0->m_uDocFields | pCur1->m_uDocFields; // not necessary
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

	return ReturnDocsChunk ( iDoc, "and" );
}

const ExtHit_t * ExtAnd_c::GetHitsChunk ( const ExtDoc_t * pDocs )
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
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX ) pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs );
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX ) pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs );

		// one of the hitlists is over
		if ( !pCur0 || !pCur1 )
		{
			// if one is over, we might still need to copy the other one. otherwise, skip it
			if ( ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid ) || ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid ) )
				continue;

			if ( pCur0 )
				while ( ( pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs ) )!=NULL );
			if ( pCur1 )
				while ( ( pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs ) )!=NULL );

			if ( !pCur0 && !pCur1 )
				break; // both are over, we're done
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

bool ExtAndZonespanned_c::IsSameZonespan ( const ExtHit_t * pHit1, const ExtHit_t * pHit2 ) const
{
	ARRAY_FOREACH ( i, m_dZones )
	{
		int iSpan1, iSpan2;
		if ( m_pZoneChecker->IsInZone ( m_dZones[i], pHit1, &iSpan1 )==SPH_ZONE_FOUND && m_pZoneChecker->IsInZone ( m_dZones[i], pHit2, &iSpan2 )==SPH_ZONE_FOUND )
		{
			assert ( iSpan1>=0 && iSpan2>=0 );
			if ( iSpan1==iSpan2 )
				return true;
		}
	}
	return false;
}

const ExtHit_t * ExtAndZonespanned_c::GetHitsChunk ( const ExtDoc_t * pDocs )
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
						if ( IsSameZonespan ( pCur0, pCur1 ) )
						{
							m_dHits[iHit] = *pCur0;
							if ( uNodePos0!=0 )
								m_dHits[iHit].m_uNodepos = uNodePos0;
							iHit++;
						}
						pCur0++;
						if ( pCur0->m_uDocid!=m_uMatchedDocid )
							break;
					} else
					{
						if ( IsSameZonespan ( pCur0, pCur1 ) )
						{
							m_dHits[iHit] = *pCur1;
							if ( uNodePos1!=0 )
								m_dHits[iHit].m_uNodepos = uNodePos1;
							iHit++;
						}
						pCur1++;
						if ( pCur1->m_uDocid!=m_uMatchedDocid )
							break;
					}
				}

				// our special GetDocsChunk made the things so simply, that we doesn't need to care about tail hits at all.
				// copy tail, while possible, unless the other child is at the end of a hit block
				if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid && !( pCur1 && pCur1->m_uDocid==DOCID_MAX ) )
				{
					while ( pCur0->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					{
						pCur0++;
					}
				}
				if ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid && !( pCur0 && pCur0->m_uDocid==DOCID_MAX ) )
				{
					while ( pCur1->m_uDocid==m_uMatchedDocid && iHit<MAX_HITS-1 )
					{
						pCur1++;
					}
				}
		}

		// move on
		if ( ( pCur0 && pCur0->m_uDocid!=m_uMatchedDocid && pCur0->m_uDocid!=DOCID_MAX )
			&& ( pCur1 && pCur1->m_uDocid!=m_uMatchedDocid && pCur1->m_uDocid!=DOCID_MAX ) )
			m_uMatchedDocid = 0;

		// warmup if needed
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX )
		{
			pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs );
		}
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
		{
			pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs );
		}

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

	return ReturnHitsChunk ( iHit, "and-zonespan" );
}

//////////////////////////////////////////////////////////////////////////

const ExtDoc_t * ExtOr_c::GetDocsChunk()
{
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
			pCur0 = m_pChildren[0]->GetDocsChunk();
		}
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
		{
			if ( uTouched & 2 ) break; // it was touched, so we can't advance, because child hitlist offsets would be lost
			pCur1 = m_pChildren[1]->GetDocsChunk();
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
					m_dDocs[iDoc].m_uDocFields = pCur0->m_uDocFields | pCur1->m_uDocFields; // not necessary
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

	return ReturnDocsChunk ( iDoc, "or" );
}

const ExtHit_t * ExtOr_c::GetHitsChunk ( const ExtDoc_t * pDocs )
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
				if ( ( pCur0->m_uHitpos<pCur1->m_uHitpos ) ||
					( pCur0->m_uHitpos==pCur1->m_uHitpos && pCur0->m_uQuerypos<=pCur1->m_uQuerypos ) )
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
				pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs );
				if ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid )
					continue;
			}
			if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
			{
				pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs );
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
		if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX ) pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs );
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX ) pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs );
		if ( !pCur0 && !pCur1 )
			break;

		m_uMatchedDocid = ( pCur0 && pCur1 )
			? Min ( pCur0->m_uDocid, pCur1->m_uDocid )
			: ( pCur0 ? pCur0->m_uDocid : pCur1->m_uDocid );
	}

	m_pCurHit[0] = pCur0;
	m_pCurHit[1] = pCur1;

	return ReturnHitsChunk ( iHit, "or" );
}

//////////////////////////////////////////////////////////////////////////

// returns documents from left subtree only
//
// each call returns only one document and rewinds docs in rhs to look for the
// same docID as in lhs
//
// we do this to return hits from rhs too which we need to affect match rank
const ExtDoc_t * ExtMaybe_c::GetDocsChunk()
{
	const ExtDoc_t * pCur0 = m_pCurDoc[0];
	const ExtDoc_t * pCur1 = m_pCurDoc[1];
	int iDoc = 0;

	// try to get next doc from lhs
	if ( !pCur0 || pCur0->m_uDocid==DOCID_MAX )
		pCur0 = m_pChildren[0]->GetDocsChunk();

	// we have nothing to do if there is no doc from lhs
	if ( pCur0 )
	{
		// look for same docID in rhs
		do
		{
			if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
				pCur1 = m_pChildren[1]->GetDocsChunk();
			else if ( pCur1->m_uDocid<pCur0->m_uDocid )
				++pCur1;
		} while ( pCur1 && pCur1->m_uDocid<pCur0->m_uDocid );

		m_dDocs [ iDoc ] = *pCur0;
		// alter doc like with | fulltext operator if we have it both in lhs and rhs
		if ( pCur1 && pCur0->m_uDocid==pCur1->m_uDocid )
		{
			m_dDocs [ iDoc ].m_uDocFields |= pCur1->m_uDocFields;
			m_dDocs [ iDoc ].m_fTFIDF += pCur1->m_fTFIDF;
		}
		++pCur0;
		++iDoc;
	}

	m_pCurDoc[0] = pCur0;
	m_pCurDoc[1] = pCur1;

	return ReturnDocsChunk ( iDoc, "maybe" );
}

//////////////////////////////////////////////////////////////////////////

ExtAndNot_c::ExtAndNot_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup )
	: ExtTwofer_c ( pFirst, pSecond, tSetup )
	, m_bPassthrough ( false )
{
}

const ExtDoc_t * ExtAndNot_c::GetDocsChunk()
{
	// if reject-list is over, simply pass through to accept-list
	if ( m_bPassthrough )
		return m_pChildren[0]->GetDocsChunk();

	// otherwise, do some removals
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
			pCur0 = m_pChildren[0]->GetDocsChunk();
			if ( !pCur0 )
				break;
		}

		// pull more docs from reject, if nedeed
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
			pCur1 = m_pChildren[1]->GetDocsChunk();

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

	return ReturnDocsChunk ( iDoc, "andnot" );
}

const ExtHit_t * ExtAndNot_c::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	return m_pChildren[0]->GetHitsChunk ( pDocs );
}

void ExtAndNot_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_bPassthrough = false;
	ExtTwofer_c::Reset ( tSetup );
}

//////////////////////////////////////////////////////////////////////////

ExtNWayT::ExtNWayT ( const CSphVector<ExtNode_i *> & dNodes, const ISphQwordSetup & tSetup )
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

int ExtNWayT::GetQwords ( ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	return m_pNode->GetQwords ( hQwords );
}

void ExtNWayT::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	assert ( m_pNode );
	m_pNode->SetQwordsIDF ( hQwords );
}

void ExtNWayT::GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
{
	assert ( m_pNode );
	m_pNode->GetTerms ( hQwords, dTermDupes );
}

uint64_t ExtNWayT::GetWordID() const
{
	assert ( m_pNode );
	return m_pNode->GetWordID();
}

template < class FSM >
const ExtDoc_t * ExtNWay_c<FSM>::GetDocsChunk()
{
	// initial warmup
	if ( !m_pDoc )
	{
		if ( !m_pDocs ) m_pDocs = m_pNode->GetDocsChunk();
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
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs );
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
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs );
			if ( m_pHits ) continue;

			m_uMatchedDocid = 0;

			// no more hits for current docs chunk; grab more docs
			pDoc = m_pDocs = m_pNode->GetDocsChunk();
			if ( !m_pDocs ) break;

			// we got docs, there must be hits
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs );
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
				m_dDocs[iDoc].m_uDocFields = 1<< ( HITMAN::GetField ( pHit->m_uHitpos ) ); // non necessary
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

	return ReturnDocsChunk ( iDoc, "nway" );
}

template < class FSM >
const ExtHit_t * ExtNWay_c<FSM>::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	// if we already emitted hits for this matches block, do not do that again
	SphDocID_t uFirstMatch = pDocs->m_uDocid;
	if ( uFirstMatch==m_uHitsOverFor )
		return NULL;

	// shortcuts
	const ExtDoc_t * pMyDoc = m_pMyDoc;
	const ExtHit_t * pMyHit = m_pMyHit;

	if ( !pMyDoc )
		return NULL;
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
			pHit = m_pHits = m_pNode->GetHitsChunk ( m_pDocs );
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

DWORD GetQposMask ( const CSphVector<ExtNode_i *> & dQwords )
{
	int iQposBase = dQwords[0]->m_iAtomPos;
	DWORD uQposMask = 0;
	for ( int i=1; i<dQwords.GetLength(); i++ )
	{
		int iQposDelta = dQwords[i]->m_iAtomPos - iQposBase;
		assert ( iQposDelta<(int)sizeof(uQposMask)*8 );
		uQposMask |= ( 1 << iQposDelta );
	}

	return uQposMask;
}


FSMphrase::FSMphrase ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & , const ISphQwordSetup & tSetup )
	: m_dAtomPos ( dQwords.GetLength() )
	, m_uQposMask ( 0 )
{
	ARRAY_FOREACH ( i, dQwords )
		m_dAtomPos[i] = dQwords[i]->m_iAtomPos;

	assert ( ( m_dAtomPos.Last()-m_dAtomPos[0]+1 )>0 );
	m_dQposDelta.Resize ( m_dAtomPos.Last()-m_dAtomPos[0]+1 );
	ARRAY_FOREACH ( i, m_dQposDelta )
		m_dQposDelta[i] = -INT_MAX;
	for ( int i=1; i<(int)m_dAtomPos.GetLength(); i++ )
		m_dQposDelta [ dQwords[i-1]->m_iAtomPos - dQwords[0]->m_iAtomPos ] = dQwords[i]->m_iAtomPos - dQwords[i-1]->m_iAtomPos;

	if ( tSetup.m_bSetQposMask )
		m_uQposMask = GetQposMask ( dQwords );
}

inline bool FSMphrase::HitFSM ( const ExtHit_t * pHit, ExtHit_t * pTarget )
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

			// emit directly into m_dHits, this is no need to disturb m_dMyHits here.
			pTarget->m_uDocid = pHit->m_uDocid;
			pTarget->m_uHitpos = uHitposWithField - uSpanlen;
			pTarget->m_uQuerypos = (WORD) m_dAtomPos[0];
			pTarget->m_uMatchlen = pTarget->m_uSpanlen = (WORD)( uSpanlen + 1 );
			pTarget->m_uWeight = m_dAtomPos.GetLength();
			pTarget->m_uQposMask = m_uQposMask;
			ResetFSM ();
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

FSMproximity::FSMproximity ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: m_iMaxDistance ( tNode.m_iOpArg )
	, m_uWordsExpected ( dQwords.GetLength() )
	, m_uExpPos ( 0 )
	, m_uQposMask ( 0 )
{
	assert ( m_iMaxDistance>0 );
	m_uMinQpos = dQwords[0]->m_iAtomPos;
	m_uQLen = dQwords.Last()->m_iAtomPos - m_uMinQpos;
	m_dProx.Resize ( m_uQLen+1 );
	m_dDeltas.Resize ( m_uQLen+1 );

	if ( tSetup.m_bSetQposMask )
		m_uQposMask = GetQposMask ( dQwords );
}

inline bool FSMproximity::HitFSM ( const ExtHit_t* pHit, ExtHit_t* pTarget )
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
	pTarget->m_uDocid = pHit->m_uDocid;
	pTarget->m_uHitpos = Hitpos_t ( m_dProx[m_iMinQindex] ); // !COMMIT strictly speaking this is creation from LCS not value
	pTarget->m_uQuerypos = (WORD) m_uMinQpos;
	pTarget->m_uSpanlen = pTarget->m_uMatchlen = (WORD)( uMax-m_dProx[m_iMinQindex]+1 );
	pTarget->m_uWeight = uWeight;
	pTarget->m_uQposMask = m_uQposMask;

	// remove current min, and force recompue
	m_dProx[m_iMinQindex] = UINT_MAX;
	m_iMinQindex = -1;
	m_uWords--;
	m_uExpPos = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////

FSMmultinear::FSMmultinear ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
	: m_iNear ( tNode.m_iOpArg )
	, m_uWordsExpected ( dNodes.GetLength() )
	, m_uFirstQpos ( 65535 )
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

inline bool FSMmultinear::HitFSM ( const ExtHit_t* pHit, ExtHit_t* pTarget )
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
		pTarget->m_uDocid = pHit->m_uDocid;
		pTarget->m_uHitpos = Hitpos_t ( m_uFirstHit ); // !COMMIT strictly speaking this is creation from LCS not value
		pTarget->m_uMatchlen = (WORD)( uHitposWithField - m_uFirstHit + m_uLastML );
		pTarget->m_uWeight = m_uWeight;
		m_uPrelastP = 0;

		if ( m_bTwofer ) // for exactly 2 words allow overlapping - so, just shift the chain, not reset it
		{
			pTarget->m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			pTarget->m_uSpanlen = 2;
			pTarget->m_uQposMask = ( 1 << ( Max ( m_uFirstQpos, pHit->m_uQuerypos ) - pTarget->m_uQuerypos ) );
			m_uFirstHit = m_uLastP = uHitposWithField;
			m_uWeight = pHit->m_uWeight;
			m_uFirstQpos = pHit->m_uQuerypos;
		} else
		{
			pTarget->m_uQuerypos = Min ( m_uFirstQpos, pHit->m_uQuerypos );
			pTarget->m_uSpanlen = (WORD) m_dNpos.GetLength();
			pTarget->m_uQposMask = 0;
			m_uLastP = 0;
			if ( m_bQposMask && pTarget->m_uSpanlen>1 )
			{
				ARRAY_FOREACH ( i, m_dNpos )
				{
					int iQposDelta = m_dNpos[i] - pTarget->m_uQuerypos;
					assert ( iQposDelta<(int)sizeof(pTarget->m_uQposMask)*8 );
					pTarget->m_uQposMask |= ( 1 << iQposDelta );
				}
			}
		}
		return true;
	}

	m_uLastP = uHitposWithField;
	return false;
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

ExtQuorum_c::ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
{
	assert ( tNode.GetOp()==SPH_QUERY_QUORUM );
	assert ( dQwords.GetLength()<MAX_HITS );

	m_iThresh = GetThreshold ( tNode, dQwords.GetLength() );
	m_iThresh = Max ( m_iThresh, 1 );
	m_iMyHitCount = 0;
	m_iMyLast = 0;
	m_bHasDupes = false;
	memset ( m_dQuorumHits, 0, sizeof(m_dQuorumHits) );

	assert ( dQwords.GetLength()>1 ); // use TERM instead
	assert ( dQwords.GetLength()<=256 ); // internal masks are upto 256 bits
	assert ( m_iThresh>=1 ); // 1 is also OK; it's a bit different from just OR
	assert ( m_iThresh<dQwords.GetLength() ); // use AND instead

	if ( dQwords.GetLength()>0 )
	{
		m_iAtomPos = dQwords[0]->m_iAtomPos;

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
	}

	ARRAY_FOREACH ( i, m_dInitialChildren )
	{
		m_dInitialChildren[i].m_pCurDoc = NULL;
		m_dInitialChildren[i].m_pCurHit = NULL;
	}
	m_dChildren = m_dInitialChildren;
	m_uMatchedDocid = 0;

	AllocDocinfo ( tSetup );
}

ExtQuorum_c::~ExtQuorum_c ()
{
	ARRAY_FOREACH ( i, m_dInitialChildren )
		SafeDelete ( m_dInitialChildren[i].m_pTerm );
}

void ExtQuorum_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_dChildren = m_dInitialChildren;
	m_uMatchedDocid = 0;
	m_iMyHitCount = 0;
	m_iMyLast = 0;

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
	uint64_t uHash = 0;
	ARRAY_FOREACH ( i, m_dChildren )
		uHash ^= m_dChildren[i].m_pTerm->GetWordID();

	return uHash;
}

const ExtDoc_t * ExtQuorum_c::GetDocsChunk()
{
	// warmup
	ARRAY_FOREACH ( i, m_dChildren )
	{
		TermTuple_t & tElem = m_dChildren[i];
		tElem.m_bStandStill = false; // clear this-round-match flag
		if ( tElem.m_pCurDoc && tElem.m_pCurDoc->m_uDocid!=DOCID_MAX )
			continue;

		tElem.m_pCurDoc = tElem.m_pTerm->GetDocsChunk();
		if ( tElem.m_pCurDoc )
			continue;

		m_dChildren.RemoveFast ( i );
		i--;
	}

	// main loop
	int iDoc = 0;
	CSphRowitem * pDocinfo = m_pDocinfo;
	bool bProceed2HitChunk = false;
	m_iMyHitCount = 0;
	m_iMyLast = 0;
	int iQuorumLeft = CountQuorum ( true );
	while ( iDoc<MAX_DOCS-1 && ( !m_bHasDupes || m_iMyHitCount+iQuorumLeft<MAX_HITS ) && iQuorumLeft>=m_iThresh && !bProceed2HitChunk )
	{
		// find min document ID, count occurrences
		ExtDoc_t tCand;

		tCand.m_uDocid = DOCID_MAX; // current candidate id
		tCand.m_uHitlistOffset = 0; // suppress gcc warnings
		tCand.m_pDocinfo = NULL;
		tCand.m_uDocFields = 0; // non necessary
		tCand.m_fTFIDF = 0.0f;

		int iQuorum = 0;
		ARRAY_FOREACH ( i, m_dChildren )
		{
			TermTuple_t & tElem = m_dChildren[i];
			assert ( tElem.m_pCurDoc->m_uDocid && tElem.m_pCurDoc->m_uDocid!=DOCID_MAX );
			if ( tElem.m_pCurDoc->m_uDocid < tCand.m_uDocid )
			{
				tCand = *tElem.m_pCurDoc;
				iQuorum = tElem.m_iCount;
			} else if ( tElem.m_pCurDoc->m_uDocid==tCand.m_uDocid )
			{
				tCand.m_uDocFields |= tElem.m_pCurDoc->m_uDocFields; // FIXME!!! check hits in case of dupes or field constrain
				tCand.m_fTFIDF += tElem.m_pCurDoc->m_fTFIDF;
				iQuorum += tElem.m_iCount;
			}
		}

		// FIXME!!! check that tail hits should be processed right after CollectMatchingHits
		if ( iQuorum>=m_iThresh && ( !m_bHasDupes || CollectMatchingHits ( tCand.m_uDocid, m_iThresh ) ) )
		{
			CopyExtDoc ( m_dDocs[iDoc++], tCand, &pDocinfo, m_iStride );

			// FIXME!!! move to children advancing
			ARRAY_FOREACH ( i, m_dChildren )
			{
				if ( m_dChildren[i].m_pCurDoc->m_uDocid==tCand.m_uDocid )
					m_dChildren[i].m_bStandStill = true;
			}
		}

		// advance children
		int iWasChildren = m_dChildren.GetLength();
		ARRAY_FOREACH ( i, m_dChildren )
		{
			TermTuple_t & tElem = m_dChildren[i];
			if ( tElem.m_pCurDoc->m_uDocid!=tCand.m_uDocid )
				continue;

			tElem.m_pCurDoc++;
			if ( tElem.m_pCurDoc->m_uDocid!=DOCID_MAX )
				continue;

			// should grab hits first
			if ( tElem.m_bStandStill )
			{
				bProceed2HitChunk = true;
				continue; // still should fast forward rest of children to pass current doc-id
			}

			tElem.m_pCurDoc = tElem.m_pTerm->GetDocsChunk();
			if ( tElem.m_pCurDoc )
				continue;

			m_dChildren.RemoveFast ( i );
			i--;
		}

		if ( iWasChildren!=m_dChildren.GetLength() )
			iQuorumLeft = CountQuorum ( false );
	}

	return ReturnDocsChunk ( iDoc, "quorum" );
}

const ExtHit_t * ExtQuorum_c::GetHitsChunk ( const ExtDoc_t * pDocs )
{
	// dupe tail hits
	if ( m_bHasDupes && m_uMatchedDocid )
		return GetHitsChunkDupesTail();

	// quorum-buffer path
	if ( m_bHasDupes )
		return GetHitsChunkDupes ( pDocs );

	return GetHitsChunkSimple ( pDocs );
}

const ExtHit_t * ExtQuorum_c::GetHitsChunkDupesTail ()
{
	ExtDoc_t dTailDocs[2];
	dTailDocs[0].m_uDocid = m_uMatchedDocid;
	dTailDocs[1].m_uDocid = DOCID_MAX;
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		int iMinChild = -1;
		DWORD uMinPosWithField = UINT_MAX;
		ARRAY_FOREACH ( i, m_dChildren )
		{
			const ExtHit_t * pCurHit = m_dChildren[i].m_pCurHit;
			if ( pCurHit && pCurHit->m_uDocid==m_uMatchedDocid && HITMAN::GetPosWithField ( pCurHit->m_uHitpos ) < uMinPosWithField )
			{
				uMinPosWithField = HITMAN::GetPosWithField ( pCurHit->m_uHitpos );
				iMinChild = i;
			}
		}

		// no hits found at children
		if ( iMinChild<0 )
		{
			m_uMatchedDocid = 0;
			break;
		}

		m_dHits[iHit++] = *m_dChildren[iMinChild].m_pCurHit;
		m_dChildren[iMinChild].m_pCurHit++;
		if ( m_dChildren[iMinChild].m_pCurHit->m_uDocid==DOCID_MAX )
			m_dChildren[iMinChild].m_pCurHit = m_dChildren[iMinChild].m_pTerm->GetHitsChunk ( dTailDocs );
	}

	return ReturnHitsChunk ( iHit, "quorum-dupes-tail" );
}

struct QuorumCmpHitPos_fn
{
	inline bool IsLess ( const ExtHit_t & a, const ExtHit_t & b ) const
	{
		return HITMAN::GetPosWithField ( a.m_uHitpos )<HITMAN::GetPosWithField ( b.m_uHitpos );
	}
};

const ExtHit_t * ExtQuorum_c::GetHitsChunkDupes ( const ExtDoc_t * pDocs )
{
	// quorum-buffer path
	int iHit = 0;
	while ( m_iMyLast<m_iMyHitCount && iHit<MAX_HITS-1 && pDocs->m_uDocid!=DOCID_MAX )
	{
		SphDocID_t uDocid = pDocs->m_uDocid;
		while ( m_dQuorumHits[m_iMyLast].m_uDocid<uDocid && m_iMyLast<m_iMyHitCount )
			m_iMyLast++;

		// find hits for current doc
		int iLen = 0;
		while ( m_dQuorumHits[m_iMyLast+iLen].m_uDocid==uDocid && m_iMyLast+iLen<m_iMyHitCount )
			iLen++;

		// proceed next document in case no hits found
		if ( !iLen )
		{
			pDocs++;
			continue;
		}

		// order hits by hit-position for current doc
		sphSort ( m_dQuorumHits+m_iMyLast, iLen, QuorumCmpHitPos_fn() );

		int iMyEnd = m_iMyLast + iLen;
		bool bCheckChildren = ( iMyEnd==MAX_HITS ); // should check children too in case quorum-buffer full
		while ( iHit<MAX_HITS-1 )
		{
			int iMinChild = -1;
			DWORD uMinPosWithField = ( m_iMyLast<iMyEnd ? HITMAN::GetPosWithField ( m_dQuorumHits[m_iMyLast].m_uHitpos ) : UINT_MAX );
			if ( bCheckChildren )
			{
				ARRAY_FOREACH ( i, m_dChildren )
				{
					const ExtHit_t * pCurHit = m_dChildren[i].m_pCurHit;
					if ( pCurHit && pCurHit->m_uDocid==uDocid && HITMAN::GetPosWithField ( pCurHit->m_uHitpos ) < uMinPosWithField )
					{
						uMinPosWithField = HITMAN::GetPosWithField ( pCurHit->m_uHitpos );
						iMinChild = i;
					}
				}
			}

			// no hits found at children and quorum-buffer over
			if ( iMinChild<0 && m_iMyLast==iMyEnd )
			{
				pDocs++;
				bCheckChildren = false;
				break;
			}

			if ( iMinChild<0 )
			{
				m_dHits[iHit++] = m_dQuorumHits[m_iMyLast++];
			} else
			{
				m_dHits[iHit++] = *m_dChildren[iMinChild].m_pCurHit;
				m_dChildren[iMinChild].m_pCurHit++;
				if ( m_dChildren[iMinChild].m_pCurHit->m_uDocid==DOCID_MAX )
					m_dChildren[iMinChild].m_pCurHit = m_dChildren[iMinChild].m_pTerm->GetHitsChunk ( pDocs );
			}
		}

		if ( m_iMyLast==iMyEnd && bCheckChildren ) // quorum-buffer over but children still have hits
			m_uMatchedDocid = uDocid;
	}

	return ReturnHitsChunk ( iHit, "quorum-dupes" );
}

const ExtHit_t * ExtQuorum_c::GetHitsChunkSimple ( const ExtDoc_t * pDocs )
{
	// warmup
	ARRAY_FOREACH ( i, m_dChildren )
	{
		TermTuple_t & tElem = m_dChildren[i];
		if ( !tElem.m_pCurHit || tElem.m_pCurHit->m_uDocid==DOCID_MAX )
			tElem.m_pCurHit = tElem.m_pTerm->GetHitsChunk ( pDocs );
	}

	// main loop
	int iHit = 0;
	while ( iHit<MAX_HITS-1 )
	{
		int iMinChild = -1;
		DWORD uMinPosWithField = UINT_MAX;

		if ( m_uMatchedDocid )
		{
			// emit that id while possible
			// OPTIMIZE: full linear scan for min pos and emission, eww
			ARRAY_FOREACH ( i, m_dChildren )
			{
				const ExtHit_t * pCurHit = m_dChildren[i].m_pCurHit;
				if ( pCurHit && pCurHit->m_uDocid==m_uMatchedDocid && HITMAN::GetPosWithField ( pCurHit->m_uHitpos ) < uMinPosWithField )
				{
					uMinPosWithField = HITMAN::GetPosWithField ( pCurHit->m_uHitpos ); // !COMMIT bench/fix, is LCS right here?
					iMinChild = i;
				}
			}

			if ( iMinChild<0 )
			{
				SphDocID_t uLastDoc = m_uMatchedDocid;
				m_uMatchedDocid = 0;
				while ( pDocs->m_uDocid!=DOCID_MAX && pDocs->m_uDocid<=uLastDoc )
					pDocs++;
			}
		}

		// get min common incoming docs and hits doc-id
		if ( !m_uMatchedDocid )
		{
			bool bDocMatched = false;
			while ( !bDocMatched && pDocs->m_uDocid!=DOCID_MAX )
			{
				iMinChild = -1;
				uMinPosWithField = UINT_MAX;
				ARRAY_FOREACH ( i, m_dChildren )
				{
					TermTuple_t & tElem = m_dChildren[i];
					if ( !tElem.m_pCurHit )
						continue;

					// fast forward hits
					while ( tElem.m_pCurHit->m_uDocid<pDocs->m_uDocid && tElem.m_pCurHit->m_uDocid!=DOCID_MAX )
						tElem.m_pCurHit++;

					if ( tElem.m_pCurHit->m_uDocid==pDocs->m_uDocid )
					{
						bDocMatched = true;
						if ( HITMAN::GetPosWithField ( tElem.m_pCurHit->m_uHitpos ) < uMinPosWithField )
						{
							uMinPosWithField = HITMAN::GetPosWithField ( tElem.m_pCurHit->m_uHitpos );
							iMinChild = i;
						}
					}

					// rescan current child
					if ( tElem.m_pCurHit->m_uDocid==DOCID_MAX )
					{
						tElem.m_pCurHit = tElem.m_pTerm->GetHitsChunk ( pDocs );
						i -= ( tElem.m_pCurHit ? 1 : 0 );
					}
				}

				if ( !bDocMatched )
					pDocs++;
			}

			assert ( !bDocMatched || pDocs->m_uDocid!=DOCID_MAX );
			if ( bDocMatched )
				m_uMatchedDocid = pDocs->m_uDocid;
			else
				break;
		}

		assert ( iMinChild>=0 );
		m_dHits[iHit++] = *m_dChildren[iMinChild].m_pCurHit;
		m_dChildren[iMinChild].m_pCurHit++;
		if ( m_dChildren[iMinChild].m_pCurHit->m_uDocid==DOCID_MAX )
			m_dChildren[iMinChild].m_pCurHit = m_dChildren[iMinChild].m_pTerm->GetHitsChunk ( pDocs );
	}

	return ReturnHitsChunk ( iHit, "quorum-simple" );
}

int ExtQuorum_c::GetThreshold ( const XQNode_t & tNode, int iQwords )
{
	return ( tNode.m_bPercentOp ? (int)floor ( 1.0f / 100.0f * tNode.m_iOpArg * iQwords + 0.5f ) : tNode.m_iOpArg );
}

bool ExtQuorum_c::CollectMatchingHits ( SphDocID_t uDocid, int iThreshold )
{
	assert ( m_dQuorumHits+m_iMyHitCount+CountQuorum ( false )<m_dQuorumHits+MAX_HITS ); // is there a space for all quorum hits?

	ExtHit_t * pHitBuf = m_dQuorumHits + m_iMyHitCount;
	int iQuorum = 0;
	int iTermHits = 0;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		TermTuple_t & tElem = m_dChildren[i];

		// getting more hits
		if ( !tElem.m_pCurHit || tElem.m_pCurHit->m_uDocid==DOCID_MAX )
			tElem.m_pCurHit = tElem.m_pTerm->GetHitsChunk ( tElem.m_pCurDoc );

		// that hit stream over for now
		if ( !tElem.m_pCurHit || tElem.m_pCurHit->m_uDocid==DOCID_MAX )
		{
			iTermHits = 0;
			continue;
		}

		while ( tElem.m_pCurHit->m_uDocid!=DOCID_MAX && tElem.m_pCurHit->m_uDocid<uDocid )
			tElem.m_pCurHit++;

		// collect matched hits but only up to quorum.count per-term
		while ( tElem.m_pCurHit->m_uDocid==uDocid && iTermHits<tElem.m_iCount )
		{
			*pHitBuf++ = *tElem.m_pCurHit++;
			iQuorum++;
			iTermHits++;
		}

		// got quorum - no need to check further
		if ( iQuorum>=iThreshold )
			break;

		// there might be tail hits - rescan current child
		if ( tElem.m_pCurHit->m_uDocid==DOCID_MAX )
		{
			i--;
		} else
		{
			iTermHits = 0;
		}
	}

	// discard collected hits is case of no quorum matched
	if ( iQuorum<iThreshold )
		return false;

	// collect all hits to move docs/hits further
	ARRAY_FOREACH ( i, m_dChildren )
	{
		TermTuple_t & tElem = m_dChildren[i];

		// getting more hits
		if ( !tElem.m_pCurHit || tElem.m_pCurHit->m_uDocid==DOCID_MAX )
			tElem.m_pCurHit = tElem.m_pTerm->GetHitsChunk ( tElem.m_pCurDoc );

		// hit stream over for current term
		if ( !tElem.m_pCurHit || tElem.m_pCurHit->m_uDocid==DOCID_MAX )
			continue;

		// collect all hits
		while ( tElem.m_pCurHit->m_uDocid==uDocid && pHitBuf<m_dQuorumHits+MAX_HITS )
			*pHitBuf++ = *tElem.m_pCurHit++;

		// no more space left at quorum-buffer
		if ( pHitBuf>=m_dQuorumHits+MAX_HITS )
			break;

		// there might be tail hits - rescan current child
		if ( tElem.m_pCurHit->m_uDocid==DOCID_MAX )
			i--;
	}

	m_iMyHitCount = pHitBuf - m_dQuorumHits;
	return true;
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


int ExtOrder_c::GetChildIdWithNextHit ( SphDocID_t uDocid )
{
	// OPTIMIZE! implement PQ instead of full-scan
	DWORD uMinPosWithField = UINT_MAX;
	int iChild = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		// is this child over?
		if ( !m_pHits[i] )
			continue;

		// skip until proper hit
		while ( m_pHits[i]->m_uDocid!=DOCID_MAX && m_pHits[i]->m_uDocid<uDocid )
			m_pHits[i]++;

		// hit-chunk over? request next one, and rescan
		if ( m_pHits[i]->m_uDocid==DOCID_MAX )
		{
			// hits and docs over here
			if ( !m_pDocsChunk[i] )
				return -1;

			m_pHits[i] = m_dChildren[i]->GetHitsChunk ( m_pDocsChunk[i] );
			i--;
			continue;
		}

		// is this our man at all?
		if ( m_pHits[i]->m_uDocid==uDocid )
		{
			// is he the best we can get?
			if ( HITMAN::GetPosWithField ( m_pHits[i]->m_uHitpos ) < uMinPosWithField )
			{
				uMinPosWithField = HITMAN::GetPosWithField ( m_pHits[i]->m_uHitpos );
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
		int iChild = GetChildIdWithNextHit ( uDocid );
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


const ExtDoc_t * ExtOrder_c::GetDocsChunk()
{
	if ( m_bDone )
		return NULL;

	// warm up
	ARRAY_FOREACH ( i, m_dChildren )
	{
		if ( !m_pDocs[i] ) m_pDocs[i] = m_pDocsChunk[i] = m_dChildren[i]->GetDocsChunk();
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
			assert ( m_pDocs[i] );
			while ( m_pDocs[i]->m_uDocid < uDocid )
				m_pDocs[i]++;

			// block end marker? pull next block and keep scanning
			if ( m_pDocs[i]->m_uDocid==DOCID_MAX )
			{
				m_pDocs[i] = m_pDocsChunk[i] = m_dChildren[i]->GetDocsChunk();
				if ( !m_pDocs[i] )
				{
					m_bDone = true;
					return ReturnDocsChunk ( iDoc, "order" );
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
				m_pHits[i] = m_dChildren[i]->GetHitsChunk ( m_pDocsChunk[i] );

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
			m_pDocs[0] = m_pDocsChunk[0] = m_dChildren[0]->GetDocsChunk();
			if ( !m_pDocs[0] )
			{
				m_bDone = true;
				break;
			}
		}
	}

	return ReturnDocsChunk ( iDoc, "order" );
}


const ExtHit_t * ExtOrder_c::GetHitsChunk ( const ExtDoc_t * pDocs )
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
	uint64_t uHash = 0;
	ARRAY_FOREACH ( i, m_dChildren )
		uHash ^= m_dChildren[i]->GetWordID();

	return uHash;
}

//////////////////////////////////////////////////////////////////////////

ExtUnit_c::ExtUnit_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const FieldMask_t& uFields,
	const ISphQwordSetup & tSetup, const char * sUnit )
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
	SafeDelete ( m_pDot );
}


void ExtUnit_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_pArg1->Reset ( tSetup );
	m_pArg2->Reset ( tSetup );
	m_pDot->Reset ( tSetup );

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
	uint64_t uHash = m_pArg1->GetWordID();
	uHash ^= m_pArg2->GetWordID();
	return uHash;
}

/// skips hits until their docids are less than the given limit
static inline void SkipHitsLtDocid ( const ExtHit_t * (*ppHits), SphDocID_t uMatch, ExtNode_i * pNode, const ExtDoc_t * pDocs )
{
	for ( ;; )
	{
		const ExtHit_t * pHit = *ppHits;
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
		{
			pHit = *ppHits = pNode->GetHitsChunk ( pDocs ); // OPTIMIZE? use that max?
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
			pHit = *ppHits = pNode->GetHitsChunk ( pDocs ); // OPTIMIZE? use that max?
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
				tDoc.m_uDocFields = m_pDoc1->m_uDocFields | m_pDoc2->m_uDocFields; // non necessary
				tDoc.m_uHitlistOffset = -1;
				tDoc.m_fTFIDF = m_pDoc1->m_fTFIDF + m_pDoc2->m_fTFIDF;
				tDoc.m_pDocinfo = NULL; // no inline support, sorry
				pDoc = NULL; // just once
			}

			if ( bValid1 && ( !bValid2 || m_pHit1->m_uHitpos < m_pHit2->m_uHitpos ) )
			{
				m_dMyHits[iMyHit++] = *m_pHit1++;
				if ( m_pHit1->m_uDocid==DOCID_MAX )
					m_pHit1 = m_pArg1->GetHitsChunk ( m_pDocs1 );

			} else
			{
				m_dMyHits[iMyHit++] = *m_pHit2++;
				if ( m_pHit2->m_uDocid==DOCID_MAX )
					m_pHit2 = m_pArg2->GetHitsChunk ( m_pDocs2 );
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
				if ( !SkipHitsLtePos ( &m_pDotHit, uMax, m_pDot, m_pDotDocs ) )
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
	int iMyHit = 0;

	if ( m_uTailDocid )
		SkipTailHits();

	while ( iMyHit<MAX_HITS-1 )
	{
		// fetch more candidate docs, if needed
		if ( !m_pDoc1 || m_pDoc1->m_uDocid==DOCID_MAX )
		{
			m_pDoc1 = m_pDocs1 = m_pArg1->GetDocsChunk();
			if ( !m_pDoc1 )
				break; // node is over
		}

		if ( !m_pDoc2 || m_pDoc2->m_uDocid==DOCID_MAX )
		{
			m_pDoc2 = m_pDocs2 = m_pArg2->GetDocsChunk();
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
			m_pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk();

		// skip preceding docs
		while ( m_pDotDoc && m_pDotDoc->m_uDocid < uDocid )
		{
			while ( m_pDotDoc->m_uDocid < uDocid )
				m_pDotDoc++;

			if ( m_pDotDoc->m_uDocid==DOCID_MAX )
				m_pDotDoc = m_pDotDocs = m_pDot->GetDocsChunk();
		}

		// we will need document hits on both routes below
		SkipHitsLtDocid ( &m_pHit1, uDocid, m_pArg1, m_pDocs1 );
		SkipHitsLtDocid ( &m_pHit2, uDocid, m_pArg2, m_pDocs2 );
		assert ( m_pHit1 && m_pHit1->m_uDocid==uDocid );
		assert ( m_pHit2 && m_pHit2->m_uDocid==uDocid );

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

			return ReturnDocsChunk ( iDoc, iMyHit );
		}

		// all hits copied; do the next candidate
		m_pDoc1++;
		m_pDoc2++;
	}

	return ReturnDocsChunk ( iDoc, iMyHit );
}


const ExtHit_t * ExtUnit_c::GetHitsChunk ( const ExtDoc_t * pDocs )
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

static void Explain ( const XQNode_t * pNode, const CSphSchema & tSchema, const CSphVector<CSphString> & dZones,
	CSphStringBuilder & tRes, int iIdent )
{
	if ( iIdent )
		tRes.Appendf ( "\n" );
	for ( int i=0; i<iIdent; i++ )
		tRes.Appendf ( "  " );
	switch ( pNode->GetOp() )
	{
		case SPH_QUERY_AND:			tRes.Appendf ( "AND(" ); break;
		case SPH_QUERY_OR:			tRes.Appendf ( "OR(" ); break;
		case SPH_QUERY_MAYBE:		tRes.Appendf ( "MAYBE(" ); break;
		case SPH_QUERY_NOT:			tRes.Appendf ( "NOT(" ); break;
		case SPH_QUERY_ANDNOT:		tRes.Appendf ( "ANDNOT(" ); break;
		case SPH_QUERY_BEFORE:		tRes.Appendf ( "BEFORE(" ); break;
		case SPH_QUERY_PHRASE:		tRes.Appendf ( "PHRASE(" ); break;
		case SPH_QUERY_PROXIMITY:	tRes.Appendf ( "PROXIMITY(distance=%d, ", pNode->m_iOpArg ); break;
		case SPH_QUERY_QUORUM:		tRes.Appendf ( "QUORUM(count=%d, ", pNode->m_iOpArg ); break;
		case SPH_QUERY_NEAR:		tRes.Appendf ( "NEAR(distance=%d", pNode->m_iOpArg ); break;
		case SPH_QUERY_SENTENCE:	tRes.Appendf ( "SENTENCE(" ); break;
		case SPH_QUERY_PARAGRAPH:	tRes.Appendf ( "PARAGRAPH(" ); break;
		default:					tRes.Appendf ( "OPERATOR-%d(", pNode->GetOp() ); break;
	}

	if ( pNode->m_dChildren.GetLength() && pNode->m_dWords.GetLength() )
		tRes.Appendf("virtually-plain, ");

	// dump spec for keyword nodes
	// FIXME? double check that spec does *not* affect non keyword nodes
	if ( !pNode->m_dSpec.IsEmpty() && pNode->m_dWords.GetLength() )
	{
		const XQLimitSpec_t & s = pNode->m_dSpec;
		if ( s.m_bFieldSpec && !s.m_dFieldMask.TestAll ( true ) )
		{
			tRes.Appendf ( "fields=(" );
			bool bNeedComma = false;
			ARRAY_FOREACH ( i, tSchema.m_dFields )
				if ( s.m_dFieldMask.Test(i) )
				{
					if ( bNeedComma )
						tRes.Appendf ( ", " );
					bNeedComma = true;
					tRes.Appendf ( "%s", tSchema.m_dFields[i].m_sName.cstr() );
				}
			tRes.Appendf ( "), " );
		}

		if ( s.m_iFieldMaxPos )
			tRes.Appendf ( "max_field_pos=%d, ", s.m_iFieldMaxPos );

		if ( s.m_dZones.GetLength() )
		{
			tRes.Appendf ( s.m_bZoneSpan ? "zonespans=(" : "zones=(" );
			bool bNeedComma = false;
			ARRAY_FOREACH ( i, s.m_dZones )
			{
				if ( bNeedComma )
					tRes.Appendf ( ", " );
				bNeedComma = true;
				tRes.Appendf ( "%s", dZones [ s.m_dZones[i] ].cstr() );
			}
			tRes.Appendf ( "), " );
		}
	}

	if ( pNode->m_dChildren.GetLength() )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
		{
			if ( i>0 )
				tRes.Appendf ( ", " );
			Explain ( pNode->m_dChildren[i], tSchema, dZones, tRes, iIdent+1 );
		}
	} else
	{
		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			const XQKeyword_t & w = pNode->m_dWords[i];
			if ( i>0 )
				tRes.Appendf(", ");
			tRes.Appendf ( "KEYWORD(%s, querypos=%d", w.m_sWord.cstr(), w.m_iAtomPos );
			if ( w.m_bExcluded )
				tRes.Appendf ( ", excluded" );
			if ( w.m_bExpanded )
				tRes.Appendf ( ", expanded" );
			if ( w.m_bFieldStart )
				tRes.Appendf ( ", field_start" );
			if ( w.m_bFieldEnd )
				tRes.Appendf ( ", field_end" );
			if ( w.m_bMorphed )
				tRes.Appendf ( ", morphed" );
			if ( w.m_fBoost!=1.0f ) // really comparing floats?
				tRes.Appendf ( ", boost=%f", w.m_fBoost );
			tRes.Appendf ( ")" );
		}
	}
	tRes.Appendf(")");
}


ExtRanker_c::ExtRanker_c ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup )
	: m_dZoneInfo ( 0 )
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

	// we generally have three (!) trees for each query
	// 1) parsed tree, a raw result of query parsing
	// 2) transformed tree, with star expansions, morphology, and other transfomations
	// 3) evaluation tree, with tiny keywords cache, and other optimizations
	// tXQ.m_pRoot, passed to ranker from the index, is the transformed tree
	// m_pRoot, internal to ranker, is the evaluation tree
	if ( tSetup.m_pCtx->m_pProfile )
	{
		tSetup.m_pCtx->m_pProfile->m_sTransformedTree.Clear();
		Explain ( tXQ.m_pRoot, tSetup.m_pIndex->GetMatchSchema(), tXQ.m_dZones,
			tSetup.m_pCtx->m_pProfile->m_sTransformedTree, 0 );
	}

	m_pDoclist = NULL;
	m_pHitlist = NULL;
	m_uPayloadMask = 0;
	m_iQwords = 0;
	m_pIndex = tSetup.m_pIndex;
	m_pCtx = tSetup.m_pCtx;
	m_pNanoBudget = tSetup.m_pStats ? tSetup.m_pStats->m_pNanoBudget : NULL;

	m_dZones = tXQ.m_dZones;
	m_dZoneStart.Resize ( m_dZones.GetLength() );
	m_dZoneEnd.Resize ( m_dZones.GetLength() );
	m_dZoneMax.Resize ( m_dZones.GetLength() );
	m_dZoneMin.Resize ( m_dZones.GetLength() );
	m_dZoneMax.Fill ( 0 );
	m_dZoneMin.Fill	( DOCID_MAX );
	m_bZSlist = tXQ.m_bNeedSZlist;
	m_dZoneInfo.Reset ( m_dZones.GetLength() );

	CSphDict * pZonesDict = NULL;
	// workaround for a particular case with following conditions
	if ( !m_pIndex->GetDictionary()->GetSettings().m_bWordDict && m_dZones.GetLength() )
		pZonesDict = m_pIndex->GetDictionary()->Clone();

	ARRAY_FOREACH ( i, m_dZones )
	{
		XQKeyword_t tDot;

		tDot.m_sWord.SetSprintf ( "%c%s", MAGIC_CODE_ZONE, m_dZones[i].cstr() );
		m_dZoneStartTerm.Add ( new ExtTerm_c ( CreateQueryWord ( tDot, tSetup, pZonesDict ), tSetup ) );
		m_dZoneStart[i] = NULL;

		tDot.m_sWord.SetSprintf ( "%c/%s", MAGIC_CODE_ZONE, m_dZones[i].cstr() );
		m_dZoneEndTerm.Add ( new ExtTerm_c ( CreateQueryWord ( tDot, tSetup, pZonesDict ), tSetup ) );
		m_dZoneEnd[i] = NULL;
	}

	SafeDelete ( pZonesDict );
}


ExtRanker_c::~ExtRanker_c ()
{
	SafeDelete ( m_pRoot );
	ARRAY_FOREACH ( i, m_dZones )
	{
		SafeDelete ( m_dZoneStartTerm[i] );
		SafeDelete ( m_dZoneEndTerm[i] );
	}

	ARRAY_FOREACH ( i, m_dZoneInfo )
	{
		ARRAY_FOREACH ( iDoc, m_dZoneInfo[i] )
		{
			SafeDelete ( m_dZoneInfo[i][iDoc].m_pHits );
		}
		m_dZoneInfo[i].Reset();
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
	ARRAY_FOREACH ( i, m_dZoneInfo )
	{
		ARRAY_FOREACH ( iDoc, m_dZoneInfo[i] )
			SafeDelete ( m_dZoneInfo[i][iDoc].m_pHits );
		m_dZoneInfo[i].Reset();
	}
}


const ExtDoc_t * ExtRanker_c::GetFilteredDocs ()
{
	#if QDEBUG
	printf ( "ranker getfiltereddocs" );
	#endif

	ESphQueryState eState = SPH_QSTATE_TOTAL;
	CSphQueryProfile * pProfile = m_pCtx->m_pProfile;

	for ( ;; )
	{
		// get another chunk
		if ( pProfile )
			eState = pProfile->Switch ( SPH_QSTATE_GET_DOCS );
		const ExtDoc_t * pCand = m_pRoot->GetDocsChunk();
		if ( !pCand )
		{
			if ( pProfile )
				pProfile->Switch ( eState );
			return NULL;
		}

		// create matches, and filter them
		if ( pProfile )
			pProfile->Switch ( SPH_QSTATE_FILTER );
		int iDocs = 0;
		SphDocID_t uMaxID = 0;
		while ( pCand->m_uDocid!=DOCID_MAX )
		{
			m_tTestMatch.m_uDocID = pCand->m_uDocid;
			m_tTestMatch.m_pStatic = NULL;
			if ( pCand->m_pDocinfo )
				memcpy ( m_tTestMatch.m_pDynamic, pCand->m_pDocinfo, m_iInlineRowitems*sizeof(CSphRowitem) );

			if ( m_pIndex->EarlyReject ( m_pCtx, m_tTestMatch ) )
			{
				pCand++;
				continue;
			}

			uMaxID = pCand->m_uDocid;
			m_dMyDocs[iDocs] = *pCand;
			m_tTestMatch.m_iWeight = (int)( (pCand->m_fTFIDF+0.5f)*SPH_BM25_SCALE ); // FIXME! bench bNeedBM25
			Swap ( m_tTestMatch, m_dMyMatches[iDocs] );
			iDocs++;
			pCand++;
		}

		// clean up zone hash
		if ( !m_bZSlist )
			CleanupZones ( uMaxID );

		if ( iDocs )
		{
			if ( m_pNanoBudget )
				*m_pNanoBudget -= g_iPredictorCostMatch*iDocs;
			m_dMyDocs[iDocs].m_uDocid = DOCID_MAX;
			if ( pProfile )
				pProfile->Switch ( eState );

			#if QDEBUG
			CSphStringBuilder tRes;
			tRes.Appendf ( "matched %p docs (%d) = [", this, iDocs );
			for ( int i=0; i<iDocs; i++ )
				tRes.Appendf ( i ? ", 0x%x" : "0x%x", DWORD ( m_dMyDocs[i].m_uDocid ) );
			tRes.Appendf ( "]" );
			printf ( "%s", tRes.cstr() );
			#endif

			return m_dMyDocs;
		}
	}
}

void ExtRanker_c::CleanupZones ( SphDocID_t uMaxDocid )
{
	if ( !uMaxDocid )
		return;

	ARRAY_FOREACH ( i, m_dZoneMin )
	{
		SphDocID_t uMinDocid = m_dZoneMin[i];
		if ( uMinDocid==DOCID_MAX )
			continue;

		CSphVector<ZoneInfo_t> & dZone = m_dZoneInfo[i];
		int iSpan = FindSpan ( dZone, uMaxDocid );
		if ( iSpan==-1 )
			continue;

		if ( iSpan==dZone.GetLength()-1 )
		{
			ARRAY_FOREACH ( iDoc, dZone )
				SafeDelete ( dZone[iDoc].m_pHits );
			dZone.Resize ( 0 );
			m_dZoneMin[i] = uMaxDocid;
			continue;
		}

		for ( int iDoc=0; iDoc<=iSpan; iDoc++ )
			SafeDelete ( dZone[iDoc].m_pHits );

		int iLen = dZone.GetLength() - iSpan - 1;
		memmove ( dZone.Begin(), dZone.Begin()+iSpan+1, sizeof(dZone[0]) * iLen );
		dZone.Resize ( iLen );
		m_dZoneMin[i] = dZone.Begin()->m_uDocid;
	}
}


void ExtRanker_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_iQwords = hQwords.GetLength ();
	if ( m_pRoot )
		m_pRoot->SetQwordsIDF ( hQwords );
}


static SphZoneHit_e ZoneCacheFind ( const ZoneVVector_t & dZones, int iZone, const ExtHit_t * pHit, int * pLastSpan )
{
	if ( !dZones[iZone].GetLength() )
		return SPH_ZONE_NO_DOCUMENT;

	ZoneInfo_t * pZone = sphBinarySearch ( dZones[iZone].Begin(), &dZones[iZone].Last(), bind ( &ZoneInfo_t::m_uDocid ), pHit->m_uDocid );
	if ( !pZone )
		return SPH_ZONE_NO_DOCUMENT;

	if ( pZone )
	{
		// remove end markers that might mess up ordering
		Hitpos_t uPosWithField = HITMAN::GetPosWithField ( pHit->m_uHitpos );
		int iSpan = FindSpan ( pZone->m_pHits->m_dStarts, uPosWithField );
		if ( iSpan<0 || uPosWithField>pZone->m_pHits->m_dEnds[iSpan] )
			return SPH_ZONE_NO_SPAN;
		if ( pLastSpan )
			*pLastSpan = iSpan;
		return SPH_ZONE_FOUND;
	}

	return SPH_ZONE_NO_DOCUMENT;
}


SphZoneHit_e ExtRanker_c::IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan )
{
	// quick route, we have current docid cached
	SphZoneHit_e eRes = ZoneCacheFind ( m_dZoneInfo, iZone, pHit, pLastSpan );
	if ( eRes!=SPH_ZONE_NO_DOCUMENT )
		return eRes;

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
			pStart = m_dZoneStartTerm[iZone]->GetDocsChunk();
			if ( !pStart )
			{
				m_dZoneMax[iZone] = DOCID_MAX;
				return SPH_ZONE_NO_DOCUMENT;
			}
		}

		if ( ( !pEnd && m_dZoneMax[iZone]!=DOCID_MAX ) || pEnd->m_uDocid==DOCID_MAX )
		{
			pEnd = m_dZoneEndTerm[iZone]->GetDocsChunk();
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
		const ExtHit_t * pStartHits = m_dZoneStartTerm[iZone]->GetHitsChunk ( dCache );
		const ExtHit_t * pEndHits = m_dZoneEndTerm[iZone]->GetHitsChunk ( dCache );
		int iReserveStart = m_dZoneStartTerm[iZone]->GetHitsCount() / Max ( m_dZoneStartTerm[iZone]->GetDocsCount(), 1 );
		int iReserveEnd = m_dZoneEndTerm[iZone]->GetHitsCount() / Max ( m_dZoneEndTerm[iZone]->GetDocsCount(), 1 );
		int iReserve = Max ( iReserveStart, iReserveEnd );

		// loop documents one by one
		while ( pStartHits && pEndHits )
		{
			// load all hits for current document
			SphDocID_t uCur = pStartHits->m_uDocid;

			// FIXME!!! replace by iterate then add elements to vector instead of searching each time
			ZoneHits_t * pZone = NULL;
			CSphVector<ZoneInfo_t> & dZones = m_dZoneInfo[iZone];
			if ( dZones.GetLength() )
			{
				ZoneInfo_t * pInfo = sphBinarySearch ( dZones.Begin(), &dZones.Last(), bind ( &ZoneInfo_t::m_uDocid ), uCur );
				if ( pInfo )
					pZone = pInfo->m_pHits;
			}
			if ( !pZone )
			{
				if ( dZones.GetLength() && dZones.Last().m_uDocid>uCur )
				{
					int iInsertPos = FindSpan ( dZones, uCur );
					assert ( iInsertPos>=0 );
					dZones.Insert ( iInsertPos, ZoneInfo_t() );
					dZones[iInsertPos].m_uDocid = uCur;
					pZone = dZones[iInsertPos].m_pHits = new ZoneHits_t();
				} else
				{
					ZoneInfo_t & tElem = dZones.Add ();
					tElem.m_uDocid = uCur;
					pZone = tElem.m_pHits = new ZoneHits_t();
				}
				pZone->m_dStarts.Reserve ( iReserve );
				pZone->m_dEnds.Reserve ( iReserve );
			}

			assert ( pEndHits->m_uDocid==uCur );

			// load all the pairs of start and end hits for it
			// do it by with the FSM:
			//
			// state 'begin':
			// - start marker -> set state 'inspan', startspan=pos++
			// - end marker -> pos++
			// - end of doc -> set state 'finish'
			//
			// state 'inspan':
			// - start marker -> startspan = pos++
			// - end marker -> set state 'outspan', endspan=pos++
			// - end of doc -> set state 'finish'
			//
			// state 'outspan':
			// - start marker -> set state 'inspan', commit span, startspan=pos++
			// - end marker -> endspan = pos++
			// - end of doc -> set state 'finish', commit span
			//
			// state 'finish':
			// - we are done.

			int bEofDoc = 0;

			// state 'begin' is here.
			while ( !bEofDoc && pEndHits->m_uHitpos < pStartHits->m_uHitpos )
			{
				++pEndHits;
				bEofDoc |= (pEndHits->m_uDocid!=uCur)?2:0;
			}

			if ( !bEofDoc )
			{
				// state 'inspan' (true) or 'outspan' (false)
				bool bInSpan = true;
				Hitpos_t iSpanBegin = pStartHits->m_uHitpos;
				Hitpos_t iSpanEnd = pEndHits->m_uHitpos;
				while ( bEofDoc!=3 ) /// action end-of-doc
				{
					// action inspan/start-marker
					if ( bInSpan )
					{
						++pStartHits;
						bEofDoc |= (pStartHits->m_uDocid!=uCur)?1:0;
					} else
						// action outspan/end-marker
					{
						++pEndHits;
						bEofDoc |= (pEndHits->m_uDocid!=uCur)?2:0;
					}

					if ( pStartHits->m_uHitpos<pEndHits->m_uHitpos && !( bEofDoc & 1 ) )
					{
						// actions for outspan/start-marker state
						// <b>...<b>..<b>..</b> will ignore all the <b> inside.
						if ( !bInSpan )
						{
							bInSpan = true;
							pZone->m_dStarts.Add ( iSpanBegin );
							pZone->m_dEnds.Add ( iSpanEnd );
							iSpanBegin = pStartHits->m_uHitpos;
						}
					} else if ( !( bEofDoc & 2 ) )
					{
						// actions for inspan/end-marker state
						// so, <b>...</b>..</b>..</b> will ignore all the </b> inside.
						bInSpan = false;
						iSpanEnd = pEndHits->m_uHitpos;
					}
				}
				// action 'commit' for outspan/end-of-doc
				if ( iSpanBegin < iSpanEnd )
				{
					pZone->m_dStarts.Add ( iSpanBegin );
					pZone->m_dEnds.Add ( iSpanEnd );
				}

				if ( pStartHits->m_uDocid==DOCID_MAX )
					pStartHits = m_dZoneStartTerm[iZone]->GetHitsChunk ( dCache );
				if ( pEndHits->m_uDocid==DOCID_MAX )
					pEndHits = m_dZoneEndTerm[iZone]->GetHitsChunk ( dCache );
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
	return ZoneCacheFind ( m_dZoneInfo, iZone, pHit, pLastSpan );
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
		DWORD uMask = pDoc->m_uDocFields;
		if ( !uMask )
		{
			// possible if we have more than 32 fields
			// honestly loading all hits etc is cumbersome, so let's just fake it
			uRank = 1;
		} else
		{
			// just sum weights over the lowest 32 fields
			int iWeights = Min ( m_iWeights, 32 );
			for ( int i=0; i<iWeights; i++ )
				if ( pDoc->m_uDocFields & (1<<i) )
					uRank += m_pWeights[i];
		}

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
ExtRanker_T<STATE>::ExtRanker_T ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup )
	: ExtRanker_c ( tXQ, tSetup )
{
	// FIXME!!! move out the disable of m_bZSlist in case no zonespan nodes
	if ( m_bZSlist )
		m_dZonespans.Reserve ( ExtNode_i::MAX_DOCS * m_dZones.GetLength() );

	m_pHitBase = NULL;
}


static inline const ExtHit_t * RankerGetHits ( CSphQueryProfile * pProfile, ExtNode_i * pRoot, const ExtDoc_t * pDocs )
{
	if ( !pProfile )
		return pRoot->GetHitsChunk ( pDocs );

	pProfile->Switch ( SPH_QSTATE_GET_HITS );
	const ExtHit_t * pHlist = pRoot->GetHitsChunk ( pDocs );
	pProfile->Switch ( SPH_QSTATE_RANK );
	return pHlist;
}


template < typename STATE >
int ExtRanker_T<STATE>::GetMatches ()
{
	if ( !m_pRoot )
		return 0;

	CSphQueryProfile * pProfile = m_pCtx->m_pProfile;
	int iMatches = 0;
	const ExtHit_t * pHlist = m_pHitlist;
	const ExtHit_t * pHitBase = m_pHitBase;
	const ExtDoc_t * pDocs = m_pDoclist;
	m_dZonespans.Resize(1);
	int	iLastZoneData = 0;

	CSphVector<int> dSpans;
	if ( m_bZSlist )
	{
		dSpans.Resize ( m_dZones.GetLength() );
		dSpans.Fill ( -1 );
	}

	// warmup if necessary
	if ( !pHlist )
	{
		if ( !pDocs )
			pDocs = GetFilteredDocs ();
		if ( !pDocs )
			return iMatches;

		pHlist = RankerGetHits ( pProfile, m_pRoot, pDocs );
		if ( !pHlist )
			return iMatches;
	}

	if ( !pHitBase )
		pHitBase = pHlist;

	// main matching loop
	const ExtDoc_t * pDoc = pDocs;
	for ( SphDocID_t uCurDocid=0; iMatches<ExtNode_i::MAX_DOCS; )
	{
		// keep ranking
		while ( pHlist->m_uDocid==uCurDocid )
		{
			m_tState.Update ( pHlist );
			if ( m_bZSlist )
			{
				ARRAY_FOREACH ( i, m_dZones )
				{
					int iSpan;
					if ( IsInZone ( i, pHlist, &iSpan )!=SPH_ZONE_FOUND )
						continue;

					if ( iSpan!=dSpans[i] )
					{
						m_dZonespans.Add ( i );
						m_dZonespans.Add ( iSpan );
						dSpans[i] = iSpan;
					}
				}
			}
			++pHlist;
		}

		// if hits block is over, get next block, but do *not* flush current doc
		if ( pHlist->m_uDocid==DOCID_MAX )
		{
			assert ( pDocs );
			pHlist = RankerGetHits ( pProfile, m_pRoot, pDocs );
			if ( pHlist )
				continue;
		}

		// otherwise (new match or no next hits block), flush current doc
		if ( uCurDocid )
		{
			assert ( uCurDocid==pDoc->m_uDocid );
			Swap ( m_dMatches[iMatches], m_dMyMatches[pDoc-m_dMyDocs] );
			m_dMatches[iMatches].m_iWeight = m_tState.Finalize ( m_dMatches[iMatches] );
			if ( m_bZSlist )
			{
				m_dZonespans[iLastZoneData] = m_dZonespans.GetLength() - iLastZoneData - 1;
				m_dMatches[iMatches].m_iTag = iLastZoneData;

				iLastZoneData = m_dZonespans.GetLength();
				m_dZonespans.Add(0);

				dSpans.Fill ( -1 );
			}
			iMatches++;
		}

		// boundary checks
		if ( !pHlist )
		{
			if ( m_bZSlist && uCurDocid )
				CleanupZones ( uCurDocid );

			// there are no more hits for current docs block; do we have a next one?
			assert ( pDocs );
			pDoc = pDocs = GetFilteredDocs ();

			// we don't, so bail out
			if ( !pDocs )
				break;

			// we do, get some hits
			pHlist = m_pRoot->GetHitsChunk ( pDocs );
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
	if ( !m_pHitBase )
		m_pHitBase = pHitBase;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

template < bool USE_BM25, bool HANDLE_DUPES >
struct RankerState_Proximity_fn : public ISphExtra
{
	BYTE m_uLCS[SPH_MAX_FIELDS];
	BYTE m_uCurLCS;
	int m_iExpDelta;
	int m_iLastHitPosWithField;
	int m_iFields;
	const int * m_pWeights;

	DWORD m_uLcsTailPos;
	DWORD m_uLcsTailQposMask;
	DWORD m_uCurQposMask;
	DWORD m_uCurPos;

	bool Init ( int iFields, const int * pWeights, ExtRanker_c *, CSphString &, DWORD )
	{
		memset ( m_uLCS, 0, sizeof(m_uLCS) );
		m_uCurLCS = 0;
		m_iExpDelta = -INT_MAX;
		m_iLastHitPosWithField = -INT_MAX;
		m_iFields = iFields;
		m_pWeights = pWeights;

		m_uLcsTailPos = 0;
		m_uLcsTailQposMask = 0;
		m_uCurQposMask = 0;
		m_uCurPos = 0;

		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		if_const ( !HANDLE_DUPES )
		{
			// all query keywords are unique
			// simpler path (just do the delta)
			const int iPosWithField = HITMAN::GetPosWithField ( pHlist->m_uHitpos );
			int iDelta = iPosWithField - pHlist->m_uQuerypos;
			if ( iPosWithField>m_iLastHitPosWithField )
				m_uCurLCS = ( ( iDelta==m_iExpDelta ) ? m_uCurLCS : 0 ) + BYTE(pHlist->m_uWeight);

			DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
			if ( m_uCurLCS>m_uLCS[uField] )
				m_uLCS[uField] = m_uCurLCS;

			m_iLastHitPosWithField = iPosWithField;
			m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1; // !COMMIT why spanlen??
		} else
		{
			// keywords are duplicated in the query
			// so there might be multiple qpos entries sharing the same hitpos
			DWORD uPos = HITMAN::GetPosWithField ( pHlist->m_uHitpos );
			DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );

			// reset accumulated data from previous field
			if ( (DWORD)HITMAN::GetField ( m_uCurPos )!=uField )
				m_uCurQposMask = 0;

			if ( uPos!=m_uCurPos )
			{
				// next new and shiny hitpos in line
				// FIXME!? what do we do with longer spans? keep looking? reset?
				if ( m_uCurLCS<2 )
				{
					m_uLcsTailPos = m_uCurPos;
					m_uLcsTailQposMask = m_uCurQposMask;
					m_uCurLCS = 1; // FIXME!? can this ever be different? ("a b" c) maybe..
				}
				m_uCurQposMask = 0;
				m_uCurPos = uPos;
				if ( m_uLCS[uField] < pHlist->m_uWeight )
					m_uLCS[uField] = BYTE(pHlist->m_uWeight);
			}

			// add that qpos to current qpos mask (for the current hitpos)
			m_uCurQposMask |= ( 1UL << pHlist->m_uQuerypos );

			// and check if that results in a better lcs match now
			int iDelta = m_uCurPos - m_uLcsTailPos;
			if ( ( m_uCurQposMask >> iDelta ) & m_uLcsTailQposMask )
			{
				// cool, it matched!
				m_uLcsTailQposMask = ( 1UL << pHlist->m_uQuerypos ); // our lcs span now ends with a specific qpos
				m_uLcsTailPos = m_uCurPos; // and in a specific position
				m_uCurLCS = BYTE ( m_uCurLCS + pHlist->m_uWeight ); // and it's longer
				m_uCurQposMask = 0; // and we should avoid matching subsequent hits on the same hitpos

				// update per-field vector
				if ( m_uCurLCS>m_uLCS[uField] )
					m_uLCS[uField] = m_uCurLCS;
			}
		}
	}

	DWORD Finalize ( const CSphMatch & tMatch )
	{
		m_uCurLCS = 0;
		m_iExpDelta = -1;
		m_iLastHitPosWithField = -1;

		if_const ( HANDLE_DUPES )
		{
			m_uLcsTailPos = 0;
			m_uLcsTailQposMask = 0;
			m_uCurQposMask = 0;
			m_uCurPos = 0;
		}

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
struct RankerState_ProximityBM25Exact_fn : public ISphExtra
{
	BYTE m_uLCS[SPH_MAX_FIELDS];
	BYTE m_uCurLCS;
	int m_iExpDelta;
	int m_iLastHitPos;
	DWORD m_uMinExpPos;
	int m_iFields;
	const int * m_pWeights;
	DWORD m_uHeadHit;
	DWORD m_uExactHit;
	int m_iMaxQuerypos;

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString &, DWORD )
	{
		memset ( m_uLCS, 0, sizeof(m_uLCS) );
		m_uCurLCS = 0;
		m_iExpDelta = -INT_MAX;
		m_iLastHitPos = -1;
		m_uMinExpPos = 0;
		m_iFields = iFields;
		m_pWeights = pWeights;
		m_uHeadHit = 0;
		m_uExactHit = 0;

		// tricky bit
		// in expr and export rankers, this gets handled by the overridden (!) SetQwordsIDF()
		// but in all the other ones, we need this, because SetQwordsIDF() won't touch the state by default
		// FIXME? this is actually MaxUniqueQpos, queries like [foo|foo|foo] might break
		m_iMaxQuerypos = pRanker->m_iMaxQpos;
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		// upd LCS
		DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
		int iPosWithField = HITMAN::GetPosWithField ( pHlist->m_uHitpos );
		int iDelta = iPosWithField - pHlist->m_uQuerypos;

		if ( iDelta==m_iExpDelta && HITMAN::GetPosWithField ( pHlist->m_uHitpos )>=m_uMinExpPos )
		{
			if ( iPosWithField>m_iLastHitPos )
				m_uCurLCS = (BYTE)( m_uCurLCS + pHlist->m_uWeight );
			if ( HITMAN::IsEnd ( pHlist->m_uHitpos )
				&& (int)pHlist->m_uQuerypos==m_iMaxQuerypos
				&& HITMAN::GetPos ( pHlist->m_uHitpos )==m_iMaxQuerypos )
			{
				m_uExactHit |= ( 1UL << HITMAN::GetField ( pHlist->m_uHitpos ) );
			}
		} else
		{
			if ( iPosWithField>m_iLastHitPos )
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
		m_iLastHitPos = iPosWithField;
		m_uMinExpPos = HITMAN::GetPosWithField ( pHlist->m_uHitpos ) + 1;
	}

	DWORD Finalize ( const CSphMatch & tMatch )
	{
		m_uCurLCS = 0;
		m_iExpDelta = -1;
		m_iLastHitPos = -1;

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
struct RankerState_ProximityPayload_fn : public RankerState_Proximity_fn<USE_BM25,false>
{
	DWORD m_uPayloadRank;
	DWORD m_uPayloadMask;

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError, DWORD )
	{
		RankerState_Proximity_fn<USE_BM25,false>::Init ( iFields, pWeights, pRanker, sError, false );
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
			RankerState_Proximity_fn<USE_BM25,false>::Update ( pHlist );
	}

	DWORD Finalize ( const CSphMatch & tMatch )
	{
		// as usual, redundant 'this' is just because gcc is stupid
		this->m_uCurLCS = 0;
		this->m_iExpDelta = -1;
		this->m_iLastHitPosWithField = -1;

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

struct RankerState_MatchAny_fn : public RankerState_Proximity_fn<false,false>
{
	int m_iPhraseK;
	BYTE m_uMatchMask[SPH_MAX_FIELDS];

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError, DWORD )
	{
		RankerState_Proximity_fn<false,false>::Init ( iFields, pWeights, pRanker, sError, false );
		m_iPhraseK = 0;
		for ( int i=0; i<iFields; i++ )
			m_iPhraseK += pWeights[i] * pRanker->m_iQwords;
		memset ( m_uMatchMask, 0, sizeof(m_uMatchMask) );
		return true;
	}

	void Update ( const ExtHit_t * pHlist )
	{
		RankerState_Proximity_fn<false,false>::Update ( pHlist );
		m_uMatchMask [ HITMAN::GetField ( pHlist->m_uHitpos ) ] |= ( 1<<(pHlist->m_uQuerypos-1) );
	}

	DWORD Finalize ( const CSphMatch & )
	{
		m_uCurLCS = 0;
		m_iExpDelta = -1;
		m_iLastHitPosWithField = -1;

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

struct RankerState_Wordcount_fn : public ISphExtra
{
	DWORD m_uRank;
	int m_iFields;
	const int * m_pWeights;

	bool Init ( int iFields, const int * pWeights, ExtRanker_c *, CSphString &, DWORD )
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

struct RankerState_Fieldmask_fn : public ISphExtra
{
	DWORD m_uRank;

	bool Init ( int, const int *, ExtRanker_c *, CSphString &, DWORD )
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

struct RankerState_Plugin_fn : public ISphExtra
{
	RankerState_Plugin_fn()
		: m_pData ( NULL )
		, m_pPlugin ( NULL )
	{}

	~RankerState_Plugin_fn()
	{
		assert ( m_pPlugin );
		if ( m_pPlugin->m_fnDeinit )
			m_pPlugin->m_fnDeinit ( m_pData );
		m_pPlugin->Release();
	}

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError, DWORD )
	{
		if ( !m_pPlugin->m_fnInit )
			return true;

		SPH_RANKER_INIT r;
		r.num_field_weights = iFields;
		r.field_weights = const_cast<int*>(pWeights);
		r.options = m_sOptions.cstr();
		r.payload_mask = pRanker->m_uPayloadMask;
		r.num_query_words = pRanker->m_iQwords;
		r.max_qpos = pRanker->m_iMaxQpos;

		char sErrorBuf [ SPH_UDF_ERROR_LEN ];
		if ( m_pPlugin->m_fnInit ( &m_pData, &r, sErrorBuf )==0 )
			return true;
		sError = sErrorBuf;
		return false;
	}

	void Update ( const ExtHit_t * p )
	{
		if ( !m_pPlugin->m_fnUpdate )
			return;

		SPH_RANKER_HIT h;
		h.doc_id = p->m_uDocid;
		h.hit_pos = p->m_uHitpos;
		h.query_pos = p->m_uQuerypos;
		h.node_pos = p->m_uNodepos;
		h.span_length = p->m_uSpanlen;
		h.match_length = p->m_uMatchlen;
		h.weight = p->m_uWeight;
		h.query_pos_mask = p->m_uQposMask;
		m_pPlugin->m_fnUpdate ( m_pData, &h );
	}

	DWORD Finalize ( const CSphMatch & tMatch )
	{
		// at some point in the future, we might start passing the entire match,
		// with blackjack, hookers, attributes, and their schema; but at this point,
		// the only sort-of useful part of a match that we are able to push down to
		// the ranker plugin is the match weight
		return m_pPlugin->m_fnFinalize ( m_pData, tMatch.m_iWeight );
	}

private:
	void *					m_pData;
	const PluginRanker_c *	m_pPlugin;
	CSphString				m_sOptions;

	virtual bool ExtraDataImpl ( ExtraData_e eType, void ** ppResult )
	{
		switch ( eType )
		{
			case EXTRA_SET_RANKER_PLUGIN:		m_pPlugin = (const PluginRanker_c*)ppResult; break;
			case EXTRA_SET_RANKER_PLUGIN_OPTS:	m_sOptions = (char*)ppResult; break;
			default:							return false;
		}
		return true;
	}
};

//////////////////////////////////////////////////////////////////////////

class FactorPool_c
{
public:
					FactorPool_c ();

	void			Prealloc ( int iElementSize, int nElements );
	BYTE *			Alloc ();
	void			Free ( BYTE * pPtr );
	int				GetElementSize() const;
	int				GetIntElementSize () const;
	void			AddToHash ( SphDocID_t uId, BYTE * pPacked );
	void			AddRef ( SphDocID_t uId );
	void			Release ( SphDocID_t uId );
	void			Flush ();

	bool			IsInitialized() const;
	SphFactorHash_t * GetHashPtr();

private:
	int				m_iElementSize;
	int				m_iNextFree;

	CSphTightVector<BYTE>	m_dPool;
	CSphTightVector<int>	m_dFree;
	SphFactorHash_t			m_dHash;

	SphFactorHashEntry_t * Find ( SphDocID_t uId ) const;
	inline DWORD	HashFunc ( SphDocID_t uId ) const;
	bool			FlushEntry ( SphFactorHashEntry_t * pEntry );
};


FactorPool_c::FactorPool_c ()
	: m_iElementSize	( 0 )
	, m_iNextFree		( 0 )
{
}


void FactorPool_c::Prealloc ( int iElementSize, int nElements )
{
	m_iElementSize = iElementSize;

	m_dPool.Resize ( nElements*GetIntElementSize() );
	m_dHash.Resize ( nElements );
	m_dFree.Reserve ( nElements );

	memset ( m_dHash.Begin(), 0, sizeof(m_dHash[0])*m_dHash.GetLength() );
}


BYTE * FactorPool_c::Alloc ()
{
	if ( m_dFree.GetLength() )
	{
		int iIndex = m_dFree.Pop();
		return m_dPool.Begin() + iIndex*GetIntElementSize();
	}

	assert ( m_iNextFree<=m_dPool.GetLength() / GetIntElementSize() );

	BYTE * pAllocated = m_dPool.Begin()+m_iNextFree*GetIntElementSize();
	m_iNextFree++;
	return pAllocated;
}


void FactorPool_c::Free ( BYTE * pPtr )
{
	if ( !pPtr )
		return;

	assert ( (pPtr-m_dPool.Begin() ) % GetIntElementSize()==0);
	assert ( pPtr>=m_dPool.Begin() && pPtr<&( m_dPool.Last() ) );

	int iIndex = ( pPtr-m_dPool.Begin() )/GetIntElementSize();
	m_dFree.Add(iIndex);
}


int FactorPool_c::GetIntElementSize () const
{
	return m_iElementSize+sizeof(SphFactorHashEntry_t);
}


int	FactorPool_c::GetElementSize() const
{
	return m_iElementSize;
}


void FactorPool_c::AddToHash ( SphDocID_t uId, BYTE * pPacked )
{
	SphFactorHashEntry_t * pNew = (SphFactorHashEntry_t *)(pPacked+m_iElementSize);
	memset ( pNew, 0, sizeof(SphFactorHashEntry_t) );

	DWORD uKey = HashFunc ( uId );
	if ( m_dHash[uKey] )
	{
		SphFactorHashEntry_t * pStart = m_dHash[uKey];
		pNew->m_pPrev = NULL;
		pNew->m_pNext = pStart;
		pStart->m_pPrev = pNew;
	}

	pNew->m_pData = pPacked;
	pNew->m_iId = uId;
	m_dHash[uKey] = pNew;
}


SphFactorHashEntry_t * FactorPool_c::Find ( SphDocID_t uId ) const
{
	DWORD uKey = HashFunc ( uId );
	if ( m_dHash[uKey] )
	{
		SphFactorHashEntry_t * pEntry = m_dHash[uKey];
		while ( pEntry )
		{
			if ( pEntry->m_iId==uId )
				return pEntry;

			pEntry = pEntry->m_pNext;
		}
	}

	return NULL;
}


void FactorPool_c::AddRef ( SphDocID_t uId )
{
	if ( !uId )
		return;

	SphFactorHashEntry_t * pEntry = Find ( uId );
	if ( pEntry )
		pEntry->m_iRefCount++;
}


void FactorPool_c::Release ( SphDocID_t uId )
{
	if ( !uId )
		return;

	SphFactorHashEntry_t * pEntry = Find ( uId );
	if ( pEntry )
	{
		pEntry->m_iRefCount--;
		bool bHead = !pEntry->m_pPrev;
		SphFactorHashEntry_t * pNext = pEntry->m_pNext;
		if ( FlushEntry ( pEntry ) && bHead )
			m_dHash [ HashFunc ( uId ) ] = pNext;
	}
}


bool FactorPool_c::FlushEntry ( SphFactorHashEntry_t * pEntry )
{
	assert ( pEntry );
	assert ( pEntry->m_iRefCount>=0 );
	if ( pEntry->m_iRefCount )
		return false;

	if ( pEntry->m_pPrev )
		pEntry->m_pPrev->m_pNext = pEntry->m_pNext;

	if ( pEntry->m_pNext )
		pEntry->m_pNext->m_pPrev = pEntry->m_pPrev;

	Free ( pEntry->m_pData );

	return true;
}


void FactorPool_c::Flush()
{
	ARRAY_FOREACH ( i, m_dHash )
	{
		SphFactorHashEntry_t * pEntry = m_dHash[i];
		while ( pEntry )
		{
			SphFactorHashEntry_t * pNext = pEntry->m_pNext;
			bool bHead = !pEntry->m_pPrev;
			if ( FlushEntry(pEntry) && bHead )
				m_dHash[i] = pNext;

			pEntry = pNext;
		}
	}
}


inline DWORD FactorPool_c::HashFunc ( SphDocID_t uId ) const
{
	return (DWORD)( uId % m_dHash.GetLength() );
}


bool FactorPool_c::IsInitialized() const
{
	return !!m_iElementSize;
}


SphFactorHash_t * FactorPool_c::GetHashPtr ()
{
	return &m_dHash;
}


//////////////////////////////////////////////////////////////////////////
// EXPRESSION RANKER
//////////////////////////////////////////////////////////////////////////

/// lean hit
/// only stores keyword id and hit position
struct LeanHit_t
{
	WORD		m_uQuerypos;
	Hitpos_t	m_uHitpos;

	LeanHit_t & operator = ( const ExtHit_t & rhs )
	{
		m_uQuerypos = rhs.m_uQuerypos;
		m_uHitpos = rhs.m_uHitpos;
		return *this;
	}
};

/// ranker state that computes weight dynamically based on user supplied expression (formula)
template < bool NEED_PACKEDFACTORS = false, bool HANDLE_DUPES = false >
struct RankerState_Expr_fn : public ISphExtra
{
public:
	// per-field and per-document stuff
	BYTE				m_uLCS[SPH_MAX_FIELDS];
	BYTE				m_uCurLCS;
	DWORD				m_uCurPos;
	DWORD				m_uLcsTailPos;
	DWORD				m_uLcsTailQposMask;
	DWORD				m_uCurQposMask;
	int					m_iExpDelta;
	int					m_iLastHitPos;
	int					m_iFields;
	const int *			m_pWeights;
	DWORD				m_uDocBM25;
	CSphBitvec			m_tMatchedFields;
	int					m_iCurrentField;
	DWORD				m_uHitCount[SPH_MAX_FIELDS];
	DWORD				m_uWordCount[SPH_MAX_FIELDS];
	CSphVector<float>	m_dIDF;
	float				m_dTFIDF[SPH_MAX_FIELDS];
	float				m_dMinIDF[SPH_MAX_FIELDS];
	float				m_dMaxIDF[SPH_MAX_FIELDS];
	float				m_dSumIDF[SPH_MAX_FIELDS];
	int					m_iMinHitPos[SPH_MAX_FIELDS];
	int					m_iMinBestSpanPos[SPH_MAX_FIELDS];
	CSphBitvec			m_tExactHit;
	CSphBitvec			m_tExactOrder;
	CSphBitvec			m_tKeywords;
	DWORD				m_uDocWordCount;
	int					m_iMaxWindowHits[SPH_MAX_FIELDS];
	CSphVector<int>		m_dTF;			///< for bm25a
	float				m_fDocBM25A;	///< for bm25a
	CSphVector<int>		m_dFieldTF;		///< for bm25f, per-field layout (ie all field0 tfs, then all field1 tfs, etc)
	int					m_iMinGaps[SPH_MAX_FIELDS];		///< number of gaps in the minimum matching window

	const char *		m_sExpr;
	ISphExpr *			m_pExpr;
	ESphAttr			m_eExprType;
	const CSphSchema *	m_pSchema;
	CSphAttrLocator		m_tFieldLensLoc;
	float				m_fAvgDocLen;
	const int64_t *		m_pFieldLens;
	int64_t				m_iTotalDocuments;
	float				m_fParamK1;
	float				m_fParamB;
	int					m_iMaxQpos;			///< among all words, including dupes
	CSphVector<WORD>	m_dTermDupes;
	CSphVector<Hitpos_t>	m_dTermsHit;
	CSphBitvec			m_tHasMultiQpos;
	int					m_uLastSpanStart;

	FactorPool_c 		m_tFactorPool;
	int					m_iMaxMatches;

	// per-query stuff
	int					m_iMaxLCS;
	int					m_iQueryWordCount;

public:
	// internal state, and factor settings
	// max_window_hits(n)
	CSphVector<DWORD>	m_dWindow;
	int					m_iWindowSize;

	// min_gaps
	int						m_iHaveMinWindow;			///< whether to compute minimum matching window, and over how many query words
	int						m_iMinWindowWords;			///< how many unique words have we seen so far
	CSphVector<LeanHit_t>	m_dMinWindowHits;			///< current minimum matching window candidate hits
	CSphVector<int>			m_dMinWindowCounts;			///< maps querypos indexes to number of occurrencess in m_dMinWindowHits

	// exact_order
	int					m_iLastField;
	int					m_iLastQuerypos;
	int					m_iExactOrderWords;

	// LCCS and Weighted LCCS
	BYTE				m_dLCCS[SPH_MAX_FIELDS];
	float				m_dWLCCS[SPH_MAX_FIELDS];
	CSphVector<WORD>	m_dNextQueryPos;				///< words positions might have gaps due to stop-words
	WORD				m_iQueryPosLCCS;
	int					m_iHitPosLCCS;
	BYTE				m_iLenLCCS;
	float				m_fWeightLCCS;

	// ATC
#define XRANK_ATC_WINDOW_LEN 10
#define XRANK_ATC_BUFFER_LEN 30
#define XRANK_ATC_DUP_DIV 0.25f
#define XRANK_ATC_EXP 1.75f
	struct AtcHit_t
	{
		int			m_iHitpos;
		WORD		m_uQuerypos;
	};
	AtcHit_t			m_dAtcHits[XRANK_ATC_BUFFER_LEN];	///< ATC hits ring buffer
	int					m_iAtcHitStart;						///< hits start at ring buffer
	int					m_iAtcHitCount;						///< hits amount in buffer
	CSphVector<float>	m_dAtcTerms;						///< per-word ATC
	CSphBitvec			m_dAtcProcessedTerms;				///< temporary processed mask
	DWORD				m_uAtcField;						///< currently processed field
	float				m_dAtc[SPH_MAX_FIELDS];				///< ATC per-field values
	bool				m_bAtcHeadProcessed;				///< flag for hits from buffer start to window start
	bool				m_bHaveAtc;							///< calculate ATC?
	bool				m_bWantAtc;

	void				UpdateATC ( bool bFlushField );
	float				TermTC ( int iTerm, bool bLeft );

public:
						RankerState_Expr_fn ();
						~RankerState_Expr_fn ();

	bool				Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError, DWORD uFactorFlags );
	void				Update ( const ExtHit_t * pHlist );
	DWORD				Finalize ( const CSphMatch & tMatch );
	bool				IsTermSkipped ( int iTerm );

public:
	/// setup per-keyword data needed to compute the factors
	/// (namely IDFs, counts, masks etc)
	/// WARNING, CALLED EVEN BEFORE INIT()!
	void SetQwords ( const ExtQwordsHash_t & hQwords )
	{
		m_dIDF.Resize ( m_iMaxQpos+1 ); // [MaxUniqQpos, MaxQpos] range will be all 0, but anyway
		m_dIDF.Fill ( 0.0f );

		m_dTF.Resize ( m_iMaxQpos+1 );
		m_dTF.Fill ( 0 );

		m_dMinWindowCounts.Resize ( m_iMaxQpos+1 );
		m_dMinWindowCounts.Fill ( 0 );

		m_iQueryWordCount = 0;
		m_tKeywords.Init ( m_iMaxQpos+1 ); // will not be tracking dupes
		bool bGotExpanded = false;
		CSphVector<WORD> dQueryPos;
		dQueryPos.Reserve ( m_iMaxQpos+1 );

		hQwords.IterateStart();
		while ( hQwords.IterateNext() )
		{
			// tricky bit
			// for query_word_count, we only want to count keywords that are not (!) excluded by the query
			// that is, in (aa NOT bb) case, we want a value of 1, not 2
			// there might be tail excluded terms these not affected MaxQpos
			ExtQword_t & dCur = hQwords.IterateGet();
			const int iQueryPos = dCur.m_iQueryPos;
			if ( dCur.m_bExcluded )
				continue;

			bool bQposUsed = m_tKeywords.BitGet ( iQueryPos );
			bGotExpanded |= bQposUsed;
			m_iQueryWordCount += ( bQposUsed ? 0 : 1 ); // count only one term at that position
			m_tKeywords.BitSet ( iQueryPos ); // just to assert at early stage!

			m_dIDF [ iQueryPos ] += dCur.m_fIDF;
			m_dTF [ iQueryPos ]++;
			if ( !bQposUsed )
				dQueryPos.Add ( (WORD)iQueryPos );
		}

		// FIXME!!! average IDF for expanded terms (aot morphology or dict=keywords)
		if ( bGotExpanded )
			ARRAY_FOREACH ( i, m_dTF )
			{
				if ( m_dTF[i]>1 )
					m_dIDF[i] /= m_dTF[i];
			}

		m_dTF.Fill ( 0 );

		// set next term position for current term in query (degenerates to +1 in the simplest case)
		dQueryPos.Sort();
		m_dNextQueryPos.Resize ( m_iMaxQpos+1 );
		m_dNextQueryPos.Fill ( (WORD)-1 ); // WORD_MAX filler
		for ( int i=0; i<dQueryPos.GetLength()-1; i++ )
		{
			WORD iCutPos = dQueryPos[i];
			WORD iNextPos = dQueryPos[i+1];
			m_dNextQueryPos[iCutPos] = iNextPos;
		}
	}

	void SetTermDupes ( const ExtQwordsHash_t & hQwords, int iMaxQpos, const ExtNode_i * pRoot )
	{
		if ( !pRoot )
			return;

		m_dTermsHit.Resize ( iMaxQpos + 1 );
		m_dTermsHit.Fill ( EMPTY_HIT );
		m_tHasMultiQpos.Init ( iMaxQpos+1 );
		m_dTermDupes.Resize ( iMaxQpos+1 );
		m_dTermDupes.Fill ( (WORD)-1 );

		CSphVector<TermPos_t> dTerms;
		dTerms.Reserve ( iMaxQpos );
		pRoot->GetTerms ( hQwords, dTerms );

		// reset excluded for all duplicates
		ARRAY_FOREACH ( i, dTerms )
		{
			WORD uAtomPos = dTerms[i].m_uAtomPos;
			WORD uQpos = dTerms[i].m_uQueryPos;
			if ( uAtomPos!=uQpos )
			{
				m_tHasMultiQpos.BitSet ( uAtomPos );
				m_tHasMultiQpos.BitSet ( uQpos );
			}

			m_tKeywords.BitSet ( uAtomPos );
			m_tKeywords.BitSet ( uQpos );
			m_dTermDupes[uAtomPos] = uQpos;

			// fill missed idf for dups
			if ( fabs ( m_dIDF[uAtomPos] )<=1e-6 )
				m_dIDF[uAtomPos] = m_dIDF[uQpos];
		}
	}

	/// finalize per-document factors that, well, need finalization
	void FinalizeDocFactors ( const CSphMatch & tMatch )
	{
		m_uDocBM25 = tMatch.m_iWeight;
		for ( int i=0; i<m_iFields; i++ )
		{
			m_uWordCount[i] = sphBitCount ( m_uWordCount[i] );
			if ( m_dMinIDF[i] > m_dMaxIDF[i] )
				m_dMinIDF[i] = m_dMaxIDF[i] = 0; // must be FLT_MAX vs -FLT_MAX, aka no hits
		}
		m_uDocWordCount = sphBitCount ( m_uDocWordCount );

		// compute real BM25
		// with blackjack, and hookers, and field lengths, and parameters
		//
		// canonical idf = log ( (N-n+0.5) / (n+0.5) )
		// sphinx idf = log ( (N-n+1) / n )
		// and we also downscale our idf by 1/log(N+1) to map it into [-0.5, 0.5] range

		// compute document length
		float dl = 0; // OPTIMIZE? could precompute and store total dl in attrs, but at a storage cost
		CSphAttrLocator tLoc = m_tFieldLensLoc;
		if ( tLoc.m_iBitOffset>=0 )
			for ( int i=0; i<m_iFields; i++ )
		{
			dl += tMatch.GetAttr ( tLoc );
			tLoc.m_iBitOffset += 32;
		}

		// compute BM25A (one value per document)
		m_fDocBM25A = 0.0f;
		for ( int iWord=1; iWord<=m_iMaxQpos; iWord++ )
		{
			if ( IsTermSkipped ( iWord ) )
				continue;

			float tf = (float)m_dTF[iWord]; // OPTIMIZE? remove this vector, hook into m_uMatchHits somehow?
			float idf = m_dIDF[iWord];
			m_fDocBM25A += tf / (tf + m_fParamK1*(1 - m_fParamB + m_fParamB*dl/m_fAvgDocLen)) * idf;
		}
		m_fDocBM25A += 0.5f; // map to [0..1] range
	}

	/// reset per-document factors, prepare for the next document
	void ResetDocFactors()
	{
		// OPTIMIZE? quick full wipe? (using dwords/sse/whatever)
		m_uCurLCS = 0;
		if_const ( HANDLE_DUPES )
		{
			m_uCurPos = 0;
			m_uLcsTailPos = 0;
			m_uLcsTailQposMask = 0;
			m_uCurQposMask = 0;
			m_uLastSpanStart = 0;
		}
		m_iExpDelta = -1;
		m_iLastHitPos = -1;
		for ( int i=0; i<m_iFields; i++ )
		{
			m_uLCS[i] = 0;
			m_uHitCount[i] = 0;
			m_uWordCount[i] = 0;
			m_dMinIDF[i] = FLT_MAX;
			m_dMaxIDF[i] = -FLT_MAX;
			m_dSumIDF[i] = 0;
			m_dTFIDF[i] = 0;
			m_iMinHitPos[i] = 0;
			m_iMinBestSpanPos[i] = 0;
			m_iMaxWindowHits[i] = 0;
			m_iMinGaps[i] = 0;
			m_dAtc[i] = 0.0f;
		}
		m_dTF.Fill ( 0 );
		m_dFieldTF.Fill ( 0 ); // OPTIMIZE? make conditional?
		m_tMatchedFields.Clear();
		m_tExactHit.Clear();
		m_tExactOrder.Clear();
		m_uDocWordCount = 0;
		m_dWindow.Resize ( 0 );
		m_fDocBM25A = 0;
		m_dMinWindowHits.Resize ( 0 );
		m_dMinWindowCounts.Fill ( 0 );
		m_iMinWindowWords = 0;
		m_iLastField = -1;
		m_iLastQuerypos = 0;
		m_iExactOrderWords = 0;

		m_dAtcTerms.Fill ( 0.0f );
		m_iAtcHitStart = 0;
		m_iAtcHitCount = 0;
		m_uAtcField = 0;

		if_const ( HANDLE_DUPES )
			m_dTermsHit.Fill ( EMPTY_HIT );
	}

	void FlushMatches ()
	{
		m_tFactorPool.Flush ();
	}

protected:
	inline void UpdateGap ( int iField, int iWords, int iGap )
	{
		if ( m_iMinWindowWords<iWords || ( m_iMinWindowWords==iWords && m_iMinGaps[iField]>iGap ) )
		{
			m_iMinGaps[iField] = iGap;
			m_iMinWindowWords = iWords;
		}
	}

	void			UpdateMinGaps ( const ExtHit_t * pHlist );
	void			UpdateFreq ( WORD uQpos, DWORD uField );

private:
	virtual bool	ExtraDataImpl ( ExtraData_e eType, void ** ppResult );
	int				GetMaxPackedLength();
	BYTE *			PackFactors();
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
	XRANK_MIN_IDF,
	XRANK_MAX_IDF,
	XRANK_SUM_IDF,
	XRANK_MIN_HIT_POS,
	XRANK_MIN_BEST_SPAN_POS,
	XRANK_EXACT_HIT,
	XRANK_EXACT_ORDER,
	XRANK_MAX_WINDOW_HITS,
	XRANK_MIN_GAPS,
	XRANK_LCCS,
	XRANK_WLCCS,
	XRANK_ATC,

	// document level factors
	XRANK_BM25,
	XRANK_MAX_LCS,
	XRANK_FIELD_MASK,
	XRANK_QUERY_WORD_COUNT,
	XRANK_DOC_WORD_COUNT,
	XRANK_BM25A,
	XRANK_BM25F,

	// field aggregation functions
	XRANK_SUM,
	XRANK_TOP
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


/// per-document field mask factor
struct Expr_FieldMask_c : public ISphExpr
{
	const CSphBitvec & m_tFieldMask;

	explicit Expr_FieldMask_c ( const CSphBitvec & tFieldMask )
		: m_tFieldMask ( tFieldMask )
	{}

	float Eval ( const CSphMatch & ) const
	{
		return (float)*m_tFieldMask.Begin();
	}

	int IntEval ( const CSphMatch & ) const
	{
		return (int)*m_tFieldMask.Begin();
	}
};


/// bitvec field factor specialization
template<>
struct Expr_FieldFactor_c<CSphBitvec> : public ISphExpr
{
	const int *		m_pIndex;
	const CSphBitvec & m_tField;

	Expr_FieldFactor_c ( const int * pIndex, const CSphBitvec & tField )
		: m_pIndex ( pIndex )
		, m_tField ( tField )
	{}

	float Eval ( const CSphMatch & ) const
	{
		return (float)( m_tField.BitGet ( *m_pIndex ) );
	}

	int IntEval ( const CSphMatch & ) const
	{
		return (int)( m_tField.BitGet ( *m_pIndex ) );
	}
};


/// generic per-document float factor
struct Expr_FloatPtr_c : public ISphExpr
{
	float * m_pVal;

	explicit Expr_FloatPtr_c ( float * pVal )
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

template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
struct Expr_BM25F_T : public ISphExpr
{
	RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * m_pState;
	float					m_fK1;
	float					m_fB;
	float					m_fWeightedAvgDocLen;
	CSphVector<int>			m_dWeights;		///< per field weights

	explicit Expr_BM25F_T ( RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * pState, float k1, float b, ISphExpr * pFieldWeights )
	{
		// bind k1, b
		m_pState = pState;
		m_fK1 = k1;
		m_fB = b;

		// bind weights
		m_dWeights.Resize ( pState->m_iFields );
		m_dWeights.Fill ( 1 );
		if ( pFieldWeights )
		{
			Expr_MapArg_c * pMapArg = (Expr_MapArg_c*)pFieldWeights;

			CSphVector<CSphNamedVariant> & dOpts = pMapArg->m_dValues;
			ARRAY_FOREACH ( i, dOpts )
			{
				// FIXME? report errors if field was not found?
				if ( !dOpts[i].m_sValue.IsEmpty() )
					continue; // weights must be int, not string
				CSphString & sField = dOpts[i].m_sKey;
				int iField = pState->m_pSchema->GetFieldIndex ( sField.cstr() );
				if ( iField>=0 )
					m_dWeights[iField] = dOpts[i].m_iValue;
			}
		}

		// compute weighted avgdl
		m_fWeightedAvgDocLen = 0;
		if ( m_pState->m_pFieldLens )
			ARRAY_FOREACH ( i, m_dWeights )
				m_fWeightedAvgDocLen += m_pState->m_pFieldLens[i] * m_dWeights[i];
		else
			m_fWeightedAvgDocLen = 1.0f;
		m_fWeightedAvgDocLen /= m_pState->m_iTotalDocuments;
	}

	float Eval ( const CSphMatch & tMatch ) const
	{
		// compute document length
		// OPTIMIZE? could precompute and store total dl in attrs, but at a storage cost
		// OPTIMIZE? could at least share between multiple BM25F instances, if there are many
		float dl = 0;
		CSphAttrLocator tLoc = m_pState->m_tFieldLensLoc;
		if ( tLoc.m_iBitOffset>=0 )
			for ( int i=0; i<m_pState->m_iFields; i++ )
		{
			dl += tMatch.GetAttr ( tLoc ) * m_dWeights[i];
			tLoc.m_iBitOffset += 32;
		}

		// compute (the current instance of) BM25F
		float fRes = 0.0f;
		for ( int iWord=1; iWord<=m_pState->m_iMaxQpos; iWord++ )
		{
			if ( m_pState->IsTermSkipped ( iWord ) )
				continue;

			// compute weighted TF
			float tf = 0.0f;
			for ( int i=0; i<m_pState->m_iFields; i++ )
				tf += m_pState->m_dFieldTF [ iWord + i*(1+m_pState->m_iMaxQpos) ] * m_dWeights[i];
			float idf = m_pState->m_dIDF[iWord]; // FIXME? zeroed out for dupes!
			fRes += tf / (tf + m_fK1*(1.0f - m_fB + m_fB*dl/m_fWeightedAvgDocLen)) * idf;
		}
		return fRes + 0.5f; // map to [0..1] range
	}
};


/// function that sums sub-expressions over matched fields
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
struct Expr_Sum_T : public ISphExpr
{
	RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * m_pState;
	ISphExpr *				m_pArg;

	Expr_Sum_T ( RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * pState, ISphExpr * pArg )
		: m_pState ( pState )
		, m_pArg ( pArg )
	{}

	virtual ~Expr_Sum_T()
	{
		SafeRelease ( m_pArg );
	}

	float Eval ( const CSphMatch & tMatch ) const
	{
		m_pState->m_iCurrentField = 0;
		float fRes = 0;
		const CSphBitvec & tFields = m_pState->m_tMatchedFields;
		int iBits = tFields.BitCount();
		while ( iBits )
		{
			if ( tFields.BitGet ( m_pState->m_iCurrentField ) )
			{
				fRes += m_pArg->Eval ( tMatch );
				iBits--;
			}
			m_pState->m_iCurrentField++;
		}
		return fRes;
	}

	int IntEval ( const CSphMatch & tMatch ) const
	{
		m_pState->m_iCurrentField = 0;
		int iRes = 0;
		const CSphBitvec & tFields = m_pState->m_tMatchedFields;
		int iBits = tFields.BitCount();
		while ( iBits )
		{
			if ( tFields.BitGet ( m_pState->m_iCurrentField ) )
			{
				iRes += m_pArg->IntEval ( tMatch );
				iBits--;
			}
			m_pState->m_iCurrentField++;
		}
		return iRes;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		assert ( m_pArg );
		m_pArg->Command ( eCmd, pArg );
	}
};


/// aggregate max over matched fields
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
struct Expr_Top_T : public ISphExpr
{
	RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * m_pState;
	ISphExpr *				m_pArg;

	Expr_Top_T ( RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * pState, ISphExpr * pArg )
		: m_pState ( pState )
		, m_pArg ( pArg )
	{}

	virtual ~Expr_Top_T()
	{
		SafeRelease ( m_pArg );
	}

	float Eval ( const CSphMatch & tMatch ) const
	{
		m_pState->m_iCurrentField = 0;
		float fRes = FLT_MIN;
		const CSphBitvec & tFields = m_pState->m_tMatchedFields;
		int iBits = tFields.BitCount();
		while ( iBits )
		{
			if ( tFields.BitGet ( m_pState->m_iCurrentField ) )
			{
				fRes = Max ( fRes, m_pArg->Eval ( tMatch ) );
				iBits--;
			}
			m_pState->m_iCurrentField++;
		}
		return fRes;
	}

	int IntEval ( const CSphMatch & tMatch ) const
	{
		m_pState->m_iCurrentField = 0;
		int iRes = INT_MIN;
		const CSphBitvec & tFields = m_pState->m_tMatchedFields;
		int iBits = tFields.BitCount();
		while ( iBits )
		{
			if ( tFields.BitGet ( m_pState->m_iCurrentField ) )
			{
				iRes = Max ( iRes, m_pArg->IntEval ( tMatch ) );
				iBits--;
			}
			m_pState->m_iCurrentField++;
		}
		return iRes;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		assert ( m_pArg );
		m_pArg->Command ( eCmd, pArg );
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
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
class ExprRankerHook_T : public ISphExprHook
{
public:
	RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * m_pState;
	const char *			m_sCheckError;
	bool					m_bCheckInFieldAggr;

public:
	explicit ExprRankerHook_T ( RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> * pState )
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
		if ( !strcasecmp ( sIdent, "min_idf" ) )
			return XRANK_MIN_IDF;
		if ( !strcasecmp ( sIdent, "max_idf" ) )
			return XRANK_MAX_IDF;
		if ( !strcasecmp ( sIdent, "sum_idf" ) )
			return XRANK_SUM_IDF;
		if ( !strcasecmp ( sIdent, "min_hit_pos" ) )
			return XRANK_MIN_HIT_POS;
		if ( !strcasecmp ( sIdent, "min_best_span_pos" ) )
			return XRANK_MIN_BEST_SPAN_POS;
		if ( !strcasecmp ( sIdent, "exact_hit" ) )
			return XRANK_EXACT_HIT;
		if ( !strcasecmp ( sIdent, "exact_order" ) )
			return XRANK_EXACT_ORDER;

		if ( !strcasecmp ( sIdent, "bm25" ) )
			return XRANK_BM25;
		if ( !strcasecmp ( sIdent, "max_lcs" ) )
			return XRANK_MAX_LCS;
		if ( !strcasecmp ( sIdent, "field_mask" ) )
			return XRANK_FIELD_MASK;
		if ( !strcasecmp ( sIdent, "query_word_count" ) )
			return XRANK_QUERY_WORD_COUNT;
		if ( !strcasecmp ( sIdent, "doc_word_count" ) )
			return XRANK_DOC_WORD_COUNT;

		if ( !strcasecmp ( sIdent, "min_gaps" ) )
			return XRANK_MIN_GAPS;

		if ( !strcasecmp ( sIdent, "lccs" ) )
			return XRANK_LCCS;
		if ( !strcasecmp ( sIdent, "wlccs" ) )
			return XRANK_WLCCS;
		if ( !strcasecmp ( sIdent, "atc" ) )
			return XRANK_ATC;

		return -1;
	}

	int IsKnownFunc ( const char * sFunc )
	{
		if ( !strcasecmp ( sFunc, "sum" ) )
			return XRANK_SUM;
		if ( !strcasecmp ( sFunc, "top" ) )
			return XRANK_TOP;
		if ( !strcasecmp ( sFunc, "max_window_hits" ) )
			return XRANK_MAX_WINDOW_HITS;
		if ( !strcasecmp ( sFunc, "bm25a" ) )
			return XRANK_BM25A;
		if ( !strcasecmp ( sFunc, "bm25f" ) )
			return XRANK_BM25F;
		return -1;
	}

	ISphExpr * CreateNode ( int iID, ISphExpr * pLeft, ESphEvalStage *, CSphString & )
	{
		int * pCF = &m_pState->m_iCurrentField; // just a shortcut
		switch ( iID )
		{
			case XRANK_LCS:					return new Expr_FieldFactor_c<BYTE> ( pCF, m_pState->m_uLCS );
			case XRANK_USER_WEIGHT:			return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_pWeights );
			case XRANK_HIT_COUNT:			return new Expr_FieldFactor_c<DWORD> ( pCF, m_pState->m_uHitCount );
			case XRANK_WORD_COUNT:			return new Expr_FieldFactor_c<DWORD> ( pCF, m_pState->m_uWordCount );
			case XRANK_TF_IDF:				return new Expr_FieldFactor_c<float> ( pCF, m_pState->m_dTFIDF );
			case XRANK_MIN_IDF:				return new Expr_FieldFactor_c<float> ( pCF, m_pState->m_dMinIDF );
			case XRANK_MAX_IDF:				return new Expr_FieldFactor_c<float> ( pCF, m_pState->m_dMaxIDF );
			case XRANK_SUM_IDF:				return new Expr_FieldFactor_c<float> ( pCF, m_pState->m_dSumIDF );
			case XRANK_MIN_HIT_POS:			return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_iMinHitPos );
			case XRANK_MIN_BEST_SPAN_POS:	return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_iMinBestSpanPos );
			case XRANK_EXACT_HIT:			return new Expr_FieldFactor_c<CSphBitvec> ( pCF, m_pState->m_tExactHit );
			case XRANK_EXACT_ORDER:			return new Expr_FieldFactor_c<CSphBitvec> ( pCF, m_pState->m_tExactOrder );
			case XRANK_MAX_WINDOW_HITS:
				{
					CSphMatch tDummy;
					m_pState->m_iWindowSize = pLeft->IntEval ( tDummy ); // must be constant; checked in GetReturnType()
					SafeRelease ( pLeft );
					return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_iMaxWindowHits );
				}
			case XRANK_MIN_GAPS:			return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_iMinGaps );
			case XRANK_LCCS:				return new Expr_FieldFactor_c<BYTE> ( pCF, m_pState->m_dLCCS );
			case XRANK_WLCCS:				return new Expr_FieldFactor_c<float> ( pCF, m_pState->m_dWLCCS );
			case XRANK_ATC:
				m_pState->m_bWantAtc = true;
				return new Expr_FieldFactor_c<float> ( pCF, m_pState->m_dAtc );

			case XRANK_BM25:				return new Expr_IntPtr_c ( &m_pState->m_uDocBM25 );
			case XRANK_MAX_LCS:				return new Expr_GetIntConst_c ( m_pState->m_iMaxLCS );
			case XRANK_FIELD_MASK:			return new Expr_FieldMask_c ( m_pState->m_tMatchedFields );
			case XRANK_QUERY_WORD_COUNT:	return new Expr_GetIntConst_c ( m_pState->m_iQueryWordCount );
			case XRANK_DOC_WORD_COUNT:		return new Expr_IntPtr_c ( &m_pState->m_uDocWordCount );
			case XRANK_BM25A:
				{
					// exprs we'll evaluate here must be constant; that is checked in GetReturnType()
					// so having a dummy match with no data work alright
					assert ( pLeft->IsArglist() );
					CSphMatch tDummy;
					m_pState->m_fParamK1 = pLeft->GetArg(0)->Eval ( tDummy );
					m_pState->m_fParamB = pLeft->GetArg(1)->Eval ( tDummy );
					m_pState->m_fParamK1 = Max ( m_pState->m_fParamK1, 0.001f );
					m_pState->m_fParamB = Min ( Max ( m_pState->m_fParamB, 0.0f ), 1.0f );
					SafeDelete ( pLeft );
					return new Expr_FloatPtr_c ( &m_pState->m_fDocBM25A );
				}
			case XRANK_BM25F:
				{
					assert ( pLeft->IsArglist() );
					CSphMatch tDummy;
					float fK1 = pLeft->GetArg(0)->Eval ( tDummy );
					float fB = pLeft->GetArg(1)->Eval ( tDummy );
					fK1 = Max ( fK1, 0.001f );
					fB = Min ( Max ( fB, 0.0f ), 1.0f );
					ISphExpr * pRes = new Expr_BM25F_T<NEED_PACKEDFACTORS, HANDLE_DUPES> ( m_pState, fK1, fB, pLeft->GetArg(2) );
					SafeDelete ( pLeft );
					return pRes;
				}

			case XRANK_SUM:					return new Expr_Sum_T<NEED_PACKEDFACTORS, HANDLE_DUPES> ( m_pState, pLeft );
			case XRANK_TOP:					return new Expr_Top_T<NEED_PACKEDFACTORS, HANDLE_DUPES> ( m_pState, pLeft );
			default:						return NULL;
		}
	}

	ESphAttr GetIdentType ( int iID )
	{
		switch ( iID )
		{
			case XRANK_LCS: // field-level
			case XRANK_USER_WEIGHT:
			case XRANK_HIT_COUNT:
			case XRANK_WORD_COUNT:
			case XRANK_MIN_HIT_POS:
			case XRANK_MIN_BEST_SPAN_POS:
			case XRANK_EXACT_HIT:
			case XRANK_EXACT_ORDER:
			case XRANK_MAX_WINDOW_HITS:
			case XRANK_BM25: // doc-level
			case XRANK_MAX_LCS:
			case XRANK_FIELD_MASK:
			case XRANK_QUERY_WORD_COUNT:
			case XRANK_DOC_WORD_COUNT:
			case XRANK_MIN_GAPS:
			case XRANK_LCCS:
				return SPH_ATTR_INTEGER;
			case XRANK_TF_IDF:
			case XRANK_MIN_IDF:
			case XRANK_MAX_IDF:
			case XRANK_SUM_IDF:
			case XRANK_WLCCS:
			case XRANK_ATC:
				return SPH_ATTR_FLOAT;
			default:
				assert ( 0 );
				return SPH_ATTR_INTEGER;
		}
	}

	/// helper to check argument types by a signature string (passed in sArgs)
	/// every character in the signature describes a type
	/// ? = any type
	/// i = integer
	/// I = integer constant
	/// f = float
	/// s = scalar (int/float)
	/// h = hash
	/// signature can also be preceded by "c:" modifier than means that all arguments must be constant
	bool CheckArgtypes ( const CSphVector<ESphAttr> & dArgs, const char * sFuncname, const char * sArgs, bool bAllConst, CSphString & sError )
	{
		if ( sArgs[0]=='c' && sArgs[1]==':' )
		{
			if ( !bAllConst )
			{
				sError.SetSprintf ( "%s() requires constant arguments", sFuncname );
				return false;
			}
			sArgs += 2;
		}

		int iLen = strlen ( sArgs );
		if ( dArgs.GetLength()!=iLen )
		{
			sError.SetSprintf ( "%s() requires %d argument(s), not %d", sFuncname, iLen, dArgs.GetLength() );
			return false;
		}

		ARRAY_FOREACH ( i, dArgs )
		{
			switch ( *sArgs++ )
			{
				case '?':
					break;
				case 'i':
					if ( dArgs[i]!=SPH_ATTR_INTEGER )
					{
						sError.SetSprintf ( "argument %d to %s() must be integer", i, sFuncname );
						return false;
					}
					break;
				case 's':
					if ( dArgs[i]!=SPH_ATTR_INTEGER && dArgs[i]!=SPH_ATTR_FLOAT )
					{
						sError.SetSprintf ( "argument %d to %s() must be scalar (integer or float)", i, sFuncname );
						return false;
					}
					break;
				case 'h':
					if ( dArgs[i]!=SPH_ATTR_MAPARG )
					{
						sError.SetSprintf ( "argument %d to %s() must be a map of constants", i, sFuncname );
						return false;
					}
					break;
				default:
					assert ( 0 && "unknown signature code" );
					break;
			}
		}

		// this is important!
		// other previous failed checks might have filled sError
		// and if anything else up the stack checks it, we need an empty message now
		sError = "";
		return true;
	}

	ESphAttr GetReturnType ( int iID, const CSphVector<ESphAttr> & dArgs, bool bAllConst, CSphString & sError )
	{
		switch ( iID )
		{
			case XRANK_SUM:
				if ( !CheckArgtypes ( dArgs, "SUM", "?", bAllConst, sError ) )
					return SPH_ATTR_NONE;
				return dArgs[0];

			case XRANK_TOP:
				if ( !CheckArgtypes ( dArgs, "TOP", "?", bAllConst, sError ) )
					return SPH_ATTR_NONE;
				return dArgs[0];

			case XRANK_MAX_WINDOW_HITS:
				if ( !CheckArgtypes ( dArgs, "MAX_WINDOW_HITS", "c:i", bAllConst, sError ) )
					return SPH_ATTR_NONE;
				return SPH_ATTR_INTEGER;

			case XRANK_BM25A:
				if ( !CheckArgtypes ( dArgs, "BM25A", "c:ss", bAllConst, sError ) )
					return SPH_ATTR_NONE;
				return SPH_ATTR_FLOAT;

			case XRANK_BM25F:
				if ( !CheckArgtypes ( dArgs, "BM25F", "c:ss", bAllConst, sError ) )
					if ( !CheckArgtypes ( dArgs, "BM25F", "c:ssh", bAllConst, sError ) )
						return SPH_ATTR_NONE;
				return SPH_ATTR_FLOAT;

			default:
				sError.SetSprintf ( "internal error: unknown hook function (id=%d)", iID );
		}
		return SPH_ATTR_NONE;
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
			case XRANK_MIN_IDF:
			case XRANK_MAX_IDF:
			case XRANK_SUM_IDF:
			case XRANK_MIN_HIT_POS:
			case XRANK_MIN_BEST_SPAN_POS:
			case XRANK_EXACT_HIT:
			case XRANK_MAX_WINDOW_HITS:
			case XRANK_LCCS:
			case XRANK_WLCCS:
				if ( !m_bCheckInFieldAggr )
					m_sCheckError = "field factors must only occur withing field aggregates in ranking expression";
				break;

			case XRANK_SUM:
			case XRANK_TOP:
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
		if ( !m_sCheckError && ( iID==XRANK_SUM || iID==XRANK_TOP ) )
		{
			assert ( m_bCheckInFieldAggr );
			m_bCheckInFieldAggr = false;
		}
	}
};


/// ctor
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
RankerState_Expr_fn <NEED_PACKEDFACTORS, HANDLE_DUPES>::RankerState_Expr_fn ()
	: m_pWeights ( NULL )
	, m_sExpr ( NULL )
	, m_pExpr ( NULL )
	, m_iMaxMatches ( 0 )
	, m_iMaxLCS ( 0 )
	, m_iQueryWordCount ( 0 )
	, m_iAtcHitStart ( 0 )
	, m_iAtcHitCount ( 0 )
	, m_uAtcField ( 0 )
	, m_bAtcHeadProcessed ( false )
	, m_bHaveAtc ( false )
	, m_bWantAtc ( false )
{}


/// dtor
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
RankerState_Expr_fn <NEED_PACKEDFACTORS, HANDLE_DUPES>::~RankerState_Expr_fn ()
{
	SafeRelease ( m_pExpr );
}


/// initialize ranker state
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
bool RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError,
																	DWORD uFactorFlags )
{
	m_iFields = iFields;
	m_pWeights = pWeights;
	m_uDocBM25 = 0;
	m_tMatchedFields.Init ( iFields );
	m_tExactHit.Init ( iFields );
	m_tExactOrder.Init ( iFields );
	m_iCurrentField = 0;
	m_iMaxQpos = pRanker->m_iMaxQpos; // already copied in SetQwords, but anyway
	m_iWindowSize = 1;
	m_iHaveMinWindow = 0;
	m_dMinWindowHits.Reserve ( Max ( m_iMaxQpos, 32 ) );
	memset ( m_dLCCS, 0 , sizeof(m_dLCCS) );
	memset ( m_dWLCCS, 0, sizeof(m_dWLCCS) );
	m_iQueryPosLCCS = 0;
	m_iHitPosLCCS = 0;
	m_iLenLCCS = 0;
	m_fWeightLCCS = 0.0f;
	m_dAtcTerms.Resize ( m_iMaxQpos + 1 );
	m_dAtcProcessedTerms.Init ( m_iMaxQpos + 1 );
	m_bAtcHeadProcessed = false;
	ResetDocFactors();

	// compute query level constants
	// max_lcs, aka m_iMaxLCS (for matchany ranker emulation) gets computed here
	// query_word_count, aka m_iQueryWordCount is set elsewhere (in SetQwordsIDF())
	m_iMaxLCS = 0;
	for ( int i=0; i<iFields; i++ )
		m_iMaxLCS += pWeights[i] * pRanker->m_iQwords;

	for ( int i=0; i<m_pSchema->GetAttrsCount(); i++ )
	{
		if ( m_pSchema->GetAttr(i).m_eAttrType!=SPH_ATTR_TOKENCOUNT )
			continue;
		m_tFieldLensLoc = m_pSchema->GetAttr(i).m_tLocator;
		break;
	}

	m_fAvgDocLen = 0.0f;
	m_pFieldLens = pRanker->GetIndex()->GetFieldLens();
	if ( m_pFieldLens )
		for ( int i=0; i<iFields; i++ )
			m_fAvgDocLen += m_pFieldLens[i];
	else
		m_fAvgDocLen = 1.0f;
	m_iTotalDocuments = pRanker->GetIndex()->GetStats().m_iTotalDocuments;
	m_fAvgDocLen /= m_iTotalDocuments;

	m_fParamK1 = 1.2f;
	m_fParamB = 0.75f;

	// not in SetQwords, because we only get iFields here
	m_dFieldTF.Resize ( m_iFields*(m_iMaxQpos+1) );
	m_dFieldTF.Fill ( 0 );

	// parse expression
	bool bUsesWeight;
	ExprRankerHook_T<NEED_PACKEDFACTORS, HANDLE_DUPES> tHook ( this );
	m_pExpr = sphExprParse ( m_sExpr, *m_pSchema, &m_eExprType, &bUsesWeight, sError, NULL, SPH_COLLATION_DEFAULT, &tHook ); // FIXME!!! profile UDF here too
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

	int iUniq = m_iMaxQpos;
	if_const ( HANDLE_DUPES )
	{
		iUniq = 0;
		ARRAY_FOREACH ( i, m_dTermDupes )
			iUniq += ( IsTermSkipped(i) ? 0 : 1 );
	}

	m_iHaveMinWindow = iUniq;

	// we either have an ATC factor in the expression or packedfactors() without no_atc=1
	if ( m_bWantAtc || ( uFactorFlags & SPH_FACTOR_CALC_ATC ) )
		m_bHaveAtc = ( iUniq>1 );

	// all seems ok
	return true;
}


/// process next hit, update factors
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
void RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::Update ( const ExtHit_t * pHlist )
{
	const DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
	const int iPos = HITMAN::GetPos ( pHlist->m_uHitpos );
	const DWORD uPosWithField = HITMAN::GetPosWithField ( pHlist->m_uHitpos );

	if_const ( !HANDLE_DUPES )
	{
		// update LCS
		int iDelta = uPosWithField - pHlist->m_uQuerypos;
		if ( iDelta==m_iExpDelta )
		{
			if ( (int)uPosWithField>m_iLastHitPos )
				m_uCurLCS = (BYTE)( m_uCurLCS + pHlist->m_uWeight );
			if ( HITMAN::IsEnd ( pHlist->m_uHitpos ) && (int)pHlist->m_uQuerypos==m_iMaxQpos && iPos==m_iMaxQpos )
				m_tExactHit.BitSet ( uField );
		} else
		{
			if ( (int)uPosWithField>m_iLastHitPos )
				m_uCurLCS = BYTE(pHlist->m_uWeight);
			if ( iPos==1 && HITMAN::IsEnd ( pHlist->m_uHitpos ) && m_iMaxQpos==1 )
				m_tExactHit.BitSet ( uField );
		}

		if ( m_uCurLCS>m_uLCS[uField] )
		{
			m_uLCS[uField] = m_uCurLCS;
			// for the first hit in current field just use current position as min_best_span_pos
			// else adjust for current lcs
			if ( !m_iMinBestSpanPos [ uField ] )
				m_iMinBestSpanPos [ uField ] = iPos;
			else
				m_iMinBestSpanPos [ uField ] = iPos - m_uCurLCS + 1;
		}
		m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1;
		m_iLastHitPos = uPosWithField;
	} else
	{
		// reset accumulated data from previous field
		if ( (DWORD)HITMAN::GetField ( m_uCurPos )!=uField )
		{
			m_uCurPos = 0;
			m_uLcsTailPos = 0;
			m_uCurQposMask = 0;
			m_uCurLCS = 0;
		}

		if ( (DWORD)uPosWithField!=m_uCurPos )
		{
			// next new and shiny hitpos in line
			// FIXME!? what do we do with longer spans? keep looking? reset?
			if ( m_uCurLCS<2 )
			{
				m_uLcsTailPos = m_uCurPos;
				m_uLcsTailQposMask = m_uCurQposMask;
				m_uCurLCS = 1;
			}
			m_uCurQposMask = 0;
			m_uCurPos = uPosWithField;
			if ( m_uLCS [ uField ]<pHlist->m_uWeight )
			{
				m_uLCS [ uField ] = BYTE ( pHlist->m_uWeight );
				m_iMinBestSpanPos [ uField ] = iPos;
				m_uLastSpanStart = iPos;
			}
		}

		// add that qpos to current qpos mask (for the current hitpos)
		m_uCurQposMask |= ( 1UL << pHlist->m_uQuerypos );

		// and check if that results in a better lcs match now
		int iDelta = ( m_uCurPos-m_uLcsTailPos );
		if ( iDelta && ( m_uCurQposMask >> iDelta ) & m_uLcsTailQposMask )
		{
			// cool, it matched!
			m_uLcsTailQposMask = ( 1UL << pHlist->m_uQuerypos ); // our lcs span now ends with a specific qpos
			m_uLcsTailPos = m_uCurPos; // and in a specific position
			m_uCurLCS = BYTE ( m_uCurLCS+pHlist->m_uWeight ); // and it's longer
			m_uCurQposMask = 0; // and we should avoid matching subsequent hits on the same hitpos

			// update per-field vector
			if ( m_uCurLCS>m_uLCS[uField] )
			{
				m_uLCS[uField] = m_uCurLCS;
				m_iMinBestSpanPos[uField] = m_uLastSpanStart;
			}
		}

		if ( iDelta==m_iExpDelta )
		{
			if ( HITMAN::IsEnd ( pHlist->m_uHitpos ) && (int)pHlist->m_uQuerypos==m_iMaxQpos && iPos==m_iMaxQpos )
				m_tExactHit.BitSet ( uField );
		} else
		{
			if ( iPos==1 && HITMAN::IsEnd ( pHlist->m_uHitpos ) && m_iMaxQpos==1 )
				m_tExactHit.BitSet ( uField );
		}
		m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1;
	}

	bool bLetsKeepup = false;
	// update LCCS
	if ( m_iQueryPosLCCS==pHlist->m_uQuerypos && m_iHitPosLCCS==iPos )
	{
		m_iLenLCCS++;
		m_fWeightLCCS += m_dIDF [ pHlist->m_uQuerypos ];
	} else
	{
		if_const ( HANDLE_DUPES && m_iHitPosLCCS && iPos<=m_iHitPosLCCS && m_tHasMultiQpos.BitGet ( pHlist->m_uQuerypos ) )
		{
			bLetsKeepup = true;
		} else
		{
			m_iLenLCCS = 1;
			m_fWeightLCCS = m_dIDF[pHlist->m_uQuerypos];
		}
	}
	if ( !bLetsKeepup )
	{
		WORD iNextQPos = m_dNextQueryPos[pHlist->m_uQuerypos];
		m_iQueryPosLCCS = iNextQPos;
		m_iHitPosLCCS = iPos + pHlist->m_uSpanlen + iNextQPos - pHlist->m_uQuerypos - 1;
	}
	if ( m_dLCCS[uField]<=m_iLenLCCS ) // FIXME!!! check weight too or keep length and weight separate
	{
		m_dLCCS[uField] = m_iLenLCCS;
		m_dWLCCS[uField] = m_fWeightLCCS;
	}

	// update ATC
	if ( m_bHaveAtc )
	{
		if ( m_uAtcField!=uField || m_iAtcHitCount==XRANK_ATC_BUFFER_LEN )
		{
			UpdateATC ( m_uAtcField!=uField );
			if ( m_uAtcField!=uField )
			{
				m_uAtcField = uField;
			}
			if ( m_iAtcHitCount==XRANK_ATC_BUFFER_LEN ) // advance ring buffer
			{
				m_iAtcHitStart = ( m_iAtcHitStart + XRANK_ATC_WINDOW_LEN ) % XRANK_ATC_BUFFER_LEN;
				m_iAtcHitCount -= XRANK_ATC_WINDOW_LEN;
			}
		}
		assert ( m_iAtcHitStart<XRANK_ATC_BUFFER_LEN && m_iAtcHitCount<XRANK_ATC_BUFFER_LEN );
		int iRing = ( m_iAtcHitStart + m_iAtcHitCount ) % XRANK_ATC_BUFFER_LEN;
		AtcHit_t & tAtcHit = m_dAtcHits [ iRing ];
		tAtcHit.m_iHitpos = iPos;
		tAtcHit.m_uQuerypos = pHlist->m_uQuerypos;
		m_iAtcHitCount++;
	}

	// update other stuff
	m_tMatchedFields.BitSet ( uField );

	// keywords can be duplicated in the query, so we need this extra check
	WORD uQpos = pHlist->m_uQuerypos;
	bool bUniq = m_tKeywords.BitGet ( pHlist->m_uQuerypos );
	if_const ( HANDLE_DUPES && bUniq )
	{
		uQpos = m_dTermDupes [ uQpos ];
		bUniq = ( m_dTermsHit[uQpos]!=pHlist->m_uHitpos && m_dTermsHit[0]!=pHlist->m_uHitpos );
		m_dTermsHit[uQpos] = pHlist->m_uHitpos;
		m_dTermsHit[0] = pHlist->m_uHitpos;
	}
	if ( bUniq )
	{
		UpdateFreq ( uQpos, uField );
	}
	// handle hit with multiple terms
	if ( pHlist->m_uSpanlen>1 )
	{
		WORD uQposSpanned = pHlist->m_uQuerypos+1;
		DWORD uQposMask = ( pHlist->m_uQposMask>>1 );
		while ( uQposMask!=0 )
		{
			WORD uQposFixed = uQposSpanned;
			if ( ( uQposMask & 1 )==1 )
			{
				bool bUniqSpanned = true;
				if_const ( HANDLE_DUPES )
				{
					uQposFixed = m_dTermDupes[uQposFixed];
					bUniqSpanned = ( m_dTermsHit[uQposFixed]!=pHlist->m_uHitpos );
					m_dTermsHit[uQposFixed] = pHlist->m_uHitpos;
				}
				if ( bUniqSpanned )
					UpdateFreq ( uQposFixed, uField );
			}
			uQposSpanned++;
			uQposMask = ( uQposMask>>1 );
		}
	}

	if ( !m_iMinHitPos[uField] )
		m_iMinHitPos[uField] = iPos;

	// update hit window, max_window_hits factor
	if ( m_iWindowSize>1 )
	{
		if ( m_dWindow.GetLength() )
		{
			// sorted_remove_if ( _1 + winsize <= hitpos ) )
			int i = 0;
			while ( i<m_dWindow.GetLength() && ( m_dWindow[i] + m_iWindowSize )<=pHlist->m_uHitpos )
				i++;
			for ( int j=0; j<m_dWindow.GetLength()-i; j++ )
				m_dWindow[j] = m_dWindow[j+i];
			m_dWindow.Resize ( m_dWindow.GetLength()-i );
		}
		m_dWindow.Add ( pHlist->m_uHitpos );
		m_iMaxWindowHits[uField] = Max ( m_iMaxWindowHits[uField], m_dWindow.GetLength() );
	} else
		m_iMaxWindowHits[uField] = 1;

	// update exact_order factor
	if ( (int)uField!=m_iLastField )
	{
		m_iLastQuerypos = 0;
		m_iExactOrderWords = 0;
		m_iLastField = (int)uField;
	}
	if ( pHlist->m_uQuerypos==m_iLastQuerypos+1 )
	{
		if ( ++m_iExactOrderWords==m_iQueryWordCount )
			m_tExactOrder.BitSet ( uField );
		m_iLastQuerypos++;
	}

	// update min_gaps factor
	if ( bUniq && m_iHaveMinWindow>1 )
	{
		uQpos = pHlist->m_uQuerypos;
		if_const ( HANDLE_DUPES )
			uQpos = m_dTermDupes[uQpos];

		switch ( m_iHaveMinWindow )
		{
		// 2 keywords, special path
		case 2:
			if ( m_dMinWindowHits.GetLength() && HITMAN::GetField ( m_dMinWindowHits[0].m_uHitpos )!=(int)uField )
			{
				m_iMinWindowWords = 0;
				m_dMinWindowHits.Resize ( 0 );
			}

			if ( !m_dMinWindowHits.GetLength() )
			{
				m_dMinWindowHits.Add() = *pHlist; // {} => {A}
				m_dMinWindowHits.Last().m_uQuerypos = uQpos;
				break;
			}

			assert ( m_dMinWindowHits.GetLength()==1 );
			if ( uQpos==m_dMinWindowHits[0].m_uQuerypos )
				m_dMinWindowHits[0].m_uHitpos = pHlist->m_uHitpos;
			else
			{
				UpdateGap ( uField, 2, HITMAN::GetPos ( pHlist->m_uHitpos ) - HITMAN::GetPos ( m_dMinWindowHits[0].m_uHitpos ) - 1 );
				m_dMinWindowHits[0] = *pHlist;
				m_dMinWindowHits[0].m_uQuerypos = uQpos;
			}
			break;

		// 3 keywords, special path
		case 3:
			if ( m_dMinWindowHits.GetLength() && HITMAN::GetField ( m_dMinWindowHits.Last().m_uHitpos )!=(int)uField )
			{
				m_iMinWindowWords = 0;
				m_dMinWindowHits.Resize ( 0 );
			}

			// how many unique words are already there in the current candidate?
			switch ( m_dMinWindowHits.GetLength() )
			{
				case 0:
					m_dMinWindowHits.Add() = *pHlist; // {} => {A}
					m_dMinWindowHits.Last().m_uQuerypos = uQpos;
					break;

				case 1:
					if ( m_dMinWindowHits[0].m_uQuerypos==uQpos )
						m_dMinWindowHits[0] = *pHlist; // {A} + A2 => {A2}
					else
					{
						UpdateGap ( uField, 2, HITMAN::GetPos ( pHlist->m_uHitpos ) - HITMAN::GetPos ( m_dMinWindowHits[0].m_uHitpos ) - 1 );
						m_dMinWindowHits.Add() = *pHlist; // {A} + B => {A,B}
						m_dMinWindowHits.Last().m_uQuerypos = uQpos;
					}
					break;

				case 2:
					if ( m_dMinWindowHits[0].m_uQuerypos==uQpos )
					{
						UpdateGap ( uField, 2, HITMAN::GetPos ( pHlist->m_uHitpos ) - HITMAN::GetPos ( m_dMinWindowHits[1].m_uHitpos ) - 1 );
						m_dMinWindowHits[0] = m_dMinWindowHits[1]; // {A,B} + A2 => {B,A2}
						m_dMinWindowHits[1] = *pHlist;
						m_dMinWindowHits[1].m_uQuerypos = uQpos;
					} else if ( m_dMinWindowHits[1].m_uQuerypos==uQpos )
					{
						m_dMinWindowHits[1] = *pHlist; // {A,B} + B2 => {A,B2}
						m_dMinWindowHits[1].m_uQuerypos = uQpos;
					} else
					{
						// new {A,B,C} window!
						// handle, and then immediately reduce it to {B,C}
						UpdateGap ( uField, 3, HITMAN::GetPos ( pHlist->m_uHitpos ) - HITMAN::GetPos ( m_dMinWindowHits[0].m_uHitpos ) - 2 );
						m_dMinWindowHits[0] = m_dMinWindowHits[1];
						m_dMinWindowHits[1] = *pHlist;
						m_dMinWindowHits[1].m_uQuerypos = uQpos;
					}
					break;

				default:
					assert ( 0 && "min_gaps current window size not in 0..2 range; must not happen" );
			}
			break;

		// slow generic update
		default:
			UpdateMinGaps ( pHlist );
			break;
		}
	}
}


template < bool PF, bool HANDLE_DUPES >
void RankerState_Expr_fn<PF, HANDLE_DUPES>::UpdateFreq ( WORD uQpos, DWORD uField )
{
	float fIDF = m_dIDF [ uQpos ];
	DWORD uHitPosMask = 1<<uQpos;

	if ( !( m_uWordCount[uField] & uHitPosMask ) )
		m_dSumIDF[uField] += fIDF;

	if ( fIDF < m_dMinIDF[uField] )
		m_dMinIDF[uField] = fIDF;

	if ( fIDF > m_dMaxIDF[uField] )
		m_dMaxIDF[uField] = fIDF;

	m_uHitCount[uField]++;
	m_uWordCount[uField] |= uHitPosMask;
	m_uDocWordCount |= uHitPosMask;
	m_dTFIDF[uField] += fIDF;

	// avoid duplicate check for BM25A, BM25F though
	// (that sort of automatically accounts for qtf factor)
	m_dTF [ uQpos ]++;
	m_dFieldTF [ uField*(1+m_iMaxQpos) + uQpos ]++;
}


template < bool PF, bool HANDLE_DUPES >
void RankerState_Expr_fn<PF, HANDLE_DUPES>::UpdateMinGaps ( const ExtHit_t * pHlist )
{
	// update the minimum MW, aka matching window, for min_gaps and ymw factors
	// we keep a window with all the positions of all the matched words
	// we keep it left-minimal at all times, so that leftmost keyword only occurs once
	// thus, when a previously unseen keyword is added, the window is guaranteed to be minimal

	WORD uQpos = pHlist->m_uQuerypos;
	if_const ( HANDLE_DUPES )
		uQpos = m_dTermDupes[uQpos];

	// handle field switch
	const int iField = HITMAN::GetField ( pHlist->m_uHitpos );
	if ( m_dMinWindowHits.GetLength() && HITMAN::GetField ( m_dMinWindowHits.Last().m_uHitpos )!=iField )
	{
		m_dMinWindowHits.Resize ( 0 );
		m_dMinWindowCounts.Fill ( 0 );
		m_iMinWindowWords = 0;
	}

	// assert we are left-minimal
	assert ( m_dMinWindowHits.GetLength()==0 || m_dMinWindowCounts [ m_dMinWindowHits[0].m_uQuerypos ]==1 );

	// another occurrence of the trailing word?
	// just update hitpos, effectively dumping the current occurrence
	if ( m_dMinWindowHits.GetLength() && m_dMinWindowHits.Last().m_uQuerypos==uQpos )
	{
		m_dMinWindowHits.Last().m_uHitpos = pHlist->m_uHitpos;
		return;
	}

	// add that word
	LeanHit_t & t = m_dMinWindowHits.Add();
	t.m_uQuerypos = uQpos;
	t.m_uHitpos = pHlist->m_uHitpos;

	int iWord = uQpos;
	m_dMinWindowCounts[iWord]++;

	// new, previously unseen keyword? just update the window size
	if ( m_dMinWindowCounts[iWord]==1 )
	{
		m_iMinGaps[iField] = HITMAN::GetPos ( pHlist->m_uHitpos ) - HITMAN::GetPos ( m_dMinWindowHits[0].m_uHitpos ) - m_iMinWindowWords;
		m_iMinWindowWords++;
		return;
	}

	// check if we can shrink the left boundary
	if ( iWord!=m_dMinWindowHits[0].m_uQuerypos )
		return;

	// yes, we can!
	// keep removing the leftmost keyword until it's unique (in the window) again
	assert ( m_dMinWindowCounts [ m_dMinWindowHits[0].m_uQuerypos ]==2 );
	int iShrink = 0;
	while ( m_dMinWindowCounts [ m_dMinWindowHits [ iShrink ].m_uQuerypos ]!=1 )
	{
		m_dMinWindowCounts [ m_dMinWindowHits [ iShrink ].m_uQuerypos ]--;
		iShrink++;
	}

	int iNewLen = m_dMinWindowHits.GetLength() - iShrink;
	memmove ( m_dMinWindowHits.Begin(), &m_dMinWindowHits[iShrink], iNewLen*sizeof(LeanHit_t) );
	m_dMinWindowHits.Resize ( iNewLen );

	int iNewGaps = HITMAN::GetPos ( pHlist->m_uHitpos ) - HITMAN::GetPos ( m_dMinWindowHits[0].m_uHitpos ) - m_iMinWindowWords + 1;
	m_iMinGaps[iField] = Min ( m_iMinGaps[iField], iNewGaps );
}


template<bool A1, bool A2>
int RankerState_Expr_fn<A1,A2>::GetMaxPackedLength()
{
	return sizeof(DWORD)*( 8 + m_tExactHit.GetSize() + m_tExactOrder.GetSize() + m_iFields*15 + m_iMaxQpos*4 + m_dFieldTF.GetLength() );
}


template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
BYTE * RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::PackFactors()
{
	DWORD * pPackStart = (DWORD *)m_tFactorPool.Alloc();
	DWORD * pPack = pPackStart;
	assert ( pPackStart );

	// leave space for size
	pPack++;
	assert ( m_tMatchedFields.GetSize()==m_tExactHit.GetSize() && m_tExactHit.GetSize()==m_tExactOrder.GetSize() );

	// document level factors
	*pPack++ = m_uDocBM25;
	*pPack++ = sphF2DW ( m_fDocBM25A );
	*pPack++ = *m_tMatchedFields.Begin();
	*pPack++ = m_uDocWordCount;

	// field level factors
	*pPack++ = (DWORD)m_iFields;
	// v.6 set these depends on number of fields
	for ( int i=0; i<m_tExactHit.GetSize(); i++ )
		*pPack++ = *( m_tExactHit.Begin() + i );
	for ( int i=0; i<m_tExactOrder.GetSize(); i++ )
		*pPack++ = *( m_tExactOrder.Begin() + i );

	for ( int i=0; i<m_iFields; i++ )
	{
		DWORD uHit = m_uHitCount[i];
		*pPack++ = uHit;
		if ( uHit )
		{
			*pPack++ = (DWORD)i;
			*pPack++ = m_uLCS[i];
			*pPack++ = m_uWordCount[i];
			*pPack++ = sphF2DW ( m_dTFIDF[i] );
			*pPack++ = sphF2DW ( m_dMinIDF[i] );
			*pPack++ = sphF2DW ( m_dMaxIDF[i] );
			*pPack++ = sphF2DW ( m_dSumIDF[i] );
			*pPack++ = (DWORD)m_iMinHitPos[i];
			*pPack++ = (DWORD)m_iMinBestSpanPos[i];
			// had exact_hit here before v.4
			*pPack++ = (DWORD)m_iMaxWindowHits[i];
			*pPack++ = (DWORD)m_iMinGaps[i]; // added in v.3
			*pPack++ = sphF2DW ( m_dAtc[i] );			// added in v.4
			*pPack++ = m_dLCCS[i];					// added in v.5
			*pPack++ = sphF2DW ( m_dWLCCS[i] );	// added in v.5
		}
	}

	// word level factors
	*pPack++ = (DWORD)m_iMaxQpos;
	for ( int i=1; i<=m_iMaxQpos; i++ )
	{
		DWORD uKeywordMask = !IsTermSkipped(i); // !COMMIT !m_tExcluded.BitGet(i);
		*pPack++ = uKeywordMask;
		if ( uKeywordMask )
		{
			*pPack++ = (DWORD)i;
			*pPack++ = (DWORD)m_dTF[i];
			*pPack++ = *(DWORD*)&m_dIDF[i];
		}
	}

	// m_dFieldTF = iWord + iField * ( 1 + iWordsCount )
	// FIXME! pack these sparse factors ( however these should fit into fixed-size FactorPool block )
	*pPack++ = m_dFieldTF.GetLength();
	memcpy ( pPack, m_dFieldTF.Begin(), m_dFieldTF.GetLength()*sizeof(m_dFieldTF[0]) );
	pPack += m_dFieldTF.GetLength();

	*pPackStart = (pPack-pPackStart)*sizeof(DWORD);
	assert ( (pPack-pPackStart)*sizeof(DWORD)<=(DWORD)m_tFactorPool.GetElementSize() );
	return (BYTE*)pPackStart;
}


template <bool NEED_PACKEDFACTORS, bool HANDLE_DUPES>
bool RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::ExtraDataImpl ( ExtraData_e eType, void ** ppResult )
{
	if_const ( eType==EXTRA_SET_MVAPOOL || eType==EXTRA_SET_STRINGPOOL || NEED_PACKEDFACTORS )
	{
		switch ( eType )
		{
			case EXTRA_SET_MVAPOOL:
				m_pExpr->Command ( SPH_EXPR_SET_MVA_POOL, ppResult );
				return true;
			case EXTRA_SET_STRINGPOOL:
				m_pExpr->Command ( SPH_EXPR_SET_STRING_POOL, ppResult );
				return true;
			case EXTRA_SET_MAXMATCHES:
				m_iMaxMatches = *(int*)ppResult;
				return true;
			case EXTRA_SET_MATCHPUSHED:
				m_tFactorPool.AddRef ( *(SphDocID_t*)ppResult );
				return true;
			case EXTRA_SET_MATCHPOPPED:
				{
					const CSphTightVector<SphDocID_t> & dReleased = *(CSphTightVector<SphDocID_t>*)ppResult;
					ARRAY_FOREACH ( i, dReleased )
						m_tFactorPool.Release ( dReleased[i] );
				}
				return true;
			case EXTRA_GET_DATA_PACKEDFACTORS:
				*ppResult = m_tFactorPool.GetHashPtr();
				return true;
			case EXTRA_GET_DATA_RANKER_STATE:
				{
					SphExtraDataRankerState_t * pState = (SphExtraDataRankerState_t *)ppResult;
					pState->m_iFields = m_iFields;
					pState->m_pSchema = m_pSchema;
					pState->m_pFieldLens = m_pFieldLens;
					pState->m_iTotalDocuments = m_iTotalDocuments;
					pState->m_tFieldLensLoc = m_tFieldLensLoc;
					pState->m_iMaxQpos = m_iMaxQpos;
				}
				return true;
			case EXTRA_GET_POOL_SIZE:
				if_const ( NEED_PACKEDFACTORS )
				{
					*(int64_t*)ppResult = (int64_t)GetMaxPackedLength()*( m_iMaxMatches+ExtNode_i::MAX_DOCS );
					return true;
				} else
					return false;
			default:
				return false;
		}
	} else
		return false;
}


/// finish document processing, compute weight from factors
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
DWORD RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::Finalize ( const CSphMatch & tMatch )
{
#ifndef NDEBUG
	// sanity check
	for ( int i=0; i<m_iFields; ++i )
	{
		assert ( m_iMinHitPos[i]<=m_iMinBestSpanPos[i] );
		if ( m_uLCS[i]==1 )
			assert ( m_iMinHitPos[i]==m_iMinBestSpanPos[i] );
	}
#endif // NDEBUG

	// finishing touches
	FinalizeDocFactors ( tMatch );
	UpdateATC ( true );

	if_const ( NEED_PACKEDFACTORS )
	{
		// pack factors
		if ( !m_tFactorPool.IsInitialized() )
			m_tFactorPool.Prealloc ( GetMaxPackedLength(), m_iMaxMatches+ExtNode_i::MAX_DOCS );
		m_tFactorPool.AddToHash ( tMatch.m_uDocID, PackFactors() );
	}

	// compute expression
	DWORD uRes = ( m_eExprType==SPH_ATTR_INTEGER )
		? m_pExpr->IntEval ( tMatch )
		: (DWORD)m_pExpr->Eval ( tMatch );

	if_const ( HANDLE_DUPES )
	{
		m_uCurPos = 0;
		m_uLcsTailPos = 0;
		m_uLcsTailQposMask = 0;
		m_uCurQposMask = 0;
	}

	// cleanup
	ResetDocFactors();
	memset ( m_dLCCS, 0 , sizeof(m_dLCCS) );
	memset ( m_dWLCCS, 0, sizeof(m_dWLCCS) );
	m_iQueryPosLCCS = 0;
	m_iHitPosLCCS = 0;
	m_iLenLCCS = 0;
	m_fWeightLCCS = 0.0f;

	// done
	return uRes;
}


template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
bool RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::IsTermSkipped ( int iTerm )
{
	assert ( iTerm>=0 && iTerm<m_iMaxQpos+1 );
	if_const ( HANDLE_DUPES )
		return !m_tKeywords.BitGet ( iTerm ) || m_dTermDupes[iTerm]!=iTerm;
	else
		return !m_tKeywords.BitGet ( iTerm );
}


template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
float RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::TermTC ( int iTerm, bool bLeft )
{
	// border case short-cut
	if ( ( bLeft && iTerm==m_iAtcHitStart ) || ( !bLeft && iTerm==m_iAtcHitStart+m_iAtcHitCount-1 ) )
		return 0.0f;

	int iRing = iTerm % XRANK_ATC_BUFFER_LEN;
	int iHitpos = m_dAtcHits[iRing].m_iHitpos;
	WORD uQuerypos = m_dAtcHits[iRing].m_uQuerypos;

	m_dAtcProcessedTerms.Clear();

	float fTC = 0.0f;

	// loop bounds for down \ up climbing
	int iStart, iEnd, iStep;
	if ( bLeft )
	{
		iStart = iTerm - 1;
		iEnd = Max ( iStart - XRANK_ATC_WINDOW_LEN, m_iAtcHitStart-1 );
		iStep = -1;
	} else
	{
		iStart = iTerm + 1;
		iEnd = Min ( iStart + XRANK_ATC_WINDOW_LEN, m_iAtcHitStart + m_iAtcHitCount );
		iStep = 1;
	}

	int iFound = 0;
	for ( int i=iStart; i!=iEnd && iFound!=m_iMaxQpos; i+=iStep )
	{
		iRing = i % XRANK_ATC_BUFFER_LEN;
		const AtcHit_t & tCur = m_dAtcHits[iRing];
		bool bGotDup = ( uQuerypos==tCur.m_uQuerypos );

		if ( m_dAtcProcessedTerms.BitGet ( tCur.m_uQuerypos ) || iHitpos==tCur.m_iHitpos )
			continue;

		float fWeightedDist = pow ( float ( abs ( iHitpos - tCur.m_iHitpos ) ), XRANK_ATC_EXP );
		float fTermTC = ( m_dIDF[tCur.m_uQuerypos] / fWeightedDist );
		if ( bGotDup )
			fTermTC *= XRANK_ATC_DUP_DIV;
		fTC += fTermTC;

		m_dAtcProcessedTerms.BitSet ( tCur.m_uQuerypos );
		iFound++;
	}

	return fTC;
}


template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
void RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES>::UpdateATC ( bool bFlushField )
{
	if ( !m_iAtcHitCount )
		return;

	int iWindowStart = m_iAtcHitStart + XRANK_ATC_WINDOW_LEN;
	int iWindowEnd = Min ( iWindowStart + XRANK_ATC_WINDOW_LEN, m_iAtcHitStart+m_iAtcHitCount );
	// border cases (hits: +below ATC window collected since start of buffer; +at the end of buffer and less then ATC window)
	if ( !m_bAtcHeadProcessed )
		iWindowStart = m_iAtcHitStart;
	if ( bFlushField )
		iWindowEnd = m_iAtcHitStart+m_iAtcHitCount;

	assert ( iWindowStart<iWindowEnd && iWindowStart>=m_iAtcHitStart && iWindowEnd<=m_iAtcHitStart+m_iAtcHitCount );
	// per term ATC
	// sigma(t' E query) ( idf(t') \ left_deltapos(t, t')^z + idf (t') \ right_deltapos(t,t')^z ) * ( t==t' ? 0.25 : 1 )
	for ( int iWinPos=iWindowStart; iWinPos<iWindowEnd; iWinPos++ )
	{
		float fTC = TermTC ( iWinPos, true ) + TermTC ( iWinPos, false );

		int iRing = iWinPos % XRANK_ATC_BUFFER_LEN;
		m_dAtcTerms [ m_dAtcHits[iRing].m_uQuerypos ] += fTC;
	}

	m_bAtcHeadProcessed = true;
	if ( bFlushField )
	{
		float fWeightedSum = 0.0f;
		ARRAY_FOREACH ( i, m_dAtcTerms )
		{
			fWeightedSum += m_dAtcTerms[i] * m_dIDF[i];
			m_dAtcTerms[i] = 0.0f;
		}

		m_dAtc[m_uAtcField] = log ( 1.0f + fWeightedSum );
		m_iAtcHitStart = 0;
		m_iAtcHitCount = 0;
		m_bAtcHeadProcessed = false;
	}
}


/// expression ranker
template < bool NEED_PACKEDFACTORS, bool HANDLE_DUPES >
class ExtRanker_Expr_T : public ExtRanker_T< RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> >
{
public:
	ExtRanker_Expr_T ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup, const char * sExpr, const CSphSchema & tSchema )
		: ExtRanker_T< RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> > ( tXQ, tSetup )
	{
		// tricky bit, we stash the pointer to expr here, but it will be parsed
		// somewhat later during InitState() call, when IDFs etc are computed
		this->m_tState.m_sExpr = sExpr;
		this->m_tState.m_pSchema = &tSchema;
	}

	void SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
	{
		ExtRanker_T< RankerState_Expr_fn<NEED_PACKEDFACTORS, HANDLE_DUPES> >::SetQwordsIDF ( hQwords );
		this->m_tState.m_iMaxQpos = this->m_iMaxQpos;
		this->m_tState.SetQwords ( hQwords );
	}

	virtual int GetMatches ()
	{
		if_const ( NEED_PACKEDFACTORS )
			this->m_tState.FlushMatches ();

		return ExtRanker_T<RankerState_Expr_fn <NEED_PACKEDFACTORS, HANDLE_DUPES> >::GetMatches ();
	}

	virtual void SetTermDupes ( const ExtQwordsHash_t & hQwords, int iMaxQpos )
	{
		this->m_tState.SetTermDupes ( hQwords, iMaxQpos, this->m_pRoot );
	}
};

//////////////////////////////////////////////////////////////////////////
// EXPRESSION FACTORS EXPORT RANKER
//////////////////////////////////////////////////////////////////////////

/// ranker state that computes BM25 as weight, but also all known factors for export purposes
struct RankerState_Export_fn : public RankerState_Expr_fn<>
{
public:
	CSphOrderedHash < CSphString, SphDocID_t, IdentityHash_fn, 256 > m_hFactors;

public:
	DWORD Finalize ( const CSphMatch & tMatch )
	{
		// finalize factor computations
		FinalizeDocFactors ( tMatch );

		// build document level factors
		// FIXME? should we build query level factors too? max_lcs, query_word_count, etc
		const int MAX_STR_LEN = 1024;
		CSphVector<char> dVal;
		dVal.Resize ( MAX_STR_LEN );
		snprintf ( dVal.Begin(), dVal.GetLength(), "bm25=%d, bm25a=%f, field_mask=%d, doc_word_count=%d",
			m_uDocBM25, m_fDocBM25A, *m_tMatchedFields.Begin(), m_uDocWordCount );

		char sTmp[MAX_STR_LEN];

		// build field level factors
		for ( int i=0; i<m_iFields; i++ )
			if ( m_uHitCount[i] )
		{
			snprintf ( sTmp, MAX_STR_LEN, ", field%d="
				"(lcs=%d, hit_count=%d, word_count=%d, "
				"tf_idf=%f, min_idf=%f, max_idf=%f, sum_idf=%f, "
				"min_hit_pos=%d, min_best_span_pos=%d, exact_hit=%d, max_window_hits=%d)",
				i,
				m_uLCS[i], m_uHitCount[i], m_uWordCount[i],
				m_dTFIDF[i], m_dMinIDF[i], m_dMaxIDF[i], m_dSumIDF[i],
				m_iMinHitPos[i], m_iMinBestSpanPos[i], m_tExactHit.BitGet ( i ), m_iMaxWindowHits[i] );

			int iValLen = strlen ( dVal.Begin() );
			int iTotalLen = iValLen+strlen(sTmp);
			if ( dVal.GetLength() < iTotalLen+1 )
				dVal.Resize ( iTotalLen+1 );

			strcpy ( &(dVal[iValLen]), sTmp ); //NOLINT
		}

		// build word level factors
		for ( int i=1; i<=m_iMaxQpos; i++ )
			if ( !IsTermSkipped ( i ) )
		{
			snprintf ( sTmp, MAX_STR_LEN, ", word%d=(tf=%d, idf=%f)", i, m_dTF[i], m_dIDF[i] );
			int iValLen = strlen ( dVal.Begin() );
			int iTotalLen = iValLen+strlen(sTmp);
			if ( dVal.GetLength() < iTotalLen+1 )
				dVal.Resize ( iTotalLen+1 );

			strcpy ( &(dVal[iValLen]), sTmp ); //NOLINT
		}

		// export factors
		m_hFactors.Add ( dVal.Begin(), tMatch.m_uDocID );

		// compute sorting expression now
		DWORD uRes = ( m_eExprType==SPH_ATTR_INTEGER )
			? m_pExpr->IntEval ( tMatch )
			: (DWORD)m_pExpr->Eval ( tMatch );

		// cleanup and return!
		ResetDocFactors();
		return uRes;
	}

	virtual bool ExtraDataImpl ( ExtraData_e eType, void ** ppResult )
	{
		if ( eType==EXTRA_GET_DATA_RANKFACTORS )
			*ppResult = &m_hFactors;
		return true;
	}
};

/// export ranker that emits BM25 as the weight, but computes and export all the factors
/// useful for research purposes, eg. exporting the data for machine learning
class ExtRanker_Export_c : public ExtRanker_T<RankerState_Export_fn>
{
public:
	ExtRanker_Export_c ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup, const char * sExpr, const CSphSchema & tSchema )
		: ExtRanker_T<RankerState_Export_fn> ( tXQ, tSetup )
	{
		m_tState.m_sExpr = sExpr;
		m_tState.m_pSchema = &tSchema;
	}

	void SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
	{
		ExtRanker_T<RankerState_Export_fn>::SetQwordsIDF ( hQwords );
		m_tState.m_iMaxQpos = m_iMaxQpos;
		m_tState.SetQwords ( hQwords );
	}
};

//////////////////////////////////////////////////////////////////////////
// RANKER FACTORY
//////////////////////////////////////////////////////////////////////////

static void CheckQueryWord ( const char * szWord, CSphQueryResult * pResult, const CSphIndexSettings & tSettings )
{
	if ( ( !tSettings.m_iMinPrefixLen && !tSettings.m_iMinInfixLen ) || !szWord )
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


static void CheckExtendedQuery ( const XQNode_t * pNode, CSphQueryResult * pResult, const CSphIndexSettings & tSettings )
{
	ARRAY_FOREACH ( i, pNode->m_dWords )
		CheckQueryWord ( pNode->m_dWords[i].m_sWord.cstr(), pResult, tSettings );

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		CheckExtendedQuery ( pNode->m_dChildren[i], pResult, tSettings );
}


struct ExtQwordOrderbyQueryPos_t
{
	bool IsLess ( const ExtQword_t * pA, const ExtQword_t * pB ) const
	{
		return pA->m_iQueryPos<pB->m_iQueryPos;
	}
};


static bool HasQwordDupes ( XQNode_t * pNode, SmallStringHash_T<int> & hQwords )
{
	ARRAY_FOREACH ( i, pNode->m_dChildren )
		if ( HasQwordDupes ( pNode->m_dChildren[i], hQwords ) )
			return true;
	ARRAY_FOREACH ( i, pNode->m_dWords )
		if ( !hQwords.Add ( 1, pNode->m_dWords[i].m_sWord ) )
			return true;
	return false;
}


static bool HasQwordDupes ( XQNode_t * pNode )
{
	SmallStringHash_T<int> hQwords;
	return HasQwordDupes ( pNode, hQwords );
}


ISphRanker * sphCreateRanker ( const XQQuery_t & tXQ, const CSphQuery * pQuery, CSphQueryResult * pResult,
	const ISphQwordSetup & tTermSetup, const CSphQueryContext & tCtx )
{
	// shortcut
	const CSphIndex * pIndex = tTermSetup.m_pIndex;

	// check the keywords
	CheckExtendedQuery ( tXQ.m_pRoot, pResult, pIndex->GetSettings() );

	// fill payload mask
	DWORD uPayloadMask = 0;
	ARRAY_FOREACH ( i, pIndex->GetMatchSchema().m_dFields )
		uPayloadMask |= pIndex->GetMatchSchema().m_dFields[i].m_bPayload << i;

	bool bGotDupes = HasQwordDupes ( tXQ.m_pRoot );

	// setup eval-tree
	ExtRanker_c * pRanker = NULL;
	switch ( pQuery->m_eRanker )
	{
		case SPH_RANK_PROXIMITY_BM25:
			if ( uPayloadMask )
				pRanker = new ExtRanker_T < RankerState_ProximityPayload_fn<true> > ( tXQ, tTermSetup );
			else if ( tXQ.m_bSingleWord )
				pRanker = new ExtRanker_WeightSum_c<WITH_BM25> ( tXQ, tTermSetup );
			else if ( bGotDupes )
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true,true> > ( tXQ, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true,false> > ( tXQ, tTermSetup );
			break;
		case SPH_RANK_BM25:				pRanker = new ExtRanker_WeightSum_c<WITH_BM25> ( tXQ, tTermSetup ); break;
		case SPH_RANK_NONE:				pRanker = new ExtRanker_None_c ( tXQ, tTermSetup ); break;
		case SPH_RANK_WORDCOUNT:		pRanker = new ExtRanker_T < RankerState_Wordcount_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_PROXIMITY:
			if ( tXQ.m_bSingleWord )
				pRanker = new ExtRanker_WeightSum_c<> ( tXQ, tTermSetup );
			else if ( bGotDupes )
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<false,true> > ( tXQ, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<false,false> > ( tXQ, tTermSetup );
			break;
		case SPH_RANK_MATCHANY:			pRanker = new ExtRanker_T < RankerState_MatchAny_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_FIELDMASK:		pRanker = new ExtRanker_T < RankerState_Fieldmask_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_SPH04:			pRanker = new ExtRanker_T < RankerState_ProximityBM25Exact_fn > ( tXQ, tTermSetup ); break;
		case SPH_RANK_EXPR:
			{
				// we need that mask in case these factors usage:
				// min_idf,max_idf,sum_idf,hit_count,word_count,doc_word_count,tf_idf,tf,field_tf
				// however ranker expression got parsed later at Init stage
				// FIXME!!! move QposMask initialization past Init
				tTermSetup.m_bSetQposMask = true;
				bool bNeedFactors = !!(tCtx.m_uPackedFactorFlags & SPH_FACTOR_ENABLE);
				if ( bNeedFactors && bGotDupes )
					pRanker = new ExtRanker_Expr_T <true, true> ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() );
				else if ( bNeedFactors && !bGotDupes )
					pRanker = new ExtRanker_Expr_T <true, false> ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() );
				else if ( !bNeedFactors && bGotDupes )
					pRanker = new ExtRanker_Expr_T <false, true> ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() );
				else if ( !bNeedFactors && !bGotDupes )
					pRanker = new ExtRanker_Expr_T <false, false> ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() );
			}
			break;

		case SPH_RANK_EXPORT:
			// TODO: replace Export ranker to Expression ranker to remove duplicated code
			tTermSetup.m_bSetQposMask = true;
			pRanker = new ExtRanker_Export_c ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() );
			break;
		default:
			pResult->m_sWarning.SetSprintf ( "unknown ranking mode %d; using default", (int)pQuery->m_eRanker );
			if ( bGotDupes )
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true,true> > ( tXQ, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true,false> > ( tXQ, tTermSetup );
			break;
		case SPH_RANK_PLUGIN:
			{
				const PluginRanker_c * p = (const PluginRanker_c *) sphPluginGet ( PLUGIN_RANKER, pQuery->m_sUDRanker.cstr() );
				assert ( p );
				pRanker = new ExtRanker_T < RankerState_Plugin_fn > ( tXQ, tTermSetup );
				pRanker->ExtraData ( EXTRA_SET_RANKER_PLUGIN, (void**)p );
				pRanker->ExtraData ( EXTRA_SET_RANKER_PLUGIN_OPTS, (void**)pQuery->m_sUDRankerOpts.cstr() );
			}
			break;
	}
	assert ( pRanker );
	pRanker->m_uPayloadMask = uPayloadMask;

	// setup IDFs
	ExtQwordsHash_t hQwords;
	int iMaxQpos = pRanker->GetQwords ( hQwords );

	const int iQwords = hQwords.GetLength ();
	int64_t iTotalDocuments = pIndex->GetStats().m_iTotalDocuments;
	if ( tCtx.m_pLocalDocs )
		iTotalDocuments = tCtx.m_iTotalDocs;

	CSphVector<const ExtQword_t *> dWords;
	dWords.Reserve ( hQwords.GetLength() );

	hQwords.IterateStart ();
	while ( hQwords.IterateNext() )
	{
		ExtQword_t & tWord = hQwords.IterateGet ();

		int64_t iTermDocs = tWord.m_iDocs;
		// shared docs count
		if ( tCtx.m_pLocalDocs )
		{
			int64_t * pDocs = (*tCtx.m_pLocalDocs)( tWord.m_sWord );
			if ( pDocs )
				iTermDocs = *pDocs;
		}

		// build IDF
		float fIDF = 0.0f;
		if ( pQuery->m_bGlobalIDF )
			fIDF = pIndex->GetGlobalIDF ( tWord.m_sWord, iTermDocs, pQuery->m_bPlainIDF );
		else if ( iTermDocs )
		{
			// (word_docs > total_docs) case *is* occasionally possible
			// because of dupes, or delayed purging in RT, etc
			// FIXME? we don't expect over 4G docs per just 1 local index
			const int64_t iTotalClamped = Max ( iTotalDocuments, iTermDocs );

			if ( !pQuery->m_bPlainIDF )
			{
				// bm25 variant, idf = log((N-n+1)/n), as per Robertson et al
				//
				// idf \in [-log(N), log(N)]
				// weight \in [-NumWords*log(N), NumWords*log(N)]
				// we want weight \in [0, 1] range
				// we prescale idfs and get weight \in [-0.5, 0.5] range
				// then add 0.5 as our final step
				//
				// for the record, idf = log((N-n+0.5)/(n+0.5)) in the original paper
				// but our variant is a bit easier to compute, and has a better (symmetric) range
				float fLogTotal = logf ( float ( 1+iTotalClamped ) );
				fIDF = logf ( float ( iTotalClamped-iTermDocs+1 ) / float ( iTermDocs ) )
					/ ( 2*fLogTotal );
			} else
			{
				// plain variant, idf=log(N/n), as per Sparck-Jones
				//
				// idf \in [0, log(N)]
				// weight \in [0, NumWords*log(N)]
				// we prescale idfs and get weight in [0, 0.5] range
				// then add 0.5 as our final step
				float fLogTotal = logf ( float ( 1+iTotalClamped ) );
				fIDF = logf ( float ( iTotalClamped ) / float ( iTermDocs ) )
					/ ( 2*fLogTotal );
			}
		}

		// optionally normalize IDFs so that sum(TF*IDF) fits into [0, 1]
		if ( pQuery->m_bNormalizedTFIDF )
			fIDF /= iQwords;

		tWord.m_fIDF = fIDF * tWord.m_fBoost;
		dWords.Add ( &tWord );
	}

	dWords.Sort ( ExtQwordOrderbyQueryPos_t() );
	ARRAY_FOREACH ( i, dWords )
	{
		const ExtQword_t * pWord = dWords[i];
		if ( !pWord->m_bExpanded )
			pResult->AddStat ( pWord->m_sDictWord, pWord->m_iDocs, pWord->m_iHits );
	}

	pRanker->m_iMaxQpos = iMaxQpos;
	pRanker->SetQwordsIDF ( hQwords );
	if ( bGotDupes )
		pRanker->SetTermDupes ( hQwords, iMaxQpos );
	if ( !pRanker->InitState ( tCtx, pResult->m_sError ) )
		SafeDelete ( pRanker );
	return pRanker;
}

//////////////////////////////////////////////////////////////////////////
/// HIT MARKER
//////////////////////////////////////////////////////////////////////////

void CSphHitMarker::Mark ( CSphVector<SphHitMark_t> & dMarked )
{
	if ( !m_pRoot )
		return;

	const ExtHit_t * pHits = NULL;
	const ExtDoc_t * pDocs = NULL;

	pDocs = m_pRoot->GetDocsChunk();
	if ( !pDocs )
		return;

	for ( ;; )
	{
		pHits = m_pRoot->GetHitsChunk ( pDocs );
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
	ExtNode_i * pNode = NULL;
	if ( pRoot )
		pNode = ExtNode_i::Create ( pRoot, tSetup );

	if ( !pRoot || pNode )
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
/// (special container actually carries all the data and does the work, see below)
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

	virtual void HintDocid ( SphDocID_t ) {}

	virtual const ExtDoc_t * GetDocsChunk();

	virtual const ExtHit_t * GetHitsChunk ( const ExtDoc_t * pMatched );

	virtual int GetQwords ( ExtQwordsHash_t & hQwords )
	{
		if ( m_pChild )
			return m_pChild->GetQwords ( hQwords );
		return -1;
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

	virtual void GetTerms ( const ExtQwordsHash_t & hQwords, CSphVector<TermPos_t> & dTermDupes ) const
	{
		if ( m_pChild )
			m_pChild->GetTerms ( hQwords, dTermDupes );
	}

	virtual bool GotHitless ()
	{
		return ( m_pChild )
			? m_pChild->GotHitless()
			: false;
	}

	virtual uint64_t GetWordID() const
	{
		if ( m_pChild )
			return m_pChild->GetWordID();
		else
			return 0;
	}
};


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
	assert ( pChild );
	assert ( m_pSetup );

	m_iAtomPos = pChild->m_iAtomPos;
	const ExtDoc_t * pChunk = pChild->GetDocsChunk();
	int iStride = 0;

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
		{
			while (	( pHits = pChild->GetHitsChunk ( pChunkHits ) )!=NULL )
			{
				for ( ; pHits->m_uDocid!=DOCID_MAX; pHits++ )
				{
					m_Hits.Add ( *pHits );
					m_pNodeCache->m_iMaxCachedHits--;
				}
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

const ExtDoc_t * ExtNodeCached_t::GetDocsChunk()
{
	if ( !m_pNode || !m_pChild )
		return NULL;

	if ( !m_pNode->m_StateOk )
		return m_pChild->GetDocsChunk();

	if ( m_iMaxTimer>0 && sphMicroTimer()>=m_iMaxTimer )
	{
		if ( m_pWarning )
			*m_pWarning = "query time exceeded max_query_time";
		return NULL;
	}

	int iDoc = Min ( m_iDocIndex+MAX_DOCS-1, m_pNode->m_Docs.GetLength()-1 ) - m_iDocIndex;
	memcpy ( &m_dDocs[0], &m_pNode->m_Docs[m_iDocIndex], sizeof(ExtDoc_t)*iDoc );
	m_iDocIndex += iDoc;

	// funny trick based on the formula of FIDF calculation.
	for ( int i=0; i<iDoc; i++ )
		m_dDocs[i].m_fTFIDF /= m_iQwords;

	return ReturnDocsChunk ( iDoc, "cached" );
}

const ExtHit_t * ExtNodeCached_t::GetHitsChunk ( const ExtDoc_t * pMatched )
{
	if ( !m_pNode || !m_pChild )
		return NULL;

	if ( !m_pNode->m_StateOk )
		return m_pChild->GetHitsChunk ( pMatched );

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
	return m_pPool [ pRawChild->GetOrder() ].CreateCachedWrapper ( pChild, pRawChild, tSetup );
}

//
// $Id$
//
