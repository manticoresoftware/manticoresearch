# 实时表

**实时表** 是 Manticore 中的主要表类型。它允许您添加、更新和删除文档，并且可以立即看到这些更改。您可以通过配置文件设置实时表，或使用 `CREATE`、`UPDATE`、`DELETE` 或 `ALTER` 等命令来操作。

在内部，实时表由一个或多个称为 **chunk（块）** 的 [普通表](../../Creating_a_table/Local_tables/Plain_table.md) 组成。chunk 分为两种类型：

- 多个 **磁盘块** - 这些块保存到磁盘中，其结构类似于 [普通表](../../Creating_a_table/Local_tables/Plain_table.md)。
- 一个 **内存块** - 存储在内存中，收集所有的更改。

内存块的大小由 [rt_mem_limit](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) 设置控制。一旦达到此限制，内存块将被传输到磁盘作为磁盘块。如果磁盘块过多，Manticore 会 [合并部分块](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) 以提高性能。

### 创建实时表：

您可以通过两种方式创建新的实时表：使用 `CREATE TABLE` 命令，或通过 HTTP JSON API 的 [_mapping endpoint](../../Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:)。

#### CREATE TABLE 命令：

<!-- example rt -->

您可以通过 SQL 和 HTTP 协议使用此命令：

<!-- intro -->
##### 通过 SQL 协议创建实时表：
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology='stem_en';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### 通过 HTTP 使用 JSON 创建实时表：
<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price float)  morphology='stem_en'"
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### 通过PHP客户端创建实时表：
<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
    'price'=>['type'=>'float'],
]);
```

<!-- intro -->
##### Python:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE forum(title text, price float)')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, price float)');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### 通过配置文件创建实时表：
<!-- request CONFIG -->

```ini
table products {
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
  stored_fields = title
}
```
<!-- end -->

#### _mapping API:

> 注意：`_mapping` API 需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保 Buddy 已安装。

<!-- example rt-mapping -->

您还可以通过 `_mapping` 端点创建一个新表。此端点允许您定义类似 Elasticsearch 的表结构，并将其转换为 Manticore 表。

请求的主体必须具有以下结构：

```json
"properties"
{
  "FIELD_NAME_1":
  {
    "type": "FIELD_TYPE_1"
  },
  "FIELD_NAME_2":
  {
    "type": "FIELD_TYPE_2"
  },

  ...

  "FIELD_NAME_N":
  {
    "type": "FIELD_TYPE_M"
  }
}
```

创建表时，Elasticsearch 数据类型将根据以下规则映射到 Manticore 类型：
-    aggregate_metric => json
-    binary => string
-    boolean => bool
-    byte => int
-    completion => string
-    date => timestamp
-    date_nanos => bigint
-    date_range => json
-    dense_vector => json
-    flattened => json
-    flat_object => json
-    float => float
-    float_range => json
-    geo_point => json
-    geo_shape => json
-    half_float => float
-    histogram => json
-    integer => int
-    integer_range => json
-    ip => string
-    ip_range => json
-    keyword => string
-    knn_vector => float_vector
-    long => bigint
-    long_range => json
-    match_only_text => text
-    object => json
-    point => json
-    scaled_float => float
-    search_as_you_type => text
-    shape => json
-    short => int
-    text => text
-    unsigned_long => int
-    version => string

<!-- intro -->
##### 通过 `_mapping` 端点创建一个实时表：
<!-- request JSON -->

```JSON
POST /your_table_name/_mapping -d '
{
  "test": {
    "mappings": {
      "properties": {
        "price": {
            "type": "float"
        },
        "title": {
            "type": "text"
        }
      }
    }
  }
}
'
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```
<!-- end -->


#### CREATE TABLE LIKE:

<!-- example create-like -->

你可以创建一个实时表的副本，包含或不包含数据。如果表很大，复制数据可能会花费一些时间。复制操作是同步模式的，但如果连接中断，它会在后台继续执行。

```sql
CREATE TABLE table_name LIKE old_table_name [WITH DATA]
```

> 注意：复制表需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法正常工作，请确保 Buddy 已安装。

<!-- intro -->
##### 示例：
<!-- request SQL -->

```sql
create table products LIKE old_products;
```

<!-- intro -->
##### 示例 (WITH DATA):
<!-- request Example (WITH DATA) -->
```sql
create table products LIKE old_products WITH DATA;
```

<!-- end -->

### 👍 你可以用实时表做的事：
* [添加文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)。
* 通过 [更新](../../Quick_start_guide.md#Update) 过程更新属性和全文字段。
* [删除文档](../../Quick_start_guide.md#Delete)。
* [清空表](../../Emptying_a_table.md)。
* 使用 `ALTER` 命令在线更改表结构，如[在线更新表结构](../../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 中所述。
* 按照 [定义表](../../Creating_a_table/Local_tables/Real-time_table.md) 的说明，在配置文件中定义表。
* 使用 [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 功能自动分配ID。

### ⛔ 你不能用实时表做的事：
* 使用 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 功能导入数据。
* 连接到 [sources](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md) 以便从外部存储轻松索引数据。
* 更新 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)，因为它由实时表自动管理。

#### 实时表文件结构

下表列出了实时表中不同文件扩展名及其对应的描述：

| 扩展名   | 描述                                                         |
| -------- | ------------------------------------------------------------ |
| `.lock`  | 锁文件，确保同一时间内只有一个进程可以访问该表。             |
| `.ram`   | 表的RAM块，存储在内存中，用作变更的累加器。                  |
| `.meta`  | 实时表的头文件，定义了表的结构和设置。                       |
| `.*.sp*` | 磁盘块，存储在磁盘上，格式与普通表相同。当RAM块大小超过 `rt_mem_limit` 时，创建这些文件。 |

有关磁盘块结构的更多信息，请参考 [普通表文件结构](../../Creating_a_table/Local_tables/Plain_table.md#Plain-table-files-structure)。

<!-- proofread -->
