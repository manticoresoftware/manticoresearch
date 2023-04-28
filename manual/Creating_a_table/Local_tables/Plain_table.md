<!-- example plain -->
# Plain table

**Plain table** is a basic element for non-[percolate](../../Creating_a_table/Local_tables/Percolate_table.md) searching. It can be defined only in a configuration file using the [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29), and is not supported in the [RT mode](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29). It is typically used in conjunction with a [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) to process data [from the external storage](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) and can later be [attached](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_a_plain_table_to_RT_table.md) to a **real-time table**.

### 👍 What you can do with a plain table:
  * Build it from external storage using a [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) and [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)
  * Perform an in-place update of [integer, float, string and MVA attribute](../../Creating_a_table/Data_types.md)
  * [update](../../Quick_start_guide.md#Update) it's killlist_target

### ⛔ What you cannot do with a plain table:
  * Insert additional data into the table once it has been built
  * Delete data from the table
  * Create, delete, or alter the table online
  * Use [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) for automatic ID generation (data from external storage must include a unique identifier)

Numeric attributes, including [MVAs](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29), are the only elements that can be updated in a plain table. All other data in the table is immutable. If updates or new records are required, the table must be rebuilt. During the rebuilding process, the existing table remains available to serve requests, and a process called  [rotation](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md) is performed when the new version is ready, bringing it online and discarding the old version.

<!-- intro -->
#### How to create a plain table

<!-- request Plain table example -->
To create a plain table, you'll need to define it in a configuration file. It's not supported by the  `CREATE TABLE` command.

Here's an example of a plain table configuration and a source for fetching data from a MySQL database:

```ini
source source {
  type             = mysql
  sql_host         = localhost
  sql_user         = myuser
  sql_pass         = mypass
  sql_db           = mydb
  sql_query        = SELECT id, title, description, category_id  from mytable
  sql_attr_uint    = category_id
  sql_field_string = title
 }

table tbl {
  type   = plain
  source = source
  path   = /path/to/table
 }
```
<!-- end -->

#### Plain table building performance
The speed at which a plain table is indexed depends on several factors, including:
* Data source retrieval speed
* [tokenization settings](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* The hardware specifications (such as CPU, RAM, and disk performance)

#### Plain table building scenarios
##### Rebuild fully when needed
For small data sets, the simplest option is to have a single plain table that is fully rebuilt as needed. This approach is acceptable when:
* The data in the table is not as fresh as the data in the source
* The time it takes to build the table increases as the data set grows

##### Main+delta scenario 
For larger data sets, a plain table can be used instead of a [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md). The main+delta scenario involves:
* Creating a smaller table for incremental indexing
* Combining the two tables using a [distributed table](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)

This approach allows for infrequent rebuilding of the larger table and more frequent processing of updates from the source. The smaller table can be rebuilt more often (e.g. every minute or even every few seconds).

However, as time goes on, the indexing duration for the smaller table will become too long, requiring a rebuild of the larger table and the emptying of the smaller one.

The main+delta schema is explained in detail in [this interactive course](https://play.manticoresearch.com/maindelta/).

The mechanism of **kill list** and [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) directive is used to ensure that documents from the current table take precedence over those from the other table.

For more information on this topic, see [here](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md).

#### Plain table files structure
The following table outlines the various file extensions used in a plain table and their respective descriptions:

| Extension | Description |
| - | - |
|`.spa` | stores document attributes in [row-wise mode](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) |
|`.spb` | stores blob attributes in [row-wise mode](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages): strings, MVA, json |
|`.spc` | stores document attributes in [columnar mode](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)  |
|`.spd` | stores matching document ID lists for each word ID |
|`.sph` | stores table header information |
|`.sphi` | stores histograms of attribute values |
|`.spi` | stores word lists (word IDs and pointers to `.spd` file) |
|`.spidx` | stores secondary indexes data |
|`.spk` | stores kill-lists |
|`.spl` | lock file |
|`.spm` | stores a bitmap of killed documents |
|`.spp` | stores hit (aka posting, aka word occurrence) lists for each word ID |
|`.spt` | stores additional data structures to speed up lookups by document ids |
|`.spe` | stores skip-lists to speed up doc-list filtering |
|`.spds` | stores document texts |
|`.tmp*` |temporary files during index_settings_and_status |
|`.new.sp*` | new version of a plain table before rotation |
|`.old.sp*` | old version of a plain table after rotation |

<!-- proofread -->
