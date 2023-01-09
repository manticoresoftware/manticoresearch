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

#include <benchmark/benchmark.h>

#include "sphinxint.h"
#include "sphinxutils.h"
#include "sphinxstem.h"


class BM_parse_expr : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		CSphColumnInfo tCol;

		tCol.m_sName = "id";
		tCol.m_eAttrType = SPH_ATTR_BIGINT;
		tSchema.AddAttr ( tCol, false );

		tCol.m_sName = "aaa";
		tCol.m_eAttrType = SPH_ATTR_INTEGER;
		tSchema.AddAttr ( tCol, false );

		tCol.m_sName = "bbb";
		tCol.m_eAttrType = SPH_ATTR_INTEGER;
		tSchema.AddAttr ( tCol, false );

		tCol.m_sName = "ccc";
		tCol.m_eAttrType = SPH_ATTR_INTEGER;
		tSchema.AddAttr ( tCol, false );

		pRow = new CSphRowitem[tSchema.GetRowSize ()];
		for ( int i = 1; i<tSchema.GetAttrsCount (); i++ )
			sphSetRowAttr ( pRow, tSchema.GetAttr ( i ).m_tLocator, i );

		sphSetRowAttr ( pRow, tSchema.GetAttr ( 0 ).m_tLocator, 123 );

		tMatch.m_tRowID = 123;
		tMatch.m_iWeight = 456;
		tMatch.m_pStatic = pRow;

		const char* ppTests[] =
		{
			"ccc/2+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "1*2*3*4*5*6*7*8*9*10+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "aaa+bbb*sin(0)*ccc+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "if(pow(sqrt(2),2)=2,123,456)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "if(2<2,3,4)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "if(2>=2,3,4)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "pow(7,5)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "sqrt(3)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "log2((2+2)*(2+2))+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "min(3,15)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "max(3,15)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "if(3<15,bbb,ccc)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40"
			, "id+@weight+if(3<15,bbb,ccc)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa"
			, "abs(-3-ccc)+id+@weight+if(3<15,bbb,ccc)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa"
			, "(aaa+bbb)*(ccc-aaa)+abs(-3-ccc)+id+@weight+if(3<15,bbb,ccc)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()"
			, "(((aaa)))+(aaa+bbb)*(ccc-aaa)+abs(-3-ccc)+id+@weight+if(3<15,bbb,ccc)+aaa+bbb+rand()+ccc+id+12*1562 mod 40+aaa+bbb+rand()"
			, "aaa-bbb*ccc+(((aaa)))+(aaa+bbb)*(ccc-aaa)+abs(-3-ccc)+id+@weight+if(3<15,bbb,ccc)+aaa+bbb+rand()+ccc+id+12*1562"
			, " aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc+(((aaa)))+(aaa+bbb)*(ccc-aaa)+abs(-3-ccc)+id+@weight+if(3<15,bbb,ccc)+aaa+bbb"
			, "bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc+(((aaa)))+(aaa+bbb)*(ccc-aaa)+abs(-3-ccc)+id+@weight"
			, "2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc+(((aaa)))+(aaa+bbb)*(ccc-aaa)+abs(-3-ccc)+id+@weight"
			, "3<5+2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc+(((aaa)))+(aaa+bbb)*(ccc-aaa)+abs(-3-ccc)+id+@weight"
			, "1 + 2*3 > 4*4+3<5+2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc+(((aaa)))+(aaa+bbb)*(ccc-aaa)"
			, "aaa/-bbb+1 + 2*3 > 4*4+3<5+2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc+(((aaa)))+(aaa+bbb)*(ccc-aaa)"
			, "-10*-10+aaa/-bbb+1 + 2*3 > 4*4+3<5+2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc"
			, "aaa+-bbb*-5+-10*-10+aaa/-bbb+1 + 2*3 > 4*4+3<5+2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc + aaa-bbb*ccc"
			, "-aaa>-bbb+aaa+-bbb*-5+-10*-10+aaa/-bbb+1 + 2*3 > 4*4+3<5+2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc"
			, "1-aaa+2-3+4-aaa>-bbb+aaa+-bbb*-5+-10*-10+aaa/-bbb+1 + 2*3 > 4*4+3<5+2.000*2e+1+2+bbb+123*aaa+ aaa    -\tbbb *\t\t\tccc"
			, "bbb/1*2/6*3+1-aaa+2-3+4-aaa>-bbb+aaa+-bbb*-5+-10*-10+aaa/-bbb+1 + 2*3 > 4*4+3<5+2.000*2e+1+2+bbb+123*aaa+ aaa   "
			, "(aaa+bbb)/sqrt(3)/sqrt(3)+bbb/1*2/6*3+1-aaa+2-3+4-aaa>-bbb+aaa+-bbb*-5+-10*-10+aaa/-bbb+1 + 2*3 > 4*4+3<5+2.000"
			, "aaa-bbb-2+(aaa+bbb)/sqrt(3)/sqrt(3)+bbb/1*2/6*3+1-aaa+2-3+4-aaa>-bbb+aaa+-bbb*-5+-10*-10+aaa/-bbb+1 + 2*3"
			, "ccc mod 10+aaa-bbb-2+(aaa+bbb)/sqrt(3)/sqrt(3)+bbb/1*2/6*3+1-aaa+2-3+4-aaa>-bbb+aaa+-bbb*-5+-10*-10+aaa/-bbb+1 + 2*3"
			, "ccc/2*4/bbb+ccc mod 10+aaa-bbb-2+(aaa+bbb)/sqrt(3)/sqrt(3)+bbb/1*2/6*3+1-aaa+2-3+4-aaa>-bbb+aaa+-bbb*-5"
			, "(2+(aaa*bbb))+3+ccc/2*4/bbb+ccc mod 10+aaa-bbb-2+(aaa+bbb)/sqrt(3)/sqrt(3)+bbb/1*2/6*3+1-aaa+2-3+4-aaa>-bbb+aaa+-bbb*-5"
			, "aaa+bbb*(ccc)-1+(2+(aaa*bbb))+3+ccc/2*4/bbb+ccc mod 10+aaa-bbb-2+(aaa+bbb)/sqrt(3)/sqrt(3)+bbb/1*2/6*3+1"
			, "aaa+bbb*ccc*2-3/4*5/6*bbb+aaa+bbb*(ccc)-1+(2+(aaa*bbb))+3+ccc/2*4/bbb+ccc mod 10+aaa-bbb-2+(aaa+bbb)/sqrt(3)/sqrt(3)"
			, "sqrt(2)+aaa+bbb*ccc*2-3/4*5/6*bbb+aaa+bbb*(ccc)-1+(2+(aaa*bbb))+3+ccc/2*4/bbb+ccc mod 10+aaa-bbb-2"
			, "rand()+sqrt(2)+aaa+bbb*ccc*2-3/4*5/6*bbb+aaa+bbb*(ccc)-1+(2+(aaa*bbb))+3+ccc/2*4/bbb+ccc mod 10+aaa-bbb-2"
			, "rand()+sqrt(2)+aaa+bbb*ccc*2-3/4*5/6*bbb+aaa+bbb*(ccc)-1+(2+(aaa*bbb))+3+ccc/2*4/bbb+ccc mod 10+aaa-bbb-2"
			, "FIBONACCI(4)+rand()+sqrt(2)+aaa+bbb*ccc*2-3/4*5/6*bbb+aaa+bbb*(ccc)-1+(2+(aaa*bbb))+3+ccc/2*4/bbb+ccc mod 10+aaa-bbb-2"
	};

		for ( const auto * szTest :  ppTests )
			dTests.Add ( {szTest, strlen(szTest)} );

		i = 0;
		iBytes = 0;
	}

	void TearDown ( const ::benchmark::State & state )
	{
		SafeDeleteArray ( pRow );
	}

	CSphSchema tSchema;
	CSphMatch tMatch;
	CSphRowitem * pRow;
	CSphVector <std::pair<CSphString,int>> dTests;
	CSphString sError;
	ExprParseArgs_t tExprArgs;
	int i = 0;
	int64_t iBytes = 0;
};

BENCHMARK_F( BM_parse_expr, parser ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		ISphExprRefPtr_c pExpr ( sphExprParse ( dTests[i].first.cstr (), tSchema, sError, tExprArgs ) );
		iBytes += dTests[i].second;
		if ( ++i >= dTests.GetLength() )
			i = 0;
	}
	st.SetBytesProcessed ( iBytes );
	st.SetItemsProcessed ( st.iterations() );
}
