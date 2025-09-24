# 简单和实时表设置

<!-- example config -->
## 在配置文件中定义表架构

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### 在配置文件中的简单表示例
<!-- request Plain -->

```ini
table <table name> {
  type = plain
  path = /path/to/table
  source = <source_name>
  source = <another source_name>
  [stored_fields = <comma separated list of full-text fields that should be stored, all are stored by default, can be empty>]
}
```
<!-- intro -->
##### 在配置文件中的实时表示例
<!-- request Real-time -->

```ini
table <table name> {
  type = rt
  path = /path/to/table

  rt_field = <full-text field name>
  rt_field = <another full-text field name>
  [rt_attr_uint = <integer field name>]
  [rt_attr_uint = <another integer field name, limit by N bits>:N]
  [rt_attr_bigint = <bigint field name>]
  [rt_attr_bigint = <another bigint field name>]
  [rt_attr_multi = <multi-integer (MVA) field name>]
  [rt_attr_multi = <another multi-integer (MVA) field name>]
  [rt_attr_multi_64 = <multi-bigint (MVA) field name>]
  [rt_attr_multi_64 = <another multi-bigint (MVA) field name>]
  [rt_attr_float = <float field name>]
  [rt_attr_float = <another float field name>]
  [rt_attr_float_vector = <float vector field name>]
  [rt_attr_float_vector = <another float vector field name>]
  [rt_attr_bool = <boolean field name>]
  [rt_attr_bool = <another boolean field name>]
  [rt_attr_string = <string field name>]
  [rt_attr_string = <another string field name>]
  [rt_attr_json = <json field name>]
  [rt_attr_json = <another json field name>]
  [rt_attr_timestamp = <timestamp field name>]
  [rt_attr_timestamp = <another timestamp field name>]

  [stored_fields = <comma separated list of full-text fields that should be stored, all are stored by default, can be empty>]

  [rt_mem_limit = <RAM chunk max size, default 128M>]
  [optimize_cutoff = <max number of RT table disk chunks>]

}
```
<!-- end -->

### 常见的简单和实时表设置

#### 类型

```ini
type = plain

type = rt
```

表类型: "plain" 或 "rt" (实时)

值: **plain** (默认), rt

#### 路径

```ini
path = path/to/table
```

表将存储或位于的位置的路径，可以是绝对路径或相对路径，不带扩展名。

值: 表的路径, **必需**

#### 存储字段

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，在配置文件中定义表时，全文字段的原始内容会被索引并存储。此设置允许您指定应存储其原始值的字段。

值: 应该存储的 **全文** 字段的以逗号分隔的列表。空值（即 `stored_fields =` ）会禁用所有字段的原始值存储。

注意: 在实时表的情况下，`stored_fields`中列出的字段也应声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

此外，请注意，您不需要在 `stored_fields` 中列出属性，因为它们的原始值无论如何都会被存储。 `stored_fields` 只能用于全文字段。

另请参见 [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) 以获取文档存储压缩选项。


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'columns' => [
            'title'=>['type'=>'text'],
            'content'=>['type'=>'text', 'options' => ['indexed', 'stored']],
            'name'=>['type'=>'text', 'options' => ['indexed']],
            'price'=>['type'=>'float']
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### Python:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)", Some(true)).await;
```

<!-- intro -->
##### Typescript:

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### Go:

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)").Execute()
```

<!-- request CONFIG -->

```ini
table products {
  stored_fields = title, content # we want to store only "title" and "content", "name" shouldn't be stored

  type = rt
  path = tbl
  rt_field = title
  rt_field = content
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

#### 仅存储字段

```ini
stored_only_fields = title,content
```

使用 `stored_only_fields` 当您希望 Manticore 将一些纯文本或实时表的字段 **存储在磁盘上但不进行索引**。这些字段无法通过全文查询进行搜索，但您仍然可以在搜索结果中检索它们的值。

例如，如果您想存储如 JSON 文档之类的数据，而这些数据应该与每个结果一起返回，但不需要被搜索、过滤或分组，这非常有用。在这种情况下，仅存储它们而不对其进行索引可以节省内存并提高性能。

您可以通过两种方法做到这一点：
- 在表配置中的 [纯文本模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 下，使用 `stored_only_fields` 设置。
- 在 SQL 接口中 ([实时模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29))，在定义文本字段时使用 [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) 属性（而不是 `indexed` 或 `indexed stored`）。在 SQL 中，您不需要在 `CREATE TABLE` 语句中包含 `stored_only_fields` — 它不被支持。

`stored_only_fields` 的值是一个以逗号分隔的字段名称列表。默认情况下，它是空的。如果您使用的是实时表，`stored_only_fields` 中列出的每个字段还必须声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

注意：您不需要在 `stored_only_fields` 中列出属性，因为它们的原始值无论如何都会被存储。

仅存储字段与字符串属性的比较：

- **仅存储字段**：
  - 仅存储在磁盘上
  
  - 压缩格式

  
  - 只能被检索（不能用于过滤、排序等）
- **字符串属性**：
  - 存储在磁盘和内存中

  

  - 未压缩格式（除非您使用列式存储）

  

  - 可用于排序、过滤、分组等
如果您希望 Manticore 为您存储文本数据，而您 **仅** 想将其存储在磁盘上（例如：每个结果都返回的 JSON 数据），而不在内存中或可搜索/可过滤/可分组，请使用 `stored_only_fields`，或将 `stored` 作为您的文本字段属性。
在使用 SQL 接口创建表时，将您的文本字段标记为 `stored`（而不是 `indexed` 或 `indexed stored`）。在您的 `CREATE TABLE` 语句中不需要 `stored_only_fields` 选项；包含它可能会导致查询失败。

#### json_secondary_indexes

```ini

json_secondary_indexes = json_attr

```

<!-- example json_secondary_indexes -->
默认情况下，所有属性（除 JSON 属性外）都会生成二级索引。然而，可以使用 `json_secondary_indexes` 设置显式生成 JSON 属性的二级索引。当 JSON 属性包含在该选项中时，其内容将被扁平化成多个二级索引。这些索引可以被查询优化器用于加速查询。

您可以使用 [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) 命令查看可用的二级索引。

值: 应该生成二级索引的 JSON 属性的以逗号分隔的列表。
<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, j json secondary_index='1')
```
<!-- request JSON -->

```JSON

POST /cli -d "
CREATE TABLE products(title text, j json secondary_index='1')"
```
<!-- request PHP -->
```php
$params = [
    'body' => [
        'columns' => [
            'title'=>['type'=>'text'],
            'j'=>['type'=>'json', 'options' => ['secondary_index' => 1]]
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### Python:

<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```
<!-- intro -->
##### Python-asyncio:

<!-- request Python-asyncio -->
```python

await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```
<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```
<!-- intro -->
##### Java:

<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, j json secondary_index='1')");
```
<!-- intro -->
##### C#:

<!-- request C# -->
```clike

utilsApi.Sql("CREATE TABLE products(title text, j json secondary_index='1')");

```
<!-- intro -->
##### Rust:

<!-- request Rust -->
```rust

utils_api.sql("CREATE TABLE products(title text, j json secondary_index='1')", Some(true)).await;
```
<!-- intro -->
##### Typescript:

<!-- request Typescript -->
```typescript

res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```
<!-- intro -->
##### Go:


<!-- request Go -->

```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, j json secondary_index='1')").Execute()
```

<!-- request CONFIG -->
```ini
table products {
  json_secondary_indexes = j
  type = rt
  path = tbl
  rt_field = title

  rt_attr_json = j

}

```
<!-- end -->
### 实时表设置：

#### diskchunk_flush_search_timeout

```ini

diskchunk_flush_search_timeout = 10s
```
防止自动刷新 RAM 块的超时时间，如果表中没有搜索。详情见 [这里](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

#### diskchunk_flush_write_timeout

```ini

diskchunk_flush_write_timeout = 60s

```

如果没有写入操作，自动刷新 RAM 块的超时时间。详情见 [这里](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)。
#### optimize_cutoff
RT 表的最大磁盘块数量。详情见 [这里](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。

#### rt_field

```ini

rt_field = subject

```
此字段声明确定将要索引的全文字段。字段名必须唯一，且保持顺序。当插入数据时，字段值必须按配置中指定的顺序排列。
这是一个多值的可选字段。

#### rt_attr_uint

```ini

rt_attr_uint = gid

```
此声明定义了一个无符号整数属性。
值：字段名或者 field_name:N（其中 N 是要保留的最大位数）。

#### rt_attr_bigint

```ini

rt_attr_bigint = gid

```
此声明定义了一个 BIGINT 属性。
值：字段名，允许多条记录。

#### rt_attr_multi

```ini

rt_attr_multi = tags

```
声明一个多值属性（MVA），包含无符号 32 位整数值。
值：字段名。允许多条记录。

#### rt_attr_multi_64

```ini

rt_attr_multi_64 = wide_tags

```
声明一个多值属性（MVA），包含有符号 64 位 BIGINT 值。
值：字段名。允许多条记录。
#### rt_attr_float

```ini

rt_attr_float = lat

rt_attr_float = lon

```
声明单精度浮点属性，采用 32 位 IEEE 754 格式。
值：字段名。允许多条记录。
#### rt_attr_float_vector

```ini

rt_attr_float_vector = image_vector

rt_attr_float_vector = text_vector

```

声明一个浮点向量属性，用于存储嵌入向量并支持 k 最近邻（KNN）向量搜索。

值：字段名。允许多条记录。

每个向量属性存储一个浮点数数组，表示数据（如文本、图像或其他内容）的高维向量。这些向量通常由机器学习模型生成，可用于相似度搜索、推荐、语义搜索及其他 AI 功能。
**重要：** 浮点向量属性需要额外的 KNN 配置以启用向量搜索功能。
##### 配置向量属性的 KNN
要对浮点向量属性启用 KNN 搜索，必须添加一个 `knn` 配置，指定索引参数：
```ini

rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```
**必需的 KNN 参数：**

- `name`：向量属性名称（必须与 `rt_attr_float_vector` 名称匹配）
- `type`：索引类型，目前仅支持 `"hnsw"`
- `dims`：向量维度数（必须与您的嵌入模型输出匹配）

- `hnsw_similarity`：距离函数，取值为 `"L2"`、`"IP"`（内积）或 `"COSINE"`

**可选 KNN 参数：**

- `hnsw_m`：图中的最大连接数
- `hnsw_ef_construction`：构建时的准确性和时间折中
更多关于 KNN 向量搜索的详情，请参阅 [KNN 文档](../../Searching/KNN.md)。

#### rt_attr_bool

```ini

rt_attr_bool = available

```
声明具有 1 位无符号整数值的布尔属性。
值：字段名。

#### rt_attr_string

```ini

rt_attr_string = title

```
字符串属性声明。
值：字段名。

#### rt_attr_json

```ini

rt_attr_json = properties

```
声明 JSON 属性。
值：字段名。

#### rt_attr_timestamp

```ini

rt_attr_timestamp = date_added

```
声明时间戳属性。
值：字段名。

#### rt_mem_limit

```ini

rt_mem_limit = 512M

```
表的 RAM 块的内存限制。可选，默认值为 128M。
RT 表将部分数据存储在内存中，称为“RAM 块”，也维护多个磁盘表，称为“磁盘块”。该指令允许您控制 RAM 块的大小。当内存中数据过多时，RT 表将把数据刷写到磁盘，激活新创建的磁盘块，并重置 RAM 块。
请注意，该限制是严格的，RT 表绝不会分配超过 rt_mem_limit 指定的内存。此外，内存不会预分配，因此指定 512MB 限制但仅插入 3MB 数据时，仅分配 3MB，而非 512MB。
`rt_mem_limit` 不会被超出，但实际的 RAM 块大小可能远低于限制。RT 表根据数据插入速度动态调整实际限制，以最小化内存使用并最大化写入速度。具体流程如下：
* 默认情况下，RAM 块大小为 `rt_mem_limit` 的 50%，称为 "`rt_mem_limit` 限制"。

* 当 RAM 块积累的数据达到 `rt_mem_limit * rate`（默认是 `rt_mem_limit` 的 50%）时，Manticore 开始将 RAM 块保存为新的磁盘块。

* 保存新磁盘块期间，Manticore 会评估新增/更新文档数目。

* 保存完成后，更新 `rt_mem_limit` 的速率。

* 每次重启 searchd 时，速率重置为 50%。

例如，如果保存到磁盘块的数据为 90MB，且在保存过程中又有 10MB 数据到达，则速率为 90%。下一次 RT 表将在达到 `rt_mem_limit` 的 90% 时刷新数据。插入速度越快，`rt_mem_limit` 速率越低。速率范围为 33.3% 至 95%。你可以通过 [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 命令查看表的当前速率。

##### 如何更改 rt_mem_limit 和 optimize_cutoff
在实时模式下，您可以使用 `ALTER TABLE` 语句调整 RAM 块的大小限制和磁盘块的最大数量。要将表 "t" 的 `rt_mem_limit` 设置为 1GB，请运行以下查询：`ALTER TABLE t rt_mem_limit='1G'`。要更改磁盘块的最大数量，请运行查询：`ALTER TABLE t optimize_cutoff='5'`。
在普通模式下，您可以通过更新表配置或运行命令 `ALTER TABLE <table_name> RECONFIGURE` 来更改 `rt_mem_limit` 和 `optimize_cutoff` 的值。
##### 有关 RAM 块的重要说明
* 实时表类似于 [分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)，由多个本地表（也称为磁盘块）组成。
* 每个 RAM 块由多个段组成，这些段是仅在 RAM 中存在的特殊表。
* 虽然磁盘块存储在磁盘上，但 RAM 块存储在内存中。
* 对实时表进行的每次事务会生成一个新段，RAM 块的各段在每次事务提交后都会合并。进行数百或数千条数据的批量插入（bulk INSERT）比多次单独插入一条数据效率更高，因为这样可以减少 RAM 块分段合并带来的开销。
* 当分段数量超过 32 时，系统会进行合并操作，以使分段数保持在 32 以内。
* 实时表始终有一个 RAM 块（可能为空）和一个或多个磁盘块。
* 合并大型分段所需时间更长，因此最好避免 RAM 块过大（即 `rt_mem_limit` 太大）。
* 磁盘块的数量取决于表中的数据量以及 `rt_mem_limit` 设置。
* searchd 会在关闭时以及根据 [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) 设置周期性地将 RAM 块刷新到磁盘（以持久化文件形式，而不是磁盘块形式）。将数 GB 的数据刷新到磁盘可能需要一些时间。

* 大型 RAM 块在刷新到 `.ram` 文件以及 RAM 块已满并转储为磁盘块时都会对存储造成更大压力。

* 在 RAM 块刷新到磁盘之前，它由 [二进制日志](../../Logging/Binary_logging.md) 备份。更高的 `rt_mem_limit` 会增加重放二进制日志和恢复 RAM 块所需的时间。

* RAM 块可能会比磁盘块略慢。

* 虽然 RAM 块本身只占用不超过 `rt_mem_limit` 的内存，但在某些情况下，Manticore 可能会占用更多内存，例如在插入数据的事务尚未提交时，您已传输的数据会持续留在内存中。

##### RAM 块刷新条件

除了 `rt_mem_limit` 外，RAM 块的刷新行为还受到以下选项和条件的影响：
* 冻结状态。如果表被 [冻结](../../Securing_and_compacting_a_table/Freezing_a_table.md)，则刷新会被延迟。这是一条永久规则，无法被覆盖。如果表被冻结时达到 `rt_mem_limit` 条件，所有进一步插入将被延迟，直到表解冻。

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)：该选项定义在无写入的情况下，RAM 块自动刷新的超时时间（秒）。如果在此时间内无写入发生，RAM 块将被刷到磁盘。设置为 `-1` 时，基于写入活动的自动刷新将被禁用。默认值为1秒。
* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)：该选项设置如果没有对表进行查询时，防止自动刷新 RAM 块的超时时间（秒）。只有在此时间内至少发生过一次查询，自动刷新才会发生。默认值为30秒。

* 正在进行优化：如果当前正在运行优化进程，并且现有磁盘块数量已

  达到或超过配置的内部 `cutoff` 阈值，则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过。

* RAM 块段中文档数过少：如果各 RAM 段中的文档总数低于最小阈值（8192），

  则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过，以避免产生过小的磁盘块。这有助于减少不必要的磁盘写入和块碎片。
这些超时是协同工作的。只要其中*任意一个*超时，RAM 块就会被刷新。这确保即使没有写入，数据最终也会被持久保存到磁盘，反之，即使持续写入但没有查询，数据同样会被持久化。这些设置为 RAM 块刷新频率提供了更细粒度的控制，实现数据持久性与性能的平衡。按表指定的设置具有更高优先级，可覆盖实例级默认值。
### 普通表设置：
#### source
```ini

source = srcpart1

source = srcpart2

source = srcpart3

```
source 字段指定在当前表建立索引时从哪一个来源获取文档。至少需要指定一个来源。来源可以是不同类型（例如，一个可以是 MySQL，另一个可以是 PostgreSQL）。有关从外部存储建立索引的详细信息，[请阅读这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)
取值：**必须**提供来源名称。允许多个值。

#### killlist_target

```ini

killlist_target = main:kl

```
此设置确定将在哪个表上应用 kill-list。当前表中更新或删除的目标表中的匹配项将被抑制。在`:kl`模式下，要抑制的文档取自 [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在`:id`模式下，当前表中的所有文档 ID 会在目标表中被抑制。如果两者都未指定，则两种模式都会生效。[在这里了解有关 kill-list 的更多信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)
值：**未指定**（默认）、target_table_name:kl、target_table_name:id、target_table_name。允许多个值
#### columnar_attrs

```ini

columnar_attrs = *

columnar_attrs = id, attr1, attr2, attr3

```

此配置设置确定哪些属性应存储在[列式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中，而不是行级存储中。
您可以设置`columnar_attrs = *`以将所有支持的数据类型存储在列式存储中。
此外，`id` 是支持存储在列式存储中的属性。

#### columnar_strings_no_hash

```ini

columnar_strings_no_hash = attr1, attr2, attr3
```

默认情况下，存储在列式存储中的所有字符串属性都存储预计算的哈希值。这些哈希用于分组和过滤。但是，它们占用了额外的空间，如果您不需要按该属性分组，可以通过禁用哈希生成来节省空间。
### 通过 CREATE TABLE 在线创建实时表
<!-- example rt_mode -->

##### CREATE TABLE 的通用语法

```sql

CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```
##### 数据类型：
有关数据类型的更多信息，请参阅[有关数据类型的更多内容](../../Creating_a_table/Data_types.md)。
| 类型 | 配置文件中的等价项 | 说明 | 别名 |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | 选项：indexed, stored。默认：**两者**。若只需存储文本但不索引，指定“stored”。若只需索引文本，指定“indexed”。 | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| 整数	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| 大整数	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | 浮点数  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点向量  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | 多值整数 |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多值大整数  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值 |   |

| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项 `indexed, attribute` 会使该值同时具备全文索引和可过滤、排序、分组属性  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳  |   |

| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N 是要保留的最大位数  |   |
<!-- intro -->
##### 通过 CREATE TABLE 创建实时表的示例

<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```
这将创建“products”表，其中包含两个字段：“title”（全文索引）和“price”（浮点数），并将“morphology”设置为“stem_en”。
```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
这将创建“products”表，包含三个字段：


* “title”被索引，但不存储。

* “description”被存储，但不索引。
* “author”既存储又索引。
<!-- end -->
## 引擎

```ini

create table ... engine='columnar';

create table ... engine='rowwise';
```
engine 设置更改表中所有属性的默认[属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。您也可以为[每个属性单独指定 `engine`](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages)。


有关如何为普通表启用列式存储的信息，请参阅 [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)。
值：

* columnar - 为所有表属性启用列式存储，JSON 除外。

* **rowwise（默认）** - 不更改任何设置，使用传统的行式存储。
## 其他设置

以下设置适用于实时表和普通表，无论是在配置文件中指定还是使用 `CREATE` 或 `ALTER` 命令在线设置。

### 性能相关

#### 访问表文件

Manticore 支持两种读取表数据的访问模式：seek+read 和 mmap。
在 seek+read 模式下，服务器使用 `pread` 系统调用读取文档列表和关键词位置，分别由 `*.spd` 和 `*.spp` 文件表示。服务器使用内部读缓冲区来优化读取过程，这些缓冲区的大小可以通过选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 进行调整。还有一个选项 [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) 控制 Manticore 启动时如何打开文件。
在 mmap 访问模式下，搜索服务器使用 `mmap` 系统调用将表文件映射到内存中，操作系统缓存文件内容。选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 对该模式下的对应文件无效。mmap 读取器还可以使用特权调用 `mlock` 锁定表数据在内存中，防止操作系统将缓存数据换出到磁盘。
要控制使用哪种访问模式，可以使用选项 **access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists** 和 **access_dict**，其取值如下：
| 值 | 描述 |
| - | - |


| file | 服务器使用 seek+read 结合内部缓冲区从磁盘读取表文件 |
| mmap | 服务器将表文件映射到内存，操作系统缓存文件内容 |
| mmap_preread | 服务器将表文件映射到内存，后台线程预读一次以预热缓存 |
| mlock | 服务器将表文件映射到内存，并调用 mlock() 系统调用缓存文件内容且将其锁定在内存中以防止被换出 |
| 设置项 | 可选值 | 描述 |
| - | - | - |
| access_plain_attrs  | mmap，**mmap_preread**（默认），mlock | 控制如何读取 `*.spa`（普通属性）`*.spe`（跳跃列表）`*.spt`（查找表）`*.spm`（已删除文档） |

| access_blob_attrs   | mmap，**mmap_preread**（默认），mlock | 控制如何读取 `*.spb`（二进制大对象属性，如字符串、多值和 JSON 属性） |

| access_doclists     | **file**（默认），mmap，mlock | 控制如何读取 `*.spd`（文档列表）数据 |
| access_hitlists     | **file**（默认），mmap，mlock | 控制如何读取 `*.spp`（命中列表）数据 |
| access_dict         | mmap，**mmap_preread**（默认），mlock | 控制如何读取 `*.spi`（字典） |
以下表格可以帮助你选择所需的模式：
| 表部分 | 保留于磁盘 | 保留于内存 | 服务器启动时缓存于内存 | 锁定于内存 |
| - | - | - | - | - |
| [行存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)（非列存储）的普通属性，跳跃列表，词列表，查找表，已删除文档 | mmap | mmap | **mmap_preread**（默认） | mlock |
| 行存储的字符串、多值属性（MVA）和 JSON 属性 | mmap | mmap | **mmap_preread**（默认） | mlock |

| [列存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 的数值、字符串和多值属性 | 始终 | 仅通过操作系统 | 否 | 不支持 |

| 文档列表 | **file**（默认） | mmap | 否 | mlock |
| 命中列表 | **file**（默认） | mmap | 否 | mlock |
| 字典 | mmap | mmap | **mmap_preread**（默认） | mlock |
##### 推荐说明：
* 若追求**最快的搜索响应时间**并且内存充足，推荐使用 [行存储](../../Creating_a_table/Data_types.md#JSON) 属性并通过 `mlock` 锁定到内存中。同时，对 doclists/hitlists 使用 mlock。

* 如果你优先考虑**启动后性能不会降低**且能接受更长的启动时间，可使用 [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options) 选项。若希望加快 searchd 重启，则保持默认的 `mmap_preread` 选项。
* 如果想**节省内存**，同时确保有足够内存容纳所有属性，则避免使用 `mlock`。操作系统会根据频繁磁盘读写决定哪些保留在内存。
* 如果行存储属性**无法全部放入内存**，可选择使用 [列存储属性](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。
* 如果对全文检索**性能无特别要求**，且想节省内存，可设置 `access_doclists/access_hitlists=file`。
默认模式综合了：

* mmap 映射，

* 非列存储属性的预读，

* 列存储属性的 seek+read，无预读，

* doclists/hitlists 的 seek+read，无预读。
该模式在大部分场景下提供了良好的搜索性能、内存利用率和较快的 searchd 重启速度。
### 其他性能相关设置

#### attr_update_reserve

```ini

attr_update_reserve = 256k

```
该设置为对 blob 属性（如多值属性 MVA、字符串和 JSON）更新时预留额外空间。默认值为 128k。更新这些属性时，长度可能变化。如果更新后的字符串比之前短，新的数据会覆盖 `*.spb` 文件中的旧数据；如果长，则写入 `*.spb` 文件末尾。该文件为内存映射文件，调整大小可能会比较缓慢，取决于操作系统的内存映射文件实现。使用此设置可避免频繁调整文件大小，通过在 .spb 文件末尾预留额外空间实现。
取值：大小，默认 **128k**。

#### docstore_block_size

```ini

docstore_block_size = 32k

```
此设置控制文档存储中使用的块大小。默认值为16kb。当使用stored_fields或stored_only_fields存储原始文档文本时，文本会存储在表中并进行压缩以提高效率。为了优化磁盘访问和小文档的压缩比，这些文档会被连接成块。索引过程会收集文档，直到其总大小达到此选项指定的阈值。此时，文档块会被压缩。此选项可以调整以实现更好的压缩比（通过增加块大小）或更快地访问文档文本（通过减小块大小）。
值：大小，默认值 **16k**。

#### docstore_compression

```ini

docstore_compression = lz4hc

```
此设置决定用于压缩存储在文档存储中的文档块的压缩类型。如果指定了stored_fields或stored_only_fields，文档存储会存储压缩的文档块。‘lz4’提供快速的压缩和解压速度，而‘lz4hc’（高压缩）牺牲部分压缩速度以换取更好的压缩比。‘none’则完全禁用压缩。
取值：**lz4**（默认），lz4hc，none。

#### docstore_compression_level

```ini

docstore_compression_level = 12

```
在文档存储中使用‘lz4hc’压缩时的压缩级别。通过调整压缩级别，可以在使用‘lz4hc’压缩时获得性能和压缩比之间的合理平衡。请注意，此选项在使用‘lz4’压缩时不起作用。
值：1到12之间的整数，默认值为 **9**。

#### preopen

```ini

preopen = 1

```
此设置指示searchd是否应在启动或轮换时打开所有表文件，并在运行时保持打开状态。默认情况下，文件不预先打开。预先打开的表每个表需要几个文件描述符，但它消除了每次查询调用open()的需要，并且避免了高负载下表轮换时可能发生的竞争条件。然而，如果您服务于许多表，按查询打开它们可能更有效，以节省文件描述符。
值：**0**（默认），或 1。

#### read_buffer_docs

```ini

read_buffer_docs = 1M

```
用于存储每个关键字文档列表的缓冲区大小。增加此值将导致查询执行时内存使用增加，但可能减少I/O时间。
值：大小，默认 **256k**，最小值为8k。

#### read_buffer_hits

```ini

read_buffer_hits = 1M

```

用于存储每个关键字命中的缓冲区大小。增加此值将导致查询执行时内存使用增加，但可能减少I/O时间。

值：大小，默认 **256k**，最小值为8k。
### 纯表磁盘占用设置
#### inplace_enable

<!-- example inplace_enable -->

```ini

inplace_enable = {0|1}

```


启用原地表反转。可选，默认值为0（使用单独的临时文件）。
`inplace_enable`选项在索引纯表时减少磁盘占用，同时略微降低索引速度（使用大约2倍少的磁盘，但性能约为原始性能的90-95%）。

索引包含两个主要阶段。第一阶段，收集文档、处理并按关键字部分排序，中间结果写入临时文件（.tmp*）。第二阶段，文档完全排序，创建最终表文件。在运行时重建生产表需要大约3倍的峰值磁盘占用：首先是中间临时文件，其次是新构建的副本，第三是在此期间提供生产查询的旧表。（中间数据大小与最终表相当。）对于大型数据集合，这可能占用过多磁盘空间，而`inplace_enable`选项可用于减少它。启用后，它会重用临时文件，将最终数据输出回这些文件，并在完成后重命名它们。但这可能需要额外的临时数据块重定位，导致性能影响。

该指令对[searchd](../../Starting_the_server/Manually.md)无效，仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)。
<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->
```ini
table products {
  inplace_enable = 1
  path = products

  source = src_base

}

```
<!-- end -->
#### inplace_hit_gap

<!-- example inplace_hit_gap -->

```ini

inplace_hit_gap = size
```

[原地反转](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable)的微调选项。控制预分配的命中列表间隙大小。可选，默认值为0。

该指令仅影响[searchd](../../Starting_the_server/Manually.md)工具，对[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)无影响。
<!-- intro -->
##### CONFIG:
<!-- request CONFIG -->

```ini
table products {
  inplace_hit_gap = 1M
  inplace_enable = 1
  path = products

  source = src_base

}

```
<!-- end -->
#### inplace_reloc_factor

<!-- example inplace_reloc_factor -->

```ini

inplace_reloc_factor = 0.1
```

inplace_reloc_factor设置确定索引期间在内存区中使用的重定位缓冲区大小。默认值为0.1。

此选项为可选，仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)工具，而不影响[searchd](../../Starting_the_server/Manually.md)服务器。
<!-- intro -->
##### CONFIG:
<!-- request CONFIG -->

```ini
table products {
  inplace_reloc_factor = 0.1
  inplace_enable = 1
  path = products

  source = src_base

}

```
<!-- end -->
#### inplace_write_factor

<!-- example inplace_write_factor -->

```ini


inplace_write_factor = 0.1
```

控制索引期间用于原地写入的缓冲区大小。可选，默认值为0.1。

请注意，该指令仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)工具，不影响[searchd](../../Starting_the_server/Manually.md)服务器。
<!-- intro -->
##### CONFIG:
<!-- request CONFIG -->

```ini
table products {
  inplace_write_factor = 0.1
  inplace_enable = 1
  path = products

  source = src_base
}
```
<!-- end -->
### 自然语言处理特定设置
支持以下设置。它们均描述在章节 [NLP 和分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) 中。
* [bigram_freq_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)
* [bigram_index](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)
* [blend_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)
* [blend_mode](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)
* [charset_table](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#charset_table)
* [dict](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#dict)
* [embedded_limit](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit)
* [exceptions](../../Creating_a_table/NLP_and_tokenization/Exceptions.md)
* [expand_keywords](../../Searching/Options.md#expand_keywords)
* [global_idf](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#global_idf)
* [hitless_words](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#hitless_words)
* [html_index_attrs](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_index_attrs)
* [html_remove_elements](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements)
* [html_strip](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)
* [ignore_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)
* [index_exact_words](../../Creating_a_table/NLP_and_tokenization/Morphology.md#index_exact_words)
* [index_field_lengths](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_field_lengths)
* [index_sp](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)
* [index_token_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter)
* [index_zones](../../Creating_a_table/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)
* [infix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#infix_fields)
* [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target)
* [max_substring_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#max_substring_len)
* [min_infix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)
* [min_prefix_len](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)
* [min_stemming_len](../../Creating_a_table/NLP_and_tokenization/Morphology.md#min_stemming_len)
* [min_word_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)
* [morphology](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology)
* [morphology_skip_fields](../../Creating_a_table/NLP_and_tokenization/Morphology.md#morphology_skip_fields)
* [ngram_chars](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
* [ngram_len](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)
* [overshort_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#overshort_step)
* [phrase_boundary](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)
* [phrase_boundary_step](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)
* [prefix_fields](../../Creating_a_table/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields)
* [regexp_filter](../../Creating_a_table/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
* [stopwords](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)
* [stopword_step](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step)
* [stopwords_unstemmed](../../Creating_a_table/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed)

