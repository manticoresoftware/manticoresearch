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

#endif // _sphinxstd_

//
// $Id$
//
