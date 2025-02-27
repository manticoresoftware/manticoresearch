# Real-time table

A **real-time table** is a main type of table in Manticore. It lets you add, update, and delete documents, and you can see these changes right away. You can set up a real-time Table in a configuration file or use commands like `CREATE`, `UPDATE`, `DELETE`, or `ALTER`.

Internally a real-time table consists of one or more [plain tables](../../Creating_a_table/Local_tables/Plain_table.md) called **chunks**. There are two kinds of chunks:

* multiple **disk chunks** - these are saved on a disk and are structured like a [plain table](../../Creating_a_table/Local_tables/Plain_table.md).
* single **ram chunk** - this is kept in memory and collects all changes.

The size of the RAM chunk is controlled by the [rt_mem_limit](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit) setting. Once this limit is reached, the RAM chunk is transferred to disk as a disk chunk. If there are too many disk chunks, Manticore [combines some of them](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) to improve performance.

### Creating a real-time table:

You can create a new real-time table in two ways: by using the `CREATE TABLE` command or through the [_mapping endpoint](../../Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) of the HTTP JSON API.

#### CREATE TABLE command:

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
##### Typescript:

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE forum(title text, price float)');
```

<!-- intro -->
##### Go:

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE forum(title text, price float)").Execute()
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

#### _mapping API:

> NOTE: The `_mapping` API requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

<!-- example rt-mapping -->

Alternatively, you can create a new table via the `_mapping` endpoint. This endpoint allows you to define an Elasticsearch-like table structure to be converted to a Manticore table.

The body of your request must have the following structure:

```json
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
-    knn_vector => float_vector
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
  "properties": {
    "price": {
        "type": "float"
    },
    "title": {
        "type": "text"
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


#### CREATE TABLE LIKE:

<!-- example create-like -->

You can create a copy of a real-time table, with or without its data. Please note that if the table is large, copying it with data may take some time. Copying works in synchronous mode, but if the connection is dropped, it will continue in the background.

```sql
CREATE TABLE [IF NOT EXISTS] table_name LIKE old_table_name [WITH DATA]
```

> NOTE: Copying a table requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

<!-- intro -->
##### Example:
<!-- request SQL -->

```sql
create table products LIKE old_products;
```

<!-- intro -->
##### Example (WITH DATA):
<!-- request Example (WITH DATA) -->
```sql
create table products LIKE old_products WITH DATA;
```

<!-- end -->

### 👍 What you can do with a real-time table:
* [Add documents](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md).
* Update attributes and full-text fields using the [Update](../../Quick_start_guide.md#Update) process.
* [Delete documents](../../Quick_start_guide.md#Delete).
* [Empty the table](../../Emptying_a_table.md).
* Change the schema online with the `ALTER` command, as explained in [Change schema online](../../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode).
* Define the table in a configuration file, as detailed in [Define table](../../Creating_a_table/Local_tables/Real-time_table.md).
* Use the [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) feature for automatic ID provisioning.

### ⛔ What you cannot do with a real-time table:
* Ingest data using the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) feature.
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

 For more information on the structure of disk chunks, refer to the [plain table files structure](../../Creating_a_table/Local_tables/Plain_table.md#Plain-table-files-structure).
<!-- proofread -->
