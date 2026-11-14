# DocID lookup cache validation

The native C++ cache stores sorted numeric document IDs and row IDs
beside the existing compressed lookup. `docid_lookup_cache_size` is a server-wide
memory ceiling, defaulting to `64m`; `0` disables caching. The whole allocation, including
alignment padding and the handle, is charged to one process-wide budget.
Rejected or incomplete caches retain the existing compressed reader. Disk format
and UUID lookup remain unchanged. Chunk destruction/remapping reclaims storage;
there is no eviction or automatic admission retry.

Disk-chunk load, single-ID lookups and full-count queries allocate no native
cache. The first sparse batch (at least 16 IDs, at most 1/64 of chunk rows)
records demand. Each subsequent eligible build call decodes at most 16,384 entries
per chunk; a query visiting several chunks can advance several builds. One builder
proceeds per chunk while other readers fall back without waiting. An acquire/release
pointer publishes the immutable mapping only after uniqueness is established.
Scalar `Find()` keeps the compressed path, with no cache-pointer check. In-progress
allocations count against the ceiling and are reclaimed on remap/destruction.
Admission covers a whole chunk; actual use need not equal the configured ceiling.

Directory nodes contain eight child maxima on 64-byte cache lines and sixteen
on 128/256-byte lines. The OS cache-line query has a 64-byte fallback. Sparse
numeric ID lists and nonnegative updates (at most 1/64 of the chunk's rows) use
point/batch resolution; dense lists and duplicate-ID chunks retain merging.
A complete cache bounds sparse IN-list estimates by the input count; lookup
cost includes misses. Explicit SQL hints remain authoritative.

The compiled GoogleTests cover cache/leaf/directory boundaries, unsigned ID
extremes, batch tails and output positions, budget accounting/reclamation,
concurrent construction/readers, current/legacy files, duplicate fallback,
row-ID partitions and cache-aware planning. Build and run them with:

```sh
cmake --build build --target gmanticoretest searchd -j 6
cmake -E chdir build src/gtests/gmanticoretest
python3 test/docid-cache/test_integration.py build/src/searchd
```

The Python integration check uses only the standard library and isolated
loopback servers/temporary data. It checks default, zero, refused and admitted cache
budgets, automatic/explicit plans, sparse batches and tails, missing/repeated/
dead IDs, lazy allocation, UPDATE, REPLACE, ALTER, restart and DROP/reclamation. It requires
Python 3.8+ and is invoked explicitly, independently of UberTest/CLT.

Performance reports should identify both binaries and the workload, distinguish
lookup kernel throughput from SQL QPS, include negative/control cases, and
account for the additional memory beside mapped lookup data. Runtime cache-line
selection alone has not established a SQL-QPS gain on 64-byte Linux hardware.

The PR also partitions the separate shared docstore block cache into at most
16 LRU caches and uses the existing seeded XXH3 helper for block keys. Its entry
budgets sum to `docstore_cache_size`; default 16 MiB gives 16 partitions, while
budgets below 2 MiB keep one. Hash-table and allocator overhead are additional.
Partition capacity and a single hot block can still limit reuse or concurrency.
The reader registry keeps its original locked lookup; no reader TLS or global
generation counter is included in this PR.

`DocstoreBlockCacheTest` covers clear/index-key reuse, concurrent reads under
eviction pressure, and large fields with tiny-cache fallback. The native DocID
tests also cover distinct unsigned 32-bit batch output types, including the
Windows `RowID_t`/`uint32_t` ABI difference. Block-cache performance comparisons
must state the DocID budget and compare equal budgets in both binaries. A
same-binary comparison of `docstore_cache_size=0` and `16m` answers a separate
question from the benefit of changing the block-cache implementation.

`docstore_cache=1` enables the adaptive implementation and is the default;
`docstore_cache=0` disables block caching regardless of the configured budget.
`docstore_cache_size=0` also disables it. The cache retains the configured
`docstore_cache_size` hard entry limit and reclaims cold, unreferenced blocks
after a stable smaller hot set is observed. Both modes already allocate only
on demand; the adaptive change adds reclamation rather than preallocation or
automatic growth beyond the configured ceiling. Lookup epochs are per shard,
updated under the existing lock, and require no TLS or global atomic counter.
Idle shards do not advance epochs and no low-hit-rate bypass is implemented.

The public stored-field tests retain fixed-policy and adaptive controls.
`AdaptiveLru` tests additionally check demand growth, hard limits, hot-set
changes, unchanged fixed-policy retention, pinned references and concurrent reads
under admission/reclamation pressure. The `docstore_cache_used_bytes` status
metric reports charged cached entry bytes, separately from process RSS. Compare
fixed/adaptive at the same ceiling before attributing an effect to the new
policy; raising 16 MiB to 512 MiB is a separate capacity comparison.
The adaptive follow-up uses per-shard virtual-memory pools for decoded payloads,
with direct decompression. Small blocks use 256-byte size classes packed into
64 KiB regions; large blocks use page-rounded dedicated mappings. Empty regions
are unmapped, with at most one 64 KiB empty spare per shard. Partially occupied
regions can retain unused slots, separately from charged entry bytes. This
bypasses process heap retention. The fixed-policy test control and oversized/mapping-failure
fallbacks still use the heap. Metadata and temporary/input/output buffers also remain on the heap.
The mapped/spare status metrics account for pool mappings, including in-flight
blocks; the entry ceiling does not bound total mappings or process RSS.
Pool tests cover reuse, excess unmapping, rounding, no-spare mode and concurrent
allocation/release. Stored-field tests also cover uncompressed small/large
fields and clear/recreate in both modes. Measure process RSS and footprint
alongside entry counters before claiming physical memory savings.

`python3 test/docid-cache/test_blockcache_config.py build/src/searchd` checks
adaptive behavior with the default or explicit enable setting, explicit disable
at both normal and large budgets, zero-sized budgets, and stored-field
replacement/cache cleanup. The fixed-policy control is not a server setting.
