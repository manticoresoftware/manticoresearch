# Таблица реального времени

**Таблица реального времени** — это основной тип таблицы в Manticore. Она позволяет добавлять, обновлять и удалять документы, и вы сразу же видите эти изменения. Вы можете настроить таблицу реального времени в конфигурационном файле или использовать команды `CREATE`, `UPDATE`, `DELETE` или `ALTER`.

Внутри таблица реального времени состоит из одной или нескольких [простых таблиц](../../Creating_a_table/Local_tables/Plain_table.md), называемых **чанками**. Есть два типа чанков:

* несколько **чанков на диске** — сохраняются на диске и структурированы как [простая таблица](../../Creating_a_table/Local_tables/Plain_table.md).
* один **чанк в оперативной памяти** — хранится в памяти и собирает все изменения.

Размер чанка в оперативной памяти контролируется настройкой [rt_mem_limit](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_mem_limit). Когда этот лимит достигнут, чанк в памяти переносится на диск как дискчанк. Если дискчанков становится слишком много, Manticore [объединяет некоторые из них](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks) для улучшения производительности.

### Создание таблицы реального времени:

Вы можете создать новую таблицу реального времени двумя способами: с помощью команды `CREATE TABLE` или через [_mapping endpoint](../../Creating_a_table/Local_tables/Real-time_table.md#_mapping-API:) HTTP JSON API.

#### Команда CREATE TABLE:

<!-- example rt -->

Вы можете использовать эту команду как через SQL, так и через HTTP протоколы:

<!-- intro -->
##### Создание таблицы реального времени через SQL протокол:
<!-- request SQL -->

```sql
CREATE TABLE products(title text, price float) morphology='stem_en';
```
<!-- response SQL -->

```sql
Query OK, 0 rows affected (0.00 sec)
```

<!-- intro -->
##### Создание таблицы реального времени через JSON по HTTP:
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
##### Создание таблицы реального времени через PHP клиент:
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
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE forum(title text, price float)')
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
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE forum(title text, price float)", Some(true)).await;
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
##### Создание таблицы реального времени через конфигурационный файл:
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

> ПРИМЕЧАНИЕ: API `_mapping` требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

<!-- example rt-mapping -->

Альтернативно, вы можете создать новую таблицу через endpoint `_mapping`. Этот endpoint позволяет определить структуру таблицы, похожую на Elasticsearch, для последующего преобразования в таблицу Manticore.

Тело вашего запроса должно иметь следующую структуру:

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

При создании таблицы типы данных Elasticsearch будут преобразованы в типы Manticore по следующим правилам:
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
##### Создание таблицы реального времени через endpoint _mapping:
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

Вы можете создать копию таблицы реального времени, с данными или без. Учтите, что если таблица большая, копирование с данными может занять некоторое время. Копирование работает в синхронном режиме, но если соединение прервано, оно продолжится в фоне.

```sql
CREATE TABLE [IF NOT EXISTS] table_name LIKE old_table_name [WITH DATA]
```

> ПРИМЕЧАНИЕ: Копирование таблицы требует [Manticore Buddy](Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

<!-- intro -->
##### Пример:
<!-- request SQL -->

```sql
create table products LIKE old_products;
```

<!-- intro -->
##### Пример (С ДАННЫМИ):
<!-- request Example (WITH DATA) -->
```sql
create table products LIKE old_products WITH DATA;
```

<!-- end -->

### 👍 Что вы можете делать с таблицей реального времени:
* [Добавлять документы](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md).
* Обновлять атрибуты и полнотекстовые поля с помощью процесса [Update](../../Quick_start_guide.md#Update).
* [Удалять документы](../../Quick_start_guide.md#Delete).
* [Опустошать таблицу](../../Emptying_a_table.md).
* Изменять схему онлайн с помощью команды `ALTER`, как описано в [Обновление схемы онлайн](../../Updating_table_schema_and_settings.md#Updating-table-schema-in-RT-mode).
* Определять таблицу в конфигурационном файле, как подробно описано в [Определение таблицы](../../Creating_a_table/Local_tables/Real-time_table.md).
* Использовать функцию [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) для автоматического назначения ID.

### ⛔ Чего вы не можете делать с таблицей реального времени:
* Загружать данные с помощью функции [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).
* Подключать её к [источникам](../../Data_creation_and_modification/Adding_data_from_external_storages/Fetching_from_databases/Execution_of_fetch_queries.md) для упрощённого индексирования из внешних хранилищ.
* Обновите [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target), так как он автоматически управляется таблицей реального времени.

#### Структура файлов таблицы реального времени

Ниже приведена таблица, в которой описаны различные расширения файлов и их соответствующие описания в таблице реального времени:

| Extension | Description |
| - | - |
| `.lock` | Файл блокировки, который обеспечивает доступ к таблице только одному процессу в одно время. |
| `.ram` | RAM-чанк таблицы, хранящийся в памяти и используемый как аккумулятор изменений. |
| `.meta` | Заголовки таблицы реального времени, определяющие её структуру и настройки. |
| `.*.sp*` | Дисковые чанки, которые хранятся на диске в том же формате, что и обычные таблицы. Они создаются, когда размер RAM чанка превышает rt_mem_limit.|

 Для получения дополнительной информации о структуре дисковых чанков см. [структуру файлов обычной таблицы](../../Creating_a_table/Local_tables/Plain_table.md#Plain-table-files-structure).
<!-- proofread -->

