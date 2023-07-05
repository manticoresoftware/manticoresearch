//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "attribute.h"

#include "sphinxint.h"
#include "sphinxjson.h"
#include "indexcheck.h"
#include "schema/locator.h"

#if __has_include( <charconv>)
#include <charconv>
#endif

//////////////////////////////////////////////////////////////////////////
// blob attributes

enum
{
	BLOB_ROW_LEN_BYTE = 0,
	BLOB_ROW_LEN_WORD = 1,
	BLOB_ROW_LEN_DWORD = 2
};


static BYTE CalcBlobRowFlags ( DWORD uTotalLen )
{
	if ( uTotalLen<0xFF )
		return BLOB_ROW_LEN_BYTE;

	if ( uTotalLen<0xFFFF )
		return BLOB_ROW_LEN_WORD;

	return BLOB_ROW_LEN_DWORD;
}


static DWORD RowFlagsToLen ( BYTE uFlags )
{
	switch ( uFlags )
	{
	case BLOB_ROW_LEN_BYTE:		return 1;
	case BLOB_ROW_LEN_WORD:		return 2;
	case BLOB_ROW_LEN_DWORD:	return 4;
	default:
		assert ( 0 && "Unknown blob flags" );
		return 0;
	}
}


class AttributePacker_i
{
public:
	virtual								~AttributePacker_i(){}
	virtual bool						SetData ( const BYTE * pData, int iDataLen, CSphString & sError ) = 0;
	virtual const CSphVector<BYTE> &	GetData() const = 0;
};


class AttributePacker_c : public AttributePacker_i
{
public:
	bool SetData ( const BYTE * pData, int iDataLen, CSphString & /*sError*/ ) override
	{
		m_dData.Resize ( iDataLen );
		memcpy ( m_dData.Begin(), pData, iDataLen );
		return true;
	}

	const CSphVector<BYTE> & GetData() const override
	{
		return m_dData;
	}

protected:
	CSphVector<BYTE>	m_dData;
};


// packs MVAs coming from updates (pairs of DWORDS for each value)
template <typename INT>
class AttributePacker_MVA_T : public AttributePacker_c
{
public:
	bool SetData ( const BYTE * pData, int iDataLen, CSphString & /*sError*/ ) override
	{
		int iValueSize = sizeof ( int64_t );
		int nValues = iDataLen/iValueSize;
		m_dData.Resize ( nValues*sizeof(INT) );
		auto * pResult = (INT*)m_dData.Begin();

		for ( int i = 0; i<nValues; i++ )
		{
			auto iVal = sphUnalignedRead ( *(int64_t*)const_cast<BYTE*>(pData) );
			*pResult = INT(iVal);
			pResult++;
			pData += iValueSize;
		}

		return true;
	}
};

using AttributePacker_MVA32_c = AttributePacker_MVA_T<DWORD>;

class AttributePacker_Json_c : public AttributePacker_c
{
public:
	bool SetData ( const BYTE * pData, int iDataLen, CSphString & sError ) override
	{
		m_dData.Resize(0);
		if ( !iDataLen )
			return true;

		// WARNING, tricky bit
		// flex lexer needs last two (!) bytes to be zeroes
		// asciiz string supplies one, and we fill out the extra one
		// and that works, because CSphString always allocates a small extra gap
		char * szData = const_cast<char*>((const char*)pData);
		szData[iDataLen] = '\0';
		szData[iDataLen+1] = '\0';

		return sphJsonParse ( m_dData, szData, g_bJsonAutoconvNumbers, g_bJsonKeynamesToLowercase, true, sError );
	}
};

//////////////////////////////////////////////////////////////////////////
class BlobRowBuilder_Base_c : public BlobRowBuilder_i
{
public:
	bool					SetAttr ( int iAttr, const BYTE * pData, int iDataLen, CSphString & sError ) override;

protected:
	CSphVector<std::unique_ptr<AttributePacker_i>> m_dAttrs;
};


bool BlobRowBuilder_Base_c::SetAttr ( int iAttr, const BYTE * pData, int iDataLen, CSphString & sError )
{
	return m_dAttrs[iAttr]->SetData ( pData, iDataLen, sError );
}

//////////////////////////////////////////////////////////////////////////
class BlobRowBuilder_File_c : public BlobRowBuilder_Base_c
{
public:
							BlobRowBuilder_File_c ( const ISphSchema & tSchema, SphOffset_t tSpaceForUpdates, bool bJsonPacked );

	bool					Setup ( const CSphString & sFile, CSphString & sError );

	SphOffset_t				Flush() override;
	SphOffset_t				Flush ( const BYTE * pOldRow ) override;
	bool					Done ( CSphString & sError ) override;

private:
	CSphWriter				m_tWriter;
	SphOffset_t				m_tSpaceForUpdates {0};
};


BlobRowBuilder_File_c::BlobRowBuilder_File_c ( const ISphSchema & tSchema, SphOffset_t tSpaceForUpdates, bool bJsonPacked )
	: m_tSpaceForUpdates ( tSpaceForUpdates )
{
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		if ( !sphIsBlobAttr(tCol) )
			continue;

		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_STRING:
		case SPH_ATTR_INT64SET:
			m_dAttrs.Add ( std::make_unique<AttributePacker_c>() );
			break;

		case SPH_ATTR_UINT32SET:
			m_dAttrs.Add ( std::make_unique<AttributePacker_MVA32_c>() );
			break;

		case SPH_ATTR_JSON:
			if ( bJsonPacked )
				m_dAttrs.Add ( std::make_unique<AttributePacker_c>() );
			else
				m_dAttrs.Add ( std::make_unique<AttributePacker_Json_c>() );
			break;

		default:
			break;
		}
	}
}


bool BlobRowBuilder_File_c::Setup ( const CSphString & sFile, CSphString & sError )
{
	if ( !m_tWriter.OpenFile ( sFile, sError ) )
		return false;

	// reserve space
	m_tWriter.PutOffset(0);

	return true;
}


SphOffset_t BlobRowBuilder_File_c::Flush()
{
	SphOffset_t tRowOffset = m_tWriter.GetPos();

	DWORD uTotalLen = 0;
	for ( const auto & i : m_dAttrs )
		uTotalLen += i->GetData().GetLength();

	BYTE uFlags = CalcBlobRowFlags(uTotalLen);
	m_tWriter.PutByte(uFlags);

	uTotalLen = 0;
	for ( const auto & i : m_dAttrs )
	{
		uTotalLen += i->GetData().GetLength();
		switch ( uFlags )
		{
		case BLOB_ROW_LEN_BYTE:
			m_tWriter.PutByte((BYTE)uTotalLen);
			break;

		case BLOB_ROW_LEN_WORD:
			m_tWriter.PutWord((WORD)uTotalLen);
			break;

		case BLOB_ROW_LEN_DWORD:
			m_tWriter.PutDword(uTotalLen);
			break;

		default:
			break;
		}	
	}

	for ( const auto & i : m_dAttrs )
		m_tWriter.PutBytes ( i->GetData().Begin(), i->GetData().GetLength() );

	return tRowOffset;
}


SphOffset_t BlobRowBuilder_File_c::Flush ( const BYTE * pOldRow )
{
	assert ( pOldRow );
	SphOffset_t tRowOffset = m_tWriter.GetPos();
	m_tWriter.PutBytes ( pOldRow, sphGetBlobTotalLen ( pOldRow, m_dAttrs.GetLength() ) );

	return tRowOffset;
}


bool BlobRowBuilder_File_c::Done ( CSphString & sError )
{
	SphOffset_t tTotalSize = m_tWriter.GetPos();
	SeekAndPutOffset ( m_tWriter, 0, tTotalSize );
	m_tWriter.SeekTo ( tTotalSize + m_tSpaceForUpdates, true );
	m_tWriter.CloseFile(); // closing file implies it will NOT be unlinked

	if ( m_tWriter.IsError() )
	{
		sError.SetSprintf ( "error writing .SPB, %s", sError.cstr() ); // keep original error from writer
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

class BlobRowBuilder_Mem_c : public BlobRowBuilder_Base_c
{
public:
							BlobRowBuilder_Mem_c ( CSphTightVector<BYTE> & dPool );
							BlobRowBuilder_Mem_c ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool );

	SphOffset_t				Flush() override;
	SphOffset_t				Flush ( const BYTE * pOldRow ) override;
	bool					Done ( CSphString & /*sError*/ ) override { return true; }

protected:
	CSphTightVector<BYTE> & m_dPool;
};


BlobRowBuilder_Mem_c::BlobRowBuilder_Mem_c ( CSphTightVector<BYTE> & dPool )
	: m_dPool ( dPool )
{}


BlobRowBuilder_Mem_c::BlobRowBuilder_Mem_c ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool )
	: m_dPool ( dPool )
{
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		if ( tCol.IsColumnar() )
			continue;

		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_STRING:
		case SPH_ATTR_JSON:			// json doesn't go to a separate packer because we work with pre-parsed json in this case
		case SPH_ATTR_INT64SET:
			m_dAttrs.Add ( std::make_unique<AttributePacker_c>() );
			break;

		case SPH_ATTR_UINT32SET:
			m_dAttrs.Add ( std::make_unique<AttributePacker_MVA32_c>() );
			break;

		default:
			break;
		}
	}
}


SphOffset_t BlobRowBuilder_Mem_c::Flush()
{
	SphOffset_t tRowOffset = m_dPool.GetLength();

	DWORD uTotalLen = 0;
	for ( const auto & i : m_dAttrs )
		uTotalLen += i->GetData().GetLength();

	m_dPool.Reserve ( uTotalLen + m_dAttrs.GetLength()*sizeof(DWORD) + 1 );

	BYTE uFlags = CalcBlobRowFlags(uTotalLen);
	m_dPool.Add(uFlags);

	uTotalLen = 0;
	BYTE * pPtr;
	for ( const auto & i : m_dAttrs )
	{
		uTotalLen += i->GetData().GetLength();
		switch ( uFlags )
		{
		case BLOB_ROW_LEN_BYTE:
			m_dPool.Add((BYTE)uTotalLen);
			break;

		case BLOB_ROW_LEN_WORD:
			pPtr = m_dPool.AddN ( sizeof(WORD) );
			sphUnalignedWrite ( pPtr, (WORD)uTotalLen );
			break;

		case BLOB_ROW_LEN_DWORD:
			pPtr = m_dPool.AddN ( sizeof(DWORD) );
			sphUnalignedWrite ( pPtr, (DWORD)uTotalLen );
			break;
		}	
	}

	for ( const auto & i : m_dAttrs )
	{
		int iLen = i->GetData().GetLength();
		pPtr = m_dPool.AddN(iLen);
		memcpy ( pPtr, i->GetData().Begin(), iLen );
	}

	return tRowOffset;
}


SphOffset_t BlobRowBuilder_Mem_c::Flush ( const BYTE * pOldRow )
{
	assert ( 0 );
	return 0;
}


//////////////////////////////////////////////////////////////////////////

class BlobRowBuilder_MemUpdate_c : public BlobRowBuilder_Mem_c
{
public:
	BlobRowBuilder_MemUpdate_c ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool, const CSphBitvec & dAttrsUpdated );
};


BlobRowBuilder_MemUpdate_c::BlobRowBuilder_MemUpdate_c ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool, const CSphBitvec & dAttrsUpdated )
	: BlobRowBuilder_Mem_c ( dPool )
{
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);

		if ( !dAttrsUpdated.BitGet(i) && sphIsBlobAttr(tCol) )
		{
			m_dAttrs.Add ( std::make_unique<AttributePacker_c>() );
			continue;
		}

		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_STRING:
			m_dAttrs.Add ( std::make_unique<AttributePacker_c>() );
			break;

		case SPH_ATTR_UINT32SET:
			m_dAttrs.Add ( std::make_unique<AttributePacker_MVA_T<DWORD>>() );
			break;

		case SPH_ATTR_INT64SET:
			m_dAttrs.Add ( std::make_unique<AttributePacker_MVA_T<int64_t>>() );
			break;

		case SPH_ATTR_JSON:
			m_dAttrs.Add ( std::make_unique<AttributePacker_Json_c>() );
			break;

		default:
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<BlobRowBuilder_i> sphCreateBlobRowBuilder ( const ISphSchema & tSchema, const CSphString & sFile, SphOffset_t tSpaceForUpdates, CSphString & sError )
{
	auto pBuilder = std::make_unique<BlobRowBuilder_File_c> ( tSchema, tSpaceForUpdates, false );
	if ( !pBuilder->Setup ( sFile, sError ) )
		pBuilder = nullptr;

	return pBuilder;
}


std::unique_ptr<BlobRowBuilder_i> sphCreateBlobRowJsonBuilder ( const ISphSchema & tSchema, const CSphString & sFile, SphOffset_t tSpaceForUpdates, CSphString & sError )
{
	auto pBuilder = std::make_unique<BlobRowBuilder_File_c> ( tSchema, tSpaceForUpdates, true );
	if ( !pBuilder->Setup ( sFile, sError ) )
		pBuilder = nullptr;

	return pBuilder;
}


std::unique_ptr<BlobRowBuilder_i> sphCreateBlobRowBuilder ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool )
{
	return std::make_unique<BlobRowBuilder_Mem_c> ( tSchema, dPool );
}


std::unique_ptr<BlobRowBuilder_i> sphCreateBlobRowBuilderUpdate ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool, const CSphBitvec & dAttrsUpdated )
{
	return std::make_unique<BlobRowBuilder_MemUpdate_c> ( tSchema, dPool, dAttrsUpdated );
}

//////////////////////////////////////////////////////////////////////////

static int64_t GetBlobRowOffset ( const CSphRowitem * pDocinfo, int iBlobRowOffset )
{
	return sphUnalignedRead ( *( (const int64_t*)pDocinfo + iBlobRowOffset ) );
}

static int64_t GetBlobRowOffset ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator )
{
	// blob row locator NEEDS to be 1st or 2nd attribute after docid (see tLocator.m_iBlobRowOffset)
	return GetBlobRowOffset ( tLocator.m_bDynamic ? tMatch.m_pDynamic : tMatch.m_pStatic, tLocator.m_iBlobRowOffset );
}

template <typename T>
static const BYTE * GetBlobAttr ( int iBlobAttrId, int nBlobAttrs, const BYTE * pRow, int & iLengthBytes )
{
	T uLen1 = sphUnalignedRead ( *( (const T*)pRow + iBlobAttrId ) );
	T uLen0 = iBlobAttrId > 0 ? sphUnalignedRead ( *( (const T*)pRow + iBlobAttrId - 1 ) ) : 0;
	iLengthBytes = (int)uLen1 - uLen0;
	assert ( iLengthBytes >= 0 );

	return iLengthBytes ? (const BYTE*)( (const T*)pRow + nBlobAttrs ) + uLen0 : nullptr;
}


static const BYTE * GetBlobAttr ( const BYTE * pRow, int iBlobAttrId, int nBlobAttrs, int & iLengthBytes )
{
	switch ( *pRow )
	{
	case BLOB_ROW_LEN_BYTE:		return GetBlobAttr<BYTE> ( iBlobAttrId, nBlobAttrs, pRow+1, iLengthBytes );
	case BLOB_ROW_LEN_WORD:		return GetBlobAttr<WORD> ( iBlobAttrId, nBlobAttrs, pRow+1, iLengthBytes );
	case BLOB_ROW_LEN_DWORD:	return GetBlobAttr<DWORD>( iBlobAttrId, nBlobAttrs, pRow+1, iLengthBytes );
	default:
		break;
	}

	assert ( 0 && "Unknown blob row type" );
	return nullptr;
}

// same as above, but returns pair instead of confusing result-by-ref.
template <typename T>
static ByteBlob_t GetBlobAttr ( int iBlobAttrId, int nBlobAttrs, const BYTE * pRow )
{
	auto pTRow = (const T*)pRow;
	T uLen1 = sphUnalignedRead ( pTRow[iBlobAttrId] );
	T uLen0 = (iBlobAttrId > 0) ? sphUnalignedRead ( pTRow[iBlobAttrId - 1] ) : 0;
	auto iLengthBytes = (int)uLen1-uLen0;
	assert ( iLengthBytes>=0 );

	return {iLengthBytes ? (const BYTE *)(pTRow + nBlobAttrs) + uLen0 : nullptr, iLengthBytes };
}


static ByteBlob_t GetBlobAttr ( const BYTE * pRow, int iBlobAttrId, int nBlobAttrs )
{
	switch ( *pRow )
	{
	case BLOB_ROW_LEN_BYTE:		return GetBlobAttr<BYTE> ( iBlobAttrId, nBlobAttrs, pRow+1 );
	case BLOB_ROW_LEN_WORD:		return GetBlobAttr<WORD> ( iBlobAttrId, nBlobAttrs, pRow+1 );
	case BLOB_ROW_LEN_DWORD:	return GetBlobAttr<DWORD>( iBlobAttrId, nBlobAttrs, pRow+1 );
	default:
		break;
	}

	assert ( 0 && "Unknown blob row type" );
	return { nullptr, 0 };
}


const BYTE * sphGetBlobAttr ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool, int & iLengthBytes )
{
	assert ( pBlobPool );
	int64_t iOffset = GetBlobRowOffset ( tMatch, tLocator );
	return GetBlobAttr ( pBlobPool+iOffset, tLocator.m_iBlobAttrId, tLocator.m_nBlobAttrs, iLengthBytes );
}

ByteBlob_t sphGetBlobAttr ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool )
{
	assert ( pBlobPool );
	int64_t iOffset = GetBlobRowOffset ( tMatch, tLocator );
	return GetBlobAttr ( pBlobPool+iOffset, tLocator.m_iBlobAttrId, tLocator.m_nBlobAttrs );
}

const BYTE * sphGetBlobAttr ( const CSphRowitem * pDocinfo, const CSphAttrLocator & tLocator, const BYTE * pBlobPool, int & iLengthBytes )
{
	assert ( pBlobPool );
	int64_t iOffset = GetBlobRowOffset ( pDocinfo, tLocator.m_iBlobRowOffset );
	return GetBlobAttr ( pBlobPool+iOffset, tLocator.m_iBlobAttrId, tLocator.m_nBlobAttrs, iLengthBytes );
}

ByteBlob_t sphGetBlobAttr ( const CSphRowitem * pDocinfo, const CSphAttrLocator & tLocator, const BYTE * pBlobPool )
{
	assert ( pBlobPool );
	int64_t iOffset = GetBlobRowOffset ( pDocinfo, tLocator.m_iBlobRowOffset );
	return GetBlobAttr ( pBlobPool+iOffset, tLocator.m_iBlobAttrId, tLocator.m_nBlobAttrs );
}

template <typename T>
static int GetBlobAttrLen ( int iBlobAttrId, const BYTE * pRow )
{
	assert ( pRow );

	T uLen1 = sphUnalignedRead ( *((const T*)pRow + iBlobAttrId) );
	T uLen0 = iBlobAttrId > 0 ? sphUnalignedRead ( *((const T*)pRow+iBlobAttrId-1) ) : 0;

	return (int)uLen1-uLen0;
}


int sphGetBlobAttrLen ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool )
{
	assert ( pBlobPool );

	int64_t iOffset = GetBlobRowOffset ( tMatch, tLocator );
	const BYTE * pRow = pBlobPool+iOffset;
	switch ( *pRow )
	{
	case BLOB_ROW_LEN_BYTE:		return GetBlobAttrLen<BYTE> ( tLocator.m_iBlobAttrId, pRow+1 );
	case BLOB_ROW_LEN_WORD:		return GetBlobAttrLen<WORD> ( tLocator.m_iBlobAttrId, pRow+1 );
	case BLOB_ROW_LEN_DWORD:	return GetBlobAttrLen<DWORD> ( tLocator.m_iBlobAttrId, pRow+1 );
	default:
		break;
	}

	assert ( 0 && "Unknown blob row type" );
	return 0;
}


template <typename T>
static int GetBlobTotalLen ( const BYTE * pRow, int nBlobAttrs )
{
	assert ( pRow );
	return sphUnalignedRead ( *((const T *)pRow + nBlobAttrs - 1 ) ) + nBlobAttrs*sizeof(T) + 1;
}


DWORD sphGetBlobTotalLen ( const BYTE * pRow, int nBlobAttrs )
{
	switch ( *pRow )
	{
	case BLOB_ROW_LEN_BYTE:		return GetBlobTotalLen<BYTE> ( pRow+1, nBlobAttrs );
	case BLOB_ROW_LEN_WORD:		return GetBlobTotalLen<WORD> ( pRow+1, nBlobAttrs );
	case BLOB_ROW_LEN_DWORD:	return GetBlobTotalLen<DWORD> ( pRow+1, nBlobAttrs );
	default:
		break;
	}

	assert ( 0 && "Unknown blob row type" );
	return 0;
}


int64_t sphCopyBlobRow ( CSphTightVector<BYTE> & dDstPool, const CSphTightVector<BYTE> & dSrcPool, int64_t iOffset, int nBlobs )
{
	const BYTE * pSrcBlob = dSrcPool.Begin()+iOffset;

	int iBlobLen = sphGetBlobTotalLen ( pSrcBlob, nBlobs );
	int64_t iNewOffset = dDstPool.GetLength();
	dDstPool.Append ( pSrcBlob, iBlobLen );

	return iNewOffset;
}


void sphAddAttrToBlobRow ( const CSphRowitem * pDocinfo, CSphTightVector<BYTE> & dBlobRow, const BYTE * pPool, int nBlobs, const CSphAttrLocator * pOldBlobRowLoc )
{
	dBlobRow.Resize ( 0 );
	if ( nBlobs )
	{
		assert(pOldBlobRowLoc);
		const BYTE * pOldRow = pPool + sphGetRowAttr ( pDocinfo, *pOldBlobRowLoc );
		DWORD uOldBlobLen = sphGetBlobTotalLen ( pOldRow, nBlobs );
		DWORD uLenSize = RowFlagsToLen ( *pOldRow );
		dBlobRow.Resize ( uOldBlobLen + uLenSize );
		BYTE * pNewRow = dBlobRow.Begin();
		DWORD uAttrLengthSize = uLenSize*nBlobs+1;			// old blob lengths + flags
		memcpy ( pNewRow, pOldRow, uAttrLengthSize );
		pNewRow += uAttrLengthSize;
		pOldRow += uAttrLengthSize;
		memcpy ( pNewRow, pOldRow-uLenSize, uLenSize );		// new attr length (last cumulative length)
		pNewRow += uLenSize;
		memcpy ( pNewRow, pOldRow, uOldBlobLen-uAttrLengthSize );
	}
	else
	{
		dBlobRow.Add ( CalcBlobRowFlags(0) );	// 1-byte flags
		dBlobRow.Add ( 0 );						// 1-byte length
	}
}


void sphRemoveAttrFromBlobRow ( const CSphRowitem * pDocinfo, CSphTightVector<BYTE> & dBlobRow, const BYTE * pPool, int nBlobs, int iBlobAttrId, const CSphAttrLocator & tBlobRowLoc )
{
	if ( nBlobs<=1 )
	{
		dBlobRow.Resize(0);
		return;
	}

	const BYTE * pOldRow = pPool + sphGetRowAttr ( pDocinfo, tBlobRowLoc );
	BYTE uFlags = *pOldRow;
	CSphVector<DWORD> dAttrLengths;
	for ( int i = 0; i < nBlobs; i++ )
		if ( i!=iBlobAttrId )
		{
			switch ( uFlags )
			{
			case BLOB_ROW_LEN_BYTE:
				dAttrLengths.Add ( GetBlobAttrLen<BYTE> ( i, pOldRow+1 ) );
				break;

			case BLOB_ROW_LEN_WORD:
				dAttrLengths.Add ( GetBlobAttrLen<WORD> ( i, pOldRow+1 ) );
				break;

			case BLOB_ROW_LEN_DWORD:
				dAttrLengths.Add ( GetBlobAttrLen<DWORD> ( i, pOldRow+1 ) );
				break;

			default:
				break;
			}
		}

	DWORD uTotalLength = 0;
	for ( auto i : dAttrLengths )
		uTotalLength += i;

	dBlobRow.Resize ( 1 + (nBlobs-1)*RowFlagsToLen(uFlags) + uTotalLength );
	BYTE * pNewRow = dBlobRow.Begin();

	// flags
	BYTE uNewFlags = CalcBlobRowFlags ( uTotalLength );
	*pNewRow++ = uNewFlags;

	// attribute lengths
	DWORD uCumulativeLength = 0;
	ARRAY_FOREACH ( i, dAttrLengths )
	{
		uCumulativeLength += dAttrLengths[i];

		switch ( uNewFlags )
		{
		case BLOB_ROW_LEN_BYTE:
			sphUnalignedWrite ( pNewRow, (BYTE)uCumulativeLength );
			pNewRow += sizeof(BYTE);
			break;

		case BLOB_ROW_LEN_WORD:
			sphUnalignedWrite ( pNewRow, (WORD)uCumulativeLength );
			pNewRow += sizeof(WORD);
			break;

		case BLOB_ROW_LEN_DWORD:
			sphUnalignedWrite ( pNewRow, (DWORD)uCumulativeLength );
			pNewRow += sizeof(DWORD);
			break;

		default:
			break;
		}
	}

	// attribute data
	for ( int i = 0; i < nBlobs; i++ )
		if ( i!=iBlobAttrId )
		{
			int iLengthBytes = 0;
			const BYTE * pBlob = GetBlobAttr ( pOldRow, i, nBlobs, iLengthBytes );
			memcpy ( pNewRow, pBlob, iLengthBytes );
			pNewRow += iLengthBytes;
		}
}


template<typename T>
static bool CheckMVAValues ( const T * pMVA, DWORD uLengthBytes, int iBlobAttrId, CSphString & sError )
{
	if ( uLengthBytes % sizeof(T) )
	{
		sError.SetSprintf ( "Blob row error: MVA attribute length=%u is not a multiple of %u (blob attribute %d)", uLengthBytes, DWORD(sizeof(T)), iBlobAttrId );
		return false;
	}

	int nValues = int(uLengthBytes/sizeof(T));
	for ( int i = 0; i < nValues-1; i++ )
		if ( pMVA[i]>=pMVA[i+1] )
		{
			sError.SetSprintf ( "Blob row error: descending MVA values found (blob attribute %d)", iBlobAttrId );
			return false;
		}

	return true;
}


bool sphCheckBlobRow ( int64_t iOff, DebugCheckReader_i & tBlobs, const CSphSchema & tSchema, CSphString & sError )
{
	CSphVector<ESphAttr> dBlobAttrs;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( sphIsBlobAttr(tAttr) )
			dBlobAttrs.Add ( tAttr.m_eAttrType );
	}

	int64_t iBlobsElemCount = tBlobs.GetLengthBytes();
	if ( iOff<0 || iOff>iBlobsElemCount )
	{
		sError.SetSprintf ( "Blob offset out of bounds: " INT64_FMT " (max: " INT64_FMT ")", iOff, iBlobsElemCount );
		return false;
	}

	tBlobs.SeekTo ( iOff, 16 );

	BYTE uType = 0;
	tBlobs.GetBytes ( &uType, sizeof(uType) );
	if ( uType!=BLOB_ROW_LEN_BYTE && uType!=BLOB_ROW_LEN_WORD && uType!=BLOB_ROW_LEN_DWORD )
	{
		sError.SetSprintf ( "Unknown blob row type: %u", uType );
		return false;
	}

	int nBlobAttrs = dBlobAttrs.GetLength();
	DWORD uLenSize = RowFlagsToLen ( uType );
	DWORD uAttrLengths = uLenSize*nBlobAttrs;
	if ( iOff + uAttrLengths > iBlobsElemCount )
		sError = "Blob row too long";

	CSphFixedVector<BYTE> dLengths ( uAttrLengths );
	tBlobs.GetBytes ( dLengths.Begin(), (int) dLengths.GetLengthBytes() );
	const BYTE * pLen = dLengths.Begin();

	CSphVector<int> dAttrLengths ( nBlobAttrs );

	for ( int i = 0; i < nBlobAttrs; i++ )
	{
		switch ( uType )
		{
		case BLOB_ROW_LEN_BYTE:		dAttrLengths[i] = GetBlobAttrLen<BYTE> ( i, pLen );	break;
		case BLOB_ROW_LEN_WORD:		dAttrLengths[i] = GetBlobAttrLen<WORD> ( i, pLen );	break;
		case BLOB_ROW_LEN_DWORD:	dAttrLengths[i] = GetBlobAttrLen<DWORD> ( i, pLen );	break;
		default:
			break;
		}
	}

	for ( int i = 0; i < nBlobAttrs-1; i++ )
		if ( dAttrLengths[i]<0 )
		{
			sError = "Blob row error: negative attribute length";
			return false;
		}

	DWORD uTotalLength = 0;
	for ( auto i : dAttrLengths )
		uTotalLength += (DWORD)i;

	if ( iOff+uAttrLengths+uTotalLength > iBlobsElemCount )
	{
		sError = "Blob row too long";
		return false;
	}

	CSphFixedVector<BYTE> dAttrs ( uTotalLength );
	tBlobs.GetBytes ( dAttrs.Begin(), (int) dAttrs.GetLengthBytes() );
	const BYTE * pAttr = dAttrs.Begin();
	for ( int i = 0; i < nBlobAttrs; i++ )
	{
		DWORD uLength = (DWORD)dAttrLengths[i];

		switch ( dBlobAttrs[i] )
		{
		case SPH_ATTR_UINT32SET:
			if ( !CheckMVAValues ( (const DWORD *)pAttr, uLength, i, sError ) )
				return false;
			break;

		case SPH_ATTR_INT64SET:
			if ( !CheckMVAValues ( (const int64_t *)pAttr, uLength, i, sError ) )
				return false;
			break;

		case SPH_ATTR_STRING:
			for ( DWORD j = 0; j < uLength; j++ )
				if ( !pAttr[j] )
				{
					sError.SetSprintf ( "Blob row error: string value contains zeroes (blob attribute %d)", i );
					return false;
				}
			break;

		default:
			break;
		}

		pAttr += uLength;
	}

	return true;
}


const char * sphGetBlobLocatorName()
{
	static const char * BLOB_LOCATOR_ATTR = "$_blob_locator";
	return BLOB_LOCATOR_ATTR;
}

static const CSphString g_sDocidName { "id" };

const char * sphGetDocidName()
{
	return g_sDocidName.cstr();
}


const CSphString & sphGetDocidStr()
{
	return g_sDocidName;
}


bool sphIsBlobAttr ( ESphAttr eAttr )
{
	return eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_JSON	|| eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET;
}


bool sphIsBlobAttr ( const CSphColumnInfo & tAttr )
{
	if ( tAttr.IsColumnar() )
		return false;

	return sphIsBlobAttr ( tAttr.m_eAttrType );
}


bool IsMvaAttr ( ESphAttr eAttr )
{
	return eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET || eAttr==SPH_ATTR_UINT32SET_PTR || eAttr==SPH_ATTR_INT64SET_PTR;
}


//////////////////////////////////////////////////////////////////////////
// data ptr attributes

int sphCalcPackedLength ( int iLengthBytes )
{
	return sphCalcZippedLen(iLengthBytes) + iLengthBytes;
}

BYTE * sphPackedBlob ( ByteBlob_t dBlob )
{
	if ( !dBlob.first ) return nullptr;
	return const_cast<BYTE*>(dBlob.first-sphCalcZippedLen (dBlob.second));
}


// allocate buf and pack blob tBlob into it, return pointer to buf
BYTE * sphPackPtrAttr ( ByteBlob_t dBlob )
{
	if ( !dBlob.second )
		return nullptr;

	assert ( dBlob.first );
	BYTE * pPacked = sphAllocateSmall ( sphCalcPackedLength ( dBlob.second ));
	sphPackPtrAttr ( pPacked, std::move(dBlob) );
	return pPacked;
}

// pack blob pData[iLengthBytes] into preallocated buf
int sphPackPtrAttr ( BYTE * pPrealloc, ByteBlob_t dBlob )
{
	assert ( pPrealloc && IsValid ( dBlob ) );
	int iZippedLen = ZipToPtrBE ( pPrealloc, dBlob.second );
	memcpy ( pPrealloc+iZippedLen, dBlob.first, dBlob.second );
	return iZippedLen+dBlob.second;
}

void sphPackPtrAttrInPlace ( TightPackedVec_T<BYTE> & dAttr, int iSize )
{
	BYTE bSize[20];
	if ( iSize<0 ) iSize = dAttr.GetLength();
	int iZippedLen = ZipToPtrBE ( bSize, iSize );
	dAttr.Resize ( iZippedLen+iSize );
	BYTE * pData = dAttr.Begin ();
	memmove ( pData+iZippedLen, pData, iSize );
	memcpy ( pData, bSize, iZippedLen );
}

// allocate buf for pack of iLengthBytes, pack size, then put pointer to payload in *ppData, and return buf
BYTE * sphPackPtrAttr ( int iLengthBytes, BYTE ** ppData )
{
	assert ( ppData );
	BYTE * pPacked = sphAllocateSmall ( sphCalcPackedLength ( iLengthBytes ) );
	*ppData = pPacked;
	*ppData += ZipToPtrBE ( pPacked, iLengthBytes );
	return pPacked;
}


ByteBlob_t sphUnpackPtrAttr ( const BYTE * pData )
{
	if ( !pData )
		return { nullptr, 0 };

	auto iLen = (int)UnzipIntBE ( pData );
	return { pData, iLen };
}

BYTE * sph::CopyPackedAttr ( const BYTE* pData )
{
	return sphPackPtrAttr ( sphUnpackPtrAttr ( pData ));
}

ESphAttr sphPlainAttrToPtrAttr ( ESphAttr eAttrType )
{
	switch ( eAttrType )
	{
	case SPH_ATTR_STRING:		return SPH_ATTR_STRINGPTR;
	case SPH_ATTR_JSON:			return SPH_ATTR_JSON_PTR;
	case SPH_ATTR_UINT32SET:	return SPH_ATTR_UINT32SET_PTR;
	case SPH_ATTR_INT64SET:		return SPH_ATTR_INT64SET_PTR;
	case SPH_ATTR_JSON_FIELD:	return SPH_ATTR_JSON_FIELD_PTR;
	default:					return eAttrType;
	};
}


bool sphIsDataPtrAttr ( ESphAttr eAttr )
{
	return eAttr==SPH_ATTR_STRINGPTR || eAttr==SPH_ATTR_FACTORS || eAttr==SPH_ATTR_FACTORS_JSON
	|| eAttr==SPH_ATTR_UINT32SET_PTR ||	eAttr==SPH_ATTR_INT64SET_PTR
	|| eAttr==SPH_ATTR_JSON_PTR || eAttr==SPH_ATTR_JSON_FIELD_PTR;
}

//////////////////////////////////////////////////////////////////////////
// misc attribute-related

template < typename T >
static void MVA2Str ( const T * pMVA, int iLengthBytes, StringBuilder_c &dStr )
{
	int nValues = iLengthBytes / sizeof ( T );
	dStr.GrowEnough (( SPH_MAX_NUMERIC_STR+1 ) * nValues );
	Comma_c sComma ( "," );
	for ( int i = 0; i<nValues; ++i )
	{
		dStr << sComma;
#if __has_include( <charconv>)
		dStr.SetPos ( std::to_chars ( dStr.end (), dStr.AfterEnd(), pMVA[i] ).ptr );
#else
		dStr += sph::NtoA ( dStr.end(), pMVA[i] );
#endif
	}
	*dStr.end() = '\0';
}


bool sphIsInternalAttr ( const CSphString & sAttrName )
{
	return sAttrName==sphGetBlobLocatorName();
}


bool sphIsInternalAttr ( const CSphColumnInfo & tCol )
{
	return sphIsInternalAttr ( tCol.m_sName );
}


void sphMVA2Str ( ByteBlob_t dMVA, bool b64bit, StringBuilder_c & dStr )
{
	if ( b64bit )
		MVA2Str ( ( const int64_t * ) dMVA.first, dMVA.second, dStr );
	else
		MVA2Str ( ( const DWORD * ) dMVA.first, dMVA.second, dStr );
}


void sphPackedMVA2Str ( const BYTE * pMVA, bool b64bit, StringBuilder_c & dStr )
{
	auto dMVA = sphUnpackPtrAttr ( pMVA );
	sphMVA2Str( dMVA, b64bit, dStr );
}

bool IsNotRealAttribute ( const CSphColumnInfo & tColumn )
{
	return tColumn.m_uFieldFlags & CSphColumnInfo::FIELD_STORED;
}


const char * AttrType2Str ( ESphAttr eAttrType )
{
	switch ( eAttrType )
	{
	case SPH_ATTR_NONE:				return "SPH_ATTR_NONE";
	case SPH_ATTR_INTEGER:			return "SPH_ATTR_INTEGER";
	case SPH_ATTR_TIMESTAMP:		return "SPH_ATTR_TIMESTAMP";
	case SPH_ATTR_BOOL:				return "SPH_ATTR_BOOL";
	case SPH_ATTR_FLOAT:			return "SPH_ATTR_FLOAT";
	case SPH_ATTR_BIGINT:			return "SPH_ATTR_BIGINT";
	case SPH_ATTR_STRING:			return "SPH_ATTR_STRING";
	case SPH_ATTR_POLY2D:			return "SPH_ATTR_POLY2D";
	case SPH_ATTR_STRINGPTR:		return "SPH_ATTR_STRINGPTR";
	case SPH_ATTR_TOKENCOUNT:		return "SPH_ATTR_TOKENCOUNT";
	case SPH_ATTR_JSON:				return "SPH_ATTR_JSON";
	case SPH_ATTR_UINT32SET:		return "SPH_ATTR_UINT32SET";
	case SPH_ATTR_INT64SET:			return "SPH_ATTR_INT64SET";
	case SPH_ATTR_MAPARG:			return "SPH_ATTR_MAPARG";
	case SPH_ATTR_FACTORS:			return "SPH_ATTR_FACTORS";
	case SPH_ATTR_JSON_FIELD:		return "SPH_ATTR_JSON_FIELD";
	case SPH_ATTR_FACTORS_JSON:		return "SPH_ATTR_FACTORS_JSON";
	case SPH_ATTR_UINT32SET_PTR:	return "SPH_ATTR_UINT32SET_PTR";
	case SPH_ATTR_INT64SET_PTR:		return "SPH_ATTR_INT64SET_PTR";
	case SPH_ATTR_JSON_PTR:			return "SPH_ATTR_JSON_PTR";
	case SPH_ATTR_JSON_FIELD_PTR:	return "SPH_ATTR_JSON_FIELD_PTR";
	default: return "";
	}

	return "SPH_ATTR_NONE";
}