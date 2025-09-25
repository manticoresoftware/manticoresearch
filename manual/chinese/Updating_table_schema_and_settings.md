# 更新表模式

## 在 RT 模式下更新表模式

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

此功能仅支持一次为 RT 表添加一个字段或将 `int` 列扩展为 `bigint`。支持的数据类型有：
* `int` - 整数属性
* `timestamp` - 时间戳属性
* `bigint` - 大整数属性
* `float` - 浮点属性
* `bool` - 布尔属性
* `multi` - 多值整数属性
* `multi64` - 多值大整数属性
* `json` - json 属性
* `string` / `text attribute` / `string attribute` - 字符串属性
* `text` / `text indexed stored` / `string indexed stored` - 全文索引字段，原始值存储于 docstore 中
* `text indexed` / `string indexed` - 仅索引的全文索引字段（原始值不存储于 docstore）
* `text indexed attribute` / `string indexed attribute` - 全文索引字段 + 字符串属性（不在 docstore 中存储原始值）
* `text stored` / `string stored` - 仅存储于 docstore，不全文索引，也不是字符串属性
* 向任何属性添加 `engine='columnar'`（除 json 外）将使其存储在 [列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

#### 重要说明：
* ❗建议在执行 `ALTER` 之前 **备份表文件**，以避免突发断电或类似问题导致数据损坏。
* 添加列时无法查询表。
* 新创建属性的值默认设置为 0。
* `ALTER` 不适用于分布式表和无属性的表。
* 不能删除 `id` 列。
* 当删除既是全文字段又是字符串属性的字段时，第一次 `ALTER DROP` 删除属性，第二次才删除全文字段。
* 仅支持在 [RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下添加/删除全文字段。

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

## 在 RT 模式下更新表全文设置

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

可以使用 `ALTER` 修改表在 [RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 下的全文设置，但仅影响新文档，不影响现有文档。
示例：
* 创建一个包含全文字段和 `charset_table` 的表，`charset_table` 仅允许搜索字符 `a`、`b` 和 `c`。
* 插入文档 'abcd' 并通过查询 `abcd` 查找，`d` 被忽略因为它不在 `charset_table` 数组中。
* 后来需要 `d` 也可被搜索，于是通过 `ALTER` 添加了它。
* 但相同的查询 `where match('abcd')` 仍显示只搜索了 `abc`，因为现有文档记住了之前的 `charset_table` 内容。
* 然后添加另一个文档 `abcd` 并再次查询 `abcd`。
* 现在，它能找到两个文档，且 `show meta` 显示它用到了两个关键词：`abc`（用于旧文档）和 `abcd`（用于新文档）。

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

可以在 RT 模式下更改实时表名称。
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> 注意：重命名实时表需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果不起作用，请确认 Buddy 已安装。

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在 Plain 模式下更新表全文设置

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` 也可重新配置 [Plain 模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 下的 RT 表，使配置文件中的新分词、形态学及其他文本处理设置对新文档生效。注意，现有文档保持不变。内部它会强制将当前 RAM 数据块保存为新磁盘数据块，并调整表头，使新文档使用更新的全文设置进行分词。

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

## 重新构建二级索引

<!-- example ALTER REBUILD SECONDARY -->
```sql
ALTER TABLE table REBUILD SECONDARY
```

也可以使用 `ALTER` 重新构建指定表的二级索引。有时，整个表或表中的一个或多个属性的二级索引可能被禁用：
* 当属性更新时，该属性的二级索引会被禁用。
* 如果 Manticore 加载的表使用的是不支持的旧版二级索引，则该表的二级索引会被禁用。

`ALTER TABLE table REBUILD SECONDARY` 从属性数据重建二级索引并重新启用它们。

另外，旧版本的二级索引可能受支持但缺少某些功能。`REBUILD SECONDARY` 可用于更新二级索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->


## 重建 KNN 索引

<!-- example ALTER REBUILD KNN -->
```sql
ALTER TABLE table REBUILD KNN
```

该命令会重新处理表中所有向量数据，并从头重建 KNN 索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在 RT 模式下更新属性 API 密钥（用于嵌入生成）

<!-- example api_key -->

当远程模型用于自动嵌入时，可以使用 `ALTER` 修改 API 密钥：

```sql
ALTER TABLE table_name MODIFY COLUMN column_name API_KEY='key';
```

<!-- request Example -->
```sql
ALTER TABLE rt MODIFY COLUMN vector API_KEY='key';
```

<!-- end -->

## 修改分布式表

<!-- example local_dist -->

要更改分布式表中本地或远程节点列表，使用与[创建表](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)相同的语法。只需将命令中的 `CREATE` 替换为 `ALTER`，并移除 `type='distributed'`：

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> 注意：在线更改分布式表的模式需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如无效，请确认 Buddy 已安装。

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

