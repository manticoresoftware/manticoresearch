//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarrt_
#define _columnarrt_

#include <cstdint>
#include "sphinxdefs.h"
#include "sphinxstd.h"
#include "columnarlib.h"
#include "fileio.h"

class CSphSchema;

class ColumnarRT_i : public columnar::Columnar_i
{
public:
	virtual int64_t		AllocatedBytes() const = 0;
	virtual void		Save ( Writer_i & tWriter ) = 0;
};

class MemoryReader_c;
class MemoryWriter_c;
class ColumnarAttrRT_i;

class ColumnarBuilderRT_i : public columnar::Builder_i
{
public:
	virtual void		Save ( MemoryWriter_c & tWriter ) = 0;
	virtual CSphVector<std::unique_ptr<ColumnarAttrRT_i>> & GetAttrs() = 0;
	virtual const CSphVector<std::unique_ptr<ColumnarAttrRT_i>>& GetAttrs() const = 0;
};

// used in accumulator. loads whole storage (no schema to save memory)
std::unique_ptr<ColumnarBuilderRT_i>	CreateColumnarBuilderRT ( MemoryReader_c& tReader );

// create an empty builder, create storage from schema
std::unique_ptr<ColumnarBuilderRT_i>	CreateColumnarBuilderRT ( const CSphSchema & tSchema );

// initialize columnar from accumulator; setup schema
// columnar reader will take ownership of attributes in columnar builder (and remove them from builder)
std::unique_ptr<ColumnarRT_i>			CreateColumnarRT ( const CSphSchema & tSchema, ColumnarBuilderRT_i* pBuilder );

// columnar reader will NOT take ownership of attributes in columnar builder
std::unique_ptr<ColumnarRT_i>			CreateLightColumnarRT ( const CSphSchema& tSchema, const ColumnarBuilderRT_i* pBuilder );

// used by ram segments and binlog
std::unique_ptr<ColumnarRT_i>			CreateColumnarRT ( const CSphSchema & tSchema, CSphReader & tReader, CSphString & sError );

void RemoveColumnarDuplicates ( std::unique_ptr<ColumnarBuilderRT_i> & pBuilder, const CSphFixedVector<RowID_t> & dRowMap, const CSphSchema & tSchema );

#endif // _columnarrt_
