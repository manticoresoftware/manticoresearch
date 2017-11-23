//
// Created by alexey on 28.09.17.
//

#include <gtest/gtest.h>

#include "sphinx.h"
#include "sphinxfilter.h"

class filter_block_level : public ::testing::Test
{

protected:
	void SetDefault ( )
	{
		tOpt.m_sAttrName = "gid";
		tOpt.m_bExclude = false;
		tOpt.m_bHasEqualMin = true;
		tOpt.m_bHasEqualMax = true;
		tOpt.m_bOpenLeft = false;
		tOpt.m_bOpenRight = false;
		tOpt.m_eType = SPH_FILTER_RANGE;
	}

	virtual void SetUp ()
	{
		SetDefault ();
	}

	CSphFilterSettings tOpt;
};

TEST_F ( filter_block_level, range )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(SphDocID_t) + 1 ), dMax ( DWSIZEOF(SphDocID_t) + 1 );
	CSphScopedPtr<ISphFilter> tFilter ( NULL );

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_iMinValue = 10;
	tOpt.m_iMaxValue = 40;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter >=10 && <=40 vs block 1-5
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 1-10
	*DOCINFO2ATTRS ( dMax.Begin() ) = 10;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 40-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 40;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 41-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 41;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=10 && <=40 vs block 9-41
	*DOCINFO2ATTRS ( dMin.Begin() ) = 9;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 41;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault();
	tOpt.m_iMaxValue = 40;
	tOpt.m_bOpenLeft = true;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter <=40 vs block 41-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 41;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 40-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 40;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 39-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 39;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 30-40
	*DOCINFO2ATTRS ( dMin.Begin() ) = 30;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 40;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <=40 vs block 1-4
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 4;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_iMinValue = 15;
	tOpt.m_bOpenRight = true;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter >=15 vs block 10-14
	*DOCINFO2ATTRS ( dMin.Begin() ) = 10;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 14;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 10-15
	*DOCINFO2ATTRS ( dMin.Begin() ) = 10;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 15;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 10-16
	*DOCINFO2ATTRS ( dMin.Begin() ) = 10;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 16;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 55-65
	*DOCINFO2ATTRS ( dMin.Begin() ) = 55;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 65;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >=15 vs block 1-5
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	SetDefault ();
	tOpt.m_iMinValue = 10;
	tOpt.m_iMaxValue = 40;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter >10 && <40 vs block 1-5
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 1-10
	*DOCINFO2ATTRS ( dMax.Begin() ) = 10;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 40-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 40;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 41-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 41;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 39-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 39;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 9-41
	*DOCINFO2ATTRS ( dMin.Begin() ) = 9;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 41;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_iMaxValue = 40;
	tOpt.m_bOpenLeft = true;
	tOpt.m_bHasEqualMax = false;
	tOpt.m_bHasEqualMin = false;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter <40 vs block 40-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 40;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 41-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 41;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 39-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = 39;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 50;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 30-40
	*DOCINFO2ATTRS ( dMin.Begin() ) = 30;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 40;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 1-4
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 4;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_iMinValue = 15;
	tOpt.m_bOpenRight = true;
	tOpt.m_bHasEqualMax = false;
	tOpt.m_bHasEqualMin = false;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter >15 vs block 10-15
	*DOCINFO2ATTRS ( dMin.Begin() ) = 10;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 15;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 10-16
	*DOCINFO2ATTRS ( dMin.Begin() ) = 10;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 16;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 55-65
	*DOCINFO2ATTRS ( dMin.Begin() ) = 55;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 65;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 1-5
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 5;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, range_float )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(SphDocID_t) + 1 ), dMax ( DWSIZEOF(SphDocID_t) + 1 );
	CSphScopedPtr<ISphFilter> tFilter ( NULL );

	tCol.m_eAttrType = SPH_ATTR_FLOAT;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_FLOATRANGE;
	tOpt.m_fMinValue = 10.0f;
	tOpt.m_fMaxValue = 40.0f;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter >10 && <40 vs block 1-5
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 1.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 5.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 1-10.1
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 10.1f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 39.9-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 39.9f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 50.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 40-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW (  40.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >10 && <40 vs block 9-41
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 9.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 41.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	///
	SetDefault ();
	tOpt.m_eType = SPH_FILTER_FLOATRANGE;
	tOpt.m_fMinValue = 0.0f;
	tOpt.m_fMaxValue = 40.0f;
	tOpt.m_bOpenLeft = true;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter <40 vs block 40-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 40.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 50.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 39.9-50
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 39.9f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 50.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 30-40
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 30.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 40.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter <40 vs block 1-4
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 1.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 4.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	tOpt.m_fMinValue = 30.0f;
	tOpt.m_fMaxValue = 40.0f;
	tOpt.m_bOpenLeft = true; // FIXME!!! OpenLeft should work for FLOAT range too
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter <40 vs block 1-4
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 1.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 4.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) ); // FIXME!!! should be TRUE due to m_bOpenLeft option

	///
	SetDefault ();
	tOpt.m_eType = SPH_FILTER_FLOATRANGE;
	tOpt.m_fMinValue = 15.0f;
	tOpt.m_fMaxValue = 100.0f;
	tOpt.m_bOpenRight = true;
	tOpt.m_bHasEqualMin = false;
	tOpt.m_bHasEqualMax = false;
	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter >15 vs block 10-15
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 10.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 15.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 10-16
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 10.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 16.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 55-65
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 55.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 65.0f );
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 1-5
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 1.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 5.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter >15 vs block 150-200
	*DOCINFO2ATTRS ( dMin.Begin() ) = sphF2DW ( 150.0f );
	*DOCINFO2ATTRS ( dMax.Begin() ) = sphF2DW ( 200.0f );
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) ); // FIXME!!! should be TRUE due to m_bOpenRight option

}

TEST_F ( filter_block_level, values )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(SphDocID_t) + 1 ), dMax ( DWSIZEOF(SphDocID_t) + 1 );
	CSphScopedPtr<ISphFilter> tFilter ( NULL );

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dValues[] = { 10, 40, 100 };
	tOpt.SetExternalValues ( dValues, sizeof ( dValues ) / sizeof ( dValues[0] ) );

	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter values vs block 1-9
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 9;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 11-39
	*DOCINFO2ATTRS ( dMin.Begin() ) = 11;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 39;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-101
	*DOCINFO2ATTRS ( dMin.Begin() ) = 9;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 101;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 41-101
	*DOCINFO2ATTRS ( dMin.Begin() ) = 41;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 101;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	/// single value
	SetDefault ();
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dValuesSingle[] = { 10 };
	tOpt.SetExternalValues ( dValuesSingle, sizeof ( dValuesSingle ) / sizeof ( dValuesSingle[0] ) );

	tFilter = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter values vs block 1-9
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 9;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 11-39
	*DOCINFO2ATTRS ( dMin.Begin() ) = 11;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 39;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-101
	*DOCINFO2ATTRS ( dMin.Begin() ) = 9;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 11;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, and2 )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(SphDocID_t) + 1 ), dMax ( DWSIZEOF(SphDocID_t) + 1 );
	CSphScopedPtr<ISphFilter> tFilter ( NULL );

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dVal1[] = { 10 };
	tOpt.SetExternalValues ( dVal1, sizeof ( dVal1 ) / sizeof ( dVal1[0] ) );

	ISphFilter * pFilter1 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter1!=NULL );

	SphAttr_t dVal2[] = { 20 };
	tOpt.SetExternalValues ( dVal2, sizeof ( dVal2 ) / sizeof ( dVal2[0] ) );

	ISphFilter * pFilter2 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter2!=NULL );

	tFilter = sphJoinFilters ( pFilter1, pFilter2 );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter values vs block 5-9
	*DOCINFO2ATTRS ( dMin.Begin() ) = 5;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 9;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 11-29
	*DOCINFO2ATTRS ( dMin.Begin() ) = 11;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 29;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-21
	*DOCINFO2ATTRS ( dMin.Begin() ) = 9;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 21;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, and3 )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(SphDocID_t) + 1 ), dMax ( DWSIZEOF(SphDocID_t) + 1 );
	CSphScopedPtr<ISphFilter> tFilter ( NULL );

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dVal1[] = { 10 };
	tOpt.SetExternalValues ( dVal1, sizeof ( dVal1 ) / sizeof ( dVal1[0] ) );

	ISphFilter * pFilter1 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter1!=NULL );

	SphAttr_t dVal2[] = { 15 };
	tOpt.SetExternalValues ( dVal2, sizeof ( dVal2 ) / sizeof ( dVal2[0] ) );

	ISphFilter * pFilter2 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter2!=NULL );

	SphAttr_t dVal3[] = { 20 };
	tOpt.SetExternalValues ( dVal3, sizeof ( dVal3 ) / sizeof ( dVal3[0] ) );

	ISphFilter * pFilter3 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter3!=NULL );

	tFilter = sphJoinFilters ( pFilter1, sphJoinFilters ( pFilter2, pFilter3 ) );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter values vs block 11-14
	*DOCINFO2ATTRS ( dMin.Begin() ) = 11;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 14;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 9-16
	*DOCINFO2ATTRS ( dMin.Begin() ) = 9;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 16;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 5-25
	*DOCINFO2ATTRS ( dMin.Begin() ) = 5;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 25;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}

TEST_F ( filter_block_level, and )
{
	CSphString sWarning, sError;
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	CSphFixedVector<DWORD> dMin ( DWSIZEOF(SphDocID_t) + 1 ), dMax ( DWSIZEOF(SphDocID_t) + 1 );
	CSphScopedPtr<ISphFilter> tFilter ( NULL );

	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "gid";
	tSchema.AddAttr ( tCol, false );
	
	tOpt.m_eType = SPH_FILTER_VALUES;
	SphAttr_t dVal1[] = { 10 };
	tOpt.SetExternalValues ( dVal1, sizeof ( dVal1 ) / sizeof ( dVal1[0] ) );

	ISphFilter * pFilter1 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter1!=NULL );

	SphAttr_t dVal2[] = { 14 };
	tOpt.SetExternalValues ( dVal2, sizeof ( dVal2 ) / sizeof ( dVal2[0] ) );

	ISphFilter * pFilter2 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter2!=NULL );

	SphAttr_t dVal3[] = { 18 };
	tOpt.SetExternalValues ( dVal3, sizeof ( dVal3 ) / sizeof ( dVal3[0] ) );

	ISphFilter * pFilter3 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter3!=NULL );

	SphAttr_t dVal4[] = { 20 };
	tOpt.SetExternalValues ( dVal4, sizeof ( dVal4 ) / sizeof ( dVal4[0] ) );

	ISphFilter * pFilter4 = sphCreateFilter ( tOpt, tSchema, NULL, NULL, sError, sWarning, SPH_COLLATION_DEFAULT, false );
	ASSERT_TRUE ( pFilter4!=NULL );

	tFilter = sphJoinFilters ( pFilter1, sphJoinFilters ( pFilter2, sphJoinFilters ( pFilter3, pFilter4 ) ) );
	ASSERT_TRUE ( tFilter.Ptr()!=NULL );

	// filter values vs block 5-19
	*DOCINFO2ATTRS ( dMin.Begin() ) = 5;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 19;
	ASSERT_FALSE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );

	// filter values vs block 1-30
	*DOCINFO2ATTRS ( dMin.Begin() ) = 1;
	*DOCINFO2ATTRS ( dMax.Begin() ) = 30;
	ASSERT_TRUE ( tFilter->EvalBlock ( dMin.Begin(), dMax.Begin() ) );
}
