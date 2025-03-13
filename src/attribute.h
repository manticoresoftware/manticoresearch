//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _attribute_
#define _attribute_

#include "sphinxdefs.h"
#include "sphinxexpr.h"

class CSphMatch;
class ISphSchema;
class CSphSchema;
struct CSphAttrLocator;

//////////////////////////////////////////////////////////////////////////
// blob attributes

struct ThrottleState_t;
class DebugCheckReader_i;

class BlobRowBuilder_i
{
public:
	virtual				~BlobRowBuilder_i() {}

	virtual bool		SetAttr ( int iAttr, const BYTE * pData, int iDataLen, CSphString & sError ) = 0;
	virtual std::pair<SphOffset_t,SphOffset_t> Flush() = 0;
	virtual std::pair<SphOffset_t,SphOffset_t> Flush ( const BYTE * pOldRow ) = 0;
	virtual bool		Done ( CSphString & sError ) = 0;
};

struct TypedAttribute_t
{
	CSphString	m_sName;
	ESphAttr	m_eType;
};


// create file-based blob row builder
std::unique_ptr<BlobRowBuilder_i>	sphCreateBlobRowBuilder ( const ISphSchema & tSchema, const CSphString & sFile, SphOffset_t tSpaceForUpdates, int iBufferSize, CSphString & sError );

// create file-based blob row builder with JSON already packed
std::unique_ptr<BlobRowBuilder_i>	sphCreateBlobRowJsonBuilder ( const ISphSchema & tSchema, const CSphString & sFile, SphOffset_t tSpaceForUpdates, int iBufferSize, CSphString & sError );

// create mem-based blob row builder
std::unique_ptr<BlobRowBuilder_i>	sphCreateBlobRowBuilder ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool );

// create mem-based blob row builder for updates
std::unique_ptr<BlobRowBuilder_i>	sphCreateBlobRowBuilderUpdate ( const ISphSchema & tSchema, const CSphVector<TypedAttribute_t> & dAttrs, CSphTightVector<BYTE> & dPool, const CSphBitvec & dAttrsUpdated );

// fetches a attribute data and its length from the pool
const BYTE *		sphGetBlobAttr ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool, int & iLengthBytes );

ByteBlob_t			sphGetBlobAttr ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool );

// fetch attribute data from a given blob row
ByteBlob_t			sphGetBlobAttr ( const BYTE * pBlobRow, const CSphAttrLocator & tLocator );

// same as above, but works with docinfo
const BYTE *		sphGetBlobAttr ( const CSphRowitem * pDocinfo, const CSphAttrLocator & tLocator, const BYTE * pBlobPool, int & iLengthBytes );

ByteBlob_t			sphGetBlobAttr ( const CSphRowitem * pDocinfo, const CSphAttrLocator & tLocator, const BYTE * pBlobPool );

// returns blob attribute length
int					sphGetBlobAttrLen ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool );

// return the total length (in bytes) of a given blob row
DWORD				sphGetBlobTotalLen ( const BYTE * pBlobRow, int nBlobAttrs );

// copy whole blob row, return offset in new pool
int64_t				sphCopyBlobRow ( CSphTightVector<BYTE> & dDstPool, const CSphTightVector<BYTE> & dSrcPool, int64_t iOffset, int nBlobs );

// add a new blob attr to a row (on ALTER)
void				sphAddAttrToBlobRow ( const CSphRowitem * pDocinfo, CSphTightVector<BYTE> & dBlobRow, const BYTE * pPool, int nBlobs, const CSphAttrLocator * pOldBlobRowLoc );

// remove a blob attr from a blob row (on ALTER)
void				sphRemoveAttrFromBlobRow ( const CSphRowitem * pDocinfo, CSphTightVector<BYTE> & dBlobRow, const BYTE * pPool, int nBlobs, int iBlobAttrId, const CSphAttrLocator & tBlobRowLoc );

// verify blob row record
bool				sphCheckBlobRow ( int64_t iOff, DebugCheckReader_i & tBlobs, const CSphSchema & tSchema, CSphString & sError );

// return blob locator attribute name
const char *		sphGetBlobLocatorName();

// return null mask attribute name
const char *		GetNullMaskAttrName();

// current docid attribute name
const char *		sphGetDocidName();
const CSphString &	sphGetDocidStr();

// returns true if this is a blob attr type
bool				sphIsBlobAttr ( ESphAttr eAttr );

// returns true if this is a blob attr type; returns false for columnar attrs
bool				sphIsBlobAttr ( const CSphColumnInfo & tAttr );

bool				IsMvaAttr ( ESphAttr eAttr );

//////////////////////////////////////////////////////////////////////////
// data ptr attributes

// pack byteblob (length+data), return allocated storage
BYTE *				sphPackPtrAttr ( ByteBlob_t dBlob );

// pack byteblob attr to preallocated storage, return size of placed packet
int					sphPackPtrAttr ( BYTE * pPrealloc, ByteBlob_t dBlob );

// pack byteblob attr in-place (add place for size and write packed length before the blob)
void				sphPackPtrAttrInPlace ( TightPackedVec_T<BYTE>& dAttr, int iSize=-1 );

// allocate buffer, store zipped length, set pointer to free space in buffer
BYTE *				sphPackPtrAttr ( int iLengthBytes, BYTE ** pData );

// unpack data pointer attr, return length
ByteBlob_t			sphUnpackPtrAttr ( const BYTE * pData );

// calculate packed data attr length
int					sphCalcPackedLength ( int iLengthBytes );
BYTE *				sphPackedBlob ( ByteBlob_t dBlob );


// convert plain attr type to corresponding in-memort (_PTR) attr type
ESphAttr			sphPlainAttrToPtrAttr ( ESphAttr eAttrType );

// is this a data ptr attribute?
bool				sphIsDataPtrAttr ( ESphAttr eAttrType );

// just repack (matter of optimizing)
FORCE_INLINE BYTE * sphCopyPackedAttr ( const BYTE * pData ) { return sphPackPtrAttr ( sphUnpackPtrAttr ( pData ) ); }

//////////////////////////////////////////////////////////////////////////
// misc attribute-related

bool	sphIsInternalAttr ( const CSphString & sAttrName );
bool	sphIsInternalAttr ( const CSphColumnInfo & tCol );
void	sphMVA2Str ( ByteBlob_t dMVA, bool b64bit, StringBuilder_c & dStr );
void	sphPackedMVA2Str ( const BYTE * pMVA, bool b64bit, StringBuilder_c & dStr );
void	sphFloatVec2Str ( ByteBlob_t dFloatVec, StringBuilder_c & dStr );
void	sphPackedFloatVec2Str ( const BYTE * pData, StringBuilder_c & dStr );

/// check if tColumn is actually stored field (so, can't be used in filters/expressions)
bool	IsNotRealAttribute ( const CSphColumnInfo & tColumn );

FORCE_INLINE DocID_t sphGetDocID ( const CSphRowitem * pData )
{
	assert ( pData );
#if USE_LITTLE_ENDIAN
	return *(DocID_t *) ( const_cast<CSphRowitem *>(pData) );
#else
	return DocID_t ( pData[0] )+( DocID_t ( pData[1] ) << ROWITEM_BITS );
#endif
}

FORCE_INLINE void sphDeallocatePacked ( const BYTE* pBlob )
{
	if ( !pBlob )
		return;
#if WITH_SMALLALLOC
	const BYTE * pFoo = pBlob;
	sphDeallocateSmall ( pBlob, sphCalcPackedLength ( UnzipIntBE ( pFoo ) ) );
#else
	sphDeallocateSmall ( pBlob );
#endif
}

const char * AttrType2Str ( ESphAttr eAttrType );

#endif

