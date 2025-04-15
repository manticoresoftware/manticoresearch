# 远程表

在 Manticore Search 中，远程表由分布式表定义中的 [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 前缀表示。分布式表可以包含本地表和远程表的组合。如果没有提供本地表，则分布式表将完全是远程的，仅作为代理服务。例如，您可能有一个 Manticore 实例，它监听多个端口并提供不同的协议，然后将查询重定向到仅通过 Manticore 的内部二进制协议接受连接的后端服务器，使用持久连接以减少建立连接的开销。
尽管一个完全远程的分布式表并不提供本地表，但它仍然消耗机器资源，因为它仍然需要执行最终计算，例如合并结果和计算最终聚合值。

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

`agent` 指令声明每次搜索封闭分布式表时要搜索的远程代理。这些代理本质上是指向网络表的指针。指定的值包括地址，也可以包括多个替代（代理镜像），无论是仅地址还是地址和表列表。

地址规范必须是以下之一：

```ini
address = hostname[:port] # 例如 server2:9312
address = /absolute/unix/socket/path # 例如 /var/run/manticore2.sock
```

`hostname` 是远程主机名，`port` 是远程 TCP 端口号，`table-list` 是以逗号分隔的表名列表，方括号 [] 表示可选子句。

如果省略表名，则假定与定义此行的表相同。换句话说，在为 'mycoolindex' 分布式表定义代理时，您可以只指向地址，假定您是在代理的端点上查询 mycoolindex 表。

如果省略端口号，则假定为 **9312**。如果定义了但无效（例如 70000），则将跳过该代理。

您可以将每个代理指向一个或多个位于一个或多个联网服务器上的远程表，没有限制。这启用了几种不同的使用模式：
* 在多个代理服务器上进行分片并创建任意集群拓扑
* 在多个代理服务器上进行镜像以实现高可用性和负载均衡
* 在本地主机内进行分片以利用多个核心（然而，仅使用多个本地表会更简单）

所有代理都是并行搜索的。索引列表逐字传递给远程代理。该列表在代理内的确切搜索方式（即顺序或并行）完全取决于代理的配置（参见 [threads](../../Server_settings/Searchd.md#threads) 设置）。主服务器对此没有远程控制。

重要的是要注意，`LIMIT` 选项在代理查询中被忽略。这是因为每个代理可以包含不同的表，因此由客户端负责将限制应用于最终结果集。这就是为什么在查询日志中查看时，针对物理表的查询与针对分布式表的查询不同。查询不能只是原始查询的简单复制，因为这不会产生正确的结果。

例如，如果客户端执行查询 SELECT ... LIMIT 10, 10，而第二个代理仅有 10 个文档，广播原始 `LIMIT 10, 10` 查询将导致从第二个代理接收到 0 个文档。然而，`LIMIT 10,10` 应返回结果集中的文档 10-20。为了解决这个问题，查询必须以更广泛的限制发送到代理，例如默认的 max_matches 值 1000。

例如，如果有一个分布式表 dist 引用远程表 user，客户端查询 `SELECT * FROM dist LIMIT 10,10` 将转换为 `SELECT * FROM user LIMIT 0,1000` 并发送到远程表 user。一旦分布式表接收到结果，它将应用 LIMIT 10,10 并返回请求的 10 个文档。

```sql
SELECT * FROM dist LIMIT 10,10;
```

查询将转换为：

```sql
SELECT * FROM user LIMIT 0,1000
```

此外，值可以为每个单独的代理指定选项，例如：
* [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - `random`, `roundrobin`, `nodeads`, `noerrors` （覆盖特定代理的全局 `ha_strategy` 设置）
* `conn` - `pconn`，持久（相当于在表级设置 `agent_persistent`）
* `blackhole` `0`,`1` （与代理的 [agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) 设置相同）
* `retry_count` 一个整数值（对应于 [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count)，但提供的值不会乘以镜像的数量）

```ini
agent = address1:table-list[[ha_strategy=value, conn=value, blackhole=value]]
```

示例：

```ini
# box1 上的配置
# 在 3 台服务器上对表进行分片
agent = box2:9312:shard1
agent = box3:9312:shard2

# box2 上的配置
# 在 3 台服务器上对表进行分片
agent = box1:9312:shard2
agent = box3:9312:shard3

# box3 上的配置
# 在 3 台服务器上对表进行分片
agent = box1:9312:shard1
agent = box2:9312:shard3

# 每个代理选项
agent = box1:9312:shard1[ha_strategy=nodeads]
agent = box2:9312:shard2[conn=pconn]
agent = box2:9312:shard2[conn=pconn,ha_strategy=nodeads]
agent = test:9312:any[blackhole=1]
agent = test:9312|box2:9312|box3:9312:any2[retry_count=2]
agent = test:9312|box2:9312:any2[retry_count=2,conn=pconn,ha_strategy=noerrors]
```

为了获得最佳性能，建议将位于同一服务器上的远程表放置在同一记录内。例如，您应该使用以下格式，而不是：
```ini
agent = remote:9312:idx1
agent = remote:9312:idx2
```
您应该更倾向于：
```ini
agent = remote:9312:idx1,idx2
```

## agent_persistent

```ini
agent_persistent = remotebox:9312:index2
```

`agent_persistent` 选项允许您持久连接到代理，这意味着在执行查询后，连接不会被断开。此指令的语法与 `agent` 指令相同。然而，主机将保持一个连接打开并在随后的查询中重用它，而不是为每个查询打开新连接然后关闭。每个代理主机的最大持久连接数由 searchd 部分中的 [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) 选项定义。

重要的是要注意，必须将 [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) 设置为大于 0 的值，才能使用持久代理连接。如果未定义，则默认为 0，且 `agent_persistent` 指令将与 `agent` 指令的行为相同。

使用持久的主代理连接可以减少 TCP 端口压力，并节省连接握手的时间，从而提高效率。

## agent_blackhole

```ini
agent_blackhole = testbox:9312:testindex1,testindex2
````

`agent_blackhole` 指令允许您将查询转发到远程代理，而无需等待或处理其响应。这对于调试或测试生产集群非常有用，因为您可以设置一个单独的调试/测试实例，并从生产主（聚合器）实例将请求转发给它，而不会干扰生产工作。主搜索将尝试连接到黑洞代理并正常发送查询，但不会等待或处理任何响应，且所有黑洞代理的网络错误将被忽略。值的格式与常规 `agent` 指令相同。

## agent_connect_timeout

```ini
agent_connect_timeout = 300
````

`agent_connect_timeout` 指令定义了连接到远程代理的超时时间。默认情况下，值被假定为毫秒，但可以有 [其他后缀](../../Server_settings/Special_suffixes.md))。默认值为 1000（1 秒）。

连接到远程代理时，`searchd` 最多将等待这一段时间以成功完成连接。如果超时达到但连接尚未建立，并且启用了 `retries`，则会启动重试。

## agent_query_timeout

```ini
agent_query_timeout = 10000 # 我们的查询可能很长，允许最多 10 秒
```

`agent_query_timeout` 设置了 `searchd` 等待远程代理完成查询的时间。默认值为 3000 毫秒（3 秒），但可以添加 `后缀` 以表示不同的时间单位。

建立连接后，`searchd` 将最多等待 agent_query_timeout 以完成远程查询。请注意，此超时与 `agent_connection_timeout` 是分开的，远程代理可能造成的总延迟将是两个值的总和。如果达到 agent_query_timeout，将 **不** 重试查询，而是会产生警告。

请注意，行为也会受到 [reset_network_timeout_on_packet](../../Server_settings/Searchd.md#reset_network_timeout_on_packet) 的影响。

## agent_retry_count

`agent_retry_count` 是一个整数，指定在报告致命查询错误之前，Manticore 将尝试连接和查询分布式表中的远程代理的次数。它的工作方式与配置文件 "searchd" 部分中定义的 `agent_retry_count` 类似，但专门针对该表。

## mirror_retry_count

`mirror_retry_count` 的功能与 `agent_retry_count` 相同。如果同时提供了这两个值，`mirror_retry_count` 将优先，并会产生警告。

## 实例级别的选项

以下选项管理远程代理的整体行为，并在 **配置文件的 searchd 部分** 指定。它们设置了整个 Manticore 实例的默认值。

* `agent_connect_timeout` - `agent_connect_timeout` 参数的默认值。
* `agent_query_timeout` - `agent_query_timeout` 参数的默认值。也可以在分布式（网络）表中使用相同的设置名称进行每个查询的覆盖。
* `agent_retry_count` 是一个整数，指定在报告致命查询错误之前，Manticore 将尝试连接和查询分布式表中的远程代理的次数。默认值为 0（即没有重试）。可以使用 'OPTION retry_count=XXX' 语句在每个查询的基础上指定此值。如果提供了每个查询的选项，它将优先于配置中指定的值。

请注意，如果您在分布式表的定义中使用 **代理镜像**，则服务器将在每次连接尝试之前根据指定的 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 选择不同的镜像。在这种情况下，[agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) 将对集合中的所有镜像进行汇总。

例如，如果您有 10 个镜像并设置 `agent_retry_count=5`，则服务器将尝试最多 50 次重试（假设每 10 个镜像平均尝试 5 次）。在选项 `ha_strategy = roundrobin` 的情况下，每个镜像实际上将确切尝试 5 次。
与此同时，在`agent`定义中作为[retry_count](../../Searching/Options.md#retry_count)选项提供的值作为绝对限制。换句话说，在agent定义中的`[retry_count=2]`选项意味着最多会进行2次尝试，无论线路中有1个还是10个镜像。

### agent_retry_delay

`agent_retry_delay`是一个整数值，确定在发生故障时，Manticore Search在重试查询远程代理之前等待的时间（以毫秒为单位）。该值可以在searchd配置中全局指定，也可以在每个查询的基础上使用`OPTION retry_delay=XXX`子句进行指定。如果同时提供两个选项，则每个查询的选项将优先于全局选项。默认值为500毫秒（0.5秒）。只有当agent_retry_count或每个查询的`OPTION retry_count`非零时，此选项才相关。

### client_timeout

`client_timeout`选项设置在使用持久连接时请求之间的最大等待时间。该值以秒或带时间后缀的形式表示。默认值为5分钟。

示例：

```ini
client_timeout = 1h
```

### hostname_lookup

`hostname_lookup`选项定义了更新主机名的策略。默认情况下，代理主机名的IP地址在服务器启动时进行缓存，以避免对DNS的过度访问。然而，在某些情况下，IP可能会动态变化（例如，云托管），并且可能希望不缓存IP。将此选项设置为`request`会禁用缓存，并为每个查询查询DNS。IP地址也可以通过`FLUSH HOSTNAMES`命令手动更新。

### listen_tfo

`listen_tfo`选项允许所有侦听器使用TCP_FASTOPEN标志。默认情况下，由系统管理，但可以通过将其设置为'0'显式关闭。

有关TCP Fast Open扩展的更多信息，请参阅[Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一个TCP往返延迟。

实际上，使用TFO可以优化客户端-代理的网络效率，类似于使用`agent_persistent`时，但不会保持活动连接，并且没有对最大连接数的限制。

大多数现代操作系统支持TFO。Linux（作为最进步的之一）自2011年以来就支持它，从3.7开始的内核（用于服务器端）。Windows自某些版本的Windows 10以来就支持它。其他系统，例如FreeBSD和MacOS，也在其中。

对于Linux系统，服务器检查变量`/proc/sys/net/ipv4/tcp_fastopen`并相应地进行处理。位0管理客户端端，而位1管理侦听器。默认情况下，系统将该参数设置为1，即客户端已启用，监听器已禁用。

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

`persistent_connections_limit`选项定义与远程持久代理的同时持久连接的最大数量。这是一个实例范围的设置，必须在searchd配置部分中定义。每次与在`agent_persistent`下定义的代理建立连接时，我们尝试重用现有连接（如果存在）或创建新连接并保存以供将来使用。然而，在某些情况下，可能需要限制持久连接的数量。该指令定义限制，并影响所有分布式表中每个代理主机的连接数量。

建议将该值设置为小于或等于代理配置中的[max_connections](../../Server_settings/Searchd.md#max_connections)选项。

## 分布式片段创建

分布式表的一个特殊案例是单个本地和多个远程表，仅用于[分布式片段创建](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-snippets-creation)，当片段来自文件时。在这种情况下，本地表可以充当“模板”表，在构建片段时提供分词设置。

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

`snippets_file_prefix`是一个可选前缀，可以在生成片段时添加到本地文件名。默认值是当前工作文件夹。

要了解有关分布式片段创建的更多信息，请参阅[CALL SNIPPETS](../../Searching/Highlighting.md)。

## 分布式渗透表（DPQ表）

您可以从多个[渗透](../../Creating_a_table/Local_tables/Percolate_table.md)表创建一个分布式表。这种类型表的构造语法与其他分布式表相同，并可以包括多个`local`表以及`agents`。

对于DPQ，列出存储查询和搜索（使用[CALL PQ](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)）的操作是透明的，工作方式就像所有表都是一个单一的本地表。然而，诸如`insert`、`replace`、`truncate`等数据操作语句不可用。

如果您在代理列表中包含非渗透表，则行为将无法定义。如果不正确的代理与PQ表的外部模式（id、查询、标签、过滤器）具有相同模式，则在列出存储的PQ规则时不会触发错误，并可能将其自己的非PQ字符串污染存储在PQ表中的实际PQ规则列表。因此，请小心并注意这可能导致的混淆。对这样的不正确代理的`CALL PQ`将触发错误。
有关对分布式渗透表进行查询的更多信息，请参见[对分布式渗透表进行查询](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ).

<!-- proofread -->

