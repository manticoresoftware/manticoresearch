//
// Copyright (c) 2019-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "nodes.h"

#include "configuration.h"
#include "searchdaemon.h"
#include "coroutine.h"

StrVec_t ParseNodesFromString ( CSphString sNodes )
{
	StrVec_t dNodes;
	if ( sNodes.IsEmpty() )
		return dNodes;

	// sNodes passed by value, so we free to change it.
	auto* szCur = const_cast<char*> ( sNodes.cstr() );
	while ( *szCur ) {
		// skip spaces
		while ( *szCur && memchr ( ";, \t\n\r", *szCur, 6 ) != nullptr )
			++szCur;

		const char* szAddrs = szCur;
		while ( *szCur && memchr ( ";, \t\n\r", *szCur, 6 ) == nullptr )
			++szCur;

		// replace delimiter with 0 for ParseListener and skip delimiter itself
		if ( *szCur ) {
			*szCur = '\0';
			++szCur;
		}

		if ( *szAddrs )
			dNodes.Add ( szAddrs );
	}

	return dNodes;
}

CSphString GetStringAddr ( const ListenerDesc_t& tListen )
{
	if ( !tListen.m_sAddr.IsEmpty() )
		return tListen.m_sAddr;

	std::array<char, SPH_ADDRESS_SIZE> sAddr {};
	sphFormatIP ( sAddr.data(), sAddr.size(), tListen.m_uIP );
	return sAddr.data();
}

// get nodes of specific type from string
template<typename PROC>
class ISphDescIterator_T : public PROC
{
public:
    template<typename... V>
    explicit ISphDescIterator_T ( V&&... tVargs ) : PROC { std::forward<V> ( tVargs )... }
    {}

	bool ProcessNodes ( const VecTraits_T<CSphString>& dNodes )
	{
		if ( dNodes.IsEmpty() )
			return TlsMsg::Err ( "empty nodes list" );

		for ( int iRetry = 0; iRetry < g_iNodeRetry; ++iRetry )
		{
			if ( dNodes.all_of ( [this] ( const auto& sNode ) { return PROC::SetNode ( sNode.cstr() ); } ) )
				return true;

			if ( iRetry + 1 >= g_iNodeRetry )
				break;

			TlsMsg::ResetErr();
			PROC::ResetNodes();
			// should wait and retry for DNS set
			Threads::Coro::SleepMsec ( g_iAnyNodesTimeoutMs );
		}

		return PROC::IsValid();
	}

	bool ProcessNodes ( const char* szNodes )
	{
		if ( !szNodes || !*szNodes )
			return TlsMsg::Err ("empty nodes list");

		return ProcessNodes ( ParseNodesFromString ( szNodes ) );
	}
};


// get nodes functor to collect listener API with external address
class AgentDescIterator_c
{
	VecAgentDesc_t& m_dNodes;

public:

	explicit AgentDescIterator_c ( VecAgentDesc_t& dNodes )
		: m_dNodes { dNodes }
	{}

	[[nodiscard]] bool IsValid() const noexcept { return !m_dNodes.IsEmpty(); }

	void ResetNodes() { m_dNodes.Reset(); }

	bool SetNode ( const CSphString& sNode )
	{
		// filter out own address to do not query itself
		if ( MyIncomingApiAddrBeginsWith ( sNode.cstr() ) )
			return true;

		CSphString sError;
		ListenerDesc_t tListen = ParseListener ( sNode.cstr(), &sError );
		TlsMsg::Err ( sError );

		if ( tListen.m_eProto == Proto_e::UNKNOWN )
			return false;

		if ( tListen.m_eProto != Proto_e::SPHINX )
			return true;

		if ( tListen.m_uIP == 0 )
			return true;

		AgentDesc_t& tDesc = m_dNodes.Add();
		tDesc.m_sAddr = GetStringAddr ( tListen );
		tDesc.m_uAddr = tListen.m_uIP;
		tDesc.m_iPort = tListen.m_iPort;
		tDesc.m_bNeedResolve = false;
		tDesc.m_bPersistent = false;
		tDesc.m_iFamily = AF_INET;
		return true;
	}
};


VecAgentDesc_t GetDescAPINodes ( const VecTraits_T<CSphString>& dNodes )
{
	TlsMsg::ResetErr();
	VecAgentDesc_t dApiNodes;
	ISphDescIterator_T<AgentDescIterator_c> tIt { dApiNodes };
	tIt.ProcessNodes ( dNodes );
	if ( TlsMsg::HasErr() )
		sphLogDebugRpl ( "node parse error: %s", TlsMsg::szError() );
	return dApiNodes;
}


// get nodes functor to collect listener with specific protocol
class ListenerProtocolIterator_c
{
	Proto_e m_eProto;
	bool m_bResolve;
	sph::StringSet m_hNodes;

public:
	explicit ListenerProtocolIterator_c ( Proto_e eProto, bool bResolve )
		: m_eProto ( eProto )
		, m_bResolve ( bResolve )
	{}

	[[nodiscard]] CSphString DumpNodes() const noexcept
	{
		StringBuilder_c sNodes ( "," );
		for_each ( m_hNodes, [&sNodes] ( const auto& tNode ) { sNodes += tNode.first.cstr(); } );
		return sNodes.cstr();
	}

	[[nodiscard]] StrVec_t GetNodes() const noexcept
	{
		StrVec_t dNodes;
		for_each ( m_hNodes, [&dNodes] ( const auto& tNode ) { dNodes.Add ( tNode.first ); } );
#ifndef NDEBUG
		auto iLen = dNodes.GetLength();
		dNodes.Uniq();
		assert ( iLen == dNodes.GetLength() );
#endif
		return dNodes;
	}

	[[nodiscard]] bool IsValid() const noexcept
	{
		return !m_hNodes.IsEmpty();
	}

	void ResetNodes()
	{
		m_hNodes.Reset();
	}

	bool SetNode ( const CSphString& sNode )
	{
		CSphString sError;
		ListenerDesc_t tListen = ParseResolveListener ( sNode.cstr(), m_bResolve, &sError );
		TlsMsg::Err ( sError );

		if ( tListen.m_eProto == Proto_e::UNKNOWN )
			return false;

		// filter out wrong protocol
		if ( tListen.m_eProto != m_eProto )
			return true;

		auto sAddr = SphSprintf( "%s:%d", GetStringAddr ( tListen ).cstr(), tListen.m_iPort );

		m_hNodes.Add ( sAddr );
		return true;
	}
};

StrVec_t FilterNodesByProto ( const VecTraits_T<CSphString>& dSrcNodes, Proto_e eProto, bool bResolve )
{
	TlsMsg::ResetErr();
	ISphDescIterator_T<ListenerProtocolIterator_c> tIt ( eProto, bResolve );
	tIt.ProcessNodes ( dSrcNodes );
	return tIt.GetNodes();
}
