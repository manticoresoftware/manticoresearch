<!-- example plain -->
# Plain table

**Plain table** is a basic element for non-[percolate](../../Creating_a_table/Local_tables/Percolate_table.md) searching. It can be specified only in a configuration file in the [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29). It's not supported in the [RT mode](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29). It's normally used together with a [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) to process data [from an external storage](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) and afterwards can be [attached](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_a_plain_table_to_RT_table.md) to a **real-time table**.

### 👍 What you can do with a plain table:
  * build it from an external storage with help of [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) and [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)
  * do an in-place update of an [integer, float, string and MVA attribute](../../Creating_a_table/Data_types.md)
  * [update](../../Quick_start_guide.md#Update) it's killlist_target

### ⛔ What you cannot do with a plain table:
  * insert more data into a table after it's built
  * delete data from it
  * create/delete/alter a plain table online (you need to define it in a configuration file)
  * use [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) for automatic ID generation. When you fetch data from an external storage it must include a unique identifier for each document

Except numeric attributes (including [MVA](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29)), the rest of the data in a plain table is immutable. If you need to update/add new records you need to rebuild the table. While table is being rebuilt, existing table is still available for serving requests. When a new version of the table is ready, a process called [rotation](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md) is performed which puts the new version online and discards the old one.

<!-- intro -->
#### How to create a plain table
Here is an example of a plain table configuration and a source for it which implements fetching data from MySQL:
<!-- request Plain table example -->
A plain table can be only defined in a configuration file. It's not supported by command `CREATE TABLE`

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
Speed of plain indexing depends on several factors:
* how fast the source can be providing the data
* [tokenization settings](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* your hardware (CPU, amount of RAM, disk performance)

#### Plain table building scenarios
##### Rebuild fully when needed
In the simplest usage scenario, we would use a single plain table
which we just fully rebuild from time to time. It works fine for smaller data sets and if you are ready that:
* the table will be not as fresh as data in the source
* indexing duration grows with the data, the more data you have in the source the longer it will take to build the table

##### Main+delta
If you have a bigger data set and still want to use a plain table rather than [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md) what you can do is:
* make another smaller table for incremental indexing
* combine the both using a [distributed table](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)

What it can give is you can rebuild the bigger table seldom (say once per week), save the position of the freshest indexed document and after that use the smaller table to process anything new or updated from your source. Since you will only need to fetch the updates from your storage you can do it much more frequently (say once per minute or even each few seconds).

But after a while the smaller indexing duration will become too high and that will be the moment when you need to rebuild the bigger table and empty the smaller one.

This is called **main+delta schema** and you can learn more about it in [this interactive course](https://play.manticoresearch.com/maindelta/).

When you build a smaller "delta" table it can get documents that are already in the "main" table. To let Manticore know that documents from the current table should take precedence there's a mechanism called **kill list** and corresponding directive [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target).

More information on this topic can be found [here](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md).

#### Plain table files structure
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
