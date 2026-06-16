//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "indexsettings.h"
#include "aggregate_hit.h"

/// possible bin states
enum ESphBinState {
	BIN_ERR_READ = -2, ///< bin read error
	BIN_ERR_END = -1,  ///< bin end
	BIN_POS = 0,	   ///< bin is in "expects pos delta" state
	BIN_DOC = 1,	   ///< bin is in "expects doc delta" state
	BIN_WORD = 2	   ///< bin is in "expects word delta" state
};


enum ESphBinRead {
	BIN_READ_OK,	   ///< bin read ok
	BIN_READ_EOF,	   ///< bin end
	BIN_READ_ERROR,	   ///< bin read error
	BIN_PRECACHE_OK,   ///< precache ok
	BIN_PRECACHE_ERROR ///< precache failed
};

/// bin, block input buffer
struct CSphBin
{
	static const int MIN_SIZE = 8192;
	static const int WARN_SIZE = 262144;

protected:
	ESphHitless m_eMode;
	CSphFixedVector<BYTE> m_dBuffer {0};
	BYTE* m_pCurrent = nullptr;
	int m_iLeft = 0;
	int m_iDone = 0;
	ESphBinState m_eState { BIN_POS };
	DictFormat_e m_eDictFormat = DictFormat_e::CRC;
	bool m_bError = false; // FIXME? sort of redundant, but states are a mess

	AggregateHit_t m_tHit;				///< currently decoded hit
	CSphFixedVector<BYTE> m_dKeyword { 0 }; ///< currently decoded hit keyword (in keywords dict mode)

#ifndef NDEBUG
	SphWordID_t m_iLastWordID = 0;
	CSphFixedVector<BYTE> m_dLastKeyword { 0 };
#endif

	int m_iFile = -1;				   ///< my file
	SphOffset_t* m_pFilePos = nullptr; ///< shared current offset in file

public:
	SphOffset_t m_iFilePos = 0; ///< my current offset in file
	int m_iFileLeft = 0;		///< how much data is still unread from the file

public:
	explicit CSphBin ( ESphHitless eMode = SPH_HITLESS_NONE, DictFormat_e eDictFormat = DictFormat_e::CRC );

	static int CalcBinSize ( int iMemoryLimit, int iBlocks, const char* sPhase );
	bool IsWordDict() const { return m_eDictFormat!=DictFormat_e::CRC; }
	void Init ( int iFD, SphOffset_t* pSharedOffset, const int iBinSize );

	SphWordID_t ReadVLB();
	int ReadByte();
	ESphBinRead ReadBytes ( void* pDest, int iBytes );
	int ReadHit ( AggregateHit_t* pOut );

	DWORD UnzipInt();
	SphOffset_t UnzipOffset();

	bool IsEOF() const;
	bool IsDone() const;
	bool IsError() const
	{
		return m_bError;
	}
	ESphBinRead Precache();
};
