# 更新表结构

## 在 RT 模式下更新表结构

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

该功能仅支持对 RT 表一次添加一个字段或将 `int` 列扩展为 `bigint`。支持的数据类型包括：
* `int` - 整数字段
* `timestamp` - 时间戳字段
* `bigint` - 大整数字段
* `float` - 浮点字段
* `bool` - 布尔字段
* `multi` - 多值整数字段
* `multi64` - 多值大整数字段
* `json` - json 字段
* `string` / `text attribute` / `string attribute` - 字符串字段
* `text` / `text indexed stored` / `string indexed stored` - 全文索引字段，原始值存储在 docstore 中
* `text indexed` / `string indexed` - 仅全文索引字段（原始值不存储在 docstore）
* `text indexed attribute` / `string indexed attribute` - 全文索引字段 + 字符串属性（不在 docstore 中存储原始值）
* `text stored` / `string stored` - 只存储在 docstore 中的值，不建立全文索引，也不是字符串属性
* 给任意属性（json 除外）添加 `engine='columnar'`，将其存储在[列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中

#### 重要说明：
* ❗建议在执行 `ALTER` 前备份表文件，以防出现电源故障或类似问题导致数据损坏。
* 添加列时不可查询表。
* 新建属性的值默认为 0。
* `ALTER` 不适用于分布式表和无属性的表。
* 不能删除 `id` 列。
* 删除同时是全文字段和字符串属性的字段时，第一次 `ALTER DROP` 删除属性，第二次删除全文字段。
* 添加/删除全文字段仅支持[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。

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

您可以使用 `ALTER` 修改表的全文设置（基于[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)），但这只对新文档生效，已有文档不受影响。
示例：
* 创建一个包含全文字段且 `charset_table` 只允许 3 个可搜索字符：`a`、`b` 和 `c` 的表。
* 插入文档 'abcd' 并通过查询 `abcd` 查找，因为 `d` 不在 `charset_table` 中，故被忽略。
* 发现需要支持 `d` 作为可搜索字符，使用 `ALTER` 添加。
* 但相同的查询 `where match('abcd')` 依然只按 `abc` 搜索，因为已有文档保留了之前的 `charset_table` 内容。
* 添加另一份文档 `abcd` 并再次搜索 `abcd`。
* 现在能找到两份文档，`show meta` 显示用了两个关键词：`abc`（找旧文档）和 `abcd`（找新文档）。

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

> 注意：重命名实时表需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。若不工作，请确认 Buddy 是否已安装。

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

`ALTER` 也可以重新配置基于[普通模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 的 RT 表，使配置文件中新令牌化、形态学及其它文本处理设置生效于新文档。注意，已有文档不受影响。内部强制将当前 RAM 块存为新磁盘块并调整表头，使新文档采用更新的全文设置进行分词。

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

也可以使用 `ALTER` 重建指定表中的二级索引。二级索引有时会被禁用，可能是整个表或表内一个或多个属性：
* 当属性被更新，其二级索引会被禁用。
* 如果 Manticore 加载了不再支持的二级索引旧版本，表的二级索引将全部被禁用。

`ALTER TABLE table REBUILD SECONDARY` 从属性数据重建二级索引并重新启用。

此外，旧版本二级索引可能被支持，但缺少某些特性，`REBUILD SECONDARY` 可用来更新二级索引。

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

该命令重新处理表中所有向量数据，从头构建 KNN 索引。

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

当使用远程模型自动生成嵌入时，`ALTER` 可修改 API 密钥：

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

要更改分布式表中本地或远程节点列表，请使用与[创建表](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)时相同的语法。只需将命令中的 `CREATE` 替换为 `ALTER`，并移除 `type='distributed'`：

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> 注意：在线更改分布式表结构需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。若不工作，请确认 Buddy 是否已安装。

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

