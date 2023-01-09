//
// Copyright (c) 2018-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _killlist_
#define _killlist_

#include "fileutils.h"
#include "sphinxdefs.h"

class CSphReader;
class CSphWriter;


class DeadRowMap_c
{
public:
	virtual			~DeadRowMap_c(){}

	bool			HasDead() const;
	DWORD			GetNumDeads() const;
	virtual	int64_t	GetLengthBytes() const = 0;
	virtual uint64_t GetCoreSize () const = 0;

protected:
	bool			m_bHaveDead {false};
	mutable int64_t	m_iNumDeads = -1;		// means 'not initialized'
	DWORD			m_uRows {0};

	void			CheckForDead ( const DWORD * pData, const DWORD * pDataEnd );
	bool			Set ( RowID_t tRowID, DWORD * pData );
	inline bool		IsSet ( RowID_t tRowID, const DWORD * pData ) const
	{
		if ( !m_bHaveDead || tRowID==INVALID_ROWID )
			return false;

		assert ( tRowID < m_uRows );
		return ( pData [ tRowID>>5U ] & ( 1UL<<( tRowID&31U ) ) )!=0;
	}

private:
	virtual DWORD CountDeads () const = 0;	// heavy doc-by-doc counting
#if !(_WIN32) && !(HAVE_SYNC_FETCH)
	CSphMutex		m_tLock;
#endif
};



class DeadRowMap_Disk_c : public DeadRowMap_c
{
	friend class DeadRowMap_Ram_c;

public:
				~DeadRowMap_Disk_c() override;

	bool		Set ( RowID_t tRowID );
	inline bool	IsSet ( RowID_t tRowID ) const
	{
		return DeadRowMap_c::IsSet ( tRowID, m_tData.GetReadPtr() );
	}

	int64_t		GetLengthBytes() const override;
	uint64_t	GetCoreSize () const override;
	bool		Flush ( bool bWaitComplete, CSphString & sError ) const;
	bool		Prealloc ( DWORD uRows, const CSphString & sFilename, CSphString & sError );
	void		Dealloc();
	void		Preread ( const char * sIndexName, const char * sFor, bool bMlock );

private:
	DWORD CountDeads () const final;
	CSphMappedBuffer<DWORD> m_tData;
};


class DeadRowMap_Ram_c : public DeadRowMap_c
{
public:
				explicit DeadRowMap_Ram_c ( DWORD uRows );

	bool		Set ( RowID_t tRowID );
	bool		IsSet ( RowID_t tRowID ) const;
	void		Reset ( DWORD uRows );

	int64_t		GetLengthBytes() const override;
	uint64_t	GetCoreSize () const override;
	DWORD		GetNumAlive() const;

	void		Load ( DWORD uRows, CSphReader & tReader, CSphString & sError );
	void		Save ( CSphWriter & tWriter ) const;

private:
	DWORD CountDeads () const final;
	CSphFixedVector<DWORD> m_dData {0};
};


class DocidListReader_c
{
public:
	explicit DocidListReader_c ( const VecTraits_T<DocID_t> & dKlist )
		: m_pIterator ( dKlist.Begin() )
		, m_pMaxIterator ( dKlist.Begin() + dKlist.GetLength() ) // should be this way till VecTraits.End got fixed
	{}


	static inline bool Read ( DocID_t & tDocID, RowID_t & tRowID )
	{
		return false;
	}

	inline bool ReadDocID ( DocID_t & tDocID )
	{
		if ( m_pIterator>=m_pMaxIterator )
			return false;

		tDocID = *m_pIterator++;
		return true;
	}

	static inline void HintDocID ( DocID_t ) {}

private:
	const DocID_t * m_pIterator {nullptr};
	const DocID_t * m_pMaxIterator {nullptr};
};


template<typename READER1, typename READER2, typename FUNCTOR>
void Intersect ( READER1& tReader1, READER2& tReader2, FUNCTOR&& fnFunctor )
{
	RowID_t tRowID1 = INVALID_ROWID;
	DocID_t tDocID1 = 0, tDocID2 = 0;
	bool bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
	bool bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );

	while ( bHaveDocs1 && bHaveDocs2 )
	{
		if ( tDocID1 < tDocID2 )
		{
			tReader1.HintDocID ( tDocID2 );
			bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
		} else if ( tDocID1 > tDocID2 )
		{
			tReader2.HintDocID ( tDocID1 );
			bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );
		} else
		{
			fnFunctor ( tRowID1, tDocID1, tReader2 );
			bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
			bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );
		}
	}
}

template<typename TARGETREADER, typename KILLERREADER, typename FNACTION>
int ProcessIntersected ( TARGETREADER& tReader1, KILLERREADER& tReader2, FNACTION fnAction )
{
	int iProcessed = 0;
	Intersect ( tReader1, tReader2, [&iProcessed, fnAction = std::move ( fnAction )] ( RowID_t tRowID, DocID_t tDocID, KILLERREADER& ) {
		if ( fnAction ( tRowID, tDocID ) )
			++iProcessed;
	} );

	return iProcessed;
}

template <typename TARGET, typename KILLER, typename MAP>
int KillByLookup ( TARGET & tTargetReader, KILLER & tKillerReader, MAP & tDeadRowMap )
{
	return ProcessIntersected ( tTargetReader, tKillerReader, [&tDeadRowMap] ( RowID_t tRowID, DocID_t ) { return tDeadRowMap.Set ( tRowID ); } );
}


bool			WriteDeadRowMap ( const CSphString & sFilename, DWORD uTotalDocs, CSphString & sError );



#endif // _killlist_
