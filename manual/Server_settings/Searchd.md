# Section "Searchd" in configuration

The below settings are to be used in the `searchd` section of the Manticore Search configuration file to control the server's behavior. Below is a summary of each setting:

### access_plain_attrs

This setting sets instance-wide defaults for [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). It is optional, with a default value of `mmap_preread`.

The `access_plain_attrs` directive allows you to define the default value of [access_plain_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables managed by this searchd instance. Per-table directives have higher priority and will override this instance-wide default, providing more fine-grained control.

### access_blob_attrs

This setting sets instance-wide defaults for [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). It is optional, with a default value of `mmap_preread`.

The `access_blob_attrs` directive allows you to define the default value of [access_blob_attrs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables managed by this searchd instance. Per-table directives have higher priority and will override this instance-wide default, providing more fine-grained control.

### access_doclists

This setting sets instance-wide defaults for [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). It is optional, with a default value of `file`.

The `access_doclists` directive allows you to define the default value of [access_doclists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables managed by this searchd instance. Per-table directives have higher priority and will override this instance-wide default, providing more fine-grained control.

### access_hitlists

This setting sets instance-wide defaults for [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files). It is optional, with a default value of `file`.

The `access_hitlists` directive allows you to define the default value of [access_hitlists](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Accessing-table-files) for all tables managed by this searchd instance. Per-table directives have higher priority and will override this instance-wide default, providing more fine-grained control.

### agent_connect_timeout

This setting sets instance-wide defaults for the [agent_connect_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_connect_timeout) parameter.


### agent_query_timeout

This setting sets instance-wide defaults for the [agent_query_timeout](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_query_timeout) parameter. It can be overridden on a per-query basis using the `OPTION agent_query_timeout=XXX` clause.


### agent_retry_count

This setting is an integer that specifies how many times Manticore will attempt to connect and query remote agents through a distributed table before reporting a fatal query error. The default value is 0 (i.e., no retries). You can also set this value on a per-query basis using the `OPTION retry_count=XXX` clause. If a per-query option is provided, it will override the value specified in the configuration.

Note that if you use [agent mirrors](../Creating_a_cluster/Remote_nodes/Mirroring.md#Agent-mirrors) in the definition of your distributed table, the server will select a different mirror for each connection attempt according to the chosen [ha_strategy](../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy). In this case, the `agent_retry_count` will be aggregated for all mirrors in a set.

For example, if you have 10 mirrors and set `agent_retry_count=5`, the server will retry up to 50 times, assuming an average of 5 tries for each of the 10 mirrors (with the `ha_strategy = roundrobin` option, this will be the case).

However, the value provided as the `retry_count` option for the [agent](../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) serves as an absolute limit. In other words, the `[retry_count=2]` option in the agent definition always means a maximum of 2 attempts, regardless of whether you have specified 1 or 10 mirrors for the agent.

### agent_retry_delay

This setting is an integer in milliseconds (or [special_suffixes](../Server_settings/Special_suffixes.md)) that specifies the delay before Manticore retries querying a remote agent in case of failure. This value is only relevant when a non-zero [agent_retry_count](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) or non-zero per-query `retry_count` is specified. The default value is 500. You can also set this value on a per-query basis using the `OPTION retry_delay=XXX` clause. If a per-query option is provided, it will override the value specified in the configuration.


### attr_flush_period

<!-- example conf attr_flush_period -->
When using [Update](../Data_creation_and_modification/Updating_documents/UPDATE.md) to modify document attributes in real-time, the changes are first written to an in-memory copy of the attributes. These updates occur in a memory-mapped file, meaning the OS decides when to write the changes to disk. Upon normal shutdown of `searchd` (triggered by a `SIGTERM` signal), all changes are forced to be written to disk.

You can also instruct `searchd` to periodically write these changes back to disk to prevent data loss. The interval between these flushes is determined by `attr_flush_period`, specified in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)).

By default, the value is 0, which disables periodic flushing. However, flushing will still occur during a normal shutdown.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
attr_flush_period = 900 # persist updates to disk every 15 minutes
```
<!-- end -->

### auto_optimize

<!-- example conf auto_optimize -->
This setting controls the automatic [OPTIMIZE](../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) process for table compaction.

Starting with Manticore 4, table compaction occurs automatically. You can modify this behavior with the `auto_optimize` setting:
* 0 to disable automatic table compaction (you can still call `OPTIMIZE` manually)
* 1 to explicitly enable it
* N to enable it, but allow OPTIMIZE to start when the number of disk chunks is greater than `# of CPU cores * 2 * N`

Note that toggling `auto_optimize` on or off doesn't prevent you from running `OPTIMIZE` manually.

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
Manticore supports the automatic creation of tables that don't yet exist but are specified in INSERT statements. This feature is enabled by default. To disable it, set `auto_schema = 0` explicitly in your configuration. To re-enable it, set `auto_schema = 1` or remove the `auto_schema` setting from the configuration.

Keep in mind that the `/bulk` HTTP endpoint does not support automatic table creation.

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
This setting controls the binary log transaction flush/sync mode. It is optional, with a default value of 2 (flush every transaction, sync every second).

The directive determines how frequently the binary log will be flushed to the OS and synced to disk. There are three supported modes:

*  0, flush and sync every second. This offers the best performance, but up to 1 second worth of committed transactions can be lost in the event of a server crash or an OS/hardware crash.
*  1, flush and sync every transaction. This mode provides the worst performance but guarantees that every committed transaction's data is saved.
*  2, flush every transaction, sync every second. This mode delivers good performance and ensures that every committed transaction is saved in case of a server crash. However, in the event of an OS/hardware crash, up to 1 second worth of committed transactions can be lost.

For those familiar with MySQL and InnoDB, this directive is similar to `innodb_flush_log_at_trx_commit`. In most cases, the default hybrid mode 2 provides a nice balance of speed and safety, with full RT table data protection against server crashes and some protection against hardware ones.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_flush = 1 # ultimate safety, low speed
```
<!-- end -->


### binlog_max_log_size

<!-- example conf binlog_max_log_size -->
This setting controls the maximum binary log file size. It is optional, with a default value of 268435456, or 256 MB.

A new binlog file will be forcibly opened once the current binlog file reaches this size limit. This results in a finer granularity of logs and can lead to more efficient binlog disk usage under certain borderline workloads. A value of 0 indicates that the binlog file should not be reopened based on size.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
binlog_max_log_size = 16M
```
<!-- end -->


### binlog_path

<!-- example conf binlog_path -->
This setting determines the path for binary log (also known as transaction log) files. It is optional, with a default value of the build-time configured data directory (e.g., `/var/lib/manticore/data/binlog.*` in Linux).

Binary logs are used for crash recovery of RT table data and for attribute updates of plain disk indices that would otherwise only be stored in RAM until flush. When logging is enabled, every transaction COMMIT-ted into an RT table is written into a log file. Logs are then automatically replayed on startup after an unclean shutdown, recovering the logged changes.

The `binlog_path` directive specifies the location of binary log files. It should only contain the path; `searchd` will create and unlink multiple `binlog.*` files in the directory as necessary (including binlog data, metadata, and lock files, etc).

An empty value disables binary logging, which improves performance but puts the RT table data at risk.


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
This setting determines the maximum time to wait between requests (in seconds or [special_suffixes](../Server_settings/Special_suffixes.md)) when using persistent connections. It is optional, with a default value of five minutes.


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
When specified, this setting enables the [real-time mode](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29), which is an imperative way of managing data schema. The value should be a path to the directory where you want to store all your tables, binary logs, and everything else needed for the proper functioning of Manticore Search in this mode.
Indexing of [plain tables](../Creating_a_table/Local_tables/Plain_table.md) is not allowed when the `data_dir` is specified. Read more about the difference between the RT mode and the plain mode in [this section](../Read_this_first.md#Real-time-table-vs-plain-table).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
data_dir = /var/lib/manticore
```
<!-- end -->

### docstore_cache_size

<!-- example conf docstore_cache_size -->
This setting specifies the maximum size of document blocks from document storage that are held in memory. It is optional, with a default value of 16m (16 megabytes).

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
This setting determines the maximum number of expanded keywords for a single wildcard. It is optional, with a default value of 0 (no limit).

When performing substring searches against tables built with `dict = keywords` enabled, a single wildcard may potentially result in thousands or even millions of matched keywords (think of matching `a*` against the entire Oxford dictionary). This directive allows you to limit the impact of such expansions. Setting `expansion_limit = N` restricts expansions to no more than N of the most frequent matching keywords (per each wildcard in the query).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
expansion_limit = 16
```
<!-- end -->    


### grouping_in_utc

This setting specifies whether timed grouping in API and SQL will be calculated in the local timezone or in UTC. It is optional, with a default value of 0 (meaning 'local timezone').

By default, all 'group by time' expressions (like group by day, week, month, and year in API, also group by day, month, year, yearmonth, yearmonthday in SQL) are done using local time. For example, if you have documents with attributes timed `13:00 utc` and `15:00 utc`, in the case of grouping, they both will fall into facility groups according to your local timezone setting. If you live in `utc`, it will be one day, but if you live in `utc+10`, then these documents will be matched into different `group by day` facility groups (since 13:00 utc in UTC+10 timezone is 23:00 local time, but 15:00 is 01:00 of the next day). Sometimes such behavior is unacceptable, and it is desirable to make time grouping not dependent on timezone. You can run the server with a defined global TZ environment variable, but it will affect not only grouping but also timestamping in the logs, which may be undesirable as well. Switching 'on' this option (either in config or using [SET global](../Server_settings/Setting_variables_online.md#SET) statement in SQL) will cause all time grouping expressions to be calculated in UTC, leaving the rest of time-depentend functions (i.e. logging of the server) in local TZ.


### ha_period_karma

<!-- example conf ha_period_karma -->
This setting specifies the agent mirror statistics window size, in seconds (or [special_suffixes](../Server_settings/Special_suffixes.md)). It is optional, with a default value of 60 seconds.

For a distributed table with agent mirrors in it (see more in [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md),  the master tracks several different per-mirror counters. These counters are then used for failover and balancing (the master picks the best mirror to use based on the counters). Counters are accumulated in blocks of `ha_period_karma` seconds.

After beginning a new block, the master may still use the accumulated values from the previous one until the new one is half full. As a result, any previous history stops affecting the mirror choice after 1.5 times ha_period_karma seconds at most.

Even though at most two blocks are used for mirror selection, up to 15 last blocks are stored for instrumentation purposes. These blocks can be inspected using the [SHOW AGENT STATUS](../Node_info_and_management/Node_status.md#SHOW-AGENT-STATUS) statement.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_period_karma = 2m
```
<!-- end -->


### ha_ping_interval

<!-- example conf ha_ping_interval -->
This setting configures the interval between agent mirror pings, in milliseconds (or [special_suffixes](../Server_settings/Special_suffixes.md)).  It is optional, with a default value of 1000 milliseconds.

For a distributed table with agent mirrors in it (see more in [agent](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)),  the master sends all mirrors a ping command during idle periods. This is to track the current agent status (alive or dead, network roundtrip, etc). The interval between such pings is defined by this directive. To disable pings, set ha_ping_interval to 0.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
ha_ping_interval = 3s
```
<!-- end -->


### hostname_lookup

The hostname_lookup setting determines the hostnames renew strategy. By default, IP addresses of agent host names are cached at server start to avoid extra load on the DNS. In some cases, the IP can change dynamically (e.g. cloud hosting) t' disables the caching and queries the DNS at each query. The IP addresses can also be manually renewed with the `FLUSH HOSTNAMES` command.

### jobs_queue_size

The jobs_queue_size setting defines how many "jobs" can be in the queue at the same time. It is unlimited by default.

In most cases, a "job" means one query to a single local table (plain table or a disk chunk of a real-time table). For example, if you have a distributed table consisting of 2 local tables or a real-time table with 2 disk chunks, a search query to either of them will mostly put 2 jobs in the queue. Then, the thread pool (whose size is defined by [threads](../Server_settings/Searchd.md#threads) will process them. However, in some cases, if the query is too complex, more jobs can be created. Changing this setting is recommended when [max_connections](../Server_settings/Searchd.md#max_connections) and [threads](../Server_settings/Searchd.md#threads) are not enough to find a balance between the desired performance. 

### listen_backlog

<!-- example conf listen_backlog -->
The listen_backlog setting determines the length of the TCP listen backlog for incoming connections. This is particularly relevant for Windows builds that process requests one by one. When the connection queue reaches its limit, new incoming connections will be refused.
For non-Windows builds, the default value should work fine, and there is usually no need to adjust this setting.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
listen_backlog = 20
```
<!-- end -->


### listen

<!-- example conf listen -->
This setting lets you specify an IP address and port, or Unix-domain socket path, that Manticore will accept connections on.

The general syntax for `listen` is:

```ini
listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol [ "_vip" ] [ "_readonly" ] ]
```

You can specify:
* either an IP address (or hostname) and a port number
* or just a port number
* or a Unix socket path (not supported on Windows)
*  or an IP address and port range

If you specify a port number but not an address, `searchd` will listen on all network interfaces. Unix path is identified by a leading slash. Port range can be set only for the replication protocol.

You can also specify a protocol handler (listener) to be used for connections on this socket. The listeners are:

* **Not specified** - Manticore will accept connections at this port from:
  - other Manticore agents (i.e., a remote distributed table)
  - clients via HTTP and HTTPS
* `mysql` MySQL protocol for connections from MySQL clients. Note:
  - Compressed protocol is also supported.
  - If [SSL](../Security/SSL.md#SSL) is enabled, you can make an encrypted connection.
* `replication` - replication protocol used for nodes communication. More details can be found in the [replication](../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) section. You can specify multiple replication listeners, but they must all listen on the same IP; only the ports can be different.
* `http` - same as **Not specified**. Manticore will accept connections at this port from remote agents and clients via HTTP and HTTPS.
* `https` - HTTPS protocol. Manticore will accept **only** HTTPS connections at this port. More details can be found in section [SSL](../Security/SSL.md).
* `sphinx` - legacy binary protocol. Used to serve connections from remote [SphinxSE](../Extensions/SphinxSE.md) clients. Some Sphinx API clients implementations (an example is the Java one) require the explicit declaration of the listener.

Adding suffix `_vip` to client protocols (that is, all except `replication`, for instance `mysql_vip` or `http_vip` or just `_vip`) forces creating a dedicated thread for the connection to bypass different limitations. That's useful for node maintenance in case of severe overload when the server would either stall or not let you connect via a regular port otherwise.

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

There can be multiple `listen` directives. `searchd` will listen for client connections on all specified ports and sockets. The default config provided in Manticore packages defines listening on ports:
* `9308` and `9312` for connections from remote agents and non-MySQL based clients
* and on port `9306` for MySQL connections.

If you don't specify any `listen` in the configuration at all, Manticore will wait for connections on:
* `127.0.0.1:9306` for MySQL clients
* `127.0.0.1:9312`  for HTTP/HTTPS and connections from other Manticore nodes and clients based on the Manticore binary API.

#### Listening on privileged ports

By default, Linux won't allow you to let Manticore listen on a port below 1024 (e.g. `listen = 127.0.0.1:80:http` or `listen = 127.0.0.1:443:https`) unless you run searchd under root. If you still want to be able to start Manticore, so it listens on ports < 1024 under a non-root user, consider doing one of the following (either of these should work):
* Run the command `setcap CAP_NET_BIND_SERVICE=+eip /usr/bin/searchd`
* Add `AmbientCapabilities=CAP_NET_BIND_SERVICE` to Manticore's systemd unit (`/lib/systemd/system-generators/manticore-search-generator`) and reload the daemon (`systemctl daemon-reload`).

#### Technical details about Sphinx API protocol and TFO
<details>
Legacy Sphinx protocol has 2 phases: handshake exchanging and data flow. The handshake consists of a packet of 4 bytes from the client, and a packet of 4 bytes from the daemon with only one purpose - the client determines that the remote is a real Sphinx daemon, the daemon determines that the remote is a real Sphinx client. The main dataflow is quite simple: let's both sides declare their handshakes, and the opposite check them. That exchange with short packets implies using special `TCP_NODELAY` flag, which switches off Nagle's TCP algorithm and declares that the TCP connection will be performed as a dialogue of small packages.
However, it is not strictly defined who speaks first in this negotiation. Historically, all clients that use the binary API speak first: send handshake, then read 4 bytes from a daemon, then send a request and read an answer from the daemon. 
When we improved Sphinx protocol compatibility, we considered these things:

1. Usually, master-agent communication is established from a known client to a known host on a known port. So, it is quite not possible that the endpoint will provide a wrong handshake. So, we may implicitly assume that both sides are valid and really speak in Sphinx proto.
2. Given this assumption, we can 'glue' a handshake to the real request and send it in one packet. If the backend is a legacy Sphinx daemon, it will just read this glued packet as 4 bytes of a handshake, then request body. Since they both came in one packet, the backend socket has -1 RTT, and the frontend buffer still works despite that fact usual way.
3. Continuing the assumption: since the 'query' packet is quite small, and the handshake is even smaller, let's send both in the initial 'SYN' TCP package using modern TFO (tcp-fast-open) technique. That is: we connect to a remote node with the glued handshake + body package. The daemon accepts the connection and immediately has both the handshake and the body in a socket buffer, as they came in the very first TCP 'SYN' packet. That eliminates another one RTT.
4. Finally, teach the daemon to accept this improvement. Actually, from the application, it implies NOT to use `TCP_NODELAY`. And, from the system side, it implies to ensure that on the daemon side, accepting TFO is activated, and on the client side, sending TFO is also activated. By default, in modern systems, client TFO is already activated by default, so you only have to tune the server TFO for all things to work.

All these improvements without actually changing the protocol itself allowed us to eliminate 1.5 RTT of the TCP protocol from the connection. Which is, if the query and answer are capable of being placed in a single TCP package, decreases the whole binary API session from 3.5 RTT to 2 RTT - which makes network negotiation about 2 times faster.

So, all our improvements are stated around an initially undefined statement: 'who speaks first.' If a client speaks first, we may apply all these optimizations and effectively process connect + handshake + query in a single TFO package. Moreover, we can look at the beginning of the received package and determine a real protocol. That is why you can connect to one and the same port via API/http/https. If the daemon has to speak first, all these optimizations are impossible, and the multiprotocol is also impossible. That is why we have a dedicated port for MySQL and did not unify it with all the other protocols into a same port. Suddenly, among all clients, one was written implying that daemon should send a handshake first. That is - no possibility to all the described improvements. That is SphinxSE plugin for mysql/mariadb. So, specially for this single client we dedicated `sphinx` proto definition to work most legacy way. Namely: both sides activate `TCP_NODELAY` and exchange with small packages. The daemon sends its handshake on connect, then the client sends its, and then everything works usual way. That is not very optimal, but just works. If you use SphinxSE to connect to Manticore - you have to dedicate a listener with explicitly stated `sphinx` proto. For another clients - avoid to use this listener as it is slower. If you use another legacy Sphinx API clients - check first, if they are able to work with non-dedicated multiprotocol port. For master-agent linkage using the non-dedicated (multiprotocol) port and enabling client and server TFO works well and will definitely make working of network backend faster, especially if you have very light and fast queries.
</details>

### listen_tfo

This setting allows the TCP_FASTOPEN flag for all listeners. By default, it is managed by the system but may be explicitly switched off by setting it to '0'.

For general knowledge about the TCP Fast Open extension, please consult with [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). In short, it allows the elimination of one TCP round-trip when establishing a connection.

In practice, using TFO in many situations may optimize client-agent network efficiency, as if [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md) are in play, but without holding active connections, and also without limitation for the maximum num of connections.

On modern OS, TFO support is usually switched 'on' at the system level, but this is just a 'capability', not the rule. Linux (as the most progressive) has supported it since 2011, on kernels starting from 3.7 (for the server-side). Windows has supported it from some builds of Windows 10. Other operating systems (FreeBSD, MacOS) are also in the game.

For Linux system server checks variable `/proc/sys/net/ipv4/tcp_fastopen` and behaves according to it. Bit 0 manages client side, bit 1 rules listeners. By default, the system has this parameter set to 1, i.e., clients enabled, listeners disabled.

### log

<!-- example conf log -->
The log setting specifies the name of the log file where all `searchd` run time events will be logged. If not specified, the default name is 'searchd.log'.

Alternatively, you can use the 'syslog' as the file name. In this case, the events will be sent to the syslog daemon. To use the syslog option, you need to configure Manticore with the `-–with-syslog` option during building.


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

Makes searchd perform a sanity check of the amount of queries submitted in a single batch when using [multi-queries](../Searching/Multi-queries.md). Set it to 0 to skip the check.


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
Instance-wide limit of threads one operation can use. By default, appropriate operations can occupy all CPU cores, leaving no room for other operations. For example, `call pq` against a considerably large percolate table can utilize all threads for tens of seconds. Setting `max_threads_per_query` to, say, half of [threads](../Server_settings/Searchd.md#threads) will ensure that you can run a couple of such `call pq` operations in parallel.

You can also set this setting as a session or a global variable during runtime.

Additionally, you can control the behavior on a per-query basis with the help of the [threads OPTION](../Searching/Options.md#threads).

<!-- intro -->
##### Example:
<!-- request Example -->

```ini
max_threads_per_query = 4
```

<!-- end -->

### max_filters

<!-- example conf max_filters -->
Maximum allowed per-query filter count. This setting is only used for internal sanity checks and does not directly affect RAM usage or performance. Optional, the default is 256.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_filters = 1024
```
<!-- end -->


### max_filter_values

<!-- example conf max_filter_values -->
Maximum allowed per-filter values count. This setting is only used for internal sanity checks and does not directly affect RAM usage or performance. Optional, the default is 4096.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_filter_values = 16384
```
<!-- end -->


### max_open_files

<!-- example conf max_open_files -->
The maximum number of files that the server is allowed to open is called the "soft limit". Note that serving large fragmented real-time tables may require this limit to be set high, as each disk chunk may occupy a dozen or more files. For example, a real-time table with 1000 chunks may require thousands of files to be opened simultaneously. If you encounter the error 'Too many open files' in the logs, try adjusting this option, as it may help resolve the issue.

There is also a "hard limit" that cannot be exceeded by the option. This limit is defined by the system and can be changed in the file `/etc/security/limits.conf` on Linux. Other operating systems may have different approaches, so consult your manuals for more information.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_open_files = 10000
```
<!-- end -->

<!-- example conf max_open_files max -->
Apart from direct numeric values, you can use the magic word 'max' to set the limit equal to the available current hard limit.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_open_files = max
```
<!-- end -->


### max_packet_size

<!-- example conf max_packet_size -->
Maximum allowed network packet size. This setting limits both query packets from clients and response packets from remote agents in a distributed environment. Only used for internal sanity checks, it does not directly affect RAM usage or performance. Optional, the default is 8M.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
max_packet_size = 32M
```
<!-- end -->


### mysql_version_string

<!-- example conf mysql_version_string -->
A server version string to return via the MySQL protocol. Optional, the default is empty (returns the Manticore version).

Several picky MySQL client libraries depend on a particular version number format used by MySQL, and moreover, sometimes choose a different execution path based on the reported version number (rather than the indicated capabilities flags). For instance, Python MySQLdb 1.2.2 throws an exception when the version number is not in X.Y.ZZ format; MySQL .NET connector 6.3.x fails internally on version numbers 1.x along with a certain combination of flags, etc. To work around that, you can use the `mysql_version_string` directive and have `searchd` report a different version to clients connecting over the MySQL protocol. (By default, it reports its own version.)


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
mysql_version_string = 5.0.37
```
<!-- end -->


### net_workers

Number of network threads, the default is 1.

This setting is useful for extremely high query rates when just one thread is not enough to manage all the incoming queries.


### net_wait_tm

Controls the busy loop interval of the network thread. The default is -1, and it can be set to -1, 0, or a positive integer.

In cases where the server is configured as a pure master and just routes requests to agents, it is important to handle requests without delays and not allow the network thread to sleep. There is a busy loop for that. After an incoming request, the network thread uses CPU poll for `10 * net_wait_tm` milliseconds if 
 `net_wait_tm` is a positive number or polls only with the CPU if`net_wait_tm` is `0`.  Also, the busy loop can be disabled with `net_wait_tm = -1` - in this case, the poller sets the timeout to the actual agent's timeouts on the system polling call.

> **WARNING:** A CPU busy loop actually loads the CPU core, so setting this value to any non-default value will cause noticeable CPU usage even with an idle server.


### net_throttle_accept

Defines how many clients are accepted on each iteration of the network loop. Default is 0 (unlimited), which should be fine for most users. This is a fine-tuning option to control the throughput of the network loop in high load scenarios.


### net_throttle_action

Defines how many requests are processed on each iteration of the network loop. The default is 0 (unlimited), which should be fine for most users. This is a fine-tuning option to control the throughput of the network loop in high load scenarios.

### network_timeout

<!-- example conf network_timeout -->
Network client request read/write timeout, in seconds (or  [special_suffixes](../Server_settings/Special_suffixes.md)). Optional, the default is 5 seconds. `searchd` will forcibly close a client connection which fails to send a query or read a result within this timeout.

<!-- request Example -->

```ini
network_timeout = 10s
```
<!-- end -->

### node_address

<!-- example conf node_address -->
This setting allows you to specify the network address of the node. By default, it is set to the replication [listen](../Server_settings/Searchd.md#listen) ddress. This is correct in most cases; however, there are situations where you have to specify it manually:

* Node behind a firewall
* Network address translation enabled (NAT)
* Container deployments, such as Docker or cloud deployments
* Clusters with nodes in more than one region


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
node_address = 10.101.0.10
```
<!-- end -->

### not_terms_only_allowed

<!-- example conf not_terms_only_allowed -->
This setting determines whether to allow queries with only the [negation](../Searching/Full_text_matching/Operators.md#Negation-operator) full-text operator. Optional, the default is 0 (fail queries with only the NOT operator).


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
not_terms_only_allowed = 1
```
<!-- end -->

### optimize_cutoff

<!-- example conf optimize_cutoff -->
Sets the default table compaction threshold. Read more here - [Number of optimized disk chunks](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). This setting can be overridden with the per-query option [cutoff](../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks). It can also be changed dynamically via [SET GLOBAL](../Server_settings/Setting_variables_online.md#SET).

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
optimize_cutoff = 4
```
<!-- end -->

### persistent_connections_limit

<!-- example conf persistent_connections_limit -->
This setting determines the maximum number of simultaneous persistent connections to remote [persistent agents](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md). Each time an agent defined under `agent_persistent` is connected, we try to reuse an existing connection (if any), or connect and save the connection for future use. However, in some cases, it makes sense to limit the number of such persistent connections. This directive defines the limit. It affects the number of connections to each agent's host across all distributed tables.

It is reasonable to set the value equal to or less than the [max_connections](../Server_settings/Searchd.md#max_connections) option in the agent's config.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```
<!-- end -->


### pid_file

<!-- example conf pid_file -->
pid_file is a mandatory configuration option in Manticore search that specifies the path of the file where the process ID of the `searchd` server is stored.

The searchd process ID file is re-created and locked on startup, and contains the head server process ID while the server is running. It is unlinked on server shutdown.
The purpose of this file is to enable Manticore to perform various internal tasks, such as checking whether there is already a running instance of `searchd`, stopping `searchd`, and notifying it that it should rotate the tables. The file can also be used for external automation scripts.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
pid_file = /var/run/manticore/searchd.pid
```
<!-- end -->


### predicted_time_costs

<!-- example conf predicted_time_costs -->
Costs for the query time prediction model, in nanoseconds. Optional, the default is `doc=64, hit=48, skip=2048, match=64`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
predicted_time_costs = doc=128, hit=96, skip=4096, match=128
```
<!-- end -->

<!-- example conf predicted_time_costs 1 -->
Terminating queries before completion based on their execution time (with the max query time setting) is a nice safety net, but it comes with an inherent drawback: indeterministic (unstable) results. That is, if you repeat the very same (complex) search query with a time limit several times, the time limit will be hit at different stages, and you will get *different* result sets.

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

Of course, this is not a hard limit on the actual time spent (it is, however, a hard limit on the amount of *processing* work done), and a simple linear model is in no way an ideally precise one. So the wall clock time *may* be either below or over the target limit. However, the error margins are quite acceptable: for instance, in our experiments with a 100 msec target limit, the majority of the test queries fell into a 95 to 105 msec range, and *all* the queries were in an 80 to 120 msec range. Also, as a nice side effect, using the modeled query time instead of measuring the actual run time results in somewhat fewer gettimeofday() calls, too.

No two server makes and models are identical, so the `predicted_time_costs` directive lets you configure the costs for the model above. For convenience, they are integers, counted in nanoseconds. (The limit in max_predicted_time is counted in milliseconds, and having to specify cost values as 0.000128 ms instead of 128 ns is somewhat more error-prone.) It is not necessary to specify all four costs at once, as the missed ones will take the default values. However, we strongly suggest specifying all of them for readability.


### preopen_tables

<!-- example conf preopen_tables -->
The preopen_tables configuration directive specifies whether to forcibly preopen all tables on startup. The default value is 1, which means that all tables will be preopened regardless of the per-table [preopen](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#Other-performance-related-settings) setting. If set to 0, the per-table settings can take effect, and they will default to 0.

Pre-opening tables can prevent races between search queries and rotations that can cause queries to fail occasionally. However, it also uses more file handles. In most scenarios, it is recommended to preopen tables.

Here's an example configuration:

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
preopen_tables = 1
```
<!-- end -->

### pseudo_sharding

<!-- example conf pseudo_sharding -->
The pseudo_sharding configuration option enables parallelization of search queries to local plain and real-time tables, regardless of whether they are queried directly or through a distributed table. This feature will automatically parallelize queries to up to the number of threads specified in `searchd.threads` # of threads.

Note that if your worker threads are already busy, because you have:
* high query concurrency
* physical sharding of any kind:
  - distributed table of multiple plain/real-time tables
  - real-time table consisting of too many disk chunks

then enabling pseudo_sharding may not provide any benefits and may even result in a slight decrease in throughput. If you prioritize higher throughput over lower latency, it's recommended to disable this option.

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
This configuration sets the maximum amount of RAM allocated for cached result sets in bytes. The default value is 16777216, which is equivalent to 16 megabytes. If the value is set to 0, the query cache is disabled. For more information about the query cache, please refer to the [query cache](../Searching/Query_cache.md) for details.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
qcache_max_bytes = 16777216
```
<!-- end -->


### qcache_thresh_msec

Integer, in milliseconds. The minimum wall time threshold for a query result to be cached. Defaults to 3000, or 3 seconds. 0 means cache everything. Refer to [query cache](../Searching/Query_cache.md) for details. This value also may be expressed with time [special_suffixes](../Server_settings/Special_suffixes.md), but use it with care and don't confuse yourself with the name of the value itself, containing '_msec'.


### qcache_ttl_sec

Integer, in seconds. The expiration period for a cached result set. Defaults to 60, or 1 minute. The minimum possible value is 1 second. Refer to [query cache](../Searching/Query_cache.md) for details. This value also may be expressed with time [special_suffixes](../Server_settings/Special_suffixes.md), but use it with care and don't confuse yourself with the name of the value itself, containing '_sec'.


### query_log_format

<!-- example conf query_log_format -->
Query log format. Optional, allowed values are `plain` and `sphinxql`, default is `sphinxql`.

The `sphinxql` mode logs valid SQL statements. The `plain` mode logs queries in a plain text format (mostly suitable for purely full-text use cases). This directive allows you to switch between the two formats on search server startup. The log format can also be altered on the fly, using `SET GLOBAL query_log_format=sphinxql` syntax. Refer to [Query logging](../Logging/Query_logging.md) for more details.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
query_log_format = sphinxql
```
<!-- end -->

### query_log_min_msec

Limit (in milliseconds) that prevents the query from being written to the query log. Optional, default is 0 (all queries are written to the query log). This directive specifies that only queries with execution times that exceed the specified limit will be logged (this value also may be expressed with time [special_suffixes](../Server_settings/Special_suffixes.md), but use it with care and don't confuse yourself with the name of the value itself, containing `_msec`).

### query_log

<!-- example conf query_log -->
Query log file name. Optional, default is empty (do not log queries). All search queries (such as SELECT ... but not INSERT/REPLACE/UPDATE queries) will be logged in this file. The format is described in [Query logging](../Logging/Query_logging.md). In case of 'plain' format, you can use 'syslog' as the path to the log file. In this case, all search queries will be sent to the syslog daemon with `LOG_INFO` priority, prefixed with '[query]' instead of timestamp. To use the syslog option, Manticore must be configured with  `-–with-syslog` on building.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
query_log = /var/log/query.log
```
<!-- end -->


### query_log_mode

<!-- example conf query_log_mode -->
The query_log_mode directive allows you to set a different permission for the searchd and query log files. By default, these log files are created with 600 permission, meaning that only the user under which the server runs and root users can read the log files.
This directive can be handy if you want to allow other users to read the log files, for example, monitoring solutions running on non-root users. 

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
query_log_mode  = 666
```
<!-- end -->

### read_buffer_docs

<!-- example conf read_buffer_docs -->
The read_buffer_docs directive controls the per-keyword read buffer size for document lists. For every keyword occurrence in every search query, there are two associated read buffers: one for the document list and one for the hit list. This setting lets you control the document list buffer size.

A larger buffer size might increase per-query RAM use, but it could possibly decrease I/O time. It makes sense to set larger values for slow storage, but for storage capable of high IOPS, experimenting should be done in the low values area. 

The default value is 256K, and the minimal value is 8K. You may also set [read_buffer_docs](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_docs) on a per-table basis, which will override anything set on the server's config level.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
read_buffer_docs = 128K
```
<!-- end -->


### read_buffer_hits

<!-- example conf read_buffer_hits -->
The read_buffer_hits directive specifies the per-keyword read buffer size for hit lists in search queries. By default, the size is 256K and the minimum value is 8K. For every keyword occurrence in a search query, there are two associated read buffers, one for the document list and one for the hit list. Increasing the buffer size can increase per-query RAM use but decrease I/O time. For slow storage, larger buffer sizes make sense, while for storage capable of high IOPS, experimenting should be done in the low values area.

This setting can also be specified on a per-table basis using the read_buffer_hits option in [read_buffer_hits](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#read_buffer_hits) which will override the server-level setting.

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

When querying, some reads know in advance exactly how much data is there to be read, but some currently do not. Most prominently, hit list size is not currently known in advance. This setting lets you control how much data to read in such cases. It impacts hit list I/O time, reducing it for lists larger than unhinted read size, but raising it for smaller lists. It does **not** affect RAM usage because the read buffer will already be allocated. So it should not be greater than read_buffer.


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

Actively updated RT tables that fully fit in RAM chunks can still result in ever-growing binlogs, impacting disk use and crash recovery time. With this directive, the search server performs periodic flush checks, and eligible RAM chunks can be saved, enabling consequential binlog cleanup. See [Binary logging](../Logging/Binary_logging.md) for more details.

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

This directive lets you throttle down the I/O impact arising from the `OPTIMIZE` statements. It is guaranteed that all RT optimization activities will not generate more disk IOPS (I/Os per second) than the configured limit. Limiting rt_merge_iops can reduce search performance degradation caused by merging.

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

This directive lets you throttle down the I/O impact arising from the `OPTIMIZE` statements. I/Os larger than this limit will be broken down into two or more I/Os, which will then be accounted for as separate I/Os with regards to the [rt_merge_iops](../Server_settings/Searchd.md#rt_merge_iops) limit. Thus, it is guaranteed that all optimization activities will not generate more than (rt_merge_iops * rt_merge_maxiosize) bytes of disk I/O per second.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
rt_merge_maxiosize = 1M
```
<!-- end -->


### seamless_rotate

<!-- example conf seamless_rotate -->
Prevents `searchd` stalls while rotating tables with huge amounts of data to precache. Optional, default is 1 (enable seamless rotation). On Windows systems, seamless rotation is disabled by default.

Tables may contain some data that needs to be precached in RAM. At the moment, `.spa`, `.spb`, `.spi`, and `.spm` files are fully precached (they contain attribute data, blob attribute data, keyword table, and killed row map, respectively.) Without seamless rotate, rotating a table tries to use as little RAM as possible and works as follows:

1. New queries are temporarily rejected (with "retry" error code);
2. `searchd` waits for all currently running queries to finish;
3. The old table is deallocated, and its files are renamed;
4. New table files are renamed, and required RAM is allocated;
5. New table attribute and dictionary data are preloaded to RAM;
6. `searchd` resumes serving queries from the new table.

However, if there's a lot of attribute or dictionary data, then the preloading step could take a noticeable amount of time - up to several minutes in the case of preloading 1-5+ GB files.

With seamless rotate enabled, rotation works as follows:

1. New table RAM storage is allocated;
2. New table attribute and dictionary data are asynchronously preloaded to RAM;
3. On success, the old table is deallocated, and both tables' files are renamed;
4. On failure, the new table is deallocated;
5. At any given moment, queries are served either from the old or new table copy.

Seamless rotate comes at the cost of higher peak memory usage during the rotation (because both old and new copies of `.spa/.spb/.spi/.spm` data need to be in RAM while preloading the new copy). Average usage remains the same.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
seamless_rotate = 1
```
<!-- end -->

### secondary_indexes
<!-- example conf secondary_indexes -->

This option enables/disables the use of secondary indexes for search queries. It is optional, and the default is 1 (enabled). Note that you don't need to enable it for indexing as it is always enabled as long as the [Manticore Columnar Library](https://github.com/manticoresoftware/columnar) is installed. The latter is also required for using the indexes when searching. There are three modes available:

* `0`: Disable the use of secondary indexes on search. They can be enabled for individual queries using [analyzer hints](../Searching/Options.md#Query-optimizer-hints)
* `1`: Enable the use of secondary indexes on search. They can be disabled for individual queries using [analyzer hints](../Searching/Options.md#Query-optimizer-hints)
* `force`: Same as enable, but any errors during the loading of secondary indexes will be reported, and the whole index will not be loaded into the daemon.

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
Integer number that serves as a server identifier used as a seed to generate a unique short UUID for nodes that are part of a replication cluster. The server_id must be unique across the nodes of a cluster and in the range from 0 to 127. If server_id is not set, the MAC address or a random number will be used as a seed for the short UUID.


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

When you run `searchd --stopwait` your server needs to perform some activities before stopping, such as finishing queries, flushing RT RAM chunks, flushing attributes, and updating the binlog. These tasks require some time. `searchd --stopwait` will wait up to `shutdown_time` seconds for the server to finish its jobs. The suitable time depends on your table size and load.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
shutdown_timeout = 3m # wait for up to 3 minutes
```
<!-- end -->


### shutdown_token

SHA1 hash of the password required to invoke the 'shutdown' command from a VIP Manticore SQL connection. Without it,[debug](../Reporting_bugs.md#DEBUG) 'shutdown' subcommand will never cause the server to stop. Note that such simple hashing should not be considered strong protection, as we don't use a salted hash or any kind of modern hash function. It is intended as a fool-proof measure for housekeeping daemons in a local network.

### snippets_file_prefix

<!-- example conf snippets_file_prefix -->
A prefix to prepend to the local file names when generating snippets. Optional, default is the current working folder.

This prefix can be used in distributed snippets generation along with `load_files` or `load_files_scattered` options.

Note that this is a prefix and **not** a path! This means that if a prefix is set to "server1" and the request refers to "file23", `searchd` will attempt to open "server1file23" (all of that without quotes). So, if you need it to be a path, you have to include the trailing slash.

After constructing the final file path, the server unwinds all relative dirs and compares the final result with the value of `snippet_file_prefix`. If the result does not begin with the prefix, such a file will be rejected with an error message.

For example, if you set it to `/mnt/data` and someone calls snippet generation with the file `../../../etc/passwd` as the source, they will get the error message:

`File '/mnt/data/../../../etc/passwd' escapes '/mnt/data/' scope`

instead of the content of the file.

Also, with a non-set parameter and reading `/etc/passwd`, it will actually read /daemon/working/folder/etc/passwd since the default for the parameter is the server's working folder.

Note also that this is a local option; it does not affect the agents in any way. So you can safely set a prefix on a master server. The requests routed to the agents will not be affected by the master's setting. They will, however, be affected by the agent's own settings.

This might be useful, for instance, when the document storage locations (whether local storage or NAS mountpoints) are inconsistent across the servers.


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
Path to a file where the current SQL state will be serialized.

On server startup, this file gets replayed. On eligible state changes (e.g., SET GLOBAL), this file gets rewritten automatically. This can prevent a hard-to-diagnose problem: If you load UDF functions but Manticore crashes, when it gets (automatically) restarted, your UDF and global variables will no longer be available. Using persistent state helps ensure a graceful recovery with no such surprises.

`sphinxql_state` cannot be used to execute arbitrary commands, such as `CREATE TABLE`.

<!-- intro -->
##### Example:

<!-- request Example -->

```ini
sphinxql_state = uservars.sql
```
<!-- end -->


### sphinxql_timeout

<!-- example conf sphinxql_timeout -->
Maximum time to wait between requests (in seconds, or [special_suffixes](../Server_settings/Special_suffixes.md)) when using the SQL interface. Optional, default is 15 minutes.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
sphinxql_timeout = 15m
```
<!-- end -->


### ssl_ca

<!-- example conf ssl_ca -->
Path to the SSL Certificate Authority (CA) certificate file (also known as root certificate). Optional, default is empty. When not empty, the certificate in `ssl_cert` should be signed by this root certificate.

The server uses the CA file to verify the signature on the certificate. The file must be in PEM format.

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

The server uses this certificate as a self-signed public key to encrypt HTTP traffic over SSL. The file must be in PEM format.


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

The server uses this private key to encrypt HTTP traffic over SSL. The file must be in PEM format.


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

This setting limits the RAM usage of a common subtree optimizer (see  [multi-queries](../Searching/Multi-queries.md)). At most, this much RAM will be spent to cache document entries for each query. Setting the limit to 0 disables the optimizer.


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

This setting limits the RAM usage of a common subtree optimizer (see [multi-queries](../Searching/Multi-queries.md)). At most, this much RAM will be spent to cache keyword occurrences (hits) for each query. Setting the limit to 0 disables the optimizer.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
subtree_hits_cache = 16M
```
<!-- end -->

### threads

<!-- example threads -->
Number of working threads (or, size of thread pool) for the Manticore daemon. Manticore creates this number of OS threads on start, and they perform all jobs inside the daemon, such as executing queries, creating snippets, etc. Some operations may be split into sub-tasks and executed in parallel, for example:

* Search in a real-time table
* Search in a distributed table consisting of local tables
* Percolate query call
* and others

By default, it's set to the number of CPU cores on the server. Manticore creates the threads on start and keeps them until it's stopped. Each sub-task can use one of the threads when it needs it. When the sub-task finishes, it releases the thread so another sub-task can use it.

In the case of intensive I/O type of load, it might make sense to set the value higher than the number of CPU cores.

<!-- request Example -->
```ini
threads = 10
```

<!-- end -->

### thread_stack

<!-- example conf thread_stack -->
Maximum stack size for a job (coroutine, one search query may cause multiple jobs/coroutines). Optional, default is 128K.

Each job has its own stack of 128K. When you run a query, it's checked for how much stack it requires. If the default 128K is enough, it's just processed. If it needs more, another job with an increased stack is scheduled, which continues processing. The maximum size of such an advanced stack is limited by this setting.

Setting the value to a reasonably high rate will help with processing very deep queries without implying that overall RAM consumption will grow too high. For example, setting it to 1G does not imply that every new job will take 1G of RAM, but if we see that it requires, let's say, 100M stack, we just allocate 100M for the job. Other jobs at the same time will be running with their default 128K stack. The same way, we can run even more complex queries that need 500M. And only if we **see** internally that the job requires more than 1G of stack, we will fail and report about too low thread_stack.

However, in practice, even a query which needs 16M of stack is often too complex for parsing and consumes too much time and resources to be processed. So, the daemon will process it, but limiting such queries by the `thread_stack` setting looks quite reasonable.


<!-- intro -->
##### Example:

<!-- request Example -->

```ini
thread_stack = 8M
```
<!-- end -->


### unlink_old

<!-- example conf unlink_old -->
Determines whether to unlink `.old` table copies on successful rotation. Optional, default is 1 (do unlink).


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

When a Manticore query crashes, it can take down the entire server. With the watchdog feature enabled, `searchd` also maintains a separate lightweight process that monitors the main server process and automatically restarts it in case of abnormal termination. The watchdog is enabled by default.

<!-- request Example -->

```ini
watchdog = 0 # disable watchdog
```
<!-- end -->
<!-- proofread -->