//
// Copyright (c) 2018, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _secondaryindex_
#define _secondaryindex_

#include "sphinx.h"
#include "sphinxstd.h"
#include <math.h>


class RowidIterator_i
{
public:
	virtual			~RowidIterator_i(){}

	virtual RowID_t	GetNextRowID() = 0;
	virtual int64_t	GetNumProcessed() const = 0;
};


class CSphReader;
class CSphWriter;

enum HistogramType_e
{
	HISTOGRAM_NONE,
	HISTOGRAM_UINT32,
	HISTOGRAM_INT64,
	HISTOGRAM_FLOAT	
};


class Histogram_i
{
public:
	virtual			~Histogram_i() {}

	virtual void	Setup ( SphAttr_t tMin, SphAttr_t tMax ) = 0;
	virtual void	Insert ( SphAttr_t tAttrVal ) = 0;
	virtual void	Delete ( SphAttr_t tAttrVal ) = 0;
	virtual bool	EstimateRsetSize ( const CSphFilterSettings & tFilter, int64_t & iEstimate ) const = 0;
	virtual DWORD	GetNumValues() const = 0;
	virtual bool	IsOutdated() const = 0;

	virtual HistogramType_e		GetType() const = 0;
	virtual const CSphString &	GetAttrName() const = 0;

	virtual bool	Save ( CSphWriter & tWriter ) const = 0;
	virtual bool	Load ( CSphReader & tReader, CSphString & sError ) = 0;
};


class HistogramContainer_c
{
public:
					~HistogramContainer_c();

	bool			Save ( const CSphString & sFile, CSphString & sError );
	bool			Load ( const CSphString & sFile, CSphString & sError );
	bool			Add ( Histogram_i * pHistogram );
	void			Remove ( const CSphString & sAttr );
	Histogram_i *	Get ( const CSphString & sAttr ) const;
	DWORD			GetNumValues() const;

private:
	SmallStringHash_T<Histogram_i*>	m_dHistogramHash;

	void			Reset();
};


Histogram_i *	CreateHistogram ( const CSphString & sAttr, ESphAttr eAttrType );


struct SecondaryIndexInfo_t
{
	int		m_iFilterId {-1};
};

RowidIterator_i * CreateFilteredIterator ( const CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, const CSphVector<IndexHint_t> & dHints, const HistogramContainer_c & tHistograms, const BYTE * pDocidLookup );

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
		RowID_t tRowID = sphUnalignedRead ( *(RowID_t*)pCur );
		pCur += sizeof(RowID_t);

		if ( tCurDocID==tDocID )
			return tRowID;

		int iDocsInCheckpoint = IsLastCheckpoint(pFound) ? m_nDocs % m_nDocsPerCheckpoint : m_nDocsPerCheckpoint;
		for ( int i = 1; i < iDocsInCheckpoint; i++ )
		{
			DocID_t tDeltaDocID = sphUnzipOffset(pCur);
			assert ( tDeltaDocID>=0 );
			tRowID = sphUnalignedRead ( *(RowID_t*)pCur );
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
			tRowID = sphUnalignedRead ( *(RowID_t*)m_pCur );
			m_pCur += sizeof(RowID_t);

			m_iProcessedDocs++;
			return true;
		}

		DocID_t tDelta = sphUnzipOffset ( m_pCur );
		assert ( tDelta>=0 );

		tRowID = sphUnalignedRead ( *(RowID_t*)m_pCur );
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
		if ( IsLastCheckpoint() )
		{
			int iLeftover = m_nDocs % m_nDocsPerCheckpoint;
			m_iCheckpointDocs = iLeftover ? iLeftover : m_nDocsPerCheckpoint;
		}
		else
			m_iCheckpointDocs = m_nDocsPerCheckpoint;

		m_pCur = m_pData + m_pCurCheckpoint->m_tOffset;
		m_tCurDocID = m_pCurCheckpoint->m_tBaseDocID;
	}
};


bool WriteDocidLookup ( const CSphString & sFilename, const CSphFixedVector<DocidRowidPair_t> & dLookup, CSphString & sError );


#endif // _secondaryindex_