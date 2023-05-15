# Plain and real-time table settings

<!-- example config -->
## Defining table schema in a configuration file

```ini
table <index_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### Example of a plain table in a configuration file
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
##### Example of a real-time table in a configuration file
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

### Common plain and real-time tables settings

#### type

```ini
type = plain

type = rt
```

Table type: "plain" or "rt" (real-time)

Value: **plain** (default), rt

#### path

```ini
path = path/to/table
```

The path to where the table will be stored or located, either absolute or relative, without the extension.

Value: The path to the table, **mandatory**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

By default, the original content of full-text fields is indexed and stored when a table is defined in a configuration file. This setting allows you to specify the fields that should have their original values stored.

Value: A comma-separated list of **full-text** fields that should be stored. An empty value (i.e. `stored_fields =` ) disables the storage of original values for all fields.

Note: In the case of a real-time table, the fields listed in `stored_fields` should also be declared as [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Also, note that you don't need to list attributes in `stored_fields`, since their original values are stored anyway. `stored_fields` can only be used for full-text fields.

See also [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) for document storage compression options.


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

List of fields that will be stored in the table, but not indexed. This setting works similarly to [stored_fields](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields) except that when a field is specified in `stored_only_fields` t will only be stored, not indexed, and cannot be searched using full-text queries. It can only be retrieved in search results.

The value is a comma-separated list of fields that should be stored only, not indexed. By default, this value is empty. If a real-time table is being defined, the fields listed in `stored_only_fields` must also be declared as [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Note also, that you don't need to list attributes in`stored_only_fields`,since their original values are stored anyway. If to compare `stored_only_fields`  to string attributes the former (stored field):
* is stored on disk and does not take up memory
* is stored in a compressed format
* can only be fetched, it cannot be used for sorting, filtering or grouping

In contrast, the latter (string attribute):
* is stored on disk and in memory
* is stored in an uncompressed format
* can be used for sorting, grouping, filtering, and any other actions you want to take with attributes.

### Real-time table settings:

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

RT tables store some data in memory, known as the "RAM chunk," and also maintain a number of on-disk tables, referred to as "disk chunks." This directive allows you to control the size of the RAM chunk. When there is too much data to keep in memory, RT tables will flush it to disk, activate a newly created disk chunk, and reset the RAM chunk.

Please note that the limit is strict, and RT tables will never allocate more memory than what is specified in the rt_mem_limit. Additionally, memory is not preallocated, so specifying a 512MB limit and only inserting 3MB of data will result in allocating only 3MB, not 512MB.

The `rt_mem_limit` is never exceeded, but the actual RAM chunk size can be significantly lower than the limit. RT tables adapt to the data insertion pace and adjust the actual limit dynamically to minimize memory usage and maximize data write speed. This is how it works:
* By default, the RAM chunk size is 50% of the  `rt_mem_limit`, referred to as the  "`rt_mem_limit`".
* As soon as the RAM chunk accumulates data equivalent to `rt_mem_limit * rate` data (50% of `rt_mem_limit`  by default), Manticore starts saving the RAM chunk as a new disk chunk.
* While a new disk chunk is being saved, Manticore assesses the number of new/updated documents.
* After saving a new disk chunk, the `rt_mem_limit` rate is updated.
* The rate is reset to 50% each time you restart the searchd.

For instance, if 90MB of data is saved to a disk chunk and an additional 10MB of data arrives while the save is in progress, the rate would be 90%. Next time, the RT table will collect up to 90% of `rt_mem_limit` before flushing the data. The faster the insertion pace, the lower the `rt_mem_limit` rate. The rate varies between 33.3% to 95%. You can view the current rate of a table using the [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md) command.

##### How to change rt_mem_limit and optimize_cutoff

In real-time mode, you can adjust the size limit of RAM chunks and the maximum number of disk chunks using the `ALTER TABLE` statement. To set `rt_mem_limit` to 1 gigabyte for the table "t," run the following query: `ALTER TABLE t rt_mem_limit='1G'`. To change the maximum number of disk chunks, run the query: `ALTER TABLE t optimize_cutoff='5'`.

In the plain mode, you can change the values of `rt_mem_limit` and `optimize_cutoff` by updating the table configuration or running the command `ALTER TABLE <index_name> RECONFIGURE`

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
* Although the RAM chunk itself doesn't take up more memory than `rt_mem_limit` Manticore may take up more memory in some cases, such as when you start a transaction to insert data and don't commit it for a while. In this case, the data you have already transmitted within the transaction will remain in memory.

### Plain table settings:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

The source field specifies the source from which documents will be obtained during indexing of the current table. There must be at least one source. The sources can be of different types (e.g. one could be MySQL, another PostgreSQL). For more information on indexing from external storages, [indexing from external storages here](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Value: The name of the source is **mandatory**. Multiple values are allowed.

#### killlist_target

```ini
killlist_target = main:kl
```

This setting determines the table(s) to which the kill-list will be applied. Matches in the targeted table that are updated or deleted in the current table will be suppressed. In `:kl` mode, the documents to suppress are taken from the [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). In `:id` mode, all document IDs from the current table are suppressed in the targeted one. If neither is specified, both modes will take effect. [Learn more about kill-lists here](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Value: **not specified** (default), target_index_name:kl, target_index_name:id, target_index_name. Multiple values are allowed

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

This configuration setting determines which attributes should be stored in [the columnar storage](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) instead of the row-wise storage.

You can set `columnar_attrs = *` to store all supported data types in the columnar storage.

Additionally, `id` is a supported attribute to store in the columnar storage.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

By default, all string attributes stored in columnar storage store pre-calculated hashes. These hashes are used for grouping and filtering. However, they occupy extra space, and if you don't need to group by that attribute, you can save space by disabling hash generation.

### Creating a real-time table online via CREATE TABLE

<!-- example rt_mode -->
##### General syntax of CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Data types:

For more information on data types, see [more about data types here](../../Creating_a_table/Data_types.md).

| Type | Equivalent in a configuration file | Notes | Aliases |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Options: indexed, stored. Default: **both**. To keep text stored, but indexed, specify "stored" only. To keep text indexed only, specify "indexed" only. | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| integer	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| big integer	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | float  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | multi-integer |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | multi-bigint  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | boolean |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | string. Option `indexed, attribute` will make the value full-text indexed and filterable, sortable and groupable at the same time  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | timestamp  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N is the max number of bits to keep  |   |

<!-- intro -->
##### Examples of creating a real-time table via CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

This creates the "products" table with two fields: "title" (full-text) and "price" (float), and sets the "morphology" to "stem_en".

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
This creates the "products" table with three fields:
* "title" is indexed, but not stored.
* "description" is stored, but not indexed.
* "author" is both stored and indexed.
<!-- end -->


## Engine

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

The engine setting changes the default [attribute storage](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) for all attributes in the table. You can also specify `engine` [separately for each attribute](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).

For information on how to enable columnar storage for a plain table, see  [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) .

Values:
* columnar - Enables columnar storage for all table attributes, except for [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (default)** - Doesn't change anything and uses the traditional row-wise storage for the table.


# Other settings
The following settings are applicable for both real-time and plain tables, regardless of whether they are specified in a configuration file or set online using the `CREATE` or `ALTER` command.

## Performance related

### Accessing table files
Manticore supports two access modes for reading table data: seek+read and mmap.

In seek+read mode, the server uses the `pread` system call to read document lists and keyword positions, represented by the`*.spd` and `*.spp`  files. The server uses internal read buffers to optimize the reading process, and the size of these buffers can be adjusted using the options [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) and [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits).There is also the option  [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen) that controls how Manticore opens files at start.

In mmap access mode, the search server maps the table's file into memory using the `mmap` system call, and the OS caches the file contents. The options [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) and [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) have no effect for corresponding files in this mode. The mmap reader can also lock the table's data in memory using the`mlock` privileged call, which prevents the OS from swapping the cached data out to disk.

To control which access mode to use, the options **access_plain_attrs**, **access_blob_attrs**, **access_doclists** and **access_hitlists**  are available, with the following values:

| Value | Description |
| - | - |
| file | server reads the table files from disk with seek+read using internal buffers on file access |
| mmap | server maps the table files into memory and OS caches up its contents on file access |
| mmap_preread | server maps the table files into memory and a background thread reads it once to warm up the cache |
| mlock | server maps the table files into memory and then executes the mlock() system call to cache up the file contents and lock it into memory to prevent it being swapped out |


| Setting | Values | Description |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (default), mlock | controls how `*.spa` (plain attributes) `*.spe` (skip lists) `*.spi` (word lists) `*.spt` (lookups) `*.spm` (killed docs) will be read |
| access_blob_attrs   | mmap, **mmap_preread** (default), mlock  | controls how `*.spb` (blob attributes) (string, mva and json attributes) will be read |
| access_doclists   | **file** (default), mmap, mlock  | controls how `*.spd` (doc lists) data will be read |
| access_hitlists   | **file** (default), mmap, mlock  | controls how `*.spp` (hit lists) data will be read |

Here is a table which can help you select your desired mode:

| table part |	keep it on disk |	keep it in memory |	cached in memory on server start | lock it in memory |
| - | - | - | - | - |
| plain attributes in [row-wise](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (non-columnar) storage, skip lists, word lists, lookups, killed docs | 	mmap | mmap |	**mmap_preread** (default) | mlock |
| row-wise string, multi-value attributes (MVA) and json attributes | mmap | mmap | **mmap_preread** (default) | mlock |
| [columnar](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) numeric, string and multi-value attributes | always  | only by means of OS  | no  | not supported |
| doc lists | **file** (default) | mmap | no	| mlock |
| hit lists | **file** (default) | mmap | no	| mlock |

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

The `inplace_enable` option reduces the disk footprint during indexing of plain tables, while slightly slowing down indexing (it uses approximately 2 times less disk, but yields around 90-95% of the original performance).

Indexing is comprised of two primary phases. During the first phase, documents are collected, processed, and partially sorted by keyword, and the intermediate results are written to temporary files (.tmp*). During the second phase, the documents are fully sorted and the final table files are created. Rebuilding a production table on-the-fly requires approximately 3 times the peak disk footprint: first for the intermediate temporary files, second for the newly constructed copy, and third for the old table that will be serving production queries in the meantime. (Intermediate data is comparable in size to the final table.) This may be too much disk footprint for large data collections, and the `inplace_enable` option can be used to reduce it. When enabled, it reuses the temporary files, outputs the final data back to them, and renames them upon completion. However, this may require additional temporary data chunk relocation, which is where the performance impact comes from.

This directive has no effect on [searchd](../../Starting_the_server/Manually.md), it only affects the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).


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

The option [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) fine-tuning option. Controls preallocated hitlist gap size. Optional, default is 0.

This directive only affects the [searchd](../../Starting_the_server/Manually.md) tool, and does not have any impact on the  [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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

The inplace_reloc_factor setting determines the size of the relocation buffer within the memory arena used during indexing. The default value is 0.1. 

This option is optional and only affects the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) tool, not the [searchd](../../Starting_the_server/Manually.md)  server.

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

Controls the size of the buffer used for in-place writing during indexing. Optional, with a default value of 0.1.

It's important to note that this directive only impacts the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) tool and not the [searchd](../../Starting_the_server/Manually.md) server.


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

### Natural language processing specific settings
The following settings are supported. They are all described in section [NLP and tokenization](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md).
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