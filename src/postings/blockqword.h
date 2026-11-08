// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "indexformat.h"
#include "stream.h"

class BlockIndexQword_c : public DiskIndexQwordTraits_c, private postings::Input_i
{
public:
	using is_worddict = std::true_type;

	BlockIndexQword_c ( bool bMini, bool bExcluded, int64_t iIndexId );

	void				Reset() override;
	void				SetReadErrorTarget ( CSphString * pError ) override	{ m_pQueryError = pError; }
	bool				Setup ( const DiskIndexQwordSetup_c * pSetup ) override;
	void				SeekDocReader ( SphOffset_t iPos, int iHint ) override;
	void				SeekHitReader ( SphOffset_t iPos, int iHint ) override;
	SphOffset_t			GetDocReaderPos() const override;
	const CSphMatch &	GetNextDoc() override;
	int					GetNextDocs ( QwordDoc_t * pDocs, int iMax ) override;
	RowID_t				AdvanceTo ( RowID_t tRow ) override;
	bool				HintRowID ( RowID_t tRow ) override;
	void				SeekHitlist ( SphOffset_t uOffset ) override;
	Hitpos_t			GetNextHit() override;
	void				SetPayloadRequirements ( bool bTF, bool bHits ) override	{ m_bReadTF = bTF || bHits; m_bReadHits = bHits; }
	const char *		GetReadError() const override	{ return m_sError.empty() ? nullptr : m_sError.c_str(); }

	const std::array<uint64_t,postings::COLUMNS> & GetDecodeCounts() const { return m_tReader.GetDecodeCounts(); }

	std::unique_ptr<PostingWindow_i> CreatePostingWindow ( DWORD uFields, const PostingWindowContext_t & tContext ) override;
	std::unique_ptr<PostingBlock_i> CreatePostingBlock ( DWORD uFields, const PostingWindowContext_t & tContext ) override;

private:
	friend class BlockPostingWindow_c;
	uint64_t		GetSize() const final		{ return m_iDocFileSize; }
	const uint8_t *	GetMappedData() const final	{ return m_pDocMap; }
	void			OnDocReaderChanged() final;
	bool			Read ( uint64_t uOffset, uint8_t * pData, size_t iSize, std::string & sError ) final;
	bool			LoadLeaf ( RowID_t tMin );
	bool			PositionNext();
	void			SetDoc ( unsigned i );
	template<bool TF, bool HITS>
	int				ReadDocs ( QwordDoc_t * pDocs, int iMax );
	void			Fail();
	void			ResetCursor();

	postings::Reader_c				m_tReader;
	const BYTE *					m_pDocMap = nullptr;
	std::string						m_sError;
	CSphString *					m_pQueryError = nullptr;

	SphOffset_t	m_iStart = 0;
	SphOffset_t	m_iNext = 0;
	unsigned	m_iOrdinal = 0;
	RowID_t		m_tHint = 0;
	bool		m_bHitEOF = true;
	bool		m_bOpen = false;
	bool		m_bEOF = false;
	bool		m_bReadTF = true;
	bool		m_bReadHits = true;
};
