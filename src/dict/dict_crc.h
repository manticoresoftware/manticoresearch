//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "template_dict_traits.h"
#include "crc_engine.h"
#include "indexformat.h"

/////////////////////////////////////////////////////////////////////////////
// CRC32/64 DICTIONARIES
/////////////////////////////////////////////////////////////////////////////


/// common CRC32/64 dictionary stuff
struct DiskDictTraits_c: TemplateDictTraits_c
{
	void DictBegin ( CSphAutofile& tTempDict, CSphAutofile& tDict, int iDictLimit ) override;
	void DictEntry ( const DictEntry_t& tEntry ) override;
	void DictEndEntries ( SphOffset_t iDoclistOffset ) override;
	bool DictEnd ( DictHeader_t* pHeader, int iMemLimit, CSphString& sError ) override;
	bool DictIsError() const final { return m_wrDict.IsError(); }
	void SetSkiplistBlockSize ( int iSize ) final { m_iSkiplistBlockSize = iSize; }

protected:
	~DiskDictTraits_c() override = default; // fixme! remove

protected:
	CSphTightVector<CSphWordlistCheckpoint> m_dCheckpoints; ///< checkpoint offsets
	CSphWriter m_wrDict;									///< final dict file writer
	CSphString m_sWriterError;								///< writer error message storage
	int m_iEntries = 0;										///< dictionary entries stored
	SphOffset_t m_iLastDoclistPos = 0;
	SphWordID_t m_iLastWordID = 0;
	int m_iSkiplistBlockSize = 0;
};

/// specialized CRC32/64 implementations
template<CRCALGO ALGO>
struct CSphDictCRC: public DiskDictTraits_c
	, CCRCEngine<ALGO>
{
	typedef CCRCEngine<ALGO> tHASH;
	SphWordID_t GetWordID ( BYTE* pWord ) override;
	SphWordID_t GetWordID ( const BYTE* pWord, int iLen, bool bFilterStops ) override;
	SphWordID_t GetWordIDWithMarkers ( BYTE* pWord ) override;
	SphWordID_t GetWordIDNonStemmed ( BYTE* pWord ) override;
	bool IsStopWord ( const BYTE* pWord ) const final;

	DictRefPtr_c Clone() const override { return CloneBase ( new CSphDictCRC<ALGO>() ); }

protected:
	~CSphDictCRC() override = default; // fixme! remove
};

#include "dict_crc_impl.h"
