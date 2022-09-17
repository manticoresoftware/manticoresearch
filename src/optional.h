//
// Copyright (c) 2019-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

# pragma once

#include <utility>

// Tiny port of boost::optional.
// (advice to don't play the fool and use directly boost::optional instead. It's better!)

template<typename T>
class Optional_T
{
	using StorageType = typename std::aligned_storage<sizeof ( T ), alignof ( T )>::type;
	using ThisType = Optional_T<T>;
	using ValueType = T;
	using ReferenceType = T&;
	using ReferenceConstType = T const&;
	using RvalReferenceType = T&&;
	using ReferenceTypeOfTemporaryWrapper = T&&;
	using PointerType = T *;
	using PointerConstType = T const*;
	using ArgumentType = T const&;


	StorageType m_dStorage;
	bool m_bInitialized;

	void * address () { return &m_dStorage; }

	void const * address () const { return &m_dStorage; }

	T * ptr_ref ()
	{
		union { void * pVoid; T * pType; } caster = {address ()};
		return caster.pType;
	}

	T const * ptr_ref () const
	{
		union { void const * pVoid; T const * pType; } caster = {address ()};
		return caster.pType;
	}

	T & ref () { return *ptr_ref (); }
	T const & ref () const { return *ptr_ref (); }

	ReferenceConstType get_impl () const
	{
		return ref ();
	}

	ReferenceType get_impl ()
	{
		return ref ();
	}

	PointerConstType get_ptr_impl () const { return ptr_ref (); }

	PointerType get_ptr_impl () { return ptr_ref (); }

	void destroy_impl ()
	{
		ptr_ref ()->~T ();
		m_bInitialized = false;
	}

	template<class... Args>
	void construct ( Args && ... args )
	{
		::new ( address ()) ValueType ( std::forward<Args> ( args )... );
		m_bInitialized = true;
	}

	template<class... Args>
	void emplace_assign ( Args && ... args )
	{
		if ( m_bInitialized )
			destroy_impl ();
		construct ( std::forward<Args> ( args )... );
	}

public:
	Optional_T () : m_bInitialized ( false ) {}
	~Optional_T() { reset(); }

	// construct the object in place
	template<class... Args>
	void emplace ( Args && ... args )
	{
		this->emplace_assign ( std::forward<Args> ( args )... );
	}

	// construct the object in place, but only if it is not assigned yet
	template<class... Args>
	void emplace_once ( Args && ... args )
	{
		if ( !this->m_bInitialized )
			this->emplace_assign ( std::forward<Args> ( args )... );
	}

	// reset object
	void reset () noexcept
	{
		if ( m_bInitialized )
			destroy_impl ();
	}

	bool operator! () const noexcept { return !this->m_bInitialized; }
	explicit operator bool () const noexcept { return !this->operator!(); }

	ReferenceConstType get () const
	{
		assert ( this->m_bInitialized );
		return this->get_impl ();
	}

	ReferenceType get ()
	{
		assert ( this->m_bInitialized );
		return this->get_impl ();
	}

	// Returns a copy of the value if this is initialized, 'v' otherwise
	ReferenceConstType get_value_or ( ReferenceConstType v ) const
	{
		return this->m_bInitialized ? get () : v;
	}

	ReferenceType get_value_or ( ReferenceType v )
	{
		return this->m_bInitialized ? get () : v;
	}

	// Returns a pointer to the value if this is initialized, otherwise,
	// the behaviour is UNDEFINED
	PointerConstType operator-> () const
	{
		assert ( this->m_bInitialized );
		return this->get_ptr_impl ();
	}

	PointerType operator-> ()
	{
		assert ( this->m_bInitialized );
		return this->get_ptr_impl ();
	}

	// Returns a reference to the value if this is initialized, otherwise,
	// the behaviour is UNDEFINED
	ReferenceConstType operator* () const & { return this->get (); }
	ReferenceType operator* () & { return this->get (); }
	ReferenceTypeOfTemporaryWrapper operator* () && { return std::move ( this->get ()); }

	template<class U>
	ValueType value_or ( U && v ) const &
	{
		if ( this->m_bInitialized )
			return get ();
		else
			return std::forward<U> ( v );
	}

	template<class U>
	ValueType value_or ( U && v ) &&
	{
		if ( this->m_bInitialized )
			return std::move ( get ());
		else
			return std::forward<U> ( v );
	}

	template<typename F>
	ValueType value_or_eval ( F f ) const &
	{
		if ( this->m_bInitialized )
			return get ();
		else
			return f ();
	}

	template<typename F>
	ValueType value_or_eval ( F f ) &&
	{
		if ( this->m_bInitialized)
			return std::move ( get ());
		else
			return f ();
	}
};
