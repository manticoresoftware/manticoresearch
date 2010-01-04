//
// $Id$
//

#ifndef _sphinxint_
#define _sphinxint_

#include "sphinx.h"
#include "sphinxfilter.h"

#include <sys/stat.h>
#include <fcntl.h>

//////////////////////////////////////////////////////////////////////////

#ifdef O_BINARY
#define SPH_O_BINARY O_BINARY
#else
#define SPH_O_BINARY 0
#endif

#define SPH_O_READ	( O_RDONLY | SPH_O_BINARY )
#define SPH_O_NEW	( O_CREAT | O_RDWR | O_TRUNC | SPH_O_BINARY )

/// file writer with write buffering and int encoder
class CSphWriter : ISphNoncopyable
{
public:
					CSphWriter ();
					~CSphWriter ();

	void			SetBufferSize ( int iBufferSize );	///< tune write cache size; must be called before OpenFile() or SetFile()

	bool			OpenFile ( const CSphString & sName, CSphString & sErrorBuffer );
	void			SetFile ( int iFD, SphOffset_t * pSharedOffset );
	void			CloseFile ( bool bTruncate = false );	///< note: calls Flush(), ie. IsError() might get true after this call

	void			PutByte ( int uValue );
	void			PutBytes ( const void * pData, int iSize );
	void			PutDword ( DWORD uValue ) { PutBytes ( &uValue, sizeof(DWORD) ); }
	void			PutOffset ( SphOffset_t uValue ) { PutBytes ( &uValue, sizeof(SphOffset_t) ); }
	void			PutString ( const char * szString );

	void			SeekTo ( SphOffset_t pos ); ///< seeking inside the buffer will truncate it

#if USE_64BIT
	void			PutDocid ( SphDocID_t uValue ) { PutOffset ( uValue ); }
#else
	void			PutDocid ( SphDocID_t uValue ) { PutDword ( uValue ); }
#endif

	void			ZipInt ( DWORD uValue );
	void			ZipOffset ( SphOffset_t uValue );
	void			ZipOffsets ( CSphVector<SphOffset_t> * pData );

	bool			IsError () const	{ return m_bError; }
	SphOffset_t		GetPos () const		{ return m_iPos; }

private:
	CSphString		m_sName;
	SphOffset_t		m_iPos;
	SphOffset_t		m_iWritten;

	int				m_iFD;
	int				m_iPoolUsed;
	BYTE *			m_pBuffer;
	BYTE *			m_pPool;
	bool			m_bOwnFile;
	SphOffset_t	*	m_pSharedOffset;
	int				m_iBufferSize;

	bool			m_bError;
	CSphString *	m_pError;

	void			Flush ();
};

/// forward ref
class CSphAutofile;

/// file reader with read buffering and int decoder
class CSphReader_VLN
{
public:
	CSphReader_VLN ( BYTE * pBuf=NULL, int iSize=0 );
	virtual		~CSphReader_VLN ();

	void		SetBuffers ( int iReadBuffer, int iReadUnhinted );
	void		SetFile ( int iFD, const char * sFilename );
	void		SetFile ( const CSphAutofile & tFile );
	void		Reset ();
	void		SeekTo ( SphOffset_t iPos, int iSizeHint );

	void		SkipBytes ( int iCount );
	SphOffset_t	GetPos () const { return m_iPos+m_iBuffPos; }

	void		GetBytes ( void * pData, int iSize );
	int			GetBytesZerocopy ( const BYTE ** ppData, int iMax ); ///< zerocopy method; returns actual length present in buffer (upto iMax)

	int			GetByte ();
	DWORD		GetDword ();
	SphOffset_t	GetOffset ();
	CSphString	GetString ();
	int			GetLine ( char * sBuffer, int iMaxLen );

	DWORD		UnzipInt ();
	SphOffset_t	UnzipOffset ();

	SphOffset_t				Tell () const				{ return m_iPos + m_iBuffPos; }
	bool					GetErrorFlag () const		{ return m_bError; }
	const CSphString &		GetErrorMessage () const	{ return m_sError; }
	const CSphString &		GetFilename() const			{ return m_sFilename; }

#if USE_64BIT
	SphDocID_t	GetDocid ()		{ return GetOffset(); }
	SphDocID_t	UnzipDocid ()	{ return UnzipOffset(); }
	SphWordID_t	UnzipWordid ()	{ return UnzipOffset(); }
#else
	SphDocID_t	GetDocid ()		{ return GetDword(); }
	SphDocID_t	UnzipDocid ()	{ return UnzipInt(); }
	SphWordID_t	UnzipWordid ()	{ return UnzipInt(); }
#endif

	const CSphReader_VLN &	operator = ( const CSphReader_VLN & rhs );

protected:

	int			m_iFD;
	SphOffset_t	m_iPos;

	int			m_iBuffPos;
	int			m_iBuffUsed;
	BYTE *		m_pBuff;
	int			m_iSizeHint;	///< how much do we expect to read

	int			m_iBufSize;
	bool		m_bBufOwned;
	int			m_iReadUnhinted;

	bool		m_bError;
	CSphString	m_sError;
	CSphString	m_sFilename;

private:
	void		UpdateCache ();
};

/// scoped reader 
class CSphAutoreader : public CSphReader_VLN
{
public:
				CSphAutoreader ( BYTE * pBuf=NULL, int iSize=0 ) : CSphReader_VLN ( pBuf, iSize ) {}
				~CSphAutoreader ();

	bool		Open ( const CSphString & sFilename, CSphString & sError );
	void		Close ();
	SphOffset_t	GetFilesize ();

public:
	// added for DebugCheck()
	int			GetFD () { return m_iFD; }
};

//////////////////////////////////////////////////////////////////////////

/// per-query search context
/// everything that index needs to compute/create to process the query
class CSphQueryContext
{
public:
	// searching-only, per-query
	int							m_iWeights;						///< search query field weights count
	int							m_dWeights [ SPH_MAX_FIELDS ];	///< search query field weights

	bool						m_bEarlyLookup;			///< whether early attr value lookup is needed
	bool						m_bLateLookup;			///< whether late attr value lookup is needed

	ISphFilter *				m_pFilter;
	ISphFilter *				m_pWeightFilter;

	struct CalcItem_t
	{
		CSphAttrLocator			m_tLoc;					///< result locator
		DWORD					m_uType;				///< result type
		ISphExpr *				m_pExpr;				///< evaluator (non-owned)
	};
	CSphVector<CalcItem_t>		m_dEarlyCalc;			///< early-calc evaluators
	CSphVector<CalcItem_t>		m_dLateCalc;			///< late-calc evaluators

	const CSphVector<CSphAttrOverride> *	m_pOverrides;		///< overridden attribute values
	CSphVector<CSphAttrLocator>				m_dOverrideIn;
	CSphVector<CSphAttrLocator>				m_dOverrideOut;

	void *						m_pIndexData;			///< backend specific data

public:
	CSphQueryContext ();
	~CSphQueryContext ();

	void						BindWeights ( const CSphQuery * pQuery, const CSphSchema & tSchema );
	bool						SetupCalc ( CSphQueryResult * pResult, const CSphSchema & tInSchema, const CSphSchema & tSchema, const DWORD * pMvaPool );
	bool						CreateFilters ( bool bFullscan, const CSphVector<CSphFilterSettings> * pdFilters, const CSphSchema & tSchema, const DWORD * pMvaPool, CSphString & sError );
	bool						SetupOverrides ( const CSphQuery * pQuery, CSphQueryResult * pResult, const CSphSchema & tIndexSchema );

	void						EarlyCalc ( CSphMatch & tMatch ) const;
	void						LateCalc ( CSphMatch & tMatch ) const;
};

bool sphWriteThrottled ( int iFD, const void * pBuf, int64_t iCount, const char * sName, CSphString & sError );

void SafeClose ( int & iFD );

#endif // _sphinxint_

//
// $Id$
//
