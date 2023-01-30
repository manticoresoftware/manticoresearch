//
// Copyright (c) 2022-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxstd.h"

#ifdef PERFETTO

#include "coroutine.h"
#include <perfetto.h>

PERFETTO_DEFINE_CATEGORIES (
	perfetto::Category ( "rt" ).SetDescription ( "Events from rt-index routines" ),
	perfetto::Category ( "network" ).SetDescription ( "Network upload and download statistics" ),
	perfetto::Category ( "conn" ).SetDescription ( "One connection scope" ),
	perfetto::Category ( "wait" ).SetDescription ( "Time spent idle" ),
	perfetto::Category ( "mem" ).SetDescription ( "Memory counters" ),
	);


// trackfn should be function or external value which can be captured
// Name selected to avoid conflict with macro in google-tests
#define SCOPED_TRACEP( category, name, trackfn, ... )                         \
  struct PERFETTO_UID ( ScopedEvent ) {                                       \
    struct EventFinalizer {                                                   \
      EventFinalizer ( ... ) {}                                               \
      ~EventFinalizer() { TRACE_EVENT_END ( category, trackfn ); }            \
      EventFinalizer ( const EventFinalizer& ) = delete;                      \
      inline EventFinalizer& operator= ( const EventFinalizer& ) = delete;    \
      EventFinalizer ( EventFinalizer&& ) = default;                          \
      EventFinalizer& operator= ( EventFinalizer&& ) = delete;                \
    } finalizer;                                                              \
  } PERFETTO_UID ( scoped_event ) {                                           \
    [&]() {                                                                   \
      TRACE_EVENT_BEGIN ( category, name, trackfn, ##__VA_ARGS__ );           \
      return 0;                                                               \
    }()                                                                       \
  }


// track can be load variable
// Notice, it is stored inside, so this variant consumes a bit more memory then just SCOPED_TRACEP
#define SCOPED_TRACE_LOCAL(category, name, track, ...)                        \
  struct PERFETTO_UID(ScopedEvent) {                                          \
    struct EventFinalizer {                                                   \
      perfetto::Track m_tTrack;                                               \
      explicit EventFinalizer ( perfetto::Track&& tTrack ) : m_tTrack {std::move(tTrack)} {}        \
      EventFinalizer(...) {}                                                  \
      ~EventFinalizer() { TRACE_EVENT_END(category, m_tTrack); }              \
                                                                              \
      EventFinalizer(const EventFinalizer&) = delete;                         \
      inline EventFinalizer& operator=(const EventFinalizer&) = delete;       \
                                                                              \
      EventFinalizer(EventFinalizer&&) = default;                             \
      EventFinalizer& operator=(EventFinalizer&&) = delete;                   \
    } finalizer;                                                               \
     explicit PERFETTO_UID ( ScopedEvent ) ( perfetto::Track tTrack) : finalizer {std::move(tTrack)} { \
			  TRACE_EVENT_BEGIN ( category, name, finalizer.m_tTrack, ##__VA_ARGS__ );                \
			}  \
  } PERFETTO_UID(scoped_event) {track}

// track of current scheduler
#define SCHED_TRACK perfetto::Track::FromPointer ( Threads::Coro::CurrentScheduler() )

// track of current connection
#define CONN_TRACK perfetto::Track ( session::GetConnID() )

// any track
#define PERF_TRACK perfetto::Track

namespace Tracer
{

void Init();
void Start ( const CSphString& sFile, int64_t iMaxSize );
void Stop();

} // namespace Tracer

#else
// not def PERFETTO

#define TRACE_EVENT_BEGIN(...)
#define TRACE_EVENT_END(...)
#define SCOPED_TRACEP(...)
#define SCOPED_TRACE_LOCAL(...)
#define TRACE_COUNTER(...)

namespace Tracer
{

inline void Init () {};
inline void Start ( const CSphString& sFile, int64_t iMaxSize ) {};
inline void Stop() {};

} // namespace Tracer

#endif
// ifdef PEFRETTO


#define BEGIN_SCHED( category, name, ... ) TRACE_EVENT_BEGIN ( category, name, SCHED_TRACK, ##__VA_ARGS__ )
#define END_SCHED( category, ... ) TRACE_EVENT_END ( category, SCHED_TRACK, ##__VA_ARGS__ )
#define TRACE_SCHED( category, name, ... ) SCOPED_TRACEP ( category, name, SCHED_TRACK, ##__VA_ARGS__ )
#define TRACE_SCHED_ID( category, name, id, ... ) SCOPED_TRACEP ( category, name, PERF_TRACK ( id, SCHED_TRACK ), ##__VA_ARGS__ )
// use instead of TRACE_SCHED_ID when id is local variable, and 'error: reference to local variable 'iId' declared in enclosing' happens
#define TRACE_SCHED_VARID( category, name, id, ... ) SCOPED_TRACE_LOCAL ( category, name, PERF_TRACK ( id, SCHED_TRACK ), ##__VA_ARGS__ )

#define BEGIN_CONN( category, name, ... ) TRACE_EVENT_BEGIN ( category, name, CONN_TRACK, ##__VA_ARGS__ )
#define END_CONN( category, ... ) TRACE_EVENT_END ( category, CONN_TRACK, ##__VA_ARGS__ )
#define TRACE_CONN( category, name, ... ) SCOPED_TRACEP ( category, name, CONN_TRACK, ##__VA_ARGS__ )
#define TRACE_CONN_ID( category, name, id, ... ) SCOPED_TRACEP ( category, name, PERF_TRACK ( id, CONN_TRACK ), ##__VA_ARGS__ )
// use instead of TRACE_CONN when id is local variable, and 'error: reference to local variable 'iId' declared in enclosing' happens
#define TRACE_CONN_VARID( category, name, id, ... ) SCOPED_TRACE_LOCAL ( category, name, PERF_TRACK ( id, CONN_TRACK ), ##__VA_ARGS__ )

#define BEGIN_ID( category, name, id, ... ) TRACE_EVENT_BEGIN ( category, name, PERF_TRACK ( id ), ##__VA_ARGS__ )
#define END_ID( category, id, ... ) TRACE_EVENT_END ( category, PERF_TRACK ( id ), ##__VA_ARGS__ )
#define TRACE_ID( category, name, id, ... ) SCOPED_TRACEP ( category, name, PERF_TRACK ( id ), ##__VA_ARGS__ )
// use instead of TRACE_SCHED_ID when id is local variable, and 'error: reference to local variable 'iId' declared in enclosing' happens
#define TRACE_VARID( category, name, id, ... ) SCOPED_TRACE_LOCAL ( category, name, PERF_TRACK ( id ), ##__VA_ARGS__ )
