# 普通和实时表设置

<!-- example config -->
## 在配置文件中定义表架构

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

### 普通和实时表的常见设置

#### 类型

```ini
type = plain

type = rt
```

表类型：“plain”或“rt”（实时）

值：**plain**（默认），rt

#### 路径

```ini
path = path/to/table
```

表将存储或位于的路径，可以是绝对路径或相对路径，无需扩展名。

值：表的路径，**必填**

#### 存储字段

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，当在配置文件中定义表时，完整文本字段的原始内容会被索引并存储。此设置允许您指定应存储原始值的字段。

值：应存储的**完整文本**字段的以逗号分隔的列表。空值（即 `stored_fields =`）将禁用所有字段的原始值存储。

注意：对于实时表，`stored_fields`中列出的字段也应声明为[rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另外，请注意，您无需在`stored_fields`中列出属性，因为它们的原始值会被存储。`stored_fields`只能用于完整文本字段。

另请参见[docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size)、[docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression)用于文档存储压缩选项。


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

当您希望Manticore将某些普通或实时表字段**存储在磁盘上但不进行索引**时，请使用`stored_only_fields`。这些字段将无法通过完整文本查询进行搜索，但您仍然可以在搜索结果中检索它们的值。

例如，如果您想存储像JSON文档这样的数据，这些数据应该与每个结果一起返回，但不需要被搜索、过滤或分组，这在这种情况下非常有用。仅存储它们而不进行索引可以节省内存并提高性能。

您可以通过两种方式做到这一点：
- 在表配置中的[普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29)中，使用`stored_only_fields`设置。
- 在SQL接口中（[RT模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)），在定义文本字段时使用[stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore)属性（而不是`indexed`或`indexed stored`）。在SQL中，您无需在`CREATE TABLE`语句中包含`stored_only_fields` — 它不受支持。

`stored_only_fields`的值是字段名称的以逗号分隔的列表。默认情况下，它是空的。如果您使用的是实时表，则`stored_only_fields`中列出的每个字段也必须声明为[rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

注意：您无需在`stored_only_fields`中列出属性，因为它们的原始值会被存储。

存储仅字段与字符串属性的比较：

- **仅存储字段**：
  - 仅存储在磁盘上
  - 压缩格式
  - 只能被检索（不能用于过滤、排序等）

- **字符串属性**：
  - 存储在磁盘和内存中
  - 未压缩格式（除非您使用列存储）
  - 可用于排序、过滤、分组等

如果您希望Manticore仅将文本数据存储在磁盘上（例如：每个结果都返回的JSON数据），而不是在内存中或可搜索/可过滤/可分组，请使用`stored_only_fields`，或将`stored`作为您的文本字段属性。

使用SQL接口创建表时，将您的文本字段标记为`stored`（而不是`indexed`或`indexed stored`）。在您的`CREATE TABLE`语句中不需要`stored_only_fields`选项；包括它可能会导致查询失败。

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

默认情况下，所有属性生成二级索引，除了JSON属性。然而，可以使用`json_secondary_indexes`设置显式生成JSON属性的二级索引。当JSON属性包含在此选项中时，它的内容被展平为多个二级索引。这些索引可以被查询优化器用于加速查询。

您可以使用[SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md)命令查看可用的二级索引。

值：应生成二级索引的JSON属性的以逗号分隔的列表。

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

<!-- 介绍 -->
##### Java:
<!-- 请求Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- 介绍 -->
##### C#:
<!-- 请求C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- 介绍 -->
##### Rust:

<!-- 请求Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, j json secondary_index='1')", Some(true)).await;
```

<!-- 介绍 -->
##### Typescript:

<!-- 请求Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```

<!-- 介绍 -->
##### Go:

<!-- 请求Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, j json secondary_index='1')").Execute()
```


<!-- 请求CONFIG -->

```ini
table products {
  json_secondary_indexes = j

  type = rt
  path = tbl
  rt_field = title
  rt_attr_json = j
}
```
<!-- 结束 -->

### 实时表设置：

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

用于防止在表中没有搜索时自动刷新RAM块的超时时间。了解更多详情请点击[此处](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

在没有写入时自动刷新RAM块的超时时间。了解更多详情请点击[此处](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)。

#### optimize_cutoff

实时表的最大磁盘块数。了解更多详情请点击[此处](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。

#### rt_field

```ini
rt_field = subject
```

此字段声明确定将被索引的全文字段。字段名必须唯一，并且保留顺序。插入数据时，字段值必须按配置中指定的顺序排列。

这是一个多值的可选字段。

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

此声明定义无符号整数属性。

值：字段名或field_name:N（其中N是要保留的最大位数）。

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

此声明定义BIGINT属性。

值：字段名，允许多个记录。

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

声明具有无符号32位整数值的多值属性（MVA）。

值：字段名。允许多个记录。

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

声明具有有符号64位BIGINT值的多值属性（MVA）。

值：字段名。允许多个记录。

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

声明单精度浮点属性，采用32位IEEE 754格式。

值：字段名。允许多个记录。

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

声明浮点值向量。

值：字段名。允许多个记录。

#### rt_attr_bool

```ini
rt_attr_bool = available
```

声明具有1位无符号整数值的布尔属性。

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

声明JSON属性。

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

表的RAM块内存限制。可选，默认为128M。

实时表将一些数据存储在内存中，称为"RAM块"，并维护多个磁盘表，称为"磁盘块"。此指令允许您控制RAM块的大小。当内存中的数据过多时，实时表将把数据刷新到磁盘，激活新创建的磁盘块，并重置RAM块。

请注意，限制是严格的，实时表永远不会分配超过`rt_mem_limit`中指定的内存。此外，内存不是预分配的，因此指定512MB的限制并只插入3MB的数据将只分配3MB，而不是512MB。

`rt_mem_limit`永远不会被超过，但实际RAM块大小可能明显低于限制。实时表根据数据插入速度进行调整，动态调整实际限制以最小化内存使用并最大化数据写入速度。工作原理如下：
* 默认情况下，RAM块大小是`rt_mem_limit`的50%，称为"`rt_mem_limit`"。
* 一旦RAM块累积的数据达到`rt_mem_limit * rate`（默认为`rt_mem_limit`的50%），Manticore开始将RAM块保存为新的磁盘块。
* 在保存新磁盘块时，Manticore评估新增/更新文档的数量。
* 保存新磁盘块后，更新`rt_mem_limit`速率。
* 每次重新启动searchd时，速率重置为50%。

例如，如果90MB数据被保存到磁盘块，并且在保存过程中又到达10MB数据，则速率将为90%。下次，实时表将收集最多`rt_mem_limit`的90%的数据再刷新。插入速度越快，`rt_mem_limit`速率越低。速率在33.3%到95%之间变化。您可以使用[SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md)命令查看表的当前速率。

##### 如何更改rt_mem_limit和optimize_cutoff

在实时模式下，您可以使用`ALTER TABLE`语句调整RAM块大小限制和磁盘块的最大数量。要将表"t"的`rt_mem_limit`设置为1千兆字节，运行以下查询：`ALTER TABLE t rt_mem_limit='1G'`。要更改磁盘块的最大数量，运行查询：`ALTER TABLE t optimize_cutoff='5'`。

在普通模式下，您可以通过更新表配置或运行命令`ALTER TABLE <table_name> RECONFIGURE`来更改`rt_mem_limit`和`optimize_cutoff`的值。

##### 关于RAM块的重要注意事项

* 实时表类似于[分布式](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)表，由多个本地表组成，也称为磁盘块。
* 每个RAM块由多个段组成，这些段是特殊的仅RAM表。
* 磁盘块存储在磁盘上，而RAM块存储在内存中。
* 对实时表的每个事务都会生成一个新段，RAM块段在每个事务提交后合并。执行数百或数千文档的批量插入比多个单文档的单独插入更有效，可以减少合并RAM块段的开销。
* 当段数超过32时，它们将被合并以保持数量低于32。
* 实时表始终有一个 RAM 块（可能是空的）和一个或多个磁盘块。
* 合并较大的段需要更长时间，因此最好避免拥有非常大的 RAM 块（因此也要避免 `rt_mem_limit` 过高）。
* 磁盘块的数量取决于表中的数据和 `rt_mem_limit` 设置。
* Searchd 在关闭时以及根据 [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) 设置定期将 RAM 块刷新到磁盘（作为持久化文件，而不是作为磁盘块）。将几GB的数据刷新到磁盘可能需要一些时间。
* 大的 RAM 块在存储上施加了更大压力，无论是在刷新到磁盘的 `.ram` 文件中，还是当 RAM 块满了并转储到磁盘作为磁盘块时。
* RAM 块在刷新到磁盘之前由 [二进制日志](../../Logging/Binary_logging.md) 备份，而较大的 `rt_mem_limit` 设置将增加重放二进制日志并恢复 RAM 块所需的时间。
* RAM 块的性能可能略逊于磁盘块。
* 尽管 RAM 块本身不会占用超过 `rt_mem_limit` 的内存，但在某些情况下，Manticore 可能会占用更多内存，例如当您开始事务以插入数据并且一段时间不提交时。在这种情况下，您在事务中已经传输的数据将保留在内存中。

除了 `rt_mem_limit`，RAM 块的刷新行为还受到以下设置的影响：[diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 和 [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

* `diskchunk_flush_write_timeout`：此选项定义了如果没有写入，则自动刷新 RAM 块的超时。如果在此时间内没有写入，块将被刷新到磁盘。将其设置为 `-1` 将禁用基于写入活动的自动刷新。默认值为 1 秒。
* `diskchunk_flush_search_timeout`：此选项设置了防止自动刷新 RAM 块的超时，如果在表中没有搜索。在此时间内至少进行过一次搜索时，才会发生自动刷新。默认值为 30 秒。

这些超时共同起作用。如果 *任意* 超时被达到，则 RAM 块将被刷新。这确保即使没有写入，数据最终也会持久化到磁盘；反之，即使有持续的写入但没有搜索，数据也会被持久化。这些设置提供了对 RAM 块刷新频率的更细致控制，在数据持久性和性能考虑之间取得平衡。每个表的这些设置指令具有更高的优先级，并将覆盖实例范围的默认值。

### 普通表设置：

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

源字段指定将在当前表的索引期间获得文档的源。必须至少有一个源。源可以是不同类型（例如，一个可以是 MySQL，另一个可以是 PostgreSQL）。有关从外部存储索引的更多信息，请 [查看此处的外部存储索引](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

值：源的名称是 **强制的**。允许多个值。

#### killlist_target

```ini
killlist_target = main:kl
```

此设置确定 kill-list 将应用于哪个表。当前表中更新或删除的目标表中的匹配项将被抑制。在 `:kl` 模式下，要抑制的文档取自 [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在 `:id` 模式下，当前表中的所有文档 ID 在目标表中被抑制。如果都未指定，则会生效两个模式。 [在这里了解更多关于 kill-list 的信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

值：**未指定**（默认），target_table_name:kl，target_table_name:id，target_table_name。允许多个值。

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

此配置设置确定应存储哪些属性在 [列式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 中，而不是行式存储中。

您可以设置 `columnar_attrs = *` 以将所有支持的数据类型存储在列式存储中。

此外，`id` 是一个受支持的属性，可以存储在列式存储中。

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

默认情况下，存储在列式存储中的所有字符串属性都存储预计算的哈希。这些哈希用于分组和过滤。然而，它们占用额外的空间，如果您不需要按该属性分组，可以通过禁用哈希生成来节省空间。

### 通过 CREATE TABLE 在线创建实时表

<!-- example rt_mode -->
##### CREATE TABLE 的一般语法

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### 数据类型：

有关数据类型的更多信息，请参见 [此处的数据类型](../../Creating_a_table/Data_types.md)。

| 类型 | 在配置文件中的等效项 | 注释 | 别名 |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | 选项：indexed，stored。默认值：**两者**。要保持文本存储但已编索引，请仅指定“stored”。要仅保持文本索引，请仅指定“indexed”。 | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)
| integer
 | int，uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)
| 大整数
 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | 浮点数  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点值的向量  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | 多重整数 |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多重大整数  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值 |   |

| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项 `indexed, attribute` 将使值全文本索引，并且可过滤、可排序和可分组  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |

[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N 是要保留的最大位数  |   |
<!-- intro -->

##### 通过 CREATE TABLE 创建实时表的示例

<!-- request SQL -->
```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```
这将创建一个名为 “products” 的表，包含两个字段：“title”（全文）和“price”（浮点数），并将“morphology” 设置为“stem_en”。
```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```


这将创建一个名为 “products” 的表，包含三个字段：

* “title” 被索引，但不被存储。
* “description” 被存储，但不被索引。
* “author” 同时被存储和索引。
<!-- end -->

## 引擎

```ini

create table ... engine='columnar';
create table ... engine='rowwise';
```


引擎设置更改了表中所有属性的默认 [属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。您还可以为每个属性 [单独指定 `engine`](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages)。
有关如何为普通表启用列存储的信息，请参见 [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) 。

值：

* columnar - 启用所有表属性的列存储，除了 [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (默认)** - 不更改任何内容并使用传统的行存储。

## 其他设置

下面的设置适用于实时表和普通表，无论是通过配置文件指定还是使用 `CREATE` 或 `ALTER` 命令在线设置。

### 性能相关

#### 访问表文件
Manticore 支持两种读取表数据的访问模式：seek+read 和 mmap。
在 seek+read 模式下，服务器使用 `pread` 系统调用读取文档列表和关键字位置，这些位置由 `*.spd` 和 `*.spp` 文件表示。服务器使用内部读取缓冲区来优化读取过程，这些缓冲区的大小可以通过选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 调整。还有选项 [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen)，控制 Manticore 在启动时如何打开文件。
在 mmap 访问模式下，搜索服务器使用 `mmap` 系统调用将表的文件映射到内存中，并且操作系统缓存文件内容。选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 对此模式下的相应文件没有影响。mmap 读取器还可以使用 `mlock` 特权调用将表的数据锁定在内存中，以防止操作系统将缓存数据交换到磁盘。
要控制使用哪个访问模式，可以使用选项 **access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists** 和 **access_dict** ，它们的值如下：
| 值 | 描述 |


| - | - |
| file | 服务器从磁盘读取表文件，使用 seek+read 和内部缓冲区进行文件访问 |
| mmap | 服务器将表文件映射到内存中，并在文件访问时由操作系统缓存其内容 |
| mmap_preread | 服务器将表文件映射到内存中，并且一个后台线程读取它一次以预热缓存 |
| mlock | 服务器将表文件映射到内存中，然后执行 mlock() 系统调用以缓存文件内容并将其锁定在内存中，以防止被交换出去 |
| 设置 | 值 | 描述 |
| - | - | - |

| access_plain_attrs  | mmap、**mmap_preread**（默认）、mlock | 控制如何读取 `*.spa`（普通属性） `*.spe`（跳过列表） `*.spt`（查找） `*.spm`（已删除文档） |

| access_blob_attrs   | mmap、**mmap_preread**（默认）、mlock  | 控制如何读取 `*.spb`（二进制属性）（字符串、mva 和 json 属性） |
| access_doclists   | **file**（默认）、mmap、mlock  | 控制如何读取 `*.spd`（文档列表）数据 |
| access_hitlists   | **file**（默认）、mmap、mlock  | 控制如何读取 `*.spp`（命中列表）数据 |
| access_dict   | mmap、**mmap_preread**（默认）、mlock  | 控制如何读取 `*.spi`（字典） |
以下是一个表，可以帮助您选择所需的模式：
| 表部分 |
保留在磁盘上 |
保留在内存中 |

cached in memory on server start | lock it in memory |

| - | - | - | - | - |
| plain attributes in [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (non-columnar) storage, skip lists, word lists, lookups, killed docs | 
mmap | mmap |
**mmap_preread** (default) | mlock |
| row-wise string, multi-value attributes (MVA) and json attributes | mmap | mmap | **mmap_preread** (default) | mlock |

| [columnar](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) numeric, string and multi-value attributes | always  | only by means of OS  | no  | not supported |
| doc lists | **file** (default) | mmap | no
| mlock |
| hit lists | **file** (default) | mmap | no
| mlock |

| dictionary | mmap | mmap | **mmap_preread** (default) | mlock |

##### The recommendations are:

* For the **fastest search response time** and ample memory availability, use [row-wise](../../Creating_a_table/Data_types.md#JSON) attributes and lock them in memory using `mlock`. Additionally, use mlock for doclists/hitlists.

* If you prioritize **can't afford lower performance after start** and are willing to sacrifice longer startup time, use the [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). option. If you desire faster searchd restart, stick to the default  `mmap_preread` option.
* If you are looking to **conserve memory**, while still having enough memory for all attributes, skip the use of `mlock`. The operating system will determine what should be kept in memory based on frequent disk reads.
* If row-wise attributes  **do not fit into memory**, opt for [columnar attributes](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

* If full-text search **performance is not a concern**, and you wish to save memory, use `access_doclists/access_hitlists=file`

The default mode offers a balance of:

* mmap,

* Prereading non-columnar attributes,
* Seeking and reading columnar attributes with no preread,
* Seeking and reading doclists/hitlists with no preread.

This provides a decent search performance, optimal memory utilization, and faster searchd restart in most scenarios.

### Other performance related settings

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

This setting reserves extra space for updates to blob attributes such as multi-value attributes (MVA), strings, and JSON. The default value is 128k. When updating these attributes, their length may change. If the updated string is shorter than the previous one, it will overwrite the old data in the `*.spb` file. If the updated string is longer, it will be written to the end of the `*.spb` file. This file is memory-mapped, making resizing it a potentially slow process, depending on the operating system's memory-mapped file implementation. To avoid frequent resizing, you can use this setting to reserve extra space at the end of the .spb file.

Value: size, default **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

This setting controls the size of blocks used by the document storage. The default value is 16kb. When original document text is stored using stored_fields or stored_only_fields, it is stored within the table and compressed for efficiency. To optimize disk access and compression ratios for small documents, these documents are concatenated into blocks. The indexing process collects documents until their total size reaches the threshold specified by this option. At that point, the block of documents is compressed. This option can be adjusted to achieve better compression ratios (by increasing the block size) or faster access to document text (by decreasing the block size).

Value: size, default **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

This setting determines the type of compression used for compressing blocks of documents stored in document storage. If stored_fields or stored_only_fields are specified, the document storage stores compressed document blocks. 'lz4' offers fast compression and decompression speeds, while 'lz4hc' (high compression) sacrifices some compression speed for a better compression ratio. 'none' disables compression completely.

Values: **lz4** (default), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

The compression level used when 'lz4hc' compression is applied in document storage. By adjusting the compression level, you can find the right balance between performance and compression ratio when using 'lz4hc' compression. Note that this option is not applicable when using 'lz4' compression.

Value: An integer between 1 and 12, with a default of **9**.

#### preopen

```ini
preopen = 1
```

This setting indicates that searchd should open all table files on startup or rotation, and keep them open while running. By default, the files are not pre-opened. Pre-opened tables require a few file descriptors per table, but they eliminate the need for per-query open() calls and are immune to race conditions that might occur during table rotation under high load. However, if you are serving many tables, it may still be more efficient to open them on a per-query basis in order to conserve file descriptors.

Value: **0** (default), or 1.

#### read_buffer_docs

```ini

read_buffer_docs = 1M

```
Buffer size for storing the list of documents per keyword. Increasing this value will result in higher memory usage during query execution, but may reduce I/O time.
Value: size, default **256k**, minimum value is 8k.

#### read_buffer_hits

```ini

read_buffer_hits = 1M

```


Buffer size for storing the list of hits per keyword. Increasing this value will result in higher memory usage during query execution, but may reduce I/O time.
Value: size, default **256k**, minimum value is 8k.

### Plain table disk footprint settings

#### inplace_enable
<!-- example inplace_enable -->
```ini

inplace_enable = {0|1}
```
Enables in-place table inversion. Optional, default is 0 (uses separate temporary files).
`inplace_enable` 选项在索引普通表时减少了磁盘占用，同时稍微降低了索引速度（它大约使用 2 倍更少的磁盘，但性能约为原来的 90-95%）。
索引分为两个主要阶段。在第一阶段，收集、处理和按关键字部分排序文档，并将中间结果写入临时文件 (.tmp*)。在第二阶段，文档被完全排序并创建最终的表文件。实时重建生产表大约需要峰值磁盘占用的 3 倍：首先是中间临时文件，其次是新构建的副本，其三是旧表，它将在此期间处理生产查询。（中间数据的大小与最终表相当。）这对于大型数据集合而言可能占用过多的磁盘空间，`inplace_enable` 选项可以用来减少它。启用时，它重用临时文件，将最终数据输出回其中，并在完成后重命名它们。然而，这可能需要额外的临时数据块重新定位，这就是性能影响的来源。

此指令对 [searchd](../../Starting_the_server/Manually.md) 没有影响，它仅影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)。

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
选项 [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) 微调选项。控制预分配的命中列表间隙大小。可选，默认值为 0。
该指令仅影响 [searchd](../../Starting_the_server/Manually.md) 工具，并且对 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 没有任何影响。

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
inplace_reloc_factor 设置确定在索引期间使用的内存区域内重新定位缓冲区的大小。默认值为 0.1。
该选项是可选的，仅影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。

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
需要注意的是，该指令仅影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。

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
支持以下设置。它们在 [NLP 和标记化](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) 部分中都有描述。
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
