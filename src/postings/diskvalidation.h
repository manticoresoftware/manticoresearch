// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include <cstdint>

class CSphSchema;
namespace bson { class Bson_c; }

namespace postings
{

// Check persisted field-length statistics against the schema and document count.
bool CheckNorms ( const bson::Bson_c & tNorms, const CSphSchema & tSchema, int64_t iDocs, bool bFieldLengths );

}
