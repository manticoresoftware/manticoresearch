//
// $Id$
//

#ifndef _sphinxstd_
#define _sphinxstd_

#if _MSC_VER>=1400
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdlib.h>
#include <assert.h>

/////////////////////////////////////////////////////////////////////////////
// GENERICS
/////////////////////////////////////////////////////////////////////////////

#define Min(a,b)			((a)<(b)?(a):(b))
#define Max(a,b)			((a)>(b)?(a):(b))
#define SafeDelete(_x)		{ if (_x) { delete (_x); (_x) = NULL; } }
#define SafeDeleteArray(_x)	{ if (_x) { delete [] (_x); (_x) = NULL; } }

/// swap
template < typename T > inline void Swap ( T & v1, T & v2 )
{
	T temp = v1;
	v1 = v2;
	v2 = temp;
}

/////////////////////////////////////////////////////////////////////////////
// DEBUGGING
/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#ifndef NDEBUG

void sphAssert ( const char * sExpr, const char * sFile, int iLine );

#undef assert
#define assert(_expr) (void)( (_expr) || ( sphAssert ( #_expr, __FILE__, __LINE__ ), 0 ) )

#endif // !NDEBUG
#endif // USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////

/// compile time error struct
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

namespace Private
{
	template<int x>		struct static_assert_test{};
	template<bool x>	struct SizeError;
	template<>			struct SizeError<true>{};
}

#define STATIC_SIZE_ASSERT(_Type,_ExpSize)	\
	typedef ::Private::static_assert_test<sizeof(::Private::SizeError \
	< (bool) (sizeof(_Type) == (_ExpSize)) >)> static_assert_typedef_

/////////////////////////////////////////////////////////////////////////////

template < typename T >
struct SphLess_T
{
	inline bool operator () ( const T & a, const T & b )
	{
		return a < b;
	}
};


template < typename T >
struct SphGreater_T
{
	inline bool operator () ( const T & a, const T & b )
	{
		return b < a;
	}
};


/// generic vector
/// (don't even ask why it's not std::vector)
template < typename T, int INITIAL_LIMIT=1024 > class CSphVector
{
public:
	/// ctor
	CSphVector ()
		: m_iLength	( 0 )
		, m_iLimit	( 0 )
		, m_pData	( NULL )
	{
	}

	/// dtor
	~CSphVector ()
	{
		Reset ();
	}

	/// add entry
	T & Add ()
	{
		if ( m_iLength>=m_iLimit )
			Grow ( 1+m_iLength );
		return m_pData [ m_iLength++ ];
	}

	/// add entry
	void Add ( const T & tValue )
	{
		if ( m_iLength>=m_iLimit )
			Grow ( 1+m_iLength );
		m_pData [ m_iLength++ ] = tValue;
	}

	/// get last entry
	T & Last ()
	{
		return (*this) [ m_iLength-1 ];
	}

	/// remove element by index
	void Remove ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );

		m_iLength--;
		for ( int i=iIndex; i<m_iLength; i++ )
			m_pData[i] = m_pData[i+1];
	}

	/// remove element by index, swapping it with the tail
	void RemoveFast ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		if ( iIndex!=--m_iLength )
			Swap ( m_pData[iIndex], m_pData[m_iLength] );
	}

	/// remove element by value (warning, linear O(n) search)
	bool RemoveValue ( T tValue )
	{
		for ( int i=0; i<m_iLength; i++ )
		{
			if ( m_pData[i]==tValue )
			{
				Remove ( i );
				return true;
			}
		}
		return false;
	}

	/// grow enough to hold that much entries, if needed
	void Grow ( int iNewLimit )
	{
		// check that we really need to be called
		assert ( iNewLimit>=0 );
		if ( iNewLimit<=m_iLimit )
			return;

		// calc new limit
		if ( !m_iLimit )
			m_iLimit = INITIAL_LIMIT;
		while ( m_iLimit<iNewLimit )
			m_iLimit *= 2;

		// realloc
		// FIXME! optimize for POD case
		T * pNew = new T [ m_iLimit ];
		for ( int i=0; i<m_iLength; i++ )
			pNew[i] = m_pData[i];
		delete [] m_pData;
		m_pData = pNew;
	}

	/// resize
	void Resize ( int iNewLength )
	{
		if ( iNewLength>=m_iLength )
			Grow ( iNewLength );
		m_iLength = iNewLength;
	}

	/// reset
	void Reset ()
	{
		m_iLength = 0;
		m_iLimit = 0;
		SafeDeleteArray ( m_pData );
	}

	/// query current length
	inline int GetLength () const
	{
		return m_iLength;
	}

	/// sort
	void Sort ( int iStart=0, int iEnd=-1 )
	{
		Sort ( SphLess_T<T>(), iStart, iEnd );
	}

	/// reverse sort
	void RSort ( int iStart=0, int iEnd=-1 )
	{
		Sort ( SphGreater_T<T>(), iStart, iEnd );
	}

	/// sort
	template < typename F > void Sort ( F COMP, int iStart=0, int iEnd=-1 )
	{
		if ( m_iLength<2 ) return;
		if ( iStart<0 ) iStart = m_iLength+iStart;
		if ( iEnd<0 ) iEnd = m_iLength+iEnd;
		assert ( iStart<=iEnd );

		int st0[32], st1[32], a, b, k, i, j;
		T x;

		k = 1;
		st0[0] = iStart;
		st1[0] = iEnd;
		while ( k )
		{
			k--;
			i = a = st0[k];
			j = b = st1[k];
			x = m_pData [ (a+b)/2 ]; // FIXME! add better median at least
			while ( a<b )
			{
				while ( i<=j )
				{
					while ( COMP ( m_pData[i], x ) ) i++;
					while ( COMP ( x, m_pData[j] ) ) j--;
					if (i <= j) { Swap ( m_pData[i], m_pData[j] ); i++; j--; }
				}

				if ( j-a>=b-i )
				{
					if ( a<j ) { st0[k] = a; st1[k] = j; k++; }
					a = i;
				} else
				{
					if ( i<b ) { st0[k] = i; st1[k] = b; k++; }
					b = j;
				}
			}
		}
	}

	/// access
	/// FIXME! optimize for POD case
	const T & operator [] ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

	/// access
	/// FIXME! optimize for POD case
	T & operator [] ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

	/// copy
	const CSphVector < T, INITIAL_LIMIT > & operator = ( const CSphVector < T, INITIAL_LIMIT > & rhs )
	{
		Reset ();

		m_iLength = rhs.m_iLength;
		m_iLimit = rhs.m_iLimit;
		m_pData = new T [ m_iLimit ];
		for ( int i=0; i<rhs.m_iLength; i++ )
			m_pData[i] = rhs.m_pData[i];

		return *this;
	}

private:
	int		m_iLength;		///< entries actually used
	int		m_iLimit;		///< entries allocated
	T *		m_pData;		///< entries
};


#define ARRAY_FOREACH(_index,_array) \
	for ( int _index=0; _index<_array.GetLength(); _index++ )

/////////////////////////////////////////////////////////////////////////////

/// simple dynamic hash
/// keeps the order, so Iterate() return the entries in the order they was inserted
template < typename T, typename KEY, typename HASHFUNC, int LENGTH, int STEP >
class CSphOrderedHash
{
protected:
	struct HashEntry_t
	{
		KEY				m_tKey;				///< key, owned by the hash
		T 				m_tValue;			///< data, owned by the hash
		HashEntry_t *	m_pNextByHash;		///< next entry in hash list
		HashEntry_t *	m_pNextByOrder;		///< next entry in the insertion order
	};


protected:
	HashEntry_t *	m_dHash [ LENGTH ];		///< all the hash entries
	HashEntry_t *	m_pFirstByOrder;		///< first entry in the insertion order
	HashEntry_t *	m_pLastByOrder;			///< last entry in the insertion order

protected:
	/// find entry by key
	HashEntry_t * FindByKey ( const KEY & tKey ) const
	{
		unsigned int uHash = ( (unsigned int) HASHFUNC::Hash ( tKey ) ) % LENGTH;
		HashEntry_t * pEntry = m_dHash [ uHash ];

		while ( pEntry )
		{
			if ( pEntry->m_tKey==tKey )
				return pEntry;
			pEntry = pEntry->m_pNextByHash;
		}
		return NULL;
	}

public:
	/// ctor
	CSphOrderedHash ()
		: m_pFirstByOrder ( NULL )
		, m_pLastByOrder ( NULL )
		, m_pIterator ( NULL )
	{
		for ( int i=0; i<LENGTH; i++ )
			m_dHash[i] = NULL;
	}

	/// dtor
	~CSphOrderedHash ()
	{
		Reset ();
	}

	/// reset
	void Reset ()
	{
		HashEntry_t * pKill = m_pFirstByOrder;
		while ( pKill )
		{
			HashEntry_t * pNext = pKill->m_pNextByOrder;
			SafeDelete ( pKill );
			pKill = pNext;
		}

		for ( int i=0; i<LENGTH; i++ )
			m_dHash[i] = 0;

		m_pFirstByOrder = NULL;
		m_pLastByOrder = NULL;
		m_pIterator = NULL;
	}

	/// add new entry
	/// returns true on success
	/// returns false if this key is alredy hashed
	bool Add ( const T & tValue, const KEY & tKey )
	{
		unsigned int uHash = ( (unsigned int) HASHFUNC::Hash ( tKey ) ) % LENGTH;

		// check if this key is already hashed
		HashEntry_t * pEntry = m_dHash [ uHash ];
		HashEntry_t ** ppEntry = &m_dHash [ uHash ];
		while ( pEntry )
		{
			if ( pEntry->m_tKey==tKey )
				return false;

			ppEntry = &pEntry->m_pNextByHash;
			pEntry = pEntry->m_pNextByHash;
		}

		// it's not; let's add the entry
		assert ( !pEntry );
		assert ( !*ppEntry );

		pEntry = new HashEntry_t ();
		pEntry->m_tKey = tKey;
		pEntry->m_tValue = tValue;
		pEntry->m_pNextByHash = NULL;
		pEntry->m_pNextByOrder = NULL;

		*ppEntry = pEntry;

		if ( !m_pFirstByOrder )
			m_pFirstByOrder = pEntry;

		if ( m_pLastByOrder )
		{
			assert ( !m_pLastByOrder->m_pNextByOrder );
			assert ( !pEntry->m_pNextByOrder );
			m_pLastByOrder->m_pNextByOrder = pEntry;
		}
		m_pLastByOrder = pEntry;

		return true;
	}

	/// check if key exists
	bool Exists ( const KEY & tKey ) const
	{
		return FindByKey ( tKey )!=NULL;
	}

	/// get value pointer by key
	T * operator () ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = FindByKey ( tKey );
		return pEntry ? &pEntry->m_tValue : NULL;
	}

	/// get value reference by key, asserting that the key exists in hash
	T & operator [] ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = FindByKey ( tKey );
		assert ( pEntry && "hash missing value in operator []" );

		return pEntry->m_tValue;
	}

	/// copying
	const CSphOrderedHash<T,KEY,HASHFUNC,LENGTH,STEP> & operator = ( const CSphOrderedHash<T,KEY,HASHFUNC,LENGTH,STEP> & rhs )
	{
		Reset ();

		rhs.IterateStart ();
		while ( rhs.IterateNext() )
			Add ( rhs.IterateGet(), rhs.IterateGetKey() );

		return *this;
	}

public:
	/// start iterating
	void IterateStart () const
	{
		m_pIterator = NULL;
	}

	/// go to next existing entry
	bool IterateNext () const
	{
		m_pIterator = m_pIterator ? m_pIterator->m_pNextByOrder : m_pFirstByOrder;
		return m_pIterator!=NULL;
	}

	/// get entry value
	T & IterateGet () const
	{
		assert ( m_pIterator );
		return m_pIterator->m_tValue;
	}

	/// get entry key
	const KEY & IterateGetKey () const
	{
		assert ( m_pIterator );
		return m_pIterator->m_tKey;
	}

private:
	/// current iterator
	mutable HashEntry_t *	m_pIterator;
};

/////////////////////////////////////////////////////////////////////////////

/// immutable C string proxy
struct CSphString
{
protected:
	char *				m_sValue;

private:
	/// safety gap after the string end; for instance, UTF-8 Russian stemmer
	/// which treats strings as 16-bit word sequences needs this in some cases.
	/// note that this zero-filled gap does NOT include trailing C-string zero,
	/// and does NOT affect strlen() as well.
	static const int	SAFETY_GAP	= 4;

public:
	CSphString ()
		: m_sValue ( NULL )
	{
	}

	CSphString ( const CSphString & rhs )
		: m_sValue ( NULL )
	{
		*this = rhs;
	}

	virtual ~CSphString ()
	{
		SafeDeleteArray ( m_sValue );
	}

	const char * cstr () const
	{
		return m_sValue;
	}

	bool operator == ( const CSphString & t ) const
	{
		return strcmp ( m_sValue, t.m_sValue )==0;
	}

	bool operator == ( const char * t ) const
	{
		return strcmp ( m_sValue, t )==0;
	}

	bool operator != ( const CSphString & t ) const
	{
		return strcmp ( m_sValue, t.m_sValue )!=0;
	}

	bool operator != ( const char * t ) const
	{
		return strcmp ( m_sValue, t )!=0;
	}

	CSphString ( const char * sString )
	{
		if ( sString )
		{
			int iLen = 1+strlen(sString);
			m_sValue = new char [ iLen+SAFETY_GAP ];

			strcpy ( m_sValue, sString );
			memset ( m_sValue+iLen, 0, SAFETY_GAP );
		} else
		{
			m_sValue = NULL;
		}
	}

	const CSphString & operator = ( const CSphString & rhs )
	{
		SafeDeleteArray ( m_sValue );
		if ( rhs.m_sValue )
		{
			int iLen = 1+strlen(rhs.m_sValue);
			m_sValue = new char [ iLen+SAFETY_GAP ];

			strcpy ( m_sValue, rhs.m_sValue );
			memset ( m_sValue+iLen, 0, SAFETY_GAP );
		}
		return *this;
	}

	CSphString SubString ( int iStart, int iCount )
	{
		#ifndef NDEBUG
		int iLen = strlen(m_sValue);
		#endif
		assert ( iStart>=0 && iStart<iLen );
		assert ( iCount>0 );
		assert ( (iStart+iCount)>=0 && (iStart+iCount)<=iLen );

		CSphString sRes;
		sRes.m_sValue = new char [ 1+SAFETY_GAP+iCount ];
		strncpy ( sRes.m_sValue, m_sValue+iStart, iCount );
		memset ( sRes.m_sValue+iCount, 0, 1+SAFETY_GAP );
		return sRes;
	}

	void SetBinary ( const char * sValue, int iLen )
	{
		SafeDeleteArray ( m_sValue );
		if ( sValue )
		{
			m_sValue = new char [ 1+SAFETY_GAP+iLen ];
			memcpy ( m_sValue, sValue, iLen );
			memset ( m_sValue+iLen, 0, 1+SAFETY_GAP );
		}
	}

	bool IsEmpty () const
	{
		if ( !m_sValue )
			return true;
		return ( (*m_sValue)=='\0' );
	}
};

/////////////////////////////////////////////////////////////////////////////

/// immutable string/int variant list proxy
struct CSphVariant : public CSphString
{
protected:
	int				m_iValue;

public:
	CSphVariant *	m_pNext;
	bool			m_bTag;

public:
	/// default ctor
	CSphVariant ()
		: CSphString ()
		, m_iValue ( 0 )
		, m_pNext ( NULL )
		, m_bTag ( false )
	{
	}

	/// ctor from C string
	CSphVariant ( const char * sString )
		: CSphString ( sString )
		, m_iValue ( atoi ( m_sValue ) )
		, m_pNext ( NULL )
	{
	}

	/// copy ctor
	CSphVariant ( const CSphVariant & rhs )
		: CSphString ()
		, m_iValue ( 0 )
		, m_pNext ( NULL )
	{
		*this = rhs;
	}

	/// default dtor
	/// WARNING: automatically frees linked items!
	virtual ~CSphVariant ()
	{
		SafeDelete ( m_pNext );
	}

	/// int value getter
	int intval () const
	{
		return m_iValue;
	}

	/// default copy operator
	const CSphVariant & operator = ( const CSphVariant & rhs )
	{
		assert ( !m_pNext );
		if ( rhs.m_pNext )
			m_pNext = new CSphVariant ( *rhs.m_pNext );

		CSphString::operator = ( rhs );
		m_iValue = rhs.m_iValue;

		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////

/// match-sorting priority min-queue interface
template < typename T > class ISphQueue
{
public:
	/// virtualizing dtor
	virtual				~ISphQueue () {}

	/// base push
	/// returns false if the entry was rejected as duplicate
	/// returns true otherwise (even if it was not actually inserted)
	virtual bool		Push ( const T & tEntry ) = 0;

	/// base pop
	virtual void		Pop () = 0;

	/// get entries count
	virtual int			GetLength () const = 0;

	/// get current root
	virtual const T &	Root () const = 0;

public:
	/// match-queue specific, set if this queue needs attr values
						ISphQueue ( bool bUsesAttrs ) : m_bUsesAttrs ( bUsesAttrs ) {}

	/// match-queue specific, check if this queue needs attr values
	bool				UsesAttrs () { return m_bUsesAttrs; }

	/// match-queue specific, set attr to sort by
	virtual void		SetAttr ( int ) {};

	/// match-queue specific, get attr to sort by
	virtual int			GetAttr () { return 0; }

protected:
	bool				m_bUsesAttrs;	///<  match-queue specific, whether this queue uses attrs
};

#endif // _sphinxstd_

//
// $Id$
//
