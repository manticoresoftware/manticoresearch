// Streaming postings container v6 writer shared by plain and RT producers.
#pragma once

#include "postings_container_codecs.h"
#include <algorithm>


#include <cassert>
#include <cerrno>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#if WITH_ZLIB
#include <zlib.h>
#endif

#if !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

namespace e1 {

struct Posting
{
	Posting() = default;
	Posting ( uint32_t uRow, uint32_t uTF, uint32_t uMask, uint64_t uRef, uint32_t uFirstFieldTF=0 )
		: m_uRow ( uRow ), m_uTF ( uTF ), m_uMask ( uMask ), m_uFirstFieldTF ( uFirstFieldTF ), m_uRef ( uRef ) {}

	uint32_t m_uRow = 0;
	uint32_t m_uTF = 0;
	uint32_t m_uMask = 0;
	// Exact TF for the least-significant field when exactly two fields match.
	// Zero means unavailable; one-field TF is already m_uTF.
	uint32_t m_uFirstFieldTF = 0;
	uint64_t m_uRef = 0;
};

static_assert ( sizeof(Posting)==24 );

class Writer
{
	struct Entry
	{
		uint64_t m_uKey = 0;
		uint64_t m_uOffset = 0;
		uint32_t m_uDocs = 0;
		uint32_t m_uHasHits = 0;
		uint64_t m_uHits = 0;
	};
public:
	~Writer() { StopWorker(); }

	bool Open ( const std::string & sFilename, std::string & sError )
	{
		if ( !WaitPending ( sError ) )
			return false;
		m_sFilename = sFilename;

		m_dEntries.clear();
		m_tOut.close();
		m_tOut.clear();
		m_tOut.rdbuf()->pubsetbuf ( m_dFileBuffer.data(), m_dFileBuffer.size() );
		m_tOut.open ( sFilename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc );
		if ( !m_tOut )
			return Fail ( sError, "open" );
		char dHeader[56] = {};
		m_tOut.write ( dHeader, sizeof(dHeader) );
		if ( !Check ( sError, "write header placeholder" ) )
			return false;
		m_uFilePos = sizeof(dHeader);
		m_uPayloadCRCState = ~0u;
		StartWorker();
		return true;
	}

	bool FinishTerm ( uint64_t uKey, std::vector<Posting> & dPostings, uint64_t uHits, bool bHasHits, std::string & sError )
	{
		if ( !WaitPending ( sError ) )
			return false;
		if ( !uKey || dPostings.empty() || ( !m_dEntries.empty() && uKey<=m_dEntries.back().m_uKey ) )
		{
			sError = "invalid stable term key/postings";
			return false;
		}

		Entry tEntry { uKey, Tell(), uint32_t(dPostings.size()), bHasHits ? 1u : 0u, uHits };
		m_dEntries.push_back ( tEntry );
		dPostings.swap ( m_dSparePostings );
		{
			std::lock_guard<std::mutex> tLock ( m_tWorkerMutex );
			m_dWorkerPostings.swap ( m_dSparePostings );
			m_tWorkerEntry = tEntry;
			m_bWorkerJob = true;
			m_bWorkerPending = true;
			m_bWorkerDone = false;
		}
		m_tWorkerCV.notify_one();
		return true;
	}

	bool FinishTerm ( uint64_t uKey, const std::vector<Posting> & dPostings, uint64_t uHits, bool bHasHits, std::string & sError )
	{
		auto dCopy = dPostings;
		return FinishTerm ( uKey, dCopy, uHits, bHasHits, sError );
	}

	bool Finalize ( const std::string & sDict, const std::string & sHits, std::string & sError )
	{
		if ( !WaitPending ( sError ) )
			return false;
		if ( !m_tOut )
		{
			sError = "E1 writer is not open";
			return false;
		}

		const uint64_t uDirectory = Tell();
		for ( const Entry & tEntry : m_dEntries )
		{
			Put ( tEntry.m_uKey, 8 );
			Put ( tEntry.m_uOffset, 8 );
			Put ( tEntry.m_uDocs, 4 );
			Put ( tEntry.m_uHasHits, 4 );
			Put ( tEntry.m_uHits, 8 );
		}
		const uint64_t uSize = Tell();
		m_tOut.flush();
		if ( !Check ( sError, "flush payload/directory" ) )
			return false;

		const uint32_t uPayloadCRC = ~m_uPayloadCRCState;
		uint32_t uDictCRC = 0, uHitsCRC = 0;
		// The header is the generation commit point. Persist every referenced
		// posting component before checksumming and publishing that header.
		if ( !SyncFile ( sDict, sError ) || !SyncFile ( sHits, sError ) )
			return false;
		FaultPoint ( "after_components_fsync" );
		if ( !FileCRC ( sDict, 0, UINT64_MAX, uDictCRC, sError ) ||
			 !FileCRC ( sHits, 0, UINT64_MAX, uHitsCRC, sError ) )
			return false;


		m_tOut.seekp ( 0 );
		FaultPoint ( "before_primary_header" );
		PutRaw ( "E1POST06", 8 );
		Put ( 6, 4 );
		Put ( 56, 4 );
		Put ( uSize, 8 );
		Put ( m_dEntries.size(), 8 );
		Put ( uPayloadCRC, 4 );
		Put ( uDictCRC, 4 );
		Put ( uHitsCRC, 4 );
		Put ( 1, 4 ); // streamed layout: directory is at the tail
		Put ( uDirectory, 8 );
		m_tOut.flush();
		if ( !Check ( sError, "write final header" ) )
			return false;
		m_tOut.close();
		if ( !SyncFile ( m_sFilename, sError ) )
			return false;
		FaultPoint ( "after_primary_fsync" );
		MarkTrustedGeneration ( m_sFilename, sDict, sHits, uSize, uPayloadCRC, uDictCRC, uHitsCRC );
		return true;
	}

private:
	void StartWorker()
	{
		if ( m_tWorker.joinable() )
			return;
		m_bWorkerStop = false;
		m_tWorker = std::thread ( [this] { WorkerLoop(); } );
	}

	void StopWorker()
	{
		if ( !m_tWorker.joinable() )
			return;
		std::string sIgnored;
		WaitPending ( sIgnored );
		{
			std::lock_guard<std::mutex> tLock ( m_tWorkerMutex );
			m_bWorkerStop = true;
		}
		m_tWorkerCV.notify_one();
		m_tWorker.join();
	}

	void WorkerLoop()
	{
		for ( ;; )
		{
			Entry tEntry;
			{
				std::unique_lock<std::mutex> tLock ( m_tWorkerMutex );
				m_tWorkerCV.wait ( tLock, [this] { return m_bWorkerJob || m_bWorkerStop; } );
				if ( m_bWorkerStop )
					return;
				tEntry = m_tWorkerEntry;
				m_bWorkerJob = false;
			}

			bool bOk = false;
			std::string sError;
			try
			{
				m_dTermBuffer.clear();
				m_dTermBuffer.reserve ( m_dWorkerPostings.size()*8ull );
				m_pTermBuffer = &m_dTermBuffer;
				m_uTermBase = tEntry.m_uOffset;
				m_uTermPos = 0;
				bOk = EncodeTerm ( tEntry, m_dWorkerPostings, sError );
				m_pTermBuffer = nullptr;
				if ( bOk )
				{
					PutRaw ( m_dTermBuffer.data(), m_dTermBuffer.size() );
					bOk = Check ( sError, "append term" );
				}
			} catch ( const std::exception & tException )
			{
				m_pTermBuffer = nullptr;
				sError = std::string("asynchronous postings writer failed: ") + tException.what();
			} catch ( ... )
			{
				m_pTermBuffer = nullptr;
				sError = "asynchronous postings writer failed";
			}

			{
				std::lock_guard<std::mutex> tLock ( m_tWorkerMutex );
				m_bWorkerOk = bOk;
				m_sWorkerError = std::move ( sError );
				m_bWorkerDone = true;
			}
			m_tWorkerCV.notify_all();
		}
	}

	bool WaitPending ( std::string & sError )
	{
		if ( !m_tWorker.joinable() )
			return true;
		std::unique_lock<std::mutex> tLock ( m_tWorkerMutex );
		if ( !m_bWorkerPending )
			return true;
		m_tWorkerCV.wait ( tLock, [this] { return m_bWorkerDone; } );
		m_dWorkerPostings.swap ( m_dSparePostings );
		m_dSparePostings.clear();
		m_bWorkerDone = false;
		m_bWorkerPending = false;
		if ( !m_bWorkerOk )
		{
			sError = std::move ( m_sWorkerError );
			return false;
		}
		return true;
	}

	static void FaultPoint ( const char * szPoint )
	{
		const char * szFault = getenv ( "MANTICORE_E1_FAULT" );
		if ( szFault && !strcmp ( szFault, szPoint ) )
		{
#if defined(_WIN32)
			std::abort();
#else
			::_exit ( 86 );
#endif
		}
	}

	uint64_t Tell()
	{
		if ( m_pTermBuffer )
			return m_uTermBase+m_uTermPos;
		return m_uFilePos;
	}

	void Seek ( uint64_t uOffset )
	{
		if ( m_pTermBuffer )
		{
			assert ( uOffset>=m_uTermBase && uOffset<=m_uTermBase+m_pTermBuffer->size() );
			m_uTermPos = uOffset-m_uTermBase;
			return;
		}
		m_tOut.seekp ( uOffset );
		m_uFilePos = uOffset;
	}

	void PutRaw ( const void * pData, size_t iBytes )
	{
		if ( !iBytes )
			return;
		if ( m_pTermBuffer )
		{
			if ( m_uTermPos+iBytes>m_pTermBuffer->size() )
				m_pTermBuffer->resize ( m_uTermPos+iBytes );
			memcpy ( m_pTermBuffer->data()+m_uTermPos, pData, iBytes );
			m_uTermPos += iBytes;
			return;
		}
		if ( m_uFilePos>=56 )
			m_uPayloadCRCState = CRCUpdate ( m_uPayloadCRCState, reinterpret_cast<const uint8_t *>(pData), iBytes );
		m_tOut.write ( reinterpret_cast<const char *>(pData), iBytes );
		m_uFilePos += iBytes;
	}

	void Put ( uint64_t uValue, unsigned iBytes )
	{
		char dBytes[8];
		for ( unsigned i=0; i<iBytes; ++i )
			dBytes[i] = char ( uValue >> ( i*8 ) );
		PutRaw ( dBytes, iBytes );
	}

	void PutBytes ( const std::vector<uint8_t> & dBytes )
	{
		PutRaw ( dBytes.data(), dBytes.size() );
	}

	void PutZeroes ( size_t iBytes )
	{
		if ( !iBytes )
			return;
		if ( m_pTermBuffer )
		{
			if ( m_uTermPos+iBytes>m_pTermBuffer->size() )
				m_pTermBuffer->resize ( m_uTermPos+iBytes );
			else
				memset ( m_pTermBuffer->data()+m_uTermPos, 0, iBytes );
			m_uTermPos += iBytes;
			return;
		}
		static const char dZeroes[4096] = {};
		while ( iBytes )
		{
			const size_t iChunk = std::min ( iBytes, sizeof(dZeroes) );
			PutRaw ( dZeroes, iChunk );
			iBytes -= iChunk;
		}
	}

	static unsigned Width ( uint64_t uValue )
	{
		unsigned iWidth = 0;
		while ( uValue )
		{
			++iWidth;
			uValue >>= 1;
		}
		return iWidth;
	}

	template<typename VALUE>
	void PutPacked ( unsigned iWidth, uint32_t uCount, VALUE fnValue )
	{
		const size_t iBytes = ( uint64_t(uCount)*iWidth + 7 ) / 8;
		std::vector<uint8_t> dPacked;
		uint8_t * pPacked = nullptr;
		if ( m_pTermBuffer )
		{
			const size_t iStart = m_uTermPos;
			if ( iStart+iBytes>m_pTermBuffer->size() )
				m_pTermBuffer->resize ( iStart+iBytes );
			else if ( iBytes )
				memset ( m_pTermBuffer->data()+iStart, 0, iBytes );
			pPacked = m_pTermBuffer->data()+iStart;
			m_uTermPos += iBytes;
		} else
		{
			dPacked.resize ( iBytes );
			pPacked = dPacked.data();
		}
#if defined(__SIZEOF_INT128__)
		unsigned __int128 uBits = 0;
		unsigned iBits = 0;
		size_t iOut = 0;
		const uint64_t uMask = iWidth==64 ? UINT64_MAX : (uint64_t(1)<<iWidth)-1;
		for ( uint32_t i=0; i<uCount; ++i )
		{
			uBits |= ( static_cast<unsigned __int128>(fnValue(i)&uMask) << iBits );
			iBits += iWidth;
			if ( iBits>=64 )
			{
				const uint64_t uWord = uint64_t(uBits);
				for ( unsigned j=0; j<8; ++j )
					pPacked[iOut+j] = uint8_t(uWord>>(j*8));
				iOut += 8;
				uBits >>= 64;
				iBits -= 64;
			}
		}
		while ( iBits )
		{
			pPacked[iOut++] = uint8_t(uBits);
			uBits >>= 8;
			iBits = iBits>8 ? iBits-8 : 0;
		}
#else
		for ( uint32_t i=0; i<uCount; ++i )
		{
			uint64_t uValue = fnValue(i);
			uint64_t uBit = uint64_t(i)*iWidth;
			unsigned iDone = 0;
			while ( iDone<iWidth )
			{
				unsigned iTake = std::min ( 8u-unsigned(uBit%8), iWidth-iDone );
				pPacked[uBit/8] |= uint8_t ( ( ( uValue>>iDone ) & ( ( 1u<<iTake )-1 ) ) << (uBit%8) );
				iDone += iTake;
				uBit += iTake;
			}
		}
#endif
		if ( !m_pTermBuffer )
			PutBytes ( dPacked );
	}

	bool EncodeTerm ( const Entry & tEntry, const std::vector<Posting> & dPostings, std::string & sError )
	{
		const uint32_t uDocs = tEntry.m_uDocs;
		const uint32_t uMetaBlocks = ( uDocs+127 ) / 128;
		const bool bFrequent = uDocs>=4096;
		uint32_t uMaxFirstFieldTF = 0;
		uint32_t uRowBlocks = uMetaBlocks;
		uint32_t uPreviousContainer = UINT32_MAX;
		if ( bFrequent )
			uRowBlocks = 0;
		m_dRankBounds.assign ( (uDocs+63)/64, 0 );
		for ( uint32_t i=0; i<uDocs; ++i )
		{
			const Posting & tPosting = dPostings[i];
			uMaxFirstFieldTF = std::max ( uMaxFirstFieldTF, tPosting.m_uFirstFieldTF );
			m_dRankBounds[i/64] = std::max<uint8_t> ( m_dRankBounds[i/64], std::min(tPosting.m_uTF,255u) );
			if ( bFrequent )
			{
				const uint32_t uContainer = tPosting.m_uRow/4096;
				if ( uContainer!=uPreviousContainer )
				{
					++uRowBlocks;
					uPreviousContainer = uContainer;
				}
			}
		}
		const unsigned uFirstFieldTFWidth = Width ( uMaxFirstFieldTF );

		Put ( uDocs, 4 );
		Put ( uRowBlocks, 4 );
		Put ( uMetaBlocks, 4 );
		Put ( ( bFrequent ? 1u : 0u ) | ( uFirstFieldTFWidth<<8 ), 4 );
		Put ( 0, 8 );
		const uint64_t uDescriptors = Tell();
		PutZeroes ( uint64_t(uRowBlocks)*24 );

		uint32_t uAt = 0;
		auto & dPayload = m_dRowPayload;
		auto & dRuns = m_dRuns;
		dPayload.reserve ( 512 );
		dRuns.reserve ( 256 );
		for ( uint32_t b=0; b<uRowBlocks; ++b )
		{
			uint32_t uCount = std::min ( 128u, uDocs-uAt );
			uint32_t uCodec = 0;
			dPayload.clear();
			auto fnAppend = [&] ( uint64_t uValue, unsigned iBytes ) { for ( unsigned i=0; i<iBytes; ++i ) dPayload.push_back ( uint8_t(uValue>>(i*8)) ); };

			uint32_t uFirstDescriptor = 0, uSecondDescriptor = 0, uFlags = 0;
			if ( bFrequent )
			{
				uint32_t uEnd = uAt+1;
				uint32_t uContainer = dPostings[uAt].m_uRow/4096;
				while ( uEnd<uDocs && dPostings[uEnd].m_uRow/4096==uContainer )
					++uEnd;
				uCount = uEnd-uAt;
				dRuns.clear();
				for ( uint32_t j=uAt; j<uEnd; )
				{
					uint16_t uLo = dPostings[j].m_uRow%4096, uHi = uLo;
					while ( ++j<uEnd && dPostings[j].m_uRow%4096==uHi+1 )
						++uHi;
					dRuns.push_back ( uLo );
					dRuns.push_back ( uHi );
				}
				unsigned iBytes = uCount*2;
				if ( 512<iBytes ) { uCodec=1; iBytes=512; }
				if ( dRuns.size()*2<iBytes ) { uCodec=2; iBytes=dRuns.size()*2; }
				if ( uCodec==0 && uCount>=64 ) { uCodec=1; iBytes=512; }
				if ( uCodec==0 )
					for ( uint32_t i=uAt; i<uEnd; ++i ) fnAppend ( dPostings[i].m_uRow%4096, 2 );
				else if ( uCodec==1 )
				{
					dPayload.resize ( 512 );
					for ( uint32_t i=uAt; i<uEnd; ++i )
					{
						auto uRow = dPostings[i].m_uRow%4096;
						dPayload[uRow/8] |= uint8_t(1u<<(uRow%8));
					}
				} else
					for ( uint16_t uRun : dRuns ) fnAppend ( uRun, 2 );
				uFirstDescriptor = uContainer;
				uSecondDescriptor = uAt;
			} else
			{
				const bool bRun = uint64_t(dPostings[uAt].m_uRow)+uCount-1==dPostings[uAt+uCount-1].m_uRow;
				uCodec = bRun ? 1 : 2;
				if ( !bRun )
				{
					dPayload.resize ( size_t(uCount-1)*4 );
					uint8_t * pOut = dPayload.data();
					for ( uint32_t i=1; i<uCount; ++i, pOut+=4 )
					{
						const uint32_t uRow = dPostings[uAt+i].m_uRow;
						pOut[0] = uint8_t(uRow);
						pOut[1] = uint8_t(uRow>>8);
						pOut[2] = uint8_t(uRow>>16);
						pOut[3] = uint8_t(uRow>>24);
					}
				}
				uFirstDescriptor = dPostings[uAt].m_uRow;
				uSecondDescriptor = dPostings[uAt+uCount-1].m_uRow;
				uFlags = uCodec;
			}

			const uint64_t uPayload = Tell();
			PutBytes ( dPayload );
			const uint64_t uResume = Tell();
			Seek ( uDescriptors+uint64_t(b)*24 );
			Put ( uFirstDescriptor, 4 );
			Put ( uSecondDescriptor, 4 );
			if ( bFrequent ) { Put(uCount,2); Put(uCodec,2); Put(dPayload.size(),4); }
			else { Put(uCount,4); Put(uFlags,4); }
			Put ( uPayload, 8 );
			Seek ( uResume );
			uAt += uCount;
		}

		const uint64_t uMetadata = Tell();
		const uint64_t uResumeAfterRows = Tell();
		Seek ( tEntry.m_uOffset+16 );
		Put ( uMetadata, 8 );
		Seek ( uResumeAfterRows );
		PutZeroes ( uint64_t(uMetaBlocks)*16 );

		for ( uint32_t b=0; b<uMetaBlocks; ++b )
		{
			const uint32_t uBegin = b*128;
			const uint32_t uCount = std::min ( 128u, uDocs-uBegin );
			const uint32_t uFirstTF = dPostings[uBegin].m_uTF;
			const uint32_t uFirstMask = dPostings[uBegin].m_uMask;
			bool bSameTF = true, bSameMask = true;
			bool bSplitRefs = tEntry.m_uHasHits;
			uint64_t uLowAll=UINT64_MAX, uHighAll=0, uLowExternal=UINT64_MAX, uHighExternal=0, uMaxInlineRef=0;
			uint32_t uMaxTF=uFirstTF, uMaxMask=uFirstMask;
			for ( uint32_t i=0; i<uCount; ++i )
			{
				const Posting & tPosting = dPostings[uBegin+i];
				bSameTF &= tPosting.m_uTF==uFirstTF;
				bSameMask &= tPosting.m_uMask==uFirstMask;
				if ( bool(tPosting.m_uRef>>63)!=bool(tEntry.m_uHasHits && tPosting.m_uTF==1) )
				{
					sError = "E1 inline/TF contract";
					return false;
				}
				const uint64_t uRef = tPosting.m_uRef & uint64_t(INT64_MAX);
				uLowAll = std::min ( uLowAll, uRef );
				uHighAll = std::max ( uHighAll, uRef );
				if ( tPosting.m_uTF!=1 )
				{
					uLowExternal = std::min ( uLowExternal, uRef );
					uHighExternal = std::max ( uHighExternal, uRef );
				}
				else if ( bSplitRefs )
				{
					const uint32_t uMask = tPosting.m_uMask;
					bSplitRefs = uMask && !(uMask&(uMask-1)) && uRef<=UINT32_MAX && (uRef>>24)==uint64_t(__builtin_ctz(uMask));
					uMaxInlineRef = std::max ( uMaxInlineRef, ((uRef&0x007fffffu)<<1)|((uRef>>23)&1u) );
				}
				uMaxTF = std::max ( uMaxTF, tPosting.m_uTF );
				uMaxMask = std::max ( uMaxMask, tPosting.m_uMask );
			}
			const unsigned uTFMode = bSameTF ? ( uFirstTF==1 ? 0u : 1u ) : 2u;
			const unsigned uMaskMode = bSameMask ? ( uFirstMask==1 ? 0u : ( uFirstMask ? 1u : 3u ) ) : 2u;
			uint64_t uLow = bSplitRefs ? uLowExternal : uLowAll;
			if ( uLow==UINT64_MAX )
				uLow = 0;
			const uint64_t uMaxRefValue = bSplitRefs
				? std::max ( uMaxInlineRef, uLowExternal==UINT64_MAX ? 0 : uHighExternal-uLowExternal )
				: uHighAll-uLowAll;
			const unsigned uTFWidth = uTFMode==2 ? Width(uMaxTF) : 0;
			const unsigned uMaskWidth = uMaskMode==2 ? Width(uMaxMask) : 0;
			const unsigned uRefWidth = Width(uMaxRefValue);

			const uint32_t uFlags = (uTFMode<<4) | (uMaskMode<<6) | (uTFWidth<<9) | (uRefWidth<<16) | (tEntry.m_uHasHits<<22) | (uMaskWidth<<23) | (uint32_t(bSplitRefs)<<29);
			const uint64_t uPayload = Tell();
			if ( uTFMode==1 ) Put(dPostings[uBegin].m_uTF,4);
			else if ( uTFMode==2 ) PutPacked(uTFWidth,uCount,[&](unsigned i){ return dPostings[uBegin+i].m_uTF; });
			if ( uMaskMode==1 ) Put(dPostings[uBegin].m_uMask,4);
			else if ( uMaskMode==2 ) PutPacked(uMaskWidth,uCount,[&](unsigned i){ return dPostings[uBegin+i].m_uMask; });
			Put ( uLow, 8 );
			PutPacked ( uRefWidth, uCount, [&] ( unsigned i ) { const Posting &t=dPostings[uBegin+i];const uint64_t r=t.m_uRef&uint64_t(INT64_MAX);return bSplitRefs&&t.m_uTF==1?((r&0x007fffffu)<<1)|((r>>23)&1u):r-uLow; } );
			const uint64_t uResume = Tell();
			Seek ( uMetadata+uint64_t(b)*16 );
			Put ( uPayload, 8 ); Put ( uFlags, 4 ); Put ( uCount, 4 );
			Seek ( uResume );
		}
		// E1/5 primary ranked hints: one safe max-TF byte per 64 postings.
		// 255 is an escape meaning "unbounded", never the literal upper bound.
		PutBytes ( m_dRankBounds );
		if ( uFirstFieldTFWidth )
			PutPacked ( uFirstFieldTFWidth, uDocs, [&] ( unsigned i ) { return dPostings[i].m_uFirstFieldTF; } );

		return Check ( sError, "encode term" );
	}

	static bool FileCRC ( const std::string & sFilename, uint64_t uOffset, uint64_t uLength, uint32_t & uResult, std::string & sError )
	{
		std::ifstream tIn ( sFilename, std::ios::binary );
		if ( !tIn ) { sError = "failed to open CRC input '"+sFilename+"'"; return false; }
		tIn.seekg ( 0, std::ios::end );
		uint64_t uSize = uint64_t(tIn.tellg());
		if ( uOffset>uSize ) { sError = "invalid CRC range for '"+sFilename+"'"; return false; }
		if ( uLength==UINT64_MAX ) uLength=uSize-uOffset;
		if ( uLength>uSize-uOffset ) { sError = "invalid CRC length for '"+sFilename+"'"; return false; }
		tIn.seekg ( uOffset );
		#if WITH_ZLIB
		uLong uCRC = crc32 ( 0L, Z_NULL, 0 );
		#else
		uint32_t uCRC=~0u;
		#endif
		char dBuffer[65536];
		while ( uLength )
		{
			const size_t iChunk = size_t(std::min<uint64_t>(uLength,sizeof(dBuffer)));
			tIn.read ( dBuffer, iChunk );
			if ( size_t(tIn.gcount())!=iChunk ) { sError = "short CRC read from '"+sFilename+"'"; return false; }
			#if WITH_ZLIB
			uCRC = crc32 ( uCRC, reinterpret_cast<const Bytef *>(dBuffer), uInt(iChunk) );
			#else
			uCRC = CRCUpdate ( uCRC, reinterpret_cast<const uint8_t *>(dBuffer), iChunk );
			#endif
			uLength -= iChunk;
		}
		#if WITH_ZLIB
		uResult = uint32_t(uCRC);
		#else
		uResult=~uCRC;
		#endif
		return true;
	}

	static bool SyncFile ( const std::string & sFilename, std::string & sError )
	{
#if defined(_WIN32)
		(void)sFilename; (void)sError;
		return true;
#else
		int iFD = ::open ( sFilename.c_str(), O_RDONLY );
		if ( iFD<0 ) { sError="failed to open E1 output for fsync: "+std::string(strerror(errno)); return false; }
		int iResult = ::fsync(iFD);
		int iErrno = errno;
		::close(iFD);
		if ( iResult ) { sError="failed to fsync E1 output: "+std::string(strerror(iErrno)); return false; }
		return true;
#endif
	}

	bool Check ( std::string & sError, const char * szAction )
	{
		if ( m_tOut ) return true;
		sError = std::string("failed to ")+szAction+" for '"+m_sFilename+"'";
		return false;
	}

	bool Fail ( std::string & sError, const char * szAction )
	{
		sError = std::string("failed to ")+szAction+" E1 output '"+m_sFilename+"': "+strerror(errno);
		return false;
	}

	std::fstream m_tOut;
	std::vector<char> m_dFileBuffer = std::vector<char> ( 1u<<20 );

	std::string m_sFilename;
	std::vector<Entry> m_dEntries;
	std::vector<Posting> m_dSparePostings;
	std::vector<Posting> m_dWorkerPostings;
	std::thread m_tWorker;
	std::mutex m_tWorkerMutex;
	std::condition_variable m_tWorkerCV;
	Entry m_tWorkerEntry;
	std::string m_sWorkerError;
	bool m_bWorkerJob = false;
	bool m_bWorkerPending = false;
	bool m_bWorkerDone = false;
	bool m_bWorkerOk = false;
	bool m_bWorkerStop = false;
	std::vector<uint8_t> m_dTermBuffer;
	std::vector<uint8_t> m_dRankBounds;
	std::vector<uint8_t> m_dRowPayload;
	std::vector<uint16_t> m_dRuns;
	std::vector<uint8_t> * m_pTermBuffer = nullptr;
	uint64_t m_uTermBase = 0;
	uint64_t m_uTermPos = 0;
	uint64_t m_uFilePos = 0;
	uint32_t m_uPayloadCRCState = ~0u;
};

} // namespace e1
