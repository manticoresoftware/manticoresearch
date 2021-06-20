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


#include "sphinx_alter.h"
#include "attribute.h"
#include "fileio.h"
#include "columnarmisc.h"
#include "docstore.h"

namespace {

void CreateAttrMap ( CSphVector<int> & dAttrMap, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, int iAttrToRemove )
{
	dAttrMap.Resize ( tOldSchema.GetAttrsCount() );
	for ( int iAttr = 0; iAttr < tOldSchema.GetAttrsCount(); iAttr++ )
	{
		const CSphColumnInfo & tOldAttr = tOldSchema.GetAttr(iAttr);
		if ( iAttr!=iAttrToRemove && !tOldAttr.IsColumnar() )
			dAttrMap[iAttr] = tNewSchema.GetAttrIndex ( tOldAttr.m_sName.cstr() );
		else
			dAttrMap[iAttr] = -1;
	}
}


const CSphRowitem * CopyRow ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo, int iOldStride )
{
	memcpy ( pTmpDocinfo, pDocinfo, iOldStride*sizeof(DWORD) );
	return pDocinfo + iOldStride;
}


const CSphRowitem * CopyRowAttrByAttr ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphVector<int> & dAttrMap, int iOldStride )
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


void AddToSchema ( CSphSchema & tSchema, const CSphString & sAttrName, ESphAttr eAttrType, bool bColumnar, CSphString & sError )
{
	const CSphColumnInfo * pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );

	bool bBlob = sphIsBlobAttr(eAttrType);
	bool bRebuild = false;
	if ( bBlob && !bColumnar && !pBlobLocator )
	{
		bRebuild = true;
		CSphColumnInfo tCol ( sphGetBlobLocatorName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;

		// should be right after docid
		tSchema.InsertAttr ( 1, tCol, false );
	}

	CSphColumnInfo tInfo ( sAttrName.cstr(), eAttrType );
	tInfo.m_uAttrFlags |= bColumnar ? CSphColumnInfo::ATTR_COLUMNAR : 0;

	if ( tSchema.GetAttrId_FirstFieldLen()!=-1 )
	{
		bRebuild = true;
		tSchema.InsertAttr ( tSchema.GetAttrId_FirstFieldLen(), tInfo, false );
	}
	else
		tSchema.AddAttr ( tInfo, false );

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
	bool bColumnar = false;
	const CSphColumnInfo * pAttrToRemove = tSchema.GetAttr ( sAttrName.cstr() );
	if ( pAttrToRemove )
		bColumnar = pAttrToRemove->IsColumnar();

	bool bBlob = sphIsBlobAttr(eAttrType);
	const CSphColumnInfo * pBlobLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );

	tSchema.RemoveAttr ( sAttrName.cstr(), false );
	if ( bBlob && !bColumnar && pBlobLocator )
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

	return true;
}


class AddRemoveCtx_c
{
public:
			AddRemoveCtx_c ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo, DWORD uNumRows, const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter, WriteWrapper_c & tSPBWriter,
				const CSphString & sAttrName, const IndexAlterHelper_c & tMinMaxer );

	bool	AddRowwiseAttr();
	bool	RemoveRowwiseAttr();

private:
	const CSphSchema & 			m_tOldSchema;
	const CSphSchema & 			m_tNewSchema;
	const CSphRowitem * 		m_pDocinfo = nullptr;
	DWORD						m_uNumRows = 0;
	const BYTE * 				m_pBlobPool = nullptr;
	WriteWrapper_c & 			m_tSPAWriter;
	WriteWrapper_c & 			m_tSPBWriter;
	const CSphString & 			m_sAttrName;
	const IndexAlterHelper_c &	m_tMinMaxer;

	const CSphColumnInfo *		m_pOldBlobRowLocator = nullptr;
	const CSphColumnInfo *		m_pNewBlobRowLocator = nullptr;
	int							m_iNumOldBlobs = 0;
	int							m_iNumNewBlobs = 0;
	bool						m_bHadBlobs = false;
	bool						m_bHaveBlobs = false;
	int							m_iOldStride = 0;
	int							m_iNewStride = 0;

	CSphFixedVector<DWORD>	m_dAttrRow {0};
	CSphTightVector<BYTE>	m_dBlobRow;
};


AddRemoveCtx_c::AddRemoveCtx_c ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo, DWORD uNumRows, const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter, WriteWrapper_c & tSPBWriter,
	const CSphString & sAttrName, const IndexAlterHelper_c & tMinMaxer )
	: m_tOldSchema ( tOldSchema )
	, m_tNewSchema ( tNewSchema )
	, m_pDocinfo ( pDocinfo )
	, m_uNumRows ( uNumRows )
	, m_pBlobPool ( pBlobPool )
	, m_tSPAWriter ( tSPAWriter )
	, m_tSPBWriter ( tSPBWriter )
	, m_sAttrName ( sAttrName )
	, m_tMinMaxer ( tMinMaxer )
	, m_pOldBlobRowLocator ( tOldSchema.GetAttr ( sphGetBlobLocatorName() ) )
	, m_pNewBlobRowLocator ( tNewSchema.GetAttr ( sphGetBlobLocatorName() ) )
	, m_iOldStride ( tOldSchema.GetRowSize() )
	, m_iNewStride ( tNewSchema.GetRowSize() )
{
	for ( int i = 0; i<tOldSchema.GetAttrsCount(); i++ )
		if ( sphIsBlobAttr ( tOldSchema.GetAttr(i) ) )
			m_iNumOldBlobs++;

	for ( int i = 0; i<tNewSchema.GetAttrsCount(); i++ )
		if ( sphIsBlobAttr ( tNewSchema.GetAttr(i) ) )
			m_iNumNewBlobs++;

	m_bHadBlobs = m_iNumOldBlobs>0;
	m_bHaveBlobs = m_iNumNewBlobs>0;

	m_dAttrRow.Reset(m_iNewStride);
}


bool AddRemoveCtx_c::AddRowwiseAttr()
{
	bool bHasFieldLen = ( m_tNewSchema.GetAttrId_FirstFieldLen()!=-1 );
	CSphVector<int> dAttrMap;
	if ( m_bHadBlobs!=m_bHaveBlobs || bHasFieldLen )
		CreateAttrMap ( dAttrMap, m_tOldSchema, m_tNewSchema, -1 );

	const CSphColumnInfo * pNewAttr = m_tNewSchema.GetAttr ( m_sAttrName.cstr() );
	assert ( pNewAttr );
	bool bBlob = sphIsBlobAttr ( *pNewAttr );

	const CSphRowitem * pNextDocinfo = nullptr;

	for ( RowID_t tRowID = 0; tRowID<m_uNumRows; tRowID++ )
	{
		if ( m_bHadBlobs==m_bHaveBlobs && !bHasFieldLen )
			pNextDocinfo = CopyRow ( m_pDocinfo, m_dAttrRow.Begin(), m_iOldStride );
		else
			pNextDocinfo = CopyRowAttrByAttr ( m_pDocinfo, m_dAttrRow.Begin(), m_tOldSchema, m_tNewSchema, dAttrMap, m_iOldStride );

		if ( !pNewAttr->m_tLocator.IsBlobAttr() )
			sphSetRowAttr ( m_dAttrRow.Begin(), pNewAttr->m_tLocator, 0 );

		if ( bBlob && !m_tMinMaxer.Alter_IsMinMax ( m_pDocinfo, m_iOldStride ) )
		{
			sphAddAttrToBlobRow ( m_pDocinfo, m_dBlobRow, m_pBlobPool, m_iNumOldBlobs, m_pOldBlobRowLocator ? &m_pOldBlobRowLocator->m_tLocator : nullptr );

			SphOffset_t tRowOffset = m_tSPBWriter.GetPos();
			m_tSPBWriter.PutBytes ( m_dBlobRow.Begin(), m_dBlobRow.GetLength() );
			if ( m_tSPBWriter.IsError() )
				return false;

			assert(m_pNewBlobRowLocator);
			sphSetRowAttr ( m_dAttrRow.Begin(), m_pNewBlobRowLocator->m_tLocator, tRowOffset );
		}

		m_tSPAWriter.PutBytes ( (const BYTE *)m_dAttrRow.Begin(), m_iNewStride*sizeof(CSphRowitem) );
		if ( m_tSPAWriter.IsError() )
			return false;

		m_pDocinfo = pNextDocinfo;
	}

	return true;
}


bool AddRemoveCtx_c::RemoveRowwiseAttr()
{
	int iAttrToRemove = m_tOldSchema.GetAttrIndex ( m_sAttrName.cstr() );
	const CSphColumnInfo & tOldAttr = m_tOldSchema.GetAttr ( iAttrToRemove );
	assert ( iAttrToRemove>=0 );

	bool bBlob = sphIsBlobAttr ( tOldAttr );
	bool bBlobsModified = bBlob && ( m_bHaveBlobs==m_bHadBlobs );

	CSphVector<int> dAttrMap;
	CreateAttrMap ( dAttrMap, m_tOldSchema, m_tNewSchema, iAttrToRemove );

	const CSphRowitem * pNextDocinfo = nullptr;

	for ( RowID_t tRowID = 0; tRowID<m_uNumRows; tRowID++ )
	{
		pNextDocinfo = CopyRowAttrByAttr ( m_pDocinfo, m_dAttrRow.Begin(), m_tOldSchema, m_tNewSchema, dAttrMap, m_iOldStride );

		if ( bBlobsModified && !m_tMinMaxer.Alter_IsMinMax ( m_pDocinfo, m_iOldStride ) )
		{
			assert(m_pOldBlobRowLocator);
			sphRemoveAttrFromBlobRow ( m_pDocinfo, m_dBlobRow, m_pBlobPool, m_iNumOldBlobs, tOldAttr.m_tLocator.m_iBlobAttrId, m_pOldBlobRowLocator->m_tLocator );

			SphOffset_t tRowOffset = m_tSPBWriter.GetPos();
			m_tSPBWriter.PutBytes ( m_dBlobRow.Begin(), m_dBlobRow.GetLength() );
			if ( m_tSPBWriter.IsError() )
				return false;

			assert(m_pNewBlobRowLocator);
			sphSetRowAttr ( m_dAttrRow.Begin(), m_pNewBlobRowLocator->m_tLocator, tRowOffset );
		}

		m_tSPAWriter.PutBytes ( (const BYTE *)m_dAttrRow.Begin(), m_iNewStride*sizeof(CSphRowitem) );
		if ( m_tSPAWriter.IsError() )
			return false;

		m_pDocinfo = pNextDocinfo;
	}

	return true;
}


} // unnamed namespace

//////////////////////////////////////////////////////////////////////////

class WriteWrapper_Disk_c : public WriteWrapper_c
{
public:
	WriteWrapper_Disk_c ( CSphWriter & tWriter )
		: m_tWriter ( tWriter )
	{}

	void PutBytes ( const BYTE * pData, int iSize ) override
	{
		m_tWriter.PutBytes ( pData, iSize );
	}

	SphOffset_t	GetPos() const override
	{
		return m_tWriter.GetPos();
	}

	bool IsError() const override
	{
		return m_tWriter.IsError();
	}

private:
	CSphWriter & m_tWriter;
};

//////////////////////////////////////////////////////////////////////////

template<typename T>
class WriteWrapper_Mem_T : public WriteWrapper_c
{
public:
	WriteWrapper_Mem_T ( CSphTightVector<T> & tBuffer )
		: m_tBuffer ( tBuffer )
	{}

	void PutBytes ( const BYTE * pData, int iSize ) override
	{
		assert ( iSize % sizeof(T) == 0 );
		T * pNew = m_tBuffer.AddN ( iSize/sizeof(T) );
		memcpy ( pNew, pData, iSize );
	}

	SphOffset_t	GetPos() const override
	{
		return m_tBuffer.GetLength()*sizeof(T);
	}

	bool IsError() const override
	{
		return false;
	}

protected:
	CSphTightVector<T> & m_tBuffer;
};

//////////////////////////////////////////////////////////////////////////

bool IndexAlterHelper_c::Alter_AddRemoveRowwiseAttr ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo, DWORD uNumRows, const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter,
	WriteWrapper_c & tSPBWriter, bool bAddAttr, const CSphString & sAttrName )
{
	AddRemoveCtx_c tCtx ( tOldSchema, tNewSchema, pDocinfo, uNumRows, pBlobPool, tSPAWriter, tSPBWriter, sAttrName, *this );
	if ( bAddAttr )
		return tCtx.AddRowwiseAttr();
	else
		return tCtx.RemoveRowwiseAttr();
}


bool IndexAlterHelper_c::Alter_AddRemoveColumnar ( bool bAdd, const ISphSchema & tOldSchema, const ISphSchema & tNewSchema, columnar::Columnar_i * pColumnar, columnar::Builder_i * pBuilder, DWORD uRows, const CSphString & sIndex, CSphString & sError )
{
	std::string sErrorSTL;

	CSphVector<std::pair<std::unique_ptr<columnar::Iterator_i>,ESphAttr>> dIterators;
	for ( int i = 0; i < tNewSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tNewAttr = tNewSchema.GetAttr(i);
		if ( !tNewAttr.IsColumnar() )
			continue;

		const CSphColumnInfo * pOldAttr = tOldSchema.GetAttr ( tNewAttr.m_sName.cstr() );
		if ( !pOldAttr )
		{
			dIterators.Add ( { nullptr, tNewAttr.m_eAttrType } );
			continue;
		}

		assert(pColumnar);
		columnar::Iterator_i * pIterator = pColumnar->CreateIterator ( tNewAttr.m_sName.cstr(), {}, sErrorSTL );
		if ( !pIterator )
		{
			sError.SetSprintf ( "%s attribute to %s: %s", bAdd ? "adding" : "removing", sIndex.cstr(), sErrorSTL.c_str() );
			return false;
		}

		dIterators.Add ( { std::unique_ptr<columnar::Iterator_i>(pIterator), pOldAttr->m_eAttrType } );
	}

	CSphVector<int64_t> dTmp;
	for ( RowID_t tRowID = 0; tRowID < uRows; tRowID++ )
	{
		for ( int iColumnarAttr = 0; iColumnarAttr < dIterators.GetLength(); iColumnarAttr++ )
		{
			auto & tIterator = dIterators[iColumnarAttr];
			if ( tIterator.first )
			{
				Verify ( AdvanceIterator ( tIterator.first.get(), tRowID ) );
				SetColumnarAttr ( iColumnarAttr, tIterator.second, pBuilder, tIterator.first.get(), dTmp );
			}
			else
				SetDefaultColumnarAttr ( iColumnarAttr, tIterator.second, pBuilder );
		}
	}

	if ( pBuilder && !pBuilder->Done(sErrorSTL) )
	{
		sError = sErrorSTL.c_str();
		return false;
	}

	return true;
}


bool IndexAlterHelper_c::Alter_AddRemoveFromSchema ( CSphSchema & tSchema, const CSphString & sAttrName, ESphAttr eAttrType, bool bColumnar, bool bAdd, CSphString & sError )
{
	if ( bAdd && bColumnar && !IsColumnarLibLoaded() )
	{
		sError.SetSprintf ( "Unable to add a columnar attribute '%s': columnar library not loaded", sAttrName.cstr() );
		return false;
	}

	if ( bAdd )
	{
		AddToSchema ( tSchema, sAttrName, eAttrType, bColumnar, sError );
		return true;
	}

	return RemoveFromSchema ( tSchema, sAttrName, eAttrType, sError );
}


bool IndexAlterHelper_c::Alter_AddRemoveFieldFromSchema ( bool bAdd, CSphSchema & tSchema, const CSphString & sFieldName, DWORD uFieldFlags, CSphString & sError )
{
	if ( bAdd )
	{
		CSphColumnInfo tField;
		tField.m_sName = sFieldName;
		tField.m_uFieldFlags = uFieldFlags;
		//tField.m_bPayload = false; // fixme? support it or not?
		tSchema.AddField(tField);
		return true;
	}
	else
	{
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


void IndexAlterHelper_c::Alter_AddRemoveFieldFromDocstore ( DocstoreBuilder_i & tBuilder, const Docstore_i * pDocstore, DWORD uNumDocs, const CSphSchema & tNewSchema )
{
	struct Field_t
	{
		CSphString	m_sName;
		int			m_iOldId = -1;
		int			m_iRsetId = -1;
	};

	CSphVector<Field_t> dStoredFields;
	for ( int i = 0; i < tNewSchema.GetFieldsCount(); i++ )
		if ( tNewSchema.IsFieldStored(i) )
		{
			const CSphString & sName = tNewSchema.GetFieldName(i);
			int iFieldId = pDocstore ? pDocstore->GetFieldId ( sName, DOCSTORE_TEXT ) : -1;
			dStoredFields.Add ( { sName, iFieldId, -1 } );
		}

	IntVec_t dStoredFieldIds;
	for ( auto & i : dStoredFields )
	{
		if ( i.m_iOldId>=0 )
		{
			i.m_iRsetId = dStoredFieldIds.GetLength();
			dStoredFieldIds.Add ( i.m_iOldId );
		}

		tBuilder.AddField ( i.m_sName, DOCSTORE_TEXT );
	}

	DocstoreDoc_t tOldDoc;
	DocstoreBuilder_i::Doc_t tNewDoc;
	tNewDoc.m_dFields.Resize ( dStoredFields.GetLength() );

	for ( RowID_t tRowID = 0; tRowID < uNumDocs; tRowID++ )
	{
		if ( pDocstore )
			tOldDoc = pDocstore->GetDoc ( tRowID, &dStoredFieldIds, -1, false );

		ARRAY_FOREACH ( i, dStoredFields )
		{
			const Field_t & tField = dStoredFields[i];
			if ( tField.m_iRsetId!=-1 )
				tNewDoc.m_dFields[i] = tOldDoc.m_dFields[tField.m_iRsetId];
			else
				tNewDoc.m_dFields[i] = {nullptr,0};
		}

		tBuilder.AddDoc ( tRowID, tNewDoc );
	}

	tBuilder.Finalize();
}

//////////////////////////////////////////////////////////////////////////

WriteWrapper_c * CreateWriteWrapperDisk ( CSphWriter & tWriter )
{
	return new WriteWrapper_Disk_c(tWriter);
}


WriteWrapper_c * CreateWriteWrapperMem ( CSphTightVector<CSphRowitem> & dSPA )
{
	return new WriteWrapper_Mem_T<CSphRowitem>(dSPA);
}


WriteWrapper_c * CreateWriteWrapperMem ( CSphTightVector<BYTE> & dSPB )
{
	return new WriteWrapper_Mem_T<BYTE>(dSPB);
}
