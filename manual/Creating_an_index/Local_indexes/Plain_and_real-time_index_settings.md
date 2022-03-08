# Plain and real-time index settings

<!-- example config -->
## Defining index schema in a configuration file

```ini
index <index_name>[:<parent index name>] {
...
}
```

<!-- intro -->
##### Example of a plain index in a configuration file
<!-- request Plain -->

```ini
index <index_name> {
  type = plain
  path = /path/to/index
  source = <source_name>
  source = <another source_name>
  [stored_fields = <comma separated list of full-text fields that should be stored>]
}
```
<!-- intro -->
##### Example of a real-time index in a configuration file
<!-- request Real-time -->

```ini
index <index name> {
  type = rt
  path = /path/to/index

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

  [stored_fields = <comma separated list of full-text fields that should be stored>]

  [rt_mem_limit = <RAM chunk max size, default 128M>]

}
```
<!-- end -->

### Common plain and real-time indexes settings

#### type

```ini
type = plain

type = rt
```

Index type: "plain" or "rt" (real-time)

Value: **plain** (default), rt

#### path

```ini
path = path/to/index
```

Absolute or relative path without extension where to store the index or where to look for it

Value: path to the index, **mandatory**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

By default when an index is defined in a configuration file, full-text fields' original content is not stored, but just indexed. If this option is set, values from the fields will be both indexed and stored.

Value: comma separated list of **full-text** fields that should be stored. Default is empty (i.e. does not store original field text) for [Plain mode](../../Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain-mode%29), but is enabled for every field for [RT mode](../../Creating_an_index/Local_indexes.md#Online-schema-management-%28RT-mode%29) as long as it's declared as just `text`.

Note, in case of a real-time index the fields listed in `stored_only_fields` should be also declared as [rt_field](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_field).

Note also, that you don't need to list attributes in `stored_fields`, since their original values are stored anyway. `stored_fields` is only for full-text fields.

See also [docstore_block_size](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#docstore_block_size), [docstore_compression](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#docstore_compression) for document storage compression options.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text stored indexed, content text stored indexed, name text indexed, price float)
```

<!-- request HTTP -->

```http
POST /cli -d "
CREATE TABLE products(title text stored indexed, content text stored indexed, name text indexed, price float)"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'columns' => [
            'title'=>['type'=>'text', 'options' => ['indexed', 'stored']],
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
utilsApi.sql('CREATE TABLE products(title text stored indexed, content text stored indexed, name text indexed, price float)')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text stored indexed, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text stored indexed, content text stored indexed, name text indexed, price float)");
```

<!-- request CONFIG -->

```ini
index products {
  stored_fields = title,content

  type = rt
  path = idx
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

A list of fields that will be stored in the index but will be not indexed. Similar to [stored_fields](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#stored_fields) except when a field is specified in `stored_only_fields` it is only stored, not indexed and can’t be searched with fulltext queries. It can only be returned with search results.

Value: comma separated list of fields that should be stored only, not indexed. Default is empty. Note, in case of a real-time index the fields listed in `stored_only_fields` should be also declared as [rt_field](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_field).

Note also, that you don't need to list attributes in `stored_only_fields`, since their original values are stored anyway. If to compare `stored_only_fields` to string attributes the former (stored field):
* is stored on disk and doesn't require memory
* is stored compressed
* can be only fetched, you can't sort/filter/group by the value

The latter (string attribute) is:
* stored on disk and in memory
* stored uncompressed
* can be used for sorting, grouping, filtering and anything else you want to do with attributes.

### Real-time index settings:

#### rt_field

```ini
rt_field = subject
```

Full-text fields to be indexed. The names must be unique. The order is preserved; and so field values in `INSERT` statements without an explicit list of inserted columns will have to be in the same order as configured.

Value: at least one full-text field should be specified in an index, multiple records allowed.

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

Unsigned integer attribute declaration

Value: field_name or field_name:N, can be multiple records. N is the max number of bits to keep.

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

BIGINT attribute declaration

Value: field name, multiple records allowed

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

Multi-valued attribute (MVA) declaration. Declares the UNSIGNED INTEGER (unsigned 32-bit) MVA attribute. Multi-value (ie. there may be more than one such attribute declared), optional.

Value: field name, multiple records allowed.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Multi-valued attribute (MVA) declaration. Declares the BIGINT (signed 64-bit) MVA attribute. Multi-value (ie. there may be more than one such attribute declared), optional.

Value: field name, multiple records allowed.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Floating point attribute declaration. Multi-value (an arbitrary number of attributes is allowed), optional. Declares a single precision, 32-bit IEEE 754 format float attribute.

Value: field name, multiple records allowed.

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Boolean attribute declaration. Multi-value (there might be multiple attributes declared), optional. Declares a 1-bit unsigned integer attribute.

Value: field name, multiple records allowed.

#### rt_attr_string

```ini
rt_attr_string = title
```

String attribute declaration. Multi-value (an arbitrary number of attributes is allowed), optional.

Value: field name, multiple records allowed.

#### rt_attr_json

```ini
rt_attr_json = properties
```

JSON attribute declaration. Multi-value (ie. there may be more than one such attribute declared), optional.

Value: field name, multiple records allowed.

#### rt_attr_timestamp

```ini
rt_attr_timestamp = date_added
```

Timestamp attribute declaration. Multi-value (an arbitrary number of attributes is allowed), optional.

Value: field name, multiple records allowed.

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

RAM chunk size limit. Optional, default is 128M.

RT index keeps some data in memory ("RAM chunk") and also maintains a number of on-disk indexes ("disk chunks"). This directive lets you control the RAM chunk size. Once there’s too much data to keep in RAM, RT index will flush it to disk, activate a newly created disk chunk, and reset the RAM chunk.

The limit is pretty strict: RT index never allocates more memory than it’s limited to. The memory is not preallocated either, hence, specifying 512 MB limit and only inserting 3 MB of data should result in allocating 3 MB, not 512 MB.

The `rt_mem_limit` is never exceeded, but the actual RAM chunk can be significantly lower than the limit. Real-time index learns by your data insertion pace and adapts the actual limit to decrease RAM consumption and increase data write speed. How it works: 
* By default RAM chunk size is 50% of `rt_mem_limit`. It's called "`rt_mem_limit` rate".
* As soon as RAM chunk accumulates `rt_mem_limit * rate` data (50% of `rt_mem_limit` by default) Manticore starts saving the RAM chunk as a new disk chunk.
* While a new disk chunk is being saved, Manticore checks how many new/replaced documents have appeared.
* Upon saving a new disk chunk we update the `rt_mem_limit` rate.
* The rate is reset to 50% as soon as you restart the searchd.

For example, if we saved 90M docs to a disk chunk and 10M more docs arrived while saving, the rate is 90%, so next time we collect up to 90% of `rt_mem_limit` before starting flushing. The higher is the speed of insertion, the lower is the `rt_mem_limit` rate. The rate varies in the range of 33.3% to 95%. You can see index's current rate in [SHOW INDEX <idx> STATUS](Profiling_and_monitoring/Index_settings_and_status/SHOW_INDEX_STATUS.md).

##### How to change rt_mem_limit

In RT mode the RAM chunk size limit can be changed using `ALTER TABLE` . To set `rt_mem_limit` to 1 Gb for index 't' run query `ALTER TABLE t rt_mem_limit='1G'`.

In plain mode `rt_mem_limit` can be changed so:

* edit `rt_mem_limit` value in index configuration
* run `ALTER TABLE <index_name> RECONFIGURE`

##### Important notes about RAM chunks

* RT index is quite similar to a [distributed](../../Creating_an_index/Creating_a_distributed_index/Creating_a_local_distributed_index.md#Creating-a-local-distributed-index) index consisting of multiple local indexes. The local indexes are called "disk chunks".
* RAM chunk internally consists of multiple "segments".
* While disk chunks are stored on disk, the segments of RAM chunk are special RAM-only "indexes".
* Any transaction you make to a real-time index generates a new segment. RAM chunk segments are merged after each transaction commit. Therefore it is beneficial to do bulk INSERTs of hundreds/thousands documents rather than hundreds/thousands different inserts with 1 document to avoid the overhead from merging RAM chunk segments.
* When the number of segments gets greater than 32, the segments get merged, so the count is not greater than 32.
* RT index always has a single RAM-chunk (may be empty) and one or multiple disk chunks.
* Merging larger segments take longer, that's why it may be suboptimal to have very large RAM chunk (and therefore `rt_mem_limit`).
* Number of disk chunks depends on the amount of data in the index and `rt_mem_limit` setting.
* Searchd flushes RAM chunk to disk (not as a disk chunk, just persists) on shutdown and periodically according to [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Flushing several gigabytes to disk may take some time.
* Large RAM chunk will put more pressure on the storage:
  - when flushing the RAM chunk to disk into the `.ram` file
  - when the RAM chunk is full and is dumped to disk as a disk chunk.
* Until flushed RAM chunk is not persisted on disk there's a [binary log](../../Logging/Binary_logging.md) as its backup for the case of a sudden daemon shutdown. In this case the larger you have `rt_mem_limit`, the longer will it take to replay the binlog on start to recover the RAM chunk.
* RAM chunk may be performing slightly slower than a disk chunk.
* Even though a RAM chunk doesn't take more than `rt_mem_limit` Manticore itself can take more in some cases, e.g. if you begin a transaction to insert data and don't commit it for some time, then the data you have already transmitted within the transaction to Manticore is kept in memory.

### Plain index settings:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

Specifies document source to get documents from when the current index is indexed. There must be at least one source. The sources can be of different types (e.g. one - mysql, another - postgresql). Read more about [indexing from external storages here](../../Adding_data_from_external_storages/Plain_indexes_creation.md)

Value: name of the source to build the index from, **mandatory**. Can be multiple records.

#### killlist_target

```ini
killlist_target = main:kl
```

Sets the index(es) that the kill-list will be applied to. Suppresses matches in the targeted index that are updated or deleted in the current index. In `:kl` mode the documents to suppress are taken from the [kill-list](../../Adding_data_from_external_storages/Adding_data_from_indexes/Killlist_in_plain_indexes.md). In `:id` mode all document ids from the current index are suppressed in the targeted one. If neither is specified the both modes take effect. [Read more about kill-lists here](../../Adding_data_from_external_storages/Adding_data_from_indexes/Killlist_in_plain_indexes.md)

Value: **not specified** (default), target_index_name:kl, target_index_name:id, target_index_name. Multiple values are allowed

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Specifies what attributes should be stored in [the columnar storage](../../Creating_an_index/Data_types.md#Row-wise-and-columnar-attribute-storages) instead of the default row-wise storage.

You can do `columnar_attrs = *` to store fields of all supported data types in the columnar storage.

`id` is also supported.

### Creating a real-time index online via CREATE TABLE

<!-- example rt_mode -->
##### General syntax of CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Data types:

Read [more about data types here](../../Creating_an_index/Data_types.md).

| Type | Equivalent in a configuration file | Notes | Aliases |
| - | - | - | - |
| [text](../../Creating_an_index/Data_types.md#Text) | [rt_field](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_field)  | Options: indexed, stored. Default - **both**. To keep text stored, but indexed specify "stored" only. To keep text indexed only specify only "indexed". At least one "text" field should be specified in an index | string |
| [integer](../../Creating_an_index/Data_types.md#Integer) | [rt_attr_uint](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_uint)	| integer	 | int, uint |
| [bigint](../../Creating_an_index/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_bigint)	| big integer	 |   |
| [float](../../Creating_an_index/Data_types.md#Float) | [rt_attr_float](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_float)   | float  |   |
| [multi](../../Creating_an_index/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_multi)   | multi-integer |   |
| [multi64](../../Creating_an_index/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_multi_64) | multi-bigint  |   |
| [bool](../../Creating_an_index/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_bool) | boolean |   |
| [json](../../Creating_an_index/Data_types.md#JSON) | [rt_attr_json](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_an_index/Data_types.md#String) | [rt_attr_string](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_string) | string. Option: indexed - also index the strings in a full-text field with same name.   |   |
| [timestamp](../../Creating_an_index/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_timestamp) | timestamp  |   |
| [bit(n)](../../Creating_an_index/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_attr_uint) | N is the max number of bits to keep  |   |

<!-- intro -->
##### Examples of creating a real-time index via CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

creates table "products" with two fields: "title" (full-text) and "price" (float) and setting "morphology" with value "stem_en"

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
creates table "products" with three fields:
* field "title" - indexed, but not stored
* field "description" - stored, but not indexed
* field "author" - both stored and indexed
<!-- end -->


## Engine

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

Changes default [attribute storage](../../Creating_an_index/Data_types.md#Row-wise-and-columnar-attribute-storages) for all attributes in the index. Can be overridden by specifying `engine` [separately for each attribute](../../Creating_an_index/Data_types.md#How-to-switch-between-the-storages).

See [columnar_attrs](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#columnar_attrs) on how to enable columnar storage for a plain index.

Values:
* columnar - enables columnar storage for all index attributes except for [json](../../Creating_an_index/Data_types.md#JSON)
* **rowwise (default)** - doesn't change anything, i.e. makes Manticore use the traditional row-wise storage for the index


# Other settings
The following settings are similar for both real-time and plain index in either mode: whether specified in a configuration file or online via `CREATE` or `ALTER` command.

## Performance related

### Accessing index files
Manticore uses two access modes to read index data - seek+read and mmap.

In seek+read mode the server performs system call `pread` to read document lists and keyword positions, i.e. `*.spd` and `*.spp` files. Internal read buffers are used to optimize reading. The size of these buffers can be tuned with options [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) and [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). There is also option [preopen](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#preopen) that allows to control how Manticore opens files at start.

In the mmap access mode the search server just maps index's file into memory with `mmap` system call and OS caches file contents by itself. Options [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) and [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) have no effect for corresponding files in this mode. The mmap reader can also lock index's data in memory via `mlock` privileged call which prevents swapping out the cached data to disk by OS.

To control what access mode will be used **access_plain_attrs**, **access_blob_attrs**, **access_doclists** and **access_hitlists** options are available with the following values:

| Value | Description |
| - | - |
| file | server reads index file from disk with seek+read using internal buffers on file access |
| mmap | server maps index file into memory and OS caches up its contents on file access |
| mmap_preread | server maps index file into memory and a background thread reads it once to warm up the cache |
| mlock | server maps index file into memory and then issues mlock system call to cache up the file contents and lock it into memory to prevent it being swapped out |


| Setting | Values | Description |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (default), mlock | controls how `*.spa` (plain attributes) `*.spe` (skip lists) `*.spi` (word lists) `*.spt` (lookups) `*.spm` (killed docs) will be read |
| access_blob_attrs   | mmap, **mmap_preread** (default), mlock  | controls how `*.spb` (blob attributes) (string, mva and json attributes) will be read |
| access_doclists   | **file** (default), mmap, mlock  | controls how `*.spd` (doc lists) data will be read |
| access_hitlists   | **file** (default), mmap, mlock  | controls how `*.spp` (hit lists) data will be read |

Here is a table which can help you select your desired mode:

| index part |	keep it on disk |	keep it in memory |	cached in memory on server start | lock it in memory |
| - | - | - | - | - |
| plain attributes in [row-wise](../../Creating_an_index/Data_types.md#Row-wise-and-columnar-attribute-storages) (non-columnar) storage, skip lists, word lists, lookups, killed docs | 	mmap | mmap |	**mmap_preread** (default) | mlock |
| row-wise string, multi-value attributes (MVA) and json attributes | mmap | mmap | **mmap_preread** (default) | mlock |
| [columnar](../../Creating_an_index/Data_types.md#Row-wise-and-columnar-attribute-storages) numeric, string and multi-value attributes | always  | only by means of OS  | no  | not supported |
| doc lists | **file** (default) | mmap | no	| mlock |
| hit lists | **file** (default) | mmap | no	| mlock |

##### The recommendations are:

* If you want **the best search response time** and have enough memory - use [row-wise](../../Creating_an_index/Data_types.md#Row-wise-and-columnar-attribute-storages) attributes and `mlock` for attributes and for doclists/hitlists
* If you **can't afford lower performance on start** and are ready to wait longer on start until it's warmed up - use [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). If you want searchd to be able to restart faster - stay with `mmap_preread`
* If you want to **save RAM**, but still have enough RAM for all the attributes - do not use `mlock`, then your OS will decide what should be in memory at any given moment of time depending on what is read from disk more frequently
* If row-wise **attributes don't fit into RAM** - use [columnar attributes](../../Creating_an_index/Data_types.md#Row-wise-and-columnar-attribute-storages)
* If full-text search **performance is not a priority** and you want to save RAM - use `access_doclists/access_hitlists=file`

The default mode is to:
* mmap
* preread non-columnar attributes
* seek+read columnar attributes with no preread
* seek+read doclists/hitlists with no preread

which provides decent search performance, optimal memory usage and faster searchd restart in most cases.

### Other performance related settings

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Sets the space to be reserved for blob attribute updates. Optional, default value is 128k. When blob attributes (multi-value attributes (MVA), strings, JSON) are updated, their length may change. If the updated string (or MVA or JSON) is shorter than the old one, it overwrites the old one in the `*.spb` file. But if the updated string is longer, the updates are written to the end of the `*.spb` file. This file is memory mapped, that's why resizing it may be a rather slow process, depending on the OS implementation of memory mapped files. To avoid frequent resizes, you can specify the extra space to be reserved at the end of the .spb file by using this setting.

Value: size, default **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Size of the block of documents used by document storage. Optional, default is 16kb. When stored_fields or stored_only_fields are specified, original document text is stored inside the index. To use less disk space, documents are compressed. To get more efficient disk access and better compression ratios on small documents, documents are concatenated into blocks. When indexing, documents are collected until their total size reaches the threshold. After that, this block of documents is compressed. This option can be used to get better compression ratio (by increasing block size) or to get faster access to document text (by decreasing block size).

Value: size, default **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Type of compression used to compress blocks of documents used by document storage. When stored_fields or stored_only_fields are specified, document storage stores compressed document blocks. 'lz4' has fast compression and decompression speeds, 'lz4hc' (high compression) has the same fast decompression but compression speed is traded for better compression ratio. 'none' disables compression.

Value: **lz4** (default),  lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Compression level in document storage when 'lz4hc' compression is used. When 'lz4hc' compression is used, compression level can be fine-tuned to get better performance or better compression ratio. Does not work with 'lz4' compression.

Value: 1-12 (default **9**).

#### preopen

```ini
preopen = 1
```

This option tells searchd that it should pre-open all index files on startup (or rotation) and keep them open while it runs. Currently, the default mode is not to pre-open the files. Pre-opened indexes take a few (currently 2) file descriptors per index. However, they save on per-query open() calls; and also they are invulnerable to subtle race conditions that may happen during index rotation under high load. On the other hand, when serving many indexes (100s to 1000s), it still might be desired to open them on per-query basis in order to save file descriptors

Value: **0** (default), 1.

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

Per-keyword read buffer size for document lists. The higher the value the higher per-query RAM use is, but possibly lower IO time

Value: size, default **256k**, min 8k.

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

Per-keyword read buffer size for hit lists. The higher the value the higher per-query RAM use is, but possibly lower IO time

Value: size, default **256k**, min 8k.

### Plain index disk footprint settings

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

Whether to enable in-place index inversion. Optional, default is 0 (use separate temporary files).

`inplace_enable` greatly reduces indexing disk footprint for a plain index, at a cost of slightly slower indexing (it uses around 2x less disk, but yields around 90-95% the original performance).

Indexing involves two major phases. The first phase collects, processes, and partially sorts documents by keyword, and writes the intermediate result to temporary files (.tmp\*). The second phase fully sorts the documents, and creates the final index files. Thus, rebuilding a production index on the fly involves around 3x peak disk footprint: 1st copy for the intermediate temporary files, 2nd copy for newly constructed copy, and 3rd copy for the old index that will be serving production queries in the meantime. (Intermediate data is comparable in size to the final index.) That might be too much disk footprint for big data collections, and `inplace_enable` allows to reduce it. When enabled, it reuses the temporary files, outputs the final data back to them, and renames them on completion. However, this might require additional temporary data chunk relocation, which is where the performance impact comes from.

This directive does not affect [searchd](../../Starting_the_server/Manually.md) in any way, it only affects [indexer](../../Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool).


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
index products {
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

[In-place inversion](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#inplace_enable) fine-tuning option. Controls preallocated hitlist gap size. Optional, default is 0.

This directive does not affect [searchd](../../Starting_the_server/Manually.md) in any way, it only affects [indexer](../../Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool).


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
index products {
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

Controls relocation buffer size within indexing memory arena. Optional, default is 0.1.

This directive does not affect [searchd](../../Starting_the_server/Manually.md) in any way, it only affects [indexer](../../Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool).


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
index products {
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

Controls in-place write buffer size within indexing memory arena. Optional, default is 0.1.

This directive does not affect [searchd](../../Starting_the_server/Manually.md) in any way, it only affects [indexer](../../Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool).


<!-- intro -->
##### CONFIG:

<!-- request CONFIG -->

```ini
index products {
  inplace_write_factor = 0.1
  inplace_enable = 1

  path = products
  source = src_base
}
```
<!-- end -->

### Natural language processing specific settings
The following settings are supported. They are all described in section [NLP and tokenization](../../Creating_an_index/NLP_and_tokenization/Data_tokenization.md).
* [bigram_freq_words](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#bigram_freq_words)
* [bigram_index](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#bigram_index)
* [blend_chars](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#blend_chars)
* [blend_mode](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#blend_mode)
* [charset_table](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#charset_table)
* [dict](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#dict)
* [embedded_limit](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#embedded_limit)
* [exceptions](../../Creating_an_index/NLP_and_tokenization/Exceptions.md)
* [expand_keywords](../../Searching/Options.md#expand_keywords)
* [global_idf](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#global_idf)
* [hitless_words](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#hitless_words)
* [html_index_attrs](../../Creating_an_index/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_index_attrs)
* [html_remove_elements](../../Creating_an_index/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_remove_elements)
* [html_strip](../../Creating_an_index/NLP_and_tokenization/Advanced_HTML_tokenization.md#html_strip)
* [ignore_chars](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ignore_chars)
* [index_exact_words](../../Creating_an_index/NLP_and_tokenization/Morphology.md#index_exact_words)
* [index_field_lengths](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#index_field_lengths)
* [index_sp](../../Creating_an_index/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_sp)
* [index_token_filter](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#index_token_filter)
* [index_zones](../../Creating_an_index/NLP_and_tokenization/Advanced_HTML_tokenization.md#index_zones)
* [infix_fields](../../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#infix_fields)
* [killlist_target](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#killlist_target)
* [max_substring_len](../../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#max_substring_len)
* [min_infix_len](../../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#min_infix_len)
* [min_prefix_len](../../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#min_prefix_len)
* [min_stemming_len](../../Creating_an_index/NLP_and_tokenization/Morphology.md#min_stemming_len)
* [min_word_len](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#min_word_len)
* [morphology](../../Creating_an_index/NLP_and_tokenization/Morphology.md#morphology)
* [morphology_skip_fields](../../Creating_an_index/NLP_and_tokenization/Morphology.md#morphology_skip_fields)
* [ngram_chars](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_chars)
* [ngram_len](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#ngram_len)
* [overshort_step](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#overshort_step)
* [phrase_boundary](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary)
* [phrase_boundary_step](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#phrase_boundary_step)
* [prefix_fields](../../Creating_an_index/NLP_and_tokenization/Wildcard_searching_settings.md#prefix_fields)
* [regexp_filter](../../Creating_an_index/NLP_and_tokenization/Low-level_tokenization.md#regexp_filter)
* [stopwords](../../Creating_an_index/NLP_and_tokenization/Ignoring_stop-words.md#stopwords)
* [stopword_step](../../Creating_an_index/NLP_and_tokenization/Ignoring_stop-words.md#stopword_step)
* [stopwords_unstemmed](../../Creating_an_index/NLP_and_tokenization/Ignoring_stop-words.md#stopwords_unstemmed)
* [stored_fields](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#stored_fields)
* [stored_only_fields](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md)
* [wordforms](../../Creating_an_index/NLP_and_tokenization/Wordforms.md#wordforms)
