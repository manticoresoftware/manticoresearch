# 普通表和实时表设置

<!-- example config -->
## 在配置文件中定义表模式

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

### 普通表和实时表的常见设置

#### type

```ini
type = plain

type = rt
```

表类型："plain" 或 "rt"（实时）

取值：**plain**（默认），rt

#### path

```ini
path = path/to/table
```

表存储或所在路径，可以是绝对路径或相对路径，不带扩展名。

取值：表的路径，**必填**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，当在配置文件中定义表时，全文字段的原始内容会被索引并存储。此设置允许您指定应存储其原始值的字段。

取值：以逗号分隔的应存储的**全文**字段列表。空值（即 `stored_fields =`）表示禁用所有字段的原始值存储。

注意：对于实时表，`stored_fields` 中列出的字段也应声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另外，您无需在 `stored_fields` 中列出属性字段，因为它们的原始值无论如何都会被存储。`stored_fields` 只能用于全文字段。

另请参见文档存储压缩选项 [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size)、[docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression)。


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

当您希望 Manticore 将普通表或实时表的某些字段**存储到磁盘但不索引**时，请使用 `stored_only_fields`。这些字段不会通过全文查询进行搜索，但您仍然可以在搜索结果中检索它们的值。

例如，如果您想存储像 JSON 文档这样的数据，并希望每个结果都返回这些数据，但不需要对其进行搜索、过滤或分组，这种方式非常有用。在这种情况下，仅存储而不索引可以节省内存并提高性能。

您可以通过两种方式实现：
- 在表配置的[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)中，使用 `stored_only_fields` 设置。
- 在 SQL 接口（[实时模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)）中，定义文本字段时使用 [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) 属性（而非 `indexed` 或 `indexed stored`）。在 SQL 中，您无需包含 `stored_only_fields` — `CREATE TABLE` 语句不支持该选项。

`stored_only_fields` 的值是以逗号分隔的字段名列表。默认值为空。如果使用实时表，`stored_only_fields` 中列出的每个字段也必须声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

注意：您无需在 `stored_only_fields` 中列出属性字段，因为它们的原始值无论如何都会被存储。

存储仅字段与字符串属性的比较：

- **存储仅字段**：
  - 仅存储在磁盘上
  - 压缩格式
  - 只能检索（不能用于过滤、排序等）

- **字符串属性**：
  - 存储在磁盘和内存中
  - 非压缩格式（除非使用列存储）
  - 可用于排序、过滤、分组等

如果您希望 Manticore 存储文本数据，仅存储在磁盘上（例如：每个结果返回的 json 数据），且不存储在内存中或可搜索/过滤/分组，请使用 `stored_only_fields`，或将文本字段属性设置为 `stored`。

使用 SQL 接口创建表时，将文本字段标记为 `stored`（而非 `indexed` 或 `indexed stored`）。在 `CREATE TABLE` 语句中不需要 `stored_only_fields` 选项；包含该选项可能导致查询失败。

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

默认情况下，除 JSON 属性外，所有属性都会生成二级索引。但是，可以通过 `json_secondary_indexes` 设置显式生成 JSON 属性的二级索引。当 JSON 属性包含在此选项中时，其内容会被展平成多个二级索引。查询优化器可以使用这些索引来加速查询。

您可以使用 [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) 命令查看可用的二级索引。

取值：应生成二级索引的 JSON 属性的逗号分隔列表。

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

防止自动刷新RAM块的超时时间，如果表中没有搜索。更多信息请参见[这里](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

如果没有写入操作，自动刷新RAM块的超时时间。更多信息请参见[这里](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)。

#### optimize_cutoff

RT表的最大磁盘块数量。更多信息请参见[这里](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。

#### rt_field

```ini
rt_field = subject
```

此字段声明确定将被索引的全文字段。字段名称必须唯一，且顺序保持不变。插入数据时，字段值必须与配置中指定的顺序相同。

这是一个多值的可选字段。

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

此声明定义一个无符号整数属性。

值：字段名或 field_name:N（其中N是要保留的最大位数）。

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

此声明定义一个BIGINT属性。

值：字段名，允许多条记录。

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

声明一个多值属性（MVA），值为无符号32位整数。

值：字段名。允许多条记录。

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

声明一个多值属性（MVA），值为有符号64位BIGINT。

值：字段名。允许多条记录。

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

声明单精度浮点属性，采用32位IEEE 754格式。

值：字段名。允许多条记录。

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

声明一个浮点向量属性，用于存储嵌入向量并支持k近邻（KNN）向量搜索。

值：字段名。允许多条记录。

每个向量属性存储一个浮点数数组，表示数据（如文本、图像或其他内容）的高维向量。这些向量通常由机器学习模型生成，可用于相似度搜索、推荐、语义搜索及其他AI驱动的功能。

**重要：** 浮点向量属性需要额外的KNN配置以启用向量搜索功能。

##### 配置向量属性的KNN

要在浮点向量属性上启用KNN搜索，必须添加一个`knn`配置，指定索引参数：

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**必需的KNN参数：**
- `name`：向量属性名称（必须与`rt_attr_float_vector`名称匹配）
- `type`：索引类型，目前仅支持`"hnsw"`
- `dims`：向量维度数（必须与嵌入模型输出匹配）
- `hnsw_similarity`：距离函数 - `"L2"`、`"IP"`（内积）或`"COSINE"`

**可选的KNN参数：**
- `hnsw_m`：图中的最大连接数
- `hnsw_ef_construction`：构建时的准确度与速度权衡

更多关于KNN向量搜索的细节，请参见[KNN文档](../../Searching/KNN.md)。

#### rt_attr_bool

```ini
rt_attr_bool = available
```

声明一个布尔属性，值为1位无符号整数。

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

声明一个JSON属性。

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

表的RAM块内存限制。可选，默认值为128M。

RT表将部分数据存储在内存中，称为“RAM块”，同时维护多个磁盘表，称为“磁盘块”。此指令允许您控制RAM块的大小。当内存中数据过多时，RT表会将其刷新到磁盘，激活新创建的磁盘块，并重置RAM块。

请注意，该限制是严格的，RT表绝不会分配超过rt_mem_limit指定的内存。此外，内存不会预分配，因此指定512MB限制但仅插入3MB数据时，只会分配3MB，而非512MB。

`rt_mem_limit`不会被超出，但实际RAM块大小可能远低于限制。RT表会根据数据插入速度动态调整实际限制，以最小化内存使用并最大化数据写入速度。其工作原理如下：
* 默认情况下，RAM块大小为`rt_mem_limit`的50%，称为“`rt_mem_limit`限制”。
* 一旦RAM块积累的数据达到`rt_mem_limit * rate`（默认50%），Manticore开始将RAM块保存为新的磁盘块。
* 在保存新磁盘块期间，Manticore评估新增/更新文档数量。
* 保存新磁盘块后，更新`rt_mem_limit`的rate值。
* 每次重启searchd时，rate重置为50%。

例如，如果保存到磁盘块的数据为90MB，且保存过程中又有10MB数据到达，则rate为90%。下次RT表将在达到`rt_mem_limit`的90%时刷新数据。插入速度越快，`rt_mem_limit`的rate越低。rate范围在33.3%到95%之间。您可以使用[SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md)命令查看表的当前rate。

##### 如何更改rt_mem_limit和optimize_cutoff

在实时模式下，您可以使用 `ALTER TABLE` 语句调整 RAM 块的大小限制和磁盘块的最大数量。要将表 "t" 的 `rt_mem_limit` 设置为 1 GB，请运行以下查询：`ALTER TABLE t rt_mem_limit='1G'`。要更改磁盘块的最大数量，请运行查询：`ALTER TABLE t optimize_cutoff='5'`。

在普通模式下，您可以通过更新表配置或运行命令 `ALTER TABLE <table_name> RECONFIGURE` 来更改 `rt_mem_limit` 和 `optimize_cutoff` 的值。

##### 关于 RAM 块的重要说明

* 实时表类似于由多个本地表组成的[分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)，这些本地表也称为磁盘块。
* 每个 RAM 块由多个段组成，这些段是特殊的仅限 RAM 的表。
* 磁盘块存储在磁盘上，而 RAM 块存储在内存中。
* 对实时表的每个事务都会生成一个新段，RAM 块段在每次事务提交后合并。执行数百或数千文档的批量 INSERT 比多次单文档 INSERT 更高效，以减少合并 RAM 块段的开销。
* 当段的数量超过 32 时，它们将被合并以保持数量低于 32。
* 实时表始终有一个 RAM 块（可能为空）和一个或多个磁盘块。
* 合并较大的段需要更长时间，因此最好避免拥有非常大的 RAM 块（因此也避免设置过大的 `rt_mem_limit`）。
* 磁盘块的数量取决于表中的数据和 `rt_mem_limit` 设置。
* Searchd 在关闭时以及根据 [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) 设置定期将 RAM 块刷新到磁盘（作为持久化文件，而非磁盘块）。刷新几个 GB 到磁盘可能需要一些时间。
* 大的 RAM 块在刷新到磁盘的 `.ram` 文件以及 RAM 块满时转储为磁盘块时，会对存储造成更大压力。
* RAM 块由[二进制日志](../../Logging/Binary_logging.md)备份，直到刷新到磁盘，较大的 `rt_mem_limit` 设置会增加重放二进制日志和恢复 RAM 块所需的时间。
* RAM 块的速度可能略慢于磁盘块。
* 虽然 RAM 块本身占用的内存不会超过 `rt_mem_limit`，但在某些情况下，Manticore 可能会占用更多内存，例如当您启动一个插入数据的事务但长时间不提交时，此时您已传输的数据会保留在内存中。

##### RAM 块刷新条件

除了 `rt_mem_limit`，RAM 块的刷新行为还受以下选项和条件影响：

* 冻结状态。如果表处于[冻结](../../Securing_and_compacting_a_table/Freezing_a_table.md)状态，刷新将被推迟。这是永久规则，无法被覆盖。如果在表冻结时达到 `rt_mem_limit` 条件，所有后续插入将被延迟，直到表解冻。

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)：此选项定义了如果没有写入操作，自动刷新 RAM 块的超时时间（秒）。如果在此时间内没有写入，块将被刷新到磁盘。设置为 `-1` 禁用基于写入活动的自动刷新。默认值为 1 秒。

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)：此选项设置如果表中没有搜索，防止自动刷新 RAM 块的超时时间（秒）。只有在此时间内至少有一次搜索时，才会自动刷新。默认值为 30 秒。

* 正在进行的优化：如果当前有优化进程运行，且现有磁盘块数量已达到或超过配置的内部 `cutoff` 阈值，则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过。
* RAM 段中文档数量过少：如果 RAM 段中的文档总数低于最小阈值（8192），则由 `diskchunk_flush_write_timeout` 或 `diskchunk_flush_search_timeout` 超时触发的刷新将被跳过，以避免创建非常小的磁盘块。这有助于减少不必要的磁盘写入和块碎片。

这些超时设置是协同工作的。只要达到任一超时，RAM 块就会被刷新。这确保即使没有写入，数据最终也会持久化到磁盘；反之，即使有持续写入但没有搜索，数据也会被持久化。这些设置提供了更细粒度的控制，平衡数据持久性和性能考虑。每个表的指令优先级更高，会覆盖实例范围的默认值。
### 普通表设置：

#### source

source = srcpart1

source = srcpart2

```ini
source = srcpart3
source 字段指定在当前表索引期间将从哪个源获取文档。必须至少有一个源。源可以是不同类型的（例如，一个可能是 MySQL，另一个是 PostgreSQL）。有关从外部存储索引的更多信息，[请阅读这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)
值：源名称是**必需的**。允许多个值。
```

#### killlist_target

killlist_target = main:kl

此设置确定将应用 kill-list 的表。当前表中更新或删除的目标表中的匹配项将被抑制。在 `:kl` 模式下，要抑制的文档取自 [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在 `:id` 模式下，当前表中的所有文档 ID 都会在目标表中被抑制。如果两者都未指定，则两种模式都会生效。[在此了解有关 kill-list 的更多信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

```ini
值：**未指定**（默认），target_table_name:kl，target_table_name:id，target_table_name。允许多个值
```

#### columnar_attrs

columnar_attrs = *

columnar_attrs = id, attr1, attr2, attr3

```ini
此配置设置确定哪些属性应存储在[列存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)中，而非行存储中。
您可以设置 `columnar_attrs = *` 以将所有支持的数据类型存储在列存储中。
```

此外，`id` 是支持存储在列存储中的属性。

#### columnar_strings_no_hash

columnar_strings_no_hash = attr1, attr2, attr3

默认情况下，存储在列存储中的所有字符串属性都会存储预计算的哈希值。这些哈希用于分组和过滤。然而，它们占用额外空间，如果您不需要按该属性分组，可以通过禁用哈希生成来节省空间。

```ini
### 通过 CREATE TABLE 在线创建实时表
```

##### CREATE TABLE 的通用语法

CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]

<!-- example rt_mode -->
##### 数据类型：

```sql
有关数据类型的更多信息，请参见[此处关于数据类型的更多内容](../../Creating_a_table/Data_types.md)。
```

| 类型 | 配置文件中的等价项 | 说明 | 别名 |

| - | - | - | - |

| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | 选项：indexed, stored。默认：**两者**。若只想存储文本但不索引，指定 "stored"。若只想索引文本不存储，指定 "indexed"。 | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| 整数	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| 大整数	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | 浮点数  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点值向量  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | 多值整数 |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多值大整数  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值 |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项 `indexed, attribute` 会使值同时具备全文索引、可过滤、可排序和可分组功能  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N 是要保留的最大位数  |   |
##### 通过 CREATE TABLE 创建实时表的示例
CREATE TABLE products (title text, price float) morphology='stem_en'

<!-- intro -->
这将创建一个名为 "products" 的表，包含两个字段："title"（全文索引）和 "price"（浮点数），并将 "morphology" 设置为 "stem_en"。
<!-- request SQL -->

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```

这将创建一个名为 "products" 的表，包含三个字段：

```sql
* "title" 被索引，但不存储。
```
* "description" 被存储，但不索引。
* "author" 同时被存储和索引。
## 引擎
create table ... engine='columnar';
<!-- end -->


create table ... engine='rowwise';

```ini
engine 设置更改表中所有属性的默认[属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。您也可以为[每个属性单独指定 engine](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages)。
有关如何为普通表启用列存储的信息，请参见 [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)。
```

取值：

* columnar - 为所有表属性启用列存储，除了 [json](../../Creating_a_table/Data_types.md#JSON)

* **rowwise（默认）** - 不做任何更改，使用传统的行存储。
## 其他设置
以下设置适用于实时表和普通表，无论是在配置文件中指定还是通过 `CREATE` 或 `ALTER` 命令在线设置。


### 性能相关
#### 访问表文件

Manticore 支持两种读取表数据的访问模式：seek+read 和 mmap。

在 seek+read 模式下，服务器使用 `pread` 系统调用读取文档列表和关键字位置，这些由 `*.spd` 和 `*.spp` 文件表示。服务器使用内部读取缓冲区来优化读取过程，这些缓冲区的大小可以通过选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 进行调整。还有一个选项 [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) 用于控制 Manticore 启动时如何打开文件。
在 mmap 访问模式下，搜索服务器使用 `mmap` 系统调用将表文件映射到内存中，操作系统缓存文件内容。选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 对该模式下的相应文件无效。mmap 读取器还可以使用特权调用 `mlock` 锁定表数据在内存中，防止操作系统将缓存数据交换到磁盘。

为了控制使用哪种访问模式，提供了选项 **access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists** 和 **access_dict**，其取值如下：

| 值 | 描述 |

| - | - |

| file | 服务器使用内部缓冲区通过 seek+read 从磁盘读取表文件 |
| mmap | 服务器将表文件映射到内存，操作系统缓存其内容 |
| mmap_preread | 服务器将表文件映射到内存，后台线程会读取一次以预热缓存 |
| mlock | 服务器将表文件映射到内存，然后执行 mlock() 系统调用缓存文件内容并锁定在内存中，防止被交换出去 |
| 设置 | 取值 | 描述 |
| - | - | - |


| access_plain_attrs  | mmap, **mmap_preread** (默认), mlock | 控制如何读取 `*.spa`（普通属性）、`*.spe`（跳跃列表）、`*.spt`（查找表）、`*.spm`（已删除文档） |
| access_blob_attrs   | mmap, **mmap_preread** (默认), mlock  | 控制如何读取 `*.spb`（blob 属性）（字符串、多值和 json 属性） |
| access_doclists   | **file** (默认), mmap, mlock  | 控制如何读取 `*.spd`（文档列表）数据 |
| access_hitlists   | **file** (默认), mmap, mlock  | 控制如何读取 `*.spp`（命中列表）数据 |
| access_dict   | mmap, **mmap_preread** (默认), mlock  | 控制如何读取 `*.spi`（字典） |
下面的表格可以帮助您选择所需的模式：
| 表部分 |	保留在磁盘 |	保留在内存 |	服务器启动时缓存到内存 | 锁定在内存 |

| - | - | - | - | - |

| [行存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)（非列存储）的普通属性、跳跃列表、词表、查找表、已删除文档 | 	mmap | mmap |	**mmap_preread** (默认) | mlock |
| 行存储的字符串、多值属性（MVA）和 json 属性 | mmap | mmap | **mmap_preread** (默认) | mlock |
| [列存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 的数值、字符串和多值属性 | 始终 | 仅通过操作系统 | 否 | 不支持 |
| 文档列表 | **file** (默认) | mmap | 否	| mlock |
| 命中列表 | **file** (默认) | mmap | 否	| mlock |
| 字典 | mmap | mmap | **mmap_preread** (默认) | mlock |
##### 推荐如下：
* 若追求 **最快的搜索响应时间** 且内存充足，使用 [行存储](../../Creating_a_table/Data_types.md#JSON) 属性并通过 `mlock` 锁定在内存中。同时，对文档列表/命中列表也使用 mlock。

* 如果优先考虑 **启动后不允许性能下降**，且愿意接受更长的启动时间，可使用 [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options) 选项。若希望更快的 searchd 重启，则保持默认的 `mmap_preread` 选项。

* 若希望 **节省内存**，但仍有足够内存容纳所有属性，则跳过使用 `mlock`。操作系统将根据频繁的磁盘读取决定保留哪些内容在内存中。
* 如果行存储属性 **无法全部放入内存**，则选择 [列存储属性](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。
* 如果全文搜索 **性能不是重点**，且希望节省内存，则使用 `access_doclists/access_hitlists=file`。
默认模式提供了以下平衡：
* mmap，

* 预读非列存储属性，
* 对列存储属性无预读的 seek+read，
* 对文档列表/命中列表无预读的 seek+read。
这在大多数场景下提供了良好的搜索性能、内存利用率和更快的 searchd 重启。
### 其他性能相关设置

#### attr_update_reserve

attr_update_reserve = 256k

此设置为更新 blob 属性（如多值属性 MVA、字符串和 JSON）预留额外空间。默认值为 128k。更新这些属性时，其长度可能变化。如果更新后的字符串比之前短，则会覆盖 `*.spb` 文件中的旧数据；如果更新后的字符串更长，则写入 `*.spb` 文件末尾。该文件是内存映射的，调整其大小可能较慢，具体取决于操作系统的内存映射文件实现。为避免频繁调整大小，可使用此设置在 .spb 文件末尾预留额外空间。

```ini
取值：大小，默认 **128k**。
```

#### docstore_block_size

docstore_block_size = 32k

此设置控制文档存储使用的块大小。默认值为16kb。当使用stored_fields或stored_only_fields存储原始文档文本时，文本存储在表内并进行压缩以提高效率。为了优化小文档的磁盘访问和压缩比，这些文档会被连接成块。索引过程会收集文档，直到它们的总大小达到此选项指定的阈值。此时，文档块会被压缩。可以调整此选项以实现更好的压缩比（通过增加块大小）或更快的文档文本访问速度（通过减小块大小）。

```ini
值：大小，默认 **16k**。
```

#### docstore_compression

docstore_compression = lz4hc

此设置决定用于压缩存储在文档存储中的文档块的压缩类型。如果指定了stored_fields或stored_only_fields，文档存储会存储压缩的文档块。‘lz4’提供快速的压缩和解压速度，而‘lz4hc’（高压缩）牺牲部分压缩速度以获得更好的压缩比。‘none’完全禁用压缩。

```ini
值：**lz4**（默认），lz4hc，none。
```

#### docstore_compression_level

docstore_compression_level = 12

当文档存储中使用‘lz4hc’压缩时使用的压缩级别。通过调整压缩级别，可以在使用‘lz4hc’压缩时找到性能和压缩比之间的平衡。请注意，此选项在使用‘lz4’压缩时不适用。

```ini
值：1到12之间的整数，默认值为 **9**。
```

#### preopen

preopen = 1

此设置指示searchd在启动或轮换时应打开所有表文件，并在运行时保持打开状态。默认情况下，文件不会预先打开。预先打开的表每个表需要几个文件描述符，但它们消除了每次查询调用open()的需求，并且在高负载下表轮换时不会发生竞争条件。然而，如果您服务许多表，按查询打开它们可能更有效以节省文件描述符。

```ini
值：**0**（默认），或1。
```

#### read_buffer_docs

read_buffer_docs = 1M

用于存储每个关键字文档列表的缓冲区大小。增加此值将在查询执行期间导致更高的内存使用，但可能减少I/O时间。

```ini
值：大小，默认 **256k**，最小值为8k。
```

#### read_buffer_hits

read_buffer_hits = 1M

用于存储每个关键字命中列表的缓冲区大小。增加此值将在查询执行期间导致更高的内存使用，但可能减少I/O时间。

```ini
值：大小，默认 **256k**，最小值为8k。
```

### 纯表磁盘占用设置

#### inplace_enable

inplace_enable = {0|1}

启用就地表反转。可选，默认值为0（使用单独的临时文件）。

<!-- example inplace_enable -->

```ini
`inplace_enable`选项在索引纯表时减少磁盘占用，同时略微降低索引速度（它使用大约2倍更少的磁盘，但性能约为原始的90-95%）。
```

索引由两个主要阶段组成。第一阶段，收集文档，处理并按关键字部分排序，中间结果写入临时文件（.tmp*）。第二阶段，文档完全排序并创建最终表文件。在线重建生产表大约需要3倍峰值磁盘占用：首先是中间临时文件，其次是新构建的副本，第三是同时为生产查询服务的旧表。（中间数据大小与最终表相当。）对于大型数据集合，这可能占用过多磁盘，`inplace_enable`选项可用于减少它。启用时，它重用临时文件，将最终数据写回这些文件，并在完成时重命名它们。但这可能需要额外的临时数据块重定位，这就是性能影响的来源。

此指令对[searchd](../../Starting_the_server/Manually.md)无效，仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)。

##### CONFIG:

table products {


<!-- intro -->
  inplace_enable = 1

<!-- request CONFIG -->

```ini
  path = products
  source = src_base

}
#### inplace_hit_gap
inplace_hit_gap = size
```
<!-- end -->

[就地反转](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable)的微调选项。控制预分配的命中列表间隙大小。可选，默认值为0。

<!-- example inplace_hit_gap -->

```ini
此指令仅影响[searchd](../../Starting_the_server/Manually.md)工具，对[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)无影响。
```

##### CONFIG:

table products {

<!-- intro -->
  inplace_hit_gap = 1M

<!-- request CONFIG -->

```ini
  inplace_enable = 1
  path = products
  source = src_base

}
#### inplace_reloc_factor
inplace_reloc_factor = 0.1
```
<!-- end -->

inplace_reloc_factor设置决定索引期间内存区域中重定位缓冲区的大小。默认值为0.1。

<!-- example inplace_reloc_factor -->

```ini
此选项为可选，仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)工具，不影响[searchd](../../Starting_the_server/Manually.md)服务器。
```

##### CONFIG:

table products {

<!-- intro -->
  inplace_reloc_factor = 0.1

<!-- request CONFIG -->

```ini
  inplace_enable = 1
  path = products
  source = src_base

}
#### inplace_write_factor
inplace_write_factor = 0.1
```
<!-- end -->

控制索引期间就地写入使用的缓冲区大小。可选，默认值为0.1。

<!-- example inplace_write_factor -->

```ini
请注意，此指令仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)工具，不影响[searchd](../../Starting_the_server/Manually.md)服务器。
```

##### CONFIG:

table products {


<!-- intro -->
  inplace_write_factor = 0.1

<!-- request CONFIG -->

```ini
  inplace_enable = 1
  path = products
  source = src_base

}
### 自然语言处理特定设置
支持以下设置。它们均在章节 [NLP 和分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) 中进行了描述。
```
<!-- end -->

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
* [stored_only_fields](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md)
* [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)
<!-- proofread -->

