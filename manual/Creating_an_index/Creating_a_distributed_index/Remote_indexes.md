# Remote indexes

Remote indexes are represented by [agent](Creating_an_index/Creating_a_distributed_index/Creating_a_distributed_index.md) in the definition of a distributed index.
Any number of locals and agents may be combined in a distributed index. If no locals provided, it will be purely remote index, which serves just as a proxy. For example, you may have a frontend Manticore instance which listens on a number of ports and serves different protocols, and then just redirects queries to backends that accept connections only via Manticore's internal binary protocol with, perhaps, persistent connections to avoid a connection establishing overhead.
Despite the fact, that such distributed index doesn't serve local indexes itself, it still consumes machine resources, since it still needs to make final calculations such as merging results and calculating final aggregated values. 

## agent

```ini
agent = address1 [ | address2 [...] ][:index-list]
agent = address1[:index-list [ | address2[:index-list [...] ] ] ]
```

`agent` directive declares remote agents that are searched every time when the enclosing distributed index is searched. The agents are, essentially, pointers to networked indexes. The value specifies address, and also can additionally specify multiple alternatives (agent mirrors) for either the address only, or the address and index list:

In both cases the address specification must be one of the following:

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

`hostname` is the remote host name, `port` is the remote TCP port number, `index-list` is a comma-separated list of index names, and square brackets [] designate an optional clause.

When index name is omitted, it is assumed the same index as the one where this line is defined. I.e. when defining agents for distributed index 'mycoolindex' you can just point the address, and it is assumed to query 'mycoolindex' index on agent's endpoints.

When port number is omitted, it is assumed that it is **9312**. However when it's defined, but invalid (say, port 70000), it will skip such agent.

In other words, you can point every single agent to one or more remote indexes, residing on one or more networked servers. There are absolutely no restrictions on the pointers. To point out a couple important things, the host can be localhost, and the remote index can be a distributed index in turn, all that is legal. That enables a bunch of very different usage modes:
* sharding over multiple agent servers, and creating an arbitrary cluster topology
* sharding over multiple agent servers, mirrored for HA/LB (High Availability and Load Balancing) purposes
* sharding within localhost, to utilize multiple cores (however, it is simpler just to use multiple local indexes)

All agents are searched in parallel. An index list is passed verbatim to the remote agent. How exactly that list is searched within the agent (ie. sequentially or in parallel too) depends solely on the agent configuration (ie. [threads](Server_settings/Searchd#threads) setting). Master has no remote control over that.

The value can additionally enumerate per agent options such as:
* [ha_strategy](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - random, roundrobin, nodeads, noerrors (replaces index-wide `ha_strategy` for particular agent)
* `conn` - pconn, persistent (same as `agent_persistent` on index-wide declaration)
* `blackhole` 0,1 (same as [agent_blackhole](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#agent_blackhole) agent declaration)
* `retry_count` - integer (same as [agent_retry_count](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#agent_retry_count) , but the provided value will not be multiplied to the number of mirrors)

```ini
agent = address1:index-list[[ha_strategy=value] | [conn=value] | [blackhole=value]]
```

Example:

```ini
# config on box2
# sharding an index over 3 servers
agent = box2:9312:shard2
agent = box3:9312:shard3

# config on box2
# sharding an index over 3 servers
agent = box1:9312:shard2
agent = box3:9312:shard3

# config on box3
# sharding an index over 3 servers
agent = box1:9312:shard2
agent = box2:9312:shard3

# per agent options
agent = box1:9312:shard1[ha_strategy=nodeads]
agent = box2:9312:shard2[conn=pconn]
agent = test:9312:any[blackhole=1]
agent = test:9312|box2:9312|box3:9312:any2[retry_count=2]
```

## agent_persistent

```ini
agent_persistent = remotebox:9312:index2
```

`agent_persistent` asks to persistently connect to agent (i.e. don't drop connection after query). This directive syntax matches that of the `agent` directive. The only difference is that the master will **not** open a new connection to the agent for every query and then close it. Rather, it will keep a connection open and attempt to reuse for the subsequent queries. The maximal number of such persistent connections per one agent host is limited by [persistent_connections_limit](Server_settings/Searchd.md#persistent_connections_limit) option of searchd section.

Note, that you **have** to set the last one in something greater than 0 if you want to use persistent agent connections. Otherwise - when [persistent_connections_limit](Server_settings/Searchd.md#persistent_connections_limit) is not defined, it assumes the zero num of persistent connections, and `agent_persistent` acts exactly as simple `agent`.

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

`agent_connect_timeout` defines remote agent connection timeout. By default the value is assumed to be in milliseconds, but can have [another suffix](Server_settings/Special_suffixes.md)). Optional, default is 1000 (ie. 1 second).

When connecting to remote agents, `searchd` will wait at most this much time for connect() call to complete successfully. If the timeout is reached but connect() does not complete, and `retries` are enabled, retry will be initiated.

## agent_query_timeout

```ini
agent_query_timeout = 10000 # our query can be long, allow up to 10 sec
```

`agent_query_timeout` defines remote agent query timeout. By default the value is assumed to be in milliseconds, but can be `suffixed`). Optional, default is 3000 (ie. 3 seconds).

After connection, `searchd` will wait at most this much time for remote queries to complete. This timeout is fully separate from connection timeout; so the maximum possible delay caused by a remote agent equals to the sum of `agent_connection_timeout` and `agent_query_timeout`. Queries will **not** be retried if this timeout is reached; a warning will be produced instead.

## agent_retry_count

Integer `agent_retry_count` specifies how many times manticore will try to connect and query remote agents in distributed index before reporting fatal query error. It works the same as `agent_retry_count` in searchd section, but define the value for concrete index. 

## mirror_retry_count

`mirror_retry_count` is the same as `agent_retry_count`. If both values provided, `mirror_retry_count` will be taken, and the warning about it will be fired.

## Instance-wide options

These options manage overall behaviour regarding remote agents. They are to be specified in **searchd section of the configuration file** and define defaults for the whole Manticore instance.

* `agent_connect_timeout` - instance-wide defaults for `agent_connect_timeout` parameter. 
* `agent_query_timeout` - instance-wide defaults for `agent_query_timeout` parameter. The last defined in distributed (network) indexes, or also may be overridden per-query using a setting of the same name.
* `agent_retry_count` integer, specifies how many times manticore will try to connect and query remote agents in distributed index before reporting fatal query error. Default is 0 (i.e. no retries). This value may be also specified on per-query basis using 'OPTION retry_count=XXX' clause. If per-query option exists, it will override the one specified in config.

  Note, that if you use **agent mirrors** in definition of your distributed index, then before every attempt of connect server will select different mirror, according to specified [ha_strategy](Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) specified. In this case [agent_retry_count](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#agent_retry_count) will be aggregated for all mirrors in a set.

  For example, if you have 10 mirrors, and set `agent_retry_count=5`, then server will retry up to 50 times, assuming average 5 tries per every of 10 mirrors (in case of option `ha_strategy = roundrobin` it will be actually exactly 5 times per mirror).

  At the same time value provided as [retry_count](Searching/Options.md#retry_count) option of `agent` definition serves as absolute limit. Other words, `[retry_count=2]` option in agent definition means always at most 2 tries, no mean if you have 1 or 10 mirrors in a line.

### agent_retry_delay

`agent_retry_delay` integer, specifies the delay manticore rest before retrying to query a remote agent, if it fails. Value is in in milliseconds (or, may include time suffix). The value has sense only if non-zero `agent_retry_count` or non-zero per-query `OPTION retry_count` specified. Default is 500 (i.e., half a second). This value may be also specified on per-query basis using `OPTION retry_delay=XXX` clause. If per-query option exists, it will override the one specified in config.

### client_timeout

`client_timeout` defines maximum time to wait between requests when using persistent connections. It is in seconds, or may be written with time suffix. By default is 5 minutes.

Example:

```ini
client_timeout = 1h
```

### hostname_lookup

`hostname_lookup` defines hostnames renew strategy. By default, IP addresses of agent host names are cached at server start to avoid extra flood to DNS. In some cases the IP can change dynamically (e.g. cloud hosting) and it might be desired to don't cache the IPs. Setting this option to 'request' disabled the caching and queries the DNS at each query. The IP addresses can also be manually renewed with `FLUSH HOSTNAMES` command.

### listen_tfo

`listen_tfo` allows TCP_FASTOPEN flag for all listeners. By default it is managed by system, but may be explicitly switched off by setting to '0'.

For general knowledge about TCP Fast Open extension you can visit Wikipedia. Shortly speaking, it allows to eliminate one TCP round-trip when establishing connection.

In practice using TFO in many situation may optimize client-agent network efficiency as if `agent_persistent` are in play, but without holding active connections, and also without limitation for the maximum num of connections.

On modern OS TFO support usually switched 'on' on the system level, but this is just 'capability', not the rule. Linux (as most progressive) supports it since 2011, on kernels starting from 3.7 (for server side). Windows supports it from some build of Windows 10. Another (FreeBSD, MacOS) also in game.

For Linux system server checks variable `/proc/sys/net/ipv4/tcp_fastopen` and behaves according to it. Bit 0 manages client side, bit 1 rules listeners. By default system has this param set to 1, i.e. clients enabled, listeners disabled.

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

`persistent_connections_limit` defines maximum # of simultaneous persistent connections to remote persistent agents. This is instance-wide option and has to be defined in searchd config section. Each time connecting agent defined under `agent_persistent` we try to reuse existing connection (if any), or connect and save the connection for the future. However we can't hold unlimited # of such persistent connections, since each one holds a worker on agent's side (and finally we'll receive the 'maxed out' error, when all of them are busy). This very directive limits the number. It affects the num of connections to each agent's host, across all distributed indexes.

It is reasonable to set the value equal or less than [max_connections](Server_settings/Searchd#max_connections) option of the agent's config.

## Distributed snippets creation

One special case of a distributed index is a single local + multiple remotes. It is solely used for [distributed snippets creation](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#Distributed-snippets-creation), when snippets are sourced from files. In this case the local may be a 'template' index, since it is used just to provide settings for tokenization when building snippets.

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

`snippets_file_prefix` is a prefix to prepend to the local file names when generating snippets. Optional, default is current working folder.

Head about [CALL SNIPPETS](Searching/Highlighting.md) to learn more about distributed snippets creation.

## Distributed percolate indexes (DPQ indexes)

You can construct a distributed index from several [percolate](Creating_an_index/Local_indexes/Percolate_index.md) indexes. The syntax is absolutely the same as for other distributed indexes. It can include several `local` indexes as well as several `agents`.

For DPQ the operations of listing stored queries and searching through them ([CALL PQ](Searching/Percolate_query.md#Performing-a-percolate-query)) are transparent and works as if all the indexes were one solid local index. However data manipulation statements such as `insert`, `replace`, `truncate` are not available.

If you mention a non-percolate index among the agents, the behaviour will be undefined. Most likely in case if the erroneous agent has the same schema as the outer schema of the pq index (id, query, tags, filters) - it will not trigger an error when listing stored PQ rules hence may pollute the list of actual PQ rules stored in PQ indexes with it's own non-pq strings, so be aware of the confusion! `CALL PQ` to such wrong agent will definitely trigger an error.

Read more about [making queries to a distribute percolate index](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#Distributed-percolate-indexes-%28DPQ-indexes%29).
