// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "postings/blockqword.h"
#include "postings/disk.h"

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


int main ( int argc, char ** argv )
{
	if ( argc!=2 )
		return 2;

	for ( unsigned n : { 1u, 127u, 128u, 255u, 256u, 257u, 513u, 1025u } )
		for ( bool bMembership : { false, true } )
			for ( auto eAccess : { FileAccess_e::MMAP, FileAccess_e::FILE } )
				Test ( argv[1], n, bMembership, eAccess );

	Check ( !std::remove(argv[1]) );
	std::cout << g_uChecks << " qword batch checks passed\n";
}
