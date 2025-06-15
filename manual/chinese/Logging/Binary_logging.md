# 二进制日志

二进制日志作为[实时](../Creating_a_table/Local_tables/Real-time_table.md)表数据的恢复机制。当启用二进制日志时，`searchd`会将每个事务记录到binlog文件中，并在非正常关闭后利用其进行恢复。在正常关闭期间，RAM块会被保存到磁盘，然后所有binlog文件会被删除。

## 启用和禁用二进制日志

默认情况下，为了保护数据完整性，二进制日志是启用的。在Linux系统中，[Plain模式](../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)下`binlog.*`文件的默认位置是`/var/lib/manticore/data/`。在[RT模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)下，二进制日志存储在`<data_dir>/binlog/`文件夹中，除非另有指定。

### 全局二进制日志配置

<!-- example binlog_path -->
要在全局范围内禁用二进制日志，请在`searchd`配置中将`binlog_path`设置为空值。
禁用二进制日志需要重启守护进程，且如果系统意外关闭，数据将面临风险。

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

### 按表二进制日志配置

<!-- Example binlog0 -->
为了更细粒度的控制，可以通过将`binlog`表参数设置为`0`来禁用实时表的二进制日志。此选项不适用于Percolate表。

<!-- request Example -->
```sql
create table a (id bigint, s string attribute) binlog='0';
```
<!-- end -->

<!-- Example binlog_alter -->
对于已有的RT表，也可以通过修改`binlog`参数来禁用二进制日志。

<!-- request Example -->
```sql
alter table FOO binlog='0';
```
<!-- end -->

<!-- Example binlog_alter2 -->
如先前禁用了二进制日志，也可以通过将`binlog`参数重新设置为`1`来重新启用：

<!-- request Example -->
```sql
alter table FOO binlog='1';
```
<!-- end -->

#### 重要注意事项：
* **依赖全局设置**：按表的二进制日志设置只有在searchd配置中全局启用了二进制日志（`binlog_path`不为空）时才会生效。
* **二进制日志状态与事务ID信息**：修改表的二进制日志状态会强制立即[刷新表](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE)。如果您关闭了表的二进制日志，其事务ID (TID) 会变为`-1`。这表示二进制日志未激活，且未跟踪任何更改。反之，如果您启用表的二进制日志，其事务ID将变为非负数（零或更大），表示该表的更改正在被记录。您可以通过命令`SHOW TABLE <name> STATUS`查看事务ID。事务ID反映了表的更改是否正在被记录（非负数）或未被记录（`-1`）。

## 操作

当二进制日志启用时，对RT表的每次更改都会保存到日志文件中。如果系统意外关闭，这些日志会在系统重启时自动使用，以恢复所有记录的更改。

### 日志大小

<!-- Example binlog_max_log_size -->
在正常操作期间，当已记录数据达到某个限制（由`binlog_max_log_size`设定）时，会启动一个新的日志文件。旧的日志文件会被保留，直到其所有更改均被完全处理并保存为磁盘块。如果该限制设置为`0`，则日志文件会一直保存，直到系统正常关闭。默认情况下，文件大小没有限制。

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

每个binlog文件都以零填充的数字命名，如`binlog.0000`、`binlog.0001`等，通常显示四位数字。您可以通过设置`binlog_filename_digits`更改数字位数。如果binlog文件数量超过数字位数所能容纳的数量，位数会自动增加以容纳所有文件。

**重要**：若要更改数字位数，必须先保存所有表数据并正常关闭系统。然后删除旧日志文件并重启系统。

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
您可以选择两种管理二进制日志文件的方式，通过`binlog_common`指令设置：
* 为每个表使用独立文件（默认，`0`）：每个表在其自己的日志文件中保存更改。此设置适合有许多表在不同时间更新的情况。它允许表之间独立更新，且一个表的日志文件出现问题时不会影响其他表。
* 所有表使用单一文件（`1`）：所有表共用同一个二进制日志文件。此方法管理文件更简单，因为文件数量较少。但如果某个表仍需保存更新，文件可能会被保留更长时间。如果许多表同时需要更新，此设置可能会降低效率，因为所有更改需排队写入同一文件。

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
存在四种不同的binlog刷新策略，由`binlog_flush`指令控制：

* `0` - 每秒将数据写入磁盘（刷新），且Manticore在刷新后立即启动对磁盘上的数据安全措施（[同步](https://linux.die.net/man/8/sync)）。此方法最快，但如果服务器或计算机突然崩溃，最近写入但未安全保存的数据可能会丢失。
* `1` - 每个事务后立即写入binlog并同步。此方法最安全，确保每次更改立即被保存，但会降低写入速度。
* `2` - 每个事务后写入数据，并且每秒启动一次同步。这种方法提供了一个平衡，定期且快速地写入数据。然而，如果计算机发生故障，一些正在保护的数据可能无法完成保存。此外，同步时间可能会超过一秒，具体取决于磁盘。
* `3` - 类似于 `2`，但它还确保在因超过 `binlog_max_log_size` 而关闭二进制日志文件之前进行同步。

默认模式是 `2`，它在每个事务后写入数据并每秒开始同步，实现速度与安全性的平衡。

<!-- request Example -->
```ini
searchd {
...
    binlog_flush = 1 # ultimate safety, low write speed
...
}
```
<!-- end -->

### 集群二进制日志支持

<!-- Example binlog_cluster -->
在使用 Galera 的集群设置中，节点恢复行为至关重要。通常，如果节点已经干净关闭并且最后的序列号（seqno）已正确保存，Galera 会通过 IST（增量状态传输）来处理节点不同步的情况。然而，在崩溃并且 seqno 未被保存的情况下，Galera 会触发 SST（状态快照传输），这非常耗费资源，并且由于高 I/O 活动可能显著拖慢集群速度。

为了解决这一问题，引入了集群二进制日志支持。此功能扩展了现有的二进制日志功能，帮助减少 SST 的需求，允许恢复节点从本地二进制日志中重放缺失的事务，并以有效的 seqno 重新加入集群。

对于任何集群操作，集群二进制日志默认启用。不过，您可以通过设置环境变量来禁用它：

<!-- request binlog_cluster -->
```bash
MANTICORE_REPLICATION_BINLOG=0
```
<!-- end -->

该功能通过将二进制日志的本地持久性与 Galera 分布式同步能力相结合，减少了停机时间并避免了完整数据传输。

### 恢复

在非正常关闭后的恢复过程中，会重放二进制日志，并恢复自最后一次良好磁盘状态以来记录的所有事务。事务带有校验和，因此如果二进制日志文件损坏，垃圾数据将**不会**被重放；此类损坏事务将被检测到并停止重放。

### 刷写 RT RAM 块

<!-- Example rt_flush_period -->
针对完全适合 RAM 块的小型 RT 表进行密集更新，可能导致二进制日志不断增长，且直到干净关闭之前永远无法被取消链接。二进制日志本质上是对磁盘上最后已知良好保存状态的追加式增量，除非 RAM 块被保存，否则二进制日志不能被取消链接。不断增长的二进制日志对于磁盘使用和崩溃恢复时间都不是理想的。为了解决此问题，可以使用 `rt_flush_period` 指令配置 `searchd` 定期执行 RAM 块刷写。启用定期刷写后，`searchd` 将维护一个单独的线程，检查是否需要将 RT 表 RAM 块写回磁盘。一旦发生该操作，相应的二进制日志可以（且会）被安全地取消链接。

默认的 RT 刷写周期设置为 10 小时。

<!-- request Example -->
```ini
searchd {
...
    rt_flush_period = 3600 # 1 hour
...
}
```
<!-- end -->

需要注意的是，`rt_flush_period` 仅控制检查的频率。并不保证特定的 RAM 块一定会被保存。例如，定期重新保存只接收少量行更新的大型 RAM 块是没有意义的。Manticore 会使用一些启发式方法自动确定是否执行刷写。

<!-- proofread -->

