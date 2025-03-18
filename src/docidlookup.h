//
// Copyright (c) 2018-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _docidlookup_
#define _docidlookup_

#include "secondaryindex.h"

class CSphWriter;

struct DocidRowidPair_t
{
	DocID_t m_tDocID;
	RowID_t	m_tRowID;
};


struct DocidLookupCheckpoint_t
{
	DocID_t		m_tBaseDocID {0};
	SphOffset_t	m_tOffset {0};
};


class DocidLookupWriter_c
{
public:
					DocidLookupWriter_c ( CSphWriter& tWriter, DWORD nDocs );

	void			Start();
	void			AddPair ( const DocidRowidPair_t & tPair );
	bool			Finalize ( CSphString & sError );

private:
	static const int DOCS_PER_LOOKUP_CHECKPOINT = 64;

	CSphWriter&		m_tWriter;
	DWORD			m_nDocs;

	int				m_iProcessed {0};
	int				m_iCheckpoint {0};
	SphOffset_t		m_tCheckpointStart {0};
	DocID_t			m_tLastDocID {0};
	CSphFixedVector<DocidLookupCheckpoint_t> m_dCheckpoints {0};
};


class LookupReader_c
{
public:
			LookupReader_c() = default;
			explicit LookupReader_c ( const BYTE * pData );


	void	SetData ( const BYTE * pData );

	inline RowID_t Find ( DocID_t tDocID ) const
	{
		if ( !m_pCheckpoints || (uint64_t)tDocID<(uint64_t)m_pCheckpoints->m_tBaseDocID || (uint64_t)tDocID>(uint64_t)m_tMaxDocID )
			return INVALID_ROWID;

		const DocidLookupCheckpoint_t * pFound = FindCheckpoint ( tDocID, m_pCheckpoints );
		if ( !pFound )
			return INVALID_ROWID;

		const BYTE * pCur = m_pData + pFound->m_tOffset;
		DocID_t tCurDocID = pFound->m_tBaseDocID;

		// 1st entry doesnt have docid
		RowID_t tRowID = sphUnalignedRead ( *(const RowID_t*)pCur );
		pCur += sizeof(RowID_t);

		if ( tCurDocID==tDocID )
			return tRowID;

		int iDocsInCheckpoint = GetNumDocsInCheckpoint(pFound);
		for ( int i = 1; i < iDocsInCheckpoint; i++ )
		{
			uint64_t tDeltaDocID = UnzipOffsetBE(pCur);
			tRowID = sphUnalignedRead ( *(const RowID_t*)pCur );
			pCur += sizeof(RowID_t);

			tCurDocID += tDeltaDocID;
			if ( tCurDocID==tDocID )
				return tRowID;
			else if ( (uint64_t)tCurDocID>(uint64_t)tDocID )
				return INVALID_ROWID;
		}

		return INVALID_ROWID;
	}

protected:
	DWORD							m_nDocs {0};
	DWORD							m_nDocsPerCheckpoint {0};
	int								m_nCheckpoints {0};
	DocID_t							m_tMaxDocID {0};
	const BYTE *					m_pData {nullptr};
	const DocidLookupCheckpoint_t *	m_pCheckpoints {nullptr};


	inline const DocidLookupCheckpoint_t * FindCheckpoint ( DocID_t tDocID, const DocidLookupCheckpoint_t * pStart ) const
	{
		if ( (uint64_t)tDocID<(uint64_t)pStart->m_tBaseDocID || (uint64_t)tDocID>(uint64_t)m_tMaxDocID )
			return nullptr;

		const DocidLookupCheckpoint_t * pEnd = m_pCheckpoints+m_nCheckpoints-1;
		const DocidLookupCheckpoint_t * pFound = sphBinarySearchFirst ( pStart, pEnd, []( const DocidLookupCheckpoint_t & i ){ return (uint64_t)i.m_tBaseDocID; }, (uint64_t)tDocID );
		assert ( pFound );

		if ( (uint64_t)pFound->m_tBaseDocID>(uint64_t)tDocID )
		{
			if ( pFound==m_pCheckpoints )
				return nullptr;

			return pFound-1;
		}

		return pFound;
	}

	inline bool IsLastCheckpoint ( const DocidLookupCheckpoint_t * pCheckpoint ) const
	{
		return pCheckpoint==m_pCheckpoints+m_nCheckpoints-1;
	}

	inline int GetNumDocsInCheckpoint ( const DocidLookupCheckpoint_t * pCheckpoint ) const
	{
		if ( !IsLastCheckpoint(pCheckpoint) )
			return m_nDocsPerCheckpoint;

		int iLeftover = m_nDocs % m_nDocsPerCheckpoint;
		return iLeftover ? iLeftover : m_nDocsPerCheckpoint;
	}
};


class LookupReaderIterator_c : private LookupReader_c
{
public:
			LookupReaderIterator_c ( const BYTE * pData );

	void	SetData ( const BYTE * pData );

	inline bool Read ( DocID_t & tDocID, RowID_t & tRowID )
	{
		if ( IsEnd() )
			return false;

		// step to next checkpoint
		if ( m_iProcessedDocs==m_iCheckpointDocs )
		{
			if ( IsLastCheckpoint() )
			{
				m_pCurCheckpoint = nullptr;
				return false;
			}

			SetCheckpoint ( m_pCurCheckpoint+1 );
		}

		// no docid stored for first entry
		if ( !m_iProcessedDocs )
		{
			tDocID = m_tCurDocID;
			tRowID = sphUnalignedRead ( *(RowID_t*)const_cast<BYTE*>(m_pCur) );
			m_pCur += sizeof(RowID_t);

			m_iProcessedDocs++;
			return true;
		}

		uint64_t tDelta = UnzipOffsetBE ( m_pCur );

		tRowID = sphUnalignedRead ( *(RowID_t*)const_cast<BYTE*>(m_pCur) );
		m_pCur += sizeof(RowID_t);

		m_tCurDocID += tDelta;
		tDocID = m_tCurDocID;

		m_iProcessedDocs++;
		return true;
	}

	inline bool ReadDocID ( DocID_t & tDocID )
	{
		RowID_t tRowID;
		return Read ( tDocID, tRowID );
	}

	inline void HintDocID ( DocID_t tDocID )
	{
		// inside last block? do nothing
		if ( IsEnd() || IsLastCheckpoint() )
			return;

		// tDocID is inside current block (check with next min docid)? do nothing
		if ( (uint64_t)tDocID < uint64_t ( ( m_pCurCheckpoint+1 )->m_tBaseDocID ) )
			return;

		// perform binary search starting with next checkpoint
		SetCheckpoint ( FindCheckpoint ( tDocID, m_pCurCheckpoint+1 ) );
	}

private:
	const DocidLookupCheckpoint_t *	m_pCurCheckpoint {nullptr};
	const BYTE *					m_pCur {nullptr};
	DocID_t							m_tCurDocID {0};
	int								m_iProcessedDocs;
	int								m_iCheckpointDocs;


	inline bool IsLastCheckpoint() const
	{
		return LookupReader_c::IsLastCheckpoint(m_pCurCheckpoint);
	}

	inline bool IsEnd() const
	{
		return !m_pCurCheckpoint;
	}

	inline void SetCheckpoint ( const DocidLookupCheckpoint_t * pCheckpoint )
	{
		m_pCurCheckpoint = pCheckpoint;
		if ( !m_pCurCheckpoint )
			return;

		m_iProcessedDocs = 0;
		m_iCheckpointDocs = GetNumDocsInCheckpoint(m_pCurCheckpoint);
		m_pCur = m_pData + m_pCurCheckpoint->m_tOffset;
		m_tCurDocID = m_pCurCheckpoint->m_tBaseDocID;
	}
};


RowIteratorsWithEstimates_t CreateLookupIterator ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, const BYTE * pDocidLookup, uint32_t uTotalDocs );
bool	WriteDocidLookup ( const CSphString & sFilename, const VecTraits_T<DocidRowidPair_t> & dLookup, CSphString & sError );

struct CmpDocidLookup_fn
{
	static inline bool IsLess ( const DocidRowidPair_t & a, const DocidRowidPair_t & b )
	{
		if ( a.m_tDocID==b.m_tDocID )
			return a.m_tRowID < b.m_tRowID;

		return (uint64_t)a.m_tDocID < (uint64_t)b.m_tDocID;
	}
};

#endif
