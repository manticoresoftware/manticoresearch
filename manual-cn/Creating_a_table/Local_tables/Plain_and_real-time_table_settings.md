# 普通表和实时表设置

<!-- example config -->
## 在配置文件中定义表架构

```ini
table <index_name>[:<parent table name>] {
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

### 通用平面和实时表设置

#### type

```ini
type = plain

type = rt
```

表类型: "plain" or "rt" (real-time)

值: **plain** (默认), rt

#### path

```ini
path = path/to/table
```

表将被存储或放置的位置路径，可以是绝对路径或相对路径，且不包含扩展名。

值：表的路径，**必填**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

默认情况下，当在配置文件中定义表时，全文字段的原始内容会被索引和存储。此设置允许您指定应存储其原始值的字段。

值：要存储的**全文**字段的逗号分隔列表。空值（即 `stored_fields =`）会禁用所有字段的原始值存储。

注意：在实时表的情况下，`stored_fields` 中列出的字段也应声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另外，注意您不需要在 `stored_fields` 中列出属性，因为它们的原始值无论如何都会被存储。`stored_fields` 只能用于全文字段。

另请参见 [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size)、[docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression)，以获取文档存储压缩选项。


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
    'index' => 'products'
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

将存储在表中但不被索引的字段列表。此设置的工作方式与 [存储字段](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) 类似，不同之处在于，当字段在 `stored_only_fields` 中指定时，它仅会被存储，而不会被索引，因此无法使用全文查询进行搜索。它只能在搜索结果中检索。

值是一个以逗号分隔的字段列表，这些字段应仅存储而不被索引。默认情况下，此值为空。如果正在定义实时表，则 `stored_only_fields` 中列出的字段必须也声明为 [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)。

另请注意，您不需要在 `stored_only_fields` 中列出属性，因为它们的原始值无论如何都会被存储。将 `stored_only_fields` 与字符串属性进行比较时，前者（存储字段）：

- 存储在磁盘上，不占用内存
- 以压缩格式存储
- 只能被获取，不能用于排序、过滤或分组

相对而言，后者（字符串属性）：

- 存储在磁盘上和内存中
- 以未压缩格式存储
- 可用于排序、分组、过滤及任何您想要对属性进行的其他操作。

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

默认情况下，所有属性都会生成二级索引，除了 JSON 属性。然而，可以通过 `json_secondary_indexes` 设置显式生成 JSON 属性的二级索引。当 JSON 属性包含在此选项中时，其内容会被展平为多个二级索引。这些索引可以被查询优化器用于加速查询。

值：一个以逗号分隔的 JSON 属性列表，用于生成二级索引。

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
    'index' => 'products'
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

### 实时表设置

#### optimize_cutoff

实时表的最大磁盘块数量。了解更多信息 [这里](../../Securing_and_compacting_a_table/Compacting_a_table.md#优化后的磁盘块数量)。

#### rt_field

```ini
rt_field = subject
```

此字段声明确定将被索引的全文本字段。字段名称必须是唯一的，顺序会被保留。在插入数据时，字段值必须与配置中指定的顺序相同。

这是一个多值的可选字段。

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

此声明定义了一个无符号整数属性。

值：字段名称或字段名称

（其中 N 是要保留的最大位数）。

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

声明一个具有无符号 32 位整数值的多值属性（MVA）。

值：字段名称。允许多个记录。

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

声明一个具有有符号 64 位 BIGINT 值的多值属性（MVA）。

值：字段名称。允许多个记录。

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

声明使用单精度、32 位 IEEE 754 格式的浮点属性。

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

表的 RAM 块的内存限制。可选，默认值为 128M。

RT 表将一些数据存储在内存中，称为“RAM 块”，并维护多个称为“磁盘块”的磁盘表。此指令允许您控制 RAM 块的大小。当内存中要保留的数据过多时，RT 表将其刷新到磁盘，激活新创建的磁盘块，并重置 RAM 块。

请注意，限制是严格的，RT 表永远不会分配超过 `rt_mem_limit` 指定的内存。此外，内存不会预分配，因此指定 512MB 限制并只插入 3MB 数据将导致只分配 3MB，而不是 512MB。

`rt_mem_limit` 从不被超出，但实际的 RAM 块大小可能显著低于限制。RT 表根据数据插入速度进行调整，动态调整实际限制以最小化内存使用并最大化数据写入速度。其工作原理如下：

- 默认情况下，RAM 块大小为 `rt_mem_limit` 的 50%，称为 "`rt_mem_limit`"。
- 一旦 RAM 块累计的数据达到 `rt_mem_limit * rate`（默认为 `rt_mem_limit` 的 50%），Manticore 开始将 RAM 块保存为新的磁盘块。
- 在保存新的磁盘块时，Manticore 评估新增/更新的文档数量。
- 保存新的磁盘块后，更新 `rt_mem_limit` 速率。
- 每次重启 `searchd` 时，速率都会重置为 50%。

例如，如果 90MB 的数据被保存到磁盘块，并且在保存过程中再到达 10MB 的数据，速率将达到 90%。下次，RT 表将在刷新数据之前收集最多 90% 的 `rt_mem_limit`。插入速度越快，`rt_mem_limit` 速率越低。速率在 33.3% 到 95% 之间变化。您可以使用 [SHOW TABLE  STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) 命令查看表的当前速率。

##### 如何更改 rt_mem_limit 和 optimize_cutoff

在实时模式下，您可以使用 `ALTER TABLE` 语句调整 RAM 块的大小限制和最大磁盘块数。要将表“t”的 `rt_mem_limit` 设置为 1GB，请运行以下查询：`ALTER TABLE t rt_mem_limit='1G'`。要更改最大磁盘块数，请运行查询：`ALTER TABLE t optimize_cutoff='5'`。

在普通模式下，您可以通过更新表配置或运行命令 `ALTER TABLE <index_name> RECONFIGURE` 来更改 `rt_mem_limit` 和 `optimize_cutoff` 的值。

##### 关于 RAM 块的重要说明

- 实时表类似于包含多个本地表的 [分布式](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#创建本地分布式表)，也称为磁盘块。
- 每个 RAM 块由多个段组成，这些段是特殊的仅在 RAM 中的表。
- 磁盘块存储在磁盘上，而 RAM 块存储在内存中。
- 对实时表进行的每个事务都会生成一个新的段，而 RAM 块段会在每次事务提交后合并。执行数百或数千个文档的批量 INSERT 比多次单独插入一条文档更有效，以减少合并 RAM 块段的开销。
- 当段的数量超过 32 时，它们将被合并，以保持数量低于 32。
- 实时表始终有一个 RAM 块（可能为空）和一个或多个磁盘块。
- 合并较大的段需要更长的时间，因此最好避免具有非常大的 RAM 块（因此 `rt_mem_limit`）。
- 磁盘块的数量取决于表中的数据和 `rt_mem_limit` 设置。
- Searchd 根据 [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) 设置定期将 RAM 块刷新到磁盘（作为持久文件，而不是磁盘块）。将几 GB 的数据刷新到磁盘可能需要一些时间。
- 大 RAM 块在将数据刷新到磁盘的 `.ram` 文件时以及当 RAM 块满并转储到磁盘作为磁盘块时，会对存储施加更大压力。
- RAM 块在刷新到磁盘之前会被 二进制日志 备份，而较大的 `rt_mem_limit` 设置会增加重放二进制日志和恢复 RAM 块所需的时间。
- RAM 块可能比磁盘块稍慢。
- 尽管 RAM 块本身不会占用超过 `rt_mem_limit` 的内存，但在某些情况下，Manticore 可能会占用更多内存，例如当您开始一个事务以插入数据并且一段时间内不提交时。在这种情况下，您在事务中已经传输的数据将保留在内存中。

### 普通表设置：

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

source 字段指定在当前表索引过程中获取文档的来源。至少必须有一个来源。这些来源可以是不同类型的（例如，一个可以是 MySQL，另一个可以是 PostgreSQL）。有关从外部存储进行索引的更多信息，请参阅 [从外部存储进行索引](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)。

值：源的名称是 **必填**。允许多个值。

#### killlist_target

```ini
killlist_target = main:kl
```

此设置确定将应用于 kill-list 的表。如果在当前表中更新或删除的匹配项在目标表中存在，则将被抑制。在 `:kl` 模式下，要抑制的文档取自 [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在 `:id` 模式下，当前表中的所有文档 ID 在目标表中都将被抑制。如果都不指定，则两种模式均会生效。[了解更多关于 kill-lists 的信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。

值：**未指定**（默认）、target_index_name

、target_index_name

、target_index_name。允许多个值。

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

### 平面表设置：

#### source

```
ini复制代码source = srcpart1
source = srcpart2
source = srcpart3
```

source 字段指定在当前表索引过程中获取文档的来源。至少必须有一个来源。这些来源可以是不同类型的（例如，一个可以是 MySQL，另一个可以是 PostgreSQL）。有关从外部存储进行索引的更多信息，请参阅 [从外部存储进行索引](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)。

值：源的名称是 **必填**。允许多个值。

#### killlist_target

```
ini


复制代码
killlist_target = main:kl
```

此设置确定将应用于 kill-list 的表。如果在当前表中更新或删除的匹配项在目标表中存在，则将被抑制。在 `:kl` 模式下，要抑制的文档取自 [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。在 `:id` 模式下，当前表中的所有文档 ID 在目标表中都将被抑制。如果都不指定，则两种模式均会生效。[了解更多关于 kill-lists 的信息](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)。

值：**未指定**（默认）、target_index_name

、target_index_name

、target_index_name。允许多个值。



#### columnar_attrs

```
ini复制代码columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

此配置设置决定哪些属性应存储在 [列式存储](../../Creating_a_table/Data_types.md#行存储和列存储属性) 中，而不是行式存储。

您可以设置 `columnar_attrs = *` 将所有支持的数据类型存储在列式存储中。

此外，`id` 是一个支持存储在列式存储中的属性。

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

默认情况下，所有存储在列式存储中的字符串属性都会存储预计算的哈希值。这些哈希值用于分组和过滤。然而，它们会占用额外的空间，如果您不需要按该属性进行分组，则可以通过禁用哈希生成来节省空间。

### 通过CREATE TABLE在线创建实时表

<!-- example rt_mode -->
##### CREATE TABLE的通用语法

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### 数据类型：

有关数据类型的更多信息，请参阅[有关数据类型的更多信息](../../Creating_a_table/Data_types.md)。

| 类型                                                         | 配置文件中的等效项                                           | 说明                                                         | 别名      |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ | --------- |
| [text](../../Creating_a_table/Data_types.md#文本-Text)            | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field) | 选项：indexed, stored。默认值：**both**。要仅保留文本存储但索引，请仅指定“stored”。要仅保留文本索引，请仅指定“indexed”。 | string    |
| [integer](../../Creating_a_table/Data_types.md#整数Integer)      | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | integer                                                      | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#大整数Big-Integer)   | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint) | big integer                                                  |           |
| [float](../../Creating_a_table/Data_types.md#浮点数Float)          | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float) | float                                                        |           |
| [float_vector](../../Creating_a_table/Data_types.md#浮点向量Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | 浮点值向量                                                   |           |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-(MVA)) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi) | 多个整数                                                     |           |
| [multi64](../../Creating_a_table/Data_types.md#多值大整数Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | 多个大整数                                                   |           |
| [bool](../../Creating_a_table/Data_types.md#布尔值Boolean)         | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | 布尔值                                                       |           |
| [json](../../Creating_a_table/Data_types.md#JSON)            | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON                                                         |           |
| [string](../../Creating_a_table/Data_types.md#字符串String)        | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | 字符串。选项`indexed, attribute`将使值进行全文索引，并同时可过滤、排序和分组。 |           |
| [timestamp](../../Creating_a_table/Data_types.md#时间戳Timestamp) | [rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | 时间戳                                                       |           |
| [bit(n)](../../Creating_a_table/Data_types.md#整数Integer)       | [rt_attr_uint field_name](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N是保留的最大位数                                            |           |

<!-- intro -->

##### 通过CREATE TABLE创建实时表的示例
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

这将创建一个名为“products”的表，其中包含两个字段：“title”（全文）和“price”（浮点数），并将“morphology”设置为“stem_en”。

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
这将创建一个名为“products”的表，其中包含三个字段：

- “title”被索引，但未存储。
- “description”被存储，但未被索引。
- “author”同时被存储和索引。

<!-- end -->


## 引擎

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

`engine` 设置更改表中所有属性的默认 [属性存储](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)。你还可以为每个属性单独指定 `engine` [设置](../../Creating_a_table/Data_types.md#如何在存储方式之间切换)。

有关如何为普通表启用列式存储的信息，请参阅 [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs)。

值：

- `columnar` - 为所有表属性启用列式存储，除了 [json](../../Creating_a_table/Data_types.md#JSON)
- **rowwise（默认）** - 不做任何更改，并使用传统的行式存储。


## 其他设置
以下设置适用于实时表和普通表，无论是指定在配置文件中，还是通过 `CREATE` 或 `ALTER` 命令在线设置。

### 性能相关

#### 访问表文件
Manticore 支持两种读取表数据的访问模式：seek+read 和 mmap。

在 seek+read 模式下，服务器使用 `pread` 系统调用读取文档列表和关键字位置，这些位置由 `*.spd` 和 `*.spp` 文件表示。服务器使用内部读取缓冲区来优化读取过程，这些缓冲区的大小可以通过选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 调整。还有一个选项 [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) 控制 Manticore 启动时如何打开文件。

在 mmap 访问模式下，搜索服务器使用 `mmap` 系统调用将表文件映射到内存中，操作系统缓存文件内容。选项 [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) 和 [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) 对该模式下的相应文件无效。mmap 读取器还可以使用 `mlock` 特权调用锁定表的数据在内存中，防止操作系统将缓存的数据换出到磁盘。

要控制使用哪种访问模式，可以使用以下选项 **access_plain_attrs**、**access_blob_attrs**、**access_doclists**、**access_hitlists** 和 **access_dict**，其值如下：

| 值           | 描述                                                         |
| ------------ | ------------------------------------------------------------ |
| file         | 服务器使用 seek+read 从磁盘读取表文件，使用内部缓冲区进行文件访问 |
| mmap         | 服务器将表文件映射到内存，操作系统在文件访问时缓存其内容     |
| mmap_preread | 服务器将表文件映射到内存，后台线程先读取一次以预热缓存       |
| mlock        | 服务器将表文件映射到内存，然后执行 mlock() 系统调用以缓存文件内容并将其锁定在内存中，防止其被换出 |

| 设置               | 值                                    | 描述                                                         |
| ------------------ | ------------------------------------- | ------------------------------------------------------------ |
| access_plain_attrs | mmap、**mmap_preread**（默认）、mlock | 控制 `*.spa`（普通属性）`*.spe`（跳过列表）`*.spt`（查找）`*.spm`（已删除文档）的读取方式 |
| access_blob_attrs  | mmap、**mmap_preread**（默认）、mlock | 控制 `*.spb`（Blob 属性）（字符串、MVA 和 JSON 属性）的读取方式 |
| access_doclists    | **file**（默认）、mmap、mlock         | 控制 `*.spd`（文档列表）数据的读取方式                       |
| access_hitlists    | **file**（默认）、mmap、mlock         | 控制 `*.spp`（命中列表）数据的读取方式                       |
| access_dict        | mmap、**mmap_preread**（默认）、mlock | 控制 `*.spi`（字典）的读取方式                               |

以下表格可帮助你选择所需的模式：

| 表部分                                                       | 保持在磁盘上     | 保持在内存中   | 服务器启动时缓存         | 锁定在内存中 |
| ------------------------------------------------------------ | ---------------- | -------------- | ------------------------ | ------------ |
| 普通属性在 [行式](../../Creating_a_table/Data_types.md#行存储和列存储属性) 存储（非列式）中，跳过列表、字词列表、查找、已删除文档 | mmap             | mmap           | **mmap_preread**（默认） | mlock        |
| 行式字符串、多值属性（MVA）和 JSON 属性                      | mmap             | mmap           | **mmap_preread**（默认） | mlock        |
| [列式](../../Creating_a_table/Data_types.md#行存储和列存储属性) 数值、字符串和多值属性 | 始终             | 仅通过操作系统 | 无                       | 不支持       |
| 文档列表                                                     | **file**（默认） | mmap           | 无                       | mlock        |
| 命中列表                                                     | **file**（默认） | mmap           | 无                       | mlock        |
| 字典                                                         | mmap             | mmap           | **mmap_preread**（默认） | mlock        |

##### 建议如下：

- 为了获得**最快的搜索响应时间**并有足够的内存，使用 [行式](../../Creating_a_table/Data_types.md#JSON) 属性，并通过 `mlock` 将其锁定在内存中。此外，还应对文档列表/命中列表使用 `mlock`。
- 如果你优先考虑**启动后无法承受较低的性能**，并愿意牺牲更长的启动时间，请使用 [--force-preread](../../Starting_the_server/Manually.md#searchd-命令行选项) 选项。如果你希望更快的 searchd 重启，请坚持使用默认的 `mmap_preread` 选项。
- 如果你希望**节省内存**，同时仍然有足够的内存存储所有属性，请跳过使用 `mlock`。操作系统将决定哪些内容应保持在内存中，基于频繁的磁盘读取。
- 如果行式属性**不适合内存**，则选择 [列式属性](../../Creating_a_table/Data_types.md#行存储和列存储属性)。
- 如果全文搜索**性能不是问题**，并希望节省内存，请使用 `access_doclists/access_hitlists=file`。

默认模式提供了一个平衡：

- mmap，
- 预读取非列式属性，
- 使用无预读取的方式查找和读取列式属性，
- 使用无预读取的方式查找和读取文档列表/命中列表。

在大多数场景中，这提供了良好的搜索性能、最佳的内存利用率和更快的 searchd 重启。

### 其他性能相关设置

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

此设置为更新 Blob 属性（如多值属性（MVA）、字符串和 JSON）保留额外空间。默认值为 128k。在更新这些属性时，它们的长度可能会改变。如果更新后的字符串短于之前的字符串，它将覆盖 `*.spb` 文件中的旧数据。如果更新后的字符串更长，则将写入 `*.spb` 文件的末尾。该文件是内存映射的，这使得调整大小的过程可能会较慢，具体取决于操作系统的内存映射文件实现。为了避免频繁的调整大小，可以使用此设置在 .spb 文件末尾保留额外空间。

值：大小，默认 **128k**。

#### docstore_block_size

```ini
docstore_block_size = 32k
```

此设置控制文档存储使用的块大小。默认值为 16kb。当原始文档文本使用 `stored_fields` 或 `stored_only_fields` 存储时，它会在表内存储并进行压缩以提高效率。为了优化小文档的磁盘访问和压缩比，这些文档被连接成块。索引过程收集文档，直到其总大小达到此选项指定的阈值。此时，文档块会被压缩。可以调整此选项以获得更好的压缩比（通过增大块大小）或更快的文档文本访问（通过减小块大小）。

值：大小，默认 **16k**。

#### docstore_compression

```ini
docstore_compression = lz4hc
```

此设置决定用于压缩存储在文档存储中的文档块的压缩类型。如果指定了 `stored_fields` 或 `stored_only_fields`，文档存储将存储压缩的文档块。`lz4` 提供快速的压缩和解压速度，而 `lz4hc`（高压缩）则在获得更好压缩比的同时牺牲了一些压缩速度。`none` 完全禁用压缩。

值：**lz4**（默认）、`lz4hc`、`none`。

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

在文档存储中应用 `lz4hc` 压缩时使用的压缩级别。通过调整压缩级别，你可以在使用 `lz4hc` 压缩时找到性能与压缩比之间的最佳平衡。请注意，当使用 `lz4` 压缩时，此选项不适用。

值：介于 1 到 12 之间的整数，默认 **9**。

#### preopen

```ini
preopen = 1
```

此设置指示 `searchd` 在启动或轮换时打开所有表文件，并在运行时保持它们处于打开状态。默认情况下，文件不会预先打开。预打开的表每个表需要一些文件描述符，但它们消除了每次查询的 open() 调用的需要，并且在高负载下的表轮换期间不会发生竞争条件。然而，如果你服务许多表，仍然可能更有效地按查询打开它们，以节省文件描述符。

值：**0**（默认）或 1。

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

用于存储每个关键字文档列表的缓冲区大小。增加此值将导致查询执行期间的内存使用量增加，但可能减少 I/O 时间。

值：大小，默认 **256k**，最小值为 8k。

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

用于存储每个关键字命中列表的缓冲区大小。增加此值将导致查询执行期间的内存使用量增加，但可能减少 I/O 时间。

值：大小，默认 **256k**，最小值为 8k。

### 普通表磁盘占用设置

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

启用就地表反转。可选，默认值为 0（使用单独的临时文件）。

`inplace_enable` 选项在索引普通表时减少磁盘占用，但会稍微降低索引速度（大约使用 2 倍更少的磁盘，但性能约为原始性能的 90-95%）。

索引分为两个主要阶段。在第一个阶段，文档被收集、处理并按关键字部分排序，结果中间结果被写入临时文件（.tmp*）。在第二个阶段，文档被完全排序，最终的表文件被创建。实时重建生产表需要大约 3 倍的峰值磁盘占用：首先是中间临时文件，其次是新构建的副本，最后是将在此期间服务生产查询的旧表。（中间数据的大小与最终表相当。）这对于大型数据集可能过于占用磁盘，而 `inplace_enable` 选项可以用来减少占用。当启用时，它重用临时文件，将最终数据输出回这些文件，并在完成后重命名它们。然而，这可能需要额外的临时数据块重新定位，这就是性能影响的来源。

此指令对 [searchd](../../Starting_the_server/Manually.md) 没有影响，仅对 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具) 产生影响。


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

此选项是 [inplace_enable](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) 的微调选项。控制预分配的命中列表间隙大小。可选，默认值为 0。

此指令仅影响 [searchd](../../Starting_the_server/Manually.md) 工具，对 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具) 没有影响。

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

`inplace_reloc_factor` 设置决定了在索引期间使用的内存区域内重新定位缓冲区的大小。默认值为 0.1。

此选项是可选的，仅影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具) 工具，而不影响 [searchd](../../Starting_the_server/Manually.md) 服务器。

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

控制索引过程中就地写入时使用的缓冲区大小。可选，默认值为 0.1。

需要注意的是，此指令仅影响 [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-工具) 工具，对 [searchd](../../Starting_the_server/Manually.md) 服务器没有影响。


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
支持以下设置。它们都在 [NLP 和分词](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md) 部分进行了描述。
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
