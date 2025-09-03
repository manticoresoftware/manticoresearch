# 更新表结构

## 在 RT 模式下更新表结构

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

此功能仅支持一次为 RT 表添加一个字段，或将 `int` 列扩展为 `bigint`。支持的数据类型有：
* `int` - 整数属性
* `timestamp` - 时间戳属性
* `bigint` - 大整数属性
* `float` - 浮点属性
* `bool` - 布尔属性
* `multi` - 多值整数属性
* `multi64` - 多值大整数属性
* `json` - json 属性
* `string` / `text attribute` / `string attribute` - 字符串属性
* `text` / `text indexed stored` / `string indexed stored` - 全文索引字段，原始值存储在文档存储中
* `text indexed` / `string indexed` - 仅索引的全文索引字段（原始值不存储在文档存储中）
* `text indexed attribute` / `string indexed attribute` - 全文索引字段 + 字符串属性（不在文档存储中保存原始值）
* `text stored` / `string stored` - 该值仅存储在文档存储中，不做全文索引，也不是字符串属性
* 向任何属性（除 json 之外）添加 `engine='columnar'` 会使其存储在[列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中

#### 重要注意事项：
* ❗建议在执行 `ALTER` 之前**备份表文件**，以防止突然断电或其他类似问题导致数据损坏。
* 添加列时无法查询表。
* 新创建的属性值均设置为 0。
* `ALTER` 不适用于分布式表和无属性的表。
* 不能删除 `id` 列。
* 当删除同时是全文字段和字符串属性的字段时，第一次 `ALTER DROP` 删除属性，第二次删除全文字段。
* 添加/删除全文字段只支持在[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下进行。

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

## 在 RT 模式下更新表的全文设置

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

你可以使用 `ALTER` 修改表在[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)下的全文设定。但该修改只影响新文档，已有文档不受影响。
示例：
* 创建一个表，带有全文字段和允许仅搜索 3 个字符的 `charset_table`，字符为 `a`、`b` 和 `c`。
* 然后插入文档 'abcd'，用查询 `abcd` 查找，`d` 会被忽略，因为它不在 `charset_table` 数组中。
* 后来发现，希望 `d` 也能被搜索，于是利用 `ALTER` 添加了它。
* 但同样的查询 `where match('abcd')` 仍然显示为搜索 `abc`，因为已有文档记住了之前的 `charset_table` 内容。
* 然后再添加一个新的文档 `abcd` 并再次用 `abcd` 搜索。
* 现在找到两个文档，`show meta` 显示使用了两个关键词：`abc`（用于旧文档）和 `abcd`（用于新文档）。

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

你可以在 RT 模式下更改实时表的名称。
```sql
ALTER TABLE table_name RENAME new_table_name;
```

> 注意：重命名实时表需要使用 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法生效，请确保 Buddy 已安装。

<!-- request Example -->
```sql
ALTER TABLE table_name RENAME new_table_name;
```

<!-- response Example -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在普通模式下更新表的全文设置

<!-- example ALTER RECONFIGURE -->
```sql
ALTER TABLE table RECONFIGURE
```

`ALTER` 也可以在[普通模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)下重新配置 RT 表，使配置文件中的新的分词、形态学及其他文本处理设置对新文档生效。注意，已有文档保持不变。内部实现是强制将当前内存块保存为新的磁盘块，并调整表头，使得新文档使用更新的全文设置进行分词。

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

你也可以使用 `ALTER` 重建指定表的二级索引。有时，二级索引可能被禁用，针对整个表或表中的某些属性：
* 当属性被更新时，其二级索引会被禁用。
* 如果 Manticore 加载了带有旧版本二级索引且该版本不再支持的表，二级索引会被禁用。

`ALTER TABLE table REBUILD SECONDARY` 会根据属性数据重建二级索引，并重新启用它们。

此外，旧版本的二级索引可能仍受支持，但缺少某些功能。可以使用 `REBUILD SECONDARY` 对其进行更新。

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

该命令会重新处理表中的所有向量数据，并从头构建 KNN 索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在 RT 模式下更新属性 API 密钥（用于生成嵌入）

<!-- example api_key -->

在使用远程模型自动生成嵌入时，可以使用 `ALTER` 修改 API 密钥：

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

若要更改分布式表中本地或远程节点列表，使用创建表时相同的语法，只需将命令中的 `CREATE` 替换为 `ALTER`，并去掉 `type='distributed'`：

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> 注意：在线更改分布式表的结构需要使用 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法生效，请确保 Buddy 已安装。









