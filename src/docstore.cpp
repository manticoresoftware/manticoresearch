//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "docstore.h"

#include "std/lrucache.h"
#include "fileio.h"
#include "memio.h"
#include "fileutils.h"
#include "attribute.h"
#include "indexcheck.h"
#include "lz4/lz4.h"
#include "lz4/lz4hc.h"
#include "sphinxint.h"


enum BlockFlags_e : BYTE
{
	BLOCK_FLAG_COMPRESSED		= 1 << 0,
	BLOCK_FLAG_FIELD_REORDER	= 1 << 1
};

enum BlockType_e : BYTE
{
	BLOCK_TYPE_SMALL,
	BLOCK_TYPE_BIG,
	BLOCK_TYPE_TOTAL
};

enum DocFlags_e : BYTE
{
	DOC_FLAG_ALL_EMPTY		= 1 << 0,
	DOC_FLAG_EMPTY_BITMASK	= 1 << 1
};

enum FieldFlags_e : BYTE
{
	FIELD_FLAG_COMPRESSED	= 1 << 0,
	FIELD_FLAG_EMPTY		= 1 << 1
};

static const int STORAGE_VERSION = 1;

//////////////////////////////////////////////////////////////////////////

static BYTE Compression2Byte ( Compression_e eComp )
{
	switch (eComp)
	{
	case Compression_e::NONE:	return 0;
	case Compression_e::LZ4:	return 1;
	case Compression_e::LZ4HC:	return 2;
	default:
		assert ( 0 && "Unknown compression type" );
		return 0;
	}
}


static Compression_e Byte2Compression ( BYTE uComp )
{
	switch (uComp)
	{
	case 0:		return Compression_e::NONE;
	case 1:		return Compression_e::LZ4;
	case 2:		return Compression_e::LZ4HC;
	default:
		assert ( 0 && "Unknown compression type" );
		return Compression_e::NONE;
	}
}


static void PackData ( CSphVector<BYTE> & dDst, const BYTE * pData, DWORD uSize, bool bText, bool bPack )
{
	if ( bPack )
	{
		const DWORD GAP = 8;
		dDst.Resize ( uSize+GAP );
		dDst.Resize ( sphPackPtrAttr ( dDst.Begin (), {pData, uSize} ));
	}
	else
	{	
		dDst.Resize ( uSize + ( bText ? 1 : 0 ) );
		memcpy ( dDst.Begin(), pData, uSize );
		if ( bText )
		{
			dDst[uSize] = '\0';
			dDst.Resize(uSize);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
class Compressor_i
{
public:
	virtual			~Compressor_i(){}

	virtual bool	Compress ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const = 0;
	virtual bool	Decompress ( const VecTraits_T<BYTE> & dCompressed, VecTraits_T<BYTE> & dDecompressed ) const = 0;
};


class Compressor_None_c : public Compressor_i
{
public:
	bool			Compress ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const final { return false; }
	bool			Decompress ( const VecTraits_T<BYTE> & dCompressed, VecTraits_T<BYTE> & dDecompressed ) const  final { return true; }
};


class Compressor_LZ4_c : public Compressor_i
{
public:
	bool			Compress ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const override;
	bool			Decompress ( const VecTraits_T<BYTE> & dCompressed, VecTraits_T<BYTE> & dDecompressed ) const final;

protected:
	virtual int		DoCompression ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const;
};


class Compressor_LZ4HC_c : public Compressor_LZ4_c
{
public:
					Compressor_LZ4HC_c ( int iCompressionLevel );

protected:
	int				DoCompression ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const final;

private:
	int				m_iCompressionLevel = DEFAULT_COMPRESSION_LEVEL;
};


bool Compressor_LZ4_c::Compress ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const
{
	const int MIN_COMPRESSIBLE_SIZE = 64;
	if ( dUncompressed.GetLength() < MIN_COMPRESSIBLE_SIZE )
		return false;

	dCompressed.Resize ( int ( dUncompressed.GetLength()*1.5f ) );
	int iCompressedSize = DoCompression ( dUncompressed, dCompressed );

	const float WORST_COMPRESSION_RATIO = 0.95f;
	if ( iCompressedSize<0 || float(iCompressedSize)/dUncompressed.GetLength() > WORST_COMPRESSION_RATIO )
		return false;

	dCompressed.Resize(iCompressedSize);
	return true;
}


bool Compressor_LZ4_c::Decompress ( const VecTraits_T<BYTE> & dCompressed, VecTraits_T<BYTE> & dDecompressed ) const
{
	int iRes = LZ4_decompress_safe ( (const char *)dCompressed.Begin(), (char *)dDecompressed.Begin(), dCompressed.GetLength(), dDecompressed.GetLength() );
	return iRes==dDecompressed.GetLength();
}


int	Compressor_LZ4_c::DoCompression ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const
{
	return LZ4_compress_default ( (const char *)dUncompressed.Begin(), (char *)dCompressed.Begin(), dUncompressed.GetLength(), dCompressed.GetLength() );
}


Compressor_LZ4HC_c::Compressor_LZ4HC_c ( int iCompressionLevel )
	: m_iCompressionLevel ( iCompressionLevel )
{}


int	Compressor_LZ4HC_c::DoCompression ( const VecTraits_T<BYTE> & dUncompressed, CSphVector<BYTE> & dCompressed ) const
{
	return LZ4_compress_HC ( (const char *)dUncompressed.Begin(), (char *)dCompressed.Begin(), dUncompressed.GetLength(), dCompressed.GetLength(), m_iCompressionLevel );
}


std::unique_ptr<Compressor_i> CreateCompressor ( Compression_e eComp, int iCompressionLevel )
{
	switch (  eComp )
	{
		case Compression_e::LZ4:	return std::make_unique<Compressor_LZ4_c>();
		case Compression_e::LZ4HC:	return std::make_unique<Compressor_LZ4HC_c> ( iCompressionLevel );
		default:					return std::make_unique<Compressor_None_c>();
	}
}

//////////////////////////////////////////////////////////////////////////

static CSphString BuildCompoundName ( const CSphString & sName, DocstoreDataType_e eType )
{
	CSphString sCompound;
	sCompound.SetSprintf ( "%d%s", eType, sName.cstr() );
	return sCompound;
}


class DocstoreFields_c : public DocstoreFields_i
{
public:
	struct Field_t
	{
		CSphString			m_sName;
		DocstoreDataType_e	m_eType;
	};


	int				AddField ( const CSphString & sName, DocstoreDataType_e eType ) final;
	void			RemoveField ( const CSphString & sName, DocstoreDataType_e eType ) final;
	int				GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const final;

	int				GetNumFields() const final { return m_dFields.GetLength(); }
	const Field_t &	GetField ( int iField ) const { return m_dFields[iField]; }
	void			Load ( CSphReader & tReader );
	void			Save ( CSphWriter & tWriter );

private:
	CSphVector<Field_t>			m_dFields;
	SmallStringHash_T<int>		m_hFields;
};


int DocstoreFields_c::AddField ( const CSphString & sName, DocstoreDataType_e eType )
{
	int iField = m_dFields.GetLength();
	m_dFields.Add ( {sName, eType} );
	m_hFields.Add ( iField, BuildCompoundName ( sName, eType ) );
	return iField;
}


void DocstoreFields_c::RemoveField ( const CSphString & sName, DocstoreDataType_e eType )
{
	int iFieldId = GetFieldId ( sName, eType );
	if ( iFieldId==-1 )
		return;

	m_dFields.Remove ( iFieldId, 1 );

	m_hFields.Reset();
	ARRAY_FOREACH ( i, m_dFields )
		m_hFields.Add (  i, BuildCompoundName ( m_dFields[i].m_sName, m_dFields[i].m_eType ) );
}


int DocstoreFields_c::GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const
{
	int * pField = m_hFields ( BuildCompoundName ( sName, eType ) );
	return pField ? *pField : -1;
}


void DocstoreFields_c::Load ( CSphReader & tReader )
{
	assert ( !GetNumFields() );

	DWORD uNumFields = tReader.GetDword();
	for ( int i = 0; i < (int)uNumFields; i++ )
	{	
		auto eType = (DocstoreDataType_e)tReader.GetByte();
		CSphString sName = tReader.GetString();
		AddField ( sName, eType );
	}
}


void DocstoreFields_c::Save ( CSphWriter & tWriter )
{
	tWriter.PutDword ( GetNumFields() );
	for ( int i = 0, iNumFields = GetNumFields(); i < iNumFields; ++i )
	{
		tWriter.PutByte ( GetField(i).m_eType );
		tWriter.PutString ( GetField(i).m_sName );
	}
}

//////////////////////////////////////////////////////////////////////////

struct BlockData_t
{
	BYTE	m_uFlags = 0;
	DWORD	m_uNumDocs = 0;
	BYTE *	m_pData = nullptr;
	DWORD	m_uSize = 0;
};


struct HashKey_t
{
	int64_t		m_iIndexId;
	SphOffset_t	m_tOffset;

	bool operator == ( const HashKey_t & tKey ) const { return m_iIndexId==tKey.m_iIndexId && m_tOffset==tKey.m_tOffset; }
};


struct BlockUtil_t
{
	static DWORD GetHash ( const HashKey_t & tKey )
	{
		DWORD uCRC32 = sphCRC32 ( &tKey.m_iIndexId, sizeof(tKey.m_iIndexId) );
		return sphCRC32 ( &tKey.m_tOffset, sizeof(tKey.m_tOffset), uCRC32 );
	}

	static DWORD GetSize ( const BlockData_t & tValue )	{ return tValue.m_uSize; }
	static void Reset ( BlockData_t & tValue )			{ SafeDeleteArray ( tValue.m_pData ); }
};


class BlockCache_c : public LRUCache_T<HashKey_t, BlockData_t, BlockUtil_t>
{
	using BASE = LRUCache_T<HashKey_t, BlockData_t, BlockUtil_t>;
	using BASE::BASE;

public:
	void					DeleteAll ( int64_t iIndexId ) { BASE::Delete ( [iIndexId]( const HashKey_t & tKey ){ return tKey.m_iIndexId==iIndexId; } ); }

	static void				Init ( int64_t iCacheSize );
	static void				Done()	{ SafeDelete(m_pBlockCache); }
	static BlockCache_c *	Get()	{ return m_pBlockCache; }

private:
	static BlockCache_c *	m_pBlockCache;
};

BlockCache_c * BlockCache_c::m_pBlockCache = nullptr;

void BlockCache_c::Init ( int64_t iCacheSize )
{
	assert ( !m_pBlockCache );
	if ( iCacheSize > 0 )
		m_pBlockCache = new BlockCache_c(iCacheSize);
}

//////////////////////////////////////////////////////////////////////////

class DocstoreReaders_c
{
public:
						~DocstoreReaders_c();

	void				CreateReader ( int64_t iSessionId, int64_t iIndexId, const CSphAutofile & tFile, DWORD uBlockSize );
	CSphReader *		GetReader ( int64_t iSessionId, int64_t iIndexId );
	void				DeleteBySessionId ( int64_t iSessionId );
	void				DeleteByIndexId ( int64_t iIndexId );

	static void					Init();
	static void					Done();
	static DocstoreReaders_c *	Get();

private:
	struct HashKey_t
	{
		int64_t	m_iSessionId;
		int64_t	m_iIndexId;
		
		bool operator == ( const HashKey_t & tKey ) const;
		static DWORD Hash ( const HashKey_t & tKey );
	};

	int			m_iTotalReaderSize = 0;
	CSphMutex	m_tLock;
	CSphOrderedHash<CSphReader *, HashKey_t, HashKey_t, 1024> m_tHash;

	static DocstoreReaders_c * m_pReaders;

	static const int MIN_READER_CACHE_SIZE = 262144;
	static const int MAX_READER_CACHE_SIZE = 1048576;
	static const int MAX_TOTAL_READER_SIZE = 8388608;

	void		Delete ( CSphReader * pReader, const HashKey_t tKey );
};


DocstoreReaders_c * DocstoreReaders_c::m_pReaders = nullptr;


bool DocstoreReaders_c::HashKey_t::operator == ( const HashKey_t & tKey ) const
{
	return m_iSessionId==tKey.m_iSessionId && m_iIndexId==tKey.m_iIndexId;
}


DWORD DocstoreReaders_c::HashKey_t::Hash ( const HashKey_t & tKey )
{
	DWORD uCRC32 = sphCRC32 ( &tKey.m_iSessionId, sizeof(tKey.m_iSessionId) );
	return sphCRC32 ( &tKey.m_iIndexId, sizeof(tKey.m_iIndexId), uCRC32 );
}


DocstoreReaders_c::~DocstoreReaders_c()
{
	for ( auto & tDocstore : m_tHash )
		SafeDelete ( tDocstore.second );
}


void DocstoreReaders_c::CreateReader ( int64_t iSessionId, int64_t iIndexId, const CSphAutofile & tFile, DWORD uBlockSize )
{
	ScopedMutex_t tLock(m_tLock);

	if ( m_tHash ( { iSessionId, iIndexId } ) )
		return;

	int iBufferSize = (int)uBlockSize*8;
	iBufferSize = Min ( iBufferSize, MAX_READER_CACHE_SIZE );
	iBufferSize = Max ( iBufferSize, MIN_READER_CACHE_SIZE );

	if ( iBufferSize<=(int)uBlockSize )
		return;

	if ( m_iTotalReaderSize+iBufferSize > MAX_TOTAL_READER_SIZE )
		return;

	CSphReader * pReader = new CSphReader ( nullptr, iBufferSize );
	pReader->SetFile(tFile);

	Verify ( m_tHash.Add ( pReader, {iSessionId, iIndexId} ) );
	m_iTotalReaderSize += iBufferSize;
}


CSphReader * DocstoreReaders_c::GetReader ( int64_t iSessionId, int64_t iIndexId )
{
	ScopedMutex_t tLock(m_tLock);
	CSphReader ** ppReader = m_tHash ( { iSessionId, iIndexId } );
	return ppReader ? *ppReader : nullptr;
}


void DocstoreReaders_c::Delete ( CSphReader * pReader, const HashKey_t tKey )
{
	m_iTotalReaderSize -= pReader->GetBufferSize();
	assert ( m_iTotalReaderSize>=0 );

	SafeDelete(pReader);
	m_tHash.Delete(tKey);
}


void DocstoreReaders_c::DeleteBySessionId ( int64_t iSessionId )
{
	ScopedMutex_t tLock(m_tLock);

	// fixme: create a separate (faster) lookup?
	CSphVector<std::pair<CSphReader*,HashKey_t>> dToDelete;
	for ( auto & tDocstore : m_tHash )
		if ( tDocstore.first.m_iSessionId==iSessionId )
			dToDelete.Add ( { tDocstore.second, tDocstore.first } );

	for ( const auto & i : dToDelete )
		Delete ( i.first, i.second );
}


void DocstoreReaders_c::DeleteByIndexId ( int64_t iIndexId )
{
	ScopedMutex_t tLock(m_tLock);

	// fixme: create a separate (faster) lookup?
	CSphVector<std::pair<CSphReader*,HashKey_t>> dToDelete;
	for ( auto& tDocstore : m_tHash )
		if ( tDocstore.first.m_iIndexId==iIndexId )
			dToDelete.Add ( { tDocstore.second, tDocstore.first } );

	for ( const auto & i : dToDelete )
		Delete ( i.first, i.second );
}


void DocstoreReaders_c::Init ()
{
	assert(!m_pReaders);
	m_pReaders = new DocstoreReaders_c;
}


void DocstoreReaders_c::Done()
{
	SafeDelete(m_pReaders);
}


DocstoreReaders_c * DocstoreReaders_c::Get()
{
	return m_pReaders;
}

//////////////////////////////////////////////////////////////////////////

static void CreateFieldRemap ( VecTraits_T<int> & dFieldInRset, const VecTraits_T<int> * pFieldIds )
{
	if ( pFieldIds )
		ARRAY_CONSTFOREACH ( i, dFieldInRset )
		{
			int * pFound = pFieldIds->BinarySearch(i);
			dFieldInRset[i] = pFound ? pFieldIds->Idx ( pFound ) : -1;
		}
	else
		ARRAY_CONSTFOREACH ( i, dFieldInRset )
			dFieldInRset[i] = i;
}


//////////////////////////////////////////////////////////////////////////

class Docstore_c : public Docstore_i, public DocstoreSettings_t
{
	friend class DocstoreChecker_c;

public:
						Docstore_c ( int64_t iIndexId, const CSphString & sFilename );
						~Docstore_c() override;

	bool				Init ( CSphString & sError );

	int					GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const final;
	void				CreateReader ( int64_t iSessionId ) const final;
	DocstoreDoc_t		GetDoc ( RowID_t tRowID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const final;
	DocstoreSettings_t	GetDocstoreSettings() const final;

private:
	struct Block_t
	{
		SphOffset_t	m_tOffset = 0;
		DWORD		m_uSize = 0;
		DWORD		m_uHeaderSize = 0;
		RowID_t		m_tRowID = INVALID_ROWID;
		BlockType_e m_eType = BLOCK_TYPE_SMALL;
	};

	struct FieldInfo_t
	{
		BYTE	m_uFlags = 0;
		DWORD	m_uCompressedLen = 0;
		DWORD	m_uUncompressedLen = 0;
	};


	int64_t						m_iIndexId = 0;
	CSphString					m_sFilename;
	CSphAutofile				m_tFile;
	CSphFixedVector<Block_t>	m_dBlocks{0};
	std::unique_ptr<Compressor_i> m_pCompressor;
	DocstoreFields_c			m_tFields;

	const Block_t *				FindBlock ( RowID_t tRowID ) const;
	void						ReadFromFile ( BYTE * pData, int iLength, SphOffset_t tOffset, int64_t iSessionId ) const;
	DocstoreDoc_t				ReadDocFromSmallBlock ( const Block_t & tBlock, RowID_t tRowID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const;
	DocstoreDoc_t				ReadDocFromBigBlock ( const Block_t & tBlock, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const;
	BlockData_t					UncompressSmallBlock ( const Block_t & tBlock, int64_t iSessionId ) const;
	BlockData_t					UncompressBigBlockField ( SphOffset_t tOffset, const FieldInfo_t & tInfo, int64_t iSessionId ) const;

	bool						ProcessSmallBlockDoc ( RowID_t tCurDocRowID, RowID_t tRowID, const VecTraits_T<int> * pFieldIds, const CSphFixedVector<int> & dFieldInRset, bool bPack, MemoryReader2_c & tReader, CSphBitvec & tEmptyFields, DocstoreDoc_t & tResult ) const;
	void						ProcessBigBlockField ( int iField, const FieldInfo_t & tInfo, int iFieldInRset, bool bPack, int64_t iSessionId, SphOffset_t & tOffset, DocstoreDoc_t & tResult ) const;
};


Docstore_c::Docstore_c ( int64_t iIndexId, const CSphString & sFilename )
	: m_iIndexId ( iIndexId )
	, m_sFilename ( sFilename )
{}


Docstore_c::~Docstore_c ()
{
	BlockCache_c * pBlockCache = BlockCache_c::Get();
	if ( pBlockCache )
		pBlockCache->DeleteAll(m_iIndexId);

	DocstoreReaders_c * pReaders = DocstoreReaders_c::Get();
	if ( pReaders )
		pReaders->DeleteByIndexId(m_iIndexId);
}


bool Docstore_c::Init ( CSphString & sError )
{
	CSphAutoreader tReader;
	if ( !tReader.Open ( m_sFilename, sError ) )
		return false;

	DWORD uStorageVersion = tReader.GetDword();
	if ( uStorageVersion > STORAGE_VERSION )
	{
		sError.SetSprintf ( "Unable to load docstore: %s is v.%d, binary is v.%d", m_sFilename.cstr(), uStorageVersion, STORAGE_VERSION );
		return false;
	}

	m_uBlockSize = tReader.GetDword();
	m_eCompression = Byte2Compression ( tReader.GetByte() );

	m_pCompressor = CreateCompressor ( m_eCompression, m_iCompressionLevel );
	if ( !m_pCompressor )
		return false;

	m_tFields.Load(tReader);

	DWORD uNumBlocks = tReader.GetDword();
	if ( !uNumBlocks )
		return true;

	SphOffset_t tHeaderOffset = tReader.GetOffset();

	tReader.SeekTo ( tHeaderOffset, 0 );

	m_dBlocks.Reset(uNumBlocks);
	DWORD tPrevBlockRowID = 0;
	SphOffset_t tPrevBlockOffset = 0;
	for ( auto & i : m_dBlocks )
	{
		i.m_tRowID = tReader.UnzipRowid() + tPrevBlockRowID;
		i.m_eType = (BlockType_e)tReader.GetByte();
		i.m_tOffset = tReader.UnzipOffset() + tPrevBlockOffset;
		if ( i.m_eType==BLOCK_TYPE_BIG )
			i.m_uHeaderSize = tReader.UnzipInt();

		tPrevBlockRowID = i.m_tRowID;
		tPrevBlockOffset = i.m_tOffset;
	}

	for ( int i = 1; i<m_dBlocks.GetLength(); i++ )
		m_dBlocks[i-1].m_uSize = m_dBlocks[i].m_tOffset-m_dBlocks[i-1].m_tOffset;

	m_dBlocks.Last().m_uSize = tHeaderOffset-m_dBlocks.Last().m_tOffset;

	if ( tReader.GetErrorFlag() )
		return false;

	tReader.Close();

	if ( m_tFile.Open ( m_sFilename, SPH_O_READ, sError ) < 0 )
		return false;

	return true;
}


const Docstore_c::Block_t * Docstore_c::FindBlock ( RowID_t tRowID ) const
{
	const Block_t * pFound = sphBinarySearchFirst ( m_dBlocks.Begin(), m_dBlocks.End()-1, bind(&Block_t::m_tRowID), tRowID );
	assert(pFound);

	if ( pFound->m_tRowID>tRowID )
	{
		if ( pFound==m_dBlocks.Begin() )
			return nullptr;

		return pFound-1;
	}

	return pFound;
}


void Docstore_c::CreateReader ( int64_t iSessionId ) const
{
	DocstoreReaders_c * pReaders = DocstoreReaders_c::Get();
	if ( pReaders )
		pReaders->CreateReader ( iSessionId, m_iIndexId, m_tFile, m_uBlockSize );
}


int Docstore_c::GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const
{
	return m_tFields.GetFieldId (sName, eType );
}


DocstoreDoc_t Docstore_c::GetDoc ( RowID_t tRowID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const
{
#ifndef NDEBUG
	// assume that field ids are sorted
	for ( int i = 1; pFieldIds && i < pFieldIds->GetLength(); ++i )
		assert ( (*pFieldIds)[i-1] < (*pFieldIds)[i] );
#endif

	const Block_t * pBlock = FindBlock(tRowID);
	assert ( pBlock );

	if ( pBlock->m_eType==BLOCK_TYPE_SMALL )
		return ReadDocFromSmallBlock ( *pBlock, tRowID, pFieldIds, iSessionId, bPack );
	else
		return ReadDocFromBigBlock ( *pBlock, pFieldIds, iSessionId, bPack );
}


struct ScopedBlock_t
{
	int64_t		m_iIndexId = INT64_MAX;
	SphOffset_t	m_tOffset = 0;

	~ScopedBlock_t()
	{
		if ( m_iIndexId==INT64_MAX )
			return;

		BlockCache_c * pBlockCache = BlockCache_c::Get();
		assert ( pBlockCache );
		pBlockCache->Release ( { m_iIndexId, m_tOffset } );
	}
};


void Docstore_c::ReadFromFile ( BYTE * pData, int iLength, SphOffset_t tOffset, int64_t iSessionId ) const
{
	DocstoreReaders_c * pReaders = DocstoreReaders_c::Get();
	CSphReader * pReader = nullptr;
	if ( pReaders )
		pReader = pReaders->GetReader ( iSessionId, m_iIndexId );

	if ( pReader )
	{
		pReader->SeekTo ( tOffset, iLength );
		pReader->GetBytes ( pData, iLength );
	}
	else
		sphPread ( m_tFile.GetFD(), pData, iLength, tOffset );
}


BlockData_t Docstore_c::UncompressSmallBlock ( const Block_t & tBlock, int64_t iSessionId ) const
{
	BlockData_t tResult;
	CSphFixedVector<BYTE> dBlock ( tBlock.m_uSize );

	ReadFromFile ( dBlock.Begin(), dBlock.GetLength(), tBlock.m_tOffset, iSessionId );

	MemoryReader2_c tBlockReader ( dBlock.Begin(), dBlock.GetLength() );
	tResult.m_uFlags = tBlockReader.GetVal<BYTE>();
	tResult.m_uNumDocs = tBlockReader.UnzipInt();
	tResult.m_uSize = tBlockReader.UnzipInt();
	DWORD uCompressedLength = tResult.m_uSize;
	bool bCompressed = tResult.m_uFlags & BLOCK_FLAG_COMPRESSED;
	if ( bCompressed )
		uCompressedLength = tBlockReader.UnzipInt();

	const BYTE * pBody = dBlock.Begin() + tBlockReader.GetPos();

	CSphFixedVector<BYTE> dDecompressed(0);
	if ( bCompressed )
	{
		dDecompressed.Reset ( tResult.m_uSize );
		Verify ( m_pCompressor->Decompress ( VecTraits_T<const BYTE> (pBody, uCompressedLength), dDecompressed) );
		tResult.m_pData = dDecompressed.LeakData();
	}
	else
	{
		// we can't just pass tResult.m_pData because it doesn't point to the start of the allocated block
		tResult.m_pData = new BYTE[tResult.m_uSize];
		memcpy ( tResult.m_pData, pBody, tResult.m_uSize );
	}

	return tResult;
}


bool Docstore_c::ProcessSmallBlockDoc ( RowID_t tCurDocRowID, RowID_t tRowID, const VecTraits_T<int> * pFieldIds, const CSphFixedVector<int> & dFieldInRset, bool bPack, MemoryReader2_c & tReader, CSphBitvec & tEmptyFields, DocstoreDoc_t & tResult ) const
{
	bool bDocFound = tCurDocRowID==tRowID;
	if ( bDocFound )
		tResult.m_dFields.Resize ( pFieldIds ? pFieldIds->GetLength() : m_tFields.GetNumFields() );

	DWORD uBitMaskSize = tEmptyFields.GetSizeBytes();

	BYTE uDocFlags = tReader.GetVal<BYTE>();
	if ( uDocFlags & DOC_FLAG_ALL_EMPTY )
	{
		for ( auto & i : tResult.m_dFields )
			i.Resize(0);

		return bDocFound;
	}

	bool bHasBitmask = !!(uDocFlags & DOC_FLAG_EMPTY_BITMASK);
	if ( bHasBitmask )
	{
		memcpy ( tEmptyFields.Begin(), tReader.Begin()+tReader.GetPos(), uBitMaskSize );
		tReader.SetPos ( tReader.GetPos()+uBitMaskSize );
	}

	for ( int iField = 0; iField < m_tFields.GetNumFields(); iField++ )
		if ( !bHasBitmask || !tEmptyFields.BitGet(iField) )
		{
			DWORD uFieldLength = tReader.UnzipInt();
			int iFieldInRset = dFieldInRset[iField];
			if ( bDocFound && iFieldInRset!=-1 )
				PackData ( tResult.m_dFields[iFieldInRset], tReader.Begin()+tReader.GetPos(), uFieldLength, m_tFields.GetField(iField).m_eType==DOCSTORE_TEXT, bPack );

			tReader.SetPos ( tReader.GetPos()+uFieldLength );
		}

	return bDocFound;
}


DocstoreDoc_t Docstore_c::ReadDocFromSmallBlock ( const Block_t & tBlock, RowID_t tRowID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const
{
	BlockCache_c * pBlockCache = BlockCache_c::Get();

	BlockData_t tBlockData;
	bool bFromCache = pBlockCache && pBlockCache->Find ( { m_iIndexId, tBlock.m_tOffset }, tBlockData );
	if ( !bFromCache )
	{
		tBlockData = UncompressSmallBlock ( tBlock, iSessionId );
		bFromCache = pBlockCache && pBlockCache->Add ( { m_iIndexId, tBlock.m_tOffset }, tBlockData );
	}

	ScopedBlock_t tScopedBlock;
	CSphFixedVector<BYTE> tDataPtr {0};	// scoped array ptr
	if ( bFromCache )
	{
		tScopedBlock.m_iIndexId = m_iIndexId;
		tScopedBlock.m_tOffset = tBlock.m_tOffset;
	}
	else
		tDataPtr.Set ( tBlockData.m_pData, 0 );

	CSphFixedVector<int> dFieldInRset (	m_tFields.GetNumFields() );
	CreateFieldRemap ( dFieldInRset, pFieldIds );

	DocstoreDoc_t tResult;

	RowID_t tCurDocRowID = tBlock.m_tRowID;
	MemoryReader2_c tReader ( tBlockData.m_pData, tBlockData.m_uSize );
	CSphBitvec tEmptyFields ( m_tFields.GetNumFields() );
	for ( int i = 0; i < (int)tBlockData.m_uNumDocs; i++ )
	{
		if ( ProcessSmallBlockDoc ( tCurDocRowID, tRowID, pFieldIds, dFieldInRset, bPack, tReader, tEmptyFields, tResult ) )
			break;

		tCurDocRowID++;
	}

	return tResult;
}


BlockData_t Docstore_c::UncompressBigBlockField ( SphOffset_t tOffset, const FieldInfo_t & tInfo, int64_t iSessionId ) const
{
	BlockData_t tResult;
	bool bCompressed = !!( tInfo.m_uFlags & FIELD_FLAG_COMPRESSED );
	DWORD uDataLen = bCompressed ? tInfo.m_uCompressedLen : tInfo.m_uUncompressedLen;

	CSphFixedVector<BYTE> dField ( uDataLen );
	ReadFromFile ( dField.Begin(), dField.GetLength(), tOffset, iSessionId );

	tResult.m_uSize = tInfo.m_uUncompressedLen;

	CSphFixedVector<BYTE> dDecompressed(0);
	if ( bCompressed )
	{
		dDecompressed.Reset ( tResult.m_uSize );
		Verify ( m_pCompressor->Decompress ( dField, dDecompressed ) );
		tResult.m_pData = dDecompressed.LeakData();
	}
	else
		tResult.m_pData = dField.LeakData();

	return tResult;
}


void Docstore_c::ProcessBigBlockField ( int iField, const FieldInfo_t & tInfo, int iFieldInRset, bool bPack, int64_t iSessionId, SphOffset_t & tOffset, DocstoreDoc_t & tResult ) const
{
	if ( tInfo.m_uFlags & FIELD_FLAG_EMPTY )
		return;

	bool bCompressed = !!( tInfo.m_uFlags & FIELD_FLAG_COMPRESSED );
	SphOffset_t tOffsetDelta = bCompressed ? tInfo.m_uCompressedLen : tInfo.m_uUncompressedLen;
	if ( iFieldInRset==-1 )
	{
		tOffset += tOffsetDelta;
		return;
	}

	BlockCache_c * pBlockCache = BlockCache_c::Get();

	BlockData_t tBlockData;
	bool bFromCache = pBlockCache && pBlockCache->Find ( { m_iIndexId, tOffset }, tBlockData );
	if ( !bFromCache )
	{
		tBlockData = UncompressBigBlockField ( tOffset, tInfo, iSessionId );
		bFromCache = pBlockCache && pBlockCache->Add ( { m_iIndexId, tOffset }, tBlockData );
	}

	ScopedBlock_t tScopedBlock;
	CSphFixedVector<BYTE> tDataPtr {0};	// scoped array ptr
	if ( bFromCache )
	{
		tScopedBlock.m_iIndexId = m_iIndexId;
		tScopedBlock.m_tOffset = tOffset;
	}
	else
		tDataPtr.Set ( tBlockData.m_pData, 0 );

	PackData ( tResult.m_dFields[iFieldInRset], tBlockData.m_pData, tBlockData.m_uSize, m_tFields.GetField(iField).m_eType==DOCSTORE_TEXT, bPack );

	tOffset += tOffsetDelta;
}


DocstoreDoc_t Docstore_c::ReadDocFromBigBlock ( const Block_t & tBlock, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const
{
	CSphFixedVector<FieldInfo_t> dFieldInfo ( m_tFields.GetNumFields() );
	CSphFixedVector<BYTE> dBlockHeader(tBlock.m_uHeaderSize);

	ReadFromFile ( dBlockHeader.Begin(), dBlockHeader.GetLength(), tBlock.m_tOffset, iSessionId );
	MemoryReader2_c tReader ( dBlockHeader.Begin(), dBlockHeader.GetLength() );

	CSphVector<int> dFieldSort;
	BYTE uBlockFlags = tReader.GetVal<BYTE>();
	bool bNeedReorder = !!( uBlockFlags & BLOCK_FLAG_FIELD_REORDER );
	if ( bNeedReorder )
	{
		dFieldSort.Resize ( m_tFields.GetNumFields() );
		for ( auto & i : dFieldSort )
			i = tReader.UnzipInt();
	}

	for ( int i = 0; i < m_tFields.GetNumFields(); i++ )
	{
		int iField = bNeedReorder ? dFieldSort[i] : i;
		FieldInfo_t & tInfo = dFieldInfo[iField];

		tInfo.m_uFlags = tReader.GetVal<BYTE>();
		if ( tInfo.m_uFlags & FIELD_FLAG_EMPTY )
			continue;

		tInfo.m_uUncompressedLen = tReader.UnzipInt();
		if ( tInfo.m_uFlags & FIELD_FLAG_COMPRESSED )
			tInfo.m_uCompressedLen = tReader.UnzipInt();
	}

	dBlockHeader.Reset(0);

	CSphFixedVector<int> dFieldInRset ( m_tFields.GetNumFields() );
	CreateFieldRemap ( dFieldInRset, pFieldIds );

	DocstoreDoc_t tResult;
	tResult.m_dFields.Resize ( pFieldIds ? pFieldIds->GetLength() : m_tFields.GetNumFields() );

	SphOffset_t tOffset = tBlock.m_tOffset+tBlock.m_uHeaderSize;

	// i == physical field order in file
	// dFieldSort[i] == field order as in m_dFields
	// dFieldInRset[iField] == field order in result set
	for ( int i = 0; i < m_tFields.GetNumFields(); i++ )
	{
		int iField = bNeedReorder ? dFieldSort[i] : i;
		ProcessBigBlockField ( iField, dFieldInfo[iField], dFieldInRset[iField], bPack, iSessionId, tOffset, tResult );
	}

	return tResult;
}


DocstoreSettings_t Docstore_c::GetDocstoreSettings() const
{
	return *this;
}

//////////////////////////////////////////////////////////////////////////
DocstoreBuilder_i::Doc_t::Doc_t()
{}

DocstoreBuilder_i::Doc_t::Doc_t ( const DocstoreDoc_t & tDoc )
{
	m_dFields.Resize ( tDoc.m_dFields.GetLength() );
	ARRAY_FOREACH ( i, m_dFields )
		m_dFields[i] = VecTraits_T<BYTE> ( tDoc.m_dFields[i].Begin(), tDoc.m_dFields[i].GetLength() );
}

//////////////////////////////////////////////////////////////////////////

class DocstoreBuilder_c : public DocstoreBuilder_i, public DocstoreSettings_t
{
public:
			DocstoreBuilder_c ( CSphString sFilename, const DocstoreSettings_t & tSettings, int iBufferSize );

	bool	Init ( CSphString & sError );

	void	AddDoc ( RowID_t tRowID, const Doc_t & tDoc ) final;
	int		AddField ( const CSphString & sName, DocstoreDataType_e eType ) final			{ return m_tFields.AddField ( sName, eType ); }
	void	RemoveField ( const CSphString & sName, DocstoreDataType_e eType ) final		{ return m_tFields.RemoveField ( sName, eType ); }
	int		GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const final	{ return m_tFields.GetFieldId ( sName, eType ); }
	void	Finalize() final;

private:
	struct StoredDoc_t
	{
		RowID_t							m_tRowID;
		CSphVector<CSphVector<BYTE>>	m_dFields;
	};

	CSphString				m_sFilename;
	CSphVector<StoredDoc_t>	m_dStoredDocs;
	CSphVector<BYTE>		m_dHeader;
	CSphVector<BYTE>		m_dBuffer;
	std::unique_ptr<Compressor_i> m_pCompressor;
	MemoryWriter2_c			m_tHeaderWriter;
	CSphWriter				m_tWriter;
	DocstoreFields_c		m_tFields;
	int						m_iBufferSize = 0;
	DWORD					m_uStoredLen = 0;
	int						m_iNumBlocks = 0;
	SphOffset_t				m_tHeaderOffset = 0;
	SphOffset_t				m_tPrevBlockOffset = 0;
	DWORD					m_tPrevBlockRowID = 0;

	using SortedField_t = std::pair<int,int>;
	CSphVector<SortedField_t>		m_dFieldSort;
	CSphVector<CSphVector<BYTE>>	m_dCompressedBuffers;

	void	WriteInitialHeader();
	void	WriteTrailingHeader();
	void	WriteBlock();
	void	WriteSmallBlockHeader ( SphOffset_t tBlockOffset );
	void	WriteBigBlockHeader ( SphOffset_t tBlockOffset, SphOffset_t tHeaderSize );
	void	WriteSmallBlock();
	void	WriteBigBlock();
};


DocstoreBuilder_c::DocstoreBuilder_c ( CSphString sFilename, const DocstoreSettings_t & tSettings, int iBufferSize )
	: m_sFilename ( std::move (sFilename) )
	, m_tHeaderWriter ( m_dHeader )
	, m_iBufferSize ( iBufferSize )
{
	*(DocstoreSettings_t*)this = tSettings;
}


bool DocstoreBuilder_c::Init ( CSphString & sError )
{
	m_pCompressor = CreateCompressor ( m_eCompression, m_iCompressionLevel );
	if ( !m_pCompressor )
		return false;

	m_tWriter.SetBufferSize(m_iBufferSize);
	return m_tWriter.OpenFile ( m_sFilename, sError );
}


void DocstoreBuilder_c::AddDoc ( RowID_t tRowID, const Doc_t & tDoc )
{
	assert ( tDoc.m_dFields.GetLength()==m_tFields.GetNumFields() );

	DWORD uLen = 0;
	for ( const auto & i : tDoc.m_dFields )
		uLen += i.GetLength();

	if ( m_uStoredLen+uLen > m_uBlockSize )
		WriteBlock();

	StoredDoc_t & tStoredDoc = m_dStoredDocs.Add();
	tStoredDoc.m_tRowID = tRowID;
	tStoredDoc.m_dFields.Resize ( m_tFields.GetNumFields() );
	for ( int i = 0; i<m_tFields.GetNumFields(); i++ )
	{
		int iLen = tDoc.m_dFields[i].GetLength();

		// remove trailing zero
		if ( m_tFields.GetField(i).m_eType==DOCSTORE_TEXT && iLen>0 && tDoc.m_dFields[i][iLen-1]=='\0' )
			iLen--;

		tStoredDoc.m_dFields[i].Resize(iLen);
		memcpy ( tStoredDoc.m_dFields[i].Begin(), tDoc.m_dFields[i].Begin(), iLen );
	}

	m_uStoredLen += uLen;
}


void DocstoreBuilder_c::Finalize()
{
	WriteBlock();
	WriteTrailingHeader();
}


void DocstoreBuilder_c::WriteInitialHeader()
{
	m_tWriter.PutDword ( STORAGE_VERSION );
	m_tWriter.PutDword ( m_uBlockSize );
	m_tWriter.PutByte ( Compression2Byte(m_eCompression) );
	m_tFields.Save(m_tWriter);

	m_tHeaderOffset = m_tWriter.GetPos();

	// reserve space for number of blocks
	m_tWriter.PutDword(0);

	// reserve space for header offset
	m_tWriter.PutOffset(0);
}


void DocstoreBuilder_c::WriteTrailingHeader()
{
	SphOffset_t tHeaderPos = m_tWriter.GetPos();

	// write header
	m_tWriter.PutBytes ( m_dHeader.Begin(), m_dHeader.GetLength() );

	// rewind to the beginning, store num_blocks, offset to header
	m_tWriter.Flush();	// flush is necessary, see similar code in BlobRowBuilder_File_c::Done
	m_tWriter.SeekTo(m_tHeaderOffset); 
	m_tWriter.PutDword(m_iNumBlocks);
	m_tWriter.PutOffset(tHeaderPos);
	m_tWriter.CloseFile();
}


void DocstoreBuilder_c::WriteSmallBlockHeader ( SphOffset_t tBlockOffset )
{
	m_tHeaderWriter.ZipInt ( m_dStoredDocs[0].m_tRowID-m_tPrevBlockRowID );		// initial block rowid delta
	m_tHeaderWriter.PutByte ( BLOCK_TYPE_SMALL );								// block type
	m_tHeaderWriter.ZipOffset ( tBlockOffset-m_tPrevBlockOffset );				// block offset	delta

	m_tPrevBlockOffset = tBlockOffset;
	m_tPrevBlockRowID = m_dStoredDocs[0].m_tRowID;
}


void DocstoreBuilder_c::WriteBigBlockHeader ( SphOffset_t tBlockOffset, SphOffset_t tHeaderSize )
{
	m_tHeaderWriter.ZipInt ( m_dStoredDocs[0].m_tRowID-m_tPrevBlockRowID );		// initial block rowid delta
	m_tHeaderWriter.PutByte ( BLOCK_TYPE_BIG );									// block type
	m_tHeaderWriter.ZipOffset ( tBlockOffset-m_tPrevBlockOffset );				// block offset	delta
	m_tHeaderWriter.ZipInt ( tHeaderSize );										// on-disk header size

	m_tPrevBlockOffset = tBlockOffset;
	m_tPrevBlockRowID = m_dStoredDocs[0].m_tRowID;
}


void DocstoreBuilder_c::WriteSmallBlock()
{
	m_dCompressedBuffers.Resize(1);
	m_dBuffer.Resize(0);
	MemoryWriter2_c tMemWriter ( m_dBuffer );

#ifndef NDEBUG
	for ( int i=1; i < m_dStoredDocs.GetLength(); i++ )
		assert ( m_dStoredDocs[i].m_tRowID-m_dStoredDocs[i-1].m_tRowID==1 );
#endif // !NDEBUG

	CSphBitvec tEmptyFields ( m_tFields.GetNumFields() );
	for ( const auto & tDoc : m_dStoredDocs )
	{
		tEmptyFields.Clear();
		ARRAY_FOREACH ( iField, tDoc.m_dFields )
			if ( !tDoc.m_dFields[iField].GetLength() )
				tEmptyFields.BitSet(iField);

		int iEmptyFields = tEmptyFields.BitCount();
		if ( iEmptyFields==m_tFields.GetNumFields() )
			tMemWriter.PutByte ( DOC_FLAG_ALL_EMPTY );
		else
		{
			bool bNeedsBitmask = iEmptyFields && ( tEmptyFields.GetSizeBytes() < iEmptyFields );

			tMemWriter.PutByte ( bNeedsBitmask ? DOC_FLAG_EMPTY_BITMASK : 0 );
			if ( bNeedsBitmask )
				tMemWriter.PutBytes ( tEmptyFields.Begin(), tEmptyFields.GetSizeBytes() );

			ARRAY_FOREACH ( iField, tDoc.m_dFields )
				if ( !bNeedsBitmask || !tEmptyFields.BitGet(iField) )
				{
					const CSphVector<BYTE> & tField = tDoc.m_dFields[iField];
					tMemWriter.ZipInt ( tField.GetLength() );
					tMemWriter.PutBytes ( tField.Begin(), tField.GetLength() );
				}
		}
	}

	CSphVector<BYTE> & dCompressedBuffer = m_dCompressedBuffers[0];
	BYTE uBlockFlags = 0;
	bool bCompressed = m_pCompressor->Compress ( m_dBuffer, dCompressedBuffer );
	if ( bCompressed )
		uBlockFlags |= BLOCK_FLAG_COMPRESSED;

	WriteSmallBlockHeader ( m_tWriter.GetPos() );

	m_tWriter.PutByte ( uBlockFlags );									// block flags
	m_tWriter.ZipInt ( m_dStoredDocs.GetLength() );						// num docs
	m_tWriter.ZipInt ( m_dBuffer.GetLength() );							// uncompressed length

	if ( bCompressed )
		m_tWriter.ZipInt ( dCompressedBuffer.GetLength() );		// compressed length

	// body data
	if ( bCompressed )
		m_tWriter.PutBytes ( dCompressedBuffer.Begin(), dCompressedBuffer.GetLength() ); // compressed data
	else
		m_tWriter.PutBytes ( m_dBuffer.Begin(), m_dBuffer.GetLength() ); // uncompressed data
}


void DocstoreBuilder_c::WriteBigBlock()
{
	assert ( m_dStoredDocs.GetLength()==1 );
	StoredDoc_t & tDoc = m_dStoredDocs[0];

	m_dCompressedBuffers.Resize ( m_tFields.GetNumFields() );

	bool bNeedReorder = false;
	CSphBitvec tCompressedFields ( m_tFields.GetNumFields() );
	int iPrevSize = 0;
	ARRAY_FOREACH ( iField, tDoc.m_dFields )
	{
		const CSphVector<BYTE> & dField = tDoc.m_dFields[iField];
		CSphVector<BYTE> & dCompressedBuffer = m_dCompressedBuffers[iField];
		bool bCompressed = m_pCompressor->Compress ( dField, dCompressedBuffer );
		if ( bCompressed )
			tCompressedFields.BitSet(iField);

		int iStoredSize = bCompressed ? dCompressedBuffer.GetLength() : dField.GetLength();
		bNeedReorder |= iStoredSize < iPrevSize;
		iPrevSize = dCompressedBuffer.GetLength();
	}

	if ( bNeedReorder )
	{
		m_dFieldSort.Resize ( m_tFields.GetNumFields() );
		ARRAY_FOREACH ( iField, tDoc.m_dFields )
		{
			m_dFieldSort[iField].first = iField;
			m_dFieldSort[iField].second = tCompressedFields.BitGet(iField) ? m_dCompressedBuffers[iField].GetLength() : tDoc.m_dFields[iField].GetLength();
		}

		m_dFieldSort.Sort ( ::bind(&SortedField_t::second) );
	}

	SphOffset_t tOnDiskHeaderStart = m_tWriter.GetPos();
	BYTE uBlockFlags = bNeedReorder ? BLOCK_FLAG_FIELD_REORDER : 0;
	m_tWriter.PutByte(uBlockFlags);											// block flags

	if ( bNeedReorder )
	{
		for ( const auto & i : m_dFieldSort )
			m_tWriter.ZipInt(i.first);										// field reorder map
	}

	for ( int i = 0; i < m_tFields.GetNumFields(); i++ )
	{
		int iField = bNeedReorder ? m_dFieldSort[i].first : i;
		bool bCompressed = tCompressedFields.BitGet(iField);
		bool bEmpty = !tDoc.m_dFields[iField].GetLength();

		BYTE uFieldFlags = 0;
		uFieldFlags |= bCompressed ? FIELD_FLAG_COMPRESSED : 0;
		uFieldFlags |= bEmpty ? FIELD_FLAG_EMPTY : 0;
		m_tWriter.PutByte(uFieldFlags);										// field flags

		if ( bEmpty )
			continue;

		m_tWriter.ZipInt ( tDoc.m_dFields[iField].GetLength() );			// uncompressed len
		if ( bCompressed )
			m_tWriter.ZipInt ( m_dCompressedBuffers[iField].GetLength() );	// compressed len (if compressed)
	}

	SphOffset_t tOnDiskHeaderSize = m_tWriter.GetPos() - tOnDiskHeaderStart;

	for ( int i = 0; i < m_tFields.GetNumFields(); i++ )
	{
		int iField = bNeedReorder ? m_dFieldSort[i].first : i;
		bool bCompressed = tCompressedFields.BitGet(iField);
		bool bEmpty = !tDoc.m_dFields[iField].GetLength();

		if ( bEmpty )
			continue;

		if ( bCompressed )
			m_tWriter.PutBytes ( m_dCompressedBuffers[iField].Begin(), m_dCompressedBuffers[iField].GetLength() );	// compressed data
		else
			m_tWriter.PutBytes( tDoc.m_dFields[iField].Begin(), tDoc.m_dFields[iField].GetLength() );				// uncompressed data
	}

	WriteBigBlockHeader ( tOnDiskHeaderStart, tOnDiskHeaderSize );
}


void DocstoreBuilder_c::WriteBlock()
{
	if ( !m_tWriter.GetPos() )
		WriteInitialHeader();

	if ( !m_dStoredDocs.GetLength() )
		return;

	bool bBigBlock = m_dStoredDocs.GetLength()==1 && m_uStoredLen>=m_uBlockSize;

	if ( bBigBlock )
		WriteBigBlock();
	else
		WriteSmallBlock();

	m_iNumBlocks++;
	m_uStoredLen = 0;
	m_dStoredDocs.Resize(0);
}

//////////////////////////////////////////////////////////////////////////

class DocstoreRT_c : public DocstoreRT_i
{
public:
						~DocstoreRT_c() override;

	void				AddDoc ( RowID_t tRowID, const DocstoreBuilder_i::Doc_t & tDoc ) final;
	int					AddField ( const CSphString & sName, DocstoreDataType_e eType ) final;
	void				RemoveField ( const CSphString & sName, DocstoreDataType_e eType ) final;
	void				Finalize() final {}

	void				SwapRows ( RowID_t tDstID, RowID_t tSrcID ) final;
	void				DropTail ( RowID_t tTailID ) final;

	DocstoreDoc_t		GetDoc ( RowID_t tRowID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const final;
	int					GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const final;
	DocstoreSettings_t	GetDocstoreSettings() const final;
	void				CreateReader ( int64_t iSessionId ) const final {}

	bool				Load ( CSphReader & tReader ) final;
	void				Save ( Writer_i & tWriter ) final;
	void				Load ( MemoryReader_c & tReader ) final;
	void				Save ( MemoryWriter_c & tWriter ) final;

	void				AddPackedDoc ( RowID_t tRowID, const DocstoreRT_i * pSrcDocstore, RowID_t tSrcRowID ) final;

	int64_t				AllocatedBytes() const final;

	static int			GetDocSize ( const BYTE * pDoc, int iFieldCount );
	bool				CheckFieldsLoaded ( CSphString & sError ) const final;

private:
	CSphVector<BYTE *>	m_dDocs;
	int					m_iLoadedFieldCount = 0;
	DocstoreFields_c	m_tFields;
	int64_t				m_iAllocated = 0;
};


DocstoreRT_c::~DocstoreRT_c()
{
	for ( auto & i : m_dDocs )
		SafeDeleteArray(i);
}


void DocstoreRT_c::AddDoc ( RowID_t tRowID, const DocstoreBuilder_i::Doc_t & tDoc )
{
	assert ( (RowID_t)(m_dDocs.GetLength())==tRowID );

	CSphFixedVector<int> tFieldLengths(tDoc.m_dFields.GetLength());

	int iPackedLen = 0;
	ARRAY_FOREACH ( i, tDoc.m_dFields )
	{
		int iLen = tDoc.m_dFields[i].GetLength();

		// remove trailing zero
		if ( m_tFields.GetField(i).m_eType==DOCSTORE_TEXT && iLen>0 && tDoc.m_dFields[i][iLen-1]=='\0' )
			iLen--;

		iPackedLen += sphCalcZippedLen(iLen)+iLen;
		tFieldLengths[i] = iLen;
	}

	BYTE * & pPacked = m_dDocs.Add();
	pPacked = new BYTE[iPackedLen];
	BYTE * pPtr = pPacked;

	ARRAY_FOREACH ( i, tDoc.m_dFields )
		pPtr += sphPackPtrAttr ( pPtr, {tDoc.m_dFields[i].Begin (), tFieldLengths[i]} );

	m_iAllocated += iPackedLen;

	assert ( pPtr-pPacked==iPackedLen );
}


void DocstoreRT_c::SwapRows ( RowID_t tDstID, RowID_t tSrcID )
{
	assert ( tDstID!=INVALID_ROWID );
	assert ( tSrcID!=INVALID_ROWID );
	::Swap ( m_dDocs[tDstID], m_dDocs[tSrcID]);
}


void DocstoreRT_c::DropTail ( RowID_t tTailID )
{
	int iFieldsCount = m_tFields.GetNumFields ();
	for ( auto i = tTailID, iLen = (RowID_t) m_dDocs.GetLength (); i<iLen; ++i )
		if ( m_dDocs[i])
		{
			m_iAllocated -= GetDocSize ( m_dDocs[i], iFieldsCount );
			SafeDeleteArray( m_dDocs[i] );
		}
	m_dDocs.Resize ( tTailID );
}


int DocstoreRT_c::AddField ( const CSphString & sName, DocstoreDataType_e eType )
{
	return m_tFields.AddField ( sName, eType );
}


void DocstoreRT_c::RemoveField ( const CSphString & sName, DocstoreDataType_e eType )
{
	return m_tFields.RemoveField ( sName, eType );
}


DocstoreDoc_t DocstoreRT_c::GetDoc ( RowID_t tRowID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const
{
#ifndef NDEBUG
	// assume that field ids are sorted
	for ( int i = 1; pFieldIds && i < pFieldIds->GetLength(); i++ )
		assert ( (*pFieldIds)[i-1] < (*pFieldIds)[i] );
#endif

	CSphFixedVector<int> dFieldInRset (	m_tFields.GetNumFields() );
	CreateFieldRemap ( dFieldInRset, pFieldIds );

	DocstoreDoc_t tResult;
	tResult.m_dFields.Resize ( pFieldIds ? pFieldIds->GetLength() : m_tFields.GetNumFields() );

	const BYTE * pDoc = m_dDocs[tRowID];
	for ( int iField = 0; iField < m_tFields.GetNumFields(); iField++ )
	{
		DWORD uFieldLength = UnzipIntBE(pDoc);
		int iFieldInRset = dFieldInRset[iField];
		if ( iFieldInRset!=-1 )
			PackData ( tResult.m_dFields[iFieldInRset], pDoc, uFieldLength, m_tFields.GetField(iField).m_eType==DOCSTORE_TEXT, bPack );

		pDoc += uFieldLength;
	}

	return tResult;
}


int DocstoreRT_c::GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const
{
	return m_tFields.GetFieldId ( sName, eType );
}


DocstoreSettings_t DocstoreRT_c::GetDocstoreSettings() const
{
	assert ( 0 && "No settings for RT docstore" );
	return DocstoreSettings_t();
}


int DocstoreRT_c::GetDocSize ( const BYTE * pDoc, int iFieldCount )
{
	const BYTE * p = pDoc;
	for ( int iField = 0; iField<iFieldCount; iField++ )
		p += UnzipIntBE(p);

	return p-pDoc;
}

template<typename T>
int64_t DocstoreLoad_T ( CSphVector<BYTE *> & dDocs, T & tReader )
{
	int64_t iAllocated = 0;
	DWORD uNumDocs = tReader.UnzipInt();
	dDocs.Resize (uNumDocs);
	for ( auto & i : dDocs )
	{
		DWORD uDocLen = tReader.UnzipInt();
		i = new BYTE[uDocLen];
		tReader.GetBytes ( i, uDocLen );
		iAllocated += uDocLen;
	}

	return iAllocated;
}


template<typename T>
void DocstoreSave_T ( const CSphVector<BYTE *> & dDocs, int iFieldCount , T & tWriter )
{
	tWriter.ZipInt ( dDocs.GetLength() );
	for ( const auto & i : dDocs )
	{
		int iDocLen = DocstoreRT_c::GetDocSize ( i, iFieldCount );
		tWriter.ZipInt ( iDocLen );
		tWriter.PutBytes ( i, iDocLen );
	}
}

bool DocstoreRT_c::Load ( CSphReader & tReader )
{
	assert ( !m_dDocs.GetLength() && !m_iAllocated );
	m_iAllocated += DocstoreLoad_T<CSphReader> ( m_dDocs, tReader );
	return !tReader.GetErrorFlag();
}

void DocstoreRT_c::Save ( Writer_i & tWriter )
{
	DocstoreSave_T<Writer_i> ( m_dDocs, m_tFields.GetNumFields(), tWriter );
}

void DocstoreRT_c::Load ( MemoryReader_c & tReader )
{
	assert ( !m_dDocs.GetLength() && !m_iAllocated );
	m_iLoadedFieldCount = tReader.GetDword();
	m_iAllocated += DocstoreLoad_T<MemoryReader_c> ( m_dDocs, tReader );
}

void DocstoreRT_c::Save ( MemoryWriter_c & tWriter )
{
	int iFieldCount = m_tFields.GetNumFields();
	tWriter.PutDword ( iFieldCount );
	DocstoreSave_T<MemoryWriter_c> ( m_dDocs, iFieldCount, tWriter );
}

bool DocstoreRT_c::CheckFieldsLoaded ( CSphString & sError ) const
{
	if ( !m_iLoadedFieldCount )
		return true;

	int iFieldsCount = m_tFields.GetNumFields();
	if ( m_iLoadedFieldCount!=iFieldsCount )
	{
		sError.SetSprintf ( "wrong fields count, loaded %d, stored %d", m_iLoadedFieldCount, iFieldsCount );
		return false;
	}

	return true;
}


void DocstoreRT_c::AddPackedDoc ( RowID_t tRowID, const DocstoreRT_i * pSrcDocstore, RowID_t tSrcRowID )
{
	const DocstoreRT_c * pSrc = (const DocstoreRT_c *)pSrcDocstore;
	int iFieldsCount = m_tFields.GetNumFields();
	assert ( iFieldsCount==pSrc->m_tFields.GetNumFields() );

	// get raw doc and its length
	const BYTE * pSrcPacked = pSrc->m_dDocs[tSrcRowID];
	const int iSrcPackedLen = pSrc->GetDocSize ( pSrcPacked, iFieldsCount );
	
	// copy doc into new place
	BYTE * pDst = new BYTE[iSrcPackedLen];
	memcpy ( pDst, pSrcPacked, iSrcPackedLen );

	assert ( (RowID_t)(m_dDocs.GetLength())==tRowID );
	m_dDocs.Add ( pDst );
	m_iAllocated += GetDocSize ( pDst, iFieldsCount );
}

int64_t DocstoreRT_c::AllocatedBytes() const
{
	return m_iAllocated + m_dDocs.AllocatedBytes();
}

//////////////////////////////////////////////////////////////////////////

std::atomic<int64_t> DocstoreSession_c::m_tUIDGenerator { 0 };

DocstoreSession_c::DocstoreSession_c()
	: m_iUID ( m_tUIDGenerator.fetch_add ( 1, std::memory_order_relaxed ) )
{}


DocstoreSession_c::~DocstoreSession_c()
{
	DocstoreReaders_c * pReaders = DocstoreReaders_c::Get();
	if ( pReaders )
		pReaders->DeleteBySessionId(m_iUID);
}

//////////////////////////////////////////////////////////////////////////

class DocstoreChecker_c
{
public:
						DocstoreChecker_c ( CSphAutoreader & tReader, DebugCheckError_i & tReporter, int64_t iRowsCount );

	bool				Check();

private:
	CSphAutoreader &	m_tReader;
	DebugCheckError_i &	m_tReporter;
	const char *		m_szFilename = nullptr;
	DocstoreFields_c	m_tFields;
	std::unique_ptr<Compressor_i> m_pCompressor;
	int64_t				m_iRowsCount = 0;

	void				CheckSmallBlockDoc ( MemoryReader2_c & tReader, CSphBitvec & tEmptyFields, SphOffset_t tOffset );
	void				CheckSmallBlock ( const Docstore_c::Block_t & tBlock );
	void				CheckBlock ( const Docstore_c::Block_t & tBlock );
	void				CheckBigBlockField ( const Docstore_c::FieldInfo_t & tInfo, SphOffset_t & tOffset );
	void				CheckBigBlock ( const Docstore_c::Block_t & tBlock );
};


DocstoreChecker_c::DocstoreChecker_c ( CSphAutoreader & tReader, DebugCheckError_i & tReporter, int64_t iRowsCount )
	: m_tReader ( tReader )
	, m_tReporter ( tReporter )
	, m_szFilename ( tReader.GetFilename().cstr() )
	, m_iRowsCount ( iRowsCount )
{}


bool DocstoreChecker_c::Check()
{
	DWORD uStorageVersion = m_tReader.GetDword();
	if ( uStorageVersion > STORAGE_VERSION )
		return m_tReporter.Fail ( "Unable to load docstore: %s is v.%d, binary is v.%d", m_szFilename, uStorageVersion, STORAGE_VERSION );

	m_tReader.GetDword();	// block size
	BYTE uCompression = m_tReader.GetByte();
	if ( uCompression > 2 )
		return m_tReporter.Fail ( "Unknown docstore compression %u in %s", uCompression, m_szFilename );

	Compression_e eCompression = Byte2Compression(uCompression);
	m_pCompressor = CreateCompressor ( eCompression, DEFAULT_COMPRESSION_LEVEL );
	if ( !m_pCompressor )
		return m_tReporter.Fail ( "Unable to create compressor in %s", m_szFilename );

	DWORD uNumFields = m_tReader.GetDword();
	const DWORD MAX_SANE_FIELDS = 32768;
	if ( uNumFields > MAX_SANE_FIELDS )
		return m_tReporter.Fail ( "Too many docstore fields (%u) in %s", uNumFields, m_szFilename );

	for ( int i = 0; i < (int)uNumFields; i++ )
	{
		BYTE uDataType = m_tReader.GetByte();
		if ( uDataType > DOCSTORE_TOTAL )
			return m_tReporter.Fail ( "Unknown docstore data type (%u) in %s", uDataType, m_szFilename );

		DocstoreDataType_e eType = (DocstoreDataType_e)uDataType;
		CSphString sName = m_tReader.GetString();
		const int MAX_SANE_FIELD_NAME_LEN = 32768;
		if ( sName.Length() > MAX_SANE_FIELD_NAME_LEN )
			return m_tReporter.Fail ( "Docstore field name too long (%d) in %s", sName.Length(), m_szFilename );

		m_tFields.AddField ( sName, eType );
	}

	DWORD uNumBlocks = m_tReader.GetDword();
	// docstore from empty index
	if ( !uNumBlocks )
	{
		if ( !m_iRowsCount )
			return true;

		return m_tReporter.Fail ( "Docstore has 0 blocks but " INT64_FMT " documents in %s", m_iRowsCount, m_szFilename );
	}

	SphOffset_t tHeaderOffset = m_tReader.GetOffset();
	if ( tHeaderOffset <= 0 || tHeaderOffset >= m_tReader.GetFilesize() )
		return m_tReporter.Fail ( "Wrong docstore header offset (" INT64_FMT ") in %s", tHeaderOffset, m_szFilename );

	m_tReader.SeekTo ( tHeaderOffset, 0 );

	CSphFixedVector<Docstore_c::Block_t> dBlocks(uNumBlocks);

	DWORD tPrevBlockRowID = 0;
	SphOffset_t tPrevBlockOffset = 0;
	for ( auto & i : dBlocks )
	{
		RowID_t uUnzipped = m_tReader.UnzipRowid();
		if ( (int64_t)uUnzipped + tPrevBlockRowID >= (int64_t)0xFFFFFFFF )
			m_tReporter.Fail ( "Docstore rowid overflow in %s", m_szFilename );

		i.m_tRowID = uUnzipped + tPrevBlockRowID;
		BYTE uBlockType = m_tReader.GetByte();
		if ( uBlockType>BLOCK_TYPE_TOTAL )
			return m_tReporter.Fail ( "Unknown docstore block type (%u) in %s", uBlockType, m_szFilename );

		i.m_eType = (BlockType_e)uBlockType;
		i.m_tOffset = m_tReader.UnzipOffset() + tPrevBlockOffset;
		if ( i.m_tOffset <= 0 || i.m_tOffset >= m_tReader.GetFilesize() )
			return m_tReporter.Fail ( "Wrong docstore block offset (" INT64_FMT ") in %s", i.m_tOffset, m_szFilename );

		if ( i.m_eType==BLOCK_TYPE_BIG )
			i.m_uHeaderSize = m_tReader.UnzipInt();

		tPrevBlockRowID = i.m_tRowID;
		tPrevBlockOffset = i.m_tOffset;
	}

	for ( int i = 1; i<dBlocks.GetLength(); i++ )
	{
		if ( dBlocks[i-1].m_tOffset>=dBlocks[i].m_tOffset )
			return m_tReporter.Fail ( "Descending docstore block offset in %s", m_szFilename );

		dBlocks[i-1].m_uSize = dBlocks[i].m_tOffset-dBlocks[i-1].m_tOffset;
	}

	if ( dBlocks.GetLength() )
		dBlocks.Last().m_uSize = tHeaderOffset-dBlocks.Last().m_tOffset;

	for ( auto & i : dBlocks )
	{
		if ( i.m_tOffset+i.m_uSize > m_tReader.GetFilesize() )
			return m_tReporter.Fail ( "Docstore block size+offset out of bounds in %s", m_szFilename );

		CheckBlock(i);
	}

	if ( m_tReader.GetErrorFlag() )
		return m_tReporter.Fail ( "%s", m_tReader.GetErrorMessage().cstr() );

	return true;
}


void DocstoreChecker_c::CheckSmallBlockDoc ( MemoryReader2_c & tReader, CSphBitvec & tEmptyFields, SphOffset_t tOffset )
{
	BYTE uDocFlags = tReader.GetVal<BYTE>();

	if ( uDocFlags & ( ~(DOC_FLAG_ALL_EMPTY | DOC_FLAG_EMPTY_BITMASK) ) )
		m_tReporter.Fail ( "Unknown docstore doc flag (%u) in %s (offset " INT64_FMT ")", uDocFlags, m_szFilename, tOffset );

	if ( uDocFlags & DOC_FLAG_ALL_EMPTY )
		return;

	DWORD uBitMaskSize = tEmptyFields.GetSizeBytes();

	bool bHasBitmask = !!(uDocFlags & DOC_FLAG_EMPTY_BITMASK);
	if ( bHasBitmask )
	{
		memcpy ( tEmptyFields.Begin(), tReader.Begin()+tReader.GetPos(), uBitMaskSize );
		tReader.SetPos ( tReader.GetPos()+uBitMaskSize );
	}

	for ( int iField = 0; iField < m_tFields.GetNumFields(); iField++ )
		if ( !bHasBitmask || !tEmptyFields.BitGet(iField) )
		{
			DWORD uFieldLength = tReader.UnzipInt();
			tReader.SetPos ( tReader.GetPos()+uFieldLength );
			if ( tReader.GetPos() > tReader.GetLength() )
				m_tReporter.Fail ( "Out of bounds in docstore field data in %s (offset " INT64_FMT ")", m_szFilename, tOffset );
		}
}


void DocstoreChecker_c::CheckSmallBlock ( const Docstore_c::Block_t & tBlock )
{
	CSphFixedVector<BYTE> dBlock ( tBlock.m_uSize );

	m_tReader.SeekTo ( tBlock.m_tOffset, 0 );
	m_tReader.GetBytes ( dBlock.Begin(), dBlock.GetLength() );

	MemoryReader2_c tBlockReader ( dBlock.Begin(), dBlock.GetLength() );
	BlockData_t tResult;
	tResult.m_uFlags = tBlockReader.GetVal<BYTE>();
	tResult.m_uNumDocs = tBlockReader.UnzipInt();
	tResult.m_uSize = tBlockReader.UnzipInt();
	DWORD uCompressedLength = tResult.m_uSize;
	bool bCompressed = tResult.m_uFlags & BLOCK_FLAG_COMPRESSED;
	if ( bCompressed )
		uCompressedLength = tBlockReader.UnzipInt();

	if ( tResult.m_uFlags!=0 && tResult.m_uFlags!=BLOCK_FLAG_COMPRESSED )
		m_tReporter.Fail ( "Unknown docstore small block flag (%u) in %s (offset " INT64_FMT ")", tResult.m_uFlags, m_szFilename, tBlock.m_tOffset );

	if ( uCompressedLength>tResult.m_uSize )
		m_tReporter.Fail ( "Docstore block size mismatch: compressed=%u, uncompressed=%u in %s (offset " INT64_FMT ")", uCompressedLength, tResult.m_uSize, m_szFilename, tBlock.m_tOffset );

	if ( !tResult.m_uNumDocs )
		m_tReporter.Fail ( "Docstore block invalid document count: %d", tResult.m_uNumDocs );

	const BYTE * pBody = dBlock.Begin() + tBlockReader.GetPos();

	CSphFixedVector<BYTE> dDecompressed(0);
	if ( bCompressed )
	{
		dDecompressed.Reset ( tResult.m_uSize );
		if ( !m_pCompressor->Decompress ( VecTraits_T<const BYTE> (pBody, uCompressedLength), dDecompressed) )
			m_tReporter.Fail ( "Error decompressing small block in %s (offset " INT64_FMT ")", m_szFilename, tBlock.m_tOffset );

		tResult.m_pData = dDecompressed.LeakData();
	}
	else
	{
		// we can't just pass tResult.m_pData because it doesn't point to the start of the allocated block
		tResult.m_pData = new BYTE[tResult.m_uSize];
		memcpy ( tResult.m_pData, pBody, tResult.m_uSize );
	}

	MemoryReader2_c tReader ( tResult.m_pData, tResult.m_uSize );
	CSphBitvec tEmptyFields ( m_tFields.GetNumFields() );
	for ( int i = 0; i < (int)tResult.m_uNumDocs; i++ )
		CheckSmallBlockDoc ( tReader, tEmptyFields, tBlock.m_tOffset );

	SafeDelete ( tResult.m_pData );
}


void DocstoreChecker_c::CheckBigBlockField ( const Docstore_c::FieldInfo_t & tInfo, SphOffset_t & tOffset )
{
	if ( tInfo.m_uFlags & FIELD_FLAG_EMPTY )
		return;

	bool bCompressed = !!( tInfo.m_uFlags & FIELD_FLAG_COMPRESSED );
	SphOffset_t tOffsetDelta = bCompressed ? tInfo.m_uCompressedLen : tInfo.m_uUncompressedLen;
	BlockData_t tBlockData;

	CSphFixedVector<BYTE> dField ( tOffsetDelta );
	m_tReader.SeekTo ( tOffset, 0 );
	m_tReader.GetBytes ( dField.Begin(), dField.GetLength() );

	tBlockData.m_uSize = tInfo.m_uUncompressedLen;

	if ( bCompressed )
	{
		CSphFixedVector<BYTE> dDecompressed(0);
		dDecompressed.Reset ( tBlockData.m_uSize );
		if ( !m_pCompressor->Decompress ( dField, dDecompressed ) )
			m_tReporter.Fail ( "Error decompressing big block in %s (offset " INT64_FMT ")", m_szFilename, tOffset );
	}

	tOffset += tOffsetDelta;

	if ( tOffset > m_tReader.GetFilesize() )
		m_tReporter.Fail ( "Docstore block size+offset out of bounds in %s (offset " INT64_FMT ")", m_szFilename, tOffset );
}


void DocstoreChecker_c::CheckBigBlock ( const Docstore_c::Block_t & tBlock )
{
	CSphFixedVector<Docstore_c::FieldInfo_t> dFieldInfo ( m_tFields.GetNumFields() );

	CSphFixedVector<BYTE> dBlockHeader(tBlock.m_uHeaderSize);
	CSphFixedVector<BYTE> dBlock ( tBlock.m_uSize );

	m_tReader.SeekTo ( tBlock.m_tOffset, 0 );
	m_tReader.GetBytes ( dBlockHeader.Begin(), dBlockHeader.GetLength() );

	MemoryReader2_c tReader ( dBlockHeader.Begin(), dBlockHeader.GetLength() );

	CSphVector<int> dFieldSort;
	BYTE uBlockFlags = tReader.GetVal<BYTE>();
	if ( uBlockFlags & ~BLOCK_FLAG_FIELD_REORDER )
		m_tReporter.Fail ( "Unknown docstore big block flag (%u) in %s (offset " INT64_FMT ")", uBlockFlags, m_szFilename, tBlock.m_tOffset );

	bool bNeedReorder = !!( uBlockFlags & BLOCK_FLAG_FIELD_REORDER );
	if ( bNeedReorder )
	{
		dFieldSort.Resize ( m_tFields.GetNumFields() );
		for ( auto & i : dFieldSort )
		{
			i = tReader.UnzipInt();
			if ( i<0 || i>m_tFields.GetNumFields() )
				m_tReporter.Fail ( "Error in docstore field remap (%d) in %s (offset " INT64_FMT ")", i, m_szFilename, tBlock.m_tOffset );
		}
	}

	for ( int i = 0; i < m_tFields.GetNumFields(); i++ )
	{
		int iField = bNeedReorder ? dFieldSort[i] : i;
		Docstore_c::FieldInfo_t & tInfo = dFieldInfo[iField];

		tInfo.m_uFlags = tReader.GetVal<BYTE>();
		if ( tInfo.m_uFlags & (~(FIELD_FLAG_EMPTY | FIELD_FLAG_COMPRESSED) ) )
			m_tReporter.Fail ( "Unknown docstore big block field flag (%u) in %s (offset " INT64_FMT ")", tInfo.m_uFlags, m_szFilename, tBlock.m_tOffset );

		if ( tInfo.m_uFlags & FIELD_FLAG_EMPTY )
			continue;

		tInfo.m_uUncompressedLen = tReader.UnzipInt();
		if ( tInfo.m_uFlags & FIELD_FLAG_COMPRESSED )
			tInfo.m_uCompressedLen = tReader.UnzipInt();

		if ( tInfo.m_uCompressedLen>tInfo.m_uUncompressedLen )
			m_tReporter.Fail ( "Docstore block size mismatch: compressed=%u, uncompressed=%u in %s (offset " INT64_FMT ")", tInfo.m_uCompressedLen, tInfo.m_uUncompressedLen, m_szFilename, tBlock.m_tOffset );

		if ( tReader.GetPos() > tReader.GetLength() )
			m_tReporter.Fail ( "Out of bounds in docstore field data in %s (offset " INT64_FMT ")", m_szFilename, tBlock.m_tOffset );
	}

	SphOffset_t tOffset = tBlock.m_tOffset+tBlock.m_uHeaderSize;

	for ( int i = 0; i < m_tFields.GetNumFields(); i++ )
		CheckBigBlockField ( dFieldInfo[bNeedReorder ? dFieldSort[i] : i], tOffset );
}


void DocstoreChecker_c::CheckBlock ( const Docstore_c::Block_t & tBlock )
{
	if ( tBlock.m_eType==BLOCK_TYPE_SMALL )
		CheckSmallBlock(tBlock);
	else
		CheckBigBlock(tBlock);
}

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<Docstore_i> CreateDocstore ( int64_t iIndexId, const CSphString & sFilename, CSphString & sError )
{
	auto pDocstore = std::make_unique<Docstore_c>( iIndexId, sFilename );
	if ( !pDocstore->Init(sError) )
		return nullptr;

	return pDocstore;
}


std::unique_ptr<DocstoreBuilder_i> CreateDocstoreBuilder ( const CSphString & sFilename, const DocstoreSettings_t & tSettings, int iBufferSize, CSphString & sError )
{
	auto pBuilder = std::make_unique<DocstoreBuilder_c>( sFilename, tSettings, iBufferSize );
	if ( !pBuilder->Init(sError) )
		return nullptr;

	return pBuilder;
}


std::unique_ptr<DocstoreRT_i> CreateDocstoreRT()
{
	return std::make_unique<DocstoreRT_c>();
}


std::unique_ptr<DocstoreFields_i> CreateDocstoreFields()
{
	return std::make_unique<DocstoreFields_c>();
}


void InitDocstore ( int64_t iCacheSize )
{
	BlockCache_c::Init(iCacheSize);
	DocstoreReaders_c::Init();
}


void ShutdownDocstore()
{
	BlockCache_c::Done();
	DocstoreReaders_c::Done();
}


bool CheckDocstore ( CSphAutoreader & tReader, DebugCheckError_i & tReporter, int64_t iRowsCount )
{
	DocstoreChecker_c tChecker ( tReader, tReporter, iRowsCount );
	return tChecker.Check();
}
