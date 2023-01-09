//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

// specific profiler with vtune used for profiling daemon's network loop
#pragma once

// vtune
#ifdef USE_VTUNE
#include "ittnotify.h"
#if _WIN32
#pragma comment(linker, "/defaultlib:libittnotify.lib")
#pragma message("Automatically linking with libittnotify.lib")
#endif
#endif

extern bool g_bVtune;
extern int64_t g_tmStarted;

struct LoopProfiler_t
{
#ifdef USE_VTUNE
	__itt_domain *			m_pDomain;
	__itt_string_handle *	m_pTaskPoll;
	__itt_string_handle *	m_pTaskTick;
	__itt_string_handle *	m_pTaskActions;
	__itt_string_handle *	m_pTaskAT;
	__itt_string_handle *	m_pTaskAR;
	__itt_string_handle *	m_pTaskRemove;
	__itt_string_handle *	m_pTaskNext;
	__itt_string_handle *	m_pTaskExt;
	__itt_string_handle *	m_pTaskClean;
	__itt_string_handle *	m_pTaskStat;
#endif

	bool					m_bEnable;
	int64_t					m_tmTotal = 0;
	int m_iPerfEv=0, m_iPerfNext=0, m_iPerfExt=0, m_iPerfClean=0;

	LoopProfiler_t ()
	{
		m_bEnable = g_bVtune;
#ifdef USE_VTUNE
		__itt_thread_set_name ( "net-loop" );
		m_pDomain = __itt_domain_create ( "Task Domain" );
		m_pTaskPoll = __itt_string_handle_create ( "poll" );
		m_pTaskTick = __itt_string_handle_create ( "tick" );
		m_pTaskActions = __itt_string_handle_create ( "actions" );
		m_pTaskAT = __itt_string_handle_create ( "Ta" );
		m_pTaskAR = __itt_string_handle_create ( "ra" );
		m_pTaskRemove = __itt_string_handle_create ( "remove" );
		m_pTaskNext = __itt_string_handle_create ( "next" );
		m_pTaskExt = __itt_string_handle_create ( "ext" );
		m_pTaskClean = __itt_string_handle_create ( "clean" );
		m_pTaskStat = __itt_string_handle_create ( "stat" );
		if ( !m_bEnable )
			m_pDomain->flags = 0;
		else
			__itt_resume ();
#endif
	}

	void End ()
	{
		EndTask();
#ifdef USE_VTUNE
		if ( m_bEnable )
		{
			int64_t tmNow = sphMicroTimer();
			int64_t tmDelta = tmNow - m_tmTotal;
			int64_t tmPassed = tmNow - g_tmStarted;
			sphLogDebug ( "loop=%.3f, (act=%d, next=%d, ext=%d, cln=%d), passed=%.3f", ((float)tmDelta)/1000.0f,
				m_iPerfEv, m_iPerfNext, m_iPerfExt, m_iPerfClean, ((float)tmPassed)/1000000.0f );
		}
#endif
	}
	void EndTask ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_end ( m_pDomain );
#endif
	}
	void Start ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
		{
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskTick );
			m_tmTotal = sphMicroTimer();
			m_iPerfEv = m_iPerfNext = m_iPerfExt = m_iPerfClean = 0;
		}
#endif
	}
	void StartPoll ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskPoll );
#endif
	}
	void StartTick ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskActions );
#endif
	}
	void StartRemove ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskRemove );
#endif
	}
	void StartExt ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskExt );
#endif
	}
	void StartAt()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskAT );
#endif
	}
	void StartAr()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskAR );
#endif
	}
	void StartClean()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskClean );
#endif
	}
	void StartNext()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskNext );
#endif
	}
	void StartStat()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskStat );
#endif
	}
};
