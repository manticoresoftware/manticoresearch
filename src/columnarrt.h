//
// Copyright (c) 2021, Manticore Software LTD (http://manticoresearch.com)
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
	virtual void		Save ( CSphWriter & tWriter ) = 0;
};

class MemoryReader_c;
class MemoryWriter_c;
class ColumnarAttrRT_i;

class ColumnarBuilderRT_i : public columnar::Builder_i
{
public:
	virtual void		Kill ( const CSphVector<RowID_t> & dKilled ) = 0;
	virtual void		Save ( MemoryWriter_c & tWriter ) = 0;
	virtual CSphVector<ColumnarAttrRT_i*> & GetAttrs() = 0;
};

// used in accumulator. loads whole storage (no schema to save memory)
ColumnarBuilderRT_i *	CreateColumnarBuilderRT ( MemoryReader_c & tReader );

// create an empty builder, create storage from schema
ColumnarBuilderRT_i *	CreateColumnarBuilderRT ( const CSphSchema & tSchema );

// initialize columnar from accumulator; setup schema
// columnar reader can either take ownership of attributes in columnar builder (and remove them from builder) or not
ColumnarRT_i *			CreateColumnarRT ( const CSphSchema & tSchema, ColumnarBuilderRT_i * pBuilder, bool bTakeOwnership=true );

// used by ram segments and binlog
ColumnarRT_i *			CreateColumnarRT ( const CSphSchema & tSchema, CSphReader & tReader, CSphString & sError );

#endif // _columnarrt_
