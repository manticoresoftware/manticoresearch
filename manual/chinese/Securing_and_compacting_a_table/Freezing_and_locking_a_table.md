# 冻结和锁定表

“冻结”表在你想制作物理副本或备份时很有用。它会将表文件标记为已冻结，并显示这些文件的存放位置。冻结后，你可以安全地将这些文件复制到其他地方。被冻结的表仍然可以继续添加新文档，直到达到 `rt_mem_limit`，但这些新数据只会留在内存中，在表解冻之前不会写入磁盘。如果表超过了 `rt_mem_limit`，所有更改都会暂停，直到表被解冻。如果守护进程意外停止，任何未保存的数据都会从 binlog 中恢复。

“锁定”表适用于逻辑备份。它不会停止内部维护任务，例如磁盘块合并操作或将 RAM 块写入磁盘。它只会阻止写入操作。这意味着你不能向已锁定的表中插入、替换或更新数据。这对 `mysqldump` 之类的工具很有帮助，因为锁定可以确保数据在逻辑上保持一致。比如说，如果在转储期间替换了一条文档记录，在没有锁的情况下，旧版本可能已经出现在转储里，而新版本也可能随后出现，而且两者的文档 ID 相同。锁定表可以避免这种情况。


## 冻结表
<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 会让一个实时/普通表为安全的 [备份](../Securing_and_compacting_a_table/Backup_and_restore.md) 做好准备。具体来说，它会：
1. 停用表压缩。如果表当前正在压缩，`FREEZE` 会优雅地中断它。
2. 将当前 RAM 块转移为磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表的锁定计数器。
6. 显示与该表关联的实际文件列表。

如果表已经处于冻结（锁定）状态，`FREEZE` 会：
1. 增加表的锁定计数器。
2. 显示与该表关联的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。如果你想创建自己的备份方案，或者出于其他原因需要冻结表，也可以照做。只需按以下步骤执行：
1. 冻结一张表（或几张）。
2. 捕获 `FREEZE` 命令的输出，并备份指定的文件。
3. 完成后对表执行 `UNFREEZE`。

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

`file` 列表示运行中的实例在 [data_dir](../Server_settings/Searchd.md#data_dir) 内对应的表文件路径。`normalized` 列显示这些文件的绝对路径。要备份表，只需直接复制提供的文件，无需额外准备。

当表被冻结时，你不能执行 `UPDATE` 查询；它们会一直等待到表解冻为止。

另外，在表被冻结期间，`DELETE` 和 `REPLACE` 查询也有一些限制：
* 如果 `DELETE` 作用于当前 RAM 块中的文档，则允许执行。
* 如果 `DELETE` 影响的是磁盘块中的文档，但该文档之前已经被删除，则允许执行。
* 如果 `DELETE` 会修改实际的磁盘块，则会等待直到表被解冻。

手动对冻结表的 RAM 块执行 `FLUSH` 会返回“success”，但实际上不会发生真正的保存。

冻结表上的 `DROP`/`TRUNCATE` **是**允许的，因为这些操作不是隐式的。我们假设如果你要截断或删除一张表，就不需要为它做备份；因此，它本来就不应该被冻结。

向冻结表执行 `INSERT` 是支持的，但有上限：新数据会像平常一样先存入 RAM，直到达到 `rt_mem_limit`；之后新的插入会等待，直到表解冻。

如果你在有冻结表的情况下关闭守护进程，它的行为会像经历了一次非正常关机（例如 `kill -9`）：新插入的数据不会被保存到磁盘上的 RAM 块中，重启后会从二进制日志中恢复（如果有的话），否则就会丢失（如果禁用了二进制日志）。

## 解冻表

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令会减少表的锁定计数器；当它降到零时，会重新激活此前被阻塞的操作，并恢复内部压缩服务。任何正在等待表解冻的操作也会恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

## 检查表的锁定状态

<!-- example show_table_status -->

你可以使用 `SHOW table_name STATUS` 来检查表是否被冻结。

锁定计数器会在表状态的 `locked` 列中显示。值为零表示表未被冻结，而非零值表示当前活动锁的数量。每一次显式的 `FREEZE` 命令，以及每一次隐式锁定（例如表属于一个集群，而复制流程将其选为副本的 donor）都会增加这个计数器。每一次 `UNFREEZE` 命令都会减少该计数器，最终降到零。

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

你可以在 SQL 客户端会话中显式锁定表，以便与其他会话协调访问，或者在你需要独占访问时阻止其他会话修改数据。一个会话只能为自己锁定或解锁表，不能代替其他会话进行锁定或解锁。锁只能在通过 MySQL 协议连接的 SQL 会话中使用。表被锁定时，任何协议都不能修改其数据（SQL / HTTP / binary）。锁也不适用于属于复制集群的表。


Manticore 只支持 **读（共享）** 锁，不支持 **写（独占）** 锁。

当会话请求读锁时，Manticore 会：
1. 检查连接是否使用 MySQL 协议。
2. 检查该表是否可以被锁定。它必须是本地的实时表或 percolate 表，并且不能属于复制集群。
3. 自动释放该会话已经持有的任何锁。
4. 等待该表上所有正在进行的 insert、replace、update 或 delete 操作完成。
5. 增加表的读锁计数器（参见 [SHOW LOCKS](../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS)）。

任何修改语句（DML），例如 `INSERT`、`REPLACE`、`UPDATE`、`DELETE` 和 `TRUNCATE`，都会先检查表是否被读锁锁定。如果是，语句会因 `table is locked` 错误而失败。DDL 语句（例如 `CREATE`、`ALTER`、`DROP`）不受读锁阻止。


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
`UNLOCK TABLES` 命令会显式释放当前 SQL 会话持有的任何表锁。

如果客户端会话的连接终止，无论是正常还是异常，守护进程都会隐式释放该会话持有的所有
表锁。如果客户端重新连接，这些锁就不再生效。

<!-- request Example -->

```sql
UNLOCK TABLES;
```

<!-- end -->


## SHOW LOCKS

<!-- example show_locks -->

`SHOW LOCKS` 命令会列出当前所有已锁定或已冻结的表。
每一行都会显示表类型、表名、锁的类型，以及表示该锁被应用了多少次的计数器。

**锁类型** 可以是：
- `read` — 表受读锁保护。修改语句会失败，直到锁被释放。
- `freeze` — 表处于冻结状态。这会阻止任何会将数据写入磁盘的操作，直到表被解冻。

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
