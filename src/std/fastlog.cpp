//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "fastlog.h"
#include "sphinxutils.h"


LogMessage_t::LogMessage_t ( BYTE uLevel )
	: m_eLevel ( (ESphLogLevel)uLevel )
{}

LogMessage_t::~LogMessage_t()
{
	sphLogf ( m_eLevel, "%s", m_dLog.cstr() );
}

LocMessage_c::LocMessage_c ( LocMessages_c* pOwner )
	: m_pOwner ( pOwner )
{}

LocMessage_c::~LocMessage_c()
{
	m_pOwner->Append ( m_dLog );
}

void LocMessages_c::Append ( StringBuilder_c& dMsg )
{
	auto pLeaf = new MsgList;
	dMsg.MoveTo ( pLeaf->m_sMsg );
	pLeaf->m_pNext = m_sMsgs;
	m_sMsgs = pLeaf;
	++m_iMsgs;
}

void LocMessages_c::Swap ( LocMessages_c& rhs ) noexcept
{
	::Swap ( m_sMsgs, rhs.m_sMsgs );
	::Swap ( m_iMsgs, rhs.m_iMsgs );
}


int LocMessages_c::Print() const
{
	for ( auto pHead = m_sMsgs; pHead; pHead = pHead->m_pNext )
		sphLogDebug ( "%s", pHead->m_sMsg.scstr() );
	return m_iMsgs;
}

LocMessages_c::~LocMessages_c()
{
	for ( auto pHead = m_sMsgs; pHead != nullptr; )
	{
		auto pNext = pHead->m_pNext;
		SafeDelete ( pHead );
		pHead = pNext;
	}
}