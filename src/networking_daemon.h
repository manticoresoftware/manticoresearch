//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

static constexpr Str_t g_sMaxedOutMessage = FROMS("maxed out, dismissing client");

struct Listener_t
{
	int					m_iSock;
	bool				m_bTcp;
	Proto_e				m_eProto;
	bool				m_bVIP;
	bool 				m_bReadOnly;
};

class CSphNetLoop;
struct ISphNetAction :  NetPollEvent_t
{
	explicit ISphNetAction ( int iSock ) : NetPollEvent_t ( iSock ) {}

	/// callback for network polling when something happened on subscribed socket.
	/// it is called for processing result of epoll/kqueue/iocp linked with a socket
	/// @brief process network event (ready to read / ready to write / error)
	///
	/// @param uGotEvents	bitfield of arived events, as NetPollEvent_t::Event_e
	/// @param pLoop		where you can put your derived action (say, Accept -> SqlServe )
	/// timer is always removed when processing.
	/// If it is timeout - the event is also already removed from the poller.
	/// timer is always removed when processing. If it is timeout - the event is also already removed from the poller.
	virtual void		Process ( DWORD uGotEvents, CSphNetLoop * pLoop ) = 0;

	/// invoked when CSphNetLoop with this action destroying
	/// usually action is owned by netloop (signaller, acceptor), so it just destroys itself here.
	virtual void NetLoopDestroying ()  REQUIRES ( NetPoollingThread ) { Release (); }
};

// event that wakes-up poll net loop from finished thread pool job
// declared here to make available for testing
class CSphWakeupEvent final : public PollableEvent_t, public ISphNetAction
{
public:
	CSphWakeupEvent ();
	void Process ( DWORD uGotEvents, CSphNetLoop * ) final;
	void Wakeup ();

protected:
	~CSphWakeupEvent () final;
};

using WakeupEventRefPtr_c = CSphRefcountedPtr<CSphWakeupEvent>;

/////////////////////////////////////////////////////////////////////////////
/// NETWORK THREAD
/////////////////////////////////////////////////////////////////////////////

class CSphNetLoop : public ISphRefcountedMT
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

protected:
	~CSphNetLoop () override;

public:
	explicit CSphNetLoop ( const VecTraits_T<Listener_t> & dListeners );
	void LoopNetPoll ();
	void StopNetLoop ();

	void AddAction ( ISphNetAction * pElem ) EXCLUDES ( NetPoollingThread );
	void RemoveEvent ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread );
};

// redirect async socket io to netloop
class SockWrapper_c
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	SockWrapper_c ( int iSocket, CSphNetLoop* pNetLoop );
	~SockWrapper_c ();

	int64_t SockRecv ( char * pData, int64_t iLen );
	int64_t SockSend ( const char * pData, int64_t iLen );
	int SockPoll ( int64_t tmTimeUntil, bool bWrite );

	int64_t GetTimeoutUS() const;
	void SetTimeoutUS( int64_t iTimeoutUS );

	int64_t GetWTimeoutUS () const;
	void SetWTimeoutUS ( int64_t iTimeoutUS );

	int64_t GetTotalSent () const;
	int64_t GetTotalReceived () const;
};

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
	/// @iSpace is how many bytes (at least) available to reserve. That is to obey g_iMaxPacketSize.
	/// Reader should allocate need space by AllocateBuffer call, using this param or other hints
	virtual int 	ReadFromBackend ( int iNeed, int iSpace, bool bIntr ) = 0;

	/// for iNeed==0 just try oneshot non-blocking read try to look if anything at all arived.
	/// returns -1 on error, or N of appended bytes.
	/// iSpace limits max quantity of data (limited by g_iMaxPacketSize), however m.b. ignored by compressed backends
	int				AppendData ( int iNeed, int iSpace, bool bIntr );

	/// internal - return place available to not exceed iHardLimit. Dispose consumed data, if necesary.
	int				GetRoomForTail ( int iHardLimit );

	/// internal - discard processed data, then ensure at least iSpace is available, and return blob for it.
	/// ReadFromBackend on return will process results right way, nothing will be lost or ignored.
	VecTraits_T<BYTE> AllocateBuffer ( int iSpace );

public:
	AsyncNetInputBuffer_c ();

	/// read at least 1 byte, and up to g_iMaxPacketSize
	int				ReadAny ();

	/// try to peek first bytes from socket and imagine proto from this
	/// @param bLight determines whether just look to existing (buffered) data, or also query socket, if no such data.
	Proto_e			Probe ( bool bLight );

	/// Ensure we have iLen bytes available in buffer. If not - read new chunk from backend.
	/// return true on success
	bool			ReadFrom ( int iLen, bool bIntr = true );

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

	/// set timeout for backend, in microseconds
	virtual void SetTimeoutUS ( int64_t iTimeoutUS ) = 0;

	/// get timeout from backend, in microseconds
	virtual int64_t GetTimeoutUS () const = 0;

	/// get total N of bytes received via this buffer
	virtual int64_t GetTotalReceived() const = 0;
};


class AsyncNetBuffer_c : public AsyncNetInputBuffer_c, public NetGenericOutputBuffer_c
{
public:
	void SyncErrorState();
};

std::unique_ptr<AsyncNetBuffer_c> MakeAsyncNetBuffer ( std::unique_ptr<SockWrapper_c> pSock );
