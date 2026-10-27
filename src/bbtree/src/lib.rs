//! Optional, immutable DocID cache behind a small C ABI. All requested bytes,
//! including the handle, are charged against a process-wide allocation budget.
use bbtree::{AllocError, Allocator, Auto, BBTreeMap};
use std::alloc::{Layout, alloc, alloc_zeroed, dealloc};
use std::mem::ManuallyDrop;
use std::ptr::{self, NonNull};
use std::sync::atomic::{AtomicUsize, Ordering};

static LIMIT: AtomicUsize = AtomicUsize::new(0);
static USED: AtomicUsize = AtomicUsize::new(0);

fn reserve(bytes: usize) -> bool {
    USED.fetch_update(Ordering::Relaxed, Ordering::Relaxed, |used| {
        used.checked_add(bytes)
            .filter(|&next| next <= LIMIT.load(Ordering::Relaxed))
    })
    .is_ok()
}

#[derive(Clone, Copy)]
struct Budgeted(NonNull<AtomicUsize>);

// SAFETY: the counter is inside a stable, explicitly allocated Cache. The map
// and every allocator copy are destroyed before that counter is deallocated.
// All shared access to the counter is atomic.
unsafe impl Send for Budgeted {}
// SAFETY: same stable lifetime and atomic counter as Send above.
unsafe impl Sync for Budgeted {}

// SAFETY: each successful allocation is unique and zeroed with its requested
// layout; deallocation uses that layout. Failure never unwinds.
unsafe impl Allocator for Budgeted {
    fn allocate_zeroed(&self, layout: Layout) -> Result<NonNull<u8>, AllocError> {
        if !reserve(layout.size()) {
            return Err(AllocError);
        }
        // SAFETY: bbtree supplies valid, nonzero node layouts.
        let ptr = NonNull::new(unsafe { alloc_zeroed(layout) });
        let Some(ptr) = ptr else {
            USED.fetch_sub(layout.size(), Ordering::Relaxed);
            return Err(AllocError);
        };
        // SAFETY: the owning Cache outlives the map and its allocator copies.
        unsafe { self.0.as_ref() }.fetch_add(layout.size(), Ordering::Relaxed);
        Ok(ptr)
    }

    unsafe fn deallocate(&self, ptr: NonNull<u8>, layout: Layout) {
        // SAFETY: caller returns a live block with the same layout.
        unsafe { dealloc(ptr.as_ptr(), layout) };
        // SAFETY: map destruction completes before the Cache counter is freed.
        unsafe { self.0.as_ref() }.fetch_sub(layout.size(), Ordering::Relaxed);
        USED.fetch_sub(layout.size(), Ordering::Relaxed);
    }
}

type Map = BBTreeMap<u64, u32, Auto, Budgeted>;

pub struct Cache {
    bytes: AtomicUsize,
    map: ManuallyDrop<Map>,
}

/// Called during configuration before caches are built. Reducing the limit
/// does not evict live caches; it prevents new allocations until usage drops.
#[unsafe(no_mangle)]
pub extern "C" fn bb_docid_set_budget(bytes: usize) {
    LIMIT.store(bytes, Ordering::Relaxed);
}

#[unsafe(no_mangle)]
pub extern "C" fn bb_docid_used() -> usize {
    USED.load(Ordering::Relaxed)
}

/// Returns null on disabled budget, exhaustion or handle allocation failure.
#[unsafe(no_mangle)]
pub extern "C" fn bb_docid_new() -> *mut Cache {
    let layout = Layout::new::<Cache>();
    if !reserve(layout.size()) {
        return ptr::null_mut();
    }
    // SAFETY: the nonzero layout describes Cache; no references exist yet.
    let raw = unsafe { alloc(layout) }.cast::<Cache>();
    let Some(raw) = NonNull::new(raw) else {
        USED.fetch_sub(layout.size(), Ordering::Relaxed);
        return ptr::null_mut();
    };
    // SAFETY: addr_of_mut does not read the uninitialized allocation.
    let counter = unsafe { NonNull::new_unchecked(ptr::addr_of_mut!((*raw.as_ptr()).bytes)) };
    // SAFETY: the complete value is initialized in its final, stable location.
    unsafe {
        ptr::write(
            raw.as_ptr(),
            Cache {
                bytes: AtomicUsize::new(layout.size()),
                map: ManuallyDrop::new(Map::new_in(Budgeted(counter))),
            },
        )
    };
    raw.as_ptr()
}

/// # Safety
/// cache is a live handle. Inserts finish before publication to C++ readers.
/// Duplicate IDs retain the first RowID. C++ declines caching duplicate IDs
/// because checkpoint selection can have different duplicate semantics.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn bb_docid_insert(cache: *mut Cache, key: u64, row: u32) -> bool {
    // SAFETY: the owner guarantees a live handle until insertion finishes.
    unsafe { &*cache }
        .map
        .update_or_insert(key, row, |_, first, _| *first)
        .is_ok()
}

/// # Safety
/// cache is a live, fully built handle; no concurrent free is permitted.
/// UINT32_MAX is the existing Manticore INVALID_ROWID sentinel.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn bb_docid_find(cache: *const Cache, key: u64) -> u32 {
    // SAFETY: the owner keeps the immutable map alive for every reader.
    unsafe { &*cache }.map.get_copied(&key).unwrap_or(u32::MAX)
}

/// # Safety
/// cache is a live handle. Reports requested live storage, not process RSS.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn bb_docid_bytes(cache: *const Cache) -> usize {
    // SAFETY: caller keeps the handle alive; the counter is atomic.
    unsafe { &*cache }.bytes.load(Ordering::Relaxed)
}

/// # Safety
/// cache was returned by bb_docid_new, is not freed, and has no active readers.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn bb_docid_free(cache: *mut Cache) {
    if cache.is_null() {
        return;
    }
    // SAFETY: unique ownership is transferred back after all readers finish.
    // map must die before its allocator counter in the same allocation.
    unsafe { ManuallyDrop::drop(&mut (*cache).map) };
    let layout = Layout::new::<Cache>();
    // SAFETY: the handle was allocated with exactly this layout.
    unsafe { dealloc(cache.cast(), layout) };
    USED.fetch_sub(layout.size(), Ordering::Relaxed);
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Mutex;
    static SERIAL: Mutex<()> = Mutex::new(());

    #[test]
    fn disabled_budget_does_not_allocate() {
        let _guard = SERIAL.lock().unwrap();
        bb_docid_set_budget(0);
        assert!(bb_docid_new().is_null());
        assert_eq!(bb_docid_used(), 0);
    }

    #[test]
    fn unsigned_ids_row_zero_and_first_duplicate_are_preserved() {
        let _guard = SERIAL.lock().unwrap();
        bb_docid_set_budget(1 << 20);
        unsafe {
            let cache = bb_docid_new();
            assert!(!cache.is_null());
            assert!(bb_docid_insert(cache, 0, 0));
            assert!(bb_docid_insert(cache, 1 << 63, 3));
            assert!(bb_docid_insert(cache, u64::MAX, 7));
            assert!(bb_docid_insert(cache, 0, 9));
            assert_eq!(bb_docid_find(cache, 0), 0);
            assert_eq!(bb_docid_find(cache, 1 << 63), 3);
            assert_eq!(bb_docid_find(cache, u64::MAX), 7);
            assert_eq!(bb_docid_find(cache, 1), u32::MAX);
            assert!(bb_docid_bytes(cache) > 0);
            bb_docid_free(cache);
        }
        assert_eq!(bb_docid_used(), 0);
    }

    #[test]
    fn budget_exhaustion_returns_error_and_releases_all_pages() {
        let _guard = SERIAL.lock().unwrap();
        bb_docid_set_budget(32 * 1024);
        unsafe {
            let cache = bb_docid_new();
            assert!(!cache.is_null());
            let mut count = 0;
            while count < 100_000 && bb_docid_insert(cache, count as u64 * 2, count) {
                count += 1;
            }
            assert!(count > 0 && count < 100_000);
            assert!(bb_docid_used() <= 32 * 1024);
            for row in 0..count {
                assert_eq!(bb_docid_find(cache, row as u64 * 2), row);
            }
            bb_docid_free(cache);
        }
        assert_eq!(bb_docid_used(), 0);
    }

    #[test]
    fn readers_share_an_immutable_map_and_budget_covers_multiple_caches() {
        let _guard = SERIAL.lock().unwrap();
        bb_docid_set_budget(1 << 20);
        unsafe {
            let cache = bb_docid_new();
            for i in 0..1024 {
                assert!(bb_docid_insert(cache, i as u64, i));
            }
            let address = cache as usize;
            std::thread::scope(|scope| {
                for _ in 0..4 {
                    scope.spawn(move || {
                        for i in 0..1024 {
                            assert_eq!(bb_docid_find(address as *const Cache, i as u64), i);
                        }
                    });
                }
            });
            let second = bb_docid_new();
            assert!(!second.is_null());
            assert_eq!(
                bb_docid_used(),
                bb_docid_bytes(cache) + bb_docid_bytes(second)
            );
            bb_docid_free(second);
            bb_docid_free(cache);
        }
        assert_eq!(bb_docid_used(), 0);
    }

    #[test]
    fn concurrent_builders_never_exceed_shared_budget() {
        let _guard = SERIAL.lock().unwrap();
        const BUDGET: usize = 128 * 1024;
        bb_docid_set_budget(BUDGET);
        let barrier = std::sync::Barrier::new(8);
        std::thread::scope(|scope| {
            for _ in 0..8 {
                let barrier = &barrier;
                scope.spawn(move || unsafe {
                    let cache = bb_docid_new();
                    barrier.wait();
                    if !cache.is_null() {
                        for row in 0..10_000 {
                            if !bb_docid_insert(cache, row as u64, row) {
                                break;
                            }
                            assert!(bb_docid_used() <= BUDGET);
                        }
                    }
                    barrier.wait();
                    bb_docid_free(cache);
                });
            }
        });
        assert_eq!(bb_docid_used(), 0);
    }
}
