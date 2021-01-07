//
// Copyright (c) 2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "task_info.h"
#include "threadutils.h"

namespace { // static
	const size_t NINFOS = 256;
	RenderFnPtr pInfos[NINFOS] = { nullptr };
	std::atomic<int> dCounters[NINFOS];
	BYTE uFreeInfoSlot = 1; // 0-th slot is a mark of 'invalid'
}

BYTE RegisterRenderer ( RenderFnPtr pFunc )
{
	pInfos[uFreeInfoSlot] = pFunc;
	if ( uFreeInfoSlot==1 ) // that is first one
		for ( auto & i : dCounters )
			i.store ( 0, std::memory_order_relaxed );
	return uFreeInfoSlot++;
}

void internal_myinfo::RefCountInc ( BYTE eType )
{
	assert ( eType<uFreeInfoSlot );
	dCounters[eType].fetch_add ( 1, std::memory_order_relaxed );
}

void internal_myinfo::RefCountDec ( BYTE eType )
{
	assert ( eType<uFreeInfoSlot );
	dCounters[eType].fetch_sub ( 1, std::memory_order_relaxed );
}

int myinfo::Count ( BYTE eType )
{
	assert ( eType<uFreeInfoSlot );
	return dCounters[eType].load ( std::memory_order_relaxed );
}

int myinfo::CountAll ()
{
	int iRes = 0;
	for ( const auto & i : dCounters )
		iRes += i.load ( std::memory_order_relaxed );
	return iRes;
}

void PublicThreadDesc_t::Swap ( PublicThreadDesc_t & rhs )
{
	::Swap ( m_iThreadID, rhs.m_iThreadID );
	::Swap ( m_tmStart, rhs.m_tmStart );
	::Swap ( m_tmLastJobStartTimeUS, rhs.m_tmLastJobStartTimeUS );
	::Swap ( m_tmLastJobDoneTimeUS, rhs.m_tmLastJobDoneTimeUS );
	::Swap ( m_tmTotalWorkedTimeUS, rhs.m_tmTotalWorkedTimeUS );
	::Swap ( m_tmTotalWorkedCPUTimeUS, rhs.m_tmTotalWorkedCPUTimeUS );
	::Swap ( m_iTotalJobsDone, rhs.m_iTotalJobsDone );
	::Swap ( m_sThreadName, rhs.m_sThreadName );
	::Swap ( m_sClientName, rhs.m_sClientName );
	::Swap ( m_sDescription, rhs.m_sDescription );
	::Swap ( m_sProto, rhs.m_sProto );
	::Swap ( m_tmConnect, rhs.m_tmConnect );
	m_pQuery.Swap ( rhs.m_pQuery );
	::Swap ( m_sCommand, rhs.m_sCommand );
	::Swap ( m_iConnID, rhs.m_iConnID );
	::Swap ( m_eProto, rhs.m_eProto );
	::Swap ( m_eTaskState, rhs.m_eTaskState );
	::Swap ( m_sChain, rhs.m_sChain );
}


void CopyBasicThreadInfo ( const Threads::LowThreadDesc_t * pSrc, PublicThreadDesc_t & dDst )
{
	dDst.m_iThreadID = pSrc->m_iThreadID;
	dDst.m_tmStart.emplace_once ( pSrc->m_tmStart );
	dDst.m_tmLastJobStartTimeUS = pSrc->m_tmLastJobStartTimeUS;
	dDst.m_tmLastJobDoneTimeUS = pSrc->m_tmLastJobDoneTimeUS;
	dDst.m_tmTotalWorkedTimeUS = pSrc->m_tmTotalWorkedTimeUS;
	dDst.m_tmTotalWorkedCPUTimeUS = pSrc->m_tmTotalWorkedCPUTimeUS;
	dDst.m_iTotalJobsDone = pSrc->m_iTotalJobsDone;
	dDst.m_sThreadName = pSrc->m_sThreadName;
}

void RenderPublicTaskInfo ( const void * pSrc, PublicThreadDesc_t & dDst, BYTE eType )
{
	if ( pInfos[eType] )
		pInfos[eType] ( pSrc, dDst );
}

PublicThreadDesc_t GatherPublicTaskInfo ( const Threads::LowThreadDesc_t * pSrc, int iCols )
{
	PublicThreadDesc_t dDst;
	if (!pSrc)
		return dDst;

	dDst.m_iDescriptionLimit = iCols; // works as call-back
	hazard::Guard_c tGuard;
	auto pSrcInfo = (TaskInfo_t *) tGuard.Protect ( pSrc->m_pTaskInfo );
	while ( pSrcInfo )
	{
		RenderPublicTaskInfo ( pSrcInfo, dDst, pSrcInfo->m_eType );
		pSrcInfo = (TaskInfo_t *) tGuard.Protect ( pSrcInfo->m_pPrev );
	}
	tGuard.Release();
	CopyBasicThreadInfo ( pSrc, dDst );
	return dDst;
}


// generic is empty
DEFINE_RENDER ( TaskInfo_t ) {};

DEFINE_RENDER ( MiniTaskInfo_t )
{
	auto & tInfo = *(MiniTaskInfo_t *) pSrc;
	dDst.m_tmStart.emplace_once ( tInfo.m_tmStart );
	dDst.m_sCommand = tInfo.m_sCommand;
	hazard::Guard_c tGuard;
	dDst.m_sChain << (int) tInfo.m_eType << ":Mini ";
	auto pDescription = tGuard.Protect ( tInfo.m_pHazardDescription );
	if ( pDescription )
		dDst.m_sDescription << *pDescription;
}

DEFINE_RENDER ( ClientTaskInfo_t )
{
	auto & tInfo = *(ClientTaskInfo_t *) pSrc;

	// MiniTaskInfo_t::Render, but without chain
	dDst.m_tmStart.emplace_once ( tInfo.m_tmStart );
	dDst.m_sCommand = tInfo.m_sCommand;
	hazard::Guard_c tGuard;
	auto pDescription = tGuard.Protect ( tInfo.m_pHazardDescription );
	if ( pDescription )
	{
		if ( dDst.m_iDescriptionLimit<0 ) // no limit
			dDst.m_sDescription << *pDescription;
		else
			dDst.m_sDescription.AppendChunk( { pDescription->scstr(), Min ( tInfo.m_iDescriptionLen, dDst.m_iDescriptionLimit ) });
	}

	// Client render
	dDst.m_sClientName << tInfo.m_sClientName;
	if ( tInfo.m_bVip )
		dDst.m_sClientName << "vip";
	dDst.m_iConnID = tInfo.m_iConnID;
	dDst.m_eTaskState = tInfo.m_eTaskState;
	dDst.m_eProto = tInfo.m_eProto;
	dDst.m_sProto << ProtoName ( tInfo.m_eProto );
	dDst.m_sChain << (int) tInfo.m_eType << ":Client ";
	if ( tInfo.m_bSsl )
		dDst.m_sProto << "ssl";
}

// expect that we own or live shorter than client info. So, called it 'hazard' because of it.
template <typename FILTER>
TaskInfo_t * HazardGetNode ( FILTER fnFilter )
{
	auto pSrc = (TaskInfo_t *) Threads::MyThd ().m_pTaskInfo.load ( std::memory_order_relaxed );
	for ( ; pSrc; pSrc = (TaskInfo_t *) pSrc->m_pPrev.load ( std::memory_order_relaxed ) )
		if ( fnFilter ( pSrc ) )
			break;
	return pSrc;
}

MiniTaskInfo_t * HazardGetMini()
{
	return (MiniTaskInfo_t *) HazardGetNode ( [] ( TaskInfo_t * pNode ) {
		return pNode->m_eType==MiniTaskInfo_t::m_eTask || pNode->m_eType==ClientTaskInfo_t::m_eTask;
	} );
}

ClientTaskInfo_t * HazardGetClient ()
{
	return (ClientTaskInfo_t *) HazardGetNode (
			[] ( TaskInfo_t * pNode ) { return pNode->m_eType==ClientTaskInfo_t::m_eTask; } );
}

TaskInfo_t * myinfo::HazardTaskInfo ()
{
	return (TaskInfo_t *) Threads::MyThd ().m_pTaskInfo.load ( std::memory_order_acquire );
}

TaskInfo_t * myinfo::GetHazardTypedNode ( BYTE eType )
{
	return (ClientTaskInfo_t *) HazardGetNode (
			[eType] ( TaskInfo_t * pNode ) { return pNode->m_eType==eType; } );
}

// bind current taskinfo content to handler
Threads::Handler myinfo::StickParent ( Threads::Handler fnHandler )
{
	auto pParent = myinfo::HazardTaskInfo();
	return [pParent, fnHandler=std::move(fnHandler)]
	{
		Threads::MyThd ().m_pTaskInfo.store ( pParent, std::memory_order_release );
		fnHandler ();
	};
}

// bind current taskinfo and add new scoped mini info for coro handler
Threads::Handler myinfo::OwnMini ( Threads::Handler fnHandler )
{
	auto pParent = myinfo::HazardTaskInfo();
	return [pParent, fnHandler=std::move(fnHandler)]
	{
		Threads::MyThd ().m_pTaskInfo.store ( pParent, std::memory_order_release );
		ScopedMiniInfo_t _ ( new MiniTaskInfo_t );
		fnHandler ();
	};
}

// returns ClientTaskInfo_t::m_iDistThreads
int myinfo::DistThreads()
{
	auto pConn = HazardGetClient ();
	if ( pConn )
		return pConn->m_iDistThreads;

	sphWarning ( "internal error: myinfo::DistThreads () invoked with empty tls!" );
	return 0;
}

// set ClientTaskInfo_t::m_iDistThreads
void myinfo::SetDistThreads ( int iValue )
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		pNode->m_iDistThreads = iValue;
	else
		sphWarning ( "internal error: myinfo::ClientTaskInfo_t () invoked with empty tls!" );
}

// returns ClientTaskInfo_t::m_iThrottlingPeriod
int myinfo::ThrottlingPeriodMS()
{
	auto pConn = HazardGetClient ();
	if ( pConn )
		return pConn->m_iThrottlingPeriod;

	sphWarning ( "internal error: myinfo::ThrottlingPeriodMS () invoked with empty tls!" );
	return 0;
}

// set ClientTaskInfo_t::m_iThrottlingPeriod
void myinfo::SetThrottlingPeriodMS ( int iValue )
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		pNode->m_iThrottlingPeriod = iValue;
	else
		sphWarning ( "internal error: myinfo::SetThrottlingPeriodMS () invoked with empty tls!" );
}

int myinfo::ConnID ()
{
	auto pConn = HazardGetClient();
	if ( pConn )
		return pConn->m_iConnID;

	sphWarning ( "internal error: myinfo::ConnID () invoked with empty tls!" );
	return -1;
}

bool myinfo::IsVIP ()
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		return pNode->m_bVip;

	sphWarning ( "internal error: myinfo::IsVIP () invoked with empty tls!" );
	return false;
}

bool myinfo::IsSSL ()
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		return pNode->m_bSsl;

	sphWarning ( "internal error: myinfo::IsSSL () invoked with empty tls!" );
	return false;
}

void myinfo::SetSSL ( bool bValue )
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		pNode->m_bSsl = bValue;
	else
		sphWarning ( "internal error: myinfo::SetSSL () invoked with empty tls!" );
}

void myinfo::SetCommand ( const char * sCommand )
{
	auto pNode = HazardGetMini ();
	if ( pNode )
		pNode->m_sCommand = sCommand;
	else
		sphWarning ( "internal error: myinfo::SetCommand () invoked with empty tls!" );
}

void myinfo::SetProto ( Proto_e eProto )
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		pNode->m_eProto = eProto;
	else
		sphWarning ( "internal error: myinfo::SetProto () invoked with empty tls!" );
}

Proto_e myinfo::GetProto ()
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		return pNode->m_eProto;

	sphWarning ( "internal error: myinfo::GetProto () invoked with empty tls!" );
	return Proto_e::UNKNOWN;
}

void myinfo::TaskState ( TaskState_e eState )
{
	auto pNode = HazardGetClient ();
	if ( pNode )
	{
		pNode->m_eTaskState = eState;
		pNode->m_tmStart = sphMicroTimer();
	} else
		sphWarning ( "internal error: myinfo::TaskState () invoked with empty tls!" );
}

const char* myinfo::szClientName()
{
	auto pNode = HazardGetClient ();
	if ( pNode )
		return pNode->m_sClientName.cstr();

	sphWarning ( "internal error: myinfo::szClientName () invoked with empty tls!" );
	return "";
}

Str_t myinfo::UnsafeDescription ()
{
	auto pNode = HazardGetMini ();
	assert (pNode);

	if ( pNode )
	{
		if ( pNode->m_pHazardDescription )
			return { pNode->m_pHazardDescription->cstr (), pNode->m_iDescriptionLen };
		else
			return dEmptyStr;
	}

	sphWarning ( "internal error: myinfo::Description () invoked with empty tls!" );
	return dEmptyStr;
}

void SetMiniDescription ( MiniTaskInfo_t * pNode, CSphString * pString, int iLen )
{
	assert ( pNode );
	assert ( pString );

	if ( pNode->m_iDescriptionLen>myinfo::HazardDescriptionSizeLimit )
		pNode->m_pHazardDescription.RetireNow ( pString );
	else
		pNode->m_pHazardDescription = pString;

	pNode->m_iDescriptionLen = iLen;
	pNode->m_tmStart = sphMicroTimer();
}

void SetMiniThreadInfo ( MiniTaskInfo_t * pNode, const char * sTemplate, ... )
{
	assert ( pNode );

	StringBuilder_c sBuf;
	va_list ap;
	va_start ( ap, sTemplate );
	sBuf.vSprintf ( sTemplate, ap );
	va_end ( ap );

	auto pString = new CSphString;
	auto iLen = sBuf.GetLength();
	sBuf.MoveTo ( *pString );

	SetMiniDescription ( pNode, pString, iLen );
}

void myinfo::SetDescription ( CSphString sString, int iLen )
{
	auto pNode = HazardGetMini ();
	assert ( pNode );
	if ( !pNode )
	{
		sphWarning ( "internal error: myinfo::SetDescription () invoked with empty tls!" );
		return;
	}
	SetMiniDescription ( pNode, new CSphString ( std::move ( sString ) ), iLen );
}

void myinfo::SetThreadInfo ( const char * sTemplate, ... )
{
	auto pNode = HazardGetMini ();
	assert ( pNode );
	if ( !pNode )
	{
		sphWarning ( "internal error: myinfo::SetThreadInfo () invoked with empty tls!" );
		return;
	}

	StringBuilder_c sBuf;
	va_list ap;
	va_start ( ap, sTemplate );
	sBuf.vSprintf ( sTemplate, ap );
	va_end ( ap );

	auto pString = new CSphString;
	auto iLen = sBuf.GetLength();
	sBuf.MoveTo ( *pString );

	SetMiniDescription ( pNode, pString, iLen );
}

MiniTaskInfo_t * MakeSystemInfo ( const char * sDescription )
{
	auto pInfo = new MiniTaskInfo_t;
	pInfo->m_sCommand = "SYSTEM";
	SetMiniThreadInfo( pInfo, "SYSTEM %s", sDescription );
	return pInfo;
}

ScopedMiniInfo_t PublishSystemInfo ( const char * sDescription )
{
	return ScopedMiniInfo_t ( MakeSystemInfo ( sDescription ) );
}

volatile int &getDistThreads ()
{
	static int iDistThreads = 0;
	return iDistThreads;
}

int GetEffectiveDistThreads ()
{
	auto iSessionVal = myinfo::DistThreads ();
	return iSessionVal ? iSessionVal : getDistThreads ();
}