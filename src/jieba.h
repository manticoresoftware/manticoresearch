//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "indexsettings.h"

#if WITH_JIEBA

bool CheckConfigJieba ( CSphIndexSettings & tSettings, CSphString & sError );
bool StrToJiebaMode ( JiebaMode_e & eMode, const CSphString & sValue, CSphString & sError );
bool SpawnFilterJieba ( std::unique_ptr<ISphFieldFilter> & pFieldFilter, const CSphIndexSettings & tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, FilenameBuilder_i * pFilenameBuilder, CSphString & sError );

#else


inline bool CheckConfigJieba ( CSphIndexSettings & tSettings, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor==Preprocessor_e::JIEBA )
	{
		tSettings.m_ePreprocessor = Preprocessor_e::NONE;
		sError.SetSprintf ( "Jieba options specified, but no Jieba support compiled; ignoring" );
		return false;
	}

	return true;
}

inline bool SpawnFilterJieba ( std::unique_ptr<ISphFieldFilter> &, const CSphIndexSettings &, const CSphTokenizerSettings &,
		const char *, CSphString & )
{
	return true;
}

#endif
