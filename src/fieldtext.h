//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include <memory>

class CSphHTMLStripper;
class ISphFieldFilter;
class CSphIndexSettings;

// the text a field turns into before it is tokenized or highlighted with html_strip_mode=index: the field filter,
// then the HTML stripper of the table (without the paragraph and zone markers indexing adds).
// Lives in lmanticore so that both the snippet builder (searchd) and the RT insert path (searchd, indexer) share it.

std::unique_ptr<CSphHTMLStripper>	CreateIndexHtmlStripper ( const CSphIndexSettings & tSettings, CSphString & sError );

// applies the filter and the stripper to dSourceText, writing the modified text into dDestText.
// bUseOriginal is in/out: true on entry when dSourceText still holds the text to process (rather than dDestText);
// set to false once dDestText holds the modified text. Returns true when the field filter changed the text.
bool								FilterAndStripText ( const VecTraits_T<BYTE> & dSourceText, CSphVector<BYTE> & dDestText, ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper, bool & bUseOriginal );

// the prepared text of a field, always materialized into dPrepared (a copy of the source when nothing changed it)
void								PrepareFieldText ( const VecTraits_T<const BYTE> & dSource, CSphVector<BYTE> & dPrepared, ISphFieldFilter * pFilter, const CSphHTMLStripper * pStripper );
