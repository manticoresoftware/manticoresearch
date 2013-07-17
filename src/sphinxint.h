//
// $Id$
//

//
// Copyright (c) 2001-2013, Andrew Aksyonoff
// Copyright (c) 2008-2013, Sphinx Technologies Inc
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
#include "sphinxrt.h"
#include "sphinxquery.h"
#include "sphinxexcerpt.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <float.h>

//////////////////////////////////////////////////////////////////////////
// INTERNAL CONSTANTS
//////////////////////////////////////////////////////////////////////////

#ifdef O_BINARY
#define SPH_O_BINARY O_BINARY
#else
#define SPH_O_BINARY 0
#endif

#define SPH_O_READ	( O_RDONLY | SPH_O_BINARY )
#define SPH_O_NEW	( O_CREAT | O_RDWR | O_TRUNC | SPH_O_BINARY )

#define MVA_DOWNSIZE		DWORD			// MVA32 offset type
#define MVA_OFFSET_MASK		0x7fffffffUL	// MVA offset mask
#define MVA_ARENA_FLAG		0x80000000UL	// MVA global-arena flag

//////////////////////////////////////////////////////////////////////////

const DWORD		INDEX_MAGIC_HEADER			= 0x58485053;		///< my magic 'SPHX' header
const DWORD		INDEX_FORMAT_VERSION		= 40;				///< my format version

const char		MAGIC_SYNONYM_WHITESPACE	= 1;				// used internally in tokenizer only
const char		MAGIC_CODE_SENTENCE			= 2;				// emitted from tokenizer on sentence boundary
const char		MAGIC_CODE_PARAGRAPH		= 3;				// emitted from stripper (and passed via tokenizer) on paragraph boundary
const char		MAGIC_CODE_ZONE				= 4;				// emitted from stripper (and passed via tokenizer) on zone boundary; followed by zero-terminated zone name

const char		MAGIC_WORD_HEAD				= 1;				// prepended to keyword by source, stored in (crc) dictionary
const char		MAGIC_WORD_TAIL				= 1;				// appended to keyword by source, stored in (crc) dictionary
const char		MAGIC_WORD_HEAD_NONSTEMMED	= 2;				// prepended to keyword by source, stored in dictionary
const char		MAGIC_WORD_BIGRAM			= 3;				// used as a bigram (keyword pair) separator, stored in dictionary

extern const char *		MAGIC_WORD_SENTENCE;	///< value is "\3sentence"
extern const char *		MAGIC_WORD_PARAGRAPH;	///< value is "\3paragraph"

//////////////////////////////////////////////////////////////////////////
// INTERNAL GLOBALS
//////////////////////////////////////////////////////////////////////////

/// binlog, defined in sphinxrt.cpp
extern class ISphBinlog *		g_pBinlog;

/// costs for max_predicted_time limits, defined in sphinxsearch.cpp
/// measured in nanoseconds (that is, 1e-9)
extern int g_iPredictorCostSkip;
extern int g_iPredictorCostDoc;
extern int g_iPredictorCostHit;
extern int g_iPredictorCostMatch;

extern bool g_bJsonStrict;
extern bool g_bJsonAutoconvNumbers;
extern bool g_bJsonKeynamesToLowercase;

//////////////////////////////////////////////////////////////////////////
// INTERNAL HELPER FUNCTIONS, CLASSES, ETC
//////////////////////////////////////////////////////////////////////////

#define SPH_QUERY_STATES \
	SPH_QUERY_STATE ( UNKNOWN,		"unknown" ) \
	SPH_QUERY_STATE ( NET_READ,		"net_read" ) \
	SPH_QUERY_STATE ( IO,			"io" ) \
	SPH_QUERY_STATE ( DIST_CONNECT,	"dist_connect" ) \
	SPH_QUERY_STATE ( LOCAL_DF,		"local_df" ) \
	SPH_QUERY_STATE ( LOCAL_SEARCH,	"local_search" ) \
	SPH_QUERY_STATE ( SQL_PARSE,	"sql_parse" ) \
	SPH_QUERY_STATE ( DICT_SETUP,	"dict_setup" ) \
	SPH_QUERY_STATE ( PARSE,		"parse" ) \
	SPH_QUERY_STATE ( TRANSFORMS,	"transforms" ) \
	SPH_QUERY_STATE ( INIT,			"init" ) \
	SPH_QUERY_STATE ( OPEN,			"open" ) \
	SPH_QUERY_STATE ( READ_DOCS,	"read_docs" ) \
	SPH_QUERY_STATE ( READ_HITS,	"read_hits" ) \
	SPH_QUERY_STATE ( GET_DOCS,		"get_docs" ) \
	SPH_QUERY_STATE ( GET_HITS,		"get_hits" ) \
	SPH_QUERY_STATE ( FILTER,		"filter" ) \
	SPH_QUERY_STATE ( RANK,			"rank" ) \
	SPH_QUERY_STATE ( SORT,			"sort" ) \
	SPH_QUERY_STATE ( FINALIZE,		"finalize" ) \
	SPH_QUERY_STATE ( DIST_WAIT,	"dist_wait" ) \
	SPH_QUERY_STATE ( AGGREGATE,	"aggregate" ) \
	SPH_QUERY_STATE ( NET_WRITE,	"net_write" ) \
	SPH_QUERY_STATE ( EVAL_POST,	"eval_post" ) \
	SPH_QUERY_STATE ( SNIPPET,		"eval_snippet" ) \
	SPH_QUERY_STATE ( EVAL_UDF,		"eval_udf" ) \
	SPH_QUERY_STATE ( TABLE_FUNC,	"table_func" )


/// possible query states, used for profiling
enum ESphQueryState
{
	SPH_QSTATE_INFINUM = -1,

#define SPH_QUERY_STATE(_name,_desc) SPH_QSTATE_##_name,
	SPH_QUERY_STATES
#undef SPH_QUERY_STATE

	SPH_QSTATE_TOTAL
};
STATIC_ASSERT ( SPH_QSTATE_UNKNOWN==0, BAD_QUERY_STATE_ENUM_BASE );


/// search query profile
class CSphQueryProfile
{
public:
	ESphQueryState	m_eState;							///< current state
	int64_t			m_tmStamp;							///< timestamp when we entered the current state

	int				m_dSwitches [ SPH_QSTATE_TOTAL+1 ];	///< number of switches to given state
	int64_t			m_tmTotal [ SPH_QSTATE_TOTAL+1 ];	///< total time spent per state

	CSphStringBuilder	m_sTransformedTree;					///< transformed query tree

public:
	/// create empty and stopped profile
	CSphQueryProfile()
	{
		Start ( SPH_QSTATE_TOTAL );
	}

	/// switch to a new query state, and record a timestamp
	/// returns previous state, to simplify Push/Pop like scenarios
	ESphQueryState Switch ( ESphQueryState eNew )
	{
		int64_t tmNow = sphMicroTimer();
		ESphQueryState eOld = m_eState;
		m_dSwitches [ eOld ]++;
		m_tmTotal [ eOld ] += tmNow - m_tmStamp;
		m_eState = eNew;
		m_tmStamp = tmNow;
		return eOld;
	}

	/// reset everything and start profiling from a given state
	void Start ( ESphQueryState eNew )
	{
		memset ( m_dSwitches, 0, sizeof(m_dSwitches) );
		memset ( m_tmTotal, 0, sizeof(m_tmTotal) );
		m_eState = eNew;
		m_tmStamp = sphMicroTimer();
	}

	/// stop profiling
	void Stop()
	{
		Switch ( SPH_QSTATE_TOTAL );
	}
};


/// file writer with write buffering and int encoder
class CSphWriter : ISphNoncopyable
{
public:
					CSphWriter ();
	virtual			~CSphWriter ();

	void			SetBufferSize ( int iBufferSize );	///< tune write cache size; must be called before OpenFile() or SetFile()

	bool			OpenFile ( const CSphString & sName, CSphString & sError );
	void			SetFile ( CSphAutofile & tAuto, SphOffset_t * pSharedOffset, CSphString & sError );
	void			CloseFile ( bool bTruncate = false );	///< note: calls Flush(), ie. IsError() might get true after this call
	void			UnlinkFile (); /// some shit happened (outside) and the file is no more actual.

	void			PutByte ( int uValue );
	void			PutBytes ( const void * pData, int64_t iSize );
	void			PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void			PutOffset ( SphOffset_t uValue ) { PutBytes ( &uValue, sizeof(SphOffset_t) ); }
	void			PutString ( const char * szString );
	void			PutString ( const CSphString & sString );
	void			Tag ( const char * sTag );

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
	void			SetThrottle ( ThrottleState_t * pState ) { m_pThrottle = pState; }

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
	ThrottleState_t * m_pThrottle;

	virtual void	Flush ();
};


/// file which closes automatically when going out of scope
class CSphAutofile : ISphNoncopyable
{
protected:
	int			m_iFD;			///< my file descriptor
	CSphString	m_sFilename;	///< my file name
	bool		m_bTemporary;	///< whether to unlink this file on Close()
	bool		m_bWouldTemporary; ///< backup of the m_bTemporary

	CSphIndexProgress *					m_pStat;

public:
					CSphAutofile ();
					CSphAutofile ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
					~CSphAutofile ();

	int				Open ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
	void			Close ();
	void			SetTemporary(); ///< would be set if a shit happened and the file is not actual.

public:
	int				GetFD () const { return m_iFD; }
	const char *	GetFilename () const;
	SphOffset_t		GetSize ( SphOffset_t iMinSize, bool bCheckSizeT, CSphString & sError );
	SphOffset_t		GetSize ();

	bool			Read ( void * pBuf, int64_t iCount, CSphString & sError );
	void			SetProgressCallback ( CSphIndexProgress * pStat );
};


/// file reader with read buffering and int decoder
class CSphReader
{
public:
	CSphQueryProfile *	m_pProfile;
	ESphQueryState		m_eProfileState;

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
	bool		Tag ( const char * sTag );

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
	void		SetThrottle ( ThrottleState_t * pState ) { m_pThrottle = pState; }

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
	ThrottleState_t * m_pThrottle;

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

/// generic COM-like uids
enum ExtraData_e
{
	EXTRA_GET_DATA_ZONESPANS,
	EXTRA_GET_DATA_ZONESPANLIST,
	EXTRA_GET_DATA_RANKFACTORS,
	EXTRA_GET_DATA_PACKEDFACTORS,
	EXTRA_GET_DATA_RANKER_STATE,
	EXTRA_SET_MVAPOOL,
	EXTRA_SET_STRINGPOOL,
	EXTRA_SET_MAXMATCHES,
	EXTRA_SET_MATCHPUSHED,
	EXTRA_SET_MATCHPOPPED
};

/// generic COM-like interface
class ISphExtra
{
public:
	virtual						~ISphExtra () {}
	inline bool					ExtraData	( ExtraData_e eType, void** ppData )
	{
		return ExtraDataImpl ( eType, ppData );
	}
private:
	virtual bool ExtraDataImpl ( ExtraData_e, void** )
	{
		return false;
	}
};


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

	bool						m_bPackedFactors;		///< whether we need to store packed factors for our query

	ISphFilter *				m_pFilter;
	ISphFilter *				m_pWeightFilter;

	struct CalcItem_t
	{
		CSphAttrLocator			m_tLoc;					///< result locator
		ESphAttr				m_eType;				///< result type
		ISphExpr *				m_pExpr;				///< evaluator (non-owned)
	};
	CSphVector<CalcItem_t>		m_dCalcFilter;			///< items to compute for filtering
	CSphVector<CalcItem_t>		m_dCalcSort;			///< items to compute for sorting/grouping
	CSphVector<CalcItem_t>		m_dCalcFinal;			///< items to compute when finalizing result set

	const CSphVector<CSphAttrOverride> *	m_pOverrides;		///< overridden attribute values
	CSphVector<CSphAttrLocator>				m_dOverrideIn;
	CSphVector<CSphAttrLocator>				m_dOverrideOut;

	void *						m_pIndexData;			///< backend specific data
	CSphQueryProfile *			m_pProfile;
	const SmallStringHash_T<int64_t> *		m_pLocalDocs;
	int64_t									m_iTotalDocs;

public:
	CSphQueryContext ();
	~CSphQueryContext ();

	void						BindWeights ( const CSphQuery * pQuery, const CSphSchema & tSchema );
	bool						SetupCalc ( CSphQueryResult * pResult, const ISphSchema & tInSchema, const CSphSchema & tSchema, const DWORD * pMvaPool );
	bool						CreateFilters ( bool bFullscan, const CSphVector<CSphFilterSettings> * pdFilters, const ISphSchema & tSchema, const DWORD * pMvaPool, const BYTE * pStrings, CSphString & sError );
	bool						SetupOverrides ( const CSphQuery * pQuery, CSphQueryResult * pResult, const CSphSchema & tIndexSchema, const ISphSchema & tOutgoingSchema );

	void						CalcFilter ( CSphMatch & tMatch ) const;
	void						CalcSort ( CSphMatch & tMatch ) const;
	void						CalcFinal ( CSphMatch & tMatch ) const;

	void						FreeStrFilter ( CSphMatch & tMatch ) const;
	void						FreeStrSort ( CSphMatch & tMatch ) const;
	void						FreeStrFinal ( CSphMatch & tMatch ) const;

	// note that RT index bind pools at segment searching, not at time it setups context
	void						ExprCommand ( ESphExprCommand eCmd, void * pArg );
	void						SetStringPool ( const BYTE * pStrings );
	void						SetMVAPool ( const DWORD * pMva );
	void						SetupExtraData ( ISphExtra * pData );
};

//////////////////////////////////////////////////////////////////////////
// MEMORY TRACKER
//////////////////////////////////////////////////////////////////////////

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
		SPH_MEM_ALTER_SQL,

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
// BLOCK-LEVEL ATTRIBUTE INDEX BUILDER
//////////////////////////////////////////////////////////////////////////

#define DOCINFO_INDEX_FREQ 128 // FIXME? make this configurable
#define SPH_SKIPLIST_BLOCK 128 ///< must be a power of two

inline int64_t MVA_UPSIZE ( const DWORD * pMva )
{
	int64_t iMva = (int64_t)( (uint64_t)pMva[0] | ( ( (uint64_t)pMva[1] )<<32 ) );
	return iMva;
}

// FIXME!!! for over INT_MAX attributes
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
	CSphVector<int64_t>			m_dMvaMin;
	CSphVector<int64_t>			m_dMvaMax;
	CSphVector<int64_t>			m_dMvaIndexMin;
	CSphVector<int64_t>			m_dMvaIndexMax;
	DWORD						m_uStride;		// size of attribute's chunk (in DWORDs)
	DWORD						m_uElements;	// counts total number of collected min/max pairs
	int							m_iLoop;		// loop inside one set
	DWORD *						m_pOutBuffer;	// storage for collected min/max
	DWORD *						m_pOutMax;		// storage max for bound checking
	DOCID						m_uStart;		// first and last docids of current chunk
	DOCID						m_uLast;
	DOCID						m_uIndexStart;	// first and last docids of whole index
	DOCID						m_uIndexLast;
	int							m_iMva64;

private:
	void ResetLocal();
	void FlushComputed();
	void UpdateMinMaxDocids ( DOCID uDocID );
	void CollectRowMVA ( int iAttr, DWORD uCount, const DWORD * pMva );
	void CollectWithoutMvas ( const DWORD * pCur );

public:
	explicit AttrIndexBuilder_t ( const CSphSchema & tSchema );

	void Prepare ( DWORD * pOutBuffer, DWORD * pOutMax );

	bool Collect ( const DWORD * pCur, const DWORD * pMvas, int64_t iMvasCount, CSphString & sError, bool bHasMvaID );

	void FinishCollect ();

	/// actually used part of output buffer, only used with index merge
	/// (we reserve space for rows from both indexes, but might kill some rows)
	inline int64_t GetActualSize() const
	{
		return int64_t ( m_uElements ) * m_uStride * 2;
	}

	/// how many DWORDs will we need for block index
	inline int64_t GetExpectedSize ( int64_t iMaxDocs ) const
	{
		assert ( iMaxDocs>=0 );
		int64_t iDocinfoIndex = ( iMaxDocs + DOCINFO_INDEX_FREQ - 1 ) / DOCINFO_INDEX_FREQ;
		return ( iDocinfoIndex + 1 ) * m_uStride * 2;
	}
};

typedef AttrIndexBuilder_t<> AttrIndexBuilder_c;

// dirty hack for some build systems which not has LLONG_MAX
#ifndef LLONG_MAX
#define LLONG_MAX (((unsigned long long)(-1))>>1)
#endif

#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#endif


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
		m_dMvaMin[i] = LLONG_MAX;
		m_dMvaMax[i] = ( i>=m_iMva64 ? LLONG_MIN : 0 );
	}
	m_uStart = m_uLast = 0;
	m_iLoop = 0;
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::FlushComputed ()
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

		case SPH_ATTR_UINT32SET:
			m_dMvaAttrs.Add ( tCol.m_tLocator );
			break;

		default:
			break;
		}
	}

	m_iMva64 = m_dMvaAttrs.GetLength();
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		if ( tCol.m_eAttrType==SPH_ATTR_INT64SET )
			m_dMvaAttrs.Add ( tCol.m_tLocator );
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
	memset ( pOutBuffer, 0, ( pOutMax-pOutBuffer )*sizeof(DWORD) );

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
		m_dMvaIndexMin[i] = LLONG_MAX;
		m_dMvaIndexMax[i] = ( i>=m_iMva64 ? LLONG_MIN : 0 );
	}
	ResetLocal();
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::CollectWithoutMvas ( const DWORD * pCur )
{
	// check if it is time to flush already collected values
	if ( m_iLoop>=DOCINFO_INDEX_FREQ )
		FlushComputed ();

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
void AttrIndexBuilder_t<DOCID>::CollectRowMVA ( int iAttr, DWORD uCount, const DWORD * pMva )
{
	if ( iAttr>=m_iMva64 )
	{
		assert ( ( uCount%2 )==0 );
		for ( ; uCount>0; uCount-=2, pMva+=2 )
		{
			int64_t iVal = MVA_UPSIZE ( pMva );
			m_dMvaMin[iAttr] = Min ( m_dMvaMin[iAttr], iVal );
			m_dMvaMax[iAttr] = Max ( m_dMvaMax[iAttr], iVal );
		}
	} else
	{
		for ( ; uCount>0; uCount--, pMva++ )
		{
			DWORD uVal = *pMva;
			m_dMvaMin[iAttr] = Min ( m_dMvaMin[iAttr], uVal );
			m_dMvaMax[iAttr] = Max ( m_dMvaMax[iAttr], uVal );
		}
	}
}

template < typename DOCID >
bool AttrIndexBuilder_t<DOCID>::Collect ( const DWORD * pCur, const DWORD * pMvas, int64_t iMvasCount, CSphString & sError, bool bHasMvaID )
{
	CollectWithoutMvas ( pCur );

	const DWORD * pRow = DOCINFO2ATTRS_T<DOCID>(pCur);
	SphDocID_t uDocID = DOCINFO2ID_T<DOCID>(pCur);

	// MVAs
	ARRAY_FOREACH ( i, m_dMvaAttrs )
	{
		SphAttr_t uOff = sphGetRowAttr ( pRow, m_dMvaAttrs[i] );
		if ( !uOff )
			continue;

		// sanity checks
		if ( uOff>=iMvasCount )
		{
			sError.SetSprintf ( "broken index: mva offset out of bounds, id=" DOCID_FMT, (SphDocID_t)uDocID );
			return false;
		}

		const DWORD * pMva = pMvas + uOff; // don't care about updates at this point

		if ( bHasMvaID && i==0 && DOCINFO2ID_T<DOCID> ( pMva-DWSIZEOF(DOCID) )!=uDocID )
		{
			sError.SetSprintf ( "broken index: mva docid verification failed, id=" DOCID_FMT, (SphDocID_t)uDocID );
			return false;
		}

		DWORD uCount = *pMva++;
		if ( ( uOff+uCount>=iMvasCount ) || ( i>=m_iMva64 && ( uCount%2 )!=0 ) )
		{
			sError.SetSprintf ( "broken index: mva list out of bounds, id=" DOCID_FMT, (SphDocID_t)uDocID );
			return false;
		}

		// walk and calc
		CollectRowMVA ( i, uCount, pMva );
	}
	return true;
}

template < typename DOCID >
void AttrIndexBuilder_t<DOCID>::FinishCollect ()
{
	assert ( m_pOutBuffer );
	if ( m_iLoop )
		FlushComputed ();

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
}

//////////////////////////////////////////////////////////////////////////
// INLINES, FIND_XXX() GENERIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////

/// find a value-enclosing span in a sorted vector (aka an index at which vec[i] <= val < vec[i+1])
template < typename T, typename U >
static int FindSpan ( const CSphVector<T> & dVec, U tRef, int iSmallTreshold=8 )
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
// INLINES, UTF-8 TOOLS
//////////////////////////////////////////////////////////////////////////

#define SPH_MAX_UTF8_BYTES 4

/// decode UTF-8 codepoint
/// advances buffer ptr in all cases, including the end of buffer (ie. zero byte)!
/// so eof MUST be handled, otherwise, you get OOB
///
/// returns -1 on failure
/// returns 0 on end of buffer
/// returns codepoint on success
inline int sphUTF8Decode ( const BYTE * & pBuf )
{
	BYTE v = *pBuf++;
	if ( !v )
		return 0;

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
	if ( iBytes<2 || iBytes>SPH_MAX_UTF8_BYTES )
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


/// encode UTF-8 codepoint to buffer, macro version for the Really Critical places
#define SPH_UTF8_ENCODE(_ptr,_code) \
	if ( (_code)<0x80 ) \
	{ \
		*_ptr++ = (BYTE)( (_code) & 0x7F ); \
	} else if ( (_code)<0x800 ) \
	{ \
		_ptr[0] = (BYTE)( ( ((_code)>>6) & 0x1F ) | 0xC0 ); \
		_ptr[1] = (BYTE)( ( (_code) & 0x3F ) | 0x80 ); \
		_ptr += 2; \
	} else if ( (_code)<0x10000 )\
	{ \
		_ptr[0] = (BYTE)( ( ((_code)>>12) & 0x0F ) | 0xE0 ); \
		_ptr[1] = (BYTE)( ( ((_code)>>6) & 0x3F ) | 0x80 ); \
		_ptr[2] = (BYTE)( ( (_code) & 0x3F ) | 0x80 ); \
		_ptr += 3; \
	} else \
	{ \
		_ptr[0] = (BYTE)( ( ((_code)>>18) & 0x0F ) | 0xF0 ); \
		_ptr[1] = (BYTE)( ( ((_code)>>12) & 0x3F ) | 0x80 ); \
		_ptr[2] = (BYTE)( ( ((_code)>>6) & 0x3F ) | 0x80 ); \
		_ptr[3] = (BYTE)( ( (_code) & 0x3F ) | 0x80 ); \
		_ptr += 4; \
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

	} else if ( iCode<0x10000 )
	{
		pBuf[0] = (BYTE)( ( (iCode>>12) & 0x0F ) | 0xE0 );
		pBuf[1] = (BYTE)( ( (iCode>>6) & 0x3F ) | 0x80 );
		pBuf[2] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 3;
	} else
	{
		pBuf[0] = (BYTE)( ( (iCode>>18) & 0x0F ) | 0xF0 );
		pBuf[1] = (BYTE)( ( (iCode>>12) & 0x3F ) | 0x80 );
		pBuf[2] = (BYTE)( ( (iCode>>6) & 0x3F ) | 0x80 );
		pBuf[3] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 4;
	}
}


/// compute UTF-8 string length in codepoints
inline int sphUTF8Len ( const char * pStr )
{
	if ( !pStr || *pStr=='\0' )
		return 0;

	const BYTE * pBuf = (const BYTE*) pStr;
	int iRes = 0, iCode;

	while ( ( iCode = sphUTF8Decode(pBuf) )!=0 )
		if ( iCode>0 )
			iRes++;

	return iRes;
}


/// compute UTF-8 string length in codepoints
inline int sphUTF8Len ( const char * pStr, int iMax )
{
	if ( !pStr || *pStr=='\0' )
		return 0;

	const BYTE * pBuf = (const BYTE*) pStr;
	const BYTE * pMax = pBuf + iMax;
	int iRes = 0, iCode;

	while ( pBuf<pMax && iRes<iMax && ( iCode = sphUTF8Decode ( pBuf ) )!=0 )
		if ( iCode>0 )
			iRes++;

	return iRes;
}

//////////////////////////////////////////////////////////////////////////
// MATCHING ENGINE INTERNALS
//////////////////////////////////////////////////////////////////////////

/// hit in the stream
struct ExtHit_t
{
	SphDocID_t	m_uDocid;
	Hitpos_t	m_uHitpos;
	WORD		m_uQuerypos;
	WORD		m_uNodepos;
	WORD		m_uSpanlen;
	WORD		m_uMatchlen;
	DWORD		m_uWeight;
	DWORD		m_uQposMask;
};

enum SphZoneHit_e
{
	SPH_ZONE_FOUND,
	SPH_ZONE_NO_SPAN,
	SPH_ZONE_NO_DOCUMENT
};

class ISphZoneCheck
{
public:
	virtual ~ISphZoneCheck () {}
	virtual SphZoneHit_e IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan=0 ) = 0;
};


struct SphFactorHashEntry_t
{
	SphDocID_t				m_iId;
	int						m_iRefCount;
	BYTE *					m_pData;
	SphFactorHashEntry_t *	m_pPrev;
	SphFactorHashEntry_t *	m_pNext;
};

typedef CSphTightVector<SphFactorHashEntry_t *> SphFactorHash_t;


struct SphExtraDataRankerState_t
{
	const CSphSchema *	m_pSchema;
	const int64_t *		m_pFieldLens;
	CSphAttrLocator		m_tFieldLensLoc;
	int64_t				m_iTotalDocuments;
	int					m_iFields;
	int					m_iMaxQpos;
	SphExtraDataRankerState_t ()
		: m_pSchema ( NULL )
		, m_pFieldLens ( NULL )
		, m_iTotalDocuments ( 0 )
		, m_iFields ( 0 )
		, m_iMaxQpos ( 0 )
	{ }
};


//////////////////////////////////////////////////////////////////////////
// INLINES, MISC
//////////////////////////////////////////////////////////////////////////

inline const char * sphTypeName ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_NONE:			return "none";
		case SPH_ATTR_INTEGER:		return "uint";
		case SPH_ATTR_TIMESTAMP:	return "timestamp";
		case SPH_ATTR_ORDINAL:		return "ordinal";
		case SPH_ATTR_BOOL:			return "bool";
		case SPH_ATTR_FLOAT:		return "float";
		case SPH_ATTR_BIGINT:		return "bigint";
		case SPH_ATTR_STRING:		return "string";
		case SPH_ATTR_WORDCOUNT:	return "wordcount";
		case SPH_ATTR_STRINGPTR:	return "stringptr";
		case SPH_ATTR_TOKENCOUNT:	return "tokencount";
		case SPH_ATTR_JSON:			return "json";

		case SPH_ATTR_UINT32SET:	return "mva";
		case SPH_ATTR_INT64SET:		return "mva64";
		default:					return "unknown";
	}
}

inline const char * sphTypeDirective ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_NONE:			return "???";
		case SPH_ATTR_INTEGER:		return "sql_attr_uint";
		case SPH_ATTR_TIMESTAMP:	return "sql_attr_timestamp";
		case SPH_ATTR_ORDINAL:		return "sql_attr_str2ordinal";
		case SPH_ATTR_BOOL:			return "sql_attr_bool";
		case SPH_ATTR_FLOAT:		return "sql_attr_float";
		case SPH_ATTR_BIGINT:		return "sql_attr_bigint";
		case SPH_ATTR_STRING:		return "sql_attr_string";
		case SPH_ATTR_WORDCOUNT:	return "sql_attr_wordcount";
		case SPH_ATTR_STRINGPTR:	return "sql_attr_string";
		case SPH_ATTR_TOKENCOUNT:	return "_autogenerated_tokencount";
		case SPH_ATTR_JSON:			return "sql_attr_json";

		case SPH_ATTR_UINT32SET:	return "sql_attr_multi";
		case SPH_ATTR_INT64SET:		return "sql_attr_multi bigint";
		default:					return "???";
	}
}

inline void SqlUnescape ( CSphString & sRes, const char * sEscaped, int iLen )
{
	assert ( iLen>=2 );
	assert (
		( sEscaped[0]=='\'' && sEscaped[iLen-1]=='\'' ) ||
		( sEscaped[0]=='"' && sEscaped[iLen-1]=='"' ) );

	// skip heading and trailing quotes
	const char * s = sEscaped+1;
	const char * sMax = s+iLen-2;

	sRes.Reserve ( iLen );
	char * d = (char*) sRes.cstr();

	while ( s<sMax )
	{
		if ( s[0]=='\\' )
		{
			switch ( s[1] )
			{
			case 'b': *d++ = '\b'; break;
			case 'n': *d++ = '\n'; break;
			case 'r': *d++ = '\r'; break;
			case 't': *d++ = '\t'; break;
			default:
				*d++ = s[1];
			}
			s += 2;
		} else
			*d++ = *s++;
	}

	*d++ = '\0';
}


inline void StripPath ( CSphString & sPath )
{
	if ( sPath.IsEmpty() )
		return;

	const char * s = sPath.cstr();
	if ( *s!='/' )
		return;

	const char * sLastSlash = s;
	for ( ; *s; s++ )
		if ( *s=='/' )
			sLastSlash = s;

	int iPos = (int)( sLastSlash - sPath.cstr() + 1 );
	int iLen = (int)( s - sPath.cstr() );
	sPath = sPath.SubString ( iPos, iLen - iPos );
}

//////////////////////////////////////////////////////////////////////////
// DISK INDEX INTERNALS
//////////////////////////////////////////////////////////////////////////

/// locator pair, for RT string dynamization
struct LocatorPair_t
{
	CSphAttrLocator m_tFrom;	///< source (static) locator
	CSphAttrLocator m_tTo;		///< destination (dynamized) locator
};

//////////////////////////////////////////////////////////////////////////
// DICTIONARY INTERNALS
//////////////////////////////////////////////////////////////////////////

/// dict traits
class CSphDictTraits : public CSphDict
{
public:
	explicit			CSphDictTraits ( CSphDict * pDict ) : m_pDict ( pDict ) { assert ( m_pDict ); }

	virtual void		LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer ) { m_pDict->LoadStopwords ( sFiles, pTokenizer ); }
	virtual void		LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) { m_pDict->LoadStopwords ( dStopwords ); }
	virtual void		WriteStopwords ( CSphWriter & tWriter ) { m_pDict->WriteStopwords ( tWriter ); }
	virtual bool		LoadWordforms ( const CSphVector<CSphString> & dFiles, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex ) { return m_pDict->LoadWordforms ( dFiles, pEmbedded, pTokenizer, sIndex ); }
	virtual void		WriteWordforms ( CSphWriter & tWriter ) { m_pDict->WriteWordforms ( tWriter ); }
	virtual int			SetMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sMessage ) { return m_pDict->SetMorphology ( szMorph, bUseUTF8, sMessage ); }

	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) { return m_pDict->GetWordID ( pWord, iLen, bFilterStops ); }
	virtual SphWordID_t GetWordID ( BYTE * pWord );

	virtual void		Setup ( const CSphDictSettings & ) {}
	virtual const CSphDictSettings & GetSettings () const { return m_pDict->GetSettings (); }
	virtual const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () { return m_pDict->GetStopwordsFileInfos (); }
	virtual const CSphVector <CSphSavedFile> & GetWordformsFileInfos () { return m_pDict->GetWordformsFileInfos (); }
	virtual const CSphMultiformContainer * GetMultiWordforms () const { return m_pDict->GetMultiWordforms (); }

	virtual bool		IsStopWord ( const BYTE * pWord ) const { return m_pDict->IsStopWord ( pWord ); }
	virtual uint64_t	GetSettingsFNV () const { return m_pDict->GetSettingsFNV(); }
	virtual void		SetApplyMorph ( bool bApply ) { m_pDict->SetApplyMorph ( bApply ); }

protected:
	CSphDict *			m_pDict;
};


/// dict wrapper for star-syntax support in prefix-indexes
class CSphDictStar : public CSphDictTraits
{
public:
	explicit			CSphDictStar ( CSphDict * pDict ) : CSphDictTraits ( pDict ) {}

	virtual SphWordID_t	GetWordID ( BYTE * pWord );
	virtual SphWordID_t	GetWordIDNonStemmed ( BYTE * pWord );
};


/// star dict for index v.8+
class CSphDictStarV8 : public CSphDictStar
{
public:
	CSphDictStarV8 ( CSphDict * pDict, bool bPrefixes, bool bInfixes );

	virtual SphWordID_t	GetWordID ( BYTE * pWord );

private:
	bool				m_bPrefixes;
	bool				m_bInfixes;
};


/// dict wrapper for exact-word syntax
class CSphDictExact : public CSphDictTraits
{
public:
	explicit CSphDictExact ( CSphDict * pDict ) : CSphDictTraits ( pDict ) {}
	virtual SphWordID_t	GetWordID ( BYTE * pWord );
};

//////////////////////////////////////////////////////////////////////////
// TOKEN FILTER
//////////////////////////////////////////////////////////////////////////

/// token filter base (boring proxy stuff)
class CSphTokenFilter : public ISphTokenizer
{
protected:
	ISphTokenizer *		m_pTokenizer;

public:
	explicit						CSphTokenFilter ( ISphTokenizer * pTokenizer )					: m_pTokenizer ( pTokenizer ) {}
									~CSphTokenFilter()												{ SafeDelete ( m_pTokenizer ); }

	virtual bool					SetCaseFolding ( const char * sConfig, CSphString & sError )	{ return m_pTokenizer->SetCaseFolding ( sConfig, sError ); }
	virtual void					AddPlainChar ( char c )											{ m_pTokenizer->AddPlainChar ( c ); }
	virtual void					AddSpecials ( const char * sSpecials )							{ m_pTokenizer->AddSpecials ( sSpecials ); }
	virtual bool					SetIgnoreChars ( const char * sIgnored, CSphString & sError )	{ return m_pTokenizer->SetIgnoreChars ( sIgnored, sError ); }
	virtual bool					SetNgramChars ( const char * sConfig, CSphString & sError )		{ return m_pTokenizer->SetNgramChars ( sConfig, sError ); }
	virtual void					SetNgramLen ( int iLen )										{ m_pTokenizer->SetNgramLen ( iLen ); }
	virtual bool					LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, CSphString & sError ) { return m_pTokenizer->LoadSynonyms ( sFilename, pFiles, sError ); }
	virtual void					WriteSynonyms ( CSphWriter & tWriter )							{ return m_pTokenizer->WriteSynonyms ( tWriter ); }
	virtual bool					SetBoundary ( const char * sConfig, CSphString & sError )		{ return m_pTokenizer->SetBoundary ( sConfig, sError ); }
	virtual void					Setup ( const CSphTokenizerSettings & tSettings )				{ m_pTokenizer->Setup ( tSettings ); }
	virtual const CSphTokenizerSettings &	GetSettings () const									{ return m_pTokenizer->GetSettings (); }
	virtual const CSphSavedFile &	GetSynFileInfo () const											{ return m_pTokenizer->GetSynFileInfo (); }
	virtual bool					EnableSentenceIndexing ( CSphString & sError )					{ return m_pTokenizer->EnableSentenceIndexing ( sError ); }
	virtual bool					EnableZoneIndexing ( CSphString & sError )						{ return m_pTokenizer->EnableZoneIndexing ( sError ); }
	virtual int						SkipBlended ()													{ return m_pTokenizer->SkipBlended(); }

	virtual int						GetCodepointLength ( int iCode ) const		{ return m_pTokenizer->GetCodepointLength ( iCode ); }
	virtual int						GetMaxCodepointLength () const				{ return m_pTokenizer->GetMaxCodepointLength(); }

	virtual bool					IsUtf8 () const								{ return m_pTokenizer->IsUtf8 (); }
	virtual const char *			GetTokenStart () const						{ return m_pTokenizer->GetTokenStart(); }
	virtual const char *			GetTokenEnd () const						{ return m_pTokenizer->GetTokenEnd(); }
	virtual const char *			GetBufferPtr () const						{ return m_pTokenizer->GetBufferPtr(); }
	virtual const char *			GetBufferEnd () const						{ return m_pTokenizer->GetBufferEnd (); }
	virtual void					SetBufferPtr ( const char * sNewPtr )		{ m_pTokenizer->SetBufferPtr ( sNewPtr ); }
	virtual uint64_t				GetSettingsFNV () const						{ return m_pTokenizer->GetSettingsFNV(); }

	virtual void					SetBuffer ( const BYTE * sBuffer, int iLength )	{ m_pTokenizer->SetBuffer ( sBuffer, iLength ); }
	virtual BYTE *					GetToken ()										{ return m_pTokenizer->GetToken(); }

	virtual ISphTokenizer *			GetEmbeddedTokenizer () const					{ return m_pTokenizer; }
};

//////////////////////////////////////////////////////////////////////////
// USER VARIABLES
//////////////////////////////////////////////////////////////////////////

/// value container for the intset uservar type
class UservarIntSet_c : public CSphVector<SphAttr_t>, public ISphRefcountedMT
{
};

//////////////////////////////////////////////////////////////////////////
// BINLOG INTERNALS
//////////////////////////////////////////////////////////////////////////

/// global binlog interface
class ISphBinlog : ISphNoncopyable
{
public:
	virtual				~ISphBinlog () {}

	virtual void		BinlogUpdateAttributes ( int64_t * pTID, const char * sIndexName, const CSphAttrUpdate & tUpd ) = 0;
	virtual void		NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown ) = 0;
};

//////////////////////////////////////////////////////////////////////////
// MISC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

struct SphStringSorterRemap_t
{
	CSphAttrLocator m_tSrc;
	CSphAttrLocator m_tDst;
};

struct ThrottleState_t
{
	int64_t	m_tmLastIOTime;
	int		m_iMaxIOps;
	int		m_iMaxIOSize;

	ThrottleState_t ()
		: m_tmLastIOTime ( 0 )
		, m_iMaxIOps ( 0 )
		, m_iMaxIOSize ( 0 )
	{}
};

void			SafeClose ( int & iFD );
const BYTE *	SkipQuoted ( const BYTE * p );

bool			sphSortGetStringRemap ( const ISphSchema & tSorterSchema, const ISphSchema & tIndexSchema, CSphVector<SphStringSorterRemap_t> & dAttrs );
bool			sphIsSortStringInternal ( const char * sColumnName );
/// make string lowercase but keep case of JSON.field
void			sphColumnToLowercase ( char * sVal );

void			sphCheckWordStats ( const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hDst, const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hSrc, const char * sIndex, CSphString & sWarning );
bool			sphCheckQueryHeight ( const struct XQNode_t * pRoot, CSphString & sError );
void			sphTransformExtendedQuery ( XQNode_t ** ppNode, const CSphIndexSettings & tSettings, bool bHasBooleanOptimization, const ISphKeywordsStat * pKeywords );
void			TransformAotFilter ( XQNode_t * pNode, bool bUtf8, const CSphWordforms * pWordforms, const CSphIndexSettings& tSettings );
bool			sphMerge ( const CSphIndex * pDst, const CSphIndex * pSrc, ISphFilter * pFilter, CSphString & sError, CSphIndexProgress & tProgress, ThrottleState_t * pThrottle, volatile bool * pForceTerminate );
CSphString		sphReconstructNode ( const XQNode_t * pNode, const CSphSchema * pSchema );

void			sphSetUnlinkOld ( bool bUnlink );
void			sphUnlinkIndex ( const char * sName, bool bForce );
void			sphSetDebugCheck ();

void			WriteSchema ( CSphWriter & fdInfo, const CSphSchema & tSchema );
void			ReadSchema ( CSphReader & rdInfo, CSphSchema & m_tSchema, DWORD uVersion, bool bDynamic );
void			SaveIndexSettings ( CSphWriter & tWriter, const CSphIndexSettings & m_tSettings );
void			LoadIndexSettings ( CSphIndexSettings & tSettings, CSphReader & tReader, DWORD uVersion );
void			SaveTokenizerSettings ( CSphWriter & tWriter, ISphTokenizer * pTokenizer, int iEmbeddedLimit );
void			LoadTokenizerSettings ( CSphReader & tReader, CSphTokenizerSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, DWORD uVersion, CSphString & sWarning );
void			SaveDictionarySettings ( CSphWriter & tWriter, CSphDict * pDict, bool bForceWordDict, int iEmbeddedLimit );
void			LoadDictionarySettings ( CSphReader & tReader, CSphDictSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, DWORD uVersion, CSphString & sWarning );
void			SaveFieldFilterSettings ( CSphWriter & tWriter, ISphFieldFilter * pFieldFilter );

DWORD			ReadVersion ( const char * sPath, CSphString & sError );
bool			AddFieldLens ( CSphSchema & tSchema, bool bDynamic, CSphString & sError );


enum ESphExtType
{
	SPH_EXT_TYPE_CUR = 0,
	SPH_EXT_TYPE_NEW,
	SPH_EXT_TYPE_OLD,
	SPH_EXT_TYPE_LOC
};

enum ESphExt
{
	SPH_EXT_SPH = 0,
	SPH_EXT_SPA = 1,
	SPH_EXT_MVP = 9
};

const char ** sphGetExts ( ESphExtType eType, DWORD uVersion=INDEX_FORMAT_VERSION );
int sphGetExtCount ( DWORD uVersion=INDEX_FORMAT_VERSION );
const char * sphGetExt ( ESphExtType eType, ESphExt eExt );

int sphDictCmp ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 );
int sphDictCmpStrictly ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 );

template <typename CP>
int sphCheckpointCmp ( const char * sWord, int iLen, SphWordID_t iWordID, bool bWordDict, const CP & tCP )
{
	if ( bWordDict )
		return sphDictCmp ( sWord, iLen, tCP.m_sWord, strlen ( tCP.m_sWord ) );

	int iRes = 0;
	iRes = iWordID<tCP.m_iWordID ? -1 : iRes;
	iRes = iWordID>tCP.m_iWordID ? 1 : iRes;
	return iRes;
}

template <typename CP>
int sphCheckpointCmpStrictly ( const char * sWord, int iLen, SphWordID_t iWordID, bool bWordDict, const CP & tCP )
{
	if ( bWordDict )
		return sphDictCmpStrictly ( sWord, iLen, tCP.m_sWord, strlen ( tCP.m_sWord ) );

	int iRes = 0;
	iRes = iWordID<tCP.m_iWordID ? -1 : iRes;
	iRes = iWordID>tCP.m_iWordID ? 1 : iRes;
	return iRes;
}


template < typename CP >
const CP * sphSearchCheckpoint ( const char * sWord, int iWordLen, SphWordID_t iWordID
							, bool bStarMode, bool bWordDict
							, const CP * pFirstCP, const CP * pLastCP )
{
	assert ( !bWordDict || iWordLen>0 );

	const CP * pStart = pFirstCP;
	const CP * pEnd = pLastCP;

	if ( bStarMode && sphCheckpointCmp ( sWord, iWordLen, iWordID, bWordDict, *pStart )<0 )
		return NULL;
	if ( !bStarMode && sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, *pStart )<0 )
		return NULL;

	if ( sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, *pEnd )>=0 )
		pStart = pEnd;
	else
	{
		while ( pEnd-pStart>1 )
		{
			const CP * pMid = pStart + (pEnd-pStart)/2;
			const int iCmpRes = sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, *pMid );

			if ( iCmpRes==0 )
			{
				pStart = pMid;
				break;
			} else if ( iCmpRes<0 )
				pEnd = pMid;
			else
				pStart = pMid;
		}

		assert ( pStart>=pFirstCP );
		assert ( pStart<=pLastCP );
		assert ( sphCheckpointCmp ( sWord, iWordLen, iWordID, bWordDict, *pStart )>=0
			&& sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, *pEnd )<0 );
	}

	return pStart;
}


class ISphRtDictWraper : public CSphDict
{
public:
	virtual const BYTE *	GetPackedKeywords () = 0;
	virtual int				GetPackedLen () = 0;

	virtual void			ResetKeywords() = 0;

	virtual const char *	GetLastWarning() const = 0;
	virtual void			ResetWarning() = 0;
};

ISphRtDictWraper * sphCreateRtKeywordsDictionaryWrapper ( CSphDict * pBase );


class ISphWordlist
{
public:
	virtual ~ISphWordlist () {}
	virtual void GetPrefixedWords ( const char * sPrefix, int iPrefix, const char * sWildcard, CSphVector<CSphNamedInt> & dPrefixedWords, BYTE * pDictBuf, int iFD ) const = 0;
	virtual void GetInfixedWords ( const char * sInfix, int iInfix, const char * sWildcard, CSphVector<CSphNamedInt> & dPrefixedWords ) const = 0;
};

struct ExpansionContext_t
{
	const ISphWordlist * m_pWordlist;
	BYTE * m_pBuf;
	CSphQueryResultMeta * m_pResult;
	int m_iFD;
	int m_iMinPrefixLen;
	int m_iMinInfixLen;
	int m_iExpansionLimit;
	bool m_bHasMorphology;
	bool m_bMergeSingles;
};


XQNode_t * sphExpandXQNode ( XQNode_t * pNode, ExpansionContext_t & tCtx );
int sphGetExpansionMagic ( int iDocs, int iHits );
void sphQueryAdjustStars ( XQNode_t * pNode, const CSphIndexSettings & tSettings );
XQNode_t * sphQueryExpandKeywords ( XQNode_t * pNode, const CSphIndexSettings & tSettings, bool bStarEnabled );


class CSphKeywordDeltaWriter
{
private:
	BYTE m_sLastKeyword [SPH_MAX_WORD_LEN*3+4];
	int m_iLastLen;

public:
	CSphKeywordDeltaWriter ()
	{
		Reset();
	}

	void Reset ()
	{
		m_iLastLen = 0;
	}

	template <typename F>
	void PutDelta ( F & WRITER, const BYTE * pWord, int iLen )
	{
		assert ( pWord && iLen );

		// how many bytes of a previous keyword can we reuse?
		BYTE iMatch = 0;
		int iMinLen = Min ( m_iLastLen, iLen );
		assert ( iMinLen<(int)sizeof(m_sLastKeyword) );
		while ( iMatch<iMinLen && m_sLastKeyword[iMatch]==pWord[iMatch] )
		{
			iMatch++;
		}

		BYTE iDelta = (BYTE)( iLen - iMatch );
		assert ( iDelta>0 );

		assert ( iLen < (int)sizeof(m_sLastKeyword) );
		memcpy ( m_sLastKeyword, pWord, iLen );
		m_iLastLen = iLen;

		// match and delta are usually tiny, pack them together in 1 byte
		// tricky bit, this byte leads the entry so it must never be 0 (aka eof mark)!
		if ( iDelta<=8 && iMatch<=15 )
		{
			BYTE uPacked = ( 0x80 + ( (iDelta-1)<<4 ) + iMatch );
			WRITER.PutBytes ( &uPacked, 1 );
		} else
		{
			WRITER.PutBytes ( &iDelta, 1 ); // always greater than 0
			WRITER.PutBytes ( &iMatch, 1 );
		}

		WRITER.PutBytes ( pWord + iMatch, iDelta );
	}
};

BYTE sphDoclistHintPack ( SphOffset_t iDocs, SphOffset_t iLen );

// wordlist checkpoints frequency
#define SPH_WORDLIST_CHECKPOINT 64

/// startup mva updates arena
const char *		sphArenaInit ( int iMaxBytes );

#if USE_WINDOWS
void localtime_r ( const time_t * clock, struct tm * res );
#endif

struct InfixBlock_t
{
	union
	{
		const char *	m_sInfix;
		DWORD			m_iInfixOffset;
	};
	int			m_iOffset;
};


/// infix hash builder
class ISphInfixBuilder
{
public:
	explicit		ISphInfixBuilder() {}
	virtual			~ISphInfixBuilder() {}
	virtual void	AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint ) = 0;
	virtual void	SaveEntries ( CSphWriter & wrDict ) = 0;
	virtual int		SaveEntryBlocks ( CSphWriter & wrDict ) = 0;
	virtual int		GetBlocksWordsSize () const = 0;
};


ISphInfixBuilder * sphCreateInfixBuilder ( int iCodepointBytes, CSphString * pError );
bool sphLookupInfixCheckpoints ( const char * sInfix, int iBytes, const BYTE * pInfixes, const CSphVector<InfixBlock_t> & dInfixBlocks, int iInfixCodepointBytes, CSphVector<int> & dCheckpoints );
// calculate length, upto iInfixCodepointBytes chars from infix start
int sphGetInfixLength ( const char * sInfix, int iBytes, int iInfixCodepointBytes );


/// compute utf-8 character length in bytes from its first byte
inline int sphUtf8CharBytes ( BYTE uFirst )
{
	switch ( uFirst>>4 )
	{
		case 12: return 2; // 110x xxxx, 2 bytes
		case 13: return 2; // 110x xxxx, 2 bytes
		case 14: return 3; // 1110 xxxx, 3 bytes
		case 15: return 4; // 1111 0xxx, 4 bytes
		default: return 1; // either 1 byte, or invalid/unsupported code
	}
}

//////////////////////////////////////////////////////////////////////////

/// snippet setupper
/// used by searchd and SNIPPET() function in exprs
/// should probably be refactored as a single function
/// a precursor to sphBuildExcerpts() call
class SnippetContext_t : ISphNoncopyable
{
private:
	CSphScopedPtr<CSphDict> m_tDictCloned;
	CSphScopedPtr<CSphDict> m_tExactDict;

public:
	CSphDict * m_pDict;
	CSphScopedPtr<ISphTokenizer> m_tTokenizer;
	CSphScopedPtr<CSphHTMLStripper> m_tStripper;
	ISphTokenizer * m_pQueryTokenizer;
	XQQuery_t m_tExtQuery;
	DWORD m_eExtQuerySPZ;

	SnippetContext_t()
		: m_tDictCloned ( NULL )
		, m_tExactDict ( NULL )
		, m_pDict ( NULL )
		, m_tTokenizer ( NULL )
		, m_tStripper ( NULL )
		, m_pQueryTokenizer ( NULL )
		, m_eExtQuerySPZ ( SPH_SPZ_NONE )
	{
	}

	~SnippetContext_t()
	{
		SafeDelete ( m_pQueryTokenizer );
	}

	static CSphDict * SetupExactDict ( const CSphIndexSettings & tSettings, const ExcerptQuery_t & q,
		CSphScopedPtr<CSphDict> & tExact, CSphDict * pDict, ISphTokenizer * pTok )
	{
		// handle index_exact_words
		if ( !( q.m_bHighlightQuery && tSettings.m_bIndexExactWords ) )
			return pDict;

		tExact = new CSphDictExact ( pDict );
		pTok->AddPlainChar ( '=' );
		return tExact.Ptr();
	}

	static DWORD CollectQuerySPZ ( const XQNode_t * pNode )
	{
		if ( !pNode )
			return SPH_SPZ_NONE;

		DWORD eSPZ = SPH_SPZ_NONE;
		if ( pNode->GetOp()==SPH_QUERY_SENTENCE )
			eSPZ |= SPH_SPZ_SENTENCE;
		else if ( pNode->GetOp()==SPH_QUERY_PARAGRAPH )
			eSPZ |= SPH_SPZ_PARAGRAPH;

		ARRAY_FOREACH ( i, pNode->m_dChildren )
			eSPZ |= CollectQuerySPZ ( pNode->m_dChildren[i] );

		return eSPZ;
	}

	static bool SetupStripperSPZ ( const CSphIndexSettings & tSettings, const ExcerptQuery_t & q,
		bool bSetupSPZ, CSphScopedPtr<CSphHTMLStripper> & tStripper, ISphTokenizer * pTokenizer,
		CSphString & sError )
	{
		if ( bSetupSPZ &&
			( !pTokenizer->EnableSentenceIndexing ( sError ) || !pTokenizer->EnableZoneIndexing ( sError ) ) )
		{
			return false;
		}


		if ( q.m_sStripMode=="strip" || q.m_sStripMode=="retain"
			|| ( q.m_sStripMode=="index" && tSettings.m_bHtmlStrip ) )
		{
			// don't strip HTML markup in 'retain' mode - proceed zones only
			tStripper = new CSphHTMLStripper ( q.m_sStripMode!="retain" );

			if ( q.m_sStripMode=="index" )
			{
				if (
					!tStripper->SetIndexedAttrs ( tSettings.m_sHtmlIndexAttrs.cstr (), sError ) ||
					!tStripper->SetRemovedElements ( tSettings.m_sHtmlRemoveElements.cstr (), sError ) )
				{
					sError.SetSprintf ( "HTML stripper config error: %s", sError.cstr() );
					return false;
				}
			}

			if ( bSetupSPZ )
			{
				tStripper->EnableParagraphs();
			}

			// handle zone(s) in special mode only when passage_boundary enabled
			if ( bSetupSPZ && !tStripper->SetZones ( tSettings.m_sZones.cstr (), sError ) )
			{
				sError.SetSprintf ( "HTML stripper config error: %s", sError.cstr() );
				return false;
			}
		}

		return true;
	}

	bool Setup ( const CSphIndex * pIndex, const ExcerptQuery_t & tSettings, CSphString & sError )
	{
		assert ( pIndex );
		CSphScopedPtr<CSphDict> tDictCloned ( NULL );
		m_pDict = pIndex->GetDictionary();
		if ( m_pDict->HasState() )
			m_tDictCloned = m_pDict = m_pDict->Clone();
		m_tTokenizer = pIndex->GetTokenizer()->Clone ( SPH_CLONE_INDEX ); // OPTIMIZE! do a lightweight indexing clone here
		m_pQueryTokenizer = pIndex->GetQueryTokenizer()->Clone ( SPH_CLONE_QUERY_LIGHTWEIGHT );

		// setup exact dictionary if needed
		m_pDict = SetupExactDict ( pIndex->GetSettings(), tSettings, m_tExactDict, m_pDict, m_tTokenizer.Ptr() );

		if ( tSettings.m_bHighlightQuery )
		{
			// OPTIMIZE? double lightweight clone here? but then again it's lightweight
			if ( !sphParseExtendedQuery ( m_tExtQuery, tSettings.m_sWords.cstr(), pIndex->GetQueryTokenizer(),
				&pIndex->GetMatchSchema(), m_pDict, pIndex->GetSettings() ) )
			{
				sError = m_tExtQuery.m_sParseError;
				return false;
			}
			if ( m_tExtQuery.m_pRoot )
				m_tExtQuery.m_pRoot->ClearFieldMask();

			m_eExtQuerySPZ = SPH_SPZ_NONE;
			m_eExtQuerySPZ |= CollectQuerySPZ ( m_tExtQuery.m_pRoot );
			if ( m_tExtQuery.m_dZones.GetLength() )
				m_eExtQuerySPZ |= SPH_SPZ_ZONE;
		}

		bool bSetupSPZ = ( tSettings.m_ePassageSPZ!=SPH_SPZ_NONE || m_eExtQuerySPZ!=SPH_SPZ_NONE ||
			( tSettings.m_sStripMode=="retain" && tSettings.m_bHighlightQuery ) );

		if ( !SetupStripperSPZ ( pIndex->GetSettings(), tSettings, bSetupSPZ, m_tStripper, m_tTokenizer.Ptr(), sError ) )
			return false;

		return true;
	}
};


#if USE_RLP

struct StoredDoc_t
{
	CSphMatch							m_tDocInfo;
	CSphVector<CSphString>				m_dStrAttrs;
	CSphVector<DWORD>					m_dMva;
	CSphTightVector<BYTE*>				m_dFields;
	CSphTightVector<bool>				m_dChinese;
	CSphTightVector< CSphVector<BYTE> >	m_dFieldStorage;
	CSphTightVector< CSphVector<BYTE> >	m_dNonChineseTokens;
};

// these are used to separate text before passing it to RLP
const int PROXY_DOCUMENT_START = 0xFFFA;
const int PROXY_FIELD_START_CHINESE = 0xFFFB;
const int PROXY_FIELD_START_NONCHINESE = 0xFFFC;
const int PROXY_TOKEN_SEPARATOR = 0xFFFD;

// these are used on text that is already tokenized
const int PROXY_TOKENIZED = 0xFFFA;
const int PROXY_MORPH = 0xFFFB;

const int PROXY_MARKER_LEN = 3;

#define COPY_MARKER(_ptr,_marker) \
{\
	*_ptr++ = _marker[0]; \
	*_ptr++ = _marker[1]; \
	*_ptr++ = _marker[2]; \
}

#define CMP_MARKER(_ptr, _marker) \
	( _ptr[0]==_marker[0] && _ptr[1]==_marker[1] && _ptr[2]==_marker[2] )

// proxy source
template <class T>
class CSphSource_Proxy : public T
{
public:
	explicit CSphSource_Proxy ( const char * sSourceName )
		: T ( sSourceName )
		, m_dBatchedDocs ( g_iRLPMaxBatchDocs )
		, m_iDocStart ( 0 )
		, m_iDocCount ( 0 )
		, m_pExtraTokenizer ( NULL )
	{
		assert ( sphUTF8Encode ( m_pMarkerDocStart, PROXY_DOCUMENT_START )==PROXY_MARKER_LEN );

		sphUTF8Encode ( m_pMarkerDocStart, PROXY_DOCUMENT_START );
		sphUTF8Encode ( m_pMarkerChineseField, PROXY_FIELD_START_CHINESE );
		sphUTF8Encode ( m_pMarkerNonChineseField, PROXY_FIELD_START_NONCHINESE );
		sphUTF8Encode ( m_pMarkerTokenSeparator, PROXY_TOKEN_SEPARATOR );

		sphUTF8Encode ( m_pMarkerTokenized, PROXY_TOKENIZED );
		sphUTF8Encode ( m_pMarkerMorph, PROXY_MORPH );

		const int INITIAL_BUFFER_SIZE = 1048576;
		m_dDocBuffer.Reserve ( INITIAL_BUFFER_SIZE );
	}

	virtual ~CSphSource_Proxy()
	{
		SafeDelete ( m_pExtraTokenizer );
	}

	void AppendToField ( StoredDoc_t * pCurDoc, int iField, BYTE * pToken, int iTokenLen, BYTE * pMarker )
	{
		assert ( pCurDoc && iField>=0 );
		CSphVector<BYTE> & tStorage = pCurDoc->m_dFieldStorage[iField];

		int iNewSize, iOldSize;
		iNewSize = iOldSize = tStorage.GetLength();
		if ( !iNewSize )
			iNewSize += PROXY_MARKER_LEN + 1;	// tokenized field marker + trailing zero

		iNewSize += iTokenLen+1;				// space before each token + token

		if ( pMarker )
			iNewSize += PROXY_MARKER_LEN;		// non-chinese token marker

		tStorage.Resize ( iNewSize );
		BYTE * pPtr = tStorage.Begin() + ( iOldSize ? iOldSize-1 : 0 );

		if ( !iOldSize )
		{
			memcpy ( pPtr, m_pMarkerTokenized, PROXY_MARKER_LEN );
			pPtr += PROXY_MARKER_LEN;
		}

		*pPtr++ = ' ';

		if ( pMarker )
			COPY_MARKER ( pPtr, pMarker );

		memcpy ( pPtr, pToken, iTokenLen+1 );
		pCurDoc->m_dFields[iField] = tStorage.Begin();
	}

	virtual BYTE ** NextDocument ( CSphString & sError )
	{
		ISphTokenizer * pEmbeddedTokenizer = T::m_pTokenizer->GetEmbeddedTokenizer();
		assert ( pEmbeddedTokenizer );

		if ( !m_pExtraTokenizer )
		{
			m_pExtraTokenizer = ISphTokenizer::CreateRLPFilter ( pEmbeddedTokenizer->Clone ( SPH_CLONE_INDEX ), true, g_sRLPRoot.cstr(),
				g_sRLPEnv.cstr(), T::m_pTokenizer->GetRLPContext(), false, sError );
			if ( !m_pExtraTokenizer )
				return NULL;
		}

		if ( !IsDocCacheEmpty() )
			return CopyDoc();

		if ( m_dFieldLengths.GetLength()!=T::m_tSchema.m_dFields.GetLength() )
			m_dFieldLengths.Resize ( T::m_tSchema.m_dFields.GetLength() );

		char szTmp [256];

		m_iDocStart = 0;
		int iCurDoc = 0;

		m_dDocBuffer.Resize(0);

		while ( !IsDocCacheFull() && m_dDocBuffer.GetLength() < g_iRLPMaxBatchSize )
		{
			BYTE ** pFields = T::NextDocument ( sError );
			if ( !pFields )
				break;

			int iTotalFieldLen = 0;
			for ( int i = 0; i < T::m_tSchema.m_dFields.GetLength(); i++ )
			{
				m_dFieldLengths[i] = strlen ( (const char*)pFields[i] );
				iTotalFieldLen += PROXY_MARKER_LEN+m_dFieldLengths[i]+2;
			}

			const int MAX_INDEX_LEN = 8;
			int iOldBufferLen = m_dDocBuffer.GetLength();
			m_dDocBuffer.Resize ( iOldBufferLen+PROXY_MARKER_LEN+MAX_INDEX_LEN+2+iTotalFieldLen );
			BYTE * pCurDocPtr = &(m_dDocBuffer[iOldBufferLen]);

			StoredDoc_t * pDoc = PushDoc();
			int nFields = T::m_tSchema.m_dFields.GetLength();
			CopyDocInfo ( pDoc->m_tDocInfo, T::m_tDocInfo );
			pDoc->m_dMva = T::m_dMva;
			pDoc->m_dStrAttrs = T::m_dStrAttrs;
			pDoc->m_dFields.Resize ( nFields );
			pDoc->m_dFieldStorage.Resize ( nFields );
			pDoc->m_dChinese.Resize ( nFields );
			pDoc->m_dNonChineseTokens.Resize ( 0 );

			// document start tag
			COPY_MARKER ( pCurDocPtr, m_pMarkerDocStart );

			// space
			*pCurDocPtr++ = ' ';

			// index in plain text
			int iLen = snprintf ( szTmp, sizeof(szTmp), "%d", iCurDoc );
			iLen = iLen>=0 ? iLen : sizeof(szTmp);
			memcpy ( pCurDocPtr, szTmp, iLen );
			pCurDocPtr += iLen;

			// space
			*pCurDocPtr++ = ' ';

			for ( int i = 0; i < T::m_tSchema.m_dFields.GetLength(); i++ )
			{
				int iFieldLen = m_dFieldLengths[i];
				pDoc->m_dChinese[i] = sphDetectChinese ( pFields[i], iFieldLen );

				if ( !pDoc->m_dChinese[i] )
				{
					// no chinese? just save the field storage without tokenizing it
					// it will be tokenized later in the splitter
					pDoc->m_dFieldStorage[i].Resize ( iFieldLen+1 );
					memcpy ( pDoc->m_dFieldStorage[i].Begin(), pFields[i], iFieldLen+1 );
					pDoc->m_dFields[i] = pDoc->m_dFieldStorage[i].Begin();

					COPY_MARKER ( pCurDocPtr, m_pMarkerNonChineseField );
					*pCurDocPtr++ = ' ';
				} else
				{
					COPY_MARKER ( pCurDocPtr, m_pMarkerChineseField );
					*pCurDocPtr++ = ' ';

					pEmbeddedTokenizer->SetBuffer ( pFields[i], iFieldLen );
					BYTE * pToken;
					while ( ( pToken = pEmbeddedTokenizer->GetToken() )!=NULL )
					{
						int iTokenLen = strlen ( (const char*)pToken );
						if ( sphDetectChinese ( pToken, iTokenLen ) )
						{
							// collect it in one big chinese token buffer that will be processed by RLP
							memcpy ( pCurDocPtr, pToken, iTokenLen );
							pCurDocPtr += iTokenLen;
						} else
						{
							// drop it into "non-chinese" token vector
							CSphVector<BYTE> & tChinese = pDoc->m_dNonChineseTokens.Add();
							tChinese.Resize ( iTokenLen+1 );
							memcpy ( tChinese.Begin(), pToken, iTokenLen+1 );

							// add a 'non-chinese token' marker to the chinese token stream
							*pCurDocPtr++ = ' ';
							COPY_MARKER ( pCurDocPtr, m_pMarkerTokenSeparator );
						}

						*pCurDocPtr++ = ' ';
					}
				}
			}

			m_dDocBuffer.Resize ( pCurDocPtr-m_dDocBuffer.Begin() );
			iCurDoc++;
		}

		if ( IsDocCacheEmpty() )
			return NULL;

		m_pExtraTokenizer->SetBuffer ( m_dDocBuffer.Begin(), m_dDocBuffer.GetLength() );
		BYTE * pToken;

		StoredDoc_t * pCurDoc = NULL;
		bool bIndexNext = false;
		int iField = -1;
		int iStoredToken = 0;
		while ( ( pToken = m_pExtraTokenizer->GetToken() )!=NULL )
		{
			bool bSpecial = false;
			int iTokenLen = strlen ( (const char *)pToken );
			if ( bIndexNext )
			{
				int iDoc = atoi ( (const char*)pToken );
				pCurDoc = &(m_dBatchedDocs[iDoc]);
				bIndexNext = false;
				iField = -1;
				iStoredToken = 0;
			} else
			{
				if ( iTokenLen==PROXY_MARKER_LEN )
				{
					if ( CMP_MARKER ( pToken, m_pMarkerDocStart ) )
					{
						bIndexNext = true;
						bSpecial = true;
					} else if ( CMP_MARKER ( pToken, m_pMarkerChineseField ) )
					{
						assert ( pCurDoc );
						iField++;
						pCurDoc->m_dFieldStorage[iField].Resize(0);
						pCurDoc->m_dFields[iField] = pCurDoc->m_dFieldStorage[iField].Begin();
						bSpecial = true;
					} else if ( CMP_MARKER ( pToken, m_pMarkerNonChineseField ) )
					{
						iField++;
						bSpecial = true;
					} else if ( CMP_MARKER ( pToken, m_pMarkerTokenSeparator ) )
					{
						// copy stored non-chinese token
						AppendToField ( pCurDoc, iField, pCurDoc->m_dNonChineseTokens[iStoredToken].Begin(), pCurDoc->m_dNonChineseTokens[iStoredToken].GetLength()-1, m_pMarkerMorph );
						iStoredToken++;
						bSpecial = true;
					}
				}

				// simple token; append to current field
				if ( !bSpecial )
					AppendToField ( pCurDoc, iField, pToken, iTokenLen, NULL );
			}
		}

		return CopyDoc ();
	}

private:
	CSphSource_Document *	m_pSource;
	CSphFixedVector<StoredDoc_t> m_dBatchedDocs;
	CSphVector<BYTE>		m_dDocBuffer;
	CSphVector<int>			m_dFieldLengths;
	int						m_iDocStart;
	int						m_iDocCount;
	ISphTokenizer *			m_pExtraTokenizer;

	BYTE					m_pMarkerDocStart[PROXY_MARKER_LEN];
	BYTE					m_pMarkerChineseField[PROXY_MARKER_LEN];
	BYTE					m_pMarkerNonChineseField[PROXY_MARKER_LEN];
	BYTE					m_pMarkerTokenSeparator[PROXY_MARKER_LEN];

	BYTE					m_pMarkerTokenized[PROXY_MARKER_LEN];
	BYTE					m_pMarkerMorph[PROXY_MARKER_LEN];

	bool					IsDocCacheEmpty() const	{ return !m_iDocCount; }
	bool					IsDocCacheFull() const { return m_iDocCount==m_dBatchedDocs.GetLength(); }

	StoredDoc_t * PushDoc()
	{
		assert ( !IsDocCacheFull() );

		int iEnd = (m_iDocStart+m_iDocCount) % m_dBatchedDocs.GetLength();
		m_iDocCount++;

		return &(m_dBatchedDocs[iEnd]);
	}

	StoredDoc_t * PopDoc()
	{
		assert ( !IsDocCacheEmpty() );

		StoredDoc_t * pDoc = &(m_dBatchedDocs[m_iDocStart]);
		m_iDocStart = (m_iDocStart+1) % m_dBatchedDocs.GetLength();
		m_iDocCount--;
		return pDoc;
	}

	BYTE ** CopyDoc ()
	{
		StoredDoc_t * pDoc = PopDoc();
		CopyDocInfo ( T::m_tDocInfo, pDoc->m_tDocInfo );
		T::m_tState.m_dFields = pDoc->m_dFields.Begin();
		T::m_dMva.SwapData ( pDoc->m_dMva );
		T::m_dStrAttrs.SwapData ( pDoc->m_dStrAttrs );

		return T::m_tState.m_dFields;
	}

	void CopyDocInfo ( CSphMatch & tTo, const CSphMatch & tFrom )
	{
		if ( tFrom.m_pDynamic )
		{
			int iDynamic = T::m_tSchema.GetRowSize();

			if ( !tTo.m_pDynamic )
				tTo.Reset ( iDynamic );

			memcpy ( tTo.m_pDynamic, tFrom.m_pDynamic, iDynamic*sizeof(CSphRowitem) );
		}

		tTo.m_pStatic = NULL;
		tTo.m_iDocID = tFrom.m_iDocID;
		tTo.m_iWeight = tFrom.m_iWeight;
		tTo.m_iTag = tFrom.m_iTag;
	}
};

#endif // USE_RLP

#endif // _sphinxint_

//
// $Id$
//
