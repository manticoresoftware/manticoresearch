//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
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
	std::atomic<BYTE> uFreeInfoSlot {1}; // 0-th slot is a mark of 'invalid'
}

BYTE RegisterRenderer ( RenderFnPtr pFunc ) noexcept
{
	BYTE uRender = uFreeInfoSlot.fetch_add ( 1, std::memory_order_relaxed );
	pInfos[uRender] = pFunc;
	dCounters[uRender].store ( 0 );
	return uRender;
}

void RefCount_t::Inc ( BYTE eType )
{
	if ( eType >= uFreeInfoSlot )
		sphWarning ( "Wrong RefCountInc slot! type=%d, free slot = %d", eType, uFreeInfoSlot.load() );

	assert ( eType<uFreeInfoSlot );
	if ( eType )
		dCounters[eType].fetch_add ( 1, std::memory_order_relaxed );
}

void RefCount_t::Dec ( BYTE eType )
{
	if ( eType>=uFreeInfoSlot )
		sphWarning ( "Wrong RefCountDec slot! type=%d, free slot = %d", eType, uFreeInfoSlot.load () );

	assert ( eType<uFreeInfoSlot );
	if ( eType )
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
	for ( int i = 1, iLast = uFreeInfoSlot.load ( std::memory_order_relaxed ); i<iLast; ++i )
		iRes += dCounters[i].load ( std::memory_order_relaxed );
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
	::Swap ( m_pQuery, rhs.m_pQuery );
	::Swap ( m_szCommand, rhs.m_szCommand );
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

void GatherPublicTaskInfo ( PublicThreadDesc_t& dDst, const std::atomic<void*>& pTask )
{
	hazard::Guard_c tGuard;
	auto pSrcInfo = (TaskInfo_t*)tGuard.Protect ( pTask );
	while ( pSrcInfo )
	{
		RenderPublicTaskInfo ( pSrcInfo, dDst, pSrcInfo->m_eType );
		pSrcInfo = (TaskInfo_t*)tGuard.Protect ( pSrcInfo->m_pPrev );
	}
	tGuard.Release();
}


PublicThreadDesc_t GatherPublicThreadInfo ( const Threads::LowThreadDesc_t * pSrc, int iCols )
{
	PublicThreadDesc_t dDst;
	if (!pSrc)
		return dDst;

	dDst.m_iDescriptionLimit = iCols; // works as call-back
	GatherPublicTaskInfo ( dDst, pSrc->m_pTaskInfo );
	CopyBasicThreadInfo ( pSrc, dDst );
	return dDst;
}

TaskInfo_t* myinfo::HazardTaskInfo()
{
	return (TaskInfo_t*)Threads::MyThd().m_pTaskInfo.load ( std::memory_order_acquire );
}

TaskInfo_t* myinfo::GetHazardTypedNode ( BYTE eType )
{
	return HazardGetNode ( [eType] ( TaskInfo_t* pNode ) { return pNode->m_eType == eType; } );
}


// bind current taskinfo content to handler
Threads::Handler myinfo::StickParent ( Threads::Handler fnHandler )
{
	auto pParent = myinfo::HazardTaskInfo();
	return [pParent, fnHandler = std::move ( fnHandler )] {
		Threads::MyThd().m_pTaskInfo.store ( pParent, std::memory_order_release );
		fnHandler();
	};
}

// bind current taskinfo and add new scoped mini info for coro handler
Threads::Handler myinfo::OwnMini ( Threads::Handler fnHandler )
{
	auto pParent = myinfo::HazardTaskInfo();
	return [pParent, fnHandler = std::move ( fnHandler )] {
		Threads::MyThd().m_pTaskInfo.store ( pParent, std::memory_order_release );
		ScopedMiniInfo_t _ ( new MiniTaskInfo_t );
		fnHandler();
	};
}

Threads::Handler myinfo::OwnMiniNoCount ( Threads::Handler fnHandler )
{
	auto pParent = myinfo::HazardTaskInfo();
	return [pParent, fnHandler = std::move ( fnHandler )] {
		Threads::MyThd().m_pTaskInfo.store ( pParent, std::memory_order_release );
		ScopedMiniInfoNoCount_t _ ( new MiniTaskInfo_t );
		fnHandler();
	};
}

// generic is empty
DEFINE_RENDER ( TaskInfo_t ) {};

void MiniTaskInfo_t::RenderWithoutChain ( PublicThreadDesc_t& dDst )
{
	dDst.m_tmStart.emplace_once ( m_tmStart );
	dDst.m_szCommand = m_szCommand;
	hazard::Guard_c tGuard;
	auto pDescription = tGuard.Protect ( m_pHazardDescription );
	if ( pDescription )
	{
		if ( dDst.m_iDescriptionLimit < 0 ) // no limit
			dDst.m_sDescription << *pDescription;
		else
			dDst.m_sDescription.AppendChunk ( { pDescription->scstr(), Min ( m_iDescriptionLen, dDst.m_iDescriptionLimit ) } );
	}
}

DEFINE_RENDER ( MiniTaskInfo_t )
{
	dDst.m_sChain << "Mini ";
	auto& tInfo = *(MiniTaskInfo_t*)pSrc;
	tInfo.RenderWithoutChain ( dDst );
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

void SetMiniDescription ( MiniTaskInfo_t * pNode, const char * sTemplate, ... )
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


void myinfo::SetCommand ( const char * szCommand )
{
	auto pNode = HazardGetMini ();
	if ( pNode )
		pNode->m_szCommand = szCommand;
	else
		sphWarning ( "internal error: myinfo::SetCommand () invoked with empty tls!" );
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

void myinfo::SetTaskInfo ( const char * sTemplate, ... )
{
	auto pNode = HazardGetMini ();
	assert ( pNode );
	if ( !pNode )
	{
		sphWarning ( "internal error: myinfo::SetTaskInfo () invoked with empty tls!" );
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
	pInfo->m_szCommand = "SYSTEM";
	SetMiniDescription( pInfo, "SYSTEM %s", sDescription );
	return pInfo;
}

ScopedMiniInfo_t PublishSystemInfo ( const char * sDescription )
{
	return ScopedMiniInfo_t ( MakeSystemInfo ( sDescription ) );
}
