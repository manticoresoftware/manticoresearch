# 冻结和锁定表

`冻结`表对于物理备份/复制非常有用。它“冻结”属于表的文件，并显示它们的位置。冻结后，您可以安全地备份文件到其他位置。您仍然可以向冻结表插入新文档，直到达到rt_mem_limit，但这些数据将积累在内存中，不会保存到磁盘，直到表被解冻。如果超过rt_mem_limit，更改将暂停，直到表被解冻。如果守护进程意外终止，未保存的数据将从binlog中恢复。

补充的`锁定`表对逻辑备份非常有用。它不会影响当前的内部重组操作，如优化，内存块写入磁盘等。相反，它仅禁止任何修改操作。也就是说，您无法向锁定表插入/替换/更新数据。这对类似mysqldump的工具很有用。通过锁定表，可以确保数据逻辑一致。例如，如果在导出期间替换文档，可能会发生旧版本已经在导出中，而新版本也出现在末尾，且具有相同的文档ID。通过锁定表，您可以确保这种情况不会发生。

## 冻结表
<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 命令使实时/纯表为安全的[备份](../Securing_and_compacting_a_table/Backup_and_restore.md)做好准备。具体来说，它执行：
1. 停用表压缩。如果表当前正在压缩，`FREEZE`会优雅地中断该过程。
2. 将当前的内存块转移到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 表的锁定计数器加一。
6. 显示与表关联的实际文件列表。

如果表已经被冻结（锁定），`FREEZE`将：
1. 表的锁定计数器加一。
2. 显示与表关联的实际文件列表。

内置工具[manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md)使用`FREEZE`确保数据一致性。如果您想自己创建备份解决方案或因其他原因需要冻结表，也可以这样做。只需按照以下步骤：
1. 对一张（或多张）表执行`FREEZE`。
2. 捕获`FREEZE`命令的输出，备份指定的文件。
3. 备份完成后执行`UNFREEZE`解冻表。

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

`file`列指示正在运行的实例的[data_dir](../Server_settings/Searchd.md#data_dir)中表的文件路径。`normalized`列显示相同文件的绝对路径。要备份表，只需复制提供的文件即可，无需额外准备。

表冻结时，无法执行`UPDATE`查询；它们将等待直到表解冻。

此外，冻结表时`DELETE`和`REPLACE`查询有一些限制：
* 如果`DELETE`影响当前内存块中的文档 - 允许。
* 如果`DELETE`影响磁盘块中但之前已删除的文档 - 允许。
* 如果`DELETE`影响实际磁盘块 - 将等待直到表解冻。

手动对冻结表的内存块执行`FLUSH`会报告“成功”，但不会进行实际保存。

冻结表的`DROP`/`TRUNCATE`操作**是**允许的，因为这些操作不是隐式的。我们假设如果您截断或删除表，则不需要备份，因此表最初不应该被冻结。

向冻结表中`INSERT`是支持的，但有限制：新数据将保存在内存中（如常规情况）直到达到`rt_mem_limit`；然后，新的插入将等待直到表解冻。

如果您在表冻结时关闭守护进程，它将表现得像脏关闭（例如，`kill -9`）：新插入的数据**不会**保存在磁盘上的内存块中，重新启动后将从二进制日志（如果有）恢复，否则将丢失（如果禁用二进制日志）。

## 解冻表

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE`命令减少表的锁定计数器，如果计数器达到零，将重新激活之前被阻止的操作，并恢复内部压缩服务。任何等待表解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

## 查看表的锁定状态

<!-- example show_table_status -->

您可以使用`SHOW table_name STATUS`检查表是否被冻结。

锁定计数器显示在表状态的`locked`列中。值为零表示表未冻结，非零值表明有活动锁。每次显式的`FREEZE`命令以及隐式锁定（例如表是集群的一部分且复制例程选择它作为副本的捐赠者）都会增加计数器。每次`UNFREEZE`命令会减少计数器，最终降至零。
SHOW TABLE `foo` STATUS LIKE 'locked';
+---------------+-------+
| Variable_name | Value |

<!-- request Example -->

```sql
+---------------+-------+
```

<!-- response Example -->

```sql
| locked        | 2     |
+---------------+-------+
1 row in set (0,00 sec)
## 表锁获取
lock tables tbl1 read[, tbl2 write, ...]
Manticore允许sphinxql客户端会话显式获取表锁，以便与其他会话协作访问表，或防止其他会话在会话需要独占访问期间修改表。sphinxql会话只能为自己获取或释放锁。一个会话不能为另一个会话获取锁或释放另一个会话持有的锁。锁只能在通过mysql协议连接的sphinxql会话中获取和释放，但您无法通过任何协议修改被锁定表中的数据。此外，锁不可用于集群中的表。因为如果我们拒绝来自集群的对锁定表的修改，整个集群可能会被阻塞甚至损坏。
```

<!-- end -->

Manticore仅实现读取（共享）锁。不支持写入（排他）锁。

<!-- example lock -->

```sql
读锁：
```

1. 检查当前连接协议是否为mysql。

2. 检查表是否适合加锁。它必须是本地 rt 表或 percolate 表。它不能是集群的一部分。

3. 隐式释放当前会话持有的任何表锁。

4. 等待所有当前运行的更改（插入/替换/更新）完成。
5. 增加表的读锁计数器。
修改语句，如 insert/replace/update/delete，首先检查表是否处于读锁状态。若是，将被拒绝并出现错误“table is locked”。
写锁：
1. 检查当前连接协议是否为 mysql。

2. 不检查表是否适合，甚至不检查表是否存在。

3. 隐式释放当前会话持有的任何表锁。

4. 触发警告“Write lock is not implemented.”
LOCK TABLES tbl READ, tbl2 WRITE;
## 表锁释放
UNLOCK TABLES

<!-- request Example -->

```sql
`UNLOCK` 命令显式释放当前 sphinxql 会话持有的任何表锁。
```

<!-- end -->

如果客户端会话的连接终止，无论是正常还是异常，守护进程都会隐式释放该会话持有的所有

<!-- example unlock -->

```sql
表锁。如果客户端重新连接，锁将不再生效。
```

UNLOCK TABLES;

## SHOW LOCKS
使用 `SHOW LOCKS` 命令也可以显示被锁定和冻结的表。锁计数器显示在 `Additional Info` 列。

<!-- request Example -->

```sql
SHOW LOCKS;
```

<!-- end -->


+-----------+------+-----------+-----------------+

<!-- example show_locks -->

| Type      | Name | Lock Type | Additional Info |

<!-- request Example -->

```sql
+-----------+------+-----------+-----------------+
```

<!-- response Example -->

```sql
| rt        | a    | read      | Count: 1        |
| percolate | bar  | freeze    | Count: 3        |
| rt        | foo  | freeze    | Count: 2        |
+-----------+------+-----------+-----------------+
3 rows in set (0,01 sec)
| rt        | foo  | freeze    | Count: 2        |
+-----------+------+-----------+-----------------+
3 rows in set (0,01 sec)
```

<!-- end -->

