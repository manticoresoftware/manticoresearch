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
#include <unordered_map>
#include <vector>

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

void AppendSectionStringId ( CSphVector<BYTE> & dDst, const std::unordered_map<std::string,DWORD> & hStringIds, const char * sValue )
{
	auto it = hStringIds.find ( sValue );
	ASSERT_NE ( it, hStringIds.end() );
	AppendDword ( dDst, it->second );
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
	struct ExactEntry_t
	{
		const char *				m_sWord;
		std::vector<const char *>	m_dLemmas;
	};
	struct PredictionEntry_t
	{
		const char *	m_sEnding;
		const char *	m_sRequiredPrefix;
		const char *	m_sFixedSuffix;
		const char *	m_sCurrentPrefix;
		const char *	m_sCurrentSuffix;
		const char *	m_sNormalPrefix;
		const char *	m_sNormalSuffix;
		DWORD			m_uCount;
	};

	const std::vector<ExactEntry_t> dExactEntries {
		{ "коти", { "коти", "кіт", "котити" } },
		{ "мрії", { "мрія" } },
		{ "червона", { "червоний" } },
		{ "йшли", { "йти" } },
		{ "ґніт", { "ґніт" } },
		{ "гніт", { "гніт" } },
		{ "гніти", { "гніт" } },
		{ "замки", { "замок", "замок_пристрій" } },
		{ "магазину", { "магазин" } },
		{ "команд", { "команда" } },
		{ "учасниць", { "учасниця" } },
		{ "скажи", { "сказати" } },
		{ "котом", { "кіт" } },
		{ "кітові", { "кіт", "кітовий" } }
	};
	const std::vector<PredictionEntry_t> dPredictionEntries {
		{ "ніти", "", "ніти", "", "ніти", "", "ніти", 128 },
		{ "ніти", "", "ніти", "", "ніти", "", "ніт", 54 }
	};

	std::vector<const char *> dStrings;
	std::unordered_map<std::string,DWORD> hStringIds;
	auto tAddString = [&dStrings, &hStringIds] ( const char * sValue )
	{
		if ( !hStringIds.count ( sValue ) )
		{
			hStringIds [ sValue ] = (DWORD)dStrings.size();
			dStrings.push_back ( sValue );
		}
	};

	for ( const auto & tEntry : dExactEntries )
	{
		tAddString ( tEntry.m_sWord );
		for ( const auto * sLemma : tEntry.m_dLemmas )
			tAddString ( sLemma );
	}

	for ( const auto & tRule : dPredictionEntries )
	{
		tAddString ( tRule.m_sEnding );
		tAddString ( tRule.m_sRequiredPrefix );
		tAddString ( tRule.m_sFixedSuffix );
		tAddString ( tRule.m_sCurrentPrefix );
		tAddString ( tRule.m_sCurrentSuffix );
		tAddString ( tRule.m_sNormalPrefix );
		tAddString ( tRule.m_sNormalSuffix );
	}

	CSphVector<BYTE> dStringsPayload;
	AppendDword ( dStringsPayload, dStrings.size() );
	for ( const auto * sValue : dStrings )
		AppendString ( dStringsPayload, sValue );

	CSphVector<BYTE> dExactPayload;
	AppendDword ( dExactPayload, dExactEntries.size() );
	for ( const auto & tEntry : dExactEntries )
	{
		AppendSectionStringId ( dExactPayload, hStringIds, tEntry.m_sWord );
		AppendDword ( dExactPayload, tEntry.m_dLemmas.size() );
		for ( const auto * sLemma : tEntry.m_dLemmas )
			AppendSectionStringId ( dExactPayload, hStringIds, sLemma );
	}

	CSphVector<BYTE> dPredictionPayload;
	AppendDword ( dPredictionPayload, dPredictionEntries.size() );
	for ( const auto & tRule : dPredictionEntries )
	{
		AppendSectionStringId ( dPredictionPayload, hStringIds, tRule.m_sEnding );
		AppendSectionStringId ( dPredictionPayload, hStringIds, tRule.m_sRequiredPrefix );
		AppendSectionStringId ( dPredictionPayload, hStringIds, tRule.m_sFixedSuffix );
		AppendSectionStringId ( dPredictionPayload, hStringIds, tRule.m_sCurrentPrefix );
		AppendSectionStringId ( dPredictionPayload, hStringIds, tRule.m_sCurrentSuffix );
		AppendSectionStringId ( dPredictionPayload, hStringIds, tRule.m_sNormalPrefix );
		AppendSectionStringId ( dPredictionPayload, hStringIds, tRule.m_sNormalSuffix );
		AppendDword ( dPredictionPayload, tRule.m_uCount );
	}

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
	ASSERT_STREQ ( (char*)sKnown, "коти" );

	StrVec_t dLemmas;
	sphAotLemmatizeUk ( dLemmas, (const BYTE*)"замки", tLemmatizer.get() );
	ASSERT_EQ ( dLemmas.GetLength(), 2 );
	ASSERT_STREQ ( dLemmas[0].cstr(), "замок" );
	ASSERT_STREQ ( dLemmas[1].cstr(), "замок_пристрій" );

	dLemmas.Resize ( 0 );
	sphAotLemmatizeUk ( dLemmas, (const BYTE*)"ґніти", tLemmatizer.get() );
	ASSERT_EQ ( dLemmas.GetLength(), 2 );
	ASSERT_STREQ ( dLemmas[0].cstr(), "ґніти" );
	ASSERT_STREQ ( dLemmas[1].cstr(), "ґніт" );

	BYTE sUnknown[] = "невідоме";
	sphAotLemmatizeUk ( sUnknown, tLemmatizer.get() );
	ASSERT_STREQ ( (char*)sUnknown, "невідоме" );
}

TEST_F ( UkrainianLemmatizerTest_c, NativeMatchesProxyBaselineProbeCases )
{
	CSphString sError;
	ASSERT_TRUE ( sphAotInit ( g_sUkAssetPath, sError, AOT_UK ) ) << sError.cstr();

	auto tLemmatizer = CreateLemmatizer ( AOT_UK );
	ASSERT_TRUE ( (bool)tLemmatizer );
	auto tCheckSingle = [tLemmatizer = tLemmatizer.get()] ( const char * sWord, const char * sExpected )
	{
		BYTE sBuffer [ 128 ] {};
		strncpy ( (char*)sBuffer, sWord, sizeof(sBuffer)-1 );
		sphAotLemmatizeUk ( sBuffer, tLemmatizer );
		ASSERT_STREQ ( (char*)sBuffer, sExpected ) << sWord;
	};

	auto tCheckAll = [tLemmatizer = tLemmatizer.get()] ( const char * sWord, std::initializer_list<const char *> dExpected )
	{
		StrVec_t dLemmas;
		sphAotLemmatizeUk ( dLemmas, (const BYTE*)sWord, tLemmatizer );
		ASSERT_EQ ( dLemmas.GetLength(), dExpected.size() ) << sWord;
		int i = 0;
		for ( const auto * sExpected : dExpected )
			ASSERT_STREQ ( dLemmas[i++].cstr(), sExpected ) << sWord;
	};

	tCheckSingle ( "мрії", "мрія" );
	tCheckSingle ( "червона", "червоний" );
	tCheckSingle ( "йшли", "йти" );
	tCheckSingle ( "ґніт", "ґніт" );
	tCheckSingle ( "гніт", "гніт" );
	tCheckSingle ( "інтернет-магазину", "інтернет-магазин" );
	tCheckSingle ( "команд-учасниць", "команда-учасниця" );
	tCheckSingle ( "по-західному", "по-західному" );
	tCheckSingle ( "скажи-но", "сказати-но" );
	tCheckSingle ( "псевдокотом", "псевдокіт" );
	tCheckSingle ( "мегакітові", "мегакіт" );
	tCheckSingle ( "мегакотом", "мегакіт" );

	tCheckAll ( "коти", { "коти", "кіт", "котити" } );
	tCheckAll ( "гніт", { "гніт", "ґніт" } );
	tCheckAll ( "ґніти", { "ґніти", "ґніт" } );
	tCheckAll ( "гніти", { "гніт" } );
	tCheckAll ( "команд-учасниць", { "команда-учасниця", "команд-учасниця" } );
	tCheckAll ( "мегакітові", { "мегакіт", "мегакітовий" } );
}

} // namespace
