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

#define Check(...) CheckAt ( (__VA_ARGS__), __LINE__ )

static void CheckAt ( bool bCondition, int iLine )
{
	++g_uChecks;
	if ( !bCondition )
	{
		std::cerr << "failed check " << g_uChecks << " at line " << iLine << '\n';
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


static void Test ( const char * sFile, unsigned uCount, int iPayload, FileAccess_e eAccess )
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
		a.SetPayloadRequirements ( iPayload!=0, iPayload==2 );
		b.SetPayloadRequirements ( iPayload!=0, iPayload==2 );

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

					unsigned uOrdinal = dA[i].m_tRowID/3;
					Check ( dA[i].m_uMatchHits==(iPayload ? uOrdinal%7+1 : 0) );
					Check ( dA[i].m_iHitlistPos==(iPayload==2 ? (uint64_t(1)<<34)+uOrdinal*29 : 0) );
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

			const auto & dDecodes = a.GetDecodeCounts();
			if ( !bHints )
			{
				Check ( bool(dDecodes[unsigned(postings::Column_e::TF)])==bool(iPayload) );
				Check ( bool(dDecodes[unsigned(postings::Column_e::HITREFS)])==(iPayload==2) );
			}
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
	for ( unsigned mode=0; mode<4; ++mode )
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
			} else if ( mode==3 )
			{
				na = pNative->FillAndNot ( tWindow, dA );
				nb = pBuffered->FillAndNot ( tWindow, dB );
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
	for ( unsigned iTry=0; iTry<64; ++iTry )
	{
		struct Continue_t { int m_iCalls = 0; int m_iLimit; };
		Continue_t tA { 0, int(iTry%8)-1 }, tB { 0, int(iTry%8)-1 };
		auto fnContinue = [] ( void * p ) { auto & t = *static_cast<Continue_t *>(p); return t.m_iLimit<0 || ++t.m_iCalls<=t.m_iLimit; };
		a.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
		b.SeekDocReader ( postings::FILE_HEADER_SIZE, 0 );
		pNative->Reset ( true, { &tA, fnContinue, nullptr } );
		pReference->Reset ( true, { &tB, fnContinue, nullptr } );
		RowID_t tMin = tBase+(iTry%32/8)*17;
		PostingWindow_t tRange { tBase-7, tMin, std::min ( uint64_t(INVALID_ROWID)+1, uint64_t(tBase)+4077 ) };
		DWORD dA[PostingWindow_t::WORDS], dB[PostingWindow_t::WORDS], dScratch[PostingWindow_t::WORDS] = {};
		for ( unsigned i=0; i<PostingWindow_t::WORDS; ++i )
			dA[i] = dB[i] = iTry/8==0 ? 0 : iTry/8==1 ? UINT32_MAX : (i*1664525U+1013904223U) ^ (iTry*2654435761U);

		auto na = iTry<32 ? pNative->FillAnd ( tRange, dA ) : pNative->FillAndNot ( tRange, dA );
		auto nb = pReference->FillOr ( tRange, dScratch );
		size_t iWords = size_t((tRange.m_uEnd-tRange.m_tBase+31)/32);
		if ( iTry<32 )
			bitmap::And ( dB, dB, dScratch, iWords );
		else
		{
			bitmap::AndNot ( dB, dB, dScratch, iWords );
			for ( unsigned bit=0; bit<iWords*32; ++bit )
			{
				uint64_t row = uint64_t(tRange.m_tBase)+bit;
				if ( nb.m_bStopped || row<tRange.m_tMin || row>=tRange.m_uEnd || row>=INVALID_ROWID )
					dB[bit/32] &= ~(DWORD(1)<<(bit%32));
			}
		}
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


static void TestBlockPayloads ( const char * sFile, unsigned uShape, FileAccess_e eAccess, bool bRestricted, RowID_t tBase )
{
	MemoryOutput_c tOutput;
	auto pWriter = std::make_unique<postings::Writer_c>(tOutput);
	postings::TermDesc_t tDesc;
	tDesc.m_uFields = 2;

	std::string sError;
	std::array<std::vector<QwordDoc_t>,3> dExpected;
	std::array<uint64_t,3> dStarts;
	for ( unsigned iTerm=0; iTerm<3; ++iTerm )
	{
		dStarts[iTerm] = tOutput.GetPos();
		Check ( pWriter->Begin ( tDesc, sError ) );
		for ( unsigned i=0; i<2051; ++i )
		{
			if ( (iTerm==1 && i%7==0) || (iTerm==2 && (i%17==0 || (i>700 && i<1250))) )
				continue;

			postings::Posting_t t;
			t.m_uRow = tBase+(uShape==0 ? i : uShape==1 ? i*3/2 : i*97);
			t.m_uTF = i%11+1;
			t.m_uHitRef = (uint64_t(1)<<34)+i*31+iTerm;

			unsigned f = (i+iTerm)%2;
			t.m_dFields[0] = 1U<<f;
			t.m_dFieldTF[f] = t.m_uTF;
			Check ( pWriter->Add ( t, sError ) );
			dExpected[iTerm].push_back ( { t.m_uRow, t.m_dFields[0], t.m_uTF, SphOffset_t(t.m_uHitRef) } );
		}

		Check ( pWriter->Finish(sError) );
	}
	{
		std::ofstream tFile ( sFile, std::ios::binary | std::ios::trunc );
		tFile.write ( reinterpret_cast<const char *>(tOutput.m_dData.data()), tOutput.m_dData.size() );
		Check ( bool(tFile) );
	}

	CSphString sReadError;
	DataReaderFactoryPtr_c pFactory ( NewProxyReader ( sFile, sReadError, DataReaderFactory_c::DOCS, 1024, eAccess ) );
	Check ( bool(pFactory) );
	for ( unsigned mode=0; mode<4; ++mode )
	{
		std::array<CountingQword_c,3> dWords;
		PostingBlockAnd_c tAnd({});
		for ( unsigned i=0; i<3; ++i )
		{
			dWords[i].SetDocReader(pFactory);
			dWords[i].m_iDocs = int(dExpected[i].size());
			dWords[i].SeekDocReader ( dStarts[i], 0 );
			tAnd.Add ( dWords[i].CreatePostingBlock ( bRestricted ? 1U<<(i%2) : UINT32_MAX, {} ) );
		}

		unsigned iExpected = 0;
		unsigned iSeen = 0;
		RowID_t tMin = tBase+17;
		uint64_t uEnd = std::min ( uint64_t(INVALID_ROWID), uint64_t(dExpected[0].back().m_tRowID)-3 );
		for ( unsigned turn=0; ; ++turn )
		{
			Check ( turn<10000 );

			auto tRows = tAnd.ReadRows ( tMin, uEnd, int(turn%3==0 ? 1 : turn%3==1 ? 31 : 257) );
			if ( !tRows.m_iCount )
				break;

			std::array<QwordDoc_t,257> dDocs;
			for ( unsigned iTerm=0; iTerm<3; ++iTerm )
			{
				const auto & dCounts = dWords[iTerm].GetDecodeCounts();
				if ( !(mode & 1) )
					Check ( !dCounts[2] );

				if ( !(mode & 2) )
					Check ( !dCounts[3] );

				Check ( tAnd.GetCursor(iTerm).ReadPayloads ( tRows.m_pRows, tRows.m_iCount, dDocs.data(), mode & 1, mode & 2 ) );
				for ( int j=0; j<tRows.m_iCount; ++j )
				{
					auto pExpected = std::lower_bound ( dExpected[iTerm].begin(), dExpected[iTerm].end(), tRows.m_pRows[j], [] ( const QwordDoc_t & t, RowID_t r ) { return t.m_tRowID<r; } );
					Check ( pExpected!=dExpected[iTerm].end() && pExpected->m_tRowID==dDocs[j].m_tRowID );
					Check ( dDocs[j].m_uFields==pExpected->m_uFields );
					Check ( dDocs[j].m_uMatchHits==((mode & 1) ? pExpected->m_uMatchHits : 0) );
					Check ( dDocs[j].m_iHitlistPos==((mode & 2) ? pExpected->m_iHitlistPos : 0) );
				}
			}

			for ( int j=0; j<tRows.m_iCount; ++j )
			{
				bool bFound = false;
				while ( iExpected<2051 )
				{
					unsigned i = iExpected++;
					RowID_t tRow = dExpected[0][i].m_tRowID;
					if ( tRow<tMin || i%7==0 || i%17==0 || (i>700 && i<1250) || (bRestricted && i%2) )
						continue;

					Check ( tRows.m_pRows[j]==tRow && tRow<uEnd );
					++iSeen;
					bFound = true;
					break;
				}

				Check(bFound);
			}

			if ( turn%5==0 )
				tMin = RowID_t(std::min ( uEnd, uint64_t(tRows.m_pRows[tRows.m_iCount-1])+19 ));
		}

		Check ( iSeen>0 );
		for ( unsigned i=iExpected; i<2051; ++i )
			Check ( dExpected[0][i].m_tRowID<tMin || dExpected[0][i].m_tRowID>=uEnd || i%7==0 || i%17==0 || (i>700 && i<1250) || (bRestricted && i%2) );

		Check ( !tAnd.ReadRows ( tMin, uEnd, 31 ).m_iCount );
		for ( const auto & tWord : dWords )
			Check ( !tWord.GetReadError() && !tWord.m_uCalls );

		// The lead fixture is dense throughout; followers have deliberately sparse gaps.
		if ( !bRestricted && uShape!=2 )
			Check ( !dWords[0].GetDecodeCounts()[0] );
	}

	// Exclusion may cross multiple negative leaves while positive payloads stay
	// pinned. Check dense/sparse shapes, fields, empty negatives and short batches.
	for ( unsigned mode=0; mode<4; ++mode )
	{
		std::array<CountingQword_c,3> dWords;
		PostingBlockAnd_c tAnd({});
		unsigned iPositive = mode==1 ? 2 : 1;
		for ( unsigned i=0; i<3; ++i )
		{
			dWords[i].SetDocReader(pFactory);
			dWords[i].m_iDocs = mode==3 && i>=iPositive ? 0 : int(dExpected[i].size());
			dWords[i].SeekDocReader ( dStarts[i], 0 );

			DWORD uMask = bRestricted ? 1U<<(i%2) : UINT32_MAX;
			if ( i<iPositive )
				tAnd.Add ( dWords[i].CreatePostingBlock ( uMask, {} ) );
			else if ( mode!=2 || i==1 )
				tAnd.AddExcluded ( dWords[i].CreatePostingWindow ( uMask, {} ) );
		}

		std::vector<RowID_t> dRows;
		for ( const auto & tDoc : dExpected[0] )
		{
			bool bKeep = true;
			for ( unsigned i=0; i<3; ++i )
			{
				if ( mode==2 && i==2 )
					continue;

				auto p = std::lower_bound ( dExpected[i].begin(), dExpected[i].end(), tDoc.m_tRowID, [] ( const QwordDoc_t & t, RowID_t r ) { return t.m_tRowID<r; } );
				bool bMatch = p!=dExpected[i].end() && p->m_tRowID==tDoc.m_tRowID && (!bRestricted || (p->m_uFields & (1U<<(i%2))));
				if ( mode==3 && i>=iPositive )
					bMatch = false;

				bKeep &= i<iPositive ? bMatch : !bMatch;
			}

			if ( bKeep )
				dRows.push_back(tDoc.m_tRowID);
		}

		size_t iExpected = 0;
		RowID_t tMin = tBase;
		for ( unsigned turn=0; ; ++turn )
		{
			Check ( turn<10000 );

			auto tRows = tAnd.ReadRows ( tMin, INVALID_ROWID, turn%2 ? 31 : 1 );
			if ( !tRows.m_iCount )
				break;

			std::array<QwordDoc_t,31> dDocs;
			Check ( tAnd.GetCursor(0).ReadPayloads ( tRows.m_pRows, tRows.m_iCount, dDocs.data(), true, true ) );
			for ( int i=0; i<tRows.m_iCount; ++i )
			{
				while ( iExpected<dRows.size() && dRows[iExpected]<tMin )
					++iExpected;

				Check ( iExpected<dRows.size() && tRows.m_pRows[i]==dRows[iExpected++] );

				auto p = std::lower_bound ( dExpected[0].begin(), dExpected[0].end(), tRows.m_pRows[i], [] ( const QwordDoc_t & t, RowID_t r ) { return t.m_tRowID<r; } );
				Check ( p!=dExpected[0].end() && dDocs[i].m_iHitlistPos==p->m_iHitlistPos && dDocs[i].m_uMatchHits==p->m_uMatchHits );
			}

			if ( turn%5==0 )
				tMin = RowID_t(uint64_t(tRows.m_pRows[tRows.m_iCount-1])+19);
		}

		while ( iExpected<dRows.size() && dRows[iExpected]<tMin )
			++iExpected;

		Check ( iExpected==dRows.size() );
		for ( unsigned i=iPositive; i<3; ++i )
			Check ( !dWords[i].GetDecodeCounts()[2] && !dWords[i].GetDecodeCounts()[3] );
	}

	// An interrupted negative must not expose any unchecked positive candidates.
	{
		CountingQword_c tPositive, tNegative;
		tPositive.SetDocReader(pFactory);
		tNegative.SetDocReader(pFactory);
		tPositive.m_iDocs = int(dExpected[0].size());
		tNegative.m_iDocs = int(dExpected[1].size());
		tPositive.SeekDocReader ( dStarts[0], 0 );
		tNegative.SeekDocReader ( dStarts[1], 0 );

		int iCalls = 0;
		PostingWindowContext_t tStop { &iCalls, [] ( void * p ) { ++*static_cast<int *>(p); return false; }, nullptr };
		PostingBlockAnd_c tAnd({});
		tAnd.Add ( tPositive.CreatePostingBlock ( UINT32_MAX, {} ) );
		tAnd.AddExcluded ( tNegative.CreatePostingWindow ( UINT32_MAX, tStop ) );
		Check ( !tAnd.ReadRows ( tBase, INVALID_ROWID, 31 ).m_iCount && iCalls==1 );
		Check ( !tAnd.ReadRows ( tBase, INVALID_ROWID, 31 ).m_iCount && iCalls==1 );
	}

	// Intersecting and probing must not invalidate payloads, even after the last
	// posting in the leaf was consumed. A fresh view after reset must work too.
	CountingQword_c tWord;
	tWord.SetDocReader(pFactory);
	tWord.m_iDocs = int(dExpected[0].size());
	for ( unsigned iTry=0; iTry<3; ++iTry )
	{
		tWord.Reset();
		tWord.m_iDocs = int(dExpected[0].size());
		tWord.SeekDocReader ( dStarts[0], 0 );

		auto pCursor = tWord.CreatePostingBlock ( UINT32_MAX, {} );
		PostingLeaf_t tLeaf;
		Check ( pCursor->Prepare ( tBase+3, tLeaf ) );

		RowID_t tLast = RowID_t(tLeaf.m_uEnd-1);
		DWORD dBits[PostingWindow_t::WORDS];
		std::fill_n ( dBits, PostingWindow_t::WORDS, UINT32_MAX );

		PostingWindow_t tWindow { tLast, tLast, tLeaf.m_uEnd };
		Check ( pCursor->Intersect ( tWindow, dBits )==1 );
		Check ( pCursor->FilterRows ( &tLast, 1 )==1 );
		Check ( !tWord.GetDecodeCounts()[2] && !tWord.GetDecodeCounts()[3] );

		QwordDoc_t tDoc;
		Check ( pCursor->ReadPayloads ( &tLast, 1, &tDoc, true, true ) );
		Check ( tDoc.m_tRowID==tLast && tDoc.m_uMatchHits==dExpected[0][255].m_uMatchHits && tDoc.m_iHitlistPos==dExpected[0][255].m_iHitlistPos );
	}

	int iCalls = 0;
	PostingWindowContext_t tContext { &iCalls, [] ( void * p ) { return ++*static_cast<int *>(p)<3; }, nullptr };
	tWord.Reset();
	tWord.m_iDocs = int(dExpected[0].size());
	tWord.SeekDocReader ( dStarts[0], 0 );

	auto dCountsBefore = tWord.GetDecodeCounts();
	PostingBlockAnd_c tStopped(tContext);
	tStopped.Add ( tWord.CreatePostingBlock ( UINT32_MAX, tContext ) );
	Check ( !tStopped.ReadRows ( tBase, INVALID_ROWID, 31 ).m_iCount );

	int iStopped = iCalls;
	Check ( !tStopped.ReadRows ( tBase, INVALID_ROWID, 31 ).m_iCount && iCalls==iStopped );
	Check ( tWord.GetDecodeCounts()==dCountsBefore );
}


static void TestQuorum()
{
	uint32_t uSeed = 17;
	for ( int iThreshold=1; iThreshold<=32; ++iThreshold )
	{
		PostingQuorum_c tQuorum(iThreshold);
		for ( int iRound=0; iRound<2; ++iRound )
		{
			tQuorum.Reset();

			unsigned char dCounts[PostingWindow_t::ROWS] {};
			DWORD dInput[PostingWindow_t::WORDS], dOutput[PostingWindow_t::WORDS];
			Check ( !tQuorum.Finish(dOutput) );
			for ( int iTerm=0; iTerm<32; ++iTerm )
			{
				for ( int i=0; i<PostingWindow_t::WORDS; ++i )
				{
					uSeed = uSeed*1664525u+1013904223u;
					dInput[i] = i%3==0 ? UINT32_MAX : i%3==1 ? 0 : uSeed;
					for ( int j=0; j<32; ++j )
						dCounts[i*32+j] += (dInput[i]>>j)&1;
				}

				tQuorum.Add(dInput);

				bool bAny = tQuorum.Finish(dOutput);
				bool bExpected = false;
				for ( int i=0; i<PostingWindow_t::ROWS; ++i )
				{
					bool bMatch = dCounts[i]>=iThreshold;
					Check ( bool((dOutput[i/32]>>(i%32))&1)==bMatch );
					bExpected |= bMatch;
				}

				Check ( bAny==bExpected );
			}
		}
	}
}


template<typename QWORD>
static void TestHitReader ( const char * sFile, std::initializer_list<FileAccess_e> dAccess )
{
	std::vector<std::vector<DWORD>> dHits { {1,127,128,16384,0x1fffff,0x10000000,UINT32_MAX}, {}, {}, {1}, {1,2} };
	for ( unsigned i=0; i<9000; ++i )
		dHits[1].push_back(i+1);

	std::vector<BYTE> dBytes(postings::FILE_HEADER_SIZE);
	std::vector<SphOffset_t> dStarts;
	std::vector<std::vector<SphOffset_t>> dPositions;
	for ( const auto & dDoc : dHits )
	{
		dStarts.push_back(dBytes.size());
		dPositions.emplace_back();

		DWORD uPrevious = 0;
		for ( DWORD uHit : dDoc )
		{
			ZipValueBE ( [&dBytes] ( BYTE uByte ) { dBytes.push_back(uByte); }, uHit-uPrevious );
			dPositions.back().push_back(dBytes.size());
			uPrevious = uHit;
		}

		dBytes.push_back(0);
		dPositions.back().push_back(dBytes.size());
	}

	{
		std::ofstream tFile ( sFile, std::ios::binary | std::ios::trunc );
		tFile.write ( reinterpret_cast<const char *>(dBytes.data()), dBytes.size() );
		Check ( bool(tFile) );
	}

	QWORD tWord ( false, false, 0 );
	tWord.m_bHasHitlist = true;

	// Rebind the same qword across mapped and buffered readers, including the map tail.
	for ( auto eAccess : dAccess )
	{
		CSphString sError;
		DataReaderFactoryPtr_c pFactory ( NewProxyReader ( sFile, sError, DataReaderFactory_c::HITS, 1024, eAccess ) );
		Check ( bool(pFactory) );
		tWord.SetHitReader(pFactory);
		tWord.SeekHitReader ( 1, 0 );
		Check ( tWord.GetHitReaderPos()==postings::FILE_HEADER_SIZE );
		Check ( tWord.GetNextHit()==EMPTY_HIT );
		tWord.m_bHasHitlist = false;
		tWord.SeekHitlist(dStarts[0]);
		Check ( tWord.GetNextHit()==EMPTY_HIT );
		tWord.m_bHasHitlist = true;

		for ( bool bReverse : {false,true} )
			for ( size_t i=0; i<dHits.size(); ++i )
			{
				size_t iDoc = bReverse ? dHits.size()-i-1 : i;
				tWord.Reset();
				tWord.SeekHitlist(dStarts[iDoc]);
				Check ( tWord.GetHitReaderPos()==dStarts[iDoc] );
				for ( size_t j=0; j<dHits[iDoc].size(); ++j )
				{
					Check ( tWord.GetNextHit()==dHits[iDoc][j] );
					Check ( tWord.GetHitReaderPos()==dPositions[iDoc][j] );
				}

				Check ( tWord.GetNextHit()==EMPTY_HIT );
				Check ( tWord.GetHitReaderPos()==dPositions[iDoc].back() );
				Check ( tWord.GetNextHit()==EMPTY_HIT && !tWord.GetReadError() );
			}

		// A failed doclist open must also prevent a later hit seek from reviving the qword.
		tWord.SetDocReader(pFactory);
		tWord.SeekDocReader ( 1, 0 );
		Check ( tWord.GetNextDoc().m_tRowID==INVALID_ROWID && tWord.GetReadError() );
		tWord.SeekHitlist(dStarts[0]);
		Check ( tWord.GetNextHit()==EMPTY_HIT && tWord.GetReadError() );
		tWord.Reset();
		tWord.SeekHitlist(dStarts[0]);
		Check ( tWord.GetNextHit()==dHits[0][0] && !tWord.GetReadError() );
		tWord.Reset();
	}
}


int main ( int argc, char ** argv )
{
	TestQuorum();

	if ( argc!=2 )
		return 2;

	TestHitReader<BlockIndexQword_c> ( argv[1], {FileAccess_e::MMAP,FileAccess_e::FILE,FileAccess_e::MMAP} );
	TestHitReader<BlockIndexQword_T<true>> ( argv[1], {FileAccess_e::MMAP,FileAccess_e::MMAP} );
	TestHitReader<BlockIndexQword_T<false>> ( argv[1], {FileAccess_e::FILE,FileAccess_e::FILE} );

	for ( unsigned n : { 1u, 127u, 128u, 255u, 256u, 257u, 513u, 1025u } )
		for ( int iPayload : { 0, 1, 2 } )
			for ( auto eAccess : { FileAccess_e::MMAP, FileAccess_e::FILE } )
				Test ( argv[1], n, iPayload, eAccess );

	for ( unsigned shape=0; shape<3; ++shape )
		for ( unsigned fields : { 1u, 2u, 33u } )
			for ( DWORD mask : { 0u, 1u, UINT32_MAX } )
				for ( RowID_t base : { 4093u, UINT32_MAX-(shape==0 ? 2050u : shape==1 ? 3075u : 198850u)-1 } )
					for ( auto access : { FileAccess_e::MMAP, FileAccess_e::FILE } )
						TestWindows ( argv[1], shape, fields, mask, base, access );

	for ( unsigned shape=0; shape<3; ++shape )
		TestWindows ( argv[1], shape, 33, 2, 4093, FileAccess_e::FILE, true );

	for ( unsigned shape=0; shape<3; ++shape )
		for ( auto access : { FileAccess_e::MMAP, FileAccess_e::FILE } )
			for ( bool restricted : { false, true } )
				for ( RowID_t base : { 4093u, UINT32_MAX-200000u } )
					TestBlockPayloads ( argv[1], shape, access, restricted, base );

	Check ( !std::remove(argv[1]) );
	std::cout << g_uChecks << " qword batch checks passed\n";
}
