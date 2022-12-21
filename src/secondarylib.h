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
#include "secondary/secondary.h"

// FWD
struct CSphString;

bool			InitSecondary ( CSphString & sError );
void			ShutdownSecondary();
const char *	GetSecondaryVersionStr();
int				GetSecondaryStorageVersion();
bool			IsSecondaryLibLoaded();

SI::Index_i *		CreateSecondaryIndex ( const char * sFile, CSphString & sError );
std::unique_ptr<SI::Builder_i>	CreateSecondaryIndexBuilder ( const common::Schema_t & tSchema, int iMemoryLimit, const CSphString & sFile, CSphString & sError );

enum class SIDefault_e
{
	DISABLED,
	ENABLED,
	FORCE
};

void				SetSecondaryIndexDefault ( SIDefault_e eState );
SIDefault_e			GetSecondaryIndexDefault ();

#endif // _secondarylib_
