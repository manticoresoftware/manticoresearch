//
// Copyright (c) 2020-2022, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _secondarylib_
#define _secondarylib_

#include "secondary/builder.h"
#include "secondary/iterator.h"

// FWD
struct CSphString;

bool			InitSecondary ( CSphString & sError );
void			ShutdownSecondary();
const char *	GetSecondaryVersionStr();
int				GetSecondaryStorageVersion();
bool			IsSecondaryLibLoaded();

SI::Index_i *		CreateSecondaryIndex ( const char * sFile, CSphString & sError );
std::unique_ptr<SI::Builder_i>		CreateSecondaryIndexBuilder ( const std::vector<SI::SourceAttrTrait_t> & dSrcAttrs, int iMemoryLimit, SI::Collation_e eCollation, const char * sFile, CSphString & sError );

void				CollationInitSecondaryIndex ( const std::array<SI::StrHash_fn, (size_t)SI::Collation_e::TOTAL> & dCollations );
void				SetSecondaryIndexDefault ( bool bEnabled );
bool				GetSecondaryIndexDefault ();

#endif // _secondarylib_
