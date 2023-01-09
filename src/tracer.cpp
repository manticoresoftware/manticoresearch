//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "tracer.h"

#ifdef PERFETTO

#include "sphinxstd.h"
#include "fileio.h"
#include "fileutils.h"
#include "sphinxutils.h"
#include "perfetto.h"

class Trace2File_c
{
	bool m_bActive = false;
	CSphString m_sLastError;
	CSphAutofile m_fdLog;
	std::unique_ptr<perfetto::TracingSession> m_pTracing;

public:
	Trace2File_c () = default;
	~Trace2File_c()
	{
		Deinit();
	}

	const CSphString& GetLastError() const noexcept { return m_sLastError; }

	bool Init ( const CSphString& sFilename, int64_t iMaxSize )
	{
		sphWarning ( "Called tracer::Init with '%s', size " INT64_FMT, sFilename.scstr(), iMaxSize );
		if ( m_bActive )
			Deinit();

		if ( m_fdLog.Open ( sFilename.cstr(), SPH_O_NEW, m_sLastError ) >=0 )
		{
			// Event
			perfetto::protos::gen::TrackEventConfig tTrackEventCfg;
			tTrackEventCfg.add_disabled_categories ( "*" );
			tTrackEventCfg.add_enabled_categories ( "rt" );
			tTrackEventCfg.add_enabled_categories ( "network" );
			tTrackEventCfg.add_enabled_categories ( "conn" );
			tTrackEventCfg.add_enabled_categories ( "wait" );
			tTrackEventCfg.add_enabled_categories ( "mem" );

			// Trace
			perfetto::TraceConfig tTraceCfg;
			tTraceCfg.add_buffers()->set_size_kb ( iMaxSize ); // Record up to given limit

			// Trace data source
			auto* pDataSourceCfg = tTraceCfg.add_data_sources()->mutable_config();
			pDataSourceCfg->set_name ( "track_event" );
			pDataSourceCfg->set_track_event_config_raw ( tTrackEventCfg.SerializeAsString() );

			// start tracing
			m_pTracing = perfetto::Tracing::NewTrace();
			m_pTracing->Setup ( tTraceCfg, m_fdLog.GetFD() );
			m_pTracing->StartBlocking();

			m_bActive = true;
		}

		return m_bActive;
	}

	bool Deinit()
	{
		sphWarning ( "Called tracer::Deinit()" );
		if ( !m_bActive )
			return false;

		perfetto::TrackEvent::Flush();
		m_pTracing->StopBlocking();
		m_pTracing.reset();
		CSphString sFile = m_fdLog.GetFilename();
		m_fdLog.Close();
		auto iWritten = sphGetFileSize ( sFile );
		m_bActive = false;
		sphWarning ( "tracer::Deinit(): " INT64_FMT " bytes written to '%s'", iWritten, sFile.cstr() );
		return true;
	}
};

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

static Trace2File_c& GetTracer()
{
	static Trace2File_c tTracer;
	return tTracer;
}


void Tracer::Start ( const CSphString& sFile, int64_t iMaxSize )
{
	GetTracer().Init ( sFile, iMaxSize );
}

void Tracer::Stop()
{
	GetTracer().Deinit ();
}

void Tracer::Init()
{
	perfetto::TracingInitArgs args;
	args.backends |= perfetto::kInProcessBackend;
	//	args.backends |= perfetto::kSystemBackend;
	perfetto::Tracing::Initialize ( args );
	perfetto::TrackEvent::Register();
}

#endif