# Настройки обычных и реального времени таблиц

<!-- example config -->
## Определение схемы таблицы в файле конфигурации

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### Пример обычной таблицы в файле конфигурации
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
##### Пример таблицы реального времени в файле конфигурации
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

### Общие настройки для обычных таблиц и таблиц реального времени

#### type

```ini
type = plain

type = rt
```

Тип таблицы: "plain" (обычная) или "rt" (реального времени)

Значение: **plain** (по умолчанию), rt

#### path

```ini
path = path/to/table
```

Путь к месту хранения или расположения таблицы, абсолютный или относительный, без расширения.

Значение: Путь к таблице, **обязательный параметр**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

По умолчанию, при определении таблицы в файле конфигурации, исходное содержимое полнотекстовых полей индексируется и сохраняется. Данная настройка позволяет указать поля, для которых следует хранить исходные значения.

Значение: Список **полнотекстовых** полей, разделённых запятыми, для которых следует хранить исходные значения. Пустое значение (т.е. `stored_fields =` ) отключает хранение исходных значений для всех полей.

Примечание: В случае таблицы реального времени, поля, перечисленные в `stored_fields`, также должны быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также обратите внимание, что вам не нужно перечислять атрибуты в `stored_fields`, поскольку их исходные значения и так сохраняются. `stored_fields` может использоваться только для полнотекстовых полей.

См. также [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) для опций сжатия хранилища документов.


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

Используйте `stored_only_fields`, когда вы хотите, чтобы Manticore хранил некоторые поля обычной таблицы или таблицы реального времени **только на диске, но не индексировал их**. Эти поля не будут доступны для поиска с помощью полнотекстовых запросов, но вы всё равно сможете получать их значения в результатах поиска.

Например, это полезно, если вы хотите хранить данные, такие как JSON-документы, которые должны возвращаться с каждым результатом, но не нуждаются в поиске, фильтрации или группировке. В этом случае хранение их только на диске — без индексации — экономит память и повышает производительность.

Вы можете сделать это двумя способами:
- В [режиме plain](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) в конфигурации таблицы используйте настройку `stored_only_fields`.
- В SQL-интерфейсе ([режим RT](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) используйте свойство [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) при определении текстового поля (вместо `indexed` или `indexed stored`). В SQL вам не нужно включать `stored_only_fields` — эта опция не поддерживается в операторах `CREATE TABLE`.

Значение `stored_only_fields` — это список имён полей, разделённых запятыми. По умолчанию список пуст. Если вы используете таблицу реального времени, каждое поле, перечисленное в `stored_only_fields`, также должно быть объявлено как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Примечание: вам не нужно перечислять атрибуты в `stored_only_fields`, поскольку их исходные значения и так сохраняются.

Сравнение полей только для хранения со строковыми атрибутами:

- **Поле только для хранения (stored-only field)**:
  - Хранится только на диске
  - Сжатый формат
  - Может только извлекаться (не используется для фильтрации, сортировки и т.д.)

- **Строковый атрибут (string attribute)**:
  - Хранится на диске и в памяти
  - Несжатый формат (если вы не используете колоночное хранилище)
  - Может использоваться для сортировки, фильтрации, группировки и т.д.

Если вы хотите, чтобы Manticore хранил текстовые данные для вас **только** на диске (например, JSON-данные, которые возвращаются с каждым результатом), а не в памяти, и чтобы они не были доступны для поиска/фильтрации/группировки, используйте `stored_only_fields` или свойство `stored` для вашего текстового поля.

При создании таблиц с помощью SQL-интерфейса помечайте ваше текстовое поле как `stored` (а не `indexed` или `indexed stored`). Вам не понадобится опция `stored_only_fields` в вашем операторе `CREATE TABLE`; её включение может привести к неудачному выполнению запроса.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

По умолчанию вторичные индексы генерируются для всех атрибутов, кроме JSON-атрибутов. Однако вторичные индексы для JSON-атрибутов могут быть явно сгенерированы с помощью настройки `json_secondary_indexes`. Когда JSON-атрибут включён в эту опцию, его содержимое "разворачивается" в несколько вторичных индексов. Эти индексы могут использоваться оптимизатором запросов для ускорения выполнения запросов.

Вы можете просмотреть доступные вторичные индексы с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: Список JSON-атрибутов, разделённых запятыми, для которых должны быть сгенерированы вторичные индексы.

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

### Real-time table settings:

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

The timeout for preventing auto-flushing a RAM chunk if there are no searches in the table. Learn more [here](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

The timeout for auto-flushing a RAM chunk if there are no writes to it. Learn more [here](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

The maximum number of disk chunks for the RT table. Learn more [here](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

This field declaration determines the full-text fields that will be indexed. The field names must be unique, and the order is preserved. When inserting data, the field values must be in the same order as specified in the configuration.

This is a multi-value, optional field.

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

This declaration defines an unsigned integer attribute.

Value: the field name or field_name:N (where N is the maximum number of bits to keep).

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

This declaration defines a BIGINT attribute.

Value: field name, multiple records allowed.

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

Declares a multi-valued attribute (MVA) with unsigned 32-bit integer values.

Value: field name. Multiple records allowed.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Declares a multi-valued attribute (MVA) with signed 64-bit BIGINT values.

Value: field name. Multiple records allowed.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Declares floating point attributes with single precision, 32-bit IEEE 754 format.

Value: field name. Multiple records allowed.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

Declares a vector of floating-point values for storing embeddings and enabling k-nearest neighbor (KNN) vector searches.

Value: field name. Multiple records allowed.

Each vector attribute stores an array of floating-point numbers that represent data (such as text, images, or other content) as high-dimensional vectors. These vectors are typically generated by machine learning models and can be used for similarity search, recommendations, semantic search, and other AI-powered features.

**Important:** Float vector attributes require additional KNN configuration to enable vector search capabilities.

##### Configuring KNN for vector attributes

To enable KNN searches on float vector attributes, you must add a `knn` configuration that specifies the indexing parameters:

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**Required KNN parameters:**
- `name`: The name of the vector attribute (must match the `rt_attr_float_vector` name)
- `type`: Index type, currently only `"hnsw"` is supported
- `dims`: Number of dimensions in the vectors (must match your embedding model's output)
- `hnsw_similarity`: Distance function - `"L2"`, `"IP"` (inner product), or `"COSINE"`

**Optional KNN parameters:**
- `hnsw_m`: Maximum connections in the graph
- `hnsw_ef_construction`: Construction time/accuracy trade-off

For more details on KNN vector search, see the [KNN documentation](../../Searching/KNN.md).

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Declares a boolean attribute with 1-bit unsigned integer values.

Value: field name.

#### rt_attr_string

```ini
rt_attr_string = title
```

String attribute declaration.

Value: field name.

#### rt_attr_json

```ini
rt_attr_json = properties
```

Declares a JSON attribute.

Value: field name.

#### rt_attr_timestamp

```ini
rt_attr_timestamp = date_added
```

Declares a timestamp attribute.

Value: field name.

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

Memory limit for a RAM chunk of the table. Optional, default is 128M.

RT tables store some data in memory, known as the "RAM chunk", and also maintain a number of on-disk tables, referred to as "disk chunks." This directive allows you to control the size of the RAM chunk. When there is too much data to keep in memory, RT tables will flush it to disk, activate a newly created disk chunk, and reset the RAM chunk.

Please note that the limit is strict, and RT tables will never allocate more memory than what is specified in the rt_mem_limit. Additionally, memory is not preallocated, so specifying a 512MB limit and only inserting 3MB of data will result in allocating only 3MB, not 512MB.

The `rt_mem_limit` is never exceeded, but the actual RAM chunk size can be significantly lower than the limit. RT tables adapt to the data insertion pace and adjust the actual limit dynamically to minimize memory usage and maximize data write speed. This is how it works:
* By default, the RAM chunk size is 50% of the  `rt_mem_limit`, referred to as the  "`rt_mem_limit` limit".
* As soon as the RAM chunk accumulates data equivalent to `rt_mem_limit * rate` data (50% of `rt_mem_limit` by default), Manticore starts saving the RAM chunk as a new disk chunk.
* While a new disk chunk is being saved, Manticore assesses the number of new/updated documents.
* After saving a new disk chunk, the `rt_mem_limit` rate is updated.
* The rate is reset to 50% each time you restart the searchd.

For instance, if 90MB of data is saved to a disk chunk and an additional 10MB of data arrives while the save is in progress, the rate would be 90%. Next time, the RT table will collect up to 90% of `rt_mem_limit` before flushing the data. The faster the insertion pace, the lower the `rt_mem_limit` rate. The rate varies between 33.3% to 95%. You can view the current rate of a table using the [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) command.

##### How to change rt_mem_limit and optimize_cutoff

В режи real-time, можно ajust размер limit of RAM chunks и maximum number of disk chunks using `ALTER TABLE` statement. Для установ `rt_mem_limit` на 1 gigabyte для table "t", run query: `ALTER TABLE t rt_mem_limit='1G'`. Для change maximum number of disk chunks, run query: `ALTER TABLE t optimize_cutoff='5'`.

In plain mode, can change values of `rt_mem_limit` and `optimize_cutoff` by updating table configuration or running command `ALTER TABLE <table_name> RECONFIGURE`

##### Important notes about RAM chunks

* Real-time tables are similar to [distributed](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table) consisting of multiple local tables, also known as disk chunks.
* Each RAM chunk is made up of multiple segments, which are special RAM-only tables.
* While disk chunks are stored on disk, RAM chunks are stored in memory.
* Each transaction made to a real-time table generates a new segment, and RAM chunk segments are merged after each transaction commit. It is more efficient to perform bulk INSERTs of hundreds or thousands of documents rather than multiple separate INSERTs with one document to reduce the overhead from merging RAM chunk segments.
* When the number of segments exceeds 32, they will be merged to keep the count below 32.
* Real-time tables always have one RAM chunk (which may be empty) and one or more disk chunks.
* Merging larger segments takes longer, so it's best to avoid having a very large RAM chunk (and therefore `rt_mem_limit`).
* The number of disk chunks depends on the data in the table and the `rt_mem_limit` setting.
* Searchd flushes the RAM chunk to disk (as a persisted file, not as a disk chunk) on shutdown and periodically according to the [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period) setting. Flushing several gigabytes to disk may take some time.
* A large RAM chunk puts more pressure on storage, both when flushing to disk into the `.ram` file and when the RAM chunk is full and dumped to disk as a disk chunk.
* The RAM chunk is backed up by a [binary log](../../Logging/Binary_logging.md) until it is flushed to disk, and a larger `rt_mem_limit`, setting will increase the time it takes to replay the binary log and recover the RAM chunk.
* The RAM chunk may be slightly slower than a disk chunk.
* Although the RAM chunk itself doesn't take up more memory than `rt_mem_limit`, Manticore may take up more memory in some cases, such as when you start a transaction to insert data and don't commit it for a while. In this case, the data you have already transmitted within the transaction will remain in memory.

##### RAM chunk flushing conditions

In addition to `rt_mem_limit`, the flushing behavior of RAM chunks is also influenced by the following options and conditions:

* Frozen state. If the table is [frozen](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md), flushing is deferred. That is a permanent rule; nothing can override it. If the `rt_mem_limit` condition is reached while the table is frozen, all further inserts will be delayed until the table is unfrozen.

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout): This option defines the timeout (in seconds) for auto-flushing a RAM chunk if there are no writes to it.  If no write occurs within this time, the chunk will be flushed to disk. Setting it to `-1` disables auto-flushing based on write activity. The default value is 1 second.

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout): This option sets the timeout (in seconds) for preventing auto-flushing a RAM chunk if there are no searches in the table. Auto-flushing will only occur if there has been at least one search within this time. The default value is 30 seconds.

* ongoing optimization: If an optimization process is currently running, and the number of existing disk chunks has reached or exceeded a configured internal `cutoff` threshold, the flush triggered by the `diskchunk_flush_write_timeout` or `diskchunk_flush_search_timeout` timeout will be skipped.

* too few documents in RAM segments: If the number of documents across RAM segments is below a minimum threshold (8192), the flush triggered by the `diskchunk_flush_write_timeout` or `diskchunk_flush_search_timeout` timeout will be skipped to avoid creating very small disk chunks. This helps minimize unnecessary disk writes and chunk fragmentation.

These timeouts work in conjunction.  A RAM chunk will be flushed if *either* timeout is reached.  This ensures that even if there are no writes, the data will eventually be persisted to disk, and conversely, even if there are constant writes but no searches, the data will also be persisted.  These settings provide more granular control over how frequently RAM chunks are flushed, balancing the need for data durability with performance considerations.  Per-table directives for these settings have higher priority and will override the instance-wide defaults.

### Plain table settings:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

The source field specifies the source from which documents will be obtained during indexing of the current table. There must be at least one source. The sources can be of different types (e.g. one could be MySQL, another PostgreSQL). For more information on indexing from external storages, [read here](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Value: The name of the source is **mandatory**. Multiple values are allowed.

#### killlist_target

```ini
killlist_target = main:kl
```

Этот параметр определяет таблицу (или таблицы), к которой будет применен килл-лист. Совпадения в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавлены. В режиме `:kl` документы для подавления берутся из [килл-листа](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой. Если ни один из режимов не указан, будут действовать оба. [Подробнее о килл-листах здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Допускается несколько значений

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Эта настройка определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо построчного.

Вы можете установить `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колоночном хранилище.

Кроме того, `id` является поддерживаемым атрибутом для хранения в колоночном хранилище.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

По умолчанию все строковые атрибуты, хранящиеся в колоночном хранилище, сохраняют предварительно рассчитанные хэши. Эти хэши используются для группировки и фильтрации. Однако они занимают дополнительное место, и если вам не нужно группировать по этому атрибуту, вы можете сэкономить место, отключив генерацию хэшей.

### Создание таблицы в реальном времени онлайн с помощью CREATE TABLE

<!-- example rt_mode -->
##### Общий синтаксис CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Типы данных:

Для получения дополнительной информации о типах данных см. [подробнее о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: indexed, stored. По умолчанию: **оба**. Чтобы текст хранился, но не индексировался, укажите только "stored". Чтобы текст только индексировался, укажите только "indexed". | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| целое число	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| большое целое число	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | число с плавающей запятой  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор значений с плавающей запятой  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | многозначное целое число | mva |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | многозначное большое целое число  | mva64 |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | логический |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` сделает значение полнотекстово индексируемым и одновременно фильтруемым, сортируемым и группируемым  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | временная метка  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N - максимальное количество битов для хранения  |   |

<!-- intro -->
##### Примеры создания таблицы в реальном времени с помощью CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

Это создает таблицу "products" с двумя полями: "title" (полнотекстовое) и "price" (с плавающей запятой), и устанавливает "morphology" в "stem_en".

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
Это создает таблицу "products" с тремя полями:
* "title" индексируется, но не хранится.
* "description" хранится, но не индексируется.
* "author" и хранится, и индексируется.
<!-- end -->


## Engine

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

Настройка engine изменяет [хранилище атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) по умолчанию для всех атрибутов в таблице. Вы также можете указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).

Информацию о том, как включить колоночное хранилище для обычной таблицы, см. в [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) .

Значения:
* columnar - Включает колоночное хранилище для всех атрибутов таблицы, кроме [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - Ничего не меняет и использует традиционное построчное хранилище для таблицы.


## Другие настройки
Следующие настройки применимы как к таблицам в реальном времени, так и к обычным таблицам, независимо от того, указаны ли они в конфигурационном файле или установлены онлайн с помощью команд `CREATE` или `ALTER`.

### Связанные с производительностью

#### Доступ к файлам таблицы
Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.

В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, и размер этих буферов можно настроить с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также существует опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая управляет тем, как Manticore открывает файлы при запуске.

В режиме доступа mmap поисковый сервер отображает файлы таблицы в память с помощью системного вызова `mmap`, а ОС кэширует содержимое файлов. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не оказывают влияния на соответствующие файлы в этом режиме. Модуль чтения mmap также может блокировать данные таблицы в памяти с помощью привилегированного вызова `mlock`, что предотвращает вытеснение кэшированных данных на диск подкачки.

Для управления выбором режима доступа доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** со следующими значениями:

| Значение | Описание |
| - | - |
| file | сервер читает файлы таблицы с диска с помощью seek+read, используя внутренние буферы при доступе к файлу |
| mmap | сервер отображает файлы таблицы в память, и ОС кэширует их содержимое при доступе к файлу |
| mmap_preread | сервер отображает файлы таблицы в память, и фоновый поток читает их один раз для прогрева кэша |
| mlock | сервер отображает файлы таблицы в память, а затем выполняет системный вызов mlock() для кэширования содержимого файла и его блокировки в памяти, чтобы предотвратить вытеснение на диск |


| Настройка | Значения | Описание |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | управляет тем, как будут читаться `*.spa` (обычные атрибуты) `*.spe` (списки пропуска) `*.spt` (поисковые структуры) `*.spm` (удаленные документы) |
| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будут читаться `*.spb` (бинарные атрибуты) (строковые, MVA и JSON атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spp` (списки вхождений) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будет читаться `*.spi` (словарь) |

Вот таблица, которая поможет вам выбрать нужный режим:

| часть таблицы |	хранить на диске |	хранить в памяти |	кэшировать в памяти при запуске сервера | заблокировать в памяти |
| - | - | - | - | - |
| обычные атрибуты в [строчном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоночном), списки пропуска, списки слов, поисковые структуры, удаленные документы | 	mmap | mmap |	**mmap_preread** (по умолчанию) | mlock |
| строчные строковые атрибуты, многозначные атрибуты (MVA) и JSON атрибуты | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
| [Колоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и многозначные атрибуты | всегда  | только средствами ОС  | нет  | не поддерживается |
| списки документов | **file** (по умолчанию) | mmap | нет	| mlock |
| списки вхождений | **file** (по умолчанию) | mmap | нет	| mlock |
| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

##### Рекомендации:

* Для **максимально быстрого времени отклика поиска** и при наличии достаточного объема памяти используйте [строчные атрибуты](../../Creating_a_table/Data_types.md#JSON) и блокируйте их в памяти с помощью `mlock`. Кроме того, используйте mlock для списков документов/вхождений.
* Если для вас приоритетом является **недопустимость снижения производительности после запуска** и вы готовы принять более долгий запуск, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если вы хотите более быстрый перезапуск searchd, придерживайтесь опции по умолчанию `mmap_preread`.
* Если вы хотите **экономить память**, но при этом у вас достаточно памяти для всех атрибутов, не используйте `mlock`. Операционная система будет определять, что следует держать в памяти, на основе частоты чтений с диска.
* Если строчные атрибуты **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)
* Если **производительность полнотекстового поиска не критична**, и вы хотите сэкономить память, используйте `access_doclists/access_hitlists=file`

Режим по умолчанию предлагает баланс:
* mmap,
* Предварительное чтение неколоночных атрибутов,
* Поиск и чтение колоночных атрибутов без предварительного чтения,
* Поиск и чтение списков документов/вхождений без предварительного чтения.

Это обеспечивает достойную производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве сценариев.

### Другие настройки, связанные с производительностью

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Эта настройка резервирует дополнительное место для обновлений бинарных атрибутов, таких как многозначные атрибуты (MVA), строки и JSON. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может измениться. Если обновленная строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если обновленная строка длиннее, она будет записана в конец файла `*.spb`. Этот файл отображается в память, что делает его изменение потенциально медленной операцией, в зависимости от реализации файлов, отображаемых в память, в операционной системе. Чтобы избежать частого изменения размера, вы можете использовать эту настройку для резервирования дополнительного места в конце файла .spb.

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Этот параметр управляет размером блоков, используемых хранилищем документов. Значение по умолчанию составляет 16 КБ. Когда исходный текст документа сохраняется с использованием `stored_fields` или `stored_only_fields`, он хранится внутри таблицы и сжимается для эффективности. Для оптимизации доступа к диску и коэффициентов сжатия для небольших документов эти документы объединяются в блоки. Процесс индексации собирает документы до тех пор, пока их общий размер не достигнет порога, указанного этой опцией. В этот момент блок документов сжимается. Этот параметр можно настроить для достижения лучших коэффициентов сжатия (увеличивая размер блока) или более быстрого доступа к тексту документа (уменьшая размер блока).

Значение: размер, по умолчанию **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Этот параметр определяет тип сжатия, используемого для сжатия блоков документов, хранящихся в хранилище документов. Если указаны `stored_fields` или `stored_only_fields`, хранилище документов сохраняет сжатые блоки документов. 'lz4' обеспечивает высокую скорость сжатия и распаковки, в то время как 'lz4hc' (высокое сжатие) жертвует некоторой скоростью сжатия ради лучшего коэффициента сжатия. 'none' полностью отключает сжатие.

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Уровень сжатия, используемый при применении сжатия 'lz4hc' в хранилище документов. Настраивая уровень сжатия, можно найти правильный баланс между производительностью и коэффициентом сжатия при использовании сжатия 'lz4hc'. Обратите внимание, что эта опция не применима при использовании сжатия 'lz4'.

Значение: целое число от 1 до 12, по умолчанию **9**.

#### preopen

```ini
preopen = 1
```

Этот параметр указывает, что searchd должен открывать все файлы таблиц при запуске или ротации и держать их открытыми во время работы. По умолчанию файлы не открываются заранее. Предварительно открытые таблицы требуют несколько файловых дескрипторов на таблицу, но они устраняют необходимость в вызовах open() для каждого запроса и защищены от состояний гонки, которые могут возникнуть во время ротации таблиц под высокой нагрузкой. Однако, если вы обслуживаете много таблиц, может быть эффективнее открывать их для каждого запроса, чтобы экономить файловые дескрипторы.

Значение: **0** (по умолчанию) или 1.

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

Размер буфера для хранения списка документов на ключевое слово. Увеличение этого значения приведет к более высокому использованию памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: размер, по умолчанию **256k**, минимальное значение 8k.

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

Размер буфера для хранения списка вхождений (хитов) на ключевое слово. Увеличение этого значения приведет к более высокому использованию памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: размер, по умолчанию **256k**, минимальное значение 8k.

### Настройки дискового пространства для обычных таблиц

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

Включает инверсию таблицы на месте (in-place). Необязательный параметр, по умолчанию 0 (использует отдельные временные файлы).

Опция `inplace_enable` уменьшает занимаемое дисковое пространство во время индексации обычных таблиц, слегка замедляя процесс индексации (используется примерно в 2 раза меньше диска, но производительность составляет около 90-95% от исходной).

Индексация состоит из двух основных фаз. На первой фазе документы собираются, обрабатываются и частично сортируются по ключевым словам, а промежуточные результаты записываются во временные файлы (.tmp*). На второй фазе документы полностью сортируются и создаются окончательные файлы таблицы. Перестроение рабочей таблицы на лету требует примерно в 3 раза больше пикового дискового пространства: сначала для промежуточных временных файлов, затем для вновь созданной копии и, наконец, для старой таблицы, которая в это время обслуживает рабочие запросы. (Промежуточные данные сопоставимы по размеру с окончательной таблицей.) Это может быть слишком большим объемом дискового пространства для больших коллекций данных, и опция `inplace_enable` может быть использована для его уменьшения. При включении она повторно использует временные файлы, записывает в них окончательные данные и переименовывает их по завершении. Однако это может потребовать дополнительного перемещения фрагментов временных данных, что и приводит к снижению производительности.

Эта директива не влияет на [searchd](../../Starting_the_server/Manually.md), она затрагивает только утилиту [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).


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

Опция тонкой настройки [Инверсии на месте](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable). Управляет размером предварительно выделенного зазора (gap) для списка вхождений (hitlist). Необязательный параметр, по умолчанию 0.

Эта директива влияет только на утилиту [searchd](../../Starting_the_server/Manually.md) и не оказывает никакого влияния на утилиту [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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

Параметр `inplace_reloc_factor` определяет размер буфера перемещения (relocation buffer) в пределах области памяти (memory arena), используемой во время индексации. Значение по умолчанию составляет 0.1.

Эта опция является необязательной и влияет только на утилиту [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), а не на сервер [searchd](../../Starting_the_server/Manually.md).

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

Управляет размером буфера, используемого для записи на месте (in-place writing) во время индексации. Необязательный параметр со значением по умолчанию 0.1.

Важно отметить, что эта директива влияет только на утилиту [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) и не затрагивает сервер [searchd](../../Starting_the_server/Manually.md).


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

### Специфические настройки обработки естественного языка
Поддерживаются следующие настройки. Все они описаны в разделе [NLP и токенизация](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md).
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

