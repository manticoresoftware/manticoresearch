# 二进制日志

二进制日志作为[实时](../Creating_a_table/Local_tables/Real-time_table.md)表数据的恢复机制。当启用二进制日志时，`searchd` 会将每个事务记录到 binlog 文件中，并在非正常关闭后利用它进行恢复。在正常关闭期间，RAM 块会保存到磁盘，所有 binlog 文件随后都会被删除。

## 启用和禁用二进制日志

默认情况下，为了保障数据完整性，二进制日志是启用的。在 Linux 系统中，`binlog.*` 文件的默认位置在[Plain 模式](../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-(Plain-mode))下为 `/var/lib/manticore/data/`，在[RT 模式](../Creating_a_table/Local_tables.md#Online-schema-management-(RT-mode))下，二进制日志存储在 `<data_dir>/binlog/` 文件夹中，除非另有指定。

### 全局二进制日志配置

<!-- example binlog_path -->
要全局禁用二进制日志，可以在 `searchd` 配置中将 `binlog_path` 设置为空值。禁用二进制日志需要重新启动守护进程，并且在系统意外关闭时会使数据面临风险。

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

### 按表级别的二进制日志配置

<!-- Example binlog0 -->
为了更细粒度的控制，您可以通过将 `binlog` 表参数设置为 `0`，禁用实时表的二进制日志记录。此选项不适用于渗透表。

<!-- request Example -->
```sql
create table a (id bigint, s string attribute) binlog='0';
```
<!-- end -->

<!-- Example binlog_alter -->
对于现有的 RT 表，您也可以通过修改 `binlog` 参数禁用二进制日志。

<!-- request Example -->
```sql
alter table FOO binlog='0';
```
<!-- end -->

<!-- Example binlog_alter2 -->
如果之前已禁用二进制日志，您可以通过将 `binlog` 参数设置为 `1` 来重新启用它：

<!-- request Example -->
```sql
alter table FOO binlog='1';
```
<!-- end -->

#### 重要注意事项：

- **依赖全局设置**：按表级别的二进制日志设置仅在全局启用二进制日志时生效（`binlog_path` 必须不为空）。
- **二进制日志状态和事务 ID 洞察**：修改表的二进制日志状态会强制立即[刷新表](../Securing_and_compacting_a_table/Flushing_RAM_chunk_to_disk.md#FLUSH-TABLE)。如果为表关闭了二进制日志，其事务 ID (TID) 会变为 `-1`，表示没有活动的二进制日志记录；如果重新启用了二进制日志，则事务 ID 将变为非负数（零或更大），表示正在记录表的更改。您可以使用 `SHOW TABLE <name> STATUS` 命令检查事务 ID。

## 操作

启用二进制日志时，对 RT 表的每次更改都会保存到日志文件中。如果系统意外关闭，这些日志将在系统重新启动时自动重放，以恢复所有已记录的更改。

### 日志大小

<!-- Example binlog_max_log_size -->
在正常操作期间，当记录的数据达到某个限制（通过 `binlog_max_log_size` 设置）时，将开始一个新的日志文件。旧的日志文件会一直保留，直到其中的所有更改都完全处理并保存到磁盘为止。如果该限制设置为 `0`，日志文件将保留到系统正常关闭为止。默认情况下，这些文件的大小没有限制。

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

每个 binlog 文件的命名格式为零填充的数字，如 `binlog.0000`，`binlog.0001` 等，通常显示四位数。您可以通过 `binlog_filename_digits` 设置更改数字的位数。如果 binlog 文件数量超过位数能够表示的范围，位数将自动增加以容纳所有文件。

**重要**：要更改位数，必须首先保存所有表数据并正确关闭系统。然后删除旧日志文件并重新启动系统。

<!-- request Example -->
```ini
searchd {
...
    binlog_filename_digits = 6
...
```
<!-- end -->

### 二进制日志管理策略

<!-- Example binlog_common -->

您可以通过 `binlog_common` 指令选择两种管理二进制日志文件的方式：

- 每个表单独保存日志文件（默认，`0`）：每个表将更改保存到其各自的日志文件中。如果表的更新频率不同，这种设置非常适合，因为它允许表之间独立更新，不会相互等待。如果某个表的日志文件出现问题，不会影响其他表。
- 所有表使用一个日志文件（`1`）：所有表将更改记录到同一个二进制日志文件中。这种方法有助于减少日志文件数量，但可能会因为一个表的更新而延长文件的保留时间。此设置也可能在多个表同时更新时导致速度变慢，因为所有更改都需要写入一个文件。

<!-- request binlog_common -->

```ini
searchd {
...
    binlog_common = 1
...
```
<!-- end -->

### 二进制日志刷新策略

<!-- Example binlog_flush -->

二进制日志有四种不同的刷新策略，可以通过 `binlog_flush` 指令控制：

- `0` - 数据每秒写入磁盘（刷新），Manticore 立即启动安全保存操作（[同步](https://linux.die.net/man/8/sync)）后刷新。这是最快的方式，但如果服务器或计算机突然崩溃，某些未安全保存的最近写入数据可能会丢失。
- `1` - 每次事务后立即将数据写入 binlog 并同步。这是最安全的方法，确保每次更改立即保存，但会减慢写入速度。
- `2` - 每次事务后写入数据，并且每秒启动一次同步。这种方法提供了速度和安全性的平衡，但如果计算机故障，某些数据可能尚未完成保存。同步时间可能会超过一秒，具体取决于磁盘性能。
- `3` - 类似于 `2`，但在超过 `binlog_max_log_size` 关闭 binlog 文件前，确保其已同步。

默认模式是 `2`，它在每次事务后写入数据，并每秒同步，平衡了速度和安全性。

<!-- request Example -->
```ini
searchd {
...
    binlog_flush = 1 # ultimate safety, low write speed
...
}
```
<!-- end -->

### 恢复

在非正常关闭后恢复期间，binlog 会被重放，所有从上一次已保存的磁盘状态起记录的事务将被恢复。事务有校验和，因此如果 binlog 文件损坏，垃圾数据将**不会**被重放；损坏的事务将被检测到，并会停止重放。

### 刷新 RT RAM 块

<!-- Example rt_flush_period -->

对完全适合 RAM 块的小 RT 表进行密集更新可能导致不断增长的 binlog，直到干净关闭前无法取消链接。Binlog 本质上充当最后一次已保存磁盘状态的附加增量，除非 RAM 块被保存，否则不能取消链接。不断增长的 binlog 对磁盘使用和崩溃恢复时间不利。为了解决这个问题，您可以通过 `rt_flush_period` 指令配置 `searchd` 定期刷新 RAM 块。启用定期刷新后，`searchd` 将维护一个单独的线程，检查是否需要将 RT 表 RAM 块写回磁盘。一旦完成，相应的 binlog 文件可以安全取消链接。

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

需要注意的是，`rt_flush_period` 仅控制检查发生的频率，不能保证特定的 RAM 块会被保存。例如，频繁重新保存仅收到几行更新的大型 RAM 块没有意义。Manticore 会根据一些启发式方法自动确定是否执行刷新。

<!-- proofread -->
