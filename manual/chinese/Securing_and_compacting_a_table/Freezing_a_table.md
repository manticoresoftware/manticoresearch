# 冻结表

<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 使实时/普通表准备好进行安全的[备份](../Securing_and_compacting_a_table/Backup_and_restore.md)。具体来说，它：
1. 停用表的压缩。如果表当前正在压缩，`FREEZE` 将优雅地中断它。
2. 将当前的 RAM 块转移到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表的锁定计数器。
6. 显示与表相关的实际文件列表。

如果表已经被冻结（锁定），`FREEZE` 将：
1. 增加表的锁定计数器。
2. 显示与表相关的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。如果您想创建自己的备份方案或因其他原因需要冻结表，也可以这样做。只需按照以下步骤：
1. `FREEZE` 一个表（或多个）。
2. 捕获 `FREEZE` 命令的输出，并备份指定的文件。
3. 备份完成后 `UNFREEZE` 表。

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

`file` 列显示运行实例的 [data_dir](../Server_settings/Searchd.md#data_dir) 中表的文件路径。`normalized` 列显示相同文件的绝对路径。要备份表，只需复制提供的文件，无需额外准备。

当表被冻结时，不能执行 `UPDATE` 查询；它们将等待表解冻。

此外，`DELETE` 和 `REPLACE` 查询在表冻结期间有一些限制：
* 如果 `DELETE` 影响当前 RAM 块中的文档 —— 允许。
* 如果 `DELETE` 影响磁盘块中的文档但该文档之前已被删除 —— 允许。
* 如果 `DELETE` 会修改实际磁盘块 —— 将等待表解冻。

手动对冻结表的 RAM 块执行 `FLUSH` 会报告成功，但不会真正保存数据。

冻结表的 `DROP`/`TRUNCATE` 操作是允许的，因为这些操作不是隐式的。我们假设如果你截断或删除了表，就不需要备份它；因此，它本不该被冻结。

向冻结表中 `INSERT` 是支持但有限制的：新数据将存储在 RAM（如常）中，直到达到 `rt_mem_limit`；此后，新插入操作将等待表解冻。

如果您在表被冻结时关闭守护进程，它将表现得像发生了异常关闭（例如 `kill -9`）：新插入的数据不会保存在磁盘上的 RAM 块中，重启时将从二进制日志（如果存在）恢复，否则会丢失（如果禁用了二进制日志）。

## 解冻表

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令降低表的锁定计数器，当计数器降至零时，重新激活之前被阻止的操作并恢复内部的压缩服务。所有等待解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

## 检查表的锁定状态

<!-- example show_table_status -->

您可以使用 `SHOW table_name STATUS` 来检查表是否被冻结。

表状态中的 `locked` 列显示锁定计数器。值为零表示表未冻结，非零值表示当前的锁定数。每个显式的 `FREEZE` 命令以及隐式锁定（例如表是集群的一部分，复制例程选择它作为副本的捐赠者）都会增加计数器。每个 `UNFREEZE` 命令都会减少计数器，直到降为零。

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

## SHOW LOCKS

<!-- example show_locks -->

锁定的表也会显示在 `SHOW LOCKS` 命令中。锁计数器显示在 `Additional Info` 列。

<!-- request Example -->

```sql
SHOW LOCKS;
```

<!-- response Example -->

```sql
+-----------+------+-----------+-----------------+
| Type      | Name | Lock Type | Additional Info |
+-----------+------+-----------+-----------------+
| rt        | a    | freeze    | Count: 1        |
| percolate | bar  | freeze    | Count: 3        |
| rt        | foo  | freeze    | Count: 2        |
+-----------+------+-----------+-----------------+
3 rows in set (0,01 sec)
```

<!-- end -->

