# 二进制日志记录

二进制日志记录作为[实时](../Creating_a_table/Local_tables/Real-time_table.md)表数据的恢复机制。当启用二进制日志时，`searchd` 会将每个事务记录到 binlog 文件中，并在非正常关闭后利用其进行恢复。在正常关闭期间，RAM 数据块会保存到磁盘，并随后删除所有的 binlog 文件。

## 启用和禁用二进制日志

默认情况下，启用二进制日志以保护数据完整性。在 Linux 系统上，[纯模式](../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)中 `binlog.*` 文件的默认位置是 `/var/lib/manticore/data/`。在[RT 模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)下，二进制日志存储在 `<data_dir>/binlog/` 文件夹内，除非另有指定。

### 全局二进制日志配置

<!-- example binlog_path -->
要全局禁用二进制日志，请在 `searchd` 配置中将 `binlog_path` 设置为空值。  
禁用二进制日志需要重启守护进程，并且如果系统意外关闭，可能会导致数据风险。

<!-- request Example -->
```ini
searchd {
...
    binlog_path = # disable logging
...
```
<!-- end -->

<!-- example binlog_path2 -->
你可以使用以下指令设置自定义路径：

<!-- request Example -->
```ini
searchd {
...
    binlog_path = /var/data
...
```
<!-- end -->

### 按表二进制日志配置

<!-- Example binlog0 -->
为了更细粒度的控制，可以通过将 `binlog` 表参数设置为 `0` 来禁用实时表的二进制日志。此选项不适用于 percolate 表。

<!-- request Example -->
```sql
create table a (id bigint, s string attribute) binlog='0';
```
<!-- end -->

<!-- Example binlog_alter -->
对于已存在的 RT 表，也可以通过修改 `binlog` 参数来禁用二进制日志。

<!-- request Example -->
```sql
alter table FOO binlog='0';
```
<!-- end -->

<!-- Example binlog_alter2 -->
如果之前禁用了二进制日志，可以通过将 `binlog` 参数重新设置为 `1` 来启用：

<!-- request Example -->
```sql
alter table FOO binlog='1';
```
<!-- end -->

#### 重要注意事项：
* **依赖全局设置**：每个表的二进制日志配置只有在 `searchd` 配置中全局启用了二进制日志（`binlog_path` 非空）时才生效。
* **二进制日志状态和事务 ID 说明**：修改表的二进制日志状态会强制立即[刷新表](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE)。如果关闭某个表的二进制日志，其事务 ID（TID）将变为 `-1`，表示未启用二进制日志且不跟踪更改。反之，如果开始启用二进制日志，事务 ID 会变为非负数（零或更大），表示表的变更正在被记录。你可以使用命令 `SHOW TABLE <name> STATUS` 来查看事务 ID。事务 ID 显示表的变动是否被记录（非负数）或未被记录（`-1`）。

## 操作

当启用二进制日志时，对 RT 表的每次更改都会保存到日志文件中。如果系统意外关闭，系统重启时会自动利用这些日志恢复所有已记录的更改。

### 日志大小

<!-- Example binlog_max_log_size -->
在正常运行时，当已记录数据大小达到由 `binlog_max_log_size` 设置的限制时，会创建一个新的日志文件。旧日志文件会保留，直到其中的所有更改被完全处理并保存为磁盘块。如果该限制设置为 `0`，日志文件会一直保留到系统正确关闭为止。默认情况下，没有限制日志文件大小。

<!-- request Example -->

```ini
searchd {
...
    binlog_max_log_size = 16M
....
```

<!-- end -->

### 日志文件

<!-- example binlog_filename_digits -->

每个 binlog 文件的命名格式为带零填充的数字，如 `binlog.0000`、`binlog.0001` 等，通常显示四位数字。你可以通过设置 `binlog_filename_digits` 来更改数字位数。如果 binlog 文件数量超过位数限制，位数会自动增加以适应所有文件。

**重要**：要更改数字位数，必须先保存所有表数据并正确关闭系统，然后删除旧的日志文件，再重启系统。

<!-- request Example -->
```ini
searchd {
...
    binlog_filename_digits = 6
...
```
<!-- end -->

### 二进制日志策略

<!-- Example binlog_common -->
你可以通过 `binlog_common` 指令选择两种二进制日志文件管理方式：
* 每表单独文件（默认，`0`）：每个表将其更改保存在自己的日志文件中。该设置适合存在多个不同时间更新的表。它允许表独立更新而不等待其他表。此外，一个表的日志文件出现问题不会影响其他表。
* 所有表共用单一文件（`1`）：所有表使用同一个二进制日志文件。该方式更易于管理文件，因为文件较少。然而，如果有表仍需保存更新，文件可能会被保留时间过长。如果多个表同时需要更新，可能会因等待写入同一文件而降低性能。

<!-- request binlog_common -->

```ini
searchd {
...
    binlog_common = 1
...
```
<!-- end -->

### 二进制刷新策略

<!-- Example binlog_flush -->
二进制日志刷新有四种策略，由 `binlog_flush` 指令控制：

* `0` - 数据每秒写入磁盘（刷新），随后 Manticore 会立即启动将其安全保存在磁盘上（[同步](https://linux.die.net/man/8/sync)）。此方法最快，但如果服务器或计算机突然崩溃，可能会丢失一些尚未安全保存的最近写入数据。
* `1` - 数据在每次事务后写入 binlog 并立即同步。此方法最安全，因为它确保每次更改都被立即保存，但会降低写入速度。
* `2` - 数据在每次事务后写入，并且每秒启动一次同步。此方法在定期快速写入数据与安全性之间取得平衡。但如果计算机失败，某些正在保存的数据可能未完成保存。此外，根据磁盘情况，同步可能需要超过一秒钟的时间。
* `3` - 类似于 `2`，但还确保 binlog 文件在因超过 `binlog_max_log_size` 而关闭之前同步。

默认模式为 `2`，即每次事务后写入数据，并每秒启动同步，平衡速度和安全性。

<!-- request Example -->
```ini
searchd {
...
    binlog_flush = 1 # ultimate safety, low write speed
...
}
```
<!-- end -->

### 集群 binlog 支持

<!-- Example binlog_cluster -->
在使用 Galera 的集群设置中，节点恢复行为至关重要。通常，如果节点正常关闭且其最后的序列号 (seqno) 被正确保存，Galera 会通过 IST（增量状态传输）处理节点不同步的情况。然而，如果发生崩溃导致 seqno 未被保存，Galera 会触发 SST（状态快照传输），这是一种资源密集型操作，会因高 I/O 活动显著减缓集群速度。

为了解决这一问题，引入了集群 binlog 支持。该功能扩展了现有的二进制日志功能，帮助减少 SST 的需求，使恢复节点能够从本地 binlog 重放缺失的事务，并带着有效的 seqno 重新加入集群。

集群 binlog 默认在任何集群操作中启用，但可以通过设置环境变量关闭：

<!-- request binlog_cluster -->
```bash
MANTICORE_REPLICATION_BINLOG=0
```
<!-- end -->

该功能通过结合二进制日志的本地持久性与 Galera 的分布式同步能力，减少了停机时间并避免了完整数据传输。

### 恢复

在非正常关闭后的恢复过程中，会重放 binlog，恢复自上一次良好磁盘状态以来所有记录的事务。事务有校验和，因此如果 binlog 文件损坏，垃圾数据**不会**被重放；损坏的事务将被检测到并停止重放。

### 刷新 RT RAM 块

<!-- Example rt_flush_period -->
频繁更新完全适合于 RAM 块的小型 RT 表会导致 binlog 不断增长，且除非干净关闭，否则无法解除链接。binlog 实质上是针对磁盘上最后已知良好保存状态的追加型增量，除非保存了 RAM 块，否则无法解除链接。持续增长的 binlog 对磁盘使用和崩溃恢复时间不理想。为了解决此问题，可以通过配置 `searchd` 使用 `rt_flush_period` 指令执行周期性 RAM 块刷新。启用周期刷新后，`searchd` 会维护一个单独线程，检查 RT 表的 RAM 块是否需要写回磁盘。一旦发生，相关的 binlog 就可以（并且会被）安全解除链接。

默认的 RT 刷新周期设置为 10 小时。

<!-- request Example -->
```ini
searchd {
...
    rt_flush_period = 3600 # 1 hour
...
}
```
<!-- end -->

重要的是，`rt_flush_period` 仅控制检查频率。不保证某个具体的 RAM 块一定会被保存。例如，定期重存仅有少量行更新的大 RAM 块没有意义。Manticore 使用一些启发式方法自动确定是否执行刷新。

<!-- proofread -->

