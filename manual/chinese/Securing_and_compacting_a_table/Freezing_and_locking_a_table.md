# 冻结和锁定一个表

“冻结”一个表在你想要创建物理副本或备份时非常有用。它会标记表的文件为冻结状态，并显示它们的位置。冻结后，你可以安全地将这些文件复制到其他地方。你仍然可以在冻结的表中添加新文档，直到达到 `rt_mem_limit`，但新的数据会保留在内存中，直到表解冻后才会写入磁盘。如果表超过 `rt_mem_limit`，所有更改都会暂停，直到表被解冻。如果守护进程意外停止，任何未保存的数据将从二进制日志中恢复。

“锁定”一个表对于逻辑备份很有用。它不会阻止内部维护任务，例如磁盘块合并操作或将RAM块写入磁盘。它只会阻止写操作。这意味着你不能向锁定的表插入、替换或更新数据。这对于像 `mysqldump` 这样的工具来说是有帮助的，因为锁定确保了数据保持逻辑一致性。例如，在导出过程中如果替换了一个文档，旧版本可能已经包含在导出中，而新版本也可能稍后出现，两者都有相同的文档ID。锁定表可以防止这种情况发生。


## 冻结一个表
<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 准备实时/普通表进行安全的[备份](../Securing_and_compacting_a_table/Backup_and_restore.md)。具体来说，它：
1. 禁用表压缩。如果表当前正在进行压缩，`FREEZE` 将优雅地中止它。
2. 将当前RAM块转移到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表的锁定计数器。
6. 显示与表关联的实际文件列表。

如果表已经冻结（锁定），`FREEZE` 将：
1. 增加表的锁定计数器。
2. 显示与表关联的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。如果你想创建自己的备份解决方案或需要冻结表的其他原因，只需遵循以下步骤：
1. 冻结一个表（或几个）。
2. 捕获 `FREEZE` 命令的输出并备份指定的文件。
3. 完成后解冻表。

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

`file` 列指示表在运行实例的 [data_dir](../Server_settings/Searchd.md#data_dir) 中的文件路径。`normalized` 列显示相同文件的绝对路径。要备份一个表，只需复制提供的文件即可，无需额外准备。

当表被冻结时，你不能执行 `UPDATE` 查询；它们会等待直到表被解冻。

此外，当表被冻结时，`DELETE` 和 `REPLACE` 查询有一些限制：
* 如果 `DELETE` 影响当前RAM块中的文档 - 允许。
* 如果 `DELETE` 影响之前已删除的磁盘块中的文档 - 允许。
* 如果 `DELETE` 会影响实际磁盘块 - 将等待直到表被解冻。

手动刷新冻结表的RAM块将报告成功，但实际上没有真正的保存发生。

冻结表的 `DROP`/`TRUNCATE` **是**允许的，因为这些操作不是隐式的。我们假设如果你截断或删除一个表，你不需要对其进行备份；因此，它最初不应该被冻结。

向冻结的表插入数据是支持的，但有限制：新数据将存储在RAM中（如常），直到达到 `rt_mem_limit`；然后，新的插入将等待直到表被解冻。

如果你关闭带有冻结表的守护进程，它将像经历了脏关机一样行为（例如，`kill -9`）：新插入的数据将 **不** 被保存到磁盘上的RAM块上，并且在重启时将从二进制日志中恢复（如果有）或丢失（如果禁用了二进制日志记录）。

## 解冻一个表

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令减少表的锁定计数器，如果计数器降至零，则重新激活之前被阻塞的操作并恢复内部压缩服务。任何等待表解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

## 检查表的锁定状态

<!-- example show_table_status -->

你可以使用 `SHOW table_name STATUS` 来检查表是否被冻结。

锁定计数器在表的状态中显示在 `locked` 列下。值为零表示表未冻结，非零值表示活动锁定的数量。每个显式的 `FREEZE` 命令和隐式锁定（例如，当表属于集群并且复制过程将其选为副本供体时）都会增加计数器。每次 `UNFREEZE` 命令都会减少计数器，最终降至零。

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

## 锁定一个表

<!-- example lock -->

```sql
lock tables tbl1 read[, tbl2 read, ...]
```

你可以在SQL客户端会话中显式锁定表以协调与其他会话的访问或阻止其他会话在你需要独占访问时更改数据。一个会话只能为自己锁定或解锁表。它不能代表另一个会话锁定或解锁表。锁仅在通过MySQL协议连接的SQL会话中使用。当表被锁定时，不允许任何协议更改其数据（SQL / HTTP /二进制）。锁也不适用于属于复制集群的表。


Manticore 只支持 **读（共享）** 锁。 **写（独占）** 锁不支持。

当会话请求读锁时，Manticore：
1. 检查连接是否使用 MySQL 协议。
2. 检查表是否可以被锁定。它必须是本地的实时或 percolate 表，且不能是复制集群的一部分。
3. 自动释放会话已持有的任何锁。
4. 等待表上所有正在进行的插入、替换、更新或删除操作完成。
5. 增加表的读取锁计数器（参见 [SHOW LOCKS](../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS)）。

任何修改语句（DML），如 `INSERT`、`REPLACE`、`UPDATE`、`DELETE` 和 `TRUNCATE`，首先会检查表是否被读取锁定。如果是，语句将以错误 `table is locked` 失败。DDL 语句（例如 `CREATE`、`ALTER`、`DROP`）不会被读取锁阻止。


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

如果客户端会话的连接终止，无论是正常还是异常，守护进程都会隐式释放该会话持有的所有
表锁。如果客户端重新连接，锁将不再生效。

<!-- request Example -->

```sql
UNLOCK TABLES;
```

<!-- end -->


## SHOW LOCKS

<!-- example show_locks -->

`SHOW LOCKS` 命令列出当前被锁定或冻结的所有表。  
每一行显示表类型、表名、锁的类型以及指示该锁被应用次数的计数器。

**锁类型** 可以是：
- `read` — 表被读取锁保护。修改语句将在锁释放前失败。
- `freeze` — 表被冻结。这会阻止任何写入磁盘数据的操作，直到表被解冻。

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
