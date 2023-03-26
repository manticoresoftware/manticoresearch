# Section "Searchd" in configuration

The below settings are to be used in section `searchd` in the configuration file and control Manticore Search server behaviour.

### access_plain_attrs

Instance-wide defaults for [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Optional, default value is `mmap_preread`.

This directive lets you specify the default value of [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables served by this instance of searchd. Per-table directives take precedence, and will overwrite this instance-wide default value, allowing for fine-grain control.


### access_blob_attrs

Instance-wide defaults for [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Optional, default value is `mmap_preread`.

This directive lets you specify the default value of [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables served by this instance of searchd. Per-table directives take precedence, and will overwrite this instance-wide default value, allowing for fine-grain control.


### access_doclists

Instance-wide defaults for [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Optional, default value is `file`.

This directive lets you specify the default value of [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables served by this instance of searchd. Per-table directives take precedence, and will overwrite this instance-wide default value, allowing for fine-grain control.


### access_hitlists

Instance-wide defaults for [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). Optional, default value is `file`.

This directive lets you specify the default value of [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables served by this instance of searchd. Per-table directives take precedence, and will overwrite this instance-wide default value, allowing for fine-grain control.


### agent_connect_timeout

Instance-wide default for [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) parameter.


### agent_query_timeout

Instance-wide defaults for [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) parameter. Can be overridden per-query using `OPTION agent_query_timeout=XXX` clause.


### agent_retry_count

Integer, specifies how many times manticore will try to connect and query remote agents through a distributed table before reporting a fatal query error. Default is 0 (i.e. no retries). This value may be also specified on per-query basis using `OPTION retry_count=XXX` clause. If the per-query option exists, it will override the one specified in config.

Note, that if you use [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) in definition of your distributed table, then before each connect attempt the server will select a different mirror, according to the selected [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). In this case `agent_retry_count` will be aggregated for all mirrors in a set.

For example, if you have 10 mirrors and have set `agent_retry_count=5`, then the server will retry up to 50 times, assuming average 5 tries per every of 10 mirrors (in case of option `ha_strategy = roundrobin` it will be actually so).

At the same time the value provided as `retry_count` option of [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) serves as an absolute limit. In other words, `[retry_count=2]` option in the agent definition means always at most 2 tries, no matter if you have 1 or 10 mirrors specified for the agent.


### agent_retry_delay

Integer, in milliseconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). Specifies the delay sphinx rest before retrying to query a remote agent in case it fails. The value has sense only if non-zero [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) or non-zero per-query `retry_count` specified. Default is 500. This value may be also specified on per-query basis using `OPTION retry_delay=XXX` clause. If per-query option exists, it will override the one specified in config.


### attr_flush_period

<!-- example conf attr_flush_period -->
When calling [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) to update document attributes in real-time, the changes are first written to in-memory copy of attributes.  The updates are done in a memory mapped file, which means that the OS decides when to write these changes to disk. Once `searchd` shuts down normally (via `SIGTERM` being sent) it forces writing all the changes to disk.

It is also possible to tell `searchd` to periodically write these changes  back to disk to avoid them being lost. The time between those intervals is set with `attr_flush_period`, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)).

It defaults to 0, which disables the periodic flushing, but flushing will still occur at normal shut-down.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
Disables or throttles automatic [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE).

Since Manticore 4 tables compaction happens automatically. You can change it with help of the setting `auto_optimize` by changing it to:
* 0 to disable automatic tables compaction (you can still call `OPTIMIZE` manually)
* 1 to enable it explicitly
* N to enable it, but let OPTIMIZE start as soon as the number of disk chunks is greater than `# of CPU cores * 2 * N`

Note turning `auto_optimize` on/off doesn't prevent you from running `OPTIMIZE` manually.

<!-- intro -->
##### Example:

<!-- request Disable -->
```ini
auto_optimize = 0 # disable automatic OPTIMIZE
```

<!-- request Throttle -->
```ini
auto_optimize = 2 # OPTIMIZE starts at 16 chunks (on 4 cpu cores server)
```

<!-- end -->

### auto_schema

<!-- example conf auto_schema -->
Manticore supports the automatic creation of tables that do not yet exist but are specified in INSERT statements. This feature is enabled by default. To disable it, set `auto_schema = 0` explicitly in your configuration. To re-enable it, set `auto_schema = 1` or remove the `auto_schema` setting from the configuration.

Note that `/bulk` HTTP endpoint does not support automatic creation of tables.

<!-- request Disable -->
```ini
auto_schema = 0 # disable automatic table creation
```

<!-- request Enable -->
```ini
auto_schema = 1 # enable automatic table creation
```

<!-- end -->

### binlog_flush

<!-- example conf binlog_flush -->
Binary log transaction flush/sync mode. Optional, default is 2 (flush every transaction, sync every second).

This directive controls how frequently will binary log be flushed to OS and synced to disk. Three modes are supported:

*  0, flush and sync every second. Best performance, but up to 1 second worth of committed transactions can be lost both on server crash, or OS/hardware crash.
*  1, flush and sync every transaction. Worst performance, but every committed transaction data is guaranteed to be saved.
*  2, flush every transaction, sync every second. Good performance, and every committed transaction is guaranteed to be saved in case of server crash. However, in case of OS/hardware crash up to 1 second worth of committed transactions can be lost.

For those familiar with MySQL and InnoDB, this directive is entirely similar to `innodb_flush_log_at_trx_commit`. In most cases, the default hybrid mode 2 provides a nice balance of speed and safety, with full RT table data protection against server crashes, and some protection against hardware ones.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->


### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
Maximum binary log file size. Optional, default is 268435456, or 256Mb.

A new binlog file will be forcibly opened once the current binlog file reaches this limit. This achieves a finer granularity of logs and can yield more efficient binlog disk usage under certain borderline workloads. 0 means do not reopen binlog file based on size.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
Binary log (aka transaction log) files path. Optional, default is build-time configured data directory (`/var/lib/manticore/data/binlog.*` in Linux).

Binary logs are used for crash recovery of RT table data, and also for attributes updates of plain disk indices that would otherwise only be stored in RAM until flush. When logging is enabled, every transaction COMMIT-ted into an RT table gets written into a log file. Logs are then automatically replayed on startup after an unclean shutdown, recovering the logged changes.

`binlog_path` directive specifies the binary log files location. It should contain just the path; `searchd` will create and unlink multiple `binlog.*` files in the directory as necessary (binlog data, metadata, and lock files, etc).

Empty value disables binary logging. That improves performance, but puts the RT table data at risk.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_path = # disable logging
binlog_path = /var/lib/manticore/data # /var/lib/manticore/data/binlog.001 etc will be created
```
<!-- end -->    


### client_timeout

<!-- example conf client_timeout -->
Maximum time to wait between requests (in seconds or [special_suffixes](../Server_settings/Special_suffixes.md)) when using persistent connections. Optional, default is five minutes.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
client_timeout = 1h
```
<!-- end -->    


### collation_libc_locale

<!-- example conf collation_libc_locale -->
Server libc locale. Optional, default is C.

Specifies the libc locale, affecting the libc-based collations. Refer to [collations](../Searching/Collations.md) section for the details.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
collation_libc_locale = fr_FR
```
<!-- end -->


### collation_server

<!-- example conf collation_server -->
Default server collation. Optional, default is libc_ci.

Specifies the default collation used for incoming requests. The collation can be overridden on a per-query basis. Refer to [collations](../Searching/Collations.md) section for the list of available collations and other details.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
collation_server = utf8_ci
```
<!-- end -->


### data_dir

<!-- example conf data_dir -->
When specified, enables the [real-time mode](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) (imperative way of managing data schema). The value should be a path to the directory where you want to store all your tables, binary log and everything else needed for the proper functioning of Manticore Search in this mode.
Indexation of [plain tables](../Creating_a_table/Local_tables/Plain_table.md) is not allowed when the `data_dir` is specified. Read more about the difference between the RT mode and the plain mode in [this section](../Read_this_first.md#Real-time-table-vs-plain-table).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
Maximum size of document blocks from document storage that are held in memory. Optional, default is 16m (16 megabytes).

When `stored_fields` is used, document blocks are read from disk and uncompressed. Since every block typically holds several documents, it may be reused when processing the next document. For this purpose, the block is held in a server-wide cache. The cache holds uncompressed blocks.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
docstore_cache_size = 8m
```
<!-- end -->

### engine

<!-- example conf engine -->
Default attribute storage engine used when creating tables in RT mode. Can be `rowwise` (default) or `columnar`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
engine = columnar
```
<!-- end -->    


### expansion_limit

<!-- example conf expansion_limit -->
The maximum number of expanded keywords for a single wildcard. Optional, default is 0 (no limit).

When doing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands and even millions of matched keywords (think of matching 'a\*' against the entire Oxford dictionary). This directive lets you limit the impact of such expansions. Setting `expansion_limit = N` restricts expansions to no more than N of the most frequent matching keywords (per each wildcard in the query).


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->    


### grouping_in_utc

Specifies whether timed grouping in API and SQL will be calculated in local timezone, or in UTC. Optional, default is 0 (means 'local tz').

By default all 'group by time' expressions (like group by day, week,  month and year in API, also group by day, month, year, yearmonth, yearmonthday in SQL) is done using local time. I.e. when you have docs with attributes timed `13:00 utc` and `15:00 utc` - in case of grouping they both will fall into facility group according to your local tz setting. Say, if you live in `utc`, it will be one day, but if you live in `utc+10`, then these docs will be matched into different `group by day` facility groups (since 13:00 utc in UTC+10 tz 23:00 local time, but 15:00 is 01:00 of the next day). Sometimes such behavior is unacceptable, and it is desirable to make time grouping not dependent from timezone. Of course, you can run the server with defined global TZ env variable, but it will affect not only grouping, but also timestamping in the logs, which may be also undesirable. Switching 'on' this option (either in config, either using [SET global](../Server_settings/Setting_variables_online.md#SET) statement in SQL) will cause all time grouping expressions to be calculated in UTC, leaving the rest of time-depentend functions (i.e. logging of the server) in local TZ.


### ha_period_karma

<!-- example conf ha_period_karma -->
Agent mirror statistics window size, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). Optional, default is 60.

For a distributed table with agent mirrors in it (see more in [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md),  master tracks several different per-mirror counters. These counters are then used for failover and balancing. (Master picks the best mirror to use based on the counters.) Counters are accumulated in blocks of `ha_period_karma` seconds.

After beginning a new block, master may still use the accumulated values from the previous one, until the new one is half full. Thus, any previous history stops affecting the mirror choice after 1.5 times ha_period_karma seconds at most.

Despite that at most 2 blocks are used for mirror selection, upto 15 last blocks are actually stored, for instrumentation purposes. They can be inspected using [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) statement.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
Interval between agent mirror pings, in milliseconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). Optional, default is 1000.

For a distributed table with agent mirrors in it (see more in [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)), master sends all mirrors a ping command during the idle periods. This is to track the current agent status (alive or dead, network roundtrip, etc). The interval between such pings is defined by this directive. To disable pings, set ha_ping_interval to 0.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

Hostnames renew strategy. By default, IP addresses of agent host names are cached at server start to avoid extra flood to DNS. In some cases the IP can change dynamically (e.g. cloud hosting) and it might be desired to don't cache the IPs. Setting this option to 'request' disabled the caching and queries the DNS at each query. The IP addresses can also be manually renewed with `FLUSH HOSTNAMES` command.

### jobs_queue_size

Defines how many "jobs" can be in the queue at the same time. Unlimited by default.

In most cases "job" means one query to a single local table (plain table or a disk chunk of a real-time table), i.e. if you have a distributed table consisting of 2 local tables or a real-time table which has 2 disk chunks a search query to either of them will mostly put 2 jobs to the queue and then the thread pool whose size is defined by [threads](../Server_settings/Searchd.md#threads) will process them, but in some cases if the query is too complex more jobs can be created. Changing this setting is recommended when [max_connections](../Server_settings/Searchd.md#max_connections) and [threads](../Server_settings/Searchd.md#threads) are not enough to find a balance between the desired performance and load on the server.

### listen_backlog

<!-- example conf listen_backlog -->
TCP listen backlog. Optional, default is 5.

Windows builds currently can only process the requests one by one. Concurrent requests will be enqueued by the TCP stack on OS level, and requests that can not be enqueued will immediately fail with "connection refused" message. listen_backlog directive controls the length of the connection queue. Non-Windows builds should work fine with the default value.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->


### listen

<!-- example conf listen -->
This setting lets you specify IP address and port, or Unix-domain socket path, that Manticore will accept connections on.

The general syntax for `listen` is:

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

You can specify:
* either an IP address (or hostname) and a port number
* or just a port number
* or Unix socket path (not supported on Windows)
* or an IP address and ports range

If you specify a port number, but not an address, `searchd` will listen on all network interfaces. Unix path is identified by a leading slash. Ports range could be set only for the replication protocol.

You can also specify a protocol handler (listener) to be used for connections on this socket. The listeners are:

* **Not specified** - Manticore will accept connections at this port from:
  - other Manticore agents (i.e. a remote distributed table)
  - clients via HTTP and HTTPS
* `mysql` - MySQL protocol for connections from MySQL clients. Note:
  - Compressed protocol is also supported.
  - If [SSL](../Security/SSL.md#SSL) is enabled you can make an encrypted connection.
* `replication` - replication protocol, used for nodes communication. More details can be found in the [replication](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) section. You can specify multiple replication listeners, but they must all listen on the same IP, only the ports can be different.
* `http` - same as **Not specified**. Manticore will accept connections at this port from remote agents and clients via HTTP and HTTPS.
* `https` - HTTPS protocol. Manticore will accept **only** HTTPS connections at this port. More details can be found in section [SSL](../Security/SSL.md).
* `sphinx` - legacy binary protocol. Used to serve connections from remote [SphinxSE](../Extensions/SphinxSE.md) clients. Some Sphinx API clients implementations (an example is the Java one) require the explicit declaration of the listener.

Adding suffix `_vip` to client protocols (that is all, except `replication`, for instance `mysql_vip` or `http_vip` or just `_vip`) forces creating a dedicated thread for the connection to bypass different limitations. That's useful for node maintenance in case of a severe overload when the server would either stall or not let you connect via a regular port otherwise.

Suffix `_readonly` sets [read-only mode](../Security/Read_only.md) for the listener and limits it to accept only read queries.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
listen = localhost
listen = localhost:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at localhost
listen = 192.168.0.1:5000 # listen for remote agents (binary API) and http/https requests on port 5000 at 192.168.0.1
listen = /var/run/manticore/manticore.s # listen for binary API requests on unix socket
listen = /var/run/manticore/manticore.s:mysql # listen for mysql requests on unix socket
listen = 9312 # listen for remote agents (binary API) and http/https requests on port 9312 on any interface
listen = localhost:9306:mysql # listen for mysql requests on port 9306 at localhost
listen = localhost:9307:mysql_readonly # listen for mysql requests on port 9307 at localhost and accept only read queries
listen = 127.0.0.1:9308:http # listen for http requests as well as connections from remote agents (and binary API) on port 9308 at localhost
listen = 192.168.0.1:9320-9328:replication # listen for replication connections on ports 9320-9328 at 192.168.0.1
listen = 127.0.0.1:9443:https # listen for https requests (not http) on port 9443 at 127.0.0.1
listen = 127.0.0.1:9312:sphinx # listen for legacy Sphinx requests (e.g. from SphinxSE) on port 9312 at 127.0.0.1
```
<!-- end -->

There can be multiple `listen` directives, `searchd` will listen for client connections on all specified ports and sockets.  Default config provided in Manticore packages defines listening on ports:
* `9308` and `9312` for connections from remote agents and non-mysql based clients
* and on port `9306` for MySQL connections.

If you don't specify any `listen` in configuration at all Manticore will wait for connections on:
* `127.0.0.1:9306` for MySQL clients
* `127.0.0.1:9312` for HTTP/HTTPS, and connections from other Manticore nodes and clients based on Manticore binary API

#### Listening on privileged ports

By default Linux won't allow you to let Manticore listen on port below 1024 (e.g. `listen = 127.0.0.1:80:http` or `listen = 127.0.0.1:443:https`) unless you run the searchd under root. If you still want to be able to start Manticore so it listens on ports < 1024 under non-root consider doing one of the following (either of these should work):
* `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* add `AmbientCapabilities=CAP_NET_BIND_SERVICE` to Manticore's systemd unit (`/lib/systemd/system-generators/manticore-search-generator`) and reload the daemon (`systemctl daemon-reload`)

#### Technical details about Sphinx API protocol and TFO
<details>
Legacy Sphinx protocol has 2 phases: handshake exchanging and data flow. The handshake consists of a packet of 4 bytes from the client, and a packet of 4 bytes from the daemon with only one purpose - the client determines that the remote is a real Sphinx daemon, the daemon determines that the remote is a real Sphinx client. The main dataflow is quite simple: let's both sides declare their handshakes, and the opposite check them. That exchange with short packets implies using special `TCP_NODELAY` flag, which switches off Nagle's TCP algorithm and declares, that the TCP connection will be performed as a dialogue of small packages.
However it is not strictly defined who speaks first in this negotiation. Historically all clients that use the binary API speak first: send handshake, then read 4 bytes from a daemon, then send a request and read an answer from the daemon.
When we improved Sphinx protocol compatibility we considered these things:

1. Usually master-agent communication is established from a known client to a known host on a known port. So, it is quite not possible, that the endpoint will provide wrong handshake. So, we may implicitly assume, that both sides are valid and really speak in Sphinx proto.
2. Given this assumption we can 'glue' a handshake to the real request and send it in one packet. If backend is a legacy Sphinx daemon - it will just read this glued packet as 4 bytes of a hadshake, then request body. Since they both came in one packet, the backend socket has -1 RTT, and the frontend buffer still works despite that fact usual way.
3. Continuing the assumption: since the 'query' packet is quite small, and the hadshake is even smaller, let's send both in initial 'SYN' TCP package using modern TFO (tcp-fast-open) technique. That is: we connect to a remote node with the glued handshake + body package. The daemon accept the connection and immediately has both the handshake and the body in a socket buffer, as they came in the very first TCP 'SYN' packet. That eliminates another one RTT.
4. Finally teach daemon to accept this improvement. Actually, from application it implies NOT to use `TCP_NODELAY`. And, from system side it implies to ensure that on the daemon side accepting TFO is activated, and on the client side sending TFO is also activated. By default in modern systems client TFO is already activated by default, so you have only tune server TFO for all things to work.

All these improvements without actual changing of the protocol itself allowed to eliminate 1.5 RTT of TCP protocol from the connection. Which is, if query and answer are capable to be placed in a single TCP package, decreases whole binary API session from 3.5 RTT to 2 RTT - which makes network negotination about 2 times faster.

So, all our improvements are stated around initially undefined statement: 'who speaks first'. If a client speaks first - we may apply all these optimizations and effectively process connect + handshake + query in a single TFO package. Moreover, we can look at the beginning of the received package and determine a real protocol. That is why you can connect to one and the same port via API/http/https. If the daemon has to speak first - all this optimizations are impossible. And the multiprotocol is also impossible. That is why we have a dedicated port for mysql, and did not unify it with all the other protocols into a same port. Suddenly, among all clients one was written implying that daemon should send a handshake first. That is - no possibility to all the described improvements. That is SphinxSE plugin for mysql/mariadb. So, specially for this single client we dedicated `sphinx` proto definition to work most legacy way. Namely: both sides activate `TCP_NODELAY` and exchange with small packages. The daemon sends its handshake on connect, then the client sends its, and then everything works usual way. That is not very optimal, but just works. If you use SphinxSE to connect to Manticore - you have to dedicate a listener with explicitly stated `sphinx` proto. For another clients - avoid to use this listener as it is slower. If you use another legacy Sphinx API clients - check first, if they are able to work with non-dedicated multiprotocol port. For master-agent linkage using the non-dedicated (multiprotocol) port and enabling client and server TFO works well and will definitely make working of network backend faster, especially if you have very light and fast queries.
</details>

### listen_tfo

This setting allows TCP_FASTOPEN flag for all listeners. By default it is managed by system, but may be explicitly switched off by setting to '0'.

For general knowledge about TCP Fast Open extension please consult with [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). Shortly speaking, it allows to eliminate one TCP round-trip when establishing connection.

In practice using TFO in many situation may optimize client-agent network efficiency as if [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) are in play, but without holding active connections, and also without limitation for the maximum num of connections.

On modern OS TFO support usually switched 'on' on the system level, but this is just 'capability', not the rule. Linux (as most progressive) supports it since 2011, on kernels starting from 3.7 (for server side). Windows supports it from some build of Windows 10. Another (FreeBSD, MacOS) also in game.

For Linux system server checks variable `/proc/sys/net/ipv4/tcp_fastopen` and behaves according to it. Bit 0 manages client side, bit 1 rules listeners. By default system has this param set to 1, i.e. clients enabled, listeners disabled.


### log

<!-- example conf log -->
Log file name. Optional, default is 'searchd.log'. All `searchd` run time events will be logged in this file.

Also you can use the 'syslog' as the file name. In this case the events will be sent to syslog daemon. To use the syslog option the sphinx must be configured `-–with-syslog` on building.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
log = /var/log/searchd.log
```
<!-- end -->


### max_batch_queries

<!-- example conf max_batch_queries -->
Limits the amount of queries per batch. Optional, default is 32.

Makes searchd perform a sanity check of the amount of the queries submitted in a single batch when using [multi-queries](../Searching/Multi-queries.md). Set it to 0 to skip the check.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_batch_queries = 256
```
<!-- end -->

### max_connections

<!-- example max_connections -->
Maximum number of simultaneous client connections. Unlimited by default. That is usually noticeable only when using any kind of persistent connections, like cli mysql sessions or persistent remote connections from remote distributed tables. When the limit is exceeded you can still connect to the server using the [VIP connection](../Connecting_to_the_server/MySQL_protocol.md#VIP-connection). VIP connections are not counted towards the limit.

<!-- request Example -->
```ini
max_connections = 10
```

<!-- end -->

### max_threads_per_query

<!-- example max_threads_per_query -->
Instance-wide limit of threads one operation can use. By default appropriate operations can occupy all CPU cores, leaving no room for other operations. Let's say, `call pq` against considerably big percolate table can utilize all threads for tens of seconds. Setting `max_threads_per_query` to, say, half of [threads](../Server_settings/Searchd.md#threads) will ensure that you can run couple of such `call pq` in parallel.

You can also set this setting as a session or a global variable during the runtime.

You can also control the behaviour on per-query with help of the [threads OPTION](../Searching/Options.md#threads).

<!-- intro -->
##### Example:
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
Maximum allowed per-query filter count. Only used for internal sanity checks, does not directly affect RAM use or performance. Optional, default is 256.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
Maximum allowed per-filter values count. Only used for internal sanity checks, does not directly affect RAM use or performance. Optional, default is 4096.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
The maximum number of files that are allowed to be opened by the server is called "soft limit". Note that serving large fragmented real-time tables may require this limit to be set high, as each disk chunk may occupy a dozen or more files. For example, a real-time table with 1000 chunks may require thousands of files to be opened simultaneously. If you encounter the error 'Too many open files' in the logs, try adjusting this option, as it may help resolve the issue.

There is also a "hard limit" that cannot be exceeded by the option. This limit is defined by the system and can be changed in the file `/etc/security/limits.conf` on Linux. Other operating systems may have different approaches, so consult your manuals for more information.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
Apart direct numeric values, you can use magic word 'max', to set the limit equal to available current hard limit.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
Maximum allowed network packet size. Limits both query packets from clients, and response packets from remote agents in distributed environment. Only used for internal sanity checks, does not directly affect RAM use or performance. Optional, default is 8M.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
A server version string to return via MySQL protocol. Optional, default is empty (return Manticore version).

Several picky MySQL client libraries depend on a particular version number format used by MySQL, and moreover, sometimes choose a different execution path based on the reported version number (rather than the indicated capabilities flags). For instance, Python MySQLdb 1.2.2 throws an exception when the version number is not in X.Y.ZZ format; MySQL .NET connector 6.3.x fails internally on version numbers 1.x along with a certain combination of flags, etc. To workaround that, you can use `mysql_version_string` directive and have `searchd` report a different version to clients connecting over MySQL protocol. (By default, it reports its own version.)


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

Number of network threads, default is 1.

Useful for extremely high query rates, when just 1 thread is not enough to manage all the incoming queries.


### net_wait_tm

Controls busy loop interval of the network thread, default is -1, might be set to -1, 0 or a positive integer.

In case server is configured as a pure master and just routes requests to agents it is important to handle requests without delays and to not allow the network thread to sleep. There is a busy loop for that. After and incoming request the network thread uses CPU poll for `10 * net_wait_tm` milliseconds in case `net_wait_tm` is a positive number or polls only with CPU in case `net_wait_tm` is `0`. Also the busy loop can be disabled with `net_wait_tm = -1` - this way the poller sets timeout to actual agent's timeouts on system polling call.

> **WARNING:** CPU busy loop actually loads CPU core, so setting this value to any non-default will cause noticeable CPU usage even with idle server.


### net_throttle_accept

Defines how many clients are accepted on each iteration of the network loop. Default is 0 (unlimited), which should be fine for most users. This is a fine tuning option to control the throughput of the network loop in high load scenarios.


### net_throttle_action

Defines how many requests are processed on each iteration of the network loop. Default is 0 (unlimited), which should be fine for most users. This is a fine tuning option to control the throughput of the network loop in high load scenarios.

### network_timeout

<!-- example conf network_timeout -->
Network client request read/write timeout, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). Optional, default is 5 seconds. `searchd` will forcibly close a client connection which fails to send a query or read a result within this timeout.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
This setting lets you specify the network address of the node. By default it is set to replication [listen](../Server_settings/Searchd.md#listen) address. That is correct in most cases, however there are situations where you have to specify it manually:

* node behind a firewall
* network address translation enabled (NAT)
* container deployments, such as Docker or cloud deployments
* clusters with nodes in more than one region


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
Whether to allow queries with only [negation](../Searching/Full_text_matching/Operators.md#Negation-operator) full-text operator. Optional, default is 0 (fail queries with only NOT operator).


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
Sets default table compaction threshold. Read more here - [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Can be overridden with per-query option [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). Can be changed dynamically via [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
The maximum # of simultaneous persistent connections to remote [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md). Each time connecting an agent defined under `agent_persistent` we try to reuse existing connection (if any), or connect and save the connection for the future. However in some cases it makes sense to limit # of such persistent connections. This directive defines the number. It affects the number of connections to each agent's host across all distributed tables.

It is reasonable to set the value equal or less than [max_connections](../Server_settings/Searchd.md#max_connections) option of the agent's config.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
`searchd` process ID file name. Mandatory.

PID file will be re-created (and locked) on startup. It will contain head server process ID while the server is running, and it will be unlinked on server shutdown. It's mandatory because Manticore uses it internally for a number of things: to check whether there already is a running instance of `searchd`; to stop `searchd`; to notify it that it should rotate the tables. Can also be used for different external automation scripts.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
pid_file = /var/run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
Costs for the query time prediction model, in nanoseconds. Optional, default is "doc=64, hit=48, skip=2048, match=64" (without the quotes).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
Terminating queries before completion based on their execution time (with max query time setting) is a nice safety net, but it comes with an inborn drawback: indeterministic (unstable) results. That is, if you repeat the very same (complex) search query with a time limit several times, the time limit will get hit at different stages, and you will get *different* result sets.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT … OPTION max_query_time
```
<!-- request API -->

```api
SetMaxQueryTime()
```
<!-- end -->

There is a new option, [SELECT … OPTION max_predicted_time](../Searching/Options.md#max_predicted_time), that lets you limit the query time *and* get stable, repeatable results. Instead of regularly checking the actual current time while evaluating the query, which is indeterministic, it predicts the current running time using a simple linear model instead:

```ini
predicted_time =
    doc_cost * processed_documents +
    hit_cost * processed_hits +
    skip_cost * skiplist_jumps +
    match_cost * found_matches
```    

The query is then terminated early when the `predicted_time` reaches a given limit.

Of course, this is not a hard limit on the actual time spent (it is, however, a hard limit on the amount of *processing* work done), and a simple linear model is in no way an ideally precise one. So the wall clock time *may* be either below or over the target limit. However, the error margins are quite acceptable: for instance, in our experiments with a 100 msec target limit the majority of the test queries fell into a 95 to 105 msec range, and *all* of the queries were in a 80 to 120 msec range. Also, as a nice side effect, using the modeled query time instead of measuring actual run time results in somewhat less gettimeofday() calls, too.

No two server makes and models are identical, so `predicted_time_costs` directive lets you configure the costs for the model above. For convenience, they are integers, counted in nanoseconds.(The limit in max_predicted_time is counted in milliseconds, and having to specify cost values as 0.000128 ms instead of 128 ns is somewhat more error prone.) It is not necessary to specify all 4 costs at once, as the missed one will take the default values. However, we strongly suggest to specify all of them, for readability.


### preopen_tables

<!-- example conf preopen_tables -->
Whether to forcibly preopen all tables on startup. Optional, default is 1 (preopen everything).

When set to 1, this directive overrides and enforces [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) on all tables. They will be preopened, no matter what is the per-table `preopen` setting. When set to 0, per-table settings can take effect. (And they default to 0.)

Pre-opened tables avoid races between search queries and rotations that can cause queries to fail occasionally. They also make `searchd` use more file handles. In most scenarios it's therefore preferred and recommended to preopen tables.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
Enables pseudo-sharding for search queries to plain and real-time tables, no matter if they are queried directly or through a distributed table. Any search query to a local table will be automatically parallelized to up to `searchd.threads` # of threads.

Note that if your worker threads are already busy, because you have:
* high query concurrency
* physical sharding of any kind:
  - distributed table of multiple plain/real-time tables
  - real-time table consisting of too many disk chunks

then the pseudo-sharding may not bring any positive effect and in some cases can even cause slight throughput decrease. If you are looking for a higher throughput rather than lower latency it's recommended to disable it.

Enabled by default.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
pseudo_sharding = 0
```
<!-- end -->

### qcache_max_bytes

<!-- example conf qcache_max_bytes -->
Integer, in bytes. The maximum RAM allocated for cached result sets. Default is 16777216, or 16Mb. 0 means disabled. Refer to [query cache](../Searching/Query_cache.md) for details.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

Integer, in milliseconds. The minimum wall time threshold for a query result to be cached. Defaults to 3000, or 3 seconds. 0 means cache everything. Refer to [query cache](../Searching/Query_cache.md) for details. This value also may be expressed with time [special_suffixes](../Server_settings/Special_suffixes.md), but use it with care and don't confuse yourself with name of the value itself, containing '_msec'.


### qcache_ttl_sec

Integer, in seconds. The expiration period for a cached result set. Defaults to 60, or 1 minute. The minimum possible value is 1 second. Refer to [query cache](../Searching/Query_cache.md) for details. This value also may be expressed with time [special_suffixes](../Server_settings/Special_suffixes.md), but use it with care and don't confuse yourself with name of the value itself, containing '_sec'.


### query_log_format

<!-- example conf query_log_format -->
Query log format. Optional, allowed values are `plain` and `sphinxql`, default is `sphinxql`.

The `sphinxql` mode logs valid SQL statements. The `plain` mode logs queries in a plain text format (mostly suitable for purely full-text use cases). This directive allows to switch between the two formats on search server startup. The log format can also be altered on the fly, using `SET GLOBAL query_log_format=sphinxql` syntax. Refer to [Query logging](../Logging/Query_logging.md) for more details.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

Limit (in milliseconds) that prevents the query from being written to the query log. Optional, default is 0 (all queries are written to the query log). This directive specifies that only queries with execution times that exceed the specified limit will be logged (this value also may be expressed with time [special_suffixes](../Server_settings/Special_suffixes.md), but use it with care and don't confuse yourself with name of the value itself, containing `_msec`).

### query_log

<!-- example conf query_log -->
Query log file name. Optional, default is empty (do not log queries). All search queries (such as SELECT ... but not INSERT/REPLACE/UPDATE queries) will be logged in this file. The format is described in [Query logging](../Logging/Query_logging.md). In case of 'plain' format, you can use the 'syslog' as the path to the log file. In this case all search queries will be sent to syslog daemon with `LOG_INFO` priority, prefixed with '[query]' instead of timestamp. To use the syslog option the sphinx must be configured `-–with-syslog` on building.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
By default the searchd and query log files are created with 600 permission, so only the user under which server runs and root users can read the log files. query_log_mode allows settings a different permission. This can be handy to allow other users to be able to read the log files (for example monitoring solutions running on non-root users).


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
Per-keyword read buffer size for document lists. Optional, default is 256K, minimal is 8K.

For every keyword occurrence in every search query, there are two associated read buffers (one for document list and one for hit list). This setting lets you control the document list buffer size. Bigger buffer size might increase per-query RAM use, but possibly decrease IO time. Large values make sense in general for slow storage. For storage capable of high IOPS, experimenting should be done in the low values area.

You may also want to set [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) on per-table basis; that value will override anything set on server's config level.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
Per-keyword read buffer size for hit lists. Optional, default is 256K, minimal is 8K.

For every keyword occurrence in every search query, there are two associated read buffers (one for document list and one for hit list). This setting lets you control the hit list buffer size. Bigger buffer size might increase per-query RAM use, but possibly decrease IO time. Large values make sense in general for slow storage. For storage capable of high IOPS, experimenting should be done in the low values area.

You may also want to set [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) on per-table basis; that valuewill override anything set on server's config level.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
read_buffer_hits = 128K
```
<!-- end -->

### read_unhinted

<!-- example conf read_unhinted -->
Unhinted read size. Optional, default is 32K, minimal 1K

When querying, some reads know in advance exactly how much data is there  to be read, but some currently do not. Most prominently, hit list size in not currently known in advance. This setting lets you control how much data to read in such cases. It impacted hit list IO time, reducing it for lists larger than unhinted read size, but raising it for smaller lists. It **not** affects RAM usage because read buffer will be already allocated. So it should be not greater than read_buffer.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
read_unhinted = 32K
```
<!-- end -->


### rt_flush_period

<!-- example conf rt_flush_period -->
RT tables RAM chunk flush check period, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). Optional, default is 10 hours.

Actively updated RT tables that however fully fit in RAM chunks can result in ever-growing binlogs, impacting disk use and crash recovery time. With this directive the search server performs periodic flush checks, and eligible RAM chunks can get saved, enabling consequential binlog cleanup. See [Binary logging](../Logging/Binary_logging.md) for more details.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
rt_flush_period = 3600 # 1 hour
```
<!-- end -->


### rt_merge_iops

<!-- example conf rt_merge_iops -->
A maximum number of I/O operations (per second) that the RT chunks merge thread is allowed to start. Optional, default is 0 (no limit).

This directive lets you throttle down the I/O impact arising from the `OPTIMIZE` statements. It is guaranteed that all the RT optimization activity will not generate more disk iops (I/Os per second) than the configured limit. Limiting rt_merge_iops can reduce search performance degradation caused by merging.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
rt_merge_iops = 40
```
<!-- end -->

### rt_merge_maxiosize

<!-- example conf rt_merge_maxiosize -->
A maximum size of an I/O operation that the RT chunks merge thread is allowed to start. Optional, default is 0 (no limit).

This directive lets you throttle down the I/O impact arising from the `OPTIMIZE` statements. I/Os bigger than this limit will be broken down into 2 or more I/Os, which will then be accounted as separate I/Os with regards to the [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) limit. Thus, it is guaranteed that all the optimization activity will not generate more than (rt_merge_iops \* rt_merge_maxiosize) bytes of disk I/O per second.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
Prevents `searchd` stalls while rotating tables with huge amounts of data to precache. Optional, default is 1 (enable seamless rotation). On Windows systems seamless rotation is disabled by default.

Tables may contain some data that needs to be precached in RAM. At the moment, `.spa`, `.spb`, `.spi` and `.spm` files are fully precached (they contain attribute data, blob attribute data, keyword table and killed row map, respectively.) Without seamless rotate, rotating a table tries to use as little RAM as possible and works as follows:

1. new queries are temporarily rejected (with "retry" error code);
2. `searchd` waits for all currently running queries to finish;
3. old table is deallocated and its files are renamed;
4. new table files are renamed and required RAM is allocated;
5. new table attribute and dictionary data is preloaded to RAM;
6. `searchd` resumes serving queries from new table.

However, if there's a lot of attribute or dictionary data, then preloading step could take noticeable time - up to several minutes in case of preloading 1-5+ GB files.

With seamless rotate enabled, rotation works as follows:

1. new table RAM storage is allocated;
2. new table attribute and dictionary data is asynchronously preloaded to RAM;
3. on success, old table is deallocated and both tables' files are renamed;
4. on failure, new table is deallocated;
5. at any given moment, queries are served either from old or new table copy.

Seamless rotate comes at the cost of higher **peak** memory usage during the rotation (because both old and new copies of `.spa/.spb/.spi/.spm` data need to be in RAM while preloading new copy). Average usage stays the same.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

This option enables/disables the use of secondary indexes for search queries. It is optional and the default is 1 (enabled). Note that you don't need to enable it for indexing as it is always enabled as long as the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) is installed. The latter is also required for using the indexes on searching. There are three modes available:

* `0`: disable the use of secondary indexes on search. They can be enabled for individual queries using [analyzer hints](../Searching/Options.md#Query-optimizer-hints)
* `1`: enable the use of secondary indexes on search. They can be disabled for individual queries using [analyzer hints](../Searching/Options.md#Query-optimizer-hints)
* `force`: same as enable, but any errors during the loading of secondary indexes will be reported and the whole index will not be loaded into the daemon

Note that secondary indexes are not effective for full-text queries.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
secondary_indexes = 1
```

<!-- end -->

### server_id

<!-- example conf server_id -->
Integer number that serves as server identificator used as seed to generate an unique short UUID for nodes that are part of a replication cluster. The server_id must be unique across the nodes of a cluster and in range from 0 to 127. If server_id is not set, MAC address or a random number will be used as seed for the short UUID.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
server_id = 1
```
<!-- end -->


### shutdown_timeout

<!-- example conf shutdown_timeout -->
`searchd --stopwait` waiting time, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). Optional, default is 60 seconds.

When you run `searchd --stopwait` your server needs to perform some activities before stopping like finishing queries, flushing RT RAM chunk, flushing attributes and updating binlog. And it requires some time. `searchd --stopwait` will wait up to `shutdown_time` seconds for server to finish its jobs. Suitable time depends on your table size and load.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

SHA1 hash of the password which is necessary to invoke 'shutdown' command from VIP Manticore SQL connection. Without it [debug](../Reporting_bugs.md#DEBUG) 'shutdown' subcommand will never cause server's stop. Note, that such simple hashing should not be considered as a strong protection, as we don't use salted hash or any kind of modern hash function. That is just fool-proof for housekeeping daemons in a local network.

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
A prefix to prepend to the local file names when generating snippets. Optional, default is current working folder.

This prefix can be used in distributed snippets generation along with `load_files` or `load_files_scattered` options.

Note how this is a prefix, and **not** a path! Meaning that if a prefix is set to "server1" and the request refers to "file23", `searchd` will attempt to open "server1file23" (all of that without quotes). So if you need it to be a path, you have to mention the trailing slash.

After constructing final file path, server unwinds all relative dirs and compares final result with the value of `snippet_file_prefix`. If result is not begin with the prefix, such file will be rejected with error message.

So, if you set it to '/mnt/data' and somebody calls snippet generation with file '../../../etc/passwd', as the source, it will get error message

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

instead of content of the file.

Also, with non-set parameter and reading '/etc/passwd' it will actually read /daemon/working/folder/etc/passwd since default for param is exactly server's working folder.

Note also that this is a local option, it does not affect the agents anyhow. So you can safely set a prefix on a master server. The requests  routed to the agents will not be affected by the master's setting. They will however be affected by the agent's own settings.

This might be useful, for instance, when the document storage locations (be those local storage or NAS mountpoints) are inconsistent across the servers.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
snippets_file_prefix = /mnt/common/server1/
```
<!-- end -->

> **WARNING:** If you still want to access files from the FS root, you have to explicitly set `snippets_file_prefix` to empty value (by `snippets_file_prefix=` line), or to root (by `snippets_file_prefix=/`).


### sphinxql_state

<!-- example conf sphinxql_state -->
Path to a file where current SQL state will be serialized.

On server startup, this file gets replayed. On eligible state changes (eg. SET GLOBAL), this file gets rewritten automatically. This can prevent a hard-to-diagnose problem: If you load UDF functions, but Manticore crashes, when it gets (automatically) restarted, your UDF and global variables will no longer be available; using persistent state helps a graceful recovery with no such surprises.

`sphinxql_state` can't be used to execute arbitrary commands, e.g. `CREATE TABLE`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
Maximum time to wait between requests (in seconds, or [special_suffixes](../Server_settings/Special_suffixes.md)) when using SQL interface. Optional, default is 15 minutes.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
Path to the SSL Certificate Authority (CA) certificate file (aka root certificate). Optional, default is empty. When not empty the certificate in `ssl_cert` should be signed by this root certificate.

Server uses the CA file to verify the signature on the certificate. The file must be in PEM format.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ssl_ca = keys/ca-cert.pem
```
<!-- end -->


### ssl_cert

<!-- example conf ssl_cert -->
Path to the server's SSL certificate. Optional, default is empty.

Server uses this certificate as self-signed public key encrypting HTTP traffic over SSL. The file must be in PEM format.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ssl_cert = keys/server-cert.pem
```
<!-- end -->


### ssl_key

<!-- example conf ssl_key -->
Path to the SSL certificate key. Optional, default is empty.

Server uses this private key to encrypt HTTP traffic over SSL. The file must be in PEM format.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ssl_key = keys/server-key.pem
```
<!-- end -->


### subtree_docs_cache

<!-- example conf subtree_docs_cache -->
Max common subtree document cache size, per-query. Optional, default is 0 (disabled).

Limits RAM usage of a common subtree optimizer (see [multi-queries](../Searching/Multi-queries.md)). At most this much RAM will be spent to cache document entries per each query. Setting the limit to 0 disables the optimizer.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
subtree_docs_cache = 8M
```
<!-- end -->


### subtree_hits_cache

<!-- example conf subtree_hits_cache -->
Max common subtree hit cache size, per-query. Optional, default is 0 (disabled).

Limits RAM usage of a common subtree optimizer (see [multi-queries](../Searching/Multi-queries.md)). At most this much RAM will be spent to cache keyword occurrences (hits) per each query. Setting the limit to 0 disables the optimizer.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Number of working threads (or, size of thread pool) of Manticore daemon. Manticore creates this number of OS threads on start, and they perform all jobs inside the daemon such as executing queries, creating snippets, etc. Some operations may be split into sub-tasks and executed in parallel, for example:

* Search in a real-time table
* Search in a distributed table consisting of local tables
* Percolate query call
* and others

By default it's set to the number of CPU cores on the server. Manticore creates the threads on start and keep them until it's stopped. Each sub-task can use one of the threads when it needs it, when the sub-task finishes it releases the thread so another sub-task can use it.

In case of intensive I/O type of load it might make sense to set the value higher than the # of CPU cores.

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
Maximum stack size for a job (coroutine, one search query may cause multiple jobs/coroutines). Optional, default is 128K.

Each job has it's own stack of 128K. When you run a query it's checked for how much stack it requires. If the default 128K is enough, it's just processed. If it needs more we schedule another job with an increased stack, which continues processing. The maximum size of such advanced stack is limited by this setting.

Setting the value to a reasonably high rate will help with processing very deep queries without implication, that overall RAM consumption will grow too high. For example, setting it to 1G does not imply that every new job will take 1G of RAM, but if we see that it requires let's say 100M stack, we just allocate 100M for the job. Other jobs at the same time will be running with their default 128K stack. The same way we can run even more complex queries that need 500M. And if only if we **see** internally that the job requires more than 1G of stack we will fail and report about too low thread_stack.

However in practice even a query which needs 16M of stack is often too complex for parsing, and consumes too much time and resources to be processed. So, the daemon will process it, but limiting such queries by the `thread_stack` setting looks quite reasonable.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
Whether to unlink .old table copies on successful rotation. Optional, default is 1 (do unlink).


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
unlink_old = 0
```
<!-- end -->


### watchdog

<!-- example conf watchdog -->
Threaded server watchdog. Optional, default is 1 (watchdog enabled).

When Manticore query crashes it can take down the entire server. With the watchdog feature enabled, `searchd` additionally keeps a separate lightweight process that monitors the main server process, and automatically restarts the latter in case of abnormal termination. Watchdog is enabled by default.

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
