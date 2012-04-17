//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxrt_
#define _sphinxrt_

#include "sphinx.h"
#include "sphinxutils.h"

/// RAM based updateable backend interface
class ISphRtIndex : public CSphIndex
{
public:
	explicit ISphRtIndex ( const char * sIndexName, const char * sName ) : CSphIndex ( sIndexName, sName ) {}

	/// get internal schema (to use for Add calls)
	virtual const CSphSchema & GetInternalSchema () const { return m_tSchema; }

	/// insert/update document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool AddDocument ( int iFields, const char ** ppFields, const CSphMatch & tDoc, bool bReplace, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError, CSphString & sWarning ) = 0;

	/// insert/update document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool AddDocument ( ISphHits * pHits, const CSphMatch & tDoc, const char ** ppStr, const CSphVector<DWORD> & dMvas, CSphString & sError ) = 0;

	/// delete document in current txn
	/// fails in case of two open txns to different indexes
	virtual bool DeleteDocument ( const SphDocID_t * pDocs, int iDocs, CSphString & sError ) = 0;

	/// commit pending changes
	virtual void Commit () = 0;

	/// undo pending changes
	virtual void RollBack () = 0;

	/// check and periodically flush RAM chunk to disk
	virtual void CheckRamFlush () = 0;

	/// forcibly flush RAM chunk to disk
	virtual void ForceRamFlush ( bool bPeriodic=false ) = 0;

	/// forcibly save RAM chunk as a new disk chunk
	virtual void ForceDiskChunk () = 0;

	/// attach a disk chunk to current index
	virtual bool AttachDiskIndex ( CSphIndex * pIndex, CSphString & sError ) = 0;

	/// truncate index (that is, kill all data)
	virtual bool Truncate ( CSphString & sError ) = 0;

	virtual void Optimize ( volatile bool * pForceTerminate, ThrottleState_t * pThrottle ) = 0;
};

/// initialize subsystem
class CSphConfigSection;
void sphRTInit ();
void sphRTConfigure ( const CSphConfigSection & hSearchd, bool bTestMode );
bool sphRTSchemaConfigure ( const CSphConfigSection & hIndex, CSphSchema * pSchema, CSphString * pError );

/// deinitialize subsystem
void sphRTDone ();

/// RT index factory
ISphRtIndex * sphCreateIndexRT ( const CSphSchema & tSchema, const char * sIndexName, int64_t iRamSize, const char * sPath, bool bKeywordDict );

/// Get current txn index
ISphRtIndex * sphGetCurrentIndexRT();

typedef void ProgressCallbackSimple_t ();

//////////////////////////////////////////////////////////////////////////

enum ESphBinlogReplayFlags
{
	SPH_REPLAY_ACCEPT_DESC_TIMESTAMP = 1
};

/// replay stored binlog
void sphReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, DWORD uReplayFlags, ProgressCallbackSimple_t * pfnProgressCallback=NULL );

#endif // _sphinxrt_

//
// $Id$
//
