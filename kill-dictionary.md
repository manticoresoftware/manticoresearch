# Kill Dictionary (RT Disk Chunks)

## Overview
The kill dictionary is a per-disk-chunk, in-memory map of `(wordID -> docs/hits)` for killed rows. It fixes incorrect `docs`/`hits` and ranking weights when a chunk still contains dictionary stats for documents that were removed by REPLACE/DELETE. It does not modify the on-disk dictionary; it subtracts kill stats at query time.

For keyword dictionaries, kill stats are keyed by a 64-bit FNV hash (`sphFNV64`) of the normalized token, not by the raw CRC32 wordid, so CRC collisions (different words with the same CRC) do not corrupt the stats.

As discussed on a recent dev call, this feature is intended to solve https://github.com/manticoresoftware/manticoresearch/issues/4066 without requiring disabling the double-buffer functionality.

## When It Applies
- Enabled only if all indexed fields are stored in docstore.
- Applies to RT disk chunks (`CSphIndex_VLN`).
- Used by queries that require word stats (search and `CALL KEYWORDS`).

## Common Use Cases
- High-churn catalog/feed: an ecommerce site runs a bulk reindex job for the same 50k SKUs every 15 minutes because titles/specs can change and full-text fields must be reindexed. That pipeline uses REPLACE (UPDATE would not reindex text fields). Old versions stay in disk chunks until a merge, so common terms like "iphone" or "charger" appear to match more docs/hits than actually exist, skewing BM25.
- Delete-heavy retention: a news site keeps only the last 30 days of articles and deletes expired rows nightly. Until optimize/merge, those deleted docs still contribute to dictionary stats, so terms from yesterday's expired stories inflate docs/hits and affect ranking for fresh content.
- Soft-dedup ingestion: a logs pipeline replays the last hour on restart, re-ingesting the same event IDs. Each replay REPLACE kills the prior row, but raw dictionary stats still count the killed copies, so frequent tokens (hostnames, error codes) get overweighted.

## Configuration and Runtime Control
- **Config option** (searchd section): `kill_dictionary = 0|realtime|flush|idle`
  - Default: `idle`.
  - `0` disables the kill dictionary entirely.
- **Runtime toggle**: `SET GLOBAL kill_dictionary = 0|realtime|flush|idle`
  - `SHOW [GLOBAL] VARIABLES LIKE 'kill_dictionary'` reports the current value.
- **Idle rebuild timeout** (searchd section): `kill_dictionary_idle_timeout = 15s` (default)
  - Used only with `kill_dictionary=idle`. `-1` disables idle rebuilds.
  - Accepts seconds or `ms/s/m/h/d` suffixes (for example, `1500ms`, `15s`, `1m`).
- **Runtime idle timeout**: `SET GLOBAL kill_dictionary_idle_timeout = 15s`
  - `SHOW [GLOBAL] VARIABLES LIKE 'kill_dictionary_idle_timeout'` reports the current value (seconds).
- `indextool` reads the same `kill_dictionary` setting from the `searchd` config section.

## Behavior
### Build and Load
1. Killed rows are tracked in the dead-row map (`.spm`).
2. On load/rotate:
   - `realtime` / `flush`: build kill stats for each chunk. If `.spks` exists, load it; otherwise re-tokenize killed docs from docstore.
   - `idle`: load `.spks` if present; if missing, defer the full rebuild until the idle timeout.
3. For each word in killed docs, the kill dictionary stores:
   - `docs`: unique documents killed for that word
   - `hits`: total killed occurrences

### Query-Time Adjustment
For each term and chunk, the effective stats are:
- `docs = docs - kill_docs`
- `hits = hits - kill_hits`

### Update Modes
- `realtime`: kills are applied immediately (re-tokenize each newly killed doc).
- `flush`: killed rows are queued and applied at the next disk-chunk flush/merge; queries in between use the last built corrections.
- `idle`: kills are applied after the write stream goes idle (after `kill_dictionary_idle_timeout`); queries in between use raw stats for those kills.
- In `flush` and `idle` modes, queries do not wait for rebuilds; if a rebuild is in progress, raw stats are used for that chunk.

What to expect from `flush`:
- Stats become accurate after each disk-chunk flush (not immediately on every REPLACE/DELETE).
- Write latency is smoother than `realtime` because per-kill work is deferred.
- Flush operations can take longer because kill stats are applied synchronously during the flush.
- Overall throughput can be similar to, or slightly worse than, `realtime` depending on flush frequency and dataset.

## Persistence and Storage
- The in-memory kill dictionary can be persisted as `.spks` per disk chunk.
- `.spks` contains aggregated `(wordID, docs, hits)` for killed docs in that chunk.
- `.spks` includes a magic/version header; if it does not match, the file is ignored and rebuilt.
- `.spks` is written on flush/merge rebuilds (or idle rebuilds), not on every REPLACE/DELETE.
- A disk-chunk flush updates `.spks` for that chunk when its kill stats are dirty.
- If the process dies before a flush/merge, `.spks` (and `.spm`) may not include the most recent kills.
 - `.spks` intentionally does not reuse the `.spi` on-disk format: `.spi` is a dictionary index (checkpoints/offsets into wordlists) and assumes the wordlist layout, while kill stats are a direct `(wordID -> docs/hits)` map keyed by the kill-stats wordID. Reusing `.spi` would require a synthetic wordlist/offset scheme and adds complexity without real reuse of code paths.

## Observability
- `SHOW TABLE <rt> STATUS` includes `kill_dictionary_dirty_chunks` (chunks pending kill-stats rebuild, missing or dirty `.spks`).
- `SHOW THREADS OPTION FORMAT=ALL` shows `KILLSTATS <chunk>` while background kill-stats rebuild jobs are running.

## Tooling Notes (indextool)
- `indextool --dumpdict` outputs raw `docs`/`hits` plus `docs_eff`/`hits_eff` and `chunk_id`.
- `chunk_id` is the disk chunk id; RAM segments are marked as `-1`.
- RAM-segment words are length-prefixed; the dump skips the length byte so words are printed cleanly.

## Tooling Notes (mysql_word_counts)
`misc/mysql_word_counts` is a small Rust CLI used to count words from table data and compare them with dictionary output:
- Default mode reads all rows via `SELECT * FROM t LIMIT -1` over MySQL protocol and prints `word<TAB>count`.
- `--from-indextool` reads `indextool --dumpdict` output from stdin and aggregates by `keyword`/`hits`.

## Performance and Memory
Building kill stats scales with the number of killed rows:
- **Cost**: loading `.spks` is proportional to the number of entries; re-tokenization scales with killed docs and their text size.
- **Memory**: one hash entry per unique token in killed docs; each entry stores a 64-bit wordID, two 32-bit counters, and hash pointers.
- **Query overhead**: constant-time hash lookup per term.

### Memory Example
Killed docs:
- doc 1: `alpha alpha beta`
- doc 2: `beta gamma`
- doc 3: `beta`

Kill stats:
- `alpha`: docs=1, hits=2
- `beta`: docs=3, hits=3
- `gamma`: docs=1, hits=1

Approximate RAM cost:
- Per entry: at least 16 bytes for `{wordID, docs, hits}` plus hash-node overhead; expect ~32–64 bytes per entry depending on allocator/pointer sizes.
- 3 entries: ~96–192 bytes for the entries themselves.
- Plus the fixed bucket table. The kill dictionary uses 65,536 hash buckets (see `m_tKillStats`), so the bucket array alone is ~512 KB (8 bytes × 65,536).
  - A “bucket” is a slot in the hash table that points to a linked list of entries for words that hash to that slot.
  - This is a fixed cost per chunk when the kill dictionary is built; it does not grow with the number of words.

## Behavior During Merge/Optimize
- A merged chunk is built from alive documents with a fresh dictionary/docstore.
- The merged chunk starts with an empty kill dictionary; `.spks` is rebuilt as part of writing the new chunk.
- Old chunks and their `.spks` files are discarded.
- If kills occur later, kill stats are rebuilt/updated according to the active mode.

## Implementation Notes (Key Functions)
### Config and Runtime Controls
- `src/searchd.cpp::ConfigureSearchd()` reads `kill_dictionary` and `kill_dictionary_idle_timeout`.
- `src/searchd.cpp::HandleSetGlobal()` supports `SET GLOBAL kill_dictionary` and `SET GLOBAL kill_dictionary_idle_timeout`.
- `src/searchd.cpp::HandleMysqlShowVariables()` exposes the current values.
- `src/sphinxint.h`: defines `KillDictionaryMode_e`, helpers like `KillDictionaryEnabled()` and `ParseKillDictionaryMode()`.
- `src/sphinx.cpp`: initializes the global default `g_eKillDictionaryMode` (defaults to `idle`).

### Persistence
- `src/indexfiles.h` / `src/indexfiles.cpp`: register `.spks` (`SPH_EXT_SPKS`).
- `src/sphinx.cpp::CSphIndex_VLN::FlushKillStatsFile()` writes `.spks`; in `flush` mode it applies queued rowids incrementally instead of forcing a full rebuild.
- `src/sphinx.cpp::CSphIndex_VLN::HasKillStatsDirty()` treats missing `.spks` as dirty.
- `src/sphinxint.h::KillStatsFileHeader_t` defines the `.spks` header (magic, version, `dead_rows`, `entries`).
- `src/sphinx.cpp::CSphIndex_VLN::LoadKillStatsFile()` loads `.spks` at preread.
 - `src/sphinx.cpp::CSphIndex_VLN::QueueKillStatsRows()` stores killed rowids for `flush` mode.

### Idle Rebuild Scheduling
- `src/sphinxrt.cpp::SetRtKillStatsIdleTimeout()` / `GetRtKillStatsIdleTimeout()` store the idle timeout.
- `src/sphinxrt.cpp::RtIndex_c::ConditionalKillStats()` runs idle rebuilds in parallel and labels jobs as `KILLSTATS <chunk>`.
  - The job counter uses a shared `Coro::Waitable_T<int>` so it stays alive until all jobs finish.

### Status and Tooling
- `src/sphinxrt.cpp::RtIndex_c::GetStatus()` increments `kill_dictionary_dirty_chunks` per dirty chunk.
- `src/searchd.cpp::AddDiskIndexStatus()` exposes `kill_dictionary_dirty_chunks` in `SHOW TABLE <rt> STATUS`.
- `src/indextool.cpp::LoadKillDictionarySetting()` reads the config so `indextool` matches server behavior.
- `src/sphinxrt.cpp::RtIndex_c::DebugDumpDict()` prints `docs_eff`, `hits_eff`, and `chunk_id`.

### RAM-Segment Stats Correction
- `src/sphinxrt.cpp::RtIndex_c::RtQwordSetupSegment()` recomputes docs/hits for RAM segments with dead rows and returns `false` if all docs are dead (treats the word as not found).

### Query-Time Adjustment and Realtime Updates
- `src/sphinx.cpp::CSphIndex_VLN::ApplyKillStats()` subtracts kill stats from dictionary term stats during query setup.
- `src/sphinx.cpp::DiskIndexQwordSetup_c::Setup()` calls `ApplyKillStats()` so query-time stats are corrected.
- `src/sphinx.cpp::CSphIndex_VLN::GetKillStats()` controls when kill stats are built/used based on the mode.
- `src/sphinx.cpp::CSphIndex_VLN::KillStatsKey()` computes the key used for kill stats; keyword dictionaries use `sphFNV64` on the normalized token to avoid CRC collisions.
- `src/sphinx.cpp::CSphIndex_VLN::Kill()`, `KillMulti()`, `KillDupes()`, and `CheckThenKillMulti()` mark kill-stats dirty and update stats immediately in `realtime` mode.

## Validation (Local)
The following end-to-end check was used to validate that `indextool --dumpdict` aggregates match the live query stats:

```bash
wget -O /tmp/words.txt https://raw.githubusercontent.com/dwyl/english-words/refs/heads/master/words.txt
grep -v "/" /tmp/words.txt | awk 'length($0) < 42' > /tmp/words.sanitized.txt

mysql -P9306 -h0 -e "set global kill_dictionary=idle; set global kill_dictionary_idle_timeout='1s'"

manticore-load --drop --cache-gen-workers=10 --wait --quiet \
  --cache-from-disk \
  --batch-size=1000 \
  --threads=5 \
  --total=1000000 \
  --init="CREATE TABLE t(id bigint, f text, type int) optimize_cutoff='70'" \
  --load="REPLACE INTO t(id,f,type) VALUES(<int/1/300000>,'<text/{/tmp/words.sanitized.txt}/100/1000>',<int/1/100>)"

mysql -P9306 -h0 -e "flush ramchunk t"
bash -c 'while :; do v=$(mysql -NB -P9306 -h0 -e "show table t status like '\''kill_dictionary_dirty_chunks'\''" | awk "{print \$2}"); [ "$v" = "0" ] && break; sleep 1; done'

/Users/sn/manticore_github/misc/mysql_word_counts/target/release/mysql_word_counts | md5sum
# expected: abb1aca05a50d7b556af04fdd3f522f1
/Users/sn/manticore_github/build-kill-dictionary/src/indextool --dumpdict t --skip-lock | \
  /Users/sn/manticore_github/misc/mysql_word_counts/target/release/mysql_word_counts --from-indextool | md5sum
# expected: abb1aca05a50d7b556af04fdd3f522f1
```

Note: the two MD5 sums above are expected to match. If they do not, it usually means the table or tokenizer settings differ between the live scan and the dictionary, or the kill stats are still dirty.

The following checks validate that ranking weights are stable across multiple chunks (as many as possible with the default `rt_mem_limit`) vs a single optimized chunk.

```bash
mysql -P9306 -h0 -e "set global kill_dictionary=idle; set global kill_dictionary_idle_timeout='1s'"

manticore-load --drop --cache-gen-workers=10 --wait --quiet --cache-from-disk \
  --batch-size=1000 \
  --threads=5 \
  --total=1000000 \
  --init="CREATE TABLE t(id bigint, f text, type int) optimize_cutoff='999'" \
  --load="REPLACE INTO t(id,f,type) VALUES(<int/1/300000>,'<text/{/tmp/words.sanitized.txt}/100/1000>',<int/1/100>)"

mysql -P9306 -h0 -e "flush ramchunk t"
bash -c 'while :; do v=$(mysql -NB -P9306 -h0 -e "show table t status like '\''kill_dictionary_dirty_chunks'\''" | awk "{print \$2}"); [ "$v" = "0" ] && break; sleep 1; done'

mysql -NB -P9306 -h0 -e "select id, weight() from t where match('about') order by weight() desc, id asc limit 50;
select id, weight() from t where match('people') order by weight() desc, id asc limit 50;
select id, weight() from t where match('time') order by weight() desc, id asc limit 50;" | md5sum
# expected: c3c3772086279678cee80b9de2c92b7a

mysql -P9306 -h0 -e "optimize table t option sync=1, cutoff=1"

mysql -NB -P9306 -h0 -e "select id, weight() from t where match('about') order by weight() desc, id asc limit 50;
select id, weight() from t where match('people') order by weight() desc, id asc limit 50;
select id, weight() from t where match('time') order by weight() desc, id asc limit 50;" | md5sum
# expected: c3c3772086279678cee80b9de2c92b7a
```

## Performance Comparison (Local)
All runs used the same load command and dataset (`optimize_cutoff='999'`, 1,000,000 docs, 5 threads, batch 1000). For `idle`/`flush`, `kill_dictionary_idle_timeout=1s` and we measured the time to wait until `kill_dictionary_dirty_chunks` drops to zero after `FLUSH RAMCHUNK`.

| Mode        | Load time | Dirty wait (s) | Notes |
|-------------|-----------|----------------|-------|
| `0`         | 02:21     | N/A            | Kill dictionary disabled |
| `idle`      | 02:17     | 32             | `kill_dictionary_idle_timeout=1s` |
| `flush`     | 05:15     | 0              | Per-flush incremental updates |
| `realtime`  | 04:33     | N/A            | Immediate per-kill updates |

Why `flush` can still be slower than `realtime`:
- `flush` applies kill-stats updates inside the disk-chunk flush path (single-threaded and synchronous), so each flush pays the full tokenization cost for pending kills.
- `realtime` spreads the same work across writer threads as REPLACE/DELETE happens, which amortizes the cost and avoids a large serialized flush penalty.

Example where `flush` beats `realtime` (high overwrite rate + high concurrency):

```bash
mysql -P9306 -h0 -e "set global kill_dictionary=flush; set global kill_dictionary_idle_timeout='1s'"
manticore-load --drop --cache-gen-workers=10 --wait --cache-from-disk \
  --batch-size=200 \
  --threads=16 \
  --total=500000 \
  --init="CREATE TABLE t(id bigint, f text, type int) optimize_cutoff='999' rt_mem_limit='2G'" \
  --load="REPLACE INTO t(id,f,type) VALUES(<int/1/50000>,'<text/{/tmp/words.sanitized.txt}/100/1000>',<int/1/100>)"
mysql -P9306 -h0 -e "flush ramchunk t"
bash -c 'while :; do v=$(mysql -NB -P9306 -h0 -e "show table t status like '\''kill_dictionary_dirty_chunks'\''" | awk "{print $2}"); [ "$v" = "0" ] && break; sleep 1; done'

mysql -P9306 -h0 -e "set global kill_dictionary=realtime"
manticore-load --drop --cache-gen-workers=10 --wait --cache-from-disk \
  --batch-size=200 \
  --threads=16 \
  --total=500000 \
  --init="CREATE TABLE t(id bigint, f text, type int) optimize_cutoff='999' rt_mem_limit='2G'" \
  --load="REPLACE INTO t(id,f,type) VALUES(<int/1/50000>,'<text/{/tmp/words.sanitized.txt}/100/1000>',<int/1/100>)"
mysql -P9306 -h0 -e "flush ramchunk t"
```

Observed results (local):
- `flush`: Total time `01:01`, Docs/sec `8071`
- `realtime`: Total time `01:04`, Docs/sec `7724`

Single thread performance:
```
mysql -P9306 -h0 -e "set global kill_dictionary=flush; set global kill_dictionary_idle_timeout='1s'"
manticore-load --drop --cache-gen-workers=10 --wait --cache-from-disk \
  --batch-size=1000 \
  --threads=1 \
  --total=100000 \
  --init="CREATE TABLE t(id bigint, f text, type int) optimize_cutoff='999'" \
  --load="REPLACE INTO t(id,f,type) VALUES(<int/1/300000>,'<text/{/tmp/words.sanitized.txt}/100/1000>',<int/1/100>)"
time mysql -P9306 -h0 -e "flush ramchunk t"
time bash -c 'while :; do v=$(mysql -NB -P9306 -h0 -e "show table t status like '\''kill_dictionary_dirty_chunks'\''" | awk "{print \$2}"); [ "$v" = "0" ] && break; sleep 1; done'
```

| Mode        | Load time | Dirty wait (s) |
|-------------|-----------|----------------|
| 0           | 05:30     | N/A            |
| `flush`     | 07:55     | 0              |
| `realtime`  | 07:51     | N/A            |
