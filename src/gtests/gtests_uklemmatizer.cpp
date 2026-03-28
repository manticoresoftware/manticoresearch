//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>
#include "lz4/lz4.h"

#include "sphinxint.h"
#include "dict/stem/sphinxstem.h"
#include "dict/dict_base.h"

namespace {

constexpr char g_sUkAssetPath[] = "__uklemmatizer_test.pak";
constexpr char g_sUkAssetMagic[] = "MSUKLEM1";
constexpr DWORD g_uUkAssetVersion = 3;
constexpr DWORD g_uUkSectionStrings = 1;
constexpr DWORD g_uUkSectionExact = 2;
constexpr DWORD g_uUkSectionPredictions = 3;
constexpr DWORD g_uUkCodecNone = 0;
constexpr DWORD g_uUkCodecLz4 = 1;

void WriteDword ( FILE * fp, DWORD uValue )
{
	ASSERT_EQ ( fwrite ( &uValue, sizeof(uValue), 1, fp ), 1U );
}

void WriteQword ( FILE * fp, uint64_t uValue )
{
	ASSERT_EQ ( fwrite ( &uValue, sizeof(uValue), 1, fp ), 1U );
}

void AppendDword ( CSphVector<BYTE> & dDst, DWORD uValue )
{
	BYTE * pDst = dDst.AddN ( sizeof(uValue) );
	memcpy ( pDst, &uValue, sizeof(uValue) );
}

void AppendString ( CSphVector<BYTE> & dDst, const char * sValue )
{
	const auto iLen = (DWORD)strlen ( sValue );
	AppendDword ( dDst, iLen );
	BYTE * pDst = dDst.AddN ( iLen );
	memcpy ( pDst, sValue, iLen );
}

CSphVector<BYTE> CompressSection ( const CSphVector<BYTE> & dSrc, DWORD & uCodec )
{
	CSphVector<BYTE> dCompressed;
	dCompressed.Resize ( LZ4_compressBound ( dSrc.GetLength() ) );
	int iCompressed = LZ4_compress_default ( (const char*)dSrc.Begin(), (char*)dCompressed.Begin(), dSrc.GetLength(), dCompressed.GetLength() );
	if ( iCompressed>0 && iCompressed<dSrc.GetLength() )
	{
		dCompressed.Resize ( iCompressed );
		uCodec = g_uUkCodecLz4;
		return dCompressed;
	}

	uCodec = g_uUkCodecNone;
	return dSrc;
}

void WriteUkAsset ()
{
	CSphVector<const char *> dStrings;
	dStrings.Add ( "коти" );
	dStrings.Add ( "кіт" );
	dStrings.Add ( "замки" );
	dStrings.Add ( "замок" );
	dStrings.Add ( "замок_пристрій" );
	dStrings.Add ( "йшли" );
	dStrings.Add ( "йти" );
	dStrings.Add ( "ти" );
	dStrings.Add ( "" );
	dStrings.Add ( "т" );

	CSphVector<BYTE> dStringsPayload;
	AppendDword ( dStringsPayload, dStrings.GetLength() );
	for ( const auto * sValue : dStrings )
		AppendString ( dStringsPayload, sValue );

	CSphVector<BYTE> dExactPayload;
	AppendDword ( dExactPayload, 3 );
	AppendDword ( dExactPayload, 0 );
	AppendDword ( dExactPayload, 1 );
	AppendDword ( dExactPayload, 1 );

	AppendDword ( dExactPayload, 2 );
	AppendDword ( dExactPayload, 2 );
	AppendDword ( dExactPayload, 3 );
	AppendDword ( dExactPayload, 4 );

	AppendDword ( dExactPayload, 5 );
	AppendDword ( dExactPayload, 1 );
	AppendDword ( dExactPayload, 6 );

	CSphVector<BYTE> dPredictionPayload;
	AppendDword ( dPredictionPayload, 2 );
	AppendDword ( dPredictionPayload, 7 );
	AppendDword ( dPredictionPayload, 8 );
	AppendDword ( dPredictionPayload, 7 );
	AppendDword ( dPredictionPayload, 8 );
	AppendDword ( dPredictionPayload, 7 );
	AppendDword ( dPredictionPayload, 8 );
	AppendDword ( dPredictionPayload, 9 );
	AppendDword ( dPredictionPayload, 10 );

	AppendDword ( dPredictionPayload, 7 );
	AppendDword ( dPredictionPayload, 8 );
	AppendDword ( dPredictionPayload, 7 );
	AppendDword ( dPredictionPayload, 8 );
	AppendDword ( dPredictionPayload, 7 );
	AppendDword ( dPredictionPayload, 8 );
	AppendDword ( dPredictionPayload, 7 );
	AppendDword ( dPredictionPayload, 5 );

	DWORD uStringsCodec = 0;
	DWORD uExactCodec = 0;
	DWORD uPredictionCodec = 0;
	CSphVector<BYTE> dStringsSection = CompressSection ( dStringsPayload, uStringsCodec );
	CSphVector<BYTE> dExactSection = CompressSection ( dExactPayload, uExactCodec );
	CSphVector<BYTE> dPredictionSection = CompressSection ( dPredictionPayload, uPredictionCodec );

	FILE * fp = fopen ( g_sUkAssetPath, "wb" );
	ASSERT_NE ( fp, nullptr );

	ASSERT_EQ ( fwrite ( g_sUkAssetMagic, 1, sizeof(g_sUkAssetMagic)-1, fp ), sizeof(g_sUkAssetMagic)-1 );
	WriteDword ( fp, g_uUkAssetVersion );
	WriteDword ( fp, 3 );
	WriteDword ( fp, 4 );
	WriteDword ( fp, 5 );
	WriteDword ( fp, 0 );

	uint64_t uOffset = ( sizeof(g_sUkAssetMagic)-1 ) + sizeof(DWORD)*4 + sizeof(DWORD);
	uOffset += 3 * ( sizeof(DWORD)*4 + sizeof(uint64_t)*3 );

	WriteDword ( fp, g_uUkSectionStrings );
	WriteDword ( fp, uStringsCodec );
	WriteQword ( fp, uOffset );
	WriteQword ( fp, dStringsSection.GetLength() );
	WriteQword ( fp, dStringsPayload.GetLength() );
	WriteDword ( fp, 0 );
	WriteDword ( fp, 0 );
	uOffset += dStringsSection.GetLength();

	WriteDword ( fp, g_uUkSectionExact );
	WriteDword ( fp, uExactCodec );
	WriteQword ( fp, uOffset );
	WriteQword ( fp, dExactSection.GetLength() );
	WriteQword ( fp, dExactPayload.GetLength() );
	WriteDword ( fp, 0 );
	WriteDword ( fp, 0 );
	uOffset += dExactSection.GetLength();

	WriteDword ( fp, g_uUkSectionPredictions );
	WriteDword ( fp, uPredictionCodec );
	WriteQword ( fp, uOffset );
	WriteQword ( fp, dPredictionSection.GetLength() );
	WriteQword ( fp, dPredictionPayload.GetLength() );
	WriteDword ( fp, 0 );
	WriteDword ( fp, 0 );

	ASSERT_EQ ( fwrite ( dStringsSection.Begin(), 1, dStringsSection.GetLength(), fp ), (size_t)dStringsSection.GetLength() );
	ASSERT_EQ ( fwrite ( dExactSection.Begin(), 1, dExactSection.GetLength(), fp ), (size_t)dExactSection.GetLength() );
	ASSERT_EQ ( fwrite ( dPredictionSection.Begin(), 1, dPredictionSection.GetLength(), fp ), (size_t)dPredictionSection.GetLength() );

	fclose ( fp );
}

class UkrainianLemmatizerTest_c : public ::testing::Test
{
protected:
	CSphString m_sPrevBase;

	void SetUp() override
	{
		sphAotShutdown();
		m_sPrevBase = g_sLemmatizerBase;
		WriteUkAsset();
		g_sLemmatizerBase = ".";
	}

	void TearDown() override
	{
		sphAotShutdown();
		g_sLemmatizerBase = m_sPrevBase;
		unlink ( g_sUkAssetPath );
	}
};

TEST_F ( UkrainianLemmatizerTest_c, NativeSingleAndAllForms )
{
	CSphString sError;
	ASSERT_TRUE ( sphAotInit ( g_sUkAssetPath, sError, AOT_UK ) ) << sError.cstr();

	auto tLemmatizer = CreateLemmatizer ( AOT_UK );
	ASSERT_TRUE ( (bool)tLemmatizer );

	BYTE sKnown[] = "коти";
	sphAotLemmatizeUk ( sKnown, tLemmatizer.get() );
	ASSERT_STREQ ( (char*)sKnown, "кіт" );

	StrVec_t dLemmas;
	sphAotLemmatizeUk ( dLemmas, (const BYTE*)"замки", tLemmatizer.get() );
	ASSERT_EQ ( dLemmas.GetLength(), 2 );
	ASSERT_STREQ ( dLemmas[0].cstr(), "замок" );
	ASSERT_STREQ ( dLemmas[1].cstr(), "замок_пристрій" );

	dLemmas.Resize ( 0 );
	sphAotLemmatizeUk ( dLemmas, (const BYTE*)"ґніти", tLemmatizer.get() );
	ASSERT_EQ ( dLemmas.GetLength(), 2 );
	ASSERT_STREQ ( dLemmas[0].cstr(), "ґніт" );
	ASSERT_STREQ ( dLemmas[1].cstr(), "ґніти" );

	BYTE sUnknown[] = "невідоме";
	sphAotLemmatizeUk ( sUnknown, tLemmatizer.get() );
	ASSERT_STREQ ( (char*)sUnknown, "невідоме" );
}

} // namespace
