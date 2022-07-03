//
// Copyright (c) 2018-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _secondaryindex_
#define _secondaryindex_

#include "sphinxstd.h"
#include "sphinx.h"
#include "columnarmisc.h"

#include <math.h>
#include <vector>


using RowIdBlock_t = VecTraits_T<RowID_t>;

class RowidIterator_i
{
public:
	virtual			~RowidIterator_i(){}

	virtual bool	HintRowID ( RowID_t tRowID ) = 0;
	virtual bool	GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) = 0;
	virtual int64_t	GetNumProcessed() const = 0;
	virtual void	SetCutoff ( int iCutoff ) = 0;
	virtual bool	WasCutoffHit() const = 0;
	virtual void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const = 0;
};

struct SecondaryIndexInfo_t
{
	int		m_iFilterId {-1};
	int64_t	m_iRsetEstimate {0};
};

struct RowIdBoundaries_t;

class SelectSI_c
{
public:
	SelectSI_c() = default;
	virtual ~SelectSI_c() {};

	virtual bool IsEnabled ( const CSphFilterSettings & tFilter ) const;
};

RowidIterator_i * CreateFilteredIterator ( const CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, const BYTE * pDocidLookup, uint32_t uTotalDocs );

RowidIterator_i * CreateIteratorIntersect ( CSphVector<RowidIterator_i*> & dIterators, const RowIdBoundaries_t * pBoundaries );
RowidIterator_i * CreateIteratorWrapper ( common::BlockIterator_i * pIterator, const RowIdBoundaries_t * pBoundaries );
RowidIterator_i * CreateIteratorIntersect ( std::vector<common::BlockIterator_i *> & dIterators, const RowIdBoundaries_t * pBoundaries );

float	GetEnabledSecondaryIndexes ( CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, const CSphVector<IndexHint_t> & dHints, const HistogramContainer_c & tHistograms, const SelectSI_c * pCheck );
bool	ReturnIteratorResult ( RowID_t * pRowID, RowID_t * pRowIdStart, RowIdBlock_t & dRowIdBlock );

CSphVector<SecondaryIndexInfo_t> SelectIterators ( const CSphVector<CSphFilterSettings> & dFilters, bool bFilterTreeEmpty, const CSphVector<IndexHint_t> & dHints, const HistogramContainer_c * pHistograms, const SelectSI_c * pCheck );

namespace SI
{
	class Index_i;
	class Builder_i;
}

RowidIterator_i * CreateSecondaryIndexIterator ( const SI::Index_i * pCidx, CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const CSphVector<CSphFilterSettings> & dFilters, ESphCollation eCollation, const ISphSchema & tSchema, RowID_t uRowsCount, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, int iCutoff );

SelectSI_c * GetSelectIteratorsCond ( const SI::Index_i * pSidx, const ISphSchema & tSchema, ESphCollation eCollation );

std::unique_ptr<SI::Builder_i>	CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, CSphBitvec & tSIAttrs, const char * sFile, CSphString & sError );
std::unique_ptr<SI::Builder_i>	CreateIndexBuilder ( int iMemoryLimit, const CSphSchema & tSchema, const char * sFile, CSphVector<PlainOrColumnar_t> & dAttrs, CSphString & sError );

//////////////////////////////////////////////////////////////////////////

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
	explicit		DocidLookupWriter_c ( DWORD nDocs );
					~DocidLookupWriter_c();

	bool			Open ( const CSphString & sFilename, CSphString & sError );
	void			AddPair ( const DocidRowidPair_t & tPair );
	bool			Finalize ( CSphString & sError );
	CSphWriter &	GetWriter();

private:
	static const int DOCS_PER_LOOKUP_CHECKPOINT = 64;

	int				m_iProcessed {0};
	int				m_iCheckpoint {0};
	DWORD			m_nDocs {0};
	CSphWriter *	m_pWriter {nullptr};
	SphOffset_t		m_tCheckpointStart {0};
	DocID_t			m_tLastDocID {0};
	CSphFixedVector<DocidLookupCheckpoint_t> m_dCheckpoints {0};
};


class LookupReader_c
{
public:
			LookupReader_c() = default;
			LookupReader_c ( const BYTE * pData );


	void	SetData ( const BYTE * pData );

	inline RowID_t Find ( DocID_t tDocID ) const
	{
		if ( !m_pCheckpoints || tDocID<m_pCheckpoints->m_tBaseDocID || tDocID>m_tMaxDocID )
			return INVALID_ROWID;

		const DocidLookupCheckpoint_t * pFound = FindCheckpoint ( tDocID, m_pCheckpoints );
		if ( !pFound )
			return INVALID_ROWID;

		const BYTE * pCur = m_pData + pFound->m_tOffset;
		DocID_t tCurDocID = pFound->m_tBaseDocID;

		// 1st entry doesnt have docid
		RowID_t tRowID = sphUnalignedRead ( *(RowID_t*)const_cast<BYTE*>(pCur) );
		pCur += sizeof(RowID_t);

		if ( tCurDocID==tDocID )
			return tRowID;

		int iDocsInCheckpoint = GetNumDocsInCheckpoint(pFound);
		for ( int i = 1; i < iDocsInCheckpoint; i++ )
		{
			DocID_t tDeltaDocID = sphUnzipOffset(pCur);
			assert ( tDeltaDocID>=0 );
			tRowID = sphUnalignedRead ( *(RowID_t*)const_cast<BYTE*>(pCur) );
			pCur += sizeof(RowID_t);

			tCurDocID += tDeltaDocID;
			if ( tCurDocID==tDocID )
				return tRowID;
			else if ( tCurDocID>tDocID )
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
		if ( tDocID<pStart->m_tBaseDocID || tDocID>m_tMaxDocID )
			return nullptr;

		const DocidLookupCheckpoint_t * pEnd = m_pCheckpoints+m_nCheckpoints-1;
		const DocidLookupCheckpoint_t * pFound = sphBinarySearchFirst ( pStart, pEnd, bind ( &DocidLookupCheckpoint_t::m_tBaseDocID ), tDocID );
		assert ( pFound );

		if ( pFound->m_tBaseDocID>tDocID )
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

		DocID_t tDelta = sphUnzipOffset ( m_pCur );
		assert ( tDelta>=0 );

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
		if ( tDocID < ( m_pCurCheckpoint+1 )->m_tBaseDocID )
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


bool WriteDocidLookup ( const CSphString & sFilename, const VecTraits_T<DocidRowidPair_t> & dLookup, CSphString & sError );

#endif // _secondaryindex_
