# DocID lookup cache validation

The optional native C++ cache stores sorted numeric document IDs and row IDs
beside the existing compressed lookup. Set `docid_lookup_cache_size = 16m` in
`searchd` to opt in; the default budget is zero. The whole allocation, including
alignment padding and the handle, is charged to one process-wide budget.
Rejected or incomplete caches retain the existing compressed reader. Disk format
and UUID lookup remain unchanged. Chunk destruction/remapping reclaims storage;
there is no eviction or automatic admission retry.

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
loopback servers/temporary data. It checks zero, refused and admitted cache
budgets, automatic/explicit plans, sparse batches and tails, missing/repeated/
dead IDs, UPDATE, REPLACE, ALTER, restart and DROP/reclamation. It requires
Python 3.8+ and is invoked explicitly, independently of UberTest/CLT.

Performance reports should identify both binaries and the workload, distinguish
lookup kernel throughput from SQL QPS, include negative/control cases, and
account for the additional memory beside mapped lookup data. Runtime cache-line
selection alone has not established a SQL-QPS gain on 64-byte Linux hardware.
