//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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
	virtual			~DebugCheckReader_i () {};

	virtual int64_t	GetLengthBytes () = 0;
	virtual bool	GetBytes ( void * pData, int iSize ) = 0;
	virtual bool	SeekTo ( int64_t iOff, int iHint ) = 0;
};


// simple error reporter for debug checks
class DebugCheckError_c
{
public:
			DebugCheckError_c ( FILE * pFile );

	bool	Fail ( const char * szFmt, ... );
	void	Msg ( const char * szFmt, ... );
	void	Progress ( const char * szFmt, ... );
	void	Done();

	void	SetSegment ( int iSegment );
	int64_t	GetNumFails() const;

private:
	FILE *	m_pFile {nullptr};
	bool	m_bProgress {false};
	int64_t m_tStartTime {0};
	int64_t	m_nFails {0};
	int64_t	m_nFailsPrinted {0};
	int		m_iSegment {-1};
};


// common code for debug checks in RT and disk indexes
class DebugCheckHelper_c
{
protected:
	void	DebugCheck_Attributes ( DebugCheckReader_i & tAttrs, DebugCheckReader_i & tBlobs, int64_t nRows, int64_t iMinMaxBytes, const CSphSchema & tSchema, DebugCheckError_c & tReporter ) const;
	void	DebugCheck_DeadRowMap (  int64_t iSizeBytes, int64_t nRows, DebugCheckError_c & tReporter ) const;
};


// disk index checker
class DiskIndexChecker_i
{
public:
	virtual			~DiskIndexChecker_i() = default;

	virtual bool	OpenFiles ( CSphString & sError ) = 0;
	virtual void	Setup ( int64_t iNumRows, int64_t iDocinfoIndex, int64_t iMinMaxIndex, bool bCheckIdDups ) = 0;
	virtual CSphVector<SphWordID_t> & GetHitlessWords() = 0;

	virtual void	Check() = 0;
};


DiskIndexChecker_i * CreateDiskIndexChecker ( CSphIndex & tIndex, DebugCheckError_c & tReporter );

#endif // _indexcheck_
