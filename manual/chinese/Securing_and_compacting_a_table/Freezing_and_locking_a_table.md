# 冻结和锁定表

“冻结”表在您想要创建物理副本或备份时很有用。它将表的文件标记为冻结，并显示它们的存储位置。冻结后，您可以安全地将这些文件复制到其他位置。您仍然可以向冻结的表中添加新文档，直到达到 `rt_mem_limit`，但新数据会保留在内存中，直到表被解冻才会写入磁盘。如果表超过 `rt_mem_limit`，所有更改将暂停，直到表被解冻。如果守护进程意外停止，任何未保存的数据将从二进制日志中恢复。

“锁定”表对于逻辑备份很有用。它不会阻止内部维护任务，例如磁盘块合并操作或将 RAM 块写入磁盘。它仅阻止写操作。这意味着您无法在锁定的表中插入、替换或更新数据。这对于像 `mysqldump` 这样的工具很有帮助，因为锁定可以确保数据保持逻辑一致性。例如，如果没有锁定，在转储期间如果替换了一个文档，旧版本可能已经存在于转储中，而新版本可能稍后也会出现，两者具有相同的文档 ID。锁定表可以防止这种情况。


## 冻结表
<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 为实时/普通表准备安全的 [备份](../Securing_and_compacting_a_table/Backup_and_restore.md)。具体来说，它：
1. 禁用表压缩。如果表当前正在压缩，`FREEZE` 将优雅地中断它。
2. 将当前 RAM 块转移到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表的锁定计数器。
6. 显示与表关联的实际文件列表。

如果表已经冻结（锁定），`FREEZE` 将：
1. 增加表的锁定计数器。
2. 显示与表关联的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。如果您想创建自己的备份解决方案或出于其他原因需要冻结表，也可以这样做。只需按照以下步骤操作：
1. `FREEZE` 一个表（或几个）。
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

`file` 列表示运行实例的 [data_dir](../Server_settings/Searchd.md#data_dir) 中的表文件路径。`normalized` 列显示相同文件的绝对路径。要备份表，只需复制提供的文件，无需额外准备。

当表被冻结时，您无法执行 `UPDATE` 查询；它们将等待表被解冻。

此外，`DELETE` 和 `REPLACE` 查询在表被冻结时有一些限制：
* 如果 `DELETE` 影响当前 RAM 块中的文档 - 允许。
* 如果 `DELETE` 影响磁盘块中的文档但之前已被删除 - 允许。
* 如果 `DELETE` 会修改实际磁盘块 - 将等待直到表被解冻。

手动 `FLUSH` 冻结表的 RAM 块将报告“成功”，但不会发生实际保存。

`DROP`/`TRUNCATE` 冻结表 **是** 允许的，因为这些操作不是隐式的。我们假设如果您截断或删除表，您不需要备份；因此，它最初不应被冻结。

`INSERT` 到冻结表中是支持的，但有限制：新数据将存储在 RAM 中（如常）直到达到 `rt_mem_limit`；然后，新插入将等待直到表被解冻。

如果您关闭带有冻结表的守护进程，它将表现为经历了脏关闭（例如，`kill -9`）：新插入的数据将 **不会** 保存在磁盘上的 RAM 块中，并且在重启后，它将从二进制日志（如果有的话）中恢复或丢失（如果禁用了二进制日志）。

## 解冻表

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令减少表的锁定计数器，如果计数器达到零，将重新激活之前被阻止的操作并恢复内部压缩服务。任何等待表解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

## 检查表的锁定状态

<!-- example show_table_status -->

您可以使用 `SHOW table_name STATUS` 来检查表是否被冻结。

锁定计数器显示在表状态的 `locked` 列下。值为零表示表未被冻结，而非零值表示活动锁定的数量。每个显式的 `FREEZE` 命令和隐式锁定（例如，当表属于集群且复制例程将其选为副本的供体时）都会增加计数器。每个 `UNFREEZE` 命令都会减少计数器，最终降至零。

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

您可以在 SQL 客户端会话中显式锁定表，以与其他会话协调访问，或在需要独占访问时阻止其他会话更改数据。一个会话只能为自己锁定或解锁表。它不能代表其他会话锁定或解锁表。锁只能在通过 MySQL 协议连接的 SQL 会话中使用。当表被锁定时，不允许任何协议更改其数据（SQL / HTTP / 二进制）。锁对于属于复制集群的表也不起作用。


Manticore 仅支持 **读取（共享）** 锁。**写入（独占）** 锁不被支持。

当会话请求读锁时，Manticore：
1. 检查连接是否使用MySQL协议。
2. 检查表是否可以被锁定。它必须是本地实时表或percolate表，并且不能是复制集群的一部分。
3. 自动释放会话当前持有的所有锁。
4. 等待表上所有正在进行的插入、替换、更新或删除操作完成。
5. 增加表的读锁计数器（参见[SHOW LOCKS](../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS)）。

任何修改语句（DML）如 `INSERT`、`REPLACE`、`UPDATE`、`DELETE` 和 `TRUNCATE` 会首先检查表是否被读锁锁定。如果被锁定，语句将失败并返回错误 `table is locked`。DDL 语句（如 `CREATE`、`ALTER`、`DROP`）不会被读锁阻塞。


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
`UNLOCK TABLES` 命令显式释放当前SQL会话持有的所有表锁。

如果客户端会话的连接正常或异常终止，守护进程会隐式释放会话持有的所有表锁。如果客户端重新连接，这些锁将不再生效。
UNLOCK TABLES;

<!-- request Example -->

```sql
## SHOW LOCKS
```

<!-- end -->


`SHOW LOCKS` 命令列出当前被锁定或冻结的所有表。  

<!-- example show_locks -->

每一行显示表类型、名称、锁的类型以及一个计数器，该计数器指示该锁被应用的次数。
**锁类型**可以是：

`read` — 表被读锁保护。在锁释放之前，修改语句将失败。
- `freeze` — 表被冻结。这会阻止任何将数据写入磁盘的操作，直到表被解冻。
- CODE_BLOCK_10

<!-- request Example -->

```sql
SHOW LOCKS;
```

<!-- response Example -->

```sql
+-----------+------+-----------+-----------------+
| Type      | Name | Lock Type | Additional Info |
+-----------+------+-----------+-----------------+
| rt        | a    | read      | Count: 1        |
| percolate | bar  | freeze    | Count: 3        |
| rt        | foo  | freeze    | Count: 2        |
+-----------+------+-----------+-----------------+
3 rows in set (0,01 sec)
```

<!-- end -->
