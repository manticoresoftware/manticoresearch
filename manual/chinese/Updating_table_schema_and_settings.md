# 更新表结构

## 在RT模式下更新表结构

<!-- example ALTER -->

```sql
ALTER TABLE table ADD COLUMN column_name [{INTEGER|INT|BIGINT|FLOAT|BOOL|MULTI|MULTI64|JSON [secondary_index='1']|STRING|TEXT [INDEXED [ATTRIBUTE]]|TIMESTAMP|FLOAT_VECTOR [KNN options]}] [engine='columnar']

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
* `float_vector` - 向量属性。您可以使用与 [`CREATE TABLE`](Creating_a_table/Data_types.md#Float-vector) 中相同的 KNN 和 auto-embedding 选项
* 为任何属性（json除外）添加`engine='columnar'`将使其存储在[列式存储](Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中

#### 重要注意事项：
* ❗建议在`ALTER`表之前**备份表文件**，以防突然断电或其他类似问题导致数据损坏。
* 添加列时无法查询表。
* 新创建的标量属性默认设置为 `0`。
* 新增的 `float_vector` 列如果没有 `MODEL_NAME`，则初始化为零向量。
* 如果添加带有 `MODEL_NAME` 和 `FROM` 的 `float_vector` 列，`ALTER TABLE ... ADD COLUMN` 期间现有行会自动嵌入。
* 指定 `MODEL_NAME` 时，`FROM` 是必需的。使用 `FROM=''` 从所有 `text` 字段和 `string` 属性中嵌入。
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

你可以使用 `ALTER` 来修改表的全文检索设置，适用于 [RT 模式](Read_this_first.md#Real-time-mode-vs-plain-mode)。不过，它只会影响新文档，不会影响已有文档。要把新设置应用到已有文档，请先[重新索引它们](Updating_table_schema_and_settings.md#Reindexing-existing-documents-after-changing-FT-settings)。
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

`ALTER TABLE ... RECONFIGURE` 会重新加载 [plain 模式](Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)下 RT 表的全文检索设置。配置文件中的新分词、形态学以及其他文本处理设置，会应用到在该命令之后插入或替换的文档。已有文档会保留使用旧设置构建的全文索引。要把新设置应用到这些文档，请[重新索引它们](Updating_table_schema_and_settings.md#Reindexing-existing-documents-after-changing-FT-settings)。该命令会将当前 RAM chunk 刷写为新的 disk chunk，并更新表头。

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

## 在更改 FT 设置后重新索引现有文档

更改全文检索设置会影响文档插入或替换时的索引过程。要把新设置应用到已有文档，请在更改设置后重新插入这些文档。这适用于通配符索引、分词、形态学、wordforms 以及其他全文检索设置。

在普通模式下，先更新表配置并运行 `ALTER TABLE <table_name> RECONFIGURE`。在 RT 模式下，按上文所述使用 `ALTER TABLE` 修改设置。在创建导出之前先暂停应用写入，并保持暂停直到回放完成；否则，回放可能会覆盖文档导出后写入的新值。

创建一个仅包含数据的转储，使其输出 `REPLACE` 语句，然后将其回放到同一个表中：

```bash
mysqldump -h0 -P9306 --replace -t -c -e --net-buffer-length=16m manticore '<table_name>' > '<table_name>-reindex.sql'
mysql -h0 -P9306 < '<table_name>-reindex.sql'
```

`-t` 会省略 `DROP` 和 `CREATE TABLE` 语句，而 `--replace` 会让导出在回放时用相同 ID 的文档替换现有文档。每个被回放的文档都会使用当前全文检索设置建立索引。

如果你不是先保存到文件，而是直接将导出内容通过管道传给 `mysql`，则必须使用 `--skip-lock-tables`：

```bash
mysqldump -h0 -P9306 --skip-lock-tables --replace -t -c -e --net-buffer-length=16m manticore '<table_name>' | mysql -h0 -P9306
```

如果不使用 `--skip-lock-tables`，`mysqldump` 在导出期间会持有读锁，因此当导出跨越多个批次时，并发的 `REPLACE` 语句可能会失败，报出 `table '<table_name>' is locked`。对于先导出到文件、再进行回放的两步工作流，则不需要该选项，因为 `mysqldump` 会在回放开始前退出并释放锁。

该表必须存储所有全文字段：`mysqldump` 无法备份包含非存储字段的表。对于复制表，请使用[复制模式备份说明](Securing_and_compacting_a_table/Backup_and_restore.md#Backup-and-restore-with-mysqldump)。

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

## 重建嵌入

<!-- example ALTER REBUILD EMBEDDINGS -->
```sql
ALTER TABLE table REBUILD EMBEDDINGS column_name
```

此命令重新生成一个目标 `float_vector` 列的嵌入，该列配置了 `MODEL_NAME` 和 `FROM`。

在需要为现有嵌入列重新生成向量时使用此功能，例如在使用 `ALTER TABLE ... ADD COLUMN` 后稍后添加该列并希望重新处理行，或希望强制为所有行重新生成向量时。

重要行为：
* 列名是必填项。该命令一次仅重建一个嵌入列。
* 为该列中的所有行重新生成嵌入，而不仅仅是向量为零的行。
* 它还会覆盖那些手动插入向量的行，以及使用 `()` 跳过生成并存储零向量的行。
* 目标列必须是带有嵌入模型配置的索引 `float_vector`。
* 允许 `FROM=''`，表示“使用所有 `text` 字段和 `string` 属性”。

Manticore 不会持久化该列中当前向量是自动生成、由用户显式提供，还是从 `()` 创建的。如果你运行 `REBUILD EMBEDDINGS`，存储的值将从配置的 `FROM` 源为该列中的每一行重新生成，包括当前值为全零向量的行。

<!-- request Example -->
```sql
ALTER TABLE products ADD COLUMN embedding FLOAT_VECTOR KNN_TYPE='hnsw' HNSW_SIMILARITY='l2' MODEL_NAME='Xenova/all-MiniLM-L6-v2' FROM='title';
ALTER TABLE products REBUILD EMBEDDINGS embedding;
```

<!-- response Example -->
```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- end -->

## 在 RT 模式下更新嵌入生成的属性 API 参数

<!-- example api_key -->

当使用远程模型进行自动嵌入时，可以使用 `ALTER` 修改 API 参数：

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

**注意事项：**
- `API_KEY`：在 ALTER 操作期间通过实际 API 请求验证新 API 密钥。
- `API_URL`：设置为空字符串 (`''`) 以恢复到默认提供方端点。
- `API_TIMEOUT`：设置为 `'0'` 以使用默认超时时间（10 秒）。必须是非负整数。

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
