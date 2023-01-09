//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "html_stripper_tag.h"

/// HTML stripper
class CSphHTMLStripper
{
public:
	explicit CSphHTMLStripper ( bool bDefaultTags );
	bool SetIndexedAttrs ( const char* sConfig, CSphString& sError );
	bool SetRemovedElements ( const char* sConfig, CSphString& sError );
	bool SetZones ( const char* sZones, CSphString& sError );
	void EnableParagraphs();
	void Strip ( BYTE* sData ) const;

public:
	/// finds appropriate tag and zone name ( tags zone name could be prefix only )
	/// advances source to the end of the tag
	const BYTE* FindTag ( const BYTE* sSrc, const html_stripper::StripperTag_t** ppTag, const BYTE** ppZoneName, int* pZoneNameLen ) const;
	bool IsValidTagStart ( int iCh ) const;

private:
	static const int MAX_CHAR_INDEX = 28; ///< max valid char index (a-z, underscore, colon)

	CSphVector<html_stripper::StripperTag_t> m_dTags; ///< known tags to index attrs and/or to remove contents
	int m_dStart[MAX_CHAR_INDEX];	   ///< maps index of the first tag name char to start offset in m_dTags
	int m_dEnd[MAX_CHAR_INDEX];		   ///< maps index of the first tag name char to end offset in m_dTags

	int GetCharIndex ( int iCh ) const; ///< calcs index by raw char
	void UpdateTags();					///< sorts tags, updates internal helpers
};
