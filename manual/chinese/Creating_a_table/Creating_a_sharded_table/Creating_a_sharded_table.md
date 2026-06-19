# 创建分片表

> 注意：自动分片需要 [Manticore Buddy](../../Installation/Manticore_Buddy.md)。如果自动分片不起作用，请确认 Buddy 已安装并正在运行。

Manticore 支持创建 **分片表**，这是一种特殊表类型（`type='shard'`），可以透明地将读写分发到多个底层物理分片上。在扩展数据规模时，这个特性非常有价值。你既可以在单台服务器上创建本地分片表，也可以在多节点复制集群上创建复制分片表，以优化数据分布。

分片表为高性能应用提供了几个关键优势：
- 通过将数据并行写入多个分片，充分利用系统资源，从而带来更强的写入性能。与单个大表相比，这种并行写入能力可以带来更高的索引吞吐量。
- 分片表内置支持复制能力，提升高可用性。你无需手动处理复制。只需在创建分片表时设置复制因子，系统会自动管理一切。这种内置复制可确保即使部分节点失败，数据仍然持续可访问。

#### 选项

- `shards='N'` — 要创建的物理分片数量。必填。必须是正整数，最大为 3000。
- `rf='M'` — 复制因子：每个分片保留的副本数。必填。在单节点上必须为 `1`；在复制集群上必须介于 1 和集群节点数之间。
- `timeout='S'` — 创建期间等待分片准备完成的时间（单位：秒）。默认值为 `30`。当在多个节点上创建大量分片时，可以适当增大该值。

值必须加引号（`shards='10'`，而不是 `shards=10`）。选项名称不区分大小写。

#### 创建本地分片表

要创建本地分片表，像平常一样创建表，并添加 `shards='N'` 和 `rf='1'`。`shards` 是将会在表后端创建的物理分片数量。`rf` 是复制因子。在单台服务器上它必须为 `1`。

下面是一个创建包含 10 个分片的表的示例，数据会自动分布到这些分片上：

```sql
CREATE TABLE local_sharded shards='10' rf='1'
```

执行这条查询后，你会得到一个分片表，且所有分片都已完成设置。底层物理分片位于 `system` 数据库下，并且不会在 `SHOW TABLES` 中显示；你通过分片表的对外名称进行操作，系统会自动把请求路由到相应的分片。

#### 创建复制分片表

要防止服务器宕机，请在你希望参与的各个节点上搭建复制集群。在本文档中，我们默认你创建的集群名为 `c`。请按照 [replication](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) 说明添加所有需要的节点，然后按所需复制因子创建表。

例如，假设你有一个 3 节点复制集群，并希望创建一个被拆分为 10 个分片的表，每个节点上都保留每个分片的一份副本。由于有三个节点参与，你应设置 `rf='3'`：

```sql
CREATE TABLE c:cluster_sharded shards='10' rf='3'
```

之后，你就可以在任意集群节点上通过表的普通名称来操作它了，`INSERT`、`SELECT`、`UPDATE` 和 `DELETE` 都不需要集群前缀。集群前缀（`c:`）只用于 `CREATE TABLE` 和 `DROP TABLE` 这类 DDL。

创建过程中等待所有分片准备完成的默认超时时间是 30 秒。有时，在多节点复制集群上创建大量分片时，由于网络延迟，可能需要更长时间。如有需要，可以通过 `timeout` 选项增加该超时值：

```sql
CREATE TABLE c:cluster_sharded shards='10' rf='3' timeout='60'
```

如果超时被超过，表创建将失败，你需要使用更长的超时值重新尝试。

#### 删除分片表

要删除分片表，请使用标准的 `DROP TABLE` 命令。在集群环境中，请在表名中指定集群名称，以便正确定位要删除的表。支持 `IF EXISTS`。

删除本地分片表：

```sql
DROP TABLE local_sharded
DROP TABLE IF EXISTS local_sharded
```

删除复制分片表：

```sql
DROP TABLE c:cluster_sharded
DROP TABLE IF EXISTS c:cluster_sharded
```

#### 检查分片表

`DESC <table>` 会返回分片表对用户可见的字段模式（也就是你声明的列）。

`SHOW CREATE TABLE <table>` 会返回对用户可见的定义，其中包含 `shards='N' rf='M'`，而内部的 `type='shard'` 拓扑结构（每个分片的 `local=`/`agent=` 子句以及以 md5 命名的复制集群）会被有意隐藏。若要查看解析后的内部拓扑以便诊断，请使用 `SHOW CREATE TABLE <table> OPTION force=1`。

另外还提供了两个命令用于检查分片子系统的状态：

- `SHOW SHARDING STATUS [[<cluster>:]<table>]` — 列出每个分片的部署位置和健康状态。不带参数时会列出所有分片表；带表名（可选加集群前缀）时，只筛选该表。返回列：`table`、`shard`、`node`、`status`（`active`/`inactive`）、`cluster`、`replication_cluster`、`rf` 和 `rf_status`（`ok`/`degraded`/`broken`）。

   ```sql
   SHOW SHARDING STATUS
   SHOW SHARDING STATUS cluster_sharded
   SHOW SHARDING STATUS c:cluster_sharded
   ```

- `SHOW SHARDING MASTER` — 显示当前由哪个节点运行分片主进程，以及它是 `active` 还是 `inactive`。

   ```sql
   SHOW SHARDING MASTER
   ```

<!-- proofread -->

