//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "ints.h"
#include "generics.h"
#include "string.h"
#include "stringbuilder.h"


// fast diagnostic logging.
// Being a macro, it will be optimized out by compiler when not in use
enum ESphLogLevel : BYTE; // values are in sphinxutils.h
struct LogMessage_t
{
	LogMessage_t ( BYTE uLevel = 5 ); // LOG_VERBOSE_DEBUG
	~LogMessage_t();

	template<typename T>
	LogMessage_t& operator<< ( T&& t );

private:
	StringBuilder_c m_dLog;
	ESphLogLevel m_eLevel;
};

// for LOG (foo, bar) -> define LOG_LEVEL_foo as boolean, define LOG_COMPONENT_bar as expression

#define LOG_MSG LogMessage_t {}
#define LOG( Level, Component ) \
    if_const ( LOG_LEVEL_##Level ) \
        LOG_MSG << LOG_COMPONENT_##Component

#define LOGINFO( Level, Component ) \
	if_const ( LOG_LEVEL_##Level ) \
		LogMessage_t { SPH_LOG_INFO } << LOG_COMPONENT_##Component

#define LOGMSG( Verbosity, Level, Component ) \
	if_const ( LOG_LEVEL_##Level ) \
		LogMessage_t { SPH_LOG_##Verbosity } << LOG_COMPONENT_##Component

// flag to trace all threads creation/deletion (set to true and rebuild)
#define LOG_LEVEL_TPLIFE false

class LocMessages_c;
class LocMessage_c
{
	friend class LocMessages_c;
	LocMessage_c ( LocMessages_c* pOwner );

public:
	void Swap ( LocMessage_c& rhs ) noexcept;

	LocMessage_c ( const LocMessage_c& rhs );

	MOVE_BYSWAP ( LocMessage_c )
	~LocMessage_c();

	template<typename T>
	LocMessage_c& operator<< ( T&& t );

private:
	StringBuilder_c m_dLog;
	LocMessages_c* m_pOwner = nullptr;
};

struct MsgList
{
	CSphString m_sMsg = nullptr;
	MsgList* m_pNext = nullptr;
};

class LocMessages_c: public ISphNoncopyable
{
public:
	~LocMessages_c();

	LocMessage_c GetLoc();
	int Print() const;
	void Append ( StringBuilder_c& dMsg );
	void Swap ( LocMessages_c& rhs ) noexcept;

	//	CSphMutex m_tLock;
	MsgList* m_sMsgs = nullptr;
	int m_iMsgs = 0;
};

/*
 * unit logger.
 * Use LOC_ADD to add logger to a class/struct
 * Use #define LOG_LEVEL_FOO 1 - to enable logging
 * Use #define LOG_COMPONENT_BAR as informative prefix
 * Use logger as LOC(FOO,BAR) << "my cool message" for logging
 * Use m_dLogger.Print() either as direct call, either as 'evaluate expression' in debugger.
 */

#define LOC_ADD LocMessages_c m_tLogger
#define LOC_SWAP( RHS ) m_tLogger.Swap ( RHS.m_tLogger )
#define LOC_MSG m_tLogger.GetLoc()
#define LOC( Level, Component ) \
    if_const ( LOG_LEVEL_##Level ) \
        LOC_MSG << LOG_COMPONENT_##Component

#include "fastlog_impl.h"
