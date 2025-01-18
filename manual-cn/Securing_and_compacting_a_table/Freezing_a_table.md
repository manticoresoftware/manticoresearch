# 冻结表

<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 命令准备一个实时/普通表进行安全的 [备份](../Securing_and_compacting_a_table/Backup_and_restore.md)。具体步骤包括：

1. 停用表的压缩功能。如果表正在被压缩，`FREEZE` 将优雅地中断它。
2. 将当前的 RAM 块转移到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表的锁定计数器。
6. 显示与表相关的实际文件列表。

如果表已被冻结（锁定），`FREEZE` 将：

1. 增加表的锁定计数器。
2. 显示与表相关的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。您可以在创建自己的备份解决方案或需要冻结表时执行相同操作，只需遵循以下步骤：

1. `FREEZE` 表（或多个表）。
2. 捕获 `FREEZE` 命令的输出并备份指定的文件。
3. 一旦完成，执行 `UNFREEZE` 命令。

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

当表被冻结时，您不能执行 `UPDATE` 查询；它们会因“索引现在被锁定，请稍后再试”的错误消息而失败。

同时，`DELETE` 和 `REPLACE` 查询在表被冻结时也有一些限制：

- 如果 `DELETE` 影响当前 RAM 块中的文档——这是允许的。
- 如果 `DELETE` 影响磁盘块中的文档但已被先前删除——这是允许的。
- 如果 `DELETE` 会改变实际的磁盘块——它将等待直到表解冻。

手动 `FLUSH` 冻结表的 RAM 块将报告“成功”，但不会实际保存任何数据。

冻结表的 `DROP`/`TRUNCATE` 操作是允许的，因为这些操作不是隐式的。我们假设如果您要截断或删除表，则不需要备份它，因此它不应该在最初被冻结。

在冻结的表中支持 `INSERT`，但有限制：新数据将按常规存储在 RAM 中，直到达到 `rt_mem_limit`；然后，新插入将等待表解冻。

如果您在冻结表的情况下关闭守护进程，它将表现得像经历了脏关闭（例如，`kill -9`）：新插入的数据将**不会**被保存到磁盘上的 RAM 块，并且在重新启动时将从二进制日志（如果有的话）恢复，或丢失（如果禁用了二进制日志）。

# 解冻表

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令减少表的锁定计数器，如果计数器达到零，则重新激活之前被阻止的操作并恢复内部压缩服务。所有等待表解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

# 检查表的锁定状态

<!-- example show_table_status -->

可以使用`SHOW table_name STATUS` 来检查表是否被冻结。

锁定计数器显示在表的状态下的 `locked` 列中。零值表示表未被冻结，而非零值表示活动锁的数量。每个显式的 `FREEZE` 命令和隐式锁定（如当表是集群的一部分且复制例程选择它作为副本的捐赠者时）都会增加计数器。每个 `UNFREEZE` 命令都会减少计数器，最终降到零。

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

# SHOW LOCKS

<!-- example show_locks -->

锁定的表也可以通过 `SHOW LOCKS` 命令显示。锁定计数器在 `Additional Info` 列中显示。

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
