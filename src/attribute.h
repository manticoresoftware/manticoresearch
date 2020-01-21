//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _attribute_
#define _attribute_

#include "sphinx.h"

//////////////////////////////////////////////////////////////////////////
// blob attributes

struct ThrottleState_t;
class DebugCheckReader_i;

class BlobRowBuilder_i
{
public:
	virtual				~BlobRowBuilder_i() {}

	virtual bool		SetAttr ( int iAttr, const BYTE * pData, int iDataLen, CSphString & sError ) = 0;
	virtual SphOffset_t	Flush() = 0;
	virtual SphOffset_t	Flush ( const BYTE * pOldRow ) = 0;
	virtual bool		Done ( CSphString & sError ) = 0;
};


// create file-based blob row builder
BlobRowBuilder_i *	sphCreateBlobRowBuilder ( const ISphSchema & tSchema, const CSphString & sFile, SphOffset_t tSpaceForUpdates, CSphString & sError );

// create file-based blob row builder with JSON already packed
BlobRowBuilder_i *	sphCreateBlobRowJsonBuilder ( const ISphSchema & tSchema, const CSphString & sFile, SphOffset_t tSpaceForUpdates, CSphString & sError );

// create mem-based blob row builder
BlobRowBuilder_i *	sphCreateBlobRowBuilder ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool );

// create mem-based blob row builder for updates
BlobRowBuilder_i *	sphCreateBlobRowBuilderUpdate ( const ISphSchema & tSchema, CSphTightVector<BYTE> & dPool, const CSphBitvec & dAttrsUpdated );

// get an offset into blob pool from docinfo
int64_t				sphGetBlobRowOffset ( const CSphRowitem * pDocinfo );

// set blob pool offset
void				sphSetBlobRowOffset ( CSphRowitem * pDocinfo, int64_t iOffset );

// fetches a attribute data and its length from the pool
const BYTE *		sphGetBlobAttr ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool, int & iLengthBytes );

ByteBlob_t sphGetBlobAttr ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool );

// same as above, but works with docinfo
const BYTE *		sphGetBlobAttr ( const CSphRowitem * pDocinfo, const CSphAttrLocator & tLocator, const BYTE * pBlobPool, int & iLengthBytes );

ByteBlob_t sphGetBlobAttr ( const CSphRowitem * pDocinfo, const CSphAttrLocator & tLocator, const BYTE * pBlobPool );

// returns blob attribute length
int					sphGetBlobAttrLen ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool );

// return the total length (in bytes) of a given blob row
DWORD				sphGetBlobTotalLen ( const BYTE * pBlobRow, int nBlobAttrs );

// copy whole blob row, return offset in new pool
int64_t				sphCopyBlobRow ( CSphTightVector<BYTE> & dDstPool, const CSphTightVector<BYTE> & dSrcPool, int64_t iOffset, int nBlobs );

// add a new blob attr to a row (on ALTER)
void				sphAddAttrToBlobRow ( const CSphRowitem * pDocinfo, CSphTightVector<BYTE> & dBlobRow, const BYTE * pPool, int nBlobs );

// remove a blob attr from a blob row (on ALTER)
void				sphRemoveAttrFromBlobRow ( const CSphRowitem * pDocinfo, CSphTightVector<BYTE> & dBlobRow, const BYTE * pPool, int nBlobs, int iBlobAttrId );

// verify blob row record
bool				sphCheckBlobRow ( int64_t iOff, DebugCheckReader_i & tBlobs, const CSphSchema & tSchema, CSphString & sError );

// return blob locator attribute name
const char *		sphGetBlobLocatorName();

// current docid attribute name
const char *		sphGetDocidName();
const CSphString &	sphGetDocidStr();

// returns true if this is a blob attr type
bool				sphIsBlobAttr ( ESphAttr eAttr );

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
int					sphUnpackPtrAttr ( const BYTE * pData, const BYTE ** ppUnpacked );
ByteBlob_t			sphUnpackPtrAttr ( const BYTE * pData );

// calculate packed data attr length
int					sphCalcPackedLength ( int iLengthBytes );
BYTE *				sphPackedBlob ( ByteBlob_t dBlob );


// convert plain attr type to corresponding in-memort (_PTR) attr type
ESphAttr			sphPlainAttrToPtrAttr ( ESphAttr eAttrType );

// is this a data ptr attribute?
bool				sphIsDataPtrAttr ( ESphAttr eAttrType );


//////////////////////////////////////////////////////////////////////////
// misc attribute-related

bool	sphIsInternalAttr ( const CSphString & sAttrName );
bool	sphIsInternalAttr ( const CSphColumnInfo & tCol );
void	sphMVA2Str ( const BYTE * pMVA, int iLengthBytes, bool b64bit, StringBuilder_c & dStr );
void	sphPackedMVA2Str ( const BYTE * pMVA, bool b64bit, StringBuilder_c & dStr );

inline DocID_t sphGetDocID ( const CSphRowitem * pData )
{
	assert ( pData );
	return sphUnalignedRead ( *(DocID_t*)pData );
}

#endif

