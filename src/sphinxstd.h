//
// $Id$
//

#ifndef _sphinxstd_
#define _sphinxstd_

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
	int GetLength ()
	{
		return m_iLength;
	}

	/// sort
	void Sort ( int iStart=0, int iEnd=-1 )
	{
		_Sort<false> ( iStart, iEnd );
	}

	/// reverse sort
	void RSort ( int iStart=0, int iEnd=-1 )
	{
		_Sort<true> ( iStart, iEnd );
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

private:
	int		m_iLength;		///< entries actually used
	int		m_iLimit;		///< entries allocated
	T *		m_pData;		///< entries

private:
	/// sort the array
	template < bool REVERSE > void _Sort ( int iStart, int iEnd )
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
					#if USE_WINDOWS
					#pragma warning(disable:4127)
					#endif
					if ( !REVERSE )
					#if USE_WINDOWS
					#pragma warning(default:4127)
					#endif
					{
						while ( m_pData[i]<x ) i++;
						while ( x<m_pData[j] ) j--;
					} else
					{
						while ( x<m_pData[i] ) i++;
						while ( m_pData[j]<x ) j--;
					}
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
};


#define ARRAY_FOREACH(_index,_array) \
	for ( int _index=0; _index<_array.GetLength(); _index++ )

/////////////////////////////////////////////////////////////////////////////

/// simple generic hash
template < typename T, typename KEY, typename HASHFUNC, int LENGTH, int INCREASE >
class CSphGenericHash
{
protected:
	struct HashEntry_t
	{
		T 			m_tData;				///< data, owned by the hash
		KEY			m_tKey;					///< key, owned by the hash
		bool		m_bDeleted;				///< is this entry deleted
		bool		m_bEmpty;				///< is this entry empty
	};

protected:
	HashEntry_t	*	m_pHash;				///< all the hash entries

protected:
	/// this function finds hash entry by it's key.
	template < bool STOP_ON_DELETED >
	const HashEntry_t * Search ( const KEY & key ) const
	{
		if ( !m_pHash )
			return NULL;

		int index = HASHFUNC() ( key ) ;
		assert ( index >= 0 && index < LENGTH );

		for ( int count = 0; count < LENGTH * 2 / 3; count++ )
		{
			// deleted slot
			if ( m_pHash[index].m_bDeleted )
			{
				// found deleted slot
				#if USE_WINDOWS
				#pragma warning(disable:4127)
				#endif
				if ( STOP_ON_DELETED )
				#if USE_WINDOWS
				#pragma warning(default:4127)
				#endif
					return m_pHash + index;
			}
			else
			{
				// empty slot
				if ( m_pHash[index].m_bEmpty )
					return m_pHash + index;

				// found the same
				if ( m_pHash[index].m_tKey == key )
					return m_pHash + index;
			}

			// next index
			index = ( index+INCREASE ) & ( LENGTH-1 );
		}

		return NULL;
	}

public:
	/// ctor
	CSphGenericHash ()
		: m_pHash ( NULL )
		, m_pIterator ( NULL )
	{
	}

	/// dtor
	~CSphGenericHash ()
	{
		Reset ();
	}

	/// reset
	void Reset ()
	{
		SafeDeleteArray ( m_pHash );
	}

	/// add new entry
	void Add ( const T & tData, const KEY & tKey )
	{
		if ( !m_pHash )
			Create ();

		HashEntry_t * pEntry = const_cast<HashEntry_t *> ( Search<true> ( tKey ) );
		assert ( pEntry && "hash overflow" );
		assert ( pEntry->m_bEmpty && "already hashed" );

		pEntry->m_tData		= tData;
		pEntry->m_tKey		= tKey;
		pEntry->m_bDeleted	= false;
		pEntry->m_bEmpty	= false;
	}

	/// check if key exists
	bool Exists ( const KEY & key ) const
	{
		const HashEntry_t * pEntry = Search<false> ( key );
		return !( pEntry==NULL || pEntry->m_bEmpty );
	}

	/// get current value
	T & operator [] ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = const_cast<HashEntry_t *> ( Search<false> ( tKey ) );
		if ( pEntry && !pEntry->m_bEmpty )
			return pEntry->m_tData;

		assert ( 0 && "hash missing value in operator []" );
		return m_pHash[0].m_tData;
	}

	/// copying
	const CSphGenericHash<T,KEY,HASHFUNC,LENGTH,INCREASE> & operator = ( const CSphGenericHash<T,KEY,HASHFUNC,LENGTH,INCREASE> & rhs )
	{
		SafeDeleteArray ( m_pHash );
		if ( rhs.m_pHash )
		{
			Create ();
			for ( int i=0; i<LENGTH; i++ )
			{
				m_pHash[i].m_tKey = rhs.m_pHash[i].m_tKey;
				m_pHash[i].m_tData = rhs.m_pHash[i].m_tData;
				m_pHash[i].m_bDeleted = rhs.m_pHash[i].m_bDeleted;
				m_pHash[i].m_bEmpty = rhs.m_pHash[i].m_bEmpty;
			}
		}
		return *this;
	}

public:
	/// start iterating
	void IterateStart () const
	{
		m_pIterator = m_pHash-1;
	}

	/// go to next existing entry
	bool IterateNext () const
	{
		if ( !m_pHash )
			return false;

		do
		{
			++m_pIterator;
		} while ( m_pIterator!=m_pHash+LENGTH && ( m_pIterator->m_bDeleted || m_pIterator->m_bEmpty ) );
		return m_pIterator!=( m_pHash+LENGTH );
	}

	/// get entry value
	const T & IterateGet () const
	{
		assert ( m_pHash );
		assert ( m_pIterator && m_pIterator>=m_pHash && m_pIterator<m_pHash+LENGTH );
		return m_pIterator->m_tData;
	}

	/// get entry key
	const KEY & IterateGetKey () const
	{
		assert ( m_pHash );
		assert ( m_pIterator && m_pIterator>=m_pHash && m_pIterator<m_pHash+LENGTH );
		return m_pIterator->m_tKey;
	}

protected:
	/// allocate the hash on demand
	void Create ()
	{
		assert ( !m_pHash );
		m_pHash = new HashEntry_t [ LENGTH ];
		for ( int i=0; i<LENGTH; i++ )
		{
			m_pHash[i].m_bEmpty = true;
			m_pHash[i].m_bDeleted = false;
		}
	}

private:
	/// current iterator
	mutable HashEntry_t * m_pIterator;
};

/////////////////////////////////////////////////////////////////////////////

/// immutable C string proxy
struct CSphString
{
protected:
	char *	m_sValue;

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
		m_sValue = new char [ 1+strlen(sString) ];
		strcpy ( m_sValue, sString ); // FIXME! this is non-TS
	}

	const CSphString & operator = ( const CSphString & rhs )
	{
		SafeDeleteArray ( m_sValue );
		if ( rhs.m_sValue )
		{
			m_sValue = new char [ 1+strlen(rhs.m_sValue) ];
			strcpy ( m_sValue, rhs.m_sValue);
		}
		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////

/// immutable string/int variant proxy
struct CSphVariant : public CSphString
{
protected:
	int		m_iValue;

public:
	CSphVariant ()
		: CSphString ()
		, m_iValue ( 0)
	{
	}
	
	CSphVariant ( const char * sString )
		: CSphString ( sString )
		, m_iValue ( atoi ( m_sValue ) )
	{
	}

	CSphVariant ( const CSphVariant & rhs )
		: CSphString ()
		, m_iValue ( 0 )
	{
		*this = rhs;
	}

	int intval () const
	{
		return m_iValue;
	}

	const CSphVariant & operator = ( const CSphVariant & rhs )
	{
		CSphString::operator = ( rhs );
		m_iValue = rhs.m_iValue;
		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////

/// match-sorting priority min-queue interface
template< typename T, int SIZE> class ISphQueue
{
public:
	/// base push
	virtual void		Push ( const T & tEntry ) = 0;

	/// base pop
	virtual void		Pop () = 0;

	/// get entries count
	virtual int			GetLength () const = 0;

	/// get current root
	virtual const T &	Root () const = 0;
};

#endif // _sphinxstd_

//
// $Id$
//
