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

#include "queuecreator.h"

#include "std/openhash.h"
#include "schema/rset.h"
#include "columnargrouper.h"
#include "columnarsort.h"
#include "exprgeodist.h"
#include "exprremap.h"
#include "exprdocstore.h"
#include "sphinxjson.h"
#include "joinsorter.h"
#include "columnarexpr.h"
#include "sphinxfilter.h"
#include "queryprofile.h"
#include "knnmisc.h"
#include "sorterscroll.h"

static const char g_sIntAttrPrefix[] = "@int_attr_";
static const char g_sIntJsonPrefix[] = "@groupbystr_";


bool HasImplicitGrouping ( const CSphQuery & tQuery )
{
	auto fnIsImplicit = [] ( const CSphQueryItem & t )
	{
		return ( t.m_eAggrFunc!=SPH_AGGR_NONE ) || t.m_sExpr=="count(*)" || t.m_sExpr=="@distinct";
	};

	return tQuery.m_sGroupBy.IsEmpty() ? tQuery.m_dItems.any_of(fnIsImplicit) : false;
}


bool sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema )
{
	return !tQuery.m_dItems.all_of ( [&tSchema] ( const CSphQueryItem& tItem )
		{
			const CSphString & sExpr = tItem.m_sExpr;

			// all expressions that come from parser are automatically aliased
			assert ( !tItem.m_sAlias.IsEmpty() );

			return sExpr=="*"
				|| ( tSchema.GetAttrIndex ( sExpr.cstr() )>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE && tItem.m_sAlias==sExpr )
				|| IsGroupbyMagic ( sExpr );
		});
}


int GetAliasedAttrIndex ( const CSphString & sAttr, const CSphQuery & tQuery, const ISphSchema & tSchema )
{
	int iAttr = tSchema.GetAttrIndex ( sAttr.cstr() );
	if ( iAttr>=0 )
		return iAttr;

	// try aliased groupby attr (facets)
	ARRAY_FOREACH ( i, tQuery.m_dItems )
	{
		if ( sAttr==tQuery.m_dItems[i].m_sExpr )
			return tSchema.GetAttrIndex ( tQuery.m_dItems[i].m_sAlias.cstr() );
		else if ( sAttr==tQuery.m_dItems[i].m_sAlias )
			return tSchema.GetAttrIndex ( tQuery.m_dItems[i].m_sExpr.cstr() );
	}

	return iAttr;
}


static bool IsCount ( const CSphString & s )
{
	return s=="@count" || s=="count(*)";
}


static bool IsGroupby ( const CSphString & s )
{
	return s=="@groupby"
		|| s=="@distinct"
		|| s=="groupby()"
		|| IsSortJsonInternal(s);
}


bool IsGroupbyMagic ( const CSphString & s )
{
	return IsGroupby ( s ) || IsCount ( s );
}


ESphAttr DetermineNullMaskType ( int iNumAttrs )
{
	if ( iNumAttrs<=32 )
		return SPH_ATTR_INTEGER;

	if ( iNumAttrs<=64 )
		return SPH_ATTR_BIGINT;

	return SPH_ATTR_STRINGPTR;
}


const char * GetInternalAttrPrefix()
{
	return g_sIntAttrPrefix;
}


const char * GetInternalJsonPrefix()
{
	return g_sIntJsonPrefix;
}


bool IsSortStringInternal ( const CSphString & sColumnName )
{
	assert ( sColumnName.cstr ());
	return ( strncmp ( sColumnName.cstr (), g_sIntAttrPrefix, sizeof ( g_sIntAttrPrefix )-1 )==0 );
}


bool IsSortJsonInternal ( const CSphString& sColumnName  )
{
	assert ( sColumnName.cstr ());
	return ( strncmp ( sColumnName.cstr (), g_sIntJsonPrefix, sizeof ( g_sIntJsonPrefix )-1 )==0 );
}


CSphString SortJsonInternalSet ( const CSphString& sColumnName )
{
	CSphString sName;
	if ( !sColumnName.IsEmpty() )
		( StringBuilder_c () << g_sIntJsonPrefix << sColumnName ).MoveTo ( sName );
	return sName;
}

///////////////////////////////////////////////////////////////////////////////

static bool ExprHasJoinPrefix ( const CSphString & sExpr, const JoinArgs_t * pArgs )
{
	if ( !pArgs )
		return false;

	CSphString sPrefix;
	sPrefix.SetSprintf ( "%s.", pArgs->m_sIndex2.cstr() );

	const char * szFound = strstr ( sExpr.cstr(), sPrefix.cstr() );
	if ( !szFound )
		return false;

	if ( szFound > sExpr.cstr() )
	{
		char c = *(szFound-1);
		if ( ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_' )
			return false;
	}

	return true;
}


static bool IsKnnDist ( const CSphString & sExpr )
{
	return sExpr==GetKnnDistAttrName() || sExpr=="knn_dist()";
}


static inline ESphSortKeyPart Attr2Keypart ( ESphAttr eType )
{
	switch ( eType )
	{
	case SPH_ATTR_FLOAT:
		return SPH_KEYPART_FLOAT;

	case SPH_ATTR_DOUBLE:
		return SPH_KEYPART_DOUBLE;

	case SPH_ATTR_STRING:
		return SPH_KEYPART_STRING;

	case SPH_ATTR_JSON:
	case SPH_ATTR_JSON_PTR:
	case SPH_ATTR_JSON_FIELD:
	case SPH_ATTR_JSON_FIELD_PTR:
	case SPH_ATTR_STRINGPTR:
		return SPH_KEYPART_STRINGPTR;

	default:
		return SPH_KEYPART_INT;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CSphGroupSorterSettings::FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	sphFixupLocator ( m_tLocGroupby, pOldSchema, pNewSchema );
	sphFixupLocator ( m_tLocCount, pOldSchema, pNewSchema );
	sphFixupLocator ( m_tLocDistinct, pOldSchema, pNewSchema );
	sphFixupLocator ( m_tLocGroupbyStr, pOldSchema, pNewSchema );

	if ( m_pDistinctFetcher )
		m_pDistinctFetcher->FixupLocators ( pOldSchema, pNewSchema );
}

void CSphGroupSorterSettings::SetupDistinctAccuracy ( int iThresh )
{
	if ( !iThresh )
	{
		m_iDistinctAccuracy = 0;
		return;
	}

	iThresh = int ( float(iThresh) / OpenHashTable_T<int,int>::GetLoadFactor() ) + 1;
	m_iDistinctAccuracy = iThresh ? sphLog2(iThresh) + 4 : 0;
	m_iDistinctAccuracy = Min ( m_iDistinctAccuracy, 18 );
	m_iDistinctAccuracy = Max ( m_iDistinctAccuracy, 14 );
}


///////////////////////////////////////////////////////////////////////////////

class QueueCreator_c
{
public:
	bool				m_bMulti = false;
	bool				m_bCreate = true;
	bool				m_bZonespanlist = false;
	DWORD				m_uPackedFactorFlags = SPH_FACTOR_DISABLE;
	bool				m_bJoinedGroupSort = false;		// do we need joined attrs for sorting/grouping?


						QueueCreator_c ( const SphQueueSettings_t & tSettings, const CSphQuery & tQuery, CSphString & sError, StrVec_t * pExtra, QueryProfile_c * pProfile );

	bool				SetupComputeQueue();
	bool				SetupGroupQueue();
	bool				SetupQueue();

	CSphRsetSchema &	SorterSchema() const { return *m_pSorterSchema; }
	bool				HasJson() const { return m_tGroupSorterSettings.m_bJson; }
	bool				SetSchemaGroupQueue ( const CSphRsetSchema & tNewSchema );

	/// creates proper queue for given query
	/// may return NULL on error; in this case, error message is placed in sError
	/// if the pUpdate is given, creates the updater's queue and perform the index update
	/// instead of searching
	ISphMatchSorter *	CreateQueue();

private:
	const SphQueueSettings_t &	m_tSettings;
	const CSphQuery &			m_tQuery;
	CSphString &				m_sError;
	StrVec_t *					m_pExtra = nullptr;
	QueryProfile_c *			m_pProfile = nullptr;

	bool						m_bHasCount = false;
	bool						m_bHasGroupByExpr = false;
	sph::StringSet				m_hQueryAttrs;
	std::unique_ptr<CSphRsetSchema> m_pSorterSchema;

	bool						m_bGotGroupby;
	bool						m_bRandomize;
	ESphSortFunc				m_eMatchFunc = FUNC_REL_DESC;
	ESphSortFunc				m_eGroupFunc = FUNC_REL_DESC;
	CSphMatchComparatorState	m_tStateMatch;
	CSphVector<ExtraSortExpr_t> m_dMatchJsonExprs;
	CSphMatchComparatorState	m_tStateGroup;
	CSphVector<ExtraSortExpr_t> m_dGroupJsonExprs;
	CSphGroupSorterSettings		m_tGroupSorterSettings;
	CSphVector<std::pair<int,bool>> m_dGroupColumns;
	StrVec_t					m_dGroupJsonAttrs;
	bool						m_bHeadWOGroup;
	bool						m_bGotDistinct;
	bool						m_bExprsNeedDocids = false;

	// for sorter to create pooled attributes
	bool						m_bHaveStar = false;

	// fixme! transform to StringSet on end of merge!
	sph::StringSet				m_hQueryColumns; // FIXME!!! unify with Extra schema after merge master into branch
	sph::StringSet				m_hQueryDups;
	sph::StringSet				m_hExtra;

	bool	ParseQueryItem ( const CSphQueryItem & tItem );
	bool	MaybeAddGeodistColumn();
	bool	MaybeAddExprColumn();
	bool	MaybeAddExpressionsFromSelectList();
	bool	AddExpressionsForUpdates();
	bool	MaybeAddGroupbyMagic ( bool bGotDistinct );
	bool	AddKNNDistColumn();
	bool	AddJoinAttrs();
	bool	CheckJoinOnTypeCast ( const CSphString & sIdx, const CSphString & sAttr, ESphAttr eTypeCast );
	bool	AddJoinFilterAttrs();
	bool	AddJsonJoinOnFilter ( const CSphString & sAttr1, const CSphString & sAttr2, ESphAttr eTypeCast );
	bool	AddNullBitmask();
	bool	AddColumnarJoinOnFilter ( const CSphString & sAttr );
	bool	CheckHavingConstraints() const;
	bool	SetupGroupbySettings ( bool bHasImplicitGrouping );
	void	AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount );
	void	AddAttrsFromSchema ( const ISphSchema & tSchema, const CSphString & sPrefix );
	void	ModifyExprForJoin ( CSphColumnInfo & tExprCol, const CSphString & sExpr );
	void	SelectExprEvalStage ( CSphColumnInfo & tExprCol );

	void	ReplaceGroupbyStrWithExprs ( CSphMatchComparatorState & tState, int iNumOldAttrs );
	void	ReplaceStaticStringsWithExprs ( CSphMatchComparatorState & tState );
	void	ReplaceJsonWithExprs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs );
	void	AddColumnarExprsAsAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs );
	void	RemapAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs );
	static void	SetupRemapColJson ( CSphColumnInfo & tRemapCol, CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs, int iStateAttr ) ;
	const CSphColumnInfo * GetGroupbyStr ( int iAttr, int iNumOldAttrs ) const;

	bool	SetupMatchesSortingFunc();
	bool	SetupGroupSortingFunc ( bool bGotDistinct );
	bool	AddGroupbyStuff();
	void	AddKnnDistSort ( CSphString & sSortBy );
	bool	ParseJoinExpr ( CSphColumnInfo & tExprCol, const CSphString & sAttr, const CSphString & sExpr ) const;
	bool	SetGroupSorting();
	void	ExtraAddSortkeys ( const int * dAttrs );
	bool	AddStoredFieldExpressions();
	bool	AddColumnarAttributeExpressions();
	void	CreateGrouperByAttr ( ESphAttr eType, const CSphColumnInfo & tGroupByAttr, bool & bGrouperUsesAttrs );
	void	SelectStageForColumnarExpr ( CSphColumnInfo & tExprCol );
	void	FetchDependencyChains ( StrVec_t & dDependentCols );
	void	PropagateEvalStage ( CSphColumnInfo & tExprCol, StrVec_t & dDependentCols );
	bool	SetupDistinctAttr();
	bool	PredictAggregates() const;
	bool	ReplaceWithColumnarItem ( const CSphString & sAttr, ESphEvalStage eStage );
	int		ReduceMaxMatches() const;
	int		AdjustMaxMatches ( int iMaxMatches ) const;
	bool	ConvertColumnarToDocstore();
	CSphString GetAliasedColumnarAttrName ( const CSphColumnInfo & tAttr ) const;
	bool	SetupAggregateExpr ( CSphColumnInfo & tExprCol, const CSphString & sExpr, DWORD uQueryPackedFactorFlags );
	bool	SetupColumnarAggregates ( CSphColumnInfo & tExprCol );
	bool	IsJoinAttr ( const CSphString & sAttr ) const;
	void	ReplaceJsonGroupbyWithStrings ( CSphString & sJsonGroupBy );
	void	UpdateAggregateDependencies ( CSphColumnInfo & tExprCol );
	int		GetGroupbyAttrIndex() const			{ return GetAliasedAttrIndex ( m_tQuery.m_sGroupBy, m_tQuery, *m_pSorterSchema ); }
	int		GetGroupDistinctAttrIndex() const	{ return GetAliasedAttrIndex ( m_tQuery.m_sGroupDistinct, m_tQuery, *m_pSorterSchema ); }

	bool	CanCalcFastCountDistinct() const;
	bool	CanCalcFastCountFilter() const;
	bool	CanCalcFastCount() const;
	PrecalculatedSorterResults_t FetchPrecalculatedValues() const;

	ISphMatchSorter *	SpawnQueue();
	std::unique_ptr<ISphFilter>	CreateAggrFilter() const;
	void				SetupCollation();
	bool				Err ( const char * sFmt, ... ) const;
};


QueueCreator_c::QueueCreator_c ( const SphQueueSettings_t & tSettings, const CSphQuery & tQuery, CSphString & sError, StrVec_t * pExtra, QueryProfile_c * pProfile )
	: m_tSettings ( tSettings )
	, m_tQuery ( tQuery )
	, m_sError ( sError )
	, m_pExtra ( pExtra )
	, m_pProfile ( pProfile )
	, m_pSorterSchema { std::make_unique<CSphRsetSchema>() }
{
	// short-cuts
	m_sError = "";
	*m_pSorterSchema = m_tSettings.m_tSchema;

	m_dMatchJsonExprs.Resize ( CSphMatchComparatorState::MAX_ATTRS );
	m_dGroupJsonExprs.Resize ( CSphMatchComparatorState::MAX_ATTRS );
}


CSphString QueueCreator_c::GetAliasedColumnarAttrName ( const CSphColumnInfo & tAttr ) const
{
	if ( !tAttr.IsColumnarExpr() )
		return tAttr.m_sName;

	CSphString sAliasedCol;
	tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sAliasedCol );
	return sAliasedCol;
}


void QueueCreator_c::CreateGrouperByAttr ( ESphAttr eType, const CSphColumnInfo & tGroupByAttr, bool & bGrouperUsesAttrs )
{
	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema;
	const CSphAttrLocator & tLoc = tGroupByAttr.m_tLocator;

	switch ( eType )
	{
	case SPH_ATTR_JSON:
	case SPH_ATTR_JSON_FIELD:
		{
			ExprParseArgs_t tExprArgs;
			tExprArgs.m_eCollation = m_tQuery.m_eCollation;

			ISphExprRefPtr_c pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(), tSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprArgs ) };
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperJsonField ( tLoc, pExpr );
			m_tGroupSorterSettings.m_bJson = true;
		}
		break;

	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR:
		// percolate select list push matches with string_ptr

		// check if it is a columnar attr or an expression spawned instead of a columnar attr
		// even if it is an expression, spawn a new one, because a specialized grouper works a lot faster because it doesn't allocate and store string in the match
		if ( tGroupByAttr.IsColumnar() || tGroupByAttr.IsColumnarExpr() )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperColumnarString ( GetAliasedColumnarAttrName(tGroupByAttr), m_tQuery.m_eCollation );
			bGrouperUsesAttrs = false;
		}
		else if ( tGroupByAttr.m_pExpr && !tGroupByAttr.m_pExpr->IsDataPtrAttr() )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperStringExpr ( tGroupByAttr.m_pExpr, m_tQuery.m_eCollation );
			bGrouperUsesAttrs = false;
		}
		else
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperString ( tLoc, m_tQuery.m_eCollation );
		break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
		if ( tGroupByAttr.IsColumnar() || tGroupByAttr.IsColumnarExpr() )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperColumnarMVA ( GetAliasedColumnarAttrName(tGroupByAttr), eType );
			bGrouperUsesAttrs = false;
		}
		else
		{
			if ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_UINT32SET_PTR )
				m_tGroupSorterSettings.m_pGrouper = CreateGrouperMVA32(tLoc);
			else
				m_tGroupSorterSettings.m_pGrouper = CreateGrouperMVA64(tLoc);
		}
		break;

	case SPH_ATTR_BOOL:
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BIGINT:
	case SPH_ATTR_FLOAT:
		if ( tGroupByAttr.IsColumnar() || ( tGroupByAttr.IsColumnarExpr() && tGroupByAttr.m_eStage>SPH_EVAL_PREFILTER ) )
		{
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperColumnarInt ( GetAliasedColumnarAttrName(tGroupByAttr), eType );
			bGrouperUsesAttrs = false;
		}
		break;

	default:
		break;
	}

	if ( !m_tGroupSorterSettings.m_pGrouper )
		m_tGroupSorterSettings.m_pGrouper = CreateGrouperAttr(tLoc);
}


bool QueueCreator_c::SetupDistinctAttr()
{
	const CSphString & sDistinct = m_tQuery.m_sGroupDistinct;
	if ( sDistinct.IsEmpty() )
		return true;

	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema;

	int iDistinct = tSchema.GetAttrIndex ( sDistinct.cstr() );
	if ( iDistinct<0 )
	{
		CSphString sJsonCol;
		if ( !sphJsonNameSplit ( sDistinct.cstr(), m_tQuery.m_sJoinIdx.cstr(), &sJsonCol ) )
		{
			return Err ( "group-count-distinct attribute '%s' not found", sDistinct.cstr() );
			return false;
		}

		CSphColumnInfo tExprCol ( sDistinct.cstr(), SPH_ATTR_JSON_FIELD_PTR );
		tExprCol.m_eStage = SPH_EVAL_SORTER;
		tExprCol.m_uAttrFlags = CSphColumnInfo::ATTR_JOINED;
		m_pSorterSchema->AddAttr ( tExprCol, true );
		iDistinct = m_pSorterSchema->GetAttrIndex ( tExprCol.m_sName.cstr() );
	}

	const auto & tDistinctAttr = tSchema.GetAttr(iDistinct);
	if ( IsNotRealAttribute(tDistinctAttr) )
		return Err ( "group-count-distinct attribute '%s' not found", sDistinct.cstr() );

	if ( tDistinctAttr.IsColumnar() )
		m_tGroupSorterSettings.m_pDistinctFetcher = CreateColumnarDistinctFetcher ( tDistinctAttr.m_sName, tDistinctAttr.m_eAttrType, m_tQuery.m_eCollation );
	else
		m_tGroupSorterSettings.m_pDistinctFetcher = CreateDistinctFetcher ( tDistinctAttr.m_sName, tDistinctAttr.m_tLocator, tDistinctAttr.m_eAttrType );

	m_bJoinedGroupSort |= IsJoinAttr(tDistinctAttr.m_sName);

	return true;
}


bool QueueCreator_c::SetupGroupbySettings ( bool bHasImplicitGrouping )
{
	if ( m_tQuery.m_sGroupBy.IsEmpty() && !bHasImplicitGrouping )
		return true;

	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_ATTRPAIR )
		return Err ( "SPH_GROUPBY_ATTRPAIR is not supported any more (just group on 'bigint' attribute)" );

	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema;

	m_tGroupSorterSettings.m_iMaxMatches = m_tSettings.m_iMaxMatches;

	if ( !SetupDistinctAttr() )
		return false;

	CSphString sJsonColumn;
	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_MULTIPLE )
	{
		CSphVector<CSphColumnInfo> dAttrs;
		VecRefPtrs_t<ISphExpr *> dJsonKeys;

		StrVec_t dGroupBy;
		sph::Split ( m_tQuery.m_sGroupBy.cstr (), -1, ",", [&] ( const char * sToken, int iLen )
		{
			CSphString sGroupBy ( sToken, iLen );
			sGroupBy.Trim ();
			dGroupBy.Add ( std::move ( sGroupBy ));
		} );
		dGroupBy.Uniq();

		for ( auto & sGroupBy : dGroupBy )
		{
			int iAttr = tSchema.GetAttrIndex ( sGroupBy.cstr() );

			CSphString sJsonExpr;
			if ( iAttr<0 && sphJsonNameSplit ( sGroupBy.cstr(), m_tQuery.m_sJoinIdx.cstr(), &sJsonColumn ) )
			{
				sJsonExpr = sGroupBy;
				sGroupBy = sJsonColumn;
			}

			iAttr = tSchema.GetAttrIndex ( sGroupBy.cstr() );
			if ( iAttr<0 )
				return Err( "group-by attribute '%s' not found", sGroupBy.cstr() );

			auto tAttr = tSchema.GetAttr ( iAttr );
			ESphAttr eType = tAttr.m_eAttrType;
			if ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET )
				return Err ( "MVA values can't be used in multiple group-by" );

			if ( eType==SPH_ATTR_JSON && sJsonExpr.IsEmpty() )
				return Err ( "JSON blob can't be used in multiple group-by" );

			dAttrs.Add ( tAttr );
			m_dGroupColumns.Add ( { iAttr, true } );
			m_dGroupJsonAttrs.Add(sJsonExpr);

			if ( !sJsonExpr.IsEmpty() )
			{
				ExprParseArgs_t tExprArgs;
				dJsonKeys.Add ( sphExprParse ( sJsonExpr.cstr(), tSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprArgs ) );
			}
			else if ( tAttr.m_eAttrType==SPH_ATTR_JSON_FIELD )
			{
				assert ( tAttr.m_pExpr );
				dJsonKeys.Add ( tAttr.m_pExpr->Clone() );
			} else
			{
				dJsonKeys.Add ( nullptr );
			}

			m_bJoinedGroupSort |= IsJoinAttr(sGroupBy);
		}

		m_tGroupSorterSettings.m_pGrouper = CreateGrouperMulti ( dAttrs, std::move(dJsonKeys), m_tQuery.m_eCollation );
		return true;
	}

	int iGroupBy = GetGroupbyAttrIndex();
	bool bJoined = iGroupBy>=0 && m_pSorterSchema->GetAttr(iGroupBy).IsJoined();
	if ( ( iGroupBy<0 || bJoined ) && sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr(), m_tQuery.m_sJoinIdx.cstr(), &sJsonColumn ) )
	{
		const int iAttr = tSchema.GetAttrIndex ( sJsonColumn.cstr() );
		if ( iAttr<0 )
			return Err ( "groupby: no such attribute '%s'", sJsonColumn.cstr ());

		if ( tSchema.GetAttr(iAttr).m_eAttrType!=SPH_ATTR_JSON
			&& tSchema.GetAttr(iAttr).m_eAttrType!=SPH_ATTR_JSON_PTR )
			return Err ( "groupby: attribute '%s' does not have subfields (must be sql_attr_json)", sJsonColumn.cstr() );

		if ( m_tQuery.m_eGroupFunc!=SPH_GROUPBY_ATTR )
			return Err ( "groupby: legacy groupby modes are not supported on JSON attributes" );

		m_dGroupColumns.Add ( { iAttr, true } );

		ExprParseArgs_t tExprArgs;
		tExprArgs.m_eCollation = m_tQuery.m_eCollation;

		ISphExprRefPtr_c pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(), tSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprArgs ) };
		m_tGroupSorterSettings.m_pGrouper = CreateGrouperJsonField ( tSchema.GetAttr(iAttr).m_tLocator, pExpr );
		m_tGroupSorterSettings.m_bJson = true;
		m_bJoinedGroupSort |= IsJoinAttr(sJsonColumn);
		return true;
	}

	if ( bHasImplicitGrouping )
	{
		m_tGroupSorterSettings.m_bImplicit = true;
		return true;
	}

	// setup groupby attr
	if ( iGroupBy<0 )
		return Err ( "group-by attribute '%s' not found", m_tQuery.m_sGroupBy.cstr() );

	const CSphColumnInfo & tGroupByAttr = tSchema.GetAttr(iGroupBy);
	if ( m_tSettings.m_bComputeItems && tGroupByAttr.m_pExpr && tGroupByAttr.m_pExpr->UsesDocstore() )
		return Err ( "unable to group by stored field '%s'", m_tQuery.m_sGroupBy.cstr() );

	ESphAttr eType = tGroupByAttr.m_eAttrType;
	CSphAttrLocator tLoc = tGroupByAttr.m_tLocator;
	m_bJoinedGroupSort |= IsJoinAttr ( tGroupByAttr.m_sName );
	bool bGrouperUsesAttrs = true;
	switch (m_tQuery.m_eGroupFunc )
	{
		case SPH_GROUPBY_DAY:
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperDay(tLoc); break;
		case SPH_GROUPBY_WEEK:
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperWeek(tLoc); break;
		case SPH_GROUPBY_MONTH:
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperMonth(tLoc); break;
		case SPH_GROUPBY_YEAR:
			m_tGroupSorterSettings.m_pGrouper = CreateGrouperYear(tLoc); break;
		case SPH_GROUPBY_ATTR:
			CreateGrouperByAttr ( eType, tGroupByAttr, bGrouperUsesAttrs );
			break;
		default:
			return Err ( "invalid group-by mode (mode=%d)", m_tQuery.m_eGroupFunc );
	}

	m_dGroupColumns.Add ( { iGroupBy, bGrouperUsesAttrs } );
	return true;
}

// move expressions used in ORDER BY or WITHIN GROUP ORDER BY to presort phase
void QueueCreator_c::AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount )
{
	if ( !iAttrCount )
		return;

	assert ( pAttrs );
	assert ( m_pSorterSchema );

	StrVec_t dCur;

	// add valid attributes to processing list
	for ( int i=0; i<iAttrCount; ++i )
		if ( pAttrs[i]>=0 )
			dCur.Add ( m_pSorterSchema->GetAttr ( pAttrs[i] ).m_sName );

	// collect columns which affect current expressions
	ARRAY_FOREACH ( i, dCur )
	{
		const CSphColumnInfo * pCol = m_pSorterSchema->GetAttr ( dCur[i].cstr() );
		assert(pCol);
		if ( pCol->m_eStage>SPH_EVAL_PRESORT && pCol->m_pExpr )
			pCol->m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
	}

	// get rid of dupes
	dCur.Uniq();

	// fix up of attributes stages
	for ( const auto & sAttr : dCur )
	{
		auto pCol = const_cast<CSphColumnInfo *>( m_pSorterSchema->GetAttr ( sAttr.cstr() ) );
		assert(pCol);
		if ( pCol->m_eStage==SPH_EVAL_FINAL )
			pCol->m_eStage = SPH_EVAL_PRESORT;
	}
}


void QueueCreator_c::ExtraAddSortkeys ( const int * dAttrs )
{
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; ++i )
		if ( dAttrs[i]>=0 )
		{
			const auto & tAttr = m_pSorterSchema->GetAttr ( dAttrs[i] );
			m_bJoinedGroupSort |= tAttr.IsJoined();
			m_hExtra.Add ( tAttr.m_sName );

			if ( m_tSettings.m_bComputeItems )
			{
				// check if dependent columns are joined
				StrVec_t dCols;
				dCols.Add ( tAttr.m_sName );
				FetchDependencyChains(dCols);
				for ( const auto & sAttr : dCols )
				{
					const CSphColumnInfo * pAttr = m_pSorterSchema->GetAttr ( sAttr.cstr() );
					assert(pAttr);
					m_bJoinedGroupSort |= pAttr->IsJoined();
				}
			}
		}
}


bool QueueCreator_c::Err ( const char * sFmt, ... ) const
{
	va_list ap;
	va_start ( ap, sFmt );
	m_sError.SetSprintfVa ( sFmt, ap );
	va_end ( ap );
	return false;
}


void QueueCreator_c::SelectStageForColumnarExpr ( CSphColumnInfo & tExprCol )
{
	if ( !tExprCol.IsColumnarExpr() )
	{
		tExprCol.m_eStage = SPH_EVAL_PREFILTER;
		return;
	}

	// columnar expressions are a special case
	// it is sometimes faster to evaluate them in the filter than to evaluate the expression, store it in the match and then use it in the filter

	// FIXME: add sorters?
	int iRank = 0;
	iRank += tExprCol.m_sName==m_tQuery.m_sGroupBy ? 1 : 0;
	iRank += m_tQuery.m_dFilters.any_of ( [&tExprCol]( const CSphFilterSettings & tFilter ) { return tFilter.m_sAttrName==tExprCol.m_sName; } ) ? 1 : 0;

	if ( iRank>1 )
		tExprCol.m_eStage = SPH_EVAL_PREFILTER;
}


void QueueCreator_c::FetchDependencyChains ( StrVec_t & dDependentCols )
{
	ARRAY_FOREACH ( i, dDependentCols )
	{
		const CSphString & sAttr = dDependentCols[i];
		int iAttr = m_pSorterSchema->GetAttrIndex ( sAttr.cstr() );
		assert ( iAttr>=0 );

		if ( m_pSorterSchema->IsRemovedAttr(iAttr) )
			continue;

		const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr(iAttr);

		int iOldLen = dDependentCols.GetLength();

		// handle chains of dependencies (e.g. SELECT 1+attr f1, f1-1 f2 ... WHERE f2>5)
		if ( tCol.m_pExpr )
			tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );

		// some expressions depend on the column they are attached to (json fast key)
		// so filter out duplicates to avoid circular dependencies
		for ( int iNewAttr = iOldLen; iNewAttr < dDependentCols.GetLength(); iNewAttr++ )
			if ( dDependentCols[iNewAttr]==dDependentCols[i] )
				dDependentCols.Remove(iNewAttr);
	}

	dDependentCols.Uniq();
}


void QueueCreator_c::PropagateEvalStage ( CSphColumnInfo & tExprCol, StrVec_t & dDependentCols )
{
	bool bWeight = false;
	for ( const auto & sAttr : dDependentCols )
	{
		const CSphColumnInfo * pCol = m_pSorterSchema->GetAttr ( sAttr.cstr() );
		assert(pCol);
		bWeight |= pCol->m_bWeight;
	}

	if ( bWeight )
	{
		tExprCol.m_eStage = SPH_EVAL_PRESORT;
		tExprCol.m_bWeight = true;
	}

	for ( const auto & sAttr : dDependentCols )
	{
		auto pDep = const_cast<CSphColumnInfo *> ( m_pSorterSchema->GetAttr ( sAttr.cstr() ) );
		if ( pDep->m_eStage > tExprCol.m_eStage )
			pDep->m_eStage = tExprCol.m_eStage;
	}
}


bool QueueCreator_c::SetupAggregateExpr ( CSphColumnInfo & tExprCol, const CSphString & sExpr, DWORD uQueryPackedFactorFlags )
{
	switch ( tExprCol.m_eAggrFunc )
	{
	case SPH_AGGR_AVG:
		// force AVG() to be computed in doubles
		tExprCol.m_eAttrType = SPH_ATTR_DOUBLE;
		tExprCol.m_tLocator.m_iBitCount = 64;
		break;

	case SPH_AGGR_CAT:
		// force GROUP_CONCAT() to be computed as strings
		tExprCol.m_eAttrType = SPH_ATTR_STRINGPTR;
		tExprCol.m_tLocator.m_iBitCount = ROWITEMPTR_BITS;
		break;

	case SPH_AGGR_SUM:
		if ( tExprCol.m_eAttrType==SPH_ATTR_BOOL )
		{
			tExprCol.m_eAttrType = SPH_ATTR_INTEGER;
			tExprCol.m_tLocator.m_iBitCount = 32;
		} else if ( tExprCol.m_eAttrType==SPH_ATTR_INTEGER )
		{
			tExprCol.m_eAttrType = SPH_ATTR_BIGINT;
			tExprCol.m_tLocator.m_iBitCount = 64;
		}
		break;

	default:
		break;
	}

	// force explicit type conversion for JSON attributes
	if ( tExprCol.m_eAggrFunc!=SPH_AGGR_NONE && tExprCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		return Err ( "ambiguous attribute type '%s', use INTEGER(), BIGINT() or DOUBLE() conversion functions", sExpr.cstr() );

	if ( uQueryPackedFactorFlags & SPH_FACTOR_JSON_OUT )
		tExprCol.m_eAttrType = SPH_ATTR_FACTORS_JSON;

	return true;
}


bool QueueCreator_c::SetupColumnarAggregates ( CSphColumnInfo & tExprCol )
{
	StrVec_t dDependentCols;
	tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );
	FetchDependencyChains(dDependentCols);

	if ( !dDependentCols.GetLength() )
		return tExprCol.IsColumnarExpr();

	if ( dDependentCols.GetLength()==1 )
	{
		int iAttr = m_pSorterSchema->GetAttrIndex ( dDependentCols[0].cstr() );
		assert ( iAttr>=0 );

		if ( m_pSorterSchema->IsRemovedAttr(iAttr) )
			return false;

		const CSphColumnInfo & tColumnarAttr = m_pSorterSchema->GetAttr(iAttr);
		if ( tColumnarAttr.IsColumnarExpr() )
		{
			CSphString sColumnarCol;
			tColumnarAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sColumnarCol );

			// let aggregate expression know that it is working with that columnar attribute
			tExprCol.m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR_COL, &sColumnarCol );

			return true;
		}
	}

	return false;
}


void QueueCreator_c::UpdateAggregateDependencies ( CSphColumnInfo & tExprCol )
{
	/// update aggregate dependencies (e.g. SELECT 1+attr f1, min(f1), ...)
	StrVec_t dDependentCols;
	tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );
	FetchDependencyChains ( dDependentCols );
	for ( const auto & sAttr : dDependentCols )
	{
		auto pDep = const_cast<CSphColumnInfo *>( m_pSorterSchema->GetAttr ( sAttr.cstr() ) );
		assert(pDep);

		bool bJoinedAttr = ExprHasJoinPrefix ( pDep->m_sName, m_tSettings.m_pJoinArgs.get() ) && pDep->m_eStage==SPH_EVAL_SORTER && pDep->m_tLocator.m_bDynamic && !pDep->m_pExpr;
		if ( pDep->m_eStage>tExprCol.m_eStage && !bJoinedAttr )
			pDep->m_eStage = tExprCol.m_eStage;
	}
}


void QueueCreator_c::AddAttrsFromSchema ( const ISphSchema & tSchema, const CSphString & sPrefix )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		CSphString sAttrName = tSchema.GetAttr(i).m_sName;
		sAttrName.SetSprintf ( "%s%s", sPrefix.scstr(), sAttrName.cstr() );
		m_hQueryDups.Add ( sAttrName );
		m_hQueryColumns.Add ( sAttrName );
	}
}


void QueueCreator_c::ModifyExprForJoin ( CSphColumnInfo & tExprCol, const CSphString & sExpr )
{
	// even if it's over a join expr, it references another attr, so don't remove the expression
	if ( tExprCol.m_eAggrFunc!=SPH_AGGR_NONE )
		return;

	// check expr and its alias
	if ( !ExprHasJoinPrefix ( tExprCol.m_sName, m_tSettings.m_pJoinArgs.get() ) && !ExprHasJoinPrefix ( sExpr, m_tSettings.m_pJoinArgs.get() ) )
		return;

	if ( ExprHasLeftTableAttrs ( tExprCol.m_sName, *m_pSorterSchema ) || ExprHasLeftTableAttrs ( sExpr, *m_pSorterSchema ) )
		return;

	// we receive already precalculated JSON field expressions from JOIN
	// we don't need to evaluate them once again
	tExprCol.m_eAttrType = sphPlainAttrToPtrAttr ( tExprCol.m_eAttrType );
	tExprCol.m_pExpr = nullptr;
	tExprCol.m_eStage = SPH_EVAL_SORTER;
	tExprCol.m_uAttrFlags |= CSphColumnInfo::ATTR_JOINED;
}


void QueueCreator_c::SelectExprEvalStage ( CSphColumnInfo & tExprCol )
{
	// is this expression used in filter?
	// OPTIMIZE? hash filters and do hash lookups?
	if ( tExprCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		return;

	if ( tExprCol.IsJoined() )
		return;

	ARRAY_FOREACH ( i, m_tQuery.m_dFilters )
		if ( m_tQuery.m_dFilters[i].m_sAttrName==tExprCol.m_sName )
		{
			// is this a hack?
			// m_bWeight is computed after EarlyReject() get called
			// that means we can't evaluate expressions with WEIGHT() in prefilter phase
			if ( tExprCol.m_bWeight )
			{
				tExprCol.m_eStage = SPH_EVAL_PRESORT; // special, weight filter ( short cut )
				break;
			}

			// so we are about to add a filter condition,
			// but it might depend on some preceding columns (e.g. SELECT 1+attr f1 ... WHERE f1>5)
			// lets detect those and move them to prefilter \ presort phase too
			StrVec_t dDependentCols;
			tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dDependentCols );

			SelectStageForColumnarExpr(tExprCol);
			FetchDependencyChains ( dDependentCols );
			PropagateEvalStage ( tExprCol, dDependentCols );

			break;
		}
}


bool QueueCreator_c::ParseQueryItem ( const CSphQueryItem & tItem )
{
	assert ( m_pSorterSchema );
	const CSphString & sExpr = tItem.m_sExpr;
	bool bIsCount = IsCount(sExpr);
	m_bHasCount |= bIsCount;

	if ( sExpr=="*" )
	{
		m_bHaveStar = true;
		CSphString sPrefix;
		if ( m_tSettings.m_pJoinArgs )
			sPrefix.SetSprintf ( "%s.", m_tSettings.m_pJoinArgs->m_sIndex1.cstr() );

		AddAttrsFromSchema ( m_tSettings.m_tSchema, sPrefix );

		if ( m_tSettings.m_pJoinArgs )
		{
			sPrefix.SetSprintf ( "%s.", m_tSettings.m_pJoinArgs->m_sIndex2.cstr() );
			AddAttrsFromSchema ( m_tSettings.m_pJoinArgs->m_tJoinedSchema, sPrefix );
		}
	}

	// for now, just always pass "plain" attrs from index to sorter; they will be filtered on searchd level
	int iAttrIdx = m_tSettings.m_tSchema.GetAttrIndex ( sExpr.cstr() );
	bool bColumnar = iAttrIdx>=0 && m_tSettings.m_tSchema.GetAttr(iAttrIdx).IsColumnar();
	bool bPlainAttr = ( ( sExpr=="*" || ( iAttrIdx>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE && !bColumnar ) ) &&
		( tItem.m_sAlias.IsEmpty() || tItem.m_sAlias==tItem.m_sExpr ) );
	if ( iAttrIdx>=0 )
	{
		ESphAttr eAttr = m_tSettings.m_tSchema.GetAttr ( iAttrIdx ).m_eAttrType;
		if ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR
			|| eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET )
		{
			if ( tItem.m_eAggrFunc!=SPH_AGGR_NONE )
				return Err ( "can not aggregate non-scalar attribute '%s'",	tItem.m_sExpr.cstr() );

			if ( !bPlainAttr && !bColumnar && ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR ) )
			{
				bPlainAttr = true;
				for ( const auto & i : m_tQuery.m_dItems )
					if ( sExpr==i.m_sAlias )
						bPlainAttr = false;
			}
		}
	}

	if ( bPlainAttr || IsGroupby ( sExpr ) || bIsCount )
	{
		if ( sExpr!="*" && !tItem.m_sAlias.IsEmpty() )
		{
			m_hQueryDups.Add ( tItem.m_sAlias );
			if ( bPlainAttr )
				m_hQueryColumns.Add ( tItem.m_sExpr );
		}
		m_bHasGroupByExpr = IsGroupby ( sExpr );
		return true;
	}

	if ( IsKnnDist(sExpr) && m_pSorterSchema->GetAttrIndex ( GetKnnDistAttrName() )<0 )
		return Err ( "KNN_DIST() is only allowed for KNN() queries" );

	// not an attribute? must be an expression, and must be aliased by query parser
	assert ( !tItem.m_sAlias.IsEmpty() );

	// tricky part
	// we might be fed with precomputed matches, but it's all or nothing
	// the incoming match either does not have anything computed, or it has everything
	// unless it is a JOIN - then we have partially precomputed matches
	int iSorterAttr = m_pSorterSchema->GetAttrIndex ( tItem.m_sAlias.cstr() );
	if ( iSorterAttr>=0 )
	{
		if ( m_hQueryDups[tItem.m_sAlias] )
		{
			bool bJoined = !!(m_pSorterSchema->GetAttr(iSorterAttr).m_uAttrFlags & CSphColumnInfo::ATTR_JOINED);
			if ( bColumnar || bJoined )	// we might have several similar aliases for columnar attributes (and they are not plain attrs but expressions)
				return true;
			else
				return Err ( "alias '%s' must be unique (conflicts with another alias)", tItem.m_sAlias.cstr() );
		}
	}

	// a new and shiny expression, lets parse
	CSphColumnInfo tExprCol ( tItem.m_sAlias.cstr(), SPH_ATTR_NONE );
	DWORD uQueryPackedFactorFlags = SPH_FACTOR_DISABLE;
	bool bHasZonespanlist = false;
	bool bExprsNeedDocids = false;

	ExprParseArgs_t tExprParseArgs;
	tExprParseArgs.m_pAttrType = &tExprCol.m_eAttrType;
	tExprParseArgs.m_pUsesWeight = &tExprCol.m_bWeight;
	tExprParseArgs.m_pProfiler = m_tSettings.m_pProfiler;
	tExprParseArgs.m_eCollation = m_tQuery.m_eCollation;
	tExprParseArgs.m_pHook = m_tSettings.m_pHook;
	tExprParseArgs.m_pZonespanlist = &bHasZonespanlist;
	tExprParseArgs.m_pPackedFactorsFlags = &uQueryPackedFactorFlags;
	tExprParseArgs.m_pEvalStage = &tExprCol.m_eStage;
	tExprParseArgs.m_pStoredField = &tExprCol.m_uFieldFlags;
	tExprParseArgs.m_pNeedDocIds = &bExprsNeedDocids;

	// tricky bit
	// GROUP_CONCAT() adds an implicit TO_STRING() conversion on top of its argument
	// and then the aggregate operation simply concatenates strings as matches arrive
	// ideally, we would instead pass ownership of the expression to G_C() implementation
	// and also the original expression type, and let the string conversion happen in G_C() itself
	// but that ideal route seems somewhat more complicated in the current architecture
	if ( tItem.m_eAggrFunc==SPH_AGGR_CAT )
	{
		CSphString sExpr2;
		sExpr2.SetSprintf ( "TO_STRING(%s)", sExpr.cstr() );
		tExprCol.m_pExpr = sphExprParse ( sExpr2.cstr(), *m_pSorterSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprParseArgs );
	}
	else
		tExprCol.m_pExpr = sphExprParse ( sExpr.cstr(), *m_pSorterSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprParseArgs );

	m_uPackedFactorFlags |= uQueryPackedFactorFlags;
	m_bZonespanlist |= bHasZonespanlist;
	m_bExprsNeedDocids |= bExprsNeedDocids;
	tExprCol.m_eAggrFunc = tItem.m_eAggrFunc;
	tExprCol.m_iIndex = iSorterAttr>= 0 ? m_pSorterSchema->GetAttrIndexOriginal ( tItem.m_sAlias.cstr() ) : -1;
	if ( !tExprCol.m_pExpr )
		return Err ( "parse error: %s", m_sError.cstr() );

	if ( !SetupAggregateExpr ( tExprCol, tItem.m_sExpr, uQueryPackedFactorFlags ) )
		return false;

	ModifyExprForJoin ( tExprCol, tItem.m_sExpr );

	// postpone aggregates, add non-aggregates
	if ( tExprCol.m_eAggrFunc==SPH_AGGR_NONE )
	{
		SelectExprEvalStage(tExprCol);

		// add it!
		// NOTE, "final" stage might need to be fixed up later
		// we'll do that when parsing sorting clause
		m_pSorterSchema->AddAttr ( tExprCol, true );

		// remove original column after new attribute added or shadows this one
		if ( iSorterAttr>=0 )
			m_pSorterSchema->RemoveStaticAttr ( iSorterAttr );
	}
	else // some aggregate
	{
		bool bColumnarAggregate = SetupColumnarAggregates(tExprCol);
		bool bJoinAggregate = ExprHasJoinPrefix ( tExprCol.m_sName, m_tSettings.m_pJoinArgs.get() );

		// columnar aggregates have their own code path; no need to calculate them in presort
		// and aggregates over joined attrs are calculated in the join sorter
		tExprCol.m_eStage = ( bColumnarAggregate || bJoinAggregate ) ? SPH_EVAL_SORTER : SPH_EVAL_PRESORT;

		m_pSorterSchema->AddAttr ( tExprCol, true );
		m_hExtra.Add ( tExprCol.m_sName );

		if ( !bColumnarAggregate )
			UpdateAggregateDependencies ( tExprCol );

		// remove original column after new attribute added or shadows this one
		if ( iSorterAttr>=0 )
			m_pSorterSchema->RemoveStaticAttr ( iSorterAttr );
	}

	m_hQueryDups.Add ( tExprCol.m_sName );
	m_hQueryColumns.Add ( tExprCol.m_sName );

	// need to add all dependent columns for post limit expressions
	if ( tExprCol.m_eStage==SPH_EVAL_POSTLIMIT && tExprCol.m_pExpr )
	{
		StrVec_t dCur;
		tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );

		ARRAY_FOREACH ( j, dCur )
		{
			const CSphColumnInfo * pCol = m_pSorterSchema->GetAttr ( dCur[j].cstr() );
			if ( pCol && pCol->m_pExpr )
				pCol->m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
		}

		dCur.Uniq();

		for ( const auto & sAttr : dCur )
		{
			const CSphColumnInfo * pDep = m_pSorterSchema->GetAttr ( sAttr.cstr() );
			assert(pDep);
			m_hQueryColumns.Add ( pDep->m_sName );
		}
	}

	return true;
}


bool QueueCreator_c::ReplaceWithColumnarItem ( const CSphString & sAttr, ESphEvalStage eStage )
{
	const CSphColumnInfo * pAttr = m_pSorterSchema->GetAttr ( sAttr.cstr() );
	if ( !pAttr->IsColumnar() )
		return true;

	m_hQueryDups.Delete(sAttr);

	CSphQueryItem tItem;
	tItem.m_sExpr = tItem.m_sAlias = sAttr;
	if ( !ParseQueryItem ( tItem ) )
		return false;

	// force stage
	const CSphColumnInfo * pNewAttr = m_pSorterSchema->GetAttr ( sAttr.cstr() );
	const_cast<CSphColumnInfo *>(pNewAttr)->m_eStage = Min ( pNewAttr->m_eStage, eStage );

	return true;
}


// Test for @geodist and setup, if any
bool QueueCreator_c::MaybeAddGeodistColumn ()
{
	if ( !m_tQuery.m_bGeoAnchor || m_pSorterSchema->GetAttrIndex ( "@geodist" )>=0 )
		return true;

	// replace columnar lat/lon with expressions before adding geodist
	if ( !ReplaceWithColumnarItem ( m_tQuery.m_sGeoLatAttr, SPH_EVAL_PREFILTER ) ) return false;
	if ( !ReplaceWithColumnarItem ( m_tQuery.m_sGeoLongAttr, SPH_EVAL_PREFILTER ) ) return false;

	auto pExpr = CreateExprGeodist ( m_tQuery, *m_pSorterSchema, m_sError );
	if ( !pExpr )
		return false;

	CSphColumnInfo tCol ( "@geodist", SPH_ATTR_FLOAT );
	tCol.m_pExpr = pExpr; // takes ownership, no need to for explicit pExpr release
	tCol.m_eStage = SPH_EVAL_PREFILTER; // OPTIMIZE? actual stage depends on usage
	m_pSorterSchema->AddAttr ( tCol, true );
	m_hExtra.Add ( tCol.m_sName );
	m_hQueryAttrs.Add ( tCol.m_sName );

	return true;
}

// Test for @expr and setup, if any
bool QueueCreator_c::MaybeAddExprColumn ()
{
	if ( m_tQuery.m_eSort!=SPH_SORT_EXPR || m_pSorterSchema->GetAttrIndex ( "@expr" )>=0 )
		return true;

	CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT ); // enforce float type for backwards compatibility
	// (i.e. too lazy to fix those tests right now)
	bool bHasZonespanlist;
	ExprParseArgs_t tExprArgs;
	tExprArgs.m_pProfiler = m_tSettings.m_pProfiler;
	tExprArgs.m_eCollation = m_tQuery.m_eCollation;
	tExprArgs.m_pZonespanlist = &bHasZonespanlist;

	tCol.m_pExpr = sphExprParse ( m_tQuery.m_sSortBy.cstr (), *m_pSorterSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprArgs );
	if ( !tCol.m_pExpr )
		return false;

	m_bZonespanlist |= bHasZonespanlist;
	tCol.m_eStage = SPH_EVAL_PRESORT;
	m_pSorterSchema->AddAttr ( tCol, true );
	m_hQueryAttrs.Add ( tCol.m_sName );

	return true;
}


bool QueueCreator_c::AddStoredFieldExpressions()
{
	for ( int i = 0; i<m_tSettings.m_tSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tField = m_tSettings.m_tSchema.GetField(i);
		if ( !(tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED) )
			continue;

		CSphQueryItem tItem;
		tItem.m_sExpr = tField.m_sName;
		tItem.m_sAlias = tField.m_sName;

		if ( !ParseQueryItem ( tItem ) )
			return false;
	}

	return true;
}


bool QueueCreator_c::AddColumnarAttributeExpressions()
{
	for ( int i = 0; i<m_tSettings.m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSettings.m_tSchema.GetAttr(i);
		const CSphColumnInfo * pSorterAttr = m_pSorterSchema->GetAttr ( tAttr.m_sName.cstr() );

		if ( !tAttr.IsColumnar() || ( pSorterAttr && !pSorterAttr->IsColumnar() ) )
			continue;

		m_hQueryDups.Delete ( tAttr.m_sName );

		CSphQueryItem tItem;
		tItem.m_sExpr = tItem.m_sAlias = tAttr.m_sName;
		if ( !ParseQueryItem ( tItem ) )
			return false;
	}

	return true;
}


// Add computed items
bool QueueCreator_c::MaybeAddExpressionsFromSelectList ()
{
	// expressions from select items
	if ( !m_tSettings.m_bComputeItems )
		return true;

	if ( !m_tQuery.m_dItems.all_of ( [&] ( const CSphQueryItem & v ) { return ParseQueryItem ( v ); } ))
		return false;

	if ( m_bHaveStar )
	{
		if ( !AddColumnarAttributeExpressions() )
			return false;

		if ( !AddStoredFieldExpressions() )
			return false;
	}

	return true;
}


bool QueueCreator_c::AddExpressionsForUpdates()
{
	if ( !m_tSettings.m_pCollection )
		return true;

	const CSphColumnInfo * pOldDocId = m_pSorterSchema->GetAttr ( sphGetDocidName() );
	if ( !pOldDocId->IsColumnar() && !pOldDocId->IsColumnarExpr() )
		return true;

	if ( pOldDocId->IsColumnar() )
	{
		// add columnar id expressions to update queue. otherwise we won't be able to fetch docids which are needed to run updates/deletes
		CSphQueryItem tItem;
		tItem.m_sExpr = tItem.m_sAlias = sphGetDocidName();
		if ( !ParseQueryItem ( tItem ) )
			return false;
	}

	auto * pDocId = const_cast<CSphColumnInfo *> ( m_pSorterSchema->GetAttr ( sphGetDocidName() ) );
	assert(pDocId);
	pDocId->m_eStage = SPH_EVAL_PRESORT;	// update/delete queues don't have real Finalize(), so just evaluate it at presort stage

	return true;
}


bool QueueCreator_c::IsJoinAttr ( const CSphString & sAttr ) const
{
	if ( !m_tSettings.m_pJoinArgs )
		return false;

	CSphString sPrefix;
	sPrefix.SetSprintf ( "%s.", m_tSettings.m_pJoinArgs->m_sIndex2.cstr() );

	return sAttr.Begins ( sPrefix.cstr() );
}


void QueueCreator_c::ReplaceJsonGroupbyWithStrings ( CSphString & sJsonGroupBy )
{
	auto AddColumn = [this] ( const CSphColumnInfo & tCol )
		{
			m_pSorterSchema->AddAttr ( tCol, true );
			m_hQueryColumns.Add ( tCol.m_sName );
		};

	if ( m_tGroupSorterSettings.m_bJson )
	{
		bool bJoinAttr = IsJoinAttr ( m_tQuery.m_sGroupBy );

		sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
		if ( !m_pSorterSchema->GetAttr ( sJsonGroupBy.cstr() ) )
		{
			CSphColumnInfo tGroupbyStr ( sJsonGroupBy.cstr() );
			if ( bJoinAttr )
				tGroupbyStr.m_eAttrType = SPH_ATTR_STRINGPTR;
			else
				tGroupbyStr.m_eAttrType = SPH_ATTR_JSON_FIELD;

			tGroupbyStr.m_eStage = SPH_EVAL_SORTER;
			AddColumn ( tGroupbyStr );
		}

		if ( bJoinAttr )
		{
			// we can't do grouping directly on joined JSON fields
			// so we need to change the grouper
			// fixme! this will not work on stuff that generates multiple groupby keys (like JSON arrays)
			const CSphColumnInfo * pRemapped = m_pSorterSchema->GetAttr ( sJsonGroupBy.cstr() );
			assert(pRemapped);

			m_tGroupSorterSettings.m_pGrouper = CreateGrouperString ( pRemapped->m_tLocator, m_tQuery.m_eCollation );
			m_tGroupSorterSettings.m_bJson = false;
		}
	}
	else if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_MULTIPLE && m_bJoinedGroupSort )
	{
		bool bGrouperChanged = false;
		ARRAY_FOREACH ( i, m_dGroupColumns )
		{
			const CSphColumnInfo & tAttr = m_pSorterSchema->GetAttr ( m_dGroupColumns[i].first );
			bool bJoinAttr = IsJoinAttr ( tAttr.m_sName );
			bool bJson = tAttr.m_eAttrType==SPH_ATTR_JSON_PTR || tAttr.m_eAttrType==SPH_ATTR_JSON_FIELD_PTR;

			if ( bJoinAttr && bJson )
			{
				sJsonGroupBy = SortJsonInternalSet ( m_dGroupJsonAttrs[i] );
				if ( !m_pSorterSchema->GetAttr ( sJsonGroupBy.cstr() ) )
				{
					CSphColumnInfo tGroupbyStr ( sJsonGroupBy.cstr() );
					tGroupbyStr.m_eAttrType = SPH_ATTR_STRINGPTR;
					tGroupbyStr.m_eStage = SPH_EVAL_SORTER;
					AddColumn ( tGroupbyStr );
				}

				m_dGroupColumns[i].first = m_pSorterSchema->GetAttrIndex ( sJsonGroupBy.cstr() );
				m_dGroupJsonAttrs[i] = "";
				bGrouperChanged = true;
			}
		}

		if ( bGrouperChanged )
		{
			CSphVector<CSphColumnInfo> dAttrs;
			VecRefPtrs_t<ISphExpr *> dJsonKeys;
			ARRAY_FOREACH ( i, m_dGroupColumns )
			{
				dAttrs.Add ( m_pSorterSchema->GetAttr ( m_dGroupColumns[i].first ) );

				const CSphString & sJsonExpr = m_dGroupJsonAttrs[i];
				if ( !sJsonExpr.IsEmpty() )
				{
					ExprParseArgs_t tExprArgs;
					dJsonKeys.Add ( sphExprParse ( sJsonExpr.cstr(), *m_pSorterSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprArgs ) );
				}
				else
					dJsonKeys.Add(nullptr);
			}

			m_tGroupSorterSettings.m_pGrouper = CreateGrouperMulti ( dAttrs, std::move(dJsonKeys), m_tQuery.m_eCollation );
		}
	}
}


bool QueueCreator_c::MaybeAddGroupbyMagic ( bool bGotDistinct )
{
	CSphString sJsonGroupBy;
	// now let's add @groupby etc. if needed
	if ( m_bGotGroupby && m_pSorterSchema->GetAttrIndex ( "@groupby" )<0 )
	{
		ESphAttr eGroupByResult = ( !m_tGroupSorterSettings.m_bImplicit )
				? m_tGroupSorterSettings.m_pGrouper->GetResultType ()
				: SPH_ATTR_INTEGER; // implicit do not have grouper

		// all FACET group by should be the widest possible type
		if ( m_tQuery.m_bFacet || m_tQuery.m_bFacetHead	|| m_bMulti )
			eGroupByResult = SPH_ATTR_BIGINT;

		CSphColumnInfo tGroupby ( "@groupby", eGroupByResult );
		CSphColumnInfo tCount ( "@count", SPH_ATTR_BIGINT );

		tGroupby.m_eStage = SPH_EVAL_SORTER;
		tCount.m_eStage = SPH_EVAL_SORTER;

		auto AddColumn = [this] ( const CSphColumnInfo & tCol )
		{
			m_pSorterSchema->AddAttr ( tCol, true );
			m_hQueryColumns.Add ( tCol.m_sName );
		};

		AddColumn ( tGroupby );
		AddColumn ( tCount );

		if ( bGotDistinct )
		{
			CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );
			tDistinct.m_eStage = SPH_EVAL_SORTER;
			AddColumn ( tDistinct );
		}

		// add @groupbystr last in case we need to skip it on sending (like @int_attr_*)
		ReplaceJsonGroupbyWithStrings ( sJsonGroupBy );
	}

	#define LOC_CHECK( _cond, _msg ) if (!(_cond)) { m_sError = "invalid schema: " _msg; return false; }

	int iGroupby = m_pSorterSchema->GetAttrIndex ( "@groupby" );
	if ( iGroupby>=0 )
	{
		m_tGroupSorterSettings.m_bDistinct = bGotDistinct;
		m_tGroupSorterSettings.m_tLocGroupby = m_pSorterSchema->GetAttr ( iGroupby ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocGroupby.m_bDynamic, "@groupby must be dynamic" );

		int iCount = m_pSorterSchema->GetAttrIndex ( "@count" );
		LOC_CHECK ( iCount>=0, "missing @count" );

		m_tGroupSorterSettings.m_tLocCount = m_pSorterSchema->GetAttr ( iCount ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocCount.m_bDynamic, "@count must be dynamic" );

		int iDistinct = m_pSorterSchema->GetAttrIndex ( "@distinct" );
		if ( bGotDistinct )
		{
			LOC_CHECK ( iDistinct>=0, "missing @distinct" );
			m_tGroupSorterSettings.m_tLocDistinct = m_pSorterSchema->GetAttr ( iDistinct ).m_tLocator;
			LOC_CHECK ( m_tGroupSorterSettings.m_tLocDistinct.m_bDynamic, "@distinct must be dynamic" );
		}
		else
			LOC_CHECK ( iDistinct<=0, "unexpected @distinct" );

		int iGroupbyStr = m_pSorterSchema->GetAttrIndex ( sJsonGroupBy.cstr() );
		if ( iGroupbyStr>=0 )
			m_tGroupSorterSettings.m_tLocGroupbyStr = m_pSorterSchema->GetAttr ( iGroupbyStr ).m_tLocator;
	}

	if ( m_bHasCount )
		LOC_CHECK ( m_pSorterSchema->GetAttrIndex ( "@count" )>=0, "Count(*) or @count is queried, but not available in the schema" );

	#undef LOC_CHECK
	return true;
}


bool QueueCreator_c::AddKNNDistColumn()
{
	if ( m_tQuery.m_sKNNAttr.IsEmpty() || m_pSorterSchema->GetAttrIndex ( GetKnnDistAttrName() )>=0 )
		return true;

	auto pAttr = m_pSorterSchema->GetAttr ( m_tQuery.m_sKNNAttr.cstr() );
	if ( !pAttr )
	{
		m_sError.SetSprintf ( "requested KNN search attribute '%s' not found", m_tQuery.m_sKNNAttr.cstr() );
		return false;
	}

	if ( !pAttr->IsIndexedKNN() )
	{
		m_sError.SetSprintf ( "KNN index not enabled for attribute '%s'", m_tQuery.m_sKNNAttr.cstr() );
		return false;
	}

	if ( pAttr->m_tKNN.m_iDims!=m_tQuery.m_dKNNVec.GetLength() )
	{
		m_sError.SetSprintf ( "KNN index '%s' requires a vector of %d entries; %d entries specified", m_tQuery.m_sKNNAttr.cstr(), pAttr->m_tKNN.m_iDims, m_tQuery.m_dKNNVec.GetLength() );
		return false;
	}

	CSphColumnInfo tKNNDist ( GetKnnDistAttrName(), SPH_ATTR_FLOAT );
	tKNNDist.m_eStage = SPH_EVAL_PRESORT;
	tKNNDist.m_pExpr = CreateExpr_KNNDist ( m_tQuery.m_dKNNVec, *pAttr );

	m_pSorterSchema->AddAttr ( tKNNDist, true );
	m_hQueryColumns.Add ( tKNNDist.m_sName );

	return true;
}


bool QueueCreator_c::ParseJoinExpr ( CSphColumnInfo & tExprCol, const CSphString & sAttr, const CSphString & sExpr ) const
{
	tExprCol = CSphColumnInfo ( sAttr.cstr() );

	ExprParseArgs_t tExprParseArgs;
	tExprParseArgs.m_pAttrType = &tExprCol.m_eAttrType;
	tExprParseArgs.m_pProfiler = m_tSettings.m_pProfiler;
	tExprParseArgs.m_eCollation = m_tQuery.m_eCollation;
	tExprCol.m_eStage = SPH_EVAL_PRESORT;
	tExprCol.m_pExpr = sphExprParse ( sExpr.cstr(), *m_pSorterSchema, m_tSettings.m_pJoinArgs ? &(m_tSettings.m_pJoinArgs->m_sIndex2) : nullptr, m_sError, tExprParseArgs );
	tExprCol.m_uAttrFlags |= CSphColumnInfo::ATTR_JOINED;
	return !!tExprCol.m_pExpr;
}


bool QueueCreator_c::AddJsonJoinOnFilter ( const CSphString & sAttr1, const CSphString & sAttr2, ESphAttr eTypeCast )
{
	const CSphColumnInfo * pAttr = m_pSorterSchema->GetAttr ( sAttr1.cstr() );
	if ( pAttr )
	{
		if ( pAttr->m_pExpr && pAttr->m_pExpr->UsesDocstore() )
			return true;

		const_cast<CSphColumnInfo *>(pAttr)->m_eStage = Min ( pAttr->m_eStage, SPH_EVAL_PRESORT );
		return true;
	}

	if ( !sphJsonNameSplit ( sAttr1.cstr(), nullptr ) )
	{
		const CSphColumnInfo * pField = m_pSorterSchema->GetField ( sAttr1.cstr() );
		if ( pField && ( pField->m_uFieldFlags & CSphColumnInfo::FIELD_STORED ) )
			m_sError.SetSprintf ( "Unable to perform join on a stored field '%s.%s'", m_tSettings.m_pJoinArgs->m_sIndex1.cstr(), sAttr1.cstr() );
		else
			m_sError.SetSprintf ( "Unable to perform join on '%s'", sAttr1.cstr() );

		return false;
	}

	CSphColumnInfo tExprCol;
	if ( !ParseJoinExpr ( tExprCol, sAttr1, sAttr1 ) )
		return false;

	const auto & tSchema = m_tSettings.m_pJoinArgs->m_tJoinedSchema;

	// convert JSON fields to join attr type
	if ( tExprCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
	{
		// try to determine type if it was not explicitly specified
		if ( eTypeCast==SPH_ATTR_NONE )
		{
			auto * pJoinAttr = tSchema.GetAttr ( sAttr2.cstr() );
			if ( !pJoinAttr )
			{
				if ( sphJsonNameSplit ( sAttr2.cstr() ) )
					m_sError.SetSprintf ( "use implicit type conversion on join-on attribute '%s'", sAttr2.cstr() );
				else
					m_sError.SetSprintf ( "join-on attribute '%s' not found", sAttr2.cstr() );

				return false;
			}

			eTypeCast = pJoinAttr->m_eAttrType;
		}

		CSphString sConverted;
		switch ( eTypeCast )
		{
		case SPH_ATTR_STRING:
			sConverted.SetSprintf ( "TO_STRING(%s)", sAttr1.cstr() );
			break;

		case SPH_ATTR_FLOAT:
			sConverted.SetSprintf ( "DOUBLE(%s)", sAttr1.cstr() );
			break;

		default:
			sConverted.SetSprintf ( "BIGINT(%s)", sAttr1.cstr() );
			break;
		}

		if ( !ParseJoinExpr ( tExprCol, sAttr1, sConverted ) )
			return false;
	}

	m_pSorterSchema->AddAttr ( tExprCol, true );
	return true;
}


bool QueueCreator_c::AddColumnarJoinOnFilter ( const CSphString & sAttr )
{
	const CSphColumnInfo * pAttr = m_pSorterSchema->GetAttr ( sAttr.cstr() );
	if ( pAttr && pAttr->m_pExpr && !pAttr->m_pExpr->UsesDocstore() )
	{
		const_cast<CSphColumnInfo *>(pAttr)->m_eStage = Min ( pAttr->m_eStage, SPH_EVAL_PRESORT );
		return true;
	}

	return ReplaceWithColumnarItem ( sAttr, SPH_EVAL_PRESORT );
}


bool QueueCreator_c::AddJoinAttrs()
{
	if ( !m_tSettings.m_pJoinArgs )
		return true;

	const auto & tSchema = m_tSettings.m_pJoinArgs->m_tJoinedSchema;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
		if ( !sphIsInternalAttr ( tSchema.GetAttr(i).m_sName ) )
		{
			CSphColumnInfo tAttr = tSchema.GetAttr(i);
			tAttr.m_sName.SetSprintf ( "%s.%s", m_tSettings.m_pJoinArgs->m_sIndex2.cstr(), tAttr.m_sName.cstr() );
			tAttr.m_eAttrType = sphPlainAttrToPtrAttr ( tAttr.m_eAttrType );
			tAttr.m_tLocator.Reset();
			tAttr.m_eStage = SPH_EVAL_SORTER;
			tAttr.m_uAttrFlags &= ~( CSphColumnInfo::ATTR_COLUMNAR | CSphColumnInfo::ATTR_COLUMNAR_HASHES );
			tAttr.m_uAttrFlags |= CSphColumnInfo::ATTR_JOINED;
			m_pSorterSchema->AddAttr ( tAttr, true );

			m_hQueryDups.Add ( tAttr.m_sName );
			m_hQueryColumns.Add ( tAttr.m_sName );
		}

	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tField = tSchema.GetField(i);
		if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
		{
			CSphColumnInfo tAttr;
			tAttr.m_sName.SetSprintf ( "%s.%s", m_tSettings.m_pJoinArgs->m_sIndex2.cstr(), tField.m_sName.cstr() );
			tAttr.m_eAttrType = SPH_ATTR_STRINGPTR;
			tAttr.m_tLocator.Reset();
			tAttr.m_eStage = SPH_EVAL_SORTER;
			tAttr.m_uAttrFlags = CSphColumnInfo::ATTR_JOINED;
			m_pSorterSchema->AddAttr ( tAttr, true );

			m_hQueryDups.Add ( tAttr.m_sName );
			m_hQueryColumns.Add ( tAttr.m_sName );
		}
	}

	CSphColumnInfo tAttr;
	tAttr.m_sName.SetSprintf ( "%s.weight()", m_tSettings.m_pJoinArgs->m_sIndex2.cstr() );
	tAttr.m_eAttrType = SPH_ATTR_INTEGER;
	tAttr.m_tLocator.Reset();
	tAttr.m_eStage = SPH_EVAL_SORTER;
	tAttr.m_uAttrFlags = CSphColumnInfo::ATTR_JOINED;
	m_pSorterSchema->AddAttr ( tAttr, true );

	m_hQueryDups.Add ( tAttr.m_sName );
	m_hQueryColumns.Add ( tAttr.m_sName );

	return true;
}


static ESphAttr FilterType2AttrType ( ESphFilter eFilter )
{
	switch ( eFilter )
	{
	case SPH_FILTER_FLOATRANGE:
		return SPH_ATTR_FLOAT;

	case SPH_FILTER_STRING:
	case SPH_FILTER_STRING_LIST:
		return SPH_ATTR_STRINGPTR;

	default:
		return SPH_ATTR_BIGINT;
	}
}


bool QueueCreator_c::CheckJoinOnTypeCast ( const CSphString & sIdx, const CSphString & sAttr, ESphAttr eTypeCast )
{
	if ( eTypeCast==SPH_ATTR_NONE )
		return true;

	if ( !sphJsonNameSplit ( sAttr.cstr() ) )
	{
		m_sError.SetSprintf ( "Explicit type conversion used on non-json attribute '%s.%s'", sIdx.cstr(), sAttr.cstr() );
		return false;
	}

	return true;
}


bool QueueCreator_c::AddJoinFilterAttrs()
{
	if ( !m_tSettings.m_pJoinArgs )
		return true;

	const CSphString & sLeftIndex = m_tSettings.m_pJoinArgs->m_sIndex1;
	const CSphString & sRightIndex = m_tSettings.m_pJoinArgs->m_sIndex2;
	for ( const auto & i : m_tQuery.m_dOnFilters )
	{
		if ( !CheckJoinOnTypeCast ( i.m_sIdx1, i.m_sAttr1, i.m_eTypeCast1 ) ) return false;
		if ( !CheckJoinOnTypeCast ( i.m_sIdx2, i.m_sAttr2, i.m_eTypeCast2 ) ) return false;

		ESphAttr eTypeCast = i.m_eTypeCast1!=SPH_ATTR_NONE ? i.m_eTypeCast1 : i.m_eTypeCast2;

		if ( i.m_sIdx1==sLeftIndex )
		{
			if ( !AddJsonJoinOnFilter ( i.m_sAttr1, i.m_sAttr2, eTypeCast ) )	return false;
			if ( !AddColumnarJoinOnFilter ( i.m_sAttr1 ) )						return false;
		}

		if ( i.m_sIdx2==sLeftIndex )
		{
			if ( !AddJsonJoinOnFilter ( i.m_sAttr2, i.m_sAttr1, eTypeCast ) )	return false;
			if ( !AddColumnarJoinOnFilter ( i.m_sAttr2 ) )						return false;
		}
	}

	if ( NeedToMoveMixedJoinFilters ( m_tQuery, *m_pSorterSchema ) )
		for ( const auto & i : m_tQuery.m_dFilters )
		{
			const CSphString & sAttr = i.m_sAttrName;
			const CSphColumnInfo * pAttr = m_pSorterSchema->GetAttr ( sAttr.cstr() );
			if ( pAttr || !sphJsonNameSplit ( sAttr.cstr(), sRightIndex.cstr() ) )
				continue;

			CSphColumnInfo tExprCol ( sAttr.cstr(), FilterType2AttrType ( i.m_eType ) );
			tExprCol.m_eStage = SPH_EVAL_SORTER;
			tExprCol.m_uAttrFlags |= CSphColumnInfo::ATTR_JOINED;
			m_pSorterSchema->AddAttr ( tExprCol, true );

			m_hQueryDups.Add(sAttr);
			m_hQueryColumns.Add(sAttr);
		}

	return true;
}


bool QueueCreator_c::AddNullBitmask()
{
	if ( !m_tSettings.m_pJoinArgs || m_tQuery.m_eJoinType!=JoinType_e::LEFT )
		return true;

	int iNumJoinAttrs = 0;
	int iDynamic = 0;
	for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
	{
		const auto & tAttr = m_pSorterSchema->GetAttr(i);
		if ( !tAttr.m_tLocator.m_bDynamic )
			continue;

		iDynamic++;
		if ( tAttr.IsJoined() )
			iNumJoinAttrs = Max ( iNumJoinAttrs, iDynamic );
	}

	CSphColumnInfo tAttr ( GetNullMaskAttrName(), DetermineNullMaskType(iNumJoinAttrs) );
	tAttr.m_eStage = SPH_EVAL_SORTER;
	m_pSorterSchema->AddAttr ( tAttr, true );

	m_hQueryDups.Add ( tAttr.m_sName );
	m_hQueryColumns.Add ( tAttr.m_sName );

	return true;
}


bool QueueCreator_c::CheckHavingConstraints () const
{
	if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty () )
	{
		if ( !m_bGotGroupby )
			return Err ( "can not use HAVING without GROUP BY" );

		// should be column named at group by, or it's alias or aggregate
		const CSphString & sHaving = m_tSettings.m_pAggrFilter->m_sAttrName;
		if ( !IsGroupbyMagic ( sHaving ) )
		{
			bool bValidHaving = false;
			for ( const CSphQueryItem & tItem : m_tQuery.m_dItems )
			{
				if ( tItem.m_sAlias!=sHaving )
					continue;

				bValidHaving = ( IsGroupbyMagic ( tItem.m_sExpr ) || tItem.m_eAggrFunc!=SPH_AGGR_NONE );
				break;
			}

			if ( !bValidHaving )
				return Err ( "can not use HAVING with attribute not related to GROUP BY" );
		}
	}
	return true;
}


void QueueCreator_c::SetupRemapColJson ( CSphColumnInfo & tRemapCol, CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs, int iStateAttr )
{
	bool bFunc = dExtraExprs[iStateAttr].m_tKey.m_uMask==0;

	tRemapCol.m_eStage = SPH_EVAL_PRESORT;
	if ( bFunc )
	{
		tRemapCol.m_pExpr = dExtraExprs[iStateAttr].m_pExpr;
		tRemapCol.m_eAttrType = dExtraExprs[iStateAttr].m_eType;
		tState.m_eKeypart[iStateAttr] = Attr2Keypart ( tRemapCol.m_eAttrType );
	}
	else
		tRemapCol.m_pExpr = CreateExprSortJson2String ( tState.m_tLocator[iStateAttr], dExtraExprs[iStateAttr].m_pExpr );
}


const CSphColumnInfo * QueueCreator_c::GetGroupbyStr ( int iAttr, int iNumOldAttrs ) const
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	if ( m_tSettings.m_bComputeItems && iAttr>=0 && iAttr<iNumOldAttrs && tSorterSchema.GetAttr(iAttr).m_sName=="@groupby" && m_dGroupColumns.GetLength() )
	{
		// FIXME!!! add support of multi group by
		const CSphColumnInfo & tGroupCol = tSorterSchema.GetAttr ( m_dGroupColumns[0].first );
		if ( tGroupCol.m_eAttrType==SPH_ATTR_STRING || tGroupCol.m_eAttrType==SPH_ATTR_STRINGPTR )
			return &tGroupCol;
	}

	return nullptr;
}


void QueueCreator_c::ReplaceGroupbyStrWithExprs ( CSphMatchComparatorState & tState, int iNumOldAttrs )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		const CSphColumnInfo * pGroupStrBase = GetGroupbyStr ( tState.m_dAttrs[i], iNumOldAttrs );
		if ( !pGroupStrBase )
			continue;

		assert ( tState.m_dAttrs[i]>=0 && tState.m_dAttrs[i]<iNumOldAttrs );

		int iRemap = -1;
		if ( pGroupStrBase->m_eAttrType==SPH_ATTR_STRINGPTR )
		{
			// grouping by (columnar) string; and the same string is used in sorting
			// correct the locator and change the evaluation stage to PRESORT
			iRemap = tSorterSchema.GetAttrIndex ( pGroupStrBase->m_sName.cstr() );
			assert ( iRemap>=0 );
			const CSphColumnInfo & tAttr = tSorterSchema.GetAttr(iRemap);
			const_cast<CSphColumnInfo &>(tAttr).m_eStage = SPH_EVAL_PRESORT;
		}
		else if ( !pGroupStrBase->IsColumnar() )
		{
			CSphString sRemapCol;
			sRemapCol.SetSprintf ( "%s%s", GetInternalAttrPrefix(), pGroupStrBase->m_sName.cstr() );
			iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );

			if ( iRemap==-1 )
			{
				CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
				tRemapCol.m_pExpr = CreateExprSortStringFixup ( pGroupStrBase->m_tLocator );
				tRemapCol.m_eStage = SPH_EVAL_PRESORT;

				iRemap = tSorterSchema.GetAttrsCount();
				tSorterSchema.AddAttr ( tRemapCol, true );
			}
		}

		if ( iRemap!=-1 )
		{
			tState.m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
			tState.m_tLocator[i] = tSorterSchema.GetAttr(iRemap).m_tLocator;
			tState.m_dAttrs[i] = iRemap;
			tState.m_dRemapped.BitSet ( i );
		}
	}
}


void QueueCreator_c::ReplaceStaticStringsWithExprs ( CSphMatchComparatorState & tState )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) )
			continue;

		if ( tState.m_eKeypart[i]!=SPH_KEYPART_STRING )
			continue;

		int iRemap = -1;
		int iAttrId = tState.m_dAttrs[i];
		const CSphColumnInfo & tAttr = tSorterSchema.GetAttr(iAttrId);
		if ( tAttr.IsColumnar() )
		{
			CSphString sAttrName = tAttr.m_sName;
			tSorterSchema.RemoveStaticAttr(iAttrId);
			
			CSphColumnInfo tRemapCol ( sAttrName.cstr(), SPH_ATTR_STRINGPTR );
			tRemapCol.m_eStage = SPH_EVAL_PRESORT;
			tRemapCol.m_pExpr = CreateExpr_GetColumnarString ( sAttrName, tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED );
			tSorterSchema.AddAttr ( tRemapCol, true );

			iRemap = tSorterSchema.GetAttrIndex ( sAttrName.cstr() );
		}
		else
		{
			CSphString sRemapCol;
			sRemapCol.SetSprintf ( "%s%s", GetInternalAttrPrefix(), tSorterSchema.GetAttr(iAttrId).m_sName.cstr() );
			iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );
			if ( iRemap==-1 )
			{
				CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
				tRemapCol.m_eStage = SPH_EVAL_PRESORT;
				tRemapCol.m_pExpr = CreateExprSortStringFixup ( tState.m_tLocator[i] );

				iRemap = tSorterSchema.GetAttrsCount();
				tSorterSchema.AddAttr ( tRemapCol, true );
			}
		}

		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
		tState.m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
		tState.m_dRemapped.BitSet ( i );
	}
}


void QueueCreator_c::ReplaceJsonWithExprs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) )
			continue;

		const CSphString & sKey = dExtraExprs[i].m_tKey.m_sKey;
		if ( sKey.IsEmpty() )
			continue;

		CSphString sRemapCol;
		sRemapCol.SetSprintf ( "%s%s", GetInternalAttrPrefix(), sKey.cstr() );

		int iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );
		if ( iRemap==-1 )
		{
			CSphString sRemapLowercase = sRemapCol;
			sRemapLowercase.ToLower();
			iRemap = tSorterSchema.GetAttrIndex ( sRemapLowercase.cstr() );
		}

		if ( iRemap==-1 )
		{
			CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
			SetupRemapColJson ( tRemapCol, tState, dExtraExprs, i );
			iRemap = tSorterSchema.GetAttrsCount();
			ModifyExprForJoin ( tRemapCol, sKey );
			tSorterSchema.AddAttr ( tRemapCol, true );
		}

		tState.m_tLocator[i] = tSorterSchema.GetAttr(iRemap).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
		tState.m_dRemapped.BitSet ( i );
	}
}


void QueueCreator_c::AddColumnarExprsAsAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;

	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) )
			continue;

		ISphExpr * pExpr = dExtraExprs[i].m_pExpr;
		if ( !pExpr || !pExpr->IsColumnar() )
			continue;

		const CSphString & sAttrName = tSorterSchema.GetAttr ( tState.m_dAttrs[i] ).m_sName;
		CSphColumnInfo tRemapCol ( sAttrName.cstr(), dExtraExprs[i].m_eType );
		tRemapCol.m_eStage = SPH_EVAL_PRESORT;
		tRemapCol.m_pExpr = pExpr;
		tRemapCol.m_pExpr->AddRef();

		int iRemap = tSorterSchema.GetAttrsCount();
		tSorterSchema.AddAttr ( tRemapCol, true );

		// remove initial attribute from m_hExtra
		// that way it won't be evaluated twice when it is not in select list
		m_hExtra.Delete(sAttrName);

		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
		tState.m_eKeypart[i] = Attr2Keypart ( dExtraExprs[i].m_eType );
		tState.m_dRemapped.BitSet ( i );
	}
}


void QueueCreator_c::RemapAttrs ( CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs )
{
	// we have extra attrs (expressions) that we created while parsing the sort clause
	// we couldn't add them to the schema at that stage,
	// but now we can. we create attributes, assign internal names and set their expressions

	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema;
	int iNumOldAttrs = tSorterSchema.GetAttrsCount();

	ReplaceGroupbyStrWithExprs ( tState, iNumOldAttrs );
	ReplaceStaticStringsWithExprs ( tState );
	ReplaceJsonWithExprs ( tState, dExtraExprs );
	AddColumnarExprsAsAttrs ( tState, dExtraExprs );

	// need another sort keys add after setup remap
	if ( iNumOldAttrs!=tSorterSchema.GetAttrsCount() )
		ExtraAddSortkeys ( tState.m_dAttrs );
}


void QueueCreator_c::AddKnnDistSort ( CSphString & sSortBy )
{
	if ( m_pSorterSchema->GetAttr ( GetKnnDistAttrName() ) && !strstr ( sSortBy.cstr(), "knn_dist" ) )
		sSortBy.SetSprintf ( "knn_dist() asc, %s", sSortBy.cstr() );
}

// matches sorting function
bool QueueCreator_c::SetupMatchesSortingFunc()
{
	m_bRandomize = false;
	if ( m_tQuery.m_eSort==SPH_SORT_EXTENDED )
	{
		CSphString sSortBy = m_tQuery.m_sSortBy;
		AddKnnDistSort ( sSortBy );

		ESortClauseParseResult eRes = sphParseSortClause ( m_tQuery, sSortBy.cstr(), *m_pSorterSchema, m_eMatchFunc, m_tStateMatch, m_dMatchJsonExprs, m_tSettings.m_pJoinArgs.get(), m_sError );
		if ( eRes==SORT_CLAUSE_ERROR )
			return false;

		if ( eRes==SORT_CLAUSE_RANDOM )
			m_bRandomize = true;

		ExtraAddSortkeys ( m_tStateMatch.m_dAttrs );

		AssignOrderByToPresortStage ( m_tStateMatch.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );
		RemapAttrs ( m_tStateMatch, m_dMatchJsonExprs );
		return true;
	}

	if ( m_tQuery.m_eSort==SPH_SORT_EXPR )
	{
		m_tStateMatch.m_eKeypart[0] = SPH_KEYPART_INT;
		m_tStateMatch.m_tLocator[0] = m_pSorterSchema->GetAttr ( m_pSorterSchema->GetAttrIndex ( "@expr" ) ).m_tLocator;
		m_tStateMatch.m_eKeypart[1] = SPH_KEYPART_ROWID;
		m_tStateMatch.m_uAttrDesc = 1;
		m_eMatchFunc = FUNC_EXPR;
		return true;
	}

	// check sort-by attribute
	if ( m_tQuery.m_eSort!=SPH_SORT_RELEVANCE )
	{
		int iSortAttr = m_pSorterSchema->GetAttrIndex ( m_tQuery.m_sSortBy.cstr() );
		if ( iSortAttr<0 )
		{
			Err ( "sort-by attribute '%s' not found", m_tQuery.m_sSortBy.cstr() );
			return false;
		}

		const CSphColumnInfo & tAttr = m_pSorterSchema->GetAttr ( iSortAttr );
		m_tStateMatch.m_eKeypart[0] = Attr2Keypart ( tAttr.m_eAttrType );
		m_tStateMatch.m_tLocator[0] = tAttr.m_tLocator;
		m_tStateMatch.m_dAttrs[0] = iSortAttr;
		RemapAttrs ( m_tStateMatch, m_dMatchJsonExprs );
	}

	ExtraAddSortkeys ( m_tStateMatch.m_dAttrs );

	// find out what function to use and whether it needs attributes
	switch (m_tQuery.m_eSort )
	{
		case SPH_SORT_TIME_SEGMENTS:	m_eMatchFunc = FUNC_TIMESEGS; break;
		case SPH_SORT_RELEVANCE:	m_eMatchFunc = FUNC_REL_DESC; break;
		default:
			Err ( "unknown sorting mode %d", m_tQuery.m_eSort );
			return false;
	}
	return true;
}

bool QueueCreator_c::SetupGroupSortingFunc ( bool bGotDistinct )
{
	assert ( m_bGotGroupby );
	CSphString sGroupOrderBy = m_tQuery.m_sGroupSortBy;
	if ( sGroupOrderBy=="@weight desc" )
		AddKnnDistSort ( sGroupOrderBy );

	ESortClauseParseResult eRes = sphParseSortClause ( m_tQuery, sGroupOrderBy.cstr(), *m_pSorterSchema, m_eGroupFunc, m_tStateGroup, m_dGroupJsonExprs, m_tSettings.m_pJoinArgs.get(), m_sError );

	if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
	{
		if ( eRes==SORT_CLAUSE_RANDOM )
			m_sError = "groups can not be sorted by @random";

		return false;
	}

	ExtraAddSortkeys ( m_tStateGroup.m_dAttrs );

	if ( !m_tGroupSorterSettings.m_bImplicit )
	{
		for ( const auto & tGroupColumn : m_dGroupColumns )
			m_hExtra.Add ( m_pSorterSchema->GetAttr ( tGroupColumn.first ).m_sName );
	}

	if ( bGotDistinct )
	{
		m_dGroupColumns.Add ( { m_pSorterSchema->GetAttrIndex ( m_tQuery.m_sGroupDistinct.cstr() ), true } );
		assert ( m_dGroupColumns.Last().first>=0 );
		m_hExtra.Add ( m_pSorterSchema->GetAttr ( m_dGroupColumns.Last().first ).m_sName );
	}

	// implicit case
	CSphVector<int> dGroupByCols;
	for ( const auto & i : m_dGroupColumns )
		if ( i.second )
			dGroupByCols.Add ( i.first );

	AssignOrderByToPresortStage ( dGroupByCols.Begin(), dGroupByCols.GetLength() );
	AssignOrderByToPresortStage ( m_tStateGroup.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );

	// GroupSortBy str attributes setup
	RemapAttrs ( m_tStateGroup, m_dGroupJsonExprs );
	return true;
}

// set up aggregate filter for grouper
std::unique_ptr<ISphFilter> QueueCreator_c::CreateAggrFilter () const
{
	assert ( m_bGotGroupby );
	if ( m_pSorterSchema->GetAttr ( m_tSettings.m_pAggrFilter->m_sAttrName.cstr() ) )
		return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, m_tSettings.m_pAggrFilter->m_sAttrName, *m_pSorterSchema, m_sError );

	// having might reference aliased attributes but @* attributes got stored without alias in sorter schema
	CSphString sHaving;
	for ( const auto & tItem : m_tQuery.m_dItems )
		if ( tItem.m_sAlias==m_tSettings.m_pAggrFilter->m_sAttrName )
		{
			sHaving = tItem.m_sExpr;
			break;
		}

	if ( sHaving=="groupby()" )
		sHaving = "@groupby";
	else if ( sHaving=="count(*)" )
		sHaving = "@count";

	return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, sHaving, *m_pSorterSchema, m_sError );
}

void QueueCreator_c::SetupCollation()
{
	SphStringCmp_fn fnCmp = GetStringCmpFunc ( m_tQuery.m_eCollation );
	m_tStateMatch.m_fnStrCmp = fnCmp;
	m_tStateGroup.m_fnStrCmp = fnCmp;
}

bool QueueCreator_c::AddGroupbyStuff ()
{
	// need schema with group related columns however not need grouper
	m_bHeadWOGroup = ( m_tQuery.m_sGroupBy.IsEmpty () && m_tQuery.m_bFacetHead );
	auto fnIsImplicit = [] ( const CSphQueryItem & t )
	{
		return ( t.m_eAggrFunc!=SPH_AGGR_NONE ) || t.m_sExpr=="count(*)" || t.m_sExpr=="@distinct";
	};

	bool bHasImplicitGrouping = HasImplicitGrouping(m_tQuery);

	// count(*) and distinct wo group by at main query should keep implicit flag
	if ( bHasImplicitGrouping && m_bHeadWOGroup )
		m_bHeadWOGroup = !m_tQuery.m_dRefItems.any_of ( fnIsImplicit );

	if ( !SetupGroupbySettings(bHasImplicitGrouping) )
		return false;

	// or else, check in SetupGroupbySettings() would already fail
	m_bGotGroupby = !m_tQuery.m_sGroupBy.IsEmpty () || m_tGroupSorterSettings.m_bImplicit;
	m_bGotDistinct = !!m_tGroupSorterSettings.m_pDistinctFetcher;

	if ( m_bHasGroupByExpr && !m_bGotGroupby )
		return Err ( "GROUPBY() is allowed only in GROUP BY queries" );

	// check for HAVING constrains
	if ( !CheckHavingConstraints() )
		return false;

	// now let's add @groupby stuff, if necessary
	return MaybeAddGroupbyMagic(m_bGotDistinct);
}

bool QueueCreator_c::SetGroupSorting()
{
	if ( m_bGotGroupby )
	{
		if ( !SetupGroupSortingFunc ( m_bGotDistinct ) )
			return false;

		if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty() )
		{
			auto pFilter = CreateAggrFilter ();
			if ( !pFilter )
				return false;

			m_tGroupSorterSettings.m_pAggrFilterTrait = pFilter.release();
		}

		int iDistinctAccuracyThresh = m_tQuery.m_bExplicitDistinctThresh ? m_tQuery.m_iDistinctThresh : GetDistinctThreshDefault();
		m_tGroupSorterSettings.SetupDistinctAccuracy ( iDistinctAccuracyThresh );
	}

	for ( auto & tIdx: m_hExtra )
	{
		m_hQueryColumns.Add ( tIdx.first );
		if ( m_pExtra )
			m_pExtra->Add ( tIdx.first );
	}

	return true;
}


bool QueueCreator_c::PredictAggregates() const
{
	for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_pSorterSchema->GetAttr(i);
		if ( !(tAttr.m_eAggrFunc==SPH_AGGR_NONE || IsGroupbyMagic ( tAttr.m_sName ) || IsSortStringInternal ( tAttr.m_sName.cstr () )) )
			return true;
	}

	return false;
}


int QueueCreator_c::ReduceMaxMatches() const
{
	assert ( !m_bGotGroupby );
	if ( m_tQuery.m_bExplicitMaxMatches || m_tQuery.m_bHasOuter || !m_tSettings.m_bComputeItems )
		return Max ( m_tSettings.m_iMaxMatches, 1 );

	return Max ( Min ( m_tSettings.m_iMaxMatches, m_tQuery.m_iLimit+m_tQuery.m_iOffset ), 1 );
}


int QueueCreator_c::AdjustMaxMatches ( int iMaxMatches ) const
{
	assert ( m_bGotGroupby );
	if ( m_tQuery.m_bExplicitMaxMatches || m_tSettings.m_bForceSingleThread )
		return iMaxMatches;

	int iGroupbyAttr = GetGroupbyAttrIndex();
	if ( iGroupbyAttr<0 )
		return iMaxMatches;

	CSphString sModifiedAttr;
	int iCountDistinct = m_tSettings.m_fnGetCountDistinct ? m_tSettings.m_fnGetCountDistinct ( m_pSorterSchema->GetAttr(iGroupbyAttr).m_sName, sModifiedAttr ) : -1;
	if ( iCountDistinct > m_tQuery.m_iMaxMatchThresh )
		return iMaxMatches;

	return Max ( iCountDistinct, iMaxMatches );
}


bool QueueCreator_c::CanCalcFastCountDistinct() const
{
	bool bHasAggregates = PredictAggregates();
	return !bHasAggregates && m_tGroupSorterSettings.m_bImplicit && m_tGroupSorterSettings.m_bDistinct && m_tQuery.m_dFilters.IsEmpty() && m_tQuery.m_sQuery.IsEmpty() && m_tQuery.m_sKNNAttr.IsEmpty() && m_tQuery.m_eJoinType!=JoinType_e::INNER;
}


bool QueueCreator_c::CanCalcFastCountFilter() const
{
	bool bHasAggregates = PredictAggregates();
	return !bHasAggregates && m_tGroupSorterSettings.m_bImplicit && !m_tGroupSorterSettings.m_bDistinct && m_tQuery.m_dFilters.GetLength()==1 && m_tQuery.m_sQuery.IsEmpty() && m_tQuery.m_sKNNAttr.IsEmpty() && m_tQuery.m_eJoinType!=JoinType_e::INNER;
}


bool QueueCreator_c::CanCalcFastCount() const
{
	bool bHasAggregates = PredictAggregates();
	return !bHasAggregates && m_tGroupSorterSettings.m_bImplicit && !m_tGroupSorterSettings.m_bDistinct && m_tQuery.m_dFilters.IsEmpty() && m_tQuery.m_sQuery.IsEmpty() && m_tQuery.m_sKNNAttr.IsEmpty() && m_tQuery.m_eJoinType!=JoinType_e::INNER;
}


PrecalculatedSorterResults_t QueueCreator_c::FetchPrecalculatedValues() const
{
	PrecalculatedSorterResults_t tPrecalc;
	if ( CanCalcFastCountDistinct() )
	{
		int iCountDistinctAttr = GetGroupDistinctAttrIndex();
		if ( iCountDistinctAttr>0 && m_tSettings.m_bEnableFastDistinct )
			tPrecalc.m_iCountDistinct = m_tSettings.m_fnGetCountDistinct ? m_tSettings.m_fnGetCountDistinct ( m_pSorterSchema->GetAttr(iCountDistinctAttr).m_sName, tPrecalc.m_sAttr ) : -1;
	}

	if ( CanCalcFastCountFilter() )
		tPrecalc.m_iCountFilter = m_tSettings.m_fnGetCountFilter ? m_tSettings.m_fnGetCountFilter ( m_tQuery.m_dFilters[0], tPrecalc.m_sAttr ) : -1;

	if ( CanCalcFastCount() )
		tPrecalc.m_iCount = m_tSettings.m_fnGetCount ? m_tSettings.m_fnGetCount() : -1;

	return tPrecalc;
}


ISphMatchSorter * QueueCreator_c::SpawnQueue()
{
	bool bNeedFactors = !!(m_uPackedFactorFlags & SPH_FACTOR_ENABLE);

	if ( m_bGotGroupby )
	{
		m_tGroupSorterSettings.m_bGrouped = m_tSettings.m_bGrouped;
		m_tGroupSorterSettings.m_iMaxMatches = AdjustMaxMatches ( m_tGroupSorterSettings.m_iMaxMatches );
		if ( m_pProfile )
			m_pProfile->m_iMaxMatches = m_tGroupSorterSettings.m_iMaxMatches;

		PrecalculatedSorterResults_t tPrecalc = FetchPrecalculatedValues();
		return CreateSorter ( m_eMatchFunc, m_eGroupFunc, &m_tQuery, m_tGroupSorterSettings, bNeedFactors, PredictAggregates(), tPrecalc );
	}

	if ( m_tQuery.m_iLimit == -1 && m_tSettings.m_pSqlRowBuffer )
		return CreateDirectSqlQueue ( m_tSettings.m_pSqlRowBuffer, m_tSettings.m_ppOpaque1, m_tSettings.m_ppOpaque2, std::move (m_tSettings.m_dCreateSchema) );

	if ( m_tSettings.m_pCollection )
		return CreateCollectQueue ( m_tSettings.m_iMaxMatches, *m_tSettings.m_pCollection );

	int iMaxMatches = ReduceMaxMatches();
	if ( m_pProfile )
		m_pProfile->m_iMaxMatches = iMaxMatches;

	ISphMatchSorter * pPlainSorter = CreatePlainSorter ( m_eMatchFunc, m_tQuery.m_bSortKbuffer, iMaxMatches, bNeedFactors );
	if ( !pPlainSorter )
		return nullptr;

	ISphMatchSorter * pScrollSorter = CreateScrollSorter ( pPlainSorter, *m_pSorterSchema, m_eMatchFunc, m_tQuery.m_tScrollSettings, m_bMulti );
	if ( !pScrollSorter )
		return nullptr;

	return CreateColumnarProxySorter ( pScrollSorter, iMaxMatches, *m_pSorterSchema, m_tStateMatch, m_eMatchFunc, bNeedFactors, m_tSettings.m_bComputeItems, m_bMulti );
}


bool QueueCreator_c::SetupComputeQueue ()
{
	return AddJoinAttrs()
		&& AddJoinFilterAttrs()
		&& MaybeAddGeodistColumn ()
		&& AddKNNDistColumn()
		&& MaybeAddExprColumn ()
		&& MaybeAddExpressionsFromSelectList ()
		&& AddExpressionsForUpdates()
		&& AddNullBitmask();
}

bool QueueCreator_c::SetupGroupQueue ()
{
	return AddGroupbyStuff ()
		&& SetupMatchesSortingFunc ()
		&& SetGroupSorting ();
}

bool QueueCreator_c::ConvertColumnarToDocstore()
{
	// don't use docstore (need to try to keep schemas similar for multiquery to work)
	if ( m_tQuery.m_bFacet || m_tQuery.m_bFacetHead )
		return true;

	// check for columnar attributes that have FINAL eval stage
	// if we have more than 1 of such attributes (and they are also stored), we replace columnar expressions with columnar expressions
	CSphVector<int> dStoredColumnar;
	auto & tSchema = *m_pSorterSchema;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		auto & tAttr = tSchema.GetAttr(i);
		bool bStored = false;
		bool bColumnar = tAttr.m_pExpr && tAttr.m_pExpr->IsColumnar(&bStored);
		if ( bColumnar && bStored && tAttr.m_eStage==SPH_EVAL_FINAL )
			dStoredColumnar.Add(i);
	}

	if ( dStoredColumnar.GetLength()<=1 )
		return true;

	for ( auto i : dStoredColumnar )
	{
		auto & tAttr = const_cast<CSphColumnInfo&>( tSchema.GetAttr(i) );

		CSphString sColumnarAttrName;
		tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sColumnarAttrName );
		tAttr.m_pExpr = CreateExpr_GetStoredAttr ( sColumnarAttrName, tAttr.m_eAttrType );
	}

	return true;
}

bool QueueCreator_c::SetupQueue ()
{
	return SetupComputeQueue ()
		&& SetupGroupQueue ()
		&& ConvertColumnarToDocstore();
}

ISphMatchSorter * QueueCreator_c::CreateQueue ()
{
	SetupCollation();

	if ( m_bHeadWOGroup && m_tGroupSorterSettings.m_bImplicit )
	{
		m_tGroupSorterSettings.m_bImplicit = false;
		m_bGotGroupby = false;
	}

	///////////////////
	// spawn the queue
	///////////////////

	ISphMatchSorter * pTop = SpawnQueue();
	if ( !pTop )
	{
		Err ( "internal error: unhandled sorting mode (match-sort=%d, group=%d, group-sort=%d)", m_eMatchFunc, m_bGotGroupby, m_eGroupFunc );
		return nullptr;
	}

	assert ( pTop );
	pTop->SetSchema ( m_pSorterSchema.release(), false );
	pTop->SetState ( m_tStateMatch );
	pTop->SetGroupState ( m_tStateGroup );
	pTop->SetRandom ( m_bRandomize );
	if ( !m_bHaveStar && m_hQueryColumns.GetLength() )
		pTop->SetFilteredAttrs ( m_hQueryColumns, m_tSettings.m_bNeedDocids || m_bExprsNeedDocids );

	if ( m_bRandomize )
	{
		if ( m_tQuery.m_iRandSeed>=0 )
			sphSrand ( (DWORD)m_tQuery.m_iRandSeed );
		else
			sphAutoSrand();
	}

	return pTop;
}

static void ResetRemaps ( CSphMatchComparatorState & tState )
{
	for ( int i = 0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dRemapped.BitGet ( i ) && tState.m_eKeypart[i]==SPH_KEYPART_STRINGPTR )
			tState.m_dRemapped.BitClear ( i );
	}
}

bool QueueCreator_c::SetSchemaGroupQueue ( const CSphRsetSchema & tNewSchema )
{
	// need to reissue remap but with existed attributes
	ResetRemaps ( m_tStateMatch );
	ResetRemaps ( m_tStateGroup );

	*m_pSorterSchema = tNewSchema;

	return SetupGroupQueue();
}

///////////////////////////////////////////////////////////////////////////////

static ISphMatchSorter * CreateQueue ( QueueCreator_c & tCreator, SphQueueRes_t & tRes )
{
	ISphMatchSorter * pSorter = tCreator.CreateQueue ();
	tRes.m_bZonespanlist = tCreator.m_bZonespanlist;
	tRes.m_uPackedFactorFlags = tCreator.m_uPackedFactorFlags;
	tRes.m_bJoinedGroupSort	= tCreator.m_bJoinedGroupSort;
	return pSorter;
}


static void CreateSorters ( const VecTraits_T<CSphQuery> & dQueries, const VecTraits_T<ISphMatchSorter*> & dSorters, const VecTraits_T<QueueCreator_c> & dCreators,	const VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes )
{
	ARRAY_FOREACH ( i, dCreators )
	{
		if ( !dCreators[i].m_bCreate )
			continue;

		dSorters[i] = CreateQueue ( dCreators[i], tRes );
		assert ( dSorters[i]!=nullptr );
	}

	if ( tRes.m_bAlowMulti )
	{
		ISphMatchSorter * pSorter0 = nullptr;
		for ( int iCheck=0; iCheck<dSorters.GetLength(); ++iCheck )
		{
			if ( !dCreators[iCheck].m_bCreate )
				continue;

			assert ( dSorters[iCheck] );
			if ( !pSorter0 )
			{
				pSorter0 = dSorters[iCheck];
				continue;
			}

			assert ( dSorters[iCheck]->GetSchema()->GetAttrsCount()==pSorter0->GetSchema()->GetAttrsCount() );
		}
	}
}


static void CreateMultiQueue ( RawVector_T<QueueCreator_c> & dCreators, const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile )
{
	assert ( dSorters.GetLength()>1 );
	assert ( dSorters.GetLength()==dQueries.GetLength() );
	assert ( dSorters.GetLength()==dErrors.GetLength() );

	dCreators.Reserve_static ( dSorters.GetLength () );
	dCreators.Emplace_back( tQueue, dQueries[0], dErrors[0], pExtra, pProfile );
	dCreators[0].m_bMulti = true;

	// same as SetupQueue
	bool bSuccess = dCreators[0].SetupComputeQueue ();
	// copy schema WO group by and internals
	CSphRsetSchema tRefSchema = dCreators[0].SorterSchema();
	bool bHasJson = dCreators[0].HasJson();
	bool bJsonMixed = false;

	if ( bSuccess )
		bSuccess &= dCreators[0].SetupGroupQueue ();

	dCreators[0].m_bCreate = bSuccess;

	// create rest of schemas
	for ( int i=1; i<dSorters.GetLength(); ++i )
	{
		// fill extra only for initial pass
		dCreators.Emplace_back ( tQueue, dQueries[i], dErrors[i], pExtra, pProfile );
		dCreators[i].m_bMulti = true;
		if ( !dCreators[i].SetupQueue () )
		{
			dCreators[i].m_bCreate = false;
			continue;
		}

		bJsonMixed |= ( bHasJson!=dCreators[i].HasJson () );
		bHasJson |= dCreators[i].HasJson();
	}

	// FIXME!!! check attributes and expressions matches
	bool bSame = !bJsonMixed;
	const auto& tSchema0 = dCreators[0].SorterSchema();
	for ( int i=1; i<dCreators.GetLength() && bSame; ++i )
	{
		const auto & tCur = dCreators[i].SorterSchema();
		bSame &= ( tSchema0.GetDynamicSize()==tCur.GetDynamicSize() && tSchema0.GetAttrsCount()==tCur.GetAttrsCount() );
	}

	// same schemes
	if ( bSame )
		return;

	CSphRsetSchema tMultiSchema = tRefSchema;

	int iMinGroups = INT_MAX;
	int iMaxGroups = 0;
	bool bHasMulti = false;
	ARRAY_FOREACH ( iSchema, dCreators )
	{
		if ( !dCreators[iSchema].m_bCreate )
			continue;

		int iGroups = 0;
		const CSphRsetSchema & tSchema = dCreators[iSchema].SorterSchema();
		for ( int iCol=0; iCol<tSchema.GetAttrsCount(); ++iCol )
		{
			const CSphColumnInfo & tCol = tSchema.GetAttr ( iCol );
			if ( !tCol.m_tLocator.m_bDynamic && !tCol.IsColumnar() )
				continue;

			if ( IsGroupbyMagic ( tCol.m_sName ) )
			{
				++iGroups;
				if ( !IsSortJsonInternal ( tCol.m_sName ))
					continue;
			}

			const CSphColumnInfo * pMultiCol = tMultiSchema.GetAttr ( tCol.m_sName.cstr() );
			if ( pMultiCol )
			{
				bool bDisable1 = false;
				bool bDisable2 = false;
				// no need to add attributes that already exists
				if ( pMultiCol->m_eAttrType==tCol.m_eAttrType &&
					( ( !pMultiCol->m_pExpr && !tCol.m_pExpr ) ||
						( pMultiCol->m_pExpr && tCol.m_pExpr
							&& pMultiCol->m_pExpr->GetHash ( tMultiSchema, SPH_FNV64_SEED, bDisable1 )==tCol.m_pExpr->GetHash ( tSchema, SPH_FNV64_SEED, bDisable2 ) )
						) )
					continue;

				// no need to add a new column, but we need the same schema for the sorters
				if ( tCol.IsColumnar() && pMultiCol->IsColumnarExpr() )
				{
					bHasMulti = true;
					continue;
				}

				if ( !tCol.IsColumnarExpr() || !pMultiCol->IsColumnar() ) 				// need a new column
				{
					tRes.m_bAlowMulti = false; // if attr or expr differs need to create regular sorters and issue search WO multi-query
					return;
				}
			}

			bHasMulti = true;
			tMultiSchema.AddAttr ( tCol, true );
			if ( tCol.m_pExpr )
				tCol.m_pExpr->FixupLocator ( &tSchema, &tMultiSchema );
		}

		iMinGroups = Min ( iMinGroups, iGroups );
		iMaxGroups = Max ( iMaxGroups, iGroups );
	}

	// usual multi query should all have similar group by
	if ( iMinGroups!=iMaxGroups && !dQueries[0].m_bFacetHead && !dQueries[0].m_bFacet )
	{
		tRes.m_bAlowMulti = false;
		return;
	}

	// only group attributes differs - create regular sorters
	if ( !bHasMulti && !bJsonMixed )
		return;

	// setup common schemas
	for ( QueueCreator_c & tCreator : dCreators )
	{
		if ( !tCreator.m_bCreate )
			continue;

		if ( !tCreator.SetSchemaGroupQueue ( tMultiSchema ) )
			tCreator.m_bCreate = false;
	}
}

///////////////////////////////////////////////////////////////////////////////

ISphMatchSorter * sphCreateQueue ( const SphQueueSettings_t & tQueue, const CSphQuery & tQuery, CSphString & sError, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile )
{
	QueueCreator_c tCreator ( tQueue, tQuery, sError, pExtra, pProfile );
	if ( !tCreator.SetupQueue () )
		return nullptr;

	return CreateQueue ( tCreator, tRes );
}


void sphCreateMultiQueue ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, StrVec_t * pExtra, QueryProfile_c * pProfile )
{
	RawVector_T<QueueCreator_c> dCreators;
	CreateMultiQueue ( dCreators, tQueue, dQueries, dSorters, dErrors, tRes, pExtra, pProfile );
	CreateSorters ( dQueries, dSorters, dCreators, dErrors, tRes );
}
