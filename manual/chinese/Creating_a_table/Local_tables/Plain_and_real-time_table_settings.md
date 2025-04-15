# 普通和实时表设置

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

### 普通和实时表设置的通用选项

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

表将存储或位于的路径，可以是绝对路径或相对路径，不带扩展名。

值：表的路径，**必须**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，当在配置文件中定义表时，全文字段的原始内容会被索引并存储。此设置允许您指定应该存储原始值的字段。

值：应存储的**全文**字段的逗号分隔列表。空值（即 `stored_fields =` ）会禁用所有字段的原始值存储。

注意：对于实时表，`stored_fields`中列出的字段也应声明为[rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另外，请注意，您不需要在 `stored_fields` 中列出属性，因为它们的原始值无论如何都会被存储。 `stored_fields` 只能用于全文字段。

另请参见 [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size)， [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) 以获取文档存储压缩选项。


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
  stored_fields = title, content # 我们只想存储“title”和“content”，“name”不应存储

  type = rt
  path = tbl
  rt_field = title
  rt_field = content
  rt_field = name
  rt_attr_uint = price
}
```
<!-- end -->

#### 存储仅字段

```ini
stored_only_fields = title,content
```

列出将存储在表中但不被索引的字段。此设置的工作方式类似于 [stored_fields](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)，但当字段在 `stored_only_fields` 中指定时，它只会被存储，不会被索引，且不能使用全文搜索查询。这些字段只能在搜索结果中检索。

值是一个以逗号分隔的字段列表，这些字段应该仅被存储，而不被索引。默认情况下，此值为空。如果正在定义实时表，`stored_only_fields` 中列出的字段必须也被声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

还请注意，您不需要在 `stored_only_fields` 中列出属性，因为它们的原始值无论如何都会被存储。如果将 `stored_only_fields` 与字符串属性进行比较，前者（存储字段）：
* 被存储在磁盘上，并且不占用内存
* 以压缩格式存储
* 只能被提取，不能用于排序、过滤或分组

与之相对，后者（字符串属性）：
* 被存储在磁盘上和内存中
* 以未压缩格式存储
* 可用于排序、分组、过滤，以及您希望对属性执行的任何其他操作。

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- 示例 json_secondary_indexes -->

默认情况下，所有属性生成二级索引，除了 JSON 属性。但是，可以使用 `json_secondary_indexes` 设置显式生成 JSON 属性的二级索引。当 JSON 属性包含在此选项中时，其内容会被展平为多个二级索引。这些索引可被查询优化器用于加速查询。

您可使用 [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md) 命令查看可用的二级索引。

值：应生成二级索引的 JSON 属性的逗号分隔列表。

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

预防在表中没有搜索时自动冲洗 RAM 块的超时。了解更多 [这里](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

如果没有写入，则自动冲洗 RAM 块的超时。了解更多 [这里](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout)。

#### optimize_cutoff

实时表的最大磁盘块数。了解更多 [这里](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks)。

#### rt_field

```ini
rt_field = subject
```

此字段声明确定将被索引的全文本字段。字段名称必须是唯一的，并且顺序被保留。在插入数据时，字段值必须与配置中指定的顺序相同。

这是一个多值的可选字段。

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

此声明定义了一个无符号整数属性。

值：字段名称或 field_name:N（其中 N 是要保留的最大位数）。

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

此声明定义了一个 BIGINT 属性。

值：字段名称，允许多个记录。

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

声明一个多值属性（MVA），其无符号 32 位整数值。

值：字段名称。允许多个记录。

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

声明一个具有带符号的 64 位 BIGINT 值的多值属性 (MVA)。

值：字段名称。允许多个记录。

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

声明具有单精度的浮点属性，采用 32 位 IEEE 754 格式。

值：字段名称。允许多个记录。

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

声明一个浮点值的向量。

值：字段名称。允许多个记录。

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

表的 RAM 块的内存限制。可选，默认为 128M。

RT 表将一些数据存储在内存中，称为“RAM 块”，并且还维护多个称为“磁盘块”的磁盘上的表。该指令允许您控制 RAM 块的大小。当内存中无法容纳太多数据时，RT 表将其刷新到磁盘，激活一个新创建的磁盘块，并重置 RAM 块。

请注意，限制是严格的，RT 表永远不会分配超过 rt_mem_limit 指定的内存。此外，内存不会预先分配，因此指定 512MB 限制并仅插入 3MB 数据将导致只分配 3MB，而不是 512MB。

`rt_mem_limit` 永远不会被超过，但实际的 RAM 块大小可能大大低于限制。RT 表根据数据插入的速度进行调整，并动态调整实际限制，以最小化内存使用并最大化数据写入速度。其工作原理如下：
* 默认情况下，RAM 块大小是 `rt_mem_limit` 的 50%，称为 "`rt_mem_limit`"。
* 一旦 RAM 块积累的数据等同于 `rt_mem_limit * rate` 数据（默认为 `rt_mem_limit` 的 50%），Manticore 就会开始将 RAM 块保存为一个新的磁盘块。
* 在保存新的磁盘块时，Manticore 评估新/更新文档的数量。
* 保存新磁盘块后，更新 `rt_mem_limit` 速率。
* 每次重新启动 searchd 时，速率将重置为 50%。

例如，如果 90MB 数据被保存到一个磁盘块，并且在保存过程中又到达了额外的 10MB 数据，那么速率将是 90%。下次，RT 表会在刷新数据之前收集多达 90% 的 `rt_mem_limit`。插入速度越快，`rt_mem_limit` 速率越低。速率在 33.3% 到 95% 之间变化。您可以使用 [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 命令查看表的当前速率。

##### 如何更改 rt_mem_limit 和 optimize_cutoff

在实时模式下，您可以使用 `ALTER TABLE` 语句调整 RAM 块的大小限制和磁盘块的最大数量。要将表“t”的 `rt_mem_limit` 设置为 1GB，请运行以下查询：`ALTER TABLE t rt_mem_limit='1G'`。要更改磁盘块的最大数量，请运行查询：`ALTER TABLE t optimize_cutoff='5'`。

在普通模式下，您可以通过更新表配置或运行命令 `ALTER TABLE <table_name> RECONFIGURE` 来更改 `rt_mem_limit` 和 `optimize_cutoff` 的值。

##### 关于 RAM 块的重要说明

* 实时表类似于由多个本地表组成的 [分布式](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table)，也称为磁盘块。
* 每个 RAM 块由多个段组成，这些段是特殊的仅 RAM 表。
* 磁盘块存储在磁盘上，而 RAM 块存储在内存中。
* 对实时表进行的每个事务都会生成一个新段，RAM 块段在每次事务提交后合并。进行数百或数千个文档的批量 INSERT 比多个单独文档的 INSERT 更有效，以减少合并 RAM 块段的开销。
* 当段的数量超过 32 时，它们将被合并以保持数量低于 32。
* 实时表始终有一个 RAM 块（可能为空）和一个或多个磁盘块。
* 合并较大的段需要更长时间，因此最好避免有一个非常大的 RAM 块（因此 `rt_mem_limit`）。
* 磁盘块的数量取决于表中的数据和 `rt_mem_limit` 设置。
* Searchd 在关闭时将 RAM 块刷新到磁盘（作为持久化文件，而不是作为磁盘块），并根据 [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) 设置定期执行此操作。将几GB 数据刷新到磁盘可能需要一些时间。
* 一个大的 RAM 块在刷新到磁盘中的 `.ram` 文件时，以及当 RAM 块满并转储到磁盘作为磁盘块时，会对存储施加更大压力。
* RAM 块在刷新到磁盘之前由 [二进制日志](../../Logging/Binary_logging.md) 进行备份，较大的 `rt_mem_limit` 设置会增加重放二进制日志和恢复 RAM 块所需的时间。
* RAM 块可能会比磁盘块稍慢。
* 尽管 RAM 块本身不会占用超过 `rt_mem_limit` 的内存，但在某些情况下 Manticore 可能会占用更多内存，例如当您开始一笔插入数据的事务而一段时间不提交时。在这种情况下，您在事务中已传输的数据将保留在内存中。

除了 `rt_mem_limit`，RAM 块的刷新行为还受到以下设置的影响: [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) 和 [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout)。

* `diskchunk_flush_write_timeout`: 此选项定义了如果没有写入操作，RAM 块自动刷新的超时。如果在此时间内没有写入，块将被刷新到磁盘。将其设置为 `-1` 将禁用基于写入活动的自动刷新。默认值为 1 秒。
* `diskchunk_flush_search_timeout`: 此选项设置了如果在表中没有搜索操作时，防止 RAM 块自动刷新的超时。只有在此时间内至少发生一次搜索时，才会发生自动刷新。默认值为 30 秒。

这些超时是协同工作的。如果达到 *任一* 超时，RAM 块将被刷新。这确保即使没有写入，数据最终也会被持久化到磁盘；反之，即使有持续的写入但没有搜索，数据也会被持久化。这些设置提供了更细粒度的控制，以平衡数据持久性的需求与性能考虑。每个表的这些设置指令具有更高优先级，并将覆盖实例范围的默认值。

### 普通表设置：

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

source 字段指定在当前表的索引过程中将从中获取文档的源。必须至少有一个源。源可以是不同类型（例如，一个可以是 MySQL，另一个可以是 PostgreSQL）。有关从外部存储进行索引的更多信息，请参阅 [从外部存储进行索引](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

值: 源的名称是 **必需**。允许多个值。

#### killlist_target

```ini
killlist_target = main:kl
```

此设置确定将应用杀死列表的表。当前表中更新或删除的目标表中的匹配项将被抑制。在 `:kl` 模式下，要抑制的文档来自 [杀死列表](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在 `:id` 模式下，当前表中的所有文档 ID 都将在目标表中被抑制。如果都没有指定，将会同时生效两个模式。[在这里了解有关杀死列表的更多信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

值: **未指定** (默认), target_table_name:kl, target_table_name:id, target_table_name。允许多个值。

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

此配置设置决定哪些属性应存储在 [列式存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 中，而不是行式存储中。

您可以设置 `columnar_attrs = *` 将所有支持的数据类型存储在列式存储中。

此外，`id` 是存储在列式存储中的受支持属性。

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

默认情况下，存储在列式存储中的所有字符串属性存储预先计算的哈希值。这些哈希值用于分组和过滤。然而，它们占用额外的空间，如果您不需要按该属性分组，可以通过禁用哈希生成来节省空间。

### 通过 CREATE TABLE 在线创建实时表

<!-- example rt_mode -->
##### CREATE TABLE 的通用语法

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### 数据类型：

有关数据类型的更多信息，请参阅 [数据类型的更多信息](../../Creating_a_table/Data_types.md)。

| 类型 | 配置文件中的等价物 | 说明 | 别名 |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | 选项: 索引, 存储。 默认: **两者**。 要保持文本存储但处于索引状态，仅需指定“存储”。 要仅保持文本索引，仅需指定“索引”。 | 字符串 |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| 整数	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| 大整数	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | 浮点数  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点值的向量  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | 多整数 |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多大整数  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值 |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项 `indexed, attribute` 将使值进行全文索引，并在同一时间可以过滤、排序和分组 |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳 |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N 是要保留的最大位数 |   |

<!-- intro -->
##### 创建实时表的示例，通过 CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

这创建了“products”表，其中包含两个字段：“title”（全文）和“price”（浮点），并将“morphology”设置为“stem_en”。

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
这创建了“products”表，其中包含三个字段：
* “title”已被索引，但未被存储。
* “description”已被存储，但未被索引。
* “author”既已存储又已索引。
<!-- end -->


## 引擎

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

引擎设置更改表中所有属性的默认 [属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。您还可以为每个属性单独指定 `engine` [](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages)。

有关如何为普通表启用列存储的信息，请参见 [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) 。

值：
* columnar - 为除 [json](../../Creating_a_table/Data_types.md#JSON) 以外的所有表属性启用列存储。
* **rowwise (默认)** - 不更改任何内容，并使用传统的行存储。


## 其他设置
以下设置适用于普通表和实时表，无论是通过配置文件指定还是通过 `CREATE` 或 `ALTER` 命令在线设置。

### 性能相关

#### 访问表文件
Manticore 支持两种读取表数据的访问模式：seek+read 和 mmap。

在 seek+read 模式下，服务器使用 `pread` 系统调用读取文档列表和关键字位置，由 `*.spd` 和 `*.spp` 文件表示。服务器使用内部读取缓冲区来优化读取过程，并且可以使用选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 调整这些缓冲区的大小。还有选项 [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) 控制 Manticore 启动时如何打开文件。

在 mmap 访问模式下，搜索服务器使用 `mmap` 系统调用将表的文件映射到内存中，操作系统缓存文件内容。选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 对此模式下的相应文件没有影响。mmap 读取器还可以使用 `mlock` 特权调用将表的数据锁定在内存中，以防止操作系统将缓存数据换出到磁盘。

要控制使用哪种访问模式，可以使用选项 **access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists** 和 **access_dict**，其值如下：

| 值 | 描述 |
| - | - |
| file | 服务器使用内部缓冲区从磁盘读取表文件，采用 seek+read 方式 |
| mmap | 服务器将表文件映射到内存中，操作系统在访问文件时缓存其内容 |
| mmap_preread | 服务器将表文件映射到内存中，并后台线程读取一次以预热缓存 |
| mlock | 服务器将表文件映射到内存中，然后执行 mlock() 系统调用以缓存文件内容并将其锁定在内存中，以防止其被换出 |


| 设置 | 值 | 描述 |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (默认), mlock | 控制如何读取 `*.spa`（普通属性） `*.spe`（跳过列表） `*.spt`（查找） `*.spm`（被杀文档） |
| access_blob_attrs   | mmap, **mmap_preread** (默认), mlock  | 控制如何读取 `*.spb`（blob 属性）（字符串、mva 和 json 属性） |
| access_doclists   | **file** (默认), mmap, mlock  | 控制如何读取 `*.spd`（文档列表）数据 |
| access_hitlists   | **file** (默认), mmap, mlock  | 控制如何读取 `*.spp`（命中列表）数据 |
| access_dict   | mmap, **mmap_preread** (默认), mlock  | 控制如何读取 `*.spi`（字典） |

这里有一个表可以帮助您选择所需的模式：

| 表部分 |	保留在磁盘上 |	保留在内存中 |	在服务器启动时缓存到内存 | 锁定在内存中 |
| - | - | - | - | - |
| plain attributes in [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (non-columnar) storage, skip lists, word lists, lookups, killed docs | 	mmap | mmap |	**mmap_preread** (默认) | mlock |
| row-wise string, multi-value attributes (MVA) and json attributes | mmap | mmap | **mmap_preread** (默认) | mlock |
| [columnar](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) 数字、字符串和多值属性 | 始终  | 仅通过操作系统  | 否  | 不支持 |
| 文档列表 | **文件**（默认） | mmap | 否	| mlock |
| 命中列表 | **文件**（默认） | mmap | 否	| mlock |
| 字典 | mmap | mmap | **mmap_preread**（默认） | mlock |

##### 推荐措施：

* 对于 **最快的搜索响应时间** 和充足的内存可用性，使用 [行式](../../Creating_a_table/Data_types.md#JSON) 属性，并使用 `mlock` 将其锁定在内存中。此外，对于文档列表/命中列表也使用 mlock。
* 如果您优先考虑 **启动后不能降低性能** 且愿意牺牲更长的启动时间，请使用 [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options) 选项。如果您希望更快的 searchd 重启，请坚持使用默认的 `mmap_preread` 选项。
* 如果您希望 **节省内存**，同时仍确保所有属性都有足够的内存，请跳过使用 `mlock`。操作系统将根据频繁的磁盘读取决定应保留在内存中的内容。
* 如果行式属性 **不适合内存**，请选择 [列式属性](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)
* 如果全文搜索 **性能不是问题**，并且您希望节省内存，请使用 `access_doclists/access_hitlists=file`

默认模式提供平衡：
* mmap，
* 预读取非列式属性，
* 以不预读的方式查找和读取列式属性，
* 以不预读的方式查找和读取文档列表/命中列表。

这在大多数情况下提供了不错的搜索性能、最佳的内存利用率和更快的 searchd 重启。

### 其他与性能相关的设置

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

此设置为 blob 属性（如多值属性、字符串和 JSON）更新保留额外的空间。默认值为 128k。更新这些属性时，其长度可能会改变。如果更新后的字符串比之前的短，它将覆盖 `*.spb` 文件中的旧数据。如果更新后的字符串更长，则将写入 `*.spb` 文件的末尾。该文件是内存映射的，这使得调整大小在某些情况下可能是一个慢过程，具体取决于操作系统的内存映射文件实现。为了避免频繁调整大小，您可以使用此设置在 .spb 文件末尾保留额外空间。

值：大小，默认 **128k**。

#### docstore_block_size

```ini
docstore_block_size = 32k
```

此设置控制文档存储中使用的块的大小。默认值为 16kb。当原始文档文本使用 stored_fields 或 stored_only_fields 存储时，它会存储在表中并进行压缩以提高效率。为了优化小文档的磁盘访问和压缩比，这些文档被串联成块。索引过程收集文档，直到它们的总大小达到该选项指定的阈值。在此时，文档块会被压缩。此选项可以调整，以实现更好的压缩比（通过增大块大小）或更快的文档文本访问（通过减小块大小）。

值：大小，默认 **16k**。

#### docstore_compression

```ini
docstore_compression = lz4hc
```

此设置确定用于压缩存储在文档存储中的文档块的压缩类型。如果指定了 stored_fields 或 stored_only_fields，文档存储将存储压缩的文档块。'lz4' 提供快速的压缩和解压缩速度，而 'lz4hc'（高压缩）则牺牲了一些压缩速度以获得更好的压缩比。'none' 完全禁用压缩。

值：**lz4**（默认），lz4hc，none。

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

在文档存储中应用 'lz4hc' 压缩时使用的压缩级别。通过调整压缩级别，您可以在使用 'lz4hc' 压缩时找到性能和压缩比之间的最佳平衡。请注意，此选项在使用 'lz4' 压缩时不适用。

值：介于 1 和 12 之间的整数，默认值为 **9**。

#### preopen

```ini
preopen = 1
```

此设置表示 searchd 应在启动或轮换时打开所有表文件，并在运行时保持打开。默认情况下，文件不会预先打开。预先打开的表每个表需要几个文件描述符，但它们消除了每个查询 open() 调用的需要，并且免受在高负载下可能发生的表轮换的竞争条件的影响。然而，如果您提供多个表，则仍然可能更有效地按查询逐个打开它们，以节省文件描述符。

值：**0**（默认）或 1。

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

用于存储每个关键词文档列表的缓冲区大小。增加此值会导致查询执行期间更高的内存使用，但可能会减少 I/O 时间。

值：大小，默认 **256k**，最小值为 8k。

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

用于存储每个关键词命中列表的缓冲区大小。增加此值会导致查询执行期间更高的内存使用，但可能会减少 I/O 时间。

值：大小，默认 **256k**，最小值为 8k。

### 普通表磁盘占用设置

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

启用就地表反转。可选，默认值为 0（使用单独的临时文件）。
`inplace_enable` 选项在索引普通表时减少磁盘占用，同时略微降低索引速度（它使用的磁盘空间约为原来的 2 倍，但性能约为原来的 90-95%）。

索引由两个主要阶段组成。在第一个阶段，收集文档，进行处理，并按关键字部分排序， intermediate results are written to temporary files (.tmp*)。在第二个阶段，文档被完全排序，最终表文件被创建。实时重建生产表大约需要 3 倍的峰值磁盘占用：首先用于中间临时文件，其次用于新构建的副本，最后用于在此期间将服务于生产查询的旧表。(中间数据的大小与最终表相当。) 对于大型数据集合，这可能会占用过多的磁盘空间，`inplace_enable` 选项可用于减少这种占用。当启用时，它会重用临时文件，将最终数据输出回其中，并在完成时重命名。不过，这可能需要额外的临时数据块重新定位，这也是性能影响的来源。

此指令对 [searchd](../../Starting_the_server/Manually.md) 没有影响，仅影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)。


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

选项 [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) 精细调整选项。 控制预分配命中列表间隔的大小。可选，默认为 0。

此指令仅影响 [searchd](../../Starting_the_server/Manually.md) 工具，并且不会对 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 产生任何影响。

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

inplace_reloc_factor 设置决定了在索引期间使用的内存区域内的重新定位缓冲区大小。默认值为 0.1。

此选项是可选的，仅影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具，而不是 [searchd](../../Starting_the_server/Manually.md) 服务器。

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

控制在索引期间用于就地写入的缓冲区大小。可选，默认值为 0.1。

重要的是要注意，此指令仅对 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) 工具产生影响，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。


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
支持以下设置。它们在 [NLP 和分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) 部分中都有描述。
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

