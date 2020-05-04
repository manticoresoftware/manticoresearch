//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "searchdha.h"

extern int g_tmWait;
extern int g_iThrottleAction;
extern int g_iThrottleAccept;
extern int g_iConnectionID;        ///< global conn-id

struct Listener_t
{
	int					m_iSock;
	bool				m_bTcp;
	Proto_e				m_eProto;
	bool				m_bVIP;
};

enum NetEvent_e : DWORD
{
	NE_KEEP = 0,
	NE_ACCEPT = 1,
};
using NetEvBits_e = DWORD;

class CSphNetLoop;
struct ISphNetAction : ISphNoncopyable, NetPollEvent_t
{
	explicit ISphNetAction ( int iSock ) : NetPollEvent_t ( iSock ) {}

	/// callback for network polling when something happened on subscribed socket.
	/// it is called for processing result of epoll/kqueue/iocp linked with a socket
	/// @brief process network event (ready to read / ready to write / error)
	///
	/// @param uGotEvents	bitfield of arived events, as NetPollEvent_t::Event_e
	/// @param pLoop		where you can put your derived action (say, Accept -> SqlServe )
	/// @return one of three directions:
	/// 		NE_KEEP - continue polling, NE_ACCEPT - connections was accepted.
	virtual NetEvent_e		Process ( DWORD uGotEvents, CSphNetLoop * pLoop ) = 0;

	/// invoked when CSphNetLoop with this action destroyed
	virtual void			Destroy () { delete this; };
};

// event that wakes-up poll net loop from finished thread pool job
// declared here to make available for testing
class CSphWakeupEvent final : public PollableEvent_t, public ISphNetAction
{
public:
	CSphWakeupEvent ();
	~CSphWakeupEvent () final;
	NetEvent_e Process ( DWORD uGotEvents, CSphNetLoop * ) final;
	void Wakeup ();
};

/////////////////////////////////////////////////////////////////////////////
/// NETWORK THREAD
/////////////////////////////////////////////////////////////////////////////

class CSphNetLoop
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	DWORD							m_uTick = 0;

public:
	explicit CSphNetLoop ( const VecTraits_T<Listener_t> & dListeners );
	~CSphNetLoop();
	void LoopNetPoll ();

	void AddAction ( ISphNetAction * pElem );
	void Unlink ( ISphNetAction * pEvent, bool bWillClose );
	void RemoveIterEvent ( NetPollEvent_t * pEvent );
};

// redirect async socket io to netloop
class SockWrapper_c
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	SockWrapper_c ( int iSocket, bool bKeep, CSphNetLoop* pNetLoop );
	~SockWrapper_c ();

	int64_t SockRecv ( char * pData, int64_t iLen );
	int64_t SockSend ( const char * pData, int64_t iLen );
	int SockPoll ( int64_t tmTimeUntil, bool bWrite );
};

using SockWrapperPtr_c = SharedPtr_t<SockWrapper_c *>;


/// simple network request buffer
class AsyncNetInputBuffer_c : protected LazyVector_T<BYTE>, public InputBuffer_c
{
protected:
	using STORE = LazyVector_T<BYTE>;
	static const int	NET_MINIBUFFER_SIZE = STORE::iSTATICSIZE;
	bool				m_bIntr = false;

	/// lowest func to implement.
 	/// Read to the end any chunk of iNeed..iHaveSpace bytes. Return num of bytes read, or -1 on error.
	/// @iNeed is strict N user expects. For example, it needs 100 bytes, 90 are in buffer, and 10 he
	/// needs extra. So, less than 10 bytes is fail.
	/// @iHaveSpace is how many bytes is _safe_ to return. If it is requested 10 bytes, but iHaveSpace
	/// tells 10K - then anything between 10 and 10K is ok. If your backend needs more - see BufferFor()
	/// to do so.
	virtual int 	ReadFromBackend ( int iNeed, int iHaveSpace, int iReadTimeoutS, bool bIntr ) = 0;

	/// for iNeed==0 just try oneshot non-blocking read try to look if anything at all arived.
	/// returns -1 on error, or N of appended bytes.
	int				AppendData ( int iNeed, int iSpace, int iTimeoutS, bool bIntr );

	/// internal - return place available to not exceed iHardLimit. Dispose consumed data, if necesary.
	int				GetRoomForTail ( int iHardLimit );

	/// internal - ensure iSpace is in reserve, then return pointer to the end of buffer.
	/// That is to be used in ReadFromBackend, if iHaveSpace param is not enough.
	/// That is for backends with undesired side effects for returning 'to small' chunks.
	/// Say, if you have 10K (for example, uncompressed), but backend implies a chunk which after
	/// processing (decompression) will born 20K. In such case you can igrone @iHaveSpace of ReadfromBackend()
	/// but fill necessary 20K buffer, pointed by BufferFor (20k).
	/// ReadFromBackend on return will process results right way, nothing will be lost or ignored desite of @iHaveSpace
	inline BYTE * BufferFor ( int iSpace )
	{
		ReserveGap ( iSpace );
		return AddN ( 0 );
	}

public:
	AsyncNetInputBuffer_c ();

	/// read at least 1 byte, but no more than 4096 bytes, up to iHardLimit
	int				ReadAny ( int iHardLimit, int iTimeoutS );

	/// try to peek first bytes from socket and imagine proto from this
	Proto_e			Probe ( int iHardLimit, bool bLight );

	/// Ensure we have iLen bytes available in buffer. If not - read new chunk from backend.
	/// return true on success
	bool			ReadFrom ( int iLen, int iTimeoutS, bool bIntr = true );

	/// discards processed data from the buf, releasing space.
	/// @param iHowMany determines behaviour as
	/// =0 - relaxed discard. If something in buf unprocessed, just skip
	/// any positive - forcibly remove this many bytes back from current position, then rewind
	/// -1 - forcibly remove whole chunk from start to current position, then rewind
	void			DiscardProcessed ( int iHowMany = 0 );

	/// returns true, if we're interrupted by signal. Fixme! Check, if we still need to use this func.
	bool			IsIntr () const { return m_bIntr; }

	/// look what is in buf, but NOT pop
	ByteBlob_t 		Tail ();

	/// take part or whole unused content of the buf
	ByteBlob_t		PopTail ( int iSize=-1 );

	/// write uNewVal to iPos, related to current position. Return previous byte from there.
	/// m.b. used to temporary zero-terminate blob for processing, and then revert back
	BYTE 			Terminate ( int iPos, BYTE uNewVal );

	/// len from current position to the end of backend vector.
	using InputBuffer_c::HasBytes;
};


class AsyncNetBuffer_c
{
public:
	virtual ~AsyncNetBuffer_c () = default;
	virtual AsyncNetInputBuffer_c& In() = 0;
	virtual NetGenericOutputBuffer_c& Out() = 0;
};

using AsyncNetBufferPtr_c = SharedPtr_t<AsyncNetBuffer_c *>;

AsyncNetBufferPtr_c MakeAsyncNetBuffer ( SockWrapperPtr_c pSock );

// main entry point - creates netloop and loop it
void ServeNetLoop ( const VecTraits_T<Listener_t> & dListeners );
