# 实时表

**实时表** 是 Manticore 中的一种主要表类型。它允许你添加、更新和删除文档，并且可以立即看到这些变更。你可以在配置文件中设置一个实时表，或者使用 `CREATE`、`UPDATE`、`DELETE` 或 `ALTER` 等命令。

内部，一个实时表由一个或多个称为 **chunk** 的 [普通表](../../Creating_a_table/Local_tables/Plain_table.md) 组成。chunk 有两种类型：

* 多个 **磁盘 chunk** —— 它们保存在磁盘上，结构类似于 [普通表](../../Creating_a_table/Local_tables/Plain_table.md)。
* 单个 **内存 chunk** —— 它保存在内存中，并收集所有变更。

内存 chunk 的大小由 [rt_mem_limit](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) 设置控制。一旦达到此限制，内存 chunk 会被转移到磁盘，成为一个磁盘 chunk。如果磁盘 chunk 过多，Manticore 会[合并其中的一些](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)以提升性能。

### 创建实时表：

你可以通过两种方式创建新的实时表：使用 `CREATE TABLE` 命令，或通过 HTTP JSON API 的 [_mapping 端点](../../Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:)。

#### CREATE TABLE 命令：

<!-- example rt -->

此命令可通过 SQL 和 HTTP 协议使用：

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
##### 通过 HTTP 的 JSON 创建实时表：
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
##### 通过 PHP 客户端创建实时表：
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
##### Python：
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE forum(title text, price float)')
```

<!-- intro -->
##### Python-asyncio：
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE forum(title text, price float)')
```

<!-- intro -->
##### Javascript：

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, price float)');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE forum(title text, price float)", Some(true)).await;
```

<!-- intro -->
##### Typescript：

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE forum(title text, price float)');
```

<!-- intro -->
##### Go：

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE forum(title text, price float)").Execute()
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

> 注意： `_mapping` API 需要安装 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

<!-- example rt-mapping -->

另外，你也可以通过 `_mapping` 端点创建新表。该端点允许你定义一个类似 Elasticsearch 的表结构，并将其转换为 Manticore 表。

请求体必须具有如下结构：

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

创建表时，Elasticsearch 数据类型将按照以下规则映射为 Manticore 类型：
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
##### 通过 _mapping 端点创建实时表：
<!-- request JSON -->

```JSON
POST /your_table_name/_mapping -d '
{
  "properties": {
    "price": {
        "type": "float"
    },
    "title": {
        "type": "text"
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

你可以创建实时表的副本，可以选择是否包含数据。请注意，如果表很大，复制带数据操作可能需要一些时间。复制是同步模式，但如果连接断开，它将在后台继续。

```sql
CREATE TABLE [IF NOT EXISTS] table_name LIKE old_table_name [WITH DATA]
```

> 注意：复制表需要 [Manticore Buddy](Installation/Manticore_Buddy.md)。如果不能使用，请确保已安装 Buddy。

<!-- intro -->
##### 示例：
<!-- request SQL -->

```sql
create table products LIKE old_products;
```

<!-- intro -->
##### 示例（带数据）：
<!-- request Example (WITH DATA) -->
```sql
create table products LIKE old_products WITH DATA;
```

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "create table products LIKE old_products"
```

<!-- intro -->
##### JSON 示例（带数据）：
<!-- request JSON example (WITH DATA) -->
```JSON
POST /sql?mode=raw -d "create table products LIKE old_products WITH DATA"
```

<!-- end -->

### 👍 实时表的功能：
* [添加文档](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md)。
* 使用 [Update](../../Quick_start_guide.md#Update) 过程更新属性和全文字段。
* [删除文档](../../Quick_start_guide.md#Delete)。
* [清空表](../../Emptying_a_table.md)。
* 使用 `ALTER` 命令在线更改模式，如 [在线更新表模式](../../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) 中所述。
* 在配置文件中定义表结构，详见 [定义表](../../Creating_a_table/Local_tables/Real-time_table.md)。
* 使用 [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) 功能实现自动 ID 分配。

### ⛔ 实时表不支持的操作：
* 使用 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 功能导入数据。
* 将其连接到 [sources](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md)，以便于从外部存储进行索引。
* 更新 [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)，因为它由实时表自动管理。

#### 实时表文件结构

下表列出了实时表中不同文件扩展名及其各自的描述：

| Extension | Description |
| - | - |
| `.lock` | 一个锁文件，确保一次只有一个进程能访问该表。 |
| `.ram` | 表的内存块，存储在内存中，用作更改的累加器。 |
| `.meta` | 定义实时表结构和设置的表头。 |
| `.*.sp*` | 存储在磁盘上的磁盘块，格式与普通表相同。当 RAM 块大小超过 rt_mem_limit 时创建。 |

有关磁盘块结构的更多信息，请参阅[普通表文件结构](../../Creating_a_table/Local_tables/Plain_table.md#Plain-table-files-structure)。
<!-- proofread -->

