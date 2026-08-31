# 远程表

Manticore Search 中的远程表通过分布式表定义中的 [agent](../../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) 前缀表示。分布式表可以包含本地表和远程表的组合。如果没有提供本地表，分布式表将纯粹是远程的，仅作为代理。例如，您可能有一个监听多个端口并服务不同协议的 Manticore 实例，然后重定向查询到仅通过 Manticore 内部二进制协议接受连接的后端服务器，使用持久连接以减少建立连接的开销。
尽管纯远程分布式表本身不提供本地表，但它仍然消耗机器资源，因为它仍需要执行最终计算，如合并结果和计算最终的聚合值。

## agent

```ini
agent = address1 [ | address2 [...] ][:table-list]
agent = address1[:table-list [ | address2[:table-list [...] ] ] ]
```

`agent` 指令声明了每次搜索包含该分布式表时搜索的远程代理。这些代理本质上是指向网络表的指针。指定的值包括地址，还可以包括地址或地址和表列表的多个替代方案（代理镜像）。有关镜像语法，请参阅 [镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md)，有关如何选择镜像代理，请参阅 [负载均衡](../../Creating_a_cluster/Remote_nodes/Load_balancing.md)。

地址规范必须是以下之一：

```ini
address = hostname[:port] # eg. server2:9312
address = /absolute/unix/socket/path # eg. /var/run/manticore2.sock
```

`hostname` 是远程主机名，`port` 是远程 TCP 端口号，`table-list` 是逗号分隔的表名列表，方括号 [] 表示可选子句。对于 TCP 连接，此端口是远程代理/API 端口（通常为 `9312`），而不是 MySQL 端口（`9306`）。

如果省略表名，假定为定义该行的同一表。换句话说，当为分布式表 'mycoolindex' 定义代理时，可以简单指向地址，默认查询该代理端点上的 mycoolindex 表。

如果省略端口号，默认是 **9312**。如果定义但无效（例如 70000），则跳过该代理。

可以将每个代理指向一个或多个位于一个或多个网络服务器上的远程表，没有限制。这使得几种不同的使用模式成为可能：
* 在多个代理服务器上分片并创建任意的集群拓扑
* 在多个代理服务器上分片以实现高可用性和负载均衡（请参阅 [镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md) 和 [负载均衡](../../Creating_a_cluster/Remote_nodes/Load_balancing.md)）
* 在本地主机中分片以利用多个核心（不过，更简单的是使用多个本地表）

为了避免混淆：
* 一个 `agent='host1|host2:table'` 条目表示一个具有镜像后端的远程分片。
* 多个 `agent='...'` 条目表示多个远程分片。

所有代理都是并行搜索的。索引列表直接传递给远程代理。该列表在代理内具体如何搜索（顺序或并行）完全取决于代理的配置（参见[threads](../../Server_settings/Searchd.md#threads)设置）。主控无法远程控制此行为。

需要注意的是，代理查询中会忽略 `LIMIT` 选项。这是因为每个代理可能包含不同的表，因此客户端负责在最终结果集上应用限制。这就是为何对物理表的查询与对分布式表的查询在查询日志中体现不同。查询不能简单复制原始查询，否则不会产生正确结果。

例如，如果客户端执行查询 SELECT ... LIMIT 10, 10，且有两个代理，第二个代理只有 10 条文档，广播原始 `LIMIT 10, 10` 查询会导致第二个代理返回 0 条文档。然而，`LIMIT 10,10` 应该返回结果集中的第10到20条。为解决此问题，必须向代理发送更宽松限制的查询，例如默认 max_matches 值 1000。

比如，若有分布式表 dist 指向远程表 user，客户端查询 `SELECT * FROM dist LIMIT 10,10` 会被转换为 `SELECT * FROM user LIMIT 0,1000` 并发送至远程表 user。一旦分布式表收到结果，它将应用 LIMIT 10,10，返回请求的 10 条文档。

```sql
SELECT * FROM dist LIMIT 10,10;
```

查询将被转换为：

```sql
SELECT * FROM user LIMIT 0,1000
```

另外，值可以为每个单独代理指定选项，例如：
* [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) - `random`, `roundrobin`, `nodeads`, `noerrors`（覆盖特定代理的全局 `ha_strategy` 设置；另请参阅 [镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md)）
* `conn` - `pconn`，持久连接（相当于在表级设置 `agent_persistent`）
* `blackhole` `0`,`1`（与代理的 [agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) 设置相同）
* `retry_count` 整数值（对应 [agent_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_retry_count) ，但提供的值不会乘以镜像数量）

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

## 定义远程表

<!-- example remote-table-definition -->
远程表可以在配置文件中定义，也可以在分布式表上使用 SQL 定义。

<!-- intro -->
##### 配置：

<!-- request Config -->
```ini
table products_dist {
  type  = distributed
  agent = 127.0.0.1:9312|127.0.0.1:9313:products
}
```

<!-- intro -->
##### SQL：

<!-- request SQL -->
```sql
CREATE TABLE products_dist type='distributed'
  agent='127.0.0.1:9312:products|127.0.0.1:9313:products';
```
<!-- end -->

<!-- example remote-table-setup -->
假设远程表已经存在，例如：

<!-- intro -->
##### 远程节点 1 上的 SQL：

<!-- request SQL -->
```sql
CREATE TABLE products(id bigint, title text, node string);
INSERT INTO products(id,title,node) VALUES(1,'same title','node1');
```

<!-- intro -->
##### 远程节点 2 上的 SQL：

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
使用主服务器上的分布式表，就像使用其他任何表一样，然后验证它是如何存储的。

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

为了获得最佳性能，建议将位于同一服务器上的远程表置于相同记录中。例如，不要：
```ini
agent = remote:9312:idx1
agent = remote:9312:idx2
```
建议使用：
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

`agent_persistent` 选项允许你持久连接到代理，即查询执行后连接不会断开。该指令语法与 `agent` 指令相同。但不同的是，主控不会为每个查询打开新连接然后关闭，而是保持连接并重用以进行后续查询。每个代理主机的最大持久连接数由 searchd 部分的 [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) 选项定义。

需要注意的是，必须将 [persistent_connections_limit](../../Server_settings/Searchd.md#persistent_connections_limit) 设置为大于 0 的值才能使用持久代理连接。如果未定义，默认为 0，`agent_persistent` 指令的行为将与 `agent` 指令相同。

使用持久主从代理连接可以减少 TCP 端口压力，节省连接握手时间，从而提高效率。

## agent_blackhole

<!-- example agent-blackhole -->
`agent_blackhole` 指令允许您将查询转发到远程代理，而无需等待或处理它们的响应。这对于调试或测试生产集群很有用，因为您可以设置一个独立的调试/测试实例，并从生产主（聚合器）实例将其请求转发到它，而不会干扰生产工作。主 searchd 将尝试连接到黑洞代理并像平常一样发送查询，但不会等待或处理任何响应，黑洞代理上的所有网络错误都将被忽略。值的格式与常规 `agent` 指令的格式相同。

<!-- intro -->
##### Config:

<!-- request Config -->
```ini
agent_blackhole = testbox:9312:testindex1,testindex2
```
<!-- end -->

## 相关选项和支持的作用域

并非所有与远程代理相关的选项都在相同的作用域中得到支持。本页重点介绍远程表和每代理语义。有关镜像特定行为的信息，请参阅 [镜像](../../Creating_a_cluster/Remote_nodes/Mirroring.md) 和 [负载均衡](../../Creating_a_cluster/Remote_nodes/Load_balancing.md)。有关完整的守护进程级别行为，请使用 [Searchd 设置](../../Server_settings/Searchd.md) 中的链接参考页面。

| 选项 | 实例范围 | 每表 | 每查询 | 每代理 | 详细信息 |
|---|---|---|---|---|---|
| `ha_strategy` | 是 | 是 | 否 | 是 | [负载均衡](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy), [远程表：代理](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_connect_timeout` | 是 | 是 | 否 | 否 | [Searchd：agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout) |
| `agent_query_timeout` | 是 | 是 | 是 | 否 | [Searchd：agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout) |
| `agent_retry_count` / `mirror_retry_count` / `retry_count` | 是（`agent_retry_count`） | 是（`agent_retry_count` 或 `mirror_retry_count`） | 是（`OPTION retry_count=...`） | 是（`agent=...[retry_count=...]`） | [Searchd：agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count), [远程表：mirror_retry_count](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#mirror_retry_count), [远程表：agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| `agent_retry_delay` | 是 | 否 | 是 | 否 | [Searchd：agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay) |
| 每代理 `conn` | 否 | 否 | 否 | 是 | [远程表：agent](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent) |
| 每代理 `blackhole` | 否 | 否 | 否 | 是 | [远程表：agent_blackhole](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent_blackhole) |

### agent_connect_timeout

<!-- example remote-agent-connect-timeout -->
`agent_connect_timeout` 定义 Manticore 等待建立与远程代理连接的时间。它支持作为实例范围的默认值和每分布式表。完整的守护进程级别详细信息请参阅 [Searchd：agent_connect_timeout](../../Server_settings/Searchd.md#agent_connect_timeout)。

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
`agent_query_timeout` 定义 Manticore 等待已连接的远程代理完成查询的时间。它支持作为实例范围的默认值、每分布式表和每查询作为 `OPTION agent_query_timeout=...`。完整的守护进程级别详细信息请参阅 [Searchd：agent_query_timeout](../../Server_settings/Searchd.md#agent_query_timeout)。

如果达到 `agent_query_timeout`，查询不会自动重试；而是会生成警告。行为还受 [reset_network_timeout_on_packet](../../Server_settings/Searchd.md#reset_network_timeout_on_packet) 的影响。

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
##### SQL 查询选项：

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION agent_query_timeout=750;
```
<!-- end -->

### agent_retry_count

`agent_retry_count` 指定了 Manticore 在分布式表中尝试连接和查询远程代理的次数，直到报告致命查询错误。名称根据作用域而变化：使用
- `agent_retry_count` 作为实例范围的设置，
- 在分布式表中使用 `agent_retry_count` 或其别名 `mirror_retry_count`，
- 每个查询使用 `OPTION retry_count=...`，
- 以及在单个 `agent=...` 声明中使用 `[retry_count=...]`。

完整的守护进程级别详情请参见 [Searchd: agent_retry_count](../../Server_settings/Searchd.md#agent_retry_count)。

如果您使用 **agent mirrors**，服务器会在每次连接尝试之前根据 [ha_strategy](../../Creating_a_cluster/Remote_nodes/Load_balancing.md#ha_strategy) 选择不同的镜像，并且 `agent_retry_count` 会在镜像之间汇总。

### mirror_retry_count

<!-- example remote-agent-retry-count -->
`mirror_retry_count` 与 `agent_retry_count` 的作用相同，但仅作为分布式表设置。如果提供了两个值，`mirror_retry_count` 会优先。

<!-- intro -->
##### 配置:

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
##### SQL 查询选项:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=1;
```

<!-- intro -->
##### 每个代理的配置上限:

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
`agent_retry_delay` 定义了重试尝试之间的延迟。它支持作为实例范围的默认值和每个查询的 `OPTION retry_delay=...`，但不支持每个分布式表。完整的守护进程级别详情请参见 [Searchd: agent_retry_delay](../../Server_settings/Searchd.md#agent_retry_delay)。

当通过 `agent_retry_count` 或 `OPTION retry_count=...` 启用重试时，此选项才相关。

<!-- intro -->
##### 配置:

<!-- request Config -->
```ini
agent_retry_delay = 500ms
```

<!-- intro -->
##### SQL 查询选项:

<!-- request SQL -->
```sql
SELECT * FROM products_dist OPTION retry_count=2, retry_delay=300;
```
<!-- end -->

### client_timeout

<!-- example client-timeout -->
`client_timeout` 选项设置在使用持久连接时请求之间的最大等待时间。这是一个实例范围的 `searchd` 设置，而不是每个表的选项。该值以秒或带时间后缀的形式表示。默认值为 5 分钟。完整的守护进程级别详情请参见 [Searchd: client_timeout](../../Server_settings/Searchd.md#client_timeout)。

<!-- intro -->
##### 配置:

<!-- request Config -->
```ini
client_timeout = 1h
```
<!-- end -->

### hostname_lookup

`hostname_lookup` 选项定义了主机名的更新策略。默认情况下，代理主机名的 IP 地址在服务器启动时缓存，以避免频繁访问 DNS。然而，在某些情况下（例如云托管环境），IP 会动态变化，可能希望不缓存 IP。将此选项设置为 `request` 可禁用缓存，每次查询时都会查询 DNS。IP 地址也可以通过 `FLUSH HOSTNAMES` 命令手动刷新。

### listen_tfo

`listen_tfo` 选项允许对所有监听器使用 TCP_FASTOPEN 标志。默认情况下，该选项由系统管理，但可以通过将其设置为 '0' 显式关闭。

有关 TCP Fast Open 扩展的更多信息，请参阅 [Wikipedia](https://en.wikipedia.org/wiki/TCP_Fast_Open)。简而言之，它允许在建立连接时消除一次 TCP 往返。

在实际应用中，使用 TFO 可以优化客户端与代理之间的网络效率，类似于使用 `agent_persistent` 时，但无需保持活动连接，也没有最大连接数的限制。

大多数现代操作系统都支持 TFO。Linux（作为较为先进的系统之一）自 2011 年起支持它，内核版本从 3.7 开始支持（服务器端）。Windows 自 Windows 10 的某些版本开始支持该功能。其他系统，如 FreeBSD 和 MacOS，也在支持范围内。

对于 Linux 系统，服务器会检查变量 `/proc/sys/net/ipv4/tcp_fastopen` 并据此行为。位 0 管理客户端，位 1 管理监听器。默认情况下，系统将此参数设置为 1，即客户端启用，监听器禁用。

### persistent_connections_limit

```ini
persistent_connections_limit = 29 # assume that each host of agents has max_connections = 30 (or 29).
```

`persistent_connections_limit` 选项定义了与远程持久代理的最大同时持久连接数。这是一个实例级别的设置，必须在 searchd 配置部分中定义。每当连接到在 `agent_persistent` 下定义的代理时，我们会尝试重用现有连接（如果存在），或创建新连接并保存以便将来使用。然而，在某些情况下可能需要限制持久连接的数量。该指令定义了限制，并影响所有分布式表中每个代理主机的连接数。

建议将此值设置为不大于代理配置中的 [max_connections](../../Server_settings/Searchd.md#max_connections) 选项。

## 分布式片段创建

分布式表的一个特殊情况是一张本地表和多张远程表，这种情况专门用于 [分布式片段创建](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#Distributed-snippets-creation)，当片段源自文件时。在这种情况下，本地表可作为“模板”表，提供构建片段时的分词设置。

### snippets_file_prefix

```ini
snippets_file_prefix = /mnt/common/server1/
```

`snippets_file_prefix` 是一个可选的前缀，可以在生成片段时添加到本地文件名。默认值为当前工作文件夹。

如需了解更多关于分布式片段创建的信息，请参阅 [CALL SNIPPETS](../../Searching/Highlighting.md)。

## 分布式预查询表（DPQ 表）

您可以从多个 [percolate](../../Creating_a_table/Local_tables/Percolate_table.md) 表创建一个分布式表。构造此类表的语法与其他分布式表相同，可以包含多个 `local` 表以及 `agents`。

对于 DPQ，列出存储查询和通过它们搜索（使用 [CALL PQ](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)）操作是透明的，且表现得如同所有表合并成一个本地表。然而，诸如 `insert`、`replace`、`truncate` 之类的数据操作语句不可用。

如果您在代理列表中包含了非预查询表，其行为将无法定义。如果错误代理与 PQ 表的外层架构（id、query、tags、filters）相同，在列出存储的 PQ 规则时不会引发错误，并且可能会用其非 PQ 字符串污染 PQ 表中实际存储的 PQ 规则列表。因此，务必谨慎，并意识到这可能带来的混淆。对该错误代理执行 `CALL PQ` 将引发错误。

有关对分布式预查询表进行查询的更多信息，请参阅 [对分布式预查询表进行查询](../../Searching/Percolate_query.md#Performing-a-percolate-query-with-CALL-PQ)。
<!-- proofread -->

