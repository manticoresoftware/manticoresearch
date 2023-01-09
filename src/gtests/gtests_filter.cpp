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

#include <gtest/gtest.h>

#include "sphinxfilter.h"
#include "conversion.h"

class filter_block_level : public ::testing::Test
{

protected:
	void SetDefault()
	{
		tOpt.m_sAttrName = "gid";
		tOpt.m_bExclude = false;
		tOpt.m_bHasEqualMin = true;
		tOpt.m_bHasEqualMax = true;
		tOpt.m_bOpenLeft = false;
		tOpt.m_bOpenRight = false;
		tOpt.m_eType = SPH_FILTER_RANGE;
	}

	void SetUp() override
	{
		SetDefault();
	}

	CSphFilterSettings tOpt;
	CreateFilterContext_t tCtx;
};

TEST_F ( filter_block_level, range )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(DocID_t) + 1 ), dMax ( DWSIZEOF(DocID_t) + 1 );
	std::unique_ptr<ISphFilter> tFilter;

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_iMinValue = 10;
	tOpt.m_iMaxValue = 40;

	tCtx.m_pSchema = &tSchema;

	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter >=10 && <=40 vs block 1-5
	*dMin.Begin() = 1;
	*dMax.Begin() = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 1-10
	*dMax.Begin() = 10;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 40-50
	*dMin.Begin() = 40;
	*dMax.Begin() = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 41-50
	*dMin.Begin() = 41;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 9-41
	*dMin.Begin() = 9;
	*dMax.Begin() = 41;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault();
	tOpt.m_iMaxValue = 40;
	tOpt.m_bOpenLeft = true;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter <=40 vs block 41-50
	*dMin.Begin() = 41;
	*dMax.Begin() = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 40-50
	*dMin.Begin() = 40;
	*dMax.Begin() = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 39-50
	*dMin.Begin() = 39;
	*dMax.Begin() = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 30-40
	*dMin.Begin() = 30;
	*dMax.Begin() = 40;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 1-4
	*dMin.Begin() = 1;
	*dMax.Begin() = 4;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_iMinValue = 15;
	tOpt.m_bOpenRight = true;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter >=15 vs block 10-14
	*dMin.Begin() = 10;
	*dMax.Begin() = 14;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 10-15
	*dMin.Begin() = 10;
	*dMax.Begin() = 15;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 10-16
	*dMin.Begin() = 10;
	*dMax.Begin() = 16;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 55-65
	*dMin.Begin() = 55;
	*dMax.Begin() = 65;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 1-5
	*dMin.Begin() = 1;
	*dMax.Begin() = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	SetDefault ();
	tOpt.m_iMinValue = 10;
	tOpt.m_iMaxValue = 40;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter >10 && <40 vs block 1-5
	*dMin.Begin() = 1;
	*dMax.Begin() = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 1-10
	*dMax.Begin() = 10;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 40-50
	*dMin.Begin() = 40;
	*dMax.Begin() = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 41-50
	*dMin.Begin() = 41;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 39-50
	*dMin.Begin() = 39;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 9-41
	*dMin.Begin() = 9;
	*dMax.Begin() = 41;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_iMaxValue = 40;
	tOpt.m_bOpenLeft = true;
	tOpt.m_bHasEqualMax = false;
	tOpt.m_bHasEqualMin = false;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter <40 vs block 40-50
	*dMin.Begin() = 40;
	*dMax.Begin() = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 41-50
	*dMin.Begin() = 41;
	*dMax.Begin() = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 39-50
	*dMin.Begin() = 39;
	*dMax.Begin() = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 30-40
	*dMin.Begin() = 30;
	*dMax.Begin() = 40;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 1-4
	*dMin.Begin() = 1;
	*dMax.Begin() = 4;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_iMinValue = 15;
	tOpt.m_bOpenRight = true;
	tOpt.m_bHasEqualMax = false;
	tOpt.m_bHasEqualMin = false;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter >15 vs block 10-15
	*dMin.Begin() = 10;
	*dMax.Begin() = 15;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 10-16
	*dMin.Begin() = 10;
	*dMax.Begin() = 16;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 55-65
	*dMin.Begin() = 55;
	*dMax.Begin() = 65;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 1-5
	*dMin.Begin() = 1;
	*dMax.Begin() = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, range_float )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(DocID_t) + 1 ), dMax ( DWSIZEOF(DocID_t) + 1 );
	std::unique_ptr<ISphFilter> tFilter;

	tCol.m_eAttrType = SPH_ATTR_FLOAT;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_FLOATRANGE;
	tOpt.m_fMinValue = 10.0f;
	tOpt.m_fMaxValue = 40.0f;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tCtx.m_pSchema = &tSchema;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter >10 && <40 vs block 1-5
	*dMin.Begin() = sphF2DW ( 1.0f );
	*dMax.Begin() = sphF2DW ( 5.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 1-10.1
	*dMax.Begin() = sphF2DW ( 10.1f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 39.9-50
	*dMin.Begin() = sphF2DW ( 39.9f );
	*dMax.Begin() = sphF2DW ( 50.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 40-50
	*dMin.Begin() = sphF2DW (  40.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 9-41
	*dMin.Begin() = sphF2DW ( 9.0f );
	*dMax.Begin() = sphF2DW ( 41.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_eType = SPH_FILTER_FLOATRANGE;
	tOpt.m_fMinValue = 0.0f;
	tOpt.m_fMaxValue = 40.0f;
	tOpt.m_bOpenLeft = true;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter!=nullptr );

	// filter <40 vs block 40-50
	*dMin.Begin() = sphF2DW ( 40.0f );
	*dMax.Begin() = sphF2DW ( 50.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 39.9-50
	*dMin.Begin() = sphF2DW ( 39.9f );
	*dMax.Begin() = sphF2DW ( 50.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 30-40
	*dMin.Begin() = sphF2DW ( 30.0f );
	*dMax.Begin() = sphF2DW ( 40.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 1-4
	*dMin.Begin() = sphF2DW ( 1.0f );
	*dMax.Begin() = sphF2DW ( 4.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	tOpt.m_fMinValue = 30.0f;
	tOpt.m_fMaxValue = 40.0f;
	tOpt.m_bOpenLeft = true; // FIXME!!! OpenLeft should work for FLOAT range too
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter != nullptr );

	// filter <40 vs block 1-4
	*dMin.Begin() = sphF2DW ( 1.0f );
	*dMax.Begin() = sphF2DW ( 4.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) ); // FIXME!!! should be TRUE due to m_bOpenLeft option

	///
	SetDefault ();
	tOpt.m_eType = SPH_FILTER_FLOATRANGE;
	tOpt.m_fMinValue = 15.0f;
	tOpt.m_fMaxValue = 100.0f;
	tOpt.m_bOpenRight = true;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter != nullptr );

	// filter >15 vs block 10-15
	*dMin.Begin() = sphF2DW ( 10.0f );
	*dMax.Begin() = sphF2DW ( 15.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 10-16
	*dMin.Begin() = sphF2DW ( 10.0f );
	*dMax.Begin() = sphF2DW ( 16.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 55-65
	*dMin.Begin() = sphF2DW ( 55.0f );
	*dMax.Begin() = sphF2DW ( 65.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 1-5
	*dMin.Begin() = sphF2DW ( 1.0f );
	*dMax.Begin() = sphF2DW ( 5.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 150-200
	*dMin.Begin() = sphF2DW ( 150.0f );
	*dMax.Begin() = sphF2DW ( 200.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) ); // FIXME!!! should be TRUE due to m_bOpenRight option
}

TEST_F ( filter_block_level, values )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(DocID_t) + 1 ), dMax ( DWSIZEOF(DocID_t) + 1 );
	std::unique_ptr<ISphFilter> tFilter;

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dValues[] = { 10, 40, 100 };
	tOpt.SetExternalValues ( { dValues, sizeof ( dValues ) / sizeof ( dValues[0] ) } );
	tCtx.m_pSchema = &tSchema;

	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter != nullptr );

	// filter values vs block 1-9
	*dMin.Begin() = 1;
	*dMax.Begin() = 9;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 11-39
	*dMin.Begin() = 11;
	*dMax.Begin() = 39;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-101
	*dMin.Begin() = 9;
	*dMax.Begin() = 101;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 41-101
	*dMin.Begin() = 41;
	*dMax.Begin() = 101;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	/// single value
	SetDefault ();
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dValuesSingle[] = { 10 };
	tOpt.SetExternalValues ( { dValuesSingle, sizeof ( dValuesSingle ) / sizeof ( dValuesSingle[0] ) } );

	tFilter = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( tFilter != nullptr );

	// filter values vs block 1-9
	*dMin.Begin() = 1;
	*dMax.Begin() = 9;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 11-39
	*dMin.Begin() = 11;
	*dMax.Begin() = 39;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-101
	*dMin.Begin() = 9;
	*dMax.Begin() = 11;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, and2 )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(DocID_t) + 1 ), dMax ( DWSIZEOF(DocID_t) + 1 );
	std::unique_ptr<ISphFilter> tFilter;

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dVal1[] = { 10 };
	tOpt.SetExternalValues ( { dVal1, sizeof ( dVal1 ) / sizeof ( dVal1[0] ) } );

	tCtx.m_pSchema = &tSchema;

	auto pFilter1 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter1!=nullptr );

	SphAttr_t dVal2[] = { 20 };
	tOpt.SetExternalValues ( { dVal2, sizeof ( dVal2 ) / sizeof ( dVal2[0] ) } );

	auto pFilter2 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter2!=nullptr );

	tFilter = sphJoinFilters ( std::move ( pFilter1 ), std::move ( pFilter2 ) );
	ASSERT_TRUE ( tFilter != nullptr );

	// filter values vs block 5-9
	*dMin.Begin() = 5;
	*dMax.Begin() = 9;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 11-29
	*dMin.Begin() = 11;
	*dMax.Begin() = 29;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-21
	*dMin.Begin() = 9;
	*dMax.Begin() = 21;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, and3 )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(DocID_t) + 1 ), dMax ( DWSIZEOF(DocID_t) + 1 );
	std::unique_ptr<ISphFilter> tFilter;

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dVal1[] = { 10 };
	tOpt.SetExternalValues ( { dVal1, sizeof ( dVal1 ) / sizeof ( dVal1[0] ) } );

	tCtx.m_pSchema = &tSchema;

	auto pFilter1 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter1!=nullptr );

	SphAttr_t dVal2[] = { 15 };
	tOpt.SetExternalValues ( { dVal2, sizeof ( dVal2 ) / sizeof ( dVal2[0] ) } );

	auto pFilter2 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter2!=nullptr );

	SphAttr_t dVal3[] = { 20 };
	tOpt.SetExternalValues ( { dVal3, sizeof ( dVal3 ) / sizeof ( dVal3[0] ) } );

	auto pFilter3 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter3!=nullptr );

	tFilter = sphJoinFilters ( std::move ( pFilter1 ), sphJoinFilters ( std::move ( pFilter2 ), std::move ( pFilter3 ) ) );
	ASSERT_TRUE ( tFilter != nullptr );

	// filter values vs block 11-14
	*dMin.Begin() = 11;
	*dMax.Begin() = 14;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-16
	*dMin.Begin() = 9;
	*dMax.Begin() = 16;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 5-25
	*dMin.Begin() = 5;
	*dMax.Begin() = 25;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, and )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(DocID_t) + 1 ), dMax ( DWSIZEOF(DocID_t) + 1 );
	std::unique_ptr<ISphFilter> tFilter;

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dVal1[] = { 10 };
	tOpt.SetExternalValues ( { dVal1, sizeof ( dVal1 ) / sizeof ( dVal1[0] ) } );

	tCtx.m_pSchema = &tSchema;

	auto pFilter1 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter1!=nullptr );

	SphAttr_t dVal2[] = { 14 };
	tOpt.SetExternalValues ( { dVal2, sizeof ( dVal2 ) / sizeof ( dVal2[0] ) } );

	auto pFilter2 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter2!= nullptr );

	SphAttr_t dVal3[] = { 18 };
	tOpt.SetExternalValues ( { dVal3, sizeof ( dVal3 ) / sizeof ( dVal3[0] ) } );

	auto pFilter3 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter3!= nullptr );

	SphAttr_t dVal4[] = { 20 };
	tOpt.SetExternalValues ( { dVal4, sizeof ( dVal4 ) / sizeof ( dVal4[0] ) } );

	auto pFilter4 = sphCreateFilter ( tOpt, tCtx, sError, sWarning );
	ASSERT_TRUE ( pFilter4!= nullptr );

	tFilter = sphJoinFilters ( std::move ( pFilter1 ), sphJoinFilters ( std::move ( pFilter2 ), sphJoinFilters ( std::move ( pFilter3 ), std::move ( pFilter4 ) ) ) );
	ASSERT_TRUE ( tFilter != nullptr );

	// filter values vs block 5-19
	*dMin.Begin() = 5;
	*dMax.Begin() = 19;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 1-30
	*dMin.Begin() = 1;
	*dMax.Begin() = 30;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}
