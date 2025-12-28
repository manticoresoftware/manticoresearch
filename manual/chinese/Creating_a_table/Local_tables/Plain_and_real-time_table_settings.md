# 平坦和实时表设置

<!-- example config -->
## 在配置文件中定义表结构

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### 配置文件中的示例平坦表
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
##### 配置文件中的示例实时表
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

### 常见的平坦和实时表设置

#### 类型

```ini
type = plain

type = rt
```

表类型："plain" 或 "rt"（实时）

值：**plain**（默认），rt

#### 路径

```ini
path = path/to/table
```

表将被存储或定位的位置，可以是绝对路径或相对路径，不包括扩展名。

值：表的路径，**必需**

#### 存储字段

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，当在配置文件中定义表时，全文字段的原始内容会被索引并存储。此设置允许您指定应存储其原始值的字段。

值：逗号分隔的列表的 **全文** 字段。空值（例如 `stored_fields =`）会禁用所有字段的原始值存储。

注意：对于实时表，在 `stored_fields` 中列出的字段也应声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另外，请注意，您不需要在 `stored_fields` 中列出属性，因为它们的原始值会自动存储。`stored_fields` 只能用于全文字段。

请参阅 [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size)，[docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) 以获取文档存储压缩选项。


<!-- intro -->
##### SQL：

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
##### Python：
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- intro -->
##### Python-asyncio：
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- intro -->
##### JavaScript：

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### Java：
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- intro -->
##### C#：
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- intro -->
##### Rust：

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)", Some(true)).await;
```

<!-- intro -->
##### TypeScript：

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### Go：

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

#### 只存储字段

```ini
stored_only_fields = title,content
```

使用 `stored_only_fields` 当您希望 Manticore 将某些平坦表或实时表的字段存储在磁盘上但不进行索引时。这些字段不会通过全文查询进行搜索，但您仍然可以在搜索结果中检索它们的值。

例如，如果您想存储像 JSON 文档这样的数据，这些数据应该与每个结果一起返回，但不需要进行搜索、过滤或分组，则存储它们而不进行索引可以节省内存并提高性能。

您可以使用两种方式之一实现这一点：
- 在 [平坦模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 的表配置中使用 `stored_only_fields` 设置。
- 在 SQL 接口 ([实时模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) 中，当定义文本字段时使用 [存储](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) 属性（而不是 `索引` 或 `索引存储`）。在 SQL 中，您不需要包含 `stored_only_fields`——它不支持 `CREATE TABLE` 语句。

`stored_only_fields` 的值是一个逗号分隔的字段名称列表。默认为空。如果使用实时表，`stored_only_fields` 中列出的每个字段也必须声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

注意：您不需要在 `stored_only_fields` 中列出属性，因为它们的原始值会自动存储。

仅存储字段与字符串属性的比较：

- **仅存储字段**：
  - 仅存储在磁盘上
  - 压缩格式
  - 仅可检索（不能用于过滤、排序等）

- **字符串属性**：
  - 存储在磁盘和内存中
  - 未压缩格式（除非使用列式存储）
  - 可用于排序、过滤、分组等

如果您希望 Manticore 存储您仅希望存储在磁盘上的文本数据（例如：返回每个结果的 JSON 数据），并且不想将其存储在内存中或进行搜索/过滤/分组，请使用 `stored_only_fields` 或 `存储` 作为您的文本字段属性。

使用 SQL 接口创建表时，请将文本字段标记为 `存储`（而不是 `索引` 或 `索引存储`）。在 `CREATE TABLE` 语句中不需要 `stored_only_fields` 选项；包括它可能会导致查询失败。

#### json_次级索引

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

默认情况下，除了 JSON 属性外，所有属性都会生成次级索引。但是，可以使用 `json_次级索引` 设置显式生成 JSON 属性的次级索引。当 JSON 属性包含在此选项中时，其内容会被展平成多个次级索引。这些索引可以被查询优化器用来加快查询速度。

您可以使用 [SHOW TABLE <tbl> 索引](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) 命令查看可用的次级索引。

值：逗号分隔的 JSON 属性列表，用于生成次级索引。

<!-- intro -->
##### SQL：

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
##### Python：
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- intro -->
##### Python-asyncio：
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- intro -->
##### JavaScript：

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

此超时值用于防止在表中没有搜索的情况下自动刷新内存块。了解更多信息，请参阅[此处](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

此超时值用于在内存块没有写入操作时自动刷新内存块。了解更多信息，请参阅[此处](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)。

#### optimize_cutoff

RT 表的最大磁盘块数。了解更多信息，请参阅[此处](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。

#### rt_field

```ini
rt_field = subject
```

此字段声明确定将被索引的全文字段。字段名必须唯一，并且顺序保持不变。插入数据时，字段值必须与配置中的指定顺序相同。

这是一个多值可选字段。

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

此声明定义了一个无符号整数属性。

值：字段名或 field_name:N（N 是要保留的最大位数）。

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

此声明定义了一个 BIGINT 属性。

值：字段名，允许多个记录。

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

声明一个带有无符号 32 位整数值的多值属性（MVA）。

值：字段名。允许多个记录。

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

声明一个带有带符号 64 位 BIGINT 值的多值属性（MVA）。

值：字段名。允许多个记录。

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

声明具有单精度、32 位 IEEE 754 格式的浮点属性。

值：字段名。允许多个记录。

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

声明一个用于存储嵌入和启用 k-最近邻（KNN）向量搜索的浮点值向量。

值：字段名。允许多个记录。

每个向量属性存储表示数据（如文本、图像或其他内容）的高维向量数组。这些向量通常由机器学习模型生成，并可用于相似性搜索、推荐、语义搜索和其他基于 AI 的功能。

**重要提示：** 浮点向量属性需要额外的 KNN 配置以启用向量搜索功能。

##### 配置向量属性的 KNN

为了在浮点向量属性上启用 KNN 搜索，您必须添加一个 `knn` 配置，该配置指定了索引参数：

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**必需的 KNN 参数：**
- `name`：向量属性的名称（必须与 `rt_attr_float_vector` 名称匹配）
- `type`：索引类型，目前仅支持 `"hnsw"`
- `dims`：向量中的维度数（必须与您的嵌入模型的输出匹配）
- `hnsw_similarity`：距离函数 - `"L2"`、`"IP"`（内积）或 `"COSINE"`

**可选的 KNN 参数：**
- `hnsw_m`：图中的最大连接数
- `hnsw_ef_construction`：构造时间/准确性权衡

有关 KNN 向量搜索的更多详细信息，请参阅[KNN 文档](../../Searching/KNN.md)。

#### rt_attr_bool

```ini
rt_attr_bool = available
```

声明一个具有 1 位无符号整数值的布尔属性。

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

声明一个 JSON 属性。

值：字段名。

#### rt_attr_timestamp

```ini
rt_attr_timestamp = date_added
```

声明一个时间戳属性。

值：字段名。

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

表的内存块的内存限制。可选，默认值为 128M。

RT 表在内存中存储一些数据，称为“内存块”，同时维护多个磁盘块，称为“磁盘块”。此指令允许您控制内存块的大小。当内存中的数据过多无法容纳时，RT 表会将其刷新到磁盘，激活一个新的磁盘块，并重置内存块。

请注意，限制是严格的，RT 表永远不会分配超过 `rt_mem_limit` 中指定的内存。此外，内存不会预先分配，因此指定 512MB 的限制并只插入 3MB 的数据会导致分配 3MB 而不是 512MB。

`rt_mem_limit` 从不超出，但实际的内存块大小可能远低于限制。RT 表会根据数据插入速度动态调整实际限制，以最小化内存使用并最大化数据写入速度。这就是它的工作方式：
* 默认情况下，内存块大小为 `rt_mem_limit` 的 50%，称为 `rt_mem_limit` 限制。
* 当内存块积累的数据相当于 `rt_mem_limit * rate` 数据（默认为 `rt_mem_limit` 的 50%）时，Manticore 开始将内存块保存为新的磁盘块。
* 在保存新磁盘块的同时，Manticore 评估新/更新文档的数量。
* 保存新磁盘块后，`rt_mem_limit` 率会更新。
* 每次重启 searchd 时，率都会重置为 50%。

例如，如果 90MB 的数据已保存到磁盘块中，在保存过程中又有 10MB 的数据到达，则率为 90%。下次，RT 表将在收集相当于 `rt_mem_limit * rate` 的数据之前刷新数据。插入速度越快，`rt_mem_limit` 率越低。率范围为 33.3% 到 95%。您可以使用 [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 命令查看表的当前率。

##### 如何更改 rt_mem_limit 和优化 cutoff

在实时模式下，您可以使用 `ALTER TABLE` 语句调整 RAM 块的大小限制和最大磁盘块数量。要将表 "t" 的 `rt_mem_limit` 设置为 1GB，运行以下查询：`ALTER TABLE t rt_mem_limit='1G'`。要更改最大磁盘块数量，请运行查询：`ALTER TABLE t optimize_cutoff='5'`。

在普通模式下，您可以通过更新表配置或运行命令 `ALTER TABLE <表名> RECONFIGURE` 来更改 `rt_mem_limit` 和 `optimize_cutoff` 的值。

##### 关于 RAM 块的重要说明

* 实时表类似于由多个本地表组成的分布式表，也称为磁盘块。
* 每个 RAM 块由多个段组成，这些段是特殊的仅 RAM 表。
* 虽然磁盘块存储在磁盘上，但 RAM 块存储在内存中。
* 对实时表的每次事务操作都会生成一个新的段，并且在每次事务提交后，RAM 块段会被合并。批量插入数百或数千个文档比多次单独插入一个文档更有效，可以减少合并 RAM 块段的开销。
* 当段的数量超过 32 时，它们将被合并以保持计数低于 32。
* 实时表始终有一个 RAM 块（可能是空的）和一个或多个磁盘块。
* 合并较大的段需要更长的时间，因此最好避免拥有非常大的 RAM 块（因此 `rt_mem_limit` 也会更大）。
* 磁盘块的数量取决于表中的数据和 `rt_mem_limit` 设置。
* Searchd 在关闭时会将 RAM 块刷新到磁盘（作为持久化的文件，而不是磁盘块），并且根据 [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) 设置定期刷新。将几 GB 刷新到磁盘可能需要一些时间。
* 大的 RAM 块会对存储造成更大的压力，无论是刷新到磁盘的 `.ram` 文件还是当 RAM 块满时将其刷新到磁盘作为磁盘块。
* RAM 块由 [二进制日志](../../Logging/Binary_logging.md) 支持，直到它被刷新到磁盘，更大的 `rt_mem_limit` 设置会增加回放二进制日志和恢复 RAM 块所需的时间。
* RAM 块可能比磁盘块稍慢。
* 尽管 RAM 块本身不会占用比 `rt_mem_limit` 更多的内存，但在某些情况下，Manticore 可能会占用更多的内存，例如当你开始一个事务来插入数据但不立即提交时。在这种情况下，你已经在事务中传输的数据将继续保留在内存中。

##### RAM 块刷新条件

除了 `rt_mem_limit` 之外，RAM 块的刷新行为还受到以下选项和条件的影响：

* 冻结状态。如果表处于 [冻结](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md) 状态，则刷新会被推迟。这是一个永久规则；没有任何东西可以覆盖它。如果在表冻结状态下达到 `rt_mem_limit` 条件，所有后续插入都将延迟，直到表解冻。

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)：此选项定义了如果没有对该 RAM 块进行写入，则自动刷新该块的超时（秒）。如果在该时间内没有写入发生，则块将被刷新到磁盘。将其设置为 `-1` 可禁用基于写入活动的自动刷新。默认值为 1 秒。

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)：此选项设置了防止自动刷新 RAM 块的超时（秒），前提是表中没有搜索。只有在该时间内至少进行了一次搜索时才会触发自动刷新。默认值为 30 秒。

* 进行中的优化：如果正在进行优化过程，并且现有磁盘块的数量达到了或超过了配置的内部 `cutoff` 阈值，则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过。

* RAM 块中的文档太少：如果 RAM 块中的文档总数低于最小阈值（8192），则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过，以避免创建非常小的磁盘块。这有助于减少不必要的磁盘写入和块碎片化。

这些超时相互配合工作。只要任一超时到达，RAM 块就会被刷新。这确保即使没有写入，数据最终也会持久化到磁盘，反之亦然，即使有持续的写入但没有搜索，数据也会被持久化。这些设置提供了对如何频繁刷新 RAM 块的更细粒度控制，平衡了数据持久性和性能考虑。针对每个表的这些设置具有更高的优先级，并会覆盖实例范围的默认设置。

### 普通表设置：

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

source 字段指定了在当前表索引期间将从何处获取文档。必须至少有一个源。源可以是不同类型的（例如一个可以是 MySQL，另一个可以是 PostgreSQL）。有关从外部存储添加数据的更多信息，请参阅 [此处](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

值：源的名称是 **必需的**。允许多个值。

#### killlist_target

```ini
killlist_target = main:kl
```

This setting确定杀列表将应用于哪些表。当前表中更新或删除的目标表中的匹配项将被抑制。在`:kl`模式下，抑制的文档来自[杀列表](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在`:id`模式下，目标表中的所有文档ID将被抑制。如果没有指定，则两种模式都将生效。[了解更多关于杀列表的信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

值：**未指定**（默认值），target_table_name:kl，target_table_name:id，target_table_name。允许多个值

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

此配置设置确定应将哪些属性存储在[列式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中，而不是行式存储。

可以设置`columnar_attrs = *`以将所有支持的数据类型存储在列式存储中。

此外，`id`也是一个支持存储在列式存储中的属性。

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

默认情况下，所有存储在列式存储中的字符串属性都会存储预计算的哈希值。这些哈希值用于分组和过滤。但是，它们会占用额外的空间，如果您不需要按该属性分组，则可以通过禁用哈希生成来节省空间。

### 通过CREATE TABLE在线创建实时表

<!-- example rt_mode -->
##### CREATE TABLE的通用语法

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### 数据类型：

有关数据类型的更多信息，请参见[更多关于数据类型的信息](../../Creating_a_table/Data_types.md)。

| 类型 | 配置文件中的等价项 | 备注 | 别名 |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | 选项：indexed, stored。默认值：**both**。要仅保留文本存储但索引，请指定"stored"。要仅保留文本索引，请指定"indexed"。 | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| 整数	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| 大整数	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | 浮点数  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点数向量  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | 多整数 | mva |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多大整数  | mva64 |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值 |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项`indexed, attribute`将使值全文索引并可过滤、排序和分组  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N是保留的最大位数  |   |

<!-- intro -->
##### 通过CREATE TABLE创建实时表的示例
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

这创建了"products"表，包含两个字段："title"（全文）和"price"（浮点数），并将"morphology"设置为"stem_en"。

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
这创建了"products"表，包含三个字段：
* "title"已索引，但未存储。
* "description"已存储，但未索引。
* "author"既已存储又已索引。
<!-- end -->


## 引擎

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

引擎设置更改了表中所有属性的默认[属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。您也可以为每个属性单独指定`engine` [切换存储方式](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages)。

有关如何为普通表启用列式存储的信息，请参见[columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)。

值：
* columnar - 为表的所有属性启用列式存储，除了[json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise（默认值）** - 不做任何更改，并使用传统的行式存储。


## 其他设置
以下设置适用于实时表和普通表，无论它们是通过配置文件指定还是通过`CREATE`或`ALTER`命令在线设置。

### 性能相关

#### 访问表文件
Manticore支持两种读取表数据的访问模式：seek+read和mmap。

在seek+read模式下，服务器使用`pread`系统调用来读取文档列表和关键词位置，这些信息由`*.spd`和`*.spp`文件表示。服务器使用内部读缓冲区来优化读取过程，并且可以使用选项[read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs)和[read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits)调整这些缓冲区的大小。还有一个选项[preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen)，它控制Manticore在启动时如何打开文件。

在mmap访问模式下，搜索服务器使用`mmap`系统调用将表文件映射到内存中，并且操作系统会缓存文件内容。在这种模式下，选项[read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs)和[read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits)对相应的文件没有影响。mmap读取器还可以使用特权调用`mlock`将表的数据锁定在内存中，从而防止操作系统将缓存的数据交换到磁盘上。

要控制使用哪种访问模式，可用的选项包括**access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists**和**access_dict**，它们的值如下：

| 值 | 描述 |
| - | - |
| file | 服务器通过seek+read从磁盘读取表文件，并在文件访问时使用内部缓冲区 |
| mmap | 服务器将表文件映射到内存中，并在文件访问时由操作系统缓存其内容 |
| mmap_preread | 服务器将表文件映射到内存中，并在一个后台线程读取一次以预热缓存 |
| mlock | 服务器将表文件映射到内存中，然后执行mlock()系统调用来缓存文件内容并将其锁定在内存中，以防止其被交换到磁盘 |


| 设置 | 值 | 描述 |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread**（默认），mlock | 控制如何读取`*.spa`（普通属性）`*.spe`（跳过列表）`*.spt`（查找）`*.spm`（已删除文档） |
| access_blob_attrs   | mmap, **mmap_preread**（默认），mlock  | 控制如何读取`*.spb`（二进制属性）（字符串、多值和json属性） |
| access_doclists   | **file**（默认），mmap，mlock  | 控制如何读取`*.spd`（文档列表）数据 |
| access_hitlists   | **file**（默认），mmap，mlock  | 控制如何读取`*.spp`（命中列表）数据 |
| access_dict   | mmap, **mmap_preread**（默认），mlock  | 控制如何读取`*.spi`（字典） |

以下表格可以帮助您选择所需的模式：

| 表格部分 | 保持在磁盘上 | 保持在内存中 | 启动时缓存在内存中 | 锁定在内存中 |
| - | - | - | - | - |
| [行式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)（非列式）存储中的普通属性、跳过列表、词列表、查找 | 	mmap | mmap |	**mmap_preread**（默认） | mlock |
| 行式字符串、多值属性（MVA）和json属性 | mmap | mmap | **mmap_preread**（默认） | mlock |
| 列式数值、字符串和多值属性 | 总是 | 仅由操作系统决定 | 不支持 | 不支持 |
| 文档列表 | **file**（默认） | mmap | 不支持 | mlock |
| 命中列表 | **file**（默认） | mmap | 不支持 | mlock |
| 字典 | mmap | mmap | **mmap_preread**（默认） | mlock |

##### 推荐设置为：

* 若要获得**最快的搜索响应时间**和充足的内存，请使用[行式](../../Creating_a_table/Data_types.md#JSON)属性并将它们锁定在内存中使用`mlock`。此外，也使用`mlock`锁定文档列表/命中列表。
* 如果您**无法接受启动后的较低性能**并且愿意接受较长的启动时间，请使用[--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options)选项。如果您希望更快地重启searchd，则坚持使用默认的`mmap_preread`选项。
* 如果您希望**节省内存**但仍需要足够的内存来容纳所有属性，不要使用`mlock`。操作系统将根据频繁的磁盘读取来决定保留哪些内容在内存中。
* 如果行式属性**不适应内存**，请考虑使用[列式属性](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。
* 如果全文搜索**性能不是问题**并且希望节省内存，请使用`access_doclists/access_hitlists=file`。

默认模式提供以下平衡：
* mmap，
* 预读非列式属性，
* 对于列式属性进行seek和读取而无需预读，
* 对于文档列表/命中列表进行seek和读取而无需预读。

这在大多数情况下提供了良好的搜索性能、最佳的内存利用率以及更快的searchd重启。

### 其他与性能相关的设置

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

此设置为更新二进制属性（如多值属性（MVA）、字符串和JSON）预留额外空间。默认值为128k。当更新这些属性时，它们的长度可能会改变。如果更新后的字符串比之前的短，它将覆盖`*.spb`文件中的旧数据。如果更新后的字符串更长，它将写入`*.spb`文件的末尾。由于该文件是内存映射的，因此调整其大小可能是一个缓慢的过程，具体取决于操作系统的内存映射文件实现。为了避免频繁调整大小，您可以使用此设置在spb文件末尾预留额外空间。

值：大小，默认 **128k**。

#### docstore_block_size

```ini
docstore_block_size = 32k
```

This setting 控制文档存储使用的块大小。默认值为 16kb。当使用 stored_fields 或 stored_only_fields 存储原始文档文本时，这些文本会在表中存储并进行压缩以提高效率。为了优化小型文档的磁盘访问和压缩比，这些文档会被连接成块。索引过程会收集文档直到它们的总大小达到此选项指定的阈值。此时，文档块会被压缩。此选项可以调整以实现更好的压缩比（通过增加块大小）或更快的文档文本访问速度（通过减少块大小）。

Value: size, 默认 **16k**。

#### docstore_compression

```ini
docstore_compression = lz4hc
```

此设置确定用于压缩存储在文档存储中的文档块的类型。如果指定了 stored_fields 或 stored_only_fields，则文档存储会存储压缩的文档块。'lz4' 提供快速压缩和解压缩速度，而 'lz4hc'（高压缩）则牺牲一些压缩速度以获得更好的压缩比。'none' 完全禁用压缩。

Values: **lz4**（默认），lz4hc，none。

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

当使用 'lz4hc' 压缩时，在文档存储中使用的压缩级别。通过调整压缩级别，可以在使用 'lz4hc' 压缩时找到性能和压缩比之间的正确平衡。请注意，此选项不适用于使用 'lz4' 压缩的情况。

Value: 介于 1 和 12 之间的整数，默认值为 **9**。

#### preopen

```ini
preopen = 1
```

此设置指示 searchd 应在启动或轮换时打开所有表文件，并在运行过程中保持打开状态。默认情况下，文件不会预打开。预打开的表需要每个表几个文件描述符，但可以消除每次查询的 open() 调用需求，并且对高负载下表轮换期间可能出现的竞争条件免疫。然而，如果您正在服务许多表，可能仍然更有效率地在每次查询时打开它们以节省文件描述符。

Value: **0**（默认），或 1。

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

用于存储每个关键词文档列表的缓冲区大小。增加此值会导致查询执行期间内存使用量更高，但可能会减少 I/O 时间。

Value: size，默认 **256k**，最小值为 8k。

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

用于存储每个关键词命中列表的缓冲区大小。增加此值会导致查询执行期间内存使用量更高，但可能会减少 I/O 时间。

Value: size，默认 **256k**，最小值为 8k。

### 平凡表磁盘占用空间设置

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

启用就地表反转。可选，默认为 0（使用单独的临时文件）。

`inplace_enable` 选项在索引平凡表时减少了磁盘占用空间，同时略微减慢了索引速度（它大约使用两倍少的磁盘空间，但性能约为原始性能的 90-95%）。

索引过程分为两个主要阶段。在第一阶段，文档被收集、处理并按关键词部分排序，中间结果写入临时文件（.tmp*）。在第二阶段，文档完全排序并创建最终表文件。实时重建生产表需要大约三倍的峰值磁盘占用空间：首先是中间临时文件，其次是新构建的副本，最后是旧表，它将在同一时间内服务于生产查询。（中间数据与最终表大小相当）。对于大型数据集来说，这可能是太多的磁盘占用空间，因此可以使用 `inplace_enable` 选项来减少它。启用后，它会重用临时文件，将最终数据输出回它们，并在完成时重命名它们。但是，这可能需要额外的临时数据块重新定位，这就是性能影响的来源。

此指令对 [searchd](../../Starting_the_server/Manually.md) 没有影响，仅影响 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)。


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

[就地反转](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) 的细调选项。控制预分配的命中列表间隙大小。可选，默认为 0。

此指令仅影响 [searchd](../../Starting_the_server/Manually.md) 工具，不对 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 产生任何影响。

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

inplace_reloc_factor 设置确定在索引过程中使用的内存区域内的重定位缓冲区大小。默认值为 0.1。

此选项是可选的，仅影响 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。

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

控制索引期间用于就地写入的缓冲区大小。可选，默认值为 0.1。

请注意，此指令仅影响 [索引器](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。


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
以下设置是支持的。它们都在[NLP和分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)部分中描述。
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

