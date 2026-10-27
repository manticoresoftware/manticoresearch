# Experimental disk DocID point cache

This prototype adds an opt-in cache inside `LookupReader_c::Find`, used by disk
document lookup (including attribute updates and kill lists). It keeps the `.spt`
writer/format and compressed reader intact. RAM-chunk hashes, SQL ID filter/range
iterators, UUID lookups, full-text and vector indexes are unchanged.

Build with `-DWITH_BBTREE_DOCID=ON`, using native Unix and Rust/Cargo >=1.93, then
set `docid_lookup_cache_size = 256m` in `searchd`. Both switches default to off/zero.
Normal builds do not find or execute Cargo. Windows and cross compilation with
the option enabled are currently rejected explicitly. The dependency is bbtree
0.1.5 at `f0a5d361ecb4df176b3e07d4e33bb399dc3d22bf`, with transitive dependencies
locked in Cargo.lock. The MIT license option is used; see LICENSE-bbtree, also
installed with enabled builds. No upstream bbtree sources are modified.

## Ownership and failure behavior

The Rust concurrent `BBTreeMap<u64,u32>` is built by one loader before the disk
chunk is published, then only read. This backend supports fallible insertions;
the unsynchronized map's allocation failure behavior is unsuitable for a bounded
optional cache. An allocator charges the handle and every requested tree page
against one atomic process budget. Cache storage has no C++ shared-pointer
control block or separate query-time allocation. Requested bytes exclude the
system allocator's metadata, rounding and retained arenas, and are not RSS.

A chunk receives a complete cache or continues using the compressed reader. On
budget exhaustion, allocation refusal, duplicate/non-ascending numeric IDs or
load cancellation, the partial cache is destroyed and all charges are returned.
Declining duplicates preserves the existing checkpoint reader's choice of RowID.
The cache uses numeric IDs as unsigned 64-bit keys, including the sign bit, and
keeps `INVALID_ROWID` for misses. Dead-row checks stay in their existing callers.

The owning reader holds a unique handle. Chunk destruction or `SetData` releases
the cache; a remapped lookup uses the compressed reader until another explicit
build. Existing index locking/lifetimes must exclude readers during remapping or
destruction, as they already must for mapped `.spt` storage. Rust counters outlive
all their allocator copies, including during map destruction.

The budget is configured at startup, with no eviction, admission priority or
automatic retry for chunks that failed to fit. Reducing it in internal tests
does not evict existing caches. During flush/merge old and new chunks share the
same budget, so the new chunk can fall back even if the old chunk later releases
storage. `SHOW STATUS LIKE 'docid_lookup_cache_used_bytes'` exposes live charges;
disk table RAM status also includes its cache. Original mapped lookup data stays
present and the cache is additional memory.

## Reproduce

```sh
git submodule update --init --recursive mcl
cmake -S . -B build-bbtree -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_TESTING=ON -DWITH_BBTREE_DOCID=ON -DWITH_BENCHES=ON
cmake --build build-bbtree --target searchd gmanticoretest gmanticorebench -j 6
ctest --test-dir build-bbtree -R 'bbtree_docid_rust|BbtreeCache' --output-on-failure
cmake -E chdir build-bbtree src/gtests/gmanticoretest
python3 src/bbtree/test_integration.py build-bbtree/src/searchd
build-bbtree/src/gbenches/gmanticorebench --benchmark_filter='^Docid' \
  --benchmark_min_time=0.3s --benchmark_repetitions=5 \
  --benchmark_out=docid.json --benchmark_out_format=json
python3 src/bbtree/bench_sql.py build-bbtree/src/searchd --docs 1000000
```

The disabled build was also compiled and tested with
`-DWITH_BBTREE_DOCID=OFF -DBBTREE_CARGO=/not-installed/cargo`; its SQL checks run
with `python3 src/bbtree/test_integration.py build-bbtree-off/src/searchd --without-backend`.

On macOS with Homebrew ICU, the measured build additionally used
`-DWITH_ICU=ON -DWITH_ICU_FORCE_STATIC=OFF -DICU_ROOT=/opt/homebrew/opt/icu4c`,
and disabled Galera, MySQL/PostgreSQL/ODBC sources, Jieba, RE2 and Snowball. MCL
runtime modules were disabled with `-DMCL_LOCAL_RUNTIME=OFF`. The benchmark
target links the daemon SSL implementation because its embedded searchd needs
authentication/crypto symbols missing from the non-daemon stubs.

The Python scripts use temporary data directories and dynamically selected local
HTTP ports, and stop only the searchd processes they start. They need no Python
dependencies or running database. SQL lifecycle checks cover cache disabled,
budget refusal, successful admission, UPDATE, DELETE, REPLACE, ALTER, restart
and DROP/reclamation. Rust tests cover concurrent readers, concurrent builders,
shared-budget enforcement, unsigned IDs, row zero and failed-insert cleanup.
C++ tests use real current/legacy files, misses, remapping, duplicates and fallback.

## Point lookup measurements

Measured on Apple M4 Pro, macOS arm64, Apple Clang 21, Rust 1.93, RelWithDebInfo,
against Manticore main `5e8fc00f55efeee8dba760f13f5ccb09ea7b2382`.
Five repetitions, median thread CPU nanoseconds per lookup. Each benchmark
iteration performs 65,536 pre-generated probes against unique sorted random
positive even IDs; odd IDs miss. All data is warmed, single reader, no forced
CPU affinity. Setup, compression, cache build and query generation are excluded
from lookup timing. Cache construction timing includes decompression and C ABI
insert calls. Raw samples are in
[benchmarks/apple-m4-pro-2026-10-27.csv](benchmarks/apple-m4-pro-2026-10-27.csv).

| Documents | Misses | Compressed ns | bbtree ns | Point speedup |
|---:|---:|---:|---:|---:|
| 100,000 | 0% | 133.74 | 24.12 | 5.54x |
| 100,000 | 50% | 132.75 | 39.73 | 3.34x |
| 100,000 | 100% | 131.17 | 24.43 | 5.37x |
| 1,000,000 | 0% | 230.32 | 40.98 | 5.62x |
| 1,000,000 | 50% | 230.05 | 54.01 | 4.26x |
| 1,000,000 | 100% | 227.80 | 43.45 | 5.24x |

For 100k/1M entries the additional cache requests 2.253/22.505 MiB while the
mapped compressed file is 1.058/10.265 MiB. Median build time is about 1.4/15.2ms.
These results measure this fallible concurrent backend through the actual reader,
not the earlier unsynchronized-map prototype. They are not SQL throughput gains.
The workload omits cold reads, realistic NAS hardware, UUID workloads and
multi-reader throughput; no universal replacement of the RAM hash is proposed.

## SQL UPDATE measurements

Two independent runs on the same machine used one million random even IDs in a
rowwise RT table flushed to disk, with 50 HTTP/SQL updates of 1,000 existing IDs
per round. Seven timed rounds follow one warmup round; each round writes a new
attribute value. The client validates a resulting value and live cache admission.
Time includes HTTP, parsing, filtering, matching and attribute updates.

| Run | Cache disabled ms/UPDATE | Cache enabled ms/UPDATE | Enabled/disabled latency |
|---:|---:|---:|---:|
| 1 | 0.9891 | 0.9268 | -6.3% |
| 2 | 1.0565 | 1.0702 | +1.3% |

The samples overlap and do **not** establish a stable SQL latency improvement.
The cache requests 23,598,273 bytes. Flush time was about 366–369ms disabled and
386ms enabled, including the additional cache construction. Raw round samples
are in [run 1](benchmarks/sql-update-1m-run1.json) and
[run 2](benchmarks/sql-update-1m-run2.json). This path still uses the original ID
filter iterator; speeding up its internal point lookup does not remove the rest
of the SQL work. More realistic workload profiling is needed before enabling
this cache in deployments.

## Local validation

- Enabled: searchd and benchmark built; 497/497 C++ tests passed in a complete run.
- Disabled with a nonexistent Cargo path: searchd built; 492 C++ tests passed,
  with five enabled-backend tests skipped as intended.
- Five Rust tests and release Clippy with warnings denied passed.
- Targeted CTest (Rust and six C++ cache cases) passed; SQL lifecycle checks
  passed with budgets 0, 1KiB and 16MiB. Disabled-backend SQL checks also verify
  that a nonzero budget is rejected with a configuration error.
- Two additional enabled test launches stopped during global setup before any
  test executed (one reported SIGBUS); subsequent unmodified complete and
  focused runs passed. This macOS launch issue has not been diagnosed.

## Review status

Experimental and disabled by default. Native macOS is the locally validated
platform; Linux, Windows, sanitizer coverage of the complete mixed Rust/C++
binary, full UberTest/CLT suites and realistic production workloads remain to be
evaluated. The main upstream design question is whether an optional Rust/Cargo
dependency is justified by the measured point-lookup benefit and extra memory.
