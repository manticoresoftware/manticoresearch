//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "accumulator.h"

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
#include "client_task_info.h"
#include "coroutine.h"
#include "conversion.h"

#define SPHINXAPI_PORT            9312
#define SPHINXQL_PORT            9306
#define SPH_ADDRESS_SIZE        sizeof("000.000.000.000")
#define SPH_ADDRPORT_SIZE        sizeof("000.000.000.000:00000")
#define NETOUTBUF                8192

volatile bool& sphGetGotSighup() noexcept;
volatile bool& sphGetGotSigusr1() noexcept;
volatile bool& sphGetGotSigusr2() noexcept;

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
	SEARCHD_COMMAND_UNUSED_6	= 6,
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
	VER_COMMAND_SEARCH_MASTER = 20
};


/// known command versions
/// (shared here because of REPLICATE)
enum SearchdCommandV_e : WORD
{
	VER_COMMAND_SEARCH		= 0x123, // 1.35
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
	VER_COMMAND_CLUSTERPQ	= 0x106,
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
	ADD_ERROR,		// wasn't added because of config or other error
	ADD_NEEDLOAD,	// index is loaded, but need to prealloc/preload, etc.
	ADD_DISTR, 		// distributed
	ADD_SERVED,		// added and active (can be used in queries)
//	ADD_CLUSTER	= 4,
};

struct ListenerDesc_t
{
	Proto_e m_eProto = Proto_e::UNKNOWN;
	CSphString m_sUnix;
	CSphString m_sAddr;
	DWORD m_uIP = 0;
	int m_iPort = 0;
	int m_iPortsCount = 0;
	bool m_bVIP = false;
	bool m_bReadOnly = false;
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
 \param[in] bFatal whether failed resolving is fatal (false by default) - FIXME!!! true not used any more, refactor and remove that argument
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
	void		SendDouble ( double fValue )	{ SendUint64 ( sphD2QW(fValue) ); }
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

	void SendLSBWord ( WORD v )
	{
#if USE_LITTLE_ENDIAN
		SendT<WORD> ( v );
#else
		SendByte ( (BYTE)( v & 0xff ) );
		SendByte ( (BYTE)( ( v >> 8 ) & 0xff ) );
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
	void		SendBytes ( const Str_t& sStr );
	void		SendBytes ( const VecTraits_T<BYTE>& dBuf );
	void		SendBytes ( const StringBuilder_c& dBuf );
	void		SendBytes ( ByteBlob_t dData );

	// send array: first length(int), then byte blob
	void		SendString ( const char * sStr );
	void		SendString ( const Str_t& sStr );
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
	const	CSphString & GetErrorMessage() const { return m_sError; }
	void	ResetError();

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
	virtual int64_t GetTotalSent() const = 0;

protected:
	QueryProfile_c *	m_pProfile = nullptr;
	bool		m_bError = false;
	CSphString  m_sError;
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
	float			GetDouble () { return sphQW2D ( GetUint64() ); }
	CSphString		GetString ();
	CSphString		GetRawString ( int iLen );
	bool			GetString ( CSphVector<BYTE> & dBuffer );
	bool			GetBytes ( void * pBuf, int iLen );
	const BYTE *	GetBufferPtr () const { return m_pCur; }
	int				GetBufferPos () const { return int ( m_pCur - m_pBuf ); }
	void			SetBufferPos (int iPos) { m_pCur = m_pBuf + iPos; }
	bool			GetBytesZerocopy ( const BYTE ** ppData, int iLen );

	bool	GetDwords ( CSphVector<DWORD> & dBuffer, int & iGot, int iMax );
	bool	GetQwords ( CSphVector<SphAttr_t> & dBuffer, int & iGot, int iMax );

	inline int		HasBytes() const { return int ( m_pBuf - m_pCur + m_iLen ); }

	bool			GetError() const { return m_bError; }
	const CSphString & GetErrorMessage() const { return m_sError; }
	void			ResetError();

protected:
	const BYTE *	m_pBuf;
	const BYTE *	m_pCur;
	int				m_iLen;

protected:
	void			SetError ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	bool			IsDataSizeValid ( int iSize );
	bool			IsLessMaxPacket ( int iSize );

	template < typename T > T	GetT ()
	{
		if ( m_bError || !IsDataSizeValid ( sizeof( T ) ) )
			return 0;

		T iRes = sphUnalignedRead( *( T* ) const_cast<BYTE*>(m_pCur) );
		m_pCur += sizeof( T );
		return iRes;
	}

private:
	CSphString		m_sError;
	bool			m_bError = false;
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


class ServedStats_c final
{
public:
						ServedStats_c();

	void				AddQueryStat ( uint64_t uFoundRows, uint64_t uQueryTime ); //  REQUIRES ( !m_tStatsLock );
						/// since mutex is internal,
	void				CalculateQueryStats ( QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats ) const EXCLUDES ( m_tStatsLock );
#ifndef NDEBUG
	void				CalculateQueryStatsExact ( QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats ) const EXCLUDES ( m_tStatsLock );
#endif
private:
	mutable RwLock_t m_tStatsLock;
	std::unique_ptr<QueryStatContainer_i> m_pQueryStatRecords GUARDED_BY ( m_tStatsLock );

#ifndef NDEBUG
	std::unique_ptr<QueryStatContainer_i> m_pQueryStatRecordsExact GUARDED_BY ( m_tStatsLock );
#endif

	std::unique_ptr<TDigest_i>	m_pQueryTimeDigest GUARDED_BY ( m_tStatsLock );
	std::unique_ptr<TDigest_i>	m_pRowsFoundDigest GUARDED_BY ( m_tStatsLock );

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
							QueryStats_t & tQueryTimeStats ) const REQUIRES_SHARED ( m_tStatsLock );
};

// calculate index mass based on status
uint64_t CalculateMass ( const CSphIndexStatus & dStats );

struct ServedDesc_t : public ISphRefcountedMT
{
	IndexType_e m_eType = IndexType_e::PLAIN;
	CSphString	m_sIndexPath;	///< current index path; independent but related to one in m_pIndex
	CSphString	m_sGlobalIDFPath;
	StrVec_t	m_dKilllistTargets;

	CSphString	m_sCluster;
	MutableIndexSettings_c m_tSettings { MutableIndexSettings_c::GetDefaults() };

	ServedDesc_t() = default;
	ServedDesc_t ( const ServedDesc_t& rhs )
		: m_eType { rhs.m_eType }
		, m_sIndexPath { rhs.m_sIndexPath }
		, m_sGlobalIDFPath { rhs.m_sGlobalIDFPath }
		, m_dKilllistTargets { rhs.m_dKilllistTargets }
		, m_sCluster { rhs.m_sCluster }
		, m_tSettings { rhs.m_tSettings }
	{}

	void Swap ( ServedDesc_t& rhs ) noexcept
	{
		::Swap ( m_eType, rhs.m_eType );
		::Swap ( m_sIndexPath, rhs.m_sIndexPath );
		::Swap ( m_sGlobalIDFPath, rhs.m_sGlobalIDFPath );
		::Swap ( m_dKilllistTargets, rhs.m_dKilllistTargets );
		::Swap ( m_sCluster, rhs.m_sCluster );
		::Swap ( m_tSettings, rhs.m_tSettings );
	}

	ServedDesc_t ( ServedDesc_t&& rhs ) noexcept { Swap ( rhs ); }
	ServedDesc_t& operator= ( ServedDesc_t rhs ) noexcept
	{
		Swap ( rhs );
		return *this;
	}

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
};

// wrap CSphIndex into refcounted, trace it finally unlink on dtr (if requested)
class RunningIndex_c: public ISphRefcountedMT
{
	mutable Threads::Coro::RWLock_c m_tLock;
	std::unique_ptr<CSphIndex> m_pIndex GUARDED_BY ( m_tLock ); ///< owned index; will be deleted in d-tr
	mutable CSphString m_sUnlink;								///< set if we need to unlink the index on destroy.
	bool m_bLeaked = false;										///< avoid destroying index on dtr (say, if it's ownership changed due to Attach() )

private:
	~RunningIndex_c() override;

	template<typename IDX> friend class RIdx_T;
	template<typename IDX> friend class WIdx_T;
	friend class ServedIndex_c;

public:
	Threads::Coro::RWLock_c& Locker() const RETURN_CAPABILITY ( m_tLock )
	{
		return m_tLock;
	}
};

using RunningIndexRefPtr_t = CSphRefcountedPtr<RunningIndex_c>;

// wrapped ServedDesc_t - to be served as pointers in containers
class ServedIndex_c : public ServedDesc_t
{
	mutable int64_t			m_iMass = 0;	// relative weight (by access speed) of the index

	ServedIndex_c() = default;
	friend CSphRefcountedPtr<ServedIndex_c> MakeServedIndex();
	friend CSphIndex* UnlockedHazardIdxFromServed ( const ServedIndex_c& tServed );

	// no manual deletion; lifetime managed by AddRef/Release()
	~ServedIndex_c () override = default;

	// available only to friends
	inline CSphIndex* GetInternalIndexAvoidingLocks () const NO_THREAD_SAFETY_ANALYSIS
	{
		if ( !m_pIndex )
			return nullptr;
		return m_pIndex->m_pIndex.get();
	}

public:
	RunningIndexRefPtr_t m_pIndex;
	mutable SharedPtr_t<ServedStats_c> m_pStats;

public:
	// Update index mass for searching. Mass after preread typically will be less.
	void UpdateMass () const;

	// Get index mass
	static uint64_t GetIndexMass ( const ServedIndex_c* pServed );

	void SetIdx ( std::unique_ptr<CSphIndex>&& pIndex );
	void ReleaseIdx () const;
	void SetIdxAndStatsFrom ( const ServedIndex_c& tIndex );
	void SetStatsFrom ( const ServedIndex_c& tIndex );
	void SetUnlink ( CSphString sUnlink ) const;
};

using cServedIndexRefPtr_c = CSphRefcountedPtr<const ServedIndex_c>;
using ServedIndexRefPtr_c = CSphRefcountedPtr<ServedIndex_c>;
ServedIndexRefPtr_c MakeServedIndex();

/// RAII shared reader for CSphIndex* hidden in ServedIndexRefPtr_c
template<typename PIDX>
class SCOPED_CAPABILITY RIdx_T : ISphNoncopyable
{
	CSphRefcountedPtr<const ISphRefcountedMT> m_pServedKeeper;
	const RunningIndex_c& m_tRunningIndex;

public:
	// acquire read (shared) lock
	explicit RIdx_T ( const cServedIndexRefPtr_c& pServed ) ACQUIRE_SHARED ( ( *pServed ).m_pIndex->Locker(), m_tRunningIndex.m_tLock )
		: m_pServedKeeper { pServed }
		, m_tRunningIndex { *pServed->m_pIndex }
	{
		assert ( pServed );
		m_tRunningIndex.m_tLock.ReadLock();
	}

	~RIdx_T () RELEASE () { m_tRunningIndex.m_tLock.Unlock(); }

	PIDX Ptr () const NO_THREAD_SAFETY_ANALYSIS { return static_cast<PIDX> ( m_tRunningIndex.m_pIndex.get() ); }
	PIDX operator-> () const { return Ptr(); }
	explicit operator bool () const NO_THREAD_SAFETY_ANALYSIS { return m_tRunningIndex.m_pIndex!=nullptr; }
	operator PIDX () const { return Ptr(); }
};

using RIdx_c = RIdx_T<const CSphIndex*>;	// read-lock backend, and provide const access
using RWIdx_c = RIdx_T<CSphIndex*>;			// read-lock backend, and provide non-const access (that is - for inserts, deletes, etc.)


/// RAII exclusive writer for CSphIndex* hidden in ServedIndexRefPtr_c
template<typename PIDX>
class SCOPED_CAPABILITY WIdx_T : ISphNoncopyable
{
	CSphRefcountedPtr<const ISphRefcountedMT> m_pServedKeeper;
	RunningIndex_c& m_tRunningIndex;

public:
	// acquire write (exclusive) lock
	explicit WIdx_T ( const cServedIndexRefPtr_c& pServed ) ACQUIRE ( (*pServed).m_pIndex->Locker() ) ACQUIRE ( m_tRunningIndex.m_tLock )
		: m_pServedKeeper { pServed }
		, m_tRunningIndex { *pServed->m_pIndex }
	{
		assert ( pServed );
		m_tRunningIndex.m_tLock.WriteLock();
	}

	// acquire write (exclusive) lock
	explicit WIdx_T ( const ServedIndexRefPtr_c& pServed ) ACQUIRE ( ( *pServed ).m_pIndex->Locker() ) ACQUIRE ( m_tRunningIndex.m_tLock )
		: m_pServedKeeper { pServed }
		, m_tRunningIndex { *pServed->m_pIndex }
	{
		assert ( pServed );
		m_tRunningIndex.m_tLock.WriteLock();
	}

	~WIdx_T () RELEASE () { m_tRunningIndex.m_tLock.Unlock(); }

	PIDX Ptr() const NO_THREAD_SAFETY_ANALYSIS { return static_cast<PIDX> ( m_tRunningIndex.m_pIndex.get() ); }
	PIDX operator-> () const { return Ptr(); }
	explicit operator bool () const NO_THREAD_SAFETY_ANALYSIS { return m_tRunningIndex.m_pIndex!=nullptr; }
	operator PIDX () const { return Ptr(); }
};

using WIdx_c = WIdx_T<CSphIndex*>;			// write-lock backend, and provide full access

/// Accessor to naked CSphIndex* hidden in ServedIndex_c. Doesn't use r/w lock, use only to work with free or pre-locked indexes!
inline CSphIndex* UnlockedHazardIdxFromServed ( const ServedIndex_c& tServed )
{
	return tServed.GetInternalIndexAvoidingLocks();
}

void LightClone ( ServedIndexRefPtr_c& pTarget, const cServedIndexRefPtr_c& pSource );
void FullClone ( ServedIndexRefPtr_c& pTarget, const cServedIndexRefPtr_c& pSource );

ServedIndexRefPtr_c MakeLightClone ( const cServedIndexRefPtr_c& pSource );
ServedIndexRefPtr_c MakeFullClone ( const cServedIndexRefPtr_c& pSource );

using AddOrReplaceHookFn = std::function<void( const CSphString& )>;

template<typename T> using HashOfRefcnt_T = SmallStringHash_T<RefCountedRefPtr_T<T>>;
template<typename T> using cHashOfRefcnt_T = SmallStringHash_T<cRefCountedRefPtr_T<T>>;

template<typename T>
class cRefCountedHashOfRefcnt_T final: public ISphRefcountedMT, public cHashOfRefcnt_T<T>
{
protected:
	~cRefCountedHashOfRefcnt_T() final = default;
};

template<typename T = ISphRefcountedMT>
class ReadOnlyHash_T
{
public:
	using Hash_t = cRefCountedHashOfRefcnt_T<T>;
	using cRefPtrHash_t = cRefCountedRefPtr_T<cRefCountedHashOfRefcnt_T<T>>;
	using cRefCountedRefPtr_t = cRefCountedRefPtr_T<T>;
	using RefCountedRefPtr_t = RefCountedRefPtr_T<T>;
	using Snapshot_t = std::pair<cRefPtrHash_t, int64_t>;

private:
	mutable RwLock_t m_tLock; // should be very short-term
	cRefPtrHash_t m_pHash GUARDED_BY ( m_tLock ) { new Hash_t };
	int64_t m_iGeneration GUARDED_BY ( m_tLock ) { 0 }; // eliminate ABA race on insert/delete
	AddOrReplaceHookFn m_pHook = nullptr;

	template<typename TT> friend class WriteableHash_T;

public:

	Snapshot_t GetSnapshot() const
	{
		ScRL_t rLock ( m_tLock );
		return { m_pHash, m_iGeneration };
	}

	cRefPtrHash_t GetHash() const
	{
		return GetSnapshot().first;
	}

	bool IsEmpty() const
	{
		ScRL_t rLock ( m_tLock );
		return !m_pHash->GetLength();
	}

	int GetLength() const
	{
		ScRL_t rLock ( m_tLock );
		return m_pHash->GetLength();
	}

	cRefCountedRefPtr_t Get ( const CSphString& sKey ) const
	{
		auto pSnap = GetHash();
		auto* pEntry = ( *pSnap ) ( sKey );
		if ( !pEntry )
			return cRefCountedRefPtr_t {};
		return *pEntry;
	}

	// check if hash contains an entry
	bool Contains ( const CSphString& sKey ) const
	{
		return ( *GetHash() ) ( sKey ) != nullptr;
	}

	void SetAddOrReplaceHook ( AddOrReplaceHookFn pHook ) { m_pHook = std::move ( pHook ); }
	bool Add ( RefCountedRefPtr_t pEntry, const CSphString & sKey );
	void AddOrReplace ( RefCountedRefPtr_t pValue, const CSphString & sKey );
	bool Replace ( RefCountedRefPtr_t pValue, const CSphString & sKey );
	void Delete ( const CSphString &sKey );
	void ReleaseAndClear ();
	void Hook ( const CSphString & sName ) { if ( m_pHook ) m_pHook ( sName ); }
};

// created with null set of elems
// on d-tr any not-null set will replace the owner
// Note, if you want to modify existing set, you NEED to guard some way period between reading old / writing modified
template<typename T>
class WriteableHash_T
{
	using cRefCountedRefPtr_t = cRefCountedRefPtr_T<T>;
	using RefCountedRefPtr_t = RefCountedRefPtr_T<T>;
	using cRefCountedHashOfRefcnt_t = cRefCountedHashOfRefcnt_T<T>;
	using ReadOnlyHash_t = ReadOnlyHash_T<T>;
	using RefPtrHash_t = RefCountedRefPtr_T<cRefCountedHashOfRefcnt_t>;
	using cRefPtrHash_t = cRefCountedRefPtr_T<cRefCountedHashOfRefcnt_t>;

public:
	ReadOnlyHash_t& m_tOwner;
	RefPtrHash_t m_pNewHash;
	int64_t m_iGeneration {-1}; // to be different from default read-only generation, which is 0.
	Threads::Handler m_fnOnHashChanged;

	WriteableHash_T ( WriteableHash_T&& rhs ) noexcept = default;
	WriteableHash_T ( ReadOnlyHash_t& tOwner, Threads::Handler&& fnOnHashChanged )
		: m_tOwner ( tOwner )
		, m_fnOnHashChanged { std::move ( fnOnHashChanged ) }
	{}
	explicit WriteableHash_T ( ReadOnlyHash_t& tOwner ) noexcept
		: m_tOwner ( tOwner )
	{}

	bool TryCommit ( bool bForce=false )
	{
		if ( !m_pNewHash )
			return true;

		{
			cRefPtrHash_t VARIABLE_IS_NOT_USED hKeeper; // will grab previous hash, so that it will be released outside the lock block.
			{
				ScWL_t wLock ( m_tOwner.m_tLock );
				if ( !bForce && m_iGeneration!=m_tOwner.m_iGeneration )
					return false;
				// use leak since we convert 'data*' to 'const data*' here.
				hKeeper = std::exchange ( m_tOwner.m_pHash, m_pNewHash.Leak() );
				++m_tOwner.m_iGeneration;
				m_iGeneration = -1;
			}
		}
		if ( m_fnOnHashChanged )
			m_fnOnHashChanged();
		return true;
	}

	~WriteableHash_T()
	{
		TryCommit ( true );
	}

	void ResetChanges() // undo all changes (will not commit/change owner anyway)
	{
		m_pNewHash = nullptr;
		m_iGeneration = -1;
	}

	void InitEmptyHash () // init with empty - will wipe owner on commit
	{
		m_pNewHash = new cRefCountedHashOfRefcnt_t;
		m_iGeneration = -1;
	}

	void CopyOwnerHash() EXCLUDES ( m_tOwner.m_tLock ) // init with copy of owner
	{
		InitEmptyHash ();
		cRefPtrHash_t tHash;
		std::tie ( tHash, m_iGeneration ) = m_tOwner.GetSnapshot();
		for ( const auto& tElem : *tHash )
			m_pNewHash->Add ( tElem.second, tElem.first );
	}

	bool Add ( cRefCountedRefPtr_t pEntry, const CSphString& sKey ) { return m_pNewHash->Add ( std::move(pEntry), sKey ); }
	bool Add ( RefCountedRefPtr_t pEntry, const CSphString& sKey ) { return Add ( cRefCountedRefPtr_t { pEntry }, sKey ); }
};


template<typename T>
bool ReadOnlyHash_T<T>::Add ( RefCountedRefPtr_t pEntry, const CSphString& sKey )
{
	bool bSuccess;
	bool bSuitable = pEntry;
	WriteableHash_T<T> tChanger { *this };
	cRefCountedRefPtr_t pConstEntry { pEntry.Leak() };
	do {
		tChanger.CopyOwnerHash();
		bSuccess = tChanger.m_pNewHash->Add ( pConstEntry, sKey );
	} while ( !tChanger.TryCommit() );
	if ( bSuitable )
		Hook ( sKey );

	return bSuccess;
}

template<typename T>
void ReadOnlyHash_T<T>::AddOrReplace ( RefCountedRefPtr_t pValue, const CSphString& sKey )
{
	bool bSuitable = pValue;
	WriteableHash_T<T> tChanger { *this };
	cRefCountedRefPtr_t pConstValue { pValue.Leak() };
	do {
		tChanger.CopyOwnerHash();
		auto* pEntry = ( *tChanger.m_pNewHash ) ( sKey );
		if ( pEntry )
			*pEntry = pConstValue;
		else
			Verify ( tChanger.m_pNewHash->Add ( pConstValue, sKey ) );
	} while ( !tChanger.TryCommit() );
	if ( bSuitable )
		Hook ( sKey );
}

template<typename T>
bool ReadOnlyHash_T<T>::Replace ( RefCountedRefPtr_t pValue, const CSphString& sKey )
{
	if ( !Contains ( sKey ) ) // short circuit check, will also check in clone later.
		return false;

	WriteableHash_T<T> tChanger { *this };
	do {
		tChanger.CopyOwnerHash();
		auto* pEntry = ( *tChanger.m_pNewHash ) ( sKey );
		if ( !pEntry )
		{
			tChanger.ResetChanges();
			return false;
		}
		*pEntry = pValue;
	} while ( !tChanger.TryCommit() );
	return true;
}

template<typename T>
void ReadOnlyHash_T<T>::Delete ( const CSphString& sKey )
{
	WriteableHash_T<T> tChanger { *this };
	do {
		tChanger.CopyOwnerHash();
		if ( !tChanger.m_pNewHash->Delete ( sKey ) )
			tChanger.ResetChanges();
	} while ( !tChanger.TryCommit() );
}

template<typename T>
void ReadOnlyHash_T<T>::ReleaseAndClear()
{
	WriteableHash_T<T> tChanger { *this };
	tChanger.InitEmptyHash();
}

using HashOfServed_c = HashOfRefcnt_T<ServedIndex_c>;
using VecOfServed_c = CSphVector<HashOfServed_c::KeyValue_t>;
using ReadOnlyServedHash_c = ReadOnlyHash_T<ServedIndex_c>;
using WriteableServedHash_c = WriteableHash_T<ServedIndex_c>;
using ServedSnap_t = typename ReadOnlyServedHash_c::cRefPtrHash_t;

extern std::unique_ptr<ReadOnlyServedHash_c> g_pLocalIndexes;    // served (local) indexes hash
inline cServedIndexRefPtr_c GetServed ( const CSphString &sName )
{
	return g_pLocalIndexes->Get ( sName );
}

class ServedClone_c: ISphNoncopyable
{
	cServedIndexRefPtr_c m_pSource;
	ServedIndexRefPtr_c m_pTarget;

	CSphString	m_sIndex;

public:
	explicit ServedClone_c ( cServedIndexRefPtr_c pOrigin )
		: m_pSource { std::move ( pOrigin ) }
	{}

	cServedIndexRefPtr_c& Orig() { return m_pSource; }
	ServedIndexRefPtr_c& CloneRef() { return m_pTarget; }
	ServedIndexRefPtr_c& LightCloneOnce();
	ServedIndexRefPtr_c& FullCloneOnce();
};

class HashedServedClone_c: public ServedClone_c
{
	CSphString m_sIndex;
	ReadOnlyServedHash_c* m_pHash = nullptr;

public:
	HashedServedClone_c ( CSphString sIndex, ReadOnlyServedHash_c* pHash );
	~HashedServedClone_c();
};

using ResultAndIndex_t = std::pair<ESphAddIndex, ServedIndexRefPtr_c>;

ESphAddIndex ConfigureAndPreloadIndex ( const CSphConfigSection & hIndex, const char * szIndexName, StrVec_t & dWarnings, CSphString & sError );
ResultAndIndex_t AddIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bCheckDupe, bool bMutableOpt, StrVec_t * pWarnings, CSphString & sError );
bool PreallocNewIndex ( ServedIndex_c & tIdx, const CSphConfigSection * pConfig, const char * szIndexName, StrVec_t & dWarnings, CSphString & sError );

struct AttrUpdateArgs: public CSphAttrUpdateEx
{
	const CSphQuery* m_pQuery = nullptr;
	const CSphString* m_pIndexName = nullptr;
	bool m_bJson = false;
};

bool HandleUpdateAPI ( AttrUpdateArgs& tArgs, CSphIndex* pIndex, int& iUpdate );
void HandleMySqlExtendedUpdate ( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc, int& iUpdated, bool bNeedWlock );

enum class RotateFrom_e : BYTE;
bool PreloadKlistTarget ( const ServedDesc_t& tServed, RotateFrom_e eFrom, StrVec_t& dKlistTarget );
ServedIndexRefPtr_c MakeCloneForRotation ( const cServedIndexRefPtr_c& pSource, const CSphString& sIndex );

void ConfigureDistributedIndex ( std::function<bool ( const CSphString& )>&& fnCheck, DistributedIndex_t& tIdx, const char * szIndexName, const CSphConfigSection& hIndex, StrVec_t* pWarnings = nullptr );
void ConfigureLocalIndex ( ServedDesc_t* pIdx, const CSphConfigSection& hIndex, bool bMutableOpt, StrVec_t* pWarnings );

volatile bool& sphGetSeamlessRotate() noexcept;

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
	bool						m_bTagsAssigned = false;

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
	Swap ( a.m_bTagsAssigned, b.m_bTagsAssigned );
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
	StrVec_t				m_dIndexNames;

	int				GetLength() const;
	inline bool		IsEmpty() const { return GetLength()==0; }
	bool			AddResultset ( ISphMatchSorter * pQueue, const DocstoreReader_i * pDocstore, int iTag, int iCutoff );
	void			AddEmptyResultset ( const DocstoreReader_i * pDocstore, int iTag );
	void			ClampMatches ( int iLimit );
	void			ClampAllMatches();
};

class SearchHandler_c;
class PubSearchHandler_c
{
public:
						PubSearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, bool bMaster );
						~PubSearchHandler_c();

	void				RunQueries ();					///< run all queries, get all results
	void				SetQuery ( int iQuery, const CSphQuery & tQuery, std::unique_ptr<ISphTableFunc> pTableFunc );
	void				SetProfile ( QueryProfile_c * pProfile );
	void				SetStmt ( SqlStmt_t & tStmt );
	AggrResult_t *		GetResult ( int iResult );

	void				PushIndex ( const CSphString& sIndex, const cServedIndexRefPtr_c& pDesc );
	void				RunCollect( const CSphQuery& tQuery, const CSphString& sIndex, CSphString* pErrors, CSphVector<BYTE>* pCollectedDocs );

private:
	std::unique_ptr<SearchHandler_c>	m_pImpl;
};


class CSphSessionAccum
{
	std::optional<RtAccum_t> m_tAcc;

public:
	RtAccum_t * GetAcc ( RtIndex_i * pIndex, CSphString & sError );
	RtAccum_t * GetAcc ();
	RtIndex_i * GetIndex ();
};


// from mysqld_error.h
enum MysqlErrors_e
{
	MYSQL_ERR_UNKNOWN_COM_ERROR			= 1047,
	MYSQL_ERR_SERVER_SHUTDOWN			= 1053,
	MYSQL_ERR_PARSE_ERROR				= 1064,
	MYSQL_ERR_NO_SUCH_THREAD			= 1094,
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

	virtual bool IsError() const { return false; }
	virtual const char * GetError() const { return nullptr; }
};


class QueryParser_i;
class RequestBuilder_i;
class ReplyParser_i;

std::unique_ptr<QueryParser_i> CreateQueryParser ( bool bJson );
std::unique_ptr<RequestBuilder_i> CreateRequestBuilder ( Str_t sQuery, const SqlStmt_t & tStmt );
std::unique_ptr<ReplyParser_i> CreateReplyParser ( bool bJson, int & iUpdated, int & iWarnings );
StmtErrorReporter_i * CreateHttpErrorReporter();

enum ESphHttpStatus
{
	SPH_HTTP_STATUS_100,
	SPH_HTTP_STATUS_200,
	SPH_HTTP_STATUS_206,
	SPH_HTTP_STATUS_400,
	SPH_HTTP_STATUS_403,
	SPH_HTTP_STATUS_404,
	SPH_HTTP_STATUS_405,
	SPH_HTTP_STATUS_409,
	SPH_HTTP_STATUS_413,
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
	SPH_HTTP_ENDPOINT_CLI,
	SPH_HTTP_ENDPOINT_CLI_JSON,
	SPH_HTTP_ENDPOINT_ES_BULK,

	SPH_HTTP_ENDPOINT_TOTAL
};

bool CheckCommandVersion ( WORD uVer, WORD uDaemonVersion, ISphOutputBuffer & tOut );
bool IsMaxedOut ();
bool IsReadOnly ();
void sphFormatFactors ( StringBuilder_c& dOut, const unsigned int * pFactors, bool bJson );
void sphHandleMysqlInsert ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt );
void sphHandleMysqlUpdate ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, Str_t sQuery );
void sphHandleMysqlDelete ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, Str_t sQuery );
void sphHandleMysqlBegin ( StmtErrorReporter_i& tOut, Str_t sQuery );
void sphHandleMysqlCommitRollback ( StmtErrorReporter_i& tOut, Str_t sQuery, bool bCommit );
bool sphCheckWeCanModify ();
bool sphCheckWeCanModify ( StmtErrorReporter_i & tOut );
bool sphCheckWeCanModify ( const char* szStmt, RowBuffer_i& tOut );

bool				sphProcessHttpQueryNoResponce ( const CSphString& sEndpoint, const CSphString& sQuery, CSphVector<BYTE> & dResult );
void				sphHttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError );
void				LoadCompatHttp ( const char * sData );
void				SaveCompatHttp ( JsonObj_c & tRoot );
void				SetupCompatHttp();
bool				SetLogManagement ( const CSphString & sVal, CSphString & sError );
bool				IsLogManagementEnabled ();
std::unique_ptr<PubSearchHandler_c> CreateMsearchHandler ( std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, JsonQuery_c & tQuery );
uint64_t			GetDocID ( const char * szID );

void ExecuteApiCommand ( SearchdCommand_e eCommand, WORD uCommandVer, int iLength, InputBuffer_c & tBuf, ISphOutputBuffer & tOut );
void HandleCommandPing ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );

void BuildStatusOneline ( StringBuilder_c& sOut );

namespace session
{
	bool IsAutoCommit ( const ClientSession_c* );
	bool IsInTrans ( const ClientSession_c* );

	bool Execute ( Str_t sQuery, RowBuffer_i& tOut );
	void SetFederatedUser();
	void SetDumpUser ( const CSphString & sUser );
	void SetAutoCommit ( bool bAutoCommit );
	void SetInTrans ( bool bInTrans );
	bool IsAutoCommit();
	bool IsInTrans();
	QueryProfile_c* StartProfiling ( ESphQueryState );
	void SaveLastProfile();
	VecTraits_T<int64_t> LastIds();
	void SetOptimizeById ( bool bOptimizeById );
	bool GetOptimizeById();
}

void LogSphinxqlError ( const char * sStmt, const Str_t& sError );

// that is used from sphinxql command over API
void RunSingleSphinxqlCommand ( Str_t sCommand, ISphOutputBuffer & tOut );

std::unique_ptr<ISphTableFunc>		CreateRemoveRepeats();

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


void SendErrorReply ( ISphOutputBuffer & tOut, const char * sTemplate, ... );
void SetLogHttpFilter ( const CSphString & sVal );
int HttpGetStatusCodes ( ESphHttpStatus eStatus );
void HttpBuildReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sBody, int iBodyLen, bool bHtml );
void HttpBuildReplyHead ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sBody, int iBodyLen, bool bHeadReply );
void HttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError );

using HttpOptionsHash_t = SmallStringHash_T<CSphString>;
struct http_parser;

void UriPercentReplace ( Str_t & sEntity, bool bAlsoPlus=true );
void DumpHttp ( int iReqType, const CSphString & sURL, Str_t sBody );

enum MysqlColumnType_e
{
	MYSQL_COL_DECIMAL	= 0,
	MYSQL_COL_LONG		= 3,
	MYSQL_COL_FLOAT		= 4,
	MYSQL_COL_DOUBLE	= 5,
	MYSQL_COL_LONGLONG	= 8,
	MYSQL_COL_STRING	= 254,
	MYSQL_COL_UINT64	= 508
};


class RowBuffer_i : public ISphNoncopyable
{
public:
	virtual ~RowBuffer_i() {}

	virtual void PutFloatAsString ( float fVal, const char * sFormat=nullptr ) = 0;
	virtual void PutDoubleAsString ( double fVal, const char * szFormat=nullptr ) = 0;

	virtual void PutPercentAsString ( int64_t iVal, int64_t iBase )
	{
		StringBuilder_c sTime;
		if ( iBase )
			sTime.Sprintf ( "%0.2F%%", iVal*10000/iBase );
		else
			sTime << "100%";
		PutString ( sTime );
	}

	virtual void PutNumAsString ( int64_t iVal ) = 0;
	virtual void PutNumAsString ( uint64_t uVal ) = 0;
	virtual void PutNumAsString ( int iVal ) = 0;
	virtual void PutNumAsString ( DWORD uVal ) = 0;

	// pack raw array (i.e. packed length, then blob)
	virtual void PutArray ( const ByteBlob_t&, bool bSendEmpty = false ) = 0;

	// pack string
	virtual void PutString ( Str_t sMsg ) = 0;

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
	void PutArray ( const StringBuilder_c & dData, bool bSendEmpty=true )
	{
		PutArray ( (ByteBlob_t)dData, bSendEmpty );
	}

	void PutString ( const char* szMsg )
	{
		PutString ( FromSz ( szMsg ) );
	}

	void PutString ( const CSphString & sMsg )
	{
		PutString ( sMsg.cstr() );
	}

	void PutString ( const StringBuilder_c & sMsg )
	{
		PutString ( (Str_t)sMsg );
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
		PutString ( sTime );
	}

	void PutTimestampAsString ( int64_t tmTimestamp )
	{
		StringBuilder_c sTime;
		sTime.Sprintf ( "%T", tmTimestamp );
		PutString ( sTime );
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
		PutString ( sRight );
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

	virtual const CSphString & GetError() const { return m_sError; }
	virtual bool IsError() const { return m_bError; }

protected:
	bool m_bError = false;
	CSphString m_sError;
};

#endif // _searchdaemon_
