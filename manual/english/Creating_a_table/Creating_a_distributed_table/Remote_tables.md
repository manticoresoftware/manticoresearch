# Remote tables

A remote table in Manticore Search is represented by the [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) prefix in the definition of a distributed table. A distributed table can include a combination of local and remote tables. If there are no local tables provided, the distributed table will be purely remote and serve as a proxy only. For example, you might have a Manticore instance that listens on multiple ports and serves different protocols, and then redirects queries to backend servers that only accept connections via Manticore's internal binary protocol, using persistent connections to reduce the overhead of establishing connections.
Even though a purely remote distributed table doesn't serve local tables itself, it still consumes machine resources, as it still needs to perform final calculations, such as merging results and calculating final aggregated values.

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

`agent` directive declares the remote agents that are searched each time the enclosing distributed table is searched. These agents are essentially pointers to networked tables. The value specified includes the address and can also include multiple alternatives (agent mirrors) for either the address only or the address and table list. See [Mirroring](../../Creating_a_cluster/Remote_nodes/Mirroring.md) for mirror syntax and [Load balancing](../../Creating_a_cluster/Remote_nodes/Load_balancing.md) for how mirrored agents are selected.

The address specification must be one of the following:

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

The `hostname` is the remote host name, `port` is the remote TCP port number, `table-list` is a comma-separated list of table names, and square brackets [] indicate an optional clause. For TCP connections this port is the remote agent/API port (typically `9312`), not the MySQL port (`9306`).

If the table name is omitted, it is assumed to be the same table as the one where this line is defined. In other words, when defining agents for the 'mycoolindex' distributed table, you can simply point to the address, and it will be assumed that you are querying the mycoolindex table on the agent's endpoints.

If the port number is omitted, it is assumed to be **9312**. If it is defined but invalid (e.g. 70000), the agent will be skipped.

You can point each agent to one or more remote tables residing on one or more networked servers with no restrictions. This enables several different usage modes:
* Sharding over multiple agent servers and creating an arbitrary cluster topology
* Sharding over multiple agent servers mirrored for high availability and load balancing purposes (see [Mirroring](../../Creating_a_cluster/Remote_nodes/Mirroring.md) and [Load balancing](../../Creating_a_cluster/Remote_nodes/Load_balancing.md))
* Sharding within localhost to utilize multiple cores (however, it is simpler just to use multiple local tables)

To avoid confusion:
* One `agent='host1|host2:table'` entry means one remote shard with mirrored backends.
* Multiple `agent='...'` entries mean multiple remote shards.

All agents are searched in parallel. The index list is passed verbatim to the remote agent. The exact way that list is searched within the agent (i.e. sequentially or in parallel) depends solely on the agent's configuration (see the [threads](../../Server_settings/Searchd.md#threads) setting). The master has no remote control over this.

It is important to note that the `LIMIT`, option is ignored in agent queries. This is because each agent can contain different tables, so it is the responsibility of the client to apply the limit to the final result set. This is why the query to a physical table is different from the query to a distributed table when viewed in the query logs. The query cannot be a simple copy of the original query, as this would not produce the correct results.

For example, if a client makes a query SELECT ... LIMIT 10, 10, and there are two agents, with the second agent having only 10 documents, broadcasting the original `LIMIT 10, 10` query would result in receiving 0 documents from the second agent. However, `LIMIT 10,10` should return documents 10-20 from the resulting set. To resolve this, the query must be sent to the agents with a broader limit, such as the default max_matches value of 1000.

For instance, if there is a distributed table dist that refers to the remote table user, a client query `SELECT * FROM dist LIMIT 10,10` would be converted to `SELECT * FROM user LIMIT 0,1000` and sent to the remote table user. Once the distributed table receives the result, it will apply the LIMIT 10,10 and return the requested 10 documents.

```sql
SELECT * FROM dist LIMIT 10,10;
```

the query will be converted to:

```sql
SELECT * FROM user LIMIT 0,1000
```

Additionally, the value can specify options for each individual agent, such as:
* [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - `random`, `roundrobin`, `nodeads`, `noerrors` (overrides the global `ha_strategy` setting for the particular agent; see also [Mirroring](../../Creating_a_cluster/Remote_nodes/Mirroring.md))
* `conn` - `pconn`, persistent (equivalent to setting `agent_persistent` at the table level)
* `blackhole` `0`,`1` (identical to the [agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) setting for the agent)
* `retry_count` an integer value (corresponding to [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) , but the provided value will not be multiplied by the number of mirrors)

```ini
agent = address1:table-list[[ha_strategy=value, conn=value, blackhole=value]]
```

Example:

```ini
# config on box1
# sharding a table over 3 servers
agent = box2:9312:shard1
agent = box3:9312:shard2

# config on box2
# sharding a table over 3 servers
agent = box1:9312:shard2
agent = box3:9312:shard3

# config on box3
# sharding a table over 3 servers
agent = box1:9312:shard1
agent = box2:9312:shard3

# per agent options
agent = box1:9312:shard1[ha_strategy=nodeads]
agent = box2:9312:shard2[conn=pconn]
agent = box2:9312:shard2[conn=pconn,ha_strategy=nodeads]
agent = test:9312:any[blackhole=1]
agent = test:9312|box2:9312|box3:9312:any2[retry_count=2]
agent = test:9312|box2:9312:any2[retry_count=2,conn=pconn,ha_strategy=noerrors]
```

## Defining remote tables

<!-- example remote-table-definition -->
Remote tables can be defined either in a config file or with SQL on a distributed table.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
table products_dist {
  type  = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
}
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products';
```
<!-- end -->

<!-- example remote-table-setup -->
Assume the remote tables already exist, for example:

<!-- intro -->
##### SQL on remote node 1:

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node1');
```

<!-- intro -->
##### SQL on remote node 2:

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node2');
```

<!-- intro -->
##### SQL on the master:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  ha_strategy='roundrobin'
  agent_connect_timeout='200ms'
  agent_query_timeout='500ms'
  mirror_retry_count='2';
```
<!-- end -->

<!-- example remote-table-sql-usage-and-verification -->
Use the distributed table from the master exactly like any other table, then verify how it was stored.

<!-- intro -->
##### SQL request:

<!-- request SQL -->
```sql
SELECT id, title, node FROM products_dist;
SHOW CREATE TABLE products_dist;
```

<!-- intro -->
##### SQL response:

<!-- response SQL -->
```sql
+------+------------+-------+
| id   | title      | node  |
+------+------------+-------+
|    1 | same title | node1 |
+------+------------+-------+

+---------------+----------------------------------------------------------------------------------+
| Table         | Create Table                                                                     |
+---------------+----------------------------------------------------------------------------------+
| products_dist | CREATE TABLE products_dist type='distributed' agent='127.0.0.1:9312:products|... |
+---------------+----------------------------------------------------------------------------------+
```
<!-- end -->

For optimal performance, it's recommended to place remote tables that reside on the same server within the same record. For instance, instead of:
```ini
agent = remote:9312:idx1
agent = remote:9312:idx2
```
you should prefer:
```ini
agent = remote:9312:idx1,idx2
```

## agent_persistent

<!-- example agent-persistent -->
<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_persistent = remotebox:9312:index2
```
<!-- end -->

The `agent_persistent` option allows you to persistently connect to an agent, meaning the connection will not be dropped after a query is executed. The syntax for this directive is the same as the `agent` directive. However, instead of opening a new connection to the agent for each query and then closing it, the master will keep a connection open and reuse it for subsequent queries. The maximum number of persistent connections per agent host is defined by the [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) option in the searchd section.

It's important to note that the [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) must be set to a value greater than 0 in order to use persistent agent connections. If it's not defined, it defaults to 0, and the `agent_persistent` directive will act the same as the `agent`directive.

Using persistent master-agent connections reduces TCP port pressure and saves time on connection handshakes, making it more efficient.

## agent_blackhole

<!-- example agent-blackhole -->
The `agent_blackhole` directive allows you to forward queries to remote agents without waiting for or processing their responses. This is useful for debugging or testing production clusters, as you can set up a separate debugging/testing instance and forward requests to it from the production master (aggregator) instance, without interfering with production work. The master searchd will attempt to connect to the blackhole agent and send queries as normal, but will not wait for or process any responses, and all network errors on the blackhole agents will be ignored. The format of the value is identical to that of the regular `agent` directive.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_blackhole = testbox:9312:testindex1,testindex2
```
<!-- end -->

## Related options and supported scopes

The options related to remote agents are not all supported at the same scope. This page focuses on remote-table and per-agent semantics. For mirror-specific behavior, see [Mirroring](../../Creating_a_cluster/Remote_nodes/Mirroring.md) and [Load balancing](../../Creating_a_cluster/Remote_nodes/Load_balancing.md). For full daemon-level behavior, use the linked reference pages in [Searchd settings](../../Server_settings/Searchd.md).

| Option | Instance-wide | Per table | Per query | Per agent | Full details |
|---|---|---|---|---|---|
| `ha_strategy` | yes | yes | no | yes | [Load balancing](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), [Remote tables: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_connect_timeout` | yes | yes | no | no | [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout) |
| `agent_query_timeout` | yes | yes | yes | no | [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout) |
| `agent_retry_count` / `mirror_retry_count` / `retry_count` | yes (`agent_retry_count`) | yes (`agent_retry_count` or `mirror_retry_count`) | yes (`OPTION retry_count=...`) | yes (`agent=...[retry_count=...]`) | [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [Remote tables: mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count), [Remote tables: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_retry_delay` | yes | no | yes | no | [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay) |
| per-agent `conn` | no | no | no | yes | [Remote tables: agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| per-agent `blackhole` | no | no | no | yes | [Remote tables: agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) |

### agent_connect_timeout

<!-- example remote-agent-connect-timeout -->
`agent_connect_timeout` defines how long Manticore waits to establish a connection to a remote agent. It is supported as an instance-wide default and per distributed table. Full daemon-level details are in [Searchd: agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout).

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_connect_timeout = 300ms
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  agent_connect_timeout='300ms';
```
<!-- end -->

### agent_query_timeout

<!-- example remote-agent-query-timeout -->
`agent_query_timeout` defines how long Manticore waits for a connected remote agent to finish the query. It is supported as an instance-wide default, per distributed table, and per query as `OPTION agent_query_timeout=...`. Full daemon-level details are in [Searchd: agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout).

If `agent_query_timeout` is reached, the query is not retried automatically; a warning is produced instead. Behavior is also affected by [reset_network_timeout_on_packet](../../Server_settings/Searchd.md#reset_network_timeout_on_packet).

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_query_timeout = 10000 # our query can be long, allow up to 10 sec
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  agent_query_timeout='10000';
```

<!-- intro -->
##### SQL query option:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION agent_query_timeout=750;
```
<!-- end -->

### agent_retry_count

`agent_retry_count` specifies how many times Manticore will attempt to connect to and query remote agents in a distributed table before reporting a fatal query error. The name varies by scope: use 
- `agent_retry_count` as the instance-wide setting, 
- `agent_retry_count` or its alias `mirror_retry_count` on a distributed table, 
- `OPTION retry_count=...` per query, 
- and `[retry_count=...]` inside an individual `agent=...` declaration. 

Full daemon-level details are in [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count).

If you use **agent mirrors**, the server selects a different mirror before each connection attempt according to [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), and `agent_retry_count` is aggregated across mirrors.

### mirror_retry_count

<!-- example remote-agent-retry-count -->
`mirror_retry_count` serves the same purpose as `agent_retry_count`, but only as a distributed-table setting. If both values are provided, `mirror_retry_count` takes precedence.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_retry_count = 2
```

<!-- intro -->
##### SQL:

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products'
  mirror_retry_count='2';
```

<!-- intro -->
##### SQL query option:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=1;
```

<!-- intro -->
##### Config per-agent cap:

<!-- request Config -->
```ini
table products_dist {
  type = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products[retry_count=2]
}
```
<!-- end -->

### agent_retry_delay

<!-- example remote-agent-retry-delay -->
`agent_retry_delay` defines the delay between retry attempts. It is supported as an instance-wide default and per query as `OPTION retry_delay=...`, but not per distributed table. Full daemon-level details are in [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay).

This option is only relevant when retries are enabled through `agent_retry_count` or `OPTION retry_count=...`.

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_retry_delay = 500ms
```

<!-- intro -->
##### SQL query option:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=2, retry_delay=300;
```
<!-- end -->

### client_timeout

<!-- example client-timeout -->
The `client_timeout` option sets the maximum waiting time between requests when using persistent connections. This is an instance-wide `searchd` setting, not a per-table option. This value is expressed in seconds or with a time suffix. The default value is 5 minutes. Full daemon-level details are in [Searchd: client_timeout](../../Server_settings/Searchd.md#client_timeout).

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
client_timeout = 1h
```
<!-- end -->

### hostname_lookup

The `hostname_lookup` option defines the strategy for renewing hostnames. By default, the IP addresses of agent host names are cached at server start to avoid excessive access to DNS. However, in some cases, the IP can change dynamically (e.g. cloud hosting) and it may be desirable to not cache the IPs. Setting this option to `request` disables the caching and queries the DNS for each query. The IP addresses can also be manually renewed using the `FLUSH HOSTNAMES` command.

### listen_tfo

The `listen_tfo` option allows for the use of the TCP_FASTOPEN flag for all listeners. By default, it is managed by the system, but it can be explicitly turned off by setting it to '0'.

For more information about the TCP Fast Open extension, please refer to [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open).  In short, it allows to eliminate one TCP round-trip when establishing a connection.

In practice, using TFO can optimize the client-agent network efficiency, similar to when `agent_persistent`  is in use, but without holding active connections and without limitations on the maximum number of connections.

Most modern operating systems support TFO. Linux (as one of the most progressive) has supported it since 2011, with kernels starting from 3.7 (for the server side). Windows has supported it since some builds of Windows 10. Other systems, such as FreeBSD and MacOS, are also in the game.

For Linux systems, the server checks the variable `/proc/sys/net/ipv4/tcp_fastopen` and behaves accordingly. Bit 0 manages the client side, while bit 1 rules the listeners. By default, the system has this parameter set to 1, i.e., clients are enabled and listeners are disabled.

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

The `persistent_connections_limit` option defines the maximum number of simultaneous persistent connections to remote persistent agents. This is an instance-wide setting and must be defined in the searchd configuration section. Each time a connection to an agent defined under `agent_persistent` is made, we attempt to reuse an existing connection (if one exists) or create a new connection and save it for future use. However, in some cases it may be necessary to limit the number of persistent connections. This directive defines the limit and affects the number of connections to each agent's host across all distributed tables.

It is recommended to set this value equal to or less than the [max_connections](../../Server_settings/Searchd.md#max_connections) option in the agent's configuration.

## Distributed snippets creation

A special case of a distributed table is a single local and multiple remotes, which is used exclusively for  [distributed snippets creation](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-snippets-creation), when snippets are sourced from files. In this case, the local table may act as a "template" table, providing settings for tokenization when building snippets.

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

The `snippets_file_prefix` is an optional prefix that can be added to the local file names when generating snippets. The default value is the current working folder.

To learn more about distributed snippets creation, see  [CALL SNIPPETS](../../Searching/Highlighting.md).

## Distributed percolate tables (DPQ tables)

You can create a distributed table from multiple [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) tables. The syntax for constructing this type of table is the same as for other distributed tables, and can include multiple`local` tables as well as `agents`.

For DPQ, the operations of listing stored queries and searching through them (using [CALL PQ](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)) are transparent and work as if all the tables were one single local table. However, data manipulation statements such as `insert`, `replace`, `truncate` are not available.

If you include a non-percolate table in the list of agents, the behavior will be undefined. If the incorrect agent has the same schema as the outer schema of the PQ table (id, query, tags, filters), it will not trigger an error when listing stored PQ rules, and may pollute the list of actual PQ rules stored in PQ tables with its own non-PQ strings. As a result, be cautious and aware of the confusion that this may cause. A`CALL PQ` to such an incorrect agent will trigger an error.

For more information on making queries to a distributed percolate table, see [making queries to a distribute percolate table](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
<!-- proofread -->

