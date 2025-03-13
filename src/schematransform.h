//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxsort.h"

class TransformedSchemaBuilder_c
{
public:
			TransformedSchemaBuilder_c ( const ISphSchema & tOldSchema, CSphSchema & tNewSchema );

	void	AddAttr ( const CSphString & sName );
	void	Finalize();

private:
	const ISphSchema &	m_tOldSchema;
	CSphSchema &		m_tNewSchema;

	void	ReplaceColumnarAttrWithExpression ( CSphColumnInfo & tAttr, int iLocator );
};


void				RemapNullMask ( VecTraits_T<CSphMatch> & dMatches, const CSphSchema & tOldSchema, CSphSchema & tNewSchema );
int					GetStringRemapCount ( const ISphSchema & tDstSchema, const ISphSchema & tSrcSchema );
MatchProcessor_i *	CreateMatchSchemaTransform ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, GetBlobPoolFromMatch_fn fnGetBlobPool, GetColumnarFromMatch_fn fnGetColumnar );
