//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
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

/// costs for max_predicted_time estimations, in nanoseconds
/// YMMV, defaults were estimated in a very specific environment, and then rounded off
int g_iPredictorCostDoc		= 64;
int g_iPredictorCostHit		= 48;
int g_iPredictorCostSkip	= 2048;
int g_iPredictorCostMatch	= 64;

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
	int			m_iQueryPos;	///< position in the query
	bool		m_bExpanded;	///< added by prefix expansion
	bool		m_bExcluded;	///< excluded by the query (eg. bb in (aa AND NOT bb))
};


/// query words set
typedef CSphOrderedHash < ExtQword_t, CSphString, QwordsHash_fn, 256 > ExtQwordsHash_t;

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

	explicit ZoneKey_t ( int iZone=0, SphDocID_t uDocid=0 )
		: m_iZone ( iZone )
		, m_uDocid ( uDocid )
	{}

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
typedef CSphOrderedHash < ZoneInfo_t, ZoneKey_t, ZoneHash_fn, 4096 > ZoneHash_c;

/// zonespan prototype
typedef CSphOrderedHash < int, ZoneKey_t, ZoneHash_fn, 8192*1024 > ZoneSpans_c;


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
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID ) = 0;
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID ) = 0;

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords ) = 0;
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords ) = 0;
	virtual bool				GotHitless () = 0;
	virtual int					GetDocsCount () { return INT_MAX; }
	virtual SphWordID_t			GetWordID () { return 0; }		///< for now, only used for duplicate keyword checks in quorum operator

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

//////////////////////////////////////////////////////////////////////////
class ZoneSpansHolder
{
	int	*						m_pZoneVec;

public:
	int							m_iNumZones;

public:
	explicit ZoneSpansHolder ( int iNumZones )
		: m_pZoneVec ( NULL )
		, m_iNumZones ( 0 )
	{
		Init ( iNumZones );
	}
	ZoneSpansHolder ()
		: m_pZoneVec ( NULL )
		, m_iNumZones ( 0 )
	{}

	void Init ( int iNumZones )
	{
		assert ( !m_pZoneVec );
		assert ( iNumZones );

		m_iNumZones = iNumZones;
		m_pZoneVec = new int [ iNumZones*ExtNode_i::MAX_HITS ];
		ResetZones();
	}

public:
	~ZoneSpansHolder()
	{
		SafeDeleteArray ( m_pZoneVec );
	}

	inline void ResetZones ()
	{
		assert ( m_pZoneVec );
		for ( int i=0; i<m_iNumZones*ExtNode_i::MAX_HITS; i++ )
			m_pZoneVec[i] = -1;
	}

	inline int GetRowByteSize() const
	{
		return m_iNumZones*sizeof(int); //NOLINT
	}

	inline int * GetZVec ( int iPos ) const
	{
		assert ( iPos>=0 );
		assert ( iPos<ExtNode_i::MAX_HITS );
		return m_pZoneVec+(iPos*m_iNumZones);
	}

	inline void CopyZVecTo ( int iSrc, int * pDest ) const
	{
		assert ( pDest );
		memcpy ( pDest, GetZVec ( iSrc ), GetRowByteSize() );
	}
};


/// single keyword streamer
class ExtTerm_c : public ExtNode_i, ISphNoncopyable
{
public:
	ExtTerm_c ( ISphQword * pQword, const CSphSmallBitvec& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted );
	ExtTerm_c ( ISphQword * pQword, const ISphQwordSetup & tSetup );
	ExtTerm_c () {} ///< to be used in pair with Init()
	~ExtTerm_c ()
	{
		SafeDelete ( m_pQword );
	}

	void						Init ( ISphQword * pQword, const CSphSmallBitvec& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted );
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				GotHitless () { return false; }
	virtual int					GetDocsCount () { return m_pQword->m_iDocs; }
	virtual SphWordID_t			GetWordID () { return m_pQword->m_iWordID; }

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
			for ( int iField = 0; iField < CSphSmallBitvec::iTOTALBITS; iField++ )
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
	ExtDoc_t *					m_pHitDoc;			///< points to entry in m_dDocs which GetHitsChunk() currently emits hits for
	SphDocID_t					m_uHitsOverFor;		///< there are no more hits for matches block starting with this ID
	CSphSmallBitvec				m_dQueriedFields;	///< accepted fields mask
	bool						m_bHasWideFields;	///< whether fields mask for this term refer to fields 32+
	float						m_fIDF;				///< IDF for this term (might be 0.0f for non-1st occurences in query)
	int64_t						m_iMaxTimer;		///< work until this timestamp
	CSphString *				m_pWarning;
	bool						m_bNotWeighted;
	CSphQueryStats *			m_pStats;
	int64_t *					m_pNanoBudget;

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
class ExtTermHitless_c : public ExtTerm_c
{
public:
	ExtTermHitless_c ( ISphQword * pQword, const CSphSmallBitvec& uFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
		: ExtTerm_c ( pQword, uFields, tSetup, bNotWeighted )
		, m_uFieldPos ( 0 )
	{}

	virtual void				Reset ( const ISphQwordSetup & ) { m_uFieldPos = 0; }
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	virtual bool				GotHitless () { return true; }

protected:
	DWORD	m_uFieldPos;
};

//////////////////////////////////////////////////////////////////////////

class HitPointer
{
protected:
	inline void					SetMyHit ( int, bool = false ) {}
	inline void					CopyMyHit ( int, int ) {}
};

/// position filter policy
template < TermPosFilter_e T >
class TermAcceptor_c : protected HitPointer
{
public:
								TermAcceptor_c ( ISphQword *, const XQNode_t *, const ISphQwordSetup & ) {}
protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const { return true; }
	inline void					Reset() {}
};

template<>
class TermAcceptor_c<TERM_POS_FIELD_LIMIT> : protected HitPointer
{
public:
	TermAcceptor_c ( ISphQword *, const XQNode_t * pNode, const ISphQwordSetup & )
		: m_iMaxFieldPos ( pNode->m_dSpec.m_iFieldMaxPos )
	{}
protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const;
	inline void					Reset()
	{
		m_iMaxFieldPos = 0;
	}
private:
	int							m_iMaxFieldPos;
};

template<>
class TermAcceptor_c<TERM_POS_ZONES> : protected HitPointer
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



template<>
class TermAcceptor_c<TERM_POS_ZONESPAN> : public TermAcceptor_c<TERM_POS_ZONES>
{
public:
	TermAcceptor_c ( ISphQword * pWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
		: TermAcceptor_c<TERM_POS_ZONES> ( pWord, pNode, tSetup)
		, m_iMyHit ( -1 )
		, m_bFinal ( false )
		, m_dMyZones ( pNode->m_dSpec.m_dZones.GetLength() )
		, m_dFinalZones ( pNode->m_dSpec.m_dZones.GetLength() )
	{}

protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * pHit ) const;
	inline void					Reset()
	{
		TermAcceptor_c<TERM_POS_ZONES>::Reset();
		m_dMyZones.ResetZones();
		m_dFinalZones.ResetZones();
	}

	inline void					SetMyHit ( int iHit, bool bFinal = false ) { m_iMyHit = iHit; m_bFinal = bFinal;}
	inline void					CopyMyHit ( int iSrc, int iDst ) { m_dMyZones.CopyZVecTo ( iSrc, m_dFinalZones.GetZVec ( iDst ) ); }

private:
	int							m_iMyHit;	///< the current num of hit in internal buffer
	bool						m_bFinal;	///< whether we point to our buffer, or temporary one for filtered zones
protected:
	ZoneSpansHolder				m_dMyZones;	///< extra buffer for filtered zones
	ZoneSpansHolder 			m_dFinalZones; ///< the actual buffer of the linked node
};

template<>
class TermAcceptor_c<TERM_POS_NONE> : public TermAcceptor_c<TERM_POS_ZONESPAN>
{
public:
	TermAcceptor_c ( ISphQword * pWord, const XQNode_t * pNode, const ISphQwordSetup & tSetup )
		: TermAcceptor_c<TERM_POS_ZONESPAN> ( pWord, pNode, tSetup)
	{}

protected:
	inline bool					IsAcceptableHit ( const ExtHit_t * ) const { return true; }
};

///
class BufferedNode_c
{
protected:
	BufferedNode_c ()
		: m_uTermMaxID ( 0 )
		, m_pRawDocs ( NULL )
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
		m_uTermMaxID = 0;
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
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
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


template<>
bool ExtConditional<TERM_POS_ZONESPAN>::ExtraDataImpl ( ExtraData_e eData, void ** ppResult )
{
	assert ( ppResult );
	if ( eData==EXTRA_GET_DATA_ZONESPANS )
	{
		*ppResult = &m_dFinalZones;
		return true;
	}
	return false;
}

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

	virtual void HintDocid ( SphDocID_t uMinID )
	{
		m_pChildren[0]->HintDocid ( uMinID );
		m_pChildren[1]->HintDocid ( uMinID );
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
								ExtAnd_c() {} ///< to be used with Init()
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );

	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtAnd", iLevel ); }
};

class ExtAndZonespanned : public ExtAnd_c
{
public:
	ExtAndZonespanned () {} ///< to be used in pair with Init()
	inline void						Init ( ExtNode_i * pFirst, ExtNode_i * pSecond, const ISphQwordSetup & tSetup, ZoneSpansHolder * pSpans )
	{
		ExtAnd_c::Init ( pFirst, pSecond, tSetup );
		m_pSpans = pSpans;
		m_pLastBaseHit[0] = NULL;
		m_pLastBaseHit[1] = NULL;
		if ( pFirst && !pFirst->GetExtraData ( EXTRA_GET_DATA_ZONESPANS, (void**) &m_dChildzones[0] ) )
			assert ( false );
		if ( pSecond && !pSecond->GetExtraData ( EXTRA_GET_DATA_ZONESPANS, (void**) &m_dChildzones[1] ) )
			assert ( false );
	}
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );
	void DebugDump ( int iLevel ) { DebugDumpT ( "ExtAndZonespan", iLevel ); }

private:
	ZoneSpansHolder *			m_dChildzones[2];
	ZoneSpansHolder	*			m_pSpans;
	const ExtHit_t *			m_pLastBaseHit[2];

private:
	bool IsSameZonespan ( int iHit, int iProofHit ) const;
};

class ExtAndZonespan_c : public ExtConditional < TERM_POS_NONE, ExtAndZonespanned >
{
public:
	ExtAndZonespan_c ( ExtNode_i *pFirst, ExtNode_i *pSecond, const ISphQwordSetup & tSetup, const XQNode_t * pNode )
		: ExtConditional<TERM_POS_NONE,ExtAndZonespanned> ( NULL, pNode, tSetup )
	{
		ExtAndZonespanned::Init ( pFirst, pSecond, tSetup, &m_dMyZones );
	}
private:
	bool ExtraDataImpl ( ExtraData_e eData, void ** ppResult )
	{
		assert ( ppResult );
		if ( eData==EXTRA_GET_DATA_ZONESPANS )
		{
			*ppResult = &m_dFinalZones;
			return true;
		}
		return false;
	}
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
								ExtNWayT ( const CSphVector<ExtNode_i *> & dNodes, const ISphQwordSetup & tSetup );
								~ExtNWayT ();
	virtual void				Reset ( const ISphQwordSetup & tSetup );
	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				GotHitless () { return false; }
	virtual void				HintDocid ( SphDocID_t uMinID ) { m_pNode->HintDocid ( uMinID ); }

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

template < class FSM >
class ExtNWay_c : public ExtNWayT, private FSM
{
public:
	ExtNWay_c ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & tSetup )
		: ExtNWayT ( dNodes, tSetup )
		, FSM ( dNodes, tNode, tSetup )
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
	struct State_t
	{
		int m_iTagQword;
		int m_iExpHitpos;
	};

protected:
	static const bool			bTermsTree = true;		///< we work with ExtTerm nodes

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
};
/// exact phrase streamer
typedef ExtNWay_c < FSMphrase > ExtPhrase_c;

/// proximity streamer
class FSMproximity
{
protected:
	static const bool			bTermsTree = true;		///< we work with ExtTerm nodes

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
};
/// exact phrase streamer
typedef ExtNWay_c<FSMproximity> ExtProximity_c;

/// proximity streamer
class FSMmultinear
{
protected:
	static const bool			bTermsTree = true;	///< we work with generic (not just ExtTerm) nodes

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
	virtual const ExtDoc_t *	GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *	GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID );

	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

	virtual bool				GotHitless () { return false; }

	virtual void HintDocid ( SphDocID_t uMinID )
	{
		ARRAY_FOREACH ( i, m_dChildren )
			m_dChildren[i]->HintDocid ( uMinID );
	}

	static int GetThreshold ( const XQNode_t & tNode, int iQwords );

protected:
	int							m_iThresh;			///< keyword count threshold
	CSphVector<ExtNode_i*>		m_dChildren;		///< my children nodes (simply ExtTerm_c for now)
	CSphVector<const ExtDoc_t*>	m_pCurDoc;			///< current positions into children doclists
	CSphVector<const ExtHit_t*>	m_pCurHit;			///< current positions into children hitlists
	CSphSmallBitvec				m_bmMask;			///< mask of nodes that count toward threshold
	int							m_iMaskEnd;			///< index of the last bit in mask
	bool						m_bDone;			///< am i done
	SphDocID_t					m_uMatchedDocid;	///< current docid for hitlist emission

private:
	CSphSmallBitvec				m_bmInitialMask;	///< backup mask for Reset()
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
	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool				GotHitless () { return false; }

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
	virtual int					GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void				SetQwordsIDF ( const ExtQwordsHash_t & hQwords );

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
	virtual bool				InitState ( const CSphQueryContext &, CSphString & )	{ return true; }

public:
	// FIXME? hide and friend?
	virtual SphZoneHit_e		IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan=0 );
	virtual const CSphIndex *	GetIndex() { return m_pIndex; }

public:
	CSphMatch					m_dMatches[ExtNode_i::MAX_DOCS];	///< exposed for caller
	DWORD						m_uPayloadMask;						///< exposed for ranker state functors
	int							m_iQwords;							///< exposed for ranker state functors
	int							m_iMaxQpos;							///< max in-query pos among all keywords, including dupes; for ranker state functors
	int							m_iMaxUniqQpos;						///< max in-query pos among unique keywords only; for ranker state functors

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
	int64_t *					m_pNanoBudget;

protected:
	CSphVector<CSphString>		m_dZones;
	CSphVector<ExtTerm_c*>		m_dZoneStartTerm;
	CSphVector<ExtTerm_c*>		m_dZoneEndTerm;
	CSphVector<const ExtDoc_t*>	m_dZoneStart;
	CSphVector<const ExtDoc_t*>	m_dZoneEnd;
	CSphVector<SphDocID_t>		m_dZoneMax;				///< last docid we (tried) to cache
	CSphVector<SphDocID_t>		m_dZoneMin;				///< first docid we (tried) to cache
	ZoneHash_c					m_hZoneInfo;
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
	ZoneSpansHolder *	m_pZones;
	const ExtHit_t *	m_pHitBase;
	CSphVector<int>		m_dZonespans; // zonespanlists for my matches

public:
					ExtRanker_T ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup );
	virtual int		GetMatches ();

	virtual bool InitState ( const CSphQueryContext & tCtx, CSphString & sError )
	{
		return m_tState.Init ( tCtx.m_iWeights, &tCtx.m_dWeights[0], this, sError );
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


static void CalcDupeMask ( CSphSmallBitvec * pMask, const CSphVector<ExtNode_i *> & dNodes )
{
	pMask->Unset();
	ARRAY_FOREACH ( i, dNodes )
	{
		bool bUniq = true;
		for ( int j = i + 1; j < dNodes.GetLength() && bUniq; j++ )
			if ( dNodes[i]->GetWordID()==dNodes[j]->GetWordID() )
				bUniq = false;
		if ( bUniq )
			pMask->Set(i);
	}
}


template < typename T >
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

		// FIXME! tricky combo again
		// quorum+expand used KeywordsEqual() path to drill down until actual nodes
		// that path is no longer and quorum+expand+dupes might probably fail now
		// the proper solution would be to have dNodes return proper wordids somehow
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

	if ( pNode->m_dSpec.m_iFieldMaxPos )
		pQword->m_iTermPos = TERM_POS_FIELD_LIMIT;

	if ( pNode->m_dSpec.m_dZones.GetLength() )
		pQword->m_iTermPos = pNode->m_dSpec.m_bZoneSpan ? TERM_POS_ZONESPAN : TERM_POS_ZONES;

	if ( !pQword->m_bHasHitlist )
	{
		if ( tSetup.m_pWarning && pQword->m_iTermPos )
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
		case TERM_POS_ZONESPAN:			return new ExtTermPos_c<TERM_POS_ZONESPAN> ( pQword, pNode, tSetup );
		default:						return new ExtTerm_c ( pQword, pNode->m_dSpec.m_dFieldMask, tSetup, pNode->m_bNotWeighted );
	}
}

//////////////////////////////////////////////////////////////////////////

struct ExtCacheEntry_t
{
	SphDocID_t	m_uDocid;
	Hitpos_t	m_uHitpos;
	union
	{
		int		m_iQword;	///< filled in ctor, used in SetQwordsIDF()
		float	m_fTFIDF;	///< filled in SetQwordsIDF(), used in searching
	};

	bool operator < ( const ExtCacheEntry_t & r ) const
	{
		if ( m_uDocid!=r.m_uDocid )
			return m_uDocid < r.m_uDocid;
		return m_uHitpos < r.m_uHitpos;
	}
};


struct ExtCachedKeyword_t : public XQKeyword_t
{
	CSphString	m_sDictWord;
	SphWordID_t	m_iWordID;
	float		m_fIDF;
	int			m_iDocs;
	int			m_iHits;
};

/// simple in-memory multi-term cache
class ExtCached_c : public ExtNode_i
{
protected:
	CSphVector<ExtCacheEntry_t>		m_dCache;
	CSphFixedVector<ExtCachedKeyword_t>	m_dWords;
	CSphSmallBitvec					m_dFieldMask;
	bool							m_bExcluded;

	int		m_iUniqDocs;

	int		m_iCurDocsBegin;	///< start of the last docs chunk returned, inclusive, ie [begin,end)
	int		m_iCurDocsEnd;		///< end of the last docs chunk returned, exclusive, ie [begin,end)
	int		m_iCurHit;			///< end of the last hits chunk (within the last docs chunk) returned, exclusive

public:
	explicit						ExtCached_c ( const XQNode_t * pNode, const ISphQwordSetup & tSetup );
	virtual void					Reset ( const ISphQwordSetup & tSetup );
	virtual void					HintDocid ( SphDocID_t ) {}
	virtual const ExtDoc_t *		GetDocsChunk ( SphDocID_t * pMaxID );
	virtual const ExtHit_t *		GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t );

	virtual int						GetQwords ( ExtQwordsHash_t & hQwords );
	virtual void					SetQwordsIDF ( const ExtQwordsHash_t & hQwords );
	virtual bool					GotHitless () { return false; }
	virtual int						GetDocsCount () { return m_iUniqDocs; }

	void							FillCacheIDF ();
	void							PopulateCache ( const ISphQwordSetup & tSetup, bool bFillStat );
};


ExtCached_c::ExtCached_c ( const XQNode_t * pNode, const ISphQwordSetup & tSetup )
	: 	m_dWords ( pNode->m_dWords.GetLength() )
{
#ifndef NDEBUG
	// sanity checks
	// this node must be only created for a huge OR of tiny expansions
	assert ( pNode->GetOp()==SPH_QUERY_OR );
	assert ( pNode->m_dWords.GetLength() );
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		assert ( pNode->m_dWords[i].m_iAtomPos==pNode->m_dWords[0].m_iAtomPos );
		assert ( pNode->m_dWords[i].m_bExpanded );
	}
#endif

	m_iAtomPos = pNode->m_dWords[0].m_iAtomPos;
	m_dFieldMask = pNode->m_dSpec.m_dFieldMask;
	m_bExcluded = pNode->m_bNotWeighted;
	m_dCache.Reserve ( pNode->m_dWords.GetLength() );

	// copy all the keywords
	BYTE sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		ExtCachedKeyword_t & tWord = m_dWords[i];
		tWord.m_sWord = pNode->m_dWords[i].m_sWord;
		tWord.m_bFieldStart = pNode->m_dWords[i].m_bFieldStart;
		tWord.m_bFieldEnd = pNode->m_dWords[i].m_bFieldEnd;
		tWord.m_uStarPosition = pNode->m_dWords[i].m_uStarPosition;
		tWord.m_iDocs = 0;
		tWord.m_iHits = 0;

		// our little stemming buffer (morphology aware dictionary might need to change the keyword)
		strncpy ( (char*)sTmpWord, tWord.m_sWord.cstr(), sizeof(sTmpWord) );
		sTmpWord[sizeof(sTmpWord)-1] = '\0';

		// setup keyword disk reader
		tWord.m_iWordID = tSetup.m_pDict->GetWordID ( sTmpWord );
		tWord.m_sDictWord = (const char *)sTmpWord;
	}

	PopulateCache ( tSetup, true );
}

void ExtCached_c::PopulateCache ( const ISphQwordSetup & tSetup, bool bFillStat )
{
	// loop all the keywords and precache them
	ARRAY_FOREACH ( i, m_dWords )
	{
		const ExtCachedKeyword_t & tWord = m_dWords[i];
		ISphQword * pQword = tSetup.QwordSpawn ( tWord );
		pQword->m_sWord = tWord.m_sWord;
		pQword->m_iWordID = tWord.m_iWordID;
		pQword->m_sDictWord = tWord.m_sDictWord;
		pQword->m_bExpanded = true;

		bool bOk = tSetup.QwordSetup ( pQword );

		// setup keyword idf and stats
		if ( bFillStat )
		{
			m_dWords[i].m_iDocs = pQword->m_iDocs;
			m_dWords[i].m_iHits = pQword->m_iHits;

			m_iUniqDocs += pQword->m_iDocs;
		}

		if ( !bOk )
		{
			SafeDelete ( pQword )
				continue;
		}

		// read and cache all docs and hits
		for ( ;; )
		{
			const CSphMatch & tMatch = pQword->GetNextDoc ( NULL );
			if ( !tMatch.m_iDocID )
				break;

			pQword->SeekHitlist ( pQword->m_iHitlistPos );
			for ( Hitpos_t uHit = pQword->GetNextHit(); uHit!=EMPTY_HIT; uHit = pQword->GetNextHit() )
			{
				// apply field limits
				if ( !m_dFieldMask.Test ( HITMAN::GetField(uHit) ) )
					continue;

				// FIXME!!! apply zone limits too

				// ok, this hit works, copy it
				ExtCacheEntry_t & tEntry = m_dCache.Add ();
				tEntry.m_uDocid = tMatch.m_iDocID;
				tEntry.m_uHitpos = uHit;
				tEntry.m_iQword = i;
			}
		}

		// dismissed
		SafeDelete ( pQword );
	}

	// sort from keyword order to document order
	m_dCache.Sort();
	if ( bFillStat && m_dCache.GetLength() )
	{
		m_iUniqDocs = 1;
		for ( int i=1; i<m_dCache.GetLength(); i++ )
			if ( m_dCache[i].m_uDocid!=m_dCache[i-1].m_uDocid )
				m_iUniqDocs++;
	}

	// reset iterators
	m_iCurDocsBegin = 0;
	m_iCurDocsEnd = 0;
	m_iCurHit = 0;
}


void ExtCached_c::Reset ( const ISphQwordSetup & tSetup )
{
	m_dCache.Resize ( 0 );
	PopulateCache ( tSetup, false );
	FillCacheIDF();
}


const ExtDoc_t * ExtCached_c::GetDocsChunk ( SphDocID_t * pMaxID )
{
	m_iCurDocsBegin = m_iCurHit = m_iCurDocsEnd;
	if ( m_iCurDocsBegin>=m_dCache.GetLength() )
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
		tDoc.m_fTFIDF = m_dCache[iEnd].m_fTFIDF;

		while ( iEnd<m_dCache.GetLength() && m_dCache[iEnd].m_uDocid==uDocid )
		{
			tDoc.m_uDocFields |= 1<< ( HITMAN::GetField ( m_dCache[iEnd].m_uHitpos ) );
			iEnd++;
		}
	}
	m_iCurDocsEnd = iEnd;

	return ReturnDocsChunk ( iDoc, pMaxID );
}


const ExtHit_t * ExtCached_c::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t )
{
	if ( m_iCurHit>=m_iCurDocsEnd )
		return NULL;

	int iHit = 0;
	while ( pDocs->m_uDocid!=DOCID_MAX )
	{
		// skip rejected documents
		while ( m_iCurHit < m_iCurDocsEnd && m_dCache[m_iCurHit].m_uDocid < pDocs->m_uDocid )
			m_iCurHit++;
		if ( m_iCurHit>=m_iCurDocsEnd )
			break;

		// skip non-matching documents
		SphDocID_t uDocid = m_dCache[m_iCurHit].m_uDocid;
		if ( pDocs->m_uDocid < uDocid )
		{
			while ( pDocs->m_uDocid < uDocid )
				pDocs++;
			if ( pDocs->m_uDocid!=uDocid )
				continue;
		}

		// copy accepted documents
		while ( m_iCurHit < m_iCurDocsEnd && m_dCache[m_iCurHit].m_uDocid==pDocs->m_uDocid && iHit < MAX_HITS-1 )
		{
			ExtHit_t & tHit = m_dHits[iHit++];
			tHit.m_uDocid = m_dCache[m_iCurHit].m_uDocid;
			tHit.m_uHitpos = m_dCache[m_iCurHit].m_uHitpos;
			tHit.m_uQuerypos = (WORD) m_iAtomPos;
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


int ExtCached_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ARRAY_FOREACH ( i, m_dWords )
	{
		const ExtCachedKeyword_t & tWord = m_dWords[i];

		ExtQword_t tQword;
		tQword.m_sWord = tWord.m_sWord;
		tQword.m_sDictWord = tWord.m_sDictWord;
		tQword.m_iDocs = tWord.m_iDocs;
		tQword.m_iHits = tWord.m_iHits;
		tQword.m_fIDF = -1.0f;
		tQword.m_iQueryPos = m_iAtomPos;
		tQword.m_bExpanded = true;
		tQword.m_bExcluded = m_bExcluded;

		hQwords.Add ( tQword, m_dWords[i].m_sWord );
		if ( !m_bExcluded )
			iMax = Max ( iMax, m_iAtomPos );
	}
	return iMax;
}


void ExtCached_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	// pull idfs
	ARRAY_FOREACH ( i, m_dWords )
	{
		ExtQword_t * pHashed = hQwords ( m_dWords[i].m_sWord );
		if ( pHashed )
			m_dWords[i].m_fIDF = pHashed->m_fIDF;
	}

	FillCacheIDF();
}


void ExtCached_c::FillCacheIDF ()
{
	// compute tf*idf for every document
	// store it into the opening hit
	CSphVector<int> dWords;
	for ( int i=0; i<m_dCache.GetLength(); )
	{
		// single hit fastpath
		if ( i+1==m_dCache.GetLength() || m_dCache[i+1].m_uDocid!=m_dCache[i].m_uDocid )
		{
			m_dCache[i].m_fTFIDF = m_dWords [ m_dCache[i].m_iQword ].m_fIDF / ( 1.0f+SPH_BM25_K1 );
			i++;
			continue;;
		}

		dWords.Resize ( 0 );
		int j;
		SphDocID_t uDocid = m_dCache[i].m_uDocid;
		for ( j=i; j<m_dCache.GetLength() && m_dCache[j].m_uDocid==uDocid; j++ )
			dWords.Add ( m_dCache[j].m_iQword );

		dWords.Sort();
		dWords.Add ( -1 );
		int iTF = 1;
		float fTFIDF = 0.0f;
		for ( int k=1; k<dWords.GetLength(); k++ )
		{
			if ( dWords[k]!=dWords[k-1] )
			{
				fTFIDF += float(iTF) / float(iTF+SPH_BM25_K1) * m_dWords[dWords[k-1]].m_fIDF;
				iTF = 1;
			} else
			{
				iTF++;
			}
		}

		m_dCache[i].m_fTFIDF = fTFIDF;
		i = j;
	}
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
					return CreateMultiNode<ExtQuorum_c> ( pNode, tSetup, false );

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

			case SPH_QUERY_OR:
				// currently, only intended for tiny (super-rare) expanded keywords
				// might be a nice small optimization to remove the expanded restriction, though
#ifndef NDEBUG
				assert ( pNode->m_dWords.GetLength() );
				ARRAY_FOREACH ( i, pNode->m_dWords )
					assert ( pNode->m_dWords[i].m_bExpanded );
#endif
				return new ExtCached_c ( pNode, tSetup );

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
					dTerms.Add ( ExtNode_i::Create ( pChild, tSetup ) );
				}

				dTerms.Sort ( ExtNodeTF_fn() );

				ExtNode_i * pCur = dTerms[0];
				for ( int i=1; i<iChildren; i++ )
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
				case SPH_QUERY_AND:			pCur = new ExtAnd_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_ANDNOT:		pCur = new ExtAndNot_c ( pCur, pNext, tSetup ); break;
				case SPH_QUERY_SENTENCE:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, MAGIC_WORD_SENTENCE ); break;
				case SPH_QUERY_PARAGRAPH:	pCur = new ExtUnit_c ( pCur, pNext, pNode->m_dSpec.m_dFieldMask, tSetup, MAGIC_WORD_PARAGRAPH ); break;
				default:					assert ( 0 && "internal error: unhandled op in ExtNode_i::Create()" ); break;
			}
		}
		if ( pNode->GetCount() )
			return tSetup.m_pNodeCache->CreateProxy ( pCur, pNode, tSetup );
		return pCur;
	}
}

//////////////////////////////////////////////////////////////////////////

inline void ExtTerm_c::Init ( ISphQword * pQword, const CSphSmallBitvec & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
{
	m_pQword = pQword;
	m_pWarning = tSetup.m_pWarning;
	m_bNotWeighted = bNotWeighted;
	m_iAtomPos = pQword->m_iAtomPos;
	m_pHitDoc = NULL;
	m_uHitsOverFor = 0;
	m_dQueriedFields = dFields;
	m_bHasWideFields = false;
	if ( tSetup.m_pIndex && tSetup.m_pIndex->GetMatchSchema().m_dFields.GetLength()>32 )
		for ( int i=1; i<8; i++ )
			if ( m_dQueriedFields.m_dFieldsMask[i] )
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
	m_pHitDoc = NULL;
	m_uHitsOverFor = 0;
	m_dQueriedFields.Set();
	m_bHasWideFields = tSetup.m_pIndex && ( tSetup.m_pIndex->GetMatchSchema().m_dFields.GetLength()>32 );
	m_iMaxTimer = tSetup.m_iMaxTimer;
	m_pStats = tSetup.m_pStats;
	m_pNanoBudget = m_pStats ? m_pStats->m_pNanoBudget : NULL;
	AllocDocinfo ( tSetup );
}

ExtTerm_c::ExtTerm_c ( ISphQword * pQword, const CSphSmallBitvec & dFields, const ISphQwordSetup & tSetup, bool bNotWeighted )
{
	Init ( pQword, dFields, tSetup, bNotWeighted );
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
		if ( !tMatch.m_iDocID )
		{
			m_pQword->m_iDocs = 0;
			break;
		}

		if ( !m_bHasWideFields )
		{
			// fields 0-31 can be quickly checked right here, right now
			if (!( m_pQword->m_dQwordFields.m_dFieldsMask[0] & m_dQueriedFields.m_dFieldsMask[0] ))
				continue;
		} else
		{
			// fields 32+ need to be checked with CollectHitMask() and stuff
			m_pQword->CollectHitMask();
			if (!( m_pQword->m_dQwordFields.Test ( m_dQueriedFields ) ))
				continue;
		}

		ExtDoc_t & tDoc = m_dDocs[iDoc++];
		tDoc.m_uDocid = tMatch.m_iDocID;
		tDoc.m_pDocinfo = pDocinfo;
		tDoc.m_uHitlistOffset = m_pQword->m_iHitlistPos;
		tDoc.m_uDocFields = m_pQword->m_dQwordFields.GetMask32() & m_dQueriedFields.GetMask32(); // OPTIMIZE: only needed for phrase node
		tDoc.m_fTFIDF = float(m_pQword->m_uMatchHits) / float(m_pQword->m_uMatchHits+SPH_BM25_K1) * m_fIDF;
		pDocinfo += m_iStride;
	}

	m_pHitDoc = NULL;

	if ( m_pStats )
		m_pStats->m_iFetchedDocs += iDoc;
	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostDoc*iDoc;

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

		if (!( m_dQueriedFields.Test ( HITMAN::GetField ( uHit ) ) ))
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

	if ( m_pStats )
		m_pStats->m_iFetchedHits += iHit;
	if ( m_pNanoBudget )
		*m_pNanoBudget -= g_iPredictorCostHit*iHit;

	assert ( iHit>=0 && iHit<MAX_HITS );
	m_dHits[iHit].m_uDocid = DOCID_MAX;
	return ( iHit!=0 ) ? m_dHits : NULL;
}

int ExtTerm_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	m_fIDF = 0.0f;
	if ( m_bNotWeighted || hQwords.Exists ( m_pQword->m_sWord ) )
		return m_pQword->m_bExcluded ? -1 : m_pQword->m_iAtomPos;

	m_fIDF = -1.0f;
	ExtQword_t tInfo;
	tInfo.m_sWord = m_pQword->m_sWord;
	tInfo.m_sDictWord = m_pQword->m_sDictWord;
	tInfo.m_iDocs = m_pQword->m_iDocs;
	tInfo.m_iHits = m_pQword->m_iHits;
	tInfo.m_iQueryPos = m_pQword->m_iAtomPos;
	tInfo.m_fIDF = -1.0f; // suppress gcc 4.2.3 warning
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

		m_uFieldPos = 0;
	}

	// hit emission
	int iHit = 0;
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
		SphZoneHit_e eState = m_pZoneChecker->IsInZone ( m_dZones[i], pHit );
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

inline bool TermAcceptor_c<TERM_POS_ZONESPAN>::IsAcceptableHit ( const ExtHit_t * pHit ) const
{
	assert ( m_pZoneChecker );
	int * pZones = ( m_bFinal ? m_dFinalZones.GetZVec ( m_iMyHit ) : m_dMyZones.GetZVec ( m_iMyHit ) );

	bool bRes = false;
	// only check zones that actually match this document
	ARRAY_FOREACH ( i, m_dZones )
		bRes |= ( m_pZoneChecker->IsInZone ( m_dZones[i], pHit, pZones + i )==SPH_ZONE_FOUND );
	return bRes;
}

template < TermPosFilter_e T, class ExtBase >
const ExtDoc_t * ExtConditional<T,ExtBase>::GetDocsChunk ( SphDocID_t * pMaxID )
{
	SphDocID_t uSkipID = m_uLastID;
	// fetch more docs if needed
	if ( !m_pRawDocs )
	{
		m_pRawDocs = ExtBase::GetDocsChunk ( &m_uTermMaxID );
		if ( !m_pRawDocs )
			return NULL;

		m_pRawDoc = m_pRawDocs;
		m_pRawHit = NULL;
		uSkipID = 0;
	}

	// filter the hits, and build the documents list
	int iMyDoc = 0;
	int iMyHit = 0;
	TermAcceptor_c<T>::SetMyHit(0);

	const ExtDoc_t * pDoc = m_pRawDoc; // just a shortcut
	const ExtHit_t * pHit = m_pRawHit;
	SphDocID_t uLastID = m_uLastID = 0;

	CSphRowitem * pDocinfo = ExtBase::m_pDocinfo;
	for ( ;; )
	{
		// try to fetch more hits for current raw docs block if we're out
		if ( !pHit || pHit->m_uDocid==DOCID_MAX )
			pHit = ExtBase::GetHitsChunk ( m_pRawDocs, m_uTermMaxID );

		// did we touch all the hits we had? if so, we're fully done with
		// current raw docs block, and should start a new one
		if ( !pHit )
		{
			m_pRawDocs = ExtBase::GetDocsChunk ( &m_uTermMaxID );
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
		TermAcceptor_c<T>::SetMyHit ( iMyHit );
		// copy acceptable hits for this document
		while ( iMyHit<ExtBase::MAX_HITS-1 && pHit->m_uDocid==uLastID )
		{
			if ( t_Acceptor::IsAcceptableHit ( pHit ) )
			{
				m_dMyHits[iMyHit++] = *pHit;
				TermAcceptor_c<T>::SetMyHit ( iMyHit );
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

	ExtBase::m_uMaxID = iMyDoc ? m_dMyDocs[iMyDoc-1].m_uDocid : 0;
	if ( pMaxID ) *pMaxID = ExtBase::m_uMaxID;

	return iMyDoc ? m_dMyDocs : NULL;
}


template < TermPosFilter_e T, class ExtBase >
const ExtHit_t * ExtConditional<T,ExtBase>::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
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
	TermAcceptor_c<T>::SetMyHit ( 0, true );

	if ( m_eState==COPY_FILTERED )
	{
		const ExtHit_t * pMyHits = m_dMyHits;
		const ExtHit_t * pMyHit = pMyHits;
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
				TermAcceptor_c<T>::CopyMyHit ( pMyHit - pMyHits, iFilteredHits );
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
			m_pRawHit = ExtBase::GetHitsChunk ( m_pRawDocs, Min ( uMaxID, m_uTermMaxID ) );

		// no more hits for current chunk
		if ( !m_pRawHit )
		{
			m_eState = COPY_DONE;
			break;
		}

		// copy while we can
		TermAcceptor_c<T>::SetMyHit ( iFilteredHits, true );
		while ( m_pRawHit->m_uDocid==m_uLastID && iFilteredHits<ExtBase::MAX_HITS-1 )
		{
			if ( t_Acceptor::IsAcceptableHit ( m_pRawHit ) )
			{
				m_dFilteredHits[iFilteredHits++] = *m_pRawHit;
				TermAcceptor_c<T>::SetMyHit ( iFilteredHits, true );
			}
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

			if ( !pCur0 )
			{
				if ( pCur1 && pCur1->m_uDocid!=DOCID_MAX )
					m_pChildren[0]->HintDocid ( pCur1->m_uDocid );
				pCur0 = m_pChildren[0]->GetDocsChunk ( NULL );
			}
			if ( !pCur1 )
			{
				if ( pCur0 && pCur0->m_uDocid!=DOCID_MAX )
					m_pChildren[1]->HintDocid ( pCur0->m_uDocid );
				pCur1 = m_pChildren[1]->GetDocsChunk ( NULL );
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
			// if one is over, we might still need to copy the other one. otherwise, skip it
			if ( ( pCur0 && pCur0->m_uDocid==m_uMatchedDocid ) || ( pCur1 && pCur1->m_uDocid==m_uMatchedDocid ) )
				continue;

			if ( pCur0 )
				while ( ( pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs, uMaxID ) )!=NULL );
			if ( pCur1 )
				while ( ( pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs, uMaxID ) )!=NULL );

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

bool ExtAndZonespanned::IsSameZonespan ( int iLeft, int iRight ) const
{
	assert ( m_dChildzones[0] );
	assert ( m_dChildzones[1] );
	assert ( m_pSpans );

	int * pLeft = m_dChildzones[0]->GetZVec(iLeft);
	int * pRight = m_dChildzones[1]->GetZVec(iRight);
	for ( int i = 0; i<m_pSpans->m_iNumZones; ++i )
		if ( pLeft[i]>=0 && pLeft[i]==pRight[i] )
			return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
const ExtHit_t * ExtAndZonespanned::GetHitsChunk ( const ExtDoc_t * pDocs, SphDocID_t uMaxID )
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
						if ( IsSameZonespan ( pCur0-m_pLastBaseHit[0], pCur1-m_pLastBaseHit[1] ) )
						{
							m_dHits[iHit] = *pCur0;
							if ( uNodePos0!=0 )
								m_dHits[iHit].m_uNodepos = uNodePos0;
							m_dChildzones[0]->CopyZVecTo ( pCur0-m_pLastBaseHit[0], m_pSpans->GetZVec ( iHit++ ) );
						}
						pCur0++;
						if ( pCur0->m_uDocid!=m_uMatchedDocid )
							break;
					} else
					{
						if ( IsSameZonespan ( pCur0-m_pLastBaseHit[0], pCur1-m_pLastBaseHit[1] ) )
						{
							m_dHits[iHit] = *pCur1;
							if ( uNodePos1!=0 )
								m_dHits[iHit].m_uNodepos = uNodePos1;
							m_dChildzones[1]->CopyZVecTo ( pCur1-m_pLastBaseHit[1], m_pSpans->GetZVec ( iHit++ ) );
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
			pCur0 = m_pChildren[0]->GetHitsChunk ( pDocs, uMaxID );
			m_pLastBaseHit[0] = pCur0;
		}
		if ( !pCur1 || pCur1->m_uDocid==DOCID_MAX )
		{
			pCur1 = m_pChildren[1]->GetHitsChunk ( pDocs, uMaxID );
			m_pLastBaseHit[1] = pCur1;
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

FSMphrase::FSMphrase ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & , const ISphQwordSetup & )
	: m_dAtomPos ( dQwords.GetLength() )
{
	ARRAY_FOREACH ( i, dQwords )
		m_dAtomPos[i] = dQwords[i]->m_iAtomPos;

	assert ( ( m_dAtomPos.Last()-m_dAtomPos[0]+1 )>0 );
	m_dQposDelta.Resize ( m_dAtomPos.Last()-m_dAtomPos[0]+1 );
	ARRAY_FOREACH ( i, m_dQposDelta )
		m_dQposDelta[i] = -INT_MAX;
	for ( int i=1; i<(int)m_dAtomPos.GetLength(); i++ )
		m_dQposDelta [ dQwords[i-1]->m_iAtomPos - dQwords[0]->m_iAtomPos ] = dQwords[i]->m_iAtomPos - dQwords[i-1]->m_iAtomPos;
}

inline bool FSMphrase::HitFSM ( const ExtHit_t* pHit, ExtHit_t* dTarget )
{
int iHitpos = HITMAN::GetLCS ( pHit->m_uHitpos );

	// adding start state for start hit
	if ( pHit->m_uQuerypos==m_dAtomPos[0] )
	{
		State_t & tState = m_dStates.Add();
		tState.m_iTagQword = 0;
		tState.m_iExpHitpos = iHitpos + m_dQposDelta[0];
	}

	// updating states
	for ( int i=m_dStates.GetLength()-1; i>=0; i-- )
	{
		if ( m_dStates[i].m_iExpHitpos<iHitpos )
		{
			m_dStates.RemoveFast(i); // failed to match
			continue;
		}

		// get next state
		if ( m_dStates[i].m_iExpHitpos==iHitpos && m_dAtomPos [ m_dStates[i].m_iTagQword+1 ]==pHit->m_uQuerypos )
		{
			m_dStates[i].m_iTagQword++; // check for next elm in query
			m_dStates[i].m_iExpHitpos = iHitpos + m_dQposDelta [ pHit->m_uQuerypos - m_dAtomPos[0] ];
		}

		// checking if state successfully matched
		if ( m_dStates[i].m_iTagQword==m_dAtomPos.GetLength()-1 )
		{
			DWORD uSpanlen = m_dAtomPos.Last() - m_dAtomPos[0];

			// emit directly into m_dHits, this is no need to disturb m_dMyHits here.
			dTarget->m_uDocid = pHit->m_uDocid;
			dTarget->m_uHitpos = iHitpos - uSpanlen;
			dTarget->m_uQuerypos = (WORD) m_dAtomPos[0];
			dTarget->m_uMatchlen = dTarget->m_uSpanlen = (WORD)( uSpanlen + 1 );
			dTarget->m_uWeight = m_dAtomPos.GetLength();
			ResetFSM ();
			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////

FSMproximity::FSMproximity ( const CSphVector<ExtNode_i *> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & )
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

FSMmultinear::FSMmultinear ( const CSphVector<ExtNode_i *> & dNodes, const XQNode_t & tNode, const ISphQwordSetup & )
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

ExtQuorum_c::ExtQuorum_c ( CSphVector<ExtNode_i*> & dQwords, const XQNode_t & tNode, const ISphQwordSetup & )
{
	assert ( tNode.GetOp()==SPH_QUERY_QUORUM );

	m_bDone = false;
	m_iThresh = GetThreshold ( tNode, dQwords.GetLength() );
	m_iThresh = Max ( m_iThresh, 1 );

	assert ( dQwords.GetLength()>1 ); // use TERM instead
	assert ( dQwords.GetLength()<=256 ); // internal masks are upto 256 bits
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

	// compute duplicate keywords mask (aka dupe mask)
	// FIXME! will (likely) now fail with quorum+expand+dupes, need a new test?
	// FIXME! will fail with wordforms and stuff; sorry, no wordforms vs expand vs quorum support for now!
	CSphSmallBitvec bmDupes;
	CalcDupeMask ( &bmDupes, dQwords );
	m_bmMask = m_bmInitialMask = bmDupes;
	m_iMaskEnd = dQwords.GetLength() - 1;
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

	m_bmMask = m_bmInitialMask;
	m_iMaskEnd = m_dChildren.GetLength() - 1;
	m_uMatchedDocid = 0;

	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->Reset ( tSetup );
}

int ExtQuorum_c::GetQwords ( ExtQwordsHash_t & hQwords )
{
	int iMax = -1;
	ARRAY_FOREACH ( i, m_dChildren )
	{
		int iKidMax = m_dChildren[i]->GetQwords ( hQwords );
		iMax = Max ( iMax, iKidMax );
	}
	return iMax;
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
		m_bmMask.Unset(i); // clear i-th bit
		if ( m_bmMask.Test ( m_iMaskEnd ) )
			m_bmMask.Set(i); // set i-th bit to end bit; OPTIMIZE? can be made unconditional
		m_iMaskEnd--;

		m_dChildren.RemoveFast ( i );
		m_pCurDoc.RemoveFast ( i );
		m_pCurHit.RemoveFast ( i );
		i--;
	}

	// early out
	if ( m_bDone )
		return NULL;

	// main loop
	CSphSmallBitvec bmTouched; // bitmask of children that actually produced matches this time
	bmTouched.Unset();

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
		tCand.m_uDocFields = 0; // non necessary
		tCand.m_fTFIDF = 0.0f;

		int iCandMatches = 0; // amount of children that match current candidate
		ARRAY_FOREACH ( i, m_pCurDoc )
		{
			assert ( m_pCurDoc[i]->m_uDocid && m_pCurDoc[i]->m_uDocid!=DOCID_MAX );
			if ( m_pCurDoc[i]->m_uDocid < tCand.m_uDocid )
			{
				tCand = *m_pCurDoc[i];
				iCandMatches = m_bmMask.Test(i);

			} else if ( m_pCurDoc[i]->m_uDocid==tCand.m_uDocid )
			{
				tCand.m_uDocFields |= m_pCurDoc[i]->m_uDocFields; // non necessary
				tCand.m_fTFIDF += m_pCurDoc[i]->m_fTFIDF;
				iCandMatches += m_bmMask.Test(i);
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
				bmTouched.Set(i);

			m_pCurDoc[i]++;
			if ( m_pCurDoc[i]->m_uDocid!=DOCID_MAX )
				continue;

			if ( bmTouched.Test(i) )
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
			m_bmMask.Unset(i); // clear i-th bit
			if ( m_bmMask.Test ( m_iMaskEnd ) )
				m_bmMask.Set(i); // set i-th bit to end bit; OPTIMIZE? can be made unconditional
			m_iMaskEnd--;

			bmTouched.Unset(i);
			if ( bmTouched.Test ( m_dChildren.GetLength()-1 ) )
				bmTouched.Set(i);

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

int ExtQuorum_c::GetThreshold ( const XQNode_t & tNode, int iQwords )
{
	return ( tNode.m_bPercentOp ? (int)floor ( 1.0f / 100.0f * tNode.m_iOpArg * iQwords + 0.5 ) : tNode.m_iOpArg );
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

//////////////////////////////////////////////////////////////////////////

ExtUnit_c::ExtUnit_c ( ExtNode_i * pFirst, ExtNode_i * pSecond, const CSphSmallBitvec& uFields,
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
	m_pNanoBudget = tSetup.m_pStats ? tSetup.m_pStats->m_pNanoBudget : NULL;

	m_dZones = tXQ.m_dZones;
	m_dZoneStart.Resize ( m_dZones.GetLength() );
	m_dZoneEnd.Resize ( m_dZones.GetLength() );
	m_dZoneMax.Resize ( m_dZones.GetLength() );
	m_dZoneMin.Resize ( m_dZones.GetLength() );
	m_dZoneMax.Fill ( 0 );
	m_dZoneMin.Fill	( DOCID_MAX );
	m_bZSlist = tXQ.m_bNeedSZlist;

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

				Verify ( m_hZoneInfo.IterateStart ( ZoneKey_t ( i, uMinDocid ) ) );
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
			if ( m_pNanoBudget )
				*m_pNanoBudget -= g_iPredictorCostMatch*iDocs;
			m_dMyDocs[iDocs].m_uDocid = DOCID_MAX;
			return m_dMyDocs;
		}
	}
}


void ExtRanker_c::SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
{
	m_iQwords = hQwords.GetLength ();
	if ( m_pRoot )
		m_pRoot->SetQwordsIDF ( hQwords );
}


SphZoneHit_e ExtRanker_c::IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan )
{
	// quick route, we have current docid cached
	ZoneKey_t tKey ( iZone, pHit->m_uDocid ); // OPTIMIZE? allow 2-component hash keys maybe?
	ZoneInfo_t * pZone = m_hZoneInfo ( tKey );
	if ( pZone )
	{
		// remove end markers that might mess up ordering
		Hitpos_t uPos = HITMAN::GetLCS ( pHit->m_uHitpos );
		int iSpan = FindSpan ( pZone->m_dStarts, uPos );
		if ( iSpan<0 || uPos>pZone->m_dEnds[iSpan] )
			return SPH_ZONE_NO_SPAN;
		if ( pLastSpan )
			*pLastSpan = iSpan;
		return SPH_ZONE_FOUND;
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
			pZone = &m_hZoneInfo.AddUnique ( tKey );

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
					pStartHits = m_dZoneStartTerm[iZone]->GetHitsChunk ( dCache, DOCID_MAX );
				if ( pEndHits->m_uDocid==DOCID_MAX )
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
		if ( iSpan<0 || uPos>pZone->m_dEnds[iSpan] )
			return SPH_ZONE_NO_SPAN;
		if ( pLastSpan )
			*pLastSpan = iSpan;
		return SPH_ZONE_FOUND;
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
ExtRanker_T<STATE>::ExtRanker_T ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup ) : ExtRanker_c ( tXQ, tSetup )
{
	if ( m_bZSlist && !m_pRoot->GetExtraData ( EXTRA_GET_DATA_ZONESPANS, (void**) & m_pZones ))
		m_bZSlist = false;
	if ( m_bZSlist )
	{
		m_dZonespans.Reserve ( ExtNode_i::MAX_DOCS );
	}
	m_dZonespans.Resize ( 1 );
	m_pHitBase = NULL;
}

template < typename STATE >
int ExtRanker_T<STATE>::GetMatches ()
{
	if ( !m_pRoot )
		return 0;

	int iMatches = 0;
	const ExtHit_t * pHlist = m_pHitlist;
	const ExtHit_t * pHitBase = m_pHitBase;
	const ExtDoc_t * pDocs = m_pDoclist;
	m_dZonespans.Resize(1);
	int		iLastZoneData = 0;

	bool bZSlist = m_bZSlist;
	CSphVector<int> dSpans;
	if ( bZSlist )
	{
		dSpans.Resize ( m_dZones.GetLength() );
		ARRAY_FOREACH ( i, dSpans )
			dSpans[i] = -1;
	}

	// warmup if necessary
	if ( !pHlist )
	{
		if ( !pDocs ) pDocs = GetFilteredDocs ();
		if ( !pDocs ) return iMatches;

		pHlist = m_pRoot->GetHitsChunk ( pDocs, m_uMaxID );
		if ( !pHlist ) return iMatches;
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
			if ( bZSlist )
			{
				int * pZones = m_pZones->GetZVec ( pHlist-pHitBase );
				ARRAY_FOREACH ( i, m_dZones )
					if ( pZones[i]>=0 && dSpans[i]!=pZones[i] )
					{
						m_dZonespans.Add ( i );
						m_dZonespans.Add ( pZones[i] );
						dSpans[i] = pZones[i];
					}
			}
			++pHlist;
		}

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
			if ( bZSlist )
			{
				m_dZonespans[iLastZoneData] = m_dZonespans.GetLength()-iLastZoneData-1;
				m_dMatches[iMatches].m_iTag = iLastZoneData;
				iLastZoneData = m_dZonespans.GetLength();
				m_dZonespans.Add(0);
				ARRAY_FOREACH ( i, dSpans )
					dSpans[i] = -1;
			}
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
	if ( !m_pHitBase )
		m_pHitBase = pHitBase;
	return iMatches;
}

//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#pragma warning(disable:4127) // conditional expr is const for MSVC
#endif

template < bool USE_BM25, bool HANDLE_DUPES >
struct RankerState_Proximity_fn : public ISphExtra
{
	BYTE m_uLCS[SPH_MAX_FIELDS];
	BYTE m_uCurLCS;
	int m_iExpDelta;
	int m_iFields;
	const int * m_pWeights;

	DWORD m_uLcsTailPos;
	DWORD m_uLcsTailQposMask;
	DWORD m_uCurQposMask;
	DWORD m_uCurPos;

	bool Init ( int iFields, const int * pWeights, ExtRanker_c *, CSphString & )
	{
		memset ( m_uLCS, 0, sizeof(m_uLCS) );
		m_uCurLCS = 0;
		m_iExpDelta = -INT_MAX;
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
		if ( !HANDLE_DUPES )
		{
			// all query keywords are unique
			// simpler path (just do the delta)
			int iDelta = HITMAN::GetLCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
			if ( iDelta==m_iExpDelta )
				m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
			else
				m_uCurLCS = BYTE(pHlist->m_uWeight);

			DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
			if ( m_uCurLCS>m_uLCS[uField] )
				m_uLCS[uField] = m_uCurLCS;

			m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1; // !COMMIT why spanlen??
		} else
		{
			// keywords are duplicated in the query
			// so there might be multiple qpos entries sharing the same hitpos
			DWORD uPos = HITMAN::GetLCS ( pHlist->m_uHitpos );
			DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );

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

		if ( HANDLE_DUPES )
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

#if USE_WINDOWS
#pragma warning(default:4127) // conditional expr is const for MSVC
#endif

//////////////////////////////////////////////////////////////////////////

// sph04, proximity + exact boost
struct RankerState_ProximityBM25Exact_fn : public ISphExtra
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
		int iDelta = HITMAN::GetLCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
		if ( iDelta==m_iExpDelta && HITMAN::GetLCS ( pHlist->m_uHitpos )>=m_uMinExpPos )
		{
			m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
			if ( HITMAN::IsEnd ( pHlist->m_uHitpos )
				&& (int)pHlist->m_uQuerypos==m_iMaxQuerypos
				&& HITMAN::GetPos ( pHlist->m_uHitpos )==m_iMaxQuerypos )
			{
				m_uExactHit |= ( 1UL << HITMAN::GetField ( pHlist->m_uHitpos ) );
			}
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
struct RankerState_ProximityPayload_fn : public RankerState_Proximity_fn<USE_BM25,false>
{
	DWORD m_uPayloadRank;
	DWORD m_uPayloadMask;

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError )
	{
		RankerState_Proximity_fn<USE_BM25,false>::Init ( iFields, pWeights, pRanker, sError );
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

	bool Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError )
	{
		RankerState_Proximity_fn<false,false>::Init ( iFields, pWeights, pRanker, sError );
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

struct RankerState_Fieldmask_fn : public ISphExtra
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

struct FactorHashEntry_t
{
	SphDocID_t			m_iId;
	int					m_iRefCount;
	BYTE *				m_pData;
	FactorHashEntry_t *	m_pPrev;
	FactorHashEntry_t *	m_pNext;
};


class FactorPool_c
{
public:
					FactorPool_c ();

	void			Prealloc ( int iElementSize, int nElements );
	BYTE *			Alloc ();
	void			Free ( BYTE * pPtr );
	int				GetElementSize() const;
	int				GetIntElementSize () const;
	void			AddToHash ( SphDocID_t iId, BYTE * pPacked );
	void			AddRef ( SphDocID_t iId );
	void			Release ( SphDocID_t iId );
	void			Flush ();

	bool			IsInitialized() const;
	CSphTightVector<FactorHashEntry_t *> * GetHashPtr();

private:
	int				m_iElementSize;
	int				m_iNextFree;

	CSphTightVector<BYTE>	m_dPool;
	CSphTightVector<int>	m_dFree;
	CSphTightVector<FactorHashEntry_t *> m_dHash;

	FactorHashEntry_t * Find ( SphDocID_t iId ) const;
	inline DWORD	HashFunc ( SphDocID_t iId ) const;
	bool			FlushEntry ( FactorHashEntry_t * pEntry );
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
	return m_iElementSize+sizeof(FactorHashEntry_t);
}


int	FactorPool_c::GetElementSize() const
{
	return m_iElementSize;
}


void FactorPool_c::AddToHash ( SphDocID_t iId, BYTE * pPacked )
{
	FactorHashEntry_t * pNew = (FactorHashEntry_t *)(pPacked+m_iElementSize);
	memset ( pNew, 0, sizeof(FactorHashEntry_t) );

	DWORD uKey = HashFunc(iId);
	if ( m_dHash[uKey] )
	{
		FactorHashEntry_t * pStart = m_dHash[uKey];
		pNew->m_pPrev = NULL;
		pNew->m_pNext = pStart;
		pStart->m_pPrev = pNew;
	}

	pNew->m_pData = pPacked;
	pNew->m_iId = iId;
	m_dHash[uKey] = pNew;
}


FactorHashEntry_t * FactorPool_c::Find ( SphDocID_t iId ) const
{
	DWORD uKey = HashFunc(iId);
	if ( m_dHash[uKey] )
	{
		FactorHashEntry_t * pEntry = m_dHash[uKey];
		while ( pEntry )
		{
			if ( pEntry->m_iId==iId )
				return pEntry;

			pEntry = pEntry->m_pNext;
		}
	}

	return NULL;
}


void FactorPool_c::AddRef ( SphDocID_t iId )
{
	if ( !iId )
		return;

	FactorHashEntry_t * pEntry = Find ( iId );
	if ( pEntry )
		pEntry->m_iRefCount++;
}


void FactorPool_c::Release ( SphDocID_t iId )
{
	if ( !iId )
		return;

	FactorHashEntry_t * pEntry = Find ( iId );
	if ( pEntry )
	{
		pEntry->m_iRefCount--;
		bool bHead = !pEntry->m_pPrev;
		FactorHashEntry_t * pNext = pEntry->m_pNext;
		if ( FlushEntry ( pEntry ) && bHead )
			m_dHash[HashFunc(iId)] = pNext;
	}
}


bool FactorPool_c::FlushEntry ( FactorHashEntry_t * pEntry )
{
	assert ( pEntry->m_iRefCount>=0 );
	if ( pEntry->m_iRefCount )
		return false;

	assert ( pEntry );
	if ( pEntry->m_pPrev )
		pEntry->m_pPrev->m_pNext = pEntry->m_pNext;

	if ( pEntry->m_pNext )
		pEntry->m_pNext->m_pPrev = pEntry->m_pPrev;

	Free ( pEntry->m_pData );

	return true;
};


void FactorPool_c::Flush()
{
	ARRAY_FOREACH ( i, m_dHash )
	{
		FactorHashEntry_t * pEntry = m_dHash[i];
		while ( pEntry )
		{
			FactorHashEntry_t * pNext = pEntry->m_pNext;
			bool bHead = !pEntry->m_pPrev;
			if ( FlushEntry(pEntry) && bHead )
				m_dHash[i] = pNext;

			pEntry = pNext;
		}
	}
}


inline DWORD FactorPool_c::HashFunc ( SphDocID_t iId ) const
{
	return iId % m_dHash.GetLength();
}


bool FactorPool_c::IsInitialized() const
{
	return !!m_iElementSize;
}


CSphTightVector<FactorHashEntry_t *> * FactorPool_c::GetHashPtr ()
{
	return &m_dHash;
}


//////////////////////////////////////////////////////////////////////////
// EXPRESSION RANKER
//////////////////////////////////////////////////////////////////////////

/// ranker state that computes weight dynamically based on user supplied expression (formula)
template < bool NEED_PACKEDFACTORS = false >
struct RankerState_Expr_fn : public ISphExtra
{
public:
	// per-field and per-document stuff
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
	float				m_dMinIDF[SPH_MAX_FIELDS];
	float				m_dMaxIDF[SPH_MAX_FIELDS];
	float				m_dSumIDF[SPH_MAX_FIELDS];
	int					m_iMinHitPos[SPH_MAX_FIELDS];
	int					m_iMinBestSpanPos[SPH_MAX_FIELDS];
	DWORD				m_uExactHit;
	CSphBitvec			m_tKeywordMask;
	DWORD				m_uDocWordCount;
	int					m_iMaxWindowHits[SPH_MAX_FIELDS];
	CSphVector<int>		m_dTF;			///< for bm25a
	float				m_fDocBM25A;	///< for bm25a
	CSphVector<int>		m_dFieldTF;		///< for bm25f, per-field layout (ie all field0 tfs, then all field1 tfs, etc)

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
	int					m_iMaxUniqQpos;		///< among 1st occurrence of keywords in the query only

	FactorPool_c 		m_tFactorPool;
	int					m_iMaxMatches;

	// per-query stuff
	int					m_iMaxLCS;
	int					m_iQueryWordCount;

public:
	// internal state, and factor settings
	CSphVector<DWORD>	m_dWindow;
	int					m_iWindowSize;

public:
						RankerState_Expr_fn ();
						~RankerState_Expr_fn ();

	bool				Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError );
	void				Update ( const ExtHit_t * pHlist );
	DWORD				Finalize ( const CSphMatch & tMatch );

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

		m_iQueryWordCount = 0;
		m_tKeywordMask.Init ( m_iMaxUniqQpos+1 ); // will not be tracking dupes

		hQwords.IterateStart();
		while ( hQwords.IterateNext() )
		{
			// tricky bit
			// for query_word_count, we only want to count keywords that are not (!) excluded by the query
			// that is, in (aa NOT bb) case, we want a value of 1, not 2
			if ( hQwords.IterateGet().m_bExcluded )
				continue;

			const int iPos = hQwords.IterateGet().m_iQueryPos;
			m_tKeywordMask.BitSet ( iPos ); // just to assert at early stage!
			m_dIDF [ iPos ] = hQwords.IterateGet().m_fIDF;
			m_iQueryWordCount++;
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
		for ( int iWord=1; iWord<=m_iQueryWordCount; iWord++ )
		{
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
		m_iExpDelta = -1;
		for ( int i=0; i<m_iFields; i++ )
		{
			m_uMatchMask[i] = 0;
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
		}
		ARRAY_FOREACH ( i, m_dTF )
			m_dTF[i] = 0;
		ARRAY_FOREACH ( i, m_dFieldTF ) // OPTIMIZE? make conditional?
			m_dFieldTF[i] = 0;
		m_uMatchedFields = 0;
		m_uExactHit = 0;
		m_uDocWordCount = 0;
		m_dWindow.Resize ( 0 );
		m_fDocBM25A = 0;
	}

	void FlushMatches ()
	{
		m_tFactorPool.Flush ();
	}

private:
	virtual bool	ExtraDataImpl ( ExtraData_e eType, void ** ppResult );
	BYTE *			PackFactors ( int * pSize = NULL );
};


template <>
bool RankerState_Expr_fn<false>::ExtraDataImpl ( ExtraData_e eType, void ** ppResult )
{
	switch ( eType )
	{
	case EXTRA_SET_MVAPOOL:
		m_pExpr->SetMVAPool ( (DWORD*)ppResult );
		return true;
	case EXTRA_SET_STRINGPOOL:
		m_pExpr->SetStringPool ( (BYTE*)ppResult );
		return true;
	default:
		return false;
	}
}

template <>
bool RankerState_Expr_fn<true>::ExtraDataImpl ( ExtraData_e eType, void ** ppResult )
{
	switch ( eType )
	{
	case EXTRA_SET_MVAPOOL:
		m_pExpr->SetMVAPool ( (DWORD*)ppResult );
		return true;
	case EXTRA_SET_STRINGPOOL:
		m_pExpr->SetStringPool ( (BYTE*)ppResult );
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
	case EXTRA_GET_DATA_RANKFACTORS:
		*ppResult = m_tFactorPool.GetHashPtr();
		return true;
	default:
		return false;
	}
}


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
	XRANK_MAX_WINDOW_HITS,

	// document level factors
	XRANK_BM25,
	XRANK_MAX_LCS,
	XRANK_FIELD_MASK,
	XRANK_QUERY_WORD_COUNT,
	XRANK_DOC_WORD_COUNT,
	XRANK_BM25A,
	XRANK_BM25F,

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

template < bool NEED_PACKEDFACTORS >
struct Expr_BM25F_T : public ISphExpr
{
	RankerState_Expr_fn<NEED_PACKEDFACTORS> * m_pState;
	float					m_fK1;
	float					m_fB;
	float					m_fWeightedAvgDocLen;
	CSphVector<int>			m_dWeights;		///< per field weights

	explicit Expr_BM25F_T ( RankerState_Expr_fn<NEED_PACKEDFACTORS> * pState, float k1, float b, ISphExpr * pFieldWeights )
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
			Expr_ConstHash_c * pConstHash = dynamic_cast<Expr_ConstHash_c*> ( pFieldWeights );
			assert ( pConstHash );

			ARRAY_FOREACH ( i, pConstHash->m_dValues )
			{
				CSphString & sField = pConstHash->m_dValues[i].m_sName;
				sField.ToLower();

				// FIXME? report errors if field was not found?
				ARRAY_FOREACH ( j, pState->m_pSchema->m_dFields )
				{
					if ( pState->m_pSchema->m_dFields[j].m_sName==sField )
					{
						m_dWeights[j] = pConstHash->m_dValues[i].m_iValue;
						break;
					}
				}
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
			// compute weighted TF
			float tf = 0.0f;
			for ( int i=0; i<m_pState->m_iFields; i++ )
				tf += m_pState->m_dFieldTF [ iWord + i*(1+m_pState->m_iMaxQpos) ] * m_dWeights[i];
			float idf = m_pState->m_dIDF[iWord]; // FIXME? zeroed out for dupes!
			fRes += tf / (tf + m_fK1*(1 - m_fB + m_fB*dl/m_fWeightedAvgDocLen)) * idf;
		}
		return fRes + 0.5f; // map to [0..1] range
	}
};


/// function that sums sub-expressions over matched fields
template < bool NEED_PACKEDFACTORS >
struct Expr_Sum_T : public ISphExpr
{
	RankerState_Expr_fn<NEED_PACKEDFACTORS> * m_pState;
	ISphExpr *				m_pArg;

	Expr_Sum_T ( RankerState_Expr_fn<NEED_PACKEDFACTORS> * pState, ISphExpr * pArg )
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
template < bool NEED_PACKEDFACTORS >
class ExprRankerHook_T : public ISphExprHook
{
public:
	RankerState_Expr_fn<NEED_PACKEDFACTORS> * m_pState;
	const char *			m_sCheckError;
	bool					m_bCheckInFieldAggr;

public:
	explicit ExprRankerHook_T ( RankerState_Expr_fn<NEED_PACKEDFACTORS> * pState )
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
		return -1;
	}

	int IsKnownFunc ( const char * sFunc )
	{
		if ( !strcasecmp ( sFunc, "sum" ) )
			return XRANK_SUM;
		if ( !strcasecmp ( sFunc, "max_window_hits" ) )
			return XRANK_MAX_WINDOW_HITS;
		if ( !strcasecmp ( sFunc, "bm25a" ) )
			return XRANK_BM25A;
		if ( !strcasecmp ( sFunc, "bm25f" ) )
			return XRANK_BM25F;
		return -1;
	}

	ISphExpr * CreateNode ( int iID, ISphExpr * pLeft, ESphEvalStage * )
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
			case XRANK_EXACT_HIT:			return new Expr_FieldFactor_c<bool> ( pCF, &m_pState->m_uExactHit );
			case XRANK_MAX_WINDOW_HITS:
				{
					CSphMatch tDummy;
					m_pState->m_iWindowSize = pLeft->IntEval ( tDummy ); // must be constant; checked in GetReturnType()
					return new Expr_FieldFactor_c<int> ( pCF, m_pState->m_iMaxWindowHits );
				}

			case XRANK_BM25:				return new Expr_IntPtr_c ( &m_pState->m_uDocBM25 );
			case XRANK_MAX_LCS:				return new Expr_GetIntConst_c ( m_pState->m_iMaxLCS );
			case XRANK_FIELD_MASK:			return new Expr_IntPtr_c ( &m_pState->m_uMatchedFields );
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
					ISphExpr * pRes = new Expr_BM25F_T<NEED_PACKEDFACTORS> ( m_pState, fK1, fB, pLeft->GetArg(2) );
					SafeDelete ( pLeft );
					return pRes;
				}

			case XRANK_SUM:					return new Expr_Sum_T<NEED_PACKEDFACTORS> ( m_pState, pLeft );
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
			case XRANK_MAX_WINDOW_HITS:
			case XRANK_BM25: // doc-level
			case XRANK_MAX_LCS:
			case XRANK_FIELD_MASK:
			case XRANK_QUERY_WORD_COUNT:
			case XRANK_DOC_WORD_COUNT:
				return SPH_ATTR_INTEGER;
			case XRANK_TF_IDF:
			case XRANK_MIN_IDF:
			case XRANK_MAX_IDF:
			case XRANK_SUM_IDF:
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
					if ( dArgs[i]!=SPH_ATTR_CONSTHASH )
					{
						sError.SetSprintf ( "argument %d to %s() must be a hash of constants", i, sFuncname );
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
template < bool NEED_PACKEDFACTORS >
RankerState_Expr_fn <NEED_PACKEDFACTORS>::RankerState_Expr_fn ()
	: m_pWeights ( NULL )
	, m_sExpr ( NULL )
	, m_pExpr ( NULL )
	, m_iMaxMatches ( 0 )
	, m_iMaxLCS ( 0 )
	, m_iQueryWordCount ( 0 )
{}


/// dtor
template < bool NEED_PACKEDFACTORS >
RankerState_Expr_fn <NEED_PACKEDFACTORS>::~RankerState_Expr_fn ()
{
	SafeRelease ( m_pExpr );
}


/// initialize ranker state
template < bool NEED_PACKEDFACTORS >
bool RankerState_Expr_fn<NEED_PACKEDFACTORS>::Init ( int iFields, const int * pWeights, ExtRanker_c * pRanker, CSphString & sError )
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
	memset ( m_dSumIDF, 0, sizeof(m_dSumIDF) );
	memset ( m_iMinHitPos, 0, sizeof(m_iMinHitPos) );
	memset ( m_iMinBestSpanPos, 0, sizeof(m_iMinBestSpanPos) );
	memset ( m_iMaxWindowHits, 0, sizeof(m_iMaxWindowHits) );
	m_iMaxQpos = pRanker->m_iMaxQpos; // already copied in SetQwords, but anyway
	m_iMaxUniqQpos = pRanker->m_iMaxUniqQpos;
	m_uExactHit = 0;
	m_uDocWordCount = 0;
	m_iWindowSize = 1;

	for ( int i=0; i < SPH_MAX_FIELDS; i++ )
	{
		m_dMinIDF[i] = FLT_MAX;
		m_dMaxIDF[i] = -FLT_MAX;
	}

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
	ExprRankerHook_T<NEED_PACKEDFACTORS> tHook ( this );
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
template < bool NEED_PACKEDFACTORS >
void RankerState_Expr_fn<NEED_PACKEDFACTORS>::Update ( const ExtHit_t * pHlist )
{
	const DWORD uField = HITMAN::GetField ( pHlist->m_uHitpos );
	const int iPos = HITMAN::GetPos ( pHlist->m_uHitpos );

	// update LCS
	int iDelta = HITMAN::GetLCS ( pHlist->m_uHitpos ) - pHlist->m_uQuerypos;
	if ( iDelta==m_iExpDelta )
	{
		m_uCurLCS = m_uCurLCS + BYTE(pHlist->m_uWeight);
		if ( HITMAN::IsEnd ( pHlist->m_uHitpos ) && (int)pHlist->m_uQuerypos==m_iMaxUniqQpos && iPos==m_iMaxUniqQpos )
			m_uExactHit |= ( 1UL << uField );
	} else
	{
		m_uCurLCS = BYTE(pHlist->m_uWeight);
		if ( iPos==1 && HITMAN::IsEnd ( pHlist->m_uHitpos ) && m_iMaxUniqQpos==1 )
			m_uExactHit |= ( 1UL << uField );
	}
	if ( m_uCurLCS>m_uLCS[uField] )
	{
		m_uLCS[uField] = m_uCurLCS;
		m_iMinBestSpanPos[uField] = iPos - m_uCurLCS + 1;
	}
	m_iExpDelta = iDelta + pHlist->m_uSpanlen - 1;

	// update other stuff
	m_uMatchMask[uField] |= ( 1<<(pHlist->m_uQuerypos-1) );
	m_uMatchedFields |= ( 1UL<<uField );

	// keywords can be duplicated in the query, so we need this extra check
	if ( pHlist->m_uQuerypos<=m_iMaxUniqQpos && m_tKeywordMask.BitGet ( pHlist->m_uQuerypos ) )
	{
		float fIDF = m_dIDF [ pHlist->m_uQuerypos ];
		DWORD uHitPosMask = 1<<pHlist->m_uQuerypos;

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
	}

	// avoid duplicate check for BM25A, BM25F though
	// (that sort of automatically accounts for qtf factor)
	m_dTF [ pHlist->m_uQuerypos ]++;
	m_dFieldTF [ pHlist->m_uQuerypos + uField*(1+m_iMaxQpos) ]++;

	if ( !m_iMinHitPos[uField] )
		m_iMinHitPos[uField] = iPos;

	// update hit window, max_window_hits factor
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
}


template < bool NEED_PACKEDFACTORS >
BYTE * RankerState_Expr_fn<NEED_PACKEDFACTORS>::PackFactors ( int * pSize )
{
	DWORD * pPackStart = NULL;

	if ( pSize )
	{
		const DWORD MAX_PACKED_SIZE=2048;
		pPackStart = new DWORD [MAX_PACKED_SIZE];
	} else
	{
		pPackStart = (DWORD *)m_tFactorPool.Alloc();
		assert ( pPackStart );
	}

	DWORD * pPack = pPackStart;

	// document level factors
	*pPack++ = m_uDocBM25;
	*pPack++ = (DWORD)m_fDocBM25A;
	*pPack++ = m_uMatchedFields;
	*pPack++ = m_uDocWordCount;

	// field level factors
	*pPack++ = (DWORD)m_iFields;

	for ( int i=0; i<m_iFields; i++ )
	{
		DWORD uHit = m_uHitCount[i];
		*pPack++ = uHit;

		if ( uHit || pSize )
		{
			*pPack++ = (DWORD)i;
			*pPack++ = m_uLCS[i];
			*pPack++ = m_uWordCount[i];
			*pPack++ = (DWORD)m_dTFIDF[i];
			*pPack++ = (DWORD)m_dMinIDF[i];
			*pPack++ = (DWORD)m_dMaxIDF[i];
			*pPack++ = (DWORD)m_dSumIDF[i];
			*pPack++ = (DWORD)m_iMinHitPos[i];
			*pPack++ = (DWORD)m_iMinBestSpanPos[i];
			*pPack++ = ( m_uExactHit>>i ) & 1;
			*pPack++ = (DWORD)m_iMaxWindowHits[i];
		}
	}

	// word level factors
	*pPack++ = (DWORD)m_iMaxUniqQpos;
	for ( int i=1; i<=m_iMaxUniqQpos; i++ )
	{
		DWORD uKeywordMask = m_tKeywordMask.BitGet(i);
		*pPack++ = uKeywordMask;
		if ( uKeywordMask || pSize )
		{
			*pPack++ = (DWORD)i;
			*pPack++ = (DWORD)m_dTF[i];
			*pPack++ = (DWORD)m_dIDF[i];
		}
	}

	*pPack = (pPack-pPackStart)*sizeof(DWORD);
	pPack++;

	if ( pSize )
	{
		*pSize = (pPack-pPackStart)*sizeof(DWORD);
		delete [] pPackStart;
		return NULL;
	}

	assert ( (pPack-pPackStart)*sizeof(DWORD)<=(DWORD)m_tFactorPool.GetElementSize() );

	return (BYTE*)pPackStart;
}


#if USE_WINDOWS
#pragma warning(disable:4127) // conditional expr is const for MSVC
#endif

/// finish document processing, compute weight from factors
template < bool NEED_PACKEDFACTORS >
DWORD RankerState_Expr_fn<NEED_PACKEDFACTORS>::Finalize ( const CSphMatch & tMatch )
{
	// finishing touches
	FinalizeDocFactors ( tMatch );

	if ( NEED_PACKEDFACTORS )
	{
		// pack factors
		if ( !m_tFactorPool.IsInitialized() )
		{
			int iPoolElementSize = 0;
			PackFactors ( &iPoolElementSize );
			m_tFactorPool.Prealloc ( iPoolElementSize, m_iMaxMatches+ExtNode_i::MAX_DOCS );
		}

		m_tFactorPool.AddToHash ( tMatch.m_iDocID, PackFactors() );
	}

	// compute expression
	DWORD uRes = ( m_eExprType==SPH_ATTR_INTEGER )
		? m_pExpr->IntEval ( tMatch )
		: (DWORD)m_pExpr->Eval ( tMatch );

	// cleanup
	ResetDocFactors();

	// done
	return uRes;
}


/// expression ranker
template < bool NEED_PACKEDFACTORS >
class ExtRanker_Expr_T : public ExtRanker_T< RankerState_Expr_fn<NEED_PACKEDFACTORS> >
{
public:
	ExtRanker_Expr_T ( const XQQuery_t & tXQ, const ISphQwordSetup & tSetup, const char * sExpr, const CSphSchema & tSchema )
		: ExtRanker_T< RankerState_Expr_fn<NEED_PACKEDFACTORS> > ( tXQ, tSetup )
	{
		// tricky bit, we stash the pointer to expr here, but it will be parsed
		// somewhat later during InitState() call, when IDFs etc are computed
		this->m_tState.m_sExpr = sExpr;
		this->m_tState.m_pSchema = &tSchema;
	}

	void SetQwordsIDF ( const ExtQwordsHash_t & hQwords )
	{
		ExtRanker_T< RankerState_Expr_fn<NEED_PACKEDFACTORS> >::SetQwordsIDF ( hQwords );
		this->m_tState.m_iMaxQpos = this->m_iMaxQpos;
		this->m_tState.m_iMaxUniqQpos = this->m_iMaxUniqQpos;
		this->m_tState.SetQwords ( hQwords );
	}

	virtual int GetMatches ()
	{
		if ( NEED_PACKEDFACTORS )
			this->m_tState.FlushMatches ();

		return ExtRanker_T<RankerState_Expr_fn <NEED_PACKEDFACTORS> >::GetMatches ();
	}
};


#if USE_WINDOWS
#pragma warning(default:4127) // conditional expr is const for MSVC
#endif


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
			m_uDocBM25, m_fDocBM25A, m_uMatchedFields, m_uDocWordCount );

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
				m_iMinHitPos[i], m_iMinBestSpanPos[i], ( m_uExactHit>>i ) & 1, m_iMaxWindowHits[i] );

			int iValLen = strlen ( dVal.Begin() );
			int iTotalLen = iValLen+strlen(sTmp);
			if ( dVal.GetLength() < iTotalLen+1 )
				dVal.Resize ( iTotalLen+1 );

			strcpy ( &(dVal[iValLen]), sTmp );
		}

		// build word level factors
		for ( int i=1; i<=m_iMaxUniqQpos; i++ )
			if ( m_tKeywordMask.BitGet(i) )
		{
			snprintf ( sTmp, MAX_STR_LEN, ", word%d=(tf=%d, idf=%f)", i, m_dTF[i], m_dIDF[i] );
			int iValLen = strlen ( dVal.Begin() );
			int iTotalLen = iValLen+strlen(sTmp);
			if ( dVal.GetLength() < iTotalLen+1 )
				dVal.Resize ( iTotalLen+1 );

			strcpy ( &(dVal[iValLen]), sTmp );
		}

		// export factors
		m_hFactors.Add ( dVal.Begin(), tMatch.m_iDocID );

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
		m_tState.m_iMaxUniqQpos = m_iMaxUniqQpos;
		m_tState.SetQwords ( hQwords );
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
	CheckExtendedQuery ( tXQ.m_pRoot, pResult, pIndex->GetSettings(), pIndex->IsStarEnabled() );

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
			if ( pQuery->m_bPackedFactors )
				pRanker = new ExtRanker_Expr_T <true> ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() );
			else
				pRanker = new ExtRanker_Expr_T <false> ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() );
			break;

		case SPH_RANK_EXPORT:			pRanker = new ExtRanker_Export_c ( tXQ, tTermSetup, pQuery->m_sRankerExpr.cstr(), pIndex->GetMatchSchema() ); break;
		default:
			pResult->m_sWarning.SetSprintf ( "unknown ranking mode %d; using default", (int)pQuery->m_eRanker );
			if ( bGotDupes )
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true,true> > ( tXQ, tTermSetup );
			else
				pRanker = new ExtRanker_T < RankerState_Proximity_fn<true,false> > ( tXQ, tTermSetup );
			break;
	}
	assert ( pRanker );
	pRanker->m_uPayloadMask = uPayloadMask;

	// setup IDFs
	ExtQwordsHash_t hQwords;
	int iMaxQpos = pRanker->GetQwords ( hQwords );
	int iMaxUniqQpos = -1;

	const int iQwords = hQwords.GetLength ();
	const CSphSourceStats & tSourceStats = pIndex->GetStats();

	CSphVector<const ExtQword_t *> dWords;
	dWords.Reserve ( hQwords.GetLength() );

	hQwords.IterateStart ();
	while ( hQwords.IterateNext() )
	{
		ExtQword_t & tWord = hQwords.IterateGet ();
		if ( !tWord.m_bExcluded )
			iMaxUniqQpos = Max ( iMaxUniqQpos, tWord.m_iQueryPos );

		// build IDF
		float fIDF = 0.0f;
		if ( tWord.m_iDocs )
		{
			// (word_docs > total_docs) case *is* occasionally possible
			// because of dupes, or delayed purging in RT, etc
			const int iTotalClamped = Max ( tSourceStats.m_iTotalDocuments, tWord.m_iDocs );

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
				fIDF = logf ( float ( iTotalClamped-tWord.m_iDocs+1 ) / float ( tWord.m_iDocs ) )
					/ ( 2*iQwords*fLogTotal );
			} else
			{
				// plain variant, idf=log(N/n), as per Sparck-Jones
				//
				// idf \in [0, log(N)]
				// weight \in [0, NumWords*log(N)]
				// we prescale idfs and get weight in [0, 0.5] range
				// then add 0.5 as our final step
				float fLogTotal = logf ( float ( 1+iTotalClamped ) );
				fIDF = logf ( float ( iTotalClamped ) / float ( tWord.m_iDocs ) )
					/ ( 2*iQwords*fLogTotal );
			}
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

	pRanker->m_iMaxQpos = iMaxQpos;
	pRanker->m_iMaxUniqQpos = iMaxUniqQpos;
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
	if ( !m_pRoot )
		return;

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

	virtual void HintDocid ( SphDocID_t ) {}

	virtual const ExtDoc_t * GetDocsChunk ( SphDocID_t * pMaxID );

	virtual const ExtHit_t * GetHitsChunk ( const ExtDoc_t * pMatched, SphDocID_t uMaxID );

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
		{
			SphDocID_t uLastDocid = m_Docs.Last().m_uDocid;
			while (	( pHits = pChild->GetHitsChunk ( pChunkHits, uLastDocid ) )!=NULL )
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
