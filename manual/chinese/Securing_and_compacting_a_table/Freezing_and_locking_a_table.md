# 冻结和锁定表

“冻结”表在您想制作物理副本或备份时非常有用。它将表的文件标记为冻结状态，并显示它们的存储位置。冻结后，您可以安全地将这些文件复制到其他地方。您仍然可以向冻结的表中添加新文档，直到达到 `rt_mem_limit`，但新数据会保存在内存中，直到表被解冻之前不会写入磁盘。如果表超过了 `rt_mem_limit`，所有更改将暂停，直到表被解冻。如果守护进程意外停止，任何未保存的数据将从二进制日志中恢复。

“锁定”表对逻辑备份很有用。它不会停止内部维护任务，例如磁盘块合并操作或将 RAM 块写入磁盘。它只阻止写操作。这意味着您不能在锁定的表中插入、替换或更新数据。这对像 `mysqldump` 这样的工具很有帮助，因为锁定确保数据保持逻辑一致。例如，如果没有锁定，在转储期间替换文档，旧版本可能已经在转储中，而新版本也可能稍后出现，且两者具有相同的文档 ID。锁定表可以防止这种情况。


## 冻结表
<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 使实时/普通表准备好进行安全的[备份](../Securing_and_compacting_a_table/Backup_and_restore.md)。具体来说，它：
1. 停用表压缩。如果表当前正在压缩，`FREEZE` 会优雅地中断它。
2. 将当前的 RAM 块转移到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表的锁定计数器。
6. 显示与表关联的实际文件列表。

如果表已经被冻结（锁定），`FREEZE` 将：
1. 增加表的锁定计数器。
2. 显示与表关联的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。如果您想创建自己的备份解决方案或因其他原因需要冻结表，也可以这样做。只需按照以下步骤：
1. `FREEZE` 一个或多个表。
2. 捕获 `FREEZE` 命令的输出并备份指定的文件。
3. 完成后 `UNFREEZE` 表。

<!-- request Example -->
```sql
FREEZE t;
```

<!-- response Example -->
```sql
+-------------------+---------------------------------+
| file              | normalized                      |
+-------------------+---------------------------------+
| data/t/t.0.spa    | /work/anytest/data/t/t.0.spa    |
| data/t/t.0.spd    | /work/anytest/data/t/t.0.spd    |
| data/t/t.0.spds   | /work/anytest/data/t/t.0.spds   |
| data/t/t.0.spe    | /work/anytest/data/t/t.0.spe    |
| data/t/t.0.sph    | /work/anytest/data/t/t.0.sph    |
| data/t/t.0.sphi   | /work/anytest/data/t/t.0.sphi   |
| data/t/t.0.spi    | /work/anytest/data/t/t.0.spi    |
| data/t/t.0.spm    | /work/anytest/data/t/t.0.spm    |
| data/t/t.0.spp    | /work/anytest/data/t/t.0.spp    |
| data/t/t.0.spt    | /work/anytest/data/t/t.0.spt    |
| data/t/t.meta     | /work/anytest/data/t/t.meta     |
| data/t/t.ram      | /work/anytest/data/t/t.ram      |
| data/t/t.settings | /work/anytest/data/t/t.settings |
+-------------------+---------------------------------+
13 rows in set (0.01 sec)
```

<!-- end -->

`file` 列指示运行实例的 [data_dir](../Server_settings/Searchd.md#data_dir) 中表的文件路径。`normalized` 列显示相同文件的绝对路径。备份表时，只需复制提供的文件，无需额外准备。

当表被冻结时，您无法执行 `UPDATE` 查询；它们将等待直到表被解冻。

此外，`DELETE` 和 `REPLACE` 查询在表冻结时有一些限制：
* 如果 `DELETE` 影响当前 RAM 块中的文档 - 允许执行。
* 如果 `DELETE` 影响磁盘块中的文档但之前已被删除 - 允许执行。
* 如果 `DELETE` 会更改实际的磁盘块 - 将等待直到表被解冻。

手动对冻结表的 RAM 块执行 `FLUSH` 会报告“成功”，但不会真正保存。

冻结表的 `DROP`/`TRUNCATE` 是允许的，因为这些操作不是隐式的。我们假设如果您截断或删除表，您不需要备份它；因此，表最初不应被冻结。

向冻结表中 `INSERT` 是支持的，但有限制：新数据将存储在 RAM 中（如常规）直到达到 `rt_mem_limit`；然后，新的插入将等待直到表被解冻。

如果您在表冻结时关闭守护进程，它将表现得像经历了脏关闭（例如，`kill -9`）：新插入的数据不会保存在磁盘上的 RAM 块中，重启时将从二进制日志（如果有）恢复，或者如果禁用二进制日志则丢失。

## 解冻表

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令减少表的锁定计数器，如果计数器降至零，则重新激活之前被阻止的操作并恢复内部压缩服务。所有等待表解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

## 检查表的锁定状态

<!-- example show_table_status -->

您可以使用 `SHOW table_name STATUS` 来检查表是否被冻结。

锁定计数器显示在表状态的 `locked` 列中。值为零表示表未被冻结，非零值表示活动锁的数量。每个显式的 `FREEZE` 命令和隐式锁定（例如当表是集群的一部分且复制例程选择它作为副本的捐赠者）都会增加计数器。每个 `UNFREEZE` 命令都会减少计数器，最终降至零。

<!-- request Example -->

```sql
SHOW TABLE `foo` STATUS LIKE 'locked';
```

<!-- response Example -->

```sql
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| locked        | 2     |
+---------------+-------+
1 row in set (0,00 sec)
```

<!-- end -->

## 锁定表

<!-- example lock -->

```sql
lock tables tbl1 read[, tbl2 read, ...]
```

您可以在 SQL 客户端会话中显式锁定表，以协调与其他会话的访问，或在需要独占访问时阻止其他会话更改数据。会话只能为自己锁定或解锁表，不能代表其他会话锁定或解锁表。锁只能用于通过 MySQL 协议连接的 SQL 会话。表被锁定时，不允许任何协议更改其数据（SQL / HTTP / 二进制）。锁也不适用于属于复制集群的表。


Manticore 仅支持**读（共享）**锁。不支持**写（独占）**锁。

当会话请求读锁时，Manticore：
1. 检查连接是否使用 MySQL 协议。
2. 检查表是否可以被锁定。它必须是本地的实时或 percolate 表，且不能是复制集群的一部分。
3. 自动释放会话已持有的任何锁。
4. 等待表上所有正在进行的插入、替换、更新或删除操作完成。
5. 增加表的读锁计数器（参见 [SHOW LOCKS](../Freezing_and_locking_a_table.md#SHOW-LOCKS)）。

任何修改语句（插入/替换/更新/删除）首先检查表是否被读锁定。如果是，语句将失败并返回错误 `table is locked`。


<!-- request Example -->

```sql
--------------
LOCK TABLES tbl READ
--------------

Query OK, 0 rows affected (0.000 sec)

--------------
LOCK TABLES tbl READ, tbl2 WRITE
--------------

Query OK, 0 rows affected, 1 warning (0.000 sec)

--------------
SHOW WARNINGS
--------------

+---------+------+--------------------------------+
| Level   | Code | Message                        |
+---------+------+--------------------------------+
| warning | 1000 | Write lock is not implemented. |
+---------+------+--------------------------------+
1 row in set (0.000 sec)
```

<!-- end -->

## UNLOCK TABLES

<!-- example unlock -->
`UNLOCK TABLES` 命令显式释放当前 SQL 会话持有的任何表锁。

如果客户端会话的连接终止，无论是正常还是异常，守护进程都会隐式释放该会话持有的所有表锁。如果客户端重新连接，这些锁将不再生效。
UNLOCK TABLES;

<!-- request Example -->

```sql
## SHOW LOCKS
```

<!-- end -->


`SHOW LOCKS` 命令列出当前被锁定或冻结的所有表。

<!-- example show_locks -->

每行显示表类型、表名、锁的类型以及一个计数器，指示该锁被应用的次数。
**锁类型** 可以是：

- `read` — 表被读锁保护。在锁释放之前，修改语句将失败。
- `freeze` — 表被冻结。这会阻止任何将数据写入磁盘的操作，直到表被解冻。
SHOW LOCKS;

<!-- request Example -->

```sql
+-----------+------+-----------+-----------------+
```

<!-- response Example -->

```sql
| Type      | Name | Lock Type | Additional Info |
+-----------+------+-----------+-----------------+
| rt        | a    | read      | Count: 1        |
| percolate | bar  | freeze    | Count: 3        |
| rt        | foo  | freeze    | Count: 2        |
+-----------+------+-----------+-----------------+
3 rows in set (0,01 sec)
3 rows in set (0,01 sec)
```

<!-- end -->
