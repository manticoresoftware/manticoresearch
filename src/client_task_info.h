//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "sphinxstd.h"
#include "task_info.h"
#include "task_dispatcher.h"

#include <boost/intrusive/slist.hpp>

enum Profile_e {
	NONE,
	PLAIN,
	DOT,
	DOTEXPR,
	DOTEXPRURL,
};

class ClientSession_c;
using ClientTaskHook_t = boost::intrusive::slist_member_hook<>;

// client connection (session). Includes both state and settings.
struct ClientTaskInfo_t : public MiniTaskInfo_t
{
	DECLARE_RENDER( ClientTaskInfo_t );

private:
	TaskState_e m_eTaskState = TaskState_e::UNKNOWN;
	Proto_e m_eProto = Proto_e::UNKNOWN;
	int m_iConnID = -1;
	int m_iSocket = -1;
	CSphString m_sClientName; // set once before info is published and never changes. So, assume always mt-safe
	bool m_bSsl = false;
	bool m_bVip = false;
	bool m_bReadOnly = false;
	bool m_bKilled = false;

	// session variables - doesn't participate in render, used as connection-wide globals
public:
	int m_iThrottlingPeriodMS = -1;
	int m_iDistThreads = 0;
	int m_iDesiredStack = -1;
	int m_iTimeoutS = -1;
	ESphCollation m_eCollation { GlobalCollation () };
	Profile_e			m_eProfile { Profile_e::NONE };
	bool m_bPersistent = false;
	static std::atomic<int> m_iClients;
	static std::atomic<int> m_iVips;

	Dispatcher::Template_t m_tBaseDispatcherTemplate;
	Dispatcher::Template_t m_tPseudoShardingDispatcherTemplate;
	ClientTaskHook_t m_tLink;

private:
	ClientSession_c* 	m_pSession = nullptr;

public:

	// generic setters/getters. They're defined just to help keep multi-threading clean.
	void SetTaskState ( TaskState_e eState );
	TaskState_e GetTaskState() const { return m_eTaskState; }

	void SetProto ( Proto_e eProto ) { m_eProto = eProto; }
	Proto_e GetProto() const { return m_eProto; }

	void SetConnID ( int iConnID ) { m_iConnID = iConnID; }
	int GetConnID() const { return m_iConnID; }

	void SetSocket ( int iSocket ) { m_iSocket = iSocket; }
	int GetSocket() const { return m_iSocket; }

	void SetClientName ( const char* szName ) { m_sClientName = szName; }
	const char* szClientName() const { return m_sClientName.cstr(); }

	void SetSsl ( bool bSsl ) { m_bSsl = bSsl; }
	bool GetSsl() const { return m_bSsl; }

	void SetVip ( bool bVip ) { m_bVip = bVip; }
	bool GetVip() const { return m_bVip; }
	inline static int GetVips() { return m_iVips.load ( std::memory_order_relaxed ); }
	inline static int GetClients() { return m_iClients.load ( std::memory_order_relaxed ); }

	void SetReadOnly ( bool bReadOnly ) { m_bReadOnly = bReadOnly; }
	bool GetReadOnly() const { return m_bReadOnly; }

	void SetKilled ( bool bKilled ) { m_bKilled = bKilled; }
	bool GetKilled() const { return m_bKilled; }

public:
	void SetThrottlingPeriodMS ( int iThrottlingPeriodMS ) { m_iThrottlingPeriodMS = iThrottlingPeriodMS; }
	int GetThrottlingPeriodMS () const { return m_iThrottlingPeriodMS; }

	void SetDistThreads (int iDistThreads) { m_iDistThreads = iDistThreads; }
	int GetDistThreads () const { return m_iDistThreads; }

	void ExpandDesiredStack ( int iDesiredStack ) { m_iDesiredStack = Max ( iDesiredStack, m_iDesiredStack ); }
	int GetDesiredStack() const { return m_iDesiredStack; }

	void SetTimeoutS ( int iTimeoutS ) { m_iTimeoutS = iTimeoutS; }
	int GetTimeoutS() const { return m_iTimeoutS;}

	void SetCollation ( ESphCollation eCollation ) { m_eCollation = eCollation; }
	ESphCollation GetCollation() const { return m_eCollation; }

	void SetProfile ( Profile_e eProfile ) { m_eProfile = eProfile; }
	Profile_e GetProfile() const { return m_eProfile; }
	bool IsProfile () const { return m_eProfile!=Profile_e::NONE; }
	bool IsDot () const { return m_eProfile==Profile_e::DOT; }

	void SetPersistent ( bool bPersistent ) { m_bPersistent = bPersistent; }
	bool GetPersistent () const { return m_bPersistent; }

	void SetBaseDispatcherTemplate ( Dispatcher::Template_t tDispatcherTemplate ) { m_tBaseDispatcherTemplate = tDispatcherTemplate; }
	Dispatcher::Template_t GetBaseDispatcherTemplate() const { return m_tBaseDispatcherTemplate; }

	void SetPseudoShardingDispatcherTemplate ( Dispatcher::Template_t tDispatcherTemplate ) { m_tPseudoShardingDispatcherTemplate = tDispatcherTemplate; }
	Dispatcher::Template_t GetPseudoShardingDispatcherTemplate() const { return m_tPseudoShardingDispatcherTemplate; }

	void SetClientSession ( ClientSession_c* );
	ClientSession_c* GetClientSession();

public:
	static ClientTaskInfo_t& Info ( bool bStrict = false );
};

inline bool operator== ( const ClientTaskInfo_t& lhs, const ClientTaskInfo_t& rhs )
{
	return &lhs==&rhs;
}

using TaskIteratorFn = std::function<void ( ClientTaskInfo_t* )>;
void IterateTasks ( TaskIteratorFn&& );

namespace session {

	inline ClientTaskInfo_t & Info (bool bStrict=false){ return ClientTaskInfo_t::Info(bStrict); }
	inline ClientSession_c* GetClientSession() { return ClientTaskInfo_t::Info().GetClientSession(); }

	// generic getters. If more than one assumed, consider to call Info() once and use ref instead of these 'globals'
	inline void SetTaskState ( TaskState_e eState ) { ClientTaskInfo_t::Info().SetTaskState ( eState ); }
	inline TaskState_e GetTaskState() { return ClientTaskInfo_t::Info().GetTaskState(); }

	inline void SetProto ( Proto_e eProto ) { ClientTaskInfo_t::Info().SetProto (eProto); }
	inline Proto_e GetProto() { return ClientTaskInfo_t::Info().GetProto(); }

	inline void SetConnID ( int iConnID ) { ClientTaskInfo_t::Info().SetConnID (iConnID); }
	inline int GetConnID() { return ClientTaskInfo_t::Info().GetConnID(); }

	inline void SetClientName ( const char* szName ) { ClientTaskInfo_t::Info().SetClientName (szName); }
	inline const char* szClientName() { return ClientTaskInfo_t::Info().szClientName(); }

	inline void SetSsl ( bool bSsl ) { ClientTaskInfo_t::Info().SetSsl (bSsl); }
	inline bool GetSsl() { return ClientTaskInfo_t::Info().GetSsl(); }

	inline bool GetVip() { return ClientTaskInfo_t::Info().GetVip(); }
	inline int GetVips() { return ClientTaskInfo_t::GetVips(); }
	inline int GetClients() { return ClientTaskInfo_t::GetClients(); }

	inline void SetReadOnly ( bool bReadOnly ) { ClientTaskInfo_t::Info().SetReadOnly (bReadOnly); }
	inline bool GetReadOnly() { return ClientTaskInfo_t::Info().GetReadOnly(); }

	inline bool GetKilled() { return ClientTaskInfo_t::Info().GetKilled(); }

	inline void SetThrottlingPeriodMS ( int iThrottlingPeriodMS ) { ClientTaskInfo_t::Info().SetThrottlingPeriodMS ( iThrottlingPeriodMS ); }
	inline int GetThrottlingPeriodMS () { return ClientTaskInfo_t::Info().GetThrottlingPeriodMS(); }

	inline void SetDistThreads (int iDistThreads) {  ClientTaskInfo_t::Info().SetDistThreads ( iDistThreads); }
	inline int GetDistThreads () { return ClientTaskInfo_t::Info().GetDistThreads(); }

	inline void ExpandDesiredStack ( int iDesiredStack ) {  ClientTaskInfo_t::Info().ExpandDesiredStack ( iDesiredStack ); }
	inline int GetDesiredStack() { return ClientTaskInfo_t::Info().GetDesiredStack(); }

	inline void SetTimeoutS ( int iTimeoutS ) {  ClientTaskInfo_t::Info().SetTimeoutS ( iTimeoutS ); }
	inline int GetTimeoutS() { return ClientTaskInfo_t::Info().GetTimeoutS(); }

	inline void SetCollation ( ESphCollation eCollation ) {  ClientTaskInfo_t::Info().SetCollation ( eCollation ); }
	inline ESphCollation GetCollation() { return ClientTaskInfo_t::Info().GetCollation(); }

	inline void SetProfile ( Profile_e eProfile ) { ClientTaskInfo_t::Info().SetProfile ( eProfile ); }
	inline Profile_e GetProfile()  { return ClientTaskInfo_t::Info().GetProfile(); }
	inline bool IsProfile () { return ClientTaskInfo_t::Info().IsProfile(); }
	inline bool IsDot ()  { return ClientTaskInfo_t::Info().IsDot(); }

	inline void SetPersistent ( bool bPersistent ) { ClientTaskInfo_t::Info().SetPersistent(bPersistent); }
	inline bool GetPersistent () { return ClientTaskInfo_t::Info().GetPersistent(); }

} // namespace session

namespace myinfo {
	// num of client tasks, not including vips
	inline int CountClients ()
	{
		return session::GetClients() - session::GetVips();
	}

	// num of real tasks (that is mini-info + client-info)
	inline int CountTasks()
	{
		return Count ( MiniTaskInfo_t::Task() ) + Count ( ClientTaskInfo_t::Task() );
	}

} // namespace myinfo


volatile int &getDistThreads ();

// provides daemon-wide global dist-threads, or task-local, if any exists, or 0 if none
int GetEffectiveDistThreads ();

// provides unified dispatcher of global and local
Dispatcher::Template_t GetEffectiveBaseDispatcherTemplate();
Dispatcher::Template_t GetEffectivePseudoShardingDispatcherTemplate();
