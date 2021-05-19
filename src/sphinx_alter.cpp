//
// Copyright (c) 2021, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "attribute.h"
// #include "sphinx.h" // included by attribute.h

namespace {

void CreateAttrMap ( CSphVector<int> & dAttrMap, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, int iAttrToRemove )
{
	dAttrMap.Resize ( tOldSchema.GetAttrsCount() );
	for ( int iAttr = 0; iAttr < tOldSchema.GetAttrsCount(); iAttr++ )
		if ( iAttr!=iAttrToRemove )
			dAttrMap[iAttr] = tNewSchema.GetAttrIndex ( tOldSchema.GetAttr(iAttr).m_sName.cstr() );
		else
			dAttrMap[iAttr] = -1;
}


const CSphRowitem * CopyRow ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo, int iOldStride )
{
	memcpy ( pTmpDocinfo, pDocinfo, iOldStride*sizeof(DWORD) );
	return pDocinfo + iOldStride;
}


const CSphRowitem * CopyRowAttrByAttr ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo
		, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphVector<int> & dAttrMap
		, int iOldStride )
{
	for ( int iAttr = 0; iAttr < tOldSchema.GetAttrsCount(); iAttr++ )
	{
		if ( dAttrMap[iAttr]==-1 )
			continue;

		const CSphColumnInfo & tOldAttr = tOldSchema.GetAttr(iAttr);
		const CSphColumnInfo & tNewAttr = tNewSchema.GetAttr(dAttrMap[iAttr]);

		if ( sphIsBlobAttr(tOldAttr) )
			continue;

		SphAttr_t tValue = sphGetRowAttr ( pDocinfo, tOldAttr.m_tLocator );
		sphSetRowAttr ( pTmpDocinfo, tNewAttr.m_tLocator, tValue );
	}

	return pDocinfo + iOldStride;
}

void AddToSchema ( CSphSchema & tSchema, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError )
{
	const CSphColumnInfo * pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );

	bool bBlob = sphIsBlobAttr ( eAttrType );

	bool bRebuild = false;
	if ( bBlob && !pBlobLocator )
	{
		bRebuild = true;
		CSphColumnInfo tCol ( sphGetBlobLocatorName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;

		// should be right after docid
		tSchema.InsertAttr ( 1, tCol, false );
	}

	CSphColumnInfo tInfo ( sAttrName.cstr(), eAttrType );
	if ( tSchema.GetAttrId_FirstFieldLen()!=-1 )
	{
		bRebuild = true;
		tSchema.InsertAttr ( tSchema.GetAttrId_FirstFieldLen(), tInfo, false );
	} else
	{
		tSchema.AddAttr ( tInfo, false );
	}

	// rebuild locators in the schema
	if ( bRebuild )
	{
		const char * szTmpColName = "$_tmp";
		CSphColumnInfo tTmpCol ( szTmpColName, SPH_ATTR_BIGINT );
		tSchema.AddAttr ( tTmpCol, false );
		tSchema.RemoveAttr ( szTmpColName, false );
	}
}

bool RemoveFromSchema ( CSphSchema & tSchema, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError )
{
	const CSphColumnInfo * pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );
	bool bBlob = sphIsBlobAttr ( eAttrType );
	{
		tSchema.RemoveAttr ( sAttrName.cstr(), false );

		if ( bBlob && pBlobLocator )
		{
			// remove blob locator if no blobs are left
			int nBlobs = 0;
			for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
				if ( sphIsBlobAttr ( tSchema.GetAttr(i) ) )
					nBlobs++;

			if ( !nBlobs )
				tSchema.RemoveAttr ( sphGetBlobLocatorName(), false );
		}

		if ( !tSchema.GetAttrsCount() )
		{
			sError = "unable to remove the only attribute left";
			return false;
		}
	}

	return true;
}


class AddRemoveCtx_c
{
	// params
	const CSphSchema & 		m_tOldSchema;
	const CSphSchema & 		m_tNewSchema;
	const CSphRowitem * 	m_pDocinfo;
	const CSphRowitem * 	m_pDocinfoMax;
	const BYTE * 			m_pBlobPool;
	WriteWrapper_c & 		m_tSPAWriter;
	WriteWrapper_c & 		m_tSPBWriter;
	const CSphString & 		m_sAttrName;
	IndexAlterHelper_c&		m_tMinMaxer;

	// ctx
	int m_nOldBlobs;
	int m_nNewBlobs;
	bool m_bHadBlobs;
	bool m_bHaveBlobs;
	int m_iOldStride;
	int m_iNewStride;
	CSphFixedVector<DWORD> m_dAttrRow {0};
	CSphTightVector<BYTE> m_dBlobRow;

public:
	AddRemoveCtx_c ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo,
			const CSphRowitem * pDocinfoMax, const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter,
			WriteWrapper_c & tSPBWriter, const CSphString & sAttrName, IndexAlterHelper_c& tMinMaxer )
			: m_tOldSchema ( tOldSchema ), m_tNewSchema ( tNewSchema ), m_pDocinfo ( pDocinfo ),
			m_pDocinfoMax (pDocinfoMax), m_pBlobPool ( pBlobPool ), m_tSPAWriter ( tSPAWriter ),
			m_tSPBWriter ( tSPBWriter ), m_sAttrName ( sAttrName ), m_tMinMaxer (tMinMaxer)

	{
		m_nOldBlobs = 0;
		for ( int i = 0; i<tOldSchema.GetAttrsCount (); ++i )
			if ( sphIsBlobAttr ( tOldSchema.GetAttr ( i ) ) )
				++m_nOldBlobs;

		m_nNewBlobs = 0;
		for ( int i = 0; i<tNewSchema.GetAttrsCount (); ++i )
			if ( sphIsBlobAttr ( tNewSchema.GetAttr ( i ) ) )
				++m_nNewBlobs;

		m_bHadBlobs = m_nOldBlobs>0;
		m_bHaveBlobs = m_nNewBlobs>0;

		m_iOldStride = tOldSchema.GetRowSize ();
		m_iNewStride = tNewSchema.GetRowSize ();

		m_dAttrRow.Reset ( m_iNewStride );
	}

	bool AddAttr ()
	{
		const CSphColumnInfo * pAttr = m_tNewSchema.GetAttr ( m_sAttrName.cstr () );
		assert ( pAttr );

		bool bBlob = sphIsBlobAttr ( *pAttr );
		const CSphRowitem * pNextDocinfo;

		bool bHasFieldLen = ( m_tNewSchema.GetAttrId_FirstFieldLen()!=-1 );
		CSphVector<int> dAttrMap;
		if ( m_bHadBlobs!=m_bHaveBlobs || bHasFieldLen )
			CreateAttrMap ( dAttrMap, m_tOldSchema, m_tNewSchema, -1 );

		const CSphColumnInfo * pNewAttr = m_tNewSchema.GetAttr ( m_sAttrName.cstr() );
		assert ( pNewAttr );

		while ( m_pDocinfo<m_pDocinfoMax )
		{
			if ( m_bHadBlobs==m_bHaveBlobs && !bHasFieldLen )
				pNextDocinfo = CopyRow ( m_pDocinfo, m_dAttrRow.Begin (), m_iOldStride );
			else
				pNextDocinfo = CopyRowAttrByAttr ( m_pDocinfo, m_dAttrRow.Begin (), m_tOldSchema, m_tNewSchema, dAttrMap, m_iOldStride );

			if ( !pNewAttr->m_tLocator.IsBlobAttr () )
				sphSetRowAttr ( m_dAttrRow.Begin (), pNewAttr->m_tLocator, 0 );

			if ( bBlob && !m_tMinMaxer.Alter_IsMinMax ( m_pDocinfo, m_iOldStride ) )
			{
				sphAddAttrToBlobRow ( m_pDocinfo, m_dBlobRow, m_pBlobPool, m_nOldBlobs );

				SphOffset_t tRowOffset = m_tSPBWriter.GetPos ();
				m_tSPBWriter.PutBytes ( m_dBlobRow.Begin (), m_dBlobRow.GetLength () );
				if ( m_tSPBWriter.IsError () )
					return false;

				sphSetBlobRowOffset ( m_dAttrRow.Begin (), tRowOffset );
			}

			m_tSPAWriter.PutBytes ( (const BYTE *) m_dAttrRow.Begin (), m_iNewStride * sizeof ( CSphRowitem ) );
			if ( m_tSPAWriter.IsError () )
				return false;

			m_pDocinfo = pNextDocinfo;
		}
		return true;
	}

	bool RemoveAttr ()
	{
		const CSphColumnInfo * pAttr = m_tOldSchema.GetAttr ( m_sAttrName.cstr () );
		assert ( pAttr );

		bool bBlob = sphIsBlobAttr ( *pAttr );
		bool bBlobsModified = bBlob && ( m_bHaveBlobs==m_bHadBlobs );


		int iAttrToRemove = m_tOldSchema.GetAttrIndex ( m_sAttrName.cstr () );
		const CSphColumnInfo & tOldAttr = m_tOldSchema.GetAttr ( iAttrToRemove );
		assert ( iAttrToRemove>=0 );

		CSphVector<int> dAttrMap;
		CreateAttrMap ( dAttrMap, m_tOldSchema, m_tNewSchema, iAttrToRemove );

		while ( m_pDocinfo<m_pDocinfoMax )
		{
			auto * pNextDocinfo = CopyRowAttrByAttr ( m_pDocinfo, m_dAttrRow.Begin (), m_tOldSchema, m_tNewSchema, dAttrMap, m_iOldStride );
			if ( bBlobsModified && !m_tMinMaxer.Alter_IsMinMax ( m_pDocinfo, m_iOldStride ) )
			{
				sphRemoveAttrFromBlobRow ( m_pDocinfo, m_dBlobRow, m_pBlobPool, m_nOldBlobs
										   , tOldAttr.m_tLocator.m_iBlobAttrId );

				SphOffset_t tRowOffset = m_tSPBWriter.GetPos ();
				m_tSPBWriter.PutBytes ( m_dBlobRow.Begin (), m_dBlobRow.GetLength () );
				if ( m_tSPBWriter.IsError () )
					return false;

				sphSetBlobRowOffset ( m_dAttrRow.Begin (), tRowOffset );
			}

			m_tSPAWriter.PutBytes ( (const BYTE *) m_dAttrRow.Begin (), m_iNewStride * sizeof ( CSphRowitem ) );
			if ( m_tSPAWriter.IsError () )
				return false;

			m_pDocinfo = pNextDocinfo;
		}
		return true;
	}

};

} // unnamed namespace

bool IndexAlterHelper_c::Alter_AddRemoveAttr ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo, const CSphRowitem * pDocinfoMax,
	const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter, WriteWrapper_c & tSPBWriter, bool bAddAttr, const CSphString & sAttrName )
{
	AddRemoveCtx_c tCtx ( tOldSchema, tNewSchema, pDocinfo, pDocinfoMax, pBlobPool, tSPAWriter, tSPBWriter, sAttrName, *this );
	if ( bAddAttr )
		return tCtx.AddAttr();
	else
		return tCtx.RemoveAttr();
}


bool IndexAlterHelper_c::Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const
{
	return false;
}


bool IndexAlterHelper_c::Alter_AddRemoveFromSchema ( CSphSchema & tSchema, const CSphString & sAttrName, ESphAttr eAttrType, bool bAdd, CSphString & sError )
{
	if ( !bAdd )
		return RemoveFromSchema (tSchema, sAttrName, eAttrType, sError );
	AddToSchema ( tSchema, sAttrName, eAttrType, sError );
	return true;
}


bool IndexAlterHelper_c::Alter_AddRemoveFieldFromSchema ( bool bAdd, CSphSchema & tSchema, const CSphString & sFieldName, CSphString & sError )
{
	if ( bAdd )
	{
		// fixme! dupes?
		//tCol.m_uFieldFlags = CSphColumnInfo::FIELD_INDEXED; // fixme! stored must be processed someway aside
		//tCol.m_bPayload = false; // fixme? support it or not?
		tSchema.AddField ( sFieldName.cstr () );
		return true;
	} else {
		auto iIdx = tSchema.GetFieldIndex ( sFieldName.cstr () );
		if ( iIdx>=0 && tSchema.GetFieldsCount()==1 )
		{
			sError.SetSprintf ("Unable to delete last field from the index");
			return false;
		}
		tSchema.RemoveField ( iIdx );
		return true;
	}
}

