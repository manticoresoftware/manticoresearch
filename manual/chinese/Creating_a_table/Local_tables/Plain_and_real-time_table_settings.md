# 普通表和实时表设置

<!-- example config -->
## 在配置文件中定义表结构

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### 配置文件中普通表的示例
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
##### 配置文件中实时表的示例
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

### 普通表和实时表的通用设置

#### type

```ini
type = plain

type = rt
```

表类型： "plain" 或 "rt"（实时）

值：**plain**（默认），rt

#### path

```ini
path = path/to/table
```

表将被存储或定位的路径，可以是绝对路径或相对路径，不带扩展名。

值：表的路径，**必填**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，当在配置文件中定义表时，全文字段的原始内容会被索引并存储。此设置允许您指定应存储其原始值的字段。

值：应存储的**全文**字段的逗号分隔列表。空值（即 `stored_fields =`）会禁用所有字段的原始值存储。

注意：对于实时表，`stored_fields` 中列出的字段也应声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另外请注意，您不需要在 `stored_fields` 中列出属性，因为它们的原始值无论如何都会被存储。`stored_fields` 仅适用于全文字段。

另请参阅 [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size)，[docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) 以获取文档存储压缩选项。


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

#### stored_only_fields

```ini
stored_only_fields = title,content
```

当您希望 Manticore 存储普通表或实时表的某些字段**到磁盘但不进行索引**时，请使用 `stored_only_fields`。这些字段无法通过全文查询进行搜索，但您仍然可以在搜索结果中检索它们的值。

例如，如果您想存储像 JSON 文档这样的数据，这些数据应随每个结果返回，但不需要进行搜索、过滤或分组，这时仅存储而不索引可以节省内存并提高性能。

您可以通过以下两种方式实现：
- 在表配置的 [plain 模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 中，使用 `stored_only_fields` 设置。
- 在 SQL 接口（[RT 模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)）中，定义文本字段时使用 [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) 属性（而不是 `indexed` 或 `indexed stored`）。在 SQL 中，您不需要包含 `stored_only_fields` —— 它不支持在 `CREATE TABLE` 语句中使用。

`stored_only_fields` 的值是字段名称的逗号分隔列表。默认情况下为空。如果您使用的是实时表，`stored_only_fields` 中列出的每个字段也必须声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

注意：您不需要在 `stored_only_fields` 中列出属性，因为它们的原始值无论如何都会被存储。

仅存储字段与字符串属性的比较：

- **仅存储字段**：
  - 仅存储在磁盘上
  - 压缩格式
  - 仅能检索（不能用于过滤、排序等）

- **字符串属性**：
  - 存储在磁盘和内存中
  - 非压缩格式（除非使用列式存储）
  - 可用于排序、过滤、分组等

如果您希望 Manticore 存储您**仅**需要存储在磁盘上的文本数据（例如：随每个结果返回的 JSON 数据），而不需要存储在内存中或进行搜索/过滤/分组，请使用 `stored_only_fields`，或在文本字段属性中使用 `stored`。

当使用 SQL 接口创建表时，将您的文本字段标记为 `stored`（而不是 `indexed` 或 `indexed stored`）。您在 `CREATE TABLE` 语句中不需要 `stored_only_fields` 选项；包含它可能导致查询失败。

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

默认情况下，除 JSON 属性外的所有属性都会生成次级索引。但是，可以使用 `json_secondary_indexes` 设置显式生成 JSON 属性的次级索引。当 JSON 属性包含在此选项中时，其内容会被展平为多个次级索引。这些索引可以被查询优化器用于加速查询。

您可以使用 [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) 命令查看可用的次级索引。

值：应生成次级索引的 JSON 属性的逗号分隔列表。

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

防止在表中没有搜索时自动刷新RAM块的超时时间。了解更多 [here](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

在没有写入时自动刷新RAM块的超时时间。了解更多 [here](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)。

#### optimize_cutoff

实时表的最大磁盘块数量。了解更多 [here](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。

#### rt_field

```ini
rt_field = subject
```

该字段声明确定将被索引的全文字段。字段名称必须唯一，并保留顺序。插入数据时，字段值必须与配置中指定的顺序一致。

这是一个多值、可选字段。

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

此声明定义一个无符号整数属性。

值：字段名称或 field_name:N（其中 N 是要保留的最大位数）。

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

此声明定义一个 BIGINT 属性。

值：字段名称，允许多条记录。

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

声明一个包含无符号 32 位整数值的多值属性（MVA）。

值：字段名称。允许多条记录。

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

声明一个包含带符号 64 位 BIGINT 值的多值属性（MVA）。

值：字段名称。允许多条记录。

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

声明单精度 32 位 IEEE 754 格式的浮点属性。

值：字段名称。允许多条记录。

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

声明用于存储嵌入向量并启用最近邻（KNN）向量搜索的浮点值向量。

值：字段名称。允许多条记录。

每个向量属性存储代表数据（如文本、图像或其他内容）的高维向量数组。这些向量通常由机器学习模型生成，可用于相似性搜索、推荐、语义搜索和其他 AI 功能。

**重要：** 浮点向量属性需要额外的 KNN 配置以启用向量搜索功能。

##### 为向量属性配置 KNN

要在浮点向量属性上启用 KNN 搜索，必须添加一个指定索引参数的 `knn` 配置。您可以通过以下两种方式配置 KNN：

**1. 手动向量插入**（您提供预计算的向量）：

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**2. 自动嵌入**（Manticore 自动从文本生成向量）：

```ini
rt_attr_float_vector = embedding_vector
rt_field = title
rt_field = description
knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"sentence-transformers/all-MiniLM-L6-v2","from":"title"}]}
```

**必需的 KNN 参数：**
- `name`：向量属性的名称（必须与 `rt_attr_float_vector` 的名称匹配）
- `type`：索引类型，目前仅支持 `"hnsw"`
- `dims`：向量的维度数。**手动向量插入时必需**，**使用 `model_name` 时必须省略**（模型会自动确定维度）
- `hnsw_similarity`：距离函数 - `"L2"`、`"IP"`（内积）或 `"COSINE"`

**可选的 KNN 参数：**
- `hnsw_m`：图中的最大连接数（默认：16）
- `hnsw_ef_construction`：构建时间/准确性权衡（默认：200）

**自动嵌入参数**（当使用 `model_name` 时）：
- `model_name`：使用的嵌入模型（例如，`"sentence-transformers/all-MiniLM-L6-v2"`、`"openai/text-embedding-ada-002"`）。指定后，`dims` 必须省略，因为模型会自动确定维度。
- `from`：用于生成嵌入的字段名称列表（逗号分隔），或空字符串 `""` 表示使用所有文本/字符串字段。当指定 `model_name` 时，此参数是必需的。
- `api_key`：基于 API 的模型（OpenAI、Voyage、Jina）的 API 密钥。仅在使用基于 API 的嵌入服务时需要。
- `cache_path`：下载模型的缓存路径（用于 sentence-transformers 模型）。
- `use_gpu`：可选布尔值，如果可用则启用 GPU 加速。

**重要：** 在同一配置中不能同时指定 `dims` 和 `model_name`，它们是互斥的。使用 `dims` 进行手动向量插入，或使用 `model_name` 进行自动嵌入。使用 `dims` 进行手动向量插入，或使用 `model_name` 进行自动嵌入。

有关 KNN 向量搜索和自动嵌入的更多详细信息，请参阅 [KNN 文档](../../Searching/KNN.md)。

#### rt_attr_bool

```ini
rt_attr_bool = available
```

声明一个具有 1 位无符号整数值的布尔属性。

值：字段名称。

#### rt_attr_string

```ini
rt_attr_string = title
```

字符串属性声明。

值：字段名称。

#### rt_attr_json

```ini
rt_attr_json = properties
```

声明一个 JSON 属性。

值：字段名称。

#### rt_attr_timestamp

```ini
rt_attr_timestamp = date_added
```

声明一个时间戳属性。

值：字段名称。

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

表的 RAM 块内存限制。可选，默认为 128M。

实时表在内存中存储一些数据，称为“RAM 块”，同时维护一些磁盘表，称为“磁盘块”。此指令允许您控制 RAM 块的大小。当内存中数据过多时，实时表会将其刷新到磁盘，激活新创建的磁盘块，并重置 RAM 块。

请注意，此限制是严格的，实时表永远不会分配超过 rt_mem_limit 中指定的内存。此外，内存不会预分配，因此指定 512MB 的限制并仅插入 3MB 数据时，只会分配 3MB，而不是 512MB。

`rt_mem_limit` 永远不会被超过，但实际的 RAM 块大小可能显著低于该限制。RT 表会根据数据插入的速度动态调整实际限制，以最小化内存使用并最大化数据写入速度。其工作原理如下：
* 默认情况下，RAM 块大小为 `rt_mem_limit` 的 50%，称为 "`rt_mem_limit` 限制"。
* 一旦 RAM 块累积的数据量达到 `rt_mem_limit * rate`（默认为 `rt_mem_limit` 的 50%），Manticore 将开始将 RAM 块保存为新的磁盘块。
* 在新磁盘块保存期间，Manticore 会评估新/更新的文档数量。
* 保存新磁盘块后，`rt_mem_limit` 的 rate 会被更新。
* 每次重启 searchd 时，rate 会重置为 50%。

例如，如果磁盘块保存了 90MB 数据，而在保存过程中又到达了额外的 10MB 数据，rate 将为 90%。下一次，RT 表将在刷新数据前收集多达 `rt_mem_limit` 的 90%。插入速度越快，`rt_mem_limit` 的 rate 越低。rate 的范围在 33.3% 到 95% 之间。可以使用 [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 命令查看表的当前 rate。

##### 如何更改 rt_mem_limit 和 optimize_cutoff

在实时模式下，可以使用 `ALTER TABLE` 语句调整 RAM 块的大小限制和磁盘块的最大数量。要将表 "t" 的 `rt_mem_limit` 设置为 1GB，请运行以下查询：`ALTER TABLE t rt_mem_limit='1G'`。要更改磁盘块的最大数量，请运行查询：`ALTER TABLE t optimize_cutoff='5'`。

在普通模式下，可以通过更新表配置或运行命令 `ALTER TABLE <table_name> RECONFIGURE` 来更改 `rt_mem_limit` 和 `optimize_cutoff` 的值。

##### 关于 RAM 块的重要说明

* 实时表类似于由多个本地表（也称为磁盘块）组成的 [分布式](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)。
* 每个 RAM 块由多个段组成，这些段是特殊的仅内存表。
* 磁盘块存储在磁盘上，而 RAM 块存储在内存中。
* 对实时表的每次事务都会生成一个新的段，RAM 块段在每次事务提交后合并。批量插入数百或数千个文档比多次单独插入单个文档更高效，以减少合并 RAM 块段的开销。
* 当段数超过 32 时，它们将被合并以保持数量低于 32。
* 实时表始终有一个 RAM 块（可能为空）和一个或多个磁盘块。
* 合并较大的段需要更长时间，因此最好避免拥有非常大的 RAM 块（因此 `rt_mem_limit` 也应避免过大）。
* 磁盘块的数量取决于表中的数据和 `rt_mem_limit` 设置。
* Searchd 在关闭时会将 RAM 块刷新到磁盘（作为持久化文件，而不是磁盘块），并根据 [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) 设置定期刷新。将数 GB 数据刷新到磁盘可能需要一些时间。
* 较大的 RAM 块在刷新到磁盘的 `.ram` 文件时以及当 RAM 块满并作为磁盘块转储到磁盘时，会对存储造成更大压力。
* RAM 块通过 [二进制日志](../../Logging/Binary_logging.md) 进行备份，直到它被刷新到磁盘。较大的 `rt_mem_limit` 设置会增加重放二进制日志并恢复 RAM 块所需的时间。
* RAM 块可能比磁盘块稍慢。
* 虽然 RAM 块本身占用的内存不超过 `rt_mem_limit`，但在某些情况下，Manticore 可能会占用更多内存，例如当您开始一个事务插入数据但暂时不提交时。在这种情况下，事务中已传输的数据将保留在内存中。

##### RAM 块刷新条件

除了 `rt_mem_limit` 之外，RAM 块的刷新行为还受以下选项和条件的影响：

* 冻结状态。如果表被 [冻结](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md)，刷新将被延迟。这是一个永久规则；没有任何内容可以覆盖它。如果在表被冻结时达到 `rt_mem_limit` 条件，所有进一步的插入将被延迟，直到表被解冻。

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)：此选项定义了在没有写入操作时自动刷新 RAM 块的超时时间（以秒为单位）。如果在此时间内没有写入操作，块将被刷新到磁盘。将其设置为 `-1` 可禁用基于写入活动的自动刷新。默认值为 1 秒。

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)：此选项设置在表中没有搜索时防止自动刷新 RAM 块的超时时间（以秒为单位）。只有在该时间内至少有一次搜索时，才会发生自动刷新。默认值为 30 秒。

* 持续优化：如果当前正在运行优化过程，并且现有磁盘块的数量已达到或超过配置的内部 `cutoff` 阈值，则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过。

* RAM 段中的文档太少：如果 RAM 段中的文档数量低于最小阈值（8192），则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过，以避免创建非常小的磁盘块。这有助于最小化不必要的磁盘写入和块碎片化。

这些超时设置是协同工作的。如果*任一*超时达到，RAM块将会被刷新。这确保了即使没有写入操作，数据最终也会持久化到磁盘，反之，即使有持续的写入但没有搜索，数据也会被持久化。这些设置提供了更精细的控制，以平衡数据持久性与性能考虑。针对每个表的设置具有更高优先级，将覆盖实例范围的默认值。

### 平表设置：

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

source字段指定了在当前表的索引过程中文档的来源。必须至少有一个来源。来源可以是不同类型（例如，一个可以是MySQL，另一个可以是PostgreSQL）。有关从外部存储进行索引的更多信息，请[阅读此处](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

值：源名称是**必填**的。允许多个值。

#### killlist_target

```ini
killlist_target = main:kl
```

此设置确定将kill-list应用到哪些表。在目标表中与当前表更新或删除的匹配项将被抑制。在`:kl`模式下，要抑制的文档来自[kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在`:id`模式下，当前表中的所有文档ID将在目标表中被抑制。如果两者都没有指定，两种模式都将生效。[了解更多关于kill-list的信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

值：**未指定**（默认），target_table_name:kl，target_table_name:id，target_table_name。允许多个值

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

此配置设置确定哪些属性应存储在[列式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中，而不是行式存储中。

您可以设置`columnar_attrs = *`以将所有支持的数据类型存储在列式存储中。

此外，`id`是支持存储在列式存储中的属性。

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

默认情况下，所有存储在列式存储中的字符串属性都会存储预计算的哈希值。这些哈希值用于分组和过滤。然而，它们会占用额外空间，如果您不需要按该属性进行分组，可以通过禁用哈希生成来节省空间。

### 通过CREATE TABLE在线创建实时表

<!-- example rt_mode -->
##### CREATE TABLE的一般语法

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### 数据类型：

有关数据类型的更多信息，请参见[更多关于数据类型的信息](../../Creating_a_table/Data_types.md)。

| 类型 | 配置文件中的等价项 | 注释 | 别名 |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | 选项：indexed，stored。默认：**both**。若要保留文本存储但仅索引，请仅指定"stored"。若要仅索引文本，请仅指定"indexed"。 | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| 整数	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| 大整数	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | 浮点数  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点值的向量  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | 多整数 | mva |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多大整数  | mva64 |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值 |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项 `indexed, attribute` 将使值全文索引，并可过滤、排序和分组 |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N是保留的最大位数  |   |

<!-- intro -->
##### 通过CREATE TABLE创建实时表的示例
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

这创建了一个名为"products"的表，包含两个字段："title"（全文）和"price"（浮点数），并将"morphology"设置为"stem_en"。

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
这创建了一个名为"products"的表，包含三个字段：
* "title" 被索引，但未存储。
* "description" 被存储，但未索引。
* "author" 既被存储又被索引。
<!-- end -->


## 引擎

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

引擎设置会更改表中所有属性的默认[属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。您也可以为每个属性[单独指定engine](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages)。

有关如何为普通表启用列式存储的信息，请参见[columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) 。

值：
* columnar - 为表中所有属性启用列式存储，[json](../../Creating_a_table/Data_types.md#JSON)除外
* **rowwise (默认)** - 不进行任何更改，并使用传统的行式存储。


## 其他设置
以下设置适用于实时表和普通表，无论它们是通过配置文件指定还是通过`CREATE`或`ALTER`命令在线设置。

### 性能相关

#### 访问表文件
Manticore支持两种访问模式来读取表数据：seek+read和mmap。

在seek+read模式下，服务器使用`pread`系统调用来读取文档列表和关键字位置，这些由`*.spd`和`*.spp`文件表示。服务器使用内部读取缓冲区来优化读取过程，这些缓冲区的大小可以通过选项[read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs)和[read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits)进行调整。还有一个选项[preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen)控制Manticore在启动时如何打开文件。

在mmap访问模式下，搜索服务器使用`mmap`系统调用将表文件映射到内存中，操作系统会缓存文件内容。在该模式下，[read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs)和[read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits)选项对相应文件无效。mmap读取器还可以使用`mlock`特权调用将表数据锁定在内存中，防止操作系统将缓存数据交换到磁盘。

要控制使用哪种访问模式，可以使用以下选项：**access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists**和**access_dict**，其值如下：

| 值 | 描述 |
| - | - |
| file | 服务器使用内部缓冲区通过seek+read从磁盘读取表文件 |
| mmap | 服务器将表文件映射到内存中，操作系统在文件访问时缓存其内容 |
| mmap_preread | 服务器将表文件映射到内存中，后台线程一次性读取以预热缓存 |
| mlock | 服务器将表文件映射到内存中，然后执行mlock()系统调用以缓存文件内容并将其锁定在内存中，防止被交换出去 |


| 设置 | 值 | 描述 |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (默认), mlock | 控制`*.spa`（普通属性）`*.spe`（跳过列表）`*.spt`（查找）`*.spm`（已删除文档）的读取方式 |
| access_blob_attrs   | mmap, **mmap_preread** (默认), mlock  | 控制`*.spb`（blob属性）（字符串、mva和json属性）的读取方式 |
| access_doclists   | **file** (默认), mmap, mlock  | 控制`*.spd`（文档列表）数据的读取方式 |
| access_hitlists   | **file** (默认), mmap, mlock  | 控制`*.spp`（命中列表）数据的读取方式 |
| access_dict   | mmap, **mmap_preread** (默认), mlock  | 控制`*.spi`（字典）的读取方式 |

以下表格可以帮助您选择所需的模式：

| 表部分 | 保留在磁盘上 | 保留在内存中 | 服务器启动时缓存在内存中 | 锁定在内存中 |
| - | - | - | - | - |
| [行式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)（非列式）存储的普通属性、跳过列表、词列表、查找、已删除文档 |  mmap | mmap | **mmap_preread** (默认) | mlock |
| 行式字符串、多值属性（MVA）和json属性 | mmap | mmap | **mmap_preread** (默认) | mlock |
| [列式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)数字、字符串和多值属性 | 总是 | 仅通过操作系统 | 否 | 不支持 |
| 文档列表 | **file** (默认) | mmap | 否 | mlock |
| 命中列表 | **file** (默认) | mmap | 否 | mlock |
| 字典 | mmap | mmap | **mmap_preread** (默认) | mlock |

##### 建议如下：

* 为了获得**最快的搜索响应时间**并确保有充足的内存，使用[row-wise](../../Creating_a_table/Data_types.md#JSON)属性并通过`mlock`将其锁定在内存中。此外，对文档列表/命中列表使用mlock。
* 如果您优先考虑**启动后无法接受性能下降**并愿意接受较长的启动时间，请使用[--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options)选项。如果您希望加快searchd重启，请坚持使用默认的`mmap_preread`选项。
* 如果您希望**节省内存**，同时仍有足够的内存存储所有属性，请跳过使用`mlock`。操作系统将根据频繁的磁盘读取决定应保留在内存中的内容。
* 如果行式属性**无法放入内存**，请使用[columnar attributes](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)
* 如果全文搜索**性能不是问题**，并且您希望节省内存，请使用`access_doclists/access_hitlists=file`

默认模式提供了以下平衡：
* mmap，
* 预读非列式属性，
* 无预读地查找和读取列式属性，
* 无预读地查找和读取文档列表/命中列表。

这在大多数情况下提供了良好的搜索性能、最佳的内存利用率和更快的searchd重启。

### 其他性能相关设置

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

This setting 为更新 blob 属性（如多值属性、字符串和 JSON）保留额外空间。默认值为 128k。当更新这些属性时，它们的长度可能会改变。如果更新后的字符串比之前的短，则会覆盖 `*.spb` 文件中的旧数据。如果更新后的字符串更长，则会被写入 `*.spb` 文件的末尾。此文件是内存映射的，因此调整其大小可能是一个潜在的慢过程，取决于操作系统的内存映射文件实现。为了避免频繁调整大小，可以使用此设置在 `.spb` 文件末尾预留额外空间。

值：大小，默认 **128k**。

#### docstore_block_size

```ini
docstore_block_size = 32k
```

此设置控制文档存储使用的块大小。默认值为 16kb。当使用 stored_fields 或 stored_only_fields 存储原始文档文本时，这些文本将存储在表中并压缩以提高效率。为了优化小文档的磁盘访问和压缩比率，这些文档会被连接成块。索引过程会收集文档直到它们的总大小达到此选项指定的阈值。此时，文档块会被压缩。此选项可以调整以获得更好的压缩比率（通过增加块大小）或更快的文档文本访问速度（通过减少块大小）。

值：大小，默认 **16k**。

#### docstore_compression

```ini
docstore_compression = lz4hc
```

此设置确定用于压缩存储在文档存储中的文档块的类型。如果指定了 stored_fields 或 stored_only_fields，文档存储会存储压缩的文档块。'lz4' 提供快速的压缩和解压缩速度，而 'lz4hc'（高压缩）牺牲了一些压缩速度以换取更好的压缩比率。'none' 完全禁用压缩。

值：**lz4**（默认），lz4hc，none。

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

当应用 'lz4hc' 压缩时，在文档存储中使用的压缩级别。通过调整压缩级别，可以在使用 'lz4hc' 压缩时找到性能和压缩比率之间的正确平衡。请注意，此选项不适用于使用 'lz4' 压缩的情况。

值：介于 1 和 12 之间的整数，默认为 **9**。

#### preopen

```ini
preopen = 1
```

此设置指示 searchd 应在启动或轮换时打开所有表文件，并在运行过程中保持打开状态。默认情况下，文件不会预打开。预打开的表需要每个表几个文件描述符，但可以消除每次查询调用 open() 的需求，并且在高负载下可以防止表轮换期间出现的竞争条件。然而，如果您正在服务许多表，从查询的角度打开它们可能仍然更有效，以便节省文件描述符。

值：**0**（默认），或 1。

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

用于存储每个关键词文档列表的缓冲区大小。增加此值会导致查询执行期间更高的内存使用量，但可能会减少 I/O 时间。

值：大小，默认 **256k**，最小值为 8k。

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

用于存储每个关键词命中列表的缓冲区大小。增加此值会导致查询执行期间更高的内存使用量，但可能会减少 I/O 时间。

值：大小，默认 **256k**，最小值为 8k。

### 平凡表磁盘占用设置

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

启用就地表反转。可选，默认为 0（使用单独的临时文件）。

`inplace_enable` 选项在索引平凡表时减少了磁盘占用，同时略微减慢了索引速度（它大约使用 2 倍少的磁盘空间，但性能约为原始性能的 90-95%）。

索引过程分为两个主要阶段。在第一阶段，文档被收集、处理并部分按关键词排序，中间结果被写入临时文件（.tmp*）。在第二阶段，文档被完全排序并创建最终表文件。实时重建生产表需要大约三倍的峰值磁盘占用：首先是中间临时文件，其次是新构建的副本，最后是旧表，它将在同一时间内服务于生产查询。 （中间数据与最终表大小相当）。这可能对大型数据集来说磁盘占用太大，可以使用 `inplace_enable` 选项来减少它。启用后，它会重用临时文件，输出最终数据回它们，并在完成时重命名它们。然而，这可能需要额外的临时数据块重新定位，这就是性能影响的来源。

此指令对 [searchd](../../Starting_the_server/Manually.md) 没有影响，仅影响 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)。


<!-- intro -->
##### CONFIG：

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

[就地反转](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) 的细调选项。控制预分配的命中列表间隙大小。可选，默认为 0。

此指令仅影响 [searchd](../../Starting_the_server/Manually.md) 工具，对 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 没有任何影响。

<!-- intro -->
##### CONFIG：

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

inplace_reloc_factor 设置确定在索引过程中使用的内存区域内的重定位缓冲区大小。默认值为 0.1。

这个选项是可选的，仅影响 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。

<!-- intro -->
##### 配置：

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

控制在索引期间用于就地写入的缓冲区大小。可选，默认值为 0.1。

需要注意的是，此指令仅影响 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。


<!-- intro -->
##### 配置：

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
以下设置受支持。它们都在 [自然语言处理和分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) 部分中描述。
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
* [stored_fields](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)
* [stored_only_fields](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md)
* [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)
<!-- proofread -->

