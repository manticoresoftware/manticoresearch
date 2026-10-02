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

#include "sphinx.h"

class RtFieldNorms_c
{
public:
	RtFieldNorms_c() = default;
	explicit RtFieldNorms_c ( const CSphSchema & tSchema );
	void Reset ( const CSphSchema & tSchema );

	int Fields() const;
	int DenseFields() const;
	int DenseIndex ( int iSchemaField ) const;
	int SchemaField ( int iDenseField ) const;
	DWORD Get ( const DWORD * pDenseRow, int iSchemaField ) const;
	void Compact ( const DWORD * pSchemaRow, DWORD * pDenseRow ) const;
	void Expand ( const DWORD * pDenseRow, DWORD * pSchemaRow ) const;

private:
	CSphVector<int> m_dSchemaToDense;
	CSphVector<int> m_dDenseToSchema;
};

// Validate schema-wide compatibility serialization without allocating the expanded vector.
bool ValidateRtSchemaWideNorms ( int64_t iDenseCount, DWORD uRows, int iSchemaFields, int iDenseFields, CSphString & sError );
