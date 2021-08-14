//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

/// @file searchdaemon.h
/// Declarations for the stuff specifically needed by searchd to serve the indexes.


#ifndef _searchdaemon_
#define _searchdaemon_

#include "searchdconfig.h"
#include "memio.h"

/////////////////////////////////////////////////////////////////////////////
// MACHINE-DEPENDENT STUFF
/////////////////////////////////////////////////////////////////////////////

#if _WIN32
	// Win-specific headers and calls
	#include <winsock2.h>

	#define HAVE_POLL 1
	static inline int poll ( struct pollfd *pfd, int nfds, int timeout ) { return WSAPoll ( pfd, nfds, timeout ); }
	#define sphSockRecv(_sock,_buf,_len)	::recv(_sock,_buf,_len,0)
	#define sphSockSend(_sock,_buf,_len)	::send(_sock,_buf,_len,0)
	#define sphSockClose(_sock)				::closesocket(_sock)
	#define SSIZE_T long
		using sphIovec=WSABUF;
	#define IOBUFTYPE (CHAR FAR *)
	#define IOPTR(tX) (tX.buf)
	#define IOLEN(tX) (tX.len)

	// Windows hacks
	#undef EINTR
	#undef EWOULDBLOCK
	#undef ETIMEDOUT
	#undef EINPROGRESS
	#undef ECONNRESET
	#undef ECONNABORTED
	#define LOCK_EX			0
	#define LOCK_UN			1
	#define STDIN_FILENO	fileno(stdin)
	#define STDOUT_FILENO	fileno(stdout)
	#define STDERR_FILENO	fileno(stderr)
	#define ETIMEDOUT		WSAETIMEDOUT
	#define EWOULDBLOCK		WSAEWOULDBLOCK
	#define EINPROGRESS		WSAEINPROGRESS
	#define EINTR			WSAEINTR
	#define ECONNRESET		WSAECONNRESET
	#define ECONNABORTED	WSAECONNABORTED
	#define ESHUTDOWN		WSAESHUTDOWN

	#define ftruncate		_chsize
	#define getpid			GetCurrentProcessId


#else
	// UNIX-specific headers and calls
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <arpa/inet.h>

	#if HAVE_POLL
	#include <poll.h>
	#endif

	#if HAVE_EPOLL
	#include <sys/epoll.h>
	#endif

	// there's no MSG_NOSIGNAL on OS X
	#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL 0
	#endif

	#define sphSockRecv(_sock,_buf,_len)	::recv(_sock,_buf,_len,MSG_NOSIGNAL)
	#define sphSockSend(_sock,_buf,_len)	::send(_sock,_buf,_len,MSG_NOSIGNAL)
	#define sphSockClose(_sock)				::close(_sock)
	#define SSIZE_T ssize_t
	using sphIovec = iovec;
	#define IOBUFTYPE
	#define IOPTR(tX) (tX.iov_base)
	#define IOLEN(tX) (tX.iov_len)
#endif

// declarations for correct work of code analysis
#include "sphinxutils.h"
#include "sphinxint.h"
#include "sphinxrt.h"
#include "task_info.h"
#include "coroutine.h"
#include "conversion.h"

#define SPHINXAPI_PORT            9312
#define SPHINXQL_PORT            9306
#define SPH_ADDRESS_SIZE        sizeof("000.000.000.000")
#define SPH_ADDRPORT_SIZE        sizeof("000.000.000.000:00000")
#define NETOUTBUF                8192

// strict check, sphGetAddress will die with sphFatal() on failure
#define GETADDR_STRICT	true

// these defined in searchd.cpp
void sphFatal( const char* sFmt, ... ) __attribute__ (( format ( printf, 1, 2 ))) NO_RETURN;
void sphFatalLog( const char* sFmt, ... ) __attribute__ (( format ( printf, 1, 2 )));

volatile bool& sphGetGotSighup();
volatile bool& sphGetGotSigusr1();
volatile bool& sphGetGotSigusr2();

/////////////////////////////////////////////////////////////////////////////
// SOME SHARED GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////

// these are in searchd.cpp
extern int g_iReadTimeoutS;        // defined in searchd.cpp
extern int g_iWriteTimeoutS;    // sec

extern int g_iMaxPacketSize;    // in bytes; for both query packets from clients and response packets from agents


static const int64_t S2US = I64C ( 1000000 );

/////////////////////////////////////////////////////////////////////////////
// MISC GLOBALS
/////////////////////////////////////////////////////////////////////////////

/// known commands
/// (shared here because at least SEARCHD_COMMAND_TOTAL used outside the core)
enum SearchdCommand_e : WORD
{
	SEARCHD_COMMAND_SEARCH		= 0,
	SEARCHD_COMMAND_EXCERPT		= 1,
	SEARCHD_COMMAND_UPDATE		= 2,
	SEARCHD_COMMAND_KEYWORDS	= 3,
	SEARCHD_COMMAND_PERSIST		= 4,
	SEARCHD_COMMAND_STATUS		= 5,
	SEARCHD_COMMAND_FLUSHATTRS	= 7,
	SEARCHD_COMMAND_SPHINXQL	= 8,
	SEARCHD_COMMAND_PING		= 9,
	SEARCHD_COMMAND_DELETE		= 10,
	SEARCHD_COMMAND_UVAR		= 11,
	SEARCHD_COMMAND_INSERT		= 12,
	SEARCHD_COMMAND_REPLACE		= 13,
	SEARCHD_COMMAND_COMMIT		= 14,
	SEARCHD_COMMAND_SUGGEST		= 15,
	SEARCHD_COMMAND_JSON		= 16,
	SEARCHD_COMMAND_CALLPQ 		= 17,
	SEARCHD_COMMAND_CLUSTERPQ	= 18,
	SEARCHD_COMMAND_GETFIELD	= 19,

	SEARCHD_COMMAND_TOTAL,
	SEARCHD_COMMAND_WRONG = SEARCHD_COMMAND_TOTAL,
};

const char* szCommand ( int );

/// master-agent API SEARCH command protocol extensions version
enum
{
	VER_COMMAND_SEARCH_MASTER = 18
};


/// known command versions
/// (shared here because of REPLICATE)
enum SearchdCommandV_e : WORD
{
	VER_COMMAND_SEARCH		= 0x121, // 1.33
	VER_COMMAND_EXCERPT		= 0x104,
	VER_COMMAND_UPDATE		= 0x104,
	VER_COMMAND_KEYWORDS	= 0x101,
	VER_COMMAND_STATUS		= 0x101,
	VER_COMMAND_FLUSHATTRS	= 0x100,
	VER_COMMAND_SPHINXQL	= 0x100,
	VER_COMMAND_JSON		= 0x100,
	VER_COMMAND_PING		= 0x100,
	VER_COMMAND_UVAR		= 0x100,
	VER_COMMAND_CALLPQ		= 0x100,
	VER_COMMAND_CLUSTERPQ	= 0x104,
	VER_COMMAND_GETFIELD	= 0x100,

	VER_COMMAND_WRONG = 0,
};

enum UpdateType_e
{
	UPDATE_INT		= 0,
	UPDATE_MVA32	= 1,
	UPDATE_STRING	= 2,
	UPDATE_JSON		= 3
};

enum ESphAddIndex
{
	ADD_ERROR	= 0, // wasn't added because of config or other error
	ADD_DSBLED	= 1, // added into disabled hash (need to prealloc/preload, etc)
	ADD_DISTR	= 2, // distributed
	ADD_SERVED	= 3, // added and active (can be used in queries)
//	ADD_CLUSTER	= 4,
};

struct ListenerDesc_t
{
	Proto_e m_eProto = Proto_e::UNKNOWN;
	CSphString m_sUnix;
	DWORD m_uIP = 0;
	int m_iPort = 0;
	int m_iPortsCount = 0;
	bool m_bVIP = false;
};

// 'like' matcher
class CheckLike
{
private:
	CSphString m_sPattern;

public:
	explicit CheckLike( const char* sPattern );
	bool Match( const char* sValue );
};

using Generator_fn = std::function<CSphString ( void )>;
using GeneratorS_fn = std::function<StringBuilder_c ( void )>;

// string vector with 'like' matcher
class VectorLike: public StrVec_t, public CheckLike
{
	StrVec_t	m_dHeadNames;

	void FillTail (int iHas);

public:
	explicit VectorLike ( int iCols = 2 );
	explicit VectorLike ( const CSphString & sPattern ); // default 2 columns, { "Variable_name", "Value" }
	explicit VectorLike ( const CSphString & sPattern, int iCols );
	explicit VectorLike ( const CSphString & sPattern, std::initializer_list<const char*> sCols);

	void SetColNames ( std::initializer_list<const char *> sCols );
	void SetColName ( CSphString sValue, int iIdx = 0 );
	const VecTraits_T<CSphString> & Header () const;

	// returns true, if single value matches
	bool MatchAdd( const char* sValue );
	bool MatchAddf ( const char* sTemplate, ... ) __attribute__ (( format ( printf, 2, 3 )));

	// unconditionally add formatted
	void Addf ( const char * sValueTmpl, ... );

	// add pair
	void MatchTuplet ( const char * sKey, const char * sValue );
	void MatchTupletf ( const char * sKey, const char* sValueTmpl, ... ); 	// __attribute__ ((format ( printf,3, 4 )));
	void MatchTupletFn ( const char * sKey, Generator_fn && fnValuePrinter );
	void MatchTupletFn ( const char * sKey, GeneratorS_fn && fnValuePrinter );
};

CSphString GetTypeName ( IndexType_e eType );
IndexType_e TypeOfIndexConfig ( const CSphString & sType );

// forwards from searchd
bool CheckPort( int iPort, CSphString * pFatal=nullptr );
ListenerDesc_t ParseListener ( const char * sSpec, CSphString * pFatal=nullptr );

/////////////////////////////////////////////////////////////////////////////
// NETWORK SOCKET WRAPPERS
/////////////////////////////////////////////////////////////////////////////

const char* sphSockError( int = 0 );
int sphSockGetErrno();
void sphSockSetErrno( int );
int sphSockPeekErrno();
int sphSetSockNB( int );
void sphSetSockNodelay ( int );
void sphSetSockReuseAddr ( int );
void sphSetSockReusePort ( int );
void sphSetSockTFO ( int );
int sphPoll( int iSock, int64_t tmTimeout, bool bWrite = false );

/** \brief wrapper over getaddrinfo
Invokes getaddrinfo for given host which perform name resolving (dns).
 \param[in] sHost the host name
 \param[in] bFatal whether failed resolving is fatal (false by default)
 \param[in] bIP set true if sHost contains IP address (false by default)
 so no potentially lengthy network host address lockup necessary
 \return ipv4 address as DWORD, to be directly used as s_addr in connect(). */
DWORD sphGetAddress( const char* sHost, bool bFatal = false, bool bIP = false, CSphString * pFatal=nullptr );
char* sphFormatIP( char* sBuffer, int iBufferSize, DWORD uAddress );
CSphString GetMacAddress();

bool IsPortInRange( int iPort );

/////////////////////////////////////////////////////////////////////////////
// NETWORK BUFFERS
/////////////////////////////////////////////////////////////////////////////

/// dynamic send buffer
/// to remove ISphNoncopyable just add copy c-tor and operator=
/// we just cache all streamed data into internal blob;
/// NO data sending itself lives in this class.

class ISphOutputBuffer
{
public:
	ISphOutputBuffer ();
	explicit ISphOutputBuffer ( CSphVector<BYTE>& dChunk );
	virtual ~ISphOutputBuffer() {}

	void		SendInt ( int iValue )			{ SendT<int> ( htonl ( iValue ) ); }

	void		SendAsDword ( int64_t iValue ) ///< sends the 32bit MAX_UINT if the value is greater than it.
	{
		iValue = Min ( Max ( iValue, 0 ), UINT_MAX );
		SendDword ( DWORD(iValue) );
	}
	void		SendDword ( DWORD iValue )		{ SendT<DWORD> ( htonl ( iValue ) ); }
	void		SendWord ( WORD iValue )		{ SendT<WORD> ( htons ( iValue ) ); }
	void		SendFloat ( float fValue )		{ SendT<DWORD> ( htonl ( sphF2DW ( fValue ) ) ); }
	void		SendByte ( BYTE uValue )		{ SendT<BYTE> ( uValue ); }

	void SendLSBDword ( DWORD v )
	{
#if USE_LITTLE_ENDIAN
		SendT <DWORD> (v);
#else
		SendByte ( (BYTE)( v & 0xff ) );
		SendByte ( (BYTE)( (v>>8) & 0xff ) );
		SendByte ( (BYTE)( (v>>16) & 0xff ) );
		SendByte ( (BYTE)( (v>>24) & 0xff) );
#endif
	}

	void SendUint64 ( uint64_t uValue )
	{
		SendT<DWORD> ( htonl ( (DWORD)( uValue>>32) ) );
		SendT<DWORD> ( htonl ( (DWORD)( uValue & 0xffffffffUL) ) );
	}

	void SendUint64 ( int64_t iValue )
	{
		SendUint64 ( (uint64_t) iValue );
	}

	void		SendBytes ( const void * pBuf, int iLen );	///< (was) protected to avoid network-vs-host order bugs
	void		SendBytes ( const char * pBuf );    // used strlen() to get length
	void		SendBytes ( const CSphString& sStr );    // used strlen() to get length
	void		SendBytes ( const VecTraits_T<BYTE>& dBuf );
	void		SendBytes ( const StringBuilder_c& dBuf );
	void		SendBytes ( ByteBlob_t dData );

	// send array: first length(int), then byte blob
	void		SendString ( const char * sStr );
	void		SendArray ( const ISphOutputBuffer &tOut );
	void		SendArray ( const VecTraits_T<BYTE> &dBuf, int iElems=-1 );
	void		SendArray ( ByteBlob_t dData );
	void		SendArray ( const void * pBuf, int iLen );
	void		SendArray ( const StringBuilder_c &dBuf );

	void		SwapData ( CSphVector<BYTE> & rhs ) { m_dBuf.SwapData ( rhs ); }

	int				GetSentCount () const { return m_dBuf.GetLength(); }
	void *			GetBufPtr () const { return (char*) m_dBuf.begin();}

	void WriteInt ( int64_t iOff, int iValue ) // int in network file order
	{
		WriteT<int> ( iOff, htonl ( iValue ) );
	}

	BYTE* ReservePlace ( int64_t iPlace )
	{
		auto pRes = m_dBuf.AddN ( (int)iPlace );
		m_dBuf.Resize ( m_dBuf.Idx(pRes));
		return pRes;
	}

	void CommitZeroCopy ( int64_t iSize )
	{
		m_dBuf.AddN ( (int)iSize );
	}

	void Rewind ( int64_t iPos )
	{
		m_dBuf.Resize ( iPos );
	}

	CSphVector<BYTE>	m_dBuf;

	template < typename T > void WriteT ( int64_t iOff, T tValue )
	{
		sphUnalignedWrite ( m_dBuf.Begin () + iOff, tValue );
	}

private:
	template < typename T > void	SendT ( T tValue )							///< (was) protected to avoid network-vs-host order bugs
	{
		int64_t iOff = m_dBuf.GetLength64();
		m_dBuf.AddN ( sizeof(T) );
		WriteT ( iOff, tValue );
	}
};


// RAII Sphinx API block: in ctr reserve place for size, in dtr write hton int into reserved place
class APIBlob_c
{
	intptr_t m_iPos = 0;
	ISphOutputBuffer & m_dOut;
public:
	explicit APIBlob_c ( ISphOutputBuffer& dOut )
		: m_dOut ( dOut )
	{
		m_iPos = (intptr_t) dOut.GetSentCount ();
		dOut.SendInt ( 0 );
	}
	~APIBlob_c()
	{
		auto iBlobLen = m_dOut.GetSentCount ()-m_iPos-sizeof ( int );
		m_dOut.WriteInt ( m_iPos, (int) iBlobLen );
	}
};

// RAII Start Sphinx API command/request header
APIBlob_c APIHeader ( ISphOutputBuffer & dBuff, WORD uCommand, WORD uVer = 0 /* SEARCHD_OK */ );

// RAII Sphinx API answer
APIBlob_c APIAnswer ( ISphOutputBuffer & dBuff, WORD uVer = 0, WORD uStatus = 0 /* SEARCHD_OK */ );

// buffer that knows if it has requested data or not
class SmartOutputBuffer_t final: public ISphOutputBuffer
{
	CSphVector<ISphOutputBuffer *> m_dChunks;
public:
	SmartOutputBuffer_t () = default;
	~SmartOutputBuffer_t () final;

	void StartNewChunk ();
//	void AppendBuf ( SmartOutputBuffer_t &dBuf );
//	void PrependBuf ( SmartOutputBuffer_t &dBuf );
	size_t GetIOVec ( CSphVector<sphIovec> &dOut ) const;
	void Reset();
#if _WIN32
	void LeakTo ( CSphVector<ISphOutputBuffer *> dOut );
#endif
};

class NetGenericOutputBuffer_c : public ISphOutputBuffer
{
public:
	bool	GetError () const { return m_bError; }
	void	SetProfiler ( QueryProfile_c * pProfiler ) { m_pProfile = pProfiler; }

	bool Flush ()
	{
		if ( !SendBuffer ( m_dBuf ) )
			return false;
		m_dBuf.Resize ( 0 ); // check and fix!
		return true;
	}

	virtual bool SendBuffer ( const VecTraits_T<BYTE> & dData ) = 0;

	virtual void SetWTimeoutUS ( int64_t iTimeoutUS ) = 0;
	virtual int64_t GetWTimeoutUS () const = 0;

protected:
	QueryProfile_c *	m_pProfile = nullptr;
	bool		m_bError = false;
};

/// generic request buffer
class InputBuffer_c
{
public:
	InputBuffer_c ( const BYTE * pBuf, int iLen );
	InputBuffer_c ( const VecTraits_T<BYTE>& dBuf );
	virtual			~InputBuffer_c () {}

	int				GetInt () { return ntohl ( GetT<int> () ); }
	WORD			GetWord () { return ntohs ( GetT<WORD> () ); }
	DWORD			GetDword () { return ntohl ( GetT<DWORD> () ); }
	DWORD			GetLSBDword ()
	{
#if USE_LITTLE_ENDIAN
		return GetT<DWORD> ();
#else
		BYTE dB[4];
		GetBytes(dB,4);
		return dB[0] + ( dB[1]<<8 ) + ( dB[2]<<16 ) + ( dB[3]<<24 );
#endif
	}

	uint64_t		GetUint64() { uint64_t uRes = GetDword(); return (uRes<<32)+GetDword(); }
	BYTE			GetByte () { return GetT<BYTE> (); }
	float			GetFloat () { return sphDW2F ( ntohl ( GetT<DWORD> () ) ); }
	CSphString		GetString ();
	CSphString		GetRawString ( int iLen );
	bool			GetString ( CSphVector<BYTE> & dBuffer );
	bool			GetError () { return m_bError; }
	bool			GetBytes ( void * pBuf, int iLen );
	const BYTE *	GetBufferPtr () const { return m_pCur; }
	int				GetBufferPos () const { return int ( m_pCur - m_pBuf ); }
	void			SetBufferPos (int iPos) { m_pCur = m_pBuf + iPos; }
	bool			GetBytesZerocopy ( const BYTE ** ppData, int iLen );

	bool	GetDwords ( CSphVector<DWORD> & dBuffer, int & iGot, int iMax );
	bool	GetQwords ( CSphVector<SphAttr_t> & dBuffer, int & iGot, int iMax );

	inline int		HasBytes() const { return int ( m_pBuf - m_pCur + m_iLen ); }

protected:
	const BYTE *	m_pBuf;
	const BYTE *	m_pCur;
	bool			m_bError;
	int				m_iLen;

protected:
	void			SetError ( bool bError ) { m_bError = bError; }
	template < typename T > T	GetT ()
	{
		if ( m_bError || ( m_pCur + sizeof( T )>m_pBuf + m_iLen ))
		{
			SetError( true );
			return 0;
		}

		T iRes = sphUnalignedRead( *( T* ) const_cast<BYTE*>(m_pCur) );
		m_pCur += sizeof( T );
		return iRes;
	}
};

/// simple memory request buffer
using MemInputBuffer_c = InputBuffer_c;


/////////////////////////////////////////////////////////////////////////////
// SERVED INDEX DESCRIPTORS STUFF
/////////////////////////////////////////////////////////////////////////////
namespace QueryStats {
	enum
	{
		INTERVAL_1MIN,
		INTERVAL_5MIN,
		INTERVAL_15MIN,
		INTERVAL_ALLTIME,

		INTERVAL_TOTAL
	};


	enum
	{
		TYPE_AVG,
		TYPE_MIN,
		TYPE_MAX,
		TYPE_95,
		TYPE_99,

		TYPE_TOTAL,
	};
}

struct QueryStatElement_t
{
	uint64_t	m_dData[QueryStats::TYPE_TOTAL] = { 0, UINT64_MAX, 0, 0, 0, };
	uint64_t	m_uTotalQueries = 0;
};


struct QueryStats_t
{
	QueryStatElement_t	m_dStats[QueryStats::INTERVAL_TOTAL];
};


struct QueryStatRecord_t
{
	uint64_t	m_uQueryTimeMin;
	uint64_t	m_uQueryTimeMax;
	uint64_t	m_uQueryTimeSum;
	uint64_t	m_uFoundRowsMin;
	uint64_t	m_uFoundRowsMax;
	uint64_t	m_uFoundRowsSum;

	uint64_t	m_uTimestamp;
	int			m_iCount;
};


class QueryStatContainer_i
{
public:
	virtual								~QueryStatContainer_i() {}
	virtual void						Add ( uint64_t uFoundRows, uint64_t uQueryTime, uint64_t uTimestamp ) = 0;
	virtual void						GetRecord ( int iRecord, QueryStatRecord_t & tRecord ) const = 0;
	virtual int							GetNumRecords() const = 0;
};


class ServedStats_c
{
public:
						ServedStats_c();
	virtual				~ServedStats_c();

	void				AddQueryStat ( uint64_t uFoundRows, uint64_t uQueryTime ); //  REQUIRES ( !m_tStatsLock );
						/// since mutex is internal,
	void				CalculateQueryStats ( QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats ) const EXCLUDES ( m_tStatsLock );
#ifndef NDEBUG
	void				CalculateQueryStatsExact ( QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats ) const EXCLUDES ( m_tStatsLock );
#endif
private:
	mutable CSphRwlock m_tStatsLock;
	CSphScopedPtr<QueryStatContainer_i> m_pQueryStatRecords GUARDED_BY ( m_tStatsLock );

#ifndef NDEBUG
	CSphScopedPtr<QueryStatContainer_i> m_pQueryStatRecordsExact GUARDED_BY ( m_tStatsLock );
#endif

	TDigest_i *			m_pQueryTimeDigest GUARDED_BY ( m_tStatsLock ) = nullptr;
	TDigest_i *			m_pRowsFoundDigest GUARDED_BY ( m_tStatsLock ) = nullptr;

	uint64_t			m_uTotalFoundRowsMin GUARDED_BY ( m_tStatsLock )= UINT64_MAX;
	uint64_t			m_uTotalFoundRowsMax GUARDED_BY ( m_tStatsLock )= 0;
	uint64_t			m_uTotalFoundRowsSum GUARDED_BY ( m_tStatsLock )= 0;

	uint64_t			m_uTotalQueryTimeMin GUARDED_BY ( m_tStatsLock )= UINT64_MAX;
	uint64_t			m_uTotalQueryTimeMax GUARDED_BY ( m_tStatsLock )= 0;
	uint64_t			m_uTotalQueryTimeSum GUARDED_BY ( m_tStatsLock )= 0;

	uint64_t			m_uTotalQueries GUARDED_BY ( m_tStatsLock ) = 0;

	static void			CalcStatsForInterval ( const QueryStatContainer_i * pContainer, QueryStatElement_t & tRowResult,
							QueryStatElement_t & tTimeResult, uint64_t uTimestamp, uint64_t uInterval, int iRecords );

	void				DoStatCalcStats ( const QueryStatContainer_i * pContainer, QueryStats_t & tRowsFoundStats,
							QueryStats_t & tQueryTimeStats ) const EXCLUDES ( m_tStatsLock );
};

// calculate index mass based on status
uint64_t CalculateMass ( const CSphIndexStatus & dStats );

struct ServedDesc_t
{
	CSphIndex *	m_pIndex		= nullptr; ///< owned index; will be deleted in d-tr
	CSphString	m_sIndexPath;	///< current index path; independent but related to one in m_pIndex
	CSphString	m_sNewPath;		///< when reloading because of config changed, it contains path to new index.
	bool		m_bOnlyNew		= false; ///< load new clean index - no previous valid files, no .old backups possible, no way to serve if loading failed.
	CSphString	m_sGlobalIDFPath;
	int64_t		m_iMass			= 0; // relative weight (by access speed) of the index
	int			m_iRotationPriority = 0;	// rotation priority (for proper rotation of indexes chained by killlist_target). 0==high priority
	StrVec_t	m_dKilllistTargets;
	mutable CSphString	m_sUnlink;
	IndexType_e	m_eType			= IndexType_e::PLAIN;
	CSphString	m_sCluster;
	MutableIndexSettings_c m_tSettings = MutableIndexSettings_c::GetDefaults();

	// statics instead of members below used to simultaneously check pointer for null also.

	// mutable is one which can be insert/replace
	static bool IsMutable ( const ServedDesc_t* pServed )
	{
		if ( !pServed )
			return false;
		return pServed->m_eType==IndexType_e::RT || pServed->m_eType==IndexType_e::PERCOLATE;
	}

	// local is one stored locally on disk
	static bool IsLocal ( const ServedDesc_t* pServed )
	{
		if ( !pServed )
			return false;
		return IsMutable ( pServed ) || pServed->m_eType==IndexType_e::PLAIN;
	}

	// cluster is one which can deals with replication
	static bool IsCluster ( const ServedDesc_t* pServed )
	{
		if ( !pServed )
			return false;
		return !pServed->m_sCluster.IsEmpty ();
	}

	// CanSelect is one which supports select ... from (at least full-scan).
	static bool IsSelectable ( const ServedDesc_t* pServed )
	{
		if ( !pServed )
			return false;
		return IsFT ( pServed ) || pServed->m_eType==IndexType_e::PERCOLATE;
	}

	// FT is one which supports full-text searching
	static bool IsFT ( const ServedDesc_t* pServed )
	{
		if ( !pServed )
			return false;
		return pServed->m_eType==IndexType_e::PLAIN
			|| pServed->m_eType==IndexType_e::RT
			|| pServed->m_eType==IndexType_e::DISTR; // fixme! distrs not necessary ft.
	}

	// Update index mass for searching. Mass after preread typically will be less.
	static void UpdateMass ( const ServedDesc_t* pServed )
	{
		if ( !pServed )
			return;
		CSphIndexStatus tStatus;
		pServed->m_pIndex->GetStatus ( &tStatus );
		// break const, since mass value is not critical for races
		const_cast<ServedDesc_t *>(pServed)->m_iMass = CalculateMass ( tStatus );
	}

	// Update index mass for searching. Mass after preread typically will be less.
	static uint64_t GetIndexMass ( const ServedDesc_t* pServed )
	{
		if ( !pServed )
			return 0;
		return pServed->m_iMass;
	}

	virtual                ~ServedDesc_t ();
};

// wrapped ServedDesc_t - to be served as pointers in containers
// (fully block any access to internals)
// create ServedDesc[R|W]Ptr_c instance to have actual access to the members.
class ServedIndex_c : public ISphRefcountedMT, private ServedDesc_t, public ServedStats_c
{
	mutable Threads::CoroRWLock_c m_tLock;

private:
	friend class ServedDescRPtr_c;
	friend class ServedDescWPtr_c;

	const ServedDesc_t * ReadLock () const ACQUIRE_SHARED( m_tLock );
	ServedDesc_t * WriteLock () const ACQUIRE( m_tLock );
	void Unlock () const UNLOCK_FUNCTION( m_tLock );
	void UpgradeLock ( bool bVip ) const RELEASE (m_tLock) ACQUIRE (m_tLock);

protected:
	// no manual deletion; lifetime managed by AddRef/Release()
	~ServedIndex_c () override = default;

public:

	explicit ServedIndex_c ( const ServedDesc_t& tDesc );
	//ServedIndex_c ();

	// fake alias to private m_tLock to allow clang thread-safety analysis
	CSphRwlock * rwlock () const RETURN_CAPABILITY ( m_tLock )
	{ return nullptr; }

};


/// RAII shared reader for ServedDesc_t hidden in ServedIndex_c
class SCOPED_CAPABILITY ServedDescRPtr_c : ISphNoncopyable
{
public:
	ServedDescRPtr_c() = default;
	// by default acquire read (shared) lock
	explicit ServedDescRPtr_c ( const ServedIndex_c * pLock ) ACQUIRE_SHARED( pLock->m_tLock )
		: m_pLock { pLock }
	{
		if ( m_pLock )
			m_pCore = m_pLock->ReadLock();
	}

	/// unlock on going out of scope
	~ServedDescRPtr_c () RELEASE ()
	{
		if ( m_pLock )
			m_pLock->Unlock ();
	}

public:
	const ServedDesc_t * operator-> () const
	{ return m_pCore; }

	explicit operator bool () const
	{ return m_pCore!=nullptr; }

	operator const ServedDesc_t * () const
	{ return m_pCore; }

	explicit operator const ServedStats_c * () const
	{ return m_pLock; }

	const ServedDesc_t * Ptr () const
	{ return m_pCore; }

	void UpgradeLock ( bool bVip ) const NO_THREAD_SAFETY_ANALYSIS
	{
		if ( m_pLock )
			m_pLock->UpgradeLock ( bVip );
	}

private:
	const ServedDesc_t * m_pCore = nullptr;
	const ServedIndex_c * m_pLock = nullptr;
};


/// RAII exclusive writer for ServedDesc_t hidden in ServedIndex_c
class SCOPED_CAPABILITY ServedDescWPtr_c : ISphNoncopyable
{
public:
	ServedDescWPtr_c () = default;

	// acquire write (exclusive) lock
	explicit ServedDescWPtr_c ( const ServedIndex_c * pLock ) ACQUIRE ( pLock->m_tLock )
		: m_pLock { pLock }
	{
		if ( m_pLock )
			m_pCore = m_pLock->WriteLock ();
	}

	/// unlock on going out of scope
	~ServedDescWPtr_c () RELEASE ()
	{
		if ( m_pLock )
			m_pLock->Unlock ();
	}

public:
	ServedDesc_t * operator-> () const
	{ return m_pCore; }

	explicit operator bool () const
	{ return m_pCore!=nullptr; }

	operator ServedDesc_t * () const
	{ return m_pCore; }

	explicit operator const ServedStats_c * () const
	{ return m_pLock; }

	ServedDesc_t * Ptr () const
	{ return m_pCore; }

private:
	ServedDesc_t * m_pCore = nullptr;
	const ServedIndex_c * m_pLock = nullptr;
};


using ServedIndexRefPtr_c = CSphRefcountedPtr<ServedIndex_c>;

using AddOrReplaceHookFn = std::function<void( ISphRefcountedMT*, const CSphString& )>;

/// hash of ref-counted pointers, guarded by RW-lock
class GuardedHash_c : public ISphNoncopyable
{
	friend class RLockedHashIt_c;
	friend class WLockedHashIt_c;
	using RefCntHash_t = SmallStringHash_T<ISphRefcountedMT *>;

public:
	GuardedHash_c ();
	~GuardedHash_c ();

	// atomically try add an entry and adopt it
	bool AddUniq ( ISphRefcountedMT * pEntry, const CSphString &tKey ) EXCLUDES ( m_tIndexesRWLock );

	// atomically set new entry, then release previous, if not the same and is non-zero
	void AddOrReplace ( ISphRefcountedMT * pEntry, const CSphString &tKey ) EXCLUDES ( m_tIndexesRWLock );

	void SetAddOrReplaceHook( AddOrReplaceHookFn pHook )	{ m_pHook = std::move( pHook ); }

	// release and delete from hash by key
	bool Delete ( const CSphString &tKey ) EXCLUDES ( m_tIndexesRWLock );

	// delete by key if item exists, but null
	bool DeleteIfNull ( const CSphString &tKey ) EXCLUDES ( m_tIndexesRWLock );

	int GetLength () const EXCLUDES ( m_tIndexesRWLock );

	// check if value exists (even if it is nullptr)
	bool Contains ( const CSphString &tKey ) const EXCLUDES ( m_tIndexesRWLock );

	// reset the hash
	void ReleaseAndClear () EXCLUDES ( m_tIndexesRWLock );

	// returns addreffed value
	ISphRefcountedMT * Get ( const CSphString &tKey ) const EXCLUDES ( m_tIndexesRWLock );

	// if value not exist, addref and add it. Then act as Get (addref and return by key)
	ISphRefcountedMT * TryAddThenGet ( ISphRefcountedMT * pValue, const CSphString &tKey ) EXCLUDES ( m_tIndexesRWLock );

	// fake alias to private m_tLock to allow clang thread-safety analysis
	CSphRwlock * IndexesRWLock () const RETURN_CAPABILITY ( m_tIndexesRWLock )
	{ return nullptr; }

private:
	int GetLengthUnl () const REQUIRES_SHARED ( m_tIndexesRWLock );
	void Rlock () const ACQUIRE_SHARED( m_tIndexesRWLock );
	void Wlock () const ACQUIRE ( m_tIndexesRWLock );
	void Unlock () const UNLOCK_FUNCTION ( m_tIndexesRWLock );

private:
	mutable CSphRwlock m_tIndexesRWLock; // distinguishable name for catch possible warnings
	RefCntHash_t m_hIndexes GUARDED_BY ( m_tIndexesRWLock );
	AddOrReplaceHookFn m_pHook = nullptr;
};

// multi-threaded hash iterator
// iterates guarded hash, holding it's readlock
// that is important, since accidental changing of the hash during iteration
// may invalidate the iterator, and then cause crash.
// each iterator has own iteration cookie, so several of them could work in parallel

/* Usage example:
 * 	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next (); )
	{
		auto pIdx = it.Get(); // returns smart pointer to value, addrefed; autorelease on destroy
		if ( !pIdx )
			continue;
		...
		const CSphString &sIndex = it.GetName (); // returns key value
	}
 */

class SCOPED_CAPABILITY RLockedHashIt_c : public ISphNoncopyable
{
public:
	using RefPtr_c = CSphRefcountedPtr<ISphRefcountedMT>;
	explicit RLockedHashIt_c ( const GuardedHash_c * pHash ) ACQUIRE_SHARED ( pHash->m_tIndexesRWLock
																		  , m_pHash->m_tIndexesRWLock )
		: m_pHash ( pHash )
	{ m_pHash->Rlock (); }

	~RLockedHashIt_c () UNLOCK_FUNCTION ()
	{
		m_pHash->Unlock ();
	}

	bool Next () REQUIRES_SHARED ( m_pHash->m_tIndexesRWLock )
	{ return m_pHash->m_hIndexes.IterateNext ( &m_pIterator ); }

	RefPtr_c Get () REQUIRES_SHARED ( m_pHash->m_tIndexesRWLock )
	{
		assert ( m_pIterator );
		auto pRes = GuardedHash_c::RefCntHash_t::IterateGet ( &m_pIterator );
		if ( pRes )
			pRes->AddRef ();
		return RefPtr_c ( pRes );
	}

	const CSphString &GetName () REQUIRES_SHARED ( m_pHash->m_tIndexesRWLock )
	{
		assert ( m_pIterator );
		return GuardedHash_c::RefCntHash_t::IterateGetKey ( &m_pIterator );
	}

protected:
	const GuardedHash_c * m_pHash;
	void * m_pIterator = nullptr;
};

// same as above, but wlocked due to delete() member.
// since it holds exclusive, rlocked iterator will not co-exist.
// also it uses hash's internal iteration to allow deletion (it is ok since it is exclusive).
/* Usage example:
	for ( WLockedHashIt_c it ( &gAgents ); it.Next (); )
	{
		auto pAgent = it.Get (); // returns smart pointer to value, addrefed; autorelease on destroy
		if ( !pAgent )
			continue;
		...
		it.Delete ();	// it is safe. Delete current item, move iterator back to previous.
	}
 */
class SCOPED_CAPABILITY WLockedHashIt_c : public ISphNoncopyable
{
public:
	explicit WLockedHashIt_c ( GuardedHash_c * pHash ) ACQUIRE ( pHash->m_tIndexesRWLock
																  , m_pHash->m_tIndexesRWLock )
		: m_pHash ( pHash )
	{
		m_pHash->Wlock ();
		m_pHash->m_hIndexes.IterateStart ();
	}

	~WLockedHashIt_c () UNLOCK_FUNCTION ()
	{
		m_pHash->Unlock ();
	}

	bool Next () REQUIRES_SHARED ( m_pHash->m_tIndexesRWLock )
	{ return m_pHash->m_hIndexes.IterateNext (); }

	ISphRefcountedMT * Get () REQUIRES_SHARED ( m_pHash->m_tIndexesRWLock )
	{
		auto pRes = m_pHash->m_hIndexes.IterateGet ();
		if ( pRes )
			pRes->AddRef ();
		return pRes;
	}

	void Delete () REQUIRES ( m_pHash->m_tIndexesRWLock )
	{
		m_pHash->m_hIndexes.Delete ( m_pHash->m_hIndexes.IterateGetKey () );
	}

	const CSphString &GetName () REQUIRES_SHARED ( m_pHash->m_tIndexesRWLock )
	{
		return m_pHash->m_hIndexes.IterateGetKey ();
	}

protected:
	GuardedHash_c * m_pHash;
};

class SCOPED_CAPABILITY RLockedServedIt_c : public RLockedHashIt_c
{
public:
	explicit RLockedServedIt_c ( const GuardedHash_c * pHash ) ACQUIRE_SHARED ( pHash->IndexesRWLock(), m_pHash->IndexesRWLock() )
		: RLockedHashIt_c ( pHash )
	{}
	~RLockedServedIt_c() UNLOCK_FUNCTION() {} // d-tr explicitly written because attr UNLOCK_FUNCTION().

	ServedIndexRefPtr_c Get () REQUIRES_SHARED ( m_pHash->IndexesRWLock() )
	{
		auto pServed = ( ServedIndex_c * ) RLockedHashIt_c::Get ().Leak ();
		return ServedIndexRefPtr_c ( pServed );
	}
};

extern GuardedHash_c * g_pLocalIndexes;    // served (local) indexes hash
inline ServedIndexRefPtr_c GetServed ( const CSphString &sName, GuardedHash_c * pHash = g_pLocalIndexes )
{
	return ServedIndexRefPtr_c ( ( ServedIndex_c * ) pHash->Get ( sName ) );
}

void ReleaseAndClearDisabled();

ESphAddIndex ConfigureAndPreloadIndex ( const CSphConfigSection & hIndex, const char * sIndexName, StrVec_t & dWarnings, CSphString & sError );
ESphAddIndex AddIndexMT ( GuardedHash_c & dPost, const char * szIndexName, const CSphConfigSection & hIndex, bool bReplace, bool bMutableOpt, StrVec_t * pWarnings, CSphString & sError );
bool PreallocNewIndex ( ServedDesc_t & tIdx, const CSphConfigSection * pConfig, const char * szIndexName, StrVec_t & dWarnings, CSphString & sError );

struct AttrUpdateArgs: public CSphAttrUpdateEx
{
	const CSphQuery* m_pQuery = nullptr;
	const ServedDesc_t* m_pDesc = nullptr;
	const CSphString* m_pIndexName = nullptr;
	bool m_bJson = false;
};

void HandleMySqlExtendedUpdate( AttrUpdateArgs& tArgs );

/////////////////////////////////////////////////////////////////////////////
// SERVED INDEX DESCRIPTORS STUFF
/////////////////////////////////////////////////////////////////////////////

struct AgentConn_t;
struct DocstoreAndTag_t
{
	union
	{
		const DocstoreReader_i *	m_pDocstore = nullptr;
		const AgentConn_t *			m_pAgent;
	};
	int							m_iTag = 0;		// real tag, but high bit is never set.
	bool						m_bTag = false;	// boolean tag for remotes

	inline void Assign ( const DocstoreAndTag_t& rhs )
	{
		m_iTag = rhs.m_iTag;
		m_bTag = rhs.m_bTag;
		if ( m_bTag )
			m_pAgent = rhs.m_pAgent;
		else
			m_pDocstore = rhs.m_pDocstore;
	}

	inline const DocstoreReader_i * Docstore() const
	{
		return m_bTag ? nullptr : m_pDocstore;
	}

	inline const AgentConn_t * Agent () const
	{
		return m_bTag ? m_pAgent : nullptr;
	}
};

/// result from one backend (sorter). With own schema and m.b. docstore. Avoid copying of the data.
struct OneResultset_t final : public DocstoreAndTag_t
{
	CSphSchema					m_tSchema;
	CSphSwapVector<CSphMatch>	m_dMatches;

	int FillFromSorter ( ISphMatchSorter * pQueue );
	void ClampMatches ( int iLimit );
	void ClampAllMatches ();
	~OneResultset_t();
};

/// specialized swapper
inline void Swap ( OneResultset_t & a, OneResultset_t & b )
{
	a.m_tSchema.Swap( b.m_tSchema );
	a.m_dMatches.SwapData ( b.m_dMatches );
	Swap ( a.m_pDocstore, b.m_pDocstore );
	Swap ( a.m_iTag, b.m_iTag );
	Swap ( a.m_bTag, b.m_bTag );
}

/// result set aggregated across indexes
struct AggrResult_t final: CSphQueryResultMeta
{
	CSphSchema				m_tSchema;			///< result schema
	CSphSwapVector<OneResultset_t> m_dResults;	///< everything from backends (local or remote)
	StrVec_t				m_dZeroCount;
	int						m_iOffset = 0;		///< requested offset into matches array
	int						m_iCount = 0;		///< count which will be actually served (computed from total, offset and limit)
	int						m_iSuccesses = 0;
	bool					m_bTagsAssigned = false; // if matches in chunk(s) have assigned tags
	Debug (bool 			m_bSingle = false;) // single = only one chunk. False = many chunks
	Debug (bool				m_bOneSchema = false;) // either chunk's schemas are valid, or single result's schema in game.
	Debug (bool				m_bTagsCompacted = false;) // whether tags range is compact or has gaps
	Debug (bool				m_bIdxByTag = false;) // if m_dResults[iTag].m_iTag==iTag is true for all results

	int GetLength() const;
	inline bool IsEmpty() const { return GetLength()==0; }
	bool AddResultset ( ISphMatchSorter * pQueue, const DocstoreReader_i * pDocstore, int iTag );
	void ClampMatches ( int iLimit );
	void ClampAllMatches ();
};

class SearchHandler_c;
class PubSearchHandler_c
{
public:
						PubSearchHandler_c ( int iQueries, const QueryParser_i * pQueryParser, QueryType_e eQueryType, bool bMaster );
						~PubSearchHandler_c();

	void				RunQueries ();					///< run all queries, get all results
	void				SetQuery ( int iQuery, const CSphQuery & tQuery, ISphTableFunc * pTableFunc );
	void				SetProfile ( QueryProfile_c * pProfile );
	AggrResult_t *		GetResult ( int iResult );

private:
	SearchHandler_c *	m_pImpl = nullptr;
};


class CSphSessionAccum
{
public:
	CSphSessionAccum () = default;
	~CSphSessionAccum();

	RtAccum_t * GetAcc ( RtIndex_i * pIndex, CSphString & sError );
	RtIndex_i * GetIndex ();

private:
	RtAccum_t * m_pAcc = nullptr;
};


// from mysqld_error.h
enum MysqlErrors_e
{
	MYSQL_ERR_UNKNOWN_COM_ERROR			= 1047,
	MYSQL_ERR_SERVER_SHUTDOWN			= 1053,
	MYSQL_ERR_PARSE_ERROR				= 1064,
	MYSQL_ERR_FIELD_SPECIFIED_TWICE		= 1110,
	MYSQL_ERR_NO_SUCH_TABLE				= 1146,
	MYSQL_ERR_TOO_MANY_USER_CONNECTIONS	= 1203
};

class RowBuffer_i;

class StmtErrorReporter_i
{
public:
	virtual ~StmtErrorReporter_i() = default;

	virtual void Ok ( int iAffectedRows, const CSphString & sWarning, int64_t iLastInsertId ) = 0;
	virtual void Ok ( int iAffectedRows, int nWarnings=0 ) = 0;
	virtual void ErrorEx ( MysqlErrors_e iErr, const char * sError ) = 0;

	void Error ( const char * sTemplate, ... );

	virtual RowBuffer_i * GetBuffer() = 0;
};


class QueryParser_i;
class RequestBuilder_i;
class ReplyParser_i;

QueryParser_i * CreateQueryParser ( bool bJson );
RequestBuilder_i * CreateRequestBuilder ( Str_t sQuery, const SqlStmt_t & tStmt );
ReplyParser_i * CreateReplyParser ( bool bJson, int & iUpdated, int & iWarnings );

enum ESphHttpStatus
{
	SPH_HTTP_STATUS_200,
	SPH_HTTP_STATUS_206,
	SPH_HTTP_STATUS_400,
	SPH_HTTP_STATUS_403,
	SPH_HTTP_STATUS_500,
	SPH_HTTP_STATUS_501,
	SPH_HTTP_STATUS_503,
	SPH_HTTP_STATUS_526,

	SPH_HTTP_STATUS_TOTAL
};

enum ESphHttpEndpoint
{
	SPH_HTTP_ENDPOINT_INDEX,
	SPH_HTTP_ENDPOINT_SQL,
	SPH_HTTP_ENDPOINT_JSON_SEARCH,
	SPH_HTTP_ENDPOINT_JSON_INDEX,
	SPH_HTTP_ENDPOINT_JSON_CREATE,
	SPH_HTTP_ENDPOINT_JSON_INSERT,
	SPH_HTTP_ENDPOINT_JSON_REPLACE,
	SPH_HTTP_ENDPOINT_JSON_UPDATE,
	SPH_HTTP_ENDPOINT_JSON_DELETE,
	SPH_HTTP_ENDPOINT_JSON_BULK,
	SPH_HTTP_ENDPOINT_PQ,

	SPH_HTTP_ENDPOINT_TOTAL
};

bool CheckCommandVersion ( WORD uVer, WORD uDaemonVersion, ISphOutputBuffer & tOut );
bool IsMaxedOut ();
void sphFormatFactors ( StringBuilder_c& dOut, const unsigned int * pFactors, bool bJson );
void sphHandleMysqlInsert ( StmtErrorReporter_i & tOut, SqlStmt_t & tStmt, bool bReplace, bool bCommit, CSphString & sWarning, CSphSessionAccum & tAcc, CSphVector<int64_t> & dLastIds );
void sphHandleMysqlUpdate ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, Str_t sQuery, CSphString & sWarning );
void sphHandleMysqlDelete ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, Str_t sQuery, bool bCommit, CSphSessionAccum & tAcc );

bool				sphLoopClientHttp ( const BYTE * pRequest, int iRequestLen, CSphVector<BYTE> & dResult );
bool				sphProcessHttpQueryNoResponce ( ESphHttpEndpoint eEndpoint, const char * sQuery, const SmallStringHash_T<CSphString> & tOptions, CSphVector<BYTE> & dResult );
void				sphHttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError );
ESphHttpEndpoint	sphStrToHttpEndpoint ( const CSphString & sEndpoint );
CSphString			sphHttpEndpointToStr ( ESphHttpEndpoint eEndpoint );

bool LoopClientSphinx ( SearchdCommand_e eCommand, WORD uCommandVer, int iLength, InputBuffer_c & tBuf, ISphOutputBuffer & tOut, bool bManagePersist );
void HandleCommandPing ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );

void BuildStatusOneline ( StringBuilder_c& sOut );

class CSphinxqlSession;
class SphinxqlSessionPublic : public ISphNoncopyable
{
	CSphinxqlSession * m_pImpl;

public:
	SphinxqlSessionPublic();
	~SphinxqlSessionPublic();

	bool Execute ( Str_t sQuery, RowBuffer_i & tOut );
	void SetFederatedUser ();
	bool IsAutoCommit () const;
	bool IsInTrans() const;

	QueryProfile_c * StartProfiling ( ESphQueryState );
	void SaveLastProfile();
};

void LogSphinxqlError ( const char * sStmt, const char * sError );

// that is used from sphinxql command over API
void RunSingleSphinxqlCommand ( Str_t sCommand, ISphOutputBuffer & tOut );

ISphTableFunc *		CreateRemoveRepeats();

struct PercolateOptions_t
{
	enum MODE
	{
		unknown = 0, sparsed = 1, sharded = 2
	};
	bool m_bGetDocs = false;
	bool m_bVerbose = false;
	bool m_bJsonDocs = true;
	bool m_bGetQuery = false;
	bool m_bSkipBadJson = false; // don't fail whole call if one doc is bad; warn instead.
	int m_iShift = 0;
	MODE m_eMode = unknown;
	CSphString m_sIdAlias;
	CSphString m_sIndex;
};

struct CPqResult; // defined in sphinxpq.h

bool PercolateParseFilters ( const char * sFilters, ESphCollation eCollation, const CSphSchema & tSchema, CSphVector<CSphFilterSettings> & dFilters, CSphVector<FilterTreeItem_t> & dFilterTree, CSphString & sError );
void PercolateMatchDocuments ( const BlobVec_t &dDocs, const PercolateOptions_t &tOpts, CSphSessionAccum &tAcc
							   , CPqResult &tResult );

void SendArray ( const VecTraits_T<CSphString> & dBuf, ISphOutputBuffer & tOut );
void GetArray ( CSphFixedVector<CSphString> & dBuf, InputBuffer_c & tIn );
void SaveArray ( const VecTraits_T<CSphString> & dBuf, MemoryWriter_c & tOut );
void GetArray ( CSphVector<CSphString> & dBuf, MemoryReader_c & tIn );

template <typename T>
void SendArray ( const VecTraits_T<T> & dBuf, ISphOutputBuffer & tOut )
{
	tOut.SendInt ( dBuf.GetLength() );
	if ( dBuf.GetLength() )
		tOut.SendBytes ( dBuf.Begin(), sizeof(dBuf[0]) * dBuf.GetLength() );
}

template<typename T>
void GetArray ( CSphFixedVector<T> & dBuf, InputBuffer_c & tIn )
{
	int iCount = tIn.GetInt();
	if ( !iCount )
		return;

	dBuf.Reset ( iCount );
	tIn.GetBytes ( dBuf.Begin(), (int) dBuf.GetLengthBytes() );
}

template<typename T>
void GetArray ( CSphVector<T> & dBuf, MemoryReader_c & tIn )
{
	int iCount = tIn.GetDword();
	if ( !iCount )
		return;

	dBuf.Resize ( iCount );
	tIn.GetBytes ( dBuf.Begin(), (int) dBuf.GetLengthBytes() );
}

template <typename T>
void SaveArray ( const VecTraits_T<T> & dBuf, MemoryWriter_c & tOut )
{
	tOut.PutDword ( dBuf.GetLength() );
	if ( dBuf.GetLength() )
		tOut.PutBytes ( dBuf.Begin(), sizeof(dBuf[0]) * dBuf.GetLength() );
}


void SendErrorReply ( ISphOutputBuffer & tOut, const char * sTemplate, ... );

enum MysqlColumnType_e
{
	MYSQL_COL_DECIMAL	= 0,
	MYSQL_COL_LONG		= 3,
	MYSQL_COL_FLOAT	= 4,
	MYSQL_COL_LONGLONG	= 8,
	MYSQL_COL_STRING	= 254
};


class RowBuffer_i : public ISphNoncopyable
{
public:
	virtual ~RowBuffer_i() {}

	virtual void PutFloatAsString ( float fVal, const char * sFormat=nullptr ) = 0;

	virtual void PutPercentAsString ( int64_t iVal, int64_t iBase )
	{
		StringBuilder_c sTime;
		if ( iBase )
			sTime.Sprintf ( "%0.2F%%", iVal*10000/iBase );
		else
			sTime << "100%";
		PutString ( sTime.cstr () );
	}

	virtual void PutNumAsString ( int64_t iVal ) = 0;
	virtual void PutNumAsString ( uint64_t uVal ) = 0;
	virtual void PutNumAsString ( int iVal ) = 0;
	virtual void PutNumAsString ( DWORD uVal ) = 0;

	// pack raw array (i.e. packed length, then blob) into proto mysql
	virtual void PutArray ( const void * pBlob, int iLen, bool bSendEmpty = false ) = 0;

	// pack zero-terminated string (or "" if it is zero itself)
	virtual void PutString ( const char * sMsg, int iMaxLen=-1 ) = 0;

	virtual void PutMicrosec ( int64_t iUsec ) = 0;

	virtual void PutNULL() = 0;

	/// more high level. Processing the whole tables.
	// sends collected data, then reset
	virtual bool Commit() = 0;

	// wrappers for popular packets
	virtual void Eof ( bool bMoreResults=false, int iWarns=0 ) = 0;

	virtual void Error ( const char * sStmt, const char * sError, MysqlErrors_e iErr = MYSQL_ERR_PARSE_ERROR ) = 0;

	virtual void Ok ( int iAffectedRows=0, int iWarns=0, const char * sMessage=nullptr, bool bMoreResults=false, int64_t iLastInsertId=0 ) = 0;

	// Header of the table with defined num of columns
	virtual void HeadBegin ( int iColumns ) = 0;

	virtual bool HeadEnd ( bool bMoreResults=false, int iWarns=0 ) = 0;

	// add the next column. The EOF after the full set will be fired automatically
	virtual void HeadColumn ( const char * sName, MysqlColumnType_e uType=MYSQL_COL_STRING ) = 0;

	virtual void Add ( BYTE uVal ) = 0;

	// common implementations
	void PutArray ( const VecTraits_T<BYTE> & dData )
	{
		PutArray ( ( const char * )dData.begin(), dData.GetLength() );
	}

	void PutArray ( ByteBlob_t dData )
	{
		PutArray ( ( const char * )dData.first, dData.second );
	}

	void PutArray ( const StringBuilder_c & dData, bool bSendEmpty=true )
	{
		PutArray ( ( const char * )dData.begin(), dData.GetLength(), bSendEmpty );
	}

	void PutString ( const CSphString & sMsg )
	{
		PutString ( sMsg.cstr() );
	}

	void PutString ( const StringBuilder_c & sMsg )
	{
		PutString ( sMsg.cstr (), sMsg.GetLength() );
	}

	void PutTimeAsString ( int64_t tmVal )
	{
		if ( tmVal==-1 )
		{
			PutString ( "none" );
			return;
		}
		StringBuilder_c sTime;
		sTime.Sprintf ("%t", tmVal);
		PutString ( sTime.cstr() );
	}

	void PutTimestampAsString ( int64_t tmTimestamp )
	{
		StringBuilder_c sTime;
		sTime.Sprintf ( "%T", tmTimestamp );
		PutString ( sTime.cstr ());
	}

	void ErrorEx ( const char * sStmt, const char * sTemplate, ... )
	{
		char sBuf[1024];
		va_list ap;

		va_start ( ap, sTemplate );
		vsnprintf ( sBuf, sizeof(sBuf), sTemplate, ap );
		va_end ( ap );

		Error ( sStmt, sBuf, MYSQL_ERR_PARSE_ERROR );
	}

	void ErrorAbsent ( const char * sStmt, const char * sTemplate, ... )
	{
		char sBuf[1024];
		va_list ap;

		va_start ( ap, sTemplate );
		vsnprintf ( sBuf, sizeof ( sBuf ), sTemplate, ap );
		va_end ( ap );

		Error ( sStmt, sBuf, MYSQL_ERR_NO_SUCH_TABLE );
	}

	// popular pattern of 2 columns of data
	bool DataTuplet ( const char * pLeft, const char * pRight )
	{
		PutString ( pLeft );
		PutString ( pRight );
		return Commit ();
	}

	template <typename NUM>
	bool DataTuplet ( const char * pLeft, NUM tRight )
	{
		PutString ( pLeft );
		PutNumAsString ( tRight );
		return Commit();
	}

	bool DataTupletf ( const char * pLeft, const char * sFmt, ... )
	{
		StringBuilder_c sRight;
		PutString ( pLeft );
		va_list ap;
		va_start ( ap, sFmt );
		sRight.vSprintf ( sFmt, ap );
		va_end ( ap );
		PutString ( sRight.cstr() );
		return Commit();
	}

	// Fire he header for table with iSize string columns
	bool HeadOfStrings ( std::initializer_list<const char*> dNames )
	{
		HeadBegin ( (int) dNames.size() );
		for ( const char* szCol : dNames )
			HeadColumn ( szCol );
		return HeadEnd ();
	}

	bool HeadOfStrings ( const VecTraits_T<CSphString>& sNames )
	{
		HeadBegin ( (int) sNames.GetLength() );
		for ( const auto& sName : sNames )
			HeadColumn ( sName.cstr() );
		return HeadEnd ();
	}

	// table of 2 columns (we really often use them!)
	bool HeadTuplet ( const char * pLeft, const char * pRight )
	{
		HeadBegin ( 2 );
		HeadColumn ( pLeft );
		HeadColumn ( pRight );
		return HeadEnd();
	}

	bool DataRow ( const VecTraits_T<CSphString>& dRow )
	{
		for ( const auto& dValue : dRow )
			PutString ( dValue );
		return Commit();
	}

	void DataTable ( const VectorLike& dData )
	{
		if ( !HeadOfStrings ( dData.Header () ) )
			return;
		auto iStride = dData.Header().GetLength();
		for ( int i=0, iLen = dData.GetLength(); i<iLen; i+=iStride )
			if ( !DataRow ( dData.Slice ( i, iStride ) ) )
				break;
		Eof();
	}
};

#endif // _searchdaemon_
