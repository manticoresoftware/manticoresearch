#include "gtest/gtest.h"

#include "dict/infix/infix_builder.h"
#include "sphinxstd.h"

#include <cstring>

namespace
{

int CountInfixes ( const char* szWord, int iCodepointBytes, int iMinInfixLen )
{
	CSphString sError;
	auto pBuilder = sphCreateInfixBuilder ( iCodepointBytes, iMinInfixLen, &sError );
	EXPECT_TRUE ( sError.IsEmpty() );
	pBuilder->AddWord ( (const BYTE*)szWord, (int)strlen(szWord), 1, false );
	return pBuilder->GetEntriesCount();
}

}


TEST ( InfixBuilder, HonorsMinInfixLengthSingleByte )
{
	EXPECT_EQ ( CountInfixes ( "abcdef", 1, 2 ), 15 );
	EXPECT_EQ ( CountInfixes ( "abcdef", 1, 3 ), 10 );
	EXPECT_EQ ( CountInfixes ( "abcdef", 1, 4 ), 6 );
	EXPECT_EQ ( CountInfixes ( "abcdef", 1, 6 ), 1 );
}


TEST ( InfixBuilder, HonorsMinInfixLengthUtf8 )
{
	EXPECT_EQ ( CountInfixes ( "äöüß", 2, 2 ), 6 );
	EXPECT_EQ ( CountInfixes ( "äöüß", 2, 3 ), 3 );
	EXPECT_EQ ( CountInfixes ( "äöüß", 2, 4 ), 1 );
}