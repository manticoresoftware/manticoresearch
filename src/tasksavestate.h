//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
/// @file tasksavestate.h
/// On-demand save sphinxql state and user variables

#ifndef MANTICORE_TASKSAVESTATE_H
#define MANTICORE_TASKSAVESTATE_H

#include "sphinxint.h"

//////////////////////////////////////////////////////////////////////////
/// available uservar types
enum Uservar_e
{
	USERVAR_INT_SET
};

/// uservar name to value binding
struct Uservar_t
{
	Uservar_e m_eType { USERVAR_INT_SET };
	UservarIntSet_c m_pVal;
};

// create or update the variable
void SetLocalUserVar ( const CSphString& sName, CSphVector<SphAttr_t>& dSetValues );

// provide variable to outside
void ServeUserVars();

void SphinxqlStateFlush ();
bool InitSphinxqlState ( CSphString dStateFilePath, CSphString& sError);

#endif //MANTICORE_TASKSAVESTATE_H
