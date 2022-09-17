//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "client_task_info.h"
#include "client_session.h"

std::atomic<int> ClientTaskInfo_t::m_iClients { 0 };
std::atomic<int> ClientTaskInfo_t::m_iVips { 0 };

DEFINE_RENDER ( ClientTaskInfo_t )
{
	auto& tInfo = *(ClientTaskInfo_t*)pSrc;

	((MiniTaskInfo_t&)tInfo).RenderWithoutChain ( dDst );

	dDst.m_sClientName << tInfo.m_sClientName;
	if ( tInfo.m_bVip )
		dDst.m_sClientName << "vip";
	dDst.m_iConnID = tInfo.m_iConnID;
	dDst.m_eTaskState = tInfo.m_eTaskState;
	dDst.m_eProto = tInfo.m_eProto;
	dDst.m_sProto << ProtoName ( tInfo.m_eProto );
	dDst.m_sChain << "Conn ";
	if ( tInfo.m_bSsl )
		dDst.m_sProto << "ssl";
}


MiniTaskInfo_t* myinfo::HazardGetMini()
{
	return (MiniTaskInfo_t*)myinfo::HazardGetNode ( [] ( TaskInfo_t* pNode ) {
		return pNode->m_eType == MiniTaskInfo_t::m_eTask || pNode->m_eType == ClientTaskInfo_t::m_eTask;
	} );
}


ClientTaskInfo_t * HazardGetClient ()
{
	return (ClientTaskInfo_t *) myinfo::GetHazardTypedNode ( ClientTaskInfo_t::m_eTask );
}

ClientTaskInfo_t & ClientTaskInfo_t::Info ( bool bStrict )
{
	auto * pInfo = HazardGetClient();
	if ( !pInfo )
	{
		static ClientTaskInfo_t tStub;
		pInfo = &tStub;
		if ( bStrict )
			sphWarning ( "internal error: session::Info () invoked with empty tls!" );
	}

	return *pInfo;
}

void ClientTaskInfo_t::SetTaskState ( TaskState_e eState )
{
	m_eTaskState = eState;
	m_tmStart = sphMicroTimer();
}

void ClientTaskInfo_t::SetClientSession ( ClientSession_c* pSession )
{
	m_pSession = pSession;
}

ClientSession_c* ClientTaskInfo_t::GetClientSession()
{
	return m_pSession;
}
namespace {
	volatile int g_iDistThreads = 0;
}
volatile int &getDistThreads ()
{
	return g_iDistThreads;
}

int GetEffectiveDistThreads ()
{
	auto iSessionVal = ClientTaskInfo_t::Info().m_iDistThreads;
	return iSessionVal ? iSessionVal : getDistThreads ();
}

Dispatcher::Template_t GetEffectiveBaseDispatcherTemplate()
{
	auto tDispatcher = Dispatcher::GetGlobalBaseDispatcherTemplate();
	Dispatcher::Unify ( tDispatcher, ClientTaskInfo_t::Info().GetBaseDispatcherTemplate() );
	return tDispatcher;
}

Dispatcher::Template_t GetEffectivePseudoShardingDispatcherTemplate()
{
	auto tDispatcher = Dispatcher::GetGlobalPseudoShardingDispatcherTemplate();
	Dispatcher::Unify ( tDispatcher, ClientTaskInfo_t::Info().GetPseudoShardingDispatcherTemplate() );
	return tDispatcher;
}