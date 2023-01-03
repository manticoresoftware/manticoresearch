# Changelog

# Next release

Starting with this release, Manticore Search comes with Manticore Buddy, a sidecar daemon written in PHP that handles high-level functionality that does not require low latency or high throughput. Manticore Buddy operates behind the scenes, and you may not even realize it is running. Although it is invisible to the end user, it was a significant challenge to make Manticore Buddy easily installable and compatible with the main C++-based daemon. This major change will allow the team to develop a wide range of new high-level features, such as shards orchestration, access control and authentication, and various integrations like mysqldump and DBeaver. For now it already handles [SHOW QUERIES](../Node_info_and_management/SHOW_QUERIES.md#SHOW-QUERIES), [BACKUP](../Securing_and_compacting_a_table/Backup_and_restore.md#BACKUP-SQL-command-reference) and [Auto schema](../Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-schema).

### Major Changes
* Improved [cost-based optimizer](../Searching/Cost_based_optimizer.md#Cost-based-optimizer) which may increase query response time in many cases. Integrated it with [secondary indexes](../Server_settings/Searchd.md#secondary_indexes)
* Auto-schema: you can now skip creating a table, just `INSERT` and Manticore will create it automatically for you based on the first document
* `ALTER TABLE <table name> REBUILD SECONDARY`
* New tool `manticore-backup` for [backing up and restoring Manticore instance](../Securing_and_compacting_a_table/Backup_and_restore.md)
* SQL command `BACKUP` to do backups from inside Manticore
* SQL command `SHOW QUERIES` as an easy way to see running queries rather than threads
* SQL command `KILL` to kill a long-running `SELECT`
* Dynamic `max_matches` for aggregation queries to increase accuracy and lower response time

### Minor changes
* SQL commands [FREEZE/UNFREEZE](../Securing_and_compacting_a_table/Freezing_a_table.md) to prepare a real-time/plain table for a backup
* New setting `accurate_aggregation` for maximal aggregation accuracy
* Support for signed negative 64-bit IDs. Note, you still can't use IDs > 2^63, but you can now use ids in the range of from -2^63 to 0.
* Since Manticore supports secondary indexes since recently things got confusing since `index` may mean a secondary index, a full-text index or a plain/real-time index. To make things less confusing we are renaming the latter to `table`. These are the affected SQL / command line commands. Their old variants are deprecated, but are still working:
  - `index <table name>` => `table <table name>`
  - `searchd -i / --index` => `searchd -t / --table`
  - `SHOW INDEX STATUS` => `SHOW TABLE STATUS`
  - `SHOW INDEX SETTINGS` => `SHOW TABLE SETTINGS`
  - `FLUSH RTINDEX` => `FLUSH TABLE`
  - `OPTIMIZE INDEX` => `OPTIMIZE TABLE`
  - `ATTACH TABLE plain TO RTINDEX rt` => `ATTACH TABLE plain TO TABLE rt`
  - `RELOAD INDEX` => `RELOAD TABLE`
  - `RELOAD INDEXES` => `RELOAD TABLES`

  We are not planning to obsolete the old forms, but to make things compatible with the documentation we recommend to change the names in your applicaiton.  
* Queries with stateful UDFs are now forced to be executed in a single thread
* Refactoring of task/scheduler system as a prerequisite for parallel chunks merging
* **⚠️ BREAKING CHANGE**: Columnar storage format has been changed. You need to rebuild those indexes that have columnar attributes.
* **⚠️ BREAKING CHANGE**: Secondary indexes file format has got changed, so if you are using secondary indexes for searching and have `searchd.secondary_indexes = 1` in your configuration file, be aware that the new Manticore version **will skip loading the tables that have secondary indexes**. It's recommended to:
  - before you upgrade change `searchd.secondary_indexes` to 0 in the configuration file
  - run the instance. Manticore will load up the tables with a warning.
  - run `ALTER TABLE <table name> REBUILD SECONDARY` for each index to rebuild secondary indexes.

  If you are running a replication cluster, you'll need to run `ALTER TABLE <table name> REBUILD SECONDARY` on all the nodes or follow [this instruction](../Securing_and_compacting_a_table/Compacting_a_table.md#Optimizing-clustered-tables) with just change: run the `ALTER .. REBUILD SECONDARY` instead of the `OPTIMIZE`.
* **⚠️ BREAKING CHANGE**: The binlog version has been updated, so any binlogs from previous versions will not be replayed. It is important to ensure that Manticore Search is stopped cleanly during the upgrade process. This means that there should be no binlog files in `/var/lib/manticore/binlog/` except for `binlog.meta` after stopping the previous instance.
* `SHOW SETTINGS`
* `max_matches_increase_threshold`

### Packaging
* Arm64 packages for macOS and Linux
* Simplified package building for contributors
* Support for Debian Stretch and Ubuntu Xenial has been discontinued
* Centos 9 is supported
* Debian Bookworm is supported

## Bugfixes
* [Commit 10416ef7](https://github.com/manticoresoftware/manticoresearch/commit/10416ef7dddf06c0d759e32ccd6ebaa2468f7cbf) `binlog_flush = 1` has been broken all the time since Sphinx
* ... tens more ...

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
* Support for [Chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding) in HTTP protocol. You can now use chunked transfer in your application to transfer large batches with lower resource consumption (since you don't need to calculate `Content-Length`). On the server's side Manticore now always processes incoming HTTP data in streaming fashion without waiting for the whole batch to be transferred as previously, which:
  - decreases peak RAM consumption, which lowers a chance of OOM
  - decreases response time (our tests showed 11% decrease for processing a 100MB batch)
  - lets you overcome [max_packet_size](../Server_settings/Searchd.md#max_packet_size) and transfer batches much larger than the largest allowed value of `max_packet_size` (128MB), e.g. 1GB at once.
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
* [Commit ffd0499d](https://github.com/manticoresoftware/manticoresearch/commit/ffd0499d329d2c383f14c8a44c4cc84338ab56e7) Support for Java mysql connector >= 6.0.3: in [Java mysql connection 6.0.3](https://mvnrepository.com/artifact/mysql/mysql-connector-java/6.0.3) they changed the way they connect to mysql which broke compatibility with Manticore. The new behaviour is now supported.
* [Commit 1da6dbec](https://github.com/manticoresoftware/manticoresearch/commit/1da6dbec) disabled saving a new disk chunk on loading an index (e.g. on searchd startup).
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
* ❗[Commit 1da4ce89](https://github.com/manticoresoftware/manticoresearch/commits/1da4ce891e4ff13727cfd3331d72771c64ee7949) HTTP actions are not tracked in SHOW STATUS
* [Commit 381000ab](https://github.com/manticoresoftware/manticoresearch/commits/381000ab4af4c3c98ec8c730699ad7f39039cec8) disable pseudo_sharding for low frequency single keyword queries
* [Commit 800325cc](https://github.com/manticoresoftware/manticoresearch/commits/800325cca283a87801b28929d82420fafc76a0ee) fixed stored attributes vs index merge
* [Commit cddfeed6](https://github.com/manticoresoftware/manticoresearch/commits/cddfeed6296f2041c6aae18414e5cd9cabf08281) generalized distinct value fetchers; added specialized distinct fetchers for columnar strings
* [Commit fba4bb4f](https://github.com/manticoresoftware/manticoresearch/commits/fba4bb4f) fixed fetching null integer attributes from docstore
* [Commit f3009a92](https://github.com/manticoresoftware/manticoresearch/commit/f3009a9242fa16956adf871c4887e6e0303ba364) `ranker` could be specified twice in query log

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
* [Commit 00874743](https://github.com/manticoresoftware/manticoresearch/commit/00874743823eaf43e2a09a088392fd7e0a617f47) accurate [count(distinct ...)](Searching/Grouping.md#COUNT%28DISTINCT-field%29) and [FACET ... distinct](Searching/Faceted_search.md#Faceting-without-duplicates) over several local physical indexes (real-time/plain) with identical fields set/order.
* [PR #598](https://github.com/manticoresoftware/manticoresearch/pull/598) bigint support for `YEAR()` and other timestamp functions.
* [Commit 8e85d4bc](https://github.com/manticoresoftware/manticoresearch/commit/8e85d4bce596cc88f9d9615ea243a27e4afde0f7) Adaptive [rt_mem_limit](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Previously Manticore Search was collecting exactly up to `rt_mem_limit` of data before saving a new disk chunk to disk, and while saving was still collecting up to 10% more (aka double-buffer) to minimize possible insert suspension. If that limit was also exhausted, adding new documents was blocked until the disk chunk was fully saved to disk. The new adaptive limit is built on the fact that we have [auto-optimize](Server_settings/Searchd.md#auto_optimize) now, so it's not a big deal if disk chunks do not fully respect `rt_mem_limit` and start flushing a disk chunk earlier. So, now we collect up to 50% of `rt_mem_limit` and save that as a disk chunk. Upon saving we look at the statistics (how much we've saved, how many new documents have arrived while saving) and recalculate the initial rate which will be used next time. For example, if we saved 90 million documents, and another 10 million docs arrived while saving, the rate is 90%, so we know that next time we can collect up to 90% of `rt_mem_limit` before starting flushing another disk chunk. The rate value is calculated automatically from 33.3% to 95%.
* [Issue #628](https://github.com/manticoresoftware/manticoresearch/issues/628) [unpack_zlib](Adding_data_from_external_storages/Fetching_from_databases/Database_connection.md#unpack_zlib) for PostgreSQL source. Thank you, [Dmitry Voronin](https://github.com/dimv36) for the [contribution](https://github.com/manticoresoftware/manticoresearch/pull/630).
* [Commit 6d54cf2b](https://github.com/manticoresoftware/manticoresearch/commit/6d54cf2b319b131970c29410cc21abcbdf8083b1) `indexer -v` and `--version`. Previously you could still see indexer's version, but `-v`/`--version` were not supported.
* [Issue #662](https://github.com/manticoresoftware/manticoresearch/issues/662) infinit mlock limit by default when Manticore is started via systemd.
* [Commit 63c8cd05](https://github.com/manticoresoftware/manticoresearch/commit/63c8cd05cf399e705c2c14939411a92cb784735a) spinlock -> op queue for coro rwlock.
* [Commit 41130ce3](https://github.com/manticoresoftware/manticoresearch/commit/41130ce30d1734bbf8e6b8b63c4e815caf9464f2) environment variable `MANTICORE_TRACK_RT_ERRORS` useful for debugging RT segments corruption.

### Breaking changes
* Binlog version was increased, binlog from previous version won't be replayed, so make sure you stop Manticore Search cleanly during upgrade: no binlog files should be in `/var/lib/manticore/binlog/` except `binlog.meta` after stopping the previous instance.
* [Commit 3f659f36](https://github.com/manticoresoftware/manticoresearch/commit/3f659f36e87d99ee262205a8eae4663a255be630) new column "chain" in `show threads option format=all`. It shows stack of some task info tickets, most useful for profiling needs, so if you are parsing `show threads` output be aware of the new column.
* `searchd.workers` was obsoleted since 3.5.0, now it's deprecated, if you still have it in your configuration file it will trigger a warning on start. Manticore Search will start, but with a warning.
* If you use PHP and PDO to access Manticore you need to do `PDO::ATTR_EMULATE_PREPARES`

### Bugfixes
* ❗[Issue #650](https://github.com/manticoresoftware/manticoresearch/issues/650) Manticore 4.0.2 slower than Manticore 3.6.3. 4.0.2 was faster than previous versions in terms of bulk inserts, but significantly slower for single document inserts. It's been fixed in 4.2.0.
* ❗[Commit 22f4141b](https://github.com/manticoresoftware/manticoresearch/commit/22f4141bed1411c83684ae6e0bae832071cce3df) RT index could get corrupted under intensive REPLACE load, or it could crash
* [Commit 03be91e4](https://github.com/manticoresoftware/manticoresearch/commit/03be91e4f720a3f20bee0278dd8f0d44abd3ef0e) fixed average at merging groupers and group N sorter; fixed merge of aggregates
* [Commit 2ea575d3](https://github.com/manticoresoftware/manticoresearch/commit/2ea575d34efe38661f14d858f2b552456f29fe42) `indextool --check` could crash
* [Commit 7ec76d4a](https://github.com/manticoresoftware/manticoresearch/commit/7ec76d4abf01b5fca6f10a0000fe64d5d8e5fa52) RAM exhaustion issue caused by UPDATEs
* [Commit 658a727e](https://github.com/manticoresoftware/manticoresearch/commit/658a727e41f3c3fbcde9754149528f34ed76ba19) daemon could hang on INSERT
* [Commit 46e42b9b](https://github.com/manticoresoftware/manticoresearch/commit/46e42b9b386e31e53247ed634e158b533e28063e) daemon could hang on shutdown
* [Commit f8d7d517](https://github.com/manticoresoftware/manticoresearch/commit/f8d7d5177cc83d250b493b4cfc619732b29cf072) daemon could crash on shutdown
* [Commit 733accf1](https://github.com/manticoresoftware/manticoresearch/commit/733accf165142a1d1574689525ccca8907971158) daemon could hang on crash
* [Commit f7f8bd8c](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) daemon could crash on startup trying to rejoin cluster with invalid nodes list
* [Commit 14015561](https://github.com/manticoresoftware/manticoresearch/commit/140155613821ecaddcd1a3dfc4f7369b0df6aef7) distributed index could get completely forgotten in RT mode in case it couldn't resolve one of its agents on start
* [Issue #683](https://github.com/manticoresoftware/manticoresearch/issues/683) attr bit(N) engine='columnar' fails
* [Issue #682](https://github.com/manticoresoftware/manticoresearch/issues/682) create table fails, but leaves dir
* [Issue #663](https://github.com/manticoresoftware/manticoresearch/issues/663) Config fails with: unknown key name 'attr_update_reserve'
* [Issue #632](https://github.com/manticoresoftware/manticoresearch/issues/632) Manticore crash on batch queries
* [Issue #679](https://github.com/manticoresoftware/manticoresearch/issues/679) Batch queries causing crashes again with v4.0.3
* [Commit f7f8bd8c](https://github.com/manticoresoftware/manticoresearch/commit/f7f8bd8c0dbb23a59a88502dcba16b0918fb2330) fixed daemon crash on startup trying to re-join cluster with invalid nodes list
* [Issue #643](https://github.com/manticoresoftware/manticoresearch/issues/643) Manticore 4.0.2 does not accept connections after batch of inserts
* [Issue #635](https://github.com/manticoresoftware/manticoresearch/issues/635) FACET query with ORDER BY JSON.field or string attribute could crash
* [Issue #634](https://github.com/manticoresoftware/manticoresearch/issues/634) Crash SIGSEGV on query with packedfactors
* [Commit 41657f15](https://github.com/manticoresoftware/manticoresearch/commits/41657f15f8fcf1e93cfad7f835bcf705d6e8349e) morphology_skip_fields was not supported by create table

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
  - [Commit 696f8649](https://github.com/manticoresoftware/manticoresearch/commit/696f8649535f2b5285b878c9581c0d2b1139ae09) - fixed crash during SST on joiner with active index; added sha1 verify at joiner node at writing file chunks to speed up index loading; added rotation of changed index files at joiner node on index load; added removal of index files at joiner node when active index gets replaced by a new index from donor node; added replication log points at donor node for sending files and chunks
  - [Commit b296c55a](https://github.com/manticoresoftware/manticoresearch/commit/b296c55af68314a8fc66f8b104e7301d673c3b68) - crash on JOIN CLUSTER in case the address is incorrect
  - [Commit 418bf880](https://github.com/manticoresoftware/manticoresearch/commit/418bf880a5e05ffbb68f311a1c23006a9df3220e) - while initial replication of a large index the joining node could fail with `ERROR 1064 (42000): invalid GTID, (null)`, the donor could become unresponsive while another node was joining
  - [Commit 6fd350d2](https://github.com/manticoresoftware/manticoresearch/commit/6fd350d28e4905211cf8a7298cbf7cdda101c83d) - hash could be calculated wrong for a big index which could result in replication failure
  - [Issue #615](https://github.com/manticoresoftware/manticoresearch/issues/615) - replication failed on cluster restart
- [Issue #574](https://github.com/manticoresoftware/manticoresearch/issues/574) - `indextool --help` doesn't display parameter `--rotate`
- [Issue #578](https://github.com/manticoresoftware/manticoresearch/issues/578) - searchd high CPU usage while idle after ca. a day
- [Issue #587](https://github.com/manticoresoftware/manticoresearch/issues/587) - flush .meta immediately
- [Issue #617](https://github.com/manticoresoftware/manticoresearch/issues/617) - manticore.json gets emptied
- [Issue #618](https://github.com/manticoresoftware/manticoresearch/issues/618) - searchd --stopwait fails under root. It also fixes systemctl behaviour (previously it was showing failure for ExecStop and didn't wait long enough for searchd to stop properly)
- [Issue #619](https://github.com/manticoresoftware/manticoresearch/issues/619) - INSERT/REPLACE/DELETE vs SHOW STATUS. `command_insert`, `command_replace` and others were showing wrong metrics
- [Issue #620](https://github.com/manticoresoftware/manticoresearch/issues/620) - `charset_table` for a plain index had a wrong default value
- [Commit 8f753688](https://github.com/manticoresoftware/manticoresearch/commit/8f7536887d4b58a5bab5647fc067b2d0482c2038) - new disk chunks don't get mlocked
- [Issue #607](https://github.com/manticoresoftware/manticoresearch/issues/607) - Manticore cluster node crashes when unable to resolve a node by name
- [Issue #623](https://github.com/manticoresoftware/manticoresearch/issues/623) - replication of updated index can lead to undefined state
- [Commit ca03d228](https://github.com/manticoresoftware/manticoresearch/commit/ca03d2280c5197e9b311f0f03f551e3b702a8130) - indexer could hang on indexing a plain index source with a json attribute
- [Commit 53c75305](https://github.com/manticoresoftware/manticoresearch/commit/53c753054558ff3f82c7af365273d32537c5338b) - fixed not equal expression filter at PQ index
- [Commit ccf94e02](https://github.com/manticoresoftware/manticoresearch/commit/ccf94e022ba6763d6286ab5bac767b6f6a846846) - fixed select windows at list queries above 1000 matches. `SELECT * FROM pq ORDER BY id desc LIMIT 1000 , 100 OPTION max_matches=1100` was not working previously
- [Commit a0483fe9](https://github.com/manticoresoftware/manticoresearch/commit/a0483fe9bef3c45dffe96f5df4f4382aee29c072) - HTTPS request to Manticore could cause warning like "max packet size(8388608) exceeded"
- [Issue #648](https://github.com/manticoresoftware/manticoresearch/issues/648) - Manticore 3 could hang after a few updates of string attributes


## Version 3.6.0, May 3rd 2021
**Maintenance release before Manticore 4**

### Major new features
- Support for [Manticore Columnar Library](https://github.com/manticoresoftware/columnar/) for plain indexes. New setting [columnar_attrs](Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) for plain indexes
- Support for [Ukrainian Lemmatizer](https://github.com/manticoresoftware/lemmatizer-uk)
- Fully revised histograms. When building an index Manticore also builds histograms for each field in it, which it then uses for faster filtering. In 3.6.0 the algorithm was fully revised and you can get a higher performance if you have a lot of data and do a lot of filtering.

### Minor changes
- tool `manticore_new_cluster [--force]` useful for restarting a replication cluster via systemd
- [--drop-src](Adding_data_from_external_storages/Adding_data_to_tables/Merging_tables.md#Merging-tables) for `indexer --merge`
- [new mode](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) `blend_mode='trim_all'`
- added [support for escaping JSON path](Searching/Full_text_matching/Escaping.md#Escaping-json-node-names-in-SQL) with backticks
- [indextool --check](Miscellaneous_tools.md#indextool) can work in RT mode
- [FORCE/IGNORE INDEX(id)](Updating_documents/UPDATE.md#FORCE-and-IGNORE-INDEX) for SELECT/UPDATE
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
- [Commit d1dbe771](https://github.com/manticoresoftware/manticoresearch/commit/d1dbe771288328c963ca8512f564c6b7cf4f466c) - daemon crash on replay binlog with update of string attribute; set binlog version to 10
- [Commit 775d0555](https://github.com/manticoresoftware/manticoresearch/commit/775d0555562f121911d3c30085947e743fadac2d)  - fixed expression stack frame detection runtime (test 207)
- [Commit 4795dc49](https://github.com/manticoresoftware/manticoresearch/commit/4795dc49194c7745761bb5006ecb266e8a9224fa)  - percolate index filter and tags were empty for empty stored query (test 369)
- [Commit c3f0bf4d](https://github.com/manticoresoftware/manticoresearch/commit/c3f0bf4dca0e4eae6ea899dd566878df57eba085)  - breaks of replication SST flow at network with long latency and high error rate (different data centers replication); updated replication command version to 1.03
- [Commit ba2d6619](https://github.com/manticoresoftware/manticoresearch/commit/ba2d6619b8107f3fb177741bbdff3709bac248cd)  - joiner lock cluster on write operations after join into cluster (test 385)
- [Commit de4dcb9f](https://github.com/manticoresoftware/manticoresearch/commit/de4dcb9ff6c316bc2a9ef6c62eb64769f04ee103)  - wildcards matching with exact modifier (test 321)
- [Commit 6524fc6a](https://github.com/manticoresoftware/manticoresearch/commit/6524fc6af032a0932bd42c37874a619e1fb7f075)  - docid checkpoints vs docstore
- [Commit f4ab83c2](https://github.com/manticoresoftware/manticoresearch/commit/f4ab83c291e65a5873d271c67b5491f629544f1d)  - Inconsistent indexer behavior when parsing invalid xml
- [Commit 7b727e22](https://github.com/manticoresoftware/manticoresearch/commit/7b727e22fe01b1a180d4a8d3a3fea92c18eac707)  - Stored percolate query with NOTNEAR runs forever (test 349)
- [Commit 812dab74](https://github.com/manticoresoftware/manticoresearch/commit/812dab745f0c972ecde5d5b3d4a313cd6e1e143e)  - wrong weight for phrase starting with wildcard
- [Commit 1771afc6](https://github.com/manticoresoftware/manticoresearch/commit/1771afc669c75670b3511bf02ab9bfc8982b94d2)  - percolate query with wildcards generate terms without payload on matching causes interleaved hits and breaks matching (test 417)
- [Commit aa0d8c2b](https://github.com/manticoresoftware/manticoresearch/commit/aa0d8c2bcb41428d41dbfb66e42a872a79a4ee4b)  - fixed calculation of 'total' in case of parallelized query
- [Commit 18d81b3c](https://github.com/manticoresoftware/manticoresearch/commit/18d81b3cfbb930b7361dae645fe13283d4fbf6ef)  - crash in Windows with multiple concurrent sessions at daemon
- [Commit 84432f23](https://github.com/manticoresoftware/manticoresearch/commit/84432f23d6fbf7a7d0b67c8c9412eb5933b03596)  - some index settings could not be replicated
- [Commit 93411fe6](https://github.com/manticoresoftware/manticoresearch/commit/93411fe6af4246ca4acce758944d513a1660b92b)  - On high rate of adding new events netloop sometimes freeze because of atomic 'kick' event being processed once for several events a time and loosing actual actions from them
status of the query, not the server status
- [Commit d805fc12](https://github.com/manticoresoftware/manticoresearch/commit/d805fc125244ef08d14fb3d1826e2c356862d539)  - New flushed disk chunk might be lost on commit
- [Commit 63cbf008](https://github.com/manticoresoftware/manticoresearch/commit/63cbf008d50979cdf7062664fb02a672a3bb1131)  - inaccurate 'net_read' in profiler
- [Commit f5379bb2](https://github.com/manticoresoftware/manticoresearch/commit/f5379bb27234b25919966c50ccb6f469ead646e2)  - Percolate issue with arabic (right to left texts)
- [Commit 49eeb420](https://github.com/manticoresoftware/manticoresearch/commit/49eeb4202d4ce4875c07c35087d7108c1e177328)  - id not picked correctly on duplicate column name
- [Commit refactoring](https://github.com/manticoresoftware/manticoresearch/commit/4b7df262bbfec9b9e95a44e72ad326ac8a786c49) of network events to fix a crash in rare cases
- [e8420cc7](https://github.com/manticoresoftware/manticoresearch/tree/e8420cc789149c84731be0874be916e33b263bd6) fix in `indextool --dumpheader`
- [Commit ff716353](https://github.com/manticoresoftware/manticoresearch/commit/ff7163532ed39042c0d3c3c2b902ff41f94d7f0c) - TRUNCATE WITH RECONFIGURE worked wrong with stored fields

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
- [Issue #453](https://github.com/manticoresoftware/manticoresearch/issues/453) New option [indexer.ignore_non_plain=1](Adding_data_from_external_storages/Plain_tables_creation.md#ignore_non_plain) is useful in case you run `indexer --all` and have not only plain indexes in the configuration file. Without `ignore_non_plain=1` you'll get a warning and a respective exit code.
- [SHOW PLAN ... OPTION format=dot](Node_info_and_management/Profiling/Query_plan.md#Dot-format-for-SHOW-PLAN) and [EXPLAIN QUERY ... OPTION format=dot](Searching/Full_text_matching/Profiling.md#Profiling-without-running-a-query) enable visualization of full-text query plan execution. Useful for understanding complex queries.

### Deprecations
- `indexer --verbose` is deprecated as it never added anything to the indexer output
- For dumping watchdog's backtrace signal `USR2` is now to be used instead of `USR1`

### Bugfixes
- [Issue #423](https://github.com/manticoresoftware/manticoresearch/issues/423) cyrillic char period call snippets retain mode don't highlight
- [Issue #435](https://github.com/manticoresoftware/manticoresearch/issues/435) RTINDEX - GROUP N BY expression select = fatal crash
- [Commit 2b3b62bd](https://github.com/manticoresoftware/manticoresearch/commit/2b3b62bd0ce2b54544733d524a169e81f71b5388) searchd status shows Segmentation fault when in cluster
- [Commit 9dd25c19](https://github.com/manticoresoftware/manticoresearch/commit/9dd25c193ed2a53750a0e0e68566fd2b833fcb0c) 'SHOW INDEX index.N SETTINGS' doesn't address chunks >9
- [Issue #389](https://github.com/manticoresoftware/manticoresearch/issues/389) Bug that crashes Manticore
- [Commit fba16617](https://github.com/manticoresoftware/manticoresearch/commit/fba166178ff35c73a0c70d131cf3aca4a635e961) Converter creates broken indexes
- [Commit eecd61d8](https://github.com/manticoresoftware/manticoresearch/commit/eecd61d861c742471175be19a5e5455034b5db5f) stopword_step=0 vs CALL SNIPPETS()
- [Commit ea6850e4](https://github.com/manticoresoftware/manticoresearch/commit/ea6850e449601199190f38a387c04b8a7b7e90f6) count distinct returns 0 at low max_matches on a local index
- [Commit 362f27db](https://github.com/manticoresoftware/manticoresearch/commit/362f27db3421f09a41c6cce04b5baace00fa4c4d) When using aggregation stored texts are not returned in hits


## Version 3.5.2, Oct 1 2020

### New features

* OPTIMIZE reduces disk chunks to a number of chunks ( default is `2* No. of cores`) instead of a single one. The optimal number of chunks can be controlled by [cutoff](Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) option.
* NOT operator can be now used standalone. By default it is disabled since accidental single NOT queries can be slow. It can be enabled by setting new searchd directive [not_terms_only_allowed](Server_settings/Searchd.md#not_terms_only_allowed) to `0`.
* New setting [max_threads_per_query](Server_settings/Searchd.md#max_threads_per_query) sets how many threads a query can use. If the directive is not set, a query can use threads up to the value of [threads](Server_settings/Searchd.md#threads).
Per `SELECT` query the number of threads can be limited with [OPTION threads=N](Searching/Options.md#threads) overriding the global `max_threads_per_query`.
* Percolate indexes can be now be imported with [IMPORT TABLE](Adding_data_from_external_storages/Adding_data_to_tables/Importing_table.md).
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

1. [Commit 2a474dc1](https://github.com/manticoresoftware/manticoresearch/commit/2a474dc1a26e8b0f8aaaae95669caf2f1d4b7746) Crash of daemon at grouper at RT index with different chunks
2. [Commit 57a19e5a](https://github.com/manticoresoftware/manticoresearch/commit/57a19e5ad5663ef0ca7436595218fb1221d28c8e) Fastpath for empty remote docs
3. [Commit 07dd3f31](https://github.com/manticoresoftware/manticoresearch/commit/07dd3f313c63fb82c22092f9907ef24e3475250e) Expression stack frame detection runtime
4. [Commit 08ae357c](https://github.com/manticoresoftware/manticoresearch/commit/08ae357cf1012bc8e2da54c20b205b592efda3d4) Matching above 32 fields at percolate indexes
5. [Commit 16b9390f](https://github.com/manticoresoftware/manticoresearch/commit/16b9390fd4cdb07a77ac4497adb935573a1710e5) Replication listen ports range
6. [Commit 5fa671af](https://github.com/manticoresoftware/manticoresearch/commit/5fa671affeacb6441a59c8a88479bfd423df7c81) Show create table on pq
7. [Commit 54d133b6](https://github.com/manticoresoftware/manticoresearch/commit/54d133b6449105a9fb0168db3f1fbb05fb5aa1f6) HTTPS port behavior
8. [Commit fdbbe524](https://github.com/manticoresoftware/manticoresearch/commit/fdbbe5245cc296cc5c1ae3ae2fb9cb08fb66a248) Mixing docstore rows when replacing
9. [Commit afb53f64](https://github.com/manticoresoftware/manticoresearch/commit/afb53f648ef4e64fa1776c58e66f6e716ac730ad) Switch TFO unavailable message level to 'info'
10. [Commit 59d94cef](https://github.com/manticoresoftware/manticoresearch/commit/59d94cefc6e88af360b8046726a16b7eaa9f0b71) Crash on strcmp invalid use
11. [Commit 04af0349](https://github.com/manticoresoftware/manticoresearch/commit/04af0349c1e784c2230d204592d6272b440bb375) Adding index to cluster with system (stopwords) files
12. [Commit 50148b4e](https://github.com/manticoresoftware/manticoresearch/commit/50148b4eae9f3a924082005cf5d2f9c8339af7f5) Merge indexes with large dictionaries; RT optimize of large disk chunks
13. [Commit a2adf158](https://github.com/manticoresoftware/manticoresearch/commit/a2adf1582843f63d4dcc1dab8f7598e0917094a2) Indextool can dump meta from current version
14. [Commit 69f6d5f7](https://github.com/manticoresoftware/manticoresearch/commit/69f6d5f77c64b97e3f395aea8c555b11c77fb732) Issue in group order in GROUP N
15. [Commit 24d5d80f](https://github.com/manticoresoftware/manticoresearch/commit/24d5d80f508576794b751d70f9f9b524564e3c14) Explicit flush for SphinxSE after handshake
16. [Commit 31c4d78a](https://github.com/manticoresoftware/manticoresearch/commit/31c4d78ac03ee85d9a02b86075448a04ac80f9e2) Avoid copy of huge descriptions when not necessary
17. [Commit 2959e2ca](https://github.com/manticoresoftware/manticoresearch/commit/2959e2caa8610f98fcdf2e5a82160f58687f4a27) Negative time in show threads
18. [Commit f0b35710](https://github.com/manticoresoftware/manticoresearch/commit/f0b357102c70f3ac1d7da260c60b67322940e647) Token filter plugin vs zero position deltas
19. [Commit a49e5bc1](https://github.com/manticoresoftware/manticoresearch/commit/a49e5bc13ea763e6f01dd5993e891858a84d1846) Change 'FAIL' to 'WARNING' on multiple hits

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
* [`INSERT INTO PQ VALUES()`](Adding_documents_to_a_table/Adding_rules_to_a_percolate_table.md) (i.e. without providing column list) previously expected exactly `(query, tags)` as the values. It's been changed to `(id,query,tags,filters)`. The id can be set to 0 if you want it to be auto-generated.
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
2. [Commit ceabe44f](https://github.com/manticoresoftware/manticoresearch/commit/ceabe44f) Tiny read out of bounds in snippets
3. [Commit 1c3e84a3](https://github.com/manticoresoftware/manticoresearch/commit/1c3e84a3) Dangerous write into local variable for crash queries
4. [Commit 26e094ab](https://github.com/manticoresoftware/manticoresearch/commit/26e094ab) Tiny memory leak of sorter in test 226
5. [Commit d2c7f86a](https://github.com/manticoresoftware/manticoresearch/commit/d2c7f86a) Huge memory leak in test 226
6. [Commit 0dd80122](https://github.com/manticoresoftware/manticoresearch/commit/0dd80122) Cluster shows the nodes are in sync, but `count(*)` shows different numbers
7. [Commit f1c1ac3f](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Cosmetic: Duplicate and sometimes lost warning messages in the log
8. [Commit f1c1ac3f](https://github.com/manticoresoftware/manticoresearch/commit/f1c1ac3f) Cosmetic: (null) index name in log
9. [Commit 359dbd30](https://github.com/manticoresoftware/manticoresearch/commit/359dbd30) Cannot retrieve more than 70M results
10. [Commit 19f328ee](https://github.com/manticoresoftware/manticoresearch/commit/19f328ee) Can't insert PQ rules with no-columns syntax
11. [Commit bf685d5d](https://github.com/manticoresoftware/manticoresearch/commit/bf685d5d) Misleading error message when inserting a document to an index in a cluster
12. [Commit 2cf18c83](https://github.com/manticoresoftware/manticoresearch/commit/2cf18c83) `/json/replace` and `json/update` return id in exponent form
13. [Issue #324](https://github.com/manticoresoftware/manticoresearch/issues/324) Update json scalar properties and mva in the same query
14. [Commit d38409eb](https://github.com/manticoresoftware/manticoresearch/commit/d38409eb) `hitless_words` doesn't work in RT mode
15. [Commit 5813d639](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `ALTER RECONFIGURE` in rt mode should be disallowed
16. [Commit 5813d639](https://github.com/manticoresoftware/manticoresearch/commit/5813d639) `rt_mem_limit` gets reset to 128M after searchd restart
17. highlight() sometimes hangs
18. [Commit 7cd878f4](https://github.com/manticoresoftware/manticoresearch/commit/7cd878f4) Failed to use U+code in RT mode
19. [Commit 2b213de4](https://github.com/manticoresoftware/manticoresearch/commit/2b213de4) Failed to use wildcard at wordforms at RT mode
20. [Commit e9d07e68](https://github.com/manticoresoftware/manticoresearch/commit/e9d07e68) Fixed `SHOW CREATE TABLE` vs multiple wordform files
21. [Commit fc90a84f](https://github.com/manticoresoftware/manticoresearch/commit/fc90a84f) JSON query without "query" crashes searchd
22. Manticore [official docker](https://hub.docker.com/r/manticoresearch/manticore) couldn't index from mysql 8
23. [Commit 23e05d32](https://github.com/manticoresoftware/manticoresearch/commit/23e05d32) HTTP /json/insert requires id
24. [Commit bd679af0](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `SHOW CREATE TABLE` doesn't work for PQ
25. [Commit bd679af0](https://github.com/manticoresoftware/manticoresearch/commit/bd679af0) `CREATE TABLE LIKE` doesn't work properly for PQ
26. [Commit 5eacf28f](https://github.com/manticoresoftware/manticoresearch/commit/5eacf28f) End of line in settings in show index status
27. [Commit cb153228](https://github.com/manticoresoftware/manticoresearch/commit/cb153228) Empty title in "highlight" in HTTP JSON response
28. [Issue #318](https://github.com/manticoresoftware/manticoresearch/issues/318) `CREATE TABLE LIKE` infix error
29. [Commit 9040d22c](https://github.com/manticoresoftware/manticoresearch/commit/9040d22c) RT crashes under load
30. [cd512c7d ](https://github.com/manticoresoftware/manticoresearch/commit/cd512c7d ) Lost crash log on crash at RT disk chunk
31. [Issue #323](https://github.com/manticoresoftware/manticoresearch/issues/323) Import table fails and closes the connection
32. [Commit 6275316a](https://github.com/manticoresoftware/manticoresearch/commit/6275316a) `ALTER reconfigure` corrupts a PQ index
33. [Commit 9c1d221e](https://github.com/manticoresoftware/manticoresearch/commit/9c1d221e) Searchd reload issues after change index type
34. [Commit 71e2b5bb](https://github.com/manticoresoftware/manticoresearch/commit/71e2b5bb) Daemon crashes on import table with missed files
35. [Issue #322](https://github.com/manticoresoftware/manticoresearch/issues/322) Crash on select using multiple indexes, group by and ranker = none
36. [Commit c3f58490](https://github.com/manticoresoftware/manticoresearch/commit/c3f58490) `HIGHLIGHT()` doesn't higlight in string attributes
37. [Issue #320](https://github.com/manticoresoftware/manticoresearch/issues/320) `FACET` fails to sort on string attribute
38. [Commit 4f1a1f25](https://github.com/manticoresoftware/manticoresearch/commit/4f1a1f25) Error in case of missing data dir
39. [Commit 04f4ddd4](https://github.com/manticoresoftware/manticoresearch/commit/04f4ddd4) access_* are not supported in RT mode
40. [Commit 1c0616a2](https://github.com/manticoresoftware/manticoresearch/commit/1c0616a2) Bad JSON objects in strings: 1. `CALL PQ` returns "Bad JSON objects in strings: 1" when the json is greater than some value.
41. [Commit 32f943d6](https://github.com/manticoresoftware/manticoresearch/commit/32f943d6) RT-mode inconsistency. In some cases I can't drop the index since it's unknown and can't create it since the directory is not empty.
42. [Issue #319](https://github.com/manticoresoftware/manticoresearch/issues/319) Crash on select
43. [Commit 22a28dd7](https://github.com/manticoresoftware/manticoresearch/commit/22a28dd7) `max_xmlpipe2_field` = 2M returned warning on 2M field
44. [Issue #342](https://github.com/manticoresoftware/manticoresearch/issues/342) Query conditions execution bug
45. [Commit dd8dcab2](https://github.com/manticoresoftware/manticoresearch/commit/dd8dcab2) Simple 2 terms search finds a document containing only one term
46. [Commit 90919e62](https://github.com/manticoresoftware/manticoresearch/commit/90919e62) It was impossible in PQ to match a json with capital letters in keys
47. [Commit 56da086a](https://github.com/manticoresoftware/manticoresearch/commit/56da086a) Indexer crashes on csv+docstore
48. [Issue #363](https://github.com/manticoresoftware/manticoresearch/issues/363) using `[null]` in json attr in centos 7 causes corrupted inserted data
49. Major [Issue #345](https://github.com/manticoresoftware/manticoresearch/issues/345) Records not being inserted, count() is random, "replace into" returns OK
50. max_query_time slows down SELECTs too much
51. [Issue #352](https://github.com/manticoresoftware/manticoresearch/issues/352) Master-agent communication fails on Mac OS
52. [Issue #328](https://github.com/manticoresoftware/manticoresearch/issues/328) Error when connecting to Manticore with Connector.Net/Mysql 8.0.19
53. [Commit daa760d2](https://github.com/manticoresoftware/manticoresearch/commit/daa760d2fe83668038318f0f384ceefc0d075e18) Fixed escaping of \0 and optimized performance
54. [Commit 9bc5c01a](https://github.com/manticoresoftware/manticoresearch/commit/9bc5c01a493db993b90ff6c1ad51aeae028f629c) Fixed count distinct vs json
55. [Commit 4f89a965](https://github.com/manticoresoftware/manticoresearch/commit/4f89a965a3af0db548ae9d1763731ebaebe21f5a) Fixed drop table at other node failed
56. [Commit 952af5a5](https://github.com/manticoresoftware/manticoresearch/commit/952af5a5931fd72b87edc169588777778b365231) Fix crashes on tightly running call pq


## Version 3.4.2, 10 April 2020
### Critical bugfixes
* [Commit 2ffe2d26](https://github.com/manticoresoftware/manticoresearch/commit/2ffe2d261793f9681265bc4abe38d695e72c121e) fix RT index from old version fails to index data

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
* [Commit a5333644](https://github.com/manticoresoftware/manticoresearch/commit/a5333644e8e0c69746a9f24da8221fc965478e4a) fix crash on wrong field syntax in highlight()
* [Commit 7fbb9f2e](https://github.com/manticoresoftware/manticoresearch/commit/7fbb9f2e618096289cba7f8b834530b46aa94055) fix crash of server on replicate RT index with docstore
* [Commit 24a04687](https://github.com/manticoresoftware/manticoresearch/commit/24a04687eb43b4f4719c7c4a5fe3c19099240467) fix crash on highlight to index with infix or prefix option and to index wo stored fields enabled
* [Commit 3465c1ce](https://github.com/manticoresoftware/manticoresearch/commit/3465c1ce9c860c09f6ccc5b83a63e3c68c241d8d) fix false error about empty docstore and dock-id lookup for empty index
* [Commit a707722e](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) fix #314 SQL insert command with trailing semicolon
* [Commit 95628c9b](https://github.com/manticoresoftware/manticoresearch/commit/95628c9b9b8f16fe1bd18599a84e9673575adf1a) removed warning on query word(s) mismatch
* [Commit b8601b41](https://github.com/manticoresoftware/manticoresearch/commit/b8601b4144b9092be43a7caa7cd6b19cb37e14c7) fix queries in snippets segmented via ICU
* [Commit 5275516c](https://github.com/manticoresoftware/manticoresearch/commit/5275516ca4631c584e6e0127fa161a18d2658fd0) fix find/add race condition in docstore block cache
* [Commit f06ef97a](https://github.com/manticoresoftware/manticoresearch/commit/f06ef97af5c7390427e82525c150535290e42c3e) fix mem leak in docstore
* [Commit a7258ba8](https://github.com/manticoresoftware/manticoresearch/commit/a7258ba8340e246c0d8aaeb4a7a74129eba807eb) fix #316 LAST_INSERT_ID returns empty on INSERT
* [Commit 1ebd5bf8](https://github.com/manticoresoftware/manticoresearch/commit/1ebd5bf8d74113bcfa138ce63ce11966990a4215) fix #317 json/update HTTP endpoint to support array for MVA and object for JSON attribute
* [Commit e426950a](https://github.com/manticoresoftware/manticoresearch/commit/e426950a536382a142e2a42ed207c341149ff685) fix rash of indexer dumping rt without explicit id

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
* [Commit 9c33aab8](https://github.com/manticoresoftware/manticoresearch/commit/9c33aab8c1fd9059c8644d23d5f58f03010043c6) added check of index schema for duplicate attributes #293
* [Commit a0085f94](https://github.com/manticoresoftware/manticoresearch/commit/a0085f9477e191da9e0e835b91583fb9cfde698e) fix crash in hitless terms
* [Commit 68953740](https://github.com/manticoresoftware/manticoresearch/commit/68953740af8dca1833026d0f448c71416ed3044a) fix loose docstore after ATTACH
* [Commit d6f696ed](https://github.com/manticoresoftware/manticoresearch/commit/d6f696edea53ea2b9a68251e4054e4816082c285) fix docstore issue in distributed setup
* [Commit bce2b7ec](https://github.com/manticoresoftware/manticoresearch/commit/bce2b7ec883d208f31094ee42a5b6f01d18a998f) replace FixedHash with OpenHash in sorter
* [Commit e0baf739](https://github.com/manticoresoftware/manticoresearch/commit/e0baf7392937969e640fb5d10e8ccb312fe36446) fix attributes with duplicated names at index definition
* [Commit ca81114b](https://github.com/manticoresoftware/manticoresearch/commit/ca81114bf1ac7ea7adcc8509bdc14d1f30b4f461) fix html_strip in HIGHLIGHT()
* [Commit 493a5e91](https://github.com/manticoresoftware/manticoresearch/commit/493a5e916d70fd4a3f14d1ee487234434f4a7ce3) fix passage macro in HIGHLIGHT()
* [Commit a82d41c7](https://github.com/manticoresoftware/manticoresearch/commit/a82d41c775692f837f267d554d7d4a4b499d69f0) fix double buffer issues when RT index creates small or large disk chunk
* [Commit a404c85d](https://github.com/manticoresoftware/manticoresearch/commit/a404c85de0d41d9d1c75b7540479d8a3af9e61aa) fix event deletion for kqueue
* [Commit 8bea0f6f](https://github.com/manticoresoftware/manticoresearch/commit/8bea0f6fd1666b817995c4dd6e67f6f1c9161638) fix save of disk chunk for large value of rt_mem_limit of RT index
* [Commit 8707f039](https://github.com/manticoresoftware/manticoresearch/commit/8707f0395b7ace52dc0cba9c8ed92a2bc4392932) fix float overflow on indexing
* [Commit a56434ce](https://github.com/manticoresoftware/manticoresearch/commit/a56434ce71bb7714b4d8e75278aaa075b27ece80) fix insert document with negative ID into RT index fails with error now
* [Commit bbebfd75](https://github.com/manticoresoftware/manticoresearch/commit/bbebfd7551a68641154761cac962053b54806759) fix crash of server on ranker fieldmask
* [Commit 3809cc1b](https://github.com/manticoresoftware/manticoresearch/commit/3809cc1b0863e3a5a200dc423be3f7c38be56113) fix crash on using query cache
* [Commit dc2a585b](https://github.com/manticoresoftware/manticoresearch/commit/dc2a585bc724678679e4297362200bbd49ce32bb) fix crash on using RT index RAM segments with parallel inserts

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
* [Commit 6ae474c7](https://github.com/manticoresoftware/manticoresearch/commit/6ae474c7894a6bee222d5b18e59a44fdbf57843a) fix crash on SELECT query over HTTP interface
* [Commit 59577513](https://github.com/manticoresoftware/manticoresearch/commit/59577513a49eac5a4a3c5e2cb38394d3246b5d35) fix RT index saves disk chunks but does not mark some documents deleted
* [Commit e861f0fc](https://github.com/manticoresoftware/manticoresearch/commit/e861f0fca0e88924450695d4e9d6acff7a36558a) fix crash on search of multi index or multi queries with dist_threads
* [Commit 440991fc](https://github.com/manticoresoftware/manticoresearch/commit/440991fc977b8479800b45cdbc862eeb1ba5d965) fix crash on infix generation for long terms with wide utf8 codepoints
* [Commit 5fd599b4](https://github.com/manticoresoftware/manticoresearch/commit/5fd599b48bba527c023e6aa0b262dca51ffb8a1c) fix race at adding socket to IOCP
* [Commit cf10d7d3](https://github.com/manticoresoftware/manticoresearch/commit/cf10d7d3589f77adfaefbe52a51777bc9c67cf99) fix issue of bool queries vs json select list
* [Commit 996de77f](https://github.com/manticoresoftware/manticoresearch/commit/996de77f4c3ec103d965e28c36098fd07dba50c4) fix indextool check to report wrong skiplist offset, check of doc2row lookup
* [Commit 6e3fc9e8](https://github.com/manticoresoftware/manticoresearch/commit/6e3fc9e88941c9427410f9d0adeebb7dd09a900f) fix indexer produces bad index with negative skiplist offset on large data
* [Commit faed3220](https://github.com/manticoresoftware/manticoresearch/commit/faed3220b423a96401982bf47117edf1f62c584e) fix JSON converts only numeric to string and JSON string to numeric conversion at expressions
* [Commit 53319720](https://github.com/manticoresoftware/manticoresearch/commit/533197200249d648ebcbdaca5d605861ee566348) fix indextool exit with error code in case multiple commands set at command line
* [Commit 795520ac](https://github.com/manticoresoftware/manticoresearch/commit/795520ac351d22e4497ebaf6c249bf25081842b6) fix #275 binlog invalid state on error no space left on disk
* [Commit 2284da5e](https://github.com/manticoresoftware/manticoresearch/commit/2284da5e3ff77bf7be4038a4d950c0580e4159c8) fix #279 crash on IN filter to JSON attribute
* [Commit ce2e4b47](https://github.com/manticoresoftware/manticoresearch/commit/ce2e4b4765111a7d331ffc256911f12770a8942d) fix #281 wrong pipe closing call
* [Commit 535589ba](https://github.com/manticoresoftware/manticoresearch/commit/535589ba8a37baf5c6056afb5ff969fd70476feb) fix server hung at CALL PQ with recursive JSON attribute encoded as string
* [Commit a5fc8a36](https://github.com/manticoresoftware/manticoresearch/commit/a5fc8a36e1644ee051632408ab8b53e79aeb9ff4) fix advancing beyond the end of the doclist in multiand node
* [Commit a3628617](https://github.com/manticoresoftware/manticoresearch/commit/a3628617ef4ee89a811d4ba2c4a04cbbc298c1b5) fix retrieving of thread public info
* [Commit f8d2d7bb](https://github.com/manticoresoftware/manticoresearch/commit/f8d2d7bbbb90af3d2ee97424f97b05927f09d42b) fix docstore cache locks

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
* [Commit 301a806b1](https://github.com/manticoresoftware/manticoresearch/commit/301a806b14725e842300ee225521407464374c41) fix crash on replace document in disk chunk of RT index
* [Commit 46c1cad8f](https://github.com/manticoresoftware/manticoresearch/commit/46c1cad8fa11c1fc09cc34b20d978fdfcd4e6d27) fix \#269 LIMIT N OFFSET M
* [Commit 92a46edaa](https://github.com/manticoresoftware/manticoresearch/commit/92a46edaa196ef33f3ac2225fbdba2c2b88ee45d) fix DELETE statements with id explicitly set or id list provided to skip search
* [Commit 8ca78c138](https://github.com/manticoresoftware/manticoresearch/commit/8ca78c138d837caab30dc6e2343a3c4d1687bb87) fix wrong index after event removed at netloop at windowspoll poller
* [Commit 603631e2b](https://github.com/manticoresoftware/manticoresearch/commit/603631e2bff5cb0acf25f68b85621742490eccd9) fix float roundup at JSON via HTTP
* [Commit 62f64cb9e](https://github.com/manticoresoftware/manticoresearch/commit/62f64cb9e75ec28fc3c29f8414ce9656f5580254) fix remote snippets to check empty path first; fixing windows tests
* [Commit aba274c2c](https://github.com/manticoresoftware/manticoresearch/commit/aba274c2c5abc1029894ba9f95b786bda93e8f22) fix reload of config to work on windows same way as on linux
* [Commit 6b8c4242e](https://github.com/manticoresoftware/manticoresearch/commit/6b8c4242ef77c3fa4d0ccb7d76d81714b6728f0b) fix \#194 PQ to work with morphology and stemmers
* [Commit 174d31290](https://github.com/manticoresoftware/manticoresearch/commit/174d312905285aa5746f4f404c53e6dc669ef42f) fix RT retired segments management

## Version 3.1.2, 22 August 2019
### Features and Improvements
* Experimental SSL support for HTTP API
* field filter for CALL KEYWORDS
* max_matches for /json/search
* automatic sizing of default Galera gcache.size
* improved FreeBSD support

### Bugfixes
* [Commit 0a1a2c81](https://github.com/manticoresoftware/manticoresearch/commit/0a1a2c81038794983b4e30c8aefc7cc20e89b03f) fixed replication of RT index into node where same RT index exists and has different path
* [Commit 4adc0752](https://github.com/manticoresoftware/manticoresearch/commit/4adc075294ac823289f745e2cc419f18c7dcf2e2) fix flush rescheduling for indexes without activity
* [Commit d6c00a6f](https://github.com/manticoresoftware/manticoresearch/commit/d6c00a6f750c04021747e9c99e0f77707a4cbfc3) improve rescheduling of flushing RT/PQ indexes
* [Commit d0a7c959](https://github.com/manticoresoftware/manticoresearch/commit/d0a7c95906b9b5ba33c98267d54501cfe27508d6) fix \#250 index_field_lengths index option for TSV and CSV piped sources
* [Commit 1266d548](https://github.com/manticoresoftware/manticoresearch/commit/1266d54800242709fbe2c34e72eb69b6595b23bd) fix indextool wrong report for block index check on empty index
* [Commit 553ca73c](https://github.com/manticoresoftware/manticoresearch/commit/553ca73ca111f28e0888d59cd48b0d6df181ecd6) fix empty select list at Manticore SQL query log
* [Commit 56c85844](https://github.com/manticoresoftware/manticoresearch/commit/56c8584456293665ddd4ce4c94307d74473a78b9) fix indexer -h/--help response

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
* [Commit f0472223](https://github.com/manticoresoftware/manticoresearch/commit/f047222339fd0e62a07e9ca8b2a829297d5db861) fix defects in concat and group_concat
* [Commit b08147ee](https://github.com/manticoresoftware/manticoresearch/commit/b08147ee24712b5eaa51f261626b5d16c6242fdd) fix query uid at percolate index to be BIGINT attribute type
* [Commit 4cd85afa](https://github.com/manticoresoftware/manticoresearch/commit/4cd85afaf76eef0ce9fa1cd073f3fed160279890) do not crash if failed to prealloc a new disk chunk
* [Commit 1a551227](https://github.com/manticoresoftware/manticoresearch/commit/1a551227886474054111088412f2b2396f485652) add missing timestamp data type to ALTER
* [Commit f3a8e096](https://github.com/manticoresoftware/manticoresearch/commit/f3a8e096fd496a6b001d398aadbb781b2dbad929) fix crash of wrong mmap read
* [Commit 44757711](https://github.com/manticoresoftware/manticoresearch/commit/447577115192db7ac7bfea70b5ba58ee2c61229f) fix hash of clusters lock in replication
* [Commit ff476df9](https://github.com/manticoresoftware/manticoresearch/commit/ff476df955f057bd38e8b6ad6056325e9d6fbb9c) fix leak of providers in replication
* [Commit 58dcbb77](https://github.com/manticoresoftware/manticoresearch/commit/58dcbb779c4c30ca1560460c78e35ba114b83716) fix \#246 undefined sigmask in indexer
* [Commit 3dd8278e](https://github.com/manticoresoftware/manticoresearch/commit/3dd8278e747d534f69fe9c3d360eef188ab7b6d5) fix race in netloop reporting
* [Commit a02aae05](https://github.com/manticoresoftware/manticoresearch/commit/a02aae05a3fa3d1caf2821f43c7e237b6de76336) zero gap for HA strategies rebalancer

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
* [Commit 849c16e1](https://github.com/manticoresoftware/manticoresearch/commit/849c16e1040f382795ba0635cb48686f9db4f2d8) allow attribute names starting with numbers in select list
* [Commit 48e6c302](https://github.com/manticoresoftware/manticoresearch/commit/48e6c302ca37bec0bc49c79619c60bf787ed0708) fixed MVAs in UDFs, fixed MVA aliasing
* [Commit 055586a9](https://github.com/manticoresoftware/manticoresearch/commit/055586a9a2ebfa8874f68099c5bc798a145fd4ef) fixed \#187 crash when using query with SENTENCE
* [Commit 93bf52f2](https://github.com/manticoresoftware/manticoresearch/commit/93bf52f23f1c53c8a93a603387ca5463e1fc2ae8) fixed \#143 support () around MATCH()
* [Commit 599ee79c](https://github.com/manticoresoftware/manticoresearch/commit/599ee79ccd38cce0023a55cbce8f50c3dca62e38) fixed save of cluster state on ALTER cluster statement
* [Commit 230c321e](https://github.com/manticoresoftware/manticoresearch/commit/230c321e277514b191629538eb3f02d219113d95) fixed crash of server on ALTER index with blob attributes
* [Commit 5802b85a](https://github.com/manticoresoftware/manticoresearch/commit/5802b85aa93a81caea77073a3381ef912fdd677c) fixed \#196 filtering by id
* [Commit 25d2dabd](https://github.com/manticoresoftware/manticoresearch/commit/25d2dabda4a299a29bd375405862126da9b4bfac) discard searching on template indexes
* [Commit 2a30d5b4](https://github.com/manticoresoftware/manticoresearch/commit/2a30d5b49d76faab031d195f533974938769b8e6) fixed id column to have regular bigint type at SQL reply


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
* [Commit 6967fedb](https://github.com/manticoresoftware/manticoresearch/commit/6967fedb2ef818ec1c825d482563edd05e1c9245) fixed crash on many stars at select list for query into many distributed indexes
* [Commit 36df1a40](https://github.com/manticoresoftware/manticoresearch/commit/36df1a407dc08263690e3492518613ace82d69ca) fixed [\#177](https://github.com/manticoresoftware/manticoresearch/issues/177) large packet via Manticore SQL interface
* [Commit 57932aec](https://github.com/manticoresoftware/manticoresearch/commit/57932aec734583fa93359faaf5034b2e2c9d352b) fixed [\#170](https://github.com/manticoresoftware/manticoresearch/issues/170) crash of server on RT optimize with MVA updated
* [Commit edb24b87](https://github.com/manticoresoftware/manticoresearch/commit/edb24b870423add86eba471d361e0e5aff098b18) fixed server crash on binlog removed due to RT index remove after config reload on SIGHUP
* [Commit bd3e66e0](https://github.com/manticoresoftware/manticoresearch/commit/bd3e66e0085bc7e2e351b817dfc972fd8158fbce) fixed mysql handshake auth plugin payloads
* [Commit 6a217f6e](https://github.com/manticoresoftware/manticoresearch/commit/6a217f6ed82fb10f752213b15617b9cedf1e8533) fixed [\#172](https://github.com/manticoresoftware/manticoresearch/issues/172) phrase_boundary settings at RT index
* [Commit 3562f652](https://github.com/manticoresoftware/manticoresearch/commit/3562f652753e4091fd7b5b0f65b58341c8cbcb31) fixed [\#168](https://github.com/manticoresoftware/manticoresearch/issues/168) deadlock at ATTACH index to itself
* [Commit 250b3f0e](https://github.com/manticoresoftware/manticoresearch/commit/250b3f0e74ad18dda34ba080aa13ca87798a9ac9) fixed binlog saves empty meta after server crash
* [Commit 4aa6c69a](https://github.com/manticoresoftware/manticoresearch/commit/4aa6c69ad3a181cd7c5dafb990fb528121f68e59) fixed crash of server due to string at sorter from RT index with disk chunks

## Version 2.8.1 GA, 6 March 2019
### Features and improvements
* SUBSTRING_INDEX()
* SENTENCE and PARAGRAPH support for percolate queries
* systemd generator for Debian/Ubuntu; also added LimitCORE to allow core dumping

### Bugfixes
* [Commit 84fe7405](https://github.com/manticoresoftware/manticoresearch/commit/84fe7405d17b59f4dcde15a6c8e8503923503def) fixed crash of server on match mode all and empty full text query
* [Commit daa88b57](https://github.com/manticoresoftware/manticoresearch/commit/daa88b579fa373d5d2e869e43a2e178363b0fef1) fixed crash on deleting of static string
* [Commit 22078537](https://github.com/manticoresoftware/manticoresearch/commit/22078537dddbd9ce81a182dcc879f235b6e87004) fixed exit code when indextool failed with FATAL
* [Commit 0721696d](https://github.com/manticoresoftware/manticoresearch/commit/0721696d6780c200c65d596624a8187983fb7fcb) fixed [\#109](https://github.com/manticoresoftware/manticoresearch/issues/109) no matches for prefixes due to wrong exact form check
* [Commit 8af81011](https://github.com/manticoresoftware/manticoresearch/commit/8af810111b8e2f87bc9f378172eff1ab9725c7e7) fixed [\#161](https://github.com/manticoresoftware/manticoresearch/issues/161) reload of config settings for RT indexes
* [Commit e2d59277](https://github.com/manticoresoftware/manticoresearch/commit/e2d592773f4fd702ace90070a793a842a0fec6ab) fixed crash of server on access of large JSON string
* [Commit 75cd1342](https://github.com/manticoresoftware/manticoresearch/commit/75cd1342d05cdc352e3dd145cdbc6f79394a165b) fixed PQ field at JSON document altered by index stripper causes wrong match from sibling field
* [Commit e2f77543](https://github.com/manticoresoftware/manticoresearch/commit/e2f775437df922674d772ed4417780492502e65a) fixed crash of server at parse JSON on RHEL7 builds
* [Commit 3a25a580](https://github.com/manticoresoftware/manticoresearch/commit/3a25a5808feb3f8b80866991b436d6c1241618c2) fixed crash of json unescaping when slash is on the edge
* [Commit be9f4978](https://github.com/manticoresoftware/manticoresearch/commit/be9f497872bea62dbdccd64fc28294fd7776c289) fixed option 'skip_empty' to skip empty docs and not warn they're not valid json
* [Commit 266e0e7b](https://github.com/manticoresoftware/manticoresearch/commit/266e0e7b088549722a805fc837bf101ff681a5e8) fixed [\#140](https://github.com/manticoresoftware/manticoresearch/issues/161) output 8 digits on floats when 6 is not enough to be precise
* [Commit 3f6d2389](https://github.com/manticoresoftware/manticoresearch/commit/3f6d23891064ee8e94030c4231497cdd7da33a6a) fixed empty jsonobj creation
* [Commit f3c7848a](https://github.com/manticoresoftware/manticoresearch/commit/f3c7848a59b7eedc67c3403d6d4b37ce6fa94dc8) fixed [\#160](https://github.com/manticoresoftware/manticoresearch/issues/161) empty mva outputs NULL instead of an empty string
* [Commit 0afa2ed0](https://github.com/manticoresoftware/manticoresearch/commit/0afa2ed058e5759470b1d7354c722faab34f98bb) fixed fail to build without pthread_getname_np
* [Commit 9405fccd](https://github.com/manticoresoftware/manticoresearch/commit/9405fccdeb27f0302a8d9a848981d30080216777) fixed crash on server shutdown with thread_pool workers

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
* [Commit a4e19af](https://github.com/manticoresoftware/manticoresearch/commit/a4e19afee54dafdc04b0da53372dafd8c0d332d6) fixed csjon-related leak
* [Commit 28d8627](https://github.com/manticoresoftware/manticoresearch/commit/28d862774874891a03e361da1b0347ebe8066ce0) fixed crash because of missed value in json
* [Commit bf4e9ea](https://github.com/manticoresoftware/manticoresearch/commit/bf4e9ea27c349cdc9ae4e54e960a4a17b90e38fa) fixed save of empty meta for RT index
* [Commit 33b4573](https://github.com/manticoresoftware/manticoresearch/commit/33b4573529e7c7c7bce19bf9d54054866f30d99a) fixed lost form flag (exact) for sequence of lemmatizer
* [Commit 6b95d48](https://github.com/manticoresoftware/manticoresearch/commit/6b95d48240d3b5520afade19c249d79e020a5e88) fixed string attrs > 4M use saturate instead of overflow
* [Commit 621418b](https://github.com/manticoresoftware/manticoresearch/commit/621418b829e70af36aaa322a2f51ece3f86bc499) fixed crash of server on SIGHUP with disabled index
* [Commit 3f7e35d](https://github.com/manticoresoftware/manticoresearch/commit/3f7e35d1482966cc45d5abbcb2de5de3508d66b0) fixed server crash on simultaneous API session status commands
* [Commit cd9e4f1](https://github.com/manticoresoftware/manticoresearch/commit/cd9e4f1709a48ddafc128c450c2d882bc11ba50e) fixed crash of server at delete query to RT index with field filters
* [Commit 9376470](https://github.com/manticoresoftware/manticoresearch/commit/9376470d455fcd256c6abff79c431a6919dfa7ac) fixed crash of server at CALL PQ to distributed index with empty document
* [Commit 8868b20](https://github.com/manticoresoftware/manticoresearch/commit/8868b207644f4f8cc1ab0c270adb35493fed22e1) fixed cut Manticore SQL error message larger 512 chars
* [Commit de9deda](https://github.com/manticoresoftware/manticoresearch/commit/de9deda9c142823d6dbf529423417b5c670fae94) fixed crash on save percolate index without binlog
* [Commit 2b219e1](https://github.com/manticoresoftware/manticoresearch/commit/2b219e1a32791e7740bb210b7d408f96abc6374f) fixed http interface is not working in OSX
* [Commit e92c602](https://github.com/manticoresoftware/manticoresearch/commit/e92c60240f8f9e1756bfa14fba0bbb402586a389) fixed indextool false error message on check of MVA
* [Commit 238bdea](https://github.com/manticoresoftware/manticoresearch/commit/238bdea59bad89f097403f1c978658ce45f16c70) fixed write lock at FLUSH RTINDEX to not write lock whole index during save and on regular flush from rt_flush_period
* [Commit c26a236](https://github.com/manticoresoftware/manticoresearch/commit/c26a236bd42b7082db079a51a84172066d337d64) fixed ALTER percolate index stuck waiting search load
* [Commit 9ee5703](https://github.com/manticoresoftware/manticoresearch/commit/9ee5703f6c28f57cd5bff5e705966d93fe30d267) fixed max_children to use default amount of thread_pool workers for value of 0
* [Commit 5138fc0](https://github.com/manticoresoftware/manticoresearch/commit/5138fc048a1bd146b271ce6a72fe954344281f69) fixed error on indexing of data into index with index_token_filter plugin along with stopwords and stopword_step=0
* [Commit 2add3d3](https://github.com/manticoresoftware/manticoresearch/commit/2add3d319ac62fe450bf60e89033100853dc2ecf) fixed crash with absent lemmatizer_base when still using aot lemmatizers in index definitions

## Version 2.7.5 GA, 4 December 2018
### Improvements
* REGEX function
* limit/offset for json API search
* profiler points for qcache

### Bugfixes
* [Commit eb3c768](https://github.com/manticoresoftware/manticoresearch/commit/eb3c7683b9740cff4cc4cadd2ab9f990b9f7d173) fixed crash of server on FACET with multiple attribute wide types
* [Commit d915cf6](https://github.com/manticoresoftware/manticoresearch/commit/d915cf6696514f4be365ffb5981bccaa15983e65) fixed implicit group by at main select list of FACET query
* [Commit 5c25dc2](https://github.com/manticoresoftware/manticoresearch/commit/5c25dc271a8c11a1c82ca3c88e518acbc6a8c164) fixed crash on query with GROUP N BY
* [Commit 85d30a2](https://github.com/manticoresoftware/manticoresearch/commit/85d30a232d4ffd89916b673a9b2db946d272a945) fixed deadlock on handling crash at memory operations
* [Commit 85166b5](https://github.com/manticoresoftware/manticoresearch/commit/85166b5ef2be38061d62f2b20f15acaa0311258a) fixed indextool memory consumption during check
* [Commit 58fb031](https://github.com/manticoresoftware/manticoresearch/commit/58fb03166830d8b23e7ab9ce56309db7ddac22f4) fixed gmock include not needed anymore as upstream resolve itself

## Version 2.7.4 GA, 1 November 2018
### Improvements
* SHOW THREADS in case of remote distributed indexes prints the original query instead of API call
* SHOW THREADS new option `format=sphinxql` prints all queries in SQL format
* SHOW PROFILE prints additional `clone_attrs` stage

### Bugfixes
* [Commit 4f15571](https://github.com/manticoresoftware/manticoresearch/commit/4f155712a0bccc1bd01cc191647bc8cff814888e) fixed failed to build with libc without malloc_stats, malloc_trim
* [Commit f974f20](https://github.com/manticoresoftware/manticoresearch/commit/f974f20bda3214a56877c393a192be1a77150958) fixed special symbols inside words for CALL KEYWORDS result set
* [Commit 0920832](https://github.com/manticoresoftware/manticoresearch/commit/092083282ea8ae0b2e72fb6989c57ccec81e74ac) fixed broken CALL KEYWORDS to distributed index via API or to remote agent
* [Commit fd686bf](https://github.com/manticoresoftware/manticoresearch/commit/fd686bfe88b720ffd7642e36897ba45161cbd7d2) fixed distributed index agent_query_timeout propagate to agents as max_query_time
* [Commit 4ffa623](https://github.com/manticoresoftware/manticoresearch/commit/4ffa623e9d357a3b0b441615089e211f92f8de32) fixed total documents counter at disk chunk got affected by OPTIMIZE command and breaks weight calculation
* [Commit dcaf4e0](https://github.com/manticoresoftware/manticoresearch/commit/dcaf4e0e3c8e9b0fbec47bbf3307feddec0936a6) fixed multiple tail hits at RT index from blended
* [Commit eee3817](https://github.com/manticoresoftware/manticoresearch/commit/eee381754e902a43a8f499e1c950198a2a3e6ee0) fixed deadlock at rotation

## Version 2.7.3 GA, 26 September 2018
### Improvements
* sort_mode option for CALL KEYWORDS
* DEBUG on VIP connection can perform 'crash <password>' for intentional SIGEGV action on server
* DEBUG can perform 'malloc_stats' for dumping malloc stats in searchd.log 'malloc_trim' to perform a malloc_trim()
* improved backtrace is gdb is present on the system

### Bugfixes
* [Commit 0f3cc33](https://github.com/manticoresoftware/manticoresearch/commit/0f3cc333f6129451ad1ae2c97905b6c04ba5d454) fixed crash or hfailure of rename on Windows
* [Commit 1455ba2](https://github.com/manticoresoftware/manticoresearch/commit/1455ba2bf8079f03ce7f861a8d9662a360d705de) fixed crashes of server on 32-bit systems
* [Commit ad3710d](https://github.com/manticoresoftware/manticoresearch/commit/ad3710d53b5cd6a28b8c60352d0f9e285b03d9fa) fixed crash or hung of server on empty SNIPPET expression
* [Commit b36d792](https://github.com/manticoresoftware/manticoresearch/commit/b36d79214364500252665860f000ae85343528b6) fixed broken non progressive optimize and fixed progressive optimize to not create kill-list for oldest disk chunk
* [Commit 34b0324](https://github.com/manticoresoftware/manticoresearch/commit/34b032499afd42ce47a4c7247814b4031094388a) fixed queue_max_length bad reply for SQL and API at thread pool worker mode
* [Commit ae4b320](https://github.com/manticoresoftware/manticoresearch/commit/ae4b3202cbdb8014cabe2b90e269d5cf74f49871) fixed crash on adding full-scan query to PQ index with regexp or rlp options set
* [Commit f80f8d5](https://github.com/manticoresoftware/manticoresearch/commit/f80f8d5d7560187078868aed9a9575f4549e98aa) fixed crash when call one PQ after another
* [Commit 9742f5f](https://github.com/manticoresoftware/manticoresearch/commit/9742f5f0866af73f8cd8483ecd18a507ea80dd65) refactor AcquireAccum
* [Commit 39e5bc3](https://github.com/manticoresoftware/manticoresearch/commit/39e5bc3751b7295222eb76407c5d72ce1dad545b) fixed leak of memory after call pq
* [Commit 21bcc6d](https://github.com/manticoresoftware/manticoresearch/commit/21bcc6d17395f0f57dde79f6716ef303b7ea527d) cosmetic refactor (c++11 style c-trs, defaults, nullptrs)
* [Commit 2d69039](https://github.com/manticoresoftware/manticoresearch/commit/2d690398f14c736956cfdd66feb4d3091d6b3a4d) fixed memory leak on trying to insert duplicate into PQ index
* [Commit 5ed92c4](https://github.com/manticoresoftware/manticoresearch/commit/5ed92c4b66da6423fa28f11b1fd59103ed1e1fb9) fixed crash on JSON field IN with large values
* [Commit 4a5262e](https://github.com/manticoresoftware/manticoresearch/commit/4a5262e2776aa261e34ba5c914ea60804f59de3f) fixed crash of server on CALL KEYWORDS statement to RT index with expansion limit set
* [Commit 552646b](https://github.com/manticoresoftware/manticoresearch/commit/552646bb6fefde4e2525298e43e628839b421d3d) fixed invalid filter at PQ matches query;
* [Commit 204f521](https://github.com/manticoresoftware/manticoresearch/commit/204f521408b730198a5bab2c20407a3d7df94873) introduce small obj allocator for ptr attrs
* [Commit 25453e5](https://github.com/manticoresoftware/manticoresearch/commit/25453e5387152c0575d1eda1b2beb89a49c7493a) refactor ISphFieldFilter to refcounted flavour
* [Commit 1366ee0](https://github.com/manticoresoftware/manticoresearch/commit/1366ee06e828397074b69bec9265d7147170c60e) fixed ub/sigsegv when using strtod on non-terminated strings
* [Commit 94bc6fc](https://github.com/manticoresoftware/manticoresearch/commit/94bc6fcbdae4d08bcf6da2966e3ec2966091289f) fixed memory leak in json resultset processing
* [Commit e78e9c9](https://github.com/manticoresoftware/manticoresearch/commit/e78e9c948963416caff9e4e46296a58080107835) fixed read over the end of mem block applying attribute add
* [Commit fad572f](https://github.com/manticoresoftware/manticoresearch/commit/fad572fb543606b33e688eb82a485b86011545fd) fixed refactor CSphDict for refcount flavour
* [Commit fd841a4](https://github.com/manticoresoftware/manticoresearch/commit/fd841a472b472280c93f4af3506851d19f6adc45) fixed leak of AOT internal type outside
* [Commit 5ee7f20](https://github.com/manticoresoftware/manticoresearch/commit/5ee7f208c7685f2ebb9e5623a8802b721e2a7f0a) fixed memory leak tokenizer management
* [Commit 116c5f1](https://github.com/manticoresoftware/manticoresearch/commit/116c5f1abebee9a0d99afe93546a1d8e4c6c6958) fixed memory leak in grouper
* [Commit 56fdbc9](https://github.com/manticoresoftware/manticoresearch/commit/56fdbc9ab38973a9a0ba8ccee45f71cf33c16423) special free/copy for dynamic ptrs in matches (memory leak grouper)
* [Commit b1fc161](https://github.com/manticoresoftware/manticoresearch/commit/31db660f378541aa0d1e7cf2e7209bb0a1f47fd8) fixed memory leak of dynamic strings for RT
* [Commit 517b9e8](https://github.com/manticoresoftware/manticoresearch/commit/517b9e8749f092e923cd2884b674b5bb84e20755) refactor grouper
* [Commit b1fc161](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) minor refactor (c++11 c-trs, some reformats)
* [Commit 7034e07](https://github.com/manticoresoftware/manticoresearch/commit/7034e073f4cf2844762e0a464b7c3de05d3d122b) refactor ISphMatchComparator to refcounted flavour
* [Commit b1fc161](https://github.com/manticoresoftware/manticoresearch/commit/b1fc16140e5dc44290686330b476a254e0092107) privatize cloner
* [Commit efbc051](https://github.com/manticoresoftware/manticoresearch/commit/efbc0511d6809c4a57453c7283f9bf53f3fb8d97) simplify native little-endian for MVA_UPSIZE, DOCINFO2ID_T, DOCINFOSETID
* [Commit 6da0df4](https://github.com/manticoresoftware/manticoresearch/commit/6da0df4ac9946fc59f98d229b90e12c9221dd4c0) add valgrind support to to ubertests
* [Commit 1d17669](https://github.com/manticoresoftware/manticoresearch/commit/1d17669cb7ec54ac80b392e101f37a688eb98080) fixed crash because race of 'success' flag on connection
* [Commit 5a09c32](https://github.com/manticoresoftware/manticoresearch/commit/5a09c32d7ded56b2c0bd3e2ad7968cb8d6362919) switch epoll to edge-triggered flavour
* [Commit 5d52868](https://github.com/manticoresoftware/manticoresearch/commit/5d528682737fca03a4352a3093b1319ec27d598c) fixed IN statement in expression with formatting like at filter
* [Commit bd8b3c9](https://github.com/manticoresoftware/manticoresearch/commit/bd8b3c976ff8b4667f55dfdd101b20a920137ac5) fixed crash at RT index on commit of document with large docid
* [Commit ce656b8](https://github.com/manticoresoftware/manticoresearch/commit/ce656b83b747ce7c55795b53915770c13fb99cfe) fixed argless options in indextool
* [Commit 08c9507](https://github.com/manticoresoftware/manticoresearch/commit/08c9507177820f441f534bf06a11dac5e54bebb4) fixed memory leak of expanded keyword
* [Commit 30c75a2](https://github.com/manticoresoftware/manticoresearch/commit/30c75a2f525ec9bda625d903acdc9d4d2e2413dc) fixed memory leak of json grouper
* [Commit 6023f26](https://github.com/manticoresoftware/manticoresearch/commit/6023f269b6f2080e4d380b0957605ef8107c8c9f) fixed leak of global user vars
* [Commit 7c138f1](https://github.com/manticoresoftware/manticoresearch/commit/7c138f15ca23c0c8717fa12041e3db7f988fcf15) fixed leakage of dynamic strings on early rejected matches
* [Commit 9154b18](https://github.com/manticoresoftware/manticoresearch/commit/9154b18eaed5302e8330cb3a95bd968959ccb312) fixed leakage on length(<expression>)
* [Commit 43fca3a](https://github.com/manticoresoftware/manticoresearch/commit/43fca3a4e26139765d0fac8de054200dc4875fc6) fixed memory leak because strdup() in parser
* [Commit 71ff777](https://github.com/manticoresoftware/manticoresearch/commit/71ff77737e63d1019b7c9804dca7fa2351025dba) fixed refactor expression parser to accurate follow refcounts

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
* [Commit 390082](https://github.com/manticoresoftware/manticoresearch/commit/390082a7be0a1f9539b30361d11d54de35c62a44) removed wlock at optimize
* [Commit 4c3376](https://github.com/manticoresoftware/manticoresearch/commit/4c337648329be22373be33333cbc9c5664d18b80) fixed wlock at reload index settings
* [Commit b5ea8d](https://github.com/manticoresoftware/manticoresearch/commit/b5ea8dc0b18a23dbd739e92c66dd93b0094df14b) fixed memory leak on query with JSON filter
* [Commit 930e83](https://github.com/manticoresoftware/manticoresearch/commit/930e831a43fd3d1ce996185be968d1df49a088d0) fixed empty documents at PQ result set
* [Commit 53deec](https://github.com/manticoresoftware/manticoresearch/commit/53deecfecb374431f84f8592a1095a77407b8aea) fixed confusion of tasks due to removed one
* [Commit cad9b9](https://github.com/manticoresoftware/manticoresearch/commit/cad9b9c617719aad97e9f5a2ae93491153d8e504) fixed wrong remote host counting
* [Commit 90008c](https://github.com/manticoresoftware/manticoresearch/commit/90008cc15c4684d445d37a391bb4a667bbc2e365) fixed memory leak of parsed agent descriptors
* [Commit 978d83](https://github.com/manticoresoftware/manticoresearch/commit/978d839641ad6040c22c9fc3b703b1a02685f266) fixed leak in search
* [Commit 019394](https://github.com/manticoresoftware/manticoresearch/commit/0193946105ac08913362b7d7acf0c90f9b38dc85) cosmetic changes on explicit/inline c-trs, override/final usage
* [Commit 943e29](https://github.com/manticoresoftware/manticoresearch/commit/943e2997b33f9b2eda23d94a66068910f9476ebd) fixed leak of json in local/remote schema
* [Commit 02dbdd](https://github.com/manticoresoftware/manticoresearch/commit/02dbdd6f1d0b72e7e458ebebdd2c67f989577e6b) fixed leak of json sorting col expr in local/remote schema
* [Commit c74d0b](https://github.com/manticoresoftware/manticoresearch/commit/c74d0b4035e4214510376845b22cb676a8da24a3) fixed leak of const alias
* [Commit 6e5b57](https://github.com/manticoresoftware/manticoresearch/commit/6e5b57e1d3952c9695376728a143bf5434208f53) fixed leak of preread thread
* [Commit 39c740](https://github.com/manticoresoftware/manticoresearch/commit/39c740fef5bf71da0bee60a35f5dd9f471af850f) fixed stuck on exit because of stucked wait in netloop
* [Commit adaf97](https://github.com/manticoresoftware/manticoresearch/commit/adaf9772f95772ad6d5297acace6c5cb92b56ad5) fixed stuck of 'ping' behaviour on change HA agent to usual host
* [Commit 32c40e](https://github.com/manticoresoftware/manticoresearch/commit/32c40eac9cdd40b15d79e58b2f15416f164f9701) separate gc for dashboard storage
* [Commit 511a3c](https://github.com/manticoresoftware/manticoresearch/commit/511a3cf2c1f1858f6e94fe9f8175b7216db3cbd6) fixed ref-counted ptr fix
* [Commit 32c40e](https://github.com/manticoresoftware/manticoresearch/commit/af2326cd2927effbad7bec88e8dc238a566cf27e) fixed indextool crash on unexistent index
* [Commit 156edc](https://github.com/manticoresoftware/manticoresearch/commit/156edc717dc63d3ea120f42466eba6e022da88be) fixed output name of exceeding attr/field in xmlpipe indexing
* [Commit cdac6d](https://github.com/manticoresoftware/manticoresearch/commit/cdac6d1f17d384bfc8bae49e91241b2f346c3830) fixed default indexer's value if no indexer section in config
* [Commit e61ec0](https://github.com/manticoresoftware/manticoresearch/commit/e61ec00b6b27d1d5878247e2ee817f3b1e7fde16) fixed wrong embedded stopwords in disk chunk by RT index after server restart
* [Commit 5fba49](https://github.com/manticoresoftware/manticoresearch/commit/5fba49d5e28c17de4c0acbd984466127f42de6e8) fixed skip phantom (already closed, but not finally deleted from the poller) connections
* [Commit f22ae3](https://github.com/manticoresoftware/manticoresearch/commit/f22ae34b623906b3c5a05a06198dad2e548dc541) fixed blended (orphaned) network tasks
* [Commit 46890e](https://github.com/manticoresoftware/manticoresearch/commit/46890e70eb8dcd00c1bf1b030538b8f131c601c2) fixed crash on read action after write
* [Commit 03f9df](https://github.com/manticoresoftware/manticoresearch/commit/03f9df904f611c438d5ebcaaeef911b0dc8d6c39) fixed searchd crashes when running tests on windows
* [Commit e9255e](https://github.com/manticoresoftware/manticoresearch/commit/e9255ed2704790a3d7f5273d38b85433463cb3ff) fixed handle EINPROGRESS code on usual connect()
* [Commit 248b72](https://github.com/manticoresoftware/manticoresearch/commit/248b72f1544b27e9e82773284050d44eeb1714e1) fixed connection timeouts when working with TFO

## Version 2.7.1 GA, 4 July 2018
### Improvements
* improved wildcards performance on matching multiple documents at PQ
* support for fullscan queries at PQ
* support for MVA attributes at PQ
* regexp and RLP support for percolate indexes

### Bugfixes
* [Commit 688562](https://github.com/manticoresoftware/manticoresearch/commit/68856261b41f6950666f9e5122726839fb3c71d0) fixed loose of query string
* [Commit 0f1770](https://github.com/manticoresoftware/manticoresearch/commit/0f1770943cba53bfd2e0edfdf0b1495ff0dd0e89) fixed empty info at SHOW THREADS statement
* [Commit 53faa3](https://github.com/manticoresoftware/manticoresearch/commit/53faa36a916bb87868e83aa79c1a1f972fb20ca0) fixed crash on matching with NOTNEAR operator
* [Commit 26029a](https://github.com/manticoresoftware/manticoresearch/commit/26029a7cd77b518fb3a27588d7db6c8bffc73b0f) fixed error message on bad filter to PQ delete


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
* [Commit 5bcff0](https://github.com/manticoresoftware/manticoresearch/commit/5bcff05d19e495f1a44f190885b25cbb6f217c4a) fixed added not equal to tags of PQ
* [Commit 9ebc58](https://github.com/manticoresoftware/manticoresearch/commit/9ebc58916cd515eaa88da66d0895aebf0d1f2b5f) fixed added document id field to JSON document CALL PQ statement
* [Commit 8ae0e5](https://github.com/manticoresoftware/manticoresearch/commit/8ae0e593a286110d8505d88baa3ac9e433cb38c9) fixed flush statement handlers to PQ index
* [Commit c24b15](https://github.com/manticoresoftware/manticoresearch/commit/c24b152344ecd77661566cddb803487f51d3c1aa) fixed PQ filtering on JSON and string attributes
* [Commit 1b8bdd](https://github.com/manticoresoftware/manticoresearch/commit/1b8bdde19aaa362785ea4c33bdc019154f8fbac4) fixed parsing of empty JSON string
* [Commit 1ad8a0](https://github.com/manticoresoftware/manticoresearch/commit/1ad8a07dbfc1131913a8d92c261fbb48f934e8b7) fixed crash at multi-query with OR filters
* [Commit 69b898](https://github.com/manticoresoftware/manticoresearch/commit/69b89806380a229e36287ad9daf6f0ea2b5453eb) fixed indextool to use config common section (lemmatizer_base option) for commands (dumpheader)
* [Commit 6dbeaf](https://github.com/manticoresoftware/manticoresearch/commit/6dbeaf2389d2af6a9dfccb56bfc986685b9f203e) fixed empty string at result set and filter
* [Commit 39c4eb](https://github.com/manticoresoftware/manticoresearch/commit/39c4eb55ecc85f6dd54f6c8c6d6dfcf87fd8748e) fixed negative document id values
* [Commit 266b70](https://github.com/manticoresoftware/manticoresearch/commit/266b7054f98e7d30fc2829b5c2467dc9f90def92) fixed word clip length for very long words indexed
* [Commit 47823b](https://github.com/manticoresoftware/manticoresearch/commit/47823b0bd06cbd016a24345c99bb8693790b43dd) fixed matching multiple documents of wildcard queries at PQ


## Version 2.6.4 GA, 3 May 2018
### Features and improvements
* MySQL FEDERATED engine [support](Extensions/FEDERATED.md)
* MySQL packets return now SERVER_STATUS_AUTOCOMMIT flag, adds compatibility with ProxySQL
* [listen_tfo](Server_settings/Searchd.md#listen_tfo) - enable TCP Fast Open connections for all listeners
* indexer --dumpheader can dump also RT header from .meta file
* cmake build script for Ubuntu Bionic

### Bugfixes
* [Commit 355b116](https://github.com/manticoresoftware/manticoresearch/commit/355b11629174813abd9cd5bf2233be0783f77745) fixed invalid query cache entries for RT index;
* [Commit 546e229](https://github.com/manticoresoftware/manticoresearch/commit/546e2297a01cc2913bc0d33052d5bcefae8737eb) fixed index settings got lost next after seamless rotation
* [Commit 0c45098](https://github.com/manticoresoftware/manticoresearch/commit/0c4509898393993bc87194d2aca2070395ff7f83) fixed fixed infix vs prefix length set; added warning on unsupportedinfix length
* [Commit 80542fa](https://github.com/manticoresoftware/manticoresearch/commit/80542fa54d727c781635918e8d4d65a20c2fbd1e) fixed RT indexes auto-flush order
* [Commit 705d8c5](https://github.com/manticoresoftware/manticoresearch/commit/705d8c5f407726ed1c6f9745ecffa375682c8969) fixed result set schema issues for index with multiple attributes and queries to multiple indexes
* [Commit b0ba932](https://github.com/manticoresoftware/manticoresearch/commit/b0ba932fff4a082f642cb737381c8a70215d4cc9) fixed some hits got lost at batch insert with document duplicates
* [Commit 4510fa4](https://github.com/manticoresoftware/manticoresearch/commit/4510fa44ad54b5bbac53c591144501565130fcae) fixed optimize failed to merge disk chunks of RT index with large documents count

## Version 2.6.3 GA, 28 March 2018
### Improvements
* jemalloc at compilation. If jemalloc is present on system, it can be enabled with cmake flag `-DUSE_JEMALLOC=1`

### Bugfixes
* [Commit 85a6d7e](https://github.com/manticoresoftware/manticoresearch/commit/85a6d7e3db75f28c491042409bb677ed39eba07b) fixed log expand_keywords option into Manticore SQL query log
* [Commit caaa384](https://github.com/manticoresoftware/manticoresearch/commit/caaa3842f4dbd8507fd9e20bf3371a0c8ec0c087) fixed HTTP interface to correctly process query with large size
* [Commit e386d84](https://github.com/manticoresoftware/manticoresearch/commit/e386d842b36195719e459b2db1fb20e402f9a51f) fixed crash of server on DELETE to RT index with index_field_lengths enable
* [Commit cd538f3](https://github.com/manticoresoftware/manticoresearch/commit/ce528f37eb5d8284661928f6f3b340a4a1985253) fixed cpustats searchd cli option to work with unsupported systems
* [Commit 8740fd6](https://github.com/manticoresoftware/manticoresearch/commit/8740fd63ae842c2ab0e51a8fcd6180d787ed57ec) fixed utf8 substring matching with min lengths defined


## Version 2.6.2 GA, 23 February 2018
### Improvements
* improved [Percolate Queries](Searching/Percolate_query.md) performance in case of using NOT operator and for batched documents.
* [percolate_query_call](Searching/Percolate_query.md) can use multiple threads depending on [dist_threads](Server_settings/Searchd.md#threads)
* new full-text matching operator NOTNEAR/N
* LIMIT for SELECT on percolate indexes
* [expand_keywords](Searching/Options.md#expand_keywords) can accept 'start','exact' (where 'star,exact' has same effect as '1')
* ranged-main-query for  [joined fields](Adding_data_from_external_storages/Fetching_from_databases/Processing_fetched_data.md#sql_joined_field) which uses the ranged query defined by sql_query_range

### Bugfixes
* [Commit 72dcf66](https://github.com/manticoresoftware/manticoresearch/commit/72dcf669744e9b7d636dfc213d24df85ab301f6b) fixed crash on searching ram segments; deadlock on save disk chunk with double buffer; deadlock on save disk chunk during optimize
* [Commit 3613714](https://github.com/manticoresoftware/manticoresearch/commit/36137149a1c3c0893bdda5a28fc7e8244bf2d4ae) fixed indexer crash on xml embedded schema with empty attribute name
* [Commit 48d7e80](https://github.com/manticoresoftware/manticoresearch/commit/48d7e8001d2a66466ca64577f27ddc5421a67251) fixed erroneous unlinking of not-owned pid-file
* [Commit a5563a4](https://github.com/manticoresoftware/manticoresearch/commit/a5563a465ddc59ef71e65f17b68bc33f9700e838) fixed orphaned fifos sometimes left in temp folder
* [Commit 2376e8f](https://github.com/manticoresoftware/manticoresearch/commit/2376e8fc4508944b96959bd10686c6d51f5145e8) fixed empty FACET result set with wrong NULL row
* [Commit 4842b67](https://github.com/manticoresoftware/manticoresearch/commit/4842b67acdd5da75672db6c08ac563b48577ddd7) fixed broken index lock when running server as windows service
* [Commit be35fee](https://github.com/manticoresoftware/manticoresearch/commit/be35feef54ded59125750916cd211e56108eddae) fixed wrong iconv libs on mac os
* [Commit 83744a9](https://github.com/manticoresoftware/manticoresearch/commit/83744a977d7837d1d87cd506b88b0e7eb199efe6) fixed wrong count(\*)


## Version 2.6.1 GA, 26 January 2018
### Improvements
* [agent_retry_count](Server_settings/Searchd.md#agent_retry_count) in case of agents with mirrors gives the value of retries per mirror instead of per agent, the total retries per agent being agent_retry_count\*mirrors.
* [agent_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) can now be specified per index, overriding global value. An alias [mirror_retry_count](Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count) is added.
* a retry_count can be specified in agent definition and the value represents retries per agent
* Percolate Queries are now in HTTP JSON API at [/json/pq](Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
* Added -h and -v options (help and version) to executables
* [morphology_skip_fields](Creating_a_table/NLP_and_tokenization/Morphology.md#morphology) support for Real-Time indexes

### Bugfixes
* [Commit a40b079](https://github.com/manticoresoftware/manticore/commit/a40b0793feff65e40d10062568d9847c08d10f57) fixed ranged-main-query to correctly work with sql_range_step when used at MVA field
* [Commit f2f5375](https://github.com/manticoresoftware/manticore/commit/f2f53757db45bcfb1544263ce0817e856656a621) fixed issue with blackhole system loop hung and blackhole agents seems disconnected
* [Commit 84e1f54](https://github.com/manticoresoftware/manticore/commit/84e1f54aef25e0fce98870ad2dd784db5116f1d6) fixed query id to be consistent, fixed duplicated id for stored queries
* [Commit 1948423](https://github.com/manticoresoftware/manticore/commit/19484231814fcb82b21763a3a4a9f45adc6b2d40) fixed server crash on shutdown from various states
* [Commit 9a706b](https://github.com/manticoresoftware/manticore/commit/9a706b499a1d61a90076065a1a703029d49db958)  [Commit 3495fd7](https://github.com/manticoresoftware/manticore/commit/3495fd70cba8846b1a50d55d6679b039414c3d2a) timeouts on long queries
* [Commit 3359bcd8](https://github.com/manticoresoftware/manticore/commit/3359bcd89b4f79a645fe84b8cf8616ce0addff02) refactored master-agent network polling on kqueue-based systems (Mac OS X, BSD).


## Version 2.6.0, 29 December 2017
### Features and improvements
* [HTTP JSON](Connecting_to_the_server/HTTP.md): JSON queries can now do equality on attributes, MVA and JSON attributes can be used in inserts and updates, updates and deletes via JSON API can be performed on distributed indexes
* [Percolate Queries](Searching/Percolate_query.md)
* Removed support for 32-bit docids from the code. Also removed all the code that converts/loads legacy indexes with 32-bit docids.
* [Morphology only for certain fields](https://github.com/manticoresoftware/manticore/issues/7) . A new index directive morphology_skip_fields allows defining a list of fields for which morphology does not apply.
* [expand_keywords can now be a query runtime directive set using the OPTION statement](https://github.com/manticoresoftware/manticore/issues/8)

### Bugfixes
* [Commit 0cfae4c](https://github.com/manticoresoftware/manticore/commit/0cfae4c) fixed crash on debug build of server (and m.b. UB on release) when built with rlp
* [Commit 324291e](https://github.com/manticoresoftware/manticore/commit/324291e) fixed RT index optimize with progressive option enabled that merges kill-lists with wrong order
* [Commit ac0efee](https://github.com/manticoresoftware/manticore/commit/ac0efee)  minor crash on mac
* lots of minor fixes after thorough static code analysis
* other minor bugfixes

### Upgrade
In this release we've changed internal protocol used by masters and agents to speak with each other. In case you run Manticoresearch in a distributed environment with multiple instances make sure your first upgrade agents, then the masters.

## Version 2.5.1, 23 November 2017
### Features and improvements
* JSON queries on [HTTP API protocol](Connecting_to_the_server/HTTP.md). Supported search, insert, update, delete, replace operations. Data manipulation commands can be also bulked, also there are some limitations currently as MVA and JSON attributes can't be used for inserts, replaces or updates.
* [RELOAD INDEXES](Adding_data_from_external_storages/Rotating_a_table.md#RELOAD-TABLES) command
* [FLUSH LOGS](Logging/Rotating_query_and_server_logs.md) command
* [SHOW THREADS](Node_info_and_management/SHOW_THREADS.md) can show progress of optimize, rotation or flushes.
* GROUP N BY work correctly with MVA attributes
* blackhole agents are run on separate thread to not affect master query anymore
* implemented reference count on indexes, to avoid stalls caused by rotations and high load
* SHA1 hashing implemented, not exposed yet externally
* fixes for compiling on FreeBSD, macOS and Alpine

### Bugfixes
* [Commit 989752b](https://github.com/manticoresoftware/manticore/commit/989752b) filter regression with block index
* [Commit b1c3864](https://github.com/manticoresoftware/manticore/commit/b1c3864) rename PAGE_SIZE -> ARENA_PAGE_SIZE for compatibility with musl
* [Commit f2133cc](https://github.com/manticoresoftware/manticore/commit/f2133cc) disable googletests for cmake < 3.1.0
* [Commit f30ec53](https://github.com/manticoresoftware/manticore/commit/0839de7) failed to bind socket on server restart
* [Commit 0807240](https://github.com/manticoresoftware/manticore/commit/0807240) fixed crash of server on shutdown
* [Commit 3e3acc3](https://github.com/manticoresoftware/manticore/commit/3e3acc3) fixed show threads for system blackhole thread
* [Commit 262c3fe](https://github.com/manticoresoftware/manticore/commit/262c3fe) Refactored config check of iconv, fixes building on FreeBSD and Darwin

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
* [Commit a58c619](https://github.com/manticoresoftware/manticore/commit/a58c619) fixed SHOW COLLATION statement that breaks java connector
* [Commit 631cf4e](https://github.com/manticoresoftware/manticore/commit/631cf4e) fixed crashes on processing distributed indexes; added locks to distributed index hash; removed move and copy operators from agent
* [Commit 942bec0](https://github.com/manticoresoftware/manticore/commit/942bec0) fixed crashes on processing distributed indexes due to parallel reconnects
* [Commit e5c1ed2](https://github.com/manticoresoftware/manticore/commit/e5c1ed2) fixed crash at crash handler on store query to server log
* [Commit 4a4bda5](https://github.com/manticoresoftware/manticore/commit/4a4bda5) fixed a crash with pooled attributes in multiqueries
* [Commit 3873bfb](https://github.com/manticoresoftware/manticore/commit/3873bfb) fixed reduced core size by prevent index pages got included into core file
* [Commit 11e6254](https://github.com/manticoresoftware/manticore/commit/11e6254) fixed searchd crashes on startup when invalid agents are specified
* [Commit 4ca6350](https://github.com/manticoresoftware/manticore/commit/4ca6350) fixed indexer reports error in sql_query_killlist query
* [Commit 123a9f0](https://github.com/manticoresoftware/manticore/commit/123a9f0) fixed fold_lemmas=1 vs hit count
* [Commit cb99164](https://github.com/manticoresoftware/manticore/commit/cb99164) fixed inconsistent behavior of html_strip
* [Commit e406761](https://github.com/manticoresoftware/manticore/commit/e406761) fixed optimize rt index loose new settings; fixed optimize with sync option lock leaks;
* [Commit 86aeb82](https://github.com/manticoresoftware/manticore/commit/86aeb82) fixed processing erroneous multiqueries
* [Commit 2645230](https://github.com/manticoresoftware/manticore/commit/2645230) fixed result set depends on multi-query order
* [Commit 72395d9](https://github.com/manticoresoftware/manticore/commit/72395d9) fixed server crash on multi-query with bad query
* [Commit f353326](https://github.com/manticoresoftware/manticore/commit/f353326) fixed shared to exclusive lock
* [Commit 3754785](https://github.com/manticoresoftware/manticore/commit/3754785) fixed server crash for query without indexes
* [Commit 29f360e](https://github.com/manticoresoftware/manticore/commit/29f360e) fixed dead lock of server

## Version 2.3.3, 06 July 2017
* Manticore branding
