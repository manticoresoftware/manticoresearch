//
// Copyright (c) 2018, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _killlist_
#define _killlist_

#include "sphinx.h"


class CSphReader;
class CSphWriter;


class DeadRowMap_c
{
public:
	virtual			~DeadRowMap_c(){}

	bool			HasDead() const;
	virtual	int64_t	GetLengthBytes() const = 0;

protected:
	bool			m_bHaveDead {false};
	DWORD			m_uRows {0};

	void			CheckForDead ( const DWORD * pData, const DWORD * pDataEnd );
	bool			Set ( RowID_t tRowID, DWORD * pData );
	inline bool		IsSet ( RowID_t tRowID, const DWORD * pData ) const
	{
		if ( !m_bHaveDead )
			return false;

		assert ( tRowID < m_uRows );
		return ( pData [ tRowID>>5 ] & ( 1UL<<( tRowID&31 ) ) )!=0;
	}

private:
#if NO_ATOMIC
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
		return DeadRowMap_c::IsSet ( tRowID, m_tData.GetWritePtr() );
	}

	int64_t		GetLengthBytes() const override;
	bool		Flush ( bool bWaitComplete, CSphString & sError ) const;
	bool		Prealloc ( DWORD uRows, const CSphString & sFilename, CSphString & sError );
	void		Dealloc();
	void		Preread ( const char * sIndexName, const char * sFor, bool bMlock );

private:
	CSphMappedBuffer<DWORD> m_tData;
};


class DeadRowMap_Ram_c : public DeadRowMap_c
{
public:
				DeadRowMap_Ram_c ( DWORD uRows );

	bool		Set ( RowID_t tRowID );
	bool		IsSet ( RowID_t tRowID ) const;
	void		Reset ( DWORD uRows );

	int64_t		GetLengthBytes() const override;
	DWORD		GetNumAlive() const;

	void		Load ( DWORD uRows, CSphReader & tReader, CSphString & sError );
	void		Save ( CSphWriter & tWriter ) const;

private:
	CSphFixedVector<DWORD> m_dData;
};


class DocidListReader_c
{
public:
	DocidListReader_c ( const DocID_t * pKlist, int iKlistSize )
		: m_pIterator ( pKlist )
		, m_pMaxIterator ( pKlist+iKlistSize )
	{}


	DocidListReader_c ( const VecTraits_T<DocID_t> & dKlist )
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

	static inline void HintDocID ( DocID_t tDocID ) {}

private:
	const DocID_t * m_pIterator {nullptr};
	const DocID_t * m_pMaxIterator {nullptr};
};


template <typename TARGET, typename KILLER, typename MAP>
int KillByLookup ( TARGET & tTargetReader, KILLER & tKillerReader, MAP & tDeadRowMap )
{
	RowID_t tTargetRowID = INVALID_ROWID;

	DocID_t tKillerDocID = 0, tTargetDocID = 0;
	bool bHaveKillerDocs = tKillerReader.ReadDocID ( tKillerDocID );
	bool bHaveTargetDocs = tTargetReader.Read ( tTargetDocID, tTargetRowID );

	int iKilled = 0;

	while ( bHaveKillerDocs && bHaveTargetDocs )
	{
		if ( tKillerDocID < tTargetDocID )
		{
			tKillerReader.HintDocID ( tTargetDocID );
			bHaveKillerDocs = tKillerReader.ReadDocID ( tKillerDocID );
		}
		else if ( tKillerDocID > tTargetDocID )
		{
			tTargetReader.HintDocID ( tKillerDocID );
			bHaveTargetDocs = tTargetReader.Read ( tTargetDocID, tTargetRowID );
		}
		else
		{
			if ( tDeadRowMap.Set ( tTargetRowID ) )
				iKilled++;

			bHaveKillerDocs = tKillerReader.ReadDocID ( tKillerDocID );
			bHaveTargetDocs = tTargetReader.Read ( tTargetDocID, tTargetRowID );
		}
	}

	return iKilled;
}


bool			WriteDeadRowMap ( const CSphString & sFilename, DWORD uTotalDocs, CSphString & sError );



#endif // _killlist_