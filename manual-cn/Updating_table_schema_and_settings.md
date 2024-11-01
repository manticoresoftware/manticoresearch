# 更新表结构

## 在实时模式下更新表结构

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

此功能仅支持实时表中逐个添加字段或将 `int` 列扩展为 `bigint`。支持的数据类型包括：

- `int` - 整数属性
- `timestamp` - 时间戳属性
- `bigint` - 大整数属性
- `float` - 浮点属性
- `bool` - 布尔属性
- `multi` - 多值整数属性
- `multi64` - 多值大整数属性
- `json` - JSON 属性
- `string` / `text 属性` / `string 属性` - 字符串属性
- `text` / `text indexed stored` / `string indexed stored` - 带原始值存储的全文索引字段
- `text indexed` / `string indexed` - 仅索引的全文索引字段（原始值不存储在文档存储中）
- `text indexed attribute` / `string indexed attribute` - 全文索引字段 + 字符串属性（不存储原始值）
- `text stored` / `string stored` - 仅存储在文档存储中的值，不进行全文索引，也不是字符串属性
- 将 `engine='columnar'` 添加到任何属性（JSON 除外）将使其存储在 [列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 中

#### 重要说明：

* ❗建议在执行 `ALTER` 前 **备份表文件**，以避免在突发断电或类似问题时数据损坏。
* 在添加列期间无法查询表。
* 新创建属性的值默认为 0。
* `ALTER` 不适用于分布式表和没有任何属性的表。
* 不能删除 `id` 列。
* 删除同时是全文字段和字符串属性的字段时，第一次 `ALTER DROP` 会删除属性，第二次会删除全文字段。
* 添加/删除全文字段仅在 [实时模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 下支持。

<!-- request Example -->
```sql

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| group_id   | uint      |
| date_added | timestamp |
+------------+-----------+

mysql> alter table rt add column test integer;

mysql> desc rt;
+------------+-----------+
| Field      | Type      |
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
| Field      | Type      |
+------------+-----------+
| id         | bigint    |
| text       | field     |
| date_added | timestamp |
| test       | uint      |
+------------+-----------+

mysql> alter table rt add column title text indexed;

mysql> desc rt;
+------------+-----------+------------+
| Field      | Type      | Properties |
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
| Field      | Type      | Properties |
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
| Field      | Type      | Properties |
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
| Field      | Type      | Properties |
+------------+-----------+------------+
| id         | bigint    |            |
| text       | text      | indexed    |
| date_added | timestamp |            |
| test       | uint      |            |
+------------+-----------+------------+
```

<!-- end -->

## 在实时模式下更新表的全文搜索设置

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

您可以使用 `ALTER` 来修改实时模式下表的全文设置 [RT mode](Read_this_first.md#Real-time-mode-vs-plain-mode)。但是，这只会影响新文档，而不会影响现有文档。

示例：

- 创建一个包含全文字段和 `charset_table` 的表，仅允许 3 个可搜索字符：`a`、`b` 和 `c`。
- 然后插入文档 'abcd' 并通过查询 `abcd` 查找它，由于 `d` 不在 `charset_table` 数组中，因此被忽略。
- 接着，我们意识到我们希望 `d` 也可以被搜索，因此借助 `ALTER` 添加它。
- 但是同样的查询 `where match('abcd')` 仍然表示它只搜索了 `abc`，因为现有文档记住了之前的 `charset_table` 内容。
- 然后我们添加另一个文档 `abcd`，再次通过 `abcd` 搜索。
- 现在它找到了两个文档，并且 `show meta` 显示使用了两个关键词：`abc`（用于查找旧文档）和 `abcd`（用于新文档）。

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

您可以在实时模式下更改实时表的名称。
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

## 更新普通表的全文搜索设置

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` 也可以在 [普通模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-(Plain-mode)) 下重新配置 RT 表，这样配置文件中的新的分词、词法和其他文本处理设置将对新文档生效。请注意，现有文档将保持不变。内部机制会强制将当前的 RAM 块保存为一个新的磁盘块，并调整表头，使得新文档使用更新后的全文设置进行分词。

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

你还可以使用 `ALTER` 命令来重建给定表中的二级索引。在某些情况下，二级索引可能会对整个表或表中的一个或多个属性禁用：

- 当一个属性被更新时，其二级索引会被禁用。
- 如果 Manticore 加载了带有旧版本二级索引的表，并且该版本不再受支持，二级索引将对整个表禁用。

`ALTER TABLE table REBUILD SECONDARY` 会从属性数据中重建二级索引，并重新启用它们。

此外，旧版本的二级索引可能仍然受支持，但可能缺少某些功能。可以使用 `REBUILD SECONDARY` 来更新二级索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 修改分布式表

<!-- example local_dist -->

要更改分布式表中本地或远程节点的列表，请使用与 [创建表](../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table) 时相同的语法，只需在命令中将 `CREATE` 替换为 `ALTER` 并删除 `type='distributed'`：

```sql
ALTER TABLE `distr_table_name` [[local='local_index_name'], [agent='host:port:remote_index'] ... ]
```

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_index';
```

<!-- end -->
<!-- proofread -->
