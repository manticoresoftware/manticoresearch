//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxstem_
#define _sphinxstem_

#include "sphinx.h"

/// initialize English stemmar
void	stem_en_init ();

/// initialize Russian stemmar
void	stem_ru_init ();

/// stem lowercase English word
void	stem_en ( BYTE * pWord, int iLen );

/// stem lowercase Russian word in Windows-1251 encoding
void	stem_ru_cp1251 ( BYTE * pWord );

/// stem lowercase Russian word in UTF-8 encoding
void	stem_ru_utf8 ( WORD * pWord );

/// initialize Czech stemmer
void	stem_cz_init ();

/// stem lowercase Czech word
void	stem_cz ( BYTE * pWord );

/// stem Arabic word in UTF-8 encoding
void	stem_ar_utf8 ( BYTE * word );

/// calculate soundex in-place if the word is lowercase English letters only;
/// do nothing if it's not
void	stem_soundex ( BYTE * pWord );

/// double metaphone stemmer
void	stem_dmetaphone ( BYTE * pWord, bool bUTF8 );

/// pre-init AOT setup, cache size (in bytes)
void	sphAotSetCacheSize ( int iCacheSize );

/// init AOT lemmatizer
bool	sphAotInitRu ( const CSphString & sDictFile, CSphString & sError );

/// lemmatize (or guess a normal form) a Russian word in Windows-1251 encoding
void	sphAotLemmatizeRu1251 ( BYTE * pWord );

/// lemmatize (or guess a normal form) a Russian word in UTF-8 encoding, return a single "best" lemma
void	sphAotLemmatizeRuUTF8 ( BYTE * pWord );

/// lemmatize (or guess a normal form) a Russian word, return all lemmas
void	sphAotLemmatizeRu ( CSphVector<CSphString> & dLemmas, const BYTE * pWord, bool bUtf8 );

/// get lemmatizer dictionary info (file name, crc)
const CSphNamedInt &	sphAotDictinfoRu();

/// create token filter that returns all morphological hypotheses
/// NOTE, takes over wordforms from pDict, in AOT case they must be handled by the fitler
class CSphTokenFilter;
CSphTokenFilter *		sphAotCreateFilter ( ISphTokenizer * pTokenizer, CSphDict * pDict );

#endif // _sphinxstem_

//
// $Id$
//
