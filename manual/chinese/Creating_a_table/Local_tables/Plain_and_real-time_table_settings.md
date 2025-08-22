# 普通和实时表设置

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

### 通用普通和实时表设置

#### type

```ini
type = plain

type = rt
```

表类型: "plain" 或 "rt" (实时)

值: **plain** (默认), rt

#### path

```ini
path = path/to/table
```

表将被存储或位于的路径，可以是绝对路径或相对路径，不带扩展名。

值: 表的路径, **必需**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，当在配置文件中定义表时，全文字段的原始内容会被索引和存储。此设置允许您指定应存储其原始值的字段。

值: 应存储的 **全文** 字段的逗号分隔列表。空值（即 `stored_fields =`）会禁用所有字段的原始值的存储。

注意: 在实时表的情况下，列在 `stored_fields` 中的字段也应该声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另外，请注意，您不需要在 `stored_fields` 中列出属性，因为它们的原始值会被存储。`stored_fields` 仅可用于全文字段。

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

#### stored_only_fields

```ini
stored_only_fields = title,content
```

使用 `stored_only_fields` 当您希望 Manticore 在 **磁盘上存储某些字段但不进行索引**。这些字段将无法通过全文查询进行搜索，但您仍然可以在搜索结果中检索其值。

例如，如果您想存储像 JSON 文档这样的数据，这些数据应与每个结果一起返回，但不需要搜索、过滤或分组，这种方式非常有用。在这种情况下，仅存储它们——而不进行索引——可以节省内存并提高性能。

您可以通过两种方式实现此目的：
- 在表配置的 [普通模式](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) 中，使用 `stored_only_fields` 设置。
- 在 SQL 接口中（[实时模式](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)），在定义文本字段时使用 [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) 属性（而不是 `indexed` 或 `indexed stored`）。在 SQL 中，您不需要在 `CREATE TABLE` 语句中包含 `stored_only_fields`——它在此语句中不被支持。

`stored_only_fields` 的值是字段名称的逗号分隔列表。默认情况下，它是空的。如果您正在使用实时表，`stored_only_fields` 中列出的每个字段也必须声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

注意: 您不需要在 `stored_only_fields` 中列出属性，因为它们的原始值会被存储。

存储仅字段与字符串属性的比较：

- **存储仅字段**：
  - 仅存储在磁盘上
  - 压缩格式
  - 只能检索（不能用于过滤、排序等）

- **字符串属性**：
  - 存储在磁盘上和内存中
  - 未压缩格式（除非您使用的是列式存储）
  - 可用于排序、过滤、分组等

如果您希望 Manticore 存储文本数据，而您 **仅** 希望它存储在磁盘上（例如：与每个结果一起返回的 json 数据），而不在内存中或可搜索/可过滤/可分组，请使用 `stored_only_fields`，或者将 `stored` 用作您的文本字段属性。

在使用 SQL 接口创建表时，将您的文本字段标记为 `stored`（而不是 `indexed` 或 `indexed stored`）。您在 `CREATE TABLE` 语句中不需要 `stored_only_fields` 选项；包括它可能导致查询失败。

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

默认情况下，为除 JSON 属性之外的所有属性生成二级索引。但是，可以使用 `json_secondary_indexes` 设置显式生成 JSON 属性的二级索引。当 JSON 属性包含在此选项中时，其内容会扁平化为多个二级索引。这些索引可由查询优化器使用，以加快查询速度。

您可以使用 [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) 命令查看可用的二级索引。

值: 应生成二级索引的 JSON 属性的逗号分隔列表。

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

防止自动刷新 RAM 块的超时时间，如果表中没有搜索请求。详细了解请见 [此处](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

如果没有写入操作，RAM 块自动刷新的超时时间。详细了解请见 [此处](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)。

#### optimize_cutoff

RT 表的最大磁盘块数。详细了解请见 [此处](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。

#### rt_field

```ini
rt_field = subject
```

该字段声明决定将被索引的全文字段。字段名称必须唯一，顺序保持不变。在插入数据时，字段值必须与配置中指定的顺序一致。

这是一个多值的可选字段。

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

该声明定义一个无符号整数字段属性。

值：字段名或 field_name:N（其中 N 是要保留的最大位数）。

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

该声明定义一个 BIGINT 属性。

值：字段名，允许多条记录。

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

声明一个多值属性（MVA），其值为无符号 32 位整数。

值：字段名。允许多条记录。

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

声明一个多值属性（MVA），其值为有符号 64 位 BIGINT。

值：字段名。允许多条记录。

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

声明单精度浮点属性，32 位 IEEE 754 格式。

值：字段名。允许多条记录。

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

声明一个浮点值向量。

值：字段名。允许多条记录。

#### rt_attr_bool

```ini
rt_attr_bool = available
```

声明一个布尔属性，值为 1 位无符号整数。

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

表的 RAM 块内存限制。可选，默认值为 128M。

RT 表将部分数据存储在内存中，称为“RAM 块”，同时维护多个存储在磁盘上的表，称为“磁盘块”。此指令允许您控制 RAM 块的大小。当内存中数据过多时，RT 表会将其刷新到磁盘，激活新创建的磁盘块，并重置 RAM 块。

请注意该限制是严格的，RT 表绝不会分配超过 rt_mem_limit 指定大小的内存。此外，内存不会预分配，因此指定了 512MB 的限制但只插入了 3MB 的数据时，只会分配 3MB 而非 512MB。

`rt_mem_limit` 不会被超过，但实际的 RAM 块大小可能远低于该限制。RT 表会根据数据的插入速度动态调整实际限制，以最大限度减少内存使用并提高数据写入速度。其工作原理如下：
* 默认情况下，RAM 块大小为 `rt_mem_limit` 的 50%，称为 "`rt_mem_limit` 限制"。
* 当 RAM 块积累的数据达到 `rt_mem_limit * rate`（默认为 `rt_mem_limit` 的 50%）时，Manticore 开始将 RAM 块保存为一个新的磁盘块。
* 在保存新的磁盘块时，Manticore 会评估新增/更新的文档数量。
* 保存新的磁盘块后，将更新 `rt_mem_limit` 的 rate。
* 每次重启 searchd 时，rate 会重置为 50%。

例如，如果有 90MB 的数据保存到磁盘块，同时保存过程中又有额外 10MB 数据到达，则该 rate 将为 90%。下次，RT 表会在收集到 `rt_mem_limit` 的 90% 数据后刷新。插入速度越快，`rt_mem_limit` 率越低。该率在 33.3% 到 95% 之间波动。您可以使用 [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 命令查看表的当前 rate。

##### 如何修改 rt_mem_limit 和 optimize_cutoff

在实时模式下，您可以使用 `ALTER TABLE` 语句调整 RAM 块大小限制和最大磁盘块数。将表 "t" 的 `rt_mem_limit` 设置为 1GB，运行查询：`ALTER TABLE t rt_mem_limit='1G'`。更改磁盘块最大数量，运行查询：`ALTER TABLE t optimize_cutoff='5'`。

在普通模式下，您可以通过更新表配置或运行命令 `ALTER TABLE <table_name> RECONFIGURE` 来更改 `rt_mem_limit` 和 `optimize_cutoff` 的值。

##### 关于 RAM 块的重要注意事项

* 实时表类似于由多个本地表组成的 [分布式表](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)，这些本地表也称为磁盘块。
* 每个 RAM 块由多个段组成，这些段是专门的仅在内存中的表。
* 磁盘块存储在磁盘上，而 RAM 块存储在内存中。
* 对实时表的每个事务都会生成一个新段，事务提交后会合并 RAM 块的段。执行包含数百或数千个文档的批量 INSERT 比执行多次单文档 INSERT 更为高效，以减少合并 RAM 块段的开销。
* 当段数超过32个时，它们会被合并以保持数量低于32个。
* 实时表总是有一个RAM段（可能为空）和一个或多个磁盘段。
* 合并较大的段需要更长时间，因此最好避免有一个非常大的RAM段（因此要考虑`rt_mem_limit`）。
* 磁盘段的数量取决于表中的数据和`rt_mem_limit`设置。
* Searchd在关闭时和根据[rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period)设置定期将RAM段刷新到磁盘（作为持久化文件，而不是磁盘段）。将几个GB的数据刷新到磁盘可能需要一些时间。
* 大的RAM段会给存储带来更大压力，无论是刷新到磁盘的`.ram`文件，还是当RAM段满时作为磁盘段转储到磁盘时。
* RAM段由[二进制日志](../../Logging/Binary_logging.md)备份，直到它被刷新到磁盘，较大的`rt_mem_limit`设置会增加重放二进制日志和恢复RAM段所需的时间。
* RAM段可能比磁盘段稍慢。
* 虽然RAM段本身不会占用超过`rt_mem_limit`的内存，但在某些情况下Manticore可能会占用更多内存，比如当你开始一个事务插入数据但长时间不提交时。在这种情况下，你在事务中已经传输的数据将保留在内存中。

##### RAM段刷新条件

除了`rt_mem_limit`，RAM段的刷新行为还受以下选项和条件的影响：

* 冻结状态。如果表被[冻结](../../Securing_and_compacting_a_table/Freezing_a_table.md)，刷新将被推迟。这是一个永久规则，没有什么可以覆盖它。如果在表冻结时达到`rt_mem_limit`条件，所有后续插入将被延迟直到表解冻。

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)：此选项定义了在没有写入时自动刷新RAM段的超时时间（以秒为单位）。如果在此时间内没有写入发生，该段将被刷新到磁盘。将其设置为`-1`将禁用基于写入活动的自动刷新。默认值为1秒。

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)：此选项设置了在表中没有搜索时防止自动刷新RAM段的超时时间（以秒为单位）。只有在此时间内至少有一次搜索时才会发生自动刷新。默认值为30秒。

* 正在进行的优化：如果优化过程当前正在运行，且现有磁盘段数量已
  达到或超过配置的内部`cutoff`阈值，由`diskchunk_flush_write_timeout`或`diskchunk_flush_search_timeout`超时触发的刷新将被跳过。

* RAM段中文档数量太少：如果RAM段中的文档总数低于最小阈值（8192），
  由`diskchunk_flush_write_timeout`或`diskchunk_flush_search_timeout`超时触发的刷新将被跳过，以避免创建非常小的磁盘段。这有助于最小化不必要的磁盘写入和段碎片化。

这些超时是协同工作的。当*任一*超时达到时，RAM段将被刷新。这确保即使没有写入，数据最终也会被持久化到磁盘，反之，即使有持续的写入但没有搜索，数据也会被持久化。这些设置提供了对RAM段刷新频率更细粒度的控制，平衡了数据持久性和性能考虑。每个表的指令优先级更高，将覆盖实例范围的默认值。

### Plain表设置：

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

source字段指定了在当前表索引期间将从中获取文档的源。必须至少有一个源。源可以是不同类型的（例如，一个可以是MySQL，另一个可以是PostgreSQL）。有关从外部存储索引的更多信息，[请阅读这里](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

值：源的名称是**必需的**。允许多个值。

#### killlist_target

```ini
killlist_target = main:kl
```

此设置确定将应用kill-list的表。在当前表中更新或删除的目标表中的匹配项将被抑制。在`:kl`模式下，要抑制的文档来自[kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在`:id`模式下，当前表中的所有文档ID都将在目标表中被抑制。如果两者都未指定，两种模式都将生效。[在此处了解更多关于kill-lists的信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

值：**未指定**（默认），target_table_name:kl，target_table_name:id，target_table_name。允许多个值

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

此配置设置确定哪些属性应存储在[列式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)而不是行式存储中。

你可以设置`columnar_attrs = *`以将所有支持的数据类型存储在列式存储中。

此外，`id`是支持存储在列式存储中的属性。

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

默认情况下，存储在列式存储中的所有字符串属性都存储预计算的哈希值。这些哈希值用于分组和过滤。但是，它们会占用额外的空间，如果你不需要按该属性分组，可以通过禁用哈希生成来节省空间。

### 通过CREATE TABLE在线创建实时表

<!-- example rt_mode -->
##### CREATE TABLE的一般语法

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### 数据类型：

有关数据类型的更多信息，请参见[这里了解更多关于数据类型](../../Creating_a_table/Data_types.md)。

| 类型 | 配置文件中的等效项 | 备注 | 别名 |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | 选项：indexed, stored。默认值：**both**。要保持文本被存储但被索引，请仅指定“stored”。要仅保持文本被索引，请仅指定“indexed”。 | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)
| integer
 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)
| 大整数
 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | 浮点数  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点值向量  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | 多值整数 |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多值大整数  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值 |   |

| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项 `indexed, attribute` 将使该值同时支持全文索引、可筛选、可排序和可分组  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |

[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N 表示要保留的最大比特数  |   |
<!-- intro -->

##### 通过 CREATE TABLE 创建实时表的示例

<!-- request SQL -->
```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```
这将创建一个名为 “products” 的表，包含两个字段：“title”（全文索引）和 “price”（浮点数），并将“morphology”设置为 “stem_en”。
```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```


这将创建一个名为 “products” 的表，包含三个字段：

* “title” 被索引，但不存储。
* “description” 被存储，但不索引。
* “author” 同时被存储和索引。
<!-- end -->

## 引擎

```ini

create table ... engine='columnar';
create table ... engine='rowwise';
```


引擎设置更改表中所有属性的默认[属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。您也可以为每个属性[单独指定引擎](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages)。
有关如何为普通表启用列存储的信息，请参见 [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) 。

值：

* columnar - 为所有表属性启用列存储，除了 [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (默认)** - 不改变任何内容，使用传统的行存储方式。

## 其他设置

以下设置适用于实时表和普通表，无论是在配置文件中指定还是通过 `CREATE` 或 `ALTER` 命令在线设置。

### 性能相关

#### 访问表文件
Manticore 支持两种读取表数据的访问模式：seek+read 和 mmap。
在 seek+read 模式中，服务器使用 `pread` 系统调用读取文档列表和关键词位置，分别对应 `*.spd` 和 `*.spp` 文件。服务器使用内部读取缓冲区来优化读取过程，这些缓冲区的大小可以通过选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 调整。还有一个选项 [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) 控制 Manticore 启动时如何打开文件。
在 mmap 访问模式中，搜索服务器使用 `mmap` 系统调用将表文件映射到内存，操作系统缓存文件内容。选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 对该模式下对应文件无效。mmap 读取器还可以使用特权调用 `mlock` 锁定表数据在内存中，防止操作系统将缓存的数据交换到磁盘。
要控制使用哪种访问模式，提供了选项 **access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists** 和 **access_dict**，其取值如下：
| 值 | 描述 |


| - | - |
| file | 服务器使用 seek+read 方式，配合内部缓冲区，从磁盘读取表文件 |
| mmap | 服务器将表文件映射到内存，操作系统缓存其内容 |
| mmap_preread | 服务器将表文件映射到内存，并由后台线程预先读取一次以预热缓存 |
| mlock | 服务器将表文件映射到内存中，然后执行 mlock() 系统调用，将文件内容缓存并锁定在内存中，以防止被交换出去 |
| 设置 | 值 | 说明 |
| - | - | - |

| access_plain_attrs  | mmap, **mmap_preread** (默认), mlock | 控制如何读取 `*.spa`（普通属性）`*.spe`（跳表）`*.spt`（查找）`*.spm`（已删除文档） |

| access_blob_attrs   | mmap, **mmap_preread** (默认), mlock  | 控制如何读取 `*.spb`（blob 属性）（字符串、多值和 json 属性） |
| access_doclists   | **file** (默认), mmap, mlock  | 控制如何读取 `*.spd`（文档列表）数据 |
| access_hitlists   | **file** (默认), mmap, mlock  | 控制如何读取 `*.spp`（命中列表）数据 |
| access_dict   | mmap, **mmap_preread** (默认), mlock  | 控制如何读取 `*.spi`（字典） |
以下表格可帮助您选择所需模式：
| 表部分 |
保持在磁盘上 |
保持在内存中 |

服务器启动时缓存于内存 | 锁定在内存中 |

| - | - | - | - | - |
| 以 [逐行](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)（非列式）存储的普通属性、跳表、词表、查找、已删除文档 |
mmap | mmap |
**mmap_preread** (默认) | mlock |
| 逐行字符串、多值属性 (MVA) 和 json 属性 | mmap | mmap | **mmap_preread** (默认) | mlock |

| [列式](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 数值、字符串及多值属性 | 始终  | 仅通过操作系统 | 否  | 不支持 |
| 文档列表 | **file** (默认) | mmap | 否
| mlock |
| 命中列表 | **file** (默认) | mmap | 否
| mlock |

| 字典 | mmap | mmap | **mmap_preread** (默认) | mlock |

##### 推荐如下：

* 对于**最快的搜索响应时间**和充足内存，使用[逐行](../../Creating_a_table/Data_types.md#JSON)属性并通过 `mlock` 锁定在内存中。同时，对文档列表和命中列表同样使用 mlock。

* 如果您优先考虑**启动后不愿意降低性能**并愿意接受较长的启动时间，则使用 [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options) 选项。如果您希望搜索守护进程更快重启，则坚持使用默认的 `mmap_preread` 选项。
* 如果您希望**节省内存**，但仍有足够内存容纳所有属性，则避免使用 `mlock`。操作系统将根据频繁的磁盘读取决定保留哪些数据在内存中。
* 如果逐行属性**无法全部放入内存**，请选择[列式属性](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。

* 如果全文搜索**性能不是重点**，且想节省内存，使用 `access_doclists/access_hitlists=file`

默认模式在以下方面提供了平衡：

* mmap,

* 预读取非列式属性,
* 无预读取情况下查找和读取列式属性,
* 无预读取情况下查找和读取文档列表/命中列表。

这在大多数情况下提供了合理的搜索性能、内存利用率和更快的搜索守护进程重启速度。

### 其他性能相关设置

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

此设置为多值属性 (MVA)、字符串和 JSON 这类 blob 属性的更新预留额外空间。默认值为 128k。更新这些属性时，长度可能会变化。如果更新后的字符串比之前短，则会覆盖 `*.spb` 文件中的旧数据；如果更长，则写入 `*.spb` 文件末尾。该文件是内存映射的，调整其大小可能较慢，具体取决于操作系统的内存映射文件实现。为避免频繁调整大小，可以使用此设置预留文件末尾更多空间。

取值：大小，默认 **128k**。

#### docstore_block_size

```ini
docstore_block_size = 32k
```

此设置控制文档存储使用的块大小。默认值为 16kb。当使用 stored_fields 或 stored_only_fields 存储原始文档文本时，文本存储在表中并进行压缩以提高效率。为优化磁盘访问和提升小文档压缩比，文档被拼接成块。索引过程会收集文档，直到其总大小达到此选项指定的阈值，然后对文档块进行压缩。通过调整此选项，可以实现更好的压缩比（增加块大小）或更快的文档文本访问速度（减少块大小）。

取值：大小，默认 **16k**。

#### docstore_compression

```ini
docstore_compression = lz4hc
```

此设置决定文档存储中压缩文档块所用的压缩类型。如果指定了 stored_fields 或 stored_only_fields，文档存储将保存压缩的文档块。‘lz4’ 提供快速压缩和解压速度，而 ‘lz4hc’（高压缩）则牺牲部分压缩速度以获得更佳压缩比。‘none’ 则完全禁用压缩。

取值：**lz4** (默认), lz4hc, none。

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

当在文档存储中使用 ‘lz4hc’ 压缩时采用的压缩级别。通过调节此级别，可在性能和压缩比之间寻找最佳平衡。注意，此选项不适用于 ‘lz4’ 压缩。

取值：整数，范围 1 到 12，默认 **9**。

#### preopen

```ini
preopen = 1
```

此设置表示searchd应在启动或轮换时打开所有表文件，并在运行时保持打开状态。默认情况下，文件不预先打开。预先打开的表每个表需要几个文件描述符，但它们消除了每个查询的open()调用的需要，并且在高负载下表轮换期间可能发生的竞争条件免疫。然而，如果您服务许多表，按查询打开它们可能仍然更高效，以节省文件描述符。

值：**0**（默认），或1。

#### read_buffer_docs

```ini

read_buffer_docs = 1M

```
用于存储每个关键字文档列表的缓冲区大小。增加此值将在查询执行期间导致更高的内存使用，但可能减少I/O时间。
值：大小，默认 **256k**，最小值为8k。

#### read_buffer_hits

```ini

read_buffer_hits = 1M

```


用于存储每个关键字命中列表的缓冲区大小。增加此值将在查询执行期间导致更高的内存使用，但可能减少I/O时间。
值：大小，默认 **256k**，最小值为8k。

### 纯表磁盘占用设置

#### inplace_enable
<!-- example inplace_enable -->
```ini

inplace_enable = {0|1}
```
启用原地表反转。可选，默认值为0（使用单独的临时文件）。
`inplace_enable`选项在索引纯表时减少磁盘占用，同时稍微减慢索引速度（它使用大约2倍更少的磁盘，但性能约为原始的90-95%）。
索引分为两个主要阶段。第一阶段，文档被收集、处理，并按关键字部分排序，结果写入临时文件（.tmp*）。第二阶段，文档被完全排序，创建最终的表文件。在运行时重建生产表大约需要峰值磁盘占用的3倍：一是用于中间临时文件，二是新构建的副本，三是旧表在此期间仍用于生产查询。（中间数据大小与最终表相当。）对于大型数据集合，这可能是过大的磁盘占用，而`inplace_enable`选项可以用来减少它。启用后，它重用临时文件，将最终数据写回它们，完成后重命名。但这可能需要额外的临时数据块重定位，这就是性能影响的来源。

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
inplace_reloc_factor设置决定索引期间内存区中的重定位缓冲区大小。默认值为0.1。
此选项为可选，仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)工具，不影响[searchd](../../Starting_the_server/Manually.md)服务器。

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
需要注意的是，该指令仅影响[indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)工具，而不影响[searchd](../../Starting_the_server/Manually.md)服务器。

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
### 自然语言处理专用设置
支持以下设置。它们都在章节[NLP和分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)中有描述。
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

