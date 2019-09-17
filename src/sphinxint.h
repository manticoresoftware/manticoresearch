//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
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
#include "sphinxstd.h"
#include "sphinxfilter.h"
#include "sphinxquery.h"
#include "sphinxexcerpt.h"
#include "sphinxudf.h"
#include "sphinxjsonquery.h"
#include "sphinxutils.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <float.h>

#if USE_WINDOWS
#define stat		_stat64
#define fstat		_fstat64
#if _MSC_VER<1400
#define struct_stat	__stat64
#else
#define struct_stat	struct _stat64
#endif
#else
#define struct_stat        struct stat
#endif


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
#define SPH_O_APPEND ( O_RDWR | O_APPEND | SPH_O_BINARY )

#define MVA_DOWNSIZE		DWORD			// MVA32 offset type

//#define DEFAULT_MAX_MATCHES 1000

// cover on strerror
inline const char * strerrorm ( int errnum )
{
	if (errnum==EMFILE)
		return "Too many open files (on linux see /etc/security/limits.conf, 'ulimit -n', max_open_files config option)";
	return strerror (errnum);
}

// used as bufer size in num-to-string conversions
#define SPH_MAX_NUMERIC_STR 64

//////////////////////////////////////////////////////////////////////////

const DWORD		INDEX_MAGIC_HEADER			= 0x58485053;		///< my magic 'SPHX' header
const DWORD		INDEX_FORMAT_VERSION		= 58;				///< my format version

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
ISphBinlog *					GetBinlog();

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
	SPH_QUERY_STATE ( FULLSCAN,		"fullscan" ) \
	SPH_QUERY_STATE ( DICT_SETUP,	"dict_setup" ) \
	SPH_QUERY_STATE ( PARSE,		"parse" ) \
	SPH_QUERY_STATE ( TRANSFORMS,	"transforms" ) \
	SPH_QUERY_STATE ( INIT,			"init" ) \
	SPH_QUERY_STATE ( INIT_SEGMENT,	"init_segment" ) \
	SPH_QUERY_STATE ( OPEN,			"open" ) \
	SPH_QUERY_STATE ( READ_DOCS,	"read_docs" ) \
	SPH_QUERY_STATE ( READ_HITS,	"read_hits" ) \
	SPH_QUERY_STATE ( GET_DOCS,		"get_docs" ) \
	SPH_QUERY_STATE ( GET_HITS,		"get_hits" ) \
	SPH_QUERY_STATE ( FILTER,		"filter" ) \
	SPH_QUERY_STATE ( RANK,			"rank" ) \
	SPH_QUERY_STATE ( QCACHE_UP,	"qcache_update" ) \
	SPH_QUERY_STATE ( QCACHE_FINAL,	"qcache_final" ) \
	SPH_QUERY_STATE ( SORT,			"sort" ) \
	SPH_QUERY_STATE ( FINALIZE,		"finalize" ) \
	SPH_QUERY_STATE ( DYNAMIC,		"clone_attrs" ) \
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

	virtual void			BuildResult ( XQNode_t * pRoot, const CSphSchema & tSchema, const StrVec_t & dZones ) = 0;


	virtual const char *	GetResultAsStr() const
	{
		assert ( 0 && "Not implemented" );
		return nullptr;
	}

	virtual 				~CSphQueryProfile() {};
};


class CSphScopedProfile
{
private:
	CSphQueryProfile *	m_pProfile;
	ESphQueryState		m_eOldState;

public:
	explicit CSphScopedProfile ( CSphQueryProfile * pProfile, ESphQueryState eNewState )
	{
		m_pProfile = pProfile;
		m_eOldState = SPH_QSTATE_UNKNOWN;
		if ( m_pProfile )
			m_eOldState = m_pProfile->Switch ( eNewState );
	}

	~CSphScopedProfile()
	{
		if ( m_pProfile )
			m_pProfile->Switch ( m_eOldState );
	}
};


/// file writer with write buffering and int encoder
class CSphWriter : ISphNoncopyable
{
public:
	virtual			~CSphWriter ();

	void			SetBufferSize ( int iBufferSize );	///< tune write cache size; must be called before OpenFile() or SetFile()

	bool			OpenFile ( const CSphString & sName, CSphString & sError );
	void			SetFile ( CSphAutofile & tAuto, SphOffset_t * pSharedOffset, CSphString & sError );
	void			CloseFile ( bool bTruncate = false );	///< note: calls Flush(), ie. IsError() might get true after this call
	void			UnlinkFile (); /// some shit happened (outside) and the file is no more actual.

	void			PutByte ( BYTE uValue );
	void			PutBytes ( const void * pData, int64_t iSize );
	void			PutWord ( WORD uValue ) { PutBytes ( &uValue, sizeof(WORD) ); }
	void			PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void			PutOffset ( SphOffset_t uValue ) { PutBytes ( &uValue, sizeof(SphOffset_t) ); }
	void			PutString ( const char * szString );
	void			PutString ( const CSphString & sString );
	void			Tag ( const char * sTag );

	void			SeekTo ( SphOffset_t iPos, bool bTruncate = false );

	void			ZipInt ( DWORD uValue );
	void			ZipOffset ( uint64_t uValue );

	bool			IsError () const	{ return m_bError; }
	SphOffset_t		GetPos () const		{ return m_iPos; }

	virtual void	Flush ();

protected:
	CSphString		m_sName;
	SphOffset_t		m_iPos = -1;
	SphOffset_t		m_iDiskPos = 0;

	int				m_iFD = -1;
	int				m_iPoolUsed = 0;
	BYTE *			m_pBuffer = nullptr;
	BYTE *			m_pPool = nullptr;
	bool			m_bOwnFile = false;
	SphOffset_t	*	m_pSharedOffset = nullptr;
	int				m_iBufferSize = 262144;

	bool			m_bError = false;
	CSphString *	m_pError = nullptr;

private:
	void			UpdatePoolUsed();
};


/// file which closes automatically when going out of scope
class CSphAutofile : ISphNoncopyable
{
protected:
	int			m_iFD = -1;					///< my file descriptor
	CSphString	m_sFilename;				///< my file name
	bool		m_bTemporary = false;		///< whether to unlink this file on Close()
	bool		m_bWouldTemporary = false;	///< backup of the m_bTemporary

	CSphIndexProgress *	m_pStat = nullptr;

public:
					CSphAutofile () = default;
					CSphAutofile ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
					~CSphAutofile ();

	int				Open ( const CSphString & sName, int iMode, CSphString & sError, bool bTemp=false );
	void			Close ();
	void			SetTemporary(); ///< would be set if a shit happened and the file is not actual.
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
	CSphQueryProfile *	m_pProfile = nullptr;
	ESphQueryState		m_eProfileState { SPH_QSTATE_IO };

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
	uint64_t	UnzipOffset ();

	bool					GetErrorFlag () const		{ return m_bError; }
	const CSphString &		GetErrorMessage () const	{ return m_sError; }
	const CSphString &		GetFilename() const			{ return m_sFilename; }
	int						GetBufferSize() const		{ return m_iBufSize; }
	void					ResetError();

	RowID_t		UnzipRowid ()	{ return UnzipInt(); }
	SphWordID_t	UnzipWordid ()	{ return UnzipOffset(); }

	CSphReader &	operator = ( const CSphReader & rhs );

protected:

	int			m_iFD = -1;
	SphOffset_t	m_iPos = 0;

	int			m_iBuffPos = 0;
	int			m_iBuffUsed = 0;
	BYTE *		m_pBuff;
	int			m_iSizeHint = 0;	///< how much do we expect to read

	int			m_iBufSize;
	bool		m_bBufOwned = false;
	int			m_iReadUnhinted;

	bool		m_bError = false;
	CSphString	m_sError;
	CSphString	m_sFilename;

protected:
	virtual void		UpdateCache ();
};

/// file reader
class FileReader_c: public CSphReader
{
public:
	explicit FileReader_c ( BYTE* pBuf = nullptr, int iSize = 0 )
		: CSphReader ( pBuf, iSize )
	{}

	SphOffset_t GetFilesize () const;

	// added for DebugCheck()
	int GetFD () const { return m_iFD; }
};

/// scoped file reader
class CSphAutoreader : public FileReader_c
{
public:
	CSphAutoreader ( BYTE * pBuf=nullptr, int iSize=0 ) : FileReader_c ( pBuf, iSize ) {}
	~CSphAutoreader () override { Close(); }

	bool		Open ( const CSphString & sFilename, CSphString & sError );
	void		Close ();
};

class MemoryReader_c
{
public:
	MemoryReader_c ( const BYTE * pData, int iLen )
		: m_pData ( pData )
		, m_iLen ( iLen )
		, m_pCur ( pData )
	{}

	int GetPos()
	{
		return ( m_pCur - m_pData );
	}

	void SetPos ( int iOff )
	{
		assert ( iOff>=0 && iOff<=m_iLen );
		m_pCur = m_pData + iOff;
	}

	uint64_t UnzipOffset();
	DWORD UnzipInt();

	CSphString GetString()
	{
		CSphString sRes;
		DWORD iLen = GetDword();
		if ( iLen )
		{
			sRes.Reserve ( iLen );
			GetBytes ( (BYTE *)sRes.cstr(), iLen );
		}

		return sRes;
	}

	DWORD GetDword()
	{
		DWORD uRes = 0;
		GetBytes ( &uRes, sizeof(uRes) );
		return uRes;
	}

	WORD GetWord()
	{
		WORD uRes = 0;
		GetBytes ( &uRes, sizeof(uRes) );
		return uRes;
	}

	void GetBytes ( void * pData, int iLen )
	{
		if ( !iLen )
			return;

		assert ( m_pCur );
		assert ( m_pCur<m_pData+m_iLen );
		assert ( m_pCur+iLen<=m_pData+m_iLen );
		memcpy ( pData, m_pCur, iLen );
		m_pCur += iLen;
	}

	BYTE GetByte()
	{
		BYTE uVal = 0;
		GetBytes ( &uVal, sizeof(uVal) );
		return uVal;
	}

	uint64_t GetUint64()
	{
		uint64_t uVal;
		GetBytes ( &uVal, sizeof(uVal) );
		return uVal;
	}

	const BYTE * Begin() const
	{
		return m_pData;
	}

protected:
	const BYTE * m_pData = nullptr;
	const int m_iLen = 0;
	const BYTE * m_pCur = nullptr;
};

class MemoryWriter_c
{
public:
	MemoryWriter_c ( CSphVector<BYTE> & dBuf )
		: m_dBuf ( dBuf )
	{}

	int GetPos()
	{
		return m_dBuf.GetLength();
	}

	void ZipOffset ( uint64_t uVal );
	void ZipInt ( DWORD uVal );

	void PutString ( const CSphString & sVal )
	{
		int iLen = sVal.Length();
		PutDword ( iLen );
		if ( iLen )
			PutBytes ( (const BYTE *)sVal.cstr(), iLen );
	}

	void PutString ( const char * sVal )
	{
		int iLen = 0;
		if ( sVal )
			iLen = strlen ( sVal );
		PutDword ( iLen );
		if ( iLen )
			PutBytes ( (const BYTE *)sVal, iLen );
	}

	void PutDword ( DWORD uVal )
	{
		PutBytes ( (BYTE *)&uVal, sizeof(uVal) );
	}

	void PutWord ( WORD uVal )
	{
		PutBytes ( (BYTE *)&uVal, sizeof(uVal) );
	}

	void PutBytes ( const void * pData, int iLen )
	{
		if ( !iLen )
			return;

		BYTE * pCur = m_dBuf.AddN ( iLen );
		memcpy ( pCur, pData, iLen );
	}

	void PutByte ( BYTE uVal )
	{
		m_dBuf.Add ( uVal );
	}

	void PutUint64 ( uint64_t uVal )
	{
		PutBytes ( (BYTE *)&uVal, sizeof(uVal) );
	}

protected:
	CSphVector<BYTE> & m_dBuf;
};

// fixme: get rid of this
class MemoryReader2_c : public MemoryReader_c
{
public:
	MemoryReader2_c ( const BYTE * pData, int iLen )
		: MemoryReader_c ( pData, iLen )
	{}

	uint64_t UnzipInt() { return sphUnzipInt(m_pCur); }
	uint64_t UnzipOffset() { return sphUnzipOffset(m_pCur); }
};

// fixme: get rid of this
class MemoryWriter2_c : public MemoryWriter_c
{
public:
	MemoryWriter2_c ( CSphVector<BYTE> & dBuf )
		: MemoryWriter_c ( dBuf )
	{}

	void ZipOffset ( uint64_t uVal ) { sphZipValue ( uVal, (MemoryWriter_c*)this, &MemoryWriter_c::PutByte ); }
	void ZipInt ( DWORD uVal ) { sphZipValue ( uVal, (MemoryWriter_c*)this, &MemoryWriter_c::PutByte ); }
};


namespace sph
{
	int rename ( const char * sOld, const char * sNew );
}

namespace sph
{
	int rename ( const char * sOld, const char * sNew );
}

class DebugCheckReader_i
{
public:
	virtual ~DebugCheckReader_i () {};
	virtual int64_t GetLengthBytes () = 0;
	virtual bool GetBytes ( void * pData, int iSize ) = 0;
	virtual bool SeekTo ( int64_t iOff, int iHint ) = 0;
};

// common code for debug checks
class DebugCheckHelper_c
{
protected:
	void				DebugCheck_Attributes ( DebugCheckReader_i & tAttrs, DebugCheckReader_i & tBlobs, int64_t nRows, int64_t iMinMaxBytes, const CSphSchema & tSchema, DebugCheckError_c & tReporter );
	void				DebugCheck_DeadRowMap (  int64_t iSizeBytes, int64_t nRows, DebugCheckError_c & tReporter ) const;
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

	EXTRA_GET_QUEUE_WORST,
	EXTRA_GET_QUEUE_SORTVAL,
	EXTRA_GET_LAST_INSERT_ID,

	EXTRA_SET_BLOBPOOL,
	EXTRA_SET_POOL_CAPACITY,
	EXTRA_SET_MATCHPUSHED,
	EXTRA_SET_MATCHPOPPED,

	EXTRA_SET_RANKER_PLUGIN,
	EXTRA_SET_RANKER_PLUGIN_OPTS,

	EXTRA_GET_POOL_SIZE
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


class ISphRanker;
class ISphMatchSorter;
class UservarIntSet_c;

enum QueryDebug_e
{
	QUERY_DEBUG_NO_PAYLOAD = 1<<0
};


/// per-query search context
/// everything that index needs to compute/create to process the query
class CSphQueryContext : public ISphNoncopyable
{
public:
	// searching-only, per-query
	const CSphQuery &			m_tQuery;

	int							m_iWeights = 0;					///< search query field weights count
	int							m_dWeights [ SPH_MAX_FIELDS ];	///< search query field weights

	DWORD						m_uPackedFactorFlags { SPH_FACTOR_DISABLE }; ///< whether we need to calculate packed factors (and some extra options)

	ISphFilter *				m_pFilter = nullptr;
	ISphFilter *				m_pWeightFilter = nullptr;

	bool						m_bSkipQCache = false;			///< whether do not cache this query

	struct CalcItem_t
	{
		CSphAttrLocator			m_tLoc;					///< result locator
		ESphAttr				m_eType { SPH_ATTR_NONE};	///< result type
		CSphRefcountedPtr<ISphExpr>	m_pExpr;		///< evaluator (non-owned)
	};
	CSphVector<CalcItem_t>		m_dCalcFilter;			///< items to compute for filtering
	CSphVector<CalcItem_t>		m_dCalcSort;			///< items to compute for sorting/grouping
	CSphVector<CalcItem_t>		m_dCalcFinal;			///< items to compute when finalizing result set

	const void *							m_pIndexData = nullptr;	///< backend specific data
	CSphQueryProfile *						m_pProfile = nullptr;
	const SmallStringHash_T<int64_t> *		m_pLocalDocs = nullptr;
	int64_t									m_iTotalDocs = 0;
	int64_t									m_iBadRows = 0;

	const IndexSegment_c *					m_pIndexSegment {nullptr};	// intended for docid -> rowid lookups

public:
	explicit CSphQueryContext ( const CSphQuery & q );
	~CSphQueryContext ();

	void	BindWeights ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sWarning );
	bool	SetupCalc ( CSphQueryResult * pResult, const ISphSchema & tInSchema, const CSphSchema & tSchema, const BYTE * pBlobPool, const CSphVector<const ISphSchema *> & dInSchemas );
	bool	CreateFilters ( CreateFilterContext_t &tCtx, CSphString &sError, CSphString &sWarning );

	void	CalcFilter ( CSphMatch & tMatch ) const;
	void	CalcSort ( CSphMatch & tMatch ) const;
	void	CalcFinal ( CSphMatch & tMatch ) const;

	void	FreeDataFilter ( CSphMatch & tMatch ) const;
	void	FreeDataSort ( CSphMatch & tMatch ) const;

	// note that RT index bind pools at segment searching, not at time it setups context
	void	ExprCommand ( ESphExprCommand eCmd, void * pArg );
	void	SetBlobPool ( const BYTE * pBlobPool );
	void	SetupExtraData ( ISphRanker * pRanker, ISphMatchSorter * pSorter );
	void	ResetFilters();

private:
	CSphVector<const UservarIntSet_c*>		m_dUserVals;
};


CSphVector<const ISphSchema *> SorterSchemas ( ISphMatchSorter ** ppSorters, int iCount, int iSkipSorter );

//////////////////////////////////////////////////////////////////////////
// MEMORY TRACKER
//////////////////////////////////////////////////////////////////////////

#define MEM_CATEGORIES \
	MEM_CATEGORY(MEM_CORE), \
	MEM_CATEGORY(MEM_INDEX_DISK), \
	MEM_CATEGORY(MEM_INDEX_RT), \
	MEM_CATEGORY(MEM_API_HANDLE ), \
	MEM_CATEGORY(MEM_API_SEARCH ), \
	MEM_CATEGORY(MEM_API_QUERY ), \
	MEM_CATEGORY(MEM_RT_ACCUM), \
	MEM_CATEGORY(MEM_MMAPED), \
	MEM_CATEGORY(MEM_BINLOG), \
	MEM_CATEGORY(MEM_SQL_HANDLE), \
	MEM_CATEGORY(MEM_SQL_INSERT), \
	MEM_CATEGORY(MEM_SQL_SELECT), \
	MEM_CATEGORY(MEM_SQL_DELETE), \
	MEM_CATEGORY(MEM_SQL_SET), \
	MEM_CATEGORY(MEM_SQL_BEGIN), \
	MEM_CATEGORY(MEM_SQL_COMMIT), \
	MEM_CATEGORY(MEM_SQL_ALTER), \
	MEM_CATEGORY(MEM_DISK_QUERY), \
	MEM_CATEGORY(MEM_DISK_QUERYEX), \
	MEM_CATEGORY(MEM_RT_QUERY), \
	MEM_CATEGORY(MEM_RT_RES_MATCHES), \
	MEM_CATEGORY(MEM_RT_RES_STRINGS)

#define MEM_CATEGORY(_arg) _arg
enum MemCategory_e
{
	MEM_CATEGORIES,
	MEM_TOTAL
};
#undef MEM_CATEGORY

#if SPH_ALLOCS_PROFILER

void sphMemStatPush ( MemCategory_e eCategory );
void sphMemStatPop ( MemCategory_e eCategory );

// memory tracker
struct MemTracker_c : ISphNoncopyable
{
	const MemCategory_e m_eCategory; ///< category

	/// ctor
	explicit MemTracker_c ( MemCategory_e eCategory )
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

#define MEMORY(name) MemTracker_c tracker_##__LINE__##name(name);

#else // SPH_ALLOCS_PROFILER 0

#define MEMORY(name)

#endif // if SPH_ALLOCS_PROFILER

//////////////////////////////////////////////////////////////////////////
// BLOCK-LEVEL ATTRIBUTE INDEX BUILDER
//////////////////////////////////////////////////////////////////////////

#define DOCINFO_INDEX_FREQ 128 // FIXME? make this configurable

inline int64_t MVA_UPSIZE ( const DWORD * pMva )
{
#if USE_LITTLE_ENDIAN
	return *(int64_t*)pMva;
#else
	int64_t iMva = (int64_t)( (uint64_t)pMva[0] | ( ( (uint64_t)pMva[1] )<<32 ) );
	return iMva;
#endif
}

template < typename T >
bool MvaEval_Any ( const T * pMva, int nValues, const SphAttr_t * pFilter, int nFilterValues )
{
	if ( !pMva || !pFilter )
		return false;

	const T * pMvaMax = pMva+nValues;
	const SphAttr_t * pFilterMax = pFilter+nFilterValues;

	const T * L = pMva;
	const T * R = pMvaMax;
	R--;
	for ( ; pFilter < pFilterMax; pFilter++ )
	{
		while ( L<=R )
		{
			const T * pVal = L + (R - L) / 2;
			T iValue = sphUnalignedRead ( *pVal );
			if ( *pFilter > iValue )
				L = pVal + 1;
			else if ( *pFilter < iValue )
				R = pVal - 1;
			else
				return true;
		}
		R = (const T *)pMvaMax;
		R--;
	}

	return false;
}


template < typename T >
bool MvaEval_All ( const T * pMva, int nValues, const SphAttr_t * pFilter, int nFilterValues )
{
	if ( !pMva || !pFilter )
		return false;

	const T * pMvaMax = pMva+nValues;
	const SphAttr_t * pFilterMax = pFilter+nFilterValues;

	for ( const T * pVal = pMva; pVal<pMvaMax; pVal++ )
	{
		const SphAttr_t iCheck = sphUnalignedRead ( *pVal );
		if ( !sphBinarySearch ( pFilter, pFilterMax-1, iCheck ) )
			return false;
	}

	return true;
}


// FIXME!!! for over INT_MAX attributes
/// attr min-max builder
class AttrIndexBuilder_c : ISphNoncopyable
{
public:
	explicit	AttrIndexBuilder_c ( const CSphSchema & tSchema );

	void		Collect ( const CSphRowitem * pRow );
	void		FinishCollect();
	const CSphTightVector<CSphRowitem> & GetCollected() const;

private:
	CSphVector<CSphAttrLocator>	m_dIntAttrs;
	CSphVector<CSphAttrLocator>	m_dFloatAttrs;

	CSphVector<SphAttr_t>		m_dIntMin;
	CSphVector<SphAttr_t>		m_dIntMax;
	CSphVector<float>			m_dFloatMin;
	CSphVector<float>			m_dFloatMax;

	CSphVector<SphAttr_t>		m_dIntIndexMin;
	CSphVector<SphAttr_t>		m_dIntIndexMax;
	CSphVector<float>			m_dFloatIndexMin;
	CSphVector<float>			m_dFloatIndexMax;

	DWORD						m_uStride {0};
	int							m_nLocalCollected {0};

	CSphTightVector<CSphRowitem> m_dMinMaxRows;

	void						ResetLocal();
	void						FlushComputed();
};


class Docstore_c;
struct PoolPtrs_t
{
	const DocstoreReader_i * m_pDocstore = nullptr;
};


class TaggedVector_c
{
public:
	PoolPtrs_t & operator [] ( int iTag ) const
	{
		return m_dPool [ iTag & 0x7FFFFFF ];
	}

	void Resize ( int iSize )
	{
		m_dPool.Resize ( iSize );
	}

private:
	CSphVector<PoolPtrs_t> m_dPool;
};


class CSphFreeList
{
private:
	CSphTightVector<int>	m_dFree;
	int						m_iNextFree = 0;
#ifndef NDEBUG
	int						m_iSize = 0;
#endif

public:

	void Reset ( int iSize )
	{
#ifndef NDEBUG
		m_iSize = iSize;
#endif
		m_iNextFree = 0;
		m_dFree.Reserve ( iSize );
	}

	int Get ()
	{
		if ( !m_dFree.IsEmpty() )
			return m_dFree.Pop ();
		return m_iNextFree++;
	}

	void Free ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iSize );
		m_dFree.Add ( iIndex );
	}
};

//////////////////////////////////////////////////////////////////////////
// INLINES, FIND_XXX() GENERIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////

/// find a value-enclosing span in a sorted vector (aka an index at which vec[i] <= val < vec[i+1])
template < typename T, typename U >
static int FindSpan ( const VecTraits_T<T> & dVec, U tRef, int iSmallTreshold=8 )
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


inline int sphEncodeVLB8 ( BYTE * buf, uint64_t v )
{
	register BYTE b;
	register int n = 0;

	do
	{
		b = (BYTE)(v & 0x7f);
		v >>= 7;
		if ( v )
			b |= 0x80;
		*buf++ = b;
		n++;
	} while ( v );
	return n;
}


inline const BYTE * spnDecodeVLB8 ( const BYTE * pIn, uint64_t & uValue )
{
	BYTE bIn;
	int iOff = 0;

	do
	{
		bIn = *pIn++;
		uValue += ( uint64_t ( bIn & 0x7f ) ) << iOff;
		iOff += 7;
	} while ( bIn & 0x80 );

	return pIn;
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
	}

	if ( iCode<0x800 )
	{
		pBuf[0] = (BYTE)( ( (iCode>>6) & 0x1F ) | 0xC0 );
		pBuf[1] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 2;
	}

	if ( iCode<0x10000 )
	{
		pBuf[0] = (BYTE)( ( (iCode>>12) & 0x0F ) | 0xE0 );
		pBuf[1] = (BYTE)( ( (iCode>>6) & 0x3F ) | 0x80 );
		pBuf[2] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
		return 3;
	}

	pBuf[0] = (BYTE)( ( (iCode>>18) & 0x0F ) | 0xF0 );
	pBuf[1] = (BYTE)( ( (iCode>>12) & 0x3F ) | 0x80 );
	pBuf[2] = (BYTE)( ( (iCode>>6) & 0x3F ) | 0x80 );
	pBuf[3] = (BYTE)( ( iCode & 0x3F ) | 0x80 );
	return 4;
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

/// quick check for UTF-8
inline bool sphIsUTF8 ( const char * pStr )
{
	while ( *pStr )
	{
		if ( *pStr < 0 )
			return true;
		pStr++;
	}
	return false;
}

/// convert UTF-8 to codepoints, return string length
inline int sphUTF8ToWideChar ( const char * pSrc, int * pDst, int iMaxLen )
{
	const BYTE * p = (const BYTE*) pSrc;
	int iLen = 0, iCode;
	while ( ( iCode = sphUTF8Decode(p) )!=0 && iLen<iMaxLen )
	{
		*pDst++ = iCode;
		iLen++;
	}
	*pDst = 0;
	return iLen;
}

#define sphStrMatchStatic(_str, _cstr) ( strncmp ( _str, _cstr, sizeof(_str)-1 )==0 )

//////////////////////////////////////////////////////////////////////////
// MATCHING ENGINE INTERNALS
//////////////////////////////////////////////////////////////////////////

static const int FIELD_BITS = 8;
typedef Hitman_c<FIELD_BITS> HITMAN;

/// hit in the stream
/// combines posting info (docid and hitpos) with a few more matching/ranking bits
///
/// note that while in simple cases every hit would just represent a single keyword,
/// this is NOT always the case; phrase, proximity, and NEAR operators (that already
/// analyze keywords positions while matching the document) can emit a single folded
/// hit representing the entire multi-keyword match, so that the ranker could avoid
/// double work processing individual hits again. in such cases, m_uWeight, m_uSpanlen,
/// and m_uMatchlen will differ from the "usual" value of 1.
///
/// thus, in folded hits:
/// - m_uWeight is the match LCS value in all cases (phrase, proximity, near).
/// - m_uSpanlen is the match span length, ie. a distance from the first to the last
/// matching keyword. for phrase operators it natually equals m_uWeight, for other
/// operators it might be very different.
/// - m_uMatchlen is a piece of voodoo magic that only the near operator seems to use.
struct ExtHit_t
{
	RowID_t		m_tRowID;
	Hitpos_t	m_uHitpos;
	WORD		m_uQuerypos;
	WORD		m_uNodepos;
	WORD		m_uSpanlen;
	WORD		m_uMatchlen;
	DWORD		m_uWeight;		///< 1 for individual keywords, LCS value for folded phrase/proximity/near hits
	DWORD		m_uQposMask;
};

/// match in the stream
struct ExtDoc_t
{
	RowID_t			m_tRowID;
	DWORD			m_uDocFields;
	float			m_fTFIDF;
};

struct ZoneHits_t
{
	CSphVector<Hitpos_t>	m_dStarts;
	CSphVector<Hitpos_t>	m_dEnds;
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
	virtual SphZoneHit_e IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan ) = 0;
};


struct SphFactorHashEntry_t
{
	RowID_t					m_tRowID;
	int						m_iRefCount;
	BYTE *					m_pData;
	SphFactorHashEntry_t *	m_pPrev;
	SphFactorHashEntry_t *	m_pNext;
};

typedef CSphFixedVector<SphFactorHashEntry_t *> SphFactorHash_t;


struct SphExtraDataRankerState_t
{
	const CSphSchema *	m_pSchema = nullptr;
	const int64_t *		m_pFieldLens = nullptr;
	CSphAttrLocator		m_tFieldLensLoc;
	int64_t				m_iTotalDocuments = 0;
	int					m_iFields = 0;
	int					m_iMaxQpos = 0;
};


struct MatchSortAccessor_t
{
	using T = CSphMatch;
	using MEDIAN_TYPE = T *;

private:
	mutable CSphMatch m_tMedian;

public:
	MatchSortAccessor_t () = default;
	MatchSortAccessor_t ( const MatchSortAccessor_t & ) {}

	virtual ~MatchSortAccessor_t()
	{
		m_tMedian.m_pDynamic = nullptr; // not yours
	}

	MEDIAN_TYPE Key ( CSphMatch * a ) const
	{
		return a;
	}

	void CopyKey ( MEDIAN_TYPE * pMed, CSphMatch * pVal ) const
	{
		*pMed = &m_tMedian;
		m_tMedian.m_tRowID = pVal->m_tRowID;
		m_tMedian.m_iWeight = pVal->m_iWeight;
		m_tMedian.m_pStatic = pVal->m_pStatic;
		m_tMedian.m_pDynamic = pVal->m_pDynamic;
		m_tMedian.m_iTag = pVal->m_iTag;
	}

	void Swap ( T * a, T * b ) const
	{
		::Swap ( *a, *b );
	}

	T * Add ( T * p, int i ) const
	{
		return p+i;
	}

	int Sub ( T * b, T * a ) const
	{
		return (int)(b-a);
	}
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
		case SPH_ATTR_BOOL:			return "bool";
		case SPH_ATTR_FLOAT:		return "float";
		case SPH_ATTR_BIGINT:		return "bigint";
		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:	return "string"; // not 'stringptr' since it used to be human-readable
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
		case SPH_ATTR_BOOL:			return "sql_attr_bool";
		case SPH_ATTR_FLOAT:		return "sql_attr_float";
		case SPH_ATTR_BIGINT:		return "sql_attr_bigint";
		case SPH_ATTR_STRING:		return "sql_attr_string";
		case SPH_ATTR_STRINGPTR:	return "sql_attr_string";
		case SPH_ATTR_TOKENCOUNT:	return "_autogenerated_tokencount";
		case SPH_ATTR_JSON:			return "sql_attr_json";

		case SPH_ATTR_UINT32SET:	return "sql_attr_multi";
		case SPH_ATTR_INT64SET:		return "sql_attr_multi bigint";
		default:					return "???";
	}
}

inline const char * sphRtTypeDirective ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_INTEGER:		return "rt_attr_uint";
		case SPH_ATTR_TIMESTAMP:	return "rt_attr_timestamp";
		case SPH_ATTR_BOOL:			return "rt_attr_bool";
		case SPH_ATTR_FLOAT:		return "rt_attr_float";
		case SPH_ATTR_BIGINT:		return "rt_attr_bigint";
		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:	return "rt_attr_string";
		case SPH_ATTR_JSON:			return "rt_attr_json";

		case SPH_ATTR_UINT32SET:	return "rt_attr_multi";
		case SPH_ATTR_INT64SET:		return "rt_attr_multi64";
		default:					return nullptr;
	}
}

inline void SqlUnescape ( CSphString & sRes, const char * sEscaped, int iLen )
{
	assert ( iLen>=2 );
	assert (
		( sEscaped[0]=='\'' && sEscaped[iLen - 1]=='\'' ) ||
			( sEscaped[0]=='"' && sEscaped[iLen - 1]=='"' ) );

	// skip heading and trailing quotes
	const char * s = sEscaped + 1;
	const char * sMax = s + iLen - 2;

	sRes.Reserve ( iLen );
	auto * d = ( char * ) sRes.cstr ();

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
			case '0': *d++ = ' '; break;
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
	for ( ; *s; ++s )
		if ( *s=='/' )
			sLastSlash = s;

	auto iPos = (int)( sLastSlash - sPath.cstr() + 1 );
	auto iLen = (int)( s - sPath.cstr() );
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
	explicit			CSphDictTraits ( CSphDict * pDict ) : m_pDict { pDict } { SafeAddRef ( pDict ); }

	void		LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile ) final { m_pDict->LoadStopwords ( sFiles, pTokenizer, bStripFile ); }
	void		LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) final { m_pDict->LoadStopwords ( dStopwords ); }
	void		WriteStopwords ( CSphWriter & tWriter ) const final { m_pDict->WriteStopwords ( tWriter ); }
	bool		LoadWordforms ( const StrVec_t & dFiles, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex ) final { return m_pDict->LoadWordforms ( dFiles, pEmbedded, pTokenizer, sIndex ); }
	void		WriteWordforms ( CSphWriter & tWriter ) const final { m_pDict->WriteWordforms ( tWriter ); }
	int			SetMorphology ( const char * szMorph, CSphString & sMessage ) final { return m_pDict->SetMorphology ( szMorph, sMessage ); }

	SphWordID_t	GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final { return m_pDict->GetWordID ( pWord, iLen, bFilterStops ); }
	SphWordID_t GetWordID ( BYTE * pWord ) override;
	SphWordID_t	GetWordIDNonStemmed ( BYTE * pWord ) override { return m_pDict->GetWordIDNonStemmed ( pWord ); }

	void		Setup ( const CSphDictSettings & ) final {}
	const CSphDictSettings & GetSettings () const final { return m_pDict->GetSettings (); }
	const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () const final { return m_pDict->GetStopwordsFileInfos (); }
	const CSphVector <CSphSavedFile> & GetWordformsFileInfos () const final { return m_pDict->GetWordformsFileInfos (); }
	const CSphMultiformContainer * GetMultiWordforms () const final { return m_pDict->GetMultiWordforms (); }
	const CSphWordforms * GetWordforms () final { return m_pDict->GetWordforms(); }

	bool		IsStopWord ( const BYTE * pWord ) const final { return m_pDict->IsStopWord ( pWord ); }
	uint64_t	GetSettingsFNV () const final { return m_pDict->GetSettingsFNV(); }

protected:
	DictRefPtr_c	m_pDict;
};


/// dict wrapper for star-syntax support in prefix-indexes
class CSphDictStar : public CSphDictTraits
{
public:
	explicit	CSphDictStar ( CSphDict * pDict ) : CSphDictTraits ( pDict ) {}

	SphWordID_t	GetWordID ( BYTE * pWord ) override;
	SphWordID_t	GetWordIDNonStemmed ( BYTE * pWord ) override;
};


/// star dict for index v.8+
class CSphDictStarV8 : public CSphDictStar
{
	bool m_bInfixes;
public:
	CSphDictStarV8 ( CSphDict * pDict, bool bInfixes ) : CSphDictStar ( pDict ), m_bInfixes ( bInfixes )
	{}
	SphWordID_t	GetWordID ( BYTE * pWord ) final;
};


/// dict wrapper for exact-word syntax
class CSphDictExact : public CSphDictTraits
{
public:
	explicit CSphDictExact ( CSphDict * pDict ) : CSphDictTraits ( pDict ) {}
	SphWordID_t	GetWordID ( BYTE * pWord ) final;
	SphWordID_t GetWordIDNonStemmed ( BYTE * pWord ) final { return m_pDict->GetWordIDNonStemmed ( pWord ); }
};

void RemoveDictSpecials ( CSphString & sWord );
const CSphString & RemoveDictSpecials ( const CSphString & sWord, CSphString & sBuf );

//////////////////////////////////////////////////////////////////////////
// TOKEN FILTER
//////////////////////////////////////////////////////////////////////////

/// token filter base (boring proxy stuff)
class CSphTokenFilter : public ISphTokenizer
{
protected:
	TokenizerRefPtr_c		m_pTokenizer;

public:
	explicit						CSphTokenFilter ( ISphTokenizer * pTokenizer )					: m_pTokenizer ( pTokenizer ) {	SafeAddRef ( pTokenizer ); }


	bool					SetCaseFolding ( const char * sConfig, CSphString & sError ) override	{ return m_pTokenizer->SetCaseFolding ( sConfig, sError ); }
	void					AddPlainChar ( char c ) override											{ m_pTokenizer->AddPlainChar ( c ); }
	void					AddSpecials ( const char * sSpecials ) override							{ m_pTokenizer->AddSpecials ( sSpecials ); }
	bool					SetIgnoreChars ( const char * sIgnored, CSphString & sError ) override	{ return m_pTokenizer->SetIgnoreChars ( sIgnored, sError ); }
	bool					SetNgramChars ( const char * sConfig, CSphString & sError ) override		{ return m_pTokenizer->SetNgramChars ( sConfig, sError ); }
	void					SetNgramLen ( int iLen ) override										{ m_pTokenizer->SetNgramLen ( iLen ); }
	bool					LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, CSphString & sError ) override { return m_pTokenizer->LoadSynonyms ( sFilename, pFiles, sError ); }
	void					WriteSynonyms ( CSphWriter & tWriter ) const final						{ return m_pTokenizer->WriteSynonyms ( tWriter ); }
	bool					SetBoundary ( const char * sConfig, CSphString & sError ) override		{ return m_pTokenizer->SetBoundary ( sConfig, sError ); }
	void					Setup ( const CSphTokenizerSettings & tSettings ) override				{ m_pTokenizer->Setup ( tSettings ); }
	const CSphTokenizerSettings &	GetSettings () const override									{ return m_pTokenizer->GetSettings (); }
	const CSphSavedFile &	GetSynFileInfo () const override										{ return m_pTokenizer->GetSynFileInfo (); }
	bool					EnableSentenceIndexing ( CSphString & sError ) override					{ return m_pTokenizer->EnableSentenceIndexing ( sError ); }
	bool					EnableZoneIndexing ( CSphString & sError ) override						{ return m_pTokenizer->EnableZoneIndexing ( sError ); }
	int						SkipBlended () override													{ return m_pTokenizer->SkipBlended(); }

	int						GetCodepointLength ( int iCode ) const final		{ return m_pTokenizer->GetCodepointLength ( iCode ); }
	int						GetMaxCodepointLength () const final				{ return m_pTokenizer->GetMaxCodepointLength(); }

	const char *			GetTokenStart () const override						{ return m_pTokenizer->GetTokenStart(); }
	const char *			GetTokenEnd () const override						{ return m_pTokenizer->GetTokenEnd(); }
	const char *			GetBufferPtr () const override						{ return m_pTokenizer->GetBufferPtr(); }
	const char *			GetBufferEnd () const final							{ return m_pTokenizer->GetBufferEnd (); }
	void					SetBufferPtr ( const char * sNewPtr ) override		{ m_pTokenizer->SetBufferPtr ( sNewPtr ); }
	uint64_t				GetSettingsFNV () const override						{ return m_pTokenizer->GetSettingsFNV(); }

	void					SetBuffer ( const BYTE * sBuffer, int iLength ) override	{ m_pTokenizer->SetBuffer ( sBuffer, iLength ); }
	BYTE *					GetToken () override										{ return m_pTokenizer->GetToken(); }

	bool					WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const override { return m_pTokenizer->WasTokenMultiformDestination ( bHead, iDestCount ); }
};

DWORD sphParseMorphAot ( const char * );

struct CSphReconfigureSettings
{
	CSphTokenizerSettings	m_tTokenizer;
	CSphDictSettings		m_tDict;
	CSphIndexSettings		m_tIndex;
	CSphFieldFilterSettings m_tFieldFilter;
	CSphSchema				m_tSchema;
};

struct CSphReconfigureSetup
{
	TokenizerRefPtr_c	m_pTokenizer;
	DictRefPtr_c		m_pDict;
	CSphIndexSettings		m_tIndex;
	FieldFilterRefPtr_c	m_pFieldFilter;
	CSphSchema			m_tSchema;
};

uint64_t sphGetSettingsFNV ( const CSphIndexSettings & tSettings );

//////////////////////////////////////////////////////////////////////////
// USER VARIABLES
//////////////////////////////////////////////////////////////////////////

/// value container for the intset uservar type
class UservarIntSet_c : public CSphVector<SphAttr_t>, public ISphRefcountedMT
{
};

extern UservarIntSet_c * ( *g_pUservarsHook )( const CSphString & sUservar );

//////////////////////////////////////////////////////////////////////////
// BINLOG INTERNALS
//////////////////////////////////////////////////////////////////////////

struct StoredQueryDesc_t;

/// global binlog interface
class ISphBinlog : ISphNoncopyable
{
public:
	virtual				~ISphBinlog () {}

	virtual void		BinlogUpdateAttributes ( int64_t * pTID, const char * sIndexName, const CSphAttrUpdate & tUpd ) = 0;
	virtual void		NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown ) = 0;

	virtual void		BinlogReconfigure ( int64_t * pTID, const char * sIndexName, const CSphReconfigureSetup & tSetup ) = 0;
	virtual bool		IsActive () = 0; // i.e. binlog is not disabled, say, by empty path.

	virtual void		BinlogPqAdd ( int64_t * pTID, const char * sIndexName, const StoredQueryDesc_t & tStored ) = 0;
	virtual void		BinlogPqDelete ( int64_t * pTID, const char * sIndexName, const int64_t * pQueries, int iCount, const char * sTags ) = 0;
};

//////////////////////////////////////////////////////////////////////////
// MISC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////

struct SphStringSorterRemap_t
{
	CSphAttrLocator m_tSrc;
	CSphAttrLocator m_tDst;
};

const BYTE *	SkipQuoted ( const BYTE * p );

bool			sphSortGetStringRemap ( const ISphSchema & tSorterSchema, const ISphSchema & tIndexSchema, CSphVector<SphStringSorterRemap_t> & dAttrs );
bool			IsSortStringInternal ( const CSphString& sColumnName );
/// make string lowercase but keep case of JSON.field
void			sphColumnToLowercase ( char * sVal );
bool			IsSortJsonInternal ( const CSphString& sColumnName );
CSphString		SortJsonInternalSet ( const CSphString& sColumnName );

bool			sphCheckQueryHeight ( const struct XQNode_t * pRoot, CSphString & sError );
void			sphTransformExtendedQuery ( XQNode_t ** ppNode, const CSphIndexSettings & tSettings, bool bHasBooleanOptimization, const ISphKeywordsStat * pKeywords );
void			TransformAotFilter ( XQNode_t * pNode, const CSphWordforms * pWordforms, const CSphIndexSettings& tSettings );
bool			sphMerge ( const CSphIndex * pDst, const CSphIndex * pSrc, CSphString & sError, CSphIndexProgress & tProgress, volatile bool * pLocalStop, bool bSrcSettings );
CSphString		sphReconstructNode ( const XQNode_t * pNode, const CSphSchema * pSchema );
int				ExpandKeywords ( int iIndexOpt, QueryOption_e eQueryOpt, const CSphIndexSettings & tSettings );
bool			ParseMorphFields ( const CSphString & sMorphology, const CSphString & sMorphFields, const CSphVector<CSphColumnInfo> & dFields, CSphBitvec & tMorphFields, CSphString & sError );

void			sphSetUnlinkOld ( bool bUnlink );
bool			sphGetUnlinkOld ();
void			sphUnlinkIndex ( const char * sName, bool bForce );

void			WriteSchema ( CSphWriter & fdInfo, const CSphSchema & tSchema );
void			ReadSchema ( CSphReader & rdInfo, CSphSchema & m_tSchema, DWORD uVersion );
void			SaveIndexSettings ( CSphWriter & tWriter, const CSphIndexSettings & tSettings );
void			LoadIndexSettings ( CSphIndexSettings & tSettings, CSphReader & tReader, DWORD uVersion );
void			SaveTokenizerSettings ( CSphWriter & tWriter, const ISphTokenizer * pTokenizer, int iEmbeddedLimit );
bool			LoadTokenizerSettings ( CSphReader & tReader, CSphTokenizerSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning );
void			SaveDictionarySettings ( CSphWriter & tWriter, const CSphDict * pDict, bool bForceWordDict, int iEmbeddedLimit );
void			LoadDictionarySettings ( CSphReader & tReader, CSphDictSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning );
void			LoadFieldFilterSettings ( CSphReader & tReader, CSphFieldFilterSettings & tFieldFilterSettings );
void			SaveFieldFilterSettings ( CSphWriter & tWriter, const ISphFieldFilter * pFieldFilter );

bool			AddFieldLens ( CSphSchema & tSchema, bool bDynamic, CSphString & sError );

/// Get current thread local index - internal do not use
class RtIndex_i;
RtIndex_i * sphGetCurrentIndexRT();

void			RebalanceWeights ( const CSphFixedVector<int64_t> & dTimers, CSphFixedVector<float>& pWeights );

// FIXME!!! remove with converter
const char * CheckFmtMagic ( DWORD uHeader );
void ReadFileInfo ( CSphReader & tReader, const char * szFilename, bool bSharedStopwords, CSphSavedFile & tFile, CSphString * sWarning );
void SaveFieldFilterSettings ( CSphWriter & tWriter, const CSphFieldFilterSettings & tSettings );
bool WriteKillList ( const CSphString & sFilename, const DocID_t * pKlist, int nEntries, const CSphVector<KillListTarget_t> & dTargets, CSphString & sError );
void WarnAboutKillList ( const CSphVector<DocID_t> & dKillList, const CSphVector<KillListTarget_t> & dTargets );
extern const char * g_sTagInfixBlocks;
extern const char * g_sTagInfixEntries;

template < typename VECTOR >
static int sphPutBytes ( VECTOR * pOut, const void * pData, int iLen )
{
	int iOff = pOut->GetLength();
	pOut->Resize ( iOff + iLen );
	memcpy ( pOut->Begin()+iOff, pData, iLen );
	return iOff;
}

// all indexes should produce same terms for same query
struct SphWordStatChecker_t
{
	void Set ( const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hStat );
	void DumpDiffer ( const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hStat, const char * sIndex, CSphString & sWarning ) const;

	CSphVector<uint64_t> m_dSrcWords;
};


enum ESphExt
{
	SPH_EXT_SPH,
	SPH_EXT_SPA,
	SPH_EXT_SPB,
	SPH_EXT_SPI,
	SPH_EXT_SPD,
	SPH_EXT_SPP,
	SPH_EXT_SPK,
	SPH_EXT_SPE,
	SPH_EXT_SPM,
	SPH_EXT_SPT,
	SPH_EXT_SPHI,
	SPH_EXT_SPDS,
	SPH_EXT_SPL,

	SPH_EXT_TOTAL
};


struct IndexFileExt_t
{
	ESphExt			m_eExt;
	const char *	m_szExt;
	DWORD			m_uMinVer;
	bool			m_bOptional;
	bool			m_bCopy;	// file needs to be copied
	const char *	m_szDesc;
};


CSphVector<IndexFileExt_t>	sphGetExts();
CSphString					sphGetExt ( ESphExt eExt );

/// encapsulates all common actions over index files in general (copy/rename/delete etc.)
class IndexFiles_c : public ISphNoncopyable
{
	DWORD		m_uVersion = INDEX_FORMAT_VERSION;
	CSphString	m_sIndexName;	// used for information purposes (logs)
	CSphString	m_sFilename;	// prefix (i.e. folder + index name, excluding extensions)
	CSphString	m_sLastError;
	bool		m_bFatal = false; // if fatal fail happened (unable to rename during rollback)
	CSphString FullPath ( const char * sExt, const char * sSuffix = "", const char * sBase = nullptr );
	inline void SetName ( CSphString sIndex ) { m_sIndexName = std::move(sIndex); }

public:
	IndexFiles_c() = default;
	explicit IndexFiles_c ( CSphString sBase, const char* sIndex=nullptr, DWORD uVersion = INDEX_FORMAT_VERSION )
		: m_uVersion ( uVersion )
		, m_sFilename { std::move(sBase) }
	{
		if ( sIndex )
			SetName ( sIndex );
	}

	inline void SetBase ( const CSphString & sNewBase )
	{
		m_sFilename = sNewBase;
	}

	inline const CSphString& GetBase() const { return m_sFilename; }

	inline const char * ErrorMsg () const { return m_sLastError.cstr(); }
	inline bool IsFatal() const { return m_bFatal; }

	// read .sph and adopt index version from there.
	bool CheckHeader ( const char * sType="" );

	// read the beginning of .spk and parse killlist targets
	bool ReadKlistTargets ( StrVec_t & dTargets, const char * sType="" );

	DWORD GetVersion() const { return m_uVersion; }

	// simple make decorated path, like '.old' -> /path/to/index.old
	CSphString MakePath ( const char * sSuffix = "", const char * sBase = nullptr );

	bool Rename ( const char * sFrom, const char * sTo );  // move files between different bases
	bool Rollback ( const char * sBackup, const char * sPath ); // move from backup to path; fail is fatal
	bool RenameSuffix ( const char * sFromSuffix, const char * sToSuffix="" );  // move files in base between two suffixes
	bool RenameBase ( const char * sToBase );  // move files to different base
	bool RenameLock ( const char * sTo, int & iLockFD ); // safely move lock file
	bool RelocateToNew ( const char * sNewBase ); // relocate to new base and append '.new' suffix
	bool RollbackSuff ( const char * sBackupSuffix, const char * sActiveSuffix="" ); // move from backup to active; fail is fatal
	bool HasAllFiles ( const char * sType = "" ); // check that all necessary files are readable
	void Unlink ( const char * sType = "" );

	// if prev op fails with fatal error - log the message and terminate
	CSphString FatalMsg(const char * sMsg=nullptr);
};

int sphDictCmp ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 );
int sphDictCmpStrictly ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 );

template <typename CP>
int sphCheckpointCmp ( const char * sWord, int iLen, SphWordID_t iWordID, bool bWordDict, const CP & tCP )
{
	if ( bWordDict )
		return sphDictCmp ( sWord, iLen, tCP.m_sWord, strlen ( tCP.m_sWord ) );

	int iRes = 0;
	iRes = iWordID<tCP.m_uWordID ? -1 : iRes;
	iRes = iWordID>tCP.m_uWordID ? 1 : iRes;
	return iRes;
}

template <typename CP>
int sphCheckpointCmpStrictly ( const char * sWord, int iLen, SphWordID_t iWordID, bool bWordDict, const CP & tCP )
{
	if ( bWordDict )
		return sphDictCmpStrictly ( sWord, iLen, tCP.m_sWord, strlen ( tCP.m_sWord ) );

	int iRes = 0;
	iRes = iWordID<tCP.m_uWordID ? -1 : iRes;
	iRes = iWordID>tCP.m_uWordID ? 1 : iRes;
	return iRes;
}

template < typename CP >
struct SphCheckpointAccess_fn
{
	const CP & operator () ( const CP * pCheckpoint ) const { return *pCheckpoint; }
};

template < typename CP, typename PRED >
const CP * sphSearchCheckpoint ( const char * sWord, int iWordLen, SphWordID_t iWordID
							, bool bStarMode, bool bWordDict
							, const CP * pFirstCP, const CP * pLastCP, const PRED & tPred )
{
	assert ( !bWordDict || iWordLen>0 );

	const CP * pStart = pFirstCP;
	const CP * pEnd = pLastCP;

	if ( bStarMode && sphCheckpointCmp ( sWord, iWordLen, iWordID, bWordDict, tPred ( pStart ) )<0 )
		return NULL;
	if ( !bStarMode && sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, tPred ( pStart ) )<0 )
		return NULL;

	if ( sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, tPred ( pEnd ) )>=0 )
		pStart = pEnd;
	else
	{
		while ( pEnd-pStart>1 )
		{
			const CP * pMid = pStart + (pEnd-pStart)/2;
			const int iCmpRes = sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, tPred ( pMid ) );

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
		assert ( sphCheckpointCmp ( sWord, iWordLen, iWordID, bWordDict, tPred ( pStart ) )>=0
			&& sphCheckpointCmpStrictly ( sWord, iWordLen, iWordID, bWordDict, tPred ( pEnd ) )<0 );
	}

	return pStart;
}

template < typename CP >
const CP * sphSearchCheckpoint ( const char * sWord, int iWordLen, SphWordID_t iWordID, bool bStarMode, bool bWordDict , const CP * pFirstCP, const CP * pLastCP )
{
	return sphSearchCheckpoint ( sWord, iWordLen, iWordID, bStarMode, bWordDict, pFirstCP, pLastCP, SphCheckpointAccess_fn<CP>() );
}


int sphCollateLibcCI ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 );
int sphCollateLibcCS ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 );
int sphCollateUtf8GeneralCI ( const BYTE * pArg1, const BYTE * pArg2, bool bDataPtr, int iLen1, int iLen2 );
int sphCollateBinary ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 );

class ISphRtDictWraper : public CSphDict
{
protected:
	~ISphRtDictWraper() override {}
public:
	virtual const BYTE *	GetPackedKeywords () = 0;
	virtual int				GetPackedLen () = 0;

	virtual void			ResetKeywords() = 0;

	virtual const char *	GetLastWarning() const = 0;
	virtual void			ResetWarning() = 0;
};

ISphRtDictWraper * sphCreateRtKeywordsDictionaryWrapper ( CSphDict * pBase );

struct SphExpanded_t
{
	int m_iNameOff;
	int m_iDocs;
	int m_iHits;
};

struct ISphSubstringPayload
{
	virtual ~ISphSubstringPayload() {}
	int m_iTotalDocs = 0;
	int m_iTotalHits = 0;
};


void sphBuildNGrams ( const char * sWord, int iLen, char cDelimiter, CSphVector<char> & dNgrams );

// levenstein distance for words
int sphLevenshtein ( const char * sWord1, int iLen1, const char * sWord2, int iLen2 );

// levenstein distance for unicode codepoints
int sphLevenshtein ( const int * sWord1, int iLen1, const int * sWord2, int iLen2 );

struct Slice_t
{
	DWORD				m_uOff = 0;
	DWORD				m_uLen;
};

struct SuggestWord_t
{
	int	m_iNameOff;
	int m_iLen;
	int m_iDistance;
	int m_iDocs;
	DWORD m_iNameHash;
};

struct SuggestArgs_t
{
	int				m_iLimit;			// limit into result set
	int				m_iMaxEdits;		// levenstein distance threshold
	int				m_iDeltaLen;		// filter out words from dictionary these shorter \ longer then reference word
	int				m_iQueueLen;
	int				m_iRejectThr;
	bool			m_bQueryMode;

	bool			m_bResultOneline;
	bool			m_bResultStats;
	bool			m_bNonCharAllowed;

	SuggestArgs_t () : m_iLimit ( 5 ), m_iMaxEdits ( 4 ), m_iDeltaLen ( 3 ), m_iQueueLen ( 25 ), m_iRejectThr ( 4 ), m_bQueryMode ( false ), m_bResultOneline ( false ), m_bResultStats ( true ), m_bNonCharAllowed ( false )
	{}
};

struct SuggestResult_t
{
	// result set
	CSphVector<BYTE>			m_dBuf;
	CSphVector<SuggestWord_t>	m_dMatched;

	// state
	CSphVector<char>			m_dTrigrams;
	// payload
	void *						m_pWordReader = nullptr;
	void *						m_pSegments = nullptr;
	bool						m_bMergeWords = false;
	// word
	CSphString		m_sWord;
	int				m_iLen = 0;
	int				m_dCodepoints[SPH_MAX_WORD_LEN];
	int				m_iCodepoints = 0;
	bool			m_bUtf8 = false;
	bool			m_bHasExactDict = false;

	SuggestResult_t ()
	{
		m_dBuf.Reserve ( 8096 );
		m_dMatched.Reserve ( 512 );
	}

	~SuggestResult_t ()
	{
		assert ( !m_pWordReader );
		assert ( !m_pSegments );
	}

	bool SetWord ( const char * sWord, const ISphTokenizer * pTok, bool bUseLastWord );

	void Flattern ( int iLimit );
};

class ISphWordlistSuggest
{
public:
	virtual ~ISphWordlistSuggest () {}

	virtual void SuffixGetChekpoints ( const SuggestResult_t & tRes, const char * sSuffix, int iLen, CSphVector<DWORD> & dCheckpoints ) const = 0;

	virtual void SetCheckpoint ( SuggestResult_t & tRes, DWORD iCP ) const = 0;

	struct DictWord_t
	{
		const char *	m_sWord;
		int				m_iLen;
		int				m_iDocs;
	};

	virtual bool ReadNextWord ( SuggestResult_t & tRes, DictWord_t & tWord ) const = 0;
};

void sphGetSuggest ( const ISphWordlistSuggest * pWordlist, int iInfixCodepointBytes, const SuggestArgs_t & tArgs, SuggestResult_t & tRes );


class ISphWordlist
{
public:
	struct Args_t : public ISphNoncopyable
	{
		CSphVector<SphExpanded_t>	m_dExpanded;
		const bool					m_bPayload;
		int							m_iExpansionLimit;
		const bool					m_bHasExactForms;
		const ESphHitless			m_eHitless;

		ISphSubstringPayload *		m_pPayload;
		int							m_iTotalDocs;
		int							m_iTotalHits;
		const void *				m_pIndexData;

		Args_t ( bool bPayload, int iExpansionLimit, bool bHasExactForms, ESphHitless eHitless, const void * pIndexData );
		~Args_t ();
		void AddExpanded ( const BYTE * sWord, int iLen, int iDocs, int iHits );
		const char * GetWordExpanded ( int iIndex ) const;

	private:
		CSphVector<char> m_sBuf;
	};

	virtual ~ISphWordlist () {}
	virtual void GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const = 0;
	virtual void GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const = 0;
};


class CSphScopedPayload
{
public:
	CSphScopedPayload () {}
	~CSphScopedPayload ()
	{
		for ( auto & dPayload : m_dPayloads )
			SafeDelete ( dPayload );
	}
	void Add ( ISphSubstringPayload * pPayload ) { m_dPayloads.Add ( pPayload ); }

private:
	CSphVector<ISphSubstringPayload *> m_dPayloads;
};


struct ExpansionContext_t
{
	const ISphWordlist * m_pWordlist	= nullptr;
	BYTE * m_pBuf						= nullptr;
	CSphQueryResultMeta * m_pResult		= nullptr;
	int m_iMinPrefixLen					= 0;
	int m_iMinInfixLen					= 0;
	int m_iExpansionLimit				= 0;
	bool m_bHasExactForms				= false;
	bool m_bMergeSingles				= false;
	CSphScopedPayload * m_pPayloads		= nullptr;
	ESphHitless m_eHitless				{SPH_HITLESS_NONE};
	const void * m_pIndexData			= nullptr;
};

struct GetKeywordsSettings_t
{
	bool	m_bStats = true;
	bool	m_bFoldLemmas = false;
	bool	m_bFoldBlended = false;
	bool	m_bFoldWildcards = false;
	int		m_iExpansionLimit = 0;
	bool	m_bSortByDocs = false;
	bool	m_bSortByHits = false;
};

struct ISphQueryFilter
{
	TokenizerRefPtr_c		m_pTokenizer;
	CSphDict *					m_pDict = nullptr;
	const CSphIndexSettings *	m_pSettings = nullptr;
	GetKeywordsSettings_t		m_tFoldSettings;

	virtual ~ISphQueryFilter () {}

	void GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const ExpansionContext_t & tCtx );
	virtual void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords ) = 0;
};

XQNode_t * sphExpandXQNode ( XQNode_t * pNode, ExpansionContext_t & tCtx );
XQNode_t * sphQueryExpandKeywords ( XQNode_t * pNode, const CSphIndexSettings & tSettings, int iExpandKeywords );
inline int sphGetExpansionMagic ( int iDocs, int iHits )
{
	return ( iHits<=256 ? 1 : iDocs + 1 ); // magic threshold; mb make this configurable?
}
inline bool sphIsExpandedPayload ( int iDocs, int iHits )
{
	return ( iHits<=256 || iDocs<32 ); // magic threshold; mb make this configurable?
}
bool sphHasExpandableWildcards ( const char * sWord );
bool sphExpandGetWords ( const char * sWord, const ExpansionContext_t & tCtx, ISphWordlist::Args_t & tWordlist );


template<typename T>
struct ExpandedOrderDesc_T
{
	bool IsLess ( const T & a, const T & b ) const
	{
		return ( sphGetExpansionMagic ( a.m_iDocs, a.m_iHits )>sphGetExpansionMagic ( b.m_iDocs, b.m_iHits ) );
	}
};


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

#if USE_WINDOWS
void localtime_r ( const time_t * clock, struct tm * res );
void gmtime_r ( const time_t * clock, struct tm * res );
#endif

struct InfixBlock_t
{
	union
	{
		const char *	m_sInfix;
		DWORD			m_iInfixOffset;
	};
	DWORD				m_iOffset;
};


/// infix hash builder
class ISphInfixBuilder
{
public:
	explicit		ISphInfixBuilder() {}
	virtual			~ISphInfixBuilder() {}
	virtual void	AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint, bool bHasMorphology ) = 0;
	virtual void	SaveEntries ( CSphWriter & wrDict ) = 0;
	virtual int64_t	SaveEntryBlocks ( CSphWriter & wrDict ) = 0;
	virtual int		GetBlocksWordsSize () const = 0;
};


ISphInfixBuilder * sphCreateInfixBuilder ( int iCodepointBytes, CSphString * pError );
bool sphLookupInfixCheckpoints ( const char * sInfix, int iBytes, const BYTE * pInfixes, const CSphVector<InfixBlock_t> & dInfixBlocks, int iInfixCodepointBytes, CSphVector<DWORD> & dCheckpoints );
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

/// parser to build lowercaser from textual config
class CSphCharsetDefinitionParser
{
public:
	bool				Parse ( const char * sConfig, CSphVector<CSphRemapRange> & dRanges );
	const char *		GetLastError ();

protected:
	bool				m_bError = false;
	char				m_sError [ 1024 ];
	const char *		m_pCurrent = nullptr;

	bool				Error ( const char * sMessage );
	void				SkipSpaces ();
	bool				IsEof ();
	bool				CheckEof ();
	int					HexDigit ( int c );
	int					ParseCharsetCode ();
	bool				AddRange ( const CSphRemapRange & tRange, CSphVector<CSphRemapRange> & dRanges );
};


class MatchesToNewSchema_c : public ISphMatchProcessor
{
public:
							MatchesToNewSchema_c ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema );
	void					Process ( CSphMatch * pMatch ) final;

private:
	const ISphSchema *		m_pOldSchema;
	const ISphSchema *		m_pNewSchema;
	CSphVector<CSphAttrLocator>	m_dNewAttrs;
	CSphVector<int>			m_dOld2New;
	CSphVector<SphStringSorterRemap_t> m_dRemapStringCmp;

	virtual const BYTE *	GetBlobPool ( const CSphMatch * pMatch ) = 0;
};


struct StoredToken_t
{
	BYTE			m_sToken [3*SPH_MAX_WORD_LEN+4];
	// tokenized state
	const char *	m_szTokenStart;
	const char *	m_szTokenEnd;
	const char *	m_pBufferPtr;
	const char *	m_pBufferEnd;
	int				m_iTokenLen;
	int				m_iOvershortCount;
	bool			m_bBoundary;
	bool			m_bSpecial;
	bool			m_bBlended;
	bool			m_bBlendedPart;
};

//////////////////////////////////////////////////////////////////////////

void FillStoredTokenInfo ( StoredToken_t & tToken, const BYTE * sToken, ISphTokenizer * pTokenizer );

class CSphConfigSection;
CSphSource * sphCreateSourceTSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName );
CSphSource * sphCreateSourceCSVpipe ( const CSphConfigSection * pSource, FILE * pPipe, const char * sSourceName );

uint64_t sphCalcLocatorHash ( const CSphAttrLocator & tLoc, uint64_t uPrevHash );
uint64_t sphCalcExprDepHash ( const char * szTag, ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable );
uint64_t sphCalcExprDepHash ( ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable );

void sphFixupLocator ( CSphAttrLocator & tLocator, const ISphSchema * pOldSchema, const ISphSchema * pNewSchema );
ISphSchema * sphCreateStandaloneSchema ( const ISphSchema * pSchema );

// internals attributes are last no need to send them
void sphGetAttrsToSend ( const ISphSchema & tSchema, bool bAgentMode, bool bNeedId, CSphBitvec & tAttrs );


inline void FlipEndianess ( DWORD* pData )
{
	BYTE* pB = (BYTE*)pData;
	BYTE a = pB[0];
	pB[0] = pB[3];
	pB[3] = a;
	a = pB[1];
	pB[1] = pB[2];
	pB[2] = a;
};

/// SHA1 digests
static const int HASH20_SIZE = 20;
static const int SHA1_SIZE = HASH20_SIZE;
static const int SHA1_BUF_SIZE = 64;

class SHA1_c
{
public:
	SHA1_c & Init();
	SHA1_c & Update ( const BYTE * pData, int iLen );
	void Final ( BYTE digest[SHA1_SIZE] );

private:
	DWORD state[5], count[2];
	BYTE buffer[SHA1_BUF_SIZE];

	void Transform ( const BYTE buf[SHA1_BUF_SIZE] );
};

CSphString BinToHex ( const VecTraits_T<BYTE> & dHash );
CSphString BinToHex ( const BYTE * pHash, int iLen );
CSphString CalcSHA1 ( const void * pData, int iLen );
bool CalcSHA1 ( const CSphString & sFileName, CSphString & sRes, CSphString & sError );

// string and 20-bytes hash
struct TaggedHash20_t
{
	CSphString m_sTagName;
	BYTE m_dHashValue[HASH20_SIZE] = { 0 };

	// by tag + hash
	explicit TaggedHash20_t ( const char* sTag = nullptr, const BYTE* pHashValue = nullptr );

	// convert to FIPS-180-1
	CSphString ToFIPS() const;

	// load from FIPS-180-1
	int FromFIPS ( const char* sFIPS );

	// compare with raw hash
	bool operator== ( const BYTE * pRef ) const;

	inline bool Empty() const { return *this==m_dZeroHash; }

	// helper zero hash
	static const BYTE m_dZeroHash[HASH20_SIZE];
};

// set of tagged hashes
class HashCollection_c
{
	CSphVector<TaggedHash20_t> m_dHashes;
public:
	void AppendNewHash ( const char* sExt, const BYTE* pHash);

	void /*REFACTOR*/ SaveSHA() {};

};

// file writer with hashing on-the-fly.
class WriterWithHash_c : public CSphWriter
{
public:
	WriterWithHash_c ( const char* sExt, HashCollection_c* pCollector );
	~WriterWithHash_c ();

	virtual void Flush () override;
	void CloseFile ();

	// get resulting BLOB, is valid only after StopHashing()
	//const BYTE * GetHASHBlob () const;

private:
	//void StopHashing ();

	HashCollection_c * m_pCollection;
	const char * m_sExt;

	SHA1_c * m_pHasher;
	bool m_bHashDone = false;
	BYTE m_dHashValue[HASH20_SIZE] = { 0 };
};

struct SchemaItemVariant_t
{
	int m_iField = -1;
	int m_iStr = -1;
	int m_iMva = -1;
	ESphAttr m_eType = SPH_ATTR_NONE;
	CSphAttrLocator m_tLoc;
};

using SchemaItemHash_c = OpenHash_T<SchemaItemVariant_t, uint64_t, HashFunc_Int64_t>;

template <typename T>
BYTE PrereadMapping ( const char * sIndexName, const char * sFor, bool bMlock, bool bOnDisk,
	CSphBufferTrait<T> & tBuf )
{
	static volatile BYTE g_uHash;
	if ( bOnDisk || tBuf.IsEmpty() )
		return g_uHash;

	const BYTE * pCur = (BYTE *)tBuf.GetWritePtr();
	const BYTE * pEnd = (BYTE *)tBuf.GetWritePtr() + tBuf.GetLengthBytes();
	const int iHalfPage = 2048;

	g_uHash = 0xff;
	for ( ; pCur<pEnd; pCur+=iHalfPage )
		g_uHash ^= *pCur;
	g_uHash ^= *(pEnd-1);

	// we want to prevent PrereadMapping() from being aggressively optimized away
	// volatile return values *should* normally achieve that
	CSphString sWarning;
	if ( bMlock && !tBuf.MemLock ( sWarning ) )
		sphWarning ( "index '%s': %s for %s", sIndexName, sWarning.cstr(), sFor );
	return g_uHash;
}

// crash related code
struct CrashQuery_t
{
	const BYTE *	m_pQuery = nullptr;	// last query
	int				m_iSize = 0;		// last query size
	WORD			m_uCMD = 0;			// last command (header)
	WORD			m_uVer = 0;			// last command's version (header)
	bool			m_bMySQL = false;	// is query from MySQL or API
	bool			m_bHttp = false;	// is query from HTTP
	const char *	m_pIndex = nullptr;
	int				m_iIndexLen = 0;
};

void CrashQuerySetTop ( CrashQuery_t * pQuery );
CrashQuery_t CrashQueryGet();
void CrashQuerySet ( const CrashQuery_t & tCrash );

typedef void CrashQuerySetTop_fn ( CrashQuery_t * pQuery );
typedef CrashQuery_t CrashQueryGet_fn();
typedef void CrashQuerySet_fn ( const CrashQuery_t & tCrash );
void CrashQuerySetupHandlers ( CrashQuerySetTop_fn * pSetTop, CrashQueryGet_fn * pGet, CrashQuerySet_fn * pSet );

// atomic seek+read wrapper
int sphPread ( int iFD, void * pBuf, int iBytes, SphOffset_t iOffset );

#endif // _sphinxint_
