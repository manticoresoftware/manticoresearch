// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "postings/blockqword.h"
#include "postings/disk.h"
#include "std/bitmap/bitmap.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

static unsigned g_uChecks = 0;

static void Check ( bool bCondition )
{
	++g_uChecks;
	if ( !bCondition )
	{
		std::cerr << "failed check " << g_uChecks << '\n';
		std::exit(1);
	}
}


class MemoryOutput_c final : public postings::Output_i
{
public:
	std::vector<uint8_t>	m_dData = std::vector<uint8_t>(postings::FILE_HEADER_SIZE);

	uint64_t	GetPos() const final	{ return m_dData.size(); }

	bool		Write ( postings::Bytes_t tData, std::string & ) final
	{
		m_dData.insert ( m_dData.end(), tData.m_pData, tData.m_pData+tData.m_iSize );
		return true;
	}


	bool		Patch ( uint64_t uOffset, postings::Bytes_t tData, std::string & ) final
	{
		Check ( uOffset+tData.m_iSize<=m_dData.size() );
		std::copy_n ( tData.m_pData, tData.m_iSize, m_dData.data()+uOffset );
		return true;
	}
};


class CountingQword_c final : public BlockIndexQword_c
{
public:
	CountingQword_c() : BlockIndexQword_c ( false, false, 0 ) {}

	unsigned	m_uCalls = 0;

	const CSphMatch &	GetNextDoc() final
	{
		++m_uCalls;
		return BlockIndexQword_c::GetNextDoc();
	}
};


static void CheckState ( const CountingQword_c & a, const CountingQword_c & b )
{
	Check ( a.m_tDoc.m_tRowID==b.m_tDoc.m_tRowID );
	Check ( a.m_uMatchHits==b.m_uMatchHits && a.m_iHitlistPos==b.m_iHitlistPos && a.m_uHitPosition==b.m_uHitPosition );
	for ( unsigned f=0; f<postings::FIELD_WORDS; ++f )
		Check ( a.m_dQwordFields[f]==b.m_dQwordFields[f] );

	Check ( !a.GetReadError() && !b.GetReadError() );
}


static void Test ( const char * sFile, unsigned uCount, bool bMembership, FileAccess_e eAccess )
{
	MemoryOutput_c tOutput;
	auto pWriter = std::make_unique<postings::Writer_c>(tOutput);
	postings::TermDesc_t tDesc;
	tDesc.m_uFields = 256;
	std::string sError;
	Check ( pWriter->Begin ( tDesc, sError ) );
	for ( unsigned i=0; i<uCount; ++i )
	{
		postings::Posting_t t;
		t.m_uRow = i*3;
		t.m_uTF = i%7+1;
		t.m_uHitRef = (uint64_t(1)<<34)+i*29;
		t.m_dFields[i%8] = 1U<<(i%32);
		t.m_dFieldTF[(i%8)*32+i%32] = t.m_uTF;
		Check ( pWriter->Add ( t, sError ) );
	}

	Check ( pWriter->Finish(sError) );
	{
		std::ofstream tFile ( sFile, std::ios::binary | std::ios::trunc );
		tFile.write ( reinterpret_cast<const char *>(tOutput.m_dData.data()), tOutput.m_dData.size() );
		Check ( bool(tFile) );
	}

	CSphString sReadError;
	DataReaderFactoryPtr_c pFactory ( NewProxyReader ( sFile, sReadError, DataReaderFactory_c::DOCS, 1024, eAccess ) );
	Check ( bool(pFactory) );
	for ( int iBatch : { 1, 31, 128, 255, 256, 257, 600, 1024 } )
	{
		CountingQword_c a, b;
		a.SetDocReader(pFactory);
		b.SetDocReader(pFactory);
		if ( bMembership )
		{
			a.SetMembershipOnly();
			b.SetMembershipOnly();
		}

		for ( bool bHints : { false, true } )
		{
			a.Reset();
			b.Reset();
			a.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
			b.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
			std::array<QwordDoc_t,1024> dA, dB;
			Check ( a.GetNextDocs ( nullptr, 0 )==0 );
			unsigned uSeen = 0;
			for ( unsigned turn=0; ; ++turn )
			{
				if ( bHints && turn%2==0 )
				{
					RowID_t tHint = turn ? a.m_tDoc.m_tRowID+17 : 751;
					a.HintRowID(tHint);
					b.HintRowID(tHint);
				}

				unsigned uCalls = a.m_uCalls;
				int iA = a.GetNextDocs ( dA.data(), iBatch );
				int iB = b.ISphQword::GetNextDocs ( dB.data(), iBatch );
				Check ( a.m_uCalls==uCalls );
				Check ( iA==iB );
				for ( int i=0; i<iA; ++i )
				{
					Check ( dA[i].m_tRowID==dB[i].m_tRowID && dA[i].m_uFields==dB[i].m_uFields );
					Check ( dA[i].m_uMatchHits==dB[i].m_uMatchHits && dA[i].m_iHitlistPos==dB[i].m_iHitlistPos );
					if ( !bHints )
						Check ( dA[i].m_tRowID==uSeen++*3 );
				}

				CheckState ( a, b );
				if ( iA<iBatch )
					break;

				if ( bHints )
				{
					Check ( a.AdvanceTo(a.m_tDoc.m_tRowID)==b.AdvanceTo(b.m_tDoc.m_tRowID) );
					Check ( a.GetNextDoc().m_tRowID==b.GetNextDoc().m_tRowID );
					CheckState ( a, b );
				}
			}

			if ( !bHints )
				Check ( uSeen==uCount );

			Check ( a.GetNextDocs ( dA.data(), iBatch )==0 );
			Check ( a.GetNextDoc().m_tRowID==INVALID_ROWID );
		}
	}
}


static void TestWindows ( const char * sFile, unsigned uShape, unsigned uFieldCount, DWORD uMask, RowID_t tBase, FileAccess_e eAccess, bool bUniform=false )
{
	MemoryOutput_c tOutput;
	auto pWriter = std::make_unique<postings::Writer_c>(tOutput);
	postings::TermDesc_t tDesc;
	tDesc.m_uFields = uint16_t(uFieldCount);
	std::string sError;
	Check ( pWriter->Begin ( tDesc, sError ) );
	constexpr unsigned COUNT = 2051;
	for ( unsigned i=0; i<COUNT; ++i )
	{
		postings::Posting_t t;
		t.m_uRow = tBase+(uShape==0 ? i : uShape==1 ? i*3/2 : i*97);
		t.m_uTF = 1;
		t.m_uHitRef = 8+i;
		unsigned f = bUniform ? 1 : i%uFieldCount;
		t.m_dFields[f/32] = 1U<<(f%32);
		t.m_dFieldTF[f] = 1;
		Check ( pWriter->Add ( t, sError ) );
	}

	Check ( pWriter->Finish(sError) );
	{
		std::ofstream tFile ( sFile, std::ios::binary | std::ios::trunc );
		tFile.write ( reinterpret_cast<const char *>(tOutput.m_dData.data()), tOutput.m_dData.size() );
		Check ( bool(tFile) );
	}

	// Each fixture must actually exercise its intended persisted container.
	Check ( tOutput.m_dData[postings::FILE_HEADER_SIZE+postings::TERM_HEADER_SIZE+postings::LEAF_HEADER_SIZE+5]==unsigned(uShape==0 ? postings::Rows_e::RANGE : uShape==1 ? postings::Rows_e::BITMAP : postings::Rows_e::DELTA) );
	CSphString sReadError;
	DataReaderFactoryPtr_c pFactory ( NewProxyReader ( sFile, sReadError, DataReaderFactory_c::DOCS, 1024, eAccess ) );
	Check ( bool(pFactory) );
	CountingQword_c a, b;
	a.SetDocReader(pFactory);
	b.SetDocReader(pFactory);
	a.m_iDocs = b.m_iDocs = COUNT;
	auto pNative = a.CreatePostingWindow ( uMask, {} );
	auto pBuffered = CreateBufferedPostingWindow ( b, uMask, {} );
	for ( unsigned mode=0; mode<3; ++mode )
	{
		a.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
		b.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
		pNative->Reset ( true, {} );
		pBuffered->Reset ( true, {} );
		RowID_t tNext = 0;
		for ( unsigned turn=0; ; ++turn )
		{
			Check ( turn<10000 );
			auto na = pNative->AdvanceTo(tNext), nb = pBuffered->AdvanceTo(tNext);
			Check ( na.m_tNext==nb.m_tNext && !na.m_bStopped && !nb.m_bStopped );
			if ( na.m_tNext==INVALID_ROWID )
				break;

			RowID_t tMin = na.m_tNext;
			if ( mode==2 )
			{
				auto pa = pNative->Probe(tMin), pb = pBuffered->Probe(tMin);
				Check ( pa.m_bMatch==pb.m_bMatch && pa.m_tState.m_tNext==pb.m_tState.m_tNext );
			}

			PostingWindow_t tWindow { tMin>17 ? tMin-17 : 0, tMin, std::min ( uint64_t(INVALID_ROWID)+1, uint64_t(tMin)+1+turn%4*997 ) };
			DWORD dA[PostingWindow_t::WORDS], dB[PostingWindow_t::WORDS];
			std::fill_n ( dA, PostingWindow_t::WORDS, mode==1 ? 0x55555555 : 0x80000001 );
			std::copy_n ( dA, PostingWindow_t::WORDS, dB );
			if ( mode==0 )
			{
				na = pNative->FillOr ( tWindow, dA );
				nb = pBuffered->FillOr ( tWindow, dB );
			} else if ( mode==1 )
			{
				na = pNative->FillAnd ( tWindow, dA );
				nb = pBuffered->FillAnd ( tWindow, dB );
			} else
			{
				RowID_t dRowsA[128], dRowsB[128];
				int iCapacity = 1+turn%128;
				na = pNative->ReadRows ( tWindow, dRowsA, iCapacity );
				nb = pBuffered->ReadRows ( tWindow, dRowsB, iCapacity );
				Check ( na.m_iRows==nb.m_iRows && std::equal ( dRowsA, dRowsA+na.m_iRows, dRowsB ) );
			}

			Check ( na.m_tNext==nb.m_tNext && na.m_iRows==nb.m_iRows && !na.m_bStopped );
			Check ( std::equal ( dA, dA+PostingWindow_t::WORDS, dB ) );
			if ( na.m_tNext==INVALID_ROWID )
				break;

			tNext = na.m_tNext;
			if ( mode==2 && turn%5==0 )
				tNext = RowID_t(std::min ( uint64_t(INVALID_ROWID), uint64_t(tNext)+113 ));
		}

		Check ( !a.GetReadError() && a.m_uCalls==0 );
		const auto & dCounts = a.GetDecodeCounts();
		Check ( !dCounts[2] && !dCounts[3] && !dCounts[4] );
		if ( bUniform && uShape!=2 )
			Check ( !dCounts[0] && dCounts[1]>0 );

		if ( uFieldCount==1 && uMask==UINT32_MAX )
			Check ( !dCounts[1] && (uShape==2 || !dCounts[0]) );
	}

	// Compare fused AND with the former FillOr+AND implementation using identical
	// native cursors and continuation budgets, including partially consumed leaves.
	pBuffered.reset();
	auto pReference = b.CreatePostingWindow ( uMask, {} );
	for ( unsigned iTry=0; iTry<32; ++iTry )
	{
		struct Continue_t { int m_iCalls = 0; int m_iLimit; };
		Continue_t tA { 0, int(iTry%8)-1 }, tB { 0, int(iTry%8)-1 };
		auto fnContinue = [] ( void * p ) { auto & t = *static_cast<Continue_t *>(p); return t.m_iLimit<0 || ++t.m_iCalls<=t.m_iLimit; };
		a.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
		b.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
		pNative->Reset ( true, { &tA, fnContinue, nullptr } );
		pReference->Reset ( true, { &tB, fnContinue, nullptr } );
		RowID_t tMin = tBase+(iTry/8)*17;
		PostingWindow_t tRange { tBase-7, tMin, std::min ( uint64_t(INVALID_ROWID)+1, uint64_t(tBase)+4077 ) };
		DWORD dA[PostingWindow_t::WORDS], dB[PostingWindow_t::WORDS], dScratch[PostingWindow_t::WORDS] = {};
		for ( unsigned i=0; i<PostingWindow_t::WORDS; ++i )
			dA[i] = dB[i] = iTry/8==0 ? 0 : iTry/8==1 ? UINT32_MAX : (i*1664525U+1013904223U) ^ (iTry*2654435761U);

		auto na = pNative->FillAnd ( tRange, dA );
		auto nb = pReference->FillOr ( tRange, dScratch );
		size_t iWords = size_t((tRange.m_uEnd-tRange.m_tBase+31)/32);
		bitmap::And ( dB, dB, dScratch, iWords );
		Check ( na.m_iRows==int(bitmap::Count ( dB, iWords )) );
		Check ( na.m_bStopped==nb.m_bStopped && na.m_tNext==nb.m_tNext );
		Check ( std::equal ( dA, dA+PostingWindow_t::WORDS, dB ) );
		Check ( tA.m_iCalls==tB.m_iCalls );
		if ( !na.m_bStopped )
		{
			RowID_t dRowsA[128], dRowsB[128];
			na = pNative->ReadRows ( tRange, dRowsA, 128 );
			nb = pReference->ReadRows ( tRange, dRowsB, 128 );
			Check ( na.m_iRows==nb.m_iRows && na.m_tNext==nb.m_tNext );
			Check ( std::equal ( dRowsA, dRowsA+na.m_iRows, dRowsB ) );
		}
	}

	a.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
	int iChecks = 0;
	PostingWindowContext_t tContext { &iChecks, [] ( void * p ) { return ++*static_cast<int *>(p)<=2; }, nullptr };
	pNative->Reset ( true, tContext );
	DWORD dBits[PostingWindow_t::WORDS] = {};
	PostingWindow_t tWindow { tBase, tBase, std::min ( uint64_t(INVALID_ROWID)+1, uint64_t(tBase)+4096 ) };
	auto tResult = pNative->FillOr ( tWindow, dBits );
	Check ( tResult.m_bStopped || uShape==2 );
	Check ( tResult.m_iRows==int(bitmap::Count ( dBits, PostingWindow_t::WORDS )) );
	Check ( !tResult.m_bStopped || pNative->AdvanceTo(tBase).m_bStopped );
	a.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
	iChecks = 0;
	pNative->Reset ( true, tContext );
	Check ( pNative->AdvanceTo(tBase+(uShape==2 ? 100000 : 1024)).m_bStopped );
	Check ( !a.GetDecodeCounts()[0] && !a.GetDecodeCounts()[1] );

	pNative->Reset ( false, {} );
	Check ( pNative->AdvanceTo(0).m_tNext==INVALID_ROWID );
}


int main ( int argc, char ** argv )
{
	if ( argc!=2 )
		return 2;

	for ( unsigned n : { 1u, 127u, 128u, 255u, 256u, 257u, 513u, 1025u } )
		for ( bool bMembership : { false, true } )
			for ( auto eAccess : { FileAccess_e::MMAP, FileAccess_e::FILE } )
				Test ( argv[1], n, bMembership, eAccess );

	for ( unsigned shape=0; shape<3; ++shape )
		for ( unsigned fields : { 1u, 2u, 33u } )
			for ( DWORD mask : { 0u, 1u, UINT32_MAX } )
				for ( RowID_t base : { 4093u, UINT32_MAX-(shape==0 ? 2050u : shape==1 ? 3075u : 198850u)-1 } )
					for ( auto access : { FileAccess_e::MMAP, FileAccess_e::FILE } )
						TestWindows ( argv[1], shape, fields, mask, base, access );

	for ( unsigned shape=0; shape<3; ++shape )
		TestWindows ( argv[1], shape, 33, 2, 4093, FileAccess_e::FILE, true );

	Check ( !std::remove(argv[1]) );
	std::cout << g_uChecks << " qword batch checks passed\n";
}
