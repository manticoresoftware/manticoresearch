<!-- example plain -->
# Plain index

**Plain index** is a basic element for non-[percolate](Creating_an_index/Local_indexes/Percolate_index.md) searching. It can be specified only in a configuration file in [Plain mode](Creating_an_index/Local_indexes.md#Defining-index-schema-in-config-%28Plain mode%29). It is not supported in [RT mode](Creating_an_index/Local_indexes.md#Online-schema-management-%28RT-mode%29). It's normally used together with a [source](Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#source) to index data [from an external storage](Adding_data_from_external_storages/Plain_indexes_creation.md) and afterwards can be [attached](Adding_data_from_external_storages/Adding_data_from_indexes/Attaching_a_plain_index_to_RT_index.md) to a **real-time index**.

### 👍 What you can do with a plain index:
  * build it with help of [source](Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#source) and [indexer](Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool) tool which is the fastest possible way to index data
  * do an in-place update of an [integer, float, string and MVA attribute](Creating_an_index/Data_types.md)
  * [update](Quick_start_guide.md#Update) it's killlist_target

### ⛔ What you cannot do with a plain index:
  * insert more data into an index after it's built
  * update it
  * delete from it
  * create/delete/alter a plain index online (you need to define it in a configuration file)
  * use [UUID](Adding_documents_to_an_index/Adding_documents_to_a_real-time_index.md#Auto-ID) for automatic ID generation. When you fetch data from an external storage it must include a unique identifier for each document

Except numeric attributes (including [MVA](Creating_an_index/Data_types.md#Multi-value-integer-%28MVA%29)), the rest of the data in a plain index is immutable. If you need to update/add new records you need to rebuild the index. While index is being rebuilt, existing index is still available for serving requests. When a new version of the index is ready, a process called [rotation](Adding_data_from_external_storages/Rotating_an_index.md) is performed which puts the new version online and discards the old one.

<!-- intro -->
#### How to create a plain index
Here is an example of a plain index configuration and a source for it which implements fetching data from MySQL:
<!-- request Plain index example -->
A plain index can be only defined in a configuration file. It's not supported by command `CREATE TABLE`

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

index idx {
  type   = plain
  source = source
  path   = /path/to/index
 }
```
<!-- end -->

#### Plain indexing performance
Speed of plain indexing depends on several factors:
* how fast the source can be providing the data
* [tokenization settings](Creating_an_index/NLP_and_tokenization/Data_tokenization.md)
* your hardware (CPU, amount of RAM, disk performance)

#### Plain indexing scenarios
##### Rebuild fully when needed
In the simplest usage scenario, we would use a single plain index
which we just fully rebuild from time to time. It works fine for smaller data sets and if you are ready that:
* the index will be not as fresh as data in the source
* indexing duration grows with the data, the more data you have in the source the longer it will take to build the index

##### Main+delta
If you have a bigger data set and still want to use a plain index rather than [Real-Time](Creating_an_index/Local_indexes/Real-time_index.md) what you can do is:
* make another smaller index for incremental indexing
* combine the both using a [distributed index](Creating_an_index/Creating_a_distributed_index/Creating_a_local_distributed_index.md)

What it can give is you can rebuild the bigger index seldom (say once per week), save the position of the freshest indexed document and after that use the smaller index to index anything new or updated from your source. Since you will only need to fetch the updates from your storage you can do it much more frequently (say once per minute or even each few seconds).

But after a while the smaller indexing duration will become too high and that will be the moment when you need to rebuild the bigger index and empty the smaller one.

This is called **main+delta schema** and you can learn more about it in [this interactive course](https://play.manticoresearch.com/maindelta/).

When you build a smaller "delta" index it can get documents that are already in the "main" index. To let Manticore know that documents from the current index should take precedence there's a mechanism called **kill list** and corresponding directive [killlist_target](Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#killlist_target).

More information on this topic can be found [here](Adding_data_from_external_storages/Main_delta.md).

#### Plain index files structure
| Extension | Description |
| - | - |
|`.spa` | stores document attributes |
|`.spb` | stores blob attributes: strings, MVA, json |
|`.spd` | stores matching document ID lists for each word ID |
|`.sph` | stores index header information |
|`.sphi` | stores histograms of attribute values |
|`.spi` | stores word lists (word IDs and pointers to `.spd` file) |
|`.spk` | stores kill-lists |
|`.spl` | lock file |
|`.spm` | stores a bitmap of killed documents |
|`.spp` | stores hit (aka posting, aka word occurrence) lists for each word ID |
|`.spt` | stores additional data structures to speed up lookups by document ids |
|`.spe` | stores skip-lists to speed up doc-list filtering |
|`.spds` | stores document texts |
|`.tmp*` |temporary files during index_settings_and_status |
|`.new.sp*` | during indexing new version of the index is written by default in the same folder |
|`.old.sp*` | after rotation previous version files are saved with .old extension |
