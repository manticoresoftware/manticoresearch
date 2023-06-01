# Remote tables

A remote table in Manticore Search is represented by the [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) prefix in the definition of a distributed table. A distributed table can include a combination of local and remote tables. If there are no local tables provided, the distributed table will be purely remote and serve as a proxy only. For example, you might have a Manticore instance that listens on multiple ports and serves different protocols, and then redirects queries to backend servers that only accept connections via Manticore's internal binary protocol, using persistent connections to reduce the overhead of establishing connections.
Even though a purely remote distributed table doesn't serve local tables itself, it still consumes machine resources, as it still needs to perform final calculations, such as merging results and calculating final aggregated values.

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

`agent` directive declares the remote agents that are searched each time the enclosing distributed table is searched. These agents are essentially pointers to networked tables. The value specified includes the address and can also include multiple alternatives (agent mirrors) for either the address only or the address and table list.

The address specification must be one of the following:

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

The `hostname` is the remote host name, `port` is the remote TCP port number, `table-list` is a comma-separated list of table names, and square brackets [] indicate an optional clause.

If the table name is omitted, it is assumed to be the same table as the one where this line is defined. In other words, when defining agents for the 'mycoolindex' distributed table, you can simply point to the address, and it will be assumed that you are querying the mycoolindex table on the agent's endpoints.

If the port number is omitted, it is assumed to be **9312**. If it is defined but invalid (e.g. 70000), the agent will be skipped.

You can point each agent to one or more remote tables residing on one or more networked servers with no restrictions. This enables several different usage modes:
* Sharding over multiple agent servers and creating an arbitrary cluster topology
* Sharding over multiple agent servers mirrored for high availability and load balancing purposes
* Sharding within localhost to utilize multiple cores (however, it is simpler just to use multiple local tables)

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
* [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - `random`, `roundrobin`, `nodeads`, `noerrors` (overrides the global `ha_strategy` setting for the particular agent)
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

```ini
agent_persistent = remotebox:9312:index2
```

The `agent_persistent` option allows you to persistently connect to an agent, meaning the connection will not be dropped after a query is executed. The syntax for this directive is the same as the `agent` directive. However, instead of opening a new connection to the agent for each query and then closing it, the master will keep a connection open and reuse it for subsequent queries. The maximum number of persistent connections per agent host is defined by the [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) option in the searchd section.

It's important to note that the [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) must be set to a value greater than 0 in order to use persistent agent connections. If it's not defined, it defaults to 0, and the `agent_persistent` directive will act the same as the `agent`directive.

Using persistent master-agent connections reduces TCP port pressure and saves time on connection handshakes, making it more efficient.

## agent_blackhole

```ini
agent_blackhole = testbox:9312:testindex1,testindex2
````

The `agent_blackhole` directive allows you to forward queries to remote agents without waiting for or processing their responses. This is useful for debugging or testing production clusters, as you can set up a separate debugging/testing instance and forward requests to it from the production master (aggregator) instance, without interfering with production work. The master searchd will attempt to connect to the blackhole agent and send queries as normal, but will not wait for or process any responses, and all network errors on the blackhole agents will be ignored. The format of the value is identical to that of the regular `agent` directive.

## agent_connect_timeout

```ini
agent_connect_timeout = 300
````

The `agent_connect_timeout` directive defines the timeout for connecting to remote agents. By default, the value is assumed to be in milliseconds, but can have [another suffix](../../Server_settings/Special_suffixes.md)). The default value is 1000 (1 second).

When connecting to remote agents, `searchd` will wait for this amount of time at most to complete the connection successfully. If the timeout is reached but the connection has not been established, and `retries` are enabled, a retry will be initiated.

## agent_query_timeout

```ini
agent_query_timeout = 10000 # our query can be long, allow up to 10 sec
```

The `agent_query_timeout` sets the amount of time that searchd will wait for a remote agent to complete a query. The default value is 3000 milliseconds (3 seconds), but can be `suffixed` to indicate a different unit of time.

After establishing a connection, `searchd` will wait for a maximum of agent_query_timeout for remote queries to complete. Note that this timeout is separate from the `agent_connection_timeout` and the total possible delay caused by a remote agent will be the sum of both values. If the agent_query_timeout is reached, the query will **not** be retried, instead, a warning will be produced.

## agent_retry_count

The `agent_retry_count` is an integer that specifies how many times Manticore will attempt to connect and query remote agents in a distributed table before reporting a fatal query error. It works similarly to `agent_retry_count` defined in the "searchd" section of the configuration file but applies specifically to the table.

## mirror_retry_count

`mirror_retry_count` serves the same purpose as `agent_retry_count`.  If both values are provided, `mirror_retry_count` will take precedence, and a warning will be raised.

## Instance-wide options

The following options manage the overall behavior of remote agents and are specified in **the searchd section of the configuration file**. They set default values for the entire Manticore instance.

* `agent_connect_timeout` - default value for the `agent_connect_timeout` parameter.
* `agent_query_timeout` - default value for the `agent_query_timeout` parameter. This can also be overridden on a per-query basis using the same setting name in a distributed (network) table.
* `agent_retry_count` is an integer that specifies the number of times Manticore will attempt to connect and query remote agents in a distributed table before reporting a fatal query error. The default value is 0 (i.e. no retries). This value can also be specified on a per-query basis using the 'OPTION retry_count=XXX' clause. If a per-query option is provided, it will take precedence over the value specified in the config.

Note, that if you use **agent mirrors** in the definition of your distributed table, the server will select a different mirror before each connection attempt according to the specified [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) specified. In this case the [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) will be aggregated for all mirrors in the set.

For example, if you have 10 mirrors and set `agent_retry_count=5`, he server will attempt up to 50 retries (assuming an average of 5 tries per every 10 mirrors). In case of the option `ha_strategy = roundrobin`, it will actually be exactly 5 tries per mirror.

At the same time, the value provided as the [retry_count](../../Searching/Options.md#retry_count) option in the `agent` definition serves as an absolute limit. In other words, the `[retry_count=2]` option in the agent definition means there will be a maximum of 2 tries, regardless of whether there is 1 or 10 mirrors in the line.

### agent_retry_delay

The `agent_retry_delay` is an integer value that determines the amount of time, in milliseconds, that Manticore Search will wait before retrying to query a remote agent in case of a failure. This value can be specified either globally in the searchd configuration or on a per-query basis using the `OPTION retry_delay=XXX` clause. If both options are provided, the per-query option will take precedence over the global one. The default value is 500 milliseconds (0.5 seconds). This option is only relevant if agent_retry_count or the per-query `OPTION retry_count` are non-zero.

### client_timeout

The `client_timeout` option sets the maximum waiting time between requests when using persistent connections. This value is expressed in seconds or with a time suffix. The default value is 5 minutes.

Example:

```ini
client_timeout = 1h
```

### hostname_lookup

The `hostname_lookup` option defines the strategy for renewing hostnames. By default, the IP addresses of agent host names are cached at server start to avoid excessive access to DNS. However, in some cases, the IP can change dynamically (e.g. cloud hosting) and it may be desirable to not cache the IPs. Setting this option to 'request' disables the caching and queries the DNS for each query. The IP addresses can also be manually renewed using the `FLUSH HOSTNAMES` command.

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
