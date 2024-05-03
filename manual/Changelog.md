# Changelog

# Version 6.3.0 (preparing for release)
While 6.3.0 is being prepared for release, use the dev version which includes all the below changes - https://mnt.cr/dev/nightly

Changelog note markdown
### Major changes
* [Issue #839](https://github.com/manticoresoftware/manticoresearch/issues/839) Implemented [float_vector](Creating_a_table/Data_types.md#Float-vector) data type; implemented [vector search](Searching/KNN.md#KNN-vector-search).
* [Issue #1673](https://github.com/manticoresoftware/manticoresearch/issues/1673) INNER/LEFT JOIN
* [Issue #1720](https://github.com/manticoresoftware/manticoresearch/pull/1720) Changed Manticore Search license from GPLv2-or-later to GPLv3-or-later.
* [Commit 7a55](https://github.com/manticoresoftware/manticoresearch/commit/7a5508424) Running Manticore in Windows now requires Docker to run Buddy
* [Issue #1541](https://github.com/manticoresoftware/manticoresearch/issues/1541) Added a [REGEX](Searching/Full_text_matching/Operators.md#REGEX-operator) full-text operator.
* [Issue #1569](https://github.com/manticoresoftware/manticoresearch/issues/1569) Binary log is now written with transaction granularity.
* [Commit 514d](https://github.com/manticoresoftware/manticoresearch/commit/514d35b497f4bdb20b6473ff963752a978a4bb8d) Revamp of time operations for better performance and new date/time functions:
  - [CURDATE()](Functions/Date_and_time_functions.md#CURDATE%28%29) - Returns current date in local timezone
  - [QUARTER()](Functions/Date_and_time_functions.md#QUARTER%28%29) - Returns the integer quarter of the year from a timestamp argument
  - [DAYNAME()](Functions/Date_and_time_functions.md#DAYNAME%28%29) - Returns the weekday name for a given timestamp argument
  - [MONTHNAME()](Functions/Date_and_time_functions.md#MONTHNAME%28%29) - Returns the name of the month for a given timestamp argument
  - [DAYOFWEEK()](Functions/Date_and_time_functions.md#DAYOFWEEK%28%29) - Returns the integer weekday index for a given timestamp argument
  - [DAYOFYEAR()](Functions/Date_and_time_functions.md#DAYOFYEAR%28%29) - Returns the integer day of the year for a given timestamp argument
  - [YEARWEEK()](Functions/Date_and_time_functions.md#YEARWEEK%28%29) - Returns the integer year and the day code of the first day of current week for a given timestamp argument
  - [DATEDIFF()](Functions/Date_and_time_functions.md#DATEDIFF%28%29) - Returns the number of days between two given timestamps
  - [DATE()](Functions/Date_and_time_functions.md#DATE%28%29) - Formats the date part from a timestamp argument
  - [TIME()](Functions/Date_and_time_functions.md#TIME%28%29) - Formats the time part from a timestamp argument
  - [timezone](Server_settings/Searchd.md#timezone) - Timezone used by date/time-related functions
* [Commit 30e7](https://github.com/manticoresoftware/manticoresearch/commit/30e789d9657f9b3093c280c6e5b0e46a021f6dc7) Added [range](Searching/Faceted_search.md#Facet-over-set-of-ranges), [histogram](Searching/Faceted_search.md#Facet-over-histogram-values), [date_range](Searching/Faceted_search.md#Facet-over-set-of-date-ranges), and [date_histogram](Searching/Faceted_search.md#Facet-over-histogram-date-values) aggregates to the HTTP interface and similar expressions into SQL.

### Minor changes
* [Issue #212](https://github.com/manticoresoftware/manticoresearch-buddy/issues/212) Added time to [SHOW QUERIES](Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES)
* [Issue #1285](https://github.com/manticoresoftware/manticoresearch/issues/1285) Support of Filebeat versions 8.10 - 8.11
* [Issue #2072](https://github.com/manticoresoftware/manticoresearch/issues/2072) Optimized the [table compacting algorithm](Securing_and_compacting_a_table/Compacting_a_table.md#Compacting-a-Table): Previously, both manual [OPTIMIZE](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) and automatic [auto_optimize](Server_settings/Searchd.md#auto_optimize) processes would first merge chunks to ensure the count did not exceed the limit, and then expunge deleted documents from all other chunks containing deleted documents. This approach was sometimes too resource-intensive and has been disabled. Now, chunk merging occurs solely according to the [progressive_merge](Server_settings/Common.md#progressive_merge) setting. However, chunks with a high number of deleted documents are more likely to be merged.
* [Issue #1417](https://github.com/manticoresoftware/manticoresearch/issues/1417) Partial replace via [REPLACE INTO ... SET](Data_creation_and_modification/Updating_documents/REPLACE.md#SQL-REPLACE)
* [Commit 7c16](https://github.com/manticoresoftware/manticoresearch/commit/7c167b934ce416d3dfbd658744a231e638b14d9a) Updated default merge buffer sizes: `.spa` (scalar attrs): 256KB -> 8MB; `.spb` (blob attrs): 256KB -> 8MB; `.spc` (columnar attrs): 1MB, no change; `.spds` (docstore): 256KB -> 8MB; `.spidx` (secondary indexes): 256KB buffer -> 128MB memory limit; `.spi` (dictionary): 256KB -> 16MB; `.spd` (doclists): 8MB, no change; `.spp` (hitlists): 8MB, no change; `.spe` (skiplists): 256KB -> 8MB
* [Issue #1859](https://github.com/manticoresoftware/manticoresearch/issues/1859) Added [composite aggregation](Searching/Grouping.md#GROUP-BY-multiple-fields-at-once) via JSON
* [Commit 216b](https://github.com/manticoresoftware/manticoresearch-buddy/commit/216b824) Disabled PCRE.JIT due to issues with some regex patterns and no significant time benefit
* [Commit 55cd](https://github.com/manticoresoftware/manticoresearch/commit/55cdce32) Added support for vanilla Galera v.3 (api v25) (`libgalera_smm.so` from MySQL 5.x).
* [Commit 86f9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/86f98ab) Changed metric suffix from `_rate` to `_rps`
* [Commit c0c1](https://github.com/manticoresoftware/manticoresearch-helm/commit/c0c1cff780031412ef57cefab987dfd16613368e) Improved docs about balancer HA support
* [Commit d1d2](https://github.com/manticoresoftware/manticoresearch/commit/d1d2ca18857c68106e89743756260e73285be739) Changed `index` to `table` in error messages; fixed bison parser error message fixup
* [Commit fd26](https://github.com/manticoresoftware/manticoresearch/commit/fd26671b) Support `manticore.tbl` as table name.
* [Issue #1105](https://github.com/manticoresoftware/manticoresearch/issues/1105) Support for running indexer via systemd ([docs](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Schedule-indexer-via-systemd))
* [Issue #1294](https://github.com/manticoresoftware/manticoresearch/issues/1294) Secondary indexes support in GEODIST()
* [Issue #1394](https://github.com/manticoresoftware/manticoresearch/issues/1394) Simplified [show threads](Node_info_and_management/SHOW_THREADS.md#SHOW-THREADS)
* [Issue #1424](https://github.com/manticoresoftware/manticoresearch/issues/1424) Added support for the default values (`agent_connect_timeout` and `agent_query_timeout`) for `create distributed table` statement.
* [Issue #1442](https://github.com/manticoresoftware/manticoresearch/issues/1442) Added [expansion_limit](Searching/Options.md#expansion_limit) search query option that overrides `searchd.expansion_limit`.
* [Issue #1448](https://github.com/manticoresoftware/manticoresearch/issues/1448) Implemented [ALTER TABLE](Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) for int->bigint conversion.
* [Issue #146](https://github.com/manticoresoftware/manticoresearch/issues/1456) Meta information in MySQL response.
* [Issue #1494](https://github.com/manticoresoftware/manticoresearch/issues/1494) [SHOW VERSION](Node_info_and_management/SHOW_VERSION.md#SHOW-VERSION)
* [Issue #1582](https://github.com/manticoresoftware/manticoresearch/issues/1582) Support of [deleting documents by id array](Data_creation_and_modification/Deleting_documents.md?client=JSON#Deleting-documents) via JSON
* [Issue #1634](https://github.com/manticoresoftware/manticoresearch/issues/1634) Added Buddy version into `SHOW STATUS`
* [Issue #1641](https://github.com/manticoresoftware/manticoresearch/issues/1641) Match requests optimisation in case of zero docs for a keyword
* [Issue #1712](https://github.com/manticoresoftware/manticoresearch/issues/1712) Added conversion to bool attribute from the string value of "true" and "false" on posting data.
* [Issue #1713](https://github.com/manticoresoftware/manticoresearch/issues/1713) Added [access_dict](Server_settings/Searchd.md#access_dict) table and searchd option.
* [Issue #1767](https://github.com/manticoresoftware/manticoresearch/issues/1767) Added new options: [expansion_merge_threshold_docs](Server_settings/Searchd.md#expansion_merge_threshold_docs) and [expansion_merge_threshold_hits](Server_settings/Searchd.md#expansion_merge_threshold_hits) to the searchd section of the config; made the threshold for merging tiny terms of the expanded terms configurable.
* [Issue #1768](https://github.com/manticoresoftware/manticoresearch/issues/1768) Added display of the last command time in `@@system.sessions`.
* [Issue #1806](https://github.com/manticoresoftware/manticoresearch/issues/1806) Upgraded Buddy Protocol to version 2
* [Issue #1825](https://github.com/manticoresoftware/manticoresearch/pull/1825) Added an Info section to SHOW BUDDY PLUGINS
* [Issue #1837](https://github.com/manticoresoftware/manticoresearch/issues/1837) Improved memory consumption in `CALL PQ` with large packets.
* [Issue #1853](https://github.com/manticoresoftware/manticoresearch/pull/1853) Switched compiler from Clang 15 to Clang 16.
* [Issue #1857](https://github.com/manticoresoftware/manticoresearch/issues/1857) Added string comparison
* [Issue #1915](https://github.com/manticoresoftware/manticoresearch/issues/1915) Added support for joined stored fields
* [Issue #1937](https://github.com/manticoresoftware/manticoresearch/issues/1937) Log client's host:port in query-log.
* [Issue #1981](https://github.com/manticoresoftware/manticoresearch/issues/1981) Fixed wrong error
* [Issue #1983](https://github.com/manticoresoftware/manticoresearch/issues/1983) Introduced support for verbosity levels for the [query plan via JSON](Node_info_and_management/Profiling/Query_plan.md?client=JSON#Query-plan).
* [Issue #2010](https://github.com/manticoresoftware/manticoresearch/issues/2010) Disabled logging of queries from Buddy unless `log_level=debug` is set
* [Issue #2035](https://github.com/manticoresoftware/manticoresearch/issues/2035) Linux Mint 21.3 suppport
* [Issue #2056](https://github.com/manticoresoftware/manticoresearch/issues/2056) Manticore couldn't be built with Mysql 8.3+
* [Issue #2112](https://github.com/manticoresoftware/manticoresearch/issues/2112) `DEBUG DEDUP` command for real-time table chunks that can experience duplicate entries after attaching a plain table containing duplicates
* [Issue #218](https://github.com/manticoresoftware/manticoresearch-buddy/issues/218) Handle `@timestamp` column as timestamp
* [Issue #252](https://github.com/manticoresoftware/manticoresearch-buddy/issues/250) Implemented logic to enable/disable buddy plugins
* [Issue #254](https://github.com/manticoresoftware/manticoresearch-buddy/issues/254) Updated composer to a fresher version where recent CVEs are fixed
* [Issue #340](https://github.com/manticoresoftware/manticoresearch/issues/340) Minor optimization in Manticore systemd unit related with `RuntimeDirectory`
* [Issue #51](https://github.com/manticoresoftware/executor/issues/51) Added rdkafka support and updated to PHP 8.3.3
* [Issue #527](https://github.com/manticoresoftware/manticoresearch/issues/527) Support of [attaching](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_a_plain_table_to_RT_table.md#Attaching-table---general-syntax) an RT table.

### Breaking changes and deprecations
* ⚠️[Issue #1436](https://github.com/manticoresoftware/manticoresearch/issues/1436) Improved the master-agent search protocol (version updated). If you are running Manticore Search in a distributed environment with multiple instances, make sure to first upgrade the agents, then the masters.
* ⚠️[Issue #1572](https://github.com/manticoresoftware/manticoresearch/issues/1572) Added replication of distributed tables and updated the replication protocol. If you are running a replication cluster, you need to:
  - First, cleanly stop all your nodes
  - Then, start the node that was stopped last with `--new-cluster`, using the tool `manticore_new_cluster` in Linux.
  - Read about [restarting a cluster](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) for more details.
* ⚠️[Issue #1763](https://github.com/manticoresoftware/manticoresearch/pull/1763) HTTP API endpoint aliases `/json/*` have been deprecated
* ⚠️[Issue #1982](https://github.com/manticoresoftware/manticoresearch/issues/1982) Changed [profile](Node_info_and_management/Profiling/Query_profile.md#Query-profile) to [plan](Node_info_and_management/Profiling/Query_plan.md#Query-plan) in JSON, added query profiling for JSON.
* ⚠️[Commit e235](https://github.com/manticoresoftware/manticoresearch-backup/commit/e23585a) manticore-backup doesn't backup `plugin_dir` anymore
* ⚠️[Issue #171](https://github.com/manticoresoftware/manticoresearch-buddy/issues/155) Migrated Buddy to Swoole to improve performance and stability. When switching to the new version, ensure all Manticore packages are updated.
* ⚠️[Issue #196](https://github.com/manticoresoftware/manticoresearch-buddy/pull/205) Merged all core plugins into Buddy and changed the core logic
* ⚠️[Issue #38](https://github.com/manticoresoftware/executor/issues/38) Added Swoole, disabled ZTS, and removed the parallel extension

### Replication-related changes
* [Commit 3376](https://github.com/manticoresoftware/manticoresearch/commit/3376351f73db3ff0cb6cb708a304a7bd83e69cfb) Fixed replication error on SST of large files.
* [Commit 6d36](https://github.com/manticoresoftware/manticoresearch/commit/6d36c68fb7ce3cb274b467aac435788905084952) Added a retry mechanism to replication commands; fixed replication join failure on a busy network with packet loss.
* [Commit 842e](https://github.com/manticoresoftware/manticoresearch/commit/842e27e71790833ab4125d49103d2933d5df60c3) Changed the FATAL message in replication to a WARNING message.
* [Commit 8c32](https://github.com/manticoresoftware/manticoresearch/commit/8c32bce7db39d8ed3b1a06aaa1d2c3ec372084f0) Fixed the calculation of the `gcache.page_size` for replication clusters without tables or with empty tables; also fixed saving and loading of the Galera options.
* [Commit a2af](https://github.com/manticoresoftware/manticoresearch/commit/a2af06ca36abcbf893e43c8803853130b397c54b) Added functionality to skip the update nodes replication command on the node that joins the cluster.
* [Commit c054](https://github.com/manticoresoftware/manticoresearch/commit/c0541e6892f5116b1f957b40060ba8ecd01a5afb) Fixed deadlock during replication on updating blob attributes versus replacing documents.
* [Commit e80d](https://github.com/manticoresoftware/manticoresearch/commit/e80d505b963999016033613c40c1cf3151d648e7) Added [replication_connect_timeout](Server_settings/Searchd.md#replication_connect_timeout), [replication_query_timeout](Server_settings/Searchd.md#replication_query_timeout), [replication_retry_delay](Server_settings/Searchd.md#replication_retry_delay), [replication_retry_count](Server_settings/Searchd.md#replication_retry_count) searchd config options to control network during replication similar to `searchd.agent_*` but with different defaults.
* [Issue #1356](https://github.com/manticoresoftware/manticoresearch/issues/1356) Fixed replication nodes retry after some nodes are missed and name resolution of these nodes failed.
* [Issue #1445](https://github.com/manticoresoftware/manticoresearch/issues/1445) Fixed the replication log verbosity level at the `show variables`.
* [Issue #1482](https://github.com/manticoresoftware/manticoresearch/issues/1482) Fixed a replication issue for a joiner node connecting to a cluster on a pod restarted in Kubernetes.
* [Issue #1962](https://github.com/manticoresoftware/manticoresearch/issues/1962) Fixed a long wait for replication to alter on empty cluster with an invalid node name.

### Bug fixes
* [Commit 001d](https://github.com/manticoresoftware/manticoresearch/commit/001d82904e4b887c49387308291aeefb07c0fbc4) Fixed html stipper corrupting memory after processing a joined field
* [Commit 00eb](https://github.com/manticoresoftware/manticoresearch/commit/00eb2ebf) Avoided rewinding stream after flush to prevent miscommunication issues with mysqldump.
* [Commit 0553](https://github.com/manticoresoftware/manticoresearch/commit/05534114acf08c70ffcc41a23d558dec09a34373) Don't wait for preread to finish if it has not started
* [Commit 055a](https://github.com/manticoresoftware/manticoresearch/commit/055aac09f877570cd29b6594cb283182eba7f73b) Fixed large Buddy output string to split among multiple lines in the searchd log.
* [Commit 0a88](https://github.com/manticoresoftware/manticoresearch/commit/0a88056307f82c50fcdc458ad6dbc307a00e3f92) Moved MySQL interface warning about failing header `debugv` verbosity level.
* [Commit 150a](https://github.com/manticoresoftware/manticoresearch/commit/150a20286b7d98d24b844ae577d7fda4a444798d) Fixed race condition on multiple clusters management operations; prohibited creating multiple clusters with the same name or path.
* [Commit 2e40](https://github.com/manticoresoftware/manticoresearch/commit/2e405c1c9ebac6f46206e3df17ac1c19129c9aa3) Fixed implicit cutoff in fulltext queries; split MatchExtended into template partD
* [Commit 75f5](https://github.com/manticoresoftware/manticoresearch/commit/75f51421542a2a0658f9e2f9f31048c8c5224479) Fixed the discrepancy of `index_exact_words` between indexing and loading the table to the daemon.
* [Commit 7643](https://github.com/manticoresoftware/manticoresearch/commit/7643976bea142e08535d701000eff0d09a99fda2) Fixed missed error message for invalid cluster deletion.
* [Commit 7a03](https://github.com/manticoresoftware/manticoresearch/commit/7a034ac990d5ec9344320c05a6a2315583b9b858) Fixed CBO vs queue union; fixed CBO vs RT pseudo sharding
* [Commit 7b4e](https://github.com/manticoresoftware/manticoresearch/commit/7b4ea95e) When starting without the secondary index (SI) library and parameters in the configuration, the misleading warning message 'WARNING: secondary_indexes set but failed to initialize secondary library' was issued.
* [Commit 8496](https://github.com/manticoresoftware/manticoresearch/commit/8496317f099fade8d2db463fb88483e7fcdfe85b) fixed hit sorting in quorum
* [Commit 8973](https://github.com/manticoresoftware/manticoresearch-buddy/commit/8973ad3) Fixed issue with upper case options in the ModifyTable plugin
* [Commit 9935](https://github.com/manticoresoftware/manticoresearch-buddy/commit/99350d4c184379b19f3efa9751d9b5be5e6edfb5) Fixed restoring from a dump with empty json values (represented as NULL)
* [Commit a28f](https://github.com/manticoresoftware/manticoresearch/commit/a28f6e087b3d957e48816070be17d0823904936a) Fixed SST timeout at the joiner node when receiving SST by using pcon.
* [Commit b5a5](https://github.com/manticoresoftware/manticoresearch/commit/b5a55cc26debcddcda5d9caef8366a707e0dd763) Fixed a crash on selecting aliased string attribute
* [Commit c556](https://github.com/manticoresoftware/manticoresearch/commit/c556e7ed8b6da8eac8ef094650df5f0a8977916b) Added query transform of the term into `=term` of full-text query with the `morphology_skip_fields` field.
* [Commit cdc3](https://github.com/manticoresoftware/manticoresearch/commit/cdc38085145cd1025b96fcac2a46188a0425d9e1) Added missing config key (skiplist_cache_size)
* [Commit cf6e](https://github.com/manticoresoftware/manticoresearch/commit/cf6ec3bfdc3f0990fb31ff53822b2331a6667727) Fixed crash at the expression ranker with large complex query.
* [Commit e513](https://github.com/manticoresoftware/manticoresearch/commit/e5139ba75eef7223df375ae3cc3f138fa7d925f1) Fixed fulltext CBO vs invalid index hints
* [Commit eb05](https://github.com/manticoresoftware/manticoresearch/commit/eb0514b3a6f57f278252b0724eebd112ec116e0f) Interrupt preread on shutdown for faster shutdown
* [Commit f945](https://github.com/manticoresoftware/manticoresearch/commit/f94555a293cfa51004ff81135b8bd8500b955115) Changed stack calculation for fulltext queries to avoid a crash in case of a complex query
* [Issue #1262](https://github.com/manticoresoftware/manticoresearch/issues/1262) Fixed a crash of the indexer when indexing an SQL source with multiple columns having the same name.
* [Issue #1273](https://github.com/manticoresoftware/manticoresearch/issues/1273) Return 0 instead of <empty> for non-existing sysvars.
* [Issue #1289](https://github.com/manticoresoftware/manticoresearch/issues/1289) Fixed indextool error when checking external files of the RT table.
* [Issue #1335](https://github.com/manticoresoftware/manticoresearch/issues/1335) Fixed query parse error due to multi wordform inside the phrase.
* [Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) Added the replay of empty binlog files with old binlog versions.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) Fixed removal of the last empty binlog file.
* [Issue #1372](https://github.com/manticoresoftware/manticoresearch/issues/1372) Fixed incorrect relative paths (converted to absolute from the daemon's start directory) after changes in `data_dir` affect the current work directory on daemon start.
* [Issue #1393](https://github.com/manticoresoftware/manticoresearch/issues/1393) Slowest time degradation in hn_small: fetch/cache cpu info on daemon startup
* [Issue #1395](https://github.com/manticoresoftware/manticoresearch/issues/1395) Fixed warning regarding missing external file during index load.
* [Issue #1402](https://github.com/manticoresoftware/manticoresearch/issues/1402) Fixed crash at global groupers on free of data ptr attributes.
* [Issue #1403](https://github.com/manticoresoftware/manticoresearch/issues/1403) _ADDITIONAL_SEARCHD_PARAMS is not working
* [Issue #1427](https://github.com/manticoresoftware/manticoresearch/issues/1427) Fixed per table `agent_query_timeout` being replaced by the default query option `agent_query_timeout`.
* [Issue #1444](https://github.com/manticoresoftware/manticoresearch/issues/1444) Fixed crash at the grouper and ranker when using `packedfactors()` with multiple values per match.
* [Issue #1458](https://github.com/manticoresoftware/manticoresearch/issues/1458) Manticore crashes on frequent index updates
* [Issue #1481](https://github.com/manticoresoftware/manticoresearch/issues/1481) Fixed crash on cleanup of the parsed query after parse error.
* [Issue #1484](https://github.com/manticoresoftware/manticoresearch/issues/1484) Fixed HTTP JSON requests not being routed to buddy.
* [Issue #1499](https://github.com/manticoresoftware/manticoresearch/issues/1499) JSON attribute root value couldn't be an array. Fixed.
* [Issue #1507](https://github.com/manticoresoftware/manticoresearch/issues/1507) Fixed crash on table recreation within the transaction.
* [Issue #1515](https://github.com/manticoresoftware/manticoresearch/issues/1515) Fixed expansion of the short forms of the RU lemmas.
* [Issue #1579](https://github.com/manticoresoftware/manticoresearch/issues/1579) Fixed JSON and STRING attributes usage in [date_format](Functions/Date_and_time_functions.md#DATE_FORMAT()) expression.
* [Issue #1580](https://github.com/manticoresoftware/manticoresearch/issues/1580) Fixed the grouper for multiple aliases to JSON fields.
* [Issue #1594](https://github.com/manticoresoftware/manticoresearch/issues/1594) Wrong total_related in dev: fixed implict cutoff vs limit; added better fullscan detection in json queries
* [Issue #1603](https://github.com/manticoresoftware/manticoresearch/issues/1603) Fixed JSON and STRING attributes usage in all date expression.
* [Issue #1609](https://github.com/manticoresoftware/manticoresearch/issues/1609) crash on using LEVENSHTEIN()
* [Issue #1612](https://github.com/manticoresoftware/manticoresearch/issues/1612) Fixed memory corruption after a search query parse error with highlight.
* [Issue #1614](https://github.com/manticoresoftware/manticoresearch/issues/1614) Disabled wildcard expansion for terms shorter than `min_prefix_len` / `min_infix_len`.
* [Issue #1617](https://github.com/manticoresoftware/manticoresearch/issues/1617) Altered behavior to not log an error if Buddy handles the request successfully.
* [Issue #1635](https://github.com/manticoresoftware/manticoresearch/issues/1635) Fixed total at the meta of the search query for queries with limit set.
* [Issue #1640](https://github.com/manticoresoftware/manticoresearch/issues/1640) Impossible to use a table with an upper case via JSON in plain mode
* [Issue #1643](https://github.com/manticoresoftware/manticoresearch/issues/1643) Provided a default `SPH_EXTNODE_STACK_SIZE` value.
* [Issue #1646](https://github.com/manticoresoftware/manticoresearch/issues/1646) Fixed SphinxQL log of negative filter with ALL/ANY on MVA attribute.
* [Issue #1660](https://github.com/manticoresoftware/manticoresearch/issues/1660) Fix application of docid killlists from other indexes.
* [Issue #1668](https://github.com/manticoresoftware/manticoresearch/issues/1668) Fixed missed matches due to early exit on raw index full scan (without any indexes iterators); removed cutoff from the plain row iterator.
* [Issue #1671](https://github.com/manticoresoftware/manticoresearch/issues/1671) Fixed `FACET` error when querying a distributed table with agent and local tables.
* [Issue #1690](https://github.com/manticoresoftware/manticoresearch/issues/1690) Fixed crash on histogram estimation for large values.
* [Issue #1692](https://github.com/manticoresoftware/manticoresearch/issues/1692) crash on alter table tbl add column col uint
* [Issue #1742](https://github.com/manticoresoftware/manticoresearch/issues/1742) `CREATE TABLE` wasn't failing in case of a missing wordforms file.
* [Issue #1762](https://github.com/manticoresoftware/manticoresearch/issues/1762) The order of attributes in RT tables now follows the configuration order.
* [Issue #1765](https://github.com/manticoresoftware/manticoresearch/issues/1765) HTTP bool query with 'should' condition returns incorrect results
* [Issue #1769](https://github.com/manticoresoftware/manticoresearch/issues/1769) Sorting by string attributes does not work with `SPH_SORT_ATTR_DESC` and `SPH_SORT_ATTR_ASC`.
* [Issue #177](https://github.com/manticoresoftware/manticoresearch-buddy/issues/177) Disabled the `Expect: 100-continue` HTTP header for curl requests to Buddy.
* [Issue #1791](https://github.com/manticoresoftware/manticoresearch/issues/1791) crash caused by GROUP BY alias
* [Issue #1792](https://github.com/manticoresoftware/manticoresearch/issues/1792) SQL meta summary shows wrong time on windows
* [Issue #1794](https://github.com/manticoresoftware/manticoresearch/issues/1794) Fixed a single-term performance drop with JSON queries.
* [Issue #1798](https://github.com/manticoresoftware/manticoresearch/issues/1798) Incompatible filters didn't raise an error on `/search`.
* [Issue #1802](https://github.com/manticoresoftware/manticoresearch/issues/1802) Fixed `ALTER CLUSTER ADD` and `JOIN CLUSTER` operations to wait for each other, preventing a race condition where `ALTER` adds a table to the cluster while the donor sends tables to the joiner node.
* [Issue #1816](https://github.com/manticoresoftware/manticoresearch/issues/1816) `UNFREEZE` wasn't working in some cases
* [Issue #183](https://github.com/manticoresoftware/manticoresearch-buddy/pull/183) Fixed an issue with MVA restoration in some cases
* [Issue #1849](https://github.com/manticoresoftware/manticoresearch/issues/1849) Fixed indextool crash on shutdown if used with MCL.
* [Issue #1872](https://github.com/manticoresoftware/manticoresearch/issues/1872) change plugin_dir set logic on daemon start
* [Issue #1891](https://github.com/manticoresoftware/manticoresearch/issues/1891#issuecomment-2051941200) Manticore crashes with `signal 11` when inserting data
* [Issue #1920](https://github.com/manticoresoftware/manticoresearch/issues/1920) Reduced throttling for [low_priority](Searching/Options.md#Low-Priority)
* [Issue #1924](https://github.com/manticoresoftware/manticoresearch/issues/1924) Mysqldump + mysql restore bug
* [Issue #1951](https://github.com/manticoresoftware/manticoresearch/issues/1951) Fixed incorrect creation of the distributed table in the case of a missing local table or incorrect agent description; now returns an error message.
* [Issue #1972](https://github.com/manticoresoftware/manticoresearch/issues/1972) Implemented a `FREEZE` counter to avoid freeze/unfreeze issues.
* [Issue #1980](https://github.com/manticoresoftware/manticoresearch/issues/1980) Obey query timeout in OR nodes. Previously `max_query_time` could be not working in some cases.
* [Issue #1986](https://github.com/manticoresoftware/manticoresearch/issues/1986) Failed to rename new to current [manticore.json]
* [Issue #1988](https://github.com/manticoresoftware/manticoresearch/issues/1988) A full-text query could ignore a `SecondaryIndex` CBO hint
* [Issue #1990](https://github.com/manticoresoftware/manticoresearch/issues/1990) Fixed `expansion_limit` to slice final result set for call keywords from multiple disk chunks or RAM chunks.
* [Issue #1994](https://github.com/manticoresoftware/manticoresearch/issues/1994) wrong external files
* [Issue #2021](https://github.com/manticoresoftware/manticoresearch/issues/2021) A few manticore-executor processes could be left running after stopping Manticore.
* [Issue #2029](https://github.com/manticoresoftware/manticoresearch/issues/2029) Crash using Levenshtein Distance
* [Issue #2037](https://github.com/manticoresoftware/manticoresearch/issues/2037) Got error after multiple max operator ran on an empty index
* [Issue #2052](https://github.com/manticoresoftware/manticoresearch/issues/2052) crash on multi-group with JSON.field
* [Issue #2067](https://github.com/manticoresoftware/manticoresearch/issues/2067) Manticore was crashing on incorrect request to _update
* [Issue #2069](https://github.com/manticoresoftware/manticoresearch/issues/2069) Fixed an issue with string filter comparators for closed ranges in the JSON interface
* [Issue #2082](https://github.com/manticoresoftware/manticoresearch/issues/2082) `alter` failed when the data_dir path was located on a symlink
* [Issue #2102](https://github.com/manticoresoftware/manticoresearch/pull/2102) Improved special handling of SELECT queries in mysqldump to ensure the resulting INSERT statements are compatible with Manticore
* [Issue #223](https://github.com/manticoresoftware/manticoresearch-buddy/issues/223) Proper error when trying to insert a document w/o schema and w/o column names
* [Issue #239](https://github.com/manticoresoftware/manticoresearch-buddy/issues/239) Auto-schema multi-line insert could fail.
* [Issue #399](https://github.com/manticoresoftware/manticoresearch/issues/399) Added an error message on indexing if an id attribute is declared at the data source.
* [Issue #59](https://github.com/manticoresoftware/manticoresearch-helm/issues/59) Manticore cluster breakdown
* [Issue #68](https://github.com/manticoresoftware/manticoresearch-helm/issues/68) optimize.php would crash if percolate table was present
* [Issue #77](https://github.com/manticoresoftware/manticoresearch-helm/issues/77) Fixed errors when deploying on Kubernetes

### Related with manticore-backup
* [Issue #97](https://github.com/manticoresoftware/manticoresearch-backup/pull/97) Set VIP HTTP port as default when available
Various improvements: improved versions check and streaming ZSTD decompression; added user prompts for version mismatches during restore; fixed incorrect prompting behavior for different versions on restore; enhanced decompression logic to read directly from the stream rather than into working memory; added `--force` flag
* [Commit 3b35](https://github.com/manticoresoftware/manticoresearch-backup/commit/3b357ea) Added backup version display after Manticore search start to identify issues at this stage
* [Commit ad2e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ad2e5da) Updated error message for failed connections to the daemon
* [Commit ce5e](https://github.com/manticoresoftware/manticoresearch-backup/commit/ce5ea85) Fixed issue with converting absolute root backup paths to relative and removed writeable check on restore to enable restoration from different paths
* [Commit db7e](https://github.com/manticoresoftware/manticoresearch-backup/commit/db7e2b9) Added sorting to the file iterator to ensure consistency across various situations
* [Issue #106](https://github.com/manticoresoftware/manticoresearch-backup/issues/105) Backup and restore of multiple configurations
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Added defattr to prevent unusual user permissions in files after installation on RHEL
* [Issue #91](https://github.com/manticoresoftware/manticoresearch-backup/pull/91) Added extra chown to ensure files default to the root user in Ubuntu

### Related with MCL (columnar, secondary, knn libs)
* [Commit f104](https://github.com/manticoresoftware/columnar/commit/f104a4709bbb144140d09a09bc0fc1db1895e331) Vector search support
* [Commit 2169](https://github.com/manticoresoftware/columnar/commit/216953beef6676f7e34c9f6f1a30c5980d746710) Fixed cleanup of temporary files during the interrupted setup of the secondary index build. This resolves the issue where the daemon exceeded the open files limit when creating `tmp.spidx` files
* [Commit 709b](https://github.com/manticoresoftware/columnar/commit/709b9acaaac97d9a1ca8796892f9ad432021c785) Use separate streamvbyte library for columnar and SI
* [Commit 1c26](https://github.com/manticoresoftware/manticoresearch/commit/1c2623763ced14bb94ad624dde9614a938408d89) Added a warning that columnar storage doesn't support json attrs
* [Commit 574c](https://github.com/manticoresoftware/manticoresearch/commit/574c023152564fa72092c18ebd1f594abfd4b6d7) Fixed a crash on saving a disk chunk with mixed rowwise and columnar storage
* [Commit e87f](https://github.com/manticoresoftware/columnar/commit/e87f088b458d4c5b1b6159fc72ed0d57b786891a) Fixed SI iterator being hinted at an already processed block
* [Issue #1474](https://github.com/manticoresoftware/manticoresearch/issues/1474) Update is broken for rowwise MVA column with columnar engine
* [Issue #1510](https://github.com/manticoresoftware/manticoresearch/issues/1510) Fixed crash when aggregating to a columnar attribute used in `HAVING`.
* [Issue #1519](https://github.com/manticoresoftware/manticoresearch/issues/1519) Fixed crash in `expr` ranker on using columnar attribute.

### Related with Docker
* ❗[Issue #42](https://github.com/manticoresoftware/docker/issues/42) Support of [plain indexation](https://github.com/manticoresoftware/docker#building-plain-tables) via environment variables.
* ❗[Issue #47](https://github.com/manticoresoftware/docker/issues/47) Improved flexibility of configuration via environment vars
* [Issue #54](https://github.com/manticoresoftware/docker/pull/54) Improved the [backup and restore](https://github.com/manticoresoftware/docker#backup-and-restore) processes for Docker.
* [Issue #77](https://github.com/manticoresoftware/docker/issues/76) Improved entrypoint to handle backup restoration on first start only
* [Commit a27c](https://github.com/manticoresoftware/docker/commit/a27c048dd9e73d8d6bea8d9a4830cafb486b82c5) Fixed query logging to stdout
* [Issue #38](https://github.com/manticoresoftware/docker/issues/38) Mute BUDDY warnings if EXTRA is not set
* [Issue #71](https://github.com/manticoresoftware/docker/pull/71) Fixed hostname in `manticore.conf.sh`
* [Commit 3acd](https://github.com/manticoresoftware/columnar/commit/3acd6714ffce103447d7931dfc75bb1a7449f340) Fixed data unpacking in SI

# Version 6.2.12
Released: August 23rd 2023

Version 6.2.12 continues the 6.2 series and addresses issues discovered after the release of 6.2.0.

### Bugfixes
* ❗[Issue #1351](https://github.com/manticoresoftware/manticoresearch/issues/1351) "Manticore 6.2.0 doesn't start via systemctl on Centos 7": Modified `TimeoutStartSec` from `infinity` to `0` for better compatibility with Centos 7.
* ❗[Issue #1364](https://github.com/manticoresoftware/manticoresearch/issues/1364) "Crash after upgrading from 6.0.4 to 6.2.0": Added replay functionality for empty binlog files from older binlog versions.
* [PR #1334](https://github.com/manticoresoftware/manticoresearch/pull/1334) "fix typo in searchdreplication.cpp": Corrected a typo in `searchdreplication.cpp`: beggining -> beginning.
* [Issue #1337](https://github.com/manticoresoftware/manticoresearch/issues/1337) "Manticore 6.2.0 WARNING: conn (local)(12), sock=8088: bailing on failed MySQL header, AsyncNetInputBuffer_c::AppendData: error 11 (Resource temporarily unavailable) return -1": Lowered the verbosity level of the MySQL interface warning about the header to logdebugv.
* [Issue #1355](https://github.com/manticoresoftware/manticoresearch/issues/1355) "join cluster hangs when node_address can't be resolved": Improved replication retry when certain nodes are unreachable, and their name resolution fails. This should resolve issues in Kubernetes and Docker nodes related to replication. Enhanced the error message for replication start failures and made updates to test model 376. Additionally, provided a clear error message for name resolution failures.
* [Issue #1361](https://github.com/manticoresoftware/manticoresearch/issues/1361) "No lower case mapping for "Ø" in charset non_cjk": Adjusted the mapping for the 'Ø' character.
* [Issue #1365](https://github.com/manticoresoftware/manticoresearch/issues/1365) "searchd leaves binlog.meta and binlog.001 after clean stop": Ensured that the last empty binlog file is removed properly.
* [Commit 0871](https://github.com/manticoresoftware/manticoresearch/commit/0871070845e93ad702ae0dfb89b35e5f24cd0851): Fixed the `Thd_t` build issue on Windows related to atomic copy restrictions.
* [Commit 1cc0](https://github.com/manticoresoftware/manticoresearch/commit/1cc04bedc174737118fd5a3663e35f4e95bd4f8c): Addressed an issue with FT CBO vs `ColumnarScan`.
* [Commit c6bf](https://github.com/manticoresoftware/manticoresearch/commit/c6bfb30d53e3f4b67f17769667733677a661c95b): Made corrections to test 376 and added a substitution for the `AF_INET` error in the test.
* [Commit cbc3](https://github.com/manticoresoftware/manticoresearch/commit/cbc38705e0b4dcc9665e3e8cf64826290ebd11cf): Resolved a deadlock issue during replication when updating blob attributes versus replacing documents. Also removed the rlock of the index during commit because it's already locked at a more basic level.

### Minor changes
* [Commit 4f91](https://github.com/manticoresoftware/manticoresearch/commit/4f913a36e84e01f950b2797051222b0b1094d6fe) Updated info on `/bulk` endpoints in the manual.

### MCL
* Support of [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) v2.2.4

# Version 6.2.0
Released: August 4th 2023

### Major changes
* The query optimizer has been enhanced to support full-text queries, significantly improving search efficiency and performance.
* Integrations with:
  - [mysqldump](https://dev.mysql.com/doc/refman/8.0/en/mysqldump.html) - to [make logical backups](../Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump) using `mysqldump`
  - [Apache Superset](https://manticoresearch.com/blog/manticoresearch-apache-superset-integration/) and [Grafana](https://manticoresearch.com/blog/manticoresearch-grafana-integration/) to visualize data stored in Manticore
  - [HeidiSQL](https://www.heidisql.com/) and [DBForge](https://www.devart.com/dbforge/) for easier development with Manticore
* We've started using [GitHub workflows](https://github.com/manticoresoftware/manticoresearch/actions), making it simpler for contributors to utilize the same Continuous Integration (CI) process that the core team applies when preparing packages. All jobs can be run on GitHub-hosted runners, which facilitates seamless testing of changes in your fork of Manticore Search.
* We've started using [CLT](https://github.com/manticoresoftware/clt) to test complex scenarios. For example, we're now able to ensure that a package built after a commit can be properly installed across all supported Linux operating systems. The Command Line Tester (CLT) provides a user-friendly way to record tests in an interactive mode and to effortlessly replay them.
* Significant performance improvement in count distinct operation by employing a combination of hash tables and HyperLogLog.
* Enabled multithreaded execution of queries containing secondary indexes, with the number of threads limited to the count of physical CPU cores. This should considerably improve the query execution speed.
* `pseudo_sharding` has been adjusted to be limited to the number of free threads. This update considerably enhances the throughput performance.
* Users now have the option to specify [the default attribute storage engine](../Server_settings/Searchd.md#engine) via the configuration settings, providing better customization to match specific workload requirements.
* Support for [Manticore Columnar Library 2.2.0](https://github.com/manticoresoftware/columnar/) with numerous bug fixes and improvements in [Secondary indexes](../Server_settings/Searchd.md#secondary_indexes).

### Minor changes
* [Buddy #153](https://github.com/manticoresoftware/manticoresearch-buddy/issues/153): The [/pq](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table) HTTP endpoint now serves as an alias for the `/json/pq` HTTP endpoint.
* [Commit 0bf1](https://github.com/manticoresoftware/manticoresearch/commit/0bf17d9e): We've ensured multi-byte compatibility for `upper()` and `lower()`.
* [Commit 2bb9](https://github.com/manticoresoftware/manticoresearch/commit/2bb92765): Instead of scanning the index for `count(*)` queries, a precalculated value is now returned.
* [Commit 3c84](https://github.com/manticoresoftware/manticoresearch/commit/3c84091f): It's now possible to use `SELECT` for making arbitrary calculations and displaying `@@sysvars`. Unlike before, you are no longer limited to just one calculation. Therefore, queries like `select user(), database(), @@version_comment, version(), 1+1 as a limit 10` will return all the columns. Note that the optional 'limit' will always be ignored.
* [Commit 6aca](https://github.com/manticoresoftware/manticoresearch/commit/6aca32f0): Implemented the `CREATE DATABASE` stub query.
* [Commit 9dc1](https://github.com/manticoresoftware/manticoresearch/commit/9dc12334): When executing `ALTER TABLE table REBUILD SECONDARY`, secondary indexes are now always rebuilt, even if attributes weren't updated.
* [Commit 46ed](https://github.com/manticoresoftware/manticoresearch/commit/46edb089): Sorters utilizing precalculated data are now identified before using CBO to avoid unnecessary CBO calculations.
* [Commit 102a](https://github.com/manticoresoftware/manticoresearch/commit/102ac604): Implementing mocked and utilizing of the full-text expression stack to prevent daemon crashes.
* [Commit 979f](https://github.com/manticoresoftware/manticoresearch/commit/979fa27c): A speedy code path has been added for match cloning code for matches that don't use string/mvas/json attributes.
* [Commit a073](https://github.com/manticoresoftware/manticoresearch/commit/a0735ffe): Added support for the `SELECT DATABASE()` command. However, it will always return `Manticore`. This addition is crucial for integrations with various MySQL tools.
* [Commit bc04](https://github.com/manticoresoftware/manticoresearch/commit/bc04908): Modified the response format of the [/cli](../Connecting_to_the_server/HTTP.md#/cli) endpoint, and added the `/cli_json` endpoint to function as the previous `/cli`.
* [Commit d70b](https://github.com/manticoresoftware/manticoresearch/commit/d70b0d58): The `thread_stack` can now be altered during runtime using the `SET` statement. Both session-local and daemon-wide variants are available. Current values can be accessed in the `show variables` output.
* [Commit d96e](https://github.com/manticoresoftware/manticoresearch/commit/d96ec6b9): Code has been integrated into CBO to more accurately estimate the complexity of executing filters over string attributes.
* [Commit e77d](https://github.com/manticoresoftware/manticoresearch/commit/e77dd72f5a04531c352fad0d7afcd2a1cbae2510): The DocidIndex cost calculation has been improved, enhancing overall performance.
* [Commit f3ae](https://github.com/manticoresoftware/manticoresearch/commit/f3ae8bea): Load metrics, similar to 'uptime' on Linux, are now visible in the `SHOW STATUS` command.
* [Commit f3cc](https://github.com/manticoresoftware/manticoresearch/commit/f3cc0971): The field and attribute order for `DESC` and `SHOW CREATE TABLE` now match that of `SELECT * FROM`.
* [Commit f3d2](https://github.com/manticoresoftware/manticoresearch/commit/f3d248a6): Different internal parsers now provide their internal mnemonic code (e.g., `P01`) during various errors. This enhancement aids in identifying which parser caused an error and also obscures non-essential internal details.
* [Issue #271](https://github.com/manticoresoftware/manticoresearch/issues/271) "Sometimes CALL SUGGEST does not suggest a correction of a single letter typo": Improved [SUGGEST/QSUGGEST](../Searching/Spell_correction.md#CALL-QSUGGEST,-CALL-SUGGEST) behaviour for short words: added the option `sentence` to show the entire sentence
* [Issue #696](https://github.com/manticoresoftware/manticoresearch/issues/696) "Percolate index does not search properly by exact phrase query when stemming enabled": The percolate query has been modified to handle an exact term modifier, improving search functionality.
* [Issue #829](https://github.com/manticoresoftware/manticoresearch/issues/829) "DATE FORMATTING methods": added the [date_format()](../Functions/Date_and_time_functions.md#DATE_FORMAT()) select list expression, which exposes the `strftime()` function.
* [Issue #961](https://github.com/manticoresoftware/manticoresearch/issues/961) "Sorting buckets via HTTP JSON API": introduced an optional [sort property](../Searching/Faceted_search.md#HTTP-JSON) for each bucket of aggregates in the HTTP interface.
* [Issue #1062](https://github.com/manticoresoftware/manticoresearch/issues/1062) "Improve error logging of JSON insert api failure - "unsupported value type"": The `/bulk` endpoint reports information regarding the number of processed and non-processed strings (documents) in case of an error.
* [Issue #1070](https://github.com/manticoresoftware/manticoresearch/issues/1070) "CBO hints don't support multiple attributes": Enabled index hints to handle multiple attributes.
* [Issue #1106](https://github.com/manticoresoftware/manticoresearch/issues/1106) "Add tags to http search query": Tags have been added to [HTTP PQ responses](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md?client=JSON#Adding-rules-to-a-percolate-table).
* [Issue #1301](https://github.com/manticoresoftware/manticoresearch/issues/1301) "buddy should not create table in parallel": Resolved an issue that was causing failures from parallel CREATE TABLE operations. Now, only one `CREATE TABLE` operation can run at a time.
* [Issue #1303](https://github.com/manticoresoftware/manticoresearch/issues/1303) "add support of @ to column names".
* [Issue #1316](https://github.com/manticoresoftware/manticoresearch/issues/1316) "Queries on taxi dataset are slow with ps=1": The CBO logic has been refined, and the default histogram resolution has been set to 8k for better accuracy on attributes with randomly distributed values.
* [Issue #1317](https://github.com/manticoresoftware/manticoresearch/issues/1317) "Fix CBO vs fulltext on hn dataset": Enhanced logic has been implemented for determining when to use bitmap iterator intersection and when to use a priority queue.
* [Issue #1318](https://github.com/manticoresoftware/manticoresearch/issues/1318) "columnar: change iterator interface to single-call" : Columnar iterators now use a single `Get` call, replacing the previous two-step `AdvanceTo` + `Get` calls to retrieve a value.
* [Issue #1319](https://github.com/manticoresoftware/manticoresearch/issues/1319) "Aggregate calc speedup (remove CheckReplaceEntry?)": The `CheckReplaceEntry` call was removed from the group sorter to expedite the calculation of aggregate functions.
* [Issue #1320](https://github.com/manticoresoftware/manticoresearch/issues/1320) "create table read_buffer_docs/hits doesn't understand k/m/g syntax": The `CREATE TABLE` options `read_buffer_docs` and `read_buffer_hits` now support k/m/g syntax.
* [Language packs](../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) for English, German and Russian can now be effortlessly installed on Linux by executing the command `apt/yum install manticore-language-packs`. On macOS, use the command `brew install manticoresoftware/tap/manticore-language-packs`.
* Field and attribute order is now consistent between `SHOW CREATE TABLE` and `DESC` operations.
* If disk space is insufficient when executing `INSERT` queries, new `INSERT` queries will fail until enough disk space becomes available.
* The [UINT64()](../Functions/Type_casting_functions.md#UINT64%28%29) type conversion function has been added.
* The `/bulk` endpoint now processes empty lines as a [commit](../Data_creation_and_modification/Transactions.md#BEGIN,-COMMIT,-and-ROLLBACK) command. More info [here](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?client=JSON#Bulk-adding-documents).
* Warnings have been implemented for [invalid index hints](../Searching/Options.md#Query-optimizer-hints), providing more transparency and allowing for error mitigation.
* When `count(*)` is used with a single filter, queries now leverage precalculated data from secondary indexes when available, substantially speeding up query times.

### ⚠️ Breaking changes
* ⚠️ Tables created or modified in version 6.2.0 cannot be read by older versions.
* ⚠️ Document IDs are now handled as unsigned 64-bit integers during indexing and INSERT operations.
* ⚠️ The syntax for query optimizer hints has been updated. The new format is `/*+ SecondaryIndex(uid) */`. Please note that the old syntax is no longer supported.
* ⚠️ [Issue #1160](https://github.com/manticoresoftware/manticoresearch/issues/1160): The usage of `@` in table names has been disallowed to prevent syntax conflicts.
* ⚠️ String fields/attributes marked as `indexed` and `attribute` are now regarded as a single field during `INSERT`, `DESC`, and `ALTER` operations.
* ⚠️ [Issue #1057](https://github.com/manticoresoftware/manticoresearch/issues/1057): MCL libraries will no longer load on systems that don't support SSE 4.2.
* ⚠️ [Issue #1143](https://github.com/manticoresoftware/manticoresearch/issues/1143): [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) was broken. Fixed and is now effective.

### Bugfixes
* [Commit 2a6e](https://github.com/manticoresoftware/manticoresearch/commit/2a6ea8f7) "Crash on DROP TABLE": resolved a problem causing extended wait times to finish write operations (optimize, disk chunk save) on an RT table when executing a DROP TABLE statement. Added a warning to notify when a table directory is not empty after executing a DROP TABLE command.
* [Commit 2ebd](https://github.com/manticoresoftware/manticoresearch/commit/2ebd424d): Support for columnar attributes, which was missing in the code used for grouping by multiple attributes, has been added.
* [Commit 3be4](https://github.com/manticoresoftware/manticoresearch/commit/3be4503f752216b37b8cde8e47fd2d77bbce63ae) Resolved a crash issue potentially caused by disk space running out by properly handling write errors in binlog.
* [Commit 6adb](https://github.com/manticoresoftware/manticoresearch/commit/6adb0934): A crash that occasionally occurred when using multiple columnar scan iterators (or secondary index iterators) in a query has been fixed.
* [Commit 6bd9](https://github.com/manticoresoftware/manticoresearch/commit/6bd9f709): Filters were not being removed when using sorters that use precalculated data. This issue has been fixed.
* [Commit 6d03](https://github.com/manticoresoftware/manticoresearch/commit/6d03566a): The CBO code has been updated to provide better estimates for queries using filters over row-wise attributes executed in multiple threads.
* [Commit 6dd3](https://github.com/manticoresoftware/manticoresearch/commit/6dd3964), [Helm #56](https://github.com/manticoresoftware/manticoresearch-helm/issues/56) "fatal crash dump in Kubernetes cluster": Fixed a defective bloom filter for the JSON root object; fixed daemon crash due to filtering by a JSON field.
* [Commit 6e1b](https://github.com/manticoresoftware/manticoresearch/commit/6e1b849) Rectified daemon crash caused by invalid `manticore.json` config.
* [Commit 6fbc](https://github.com/manticoresoftware/manticoresearch/commit/6fbc4189) Fixed the json range filter to support int64 values.
* [Commit 9c67](https://github.com/manticoresoftware/manticoresearch/commit/9c677426) `.sph` files could be corrupted `ALTER`. Fixed.
* [Commit 77cc](https://github.com/manticoresoftware/manticoresearch/commit/77ccd8f): A shared key has been added for the replication of the replace statement to resolve a `pre_commit` error occurring when replace is replicated from multiple master nodes.
* [Commit 2884](https://github.com/manticoresoftware/manticoresearch/commit/2884519e) resolved issues with bigint checks over functions like 'date_format()'.
* [Commit 9513](https://github.com/manticoresoftware/manticoresearch/commit/9513aca5): Iterators are no longer displayed in [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META) when sorters utilize precalculated data.
* [Commit a2a7](https://github.com/manticoresoftware/manticoresearch/commit/a2a70555): The fulltext node stack size has been updated to prevent crashes on complex fulltext queries.
* [Commit a062](https://github.com/manticoresoftware/manticoresearch/commit/a0626d7e): A bug causing a crash during the replication of updates with JSON and string attributes has been resolved.
* [Commit b3e6](https://github.com/manticoresoftware/manticoresearch/commit/b3e63e5d): The string builder has been updated to use 64-bit integers to avoid crashes when dealing with large data sets.
* [Commit c472](https://github.com/manticoresoftware/manticoresearch/commit/c472e5b): Addressed a crash that was occurring with count distinct across multiple indexes.
* [Commit d073](https://github.com/manticoresoftware/manticoresearch/commit/d0730272): Fixed an issue where queries over disk chunks of RT indexes could be executed in multiple threads even if `pseudo_sharding` was disabled.
* [Commit d205](https://github.com/manticoresoftware/manticoresearch/commit/d205508e) The set of values returned by the `show index status` command has been modified and now varies depending on the type of index in use.
* [Commit e9bc](https://github.com/manticoresoftware/manticoresearch/commit/e9bc2f9) Fixed an HTTP error when processing bulk requests and an issue where the error wasn't being returned to the client from the net loop.
* [Commit f77c](https://github.com/manticoresoftware/manticoresearch/commit/f77ce0e6) use of an extended stack for PQ.
* [Commit fac2](https://github.com/manticoresoftware/manticoresearch/commit/fac2d9687a20582765aaa89be9597804c4f845af) Updated the export ranker output to align with [packedfactors()](../Functions/Searching_and_ranking_functions.md#PACKEDFACTORS%28%29).
* [Commit ff87](https://github.com/manticoresoftware/manticoresearch/commit/ff87cc4): Fixed an issue with the string list in the filter of the SphinxQL query log.
* [Issue #589](https://github.com/manticoresoftware/manticoresearch/issues/589) "The charset definition seems to depend on the ordering of codes": Fixed incorrect charset mapping for duplicates.
* [Issue #811](https://github.com/manticoresoftware/manticoresearch/issues/811) "Mapping multiple words in word forms interferes phrase search with CJK punctuations between keywords": Fixed ngram token position within phrase query with wordforms.
* [Issue #834](https://github.com/manticoresoftware/manticoresearch/issues/834) "Equals sign in search query breaks request": Ensured the exact symbol can be escaped and fixed double exact expansion by the `expand_keywords` option.
* [Issue #864](https://github.com/manticoresoftware/manticoresearch/issues/864) "exceptions/stopwords conflict"
* [Issue #910](https://github.com/manticoresoftware/manticoresearch/issues/910) "Manticore crash when calling call snippets() with libstemmer_fr and index_exact_words": Resolved internal conflicts causing crashes when `SNIPPETS()` was called.
* [Issue #946](https://github.com/manticoresoftware/manticoresearch/issues/946) "Duplicate records during SELECT": Fixed the issue of duplicate documents in the result set for a query with `not_terms_only_allowed` option to RT index with killed documents.
* [Issue #967](https://github.com/manticoresoftware/manticoresearch/issues/967) "Using JSON arguments in UDF functions leads to a crash": Fixed a daemon crash when processing a search with pseudo-sharding enabled and UDF with JSON argument.
* [Issue #1050](https://github.com/manticoresoftware/manticoresearch/issues/1050) "count(*) in FEDERATED": Fixed a daemon crash occurring with a query through a `FEDERATED` engine with aggregate.
* [Issue #1052](https://github.com/manticoresoftware/manticoresearch/issues/1052) Fixed an issue where `rt_attr_json` column was incompatible with columnar storage.
* [Issue #1072](https://github.com/manticoresoftware/manticoresearch/issues/1072) "* is removed from search query by ignore_chars": Fixed this issue so wildcards in a query aren't impacted by `ignore_chars`.
* [Issue #1075](https://github.com/manticoresoftware/manticoresearch/issues/1075) "indextool --check fails if there's a distributed table": indextool is now compatible with instances having 'distributed' and 'template' indexes in the json config.
* [Issue #1081](https://github.com/manticoresoftware/manticoresearch/issues/1081) "particular select on particular RT dataset leads to crash of searchd": Resolved daemon crash on a query with packedfactors and large internal buffer.
* [Issue #1095](https://github.com/manticoresoftware/manticoresearch/issues/1095) "With not_terms_only_allowed deleted documents are ignored"
* [Issue #1099](https://github.com/manticoresoftware/manticoresearch/issues/1099) "indextool --dumpdocids is not working": Restored functionality of the `--dumpdocids` command.
* [Issue #1100](https://github.com/manticoresoftware/manticoresearch/issues/1100) "indextool --buildidf is not working": indextool now closes the file after finishing globalidf.
* [Issue #1104](https://github.com/manticoresoftware/manticoresearch/issues/1104) "Count(*) is trying to be treated as schema set in remote tables": Resolved an issue where an error message was being sent by the daemon for queries into the distributed index when the agent returned an empty result set.
* [Issue #1109](https://github.com/manticoresoftware/manticoresearch/issues/1109) "FLUSH ATTRIBUTES hangs with threads=1".
* [Issue #1126](https://github.com/manticoresoftware/manticoresearch/issues/1126) "Lost connection to MySQL server during query - manticore 6.0.5": Crashes that were happening when using multiple filters over columnar attributes have been addressed.
* [Issue #1135](https://github.com/manticoresoftware/manticoresearch/issues/1135) "JSON string filtering case sensitivity": Corrected the collation to function correctly for filters used in HTTP search requests.
* [Issue #1140](https://github.com/manticoresoftware/manticoresearch/issues/1140) "Match in a wrong field": Fixed the damage related with `morphology_skip_fields`.
* [Issue #1155](https://github.com/manticoresoftware/manticoresearch/issues/1155) "system remote commands via API should pass g_iMaxPacketSize": Made updates to bypass the `max_packet_size` check for replication commands between nodes. Additionally, the latest cluster error has been added to the status display.
* [Issue #1302](https://github.com/manticoresoftware/manticoresearch/issues/1302) "tmp files left on failed optimize": Corrected an issue where temporary files were left behind after an error occurred during a merge or optimize process.
* [Issue #1304](https://github.com/manticoresoftware/manticoresearch/issues/1304) "add env var for buddy start timeout": Added environment variable `MANTICORE_BUDDY_TIMEOUT` (default 3 seconds) to control the daemon's wait duration for a buddy message at startup.
* [Issue #1305](https://github.com/manticoresoftware/manticoresearch/issues/1305) "Int overflow when saving PQ meta": Mitigated excessive memory consumption by daemon on saving large PQ index.
* [Issue #1306](https://github.com/manticoresoftware/manticoresearch/issues/1306) "Can't recreate RT table after altering its external file": Rectified an error of alter with empty string for external files; fixed RT index external files left after altering external files.
* [Issue #1307](https://github.com/manticoresoftware/manticoresearch/issues/1307) "SELECT statement sum(value) as value doesn't work properly": Fixed issue where select list expression with alias could hide index attribute; also fixed sum to count in int64 for integer.
* [Issue #1308](https://github.com/manticoresoftware/manticoresearch/issues/1308) "Avoid binding to localhost in replication": Ensured replication doesn't bind to localhost for host names with multiple IPs.
* [Issue #1309](https://github.com/manticoresoftware/manticoresearch/issues/1309) "reply to mysql client failed for data larger 16Mb": Fixed the issue of returning a SphinxQL packet larger than 16Mb to the client.
* [Issue #1310](https://github.com/manticoresoftware/manticoresearch/issues/1310) "wrong reference in "paths to external files should be absolute": Corrected the display of the full path to external files in `SHOW CREATE TABLE`.
* [Issue #1311](https://github.com/manticoresoftware/manticoresearch/issues/1311) "debug build crashes on long strings in snippets": Now, long strings (>255 characters) are permitted in the text targeted by the `SNIPPET()` function.
* [Issue #1312](https://github.com/manticoresoftware/manticoresearch/issues/1312) "spurious crash on use-after-delete in kqueue polling (master-agent)": Fixed crashes when the master cannot connect to the agent on kqueue-driven systems (FreeBSD, MacOS, etc.).
* [Issue #1313](https://github.com/manticoresoftware/manticoresearch/issues/1313) "too long connect to itself": When connecting from the master to agents on MacOS/BSD, a unified connect+query timeout is now used instead of just connect.
* [Issue #1314](https://github.com/manticoresoftware/manticoresearch/issues/1314) "pq (json meta) with unreached embedded synonyms fails to load": Fixed the embedded synonyms flag in pq.
* [Issue #1315](https://github.com/manticoresoftware/manticoresearch/issues/1315) "Allow some functions (sint, fibonacci, second, minute, hour, day, month, year, yearmonth, yearmonthday) to use implicitly promoted argument values".
* [Issue #1321](https://github.com/manticoresoftware/manticoresearch/issues/1321) "Enable multithreaded SI in fullscan, but limit threads": Code has been implemented into CBO to better predict multithreaded performance of secondary indexes when they're utilized in a full-text query.
* [Issue #1322](https://github.com/manticoresoftware/manticoresearch/issues/1322) "count(*) queries still slow after using precalc sorters": Iterators are no longer initiated when employing sorters that use precalculated data, circumventing detrimental performance effects.
* [Issue #1411](https://github.com/manticoresoftware/manticoresearch/issues/1141) "query log in sphinxql does not preserve original queries for MVA's": Now, `all()/any()` is logged.

# Version 6.0.4
Released: March 15 2023

### New features
* Improved integration with Logstash, Beats etc. including:
  - Support for Logstash versions 7.6 - 7.15, Filebeat versions 7.7 - 7.12  
  - Auto-schema support.
  - Added handling of bulk requests in Elasticsearch-like format.
* [Buddy commit ce90](https://github.com/manticoresoftware/manticoresearch-buddy/commit/ce907ea) Log Buddy version on Manticore start.

### Bugfixes
* [Issue #588](https://github.com/manticoresoftware/manticoresearch/issues/588), [Issue #942](https://github.com/manticoresoftware/manticoresearch/issues/942) Fixed bad character at the search meta and call keywords for bigram index.
* [Issue #1027](https://github.com/manticoresoftware/manticoresearch/issues/1027) Lowercase HTTP headers are rejected.
* ❗[Issue #1039](https://github.com/manticoresoftware/manticoresearch/issues/1039) Fixed memory leak at daemon on reading output of the Buddy console.
* [Issue #1056](https://github.com/manticoresoftware/manticoresearch/issues/1056) Fixed unexpected behavior of question mark.
* [Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - Fixed race condition in tokenizer lowercase tables causing a crash.
* [Commit 59bb](https://github.com/manticoresoftware/manticoresearch/commit/59bb54c) Fixed bulk writes processing in the JSON interface for documents with id explicitly set to null.
* [Commit 7b6b](https://github.com/manticoresoftware/manticoresearch/commit/7b6b25f) Fixed term statistics in CALL KEYWORDS for multiple same terms.
* [Commit f381](https://github.com/manticoresoftware/manticoresearch/commit/f381ad2) Default config is now created by Windows installer; paths are no longer substituted in runtime.
* [Commit 6940](https://github.com/manticoresoftware/manticoresearch/commit/6940e95), [Commit cc5a](https://github.com/manticoresoftware/manticoresearch/commit/cc5a480) Fixed replication issues for cluster with nodes in multiple networks.
* [Commit 4972](https://github.com/manticoresoftware/manticoresearch/commit/49722ab) Fixed `/pq` HTTP endpoint to be an alias of the `/json/pq` HTTP endpoint.
* [Commit 3b53](https://github.com/manticoresoftware/manticoresearch/commit/3b5385a) Fixed daemon crash on Buddy restart.
* [Buddy commit fba9](https://github.com/manticoresoftware/manticoresearch-buddy/commit/fba9c8c) Display original error on invalid request received.
* [Buddy commit db95](https://github.com/manticoresoftware/manticoresearch-buddy/commit/db9532c) Allow spaces in backup path and add some magic to regexp to support single quotes also.

# Version 6.0.2
Released: Feb 10 2023

### Bugfixes
* [Issue #1024 crash 2](https://github.com/manticoresoftware/manticoresearch/issues/1024) Crash / Segmentation Fault on Facet search with larger number of results
* ❗[Issue #1029](https://github.com/manticoresoftware/manticoresearch/issues/1029) - WARNING: Compiled-in value KNOWN_CREATE_SIZE (16) is less than measured (208). Consider to fix the value!
* ❗[Issue #1032](https://github.com/manticoresoftware/manticoresearch/issues/1032) - Manticore 6.0.0 plain index crashes
* ❗[Issue #1033](https://github.com/manticoresoftware/manticoresearch/issues/1033) - multiple distributed lost on daemon restart
* ❗[Issue #1064](https://github.com/manticoresoftware/manticoresearch/issues/1064) - race condition in tokenizer lowercase tables

# Version 6.0.0
Released: Feb 7 2023

Starting with this release, Manticore Search comes with Manticore Buddy, a sidecar daemon written in PHP that handles high-level functionality that does not require super low latency or high throughput. Manticore Buddy operates behind the scenes, and you may not even realize it is running. Although it is invisible to the end user, it was a significant challenge to make Manticore Buddy easily installable and compatible with the main C++-based daemon. This major change will allow the team to develop a wide range of new high-level features, such as shards orchestration, access control and authentication, and various integrations like mysqldump, DBeaver, Grafana mysql connector. For now it already handles [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) and [Auto schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

This release also includes more than 130 bug fixes and numerous features, many of which can be considered major.

### Major Changes
* 🔬 Experimental: you can now execute Elasticsearch-compatible [insert](Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md?example=Elasticsearch#Adding-documents-to-a-real-time-table) and [replace](Data_creation_and_modification/Updating_documents/REPLACE.md?example=Elasticsearch#REPLACE) JSON queries which enables using Manticore with tools like Logstash (version < 7.13), Filebeat and other tools from the Beats family. Enabled by default. You can disable it using `SET GLOBAL ES_COMPAT=off`.
* Support for [Manticore Columnar Library 2.0.0](https://github.com/manticoresoftware/columnar/) with numerous fixes and improvements in [Secondary indexes](../Server_settings/Searchd.md#secondary_indexes). **⚠️ BREAKING CHANGE**: Secondary indexes are ON by default as of this release. Make sure you do [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuild-secondary-index) if you are upgrading from Manticore 5. See below for more details.
* [Commit c436](https://github.com/manticoresoftware/manticoresearch/commit/c436f9023536f767610451911955ae36d90aa638) Auto-schema: you can now skip creating a table, just insert the first document and Manticore will create the table automatically based on its fields. Read more about this in detail [here](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema). You can turn it on/off using [searchd.auto_schema](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).
* Vast revamp of [cost-based optimizer](../Searching/Cost_based_optimizer.md) which lowers query response time in many cases.
  - [Issue #1008](https://github.com/manticoresoftware/manticoresearch/issues/1008) Parallelization performance estimate in CBO.
  - [Issue #1014](https://github.com/manticoresoftware/manticoresearch/issues/1014) CBO is now aware of [secondary indexes](../Server_settings/Searchd.md#secondary_indexes) and can act smarter.
  - [Commit cef9](https://github.com/manticoresoftware/columnar/commit/cef9b54dc3a4ee9138ebc4df3a671715b996d6b3) Encoding stats of columnar tables/fields are now stored in the meta data to help CBO make smarter decisions.
  - [Commit 2b95](https://github.com/manticoresoftware/manticoresearch/commit/2b95fb8c2ba422c9c5a66e2b61c1f7922f31c7aa) Added CBO hints for fine-tuning its behaviour.
* [Telemetry](../Telemetry.md#Telemetry): we are excited to announce the addition of telemetry in this release. This feature allows us to collect anonymous and depersonalized metrics that will help us improve the performance and user experience of our product. Rest assured, all data collected is **completely anonymous and will not be linked to any personal information**. This feature can be [easily turned off](../Telemetry.md#Telemetry) in the settings if desired.
* [Commit 5aaf](https://github.com/manticoresoftware/manticoresearch/commit/5aafb17d81b601e399e212a344a4738f65576cdd) [ALTER TABLE table_name REBUILD SECONDARY](../Updating_table_schema_and_settings.md#Rebuild-secondary-index) to rebuild secondary indexes whenever you want, for example:
  - when you migrate from Manticore 5 to the newer version,
  - when you did [UPDATE](../Data_creation_and_modification/Updating_documents/UPDATE.md#UPDATE) (i.e. [in-place update, not replace](Data_creation_and_modification/Updating_documents/REPLACE_vs_UPDATE.md)) of an attribute in the index
* [Issue #821](https://github.com/manticoresoftware/manticoresearch/issues/821) New tool `manticore-backup` for [backing up and restoring Manticore instance](../Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL command [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) to do backups from inside Manticore.
* SQL command [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES) as an easy way to see running queries rather than threads.
* [Issue #551](https://github.com/manticoresoftware/manticoresearch/issues/551) SQL command `KILL` to kill a long-running `SELECT`.
* Dynamic `max_matches` for aggregation queries to increase accuracy and lower response time.

### Minor changes
* [Issue #822](https://github.com/manticoresoftware/manticoresearch/issues/822) SQL commands [FREEZE/UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) to prepare a real-time/plain table for a backup.
* [Commit c470](https://github.com/manticoresoftware/manticoresearch/commit/c47052f18ad4868134de1a97e8d7da570e531dfa) New settings `accurate_aggregation` and `max_matches_increase_threshold`  for controlled aggregation accuracy.
* [Issue #718](https://github.com/manticoresoftware/manticoresearch/issues/718) Support for signed negative 64-bit IDs. Note, you still can't use IDs > 2^63, but you can now use ids in the range of from -2^63 to 0.
* As we recently added support for secondary indexes, things became confusing as "index" could refer to a secondary index, a full-text index, or a plain/real-time `index`. To reduce confusion, we are renaming the latter to "table". The following SQL/command line commands are affected by this change. Their old versions are deprecated, but still functional:
  - `index <table name>` => `table <table name>`,
  - `searchd -i / --index` => `searchd -t / --table`,
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`,
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`,
  - `FLUSH RTINDEX` => `FLUSH TABLE`,
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`,
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`,
  - `RELOAD INDEX` => `RELOAD TABLE`,
  - `RELOAD INDEXES` => `RELOAD TABLES`.

  We are not planning to make the old forms obsolete, but to ensure compatibility with the documentation, we recommend changing the names in your application. What will be changed in a future release is the "index" to "table" rename in the output of various SQL and JSON commands.
* Queries with stateful UDFs are now forced to be executed in a single thread.
* [Issue #1011](https://github.com/manticoresoftware/manticoresearch/issues/1011) Refactoring of all related to time scheduling as a prerequisite for parallel chunks merging.
* **⚠️ BREAKING CHANGE**: Columnar storage format has been changed. You need to rebuild those tables that have columnar attributes.
* **⚠️ BREAKING CHANGE**: Secondary indexes file format has been changed, so if you are using secondary indexes for searching and have `searchd.secondary_indexes = 1` in your configuration file, be aware that the new Manticore version **will skip loading the tables that have secondary indexes**. It's recommended to:
  - Before you upgrade change `searchd.secondary_indexes` to 0 in the configuration file.
  - Run the instance. Manticore will load up the tables with a warning.
  - Run `ALTER TABLE <table name> REBUILD SECONDARY` for each index to rebuild secondary indexes.

  If you are running a replication cluster, you'll need to run `ALTER TABLE <table name> REBUILD SECONDARY` on all the nodes or follow [this instruction](../Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) with just change: run the `ALTER .. REBUILD SECONDARY` instead of the `OPTIMIZE`.
* **⚠️ BREAKING CHANGE**: The binlog version has been updated, so any binlogs from previous versions will not be replayed. It is important to ensure that Manticore Search is stopped cleanly during the upgrade process. This means that there should be no binlog files in `/var/lib/manticore/binlog/` except for `binlog.meta` after stopping the previous instance.
* [Issue #849](https://github.com/manticoresoftware/manticoresearch/issues/849) `SHOW SETTINGS`: you can now see the settings from the configuration file from inside Manticore.
* [Issue #1007](https://github.com/manticoresoftware/manticoresearch/issues/1007) [SET GLOBAL CPUSTATS=1/0](../Server_settings/Setting_variables_online.md#SET) turns on/off cpu time tracking; [SHOW THREADS](../Node_info_and_management/SHOW_THREADS.md) now doesn't show CPU statistics when the cpu time tracking is off.
* [Issue #1009](https://github.com/manticoresoftware/manticoresearch/issues/1009) RT table RAM chunk segments can now be merged while the RAM chunk is being flushed.
* [Issue #1012](https://github.com/manticoresoftware/manticoresearch/issues/1012) Added secondary index progress to the output of [indexer](../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* [Issue #1013](https://github.com/manticoresoftware/manticoresearch/issues/1013) Previously a table record could be removed by Manticore from the index list if it couldn't start serving it on start. The new behaviour is to keep it in the list to try to load it on the next start.
* [indextool --docextract](../Miscellaneous_tools.md#indextool) returns all the words and hits belonging to requested document.
* [Commit 2b29](https://github.com/manticoresoftware/manticoresearch/commit/2b296ee20e520b85bcbb4383e87095d31e7165dc) Environment variable `dump_corrupt_meta` enables dumping a corrupted table meta data to log in case searchd can't load the index.
* [Commit c7a3](https://github.com/manticoresoftware/manticoresearch/commit/c7a3136eefe0eb2f69e3d99fae9c10beb5821ea6) `DEBUG META` can show `max_matches` and pseudo sharding statistics.
* [Commit 6bca](https://github.com/manticoresoftware/manticoresearch/commit/6bca4848dc5c8396dbc1e057be4b9b2d579794e8) A better error instead of the confusing "Index header format is not json, will try it as binary...".
* [Commit bef3](https://github.com/manticoresoftware/lemmatizer-uk/commit/bef3ff0386d3ee87ec57619782100972c1122e47) Ukirainian lemmatizer path has been changed.
* [Commit 4ae7](https://github.com/manticoresoftware/manticoresearch/commit/4ae789595329a2951e194d1191ddb3121459a560) Secondary indexes statistics has been added to [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META).
* [Commit 2e7c](https://github.com/manticoresoftware/manticoresearch/commit/2e7c585e) JSON interface can now be easily visualized using Swagger Editor https://manual.manticoresearch.com/Openapi#OpenAPI-specification.
* **⚠️ BREAKING CHANGE**: Replication protocol has been changed. If you are running a replication cluster, then when upgrading to Manticore 5 you need to:
  - stop all your nodes first cleanly
  - and then start the node which was stopped last with `--new-cluster` (run tool `manticore_new_cluster` in Linux).
  - read about [restarting a cluster](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) for more details.

### Changes related with Manticore Columnar Library
* Refactoring of Secondary indexes integration with Columnar storage.
* [Commit efe2](https://github.com/manticoresoftware/columnar/commit/efe26b2a) Manticore Columnar Library optimization which can lower response time by partial preliminary min/max evaluation.
* [Commit 2757](https://github.com/manticoresoftware/columnar/commit/2757b99b) If a disk chunk merge is interrupted, the daemon now cleans up the MCL-related tmp files.
* [Commit e9c6](https://github.com/manticoresoftware/manticoresearch/commit/e9c672014b63e2b49e7a5a53880583af86c3ab4e) Columnar and secondary libraries versions are dumped to log on crash.
* [Commit f5e8](https://github.com/manticoresoftware/columnar/commit/f5e84eeb8b6cb102f13e3468246702427527fef9) Added support for quick doclist rewinding to secondary indexes.
* [Commit 06df](https://github.com/manticoresoftware/manticoresearch/commit/06dfdd2e668933495dd6cb52d5ba2e0aa9b26e58) Queries like `select attr, count(*) from plain_index` (w/o filtering) are now faster in case you are using MCL.
* [Commit 0a76](https://github.com/manticoresoftware/manticoresearch/commit/0a76ecb9adbe886868b962dd425de5f475bbf55e) @@autocommit in HandleMysqlSelectSysvar for compatibility with .net connector for mysql greater than 8.25
* **⚠️ BREAKING CHANGE**: [MCL Issue #17](https://github.com/manticoresoftware/columnar/issues/17) MCL: add SSE code to columnar scan. MCL now requires at least SSE4.2.

### Packaging-related changes
* [Commit 4d19](https://github.com/manticoresoftware/manticoresearch/commit/4d19f5cbe49a31228aa09253f061165bfe80e51b) **⚠️ BREAKING CHANGE**: Support for Debian Stretch and Ubuntu Xenial has been discontinued.
* RHEL 9 support including Centos 9, Alma Linux 9 and Oracle Linux 9.
* [Issue #924](https://github.com/manticoresoftware/manticoresearch/issues/924) Debian Bookworm support.
* [Issue #636](https://github.com/manticoresoftware/manticoresearch/issues/636) Packaging: arm64 builds for Linuxes and MacOS.
* [PR #26](https://github.com/manticoresoftware/docker/pull/26) Multi-architecture (x86_64 / arm64) docker image.
* [Simplified package building for contributors](../Installation/Compiling_from_sources.md#Building-using-CI-Docker).
* It's now possible to install a specific version using APT.
* [Commit a6b8](https://github.com/manticoresoftware/manticoresearch/commit/51fddca5c2a3ebb8576fae4c18660656ba22de0f) Windows installer (previously we provided just an archive).
* Switched to compiling using CLang 15.
* **⚠️ BREAKING CHANGE**: Custom Homebrew formulas including the formula for Manticore Columnar Library. To install Manticore, MCL and any other necessary components, use the following command `brew install manticoresoftware/manticore/manticoresearch manticoresoftware/manticore/manticore-extra`.

### Bugfixes
* [Issue #479](https://github.com/manticoresoftware/manticoresearch/issues/479) Field with name `text`
* [Issue #501](https://github.com/manticoresoftware/manticoresearch/issues/501) id can't be non bigint
* [Issue #646](https://github.com/manticoresoftware/manticoresearch/issues/646) ALTER vs field with name "text"
* ❗[Issue #652](https://github.com/manticoresoftware/manticoresearch/issues/652) Possible BUG: HTTP (JSON) offset and limit affects facet results
* ❗[Issue #827](https://github.com/manticoresoftware/manticoresearch/issues/827) Searchd hangs/crashes under intensive loading
* ❗[Issue #996](https://github.com/manticoresoftware/manticoresearch/issues/996) PQ index out of memory
* ❗[Commit 1041](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` has been broken all the time since Sphinx. Fixed.
* [MCL Issue #14](https://github.com/manticoresoftware/columnar/issues/14) MCL: crash on select when too many ft fields
* [Issue #470](https://github.com/manticoresoftware/manticoresearch/issues/470) sql_joined_field can't be stored
* [Issue #713](https://github.com/manticoresoftware/manticoresearch/issues/713) Crash when using LEVENSHTEIN()
* [Issue #743](https://github.com/manticoresoftware/manticoresearch/issues/743) Manticore crashes unexpected and cant to normal restart
* [Issue #788](https://github.com/manticoresoftware/manticoresearch/issues/788) CALL KEYWORDS through /sql returns control char which breaks json
* [Issue #789](https://github.com/manticoresoftware/manticoresearch/issues/789) mariadb can't create table FEDERATED
* [Issue #796](https://github.com/manticoresoftware/manticoresearch/issues/796) WARNING: dlopen() failed: /usr/bin/lib_manticore_columnar.so: cannot open shared object file: No such file or directory
* [Issue #797](https://github.com/manticoresoftware/manticoresearch/issues/797) Manticore crashes when search with ZONESPAN is done through api
* [Issue #799](https://github.com/manticoresoftware/manticoresearch/issues/799) wrong weight when using multiple indexes and facet distinct
* [Issue #801](https://github.com/manticoresoftware/manticoresearch/issues/801) SphinxQL group query hangs after SQL index reprocessing
* [Issue #802](https://github.com/manticoresoftware/manticoresearch/issues/802) MCL: Indexer crashes in 5.0.2 and manticore-columnar-lib 1.15.4
* [Issue #813](https://github.com/manticoresoftware/manticoresearch/issues/813) Manticore 5.0.2 FEDERATED returns empty set (MySQL 8.0.28)
* [Issue #824](https://github.com/manticoresoftware/manticoresearch/issues/824) select COUNT DISTINCT on 2 indices when result is zero throws internal error
* [Issue #826](https://github.com/manticoresoftware/manticoresearch/issues/826) CRASH on delete query
* [Issue #843](https://github.com/manticoresoftware/manticoresearch/issues/843) MCL: Bug with long text field
* [Issue #856](https://github.com/manticoresoftware/manticoresearch/issues/856) 5.0.2 rtindex: Aggregate search limit behavior is not as expected
* [Issue #863](https://github.com/manticoresoftware/manticoresearch/issues/863) Hits returned is Nonetype object even for searches that should return multiple results
* [Issue #870](https://github.com/manticoresoftware/manticoresearch/issues/870) Crash with using Attribute and Stored Field in SELECT expression
* [Issue #872](https://github.com/manticoresoftware/manticoresearch/issues/872) table gets invisible after crash
* [Issue #877](https://github.com/manticoresoftware/manticoresearch/issues/877) Two negative terms in search query gives error: query is non-computable
* [Issue #878](https://github.com/manticoresoftware/manticoresearch/issues/878) a -b -c is not working via json query_string
* [Issue #886](https://github.com/manticoresoftware/manticoresearch/issues/886) pseudo_sharding with query match
* [Issue #893](https://github.com/manticoresoftware/manticoresearch/issues/893) Manticore 5.0.2 min/max function doesn't work as expecting ...
* [Issue #896](https://github.com/manticoresoftware/manticoresearch/issues/896) Field "weight" is not parsed correctly
* [Issue #897](https://github.com/manticoresoftware/manticoresearch/issues/897) Manticore service crash upon start and keep restarting
* [Issue #900](https://github.com/manticoresoftware/manticoresearch/issues/900) group by j.a, smth works wrong
* [Issue #913](https://github.com/manticoresoftware/manticoresearch/issues/913) Searchd crash when expr used in ranker, but only for queries with two proximities
* [Issue #916](https://github.com/manticoresoftware/manticoresearch/issues/916) net_throttle_action is broken
* [Issue #919](https://github.com/manticoresoftware/manticoresearch/issues/919) MCL: Manticore crashes on query execution and other crashed during cluster recovery.
* [Issue #925](https://github.com/manticoresoftware/manticoresearch/issues/925) SHOW CREATE TABLE outputs w/o backticks
* [Issue #930](https://github.com/manticoresoftware/manticoresearch/issues/930) It's now possible to query Manticore from Java via JDBC connector
* [Issue #933](https://github.com/manticoresoftware/manticoresearch/issues/933) bm25f ranking problems
* [Issue #934](https://github.com/manticoresoftware/manticoresearch/issues/934) configless indexes frozen in watchdog on the first-load state
* [Issue #937](https://github.com/manticoresoftware/manticoresearch/issues/937) Segfault when sorting facet data
* [Issue #940](https://github.com/manticoresoftware/manticoresearch/issues/940) crash on CONCAT(TO_STRING)
* [Issue #947](https://github.com/manticoresoftware/manticoresearch/issues/947) In some cases a single simple select could cause the whole instance stall, so you couldn't log in to it or run any other query until the running select is done.
* [Issue #948](https://github.com/manticoresoftware/manticoresearch/issues/948) Indexer crash
* [Issue #950](https://github.com/manticoresoftware/manticoresearch/issues/950) wrong count from facet distinct
* [Issue #953](https://github.com/manticoresoftware/manticoresearch/issues/953) LCS is calculating incorrectly in built-in sph04 ranker
* [Issue #955](https://github.com/manticoresoftware/manticoresearch/issues/955) 5.0.3 dev crashing
* [Issue #963](https://github.com/manticoresoftware/manticoresearch/issues/963) FACET with json on engine columnar crash
* [Issue #982](https://github.com/manticoresoftware/manticoresearch/issues/982) MCL: 5.0.3 crash from secondary index
* [PR #984](https://github.com/manticoresoftware/manticoresearch/pull/984) @@autocommit in HandleMysqlSelectSysvar
* [PR #985](https://github.com/manticoresoftware/manticoresearch/pull/985) Fix thread-chunk distribution in RT indexes
* [Issue #985](https://github.com/manticoresoftware/manticoresearch/issues/985) Fix thread-chunk distribution in RT indexes
* [Issue #986](https://github.com/manticoresoftware/manticoresearch/issues/986) wrong default max_query_time
* [Issue #987](https://github.com/manticoresoftware/manticoresearch/issues/987) Crash on when using regex expression in multithreaded execution
* [Issue #988](https://github.com/manticoresoftware/manticoresearch/issues/988) Broken backward index compatibility
* [Issue #989](https://github.com/manticoresoftware/manticoresearch/issues/989) indextool reports error checking columnar attributes
* [Issue #990](https://github.com/manticoresoftware/manticoresearch/issues/990) memleak of json grouper clones
* [Issue #991](https://github.com/manticoresoftware/manticoresearch/issues/991) Memleak of levenshtein func cloning
* [Issue #992](https://github.com/manticoresoftware/manticoresearch/issues/992) Error message lost when loading meta
* [Issue #993](https://github.com/manticoresoftware/manticoresearch/issues/993) Propagate errors from dynamic indexes/subkeys and sysvars
* [Issue #994](https://github.com/manticoresoftware/manticoresearch/issues/994) Crash on count distinct over a columnar string in columnar storage
* [Issue #995](https://github.com/manticoresoftware/manticoresearch/issues/995) MCL: min(pickup_datetime) from taxi1 crashes
* [Issue #997](https://github.com/manticoresoftware/manticoresearch/issues/997) empty excludes JSON query removes columns from select list
* [Issue #998](https://github.com/manticoresoftware/manticoresearch/issues/998) Secondary tasks run on current scheduler sometimes cause abnormal side effects
* [Issue #999](https://github.com/manticoresoftware/manticoresearch/issues/999) crash with facet distinct and different schemas
* [Issue #1000](https://github.com/manticoresoftware/manticoresearch/issues/1000) MCL: Columnar rt index became damaged after run without columnar library
* [Issue #1001](https://github.com/manticoresoftware/manticoresearch/issues/1001) implicit cutoff is not working in json
* [Issue #1002](https://github.com/manticoresoftware/manticoresearch/issues/1002) Columnar grouper issue
* [Issue #1003](https://github.com/manticoresoftware/manticoresearch/issues/1003) Unable to delete last field from the index
* [Issue #1004](https://github.com/manticoresoftware/manticoresearch/issues/1004) wrong behaviour after --new-cluster
* [Issue #1005](https://github.com/manticoresoftware/manticoresearch/issues/1005) "columnar library not loaded", but it's not required
* [Issue #1006](https://github.com/manticoresoftware/manticoresearch/issues/1006) no error for delete query
* [Issue #1010](https://github.com/manticoresoftware/manticoresearch/issues/1010) Fixed ICU data file location in Windows builds
* [PR #1018](https://github.com/manticoresoftware/manticoresearch/pull/1018) Handshake send problem
* [Issue #1020](https://github.com/manticoresoftware/manticoresearch/issues/1020) Display id in show create table
* [Issue #1024 crash 1](https://github.com/manticoresoftware/manticoresearch/issues/1024) Crash / Segmentation Fault on Facet search with larger number of results.
* [Issue #1026](https://github.com/manticoresoftware/manticoresearch/issues/1026) RT index: searchd "stuck" forever when many documents are being inserted and RAMchunk gets full
* [Commit 4739](https://github.com/manticoresoftware/manticoresearch/commit/4739dafd) Thread gets stuck on shutdown while replication is busy between nodes
* [Commit ab87](https://github.com/manticoresoftware/manticoresearch/commit/ab87836782e7ae43fe5f9dea739eed0d336b42c1) Mixing floats and ints in a JSON range filter could make Manticore ignore the filter
* [Commit d001](https://github.com/manticoresoftware/manticoresearch/commit/d00101c2905f6393ce3fca23c4f6dcb2506f4bd9) Float filters in JSON were inaccurate
* [Commit 4092](https://github.com/manticoresoftware/manticoresearch/commit/4092c6a8) Discard uncommitted txns if index altered (or it can crash)
* [Commit 9692](https://github.com/manticoresoftware/manticoresearch/commit/9692a840) Query syntax error when using backslash
* [Commit 0c19](https://github.com/manticoresoftware/manticoresearch/commit/0c19ad8d) workers_clients could be wrong in SHOW STATUS
* [Commit 1772](https://github.com/manticoresoftware/manticoresearch/commit/17720e1e2f2ec38337df565bee78f7ce9d9c744a) fixed a crash on merging ram segments w/o docstores
* [Commit f45b](https://github.com/manticoresoftware/manticoresearch/commit/f45b337b5c265433bb7d6262ad65d02f65edbaf3) Fixed missed ALL/ANY condition for equals JSON filter
* [Commit 3e83](https://github.com/manticoresoftware/manticoresearch/commit/3e83c1f0c0e6e7a9d4297113f69a112cbabd9e64) Replication could fail with `got exception while reading ist stream: mkstemp(./gmb_pF6TJi) failed: 13 (Permission denied)` if the searchd was started from a directory it can't write to.
* [Commit 92e5](https://github.com/manticoresoftware/manticoresearch/commit/92e5629f7150f62524b385fca670a01ba9590f5b) Since 4.0.2 crash log included only offsets. This commit fixes that.

# Version 5.0.2
Released: May 30th 2022

### Bugfixes
* ❗[Issue #791](https://github.com/manticoresoftware/manticoresearch/issues/791) - wrong stack size could cause a crash.

# Version 5.0.0
Released: May 18th 2022


### Major new features
* 🔬 Support for [Manticore Columnar Library 1.15.2](https://github.com/manticoresoftware/columnar/), which enables [Secondary indexes](../Server_settings/Searchd.md#secondary_indexes) beta version. Building secondary indexes is on by default for plain and real-time columnar and row-wise indexes (if [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) is in use), but to enable it for searching you need to set `secondary_indexes = 1` either in your configuration file or using [SET GLOBAL](../Server_settings/Setting_variables_online.md). The new functionality is supported in all operating systems except old Debian Stretch and Ubuntu Xenial.
* [Read-only mode](Security/Read_only.md): you can now specify listeners that process only read queries discarding any writes.
* New [/cli](../Connecting_to_the_server/HTTP.md#/cli) endpoint for running SQL queries over HTTP even easier.
* Faster bulk INSERT/REPLACE/DELETE via JSON over HTTP: previously you could provide multiple write commands via HTTP JSON protocol, but they were processed one by one, now they are handled as a single transaction.
* [#720](https://github.com/manticoresoftware/manticoresearch/issues/720) [Nested filters](../Searching/Filters.md#Nested-bool-query) support in JSON protocol. Previously you couldn't code things like `a=1 and (b=2 or c=3)` in JSON: `must` (AND), `should` (OR) and `must_not` (NOT) worked only on the highest level. Now they can be nested.
* Support for [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) in the HTTP protocol. You can now use chunked transfer in your application to transmit large batches with reduced resource consumption (since calculating `Content-Length` is unnecessary). On the server side, Manticore now always processes incoming HTTP data in a streaming manner, without waiting for the entire batch to be transferred as before, which:
  - reduces peak RAM usage, lowering the risk of OOM
  - decreases response time (our tests indicated an 11% reduction for processing a 100MB batch)
  - allows you to bypass [max_packet_size](../Server_settings/Searchd.md#max_packet_size) and transfer batches much larger than the maximum allowed value of `max_packet_size` (128MB), for example, 1GB at a time.
* [#719](https://github.com/manticoresoftware/manticoresearch/issues/719) HTTP interface support of `100 Continue`: now you can transfer large batches from `curl` (including curl libraries used by various programming languages) which by default does `Expect: 100-continue` and waits some time before actually sending the batch. Previously you had to add `Expect: ` header, now it's not needed.

  <details>

  Previously (note the response time):

  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  * Done waiting for 100-continue
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.0 15e927b@211223 release (columnar 1.11.4 327b3d4@211223)
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 434
  <
  * Connection #0 to host localhost left intact
  {"items":[{"insert":{"_index":"user","_id":2811798918248005633,"created":true,"result":"created","status":201}},{"insert":{"_index":"user","_id":2811798918248005634,"created":true,"result":"created","status":201}},{"insert":{"_index":"user","_id":2811798918248005635,"created":true,"result":"created","status":201}},{"insert":{"_index":"user","_id":2811798918248005636,"created":true,"result":"created","status":201}}],"errors":false}
  real	0m1.022s
  user	0m0.001s
  sys	0m0.010s
  ```

  Now:
  ```bash
  $ time curl -v -sX POST http://localhost:9318/bulk -H "Content-Type: application/x-ndjson" --data '{"insert": {"index": "user", "doc":  {"name":"Prof. Matt Heaney IV","email":"ibergnaum@yahoo.com","description":"Tempora ullam eaque consequatur. Vero aut minima ut et ut omnis officiis vel. Molestiae quis voluptatum sint numquam.","age":15,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Prof. Boyd McKenzie","email":"carlotta11@hotmail.com","description":"Blanditiis maiores odio corporis eaque illum. Aut et rerum iste. Neque et ullam quisquam officia dignissimos quo cumque.","age":84,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Mr. Johann Smith","email":"stiedemann.tristin@ziemann.com","description":"Temporibus amet magnam consequatur omnis consequatur illo fugit. Debitis natus doloremque est tempore deserunt vero. Harum eos corrupti nemo ut.","age":89,"active":1}}}
  {"insert": {"index": "user", "doc":  {"name":"Hector Pouros","email":"hickle.mafalda@hotmail.com","description":" as voluptatem inventore sit. Aliquam fugit perferendis est id aut odio et sapiente.","age":64,"active":1}}}'
  *   Trying 127.0.0.1...
  * Connected to localhost (127.0.0.1) port 9318 (#0)
  > POST /bulk HTTP/1.1
  > Host: localhost:9318
  > User-Agent: curl/7.47.0
  > Accept: */*
  > Content-Type: application/x-ndjson
  > Content-Length: 1025
  > Expect: 100-continue
  >
  < HTTP/1.1 100 Continue
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 0
  * We are completely uploaded and fine
  < HTTP/1.1 200 OK
  < Server: 4.2.1 63e5749@220405 dev
  < Content-Type: application/json; charset=UTF-8
  < Content-Length: 147
  <
  * Connection #0 to host localhost left intact
  {"items":[{"bulk":{"_index":"user","_id":2811798919590182916,"created":4,"deleted":0,"updated":0,"result":"created","status":201}}],"errors":false}
  real	0m0.015s
  user	0m0.005s
  sys	0m0.004s
  ```

  </details>

* **⚠️ BREAKING CHANGE**: [Pseudo sharding](../Server_settings/Searchd.md#pseudo_sharding) is enabled by default. If you want to disable it make sure you add `pseudo_sharding = 0` to section `searchd` of your Manticore configuration file.
* Having at least one full-text field in a real-time/plain index is not mandatory anymore. You can now use Manticore even in cases not having anything to do with full-text search.
* [Fast fetching](../Creating_a_table/Data_types.md#fast_fetch) for attributes backed by [Manticore Columnar Library](https://github.com/manticoresoftware/columnar): queries like `select * from <columnar table>` are now much faster than previously, especially if there are many fields in the schema.
* **⚠️ BREAKING CHANGE**: Implicit [cutoff](../Searching/Options.md#cutoff). Manticore now doesn't spend time and resources processing data you don't need in the result set which will be returned. The downside is that it affects `total_found` in [SHOW META](../Node_info_and_management/SHOW_META.md#SHOW-META) and [hits.total](../Searching/Full_text_matching/Basic_usage.md#HTTP-JSON) in JSON output. It is now only accurate in case you see `total_relation: eq` while `total_relation: gte` means the actual number of matching documents is greater than the `total_found` value you've got. To retain the previous behaviour you can use search option `cutoff=0`, which makes `total_relation` always `eq`.
* **⚠️ BREAKING CHANGE**: All full-text fields are now [stored](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) by default. You need to use `stored_fields = ` (empty value) to make all fields non-stored (i.e. revert to the previous behaviour).
* [#715](https://github.com/manticoresoftware/manticoresearch/issues/715) HTTP JSON supports [search options](../Searching/Options.md#General-syntax).

### Minor changes
* **⚠️ BREAKING CHANGE**: Index meta file format change. Previously meta files (`.meta`, `.sph`) were in binary format, now it's just json. The new Manticore version will convert older indexes automatically, but:
  - you can get warning like `WARNING: ... syntax error, unexpected TOK_IDENT`
  - you won't be able to run the index with previous Manticore versions, make sure you have a backup
* **⚠️ BREAKING CHANGE**: Session state support with help of [HTTP keep-alive](../Connecting_to_the_server/HTTP.md#Keep-alive). This makes HTTP stateful when the client supports it too. For example, using the new [/cli](../Connecting_to_the_server/HTTP.md#/cli) endpoint and HTTP keep-alive (which is on by default in all browsers) you can call `SHOW META` after `SELECT` and it will work the same way it works via mysql. Note, previously `Connection: keep-alive` HTTP header was supported too, but it only caused reusing the same connection. Since this version it also makes the session stateful.
* You can now specify `columnar_attrs = *` to define all your attributes as columnar in the [plain mode](Read_this_first.md#Real-time-mode-vs-plain-mode) which is useful in case the list is long.
* Faster replication SST
* **⚠️ BREAKING CHANGE**: Replication protocol has been changed. If you are running a replication cluster, then when upgrading to Manticore 5 you need to:
  - stop all your nodes first cleanly
  - and then start the node which was stopped last with `--new-cluster` (run tool `manticore_new_cluster` in Linux).
  - read about [restarting a cluster](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) for more details.
* Replication improvements:
  - Faster SST
  - Noise resistance which can help in case of unstable network between replication nodes
  - Improved logging
* Security improvement: Manticore now listens on `127.0.0.1` instead of `0.0.0.0` in case no `listen` at all is specified in config. Even though in the default configuration which is shipped with Manticore Search the `listen` setting is specified and it's not typical to have a configuration with no `listen` at all, it's still possible. Previously Manticore would listen on `0.0.0.0` which is not secure, now it listens on `127.0.0.1` which is usually not exposed to the Internet.
* Faster aggregation over columnar attributes.
* Increased `AVG()` accuracy: previously Manticore used `float` internally for aggregations, now it uses `double` which increases the accuracy significantly.
* Improved support for JDBC MySQL driver.
* `DEBUG malloc_stats` support for [jemalloc](https://github.com/jemalloc/jemalloc).
* [optimize_cutoff](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#optimize_cutoff) is now available as a per-table setting which can be set when you CREATE or ALTER a table.
* **⚠️ BREAKING CHANGE**: [query_log_format](../Server_settings/Searchd.md#query_log_format) is now **`sphinxql` by default**. If you are used to `plain` format you need to add `query_log_format = plain` to your configuration file.
* Significant memory consumption improvements: Manticore consumes significantly less RAM now in case of long and intensive insert/replace/optimize workload in case stored fields are used.
* [shutdown_timeout](../Server_settings/Searchd.md#shutdown_timeout) default value was increased from 3 seconds to 60 seconds.
* [Commit ffd0](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Support for Java mysql connector >= 6.0.3: in [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) they changed the way they connect to mysql which broke compatibility with Manticore. The new behaviour is now supported.
* [Commit 1da6](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) disabled saving a new disk chunk on loading an index (e.g. on searchd startup).
* [Issue #746](https://github.com/manticoresoftware/manticoresearch/issues/746) Support for glibc >= 2.34.
* [Issue #784](https://github.com/manticoresoftware/manticoresearch/issues/784) count 'VIP' connections separately from usual (non-VIP). Previously VIP connections were counted towards the `max_connections` limit, which could cause "maxed out" error for non-VIP connections. Now VIP connections are not counted towards the limit. Current number of VIP connections can be also seen in `SHOW STATUS` and `status`.
* [ID](../Creating_a_table/Data_types.md#Document-ID) can now be specified explicitly.
* [Issue #687](https://github.com/manticoresoftware/manticoresearch/issues/687) support zstd compression for mysql proto

### ⚠️ Other minor breaking changes
* ⚠️ BM25F formula has been slightly updated to improve search relevance. This only affects search results in case you use function [BM25F()](../Functions/Searching_and_ranking_functions.md#BM25F%28%29), it doesn't change behaviour of the default ranking formula.
* ⚠️ Changed behaviour of REST [/sql](../Connecting_to_the_server/HTTP.md#/sql?mode=raw) endpoint: `/sql?mode=raw` now requires escaping and returns an array.
* ⚠️ Format change of the response of `/bulk` INSERT/REPLACE/DELETE requests:
  - previously each sub-query constituted a separate transaction and resulted in a separate response
  - now the whole batch is considered a single transaction, which returns a single response
* ⚠️ Search options `low_priority` and `boolean_simplify` now require a value (`0/1`): previously you could do `SELECT ... OPTION low_priority, boolean_simplify`, now you need to do `SELECT ... OPTION low_priority=1, boolean_simplify=1`.
* ⚠️ If you are using old [php](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.php), [python](https://github.com/manticoresoftware/manticoresearch/blob/master/api/sphinxapi.py) or [java](https://github.com/manticoresoftware/manticoresearch/tree/master/api/java) clients please follow the corresponding link and find an updated version. **The old versions are not fully compatible with Manticore 5.**
* ⚠️ HTTP JSON requests are now logged in different format in mode `query_log_format=sphinxql`. Previously only full-text part was logged, now it's logged as is.

### New packages
* **⚠️ BREAKING CHANGE**: because of the new structure when you upgrade to Manticore 5 it's recommended to remove old packages before you install the new ones:
  - RPM-based: `yum remove manticore*`
  - Debian and Ubuntu: `apt remove manticore*`
* New deb/rpm packages structure. Previous versions provided:
  - `manticore-server` with `searchd` (main search daemon) and all needed for it
  - `manticore-tools` with `indexer` and `indextool`
  - `manticore` including everything
  - `manticore-all` RPM as a meta package referring to `manticore-server` and `manticore-tools`

  The new structure is:
  - `manticore` - deb/rpm meta package which installs all the above as dependencies
  - `manticore-server-core` - `searchd` and everything to run it alone
  - `manticore-server` - systemd files and other supplementary scripts
  - `manticore-tools` - `indexer`, `indextool` and other tools
  - `manticore-common` - default configuration file, default data directory, default stopwords
  - `manticore-icudata`, `manticore-dev`, `manticore-converter` didn't change much
  - `.tgz` bundle which includes all the packages
* Support for Ubuntu Jammy
* Support for Amazon Linux 2 via [YUM repo](../Installation/RHEL_and_Centos.md#YUM-repository)

### Bugfixes
* [Issue #815](https://github.com/manticoresoftware/manticoresearch/issues/815) Random crash when using UDF function
* [Issue #287](https://github.com/manticoresoftware/manticoresearch/issues/287) out of memory while indexing RT index
* [Issue #604](https://github.com/manticoresoftware/manticoresearch/issues/604) Breaking change 3.6.0, 4.2.0 sphinxql-parser
* [Issue #667](https://github.com/manticoresoftware/manticoresearch/issues/667) FATAL: out of memory (unable to allocate 9007199254740992 bytes)
* [Issue #676](https://github.com/manticoresoftware/manticoresearch/issues/676) Strings not passed correctly to UDFs
* ❗[Issue #698](https://github.com/manticoresoftware/manticoresearch/issues/698) Searchd crashes after trying to add a text column to a rt index
* [Issue #705](https://github.com/manticoresoftware/manticoresearch/issues/705) Indexer couldn't find all columns
* ❗[Issue #709](https://github.com/manticoresoftware/manticoresearch/issues/705) Grouping by json.boolean works wrong
* [Issue #716](https://github.com/manticoresoftware/manticoresearch/issues/716) indextool commands related to index (eg. --dumpdict) failure
* ❗[Issue #724](https://github.com/manticoresoftware/manticoresearch/issues/724) Fields disappear from the selection
* [Issue #727](https://github.com/manticoresoftware/manticoresearch/issues/727) .NET HttpClient Content-Type incompatibility when using `application/x-ndjson`
* [Issue #729](https://github.com/manticoresoftware/manticoresearch/issues/729) Field length calculation
* ❗[Issue #730](https://github.com/manticoresoftware/manticoresearch/issues/730) create/insert into/drop columnar table has a memleak
* [Issue #731](https://github.com/manticoresoftware/manticoresearch/issues/731) Empty column in results under certain conditions
* ❗[Issue #749](https://github.com/manticoresoftware/manticoresearch/issues/749) Crash of daemon on start
* ❗[Issue #750](https://github.com/manticoresoftware/manticoresearch/issues/750) Daemon hangs on start
* ❗[Issue #751](https://github.com/manticoresoftware/manticoresearch/issues/751) Crash at SST
* [Issue #752](https://github.com/manticoresoftware/manticoresearch/issues/752) Json attribute marked as columnar when engine='columnar'
* [Issue #753](https://github.com/manticoresoftware/manticoresearch/issues/753) Replication listens on 0
* [Issue #754](https://github.com/manticoresoftware/manticoresearch/issues/754) columnar_attrs = * is not working with csvpipe
* ❗[Issue #755](https://github.com/manticoresoftware/manticoresearch/issues/755) Crash on select float in columnar in rt
* ❗[Issue #756](https://github.com/manticoresoftware/manticoresearch/issues/756) Indextool changes rt index during check
* [Issue #757](https://github.com/manticoresoftware/manticoresearch/issues/757) Need a check for listeners port range intersections
* [Issue #758](https://github.com/manticoresoftware/manticoresearch/issues/758) Log original error in case RT index failed to save disk chunk
* [Issue #759](https://github.com/manticoresoftware/manticoresearch/issues/759) Only one error reported for RE2 config
* ❗[Issue #760](https://github.com/manticoresoftware/manticoresearch/issues/760) RAM consumption changes in commit 5463778558586d2508697fa82e71d657ac36510f
* [Issue #761](https://github.com/manticoresoftware/manticoresearch/issues/761) 3rd node doesn't make a non-primary cluster after dirty restart
* [Issue #762](https://github.com/manticoresoftware/manticoresearch/issues/762) Update counter gets increased by 2
* [Issue #763](https://github.com/manticoresoftware/manticoresearch/issues/763) New version 4.2.1 corrupt index created with 4.2.0 with morphology using
* [Issue #764](https://github.com/manticoresoftware/manticoresearch/issues/764) No escaping in json keys /sql?mode=raw
* ❗[Issue #765](https://github.com/manticoresoftware/manticoresearch/issues/765) Using function hides other values
* ❗[Issue #766](https://github.com/manticoresoftware/manticoresearch/issues/766) Memleak triggered by a line in FixupAttrForNetwork
* ❗[Issue #767](https://github.com/manticoresoftware/manticoresearch/issues/767) Memleak in 4.2.0 and 4.2.1 related with docstore cache
* [Issue #768](https://github.com/manticoresoftware/manticoresearch/issues/768) Strange ping-pong with stored fields over network
* [Issue #769](https://github.com/manticoresoftware/manticoresearch/issues/769) lemmatizer_base reset to empty if not mentioned in 'common' section
* [Issue #770](https://github.com/manticoresoftware/manticoresearch/issues/770) pseudo_sharding makes SELECT by id slower
* [Issue #771](https://github.com/manticoresoftware/manticoresearch/issues/771) DEBUG malloc_stats output zeros when using jemalloc
* [Issue #772](https://github.com/manticoresoftware/manticoresearch/issues/772) Drop/add column makes value invisible
* [Issue #773](https://github.com/manticoresoftware/manticoresearch/issues/773) Can't add column bit(N) to columnar table
* [Issue #774](https://github.com/manticoresoftware/manticoresearch/issues/774) "cluster" gets empty on start in manticore.json
* ❗[Commit 1da4](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP actions are not tracked in SHOW STATUS
* [Commit 3810](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) disable pseudo_sharding for low frequency single keyword queries
* [Commit 8003](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) fixed stored attributes vs index merge
* [Commit cddf](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) generalized distinct value fetchers; added specialized distinct fetchers for columnar strings
* [Commit fba4](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) fixed fetching null integer attributes from docstore
* [Commit f300](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` could be specified twice in query log

## Version 4.2.0, Dec 23 2021

### Major new features
* **Pseudo-sharding support for real-time indexes and full-text queries**. In previous release we added limited pseudo sharding support. Starting from this version you can get all benefits of the pseudo sharding and your multi-core processor by just enabling [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding). The coolest thing is that you don't need to do anything with your indexes or queries for that, just enable it and if you have free CPU it will be used to lower your response time. It supports plain and real-time indexes for full-text, filtering and analytical queries. For example, here is how enabling pseudo sharding can make most queries' **response time in average about 10x lower** on [Hacker news curated comments dataset](https://zenodo.org/record/45901/) multiplied 100 times (116 million docs in a plain index).

![Pseudo sharding on vs off in 4.2.0](4.2.0_ps_on_vs_off.png)

* [**Debian Bullseye**](https://manticoresearch.com/install/) is now supported.

<!-- example pq_transactions_4.2.0 -->
* PQ transactions are now atomic and isolated. Previously PQ transactions support was limited. It enables much **faster REPLACE into PQ**, especially when you need to replace a lot of rules at once. Performance details:

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->

It takes **48 seconds to insert 1M PQ rules** and **406 seconds to REPLACE just 40K** in 10K batches.

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:24:30 AM CET 2021
Wed Dec 22 10:25:18 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 30000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:26:23 AM CET 2021
Wed Dec 22 10:26:27 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	6m46.195s
user	0m0.035s
sys	0m0.008s
```

<!-- intro -->

#### 4.2.0

<!-- request 4.2.0 -->

It takes **34 seconds to insert 1M PQ rules** and **23 seconds to REPLACE them** in 10K batches.

```sql
root@perf3 ~ # mysql -P9306 -h0 -e "drop table if exists pq; create table pq (f text, f2 text, j json, s string) type='percolate';"; date; for m in `seq 1 1000`; do (echo -n "insert into pq (id,query,filters,tags) values "; for n in `seq 1 1000`; do echo -n "(0,'@f (cat | ( angry dog ) | (cute mouse)) @f2 def', 'j.json.language=\"en\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; [ $n != 1000 ] && echo -n ","; done; echo ";")|mysql -P9306 -h0; done; date; mysql -P9306 -h0 -e "select count(*) from pq"

Wed Dec 22 10:06:38 AM CET 2021
Wed Dec 22 10:07:12 AM CET 2021
+----------+
| count(*) |
+----------+
|  1000000 |
+----------+

root@perf3 ~ # date; (echo "begin;"; for offset in `seq 0 10000 990000`; do n=0; echo "replace into pq (id,query,filters,tags) values "; for id in `mysql -P9306 -h0 -NB -e "select id from pq limit $offset, 10000 option max_matches=1000000"`; do echo "($id,'@f (tiger | ( angry bear ) | (cute panda)) @f2 def', 'j.json.language=\"de\"', '{\"tag1\":\"tag1\",\"tag2\":\"tag2\"}')"; n=$((n+1)); [ $n != 10000 ] && echo -n ","; done; echo ";"; done; echo "commit;") > /tmp/replace.sql; date
Wed Dec 22 10:12:31 AM CET 2021
Wed Dec 22 10:14:00 AM CET 2021
root@perf3 ~ # time mysql -P9306 -h0 < /tmp/replace.sql

real	0m23.248s
user	0m0.891s
sys	0m0.047s
```

<!-- end -->

### Minor changes
*  [optimize_cutoff](Server_settings/Searchd.md#optimize_cutoff) is now available as a configuration option in section `searchd`. It's useful when you want to limit the RT chunks count in all your indexes to a particular number globally.
* [Commit 0087](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) accurate [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) and [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) over several local physical indexes (real-time/plain) with identical fields set/order.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) bigint support for `YEAR()` and other timestamp functions.
* [Commit 8e85](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Adaptive [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Previously Manticore Search was collecting exactly up to `rt_mem_limit` of data before saving a new disk chunk to disk, and while saving was still collecting up to 10% more (aka double-buffer) to minimize possible insert suspension. If that limit was also exhausted, adding new documents was blocked until the disk chunk was fully saved to disk. The new adaptive limit is built on the fact that we have [auto-optimize](Server_settings/Searchd.md#auto_optimize) now, so it's not a big deal if disk chunks do not fully respect `rt_mem_limit` and start flushing a disk chunk earlier. So, now we collect up to 50% of `rt_mem_limit` and save that as a disk chunk. Upon saving we look at the statistics (how much we've saved, how many new documents have arrived while saving) and recalculate the initial rate which will be used next time. For example, if we saved 90 million documents, and another 10 million docs arrived while saving, the rate is 90%, so we know that next time we can collect up to 90% of `rt_mem_limit` before starting flushing another disk chunk. The rate value is calculated automatically from 33.3% to 95%.
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) for PostgreSQL source. Thank you, [Dmitry Voronin](https://github.com/dimv36) for the [contribution](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Commit 6d54](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` and `--version`. Previously you could still see indexer's version, but `-v`/`--version` were not supported.
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) infinit mlock limit by default when Manticore is started via systemd.
* [Commit 63c8](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> op queue for coro rwlock.
* [Commit 4113](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) environment variable `MANTICORE_TRACK_RT_ERRORS` useful for debugging RT segments corruption.

### Breaking changes
* Binlog version was increased, binlog from previous version won't be replayed, so make sure you stop Manticore Search cleanly during upgrade: no binlog files should be in `/var/lib/manticore/binlog/` except `binlog.meta` after stopping the previous instance.
* [Commit 3f65](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) new column "chain" in `show threads option format=all`. It shows stack of some task info tickets, most useful for profiling needs, so if you are parsing `show threads` output be aware of the new column.
* `searchd.workers` was obsoleted since 3.5.0, now it's deprecated, if you still have it in your configuration file it will trigger a warning on start. Manticore Search will start, but with a warning.
* If you use PHP and PDO to access Manticore you need to do `PDO::ATTR_EMULATE_PREPARES`

### Bugfixes
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 slower than Manticore 3.6.3. 4.0.2 was faster than previous versions in terms of bulk inserts, but significantly slower for single document inserts. It's been fixed in 4.2.0.
* ❗[Commit 22f4](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT index could get corrupted under intensive REPLACE load, or it could crash
* [Commit 03be](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) fixed average at merging groupers and group N sorter; fixed merge of aggregates
* [Commit 2ea5](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` could crash
* [Commit 7ec7](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) RAM exhaustion issue caused by UPDATEs
* [Commit 658a](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) daemon could hang on INSERT
* [Commit 46e4](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) daemon could hang on shutdown
* [Commit f8d7](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) daemon could crash on shutdown
* [Commit 733a](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) daemon could hang on crash
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) daemon could crash on startup trying to rejoin cluster with invalid nodes list
* [Commit 1401](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) distributed index could get completely forgotten in RT mode in case it couldn't resolve one of its agents on start
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' fails
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) create table fails, but leaves dir
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Config fails with: unknown key name 'attr_update_reserve'
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore crash on batch queries
* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Batch queries causing crashes again with v4.0.3
* [Commit f7f8](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) fixed daemon crash on startup trying to re-join cluster with invalid nodes list
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 does not accept connections after batch of inserts
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET query with ORDER BY JSON.field or string attribute could crash
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Crash SIGSEGV on query with packedfactors
* [Commit 4165](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields was not supported by create table

## Version 4.0.2, Sep 21 2021

### Major new features
- **Full support of [Manticore Columnar Library](https://github.com/manticoresoftware/columnar)**. Previously Manticore Columnar Library was supported only for plain indexes. Now it's supported:
  - in real-time indexes for `INSERT`, `REPLACE`, `DELETE`, `OPTIMIZE`
  - in replication
  - in `ALTER`
  - in `indextool --check`
- **Automatic indexes compaction** ([Issue #478](https://github.com/manticoresoftware/manticoresearch/issues/478)). Finally, you don't have to call OPTIMIZE manually or via a crontask or other kind of automation. Manticore now does it for you automatically and by default. You can set default compaction threshold via [optimize_cutoff](../Server_settings/Setting_variables_online.md) global variable.
- **Chunk snapshots and locks system revamp**. These changes may be invisible from outside at first glance, but they improve the behaviour of many things happening in real-time indexes significantly. In a nutshell, previously most Manticore data manipulation operations relied on locks heavily, now we use disk chunk snapshots instead.
- **Significantly faster bulk INSERT performance into a real-time index**. For example on [Hetzner's server AX101](https://www.hetzner.com/dedicated-rootserver/ax101) with SSD, 128 GB of RAM and AMD's Ryzen™ 9 5950X (16*2 cores) **with 3.6.0 you could get 236K docs per second** inserted into a table with schema `name text, email string, description text, age int, active bit(1)` (default `rt_mem_limit`, batch size 25000, 16 concurrent insert workers, 16 million docs inserted overall). In 4.0.2 the same concurrency/batch/count gives **357K docs per second**.

  <details>

  - read operations (e.g. SELECTs, replication) are performed with snapshots
  - operations that just change internal index structure without modifying schema/documents (e.g. merging RAM segments, saving disk chunks, merging disk chunks) are performed with read-only snapshots and replace the existing chunks in the end
  - UPDATEs and DELETEs are performed against existing chunks, but for the case of merging that may be happening the writes are collected and are then applied against the new chunks
  - UPDATEs acquire an exclusive lock sequentially for every chunk. Merges acquire a shared lock when entering the stage of collecting attributes from the chunk. So at the same time only one (merge or update) operation has access to attributes of the chunk.
  - when merging gets to the phase, when it needs attributes it sets a special flag. When UPDATE finishes, it checks the flag, and if it's set, the whole update is stored in a special collection. Finally, when the merge finishes, it applies the updates set to the newborn disk chunk.
  - ALTER runs via an exclusive lock
  - replication runs as a usual read operation, but in addition saves the attributes before SST and forbids updates during the SST

  </details>

- **[ALTER](Updating_table_schema_and_settings.md) can add/remove a full-text field** (in RT mode). Previously it could only add/remove an attribute.
- 🔬 **Experimental: pseudo-sharding for full-scan queries** - allows to parallelize any non-full-text search query. Instead of preparing shards manually you can now just enable new option [searchd.pseudo_sharding](Server_settings/Searchd.md#pseudo_sharding) and expect up to `CPU cores` lower response time for non-full-text search queries. Note it can easily occupy all existing CPU cores, so if you care not only about latency, but throughput too - use it with caution.

### Minor changes
<!-- example -->
- Linux Mint and Ubuntu Hirsute Hippo are supported via [APT repository](Installation/Debian_and_Ubuntu.md#APT-repository)
- faster update by id via HTTP in big indexes in some cases (depends on the ids distribution)
- [671e65a2](https://github.com/manticoresoftware/lemmatizer-uk/commit/671e65a296800537123a3f8b324eeaa3f58b9632) - added caching to lemmatizer-uk


<!-- intro -->
#### 3.6.0

<!-- request 3.6.0 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m43.783s
user    0m0.008s
sys     0m0.007s
```

<!-- intro -->
#### 4.0.2

<!-- request 4.0.2 -->
```
time curl -X POST -d '{"update":{"index":"idx","id":4611686018427387905,"doc":{"mode":0}}}' -H "Content-Type: application/x-ndjson" http://127.0.0.1:6358/json/bulk

real    0m0.006s
user    0m0.004s
sys     0m0.001s
```
<!-- end -->
- [custom startup flags for systemd](Starting_the_server/Linux.md#Custom-startup-flags-using-systemd). Now you don't need to start searchd manually in case you need to run Manticore with some specific startup flag
- new function [LEVENSHTEIN()](Functions/String_functions.md#LEVENSHTEIN%28%29) which calculates Levenshtein distance
- added new [searchd startup flags](Starting_the_server/Manually.md#searchd-command-line-options) `--replay-flags=ignore-trx-errors` and `--replay-flags=ignore-all-errors` so one can still start searchd if the binlog is corrupted
- [Issue #621](https://github.com/manticoresoftware/manticoresearch/issues/621) - expose errors from RE2
- more accurate [COUNT(DISTINCT)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) for distributed indexes consisting of local plain indexes
- [FACET DISTINCT](Searching/Faceted_search.md#Faceting-without-duplicates) to remove duplicates when you do faceted search
- [exact form modifier](Searching/Full_text_matching/Operators.md#Exact-form-modifier) doesn't require [morphology](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) now and works for indexes with [infix/prefix](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) search enabled

### Breaking changes
- the new version can read older indexes, but the older versions can't read Manticore 4's indexes
- removed implicit sorting by id. Sort explicitly if required
- `charset_table`'s default value changes from `0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451` to `non_cjk`
- `OPTIMIZE` happens automatically. If you don't need it make sure to set `auto_optimize=0` in section `searchd` in the configuration file
- [Issue #616](https://github.com/manticoresoftware/manticoresearch/issues/616) `ondisk_attrs_default` were deprecated, now they are removed
- for contributors: we now use Clang compiler for Linux builds as according to our tests it can build a faster Manticore Search and Manticore Columnar Library
- if [max_matches](Searching/Options.md#max_matches) is not specified in a search query it gets updated implicitly with the lowest needed value for the sake of performance of the new columnar storage. It can affect metric `total` in [SHOW META](Node_info_and_management/SHOW_META.md#SHOW-META), but not `total_found` which is the actual number of found documents.

### Migration from Manticore 3
- make sure you a stop Manticore 3 cleanly:
  - no binlog files should be in `/var/lib/manticore/binlog/` (only `binlog.meta` should be in the directory)
  - otherwise the indexes Manticore 4 can't reply binlogs for won't be run
- the new version can read older indexes, but the older versions can't read Manticore 4's indexes, so make sure you make a backup if you want to be able to rollback the new version easily
- if you run a replication cluster make sure you:
  - stop all your nodes first cleanly
  - and then start the node which was stopped last with `--new-cluster` (run tool `manticore_new_cluster` in Linux).
  - read about [restarting a cluster](Creating_a_cluster/Setting_up_replication/Restarting_a_cluster.md#Restarting-a-cluster) for more details

### Bugfixes
- Lots of replication issues have been fixed:
  - [Commit 696f](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - fixed crash during SST on joiner with active index; added sha1 verify at joiner node at writing file chunks to speed up index loading; added rotation of changed index files at joiner node on index load; added removal of index files at joiner node when active index gets replaced by a new index from donor node; added replication log points at donor node for sending files and chunks
  - [Commit b296](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - crash on JOIN CLUSTER in case the address is incorrect
  - [Commit 418b](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - while initial replication of a large index the joining node could fail with `ERROR 1064 (42000): invalid GTID, (null)`, the donor could become unresponsive while another node was joining
  - [Commit 6fd3](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - hash could be calculated wrong for a big index which could result in replication failure
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - replication failed on cluster restart
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` doesn't display parameter `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd high CPU usage while idle after ca. a day
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - flush .meta immediately
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json gets emptied
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait fails under root. It also fixes systemctl behaviour (previously it was showing failure for ExecStop and didn't wait long enough for searchd to stop properly)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE vs SHOW STATUS. `command_insert`, `command_replace` and others were showing wrong metrics
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` for a plain index had a wrong default value
- [Commit 8f75](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - new disk chunks don't get mlocked
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - Manticore cluster node crashes when unable to resolve a node by name
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - replication of updated index can lead to undefined state
- [Commit ca03](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - indexer could hang on indexing a plain index source with a json attribute
- [Commit 53c7](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - fixed not equal expression filter at PQ index
- [Commit ccf9](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - fixed select windows at list queries above 1000 matches. `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` was not working previously
- [Commit a048](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS request to Manticore could cause warning like "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 could hang after a few updates of string attributes


## Version 3.6.0, May 3rd 2021
**Maintenance release before Manticore 4**

### Major new features
- Support for [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) for plain indexes. New setting [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) for plain indexes
- Support for [Ukrainian Lemmatizer](https://github.com/manticoresoftware/lemmatizer-uk)
- Fully revised histograms. When building an index Manticore also builds histograms for each field in it, which it then uses for faster filtering. In 3.6.0 the algorithm was fully revised and you can get a higher performance if you have a lot of data and do a lot of filtering.

### Minor changes
- tool `manticore_new_cluster [--force]` useful for restarting a replication cluster via systemd
- [--drop-src](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) for `indexer --merge`
- [new mode](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- added [support for escaping JSON path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) with backticks
- [indextool --check](Miscellaneous_tools.md#indextool) can work in RT mode
- [FORCE/IGNORE INDEX(id)](Data_creation_and_modification/Updating_documents/UPDATE.md#Updates-via-SQL) for SELECT/UPDATE
- chunk id for a merged disk chunk is now unique
- [indextool --check-disk-chunk CHUNK_NAME](Miscellaneous_tools.md#indextool)

### Optimizations
- [faster JSON parsing](https://github.com/manticoresoftware/manticoresearch/commit/0c25fc1788e3f77f60cb7fb8f1623d8466bba606), our tests show 3-4% lower latency on queries like `WHERE json.a = 1`
- non-documented command `DEBUG SPLIT` as a prerequisite for automatic sharding/rebalancing

### Bugfixes
- [Issue #584](https://github.com/manticoresoftware/manticoresearch/issues/524) - inaccurate and unstable FACET results
- [Issue #506](https://github.com/manticoresoftware/manticoresearch/issues/506) - Strange behavior when using MATCH: those who suffer from this issue need to rebuild the index as the problem was on the phase of building an index
- [Issue #387](https://github.com/manticoresoftware/manticoresearch/issues/387) - intermittent core dump when running query with SNIPPET() function
- Stack optimizations useful for processing complex queries:
  - [Issue #469](https://github.com/manticoresoftware/manticoresearch/issues/469) - SELECT results in CRASH DUMP
  - [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) - stack size detection for filter trees
- [Issue #461](https://github.com/manticoresoftware/manticoresearch/issues/461) - Update using the IN condition does not take effect correctly
- [Issue #464](https://github.com/manticoresoftware/manticoresearch/issues/464) - SHOW STATUS immediately after CALL PQ returns - [Issue #481](https://github.com/manticoresoftware/manticoresearch/issues/481) - Fixed static binary build
- [Issue #502](https://github.com/manticoresoftware/manticoresearch/issues/502) - bug in multi-queries
- [Issue #514](https://github.com/manticoresoftware/manticoresearch/issues/514) - Unable to use unusual names for columns when use 'create table'
- [Commit d1db](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - daemon crash on replay binlog with update of string attribute; set binlog version to 10
- [Commit 775d](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - fixed expression stack frame detection runtime (test 207)
- [Commit 4795](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - percolate index filter and tags were empty for empty stored query (test 369)
- [Commit c3f0](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - breaks of replication SST flow at network with long latency and high error rate (different data centers replication); updated replication command version to 1.03
- [Commit ba2d](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - joiner lock cluster on write operations after join into cluster (test 385)
- [Commit de4d](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - wildcards matching with exact modifier (test 321)
- [Commit 6524](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid checkpoints vs docstore
- [Commit f4ab](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - Inconsistent indexer behavior when parsing invalid xml
- [Commit 7b72](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Stored percolate query with NOTNEAR runs forever (test 349)
- [Commit 812d](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - wrong weight for phrase starting with wildcard
- [Commit 1771](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - percolate query with wildcards generate terms without payload on matching causes interleaved hits and breaks matching (test 417)
- [Commit aa0d](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - fixed calculation of 'total' in case of parallelized query
- [Commit 18d8](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - crash in Windows with multiple concurrent sessions at daemon
- [Commit 8443](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - some index settings could not be replicated
- [Commit 9341](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - On high rate of adding new events netloop sometimes freeze because of atomic 'kick' event being processed once for several events a time and loosing actual actions from them
status of the query, not the server status
- [Commit d805](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - New flushed disk chunk might be lost on commit
- [Commit 63cb](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - inaccurate 'net_read' in profiler
- [Commit f537](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Percolate issue with arabic (right to left texts)
- [Commit 49ee](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id not picked correctly on duplicate column name
- [Commit refa](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) of network events to fix a crash in rare cases
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) fix in `indextool --dumpheader`
- [Commit ff71](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE worked wrong with stored fields

### Breaking changes:
- New binlog format: you need to make a clean stop of Manticore before upgrading
- Index format slightly changes: the new version can read you existing indexes fine, but if you decide to downgrade from 3.6.0 to an older version the newer indexes will be unreadable
- Replication format change: don't replicate from an older version to 3.6.0 and vice versa, switch to the new version on all your nodes at once
- `reverse_scan` is deprecated. Make sure you don't use this option in your queries since 3.6.0 since they will fail otherwise
- As of this release we don't provide builds for RHEL6, Debian Jessie and Ubuntu Trusty any more. If it's mission critical for you to have them supported [contact us](https://manticoresearch.com/contact-us/)

### Deprecations
- No more implicit sorting by id. If you rely on it make sure to update your queries accordingly
- Search option `reverse_scan` has been deprecated

## Version 3.5.4, Dec 10 2020

### New Features
- New Python, Javascript and Java clients are generally available now and are well documented in this manual.
- automatic drop of a disk chunk of a real-time index. This optimization enables dropping a disk chunk automatically when [OPTIMIZing](Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) a real-time index when the chunk is obviously not needed any more (all the documents are suppressed). Previously it still required merging, now the chunk can be just dropped instantly. The [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) option is ignored, i.e. even if nothing is actually merged an obsoleted disk chunk gets removed. This is useful in case you maintain retention in your index and delete older documents. Now compacting such indexes will be faster.
- [standalone NOT](Searching/Options.md#not_terms_only_allowed) as an option for SELECT

### Minor Changes
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) New option [indexer.ignore_non_plain=1](Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) is useful in case you run `indexer --all` and have not only plain indexes in the configuration file. Without `ignore_non_plain=1` you'll get a warning and a respective exit code.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) and [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) enable visualization of full-text query plan execution. Useful for understanding complex queries.

### Deprecations
- `indexer --verbose` is deprecated as it never added anything to the indexer output
- For dumping watchdog's backtrace signal `USR2` is now to be used instead of `USR1`

### Bugfixes
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) cyrillic char period call snippets retain mode don't highlight
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY expression select = fatal crash
- [Commit 2b3b](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) searchd status shows Segmentation fault when in cluster
- [Commit 9dd2](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' doesn't address chunks >9
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Bug that crashes Manticore
- [Commit fba1](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Converter creates broken indexes
- [Commit eecd](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 vs CALL SNIPPETS()
- [Commit ea68](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct returns 0 at low max_matches on a local index
- [Commit 362f](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) When using aggregation stored texts are not returned in hits


## Version 3.5.2, Oct 1 2020

### New features

* OPTIMIZE reduces disk chunks to a number of chunks ( default is `2* No. of cores`) instead of a single one. The optimal number of chunks can be controlled by [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) option.
* NOT operator can be now used standalone. By default it is disabled since accidental single NOT queries can be slow. It can be enabled by setting new searchd directive [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) to `0`.
* New setting [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) sets how many threads a query can use. If the directive is not set, a query can use threads up to the value of [threads](Server_settings/Searchd.md#threads).
Per `SELECT` query the number of threads can be limited with [OPTION threads=N](Searching/Options.md#threads) overriding the global `max_threads_per_query`.
* Percolate indexes can be now be imported with [IMPORT TABLE](Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).
* HTTP API `/search` receives basic support for [faceting](Searching/Faceted_search.md#HTTP-JSON)/[grouping](Searching/Grouping.md) by new query node `aggs`.

### Minor changes

* If no replication listen directive is declared, the engine will try to use ports after the defined 'sphinx' port, up to 200.
* `listen=...:sphinx` needs to be explicit set for SphinxSE connections or SphinxAPI clients.
* [SHOW INDEX STATUS](Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) outputs new metrics: `killed_documents`, `killed_rate`, `disk_mapped_doclists`, `disk_mapped_cached_doclists`, `disk_mapped_hitlists` and `disk_mapped_cached_hitlists`.
* SQL command `status` now outputs `Queue\Threads` and `Tasks\Threads`.

### Deprecations:

* `dist_threads` is completely deprecated now, searchd will log a warning if the directive is still used.

### Docker

The official Docker image is now based on Ubuntu 20.04 LTS

### Packaging

Besides the usual `manticore` package, you can also install Manticore Search by components:

- `manticore-server-core` - provides `searchd`, manpage, log dir, API and galera module. It will also install `manticore-common` as the dependency.
- `manticore-server` - provides automation scripts for core (init.d, systemd), and `manticore_new_cluster` wrapper. It will also install `manticore-server-core` as the dependency.
- `manticore-common` - provides config, stopwords, generic docs and skeleton folders (datadir, modules, etc.)
- `manticore-tools` - provides auxiliary tools ( `indexer`, `indextool` etc.), their manpages and examples. It will also install `manticore-common` as the dependency.
- `manticore-icudata` (RPM) or `manticore-icudata-65l` (DEB) - provides ICU data file for icu morphology usage.
- `manticore-devel` (RPM) or `manticore-dev` (DEB) - provides dev headers for UDFs.

### Bugifixes

1. [Commit 2a47](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Crash of daemon at grouper at RT index with different chunks
2. [Commit 57a1](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Fastpath for empty remote docs
3. [Commit 07dd](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Expression stack frame detection runtime
4. [Commit 08ae](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Matching above 32 fields at percolate indexes
5. [Commit 16b9](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Replication listen ports range
6. [Commit 5fa6](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Show create table on pq
7. [Commit 54d1](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS port behavior
8. [Commit fdbb](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Mixing docstore rows when replacing
9. [Commit afb5](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Switch TFO unavailable message level to 'info'
10. [Commit 59d9](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Crash on strcmp invalid use
11. [Commit 04af](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Adding index to cluster with system (stopwords) files
12. [Commit 5014](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Merge indexes with large dictionaries; RT optimize of large disk chunks
13. [Commit a2ad](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool can dump meta from current version
14. [Commit 69f6](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Issue in group order in GROUP N
15. [Commit 24d5](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Explicit flush for SphinxSE after handshake
16. [Commit 31c4](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Avoid copy of huge descriptions when not necessary
17. [Commit 2959](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Negative time in show threads
18. [Commit f0b3](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Token filter plugin vs zero position deltas
19. [Commit a49e](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Change 'FAIL' to 'WARNING' on multiple hits

## Version 3.5.0, 22 Jul 2020

### Major new features:
* This release took so long, because we were working hard on changing multitasking mode from threads to **coroutines**. It makes configuration simpler and queries parallelization much more straightforward: Manticore just uses given number of threads (see new setting [threads](Server_settings/Searchd.md#threads)) and the new mode makes sure it's done in the most optimal way.
* Changes in [highlighting](Searching/Highlighting.md#Highlighting-options):
  - any highlighting that works with several fields (`highlight({},'field1, field2'`) or `highlight` in json queries) now applies limits per-field by default.
  - any highlighting that works with plain text (`highlight({}, string_attr)` or `snippet()` now applies limits to the whole document.
  - [per-field limits](Searching/Highlighting.md#limits_per_field) can be switched to global limits by `limits_per_field=0` option (`1` by default).
  - [allow_empty](Searching/Highlighting.md#allow_empty) is now `0` by default for highlighting via HTTP JSON.

* The same port [can now be used](Server_settings/Searchd.md#listen) for http, https and binary API (to accept connections from a remote Manticore instance). `listen = *:mysql` is still required for connections via mysql protocol. Manticore now detects automatically the type of client trying to connect to it except for MySQL (due to restrictions of the protocol).

* In RT mode a field can now be [text and string attribute](Creating_a_table/Data_types.md#String) at the same time - [GitHub issue #331](https://github.com/manticoresoftware/manticoresearch/issues/331).

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
* Support of mysql client [`status`](Node_info_and_management/Node_status.md#STATUS) command.
* [Replication](Creating_a_cluster/Setting_up_replication/Setting_up_replication.md#Setting-up-replication) can now replicate external files (stopwords, exceptions etc.).
* Filter operator [`in`](Searching/Filters.md#Set-filters) is now available via HTTP JSON interface.
* [`expressions`](Searching/Expressions.md#expressions) in HTTP JSON.
* [You can now change `rt_mem_limit` on the fly](https://github.com/manticoresoftware/manticoresearch/issues/344) in RT mode, i.e. can do `ALTER ... rt_mem_limit=<new value>`.
* You can now use [separate CJK charset tables](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology): `chinese`, `japanese` and `korean`.
* [thread_stack](Server_settings/Searchd.md#thread_stack) now limits maximum thread stack, not initial.
* Improved `SHOW THREADS` output.
* Display progress of long `CALL PQ` in `SHOW THREADS`.
* cpustat, iostat, coredump can be changed during runtime with [SET](Server_settings/Setting_variables_online.md#SET).
* `SET [GLOBAL] wait_timeout=NUM` implemented ,

### Breaking changes:
* **Index format has been changed.** Indexes built in 3.5.0 cannot be loaded by Manticore version < 3.5.0, but Manticore 3.5.0 understands older formats.
* [`INSERT INTO PQ VALUES()`](Data_creation_and_modification/Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (i.e. without providing column list) previously expected exactly `(query, tags)` as the values. It's been changed to `(id,query,tags,filters)`. The id can be set to 0 if you want it to be auto-generated.
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
1. [Issue #351](https://github.com/manticoresoftware/manticoresearch/issues/351) searchd memory leak
2. [Commit ceab](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Tiny read out of bounds in snippets
3. [Commit 1c3e](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Dangerous write into local variable for crash queries
4. [Commit 26e0](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Tiny memory leak of sorter in test 226
5. [Commit d2c7](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Huge memory leak in test 226
6. [Commit 0dd8](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Cluster shows the nodes are in sync, but `count(*)` shows different numbers
7. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Cosmetic: Duplicate and sometimes lost warning messages in the log
8. [Commit f1c1](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Cosmetic: (null) index name in log
9. [Commit 359d](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Cannot retrieve more than 70M results
10. [Commit 19f3](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Can't insert PQ rules with no-columns syntax
11. [Commit bf68](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Misleading error message when inserting a document to an index in a cluster
12. [Commit 2cf1](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` and `json/update` return id in exponent form
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Update json scalar properties and mva in the same query
14. [Commit d384](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` doesn't work in RT mode
15. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `ALTER RECONFIGURE` in rt mode should be disallowed
16. [Commit 5813](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` gets reset to 128M after searchd restart
17. highlight() sometimes hangs
18. [Commit 7cd8](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Failed to use U+code in RT mode
19. [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Failed to use wildcard at wordforms at RT mode
20. [Commit e9d0](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Fixed `SHOW CREATE TABLE` vs multiple wordform files
21. [Commit fc90](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON query without "query" crashes searchd
22. Manticore [official docker](https://hub.docker.com/r/manticoresearch/manticore) couldn't index from mysql 8
23. [Commit 23e0](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert requires id
24. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` doesn't work for PQ
25. [Commit bd67](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` doesn't work properly for PQ
26. [Commit 5eac](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) End of line in settings in show index status
27. [Commit cb15](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Empty title in "highlight" in HTTP JSON response
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` infix error
29. [Commit 9040](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT crashes under load
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Lost crash log on crash at RT disk chunk
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Import table fails and closes the connection
32. [Commit 6275](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` corrupts a PQ index
33. [Commit 9c1d](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Searchd reload issues after change index type
34. [Commit 71e2](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Daemon crashes on import table with missed files
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Crash on select using multiple indexes, group by and ranker = none
36. [Commit c3f5](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` doesn't higlight in string attributes
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` fails to sort on string attribute
38. [Commit 4f1a](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Error in case of missing data dir
39. [Commit 04f4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* are not supported in RT mode
40. [Commit 1c06](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Bad JSON objects in strings: 1. `CALL PQ` returns "Bad JSON objects in strings: 1" when the json is greater than some value.
41. [Commit 32f9](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT-mode inconsistency. In some cases I can't drop the index since it's unknown and can't create it since the directory is not empty.
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Crash on select
43. [Commit 22a2](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M returned warning on 2M field
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Query conditions execution bug
45. [Commit dd8d](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Simple 2 terms search finds a document containing only one term
46. [Commit 9091](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) It was impossible in PQ to match a json with capital letters in keys
47. [Commit 56da](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Indexer crashes on csv+docstore
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) using `[null]` in json attr in centos 7 causes corrupted inserted data
49. Major [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Records not being inserted, count() is random, "replace into" returns OK
50. max_query_time slows down SELECTs too much
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Master-agent communication fails on Mac OS
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Error when connecting to Manticore with Connector.Net/Mysql 8.0.19
53. [Commit daa7](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Fixed escaping of \0 and optimized performance
54. [Commit 9bc5](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Fixed count distinct vs json
55. [Commit 4f89](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Fixed drop table at other node failed
56. [Commit 952a](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Fix crashes on tightly running call pq


## Version 3.4.2, 10 April 2020
### Critical bugfixes
* [Commit 2ffe](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) fix RT index from old version fails to index data

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
* [Commit a533](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) fix crash on wrong field syntax in highlight()
* [Commit 7fbb](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) fix crash of server on replicate RT index with docstore
* [Commit 24a0](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) fix crash on highlight to index with infix or prefix option and to index wo stored fields enabled
* [Commit 3465](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) fix false error about empty docstore and dock-id lookup for empty index
* [Commit a707](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) fix #314 SQL insert command with trailing semicolon
* [Commit 9562](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) removed warning on query word(s) mismatch
* [Commit b860](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) fix queries in snippets segmented via ICU
* [Commit 5275](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) fix find/add race condition in docstore block cache
* [Commit f06e](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) fix mem leak in docstore
* [Commit a725](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) fix #316 LAST_INSERT_ID returns empty on INSERT
* [Commit 1ebd](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) fix #317 json/update HTTP endpoint to support array for MVA and object for JSON attribute
* [Commit e426](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) fix rash of indexer dumping rt without explicit id

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
* [Commit 9c33](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) added check of index schema for duplicate attributes #293
* [Commit a008](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) fix crash in hitless terms
* [Commit 6895](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) fix loose docstore after ATTACH
* [Commit d6f6](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) fix docstore issue in distributed setup
* [Commit bce2](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) replace FixedHash with OpenHash in sorter
* [Commit e0ba](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) fix attributes with duplicated names at index definition
* [Commit ca81](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) fix html_strip in HIGHLIGHT()
* [Commit 493a](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) fix passage macro in HIGHLIGHT()
* [Commit a82d](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) fix double buffer issues when RT index creates small or large disk chunk
* [Commit a404](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) fix event deletion for kqueue
* [Commit 8bea](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) fix save of disk chunk for large value of rt_mem_limit of RT index
* [Commit 8707](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) fix float overflow on indexing
* [Commit a564](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) fix insert document with negative ID into RT index fails with error now
* [Commit bbeb](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) fix crash of server on ranker fieldmask
* [Commit 3809](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) fix crash on using query cache
* [Commit dc2a](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) fix crash on using RT index RAM segments with parallel inserts

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
* [Commit 6ae4](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) fix crash on SELECT query over HTTP interface
* [Commit 5957](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) fix RT index saves disk chunks but does not mark some documents deleted
* [Commit e861](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) fix crash on search of multi index or multi queries with dist_threads
* [Commit 4409](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) fix crash on infix generation for long terms with wide utf8 codepoints
* [Commit 5fd5](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) fix race at adding socket to IOCP
* [Commit cf10](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) fix issue of bool queries vs json select list
* [Commit 996d](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) fix indextool check to report wrong skiplist offset, check of doc2row lookup
* [Commit 6e3f](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) fix indexer produces bad index with negative skiplist offset on large data
* [Commit faed](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) fix JSON converts only numeric to string and JSON string to numeric conversion at expressions
* [Commit 5331](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) fix indextool exit with error code in case multiple commands set at command line
* [Commit 7955](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) fix #275 binlog invalid state on error no space left on disk
* [Commit 2284](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) fix #279 crash on IN filter to JSON attribute
* [Commit ce2e](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) fix #281 wrong pipe closing call
* [Commit 5355](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) fix server hung at CALL PQ with recursive JSON attribute encoded as string
* [Commit a5fc](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) fix advancing beyond the end of the doclist in multiand node
* [Commit a362](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) fix retrieving of thread public info
* [Commit f8d2](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) fix docstore cache locks

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
* [Commit 301a](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) fix crash on replace document in disk chunk of RT index
* [Commit 46c1](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) fix \#269 LIMIT N OFFSET M
* [Commit 92a4](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) fix DELETE statements with id explicitly set or id list provided to skip search
* [Commit 8ca7](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) fix wrong index after event removed at netloop at windowspoll poller
* [Commit 6036](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) fix float roundup at JSON via HTTP
* [Commit 62f6](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) fix remote snippets to check empty path first; fixing windows tests
* [Commit aba2](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) fix reload of config to work on windows same way as on linux
* [Commit 6b8c](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) fix \#194 PQ to work with morphology and stemmers
* [Commit 174d](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) fix RT retired segments management

## Version 3.1.2, 22 August 2019
### Features and Improvements
* Experimental SSL support for HTTP API
* field filter for CALL KEYWORDS
* max_matches for /json/search
* automatic sizing of default Galera gcache.size
* improved FreeBSD support

### Bugfixes
* [Commit 0a1a](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) fixed replication of RT index into node where same RT index exists and has different path
* [Commit 4adc](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) fix flush rescheduling for indexes without activity
* [Commit d6c0](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) improve rescheduling of flushing RT/PQ indexes
* [Commit d0a7](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) fix \#250 index_field_lengths index option for TSV and CSV piped sources
* [Commit 1266](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) fix indextool wrong report for block index check on empty index
* [Commit 553c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) fix empty select list at Manticore SQL query log
* [Commit 56c8](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) fix indexer -h/--help response

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
* [Commit f047](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) fix defects in concat and group_concat
* [Commit b081](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) fix query uid at percolate index to be BIGINT attribute type
* [Commit 4cd8](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) do not crash if failed to prealloc a new disk chunk
* [Commit 1a55](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) add missing timestamp data type to ALTER
* [Commit f3a8](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) fix crash of wrong mmap read
* [Commit 4475](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) fix hash of clusters lock in replication
* [Commit ff47](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) fix leak of providers in replication
* [Commit 58dc](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) fix \#246 undefined sigmask in indexer
* [Commit 3dd8](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) fix race in netloop reporting
* [Commit a02a](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) zero gap for HA strategies rebalancer

## Version 3.0.2, 31 May 2019
### Improvements
* added mmap readers for docs and hit lists
* `/sql` HTTP endpoint response is now the same as `/json/search` response
* new directives `access_plain_attrs`, `access_blob_attrs`, `access_doclists`, `access_hitlists`
* new directive `server_id` for replication setups

### Removals
* removed HTTP `/search` endpoint

### Deprecations
* `read_buffer`, `ondisk_attrs`, `ondisk_attrs_default`, `mlock` are replaced by `access_*` directives

### Bugfixes
* [Commit 849c](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) allow attribute names starting with numbers in select list
* [Commit 48e6](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) fixed MVAs in UDFs, fixed MVA aliasing
* [Commit 0555](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) fixed \#187 crash when using query with SENTENCE
* [Commit 93bf](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) fixed \#143 support () around MATCH()
* [Commit 599e](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) fixed save of cluster state on ALTER cluster statement
* [Commit 230c](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) fixed crash of server on ALTER index with blob attributes
* [Commit 5802](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) fixed \#196 filtering by id
* [Commit 25d2](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) discard searching on template indexes
* [Commit 2a30](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) fixed id column to have regular bigint type at SQL reply


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
* [Commit 6967](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) fixed crash on many stars at select list for query into many distributed indexes
* [Commit 36df](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) fixed [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) large packet via Manticore SQL interface
* [Commit 5793](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) fixed [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) crash of server on RT optimize with MVA updated
* [Commit edb2](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) fixed server crash on binlog removed due to RT index remove after config reload on SIGHUP
* [Commit bd3e](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) fixed mysql handshake auth plugin payloads
* [Commit 6a21](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) fixed [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) phrase_boundary settings at RT index
* [Commit 3562](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) fixed [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) deadlock at ATTACH index to itself
* [Commit 250b](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) fixed binlog saves empty meta after server crash
* [Commit 4aa6](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) fixed crash of server due to string at sorter from RT index with disk chunks

## Version 2.8.1 GA, 6 March 2019
### Features and improvements
* SUBSTRING_INDEX()
* SENTENCE and PARAGRAPH support for percolate queries
* systemd generator for Debian/Ubuntu; also added LimitCORE to allow core dumping

### Bugfixes
* [Commit 84fe](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) fixed crash of server on match mode all and empty full text query
* [Commit daa8](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) fixed crash on deleting of static string
* [Commit 2207](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) fixed exit code when indextool failed with FATAL
* [Commit 0721](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) fixed [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) no matches for prefixes due to wrong exact form check
* [Commit 8af8](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) fixed [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) reload of config settings for RT indexes
* [Commit e2d5](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) fixed crash of server on access of large JSON string
* [Commit 75cd](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) fixed PQ field at JSON document altered by index stripper causes wrong match from sibling field
* [Commit e2f7](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) fixed crash of server at parse JSON on RHEL7 builds
* [Commit 3a25](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) fixed crash of json unescaping when slash is on the edge
* [Commit be9f](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) fixed option 'skip_empty' to skip empty docs and not warn they're not valid json
* [Commit 266e](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) fixed [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) output 8 digits on floats when 6 is not enough to be precise
* [Commit 3f6d](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) fixed empty jsonobj creation
* [Commit f3c7](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) fixed [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) empty mva outputs NULL instead of an empty string
* [Commit 0afa](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) fixed fail to build without pthread_getname_np
* [Commit 9405](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) fixed crash on server shutdown with thread_pool workers

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
* [Commit a4e1](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) fixed csjon-related leak
* [Commit 28d8](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) fixed crash because of missed value in json
* [Commit bf4e](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) fixed save of empty meta for RT index
* [Commit 33b4](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) fixed lost form flag (exact) for sequence of lemmatizer
* [Commit 6b95](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) fixed string attrs > 4M use saturate instead of overflow
* [Commit 6214](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) fixed crash of server on SIGHUP with disabled index
* [Commit 3f7e](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) fixed server crash on simultaneous API session status commands
* [Commit cd9e](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) fixed crash of server at delete query to RT index with field filters
* [Commit 9376](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) fixed crash of server at CALL PQ to distributed index with empty document
* [Commit 8868](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) fixed cut Manticore SQL error message larger 512 chars
* [Commit de9d](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) fixed crash on save percolate index without binlog
* [Commit 2b21](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) fixed http interface is not working in OSX
* [Commit e92c](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) fixed indextool false error message on check of MVA
* [Commit 238b](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) fixed write lock at FLUSH RTINDEX to not write lock whole index during save and on regular flush from rt_flush_period
* [Commit c26a](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) fixed ALTER percolate index stuck waiting search load
* [Commit 9ee5](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) fixed max_children to use default amount of thread_pool workers for value of 0
* [Commit 5138](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) fixed error on indexing of data into index with index_token_filter plugin along with stopwords and stopword_step=0
* [Commit 2add](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) fixed crash with absent lemmatizer_base when still using aot lemmatizers in index definitions

## Version 2.7.5 GA, 4 December 2018
### Improvements
* REGEX function
* limit/offset for json API search
* profiler points for qcache

### Bugfixes
* [Commit eb3c](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) fixed crash of server on FACET with multiple attribute wide types
* [Commit d915](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) fixed implicit group by at main select list of FACET query
* [Commit 5c25](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) fixed crash on query with GROUP N BY
* [Commit 85d3](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) fixed deadlock on handling crash at memory operations
* [Commit 8516](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) fixed indextool memory consumption during check
* [Commit 58fb](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) fixed gmock include not needed anymore as upstream resolve itself

## Version 2.7.4 GA, 1 November 2018
### Improvements
* SHOW THREADS in case of remote distributed indexes prints the original query instead of API call
* SHOW THREADS new option `format=sphinxql` prints all queries in SQL format
* SHOW PROFILE prints additional `clone_attrs` stage

### Bugfixes
* [Commit 4f15](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) fixed failed to build with libc without malloc_stats, malloc_trim
* [Commit f974](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) fixed special symbols inside words for CALL KEYWORDS result set
* [Commit 0920](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) fixed broken CALL KEYWORDS to distributed index via API or to remote agent
* [Commit fd68](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) fixed distributed index agent_query_timeout propagate to agents as max_query_time
* [Commit 4ffa](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) fixed total documents counter at disk chunk got affected by OPTIMIZE command and breaks weight calculation
* [Commit dcaf](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) fixed multiple tail hits at RT index from blended
* [Commit eee3](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) fixed deadlock at rotation

## Version 2.7.3 GA, 26 September 2018
### Improvements
* sort_mode option for CALL KEYWORDS
* DEBUG on VIP connection can perform 'crash <password>' for intentional SIGEGV action on server
* DEBUG can perform 'malloc_stats' for dumping malloc stats in searchd.log 'malloc_trim' to perform a malloc_trim()
* improved backtrace is gdb is present on the system

### Bugfixes
* [Commit 0f3c](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) fixed crash or hfailure of rename on Windows
* [Commit 1455](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) fixed crashes of server on 32-bit systems
* [Commit ad37](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) fixed crash or hung of server on empty SNIPPET expression
* [Commit b36d](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) fixed broken non progressive optimize and fixed progressive optimize to not create kill-list for oldest disk chunk
* [Commit 34b0](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) fixed queue_max_length bad reply for SQL and API at thread pool worker mode
* [Commit ae4b](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) fixed crash on adding full-scan query to PQ index with regexp or rlp options set
* [Commit f80f](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) fixed crash when call one PQ after another
* [Commit 9742](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) refactor AcquireAccum
* [Commit 39e5](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) fixed leak of memory after call pq
* [Commit 21bc](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) cosmetic refactor (c++11 style c-trs, defaults, nullptrs)
* [Commit 2d69](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) fixed memory leak on trying to insert duplicate into PQ index
* [Commit 5ed9](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) fixed crash on JSON field IN with large values
* [Commit 4a52](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) fixed crash of server on CALL KEYWORDS statement to RT index with expansion limit set
* [Commit 5526](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) fixed invalid filter at PQ matches query;
* [Commit 204f](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) introduce small obj allocator for ptr attrs
* [Commit 2545](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) refactor ISphFieldFilter to refcounted flavour
* [Commit 1366](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) fixed ub/sigsegv when using strtod on non-terminated strings
* [Commit 94bc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) fixed memory leak in json resultset processing
* [Commit e78e](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) fixed read over the end of mem block applying attribute add
* [Commit fad5](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) fixed refactor CSphDict for refcount flavour
* [Commit fd84](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) fixed leak of AOT internal type outside
* [Commit 5ee7](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) fixed memory leak tokenizer management
* [Commit 116c](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) fixed memory leak in grouper
* [Commit 56fd](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) special free/copy for dynamic ptrs in matches (memory leak grouper)
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) fixed memory leak of dynamic strings for RT
* [Commit 517b](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) refactor grouper
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) minor refactor (c++11 c-trs, some reformats)
* [Commit 7034](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) refactor ISphMatchComparator to refcounted flavour
* [Commit b1fc](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) privatize cloner
* [Commit efbc](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) simplify native little-endian for MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Commit 6da0](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) add valgrind support to to ubertests
* [Commit 1d17](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) fixed crash because race of 'success' flag on connection
* [Commit 5a09](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) switch epoll to edge-triggered flavour
* [Commit 5d52](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) fixed IN statement in expression with formatting like at filter
* [Commit bd8b](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) fixed crash at RT index on commit of document with large docid
* [Commit ce65](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) fixed argless options in indextool
* [Commit 08c9](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) fixed memory leak of expanded keyword
* [Commit 30c7](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) fixed memory leak of json grouper
* [Commit 6023](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) fixed leak of global user vars
* [Commit 7c13](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) fixed leakage of dynamic strings on early rejected matches
* [Commit 9154](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) fixed leakage on length(<expression>)
* [Commit 43fc](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) fixed memory leak because strdup() in parser
* [Commit 71ff](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) fixed refactor expression parser to accurate follow refcounts

## Version 2.7.2 GA, 27 August 2018
### Improvements
* compatibility with MySQL 8 clients
* [TRUNCATE](Emptying_a_table.md) WITH RECONFIGURE
* retired memory counter on SHOW STATUS for RT indexes
* global cache of multi agents
* improved IOCP on Windows
* VIP connections for HTTP protocol
* Manticore SQL [DEBUG](Reporting_bugs.md#DEBUG) command which can run various subcommands
* [shutdown_token](Server_settings/Searchd.md#shutdown_token) - SHA1 hash of password needed to invoke `shutdown` using DEBUG command
* new stats to SHOW AGENT STATUS (_ping, _has_perspool, _need_resolve)
* --verbose option of indexer now accept \[debugvv\] for printing debug messages

### Bugfixes
* [Commit 3900](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) removed wlock at optimize
* [Commit 4c33](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) fixed wlock at reload index settings
* [Commit b5ea](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) fixed memory leak on query with JSON filter
* [Commit 930e](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) fixed empty documents at PQ result set
* [Commit 53de](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) fixed confusion of tasks due to removed one
* [Commit cad9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) fixed wrong remote host counting
* [Commit 9000](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) fixed memory leak of parsed agent descriptors
* [Commit 978d](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) fixed leak in search
* [Commit 0193](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) cosmetic changes on explicit/inline c-trs, override/final usage
* [Commit 943e](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) fixed leak of json in local/remote schema
* [Commit 02db](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) fixed leak of json sorting col expr in local/remote schema
* [Commit c74d](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) fixed leak of const alias
* [Commit 6e5b](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) fixed leak of preread thread
* [Commit 39c7](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) fixed stuck on exit because of stucked wait in netloop
* [Commit adaf](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) fixed stuck of 'ping' behaviour on change HA agent to usual host
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) separate gc for dashboard storage
* [Commit 511a](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) fixed ref-counted ptr fix
* [Commit 32c4](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) fixed indextool crash on unexistent index
* [Commit 156e](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) fixed output name of exceeding attr/field in xmlpipe indexing
* [Commit cdac](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) fixed default indexer's value if no indexer section in config
* [Commit e61e](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) fixed wrong embedded stopwords in disk chunk by RT index after server restart
* [Commit 5fba](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) fixed skip phantom (already closed, but not finally deleted from the poller) connections
* [Commit f22a](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) fixed blended (orphaned) network tasks
* [Commit 4689](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) fixed crash on read action after write
* [Commit 03f9](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) fixed searchd crashes when running tests on windows
* [Commit e925](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) fixed handle EINPROGRESS code on usual connect()
* [Commit 248b](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) fixed connection timeouts when working with TFO

## Version 2.7.1 GA, 4 July 2018
### Improvements
* improved wildcards performance on matching multiple documents at PQ
* support for fullscan queries at PQ
* support for MVA attributes at PQ
* regexp and RLP support for percolate indexes

### Bugfixes
* [Commit 6885](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) fixed loose of query string
* [Commit 0f17](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) fixed empty info at SHOW THREADS statement
* [Commit 53fa](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) fixed crash on matching with NOTNEAR operator
* [Commit 2602](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) fixed error message on bad filter to PQ delete


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
* [SHOW STATUS](Node_info_and_management/Node_status.md#SHOW-STATUS) now outputs to server version and mysql_version_string
* added `docs_id` option for documents called in CALL PQ.
* percolate queries filter can now contain expressions
* distributed indexes can work with FEDERATED
* dummy SHOW NAMES COLLATE and `SET wait_timeout` (for better ProxySQL compatibility)

### Bugfixes
* [Commit 5bcf](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) fixed added not equal to tags of PQ
* [Commit 9ebc](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) fixed added document id field to JSON document CALL PQ statement
* [Commit 8ae0](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) fixed flush statement handlers to PQ index
* [Commit c24b](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) fixed PQ filtering on JSON and string attributes
* [Commit 1b8b](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) fixed parsing of empty JSON string
* [Commit 1ad8](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) fixed crash at multi-query with OR filters
* [Commit 69b8](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) fixed indextool to use config common section (lemmatizer_base option) for commands (dumpheader)
* [Commit 6dbe](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) fixed empty string at result set and filter
* [Commit 39c4](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) fixed negative document id values
* [Commit 266b](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) fixed word clip length for very long words indexed
* [Commit 4782](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) fixed matching multiple documents of wildcard queries at PQ


## Version 2.6.4 GA, 3 May 2018
### Features and improvements
* MySQL FEDERATED engine [support](Extensions/FEDERATED.md)
* MySQL packets return now SERVER_STATUS_AUTOCOMMIT flag, adds compatibility with ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - enable TCP Fast Open connections for all listeners
* indexer --dumpheader can dump also RT header from .meta file
* cmake build script for Ubuntu Bionic

### Bugfixes
* [Commit 355b](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) fixed invalid query cache entries for RT index;
* [Commit 546e](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) fixed index settings got lost next after seamless rotation
* [Commit 0c45](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) fixed fixed infix vs prefix length set; added warning on unsupportedinfix length
* [Commit 8054](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) fixed RT indexes auto-flush order
* [Commit 705d](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) fixed result set schema issues for index with multiple attributes and queries to multiple indexes
* [Commit b0ba](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) fixed some hits got lost at batch insert with document duplicates
* [Commit 4510](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) fixed optimize failed to merge disk chunks of RT index with large documents count

## Version 2.6.3 GA, 28 March 2018
### Improvements
* jemalloc at compilation. If jemalloc is present on system, it can be enabled with cmake flag `-DUSE_JEMALLOC=1`

### Bugfixes
* [Commit 85a6](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) fixed log expand_keywords option into Manticore SQL query log
* [Commit caaa](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) fixed HTTP interface to correctly process query with large size
* [Commit e386](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) fixed crash of server on DELETE to RT index with index_field_lengths enable
* [Commit cd53](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) fixed cpustats searchd cli option to work with unsupported systems
* [Commit 8740](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) fixed utf8 substring matching with min lengths defined


## Version 2.6.2 GA, 23 February 2018
### Improvements
* improved [Percolate Queries](Searching/Percolate_query.md) performance in case of using NOT operator and for batched documents.
* [percolate_query_call](Searching/Percolate_query.md) can use multiple threads depending on [dist_threads](Server_settings/Searchd.md#threads)
* new full-text matching operator NOTNEAR/N
* LIMIT for SELECT on percolate indexes
* [expand_keywords](Searching/Options.md#expand_keywords) can accept 'start','exact' (where 'star,exact' has same effect as '1')
* ranged-main-query for  [joined fields](Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) which uses the ranged query defined by sql_query_range

### Bugfixes
* [Commit 72dc](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) fixed crash on searching ram segments; deadlock on save disk chunk with double buffer; deadlock on save disk chunk during optimize
* [Commit 3613](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) fixed indexer crash on xml embedded schema with empty attribute name
* [Commit 48d7](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) fixed erroneous unlinking of not-owned pid-file
* [Commit a556](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) fixed orphaned fifos sometimes left in temp folder
* [Commit 2376](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) fixed empty FACET result set with wrong NULL row
* [Commit 4842](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) fixed broken index lock when running server as windows service
* [Commit be35](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) fixed wrong iconv libs on mac os
* [Commit 8374](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) fixed wrong count(\*)


## Version 2.6.1 GA, 26 January 2018
### Improvements
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) in case of agents with mirrors gives the value of retries per mirror instead of per agent, the total retries per agent being agent_retry_count\*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) can now be specified per index, overriding global value. An alias [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count) is added.
* a retry_count can be specified in agent definition and the value represents retries per agent
* Percolate Queries are now in HTTP JSON API at [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Added -h and -v options (help and version) to executables
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) support for Real-Time indexes

### Bugfixes
* [Commit a40b](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) fixed ranged-main-query to correctly work with sql_range_step when used at MVA field
* [Commit f2f5](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) fixed issue with blackhole system loop hung and blackhole agents seems disconnected
* [Commit 84e1](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) fixed query id to be consistent, fixed duplicated id for stored queries
* [Commit 1948](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) fixed server crash on shutdown from various states
* [Commit 9a70](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) timeouts on long queries
* [Commit 3359](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) refactored master-agent network polling on kqueue-based systems (Mac OS X, BSD).


## Version 2.6.0, 29 December 2017
### Features and improvements
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON queries can now do equality on attributes, MVA and JSON attributes can be used in inserts and updates, updates and deletes via JSON API can be performed on distributed indexes
* [Percolate Queries](Searching/Percolate_query.md)
* Removed support for 32-bit docids from the code. Also removed all the code that converts/loads legacy indexes with 32-bit docids.
* [Morphology only for certain fields](https://github.com/manticoresoftware/manticore/issues/7) . A new index directive morphology_skip_fields allows defining a list of fields for which morphology does not apply.
* [expand_keywords can now be a query runtime directive set using the OPTION statement](https://github.com/manticoresoftware/manticore/issues/8)

### Bugfixes
* [Commit 0cfa](https://github.com/manticoresoftware/manticore/commit/0cfae4c) fixed crash on debug build of server (and m.b. UB on release) when built with rlp
* [Commit 3242](https://github.com/manticoresoftware/manticore/commit/324291e) fixed RT index optimize with progressive option enabled that merges kill-lists with wrong order
* [Commit ac0e](https://github.com/manticoresoftware/manticore/commit/ac0efee)  minor crash on mac
* lots of minor fixes after thorough static code analysis
* other minor bugfixes

### Upgrade
In this release we've changed internal protocol used by masters and agents to speak with each other. In case you run Manticoresearch in a distributed environment with multiple instances make sure your first upgrade agents, then the masters.

## Version 2.5.1, 23 November 2017
### Features and improvements
* JSON queries on [HTTP API protocol](Connecting_to_the_server/HTTP.md). Supported search, insert, update, delete, replace operations. Data manipulation commands can be also bulked, also there are some limitations currently as MVA and JSON attributes can't be used for inserts, replaces or updates.
* [RELOAD INDEXES](Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) command
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) command
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) can show progress of optimize, rotation or flushes.
* GROUP N BY work correctly with MVA attributes
* blackhole agents are run on separate thread to not affect master query anymore
* implemented reference count on indexes, to avoid stalls caused by rotations and high load
* SHA1 hashing implemented, not exposed yet externally
* fixes for compiling on FreeBSD, macOS and Alpine

### Bugfixes
* [Commit 9897](https://github.com/manticoresoftware/manticore/commit/989752b) filter regression with block index
* [Commit b1c3](https://github.com/manticoresoftware/manticore/commit/b1c3864) rename PAGE_SIZE -> ARENA_PAGE_SIZE for compatibility with musl
* [Commit f213](https://github.com/manticoresoftware/manticore/commit/f2133cc) disable googletests for cmake < 3.1.0
* [Commit f30e](https://github.com/manticoresoftware/manticore/commit/0839de7) failed to bind socket on server restart
* [Commit 0807](https://github.com/manticoresoftware/manticore/commit/0807240) fixed crash of server on shutdown
* [Commit 3e3a](https://github.com/manticoresoftware/manticore/commit/3e3acc3) fixed show threads for system blackhole thread
* [Commit 262c](https://github.com/manticoresoftware/manticore/commit/262c3fe) Refactored config check of iconv, fixes building on FreeBSD and Darwin

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
* [SHOW META](Node_info_and_management/SHOW_META.md) shows a `multiplier` row when multi-query optimization is used

### Compiling
Manticore Search is built using cmake and the minimum gcc version required for compiling is 4.7.2.

### Folders and service
* Manticore Search runs under `manticore` user.
* Default data folder is now `/var/lib/manticore/`.
* Default log folder is now `/var/log/manticore/`.
* Default pid folder is now `/var/run/manticore/`.

### Bugfixes
* [Commit a58c](https://github.com/manticoresoftware/manticore/commit/a58c619) fixed SHOW COLLATION statement that breaks java connector
* [Commit 631c](https://github.com/manticoresoftware/manticore/commit/631cf4e) fixed crashes on processing distributed indexes; added locks to distributed index hash; removed move and copy operators from agent
* [Commit 942b](https://github.com/manticoresoftware/manticore/commit/942bec0) fixed crashes on processing distributed indexes due to parallel reconnects
* [Commit e5c1](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) fixed crash at crash handler on store query to server log
* [Commit 4a4b](https://github.com/manticoresoftware/manticore/commit/4a4bda5) fixed a crash with pooled attributes in multiqueries
* [Commit 3873](https://github.com/manticoresoftware/manticore/commit/3873bfb) fixed reduced core size by prevent index pages got included into core file
* [Commit 11e6](https://github.com/manticoresoftware/manticore/commit/11e6254) fixed searchd crashes on startup when invalid agents are specified
* [Commit 4ca6](https://github.com/manticoresoftware/manticore/commit/4ca6350) fixed indexer reports error in sql_query_killlist query
* [Commit 123a](https://github.com/manticoresoftware/manticore/commit/123a9f0) fixed fold_lemmas=1 vs hit count
* [Commit cb99](https://github.com/manticoresoftware/manticore/commit/cb99164) fixed inconsistent behavior of html_strip
* [Commit e406](https://github.com/manticoresoftware/manticore/commit/e406761) fixed optimize rt index loose new settings; fixed optimize with sync option lock leaks;
* [Commit 86ae](https://github.com/manticoresoftware/manticore/commit/86aeb82) fixed processing erroneous multiqueries
* [Commit 2645](https://github.com/manticoresoftware/manticore/commit/2645230) fixed result set depends on multi-query order
* [Commit 7239](https://github.com/manticoresoftware/manticore/commit/72395d9) fixed server crash on multi-query with bad query
* [Commit f353](https://github.com/manticoresoftware/manticore/commit/f353326) fixed shared to exclusive lock
* [Commit 3754](https://github.com/manticoresoftware/manticore/commit/3754785) fixed server crash for query without indexes
* [Commit 29f3](https://github.com/manticoresoftware/manticore/commit/29f360e) fixed dead lock of server

## Version 2.3.3, 06 July 2017
* Manticore branding
