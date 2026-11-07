// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "diskvalidation.h"
#include "stream.h"
#include "schema/schema.h"
#include "sphinxjson.h"

namespace postings
{

bool CheckNorms ( const bson::Bson_c & tNorms, const CSphSchema & tSchema, int64_t iDocs, bool bFieldLengths )
{
	using namespace bson;

	bool bValid = tSchema.GetFieldsCount()<=int(MAX_FIELDS) && Int(tNorms.ChildByName("version"))==1 && String(tNorms.ChildByName("storage"))=="field_lengths" && bFieldLengths;
	int iFirstNorm = tSchema.GetAttrId_FirstFieldLen();
	bValid &= !tSchema.GetFieldsCount() || (iFirstNorm>=0 && iFirstNorm+tSchema.GetFieldsCount()<=tSchema.GetAttrsCount());
	for ( int f=0; bValid && f<tSchema.GetFieldsCount(); ++f )
		bValid &= tSchema.GetAttr(iFirstNorm+f).m_eAttrType==SPH_ATTR_TOKENCOUNT;

	std::array<uint64_t,MAX_FIELDS> dDocs {};
	int iField = 0;
	Bson_c(tNorms.ChildByName("field_docs")).ForEach ( [&] ( const NodeHandle_t & tNode )
	{
		if ( iField<tSchema.GetFieldsCount() && iField<int(MAX_FIELDS) && Int(tNode)>=0 && uint64_t(Int(tNode))<=uint64_t(iDocs) )
			dDocs[iField++] = Int(tNode);
		else
			bValid = false;
	} );
	bValid &= iField==tSchema.GetFieldsCount();
	iField = 0;
	Bson_c(tNorms.ChildByName("field_tokens")).ForEach ( [&] ( const NodeHandle_t & tNode )
	{
		if ( iField<tSchema.GetFieldsCount() && iField<int(MAX_FIELDS) && Int(tNode)>=0 )
			bValid &= uint64_t(Int(tNode))>=dDocs[iField++];
		else
			bValid = false;
	} );
	return bValid && iField==tSchema.GetFieldsCount();
}

}
