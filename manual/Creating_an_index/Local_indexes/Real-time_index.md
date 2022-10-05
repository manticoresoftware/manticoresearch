# Real-time index

<!-- example rt -->
**Real-time index** is a main type of Manticore indexes. It allows adding, updating and deleting documents with immediate availability of the changes. Real-time index settings can be defined in a configuration file or online via `CREATE`/`UPDATE`/`DELETE`/`ALTER` commands.

Real-time index internally consists of one or multiple [plain indexes](../../Creating_an_index/Local_indexes/Plain_index.md) called **chunks**. There can be:

* multiple **disk chunks**. They are stored on disk with the same structure as any plain index
* single **ram chunk**. Stored in memory and used as an accumulator of changes

RAM chunk size is controlled by [rt_mem_limit](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#rt_mem_limit). Once the limit is exceeded the RAM chunk is flushed to disk in a form of a disk chunk. When there are too many disk chunks they can be merged into one for better performance using command [OPTIMIZE](../../Securing_and_compacting_an_index/Compacting_an_index.md#OPTIMIZE-INDEX).

<!-- intro -->
##### Creating a real-time index via SQL protocol:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology='stem_en';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro-->
##### Creating a real-time index via JSON over HTTP:
<!-- request HTTP -->

```http
POST /cli -d "CREATE TABLE products(title text, price float)  morphology='stem_en'"
```

<!-- response HTTP -->

```json
{
"total":0,
"error":"",
"warning":""
}
```

<!-- intro -->
##### Creating a real-time index via PHP client:
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
##### Creating a real-time index via a configuration file:
<!-- request CONFIG -->

```ini
index products {
  type = rt
  path = idx
  rt_field = title
  rt_attr_uint = price
  stored_fields = title
}
```
<!-- end -->

### 👍 What you can do with a real-time index:
* [Add](../../Adding_documents_to_an_index/Adding_documents_to_a_real-time_index.md) documents
* [Update](../../Quick_start_guide.md#Update) attributes and full-text fields
* [Delete](../../Quick_start_guide.md#Delete) documents
* [Truncate](../../Emptying_an_index.md) index
* [Change schema online](../../Updating_table_schema_and_settings.md#Updating-index-schema-in-RT-mode) with help of the command `ALTER`
* [Define index](../../Creating_an_index/Local_indexes/Real-time_index.md) in a configuration file
* Use [UUID](../../Adding_documents_to_an_index/Adding_documents_to_a_real-time_index.md#Auto-ID) for automatic ID provisioning

### ⛔ What you cannot do with a real-time index:
* Index data with help of [indexer](../../Adding_data_from_external_storages/Plain_indexes_creation.md#Indexer-tool)
* Link it with [sources](../../Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md) for easy indexing from external storages
* Update it's [killlist_target](../../Creating_an_index/Local_indexes/Plain_and_real-time_index_settings.md#killlist_target), it's just not needed as the real-time index takes controls of it automatically

#### Real-time index files structure
| Extension | Description |
| - | - |
| `.lock` | lock file |
| `.ram` | RAM chunk |
| `.meta` | RT index headers |
| `.*.sp*` | disk chunks (see [plain index format](../../Creating_an_index/Local_indexes/Plain_index.md#Plain-index-files-structure)) |
