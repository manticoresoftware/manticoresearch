# 远程表

在 Manticore Search 中，远程表通过分布式表定义中的 [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 前缀表示。分布式表可以包含本地表和远程表的组合。如果没有提供本地表，则分布式表将完全是远程的，仅作为代理。例如，您可能有一个 Manticore 实例监听多个端口并提供不同的协议，然后将查询重定向到仅接受通过 Manticore 内部二进制协议的后端服务器，并使用持久连接来减少建立连接的开销。

尽管纯远程分布式表本身不提供本地表，但它仍然消耗机器资源，因为它仍需要执行最终计算，例如合并结果和计算最终聚合值。

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

`agent` 指令声明每次搜索包含的分布式表时所搜索的远程代理。这些代理本质上是指向网络表的指针。指定的值包括地址，也可以包含多个替代选项（代理镜像），这些选项可以仅用于地址或地址和表列表。

地址规范必须是以下之一：

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

`hostname` 是远程主机名，`port` 是远程 TCP 端口号，`table-list` 是以逗号分隔的表名列表，方括号 [] 表示可选条款。

如果省略表名，则假定与定义该行的表相同。换句话说，在为名为 'mycoolindex' 的分布式表定义代理时，您只需指向地址，它将假定您正在查询代理端点上的 mycoolindex 表。

如果省略端口号，则假定为 **9312**。如果定义但无效（例如 70000），则该代理将被跳过。

您可以将每个代理指向一个或多个位于一个或多个网络服务器上的远程表，没有限制。这使得几种不同的使用模式成为可能：

* 在多个代理服务器上进行分片并创建任意集群拓扑
* 在多个代理服务器上进行分片，以实现高可用性和负载均衡目的的镜像
* 在本地主机内进行分片，以利用多个核心（但是，使用多个本地表更简单）

所有代理都是并行搜索的。索引列表原封不动地传递给远程代理。该列表在代理内的具体搜索方式（即顺序或并行）完全取决于代理的配置（参见 [threads](../../Server_settings/Searchd.md#threads) 设置）。主节点对其没有远程控制。

需要注意的是，`LIMIT` 选项在代理查询中被忽略。这是因为每个代理可能包含不同的表，因此将限制应用于最终结果集的责任在于客户端。这就是物理表的查询与分布式表的查询在查询日志中显示的不同原因。查询不能简单地复制原始查询，因为这不会产生正确的结果。

例如，如果客户端发出查询 `SELECT ... LIMIT 10, 10`，并且有两个代理，第二个代理只有 10 个文档，则广播原始的 `LIMIT 10, 10` 查询将导致从第二个代理接收到 0 个文档。然而，`LIMIT 10,10` 应返回结果集中的第 10 到第 20 个文档。为了解决此问题，必须以更广泛的限制发送查询，例如默认的最大匹配值 1000。

例如，如果有一个分布式表 dist 引用远程表 user，则客户端查询 `SELECT * FROM dist LIMIT 10,10` 将被转换为 `SELECT * FROM user LIMIT 0,1000` 并发送到远程表 user。一旦分布式表接收到结果，它将应用 `LIMIT 10,10` 并返回请求的 10 个文档。

```sql
SELECT * FROM dist LIMIT 10,10;
```

该查询将会转为：

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

为了获得最佳性能，建议将位于同一服务器上的远程表放在同一条记录中。例如，不要：
```ini
agent = remote:9312:idx1
agent = remote:9312:idx2
```
你应该这样：
```ini
agent = remote:9312:idx1,idx2
```

## agent_persistent

```ini
agent_persistent = remotebox:9312:index2
```

`agent_blackhole` 指令允许您将查询转发到远程代理，而无需等待或处理其响应。这对于调试或测试生产集群非常有用，因为您可以设置一个单独的调试/测试实例，并从生产主节点（聚合器）转发请求到它，而不干扰生产工作。主搜索节点将尝试连接到黑洞代理并像正常一样发送查询，但不会等待或处理任何响应，并且所有黑洞代理上的网络错误将被忽略。值的格式与常规 `agent` 指令相同。

## agent_blackhole

```ini
agent_blackhole = testbox:9312:testindex1,testindex2
````

`agent_blackhole` 指令允许您将查询转发到远程代理，而无需等待或处理其响应。这对于调试或测试生产集群非常有用，因为您可以设置一个单独的调试/测试实例，并从生产主节点（聚合器）转发请求到它，而不干扰生产工作。主搜索节点将尝试连接到黑洞代理并像正常一样发送查询，但不会等待或处理任何响应，并且所有黑洞代理上的网络错误将被忽略。值的格式与常规 `agent` 指令相同。

## agent_connect_timeout

```ini
agent_connect_timeout = 300
````

`agent_connect_timeout` 指令定义连接到远程代理的超时。默认值假定为毫秒，但可以有 [其他后缀](../../Server_settings/Special_suffixes.md)。默认值为 1000（1 秒）。

连接到远程代理时，`searchd` 最多将等待此时间以成功完成连接。如果超时到达但未建立连接，并且启用了重试，将会启动重试。

## agent_query_timeout

```ini
agent_query_timeout = 10000 # our query can be long, allow up to 10 sec
```

`agent_query_timeout` 设置 `searchd` 等待远程代理完成查询的时间。默认值为 3000 毫秒（3 秒），但可以用 `suffixed` 指示不同的时间单位。

在建立连接后，`searchd` 将等待最多 `agent_query_timeout` 的时间，以完成远程查询。如果到达了代理查询超时，查询将 **不** 被重试，而是会产生一个警告。

注意，此超时与 `agent_connection_timeout` 是分开的，远程代理可能导致的总延迟将是这两个值的总和。如果到达了代理查询超时，查询将 **不** 被重试，而是会产生一个警告。

注意，行为也受 [reset_network_timeout_on_packet](../../Server_settings/Searchd.md#reset_network_timeout_on_packet) 的影响。

## agent_retry_count

`agent_retry_count` 是一个整数，指定在报告致命查询错误之前，Manticore 将尝试连接和查询远程代理的次数。它的工作原理类似于配置文件 "searchd" 部分中定义的 `agent_retry_count`，但专门适用于该表。

## mirror_retry_count

`mirror_retry_count` 的作用与 `agent_retry_count` 相同。如果提供了两个值，则 `mirror_retry_count` 将优先，并将发出警告。

## 实例级选项

以下选项管理远程代理的整体行为，并在**配置文件的 searchd 部分**中指定。它们为整个 Manticore 实例设置默认值。

- `agent_connect_timeout` - `agent_connect_timeout` 参数的默认值。
- `agent_query_timeout` - `agent_query_timeout` 参数的默认值。也可以通过在分布式（网络）表中使用相同设置名称来覆盖。
- `agent_retry_count` 是一个整数，指定在报告致命查询错误之前，Manticore 将尝试连接和查询远程代理的次数。默认值为 0（即不重试）。此值也可以通过使用 `OPTION retry_count=XXX` 子句在每个查询中指定。如果提供了每个查询选项，则将优先于配置中指定的值。

注意，如果您在分布式表的定义中使用 **代理镜像**，服务器将在每次连接尝试之前根据指定的 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 选择不同的镜像。在这种情况下，`agent_retry_count` 将对所有镜像的集合进行汇总。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，服务器将在尝试 50 次（假设平均每个镜像尝试 5 次）。如果选项 `ha_strategy = roundrobin`，则每个镜像将实际尝试 5 次。

同时，在 `agent` 定义中提供的 `[retry_count]` 选项作为绝对限制。换句话说，`agent` 定义中的 `[retry_count=2]` 选项意味着无论镜像数量是 1 还是 10，最多只会尝试 2 次。

### agent_retry_delay

`agent_retry_delay` 是一个整数值，确定在失败的情况下，Manticore Search 在重试查询远程代理之前等待的时间，单位为毫秒。可以在 searchd 配置中全局指定此值，或者通过 `OPTION retry_delay=XXX` 子句在每个查询中指定。如果两者都提供，则每个查询选项将优先于全局选项。默认值为 500 毫秒（0.5 秒）。此选项仅在 `agent_retry_count` 或每个查询的 `OPTION retry_count` 不为零时相关。

### client_timeout

`client_timeout` 选项设置在使用持久连接时请求之间的最大等待时间。此值以秒为单位表示，或带有时间后缀。默认值为 5 分钟。

示例：

```ini
client_timeout = 1h
```

### hostname_lookup

`hostname_lookup` 选项定义了更新主机名的策略。默认情况下，代理主机名的 IP 地址在服务器启动时缓存，以避免过多访问 DNS。但是，在某些情况下，IP 可能动态更改（例如，云托管），可能希望不缓存 IP。将此选项设置为 `request` 会禁用缓存，并在每个查询时查询 DNS。也可以使用 `FLUSH HOSTNAMES` 命令手动更新 IP 地址。

### listen_tfo

`listen_tfo` 选项允许对所有监听器使用 TCP_FASTOPEN 标志。默认情况下，由系统管理，但可以通过将其设置为 '0' 显式关闭。

有关 TCP Fast Open 扩展的更多信息，请参见 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它可以在建立连接时消除一个 TCP 往返。

在实践中，使用 TFO 可以优化客户端代理的网络效率，类似于使用 `agent_persistent` 时的效果，但无需保持活动连接，并且没有最大连接数的限制。

大多数现代操作系统都支持 TFO。Linux（作为最进步的系统）自 2011 年起就支持它，内核版本从 3.7（用于服务器端）开始。Windows 自 Windows 10 的某些版本开始支持。FreeBSD 和 MacOS 等其他系统也在使用这个功能。

对于 Linux 系统，服务器检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并相应地运行。位 0 管理客户端，位 1 管理监听器。默认情况下，系统将此参数设置为 1，即启用客户端，禁用监听器。

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

`persistent_connections_limit` 选项定义到远程持久代理的最大同时持久连接数。这是一个实例级设置，必须在 searchd 配置部分定义。每次连接到在 `agent_persistent` 下定义的代理时，我们尝试重用现有连接（如果存在）或创建新连接并保存以备将来使用。但是，在某些情况下，可能需要限制持久连接的数量。此指令定义了限制，并影响到所有分布式表中每个代理主机的连接数。

建议将此值设置为小于或等于代理配置中的 [max_connections](../../Server_settings/Searchd.md#max_connections) 选项。

## 分布式片段创建

分布式表的特殊情况是单个本地和多个远程，这仅用于 [分布式片段创建](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#分布式片段创建)，当片段从文件中获取时。在这种情况下，本地表可以充当“模板”表，提供生成片段时的分词设置。

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

`snippets_file_prefix` 是一个可选的前缀，可在生成片段时添加到本地文件名中。默认值是当前的工作文件夹。

要了解更多关于分布式片段创建的信息，请参阅 [CALL SNIPPETS](../../Searching/Highlighting.md)。

## 分布式预过滤表 (DPQ)

您可以从多个 [预过滤](../../Creating_a_table/Local_tables/Percolate_table.md) 表创建一个分布式表。构建此类表的语法与其他分布式表相同，可以包含多个 `local` 表以及 `agents`。

对于 DPQ，列出存储的查询并通过它们进行搜索的操作（使用 [CALL PQ](../../Searching/Percolate_query.md#使用-CALL-PQ-执行渗透查询)）是透明的，行为就像所有表都是单个本地表一样。然而，`insert`、`replace`、`truncate` 等数据操作语句不可用。

如果在代理列表中包含非预过滤表，行为将是未定义的。如果错误的代理与 PQ 表的外部架构（id、query、tags、filters）具有相同的架构，则在列出存储的 PQ 规则时不会触发错误，但可能会通过其非 PQ 字符串污染实际存储在 PQ 表中的 PQ 规则列表。因此，必须谨慎，避免产生混淆。对此类错误代理的 `CALL PQ` 将触发错误。

有关对分布式预过滤表进行查询的更多信息，请参阅 [对分布式预过滤表进行查询](../../Searching/Percolate_query.md#使用-CALL-PQ-执行渗透查询)。

<!-- proofread -->
