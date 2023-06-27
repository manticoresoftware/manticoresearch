//
// Copyright (c) 2020-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
/// @file threadutils.inc
/// internal implementations with templates

#ifndef MANTICORE_THREADUTILS_INC
#define MANTICORE_THREADUTILS_INC

namespace Threads {
namespace details {


// list (FIFO queue) of operations to perform. Used in service queue.
template<typename Operation>
class OpQueue_T : public ISphNoncopyable
{
	Operation * m_pFront = nullptr; // The front of the queue.
	Operation * m_pBack = nullptr; // The back of the queue.
	DWORD m_uLen = 0;

public:
	// destroys all operations.
	~OpQueue_T ()
	{
		while (Operation * pOp = m_pFront)
		{
			Pop ();
			pOp->Destroy();
		}
	}

	// Get the operation at the front of the queue.
	Operation * Front () const noexcept
	{
		return m_pFront;
	}

	// Pop an operation from the front of the queue.
	void Pop () noexcept
	{
		if ( m_pFront )
		{
			auto * tmp = m_pFront;
			m_pFront = tmp->m_pNext;
			if ( !m_pFront )
				m_pBack = nullptr;
			tmp->m_pNext = nullptr;
			--m_uLen;
		}
	}

	// Push an operation on to the back of the queue.
	void Push ( Operation * pOp ) noexcept
	{
		pOp->m_pNext = nullptr;
		if ( m_pBack )
		{
			m_pBack->m_pNext = pOp;
			m_pBack = pOp;
		} else
			m_pFront = m_pBack = pOp;
		++m_uLen;
	}

	// Push an operation on to the front of the queue.
	void Push_front ( Operation * pOp ) noexcept
	{
		if ( m_pFront )
		{
			pOp->m_pNext = m_pFront;
			m_pFront = pOp;
		} else
		{
			pOp->m_pNext = nullptr;
			m_pFront = m_pBack = pOp;
		}
		++m_uLen;
	}

	// Push all operations from another queue on to the back of the queue. The
	// source queue may contain operations of a derived type.
	template<typename OtherOperation>
	void Push ( OpQueue_T<OtherOperation> & rhs ) noexcept
	{
		auto pRhsFront = rhs.m_pFront;
		if ( pRhsFront )
		{
			if ( m_pBack )
				m_pBack->m_pNext = pRhsFront;
			else
				m_pFront = pRhsFront;
			m_pBack = rhs.m_pBack;
			rhs.m_pFront = nullptr;
			rhs.m_pBack = nullptr;
			m_uLen += std::exchange ( rhs.m_uLen, 0 );
		}
	}

	// Whether the queue is empty.
	bool Empty () const noexcept
	{
		return m_pFront==nullptr;
	}

	inline DWORD GetLength() const noexcept
	{
		return m_uLen;
	}

	// Test whether an operation is already enqueued.
	bool IsEnqueued ( Operation * pOp ) const noexcept
	{
		return pOp->m_pNext || m_pBack==pOp;
	}

	// find elem (linear search), remove from list and destroy
	void RemoveAndDestroy ( Operation * pOp ) noexcept
	{
		if ( !pOp )
			return;

		if ( m_pFront==pOp )
		{
			m_pFront = pOp->m_pNext;
		} else
		{
			for ( auto pCurOp = m_pFront; pCurOp!=nullptr; pCurOp = pCurOp->m_pNext )
			{
				if ( pCurOp->m_pNext==pOp )
				{
					pCurOp->m_pNext = pOp->m_pNext;
					if ( !pCurOp->m_pNext )
						m_pBack = pCurOp;
					break;
				}
			}
		}
		if ( !m_pFront )
			m_pBack = nullptr;
		pOp->Destroy ();
		--m_uLen;
	}

	class Iterator_c
	{
		Operation * m_pIterator = nullptr;
	public:
		explicit Iterator_c ( Operation * pIterator = nullptr ) : m_pIterator ( pIterator )
		{}

		Operation & operator* () noexcept
		{
			return *m_pIterator;
		}

		Iterator_c & operator++ () noexcept
		{
			m_pIterator = m_pIterator->m_pNext;
			return *this;
		}

		bool operator!= ( const Iterator_c & rhs ) const noexcept
		{
			return m_pIterator!=rhs.m_pIterator;
		}
	};

	// c++11 style iteration
	Iterator_c begin () const noexcept
	{
		return Iterator_c ( m_pFront );
	}

	Iterator_c end () const noexcept
	{
		return Iterator_c();
	}
};

// Base class for all operations. A function pointer is used instead of virtual
// functions to avoid the associated overhead.
class SchedulerOperation_t
{
protected:
	using fnFuncType = void ( void*, SchedulerOperation_t* );
	friend class OpQueue_T<SchedulerOperation_t>;

private:
	SchedulerOperation_t* m_pNext = nullptr;
	fnFuncType* m_fnFunc;

public:
	void Complete ( void* pOwner ) noexcept
	{
		m_fnFunc ( pOwner, this );
	}

	void Destroy() noexcept
	{
		m_fnFunc ( nullptr, this );
	}

protected:
	// protect ctr and dtr of this type
	explicit SchedulerOperation_t ( fnFuncType* fnFunc )
		: m_fnFunc ( fnFunc )
	{}
	~SchedulerOperation_t() = default; // protected d-tr
};


// actual operation which completes given Handler
template<typename Handler>
class CompletionHandler_c: public SchedulerOperation_t
{
	Handler m_Handler;

public:
	explicit CompletionHandler_c ( Handler h )
		: SchedulerOperation_t ( &CompletionHandler_c::DoComplete )
		, m_Handler ( static_cast<const Handler&> ( h ) ) // force copying
	{}

	static void DoComplete ( void* pOwner, SchedulerOperation_t* pBase ) noexcept
	{
		// Take ownership of the handler object.
		auto* pHandler = static_cast<CompletionHandler_c*> ( pBase );

		// make a copy of handler before upcall.
		Handler dLocalHandler ( static_cast<const Handler&> ( pHandler->m_Handler ) );

		// deallocate before the upcall
		SafeDelete ( pHandler );

		// Make the upcall if required.
		if ( pOwner )
		{
			Threads::JobTracker_t dTrack;
			dLocalHandler();

			// barrier ensures that no operations till here would be reordered below.
			std::atomic_thread_fence ( std::memory_order_release );
		}
	}
};

template<typename HANDLER>
Threads::details::SchedulerOperation_t* Handler2Op ( HANDLER handler ) noexcept
{
	return new Threads::details::CompletionHandler_c<HANDLER> ( std::move ( handler ) );
}

}}

template<typename HANDLER>
void Threads::Scheduler_i::Schedule ( HANDLER handler, bool bVip ) noexcept
{
	ScheduleOp ( Threads::details::Handler2Op ( std::move ( handler ) ), bVip );
}

template<typename HANDLER>
void Threads::Scheduler_i::ScheduleContinuation ( HANDLER handler ) noexcept
{
	ScheduleContinuationOp ( Threads::details::Handler2Op ( std::move ( handler ) ) );
}

#endif //MANTICORE_THREADUTILS_INC
