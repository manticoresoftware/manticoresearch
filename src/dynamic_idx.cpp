//
// Copyright (c) 2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "dynamic_idx.h"
#include "sphinxsort.h"

using namespace Threads;

class Feeder_c : public RowBuffer_i
{
	CSphSchema* 	m_pSchema = nullptr;
	CSphMatch*		m_pMatch = nullptr;
	Resumer_fn		m_fnCoro;
	bool 			m_bCoroFinished = false;
	bool			m_bHaveMoreMatches = true;
	bool 			m_bAutoID = true;

	int 			m_iCurCol = 0;
	int				m_iCurMatch = 1;

	bool CallCoro()
	{
		if ( !m_bCoroFinished )
			m_bCoroFinished = m_fnCoro ();
		return m_bCoroFinished;
	}

	const CSphColumnInfo & GetNextCol()
	{
		assert ( m_pMatch );
		assert ( m_iCurCol>=0 );

		const CSphColumnInfo & dColQuery = m_pSchema->GetAttr ( m_iCurCol );
		++m_iCurCol;
		return dColQuery;
	}

	// insert column into schema
	void ColSchema ( const char * szName, MysqlColumnType_e uType )
	{
		ESphAttr eType = SPH_ATTR_STRINGPTR;
		switch ( uType )
		{
		case MYSQL_COL_LONGLONG:	eType = SPH_ATTR_BIGINT; break;
		case MYSQL_COL_LONG:		eType = SPH_ATTR_INTEGER; break;
		case MYSQL_COL_FLOAT:		eType = SPH_ATTR_FLOAT;	break;
		case MYSQL_COL_DOUBLE:		eType = SPH_ATTR_DOUBLE; break;
		default: break;
		}
		CSphString sName ( szName );
		sName.ToLower();
		if ( m_pSchema->GetAttrIndex ( sName.cstr() )<0 )
			m_pSchema->AddAttr ( CSphColumnInfo ( szName, eType ), true );
		else {
			assert ( sName=="id");
			m_bAutoID = false;
		}
	}

public:
	StringBuilder_c m_sErrors { "; " };

public:
	explicit Feeder_c ( TableFeeder_fn fnFeed )
	{
		m_fnCoro = MakeCoroExecutor ( [this, fnFeed = std::move ( fnFeed )] () { fnFeed ( this ); } );
	}

	~Feeder_c() override
	{
		while ( !m_bCoroFinished )
			CallCoro ();
	}

	// collecting schema
	void SetSchema ( CSphSchema * pSchema )
	{
		m_pSchema = pSchema;
		CallCoro(); // at finish fnCoro will be before returning from HeadEnd().
	}

	// set upstream match
	void SetSorterStuff ( CSphMatch * pMatch )
	{
		m_pMatch = pMatch;
	}

	bool FillNextMatch()
	{
		if ( m_bHaveMoreMatches )
		{
			m_iCurCol = 0;
			if ( m_bAutoID )
			{
				auto * pID = m_pSchema->GetAttr ( sphGetDocidName () );
				m_pMatch->SetAttr ( pID->m_tLocator, m_iCurMatch );
				++m_iCurCol;
			}
			++m_iCurMatch;
			CallCoro ();
		}

		return m_bHaveMoreMatches;
	}

	void PutStr ( const CSphColumnInfo & tCol, const StringBuilder_c & sMsg )
	{
		assert ( m_pMatch );
		assert ( tCol.m_eAttrType == SPH_ATTR_STRINGPTR );
		BYTE * pData = nullptr;
		m_pMatch->SetAttr ( tCol.m_tLocator, (SphAttr_t) sphPackPtrAttr ( sMsg.GetLength (), &pData ) );
		memcpy ( pData, sMsg.cstr (), sMsg.GetLength () );
	}

public:
	// Header of the table with defined num of columns
	inline void HeadBegin ( int ) override
	{
		if ( !m_pSchema )
			return;

		// add id column
		ColSchema ( sphGetDocidName (), MYSQL_COL_LONGLONG );
	}

	// add the next column.
	void HeadColumn ( const char * sName, MysqlColumnType_e uType ) override
	{
		if ( m_pSchema )
			ColSchema ( sName, uType );
	}

	bool HeadEnd ( bool bMoreResults, int iWarns ) override
	{
		if ( !m_pSchema )
		{
			assert (false && "dynamic table invoked without parent schema");
			return false;
		}

		Coro::Yield_();
		return true;
	}

	// match constructing routines
	void PutFloatAsString ( float fVal, const char * sFormat ) override
	{
		if ( !m_pMatch )
			return;

		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;
		if ( tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
			tMatch.SetAttrFloat ( tCol.m_tLocator, fVal );
		else
		{
			StringBuilder_c sData;
			sData.Appendf ( "%f", fVal );
			PutStr ( tCol, sData );
		}
	}

	void PutDoubleAsString ( double fVal, const char * szFormat ) override
	{
		if ( !m_pMatch )
			return;

		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;
		if ( tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
			tMatch.SetAttrDouble ( tCol.m_tLocator, fVal );
		else
		{
			StringBuilder_c sData;
			sData.Appendf ( "%f", fVal );
			PutStr ( tCol, sData );
		}
	}

	void PutPercentAsString ( int64_t iVal, int64_t iBase ) override
	{
		if ( iBase )
			PutFloatAsString ( iVal * 100.0f / iBase, nullptr );
		else
			PutFloatAsString ( 100.0f, nullptr );
	}

	void PutNumAsString ( int64_t iVal ) override
	{
		if ( !m_pMatch )
			return;
		auto& tCol = GetNextCol();
		auto & tMatch = *m_pMatch;

		if ( tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
			tMatch.SetAttr ( tCol.m_tLocator, iVal );
		else
		{
			StringBuilder_c sData;
			sData << iVal;
			PutStr ( tCol, sData );
		}
	}

	void PutNumAsString ( uint64_t uVal ) override
	{
		if ( !m_pMatch )
			return;
		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;

		if ( tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
			tMatch.SetAttr ( tCol.m_tLocator, uVal );
		else
		{
			StringBuilder_c sData;
			sData << uVal;
			PutStr ( tCol, sData );
		}
	}

	void PutNumAsString ( int iVal ) override
	{
		if ( !m_pMatch )
			return;
		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;

		if ( tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
			tMatch.SetAttr ( tCol.m_tLocator, iVal );
		else
		{
			StringBuilder_c sData;
			sData << iVal;
			PutStr ( tCol, sData );
		}
	}

	void PutNumAsString ( DWORD uVal ) override
	{
		if ( !m_pMatch )
			return;
		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;

		if ( tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
			tMatch.SetAttr ( tCol.m_tLocator, uVal );
		else
		{
			StringBuilder_c sData;
			sData << uVal;
			PutStr ( tCol, sData );
		}
	}

	void PutArray ( const ByteBlob_t&, bool ) override {}

	// pack string
	void PutString ( Str_t sMsg ) override
	{
		if ( !m_pMatch )
			return;

		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;

		BYTE * pData = nullptr;
		tMatch.SetAttr ( tCol.m_tLocator, (SphAttr_t) sphPackPtrAttr ( sMsg.second, &pData ) );
		memcpy ( pData, sMsg.first, sMsg.second );
	}

	void PutMicrosec ( int64_t iUsec ) override
	{
		if ( !m_pMatch )
			return;
		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;

		if ( tCol.m_eAttrType == SPH_ATTR_FLOAT )
		{
			auto fSec = (float)iUsec / 1000000.0f;
			tMatch.SetAttr ( tCol.m_tLocator, sphF2DW ( fSec ) );
		} else if ( tCol.m_eAttrType==SPH_ATTR_STRINGPTR )
		{
			StringBuilder_c sData;
			sData << iUsec;
			PutStr ( tCol, sData );
		} else
			tMatch.SetAttr ( tCol.m_tLocator, iUsec );
	}

	void PutNULL () override
	{
		if ( !m_pMatch )
			return;
		auto & tCol = GetNextCol ();
		auto & tMatch = *m_pMatch;

		if ( tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
			tMatch.SetAttr ( tCol.m_tLocator, 0 );
		else
		{
			StringBuilder_c sData;
			sData << 0;
			PutStr ( tCol, sData );
		}
	}

public:
	/// more high level. Processing the whole tables.
	// sends collected data, then reset
	bool Commit() override
	{
		Coro::Yield_ ();
		return m_bHaveMoreMatches; // true for continue iteration, false to stop
	}

	// wrappers for popular packets
	void Eof ( bool bMoreResults, int iWarns ) override
	{
		m_bHaveMoreMatches = false;
		m_pMatch = nullptr; // that should stop any further feeding
		Coro::Yield_ (); // generally not need as eof is usually the last stmt, but if not it is safe
	}

	void Error ( const char * sStmt, const char * sError, MysqlErrors_e ) override
	{
		m_sErrors.Sprintf("%s: %s", sStmt, sError);
		Eof (false,0);
	}
	void Ok ( int, int, const char *, bool, int64_t ) override {}
	void Add ( BYTE ) override {}
};

// feed schema only and skip all the data
class FeederSchema_c : public RowBuffer_i
{
	CSphSchema* 	m_pSchema = nullptr;
	CSphMatch*		m_pMatch = nullptr;
	Resumer_fn		m_fnCoro;
	bool 			m_bCoroFinished = false;
	bool			m_bHaveMoreMatches = true;

	int				m_iCurMatch = 1;

	bool CallCoro()
	{
		if ( !m_bCoroFinished )
			m_bCoroFinished = m_fnCoro ();
		return m_bCoroFinished;
	}

	void PutString ( int iCol, const char * sMsg )
	{
		if ( !m_pMatch )
			return;
		int iLen = ( sMsg && *sMsg ) ? (int) strlen ( sMsg ) : 0;

		if ( !sMsg )
			sMsg = "";

		BYTE * pData = nullptr;
		m_pMatch->SetAttr ( m_pSchema->GetAttr ( iCol ).m_tLocator, (SphAttr_t) sphPackPtrAttr ( iLen, &pData ) );
		memcpy ( pData, sMsg, iLen );
	}

public:
	StringBuilder_c m_sErrors { "; " };

public:
	explicit FeederSchema_c ( TableFeeder_fn fnFeed )
	{
		m_fnCoro = MakeCoroExecutor ( [this, fnFeed = std::move ( fnFeed )] () { fnFeed ( this ); } );
	}

	~FeederSchema_c() override
	{
		while ( !m_bCoroFinished )
			CallCoro ();
	}

	// collecting schema
	void SetSchema ( CSphSchema * pSchema )
	{
		m_pSchema = pSchema;
		m_pSchema->AddAttr ( CSphColumnInfo ( sphGetDocidName (), SPH_ATTR_BIGINT ), true );
		m_pSchema->AddAttr ( CSphColumnInfo ( "Field", SPH_ATTR_STRINGPTR ), true );
		m_pSchema->AddAttr ( CSphColumnInfo ( "Type", SPH_ATTR_STRINGPTR ), true );
		m_pSchema->AddAttr ( CSphColumnInfo ( "Properties", SPH_ATTR_STRINGPTR ), true );
	}

	// set upstream match
	void SetSorterStuff ( CSphMatch * pMatch )
	{
		m_pMatch = pMatch;
	}

	bool FillNextMatch()
	{
		if ( m_bHaveMoreMatches )
			CallCoro ();

		return m_bHaveMoreMatches;
	}

public:
	void HeadBegin ( int ) override {}

	// add the next column.
	void HeadColumn ( const char * sName, MysqlColumnType_e uType ) override
	{
		if ( !m_pSchema )
			return;

		if ( !m_pMatch )
			return;

		// docid
		m_pMatch->SetAttr ( m_pSchema->GetAttr ( 0 ).m_tLocator, m_iCurMatch );
		++m_iCurMatch;

		PutString ( 1, sName );
		switch ( uType )
		{
			case MYSQL_COL_LONGLONG:	PutString ( 2, "bigint" ); break;
			case MYSQL_COL_LONG:		PutString ( 2, "uint" ); break;
			case MYSQL_COL_FLOAT:		PutString ( 2, "float" ); break;
			case MYSQL_COL_DOUBLE:		PutString ( 2, "double" ); break;
			default:					PutString ( 2, "string" ); break;
		}
		PutString ( 3, "" );

		Coro::Yield_ ();
	}

	bool HeadEnd ( bool bMoreResults, int iWarns ) override
	{
		if ( !m_pSchema )
		{
			assert (false && "dynamic table invoked without parent schema");
			return false;
		}

		// fixme!
		m_bHaveMoreMatches = false;
		m_pMatch = nullptr; // that should stop any further feeding
		Coro::Yield_ ();
		return false;
	}

	// match constructing routines (empty for schema only)
	void PutFloatAsString ( float, const char * ) override {}
	void PutDoubleAsString ( double, const char * ) override {}
	void PutPercentAsString ( int64_t, int64_t ) override {}
	void PutNumAsString ( int64_t ) override {}
	void PutNumAsString ( uint64_t ) override {}
	void PutNumAsString ( int ) override {}
	void PutNumAsString ( DWORD ) override {}
	void PutArray ( const ByteBlob_t&, bool ) override {}
	void PutString ( Str_t ) override {}
	void PutMicrosec ( int64_t ) override {}
	void PutNULL () override {}
	bool Commit() override { return false;}
	void Eof ( bool, int ) override {}
	void Error ( const char * sStmt, const char * sError, MysqlErrors_e ) override
	{
		m_sErrors.Sprintf ( "%s:%s", sStmt, sError );
		Eof ( false, 0 );
	}
	void Ok ( int, int, const char *, bool, int64_t ) override {}
	void Add ( BYTE ) override {}
};

class GenericTableIndex_c : public CSphIndexStub
{
public:
	GenericTableIndex_c ()
		: CSphIndexStub ( "dynamic", nullptr )
	{}

	bool				MultiQuery ( CSphQueryResult & , const CSphQuery & , const VecTraits_T<ISphMatchSorter *> &, const CSphMultiQueryArgs & ) const final;

private:
	bool MultiScan ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const;

	virtual void SetSorterStuff ( CSphMatch * pMatch ) const = 0;
	virtual bool FillNextMatch () const = 0;
	virtual const StringBuilder_c& GetErrors() const = 0;
};

bool GenericTableIndex_c::MultiQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery,
		const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs &tArgs ) const
{
	MEMORY ( MEM_DISK_QUERY );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter *> dSorters;
	dSorters.Reserve ( dAllSorters.GetLength() );
	dAllSorters.Apply ([&dSorters] ( ISphMatchSorter* p) { if ( p ) dSorters.Add(p); });

	// if we have anything to work with
	if ( dSorters.IsEmpty() )
		return false;

	// non-random at the start, random at the end
	dSorters.Sort ( CmpPSortersByRandom_fn () );

	const QueryParser_i * pQueryParser = tQuery.m_pQueryParser;
	assert ( pQueryParser );

	// fast path for scans
	if ( pQueryParser->IsFullscan ( tQuery ) )
		return MultiScan ( tResult, tQuery, dSorters, tArgs );

	return false;
}

class DynMatchProcessor_c : public MatchProcessor_i, ISphNoncopyable
{
public:
	DynMatchProcessor_c ( int iTag, const CSphQueryContext &tCtx )
		: m_iTag ( iTag )
		, m_tCtx ( tCtx )
	{}

	void Process ( CSphMatch * pMatch ) final			{ ProcessMatch(pMatch); }
	bool ProcessInRowIdOrder() const final				{ return false;	}
	void Process ( VecTraits_T<CSphMatch *> & dMatches ) final { dMatches.for_each ( [this]( CSphMatch * pMatch ){ ProcessMatch(pMatch); } ); }

private:
	int							m_iTag;
	const CSphQueryContext &	m_tCtx;

	inline void ProcessMatch ( CSphMatch * pMatch )
	{
		if ( pMatch->m_iTag>=0 )
			return;

		m_tCtx.CalcFinal ( *pMatch );
		pMatch->m_iTag = m_iTag;
	}
};


bool GenericTableIndex_c::MultiScan ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const
{
	assert ( tArgs.m_iTag>=0 );
	auto & tMeta = *tResult.m_pMeta;

	QueryProfile_c * pProfiler = tMeta.m_pProfile;

	// we count documents only (before filters)
	if ( tQuery.m_iMaxPredictedMsec )
		tMeta.m_bHasPrediction = true;

	if ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE )
		tMeta.m_sWarning.SetSprintf ( "packedfactors() will not work with a fullscan; you need to specify a query" );

	// start counting
	int64_t tmQueryStart = sphMicroTimer ();
	MiniTimer_c dTimerGuard;
	int64_t tmMaxTimer = dTimerGuard.Engage ( tQuery.m_uMaxQueryMsec ); // max_query_time

	// select the sorter with max schema
	// uses GetAttrsCount to get working facets (was GetRowSize)
	int iMaxSchemaIndex = GetMaxSchemaIndexAndMatchCapacity ( dSorters ).first;
	const ISphSchema & tMaxSorterSchema = *( dSorters[iMaxSchemaIndex]->GetSchema ());
	auto dSorterSchemas = SorterSchemas ( dSorters, iMaxSchemaIndex );

	// setup calculations and result schema
	CSphQueryContext tCtx ( tQuery );

	if ( !tCtx.SetupCalc ( tMeta, tMaxSorterSchema, m_tSchema, nullptr, nullptr, dSorterSchemas ) )
		return false;

	// setup filters
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &tQuery.m_dFilters;
	tFlx.m_pFilterTree = &tQuery.m_dFilterTree;
	tFlx.m_pSchema = &tMaxSorterSchema;
	tFlx.m_eCollation = tQuery.m_eCollation;
	tFlx.m_bScan = true;

	if ( !tCtx.CreateFilters ( tFlx , tMeta.m_sError, tMeta.m_sWarning ) )
		return false;

	// prepare to work them rows
	bool bRandomize = dSorters[0]->IsRandom();

	CSphMatch tMatch;
	// note: we reserve dynamic area in match using max sorter schema, but then fill it by locators from index schema.
	// that works relying that sorter always includes all attrs from index, leaving final selection of cols
	// to result minimizer. Once we try to pre-optimize sorter schema by select list, it will cause crashes here.
	tMatch.Reset ( tMaxSorterSchema.GetDynamicSize () );
	tMatch.m_iWeight = tArgs.m_iIndexWeight;
	tMatch.m_iTag = tCtx.m_dCalcFinal.GetLength () ? -1 : tArgs.m_iTag;

	CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_FULLSCAN );

	int iCutoff = ( tQuery.m_iCutoff<=0 ) ? -1 : tQuery.m_iCutoff;

	SetSorterStuff ( &tMatch );

	while ( FillNextMatch() )
	{
		++tMeta.m_tStats.m_iFetchedDocs;

		tCtx.CalcFilter ( tMatch );
		if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
		{
			tCtx.FreeDataFilter ( tMatch );
			m_tSchema.FreeDataPtrs ( tMatch );
			continue;
		}

		if ( bRandomize )
			tMatch.m_iWeight = ( sphRand () & 0xffff ) * tArgs.m_iIndexWeight;

		// submit match to sorters
		tCtx.CalcSort ( tMatch );

		bool bNewMatch = false;
		dSorters.Apply ( [&tMatch, &bNewMatch] ( ISphMatchSorter * p ) { bNewMatch |= p->Push ( tMatch ); } );

		// stringptr expressions should be duplicated (or taken over) at this point
		tCtx.FreeDataFilter ( tMatch );
		tCtx.FreeDataSort ( tMatch );
		m_tSchema.FreeDataPtrs ( tMatch );

		// handle cutoff
		if ( bNewMatch && --iCutoff==0 )
			break;

		// handle timer
		if ( sph::TimeExceeded ( tmMaxTimer ) )
		{
			tMeta.m_sWarning = "query time exceeded max_query_time";
			break;
		}

		if ( Threads::Coro::RuntimeExceeded() )
		{
			if ( session::GetKilled() )
			{
				tMeta.m_sWarning = "query was killed";
				break;
			}
			Threads::Coro::RescheduleAndKeepCrashQuery();
		}
	}

	auto& sErrors = GetErrors();
	bool bOk = sErrors.IsEmpty();
	if ( !bOk )
		tMeta.m_sError = (CSphString) sErrors;

	SwitchProfile ( pProfiler, SPH_QSTATE_FINALIZE );

	// do final expression calculations
	if ( tCtx.m_dCalcFinal.GetLength () )
	{
		DynMatchProcessor_c tFinal ( tArgs.m_iTag, tCtx );
		dSorters.Apply ( [&] ( ISphMatchSorter * p ) { p->Finalize ( tFinal, false, tArgs.m_bFinalizeSorters ); } );
	}

	tMeta.m_iQueryTime += ( int ) ( ( sphMicroTimer () - tmQueryStart ) / 1000 );

	return bOk;
}

///////////////
/// Index for data flow
class DynamicIndex_c final: public GenericTableIndex_c
{
	mutable Feeder_c		m_tFeeder;
	mutable bool m_bSchemaCreated = false;

public:
	explicit DynamicIndex_c ( TableFeeder_fn fnFeed)
		: m_tFeeder ( std::move ( fnFeed ) )
	{}

	const CSphSchema & GetMatchSchema () const final;

protected:
	~DynamicIndex_c() final = default;

private:
	void SetSorterStuff ( CSphMatch * pMatch ) const final;
	bool FillNextMatch () const final;
	const StringBuilder_c & GetErrors () const final;
};


const CSphSchema & DynamicIndex_c::GetMatchSchema () const
{
	if ( !m_bSchemaCreated )
	{
		m_tFeeder.SetSchema ( const_cast<CSphSchema *> (&m_tSchema) );
		m_bSchemaCreated = true;
	}
	return m_tSchema;
}

void DynamicIndex_c::SetSorterStuff ( CSphMatch * pMatch ) const
{
	assert ( m_bSchemaCreated );
	m_tFeeder.SetSorterStuff(pMatch);
}

bool DynamicIndex_c::FillNextMatch () const
{
	return m_tFeeder.FillNextMatch();
}

const StringBuilder_c & DynamicIndex_c::GetErrors () const
{
	return m_tFeeder.m_sErrors;
}

///////////////
/// Index for schema data flow
class DynamicIndexSchema_c final : public GenericTableIndex_c
{
	mutable FeederSchema_c		m_tFeeder;
	mutable bool m_bSchemaCreated = false;

public:
	explicit DynamicIndexSchema_c ( TableFeeder_fn fnFeed)
		: m_tFeeder ( std::move ( fnFeed ) )
	{}

	const CSphSchema & GetMatchSchema () const final;

protected:
	~DynamicIndexSchema_c() final = default;

private:
	void SetSorterStuff ( CSphMatch * pMatch ) const final;
	bool FillNextMatch () const final;
	const StringBuilder_c & GetErrors () const final;
};


const CSphSchema & DynamicIndexSchema_c::GetMatchSchema () const
{
	if ( !m_bSchemaCreated )
	{
		m_tFeeder.SetSchema ( const_cast<CSphSchema *> (&m_tSchema) );
		m_bSchemaCreated = true;
	}
	return m_tSchema;
}

void DynamicIndexSchema_c::SetSorterStuff ( CSphMatch * pMatch ) const
{
	assert ( m_bSchemaCreated );
	m_tFeeder.SetSorterStuff(pMatch);
}

bool DynamicIndexSchema_c::FillNextMatch () const
{
	return m_tFeeder.FillNextMatch();
}

const StringBuilder_c & DynamicIndexSchema_c::GetErrors () const
{
	return m_tFeeder.m_sErrors;
}

static ServedIndexRefPtr_c MakeServed ( CSphIndex* pIndex )
{
	auto pServed = MakeServedIndex();
	pServed->SetIdx ( std::unique_ptr<CSphIndex> ( pIndex ) );
	return pServed;
}

/// external functions
ServedIndexRefPtr_c MakeDynamicIndex ( TableFeeder_fn fnFeed )
{
	return MakeServed ( new DynamicIndex_c ( std::move ( fnFeed ) ) );
}

ServedIndexRefPtr_c MakeDynamicIndexSchema ( TableFeeder_fn fnFeed )
{
	return MakeServed ( new DynamicIndexSchema_c ( std::move ( fnFeed ) ) );
}
