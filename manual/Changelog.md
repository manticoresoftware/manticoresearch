# Changelog

## Version 3.5.2, dev

### New features

* OPTIMIZE reduce disk chunks to a number of chunks ( default is 2* No. of cores) instead of a single one. The optimal number of chunks can be controlled by [cutoff](Securing_and_compacting_an_index/Compacting_an_index.md#Number-of-optimized-disk-chunks) option.
* NOT operator can be now used standalone. By default it is disabled since accidental single NOT queries can be slow. It can be enabled by setting new searchd directive [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) to `0`.
* new setting [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) sets how many threads a query can use. If the directive is not set, a query can use threads up to the value of [threads](Server_settings/Searchd.md#threads).
Per `SELECT` query the number of threads can be limited with [OPTION threads=N](Searching/Options.ms#threads) overriding the global `max_threads_per_query`.
* Percolate indexes can be now be imported with [IMPORT TABLE](Adding_data_from_external_storages/Adding_data_from_indexes/Importing_index)
* HTTP API `/search` receive basic support for [faceting](Searching/Faceted_search.md#HTTP)/[grouping](Searching/Grouping/md) by new query node `aggs`

### Minor changes

* If no replication listen directive is declared, the engine will try to use ports after the defined 'sphinx' port, up to 200.
* `listen=...:sphinx` needs to be explicit set for SphinxSE connections or SphinxAPI clients
* [SHOW INDEX STATUS](Profiling_and_monitoring/Index_settings_and_status/SHOW_INDEX_STATUS.md) outputs new metrics: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` and `disk_mapped_cached_hitlists`
* SQL `\status` now outputs `Queue\Threads` and `Tasks\Threads`

### Deprecations:

* `dist_threads` is completely deprecated now, searchd will log a warning if the directive is still used

### Docker

The official Docker image is now based on Ubuntu 20.04 LTS

### Bugifixes

[2a474dc1](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Crash of daemon at grouper at RT index with different chunks
[57a19e5a](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Fastpath for empty remote docs
[07dd3f31](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Expression stack frame detection runtime
[08ae357c](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Matching above 32 fields at percolate indexes
[16b9390f](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Replication listen ports range
[5fa671af](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Show create table on pq
[54d133b6](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS port behavior
[fdbbe524](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Mixing docstore rows when replacing
[afb53f64](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Switch TFO unavailable message level to 'info'
[59d94cef](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Crash on strcmp invalid use
[04af0349](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Adding index to cluster with system (stopwords) files
[50148b4e](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Merge indexes with large dictionaries; RT optimize of large disk chunks
[a2adf158](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool can dump meta from current version
[69f6d5f7](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Issue in group order in GROUP N
[24d5d80f](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Explicit flush for SphinxSE after handshake
[31c4d78a](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Avoid copy of huge descriptions when not necessary
[2959e2ca](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Negative time in show threads
[f0b35710](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Token filter plugin vs zero position deltas
[a49e5bc1](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Change 'FAIL' to 'WARNING' on multiple hits

## Version 3.5.0, 22 Jul 2020

### Major new features:
* This release took so long, because we were working hard on changing multitasking mode from threads to **coroutines**. It makes configuration simpler and queries parallelization much more straightforward: Manticore just uses given number of threads (see new setting [threads](Server_settings/Searchd.md#threads)) and the new mode makes sure it's done in the most optimal way.
* Changes in [highlighting](Searching/Highlighting.md#Highlighting-options):
  - any highlighting that works with several fields (`highlight({},'field1, field2'`) or `highlight` in json queries) now applies limits per-field by default.
  - any highlighting that works with plain text (`highlight({}, string_attr)` or `snippet()` now applies limits to the whole document.
  - [per-field limits](Searching/Highlighting.md#limits_per_field) can be switched to global limits by `limits_per_field=0` option (`1` by default).
  - [allow_empty](Searching/Highlighting.md#allow_empty) is now `0` by default for highlighting via HTTP JSON.

* The same port [can now be used](Server_settings/Searchd.md#listen) for http, https and binary API (to accept connections from a remote Manticore instance). `listen = *:mysql` is still required for connections via mysql protocol. Manticore now detects automatically the type of client trying to connect to it except for MySQL (due to restrictions of the protocol).

* In RT mode a field can now be [text and string attribute](Creating_an_index/Data_types.md#String) at the same time - [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331).

  In [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode) it's called `sql_field_string`. Now it's available in [RT mode](Read_this_first.md#Real-time-mode-vs-plain-mode) for real-time indexes too. You can use it as shown in the example:

  <!-- more -->
  ```sql
  create table t(f string attribute indexed);
  insert into t values(0,'abc','abc');
  select * from t where match('abc');
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.01 sec)

  mysql> select * from t where f='abc';
  +---------------------+------+
  | id                  | f    |
  +---------------------+------+
  | 2810845392541843463 | abc  |
  +---------------------+------+
  1 row in set (0.00 sec)
  ```
  <!-- \more -->

### Minor changes
* You can now [highlight string attributes](Searching/Highlighting.md#Highlighting-via-SQL).
* SSL and compression support for SQL interface
* Support of mysql client [`status`](Profiling_and_monitoring/Node_status.md#STATUS) command.
* [Replication](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) can now replicate external files (stopwords, exceptions etc.).
* Filter operator [`in`](Searching/Filters.md#Set-filters) is now available via HTTP JSON interface.
* [`expressions`](Searching/Expressions.md#expressions) in HTTP JSON.
* [You can now change `rt_mem_limit` on the fly](https://github.com/manticoresoftware/manticoresearch/issues/344) in RT mode, i.e. can do `ALTER ... rt_mem_limit=<new value>`.
* You can now use [separate CJK charset tables](Creating_an_index/NLP_and_tokenization/CJK.md): `chinese`, `japanese` and `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) now limits maximum thread stack, not initial.
* Improved `SHOW THREADS` output.
* Display progress of long `CALL PQ` in `SHOW THREADS`.
* cpustat, iostat, coredump can be changed during runtime with [SET](Server_settings/Setting_variables_online.md#SET).
* `SET [GLOBAL] wait_timeout=NUM` implemented ,

### Breaking changes:
* **Index format has been changed.** Indexes built in 3.5.0 cannot be loaded by Manticore version < 3.5.0, but Manticore 3.5.0 understands older formats.
* [`INSERT INTO PQ VALUES()`](Adding_documents_to_an_index/Adding_rules_to_a_percolate_index.md) (i.e. without providing column list) previously expected exactly `(query, tags)` as the values. It's been changed to `(id,query,tags,filters)`. The id can be set to 0 if you want it to be auto-generated.
* [`allow_empty=0`](Searching/Highlighting.md#allow_empty) is a new default in highlighting via HTTP JSON interface.
* Only absolute paths are allowed for external files (stopwords, exceptions etc.) in `CREATE TABLE`/`ALTER TABLE`.

### Deprecations:
* `ram_chunks_count` was renamed to `ram_chunk_segments_count` in `SHOW INDEX STATUS`.
* `workers` is obsolete. There's only one workers mode now.
* `dist_threads` is obsolete. All queries are as much parallel as possible now (limited by `threads` and `jobs_queue_size`).
* `max_children` is obsolete. Use [threads](Server_settings/Searchd.md#threads) to set the number of threads Manticore will use (set to the # of CPU cores by default).
* `queue_max_length` is obsolete. Instead of that in case it's really needed use [jobs_queue_size](Server_settings/Searchd.md#jobs_queue_size) to fine-tune internal jobs queue size (unlimited by default).
* All `/json/*` endpoints are now available w/o `/json/`, e.g. `/search`, `/insert`, `/delete`, `/pq` etc.
* `field` meaning "full-text field" was renamed to "text" in `describe`.
  <!-- more -->
  **3.4.2:**
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | field  | indexed stored |
  +-------+--------+----------------+
  ```
  **3.5.0**:
  ```sql
  mysql> describe t;
  +-------+--------+----------------+
  | Field | Type   | Properties     |
  +-------+--------+----------------+
  | id    | bigint |                |
  | f     | text   | indexed stored |
  +-------+--------+----------------+
  ```
  <!-- \more -->
* Cyrillic `и` doesn't map to `i` in `non_cjk` charset_table (which is a default) as it affected Russian stemmers and lemmatizers too much.
* `read_timeout`. Use [network_timeout](Server_settings/Searchd.md#network_timeout) instead which controls both reading and writing.


### Packages

* Ubuntu Focal 20.04 official package
* deb package name changed from `manticore-bin` to `manticore`

### Bugfixes:
1. [#351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd memory leak
2. [ceabe44f](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Tiny read out of bounds in snippets
3. [1c3e84a3](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Dangerous write into local variable for crash queries
4. [26e094ab](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Tiny memory leak of sorter in test 226
5. [d2c7f86a](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Huge memory leak in test 226
6. [0dd80122](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Cluster shows the nodes are in sync, but `count(*)` shows different numbers
7. [f1c1ac3f](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Cosmetic: Duplicate and sometimes lost warning messages in the log
8. [f1c1ac3f](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Cosmetic: (null) index name in log
9. [359dbd30](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Cannot retrieve more than 70M results
10. [19f328ee](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Can't insert PQ rules with no-columns syntax
11. [bf685d5d](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Misleading error message when inserting a document to an index in a cluster
12. [2cf18c83](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` and `json/update` return id in exponent form
13. [#324](https://github.com/manticoresoftware/manticoresearch/issues/324) Update json scalar properties and mva in the same query
14. [d38409eb](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` doesn't work in RT mode
15. [5813d639](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `ALTER RECONFIGURE` in rt mode should be disallowed
16. [5813d639](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` gets reset to 128M after searchd restart
17. highlight() sometimes hangs
18. [7cd878f4](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Failed to use U+code in RT mode
19. [2b213de4](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Failed to use wildcard at wordforms at RT mode
20. [e9d07e68](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Fixed `SHOW CREATE TABLE` vs multiple wordform files
21. [fc90a84f](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON query without "query" crashes searchd
22. Manticore [official docker](https://hub.docker.com/r/manticoresearch/manticore) couldn't index from mysql 8
23. [23e05d32](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert requires id
24. [bd679af0](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` doesn't work for PQ
25. [bd679af0](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` doesn't work properly for PQ
26. [5eacf28f](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) End of line in settings in show index status
27. [cb153228](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Empty title in "highlight" in HTTP JSON response
28. [#318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` infix error
29. [9040d22c](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT crashes under load
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Lost crash log on crash at RT disk chunk
31. [#323](https://github.com/manticoresoftware/manticoresearch/issues/323) Import table fails and closes the connection
32. [6275316a](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` corrupts a PQ index
33. [9c1d221e](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Searchd reload issues after change index type
34. [71e2b5bb](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Daemon crashes on import table with missed files
35. [#322](https://github.com/manticoresoftware/manticoresearch/issues/322) Crash on select using multiple indexes, group by and ranker = none
36. [c3f58490](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` doesn't higlight in string attributes
37. [#320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` fails to sort on string attribute
38. [4f1a1f25](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Error in case of missing data dir
39. [04f4ddd4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* are not supported in RT mode
40. [1c0616a2](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Bad JSON objects in strings: 1. `CALL PQ` returns "Bad JSON objects in strings: 1" when the json is greater than some value.
41. [32f943d6](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT-mode inconsistency. In some cases I can't drop the index since it's unknown and can't create it since the directory is not empty.
42. [#319](https://github.com/manticoresoftware/manticoresearch/issues/319) Crash on select
43. [22a28dd7](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M returned warning on 2M field
44. [#342](https://github.com/manticoresoftware/manticoresearch/issues/342) Query conditions execution bug
45. [dd8dcab2](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Simple 2 terms search finds a document containing only one term
46. [90919e62](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) It was impossible in PQ to match a json with capital letters in keys
47. [56da086a](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Indexer crashes on csv+docstore
48. [#363](https://github.com/manticoresoftware/manticoresearch/issues/363) using `[null]` in json attr in centos 7 causes corrupted inserted data
49. Major [#345](https://github.com/manticoresoftware/manticoresearch/issues/345) Records not being inserted, count() is random, "replace into" returns OK
50. max_query_time slows down SELECTs too much
51. [#352](https://github.com/manticoresoftware/manticoresearch/issues/352) Master-agent communication fails on Mac OS
52. [#328](https://github.com/manticoresoftware/manticoresearch/issues/328) Error when connecting to Manticore with Connector.Net/Mysql 8.0.19
53. [daa760d2](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Fixed escaping of \0 and optimized performance
54. [9bc5c01a](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Fixed count distinct vs json
55. [4f89a965](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Fixed drop table at other node failed
56. [952af5a5](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Fix crashes on tightly running call pq


## Version 3.4.2, 10 April 2020
### Critical bugfixes
* [2ffe2d26](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) fix RT index from old version fails to index data

## Version 3.4.0, 26 March 2020
### Major changes
* server works in 2 modes: rt-mode and plain-mode
   *   rt-mode requires data_dir and no index definition in config
   *   in plain-mode indexes are defined in config; no data_dir allowed
* replication available only in rt-mode

### Minor changes
* charset_table defaults to non_cjk alias
* in rt-mode full-text fields are indexed and stored by default
* full-text fields in rt-mode renamed from 'field' to 'text'
* ALTER RTINDEX is renamed to ALTER TABLE
* TRUNCATE RTINDEX is renamed to TRUNCATE TABLE

### Features
* stored-only fields
* SHOW CREATE TABLE, IMPORT TABLE

### Improvements
* much faster lockless PQ
* /sql can execute any type of SQL statement in mode=raw
* alias mysql for mysql41 protocol
* default state.sql in data_dir

### Bugfixes
* [a5333644](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) fix crash on wrong field syntax in highlight()
* [7fbb9f2e](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) fix crash of server on replicate RT index with docstore
* [24a04687](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) fix crash on highlight to index with infix or prefix option and to index wo stored fields enabled
* [3465c1ce](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) fix false error about empty docstore and dock-id lookup for empty index
* [a707722e](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) fix #314 SQL insert command with trailing semicolon
* [95628c9b](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) removed warning on query word(s) mismatch
* [b8601b41](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) fix queries in snippets segmented via ICU
* [5275516c](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) fix find/add race condition in docstore block cache
* [f06ef97a](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) fix mem leak in docstore
* [a7258ba8](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) fix #316 LAST_INSERT_ID returns empty on INSERT
* [1ebd5bf8](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) fix #317 json/update HTTP endpoint to support array for MVA and object for JSON attribute
* [e426950a](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) fix rash of indexer dumping rt without explicit id

## Version 3.3.0, 4 February 2020
### Features
* Parallel Real-Time index searching
* EXPLAIN QUERY command
* configuration file without index definitions (alpha version)
* CREATE/DROP TABLE commands (alpha version)
* indexer --print-rt - can read from a source and print INSERTs for a Real-Time index

### Improvements
* Updated to Snowball 2.0 stemmers
* LIKE filter for SHOW INDEX STATUS
* improved memory usage for high max_matches
* SHOW INDEX STATUS adds ram_chunks_count for RT indexes
* lockless PQ
* changed LimitNOFILE to 65536


### Bugfixes
* [9c33aab8](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) added check of index schema for duplicate attributes #293
* [a0085f94](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) fix crash in hitless terms
* [68953740](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) fix loose docstore after ATTACH
* [d6f696ed](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) fix docstore issue in distributed setup
* [bce2b7ec](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) replace FixedHash with OpenHash in sorter
* [e0baf739](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) fix attributes with duplicated names at index definition
* [ca81114b](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) fix html_strip in HIGHLIGHT()
* [493a5e91](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) fix passage macro in HIGHLIGHT()
* [a82d41c7](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) fix double buffer issues when RT index creates small or large disk chunk
* [a404c85d](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) fix event deletion for kqueue
* [8bea0f6f](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) fix save of disk chunk for large value of rt_mem_limit of RT index
* [8707f039](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) fix float overflow on indexing
* [a56434ce](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) fix insert document with negative ID into RT index fails with error now
* [bbebfd75](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) fix crash of server on ranker fieldmask
* [3809cc1b](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) fix crash on using query cache
* [dc2a585b](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) fix crash on using RT index RAM segments with parallel inserts

## Version 3.2.2, 19 December 2019
### Features
* Autoincrement ID for RT indexes
* Highlight support for docstore via new HIGHLIGHT() function, available also in HTTP API
* SNIPPET() can use special function QUERY() which returns current MATCH query
* new field_separator option for highlighting functions.

### Improvements and changes
* lazy fetch of stored fields for remote nodes (can significantly increase performance)
* strings and expressions don't break anymore multi-query and FACET optimizations
* RHEL/CentOS 8 build now uses mysql libclient from mariadb-connector-c-devel
* ICU data file is now shipped with the packages, icu_data_dir removed
* systemd service files include 'Restart=on-failure' policy
* indextool can now check real-time indexes online
* default conf is now /etc/manticoresearch/manticore.conf
* service on RHEL/CentOS renamed to 'manticore' from 'searchd'
* removed query_mode and exact_phrase snippet's options

### Bugfixes
* [6ae474c7](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) fix crash on SELECT query over HTTP interface
* [59577513](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) fix RT index saves disk chunks but does not mark some documents deleted
* [e861f0fc](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) fix crash on search of multi index or multi queries with dist_threads
* [440991fc](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) fix crash on infix generation for long terms with wide utf8 codepoints
* [5fd599b4](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) fix race at adding socket to IOCP
* [cf10d7d3](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) fix issue of bool queries vs json select list
* [996de77f](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) fix indextool check to report wrong skiplist offset, check of doc2row lookup
* [6e3fc9e8](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) fix indexer produces bad index with negative skiplist offset on large data
* [faed3220](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) fix JSON converts only numeric to string and JSON string to numeric conversion at expressions
* [53319720](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) fix indextool exit with error code in case multiple commands set at command line
* [795520ac](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) fix #275 binlog invalid state on error no space left on disk
* [2284da5e](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) fix #279 crash on IN filter to JSON attribute
* [ce2e4b47](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) fix #281 wrong pipe closing call
* [535589ba](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) fix server hung at CALL PQ with recursive JSON attribute encoded as string
* [a5fc8a36](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) fix advancing beyond the end of the doclist in multiand node
* [a3628617](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) fix retrieving of thread public info
* [f8d2d7bb](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) fix docstore cache locks

## Version 3.2.0, 17 October 2019
### Features
* Document storage
* new directives stored_fields, docstore_cache_size, docstore_block_size, docstore_compression, docstore_compression_level

### Improvements and changes
* improved SSL support
* non_cjk built-in charset updated
* disabled UPDATE/DELETE statements logging a SELECT in query log
* RHEL/CentOS 8 packages

### Bugfixes
* [301a806b1](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) fix crash on replace document in disk chunk of RT index
* [46c1cad8f](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) fix \#269 LIMIT N OFFSET M
* [92a46edaa](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) fix DELETE statements with id explicitly set or id list provided to skip search
* [8ca78c138](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) fix wrong index after event removed at netloop at windowspoll poller
* [603631e2b](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) fix float roundup at JSON via HTTP
* [62f64cb9e](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) fix remote snippets to check empty path first; fixing windows tests
* [aba274c2c](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) fix reload of config to work on windows same way as on linux
* [6b8c4242e](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) fix \#194 PQ to work with morphology and stemmers
* [174d31290](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) fix RT retired segments management

## Version 3.1.2, 22 August 2019
### Features and Improvements
* Experimental SSL support for HTTP API
* field filter for CALL KEYWORDS
* max_matches for /json/search
* automatic sizing of default Galera gcache.size
* improved FreeBSD support

### Bugfixes
* [0a1a2c81](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) fixed replication of RT index into node where same RT index exists and has different path
* [4adc0752](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) fix flush rescheduling for indexes without activity
* [d6c00a6f](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) improve rescheduling of flushing RT/PQ indexes
* [d0a7c959](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) fix \#250 index_field_lengths index option for TSV and CSV piped sources
* [1266d548](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) fix indextool wrong report for block index check on empty index
* [553ca73c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) fix empty select list at Manticore SQL query log
* [56c85844](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) fix indexer -h/--help response

## Version 3.1.0, 16 July 2019
### Features and Improvements
* replication for RealTime indexes
* ICU tokenizer for chinese
* new morphology option icu_chinese
* new directive icu_data_dir
* multiple statements transactions for replication
* LAST_INSERT_ID() and @session.last_insert_id
* LIKE 'pattern' for SHOW VARIABLES
* Multiple documents INSERT for percolate indexes
* Added time parsers for config
* internal task manager
* mlock for doc and hit lists components
* jail snippets path

### Removals
* RLP library support dropped in favor of ICU; all rlp\* directives removed
* updating document ID with UPDATE is disabled

### Bugfixes
* [f0472223](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) fix defects in concat and group_concat
* [b08147ee](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) fix query uid at percolate index to be BIGINT attribute type
* [4cd85afa](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) do not crash if failed to prealloc a new disk chunk
* [1a551227](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) add missing timestamp data type to ALTER
* [f3a8e096](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) fix crash of wrong mmap read
* [44757711](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) fix hash of clusters lock in replication
* [ff476df9](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) fix leak of providers in replication
* [58dcbb77](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) fix \#246 undefined sigmask in indexer
* [3dd8278e](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) fix race in netloop reporting
* [a02aae05](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) zero gap for HA strategies rebalancer

## Version 3.0.2, 31 May 2019
### Improvements
* added mmap readers for docs and hit lists
* /sql HTTP endpoint response is now the same as /json/search response
* new directives access_plain_attrs, access_blob_attrs, access_doclists, access_hitlists
* new directive server_id for replication setups

### Removals
* removed HTTP /search endpoint

### Deprecations
* ondisk_attrs, ondisk_attrs_default, mlock (replaced by
    [access]()\* directives)

### Bugfixes
* [849c16e1](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) allow attribute names starting with numbers in select list
* [48e6c302](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) fixed MVAs in UDFs, fixed MVA aliasing
* [055586a9](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) fixed \#187 crash when using query with SENTENCE
* [93bf52f2](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) fixed \#143 support () around MATCH()
* [599ee79c](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) fixed save of cluster state on ALTER cluster statement
* [230c321e](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) fixed crash of server on ALTER index with blob attributes
* [5802b85a](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) fixed \#196 filtering by id
* [25d2dabd](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) discard searching on template indexes
* [2a30d5b4](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) fixed id column to have regular bigint type at SQL reply


## Version 3.0.0, 6 May 2019
### Features and improvements
* New index storage. Non-scalar attributes are not limited anymore to 4GB size per index
* attr_update_reserve directive
* String,JSON and MVAs can be updated using UPDATE
* killlists are applied at index load time
* killlist_target directive
* multi AND searches speedup
* better average performance and RAM usage
* convert tool for upgrading indexes made with 2.x
* CONCAT() function
* JOIN CLUSTER cluster AT 'nodeaddress:port'
* ALTER CLUSTER posts UPDATE nodes
* node_address directive
* list of nodes printed in SHOW STATUS

### Behaviour changes
* in case of indexes with killists, server doesn't rotate indexes in order defined in conf, but follows the chain of killlist targets
* order of indexes in a search no longer defines the order in which killlists are applied
* Document IDs are now signed big integers

### Removed directives
* docinfo (always extern now), inplace_docinfo_gap, mva_updates_pool

## Version 2.8.2 GA, 2 April 2019
### Features and improvements
* Galera replication for percolate indexes
* OPTION morphology

### Compiling notes
Cmake minimum version is now 3.13. Compiling requires boost and libssl
development libraries.

### Bugfixes
* [6967fedb](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) fixed crash on many stars at select list for query into many distributed indexes
* [36df1a40](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) fixed [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) large packet via Manticore SQL interface
* [57932aec](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) fixed [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) crash of server on RT optimize with MVA updated
* [edb24b87](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) fixed server crash on binlog removed due to RT index remove after config reload on SIGHUP
* [bd3e66e0](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) fixed mysql handshake auth plugin payloads
* [6a217f6e](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) fixed [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) phrase_boundary settings at RT index
* [3562f652](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) fixed [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) deadlock at ATTACH index to itself
* [250b3f0e](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) fixed binlog saves empty meta after server crash
* [4aa6c69a](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) fixed crash of server due to string at sorter from RT index with disk chunks

## Version 2.8.1 GA, 6 March 2019
### Features and improvements
* SUBSTRING_INDEX()
* SENTENCE and PARAGRAPH support for percolate queries
* systemd generator for Debian/Ubuntu; also added LimitCORE to allow core dumping

### Bugfixes
* [84fe7405](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) fixed crash of server on match mode all and empty full text query
* [daa88b57](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) fixed crash on deleting of static string
* [22078537](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) fixed exit code when indextool failed with FATAL
* [0721696d](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) fixed [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) no matches for prefixes due to wrong exact form check
* [8af81011](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) fixed [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) reload of config settings for RT indexes
* [e2d59277](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) fixed crash of server on access of large JSON string
* [75cd1342](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) fixed PQ field at JSON document altered by index stripper causes wrong match from sibling field
* [e2f77543](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) fixed crash of server at parse JSON on RHEL7 builds
* [3a25a580](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) fixed crash of json unescaping when slash is on the edge
* [be9f4978](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) fixed option 'skip_empty' to skip empty docs and not warn they're not valid json
* [266e0e7b](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) fixed [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) output 8 digits on floats when 6 is not enough to be precise
* [3f6d2389](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) fixed empty jsonobj creation
* [f3c7848a](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) fixed [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) empty mva outputs NULL instead of an empty string
* [0afa2ed0](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) fixed fail to build without pthread_getname_np
* [9405fccd](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) fixed crash on server shutdown with thread_pool workers

## Version 2.8.0 GA, 28 January 2019
### Improvements
* Distributed indexes for percolate indexes
* CALL PQ new options and changes:
    *   skip_bad_json
    *   mode (sparsed/sharded)
    *   json documents can be passed as a json array
    *   shift
    *   Column names 'UID', 'Documents', 'Query', 'Tags', 'Filters' were renamed to 'id', 'documents', 'query', 'tags', 'filters'
* DESCRIBE pq TABLE
* SELECT FROM pq WHERE UID is not possible any more, use 'id' instead
* SELECT over pq indexes is on par with regular indexes (e.g. you can filter rules via REGEX())
* ANY/ALL can be used on PQ tags
* expressions have auto-conversion for JSON fields, not requiring explicit casting
* built-in 'non_cjk' charset_table and 'cjk' ngram_chars
* built-in stopwords collections for 50 languages
* multiple files in a stopwords declaration can also be separated by comma
* CALL PQ can accept JSON array of documents

### Bugfixes
* [a4e19af](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) fixed csjon-related leak
* [28d8627](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) fixed crash because of missed value in json
* [bf4e9ea](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) fixed save of empty meta for RT index
* [33b4573](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) fixed lost form flag (exact) for sequence of lemmatizer
* [6b95d48](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) fixed string attrs > 4M use saturate instead of overflow
* [621418b](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) fixed crash of server on SIGHUP with disabled index
* [3f7e35d](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) fixed server crash on simultaneous API session status commands
* [cd9e4f1](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) fixed crash of server at delete query to RT index with field filters
* [9376470](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) fixed crash of server at CALL PQ to distributed index with empty document
* [8868b20](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) fixed cut Manticore SQL error message larger 512 chars
* [de9deda](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) fixed crash on save percolate index without binlog
* [2b219e1](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) fixed http interface is not working in OSX
* [e92c602](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) fixed indextool false error message on check of MVA
* [238bdea](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) fixed write lock at FLUSH RTINDEX to not write lock whole index during save and on regular flush from rt_flush_period
* [c26a236](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) fixed ALTER percolate index stuck waiting search load
* [9ee5703](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) fixed max_children to use default amount of thread_pool workers for value of 0
* [5138fc0](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) fixed error on indexing of data into index with index_token_filter plugin along with stopwords and stopword_step=0
* [2add3d3](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) fixed crash with absent lemmatizer_base when still using aot lemmatizers in index definitions

## Version 2.7.5 GA, 4 December 2018
### Improvements
* REGEX function
* limit/offset for json API search
* profiler points for qcache

### Bugfixes
* [eb3c768](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) fixed crash of server on FACET with multiple attribute wide types
* [d915cf6](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) fixed implicit group by at main select list of FACET query
* [5c25dc2](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) fixed crash on query with GROUP N BY
* [85d30a2](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) fixed deadlock on handling crash at memory operations
* [85166b5](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) fixed indextool memory consumption during check
* [58fb031](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) fixed gmock include not needed anymore as upstream resolve itself

## Version 2.7.4 GA, 1 November 2018
### Improvements
* SHOW THREADS in case of remote distributed indexes prints the original query instead of API call
* SHOW THREADS new option `format=sphinxql` prints all queries in SQL format
* SHOW PROFILE prints additional `clone_attrs` stage

### Bugfixes
* [4f15571](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) fixed failed to build with libc without malloc_stats, malloc_trim
* [f974f20](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) fixed special symbols inside words for CALL KEYWORDS result set
* [0920832](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) fixed broken CALL KEYWORDS to distributed index via API or to remote agent
* [fd686bf](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) fixed distributed index agent_query_timeout propagate to agents as max_query_time
* [4ffa623](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) fixed total documents counter at disk chunk got affected by OPTIMIZE command and breaks weight calculation
* [dcaf4e0](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) fixed multiple tail hits at RT index from blended
* [eee3817](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) fixed deadlock at rotation

## Version 2.7.3 GA, 26 September 2018
### Improvements
* sort_mode option for CALL KEYWORDS
* DEBUG on VIP connection can perform 'crash <password>' for intentional SIGEGV action on server
* DEBUG can perform 'malloc_stats' for dumping malloc stats in searchd.log 'malloc_trim' to perform a malloc_trim()
* improved backtrace is gdb is present on the system

### Bugfixes
* [0f3cc33](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) fixed crash or hfailure of rename on Windows
* [1455ba2](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) fixed crashes of server on 32-bit systems
* [ad3710d](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) fixed crash or hung of server on empty SNIPPET expression
* [b36d792](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) fixed broken non progressive optimize and fixed progressive optimize to not create kill-list for oldest disk chunk
* [34b0324](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) fixed queue_max_length bad reply for SQL and API at thread pool worker mode
* [ae4b320](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) fixed crash on adding full-scan query to PQ index with regexp or rlp options set
* [f80f8d5](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) fixed crash when call one PQ after another
* [9742f5f](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) refactor AcquireAccum
* [39e5bc3](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) fixed leak of memory after call pq
* [21bcc6d](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) cosmetic refactor (c++11 style c-trs, defaults, nullptrs)
* [2d69039](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) fixed memory leak on trying to insert duplicate into PQ index
* [5ed92c4](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) fixed crash on JSON field IN with large values
* [4a5262e](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) fixed crash of server on CALL KEYWORDS statement to RT index with expansion limit set
* [552646b](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) fixed invalid filter at PQ matches query;
* [204f521](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) introduce small obj allocator for ptr attrs
* [25453e5](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) refactor ISphFieldFilter to refcounted flavour
* [1366ee0](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) fixed ub/sigsegv when using strtod on non-terminated strings
* [94bc6fc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) fixed memory leak in json resultset processing
* [e78e9c9](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) fixed read over the end of mem block applying attribute add
* [fad572f](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) fixed refactor CSphDict for refcount flavour
* [fd841a4](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) fixed leak of AOT internal type outside
* [5ee7f20](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) fixed memory leak tokenizer management
* [116c5f1](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) fixed memory leak in grouper
* [56fdbc9](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) special free/copy for dynamic ptrs in matches (memory leak grouper)
* [b1fc161](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) fixed memory leak of dynamic strings for RT
* [517b9e8](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) refactor grouper
* [b1fc161](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) minor refactor (c++11 c-trs, some reformats)
* [7034e07](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) refactor ISphMatchComparator to refcounted flavour
* [b1fc161](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) privatize cloner
* [efbc051](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) simplify native little-endian for MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [6da0df4](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) add valgrind support to to ubertests
* [1d17669](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) fixed crash because race of 'success' flag on connection
* [5a09c32](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) switch epoll to edge-triggered flavour
* [5d52868](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) fixed IN statement in expression with formatting like at filter
* [bd8b3c9](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) fixed crash at RT index on commit of document with large docid
* [ce656b8](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) fixed argless options in indextool
* [08c9507](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) fixed memory leak of expanded keyword
* [30c75a2](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) fixed memory leak of json grouper
* [6023f26](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) fixed leak of global user vars
* [7c138f1](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) fixed leakage of dynamic strings on early rejected matches
* [9154b18](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) fixed leakage on length(<expression>)
* [43fca3a](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) fixed memory leak because strdup() in parser
* [71ff777](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) fixed refactor expression parser to accurate follow refcounts

## Version 2.7.2 GA, 27 August 2018
### Improvements
* compatibility with MySQL 8 clients
* [TRUNCATE](Emptying_an_index.md) WITH RECONFIGURE
* retired memory counter on SHOW STATUS for RT indexes
* global cache of multi agents
* improved IOCP on Windows
* VIP connections for HTTP protocol
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) command which can run various subcommands
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 hash of password needed to invoke `shutdown` using DEBUG command
* new stats to SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* --verbose option of indexer now accept \[debugvv\] for printing debug messages

### Bugfixes
* [390082](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) removed wlock at optimize
* [4c3376](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) fixed wlock at reload index settings
* [b5ea8d](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) fixed memory leak on query with JSON filter
* [930e83](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) fixed empty documents at PQ result set
* [53deec](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) fixed confusion of tasks due to removed one
* [cad9b9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) fixed wrong remote host counting
* [90008c](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) fixed memory leak of parsed agent descriptors
* [978d83](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) fixed leak in search
* [019394](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) cosmetic changes on explicit/inline c-trs, override/final usage
* [943e29](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) fixed leak of json in local/remote schema
* [02dbdd](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) fixed leak of json sorting col expr in local/remote schema
* [c74d0b](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) fixed leak of const alias
* [6e5b57](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) fixed leak of preread thread
* [39c740](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) fixed stuck on exit because of stucked wait in netloop
* [adaf97](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) fixed stuck of 'ping' behaviour on change HA agent to usual host
* [32c40e](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) separate gc for dashboard storage
* [511a3c](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) fixed ref-counted ptr fix
* [32c40e](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) fixed indextool crash on unexistent index
* [156edc](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) fixed output name of exceeding attr/field in xmlpipe indexing
* [cdac6d](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) fixed default indexer's value if no indexer section in config
* [e61ec0](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) fixed wrong embedded stopwords in disk chunk by RT index after server restart
* [5fba49](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) fixed skip phantom (already closed, but not finally deleted from the poller) connections
* [f22ae3](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) fixed blended (orphaned) network tasks
* [46890e](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) fixed crash on read action after write
* [03f9df](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) fixed searchd crashes when running tests on windows
* [e9255e](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) fixed handle EINPROGRESS code on usual connect()
* [248b72](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) fixed connection timeouts when working with TFO

## Version 2.7.1 GA, 4 July 2018
### Improvements
* improved wildcards performance on matching multiple documents at PQ
* support for fullscan queries at PQ
* support for MVA attributes at PQ
* regexp and RLP support for percolate indexes

### Bugfixes
* [688562](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) fixed loose of query string
* [0f1770](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) fixed empty info at SHOW THREADS statement
* [53faa3](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) fixed crash on matching with NOTNEAR operator
* [26029a](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) fixed error message on bad filter to PQ delete


## Version 2.7.0 GA, 11 June 2018
### Improvements
* reduced number of syscalls to avoid Meltdown and Spectre patches impact
* internal rewrite of local index management
* remote snippets refactor
* full configuration reload
* all node connections are now independent
* proto improvements
* Windows communication switched from wsapoll to IO completion ports
* TFO can be used for communication between master and nodes
* [SHOW STATUS](Profiling_and_monitoring/Node_status.md#SHOW-STATUS) now outputs to server version and mysql_version_string
* added `docs_id` option for documents called in CALL PQ.
* percolate queries filter can now contain expressions
* distributed indexes can work with FEDERATED
* dummy SHOW NAMES COLLATE and `SET wait_timeout` (for better ProxySQL compatibility)

### Bugfixes
* [5bcff0](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) fixed added not equal to tags of PQ
* [9ebc58](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) fixed added document id field to JSON document CALL PQ statement
* [8ae0e5](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) fixed flush statement handlers to PQ index
* [c24b15](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) fixed PQ filtering on JSON and string attributes
* [1b8bdd](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) fixed parsing of empty JSON string
* [1ad8a0](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) fixed crash at multi-query with OR filters
* [69b898](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) fixed indextool to use config common section (lemmatizer_base option) for commands (dumpheader)
* [6dbeaf](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) fixed empty string at result set and filter
* [39c4eb](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) fixed negative document id values
* [266b70](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) fixed word clip length for very long words indexed
* [47823b](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) fixed matching multiple documents of wildcard queries at PQ


## Version 2.6.4 GA, 3 May 2018
### Features and improvements
* MySQL FEDERATED engine [support](Extensions/FEDERATED.md)
* MySQL packets return now SERVER_STATUS_AUTOCOMMIT flag, adds compatibility with ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - enable TCP Fast Open connections for all listeners
* indexer --dumpheader can dump also RT header from .meta file
* cmake build script for Ubuntu Bionic

### Bugfixes
* [355b116](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) fixed invalid query cache entries for RT index;
* [546e229](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) fixed index settings got lost next after seamless rotation
* [0c45098](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) fixed fixed infix vs prefix length set; added warning on unsupportedinfix length
* [80542fa](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) fixed RT indexes auto-flush order
* [705d8c5](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) fixed result set schema issues for index with multiple attributes and queries to multiple indexes
* [b0ba932](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) fixed some hits got lost at batch insert with document duplicates
* [4510fa4](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) fixed optimize failed to merge disk chunks of RT index with large documents count

## Version 2.6.3 GA, 28 March 2018
### Improvements
* jemalloc at compilation. If jemalloc is present on system, it can be enabled with cmake flag `-DUSE_JEMALLOC=1`

### Bugfixes
* [85a6d7e](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) fixed log expand_keywords option into Manticore SQL query log
* [caaa384](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) fixed HTTP interface to correctly process query with large size
* [e386d84](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) fixed crash of server on DELETE to RT index with index_field_lengths enable
* [cd538f3](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) fixed cpustats searchd cli option to work with unsupported systems
* [8740fd6](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) fixed utf8 substring matching with min lengths defined


## Version 2.6.2 GA, 23 February 2018
### Improvements
* improved [Percolate Queries](Searching/Percolate_query.md) performance in case of using NOT operator and for batched documents.
* [percolate_query_call](Searching/Percolate_query.md) can use multiple threads depending on [dist_threads](Server_settings/Searchd.md#threads)
* new full-text matching operator NOTNEAR/N
* LIMIT for SELECT on percolate indexes
* [expand_keywords](Searching/Options.md#expand_keywords) can accept 'start','exact' (where 'star,exact' has same effect as '1')
* ranged-main-query for  [joined fields](Adding_data_from_external_storages/Fetching_from_databases/Indexing_fetched_data.md#sql_joined_field) which uses the ranged query defined by sql_query_range

### Bugfixes
* [72dcf66](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) fixed crash on searching ram segments; deadlock on save disk chunk with double buffer; deadlock on save disk chunk during optimize
* [3613714](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) fixed indexer crash on xml embedded schema with empty attribute name
* [48d7e80](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) fixed erroneous unlinking of not-owned pid-file
* [a5563a4](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) fixed orphaned fifos sometimes left in temp folder
* [2376e8f](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) fixed empty FACET result set with wrong NULL row
* [4842b67](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) fixed broken index lock when running server as windows service
* [be35fee](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) fixed wrong iconv libs on mac os
* [83744a9](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) fixed wrong count(\*)


## Version 2.6.1 GA, 26 January 2018
### Improvements
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) in case of agents with mirrors gives the value of retries per mirror instead of per agent, the total retries per agent being agent_retry_count\*mirrors.
* [agent_retry_count](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#agent_retry_count) can now be specified per index, overriding global value. An alias [mirror_retry_count](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#mirror_retry_count) is added.
* a retry_count can be specified in agent definition and the value represents retries per agent
* Percolate Queries are now in HTTP JSON API at [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query).
* Added -h and -v options (help and version) to executables
* [morphology_skip_fields](Creating_an_index/NLP_and_tokenization/Morphology.md#morphology_skip_fields) support for Real-Time indexes

### Bugfixes
* [a40b079](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) fixed ranged-main-query to correctly work with sql_range_step when used at MVA field
* [f2f5375](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) fixed issue with blackhole system loop hung and blackhole agents seems disconnected
* [84e1f54](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) fixed query id to be consistent, fixed duplicated id for stored queries
* [1948423](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) fixed server crash on shutdown from various states
* [9a706b](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [3495fd7](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) timeouts on long queries
* [3359bcd8](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) refactored master-agent network polling on kqueue-based systems (Mac OS X, BSD).


## Version 2.6.0, 29 December 2017
### Features and improvements
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON queries can now do equality on attributes, MVA and JSON attributes can be used in inserts and updates, updates and deletes via JSON API can be performed on distributed indexes
* [Percolate Queries](Searching/Percolate_query.md)
* Removed support for 32-bit docids from the code. Also removed all the code that converts/loads legacy indexes with 32-bit docids.
* [Morphology only for certain fields](https://github.com/manticoresoftware/manticore/issues/7) . A new index directive morphology_skip_fields allows defining a list of fields for which morphology does not apply.
* [expand_keywords can now be a query runtime directive set using the OPTION statement](https://github.com/manticoresoftware/manticore/issues/8)

### Bugfixes
* [0cfae4c](https://github.com/manticoresoftware/manticore/commit/0cfae4c) fixed crash on debug build of server (and m.b. UB on release) when built with rlp
* [324291e](https://github.com/manticoresoftware/manticore/commit/324291e) fixed RT index optimize with progressive option enabled that merges kill-lists with wrong order
* [ac0efee](https://github.com/manticoresoftware/manticore/commit/ac0efee)  minor crash on mac
* lots of minor fixes after thorough static code analysis
* other minor bugfixes

### Upgrade
In this release we've changed internal protocol used by masters and agents to speak with each other. In case you run Manticoresearch in a distributed environment with multiple instances make sure your first upgrade agents, then the masters.

## Version 2.5.1, 23 November 2017
### Features and improvements
* JSON queries on [HTTP API protocol](Connecting_to_the_server/HTTP.md). Supported search, insert, update, delete, replace operations. Data manipulation commands can be also bulked, also there are some limitations currently as MVA and JSON attributes can't be used for inserts, replaces or updates.
* [RELOAD INDEXES](Adding_data_from_external_storages/Rotating_an_index.md#RELOAD-INDEXES) command
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) command
* [SHOW THREADS](Profiling_and_monitoring/SHOW_THREADS.md) can show progress of optimize, rotation or flushes.
* GROUP N BY work correctly with MVA attributes
* blackhole agents are run on separate thread to not affect master query anymore
* implemented reference count on indexes, to avoid stalls caused by rotations and high load
* SHA1 hashing implemented, not exposed yet externally
* fixes for compiling on FreeBSD, macOS and Alpine

### Bugfixes
* [989752b](https://github.com/manticoresoftware/manticore/commit/989752b) filter regression with block index
* [b1c3864](https://github.com/manticoresoftware/manticore/commit/b1c3864) rename PAGE_SIZE -> ARENA_PAGE_SIZE for compatibility with musl
* [f2133cc](https://github.com/manticoresoftware/manticore/commit/f2133cc) disable googletests for cmake < 3.1.0
* [f30ec53](https://github.com/manticoresoftware/manticore/commit/0839de7) failed to bind socket on server restart
* [0807240](https://github.com/manticoresoftware/manticore/commit/0807240) fixed crash of server on shutdown
* [3e3acc3](https://github.com/manticoresoftware/manticore/commit/3e3acc3) fixed show threads for system blackhole thread
* [262c3fe](https://github.com/manticoresoftware/manticore/commit/262c3fe) Refactored config check of iconv, fixes building on FreeBSD and Darwin

## Version 2.4.1 GA, 16 October 2017
### Features and improvements
* OR operator in WHERE clause between attribute filters
* Maintenance mode ( SET MAINTENANCE=1)
* [CALL KEYWORDS](Searching/Autocomplete.md#CALL-KEYWORDS) available on distributed indexes
* [Grouping in UTC](Server_settings/Searchd.md#grouping_in_utc)
* [query_log_mode](Server_settings/Searchd.md#query_log_mode) for custom log files permissions
* Field weights can be zero or negative
* [max_query_time](Searching/Options.md#max_query_time) can now affect full-scans
* added [net_wait_tm](Server_settings/Searchd.md#net_wait_tm), [net_throttle_accept](Server_settings/Searchd.md#net_throttle_accept) and [net_throttle_action](Server_settings/Searchd.md#net_throttle_action) for network thread fine tuning (in case of workers=thread_pool)
* COUNT DISTINCT works with facet searches
* IN can be used with JSON float arrays
* multi-query optimization is not broken anymore by integer/float expressions
* [SHOW META](Profiling_and_monitoring/SHOW_META.md) shows a `multiplier` row when multi-query optimization is used

### Compiling
Manticore Search is built using cmake and the minimum gcc version required for compiling is 4.7.2.

### Folders and service
* Manticore Search runs under `manticore` user.
* Default data folder is now `/var/lib/manticore/`.
* Default log folder is now `/var/log/manticore/`.
* Default pid folder is now `/var/run/manticore/`.

### Bugfixes
* [a58c619](https://github.com/manticoresoftware/manticore/commit/a58c619) fixed SHOW COLLATION statement that breaks java connector
* [631cf4e](https://github.com/manticoresoftware/manticore/commit/631cf4e) fixed crashes on processing distributed indexes; added locks to distributed index hash; removed move and copy operators from agent
* [942bec0](https://github.com/manticoresoftware/manticore/commit/942bec0) fixed crashes on processing distributed indexes due to parallel reconnects
* [e5c1ed2](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) fixed crash at crash handler on store query to server log
* [4a4bda5](https://github.com/manticoresoftware/manticore/commit/4a4bda5) fixed a crash with pooled attributes in multiqueries
* [3873bfb](https://github.com/manticoresoftware/manticore/commit/3873bfb) fixed reduced core size by prevent index pages got included into core file
* [11e6254](https://github.com/manticoresoftware/manticore/commit/11e6254) fixed searchd crashes on startup when invalid agents are specified
* [4ca6350](https://github.com/manticoresoftware/manticore/commit/4ca6350) fixed indexer reports error in sql_query_killlist query
* [123a9f0](https://github.com/manticoresoftware/manticore/commit/123a9f0) fixed fold_lemmas=1 vs hit count
* [cb99164](https://github.com/manticoresoftware/manticore/commit/cb99164) fixed inconsistent behavior of html_strip
* [e406761](https://github.com/manticoresoftware/manticore/commit/e406761) fixed optimize rt index loose new settings; fixed optimize with sync option lock leaks;
* [86aeb82](https://github.com/manticoresoftware/manticore/commit/86aeb82) fixed processing erroneous multiqueries
* [2645230](https://github.com/manticoresoftware/manticore/commit/2645230) fixed result set depends on multi-query order
* [72395d9](https://github.com/manticoresoftware/manticore/commit/72395d9) fixed server crash on multi-query with bad query
* [f353326](https://github.com/manticoresoftware/manticore/commit/f353326) fixed shared to exclusive lock
* [3754785](https://github.com/manticoresoftware/manticore/commit/3754785) fixed server crash for query without indexes
* [29f360e](https://github.com/manticoresoftware/manticore/commit/29f360e) fixed dead lock of server

## Version 2.3.3, 06 July 2017
* Manticore branding
