# 更新表结构

## 在RT模式下更新表结构

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON [secondary_index='1']|STRING|TEXT [INDEXED [ATTRIBUTE]]|TIMESTAMP}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

此功能仅支持为RT表一次添加一个字段，或将`int`列扩展为`bigint`。支持的数据类型包括：
* `int` - 整数属性
* `timestamp` - 时间戳属性
* `bigint` - 大整数属性
* `float` - 浮点数属性
* `bool` - 布尔属性
* `multi` - 多值整数属性
* `multi64` - 多值bigint属性
* `json` - JSON 属性；使用 `secondary_index='1'` 为 JSON 创建二级索引
* `string` / `text attribute` / `string attribute` - 字符串属性
* `text` / `text indexed stored` / `string indexed stored` - 全文索引字段，原始值存储在docstore中
* `text indexed` / `string indexed` - 全文索引字段，仅索引（原始值不存储在docstore中）
* `text indexed attribute` / `string indexed attribute` - 全文索引字段 + 字符串属性（不将原始值存储在docstore中）
* `text stored` / `string stored` - 值仅存储在docstore中，不进行全文索引，也不是字符串属性
* 为任何属性（json除外）添加`engine='columnar'`将使其存储在[列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中

#### 重要注意事项：
* ❗建议在`ALTER`表之前**备份表文件**，以防突然断电或其他类似问题导致数据损坏。
* 添加列时无法查询表。
* 新创建的属性值设置为0。
* `ALTER`不适用于分布式表和无任何属性的表。
* 不能删除`id`列。
* 当删除一个既是全文字段又是字符串属性的字段时，第一次`ALTER DROP`删除属性，第二次删除全文字段。
* 添加/删除全文字段仅在[RT模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下支持。

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

## 在RT模式下更新表全文设置

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

您可以使用`ALTER`在[RT模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下修改表的全文设置。但是，它只影响新文档，不影响现有文档。
示例：
* 创建一个具有全文字段和`charset_table`的表，该表只允许3个可搜索字符：`a`、`b`和`c`。
* 然后我们插入文档'abcd'并通过查询`abcd`找到它，`d`被忽略，因为它不在`charset_table`数组中
* 然后我们意识到，我们也希望`d`可搜索，因此我们借助`ALTER`添加它
* 但相同的查询`where match('abcd')`仍然显示它搜索的是`abc`，因为现有文档记住了`charset_table`的先前内容
* 然后我们添加另一个文档`abcd`并再次搜索`abcd`
* 现在它找到了两个文档，并且`show meta`显示它使用了两个关键词：`abc`（用于查找旧文档）和`abcd`（用于新文档）。

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

您可以在RT模式下更改实时表的名称。
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> 注意：重命名实时表需要[Manticore Buddy](Installation/Manticore_Buddy.md)。如果不起作用，请确保Buddy已安装。

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在普通模式下更新表全文设置

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER`还可以在[普通模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)下重新配置RT表，以便配置文件中的新分词、词法分析和其他文本处理设置对新文档生效。注意，现有文档将保持不变。在内部，它会强制将当前RAM块保存为新的磁盘块，并调整表头，以便使用更新的全文设置对新文档进行分词。

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

您还可以使用`ALTER`重建给定表中的二级索引。有时，二级索引可能对整个表或表中的一个或多个属性被禁用：
* 当属性更新时，其二级索引将被禁用。
* 如果Manticore加载了一个带有不再支持的旧版本二级索引的表，则整个表的二级索引将被禁用。

`ALTER TABLE table REBUILD SECONDARY`从属性数据重建二级索引并重新启用它们。

此外，旧版本的二级索引可能受支持，但会缺少某些功能。`REBUILD SECONDARY`可用于更新二级索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD SECONDARY;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->


## 重建KNN索引

<!-- example ALTER REBUILD KNN -->
```sql
ALTER TABLE table REBUILD KNN
```

该命令重新处理表中的所有向量数据，并从头开始重建KNN索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在RT模式下更新属性API密钥（用于嵌入生成）

<!-- example api_key -->

当远程模型用于自动嵌入时，可以使用`ALTER`修改API密钥：

```sql
ALTER TABLE table_name MODIFY COLUMN column_name API_KEY='key';
```

<!-- request Example -->
```sql
ALTER TABLE rt MODIFY COLUMN vector API_KEY='key';
```

<!-- end -->

## 更改分布式表

<!-- example local_dist -->

要更改分布式表中的本地或远程节点列表，请遵循与[创建表](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)时相同的语法。只需将命令中的`CREATE`替换为`ALTER`，并移除`type='distributed'`：

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> 注意：在线更改分布式表的模式需要[Manticore Buddy](Installation/Manticore_Buddy.md)。如果不起作用，请确保Buddy已安装。

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

