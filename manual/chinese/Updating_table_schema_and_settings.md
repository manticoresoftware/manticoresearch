# 更新表结构

## 在实时模式下更新表结构

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON|STRING|TIMESTAMP|TEXT [INDEXED [ATTRIBUTE]]}] [engine='columnar']

ALTER TABLE table DROP COLUMN column_name

ALTER TABLE table MODIFY COLUMN column_name bigint
```

此功能仅支持对 RT 表一次添加一个字段，或将 `int` 列扩展为 `bigint`。支持的数据类型有：
* `int` - 整数字段属性
* `timestamp` - 时间戳字段属性
* `bigint` - 大整数字段属性
* `float` - 浮点字段属性
* `bool` - 布尔字段属性
* `multi` - 多值整数字段属性
* `multi64` - 多值大整数字段属性
* `json` - json 字段属性
* `string` / `text attribute` / `string attribute` - 字符串字段属性
* `text` / `text indexed stored` / `string indexed stored` - 带原始值存储在文档仓库中的全文索引字段
* `text indexed` / `string indexed` - 仅全文索引字段（原始值不存储在文档仓库中）
* `text indexed attribute` / `string indexed attribute` - 全文索引字段 + 字符串属性（不存储原始值在文档仓库）
* `text stored` / `string stored` - 仅存储在文档仓库中，不做全文索引，也不是字符串属性的字段
* 向任何字段（除 json 外）添加 `engine='columnar'`，会将其存储在[列存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 中

#### 重要说明：
* ❗建议在 `ALTER` 表之前**备份表文件**，以防突然断电或其他类似问题导致数据损坏。
* 正在添加列时无法查询该表。
* 新添加的字段值默认为 0。
* `ALTER` 不适用于分布式表和没有任何属性的表。
* 不能删除 `id` 列。
* 删除既是全文字段又是字符串属性的字段时，第一次 `ALTER DROP` 删除字符串属性，第二次删除全文字段。
* 仅在 [RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 支持添加/删除全文字段。

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

## 在实时模式下更新表的全文设置

<!-- example ALTER FT -->

```sql
ALTER TABLE table ft_setting='value'[, ft_setting2='value']
```

你可以使用 `ALTER` 修改表在[RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode) 中的全文设置。但这只影响新文档，不影响已有文档。
示例：
* 创建一个带有全文字段和 `charset_table` 的表，允许只有 3 个可搜索字符：`a`，`b` 和 `c`。
* 然后插入文档 'abcd' 并用查询 `abcd` 查找，`d` 被忽略因为它不在 `charset_table` 数组内。
* 然后我们发现需要让 `d` 也可搜索，通过 `ALTER` 将其添加进去。
* 但相同的查询 `where match('abcd')` 仍然只搜索了 `abc`，因为已有文档保留了之前的 `charset_table` 内容。
* 然后添加另一个文档 `abcd` 并再次以 `abcd` 查询。
* 现在它找到了两个文档，`show meta` 显示使用了两个关键字：`abc`（查找旧文档）和 `abcd`（查找新文档）。

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

> 注意：重命名实时表需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果不工作，请确保已安装 Buddy。

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

`ALTER` 也可以在[普通模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 下重新配置 RT 表，使新的分词、形态学和其它文本处理配置文件设置对新文档生效。注意已有文档保持不变。内部操作会强制将当前的内存块保存为新的磁盘块并调整表头，使新文档使用更新后的全文设置分词。

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

你也可以使用 `ALTER` 重新构建表的二级索引。有时，整个表或表内一个或多个属性的二级索引可能被禁用：
* 属性更新时，其二级索引会被禁用。
* 如果 Manticore 加载了已不支持的旧版本二级索引，整个表的二级索引会被禁用。

`ALTER TABLE table REBUILD SECONDARY` 会基于属性数据重新构建二级索引并再次启用它们。

此外，旧版本二级索引可能被支持但缺少部分功能，`REBUILD SECONDARY` 可用于升级二级索引。

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

该命令重新处理表中所有向量数据，并从头重建 KNN 索引。

<!-- request Example -->
```sql
ALTER TABLE rt REBUILD KNN;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在实时模式下更新属性 API 参数（用于生成嵌入）

<!-- example api_key -->

`ALTER` 可用于在使用远程模型自动生成嵌入时修改 API 参数：

```sql
ALTER TABLE table_name MODIFY COLUMN column_name API_KEY='key';
ALTER TABLE table_name MODIFY COLUMN column_name API_URL='url';
ALTER TABLE table_name MODIFY COLUMN column_name API_TIMEOUT='seconds';
```

<!-- request Example -->
```sql
ALTER TABLE rt MODIFY COLUMN vector API_KEY='new-key';
ALTER TABLE rt MODIFY COLUMN vector API_URL='https://custom-api.example.com/v1/embeddings';
ALTER TABLE rt MODIFY COLUMN vector API_TIMEOUT='30';
```

**说明：**
- `API_KEY`：在 ALTER 操作期间通过实际的 API 请求验证新密钥。
- `API_URL`：设置为空字符串 (`''`) 可回复为默认提供商端点。
- `API_TIMEOUT`：设置为 `'0'` 使用默认超时（10 秒）。必须是非负整数。

<!-- end -->

## 更改分布式表

<!-- example local_dist -->

要更改分布式表中的本地或远程节点列表，请使用与[创建表](Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)时相同的语法。只需将命令中的 `CREATE` 替换为 `ALTER`，并删除 `type='distributed'`：

```sql
ALTER TABLE `distr_table_name` [[local='local_table_name'], [agent='host:port:remote_table'] ... ]
```

> 注意：在线更改分布式表的架构需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果不生效，请确保已经安装了 Buddy。

<!-- request Example -->
```sql
ALTER TABLE local_dist local='index1' local='index2' agent='127.0.0.1:9312:remote_table';
```

<!-- end -->
<!-- proofread -->

