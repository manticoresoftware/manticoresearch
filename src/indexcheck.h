//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _indexcheck_
#define _indexcheck_

#include "sphinx.h"

class DebugCheckReader_i
{
public:
	virtual			~DebugCheckReader_i () = default;

	virtual int64_t	GetLengthBytes () = 0;
	virtual bool	GetBytes ( void * pData, int iSize ) = 0;
	virtual bool	SeekTo ( int64_t iOff, int iHint ) = 0;
};


// simple error reporter for debug checks
class DebugCheckError_i
{
public:
	virtual ~DebugCheckError_i() = default;

	virtual bool Fail ( const char* szFmt, ... ) = 0;
	virtual void Msg ( const char* szFmt, ... ) = 0;
	virtual void Progress ( const char* szFmt, ... ) = 0;
	virtual void Done() = 0;
	virtual int64_t GetNumFails() const = 0;
	virtual const DocID_t* GetExtractDocs () const {return nullptr;};
};

DebugCheckError_i* MakeDebugCheckError ( FILE* fp, DocID_t* pExtract );

// common code for debug checks in RT and disk indexes
class DebugCheckHelper_c
{
protected:
	void	DebugCheck_Attributes ( DebugCheckReader_i & tAttrs, DebugCheckReader_i & tBlobs, int64_t nRows, int64_t iMinMaxBytes, const CSphSchema & tSchema, DebugCheckError_i & tReporter ) const;
	void	DebugCheck_DeadRowMap (  int64_t iSizeBytes, int64_t nRows, DebugCheckError_i & tReporter ) const;
};


// disk index checker
class DiskIndexChecker_c
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

public:
			DiskIndexChecker_c ( CSphIndex& tIndex, DebugCheckError_i& tReporter );
			~DiskIndexChecker_c();

	bool	OpenFiles ();
	void	Setup ( int64_t iNumRows, int64_t iDocinfoIndex, int64_t iMinMaxIndex, bool bCheckIdDups );
	CSphVector<SphWordID_t> & GetHitlessWords();
	void	Check();
};

void DebugCheckSchema ( const ISphSchema & tSchema, DebugCheckError_i & tReporter );
bool DebugCheckSchema ( const ISphSchema & tSchema, CSphString & sError );
bool SchemaConfigureCheckAttribute ( const CSphSchema & tSchema, const CSphColumnInfo & tCol, CSphString & sError );

#endif // _indexcheck_
