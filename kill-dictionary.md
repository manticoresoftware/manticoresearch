# Kill Dictionary (RT Disk Chunks)

## Overview
The kill dictionary is a per-disk-chunk, in-memory data structure that tracks how many docs and hits should be subtracted for killed rows. It fixes incorrect `docs`/`hits` and ranking weights when a chunk still contains old dictionary stats but some documents were killed (REPLACE/DELETE).

It does not change the on-disk dictionary. Instead, it rebuilds per-killed-doc statistics from docstore and applies them at query time.

## When It Is Used
- **Enabled only if all indexed fields are stored** in docstore.
  - If any indexed field is not stored, the kill dictionary is disabled.
- Works for disk chunks (RT saved chunks) using `CSphIndex_VLN`.
- Queries that need word stats (search or `CALL KEYWORDS`) will apply the kill dictionary correction.

## Configuration and Runtime Control
- **Config option** (searchd section): `kill_dictionary = 1|0`
  - Default: `1` (enabled).
- **Runtime toggle**: `SET GLOBAL kill_dictionary = 0|1`
  - `SHOW [GLOBAL] VARIABLES LIKE 'kill_dictionary'` reports the current value.
- `indextool` reads the same setting from the `searchd` section in the config file.

## How It Works
1. **Killed rows are tracked** via the dead-row map (`.spm`), as before.
2. **On first use**, the kill dictionary is built lazily:
   - Scan the dead-row map.
   - For each dead row, fetch stored fields from docstore.
   - Re-tokenize the document using the same indexing pipeline:
     - tokenizer clone
     - index token filter
     - AOT filters
     - HTML stripping
     - field filter
     - morph fields
   - For each token, accumulate:
     - `docs` (unique docs per word)
     - `hits` (total occurrences per word)
3. **At query time**, the word stats from the chunk dictionary are adjusted:
   - `docs = docs - kill_docs`
   - `hits = hits - kill_hits`
4. **Incremental updates**:
   - If a kill happens after the kill dictionary is built, the killed doc is re-tokenized and its stats are added to the kill dictionary immediately.

## Backward Compatibility
- **No on-disk format changes** (no new files, no header changes).
- Old chunks load as-is; the kill dictionary is rebuilt in memory when needed.
- Clusters with mixed versions will still read the same files; only nodes with this feature enabled will apply the kill correction at query time.
- If an index does not store all indexed fields, it behaves exactly as before (kill dictionary disabled).

## Performance Considerations
Building the kill dictionary is proportional to the number of killed rows and the size of their stored fields:
- **Cost**: re-tokenizing each killed document once (lazy, on demand). "Lazy" means it is built only when word stats are requested (search/`SHOW META`/`CALL KEYWORDS`/`indextool --dumpdict`). If no such queries are executed, the kill dictionary is not built.
- **Memory**: per-word hash map of doc/hit counts for killed docs. Memory grows with the number of unique tokens present in the killed documents, not with the total index size. The dictionary does **not** store the token text; it stores a `SphWordID_t` (64-bit word ID) plus two 32-bit counters, and hash-node pointers for chaining/ordering.
- **Query-time overhead**: constant-time hash lookup per term to subtract `docs`/`hits`.

Incremental kills are fast when the cache is already built; they only tokenize the newly killed documents.

### Memory Example
Assume the killed documents contain these tokens:

- doc 1: `alpha alpha beta`
- doc 2: `beta gamma`
- doc 3: `beta`

The kill dictionary stores one entry per unique token found in the killed docs:

- `alpha`: docs=1 (doc 1), hits=2
- `beta`: docs=3 (docs 1,2,3), hits=3
- `gamma`: docs=1 (doc 2), hits=1

So the in-memory structure has 3 entries total. If 10,000 killed docs all share the same 100 distinct tokens, the kill dictionary still has ~100 entries. If every killed doc has unique tokens, the map grows with that unique-token count.

Approximate RAM cost for this example:

- Per entry: ~40 bytes raw (8-byte word ID + 8 bytes of counters + 24 bytes of pointers), typically ~48–64 bytes after allocator overhead/alignment.
- 3 entries: ~120–192 bytes total.
- Plus the fixed 256-bucket pointer table: ~2 KB.

So the example above would cost roughly ~2.1–2.3 KB of extra RAM. With 100 unique tokens, expect ~6–8 KB plus the same ~2 KB bucket table.

## Persistence and Disk Storage
- The kill dictionary **is not persisted to disk**.
- Persisted data remains unchanged:
  - Dictionary (`.spi`, `.spd`, etc.)
  - Dead-row map (`.spm`)
  - Docstore files
- On restart or table reload, the in-memory kill dictionary cache is empty and rebuilt lazily when needed.

## Tooling Notes (indextool)
- `indextool --dumpdict` always outputs raw `docs`/`hits` plus extra columns `docs_eff`/`hits_eff` and `chunk_id`.
- `chunk_id` is the disk chunk id; RAM segments are marked as `-1`.
- The effective columns are adjusted by the kill dictionary (same as query-time stats).
- The authoritative values for corrected stats are from `SHOW META` / `CALL KEYWORDS` (and query-time ranking).

## Behavior During Disk-Chunk Merge/Optimize
During merge/optimize:
- A **new merged chunk** is built from alive documents, with a fresh dictionary and docstore.
- The merged chunk **starts with an empty kill dictionary cache**.
- If the merged chunk has dead rows later, the kill dictionary is built as usual (lazy on first need, incremental after that).

In practice, an optimized chunk typically has correct dictionary stats (since it is rebuilt from alive docs), so the kill dictionary tends to stay empty until new kills happen.
