# Remote tables

Remote tables are represented by the prefix [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) in the definition of a distributed table.
Any number of locals and agents may be combined in a distributed table. If no locals provided, it will be purely remote table, which serves just as a proxy. For example, you may have a frontend Manticore instance which listens on a number of ports and serves different protocols, and then just redirects queries to backends that accept connections only via Manticore's internal binary protocol with, perhaps, persistent connections to avoid a connection establishing overhead.
Despite the fact, that such distributed table doesn't serve local tables itself, it still consumes machine resources, since it still needs to make final calculations such as merging results and calculating final aggregated values.

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

`agent` directive declares remote agents that are searched every time when the enclosing distributed table is searched. The agents are, essentially, pointers to networked tables. The value specifies address, and also can additionally specify multiple alternatives (agent mirrors) for either the address only, or the address and table list:

In both cases the address specification must be one of the following:

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

`hostname` is the remote host name, `port` is the remote TCP port number, `table-list` is a comma-separated list of table names, and square brackets [] designate an optional clause.

When table name is omitted, it is assumed the same table as the one where this line is defined. I.e. when defining agents for distributed table 'mycoolindex' you can just point the address, and it is assumed to query 'mycoolindex' table on agent's endpoints.

When port number is omitted, it is assumed that it is **9312**. However when it's defined, but invalid (say, port 70000), it will skip such agent.

In other words, you can point every single agent to one or more remote tables, residing on one or more networked servers. There are absolutely no restrictions on the pointers. To point out a couple important things, the host can be localhost, and the remote table can be a distributed table in turn, all that is legal. That enables a bunch of very different usage modes:
* sharding over multiple agent servers, and creating an arbitrary cluster topology
* sharding over multiple agent servers, mirrored for HA/LB (High Availability and Load Balancing) purposes
* sharding within localhost, to utilize multiple cores (however, it is simpler just to use multiple local tables)

All agents are searched in parallel. Index list is passed verbatim to the remote agent. How exactly that list is searched within the agent (ie. sequentially or in parallel too) depends solely on the agent configuration (see [threads](../../Server_settings/Searchd.md#threads) setting). The master has no remote control over that.

Note, agent internally executes a query ignoring option `LIMIT`, since each agent can have different tables and it is a client responsibility to apply the limit to the final result set. That's the reason why the query to a physical table differs from the query to a distributed table when viewing them in the query logs. It can't be just a full copy of the original query in order to provide correct results in such a case:

* We make `SELECT ... LIMIT 10, 10`
* We have 2 agents
* Second agent has just 10 documents

If we just broadcast the original `LIMIT 10, 10` query it will receive 0 documents from the 2nd agent, but `LIMIT 10,10` should return documents 10-20 from the resulting set as you may not care about each agent in particular. That's why we need to send the query to the agents with broader limits and the upper bound in this case is `max_matches`.

For example, imagine we have table `dist` which refers to remote table `user`.

Then if client sends this query:

```sql
SELECT * FROM dist LIMIT 10,10;
```

the query will be converted to:

```sql
SELECT * FROM user LIMIT 0,1000
```

and sent to the remote table `user`. `1000` here is the default `max_matches`. Once the distributed table receives the result it will apply `LIMIT 10,10` to it and return the requested 10 documents.

The value can additionally enumerate per agent options such as:
* [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - `random`, `roundrobin`, `nodeads`, `noerrors` (replaces table-wide `ha_strategy` for particular agent)
* `conn` - `pconn`, persistent (same as `agent_persistent` on table-wide declaration)
* `blackhole` `0`,`1` (same as [agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) agent declaration)
* `retry_count` - integer (same as [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) , but the provided value will not be multiplied to the number of mirrors)

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

## agent_persistent

```ini
agent_persistent = remotebox:9312:index2
```

`agent_persistent` asks to persistently connect to agent (i.e. don't drop connection after query). This directive syntax matches that of the `agent` directive. The only difference is that the master will **not** open a new connection to the agent for every query and then close it. Rather, it will keep a connection open and attempt to reuse for the subsequent queries. The maximal number of such persistent connections per one agent host is limited by [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) option of searchd section.

Note, that you **have** to set the last one in something greater than 0 if you want to use persistent agent connections. Otherwise - when [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) is not defined, it assumes the zero num of persistent connections, and `agent_persistent` acts exactly as simple `agent`.

Persistent master-agent connections reduce TCP port pressure, and save on connection handshakes.


## agent_blackhole

```ini
agent_blackhole = testbox:9312:testindex1,testindex2
````

`agent_blackhole` lets you fire-and-forget queries to remote agents. That is useful for debugging (or just testing) production clusters: you can setup a separate debugging/testing searchd instance, and forward the requests to this instance from your production master (aggregator) instance without interfering with production work. Master searchd will attempt to connect and query blackhole agent normally, but it will neither wait nor process any responses. Also, all network errors on blackhole agents will be ignored. The value format is completely identical to regular `agent` directive.

## agent_connect_timeout

```ini
agent_connect_timeout = 300
````

`agent_connect_timeout` defines remote agent connection timeout. By default the value is assumed to be in milliseconds, but can have [another suffix](../../Server_settings/Special_suffixes.md)). Optional, default is 1000 (ie. 1 second).

When connecting to remote agents, `searchd` will wait at most this much time to complete successfully. If the timeout is reached but the connection has not been established and `retries` are enabled, retry will be initiated.

## agent_query_timeout

```ini
agent_query_timeout = 10000 # our query can be long, allow up to 10 sec
```

`agent_query_timeout` defines remote agent query timeout. By default the value is assumed to be in milliseconds, but can be `suffixed`). Optional, default is 3000 (ie. 3 seconds).

After connection, `searchd` will wait at most this much time for remote queries to complete. This timeout is fully separate from connection timeout; so the maximum possible delay caused by a remote agent equals to the sum of `agent_connection_timeout` and `agent_query_timeout`. Queries will **not** be retried if this timeout is reached; a warning will be produced instead.

## agent_retry_count

Integer `agent_retry_count` specifies how many times Manticore will try to connect and query remote agents in a distributed table before reporting a fatal query error. It works the same way as `agent_retry_count` in section "searchd" of the configuration file, but defines the value for a particular table.

## mirror_retry_count

`mirror_retry_count` is the same as `agent_retry_count`. If both values provided, `mirror_retry_count` will be taken, and the warning about it will be fired.

## Instance-wide options

These options manage overall behaviour regarding remote agents. They are to be specified in **searchd section of the configuration file** and define defaults for the whole Manticore instance.

* `agent_connect_timeout` - instance-wide defaults for `agent_connect_timeout` parameter.
* `agent_query_timeout` - instance-wide defaults for `agent_query_timeout` parameter. The last defined in distributed (network) tables, or also may be overridden per-query using a setting of the same name.
* `agent_retry_count` integer, specifies how many times manticore will try to connect and query remote agents in a distributed table before reporting fatal query error. Default is 0 (i.e. no retries). This value may be also specified on per-query basis using 'OPTION retry_count=XXX' clause. If per-query option exists, it will override the one specified in config.

Note, that if you use **agent mirrors** in definition of your distributed table, then before every attempt of connect server will select different mirror, according to specified [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) specified. In this case [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) will be aggregated for all mirrors in a set.

For example, if you have 10 mirrors, and set `agent_retry_count=5`, then server will retry up to 50 times, assuming average 5 tries per every of 10 mirrors (in case of option `ha_strategy = roundrobin` it will be actually exactly 5 times per mirror).

At the same time value provided as [retry_count](../../Searching/Options.md#retry_count) option of `agent` definition serves as absolute limit. Other words, `[retry_count=2]` option in agent definition means always at most 2 tries, no mean if you have 1 or 10 mirrors in a line.

### agent_retry_delay

`agent_retry_delay` integer, specifies the delay manticore takes before retrying to query a remote agent, if it fails. The value is in in milliseconds (or may include time suffix). The value makes sense only if non-zero `agent_retry_count` or non-zero per-query `OPTION retry_count` specified. Default is 500 (half a second). This value may be also specified on per-query basis using `OPTION retry_delay=XXX` clause. If per-query option exists, it will override the one specified in config.

### client_timeout

`client_timeout` defines maximum time to wait between requests when using persistent connections. It is in seconds, or may be written with time suffix. The default is 5 minutes.

Example:

```ini
client_timeout = 1h
```

### hostname_lookup

`hostname_lookup` defines hostnames renew strategy. By default, IP addresses of agent host names are cached at server start to avoid extra flood to DNS. In some cases the IP can change dynamically (e.g. cloud hosting) and it might be desired to don't cache the IPs. Setting this option to 'request' disabled the caching and queries the DNS at each query. The IP addresses can also be manually renewed with `FLUSH HOSTNAMES` command.

### listen_tfo

`listen_tfo` allows TCP_FASTOPEN flag for all listeners. By default it is managed by system, but may be explicitly switched off by setting to '0'.

For general knowledge about TCP Fast Open extension please consult with [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open). Shortly speaking, it allows to eliminate one TCP round-trip when establishing connection.

In practice using TFO in many situation may optimize client-agent network efficiency as if `agent_persistent` are in play, but without holding active connections, and also without limitation for the maximum num of connections.

On modern OS TFO support usually switched 'on' on the system level, but this is just 'capability', not the rule. Linux (as most progressive) supports it since 2011, on kernels starting from 3.7 (for server side). Windows supports it from some build of Windows 10. Another (FreeBSD, MacOS) also in game.

For Linux system server checks variable `/proc/sys/net/ipv4/tcp_fastopen` and behaves according to it. Bit 0 manages client side, bit 1 rules listeners. By default system has this param set to 1, i.e. clients enabled, listeners disabled.

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

`persistent_connections_limit` defines maximum # of simultaneous persistent connections to remote persistent agents. This is instance-wide option and has to be defined in searchd config section. Each time connecting an agent defined under `agent_persistent` we try to reuse existing connection (if any), or connect and save the connection for the future. However in some cases it makes sense to limit # of such persistent connections. This directive defines the number. It affects the number of connections to each agent's host across all distributed tables.

It is reasonable to set the value equal or less than [max_connections](../../Server_settings/Searchd.md#max_connections) option of the agent's config.

## Distributed snippets creation

One special case of a distributed table is a single local + multiple remotes. It is solely used for [distributed snippets creation](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-snippets-creation), when snippets are sourced from files. In this case the local may be a 'template' table, since it is used just to provide settings for tokenization when building snippets.

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

`snippets_file_prefix` is a prefix to prepend to the local file names when generating snippets. Optional, default is current working folder.

Head about [CALL SNIPPETS](../../Searching/Highlighting.md) to learn more about distributed snippets creation.

## Distributed percolate tables (DPQ tables)

You can construct a distributed table from several [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) tables. The syntax is absolutely the same as for other distributed tables. It can include several `local` tables as well as several `agents`.

For DPQ the operations of listing stored queries and searching through them ([CALL PQ](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)) are transparent and works as if all the tables were one solid local table. However data manipulation statements such as `insert`, `replace`, `truncate` are not available.

If you mention a non-percolate table among the agents, the behaviour will be undefined. Most likely in case if the erroneous agent has the same schema as the outer schema of the pq table (id, query, tags, filters) - it will not trigger an error when listing stored PQ rules hence may pollute the list of actual PQ rules stored in PQ tables with it's own non-pq strings, so be aware of the confusion! `CALL PQ` to such wrong agent will definitely trigger an error.

Read more about [making queries to a distribute percolate table](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).
