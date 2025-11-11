# 二进制日志

二进制日志作为[实时](../Creating_a_table/Local_tables/Real-time_table.md)表数据的恢复机制。当启用二进制日志时，`searchd` 会将每个事务记录到 binlog 文件中，并在非正常关闭后利用它进行恢复。在正常关闭期间，RAM 块会保存到磁盘，随后所有 binlog 文件都会被删除。

## 启用和禁用二进制日志

默认情况下，二进制日志是启用的，以保障数据完整性。在 Linux 系统上，[纯模式](../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)下 `binlog.*` 文件的默认位置是 `/var/lib/manticore/data/`。在[RT 模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)下，二进制日志存储在 `<data_dir>/binlog/` 文件夹中，除非另有指定。

### 全局二进制日志配置

<!-- example binlog_path -->
要全局禁用二进制日志，请在 `searchd` 配置中将 `binlog_path` 设置为空值。
禁用二进制日志需要重启守护进程，并且如果系统意外关闭，数据将面临风险。

<!-- request Example -->
```ini
searchd {
...
    binlog_path = # disable logging
...
```
<!-- end -->

<!-- example binlog_path2 -->
您可以使用以下指令设置自定义路径：

<!-- request Example -->
```ini
searchd {
...
    binlog_path = /var/data
...
```
<!-- end -->

### 每表二进制日志配置

<!-- Example binlog0 -->
为了更细粒度的控制，可以通过将实时表的 `binlog` 表参数设置为 `0` 来禁用该表的二进制日志。此选项不适用于 percolate 表。

<!-- request Example -->
```sql
create table a (id bigint, s string attribute) binlog='0';
```
<!-- end -->

<!-- Example binlog_alter -->
对于已有的 RT 表，也可以通过修改 `binlog` 参数来禁用二进制日志。

<!-- request Example -->
```sql
alter table FOO binlog='0';
```
<!-- end -->

<!-- Example binlog_alter2 -->
如果之前禁用了二进制日志，可以通过将 `binlog` 参数重新设置为 `1` 来重新启用：

<!-- request Example -->
```sql
alter table FOO binlog='1';
```
<!-- end -->

#### 重要注意事项：
* **依赖全局设置**：每表的二进制日志设置仅在全局启用了二进制日志（`binlog_path` 不为空）时生效。
* **二进制日志状态和事务 ID 说明**：修改表的二进制日志状态会强制立即[刷新表](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE)。如果关闭表的二进制日志，其事务 ID（TID）会变为 `-1`，表示二进制日志未激活，且不跟踪任何更改。相反，如果启用表的二进制日志，其事务 ID 会变为非负数（零或更大），表示该表的更改正在被记录。您可以使用命令 `SHOW TABLE <name> STATUS` 来检查事务 ID。事务 ID 反映了表的更改是否被记录（非负数）或未被记录（`-1`）。

## 操作

当启用二进制日志时，对 RT 表的每次更改都会保存到日志文件中。如果系统意外关闭，这些日志会在系统重新启动时自动使用，以恢复所有已记录的更改。

### 日志大小

<!-- Example binlog_max_log_size -->
在正常操作期间，当记录的数据量达到一定限制（由 `binlog_max_log_size` 设置）时，会启动一个新的日志文件。旧的日志文件会保留，直到其中的所有更改都被完全处理并保存为磁盘块。如果该限制设置为 `0`，则日志文件会一直保留，直到系统正常关闭。默认情况下，这些文件的大小没有限制。

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

每个 binlog 文件的命名格式为带零填充的数字，如 `binlog.0000`、`binlog.0001` 等，通常显示四位数字。您可以通过设置 `binlog_filename_digits` 来更改数字的位数。如果 binlog 文件数量超过数字位数能容纳的范围，数字位数会自动增加以适应所有文件。

**重要**：要更改数字位数，必须先保存所有表数据并正常关闭系统。然后删除旧的日志文件并重启系统。

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
您可以选择两种管理二进制日志文件的方式，通过 `binlog_common` 指令设置：
* 每表单独文件（默认，`0`）：每个表将其更改保存到自己的日志文件中。如果您有许多表在不同时间更新，这种设置很合适。它允许表独立更新，无需等待其他表。此外，如果某个表的日志文件出现问题，不会影响其他表。
* 所有表共用单个文件（`1`）：所有表使用同一个二进制日志文件。这种方法使文件管理更简单，因为文件数量较少。然而，如果某个表仍需保存更新，文件可能会被保留时间较长。如果许多表同时需要更新，这种设置可能会降低性能，因为所有更改都必须等待写入同一个文件。

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
有四种不同的 binlog 刷新策略，由 `binlog_flush` 指令控制：

* `0` - 数据每秒写入磁盘（刷新），并且 Manticore 在刷新后立即启动将其安全写入磁盘的操作（[同步](https://linux.die.net/man/8/sync)）。此方法速度最快，但如果服务器或计算机突然崩溃，某些最近写入但尚未安全写入的数据可能会丢失。
* `1` - 数据在每个事务后立即写入 binlog 并同步。此方法最安全，确保每次更改立即被保存，但写入速度较慢。
* `2` - 每个事务后写入数据，并且每秒启动一次同步。此方法提供了一个平衡，定期且快速地写入数据。然而，如果计算机发生故障，某些正在保存的数据可能无法完成保存。此外，根据磁盘情况，同步可能需要超过一秒的时间。
* `3` - 类似于 `2`，但还确保在因超过 `binlog_max_log_size` 而关闭 binlog 文件之前进行同步。

默认模式是 `2`，它在每个事务后写入数据，并每秒开始同步，平衡了速度和安全性。

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
在使用 Galera 的集群设置中，节点恢复行为至关重要。通常，如果节点已正常关闭且其最后的序列号（seqno）已正确保存，Galera 会通过 IST（增量状态传输）处理节点不同步的情况。然而，在发生崩溃且 seqno 未被保存的情况下，Galera 将触发 SST（状态快照传输），这是一种资源密集型操作，可能由于高 I/O 活动显著减慢集群速度。

为了解决这个问题，引入了集群 binlog 支持。此功能扩展了现有的二进制日志功能，帮助减少 SST 的需求，使恢复节点能够从本地 binlog 重放缺失的事务，并以有效的 seqno 重新加入集群。

集群 binlog 默认对任何集群操作启用。但可以通过设置环境变量来禁用：

<!-- request binlog_cluster -->
```bash
MANTICORE_REPLICATION_BINLOG=0
```
<!-- end -->

此功能通过结合二进制日志的本地持久性和 Galera 分布式同步能力，减少了停机时间并避免了完整数据传输。

### 恢复

在非正常关闭后的恢复过程中，会重放 binlog，并恢复自上次良好磁盘状态以来记录的所有事务。事务带有校验和，因此如果 binlog 文件损坏，垃圾数据将**不会**被重放；此类损坏的事务将被检测到并停止重放。

### 刷新 RT RAM 块

<!-- Example rt_flush_period -->
对完全适合于 RAM 块的小型 RT 表进行密集更新，可能导致 binlog 不断增长，直到干净关闭之前无法被删除。binlog 本质上是相对于磁盘上最后已知良好保存状态的追加式增量，除非 RAM 块被保存，否则无法删除。不断增长的 binlog 对磁盘使用和崩溃恢复时间都不理想。为了解决此问题，可以使用 `rt_flush_period` 指令配置 `searchd` 执行周期性 RAM 块刷新。启用周期性刷新后，`searchd` 将维护一个单独线程，检查 RT 表的 RAM 块是否需要写回磁盘。一旦发生，相关的 binlog 可以（且会被）安全删除。

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

需要注意的是，`rt_flush_period` 仅控制检查的频率。不能保证特定的 RAM 块一定会被保存。例如，定期重新保存仅接收少量行更新的大 RAM 块是没有意义的。Manticore 会使用一些启发式方法自动决定是否执行刷新。

<!-- proofread -->

