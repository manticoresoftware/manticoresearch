# 冻结表格

<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 准备一个实时/普通表格进行安全 [备份](../Securing_and_compacting_a_table/Backup_and_restore.md)。具体来说，它会：
1. 停止表格压缩。如果表格当前正在被压缩，`FREEZE` 会优雅地中断它。
2. 将当前的 RAM 块传输到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表格的锁定计数器。
6. 显示与表格相关的实际文件列表。

如果表格已经被冻结（锁定），`FREEZE` 会：
1. 增加表格的锁定计数器。
2. 显示与表格相关的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。如果您希望创建自己的备份解决方案或需要冻结表格出于其他原因，可以遵循以下步骤：
1. `FREEZE` 一个表格（或几个）。
2. 捕获 `FREEZE` 命令的输出并备份指定的文件。
3. 完成后 `UNFREEZE` 表格。

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

`file` 列表示运行实例的 [data_dir](../Server_settings/Searchd.md#data_dir) 中表格的文件路径。`normalized` 列显示相同文件的绝对路径。要备份表格，只需复制提供的文件，无需额外准备。

当表格被冻结时，您无法执行 `UPDATE` 查询；它们将等待表格解冻。

此外，`DELETE` 和 `REPLACE` 查询在表格冻结时有一些限制：
* 如果 `DELETE` 影响当前 RAM 块中的文档 - 是允许的。
* 如果 `DELETE` 影响磁盘块中的文档但之前已被删除 - 是允许的。
* 如果 `DELETE` 将更改实际磁盘块 - 将等待直到表格解冻。

手动 `FLUSH` 冻结表格的 RAM 块会报告“成功”，但不会实际保存。

冻结表格的 `DROP`/`TRUNCATE` **是** 允许的，因为这些操作不是隐式的。我们假设如果您截断或删除一个表格，您不需要它备份；因此，它不应该最初被冻结。

向冻结表格 `INSERT` 数据是支持的，但有限：新数据将在 RAM 中存储（如常规）直到达到 `rt_mem_limit`；然后，新插入将等待直到表格解冻。

如果您在冻结的表格上关闭守护进程，它将表现得好像经历了肮脏的关机（例如， `kill -9`）：新插入的数据将 **不** 会被保存到磁盘上的 RAM 块中，并且在重启时将从二进制日志（如果有）恢复或丢失（如果禁用了二进制日志记录）。

## 解冻表格

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令减少表格的锁定计数器，如果计数器达到零，则重新激活先前被阻止的操作并恢复内部压缩服务。任何等待表格解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

## 检查表格的锁定状态

<!-- example show_table_status -->

您可以使用 `SHOW table_name STATUS` 检查表格是否被冻结。

锁定计数器在表格状态中的 `locked` 列下显示。值为零表示表格未被冻结，而非零值反映了活动锁的数量。每个显式的 `FREEZE` 命令和隐式锁定（例如，当表格是集群的一部分并且复制例程将其选为副本的供体时）都会增加计数器。每个 `UNFREEZE` 命令减少计数器，最终降至零。

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

锁定的表格也可以通过 `SHOW LOCKS` 命令显示。锁定计数器显示在 `Additional Info` 列中。

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
3 行在集合中 (0,01 秒)
```

<!-- end -->
# 冻结表格

<!-- example freeze -->

```sql
FREEZE tbl1[, tbl2, ...]
```

`FREEZE` 准备一个实时/纯表以进行安全的 [备份](../Securing_and_compacting_a_table/Backup_and_restore.md)。具体来说，它：
1. 停用表的压缩。如果表当前正在被压缩，`FREEZE` 会优雅地中断它。
2. 将当前的 RAM 块转移到磁盘块。
3. 刷新属性。
4. 禁用可能修改磁盘文件的隐式操作。
5. 增加表的锁定计数器。
6. 显示与表关联的实际文件列表。

如果表已经被冻结（锁定），`FREEZE` 将：
1. 增加表的锁定计数器。
2. 显示与表关联的实际文件列表。

内置工具 [manticore-backup](../Securing_and_compacting_a_table/Backup_and_restore.md) 使用 `FREEZE` 来确保数据一致性。如果您想创建自己的备份解决方案或出于其他原因需要冻结表，可以按照以下步骤操作：
1. `FREEZE` 一个表（或几个）。
2. 捕获 `FREEZE` 命令的输出并备份指定的文件。
3. 一旦完成，`UNFREEZE` 表（们）。

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

`file` 列表示正在运行的实例的 [data_dir](../Server_settings/Searchd.md#data_dir) 中的表文件路径。`normalized` 列显示相同文件的绝对路径。要备份一个表，只需复制提供的文件，无需额外准备。

当表被冻结时，您无法执行 `UPDATE` 查询；它们将等到表解冻。

此外，当表被冻结时，`DELETE` 和 `REPLACE` 查询有一些限制：
* 如果 `DELETE` 影响当前的 RAM 块中的文档 - 是允许的。
* 如果 `DELETE` 影响了磁盘块中的文档但之前已被删除 - 也是允许的。
* 如果 `DELETE` 会修改实际的磁盘块 - 它将等到表解冻后进行处理。

手动 `FLUSH` 冻结表的 RAM 块将报告“成功”，但不会发生实际的保存。

对于冻结表，`DROP`/`TRUNCATE` **是** 允许的，因为这些操作不是隐式的。我们假设如果您截断或删除一个表，您不需要它备份；因此，它本来不应该最初被冻结。

向冻结表中 `INSERT` 是支持的，但有限制：新的数据将存储在 RAM 中（像往常一样），直到达到 `rt_mem_limit`；然后，新的插入操作将等待直到表被解冻。

如果您关闭了一个有冻结表的守护进程，它将表现得好像经历了一个脏关闭（例如，`kill -9`）：新插入的数据将 **不** 被保存到磁盘的 RAM 块中，并且重新启动时将从二进制日志（如果有）中恢复，或者（如果禁用二进制日志）会丢失。

# 解冻表格

<!-- example unfreeze -->

```sql
UNFREEZE tbl1[, tbl2, ...]
```

`UNFREEZE` 命令减少表的锁定计数器，如果它达到零，则重新激活之前被阻止的操作并恢复内部压缩服务。任何等待表解冻的操作也将恢复并正常完成。

<!-- request Example -->
```sql
UNFREEZE tbl;
```

<!-- end -->

# 检查表的锁定状态

<!-- example show_table_status -->

您可以使用 `SHOW table_name STATUS` 检查表是否被冻结。

锁定计数器在表的状态下的 `locked` 列中显示。零值表示表未被冻结，而非零值反映了活动锁的数量。每个显式的 `FREEZE` 命令和隐式锁定（例如，当表是集群的一部分，并且复制例程将其选为副本的捐赠者时）都会增加计数器。每个 `UNFREEZE` 命令减少计数器，最终降至零。

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

# 显示锁定状态

<!-- example show_locks -->

锁定的表也可以使用 `SHOW LOCKS` 命令进行显示。锁定计数在 `Additional Info` 列中显示。

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
