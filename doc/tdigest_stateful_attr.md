# Stateful TDigest Aggregation Storage

TDigest-based aggregations (percentiles, percentile_ranks, MAD) are the first
attribute family in Manticore that keep **live runtime state** in query matches
instead of only writing serialized blobs at the end of processing. The runtime
state lives in a pointer attribute slot and later gets converted into the usual
serialized TDigest blob that is compatible with disk/index storage.

This document captures the design trade-offs so future changes do not overlook
the hidden statefulness.

## Runtime vs serialized modes

* **Runtime state**
  * Allocated through `sphTDigestRuntimeAllocate()` and stored as an aligned
    `TDigestRuntimeState_t` just behind a `TDigestRuntimeBlobHeader_t`.
  * Holds the mutable digest (`m_tDigest`) and the pending value buffer
    (`m_dPending`) so aggregators can batch updates before flushing.
  * Lives only while the match is owned by a sorter/grouping pipeline. Once the
    state is serialized or dropped, the allocator frees it and the attribute is
    reset.

* **Serialized state**
  * Produced by `StoreTDigestBlob()` once an aggregate needs to be materialised
    into the result set.
  * Contains the TDigest centroids + min/max and is the canonical format stored
    in RT/distributed indexes, query logs, etc.

## Why it matters

1. **Attribute type ambiguity**
   * TDigest runtime blobs currently occupy a normal `SPH_ATTR_STRINGPTR`
     column. Every copy/free call has to *probe the blob payload* to determine
     whether it is a live runtime block or plain data.
   * The helper `sphIsTDigestRuntimeBlob()` peeks into the header/magic and is
     now invoked by `sphCopyPackedAttr`, `sphDeallocatePacked`, and other
     generic pointer attribute helpers.

2. **Alignment guarantees**
   * Runtime states must respect `alignof(TDigestRuntimeState_t)`. The allocator
     requests extra bytes (`RuntimePayloadReservation()`) and uses
     `std::align()` before placement-new’ing the state.
   * Because the blob is stored inside the generic pointer attribute format, we
     cannot assume the payload pointer is aligned unless we manage it ourselves.
     All code that *casts* the payload must only do so after the header has been
     validated and the alignment confirmed.

3. **Lifecycle contracts**
   * Runtime blobs are destroyed via `sphDestroyTDigestRuntimeBlob()` before the
     pointer attribute storage is freed. Skipping this call leaks the `new`
     placement (and leaves stale `m_uMagic`, which misleads the detection code).
   * Copying must clone the runtime state (see `sphCloneTDigestRuntimeBlob()`)
     instead of copying raw bytes, otherwise `m_dPending` would be shared across
     matches.
   * Aggregates must flush pending values *before* serializing or merging
     digests or they risk losing samples that were still buffered in
     `m_dPending`.

4. **Performance trade-offs**
   * Probing every pointer attribute adds a tiny constant overhead to copy/free
     paths; this was accepted to keep TDigest changes local. If a future
     refactor introduces a dedicated `SPH_ATTR_TDIGEST_PTR`, this probing logic
     can be removed.
   * The runtime-vs-serialized duality is invisible to SQL/JSON users. Test
     coverage (e.g. `test/test_482/test.xml`) needs to ensure TDigest columns
     are exercised from both single-shard and distributed paths so regressions
     on the state lifecycle are caught.

## Future directions

* Introduce a proper TDigest pointer attribute type to avoid payload probing
  and enforce alignment at the type level.
* Move runtime state ownership closer to aggregators so generic pointer
  attribute helpers do not need TDigest-specific logic.
* Expand documentation when the attribute type split happens; this file should
  remain the canonical place that describes how the runtime state works.

Until then, any code that copies, frees, or inspects pointer attributes must be
aware that TDigest blobs can hold live runtime state and handle them using the
helpers mentioned above.
