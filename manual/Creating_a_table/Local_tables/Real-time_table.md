# Real-time table

<!-- example rt -->
**Real-time table** is the main type of tables in Manticore. It allows adding, updating and deleting documents with immediate availability of the changes. Real-time table settings can be defined in a configuration file or online via `CREATE`/`UPDATE`/`DELETE`/`ALTER` commands.

Real-time table internally consists of one or multiple [plain tables](../../Creating_a_table/Local_tables/Plain_table.md) called **chunks**. There can be:

* multiple **disk chunks**. They are stored on disk with the same structure as any plain table
* single **ram chunk**. Stored in memory and used as an accumulator of changes

RAM chunk size is controlled by [rt_mem_limit](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Once the limit is exceeded the RAM chunk is flushed to disk in a form of a disk chunk. When there are too many disk chunks they can be merged into one for better performance using command [OPTIMIZE](../../Securing_and_compacting_a_table/Compacting_a_table.md#OPTIMIZE-TABLE).

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

<!-- intro-->
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

### 👍 What you can do with a real-time table:
* [Add](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md) documents
* [Update](../../Quick_start_guide.md#Update) attributes and full-text fields
* [Delete](../../Quick_start_guide.md#Delete) documents
* [Truncate](../../Emptying_a_table.md) table
* [Change schema online](../../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode) with help of the command `ALTER`
* [Define table](../../Creating_a_table/Local_tables/Real-time_table.md) in a configuration file
* Use [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) for automatic ID provisioning

### ⛔ What you cannot do with a real-time table:
* Index data with help of [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)
* Link it with [sources](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md) for easy indexing from external storages
* Update it's [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target), it's just not needed as the real-time table takes controls of it automatically

#### Real-time table files structure
| Extension | Description |
| - | - |
| `.lock` | lock file |
| `.ram` | RAM chunk |
| `.meta` | RT table headers |
| `.*.sp*` | disk chunks (see [plain table format](../../Creating_a_table/Local_tables/Plain_table.md#Plain-table-files-structure)) |
