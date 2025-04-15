# 二进制日志

二进制日志作为 [实时](../Creating_a_table/Local_tables/Real-time_table.md) 表数据的恢复机制。当启用二进制日志时，`searchd` 记录每个事务到 binlog 文件，并在不正常关闭后利用它进行恢复。在正常关闭期间，RAM 块被保存到磁盘中，所有 binlog 文件随后会被删除。

## 启用和禁用二进制日志

默认情况下，启用二进制日志以保障数据完整性。在 Linux 系统上，[纯模式](../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 的 `binlog.*` 文件默认位置为 `/var/lib/manticore/data/`。在 [RT 模式](../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29) 下，二进制日志存储在 `<data_dir>/binlog/` 文件夹中，除非另有说明。

### 全局二进制日志配置

<!-- example binlog_path -->
要全局禁用二进制日志，请在 `searchd` 配置中将 `binlog_path` 设置为空值。
禁用二进制日志需要重启守护进程，并在系统意外关闭时将数据置于风险之中。

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
为了更细粒度的控制，可以通过将实时表的 `binlog` 表参数设置为 `0` 来禁用每个表的二进制日志。此选项不适用于过滤表。

<!-- request Example -->
```sql
create table a (id bigint, s string attribute) binlog='0';
```
<!-- end -->

<!-- Example binlog_alter -->
对于现有的 RT 表，也可以通过修改 `binlog` 参数来禁用二进制日志。

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
* **对全局设置的依赖**：每表的二进制日志设置仅在全局启用二进制日志的情况下生效（`binlog_path` 不能为空）。
* **二进制日志状态和事务 ID 见解**：修改表的二进制日志状态会强制对表进行立即 [刷新](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE)。如果您为一张表关闭了二进制日志，它的事务 ID (TID) 会变为 `-1`。这表示二进制日志未激活，且没有跟踪任何更改。相反，如果您为一张表启用了二进制日志，它的事务 ID 将变为非负数（零或更高）。这表示该表的更改现在正在被记录。您可以使用命令 `SHOW TABLE <name> STATUS` 来检查事务 ID。该事务 ID 反映出是否正在记录对该表的更改（非负数）或未记录（`-1`）。

## 操作

当启用二进制日志时，对 RT 表的每个更改都将保存到日志文件中。如果系统意外关闭，这些日志将在系统重新启动时自动使用，以恢复所有已记录的更改。

### 日志大小

<!-- Example binlog_max_log_size -->
在正常操作期间，当记录的数据量达到某个限制（由 `binlog_max_log_size` 设置）时，将开始一个新的日志文件。旧的日志文件在其中的所有更改完全处理并保存为磁盘块之前会被保留。如果此限制设置为 `0`，日志文件会在系统正常关闭之前保留。默认情况下，这些文件的大小没有限制。

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

每个 binlog 文件的名称由一个零填充的数字组成，如 `binlog.0000`, `binlog.0001` 等，通常显示四位数字。您可以通过设置 `binlog_filename_digits` 更改数字的位数。如果您的 binlog 文件数量超过了数字可以容纳的位数，位数将自动增加以适应所有文件。

**重要**：要更改位数，您必须首先保存所有表数据并正常关闭系统。然后，删除旧的日志文件并重启系统。

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
您可以选择两种方式来管理二进制日志文件，可以通过 `binlog_common` 指令进行设置：
* 每个表单独文件（默认，`0`）：每个表将其更改保存在自己的日志文件中。如果您有许多在不同时间更新的表，这种设置是好的。这样允许表在不等待其他表的情况下进行更新。此外，如果某个表的日志文件出现问题，它不会影响其他表。
* 所有表共享单个文件（`1`）：所有表使用相同的二进制日志文件。这种方法使文件处理变得更加简单，因为文件数量较少。然而，如果某个表仍需要保存其更新，可能会造成文件保留时间过长。如果许多表需要同时更新，所有更改也必须等待写入同一文件，这可能减慢速度。

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
有四种不同的 binlog 刷新策略，受 `binlog_flush` 指令控制：

* `0` - 数据每秒写入磁盘（刷新），并且 Manticore 在刷新后立即启动对磁盘的安全处理（[同步](https://linux.die.net/man/8/sync)）。此方法最快，但如果服务器或计算机突然崩溃，可能会丢失一些尚未安全处理的最近写入数据。
* `1` - 数据在每个事务后写入 binlog 并立即同步。此方法是最安全的，因为它确保每个更改都立即被保存，但会降低写入速度。
* `2` - 数据在每个事务后写入，并每秒启动一次同步。此方法提供了一种平衡，定期快速地写入数据。然而，如果计算机故障，则可能有部分正在被保存的数据未完成保存。此外，根据磁盘的不同，同步可能会超过一秒钟。
* `3` - 类似于 `2`，但还确保在超出 `binlog_max_log_size` 后关闭之前同步 binlog 文件。

默认模式为 `2`，它在每个事务后写入数据并每秒开始同步，平衡了速度和安全性。

<!-- request Example -->
```ini
searchd {
...
    binlog_flush = 1 # 终极安全，低写入速度
...
}
```
<!-- end -->

### 恢复

在不干净的关闭后进行恢复时，binlogs 会被重放，所有自上一个良好的磁盘状态以来记录的事务都会被恢复。事务会进行校验和，因此在 binlog 文件损坏的情况下，垃圾数据将 **不会** 被重放；这样的损坏事务将被检测到并停止重放。

### 刷新 RT RAM 块

<!-- Example rt_flush_period -->
对于一个完全适合 RAM 块的小型 RT 表的密集更新可能导致不断增长的 binlog，直到干净关闭才能解除链接。Binlogs 本质上作为对最后已知良好保存状态的追加增量，不能解除链接，除非 RAM 块被保存。不断增长的 binlog 并不理想，会影响磁盘使用和崩溃恢复时间。为了解决这个问题，您可以配置 `searchd` 使用 `rt_flush_period` 指令执行周期性 RAM 块刷新。启用周期性刷新后，`searchd` 将维护一个单独的线程，检查 RT 表的 RAM 块是否需要写回磁盘。一旦发生这种情况，相应的 binlogs 可以（并且会）安全解除链接。

默认的 RT 刷新周期设置为 10 小时。

<!-- request Example -->
```ini
searchd {
...
    rt_flush_period = 3600 # 1 小时
...
}
```
<!-- end -->

需要注意的是 `rt_flush_period` 仅控制检查发生的频率。并不能保证特定的 RAM 块会被保存。例如，定期重新保存只接收几行更新的大 RAM 块并没有意义。Manticore 会自动根据一些启发式方法决定是否执行刷新。

<!-- proofread -->

