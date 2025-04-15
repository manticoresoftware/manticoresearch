# 更新表架构

## 在实时模式中更新表架构

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

此功能仅支持一次为实时表添加一个字段或将 `int` 列扩展为 `bigint`。支持的数据类型包括：
* `int` - 整数属性
* `timestamp` - 时间戳属性
* `bigint` - 大整数属性
* `float` - 浮点属性
* `bool` - 布尔属性
* `multi` - 多值整数属性
* `multi64` - 多值大整数属性
* `json` - json 属性
* `string` / `text attribute` / `string attribute` - 字符串属性
* `text` / `text indexed stored` / `string indexed stored` - 带原始值存储在文档库中的全文索引字段
* `text indexed` / `string indexed` - 仅索引的全文索引字段（原始值不存储在文档库中）
* `text indexed attribute` / `string indexed attribute` - 全文索引字段 + 字符串属性（不在文档库中存储原始值）
* `text stored` / `string stored` - 值仅存储在文档库中，不是全文索引字段，不是字符串属性
* 将 `engine='columnar'` 添加到任何属性（除了 json）将使其存储在 [列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### 重要注意事项：
* ❗建议在 `ALTER` 表之前 **备份表文件**，以避免在突然断电或其他类似问题时数据损坏。
* 在添加列时无法查询表。
* 新创建的属性的值默认设置为 0。
* `ALTER` 不适用于分布式表和没有任何属性的表。
* 不能删除 `id` 列。
* 当删除一个既是全文字段又是字符串属性的字段时，第一个 `ALTER DROP` 删除属性，第二个删除全文字段。
* 仅在 [实时模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中支持添加/删除全文字段。

<!-- request Example -->
```sql

mysql> desc rt;
+------------+-----------+
| 字段       | 类型      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
+------------+-----------+

mysql> alter table rt add column test integer;

mysql> desc rt;
+------------+-----------+
| 字段       | 类型      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt drop column group_id;

mysql> desc rt;
+------------+-----------+
| 字段       | 类型      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt add column title text indexed;

mysql> desc rt;
+------------+-----------+------------+
| 字段       | 类型      | 属性      |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+

mysql> alter table rt add column title text attribute;

mysql> desc rt;
+------------+-----------+------------+
| 字段       | 类型      | 属性      |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
| title      | string    |            |
+------------+-----------+------------+

mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| 字段       | 类型      | 属性      |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| title      | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
mysql> alter table rt drop column title;

mysql> desc rt;
+------------+-----------+------------+
| 字段       | 类型      | 属性      |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
```

<!-- end -->

## 在实时模式中更新表的全文设置

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

您可以使用 `ALTER` 来修改表在 [实时模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中的全文设置。然而，这仅影响新文档，而不影响现有文档。
示例：
* 创建一个具有全文字段和仅允许 3 个可搜索字符的 `charset_table` 的表：`a`、`b` 和 `c`。
* 然后我们插入文档 'abcd'，并通过查询 `abcd` 找到它，`d` 被忽略，因为它不在 `charset_table` 数组中
* 然后我们明白，我们也想让 `d` 可搜索，所以我们借助 `ALTER` 添加它
* 但是相同的查询 `where match('abcd')` 仍然表示它搜索的是 `abc`，因为现有文档记住了之前的 `charset_table` 内容
* 然后我们添加另一个文档 `abcd` 并再次按 `abcd` 搜索
* 现在它找到了两个文档，`show meta` 显示它使用了两个关键字： `abc` （找到旧文档）和 `abcd` （找到新文档）。

<!-- request Example -->
```sql
mysql> create table rt(title text) charset_table='a,b,c';

mysql> insert into rt(title) values('abcd');

mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> alter table rt charset_table='a,b,c,d';
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 1     |
| total_found   | 1     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
+---------------+-------+

mysql> insert into rt(title) values('abcd');
mysql> select * from rt where match('abcd');
+---------------------+-------+
| id                  | title |
+---------------------+-------+
| 1514630637682688055 | abcd  |
| 1514630637682688054 | abcd  |
+---------------------+-------+

mysql> show meta;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| total         | 2     |
| total_found   | 2     |
| time          | 0.000 |
| keyword[0]    | abc   |
| docs[0]       | 1     |
| hits[0]       | 1     |
| keyword[1]    | abcd  |
| docs[1]       | 1     |
| hits[1]       | 1     |
+---------------+-------+
```

<!-- end -->

## 重命名实时表

<!-- example Renaming RT tables -->

您可以在 RT 模式下更改实时表的名称。
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> 注意：重命名实时表需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 更新普通模式下的表 FT 设置

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` 还可以在 [普通模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 下重新配置 RT 表，以便来自配置文件的新标记、形态学和其他文本处理设置在新文档上生效。请注意，现有文档将保持不变。在内部，它会强制将当前 RAM 块保存为新的磁盘块并调整表头，以便使用更新的全文设置对新文档进行标记。

<!-- request Example -->
```sql
mysql> show table rt settings;
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| settings      |       |
+---------------+-------+
1 row in set (0.00 sec)

mysql> alter table rt reconfigure;
Query OK, 0 rows affected (0.00 sec)

mysql> show table rt settings;
+---------------+----------------------+
| Variable_name | Value                |
+---------------+----------------------+
| settings      | morphology = stem_en |
+---------------+----------------------+
1 row in set (0.00 sec)
```
<!-- end -->

## 重建二级索引

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

您还可以使用 `ALTER` 重建给定表中的二级索引。有时，整个表或表中的一个或多个属性的二级索引可能会被禁用：
* 当更新属性时，其二级索引会被禁用。
* 如果 Manticore 加载了一个旧版本的二级索引而不再支持，则整个表的二级索引将被禁用。

`ALTER TABLE table REBUILD SECONDARY` 从属性数据重建二级索引并重新启用它们。

此外，虽然可能支持旧版本的二级索引，但某些功能可能缺失。可以使用 `REBUILD SECONDARY` 来更新二级索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 更改分布式表

<!-- example local_dist -->

要更改分布式表中的本地或远程节点列表，请遵循与 [创建表](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table) 相同的语法。只需将命令中的 `CREATE` 替换为 `ALTER` 并删除 `type='distributed'`：

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> 注意：在线更改分布式表的架构需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->
