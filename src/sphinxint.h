//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxint_
#define _sphinxint_

#include "sphinx.h"
#include "sphinxfilter.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <float.h>

//////////////////////////////////////////////////////////////////////////

const char		MAGIC_SYNONYM_WHITESPACE	= 1;				// used internally in tokenizer only
const char		MAGIC_CODE_SENTENCE			= 2;				// emitted from tokenizer on sentence boundary
const char		MAGIC_CODE_PARAGRAPH		= 3;				// emitted from stripper (and passed via tokenizer) on paragraph boundary
const char		MAGIC_CODE_ZONE				= 4;				// emitted from stripper (and passed via tokenizer) on zone boundary; followed by zero-terminated zone name

const char		MAGIC_WORD_HEAD				= 1;				// prepended to keyword by source, stored in (crc) dictionary
const char		MAGIC_WORD_TAIL				= 1;				// appended to keyword by source, stored in (crc) dictionary
const char		MAGIC_WORD_HEAD_NONSTEMMED	= 2;				// prepended to keyword by source, stored in dictionary

extern const char *		MAGIC_WORD_SENTENCE;
extern const char *		MAGIC_WORD_PARAGRAPH;

//////////////////////////////////////////////////////////////////////////

#ifdef O_BINARY
#define SPH_O_BINARY O_BINARY
#else
#define SPH_O_BINARY 0
#endif

#define SPH_O_READ	( O_RDONLY | SPH_O_BINARY )
#define SPH_O_NEW	( O_CREAT | O_RDWR | O_TRUNC | SPH_O_BINARY )

#define MVA_DOWNSIZE		DWORD			// MVA offset type
#define MVA_OFFSET_MASK		0x7fffffffUL	// MVA offset mask
#define MVA_ARENA_FLAG		0x80000000UL	// MVA global-arena flag

/// file writer with write buffering and int encoder
class CSphWriter : ISphNoncopyable
{
public:
					CSphWriter ();
	virtual			~CSphWriter ();

	void			SetBufferSize ( int iBufferSize );	///< tune write cache size; must be called before OpenFile() or SetFile()

	bool			OpenFile ( const CSphString & sName, CSphString & sErrorBuffer );
	void			SetFile ( int iFD, SphOffset_t * pSharedOffset );
	void			CloseFile ( bool bTruncate = false );	///< note: calls Flush(), ie. IsError() might get true after this call

	void			PutByte ( int uValue );
	void			PutBytes ( const void * pData, int iSize );
	void			PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void			PutOffset ( SphOffset_t uValue ) { PutBytes ( &uValue, sizeof(SphOffset_t) ); }
	void			PutString ( const char * szString );
	void			PutString ( const CSphString & sString );

	void			SeekTo ( SphOffset_t pos ); ///< seeking inside the buffer will truncate it

#if USE_64BIT
	void			PutDocid ( SphDocID_t uValue ) { PutOffset ( uValue ); }
#else
	void			PutDocid ( SphDocID_t uValue ) { PutDword ( uValue ); }
#endif

	void			ZipInt ( DWORD uValue );
	void			ZipOffset ( SphOffset_t uValue );
	void			ZipOffsets ( CSphVector<SphOffset_t> * pData );

	bool			IsError () const	{ return m_bError; }
	SphOffset_t		GetPos () const		{ return m_iPos; }

protected:
	CSphString		m_sName;
	SphOffset_t		m_iPos;
	SphOffset_t		m_iWritten;

	int				m_iFD;
	int				m_iPoolUsed;
	BYTE *			m_pBuffer;
	BYTE *			m_pPool;
	bool			m_bOwnFile;
	SphOffset_t	*	m_pSharedOffset;
	int				m_iBufferSize;

	bool			m_bError;
	CSphString *	m_pError;

	virtual void	Flush ();
};



/// file which closes automatically when going out of scope
class CSphAutofile : ISphNoncopyable
{
protected:
	int			m_iFD;			///< my file descriptior
	CSphString	m_sFilename;	///< my file name
	bool		m_bTemporary;	///< whether to unlink this file on Close()

	CSphIndex::ProgressCallback_t *		m_pProgress; ///< for displaying progress
	CSphIndexProgress *					m_pStat;

public:
					CSphAutofile ();
					CSphAutofile ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
					~CSphAutofile ();

	int				Open ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
	void			Close ();

public:
	int				GetFD () const { return m_iFD; }
	const char *	GetFilename () const;
	SphOffset_t		GetSize ( SphOffset_t iMinSize, bool bCheckSizeT, CSphString & sError );
	SphOffset_t		GetSize ();

	bool			Read ( void * pBuf, size_t uCount, CSphString & sError );
	void			SetProgressCallback ( CSphIndex::ProgressCallback_t * pfnProgress, CSphIndexProgress * pStat );
};


/// file reader with read buffering and int decoder
class CSphReader
{
public:
	CSphReader ( BYTE * pBuf=NULL, int iSize=0 );
	virtual		~CSphReader ();

	void		SetBuffers ( int iReadBuffer, int iReadUnhinted );
	void		SetFile ( int iFD, const char * sFilename );
	void		SetFile ( const CSphAutofile & tFile );
	void		Reset ();
	void		SeekTo ( SphOffset_t iPos, int iSizeHint );

	void		SkipBytes ( int iCount );
	SphOffset_t	GetPos () const { return m_iPos+m_iBuffPos; }

	void		GetBytes ( void * pData, int iSize );
	int			GetBytesZerocopy ( const BYTE ** ppData, int iMax ); ///< zerocopy method; returns actual length present in buffer (upto iMax)

	int			GetByte ();
	DWORD		GetDword ();
	SphOffset_t	GetOffset ();
	CSphString	GetString ();
	int			GetLine ( char * sBuffer, int iMaxLen );

	DWORD		UnzipInt ();
	SphOffset_t	UnzipOffset ();

	SphOffset_t				Tell () const				{ return m_iPos + m_iBuffPos; }
	bool					GetErrorFlag () const		{ return m_bError; }
	const CSphString &		GetErrorMessage () const	{ return m_sError; }
	const CSphString &		GetFilename() const			{ return m_sFilename; }

#if USE_64BIT
	SphDocID_t	GetDocid ()		{ return GetOffset(); }
	SphDocID_t	UnzipDocid ()	{ return UnzipOffset(); }
	SphWordID_t	UnzipWordid ()	{ return UnzipOffset(); }
#else
	SphDocID_t	GetDocid ()		{ return GetDword(); }
	SphDocID_t	UnzipDocid ()	{ return UnzipInt(); }
	SphWordID_t	UnzipWordid ()	{ return UnzipInt(); }
#endif

	const CSphReader &	operator = ( const CSphReader & rhs );

protected:

	int			m_iFD;
	SphOffset_t	m_iPos;

	int			m_iBuffPos;
	int			m_iBuffUsed;
	BYTE *		m_pBuff;
	int			m_iSizeHint;	///< how much do we expect to read

	int			m_iBufSize;
	bool		m_bBufOwned;
	int			m_iReadUnhinted;

	bool		m_bError;
	CSphString	m_sError;
	CSphString	m_sFilename;

private:
	void		UpdateCache ();
};

/// scoped reader
class CSphAutoreader : public CSphReader
{
public:
				CSphAutoreader ( BYTE * pBuf=NULL, int iSize=0 ) : CSphReader ( pBuf, iSize ) {}
				~CSphAutoreader ();

	bool		Open ( const CSphString & sFilename, CSphString & sError );
	void		Close ();
	SphOffset_t	GetFilesize ();

public:
	// added for DebugCheck()
	int			GetFD () { return m_iFD; }
};

//////////////////////////////////////////////////////////////////////////

/// per-query search context
/// everything that index needs to compute/create to process the query
class CSphQueryContext
{
public:
	// searching-only, per-query
	int							m_iWeights;						///< search query field weights count
	int							m_dWeights [ SPH_MAX_FIELDS ];	///< search query field weights

	bool						m_bLookupFilter;		///< row data lookup required at filtering stage
	bool						m_bLookupSort;			///< row data lookup required at sorting stage

	ISphFilter *				m_pFilter;
	ISphFilter *				m_pWeightFilter;

	struct CalcItem_t
	{
		CSphAttrLocator			m_tLoc;					///< result locator
		DWORD					m_uType;				///< result type
		ISphExpr *				m_pExpr;				///< evaluator (non-owned)
	};
	CSphVector<CalcItem_t>		m_dCalcFilter;			///< items to compute for filtering
	CSphVector<CalcItem_t>		m_dCalcSort;			///< items to compute for sorting/grouping
	CSphVector<CalcItem_t>		m_dCalcFinal;			///< items to compute when finalizing result set

	const CSphVector<CSphAttrOverride> *	m_pOverrides;		///< overridden attribute values
	CSphVector<CSphAttrLocator>				m_dOverrideIn;
	CSphVector<CSphAttrLocator>				m_dOverrideOut;

	void *						m_pIndexData;			///< backend specific data

public:
	CSphQueryContext ();
	~CSphQueryContext ();

	void						BindWeights ( const CSphQuery * pQuery, const CSphSchema & tSchema );
	bool						SetupCalc ( CSphQueryResult * pResult, const CSphSchema & tInSchema, const CSphSchema & tSchema, const DWORD * pMvaPool );
	bool						CreateFilters ( bool bFullscan, const CSphVector<CSphFilterSettings> * pdFilters, const CSphSchema & tSchema, const DWORD * pMvaPool, CSphString & sError );
	bool						SetupOverrides ( const CSphQuery * pQuery, CSphQueryResult * pResult, const CSphSchema & tIndexSchema );

	void						CalcFilter ( CSphMatch & tMatch ) const;
	void						CalcSort ( CSphMatch & tMatch ) const;
	void						CalcFinal ( CSphMatch & tMatch ) const;
};

//////////////////////////////////////////////////////////////////////////

bool sphWriteThrottled ( int iFD, const void * pBuf, int64_t iCount, const char * sName, CSphString & sError );

void SafeClose ( int & iFD );

void sphDoStatsOrder ( const struct XQNode_t * pNode, CSphQueryResultMeta & tResult );

bool sphCheckQueryHeight ( const struct XQNode_t * pRoot, CSphString & sError );

void sphTransformExtendedQuery ( XQNode_t ** ppNode );

const BYTE * SkipQuoted ( const BYTE * p );

class ISphBinlog : ISphNoncopyable
{
public:
	virtual			~ISphBinlog () {}

	virtual void	BinlogUpdateAttributes ( const char * sIndexName, int64_t iTID, const CSphAttrUpdate & tUpd ) = 0;
	virtual void	NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown ) = 0;
};

//////////////////////////////////////////////////////////////////////////

/// memory tracker
namespace Memory
{
	enum Category_e
	{
		SPH_MEM_CORE,

		SPH_MEM_IDX_DISK,
		SPH_MEM_IDX_RT,
		SPH_MEM_IDX_RT_ACCUM,

		SPH_MEM_MMAPED,

		SPH_MEM_BINLOG,

		SPH_MEM_HANDLE_NONSQL,
		SPH_MEM_HANDLE_SQL,

		SPH_MEM_SEARCH_NONSQL,
		SPH_MEM_QUERY_NONSQL,
		SPH_MEM_INSERT_SQL,
		SPH_MEM_SELECT_SQL,
		SPH_MEM_DELETE_SQL,
		SPH_MEM_COMMIT_SET_SQL,
		SPH_MEM_COMMIT_BEGIN_SQL,
		SPH_MEM_COMMIT_SQL,

		SPH_MEM_IDX_DISK_MULTY_QUERY,
		SPH_MEM_IDX_DISK_MULTY_QUERY_EX,
		SPH_MEM_IDX_RT_MULTY_QUERY,

		SPH_MEM_IDX_RT_RES_MATCHES,
		SPH_MEM_IDX_RT_RES_STRINGS,

		SPH_MEM_TOTAL
	};
}

#if SPH_ALLOCS_PROFILER

void sphMemStatPush ( Memory::Category_e eCategory );
void sphMemStatPop ( Memory::Category_e eCategory );

// memory tracker
struct MemTracker_c : ISphNoncopyable
{
	const Memory::Category_e m_eCategory; ///< category

	/// ctor
	explicit MemTracker_c ( Memory::Category_e eCategory )
		: m_eCategory ( eCategory )
	{
		sphMemStatPush ( m_eCategory );
	}

	/// dtor
	~MemTracker_c ()
	{
		sphMemStatPop ( m_eCategory );
	}
};

#define MEMORY(name) MemTracker_c tracker_##__LINE__##name(Memory::name);

#else // SPH_ALLOCS_PROFILER 0

#define MEMORY(name)

#endif // if SPH_ALLOCS_PROFILER

//////////////////////////////////////////////////////////////////////////

///
#define DOCINFO_INDEX_FREQ 128 // FIXME? make this configurable

struct CSphDocMVA
{
	SphDocID_t							m_iDocID;
	CSphVector < CSphVector<DWORD> >	m_dMVA;
	CSphVector < DWORD >				m_dOffsets;

	explicit CSphDocMVA ( int iSize )
		: m_iDocID ( 0 )
	{
		m_dMVA.Resize ( iSize );
		m_dOffsets.Resize ( iSize );
	}

	void	Read ( CSphReader & tReader );
	void	Write ( CSphWriter & tWriter );
};

/// attr min-max builder
template < typename DOCID = SphDocID_t >
class AttrIndexBuilder_t : ISphNoncopyable
{
private:
	CSphVector<CSphAttrLocator>	m_dIntAttrs;
	CSphVector<CSphAttrLocator>	m_dFloatAttrs;
	CSphVector<CSphAttrLocator>	m_dMvaAttrs;
	CSphVector<SphAttr_t>		m_dIntMin;
	CSphVector<SphAttr_t>		m_dIntMax;
	CSphVector<SphAttr_t>		m_dIntIndexMin;
	CSphVector<SphAttr_t>		m_dIntIndexMax;
	CSphVector<float>			m_dFloatMin;
	CSphVector<float>			m_dFloatMax;
	CSphVector<float>			m_dFloatIndexMin;
	CSphVector<float>			m_dFloatIndexMax;
	CSphVector<DWORD>			m_dMvaMin;
	CSphVector<DWORD>			m_dMvaMax;
	CSphVector<DWORD>			m_dMvaIndexMin;
	CSphVector<DWORD>			m_dMvaIndexMax;
	DWORD						m_uStride;		// size of attribute's chunk (in DWORDs)
	DWORD						m_uElements;	// counts total number of collected min/max pairs
	int							m_iLoop;		// loop inside one set
	DWORD *						m_pOutBuffer;	// storage for collected min/max
	DWORD *						m_pOutMax;		// storage max for bound checking
	DOCID						m_uStart;		// first and last docids of current chunk
	DOCID						m_uLast;
	DOCID						m_uIndexStart;	// first and last docids of whole index
	DOCID						m_uIndexLast;

private:
	void ResetLocal();
	void FlushComputed ( bool bUseAttrs, bool bUseMvas );
	void UpdateMinMaxDocids ( DOCID uDocID );

public:
	explicit AttrIndexBuilder_t ( const CSphSchema & tSchema );

	void Prepare ( DWORD * pOutBuffer, DWORD * pOutMax );

	void CollectWithoutMvas ( const DWORD * pCur, bool bUseMvas );
	bool Collect ( const DWORD * pCur, const CSphSharedBuffer<DWORD> & pMvas, CSphString & sError );
	void Collect ( const DWORD * pCur, const struct CSphDocMVA & dMvas );
	void CollectMVA ( DOCID uDocID, const CSphVector< CSphVector<DWORD> > & dCurInfo );

	void FinishCollect ( bool bMvaOnly = false );

	/// actually used part of output buffer, only used with index merge
	/// (we reserve space for rows from both indexes, but might kill some rows)
	inline DWORD GetActualSize() const
	{
		return 2 * m_uElements * m_uStride;
	}

	/// how many DWORDs will we need for block index
	inline DWORD GetExpectedSize ( DWORD uMaxDocs ) const
	{
		DWORD uDocinfoIndex = ( uMaxDocs + DOCINFO_INDEX_FREQ - 1 ) / DOCINFO_INDEX_FREQ;
		return 2 * ( 1 + uDocinfoIndex ) * m_uStride;
	}
};

typedef AttrIndexBuilder_t<> AttrIndexBuilder_c;

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::ResetLocal()
{
	ARRAY_FOREACH ( i, m_dIntMin )
	{
		m_dIntMin[i] = LLONG_MAX;
		m_dIntMax[i] = 0;
	}
	ARRAY_FOREACH ( i, m_dFloatMin )
	{
		m_dFloatMin[i] = FLT_MAX;
		m_dFloatMax[i] = -FLT_MAX;
	}
	ARRAY_FOREACH ( i, m_dMvaMin )
	{
		m_dMvaMin[i] = UINT_MAX;
		m_dMvaMax[i] = 0;
	}
	m_uStart = m_uLast = 0;
	m_iLoop = 0;
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::FlushComputed ( bool bUseAttrs, bool bUseMvas )
{
	assert ( m_pOutBuffer );
	DWORD * pMinEntry = m_pOutBuffer + 2 * m_uElements * m_uStride;
	DWORD * pMinAttrs = DOCINFO2ATTRS ( pMinEntry );
	DWORD * pMaxEntry = pMinEntry + m_uStride;
	DWORD * pMaxAttrs = pMinAttrs + m_uStride;

	assert ( pMaxEntry+m_uStride<=m_pOutMax );
	assert ( pMaxAttrs+m_uStride-DOCINFO_IDSIZE<=m_pOutMax );

	m_uIndexLast = m_uLast;

	DOCINFOSETID ( pMinEntry, m_uStart );
	DOCINFOSETID ( pMaxEntry, m_uLast );

	if ( bUseAttrs )
	{
		ARRAY_FOREACH ( i, m_dIntAttrs )
		{
			m_dIntIndexMin[i] = Min ( m_dIntIndexMin[i], m_dIntMin[i] );
			m_dIntIndexMax[i] = Max ( m_dIntIndexMax[i], m_dIntMax[i] );
			sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntMin[i] );
			sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntMax[i] );
		}
		ARRAY_FOREACH ( i, m_dFloatAttrs )
		{
			m_dFloatIndexMin[i] = Min ( m_dFloatIndexMin[i], m_dFloatMin[i] );
			m_dFloatIndexMax[i] = Max ( m_dFloatIndexMax[i], m_dFloatMax[i] );
			sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMin[i] ) );
			sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMax[i] ) );
		}
	}

	if ( bUseMvas )
		ARRAY_FOREACH ( i, m_dMvaAttrs )
	{
		m_dMvaIndexMin[i] = Min ( m_dMvaIndexMin[i], m_dMvaMin[i] );
		m_dMvaIndexMax[i] = Max ( m_dMvaIndexMax[i], m_dMvaMax[i] );
		sphSetRowAttr ( pMinAttrs, m_dMvaAttrs[i], m_dMvaMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dMvaAttrs[i], m_dMvaMax[i] );
	}

	m_uElements++;
	ResetLocal();
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::UpdateMinMaxDocids ( DOCID uDocID )
{
	if ( !m_uStart )
		m_uStart = uDocID;
	if ( !m_uIndexStart )
		m_uIndexStart = uDocID;
	m_uLast = uDocID;
}

template < typename DOCID >
AttrIndexBuilder_t<DOCID>::AttrIndexBuilder_t ( const CSphSchema & tSchema )
: m_uStride ( DWSIZEOF(DOCID) + tSchema.GetRowSize() )
, m_uElements ( 0 )
, m_iLoop ( 0 )
, m_pOutBuffer ( NULL )
, m_pOutMax ( NULL )
, m_uStart ( 0 )
, m_uLast ( 0 )
, m_uIndexStart ( 0 )
, m_uIndexLast ( 0 )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
		{
			m_dMvaAttrs.Add ( tCol.m_tLocator );
			continue;
		}

		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_BOOL:
		case SPH_ATTR_BIGINT:
			m_dIntAttrs.Add ( tCol.m_tLocator );
			break;

		case SPH_ATTR_FLOAT:
			m_dFloatAttrs.Add ( tCol.m_tLocator );
			break;

		default:
			break;
		}
	}

	m_dIntMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntMax.Resize ( m_dIntAttrs.GetLength() );
	m_dIntIndexMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntIndexMax.Resize ( m_dIntAttrs.GetLength() );
	m_dFloatMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatMax.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatIndexMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatIndexMax.Resize ( m_dFloatAttrs.GetLength() );
	m_dMvaMin.Resize ( m_dMvaAttrs.GetLength() );
	m_dMvaMax.Resize ( m_dMvaAttrs.GetLength() );
	m_dMvaIndexMin.Resize ( m_dMvaAttrs.GetLength() );
	m_dMvaIndexMax.Resize ( m_dMvaAttrs.GetLength() );
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::Prepare ( DWORD * pOutBuffer, DWORD * pOutMax )
{
	m_pOutBuffer = pOutBuffer;
	m_pOutMax = pOutMax;
	m_uElements = 0;
	m_uIndexStart = m_uIndexLast = 0;
	ARRAY_FOREACH ( i, m_dIntIndexMin )
	{
		m_dIntIndexMin[i] = LLONG_MAX;
		m_dIntIndexMax[i] = 0;
	}
	ARRAY_FOREACH ( i, m_dFloatIndexMin )
	{
		m_dFloatIndexMin[i] = FLT_MAX;
		m_dFloatIndexMax[i] = -FLT_MAX;
	}
	ARRAY_FOREACH ( i, m_dMvaIndexMin )
	{
		m_dMvaIndexMin[i] = UINT_MAX;
		m_dMvaIndexMax[i] = 0;
	}
	ResetLocal();
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::CollectWithoutMvas ( const DWORD * pCur, bool bUseMvas )
{
	// check if it is time to flush already collected values
	if ( m_iLoop>=DOCINFO_INDEX_FREQ )
		FlushComputed ( true, bUseMvas );

	const DWORD * pRow = DOCINFO2ATTRS_T<DOCID>(pCur);
	UpdateMinMaxDocids ( DOCINFO2ID_T<DOCID>(pCur) );
	m_iLoop++;

	// ints
	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		SphAttr_t uVal = sphGetRowAttr ( pRow, m_dIntAttrs[i] );
		m_dIntMin[i] = Min ( m_dIntMin[i], uVal );
		m_dIntMax[i] = Max ( m_dIntMax[i], uVal );
	}

	// floats
	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pRow, m_dFloatAttrs[i] ) );
		m_dFloatMin[i] = Min ( m_dFloatMin[i], fVal );
		m_dFloatMax[i] = Max ( m_dFloatMax[i], fVal );
	}
}

template < typename DOCID >
bool AttrIndexBuilder_t<DOCID>::Collect ( const DWORD * pCur, const CSphSharedBuffer<DWORD> & pMvas, CSphString & sError )
{
	CollectWithoutMvas ( pCur, true );

	const DWORD * pRow = DOCINFO2ATTRS_T<DOCID>(pCur);
	SphDocID_t uDocID = DOCINFO2ID_T<DOCID>(pCur);

	// MVAs
	ARRAY_FOREACH ( i, m_dMvaAttrs )
	{
		SphAttr_t uOff = sphGetRowAttr ( pRow, m_dMvaAttrs[i] );
		if ( !uOff )
			continue;

		// sanity checks
		if ( uOff>=pMvas.GetNumEntries() )
		{
			sError.SetSprintf ( "broken index: mva offset out of bounds, id=" DOCID_FMT, (SphDocID_t)uDocID );
			return false;
		}

		const DWORD * pMva = &pMvas [ MVA_DOWNSIZE ( uOff ) ]; // don't care about updates at this point

		if ( i==0 && DOCINFO2ID_T<DOCID> ( pMva-DWSIZEOF(DOCID) )!=uDocID )
		{
			sError.SetSprintf ( "broken index: mva docid verification failed, id=" DOCID_FMT, (SphDocID_t)uDocID );
			return false;
		}
		if ( uOff+pMva[0]>=pMvas.GetNumEntries() )
		{
			sError.SetSprintf ( "broken index: mva list out of bounds, id=" DOCID_FMT, (SphDocID_t)uDocID );
			return false;
		}

		// walk and calc
		for ( DWORD uCount = *pMva++; uCount>0; uCount--, pMva++ )
		{
			m_dMvaMin[i] = Min ( m_dMvaMin[i], *pMva );
			m_dMvaMax[i] = Max ( m_dMvaMax[i], *pMva );
		}
	}
	return true;
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::Collect ( const DWORD * pCur, const CSphDocMVA & dMvas )
{
	CollectWithoutMvas ( pCur, true );
	ARRAY_FOREACH ( i, m_dMvaAttrs )
		ARRAY_FOREACH ( j, dMvas.m_dMVA[i] )
	{
		m_dMvaMin[i] = Min ( m_dMvaMin[i], dMvas.m_dMVA[i][j] );
		m_dMvaMax[i] = Max ( m_dMvaMax[i], dMvas.m_dMVA[i][j] );
	}
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::CollectMVA ( DOCID uDocID, const CSphVector< CSphVector<DWORD> > & dCurInfo )
{
	// check if it is time to flush already collected values
	if ( m_iLoop>=DOCINFO_INDEX_FREQ )
		FlushComputed ( false, true );

	UpdateMinMaxDocids ( uDocID );
	m_iLoop++;

	ARRAY_FOREACH ( i, dCurInfo )
		ARRAY_FOREACH ( j, dCurInfo[i] )
	{
		m_dMvaMin[i] = Min ( m_dMvaMin[i], dCurInfo[i][j] );
		m_dMvaMax[i] = Max ( m_dMvaMax[i], dCurInfo[i][j] );
	}
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::FinishCollect ( bool bMvaOnly )
{
	assert ( m_pOutBuffer );
	if ( m_iLoop )
		FlushComputed ( !bMvaOnly, true );

	DWORD * pMinEntry = m_pOutBuffer + 2 * m_uElements * m_uStride;
	DWORD * pMaxEntry = pMinEntry + m_uStride;
	CSphRowitem * pMinAttrs = DOCINFO2ATTRS_T<DOCID> ( pMinEntry );
	CSphRowitem * pMaxAttrs = DOCINFO2ATTRS_T<DOCID> ( pMaxEntry );

	assert ( pMaxEntry+m_uStride<=m_pOutMax );
	assert ( pMaxAttrs+m_uStride-DWSIZEOF(DOCID)<=m_pOutMax );

	DOCINFOSETID ( pMinEntry, m_uIndexStart );
	DOCINFOSETID ( pMaxEntry, m_uIndexLast );

	ARRAY_FOREACH ( i, m_dMvaAttrs )
	{
		sphSetRowAttr ( pMinAttrs, m_dMvaAttrs[i], m_dMvaIndexMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dMvaAttrs[i], m_dMvaIndexMax[i] );
	}

	if ( !bMvaOnly )
	{
		ARRAY_FOREACH ( i, m_dIntAttrs )
		{
			sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntIndexMin[i] );
			sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntIndexMax[i] );
		}
		ARRAY_FOREACH ( i, m_dFloatAttrs )
		{
			sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMin[i] ) );
			sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMax[i] ) );
		}
		m_uElements++;

	} else
	{
		m_uElements = 0; // rewind back for collecting the rest of attributes.
	}
}


/// find a value-enclosing span in a sorted vector (aka an index at which vec[i] <= val < vec[i+1])
template < typename T >
static int FindSpan ( const CSphVector<T> & dVec, T tRef, int iSmallTreshold=8 )
{
	// empty vector
	if ( !dVec.GetLength() )
		return -1;

	// check last semi-span
	if ( dVec.Last()<tRef || dVec.Last()==tRef )
		return dVec.GetLength()-1;

	// linear search for small vectors
	if ( dVec.GetLength()<=iSmallTreshold )
	{
		for ( int i=0; i<dVec.GetLength()-1; i++ )
			if ( ( dVec[i]<tRef || dVec[i]==tRef ) && tRef<dVec[i+1] )
				return i;
		return -1;
	}

	// binary search for longer vectors
	const T * pStart = dVec.Begin();
	const T * pEnd = &dVec.Last();

	if ( ( pStart[0]<tRef || pStart[0]==tRef ) && tRef<pStart[1] )
		return 0;

	if ( ( pEnd[-1]<tRef || pEnd[-1]==tRef ) && tRef<pEnd[0] )
		return pEnd-dVec.Begin()-1;

	while ( pEnd-pStart>1 )
	{
		if ( tRef<*pStart || *pEnd<tRef )
			break;
		assert ( *pStart<tRef );
		assert ( tRef<*pEnd );

		const T * pMid = pStart + (pEnd-pStart)/2;
		assert ( pMid+1 < &dVec.Last() );

		if ( ( pMid[0]<tRef || pMid[0]==tRef ) && tRef<pMid[1] )
			return pMid - dVec.Begin();

		if ( tRef<pMid[0] )
			pEnd = pMid;
		else
			pStart = pMid;
	}

	return -1;
}

//////////////////////////////////////////////////////////////////////////

inline int FindBit ( DWORD uValue )
{
	DWORD uMask = 0xffff;
	int iIdx = 0;
	int iBits = 16;

	// we negate bits to compare with 0
	// this makes MSVC emit 'test' instead of 'cmp'
	uValue ^= 0xffffffff;
	for ( int t=0; t<5; t++ )
	{
		if ( ( uValue & uMask )==0 )
		{
			iIdx += iBits;
			uValue >>= iBits;
		}
		iBits >>= 1;
		uMask >>= iBits;
	}
	return iIdx;
}

//////////////////////////////////////////////////////////////////////////

/// decode UTF-8 codepoint
/// advances buffer ptr in all cases but end of buffer
///
/// returns -1 on failure
/// returns 0 on end of buffer
/// returns codepoint on success
inline int sphUTF8Decode ( BYTE * & pBuf )
{
	BYTE v = *pBuf;
	if ( !v )
		return 0;
	pBuf++;

	// check for 7-bit case
	if ( v<128 )
		return v;

	// get number of bytes
	int iBytes = 0;
	while ( v & 0x80 )
	{
		iBytes++;
		v <<= 1;
	}

	// check for valid number of bytes
	if ( iBytes<2 || iBytes>4 )
		return -1;

	int iCode = ( v >> iBytes );
	iBytes--;
	do
	{
		if ( !(*pBuf) )
			return 0; // unexpected eof

		if ( ((*pBuf) & 0xC0)!=0x80 )
			return -1; // invalid code

		iCode = ( iCode<<6 ) + ( (*pBuf) & 0x3F );
		iBytes--;
		pBuf++;
	} while ( iBytes );

	// all good
	return iCode;
}


/// encode UTF-8 codepoint to buffer
/// returns number of bytes used
inline int sphUTF8Encode ( BYTE * pBuf, int iCode )
{
	if ( iCode<0x80 )
	{
		pBuf[0] = (BYTE)( iCode & 0x7F );
		return 1;

	} else if ( iCode<0x800 )
	{
		pBuf[0] = (BYTE)( ( (iCode>>6) & 0x1F ) | 0xC0 );
		pBuf[1] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 2;

	} else
	{
		pBuf[0] = (BYTE)( ( (iCode>>12) & 0x0F ) | 0xE0 );
		pBuf[1] = (BYTE)( ( (iCode>>6) & 0x3F ) | 0x80 );
		pBuf[2] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 3;
	}
}


/// compute UTF-8 string length in codepoints
inline int sphUTF8Len ( const char * pStr )
{
	BYTE * pBuf = (BYTE*) pStr;
	int iRes = 0, iCode;

	while ( ( iCode = sphUTF8Decode(pBuf) )!=0 )
		if ( iCode>0 )
			iRes++;

	return iRes;
}

#endif // _sphinxint_

//
// $Id$
//
