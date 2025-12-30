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
- **Config option** (searchd section): `kill_dictionary = 0|realtime|flush|idle`
  - Default: `idle` (enabled with idle rebuilds).
- **Runtime toggle**: `SET GLOBAL kill_dictionary = 0|realtime|flush|idle`
  - `SHOW [GLOBAL] VARIABLES LIKE 'kill_dictionary'` reports the current value (string).
- `indextool` reads the same setting from the `searchd` section in the config file.
- **Idle rebuild timeout** (searchd section): `kill_dictionary_idle_timeout = 15s` (default)
  - Used only with `kill_dictionary=idle`. `-1` disables idle rebuilds.
  - Accepts time values as seconds or with `ms/s/m/h/d` suffixes (for example, `1500ms`, `15s`, `1m`).
- **Runtime idle timeout**: `SET GLOBAL kill_dictionary_idle_timeout = 15s`
  - `SHOW [GLOBAL] VARIABLES LIKE 'kill_dictionary_idle_timeout'` reports the current value (seconds).

## How It Works
1. **Killed rows are tracked** via the dead-row map (`.spm`), as before.
2. **On load/rotate**:
   - If `kill_dictionary` is `realtime` or `flush`, the kill dictionary is built:
     - Scan the dead-row map.
     - If a persisted kill-stats file exists, load aggregated `(wordID, docs, hits)` counts.
     - Otherwise, fetch stored fields from docstore and re-tokenize using the indexing pipeline:
       - tokenizer clone
       - index token filter
       - AOT filters
       - HTML stripping
       - field filter
       - morph fields
   - If `kill_dictionary` is `idle`, only a persisted kill-stats file is loaded; a full rebuild is deferred until the idle timeout fires.
   - For each word, accumulate:
     - `docs` (unique docs per word)
     - `hits` (total occurrences per word)
3. **At query time**, the word stats from the chunk dictionary are adjusted:
   - `docs = docs - kill_docs`
   - `hits = hits - kill_hits`
4. **Incremental updates**:
   - `realtime`: kills after the initial build are added immediately (per-kill re-tokenization).
   - `flush`: kills after the initial build are **not** applied until the next flush/merge rebuilds the stats; queries in between will use stale `docs`/`hits` corrections for those kills.
   - `idle`: kills are only corrected after the write stream goes idle (after `kill_dictionary_idle_timeout`); until then, queries use raw chunk dictionary stats for those kills.
   - Persisted `.spks` is updated on flush/merge (or on idle rebuilds), not on every REPLACE/DELETE.
   - In `flush` and `idle` modes, queries do not wait for kill-stats rebuilds; if a rebuild is in progress, the query uses raw stats for that chunk.

## Backward Compatibility
- Disk chunks may include an optional kill-stats file (`.spks`). Older binaries will ignore it.
- Old chunks load as-is; the kill dictionary is rebuilt in memory when needed.
- Clusters with mixed versions will still read the same files; only nodes with this feature enabled will apply the kill correction at query time.
- If an index does not store all indexed fields and no persisted kill-stats file exists, it behaves exactly as before (kill dictionary disabled).

## Performance Considerations
Building the kill dictionary is proportional to the number of killed rows and the data source used:
- **Cost**: if the persisted kill-stats file exists, load time is proportional to the number of words in that file. If the file is missing, it falls back to docstore re-tokenization.
- **Memory**: per-word hash map of doc/hit counts for killed docs. Memory grows with the number of unique tokens present in the killed documents, not with the total index size. The dictionary does **not** store the token text; it stores a `SphWordID_t` (64-bit word ID) plus two 32-bit counters, and hash-node pointers for chaining/ordering.
- **Query-time overhead**: constant-time hash lookup per term to subtract `docs`/`hits`.

Incremental kills are fast when the cache is already built; they only tokenize the newly killed documents. In `flush` mode, this per-kill cost is skipped, and the cost shifts to the next flush/merge rebuild.

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

## Observability
- `SHOW TABLE <rt> STATUS` includes `kill_dictionary_dirty_chunks`, the number of disk chunks that have pending kill-stats rebuilds (missing or dirty `.spks`).
- `SHOW THREADS OPTION FORMAT=ALL` shows background jobs as `KILLSTATS <chunk>` while an idle rebuild is running.

## Persistence and Disk Storage
- The kill dictionary cache itself is in-memory, but aggregated kill stats can be persisted.
- Persisted data remains unchanged:
  - Dictionary (`.spi`, `.spd`, etc.)
  - Dead-row map (`.spm`)
  - Docstore files
- When `kill_dictionary` is `realtime`, `flush`, or `idle`, disk chunks may store a kill-stats file:
  - `.spks` (aggregated `wordID`, `docs`, `hits`)
  - This file is written when dead-row maps are flushed (after `SaveDiskChunk` and during merge/optimize), not on every kill.
  - A flush updates `.spks` for all disk chunks that have dead rows, not just the newly saved chunk.
- With `kill_dictionary=idle`, `.spks` is written after the idle timeout (not on every flush); corrections appear once idle rebuilds run.
- If the process dies before a flush/merge, `.spks` (and `.spm`) may not include the most recent kills.
- On restart or table reload, the in-memory kill dictionary cache is empty and rebuilt eagerly at load/rotate (if enabled) using `.spks` if present, otherwise via docstore re-tokenization.

## Tooling Notes (indextool)
- `indextool --dumpdict` always outputs raw `docs`/`hits` plus extra columns `docs_eff`/`hits_eff` and `chunk_id`.
- `chunk_id` is the disk chunk id; RAM segments are marked as `-1`.
- The effective columns are adjusted by the kill dictionary (same as query-time stats).
- The authoritative values for corrected stats are from `SHOW META` / `CALL KEYWORDS` (and query-time ranking).

## Behavior During Disk-Chunk Merge/Optimize
During merge/optimize:
- A **new merged chunk** is built from alive documents, with a fresh dictionary and docstore.
- The merged chunk **starts with an empty kill dictionary cache**.
- The kill-stats file (`.spks`) is rebuilt for the merged chunk as part of writing the new disk chunk.
- Old chunks and their kill-stats files are discarded.
- If the merged chunk has dead rows later, the kill dictionary is built at load/rotate (eager) and updated incrementally as new kills happen.

In practice, an optimized chunk typically has correct dictionary stats (since it is rebuilt from alive docs), so the kill dictionary tends to stay empty until new kills happen.
