//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "timeout_queue.h"

#include "std/stringbuilder.h"
#include "std/format.h"

inline static bool operator<( const EnqueuedTimeout_t& dLeft, const EnqueuedTimeout_t& dRight )
{
	return dLeft.m_iTimeoutTimeUS < dRight.m_iTimeoutTimeUS;
}

void TimeoutQueue_c::ShiftUp ( int iHole )
{
	if ( m_dQueue.IsEmpty() )
		return;
	int iParent = ( iHole - 1 ) / 2;
	// shift up if needed, so that worst (lesser) one's float to the top
	while ( iHole && *m_dQueue[iHole] < *m_dQueue[iParent] )
	{
		std::swap ( m_dQueue[iHole], m_dQueue[iParent] );
		m_dQueue[iHole]->m_iTimeoutIdx = iHole;
		iHole = iParent;
		iParent = ( iHole - 1 ) / 2;
	}
	m_dQueue[iHole]->m_iTimeoutIdx = iHole;
};

void TimeoutQueue_c::ShiftDown ( int iHole )
{
	if ( m_dQueue.IsEmpty() || iHole == m_dQueue.GetLength() )
		return;
	auto iMinChild = iHole * 2 + 1;
	auto iUsed = m_dQueue.GetLength();
	while ( iMinChild < iUsed )
	{
		// select smallest child
		if ( iMinChild + 1 < iUsed && *m_dQueue[iMinChild + 1] < *m_dQueue[iMinChild] )
			++iMinChild;

		// if smallest child is less than entry, do float it to the top
		if ( *m_dQueue[iHole] < *m_dQueue[iMinChild] )
			break;

		std::swap ( m_dQueue[iHole], m_dQueue[iMinChild] );
		m_dQueue[iHole]->m_iTimeoutIdx = iHole;
		iHole = iMinChild;
		iMinChild = iHole * 2 + 1;
	}
	m_dQueue[iHole]->m_iTimeoutIdx = iHole;
}

void TimeoutQueue_c::Push ( EnqueuedTimeout_t* pTask )
{
	m_dQueue.Add ( pTask );
	ShiftUp ( m_dQueue.GetLength() - 1 );
}


void TimeoutQueue_c::Pop()
{
	if ( m_dQueue.IsEmpty() )
		return;

	m_dQueue[0]->m_iTimeoutIdx = -1;
	m_dQueue.RemoveFast ( 0 );
	ShiftDown ( 0 );
}

void TimeoutQueue_c::Change ( EnqueuedTimeout_t* pTask )
{
	if ( !pTask )
		return;

	auto iHole = pTask->m_iTimeoutIdx;
	if ( iHole < 0 )
	{
		Push ( pTask );
		return;
	}

	if ( iHole && *m_dQueue[iHole] < *m_dQueue[( iHole - 1 ) / 2] )
		ShiftUp ( iHole );
	else
		ShiftDown ( iHole );
}

void TimeoutQueue_c::Remove ( EnqueuedTimeout_t* pTask )
{
	if ( !pTask )
		return;

	auto iHole = pTask->m_iTimeoutIdx;
	if ( iHole < 0 || iHole >= m_dQueue.GetLength() )
		return;

	m_dQueue.RemoveFast ( iHole );
	if ( iHole < m_dQueue.GetLength() )
	{
		if ( iHole && *m_dQueue[iHole] < *m_dQueue[( iHole - 1 ) / 2] )
			ShiftUp ( iHole );
		else
			ShiftDown ( iHole );
	}
	pTask->m_iTimeoutIdx = -1;
}

EnqueuedTimeout_t* TimeoutQueue_c::Root() const
{
	if ( m_dQueue.IsEmpty() )
		return nullptr;
	return m_dQueue[0];
}

CSphString TimeoutQueue_c::DebugDump ( const char* sPrefix ) const
{
	StringBuilder_c tBuild;
	for ( auto* cTask : m_dQueue )
		tBuild.Sprintf ( tBuild.IsEmpty() ? "%p (%l)" : ", %p(%l)", cTask, cTask->m_iTimeoutTimeUS );
	CSphString sRes;
	if ( !m_dQueue.IsEmpty() )
		sRes.SetSprintf ( "%s%d:%s", sPrefix, m_dQueue.GetLength(), tBuild.cstr() );
	else
		sRes.SetSprintf ( "%sHeap empty.", sPrefix );
	return sRes;
}

void TimeoutQueue_c::DebugDump ( const std::function<void ( EnqueuedTimeout_t* )>& fcb ) const
{
	for ( auto* cTask : m_dQueue )
		fcb ( cTask );
}