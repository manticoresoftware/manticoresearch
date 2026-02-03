# Настройки обычных и реального времени таблиц

<!-- example config -->
## Определение схемы таблицы в файле конфигурации

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### Пример обычной таблицы в файле конфигурации
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
##### Пример таблицы реального времени в файле конфигурации
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
  [rt_attr_float_vector = <float vector field name>]
  [rt_attr_float_vector = <another float vector field name>]
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

### Общие настройки для обычных таблиц и таблиц реального времени

#### type

```ini
type = plain

type = rt
```

Тип таблицы: "plain" (обычная) или "rt" (реального времени)

Значение: **plain** (по умолчанию), rt

#### path

```ini
path = path/to/table
```

Путь к месту хранения или расположения таблицы, абсолютный или относительный, без расширения.

Значение: Путь к таблице, **обязательный параметр**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

По умолчанию, при определении таблицы в файле конфигурации, исходное содержимое полнотекстовых полей индексируется и сохраняется. Данная настройка позволяет указать поля, для которых следует хранить исходные значения.

Значение: Список **полнотекстовых** полей, разделённых запятыми, для которых следует хранить исходные значения. Пустое значение (т.е. `stored_fields =` ) отключает хранение исходных значений для всех полей.

Примечание: В случае таблицы реального времени, поля, перечисленные в `stored_fields`, также должны быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).
Также обратите внимание, что вам не нужно перечислять атрибуты в `stored_fields`, поскольку их исходные значения и так сохраняются. `stored_fields` может использоваться только для полнотекстовых полей.

Примечание: В конфигурации распределённой таблицы идентификаторы документов должны быть уникальны во всех таблицах агентов. Если несколько агентов содержат одинаковый идентификатор документа, получение сохранённых полей может вернуть значения от другого агента, отличного от совпавшей строки.

См. также [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) для опций сжатия хранилища документов.


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
    'table' => 'products'
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
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
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

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)", Some(true)).await;
```

<!-- intro -->
##### Typescript:

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- intro -->
##### Go:

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)").Execute()
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

Используйте `stored_only_fields`, когда вы хотите, чтобы Manticore хранил некоторые поля обычной таблицы или таблицы реального времени **только на диске, но не индексировал их**. Эти поля не будут доступны для поиска с помощью полнотекстовых запросов, но вы всё равно сможете получать их значения в результатах поиска.

Например, это полезно, если вы хотите хранить данные, такие как JSON-документы, которые должны возвращаться с каждым результатом, но не нуждаются в поиске, фильтрации или группировке. В этом случае хранение их только на диске — без индексации — экономит память и повышает производительность.

Вы можете сделать это двумя способами:
- В [режиме plain](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) в конфигурации таблицы используйте настройку `stored_only_fields`.
- В SQL-интерфейсе ([режим RT](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) используйте свойство [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) при определении текстового поля (вместо `indexed` или `indexed stored`). В SQL вам не нужно включать `stored_only_fields` — эта опция не поддерживается в операторах `CREATE TABLE`.

Значение `stored_only_fields` — это список имён полей, разделённых запятыми. По умолчанию список пуст. Если вы используете таблицу реального времени, каждое поле, перечисленное в `stored_only_fields`, также должно быть объявлено как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Примечание: вам не нужно перечислять атрибуты в `stored_only_fields`, поскольку их исходные значения и так сохраняются.

Сравнение полей только для хранения со строковыми атрибутами:

- **Поле только для хранения (stored-only field)**:
  - Хранится только на диске
  - Сжатый формат
  - Может только извлекаться (не используется для фильтрации, сортировки и т.д.)

- **Строковый атрибут (string attribute)**:
  - Хранится на диске и в памяти
  - Несжатый формат (если вы не используете колоночное хранилище)
  - Может использоваться для сортировки, фильтрации, группировки и т.д.

Если вы хотите, чтобы Manticore хранил текстовые данные для вас **только** на диске (например, JSON-данные, которые возвращаются с каждым результатом), а не в памяти, и чтобы они не были доступны для поиска/фильтрации/группировки, используйте `stored_only_fields` или свойство `stored` для вашего текстового поля.

При создании таблиц с помощью SQL-интерфейса помечайте ваше текстовое поле как `stored` (а не `indexed` или `indexed stored`). Вам не понадобится опция `stored_only_fields` в вашем операторе `CREATE TABLE`; её включение может привести к неудачному выполнению запроса.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

По умолчанию вторичные индексы генерируются для всех атрибутов, кроме JSON-атрибутов. Однако вторичные индексы для JSON-атрибутов могут быть явно сгенерированы с помощью настройки `json_secondary_indexes`. Когда JSON-атрибут включён в эту опцию, его содержимое "разворачивается" в несколько вторичных индексов. Эти индексы могут использоваться оптимизатором запросов для ускорения выполнения запросов.

Вы можете просмотреть доступные вторичные индексы с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: Список JSON-атрибутов, разделённых запятыми, для которых должны быть сгенерированы вторичные индексы.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE products(title text, j json secondary_index='1')
```

<!-- request JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, j json secondary_index='1')"
```

<!-- request PHP -->

```php
$params = [
    'body' => [
        'columns' => [
            'title'=>['type'=>'text'],
            'j'=>['type'=>'json', 'options' => ['secondary_index' => 1]]
        ]
    ],
    'table' => 'products'
];
$index = new \Manticoresearch\Index($client);
$index->create($params);
```
<!-- intro -->
##### Python:
<!-- request Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- intro -->
##### Python-asyncio:
<!-- request Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- intro -->
##### Javascript:

<!-- request Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```

<!-- intro -->
##### Java:
<!-- request Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- intro -->
##### C#:
<!-- request C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, j json secondary_index='1')");
```

<!-- intro -->
##### Rust:

<!-- request Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, j json secondary_index='1')", Some(true)).await;
```

<!-- intro -->
##### Typescript:

<!-- request Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index=\'1\')');
```

<!-- intro -->
##### Go:

<!-- request Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, j json secondary_index='1')").Execute()
```


<!-- request CONFIG -->

```ini
table products {
  json_secondary_indexes = j

  type = rt
  path = tbl
  rt_field = title
  rt_attr_json = j
}
```
<!-- end -->

### Настройки таблицы в реальном времени:

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

Таймаут для предотвращения автоматической сброса RAM-чанка, если в таблице нет поисковых запросов. Подробнее [здесь](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

Таймаут для автоматической сброса RAM-чанка, если в него не производится запись. Подробнее [здесь](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

Максимальное количество дисковых чанков для RT-таблицы. Подробнее [здесь](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

Это объявление поля определяет полнотекстовые поля, которые будут проиндексированы. Имена полей должны быть уникальными, и их порядок сохраняется. При вставке данных значения полей должны быть в том же порядке, что и указано в конфигурации.

Это многозначное, необязательное поле.

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

Это объявление определяет атрибут беззнакового целого числа.

Значение: имя поля или field_name:N (где N — максимальное количество бит для сохранения).

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

Это объявление определяет атрибут BIGINT.

Значение: имя поля, допускается несколько записей.

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

Объявляет многозначный атрибут (MVA) со значениями беззнаковых 32-битных целых чисел.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Объявляет многозначный атрибут (MVA) со значениями знаковых 64-битных BIGINT.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Объявляет атрибуты с плавающей точкой одинарной точности, формат IEEE 754 32-бит.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

Объявляет вектор значений с плавающей точкой для хранения эмбеддингов и включения поиска по ближайшим соседям (KNN) по векторам.

Значение: имя поля. Допускается несколько записей.

Каждый векторный атрибут хранит массив чисел с плавающей точкой, которые представляют данные (такие как текст, изображения или другой контент) в виде высокоразмерных векторов. Эти векторы обычно генерируются моделями машинного обучения и могут использоваться для поиска по сходству, рекомендаций, семантического поиска и других функций на основе ИИ.

**Важно:** Атрибуты векторов с плавающей точкой требуют дополнительной конфигурации KNN для включения возможностей векторного поиска.

##### Настройка KNN для векторных атрибутов

Чтобы включить поиск KNN по атрибутам векторов с плавающей точкой, необходимо добавить конфигурацию `knn`, которая определяет параметры индексации. Вы можете настроить KNN двумя способами:

**1. Ручная вставка векторов** (вы предоставляете предварительно вычисленные векторы):

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**2. Автоматические эмбеддинги** (Manticore автоматически генерирует векторы из текста):

```ini
rt_attr_float_vector = embedding_vector
rt_field = title
rt_field = description
knn = {"attrs":[{"name":"embedding_vector","type":"hnsw","hnsw_similarity":"L2","hnsw_m":16,"hnsw_ef_construction":200,"model_name":"sentence-transformers/all-MiniLM-L6-v2","from":"title"}]}
```

**Обязательные параметры KNN:**
- `name`: Имя векторного атрибута (должно совпадать с именем `rt_attr_float_vector`)
- `type`: Тип индекса, в настоящее время поддерживается только `"hnsw"`
- `dims`: Количество измерений в векторах. **Требуется** для ручной вставки векторов, **должно быть опущено** при использовании `model_name` (модель определяет размерности автоматически)
- `hnsw_similarity`: Функция расстояния - `"L2"`, `"IP"` (скалярное произведение) или `"COSINE"`

**Необязательные параметры KNN:**
- `hnsw_m`: Максимальное количество соединений в графе (по умолчанию: 16)
- `hnsw_ef_construction`: Компромисс между временем построения и точностью (по умолчанию: 200)

**Параметры автоматических эмбеддингов** (при использовании `model_name`):
- `model_name`: Модель эмбеддингов для использования (например, `"sentence-transformers/all-MiniLM-L6-v2"`, `"openai/text-embedding-ada-002"`). При указании `dims` должно быть опущено, так как модель автоматически определяет размерности.
- `from`: Список имен полей, разделенных запятыми, для использования при генерации эмбеддингов, или пустая строка `""` для использования всех текстовых/строковых полей. Этот параметр обязателен при указании `model_name`.
- `api_key`: API-ключ для моделей на основе API (OpenAI, Voyage, Jina). Требуется только для сервисов эмбеддингов на основе API.
- `cache_path`: Необязательный путь для кэширования загруженных моделей (для моделей sentence-transformers).
- `use_gpu`: Необязательный булевый флаг для включения ускорения на GPU, если доступно.

**Важно:** Нельзя указывать одновременно `dims` и `model_name` в одной конфигурации — они взаимоисключающие. Используйте `dims` для ручной вставки векторов или `model_name` для автоматических эмбеддингов. Используйте `dims` для ручной вставки векторов или `model_name` для автоматических эмбеддингов.

Для получения более подробной информации о векторном поиске KNN и автоматических эмбеддингах см. [документацию KNN](../../Searching/KNN.md).

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Объявляет булевый атрибут со значениями беззнаковых целых чисел размером 1 бит.

Значение: имя поля.

#### rt_attr_string

```ini
rt_attr_string = title
```

Объявление строкового атрибута.

Значение: имя поля.

#### rt_attr_json

```ini
rt_attr_json = properties
```

Объявляет атрибут JSON.

Значение: имя поля.

#### rt_attr_timestamp

```ini
rt_attr_timestamp = date_added
```

Объявляет атрибут временной метки.

Значение: имя поля.

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

Ограничение памяти для RAM-чанка таблицы. Необязательно, по умолчанию 128M.

RT-таблицы хранят часть данных в памяти, известную как "RAM-чанк", а также поддерживают ряд таблиц на диске, называемых "дисковыми чанками". Эта директива позволяет контролировать размер RAM-чанка. Когда данных слишком много, чтобы хранить их в памяти, RT-таблицы сбрасывают их на диск, активируют вновь созданный дисковый чанк и сбрасывают RAM-чанк.

Обратите внимание, что ограничение является строгим, и RT-таблицы никогда не выделят больше памяти, чем указано в rt_mem_limit. Кроме того, память не предварительно выделяется, поэтому указание лимита 512 МБ и вставка только 3 МБ данных приведет к выделению только 3 МБ, а не 512 МБ.

`rt_mem_limit` никогда не превышается, но фактический размер RAM-чанка может быть значительно ниже этого лимита. RT-таблицы адаптируются к темпу вставки данных и динамически корректируют фактический лимит, чтобы минимизировать использование памяти и максимизировать скорость записи данных. Вот как это работает:
* По умолчанию размер RAM-чанка составляет 50% от `rt_mem_limit`, что называется "лимитом `rt_mem_limit`".
* Как только RAM-чанк накапливает данные, эквивалентные `rt_mem_limit * rate` (по умолчанию 50% от `rt_mem_limit`), Manticore начинает сохранять RAM-чанк как новый дисковый чанк.
* Пока новый дисковый чанк сохраняется, Manticore оценивает количество новых/обновленных документов.
* После сохранения нового дискового чанка обновляется значение `rate` для `rt_mem_limit`.
* Значение `rate` сбрасывается до 50% при каждом перезапуске searchd.

Например, если 90 МБ данных сохраняется в дисковый чанк, и дополнительные 10 МБ данных поступают во время сохранения, то `rate` составит 90%. В следующий раз RT-таблица будет накапливать до 90% от `rt_mem_limit` перед сбросом данных. Чем выше темп вставки, тем ниже значение `rate` для `rt_mem_limit`. `rate` варьируется от 33,3% до 95%. Текущее значение `rate` для таблицы можно посмотреть с помощью команды [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md).

##### Как изменить rt_mem_limit и optimize_cutoff

В режиме реального времени можно настроить лимит размера RAM-чанков и максимальное количество дисковых чанков с помощью оператора `ALTER TABLE`. Чтобы установить `rt_mem_limit` в 1 гигабайт для таблицы "t", выполните следующий запрос: `ALTER TABLE t rt_mem_limit='1G'`. Чтобы изменить максимальное количество дисковых чанков, выполните запрос: `ALTER TABLE t optimize_cutoff='5'`.

В plain-режиме можно изменить значения `rt_mem_limit` и `optimize_cutoff`, обновив конфигурацию таблицы или выполнив команду `ALTER TABLE <table_name> RECONFIGURE`

##### Важные замечания о RAM-чанках

* Таблицы реального времени похожи на [распределенные](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table), состоящие из нескольких локальных таблиц, также известных как дисковые чанки.
* Каждый RAM-чанк состоит из нескольких сегментов, которые являются специальными таблицами, хранящимися только в оперативной памяти.
* В то время как дисковые чанки хранятся на диске, RAM-чанки хранятся в оперативной памяти.
* Каждая транзакция, выполненная с таблицей реального времени, создает новый сегмент, и сегменты RAM-чанка объединяются после каждого коммита транзакции. Более эффективно выполнять массовые вставки (INSERT) сотен или тысяч документов, а не несколько отдельных INSERT с одним документом, чтобы снизить накладные расходы на объединение сегментов RAM-чанка.
* Когда количество сегментов превышает 32, они будут объединены, чтобы сохранить количество ниже 32.
* Таблицы реального времени всегда имеют один RAM-чанк (который может быть пустым) и один или несколько дисковых чанков.
* Объединение больших сегментов занимает больше времени, поэтому лучше избегать очень большого RAM-чанка (и, следовательно, большого значения `rt_mem_limit`).
* Количество дисковых чанков зависит от данных в таблице и настройки `rt_mem_limit`.
* Searchd сбрасывает RAM-чанк на диск (как постоянный файл, а не как дисковый чанк) при завершении работы и периодически в соответствии с настройкой [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайтов на диск может занять некоторое время.
* Большой RAM-чанк создает большую нагрузку на хранилище, как при сбросе на диск в файл `.ram`, так и когда RAM-чанк заполнен и сбрасывается на диск как дисковый чанк.
* RAM-чанк поддерживается [бинарным логом](../../Logging/Binary_logging.md) до тех пор, пока он не будет сброшен на диск, и большее значение `rt_mem_limit` увеличит время, необходимое для воспроизведения бинарного лога и восстановления RAM-чанка.
* RAM-чанк может быть немного медленнее, чем дисковый чанк.
* Хотя сам RAM-чанк не занимает больше памяти, чем `rt_mem_limit`, Manticore может использовать больше памяти в некоторых случаях, например, когда вы начинаете транзакцию для вставки данных и не фиксируете ее в течение длительного времени. В этом случае данные, которые вы уже передали в рамках транзакции, останутся в памяти.

##### Условия сброса RAM-чанка

Помимо `rt_mem_limit`, на поведение сброса RAM-чанков также влияют следующие опции и условия:

* Замороженное состояние. Если таблица [заморожена](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md), сброс откладывается. Это постоянное правило; ничто не может его переопределить. Если условие `rt_mem_limit` достигнуто, пока таблица заморожена, все последующие вставки будут отложены до разморозки таблицы.

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout): Эта опция определяет таймаут (в секундах) для автоматического сброса RAM-чанка, если в него не было записей. Если в течение этого времени не происходит записи, чанк будет сброшен на диск. Установка значения `-1` отключает автоматический сброс на основе активности записи. Значение по умолчанию — 1 секунда.

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout): Эта опция устанавливает таймаут (в секундах) для предотвращения автоматического сброса RAM-чанка, если в таблице не было поисковых запросов. Автоматический сброс произойдет только в том случае, если в течение этого времени был хотя бы один поисковый запрос. Значение по умолчанию — 30 секунд.

* выполняющаяся оптимизация: Если в данный момент выполняется процесс оптимизации и количество существующих дисковых чанков достигло или превысило настроенный внутренний порог `cutoff`, сброс, вызванный таймаутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен.

* слишком мало документов в RAM-сегментах: Если количество документов во всех RAM-сегментах ниже минимального порога (8192), сброс, вызванный таймаутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен, чтобы избежать создания очень маленьких дисковых чанков. Это помогает минимизировать ненужные операции записи на диск и фрагментацию чанков.

Эти таймауты работают совместно. RAM-чанк будет сброшен, если будет достигнут *любой* из таймаутов. Это гарантирует, что даже если нет операций записи, данные в конечном итоге будут сохранены на диск, и наоборот, даже если есть постоянные операции записи, но нет поисковых запросов, данные также будут сохранены. Эти настройки обеспечивают более детальный контроль над частотой сброса RAM-чанков, балансируя необходимость сохранности данных с соображениями производительности. Директивы для этих настроек на уровне таблицы имеют более высокий приоритет и переопределяют глобальные настройки экземпляра.

### Настройки обычной таблицы:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

Поле source указывает источник, из которого документы будут получены во время индексирования текущей таблицы. Должен быть указан хотя бы один источник. Источники могут быть разных типов (например, один может быть MySQL, другой — PostgreSQL). Для получения дополнительной информации об индексировании из внешних хранилищ [читайте здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Значение: Имя источника **обязательно**. Допускается несколько значений.

#### killlist_target

```ini
killlist_target = main:kl
```

Эта настройка определяет таблицу(ы), к которой будет применен kill-list. Совпадения в целевой таблице, которые обновлены или удалены в текущей таблице, будут подавлены. В режиме `:kl` документы для подавления берутся из [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой. Если ни один режим не указан, будут действовать оба режима. [Подробнее о kill-list здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Допускается несколько значений

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Эта конфигурационная настройка определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо построчного хранилища.

Вы можете установить `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колоночном хранилище.

Кроме того, `id` является поддерживаемым атрибутом для хранения в колоночном хранилище.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

По умолчанию все строковые атрибуты, хранящиеся в колоночном хранилище, сохраняют предварительно вычисленные хэши. Эти хэши используются для группировки и фильтрации. Однако они занимают дополнительное место, и если вам не нужно группировать по этому атрибуту, вы можете сэкономить место, отключив генерацию хэшей.

### Создание реальной таблицы онлайн через CREATE TABLE

<!-- example rt_mode -->
##### Общий синтаксис CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Типы данных:

Для получения дополнительной информации о типах данных см. [подробнее о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в файле конфигурации | Примечания | Псевдонимы |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: indexed, stored. По умолчанию: **оба**. Чтобы текст хранился, но не индексировался, укажите только "stored". Чтобы текст только индексировался, укажите только "indexed". | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| целое число	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| большое целое число	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | число с плавающей запятой  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор значений с плавающей запятой  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | мульти-целое число | mva |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | мульти-большое целое число  | mva64 |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | логический |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` сделает значение полнотекстово индексируемым и фильтруемым, сортируемым и группируемым одновременно  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | временная метка  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N — максимальное количество битов для хранения  |   |

<!-- intro -->
##### Примеры создания реальной таблицы через CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

Это создает таблицу "products" с двумя полями: "title" (полнотекстовое) и "price" (с плавающей запятой), и устанавливает "morphology" в "stem_en".

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
Это создает таблицу "products" с тремя полями:
* "title" индексируется, но не хранится.
* "description" хранится, но не индексируется.
* "author" и хранится, и индексируется.
<!-- end -->


## Engine

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

Настройка `engine` изменяет [способ хранения атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) по умолчанию для всех атрибутов в таблице. Вы также можете указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).

Информацию о том, как включить колоночное хранение для обычной таблицы, смотрите в разделе [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs).

Значения:
* columnar - Включает колоночное хранение для всех атрибутов таблицы, кроме [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - Ничего не меняет и использует традиционное построчное хранение для таблицы.


## Другие настройки
Следующие настройки применимы как к реальным, так и к обычным таблицам, независимо от того, указаны ли они в файле конфигурации или установлены онлайн с помощью команд `CREATE` или `ALTER`.

### Связанные с производительностью

#### Доступ к файлам таблицы
Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.

В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, и размер этих буферов можно регулировать с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также существует опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая контролирует, как Manticore открывает файлы при запуске.

В режиме доступа mmap поисковый сервер отображает файл таблицы в память с помощью системного вызова `mmap`, а ОС кэширует содержимое файла. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не оказывают влияния на соответствующие файлы в этом режиме. Читатель mmap также может заблокировать данные таблицы в памяти с помощью привилегированного вызова `mlock`, что предотвращает вытеснение кэшированных данных на диск ОС.

Для управления выбором режима доступа доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** со следующими значениями:

| Значение | Описание |
| - | - |
| file | сервер читает файлы таблицы с диска с помощью seek+read, используя внутренние буферы при доступе к файлу |
| mmap | сервер отображает файлы таблицы в память, и ОС кэширует их содержимое при доступе к файлу |
| mmap_preread | сервер отображает файлы таблицы в память, и фоновый поток читает его один раз для прогрева кэша |
| mlock | сервер отображает файлы таблицы в память, а затем выполняет системный вызов mlock() для кэширования содержимого файла и его блокировки в памяти, чтобы предотвратить его вытеснение на диск |


| Настройка | Значения | Описание |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | управляет тем, как будут читаться `*.spa` (обычные атрибуты) `*.spe` (списки пропуска) `*.spt` (поисковые таблицы) `*.spm` (удалённые документы) |
| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будут читаться `*.spb` (blob-атрибуты) (строковые, многозначные и json атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spp` (списки вхождений) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будет читаться `*.spi` (словарь) |

Вот таблица, которая поможет вам выбрать желаемый режим:

| часть таблицы |	хранить на диске |	хранить в памяти |	кэшировать в памяти при запуске сервера | заблокировать в памяти |
| - | - | - | - | - |
| обычные атрибуты в [построчном](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоночном) хранении, списки пропуска, списки слов, поисковые таблицы, удалённые документы | 	mmap | mmap |	**mmap_preread** (по умолчанию) | mlock |
| построчные строковые, многозначные атрибуты (MVA) и json атрибуты | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
| [колоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и многозначные атрибуты | всегда  | только средствами ОС  | нет  | не поддерживается |
| списки документов | **file** (по умолчанию) | mmap | нет	| mlock |
| списки вхождений | **file** (по умолчанию) | mmap | нет	| mlock |
| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

##### Рекомендации:

* Для **самого быстрого времени отклика поиска** и наличия достаточного объёма памяти используйте [построчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и блокируйте их в памяти с помощью `mlock`. Кроме того, используйте mlock для списков документов/вхождений.
* Если для вас приоритетом является **недопустимость снижения производительности после запуска**, и вы готовы принять более долгое время старта, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если вы хотите более быстрого перезапуска searchd, придерживайтесь опции по умолчанию `mmap_preread`.
* Если вы хотите **сэкономить память**, но при этом у вас достаточно памяти для всех атрибутов, пропустите использование `mlock`. Операционная система будет определять, что следует хранить в памяти, на основе частоты чтения с диска.
* Если построчные атрибуты **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)
* Если **производительность полнотекстового поиска не является проблемой**, и вы хотите сэкономить память, используйте `access_doclists/access_hitlists=file`

Режим по умолчанию предлагает баланс:
* mmap,
* Предварительное чтение неколоночных атрибутов,
* Поиск и чтение колоночных атрибутов без предварительного чтения,
* Поиск и чтение списков документов/вхождений без предварительного чтения.

Это обеспечивает достойную производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве сценариев.

### Другие настройки, связанные с производительностью

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Этот параметр резервирует дополнительное пространство для обновлений атрибутов-больших объектов (blob), таких как многозначные атрибуты (MVA), строки и JSON. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может измениться. Если обновлённая строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если обновлённая строка длиннее, она будет записана в конец файла `*.spb`. Этот файл отображается в память, что делает его изменение размера потенциально медленной операцией, в зависимости от реализации отображения файлов в память в операционной системе. Чтобы избежать частого изменения размера, можно использовать этот параметр для резервирования дополнительного пространства в конце файла .spb.

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Этот параметр управляет размером блоков, используемых хранилищем документов. Значение по умолчанию — 16kb. Когда исходный текст документа сохраняется с использованием stored_fields или stored_only_fields, он хранится внутри таблицы и сжимается для эффективности. Чтобы оптимизировать доступ к диску и степень сжатия для небольших документов, эти документы объединяются в блоки. Процесс индексации собирает документы до тех пор, пока их общий размер не достигнет порога, указанного этой опцией. В этот момент блок документов сжимается. Этот параметр можно настроить для достижения лучшей степени сжатия (увеличивая размер блока) или более быстрого доступа к тексту документа (уменьшая размер блока).

Значение: размер, по умолчанию **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Этот параметр определяет тип сжатия, используемого для сжатия блоков документов, хранящихся в хранилище документов. Если указаны stored_fields или stored_only_fields, хранилище документов сохраняет сжатые блоки документов. 'lz4' обеспечивает высокую скорость сжатия и распаковки, в то время как 'lz4hc' (высокое сжатие) жертвует некоторой скоростью сжатия для лучшей степени сжатия. 'none' полностью отключает сжатие.

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Уровень сжатия, используемый при применении сжатия 'lz4hc' в хранилище документов. Настраивая уровень сжатия, можно найти правильный баланс между производительностью и степенью сжатия при использовании сжатия 'lz4hc'. Обратите внимание, что эта опция не применима при использовании сжатия 'lz4'.

Значение: целое число от 1 до 12, по умолчанию **9**.

#### preopen

```ini
preopen = 1
```

Этот параметр указывает, что searchd должен открывать все файлы таблицы при запуске или ротации и держать их открытыми во время работы. По умолчанию файлы не открываются заранее. Предварительно открытые таблицы требуют несколько файловых дескрипторов на таблицу, но они устраняют необходимость в вызовах open() для каждого запроса и защищены от состояний гонки, которые могут возникнуть во время ротации таблиц под высокой нагрузкой. Однако, если обслуживается много таблиц, всё же может быть эффективнее открывать их для каждого запроса, чтобы экономить файловые дескрипторы.

Значение: **0** (по умолчанию), или 1.

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

Размер буфера для хранения списка документов на ключевое слово. Увеличение этого значения приведёт к большему использованию памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: размер, по умолчанию **256k**, минимальное значение — 8k.

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

Размер буфера для хранения списка вхождений (hits) на ключевое слово. Увеличение этого значения приведёт к большему использованию памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: размер, по умолчанию **256k**, минимальное значение — 8k.

### Настройки дискового пространства для обычных таблиц

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

Включает инвертирование таблицы на месте (in-place). Необязательный параметр, по умолчанию 0 (использует отдельные временные файлы).

Опция `inplace_enable` уменьшает занимаемое дисковое пространство во время индексации обычных таблиц, слегка замедляя индексацию (используется примерно в 2 раза меньше диска, но производительность составляет около 90-95% от исходной).

Индексация состоит из двух основных фаз. На первой фазе документы собираются, обрабатываются и частично сортируются по ключевым словам, а промежуточные результаты записываются во временные файлы (.tmp*). На второй фазе документы полностью сортируются и создаются окончательные файлы таблицы. Перестроение рабочей таблицы на лету требует примерно в 3 раза больше пикового дискового пространства: сначала для промежуточных временных файлов, затем для вновь созданной копии и, наконец, для старой таблицы, которая в это время обслуживает рабочие запросы. (Промежуточные данные по размеру сравнимы с окончательной таблицей.) Это может быть слишком большим объёмом дискового пространства для больших коллекций данных, и опция `inplace_enable` может быть использована для его уменьшения. При включении она повторно использует временные файлы, записывает в них окончательные данные и переименовывает их по завершении. Однако это может потребовать дополнительного перемещения фрагментов временных данных, что и приводит к снижению производительности.

Эта директива не влияет на [searchd](../../Starting_the_server/Manually.md), она влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).


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

Опция тонкой настройки [Инвертирования на месте](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable). Управляет размером зазора (gap) предварительно выделенного списка вхождений (hitlist). Необязательный параметр, по умолчанию 0.

Эта директива влияет только на инструмент [searchd](../../Starting_the_server/Manually.md) и не оказывает никакого влияния на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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

Параметр inplace_reloc_factor определяет размер буфера перемещения (relocation buffer) в пределах области памяти (memory arena), используемой во время индексации. Значение по умолчанию — 0.1.

Эта опция является необязательной и влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), а не на сервер [searchd](../../Starting_the_server/Manually.md).

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

Управляет размером буфера, используемого для записи на месте во время индексирования. Необязательный параметр со значением по умолчанию 0.1.

Важно отметить, что эта директива влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) и не затрагивает сервер [searchd](../../Starting_the_server/Manually.md).


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

### Специфические настройки обработки естественного языка
Поддерживаются следующие настройки. Все они описаны в разделе [НЛП и токенизация](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md).
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
