# Real-time table

A **Real-time table** is a main type of table in Manticore, allowing you to add, update, and delete documents with immediate availability of the changes. The settings for a Real-time Table can be defined in a configuration file or online using  `CREATE`/`UPDATE`/`DELETE`/`ALTER` commands.

A Real-time Table is comprised of one or multiple  [plain tables](../../Creating_a_table/Local_tables/Plain_table.md) called **chunks**. There are two types of chunks:

* multiple **disk chunks** -  These are stored on disk and have the same structure as a Plain Table.
* single **ram chunk** - This is stored in memory and is used as an accumulator of changes.

The size of the RAM chunk is controlled by the [rt_mem_limit](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) setting. Once this limit is reached, the RAM chunk is flushed to disk in the form of a disk chunk. If there are too many disk chunks, they can be merged into one for better performance using the  [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE) command or automatically.


### 👍 Creating a real-time table:

There are two ways you can create a new real-time table: with the `CREATE TABLE` command  or by using the `_mapping` endpoint of HTTP JSON API.

#### 👍 CREATE TABLE command:

<!-- example rt -->

You can use this command via both SQL and HTTP protocols:

<!-- intro -->
##### Creating a real-time table via SQL protocol:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology='stem_en';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### Creating a real-time table via JSON over HTTP:
<!-- request JSON -->

```JSON
POST /cli -d "CREATE TABLE products(title text, price float)  morphology='stem_en'"
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### Creating a real-time table via PHP client:
<!-- request PHP -->

```php
$index = new \Manticoresearch\Index($client);
$index->setName('products');
$index->create([
    'title'=>['type'=>'text'],
    'price'=>['type'=>'float'],
]);
```

<!-- intro -->
##### Python:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE forum(title text, price float)')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE forum(title text, price float)');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE forum(title text, price float)");
```

<!-- intro -->
##### Creating a real-time table via a configuration file:
<!-- request CONFIG -->

```ini
table products {
  type = rt
  path = tbl
  rt_field = title
  rt_attr_uint = price
  stored_fields = title
}
```
<!-- end -->

#### 👍 _mapping endpoint:

<!-- example rt-mapping -->

Alternatively, you can create a new table via the `_mapping` endpoint. This endpoint allows you to define an Elasticsearch-like table structure to be converted to a Manticore table.

The body of your request must have the following structure:

```
"properties"
{
  "FIELD_NAME_1":
  {
    "type": "FIELD_TYPE_1"
  },
  "FIELD_NAME_2":
  {
    "type": "FIELD_TYPE_2"
  },
  
  ...
  
  "FIELD_NAME_N":
  {
    "type": "FIELD_TYPE_M"
  }
}
```

When creating a table, Elasticsearch data types will be mapped to Manticore types according to the following rules:
-    aggregate_metric => json
-    binary => string
-    boolean => bool
-    byte => int
-    completion => string
-    date => timestamp
-    date_nanos => bigint
-    date_range => json
-    dense_vector => json
-    flattened => json
-    flat_object => json
-    float => float
-    float_range => json
-    geo_point => json
-    geo_shape => json
-    half_float => float
-    histogram => json
-    integer => int
-    integer_range => json
-    ip => string
-    ip_range => json
-    keyword => string
-    knn_vector => float_vector,
-    long => bigint
-    long_range => json
-    match_only_text => text
-    object => json
-    point => json
-    scaled_float => float
-    search_as_you_type => text
-    shape => json
-    short => int
-    text => text
-    unsigned_long => int
-    version => string

<!-- intro -->
##### Creating a real-time table via the _mapping endpoint:
<!-- request JSON -->

```JSON
POST /your_table_name/_mapping -d '
{
  "test": {
    "mappings": {
      "properties": {
        "price": {
            "type": "float"
        },
        "title": {
            "type": "text"
        }
      }
    }
  }
}
'
```

<!-- response JSON -->

```json
{
"total":0,
"error":"",
"warning":""
}
```
<!-- end -->

### 👍 What you can do with a real-time table:
* Add documents using the [Add](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) feature.
* Update attributes and full-text fields through the [Update](../../Quick_start_guide.md#Update) process.
* Delete documents using the [Delete](../../Quick_start_guide.md#Delete) feature.
* Emptying the table using the [Truncate](../../Emptying_a_table.md) process.
* Change the schema online using the `ALTER` command as described in [Change schema online](../../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode).
* Define the table in a configuration file as described in [Define table](../../Creating_a_table/Local_tables/Real-time_table.md).
* Use the [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) feature for automatic ID provisioning.

### ⛔ What you cannot do with a real-time table:
* Index data using the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) feature.
* Connect it to [sources](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md) for easy indexing from external storage.
* Update the [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target), as it is automatically managed by the real-time table.

#### Real-time table files structure

The following table outlines the different file extensions and their respective descriptions in a real-time table:

| Extension | Description |
| - | - |
| `.lock` | A lock file that ensures that only one process can access the table at a time. |
| `.ram` | The RAM chunk of the table, stored in memory and used as an accumulator of changes. |
| `.meta` | The headers of the real-time table that define its structure and settings. |
| `.*.sp*` | Disk chunks that are stored on disk with the same format as plain tables. They are created when the RAM chunk size exceeds the  rt_mem_limit.|

 For more information on the structure of disk chunks, refer to the [plain table format](../../Creating_a_table/Local_tables/Plain_table.md#Plain-table-files-structure))
<!-- proofread -->
