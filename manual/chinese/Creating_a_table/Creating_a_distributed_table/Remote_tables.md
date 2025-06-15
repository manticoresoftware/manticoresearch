# 远程表

Manticore Search 中的远程表通过分布式表定义中的 [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 前缀表示。分布式表可以包含本地表和远程表的组合。如果没有提供本地表，分布式表将纯粹是远程的，仅作为代理。例如，你可能有一个监听多个端口并提供不同协议的 Manticore 实例，然后将查询重定向到仅通过 Manticore 内部二进制协议接受连接的后端服务器，使用持久连接以减少建立连接的开销。
即使一个纯远程分布式表本身不服务本地表，它仍然消耗机器资源，因为它仍需执行最终计算，如合并结果和计算最终聚合值。

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

`agent` 指令声明了每次搜索封闭的分布式表时要查询的远程代理。这些代理本质上是网络表的指针。指定的值包含地址，也可以包括多个备选（代理镜像），可以是仅地址，也可以是地址和表列表。

地址规格必须是以下之一：

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

`hostname` 是远程主机名，`port` 是远程 TCP 端口号，`table-list` 是以逗号分隔的表名列表，方括号 [] 表示可选子句。

如果省略了表名，则假定表名与定义该行的表相同。换句话说，在定义 'mycoolindex' 分布式表的代理时，可以简单地指向地址，系统将假定你在代理端点查询 mycoolindex 表。

如果省略端口号，则默认使用 **9312**。如果定义了但无效（例如 70000），该代理将被跳过。

你可以将每个代理指向一个或多个位于一个或多个网络服务器上的远程表，且没有限制。这支持多种使用模式：
* 对多个代理服务器进行分片并创建任意集群拓扑
* 对多个代理服务器进行分片，镜像部署以实现高可用和负载均衡
* 在本地主机内分片以利用多核（不过，简单起见通常直接使用多个本地表）

所有代理并行搜索。索引列表按字面传递给远程代理。该列表在代理内的具体搜索方式（如顺序或并行）完全取决于代理配置（参见 [threads](../../Server_settings/Searchd.md#threads) 设置）。主节点无法远程控制此行为。

需要注意的是，`LIMIT` 选项在代理查询中被忽略。这是因为每个代理可能包含不同的表，因此由客户端负责对最终结果集应用限制。这就是为什么查询物理表和查询分布式表在查询日志中的表现不同。查询不能简单复制原始查询，否则不会产生正确结果。

例如，如果客户端执行查询 SELECT ... LIMIT 10, 10，且有两个代理，其中第二个代理只有 10 个文档，则广播原始 `LIMIT 10, 10` 查询时，第二个代理将返回 0 个文档。但 `LIMIT 10,10` 应该返回结果集合中的第 10 到 20 条文档。为解决此问题，必须向代理发送更大的限制值查询，比如默认的 max_matches 值 1000。

例如，如果有一个分布式表 dist 引用远程表 user，客户端查询 `SELECT * FROM dist LIMIT 10,10` 将被转换为 `SELECT * FROM user LIMIT 0,1000` 并发送给远程表 user。一旦分布式表收到结果，它将应用 LIMIT 10,10 并返回请求的 10 条文档。

```sql
SELECT * FROM dist LIMIT 10,10;
```

查询将转换为：

```sql
SELECT * FROM user LIMIT 0,1000
```

此外，该值可以为每个代理指定选项，例如：
* [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - `random`，`roundrobin`，`nodeads`，`noerrors`（覆盖该代理的全局 `ha_strategy` 设置）
* `conn` - `pconn`，持久连接（相当于在表级设置 `agent_persistent`）
* `blackhole` `0`，`1`（与代理的 [agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) 设置相同）
* `retry_count` 整数值（对应于 [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) ，但提供的值不会乘以镜像数）

```ini
agent = address1:table-list[[ha_strategy=value, conn=value, blackhole=value]]
```

示例：

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

为了获得最佳性能，建议将位于同一服务器上的远程表放在同一条记录中。例如，不要使用：
```ini
agent = remote:9312:idx1
agent = remote:9312:idx2
```
应优先使用：
```ini
agent = remote:9312:idx1,idx2
```

## agent_persistent

```ini
agent_persistent = remotebox:9312:index2
```

`agent_persistent` 选项允许与代理保持持久连接，即查询执行后连接不会关闭。该指令语法与 `agent` 指令相同。但不是为每个查询打开新连接再关闭，主节点将保持连接打开，并重复使用它进行后续查询。每个代理主机的最大持久连接数由 searchd 部分的 [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) 选项定义。

需要注意，必须将 [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) 设置为大于 0 的值，才能使用持久代理连接。如果未定义，则默认为 0，`agent_persistent` 指令将与 `agent` 指令表现相同。

使用持久的主-代理连接可以减少 TCP 端口压力并节省连接握手的时间，从而提高效率。

## agent_blackhole

```ini
agent_blackhole = testbox:9312:testindex1,testindex2
````

`agent_blackhole` 指令允许你将查询转发给远程代理，而不等待或处理它们的响应。这对于调试或测试生产集群非常有用，因为你可以设置一个单独的调试/测试实例，并从生产主（聚合器）实例将请求转发给它，而不会干扰生产工作。主搜索守护进程将尝试连接到黑洞代理并正常发送查询，但不会等待或处理任何响应，所有黑洞代理上的网络错误都将被忽略。该值的格式与常规的 `agent` 指令相同。

## agent_connect_timeout

```ini
agent_connect_timeout = 300
````

`agent_connect_timeout` 指令定义连接到远程代理的超时时间。默认情况下，该值被假定为毫秒，但可以带有[其他后缀](../../Server_settings/Special_suffixes.md))。默认值是1000（1秒）。

连接到远程代理时，`searchd` 最多等待该时间以成功完成连接。如果达到超时但连接未建立，且启用了 `retries`，则将启动重试。

## agent_query_timeout

```ini
agent_query_timeout = 10000 # our query can be long, allow up to 10 sec
```

`agent_query_timeout` 设置 searchd 等待远程代理完成查询的时间。默认值是3000毫秒（3秒），但可以通过后缀指定不同的时间单位。

建立连接后，`searchd` 将最多等待 agent_query_timeout 时间以完成远程查询。请注意，这个超时与 `agent_connection_timeout` 是分开的，远程代理可能产生的总延迟是两者的和。如果达到 agent_query_timeout，查询**不会**被重试，而是会产生警告。

请注意，[reset_network_timeout_on_packet](../../Server_settings/Searchd.md#reset_network_timeout_on_packet) 也会影响行为。

## agent_retry_count

`agent_retry_count` 是一个整数，指定在报告致命查询错误之前，Manticore 会在分布式表中尝试连接和查询远程代理的次数。它的工作方式类似于配置文件中“searchd”部分定义的 `agent_retry_count`，但专门针对表。

## mirror_retry_count

`mirror_retry_count` 的作用与 `agent_retry_count` 相同。如果两个值都提供了，`mirror_retry_count` 将优先，并且会产生警告。

## 实例范围的选项

以下选项管理远程代理的整体行为，并在**配置文件的 searchd 部分**中指定。它们为整个 Manticore 实例设置默认值。

* `agent_connect_timeout` - `agent_connect_timeout` 参数的默认值。
* `agent_query_timeout` - `agent_query_timeout` 参数的默认值。也可以在分布式（网络）表中使用相同设置名称按查询覆盖。
* `agent_retry_count` 是一个整数，指定 Manticore 在报告致命查询错误之前，在分布式表中尝试连接和查询远程代理的次数。默认值是 0（即不重试）。此值也可以使用查询的 'OPTION retry_count=XXX' 子句按查询指定。如果提供了查询选项，它将优先于配置文件中的值。

注意，如果你在分布式表定义中使用了**代理镜像**，服务器将根据指定的[ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 在每次连接尝试之前选择不同的镜像。在这种情况下，[agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 会为集合中所有镜像累加。

例如，如果你有10个镜像并设置 `agent_retry_count=5`，服务器最多将尝试50次重试（假设每10个镜像平均尝试5次）。如果选项 `ha_strategy = roundrobin`，则每个镜像正好尝试5次。

同时，`agent` 定义中提供的 [retry_count](../../Searching/Options.md#retry_count) 选项值作为绝对限制。换句话说，`agent` 定义中的 `[retry_count=2]` 选项意味着最多尝试2次，无论线路中有1个还是10个镜像。

### agent_retry_delay

`agent_retry_delay` 是一个整数，决定 Manticore Search 在失败时重试查询远程代理前等待的时间，单位为毫秒。此值可以全局在 searchd 配置中指定，也可以在查询中使用 `OPTION retry_delay=XXX` 子句按查询指定。如同时提供两个选项，按查询选项优先。默认值是500毫秒（0.5秒）。此选项仅在 `agent_retry_count` 或按查询的 `OPTION retry_count` 非零时有效。

### client_timeout

`client_timeout` 选项设置使用持久连接时请求之间的最大等待时间。该值以秒或带时间后缀的格式表示。默认值是5分钟。

示例：

```ini
client_timeout = 1h
```

### hostname_lookup

`hostname_lookup` 选项定义了更新主机名的策略。默认情况下，代理主机名的 IP 地址在服务器启动时缓存，以避免过度访问 DNS。但是，在某些情况下，IP 可能会动态变化（例如云托管），此时可能希望不缓存 IP。将此选项设置为 `request` 会禁用缓存，对每个查询都请求 DNS。IP 地址也可以通过 `FLUSH HOSTNAMES` 命令手动更新。

### listen_tfo

`listen_tfo` 选项允许所有监听器使用 TCP_FASTOPEN 标志。默认情况下，该选项由系统管理，但可以通过将其设置为 '0' 明确关闭。

有关 TCP Fast Open 扩展的更多信息，请参见 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时省去一次 TCP 往返。

在实践中，使用 TFO 可以优化客户端-代理的网络效率，类似于使用 `agent_persistent` 时的效果，但无需保持活动连接，也没有最大连接数的限制。

大多数现代操作系统支持 TFO。Linux （作为最先进的系统之一）自 2011 年起支持它，内核版本从 3.7 开始（针对服务器端）。Windows 从某些 Windows 10 版本开始支持。其他系统如 FreeBSD 和 MacOS 也支持。

对于 Linux 系统，服务器会检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并据此调整行为。第 0 位管理客户端，第 1 位管理监听器。默认情况下，系统将此参数设置为 1，即启用客户端，禁用监听器。

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

`persistent_connections_limit` 选项定义了到远程持久代理的最大同时持久连接数。这是一个实例范围的设置，必须在 searchd 配置部分定义。每次连接到 `agent_persistent` 下定义的代理时，我们都会尝试重用已存在的连接（如果有），或者创建新连接并保存以供将来使用。但是，在某些情况下可能需要限制持久连接的数量。此指令定义了该限制，并影响所有分布式表中每个代理主机的连接数。

建议将该值设置为小于或等于代理配置中的 [max_connections](../../Server_settings/Searchd.md#max_connections) 选项。

## 分布式摘要创建

分布式表的特殊情况是单个本地表和多个远程表，这种结构专门用于 [分布式摘要创建](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-snippets-creation)，当摘要数据来源于文件时。在此情况下，本地表可作为“模板”表，提供构建摘要时的分词设置。

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

`snippets_file_prefix` 是一个可选前缀，可以在生成摘要时添加到本地文件名中。默认值为当前工作文件夹。

了解更多关于分布式摘要创建的信息，请参见 [CALL SNIPPETS](../../Searching/Highlighting.md)。

## 分布式预编表（DPQ 表）

您可以从多个 [预编](../../Creating_a_table/Local_tables/Percolate_table.md) 表创建一个分布式表。此类表的构造语法与其他分布式表相同，可以包含多个 `local` 表以及 `agents`。

对于 DPQ，列出已存储查询和通过它们进行搜索（使用 [CALL PQ](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)）的操作是透明的，表现得就像所有表合并为一个单一的本地表一样。但数据操作语句如 `insert`、`replace`、`truncate` 不可用。

如果在代理列表中包括非预编表，则行为不确定。如果错误代理的模式与 PQ 表的外层模式相同（id，query，tags，filters），则在列出已存储 PQ 规则时不会触发错误，可能会用其自身非 PQ 字符串污染 PQ 表中实际 PQ 规则的列表。因此请谨慎并了解这种情况可能导致的混乱。对该错误代理执行 `CALL PQ` 会触发错误。

有关如何对分布式预编表进行查询的更多信息，请参见 [对分布式预编表进行查询](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
<!-- proofread -->

