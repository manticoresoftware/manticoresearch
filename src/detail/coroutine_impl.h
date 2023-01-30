//
// Copyright (c) 2022-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

// that file includes implementation details from coroutine.h

namespace Threads
{

/////////////////////////////////////////////////////////////////////////////
/// ClonableCtx_T
/////////////////////////////////////////////////////////////////////////////

template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
template<typename... PARAMS>
ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::ClonableCtx_T ( PARAMS&&... tParams )
	: m_dParentContext ( std::forward<PARAMS> ( tParams )... )
{
	m_bSingle = !m_dParentContext.IsClonable();
}

template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
void ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::LimitConcurrency ( int iDistThreads )
{
	assert ( m_iTasks == 0 );							   // can be run only when no work started
	if ( m_bSingle || !iDistThreads ) // 0 as for dist_threads means 'no limit'
		return;

	auto iContexts = iDistThreads - 1; // one context is always clone-free
	if ( !iContexts )
	{
		m_bSingle = true;
		return;
	}

	m_dChildrenContexts.Reset ( iContexts );
	m_dJobsOrder.Reset ( iContexts );
	m_dJobsOrder.ZeroVec();
}

// called once per coroutine, when it really has to process something
template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
template<ECONTEXT ORD>
std::enable_if_t<ORD == ECONTEXT::ORDERED, std::pair<REFCONTEXT, int>> ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::CloneNewContext ( bool bFirst )
{
	if ( m_bSingle || bFirst )
		return { m_dParentContext, 0 };

	auto iMyIdx = m_iTasks.fetch_add ( 1, std::memory_order_relaxed );

	auto& tCtx = m_dChildrenContexts[iMyIdx];
	if ( !tCtx )
		tCtx.emplace ( m_dParentContext );
	return { (REFCONTEXT)tCtx.value(), iMyIdx + 1 };
}

// called once per coroutine, when it really has to process something
template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
template<ECONTEXT ORD>
std::enable_if_t<ORD == ECONTEXT::UNORDERED, std::pair<REFCONTEXT, int>> ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::CloneNewContext ()
{
	if ( m_bSingle )
		return { m_dParentContext, 0 };

	auto iMyIdx = m_iTasks.fetch_add ( 1, std::memory_order_relaxed );
	if ( !iMyIdx )
		return { m_dParentContext, 0 };

	--iMyIdx; // make it back 0-based
	auto& tCtx = m_dChildrenContexts[iMyIdx];
	if ( !tCtx )
		tCtx.emplace ( m_dParentContext );
	return { (REFCONTEXT)tCtx.value(), iMyIdx + 1 };
}

// set (optionally) 'weight' of a job; ForAll will iterate jobs according to ascending weights
template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
template<ECONTEXT ORD>
std::enable_if_t<ORD == ECONTEXT::ORDERED> ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::SetJobOrder ( int iCtxID, int iOrder )
{
	assert ( iCtxID < m_iTasks + 1 );
	if ( !iCtxID ) // todo! zero (parent) context doesn't have an order
		return;
	m_dJobsOrder[iCtxID - 1] = iOrder;
}

// Num of parallel workers to complete iTasks jobs
template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
inline bool ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::IsSingle () const
{
	return m_bSingle;
}

// Num of parallel workers to complete iTasks jobs
template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
inline int ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::Concurrency ( int iTasks ) const
{
	return Min ( m_dChildrenContexts.GetLength() + 1, iTasks ); // +1 since parent is also an extra context
}

template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
int ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::NumWorked() const
{
	return m_iTasks.load ( std::memory_order_relaxed );
}

template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
template<typename FNPROCESSOR, ECONTEXT ORD>
std::enable_if_t<ORD == ECONTEXT::ORDERED> ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::ForAll ( FNPROCESSOR fnProcess, bool bIncludeRoot )
{
	assert ( !bIncludeRoot ); // for ordered context it should not be set
	if ( m_bSingle ) // nothing to do; sorters and results are already original
		return;

	int iWorkedThreads = m_iTasks; // NOT - 1, as we didn't account parent context in the counter
	CSphFixedVector<int> dOrder { iWorkedThreads };
	dOrder.FillSeq();
	dOrder.Sort ( Lesser ( [this] ( int a, int b ) { return m_dJobsOrder[a] < m_dJobsOrder[b]; } ) );

	for ( auto i : dOrder )
	{
		assert ( m_dChildrenContexts[i] );
		auto tCtx = (REFCONTEXT)m_dChildrenContexts[i].value();
		fnProcess ( tCtx );
	}
}

template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
template<typename FNPROCESSOR, ECONTEXT ORD>
std::enable_if_t<ORD == ECONTEXT::UNORDERED> ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::ForAll ( FNPROCESSOR fnProcess, bool bIncludeRoot )
{
	if ( bIncludeRoot )
		fnProcess ( m_dParentContext );

	if ( m_bSingle ) // nothing to do; sorters and results are already original
		return;

	int iWorkedThreads = m_iTasks - 1;
	CSphFixedVector<int> dOrder { iWorkedThreads };
	dOrder.FillSeq();
	dOrder.Sort ( Lesser ( [this] ( int a, int b ) { return m_dJobsOrder[a] < m_dJobsOrder[b]; } ) );

	for ( auto i : dOrder )
	{
		assert ( m_dChildrenContexts[i] );
		auto tCtx = (REFCONTEXT)m_dChildrenContexts[i].value();
		fnProcess ( tCtx );
	}
}

template<typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
void ClonableCtx_T<REFCONTEXT, CONTEXT, IS_ORDERED>::Finalize()
{
	ForAll ( [this] ( REFCONTEXT tContext ) { m_dParentContext.MergeChild ( tContext ); }, false );
}

/////////////////////////////////////////////////////////////////////////////
/// HighFreqChecker_c
/////////////////////////////////////////////////////////////////////////////
inline bool Coro::HighFreqChecker_c::operator()()
{
	using namespace std::chrono;

	if ( !m_iPivotPeriod )
		return false;

	++m_iHits;
	if ( !m_iPivotHits )
	{
		if ( !m_iHits ) // very first run, we know nothing yet.
		{
			m_tmFirstHit = steady_clock::now();
			return false;
		}

		if ( m_iHits < m_iFibCurrent )
			return false;

		if ( duration_cast<microseconds> ( steady_clock::now() - m_tmFirstHit ).count() >= m_iPivotPeriod )
		{
			m_iPivotHits = m_iHits;
		} else
		{
			m_iFibPrev = std::exchange ( m_iFibCurrent, m_iFibCurrent + m_iFibPrev ); // next fibonacci seq number
			return false;
		}
	}

	assert ( m_iPivotHits );
	if ( m_iHits == m_iPivotHits )
	{
		m_iHits = 0;
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
/// ScopedScheduler_c
/////////////////////////////////////////////////////////////////////////////

inline ScopedScheduler_c::ScopedScheduler_c ( SchedRole pRole )
{
	if ( !pRole )
		return;

	m_pRoleRef = Coro::CurrentScheduler();
	//		if ( m_pRoleRef )
	AcquireSched ( pRole );
}

inline ScopedScheduler_c::ScopedScheduler_c ( RoledSchedulerSharedPtr_t& pRole )
{
	if ( !pRole )
		return;

	m_pRoleRef = Coro::CurrentScheduler();
	//		if ( m_pRoleRef )
	AcquireSched ( pRole );
}

inline ScopedScheduler_c::~ScopedScheduler_c()
{
	if ( m_pRoleRef )
		AcquireSched ( m_pRoleRef );
}


namespace Coro
{

// if iStack<0, just immediately invoke the handler (that is bypass)
template<typename HANDLER>
void Continue ( int iStack, HANDLER handler )
{
	if ( iStack<0 ) {
		handler ();
		return;
	}
	Continue ( handler, iStack );
}

// if iStack<0, just immediately invoke the handler (that is bypass). Returns boolean result from handler
template<typename HANDLER>
bool ContinueBool ( int iStack, HANDLER handler )
{
	if ( iStack<0 )
		return handler ();

	bool bResult;
	Continue ( [&bResult, fnHandler = std::move ( handler )] { bResult = fnHandler (); }, iStack );
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
/// ConditionVariableAny_c
/////////////////////////////////////////////////////////////////////////////

template<typename LockType>
void ConditionVariableAny_c::Wait ( LockType& tMutex )
{
	auto* pActiveWorker = Worker();
	// atomically call tMutex.unlock() and block on current worker
	// store this coro in waiting-queue
	sph::Spinlock_lock tLock { m_tWaitQueueSpinlock };
	tMutex.Unlock();
	m_tWaitQueue.SuspendAndWait ( tLock, pActiveWorker );

	// relock external again before returning
	tMutex.Lock();
}

template<typename LockType>
bool ConditionVariableAny_c::WaitUntil ( LockType& tMutex, int64_t iTimestamp )
{
	auto* pActiveWorker = Worker();
	// atomically call tMutex.unlock() and block on current worker
	// store this coro in waiting-queue
	sph::Spinlock_lock tLock { m_tWaitQueueSpinlock };
	tMutex.Unlock();
	bool bResult = m_tWaitQueue.SuspendAndWaitUntil ( tLock, pActiveWorker, iTimestamp );

	// relock external again before returning
	tMutex.Lock();
	return bResult;
}

template<typename LockType>
bool ConditionVariableAny_c::WaitForMs ( LockType& tMutex, int64_t iTimePeriodMS )
{
	return WaitUntil ( tMutex, sphMicroTimer() + iTimePeriodMS*1000 );
}

template<typename LockType, typename PRED>
void ConditionVariableAny_c::Wait ( LockType& tMutex, PRED fnPred )
{
	while ( !fnPred() ) {
		Wait ( tMutex );
	}
}

template<typename LockType, typename PRED>
bool ConditionVariableAny_c::WaitUntil ( LockType& tMutex, PRED fnPred, int64_t iTimestamp )
{
	while ( !fnPred() ) {
		if ( !WaitUntil ( tMutex, iTimestamp ) )
			return fnPred();
	}
	return true;
}

template<typename LockType, typename PRED>
bool ConditionVariableAny_c::WaitForMs ( LockType& tMutex, PRED fnPred, int64_t iTimePeriodMS )
{
	return WaitUntil ( tMutex, std::forward<PRED> ( fnPred ), sphMicroTimer() + iTimePeriodMS * 1000 );
}

/////////////////////////////////////////////////////////////////////////////
/// ConditionVariable_c
/////////////////////////////////////////////////////////////////////////////

template<typename PRED>
void ConditionVariable_c::Wait ( ScopedMutex_t& lt, PRED&& fnPred ) REQUIRES ( lt )
{
	m_tCnd.Wait ( lt, std::forward<PRED> ( fnPred ) );
}

template<typename PRED>
bool ConditionVariable_c::WaitUntil ( ScopedMutex_t& lt, PRED&& fnPred, int64_t iTime ) REQUIRES ( lt )
{
	return m_tCnd.WaitUntil ( lt, std::forward<PRED> ( fnPred ), iTime );
}

template<typename PRED>
bool ConditionVariable_c::WaitForMs ( ScopedMutex_t& lt, PRED&& fnPred, int64_t iPeriodMS ) REQUIRES ( lt )
{
	return m_tCnd.WaitForMs ( lt, std::forward<PRED> ( fnPred ), iPeriodMS );
}

/////////////////////////////////////////////////////////////////////////////
/// Waitable_T
/////////////////////////////////////////////////////////////////////////////

template<typename T>
template<typename... PARAMS>
Waitable_T<T>::Waitable_T ( PARAMS&&... tParams )
	: m_tValue ( std::forward<PARAMS> ( tParams )... )
{}

template<typename T>
void Waitable_T<T>::SetValue ( T tValue )
{
	ScopedMutex_t lk ( m_tMutex );
	m_tValue = tValue;
}

template<typename T>
T Waitable_T<T>::ExchangeValue ( T tNewValue )
{
	ScopedMutex_t lk ( m_tMutex );
	return std::exchange ( m_tValue, tNewValue );
}

template<typename T>
void Waitable_T<T>::SetValueAndNotifyOne ( T tValue )
{
	SetValue ( tValue );
	NotifyOne();
}

template<typename T>
void Waitable_T<T>::SetValueAndNotifyAll ( T tValue )
{
	SetValue ( tValue );
	NotifyAll();
}

template<typename T>
void Waitable_T<T>::UpdateValueAndNotifyOne ( T tValue )
{
	if ( tValue == GetValue() )
		return;
	SetValueAndNotifyOne ( tValue );
}

template<typename T>
void Waitable_T<T>::UpdateValueAndNotifyAll ( T tValue )
{
	if ( tValue == GetValue() )
		return;
	SetValueAndNotifyAll ( tValue );
}

template<typename T>
template<typename MOD>
void Waitable_T<T>::ModifyValue ( MOD&& fnMod )
{
	ScopedMutex_t lk ( m_tMutex );
	fnMod ( m_tValue );
}

template<typename T>
template<typename MOD>
void Waitable_T<T>::ModifyValueAndNotifyOne ( MOD&& fnMod )
{
	ModifyValue ( std::forward<MOD> ( fnMod ) );
	NotifyOne();
}

template<typename T>
template<typename MOD>
void Waitable_T<T>::ModifyValueAndNotifyAll ( MOD&& fnMod )
{
	ModifyValue ( std::forward<MOD> ( fnMod ) );
	NotifyAll();
}

template<typename T>
T Waitable_T<T>::GetValue() const
{
	return m_tValue;
}

template<typename T>
const T& Waitable_T<T>::GetValueRef() const
{
	return m_tValue;
}

template<typename T>
inline void Waitable_T<T>::NotifyOne()
{
	m_tCondVar.NotifyOne();
}

template<typename T>
inline void Waitable_T<T>::NotifyAll()
{
	m_tCondVar.NotifyAll();
}

template<typename T>
void Waitable_T<T>::Wait () const
{
	ScopedMutex_t lk ( m_tMutex );
	m_tCondVar.Wait ( lk );
}

template<typename T>
bool Waitable_T<T>::WaitUntil(int64_t iTime) const
{
	ScopedMutex_t lk ( m_tMutex );
	return m_tCondVar.WaitUntil ( lk, iTime );
}

template<typename T>
bool Waitable_T<T>::WaitForMs(int64_t iPeriodMS) const
{
	ScopedMutex_t lk ( m_tMutex );
	return m_tCondVar.WaitForMs ( lk, iPeriodMS );
}

template<typename T>
template<typename PRED>
T Waitable_T<T>::Wait ( PRED&& fnPred ) const
{
	ScopedMutex_t lk ( m_tMutex );
	m_tCondVar.Wait ( lk, [this, fnPred = std::forward<PRED> ( fnPred )]() { return fnPred ( m_tValue ); } );
	return m_tValue;
}

template<typename T>
template<typename PRED>
T Waitable_T<T>::WaitUntil ( PRED&& fnPred, int64_t iTime ) const
{
	ScopedMutex_t lk ( m_tMutex );
	m_tCondVar.WaitUntil ( lk, [this, fnPred = std::forward<PRED> ( fnPred )]() { return fnPred ( m_tValue ); }, iTime );
	return m_tValue;
}

template<typename T>
template<typename PRED>
T Waitable_T<T>::WaitForMs ( PRED&& fnPred, int64_t iPeriodMs ) const
{
	ScopedMutex_t lk ( m_tMutex );
	m_tCondVar.WaitForMs ( lk, [this, fnPred = std::forward<PRED> ( fnPred )]() { return fnPred ( m_tValue ); }, iPeriodMs );
	return m_tValue;
}

template<typename T>
template<typename PRED>
void Waitable_T<T>::WaitVoid ( PRED&& fnPred ) const
{
	ScopedMutex_t lk ( m_tMutex );
	m_tCondVar.Wait ( lk, std::forward<PRED> ( fnPred ) );
}

template<typename T>
template<typename PRED>
bool Waitable_T<T>::WaitVoidUntil ( PRED&& fnPred, int64_t iTime ) const
{
	ScopedMutex_t lk ( m_tMutex );
	return m_tCondVar.WaitUntil ( lk, std::forward<PRED> ( fnPred ), iTime );
}

template<typename T>
template<typename PRED>
bool Waitable_T<T>::WaitVoidForMs ( PRED&& fnPred, int64_t iPeriodMs ) const
{
	ScopedMutex_t lk ( m_tMutex );
	return m_tCondVar.WaitForMs ( lk, std::forward<PRED> ( fnPred ), iPeriodMs );
}


} // namespace Coro
} // namespace Threads

