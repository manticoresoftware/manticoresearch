# Настройки обычных и реального времени таблиц

<!-- example config -->
## Определение схемы таблицы в конфигурационном файле

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### Пример обычной таблицы в конфигурационном файле
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
##### Пример таблицы реального времени в конфигурационном файле
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

### Общие настройки обычных и реального времени таблиц

#### type

```ini
type = plain

type = rt
```

Тип таблицы: "plain" или "rt" (реального времени)

Значение: **plain** (по умолчанию), rt

#### path

```ini
path = path/to/table
```

Путь к месту, где таблица будет храниться или расположена, абсолютный или относительный, без расширения.

Значение: Путь к таблице, **обязательно**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

По умолчанию оригинальное содержимое полнотекстовых полей индексируется и сохраняется при определении таблицы в конфигурационном файле. Эта настройка позволяет указать поля, для которых должны сохраняться оригинальные значения.

Значение: Список полнотекстовых полей, разделённых запятыми, которые должны сохраняться. Пустое значение (т.е. `stored_fields =`) отключает сохранение оригинальных значений для всех полей.

Примечание: В случае таблицы реального времени поля, перечисленные в `stored_fields`, также должны быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также обратите внимание, что не нужно включать атрибуты в `stored_fields`, так как их оригинальные значения сохраняются в любом случае. `stored_fields` можно использовать только для полнотекстовых полей.

Смотрите также [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) для опций сжатия хранения документов.


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

Используйте `stored_only_fields`, когда хотите, чтобы Manticore сохранял некоторые поля обычной или реального времени таблицы **на диске, но не индексировал их**. Эти поля не будут доступны для полнотекстового поиска, но вы всё равно сможете получать их значения в результатах поиска.

Например, это полезно, если вы хотите хранить данные, такие как JSON-документы, которые должны возвращаться с каждым результатом, но не нуждаются в поиске, фильтрации или группировке. В этом случае хранение только — без индексации — экономит память и повышает производительность.

Вы можете сделать это двумя способами:
- В [обычном режиме](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) в конфигурации таблицы используйте настройку `stored_only_fields`.
- В SQL-интерфейсе ([RT режим](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) используйте свойство [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) при определении текстового поля (вместо `indexed` или `indexed stored`). В SQL не нужно включать `stored_only_fields` — оно не поддерживается в операторах `CREATE TABLE`.

Значение `stored_only_fields` — это список имён полей, разделённых запятыми. По умолчанию пусто. Если вы используете таблицу реального времени, каждое поле, указанное в `stored_only_fields`, также должно быть объявлено как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Примечание: не нужно включать атрибуты в `stored_only_fields`, так как их оригинальные значения сохраняются в любом случае.

Сравнение полей только для хранения и строковых атрибутов:

- **Поле только для хранения**:
  - Хранится только на диске
  - Сжатый формат
  - Можно только получить (не используется для фильтрации, сортировки и т.д.)

- **Строковый атрибут**:
  - Хранится на диске и в памяти
  - Несжатый формат (если не используется колоночное хранение)
  - Может использоваться для сортировки, фильтрации, группировки и т.д.

Если вы хотите, чтобы Manticore хранил текстовые данные, которые вы _только_ хотите хранить на диске (например, json данные, возвращаемые с каждым результатом), и не хранить в памяти или делать их доступными для поиска/фильтрации/группировки, используйте `stored_only_fields` или `stored` как свойство текстового поля.

При создании таблиц через SQL-интерфейс пометьте текстовое поле как `stored` (а не `indexed` или `indexed stored`). Вам не понадобится опция `stored_only_fields` в операторе `CREATE TABLE`; её включение может привести к ошибке запроса.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

По умолчанию вторичные индексы создаются для всех атрибутов, кроме JSON-атрибутов. Однако вторичные индексы для JSON-атрибутов могут быть явно созданы с помощью настройки `json_secondary_indexes`. Когда JSON-атрибут включён в эту опцию, его содержимое разворачивается в несколько вторичных индексов. Эти индексы могут использоваться оптимизатором запросов для ускорения выполнения запросов.

Вы можете просмотреть доступные вторичные индексы с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: Список JSON-атрибутов, разделённых запятыми, для которых должны быть созданы вторичные индексы.

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

Таймаут для предотвращения автоматической очистки RAM-чанка, если в таблице нет поисковых запросов. Подробнее см. [здесь](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

Таймаут для автоматической очистки RAM-чанка, если в него не записываются данные. Подробнее см. [здесь](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

Максимальное количество дисковых чанков для RT-таблицы. Подробнее см. [здесь](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

Это объявление поля определяет полнотекстовые поля, которые будут индексироваться. Имена полей должны быть уникальными, порядок сохраняется. При вставке данных значения полей должны быть в том же порядке, что и указано в конфигурации.

Это поле с множественными значениями, необязательное.

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

Это объявление определяет атрибут беззнакового целого числа.

Значение: имя поля или field_name:N (где N — максимальное количество бит для хранения).

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

Объявляет атрибут с множественными значениями (MVA) с беззнаковыми 32-битными целочисленными значениями.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Объявляет атрибут с множественными значениями (MVA) с знаковыми 64-битными значениями BIGINT.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Объявляет атрибуты с плавающей точкой одинарной точности, формат IEEE 754, 32-битный.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

Объявляет вектор значений с плавающей точкой для хранения эмбеддингов и обеспечения поиска по векторам методом k-ближайших соседей (KNN).

Значение: имя поля. Допускается несколько записей.

Каждый векторный атрибут хранит массив чисел с плавающей точкой, которые представляют данные (например, текст, изображения или другой контент) в виде векторов высокой размерности. Эти векторы обычно генерируются моделями машинного обучения и могут использоваться для поиска по сходству, рекомендаций, семантического поиска и других функций на базе ИИ.

**Важно:** Атрибуты с векторами с плавающей точкой требуют дополнительной конфигурации KNN для включения возможностей векторного поиска.

##### Настройка KNN для векторных атрибутов

Чтобы включить поиск KNN по векторным атрибутам с плавающей точкой, необходимо добавить конфигурацию `knn`, которая задает параметры индексирования:

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**Обязательные параметры KNN:**
- `name`: имя векторного атрибута (должно совпадать с именем `rt_attr_float_vector`)
- `type`: тип индекса, в настоящее время поддерживается только `"hnsw"`
- `dims`: количество измерений в векторах (должно соответствовать выходу вашей модели эмбеддингов)
- `hnsw_similarity`: функция расстояния — `"L2"`, `"IP"` (внутреннее произведение) или `"COSINE"`

**Необязательные параметры KNN:**
- `hnsw_m`: максимальное количество связей в графе
- `hnsw_ef_construction`: компромисс между временем построения и точностью

Для подробностей о векторном поиске KNN смотрите [документацию KNN](../../Searching/KNN.md).

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Объявляет булев атрибут с 1-битными беззнаковыми целочисленными значениями.

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

Объявляет JSON-атрибут.

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

RT-таблицы хранят часть данных в памяти, известную как "RAM-чанк", а также поддерживают несколько таблиц на диске, называемых "дисковыми чанками". Эта директива позволяет контролировать размер RAM-чанка. Когда данных слишком много для хранения в памяти, RT-таблицы сбрасывают их на диск, активируют вновь созданный дисковый чанк и сбрасывают RAM-чанк.

Обратите внимание, что ограничение строгое, и RT-таблицы никогда не выделяют памяти больше, чем указано в rt_mem_limit. Кроме того, память не выделяется заранее, поэтому при указании лимита 512 МБ и вставке только 3 МБ данных будет выделено только 3 МБ, а не 512 МБ.

`rt_mem_limit` никогда не превышается, но фактический размер RAM-чанка может быть значительно меньше лимита. RT-таблицы адаптируются к скорости вставки данных и динамически регулируют фактический лимит, чтобы минимизировать использование памяти и максимизировать скорость записи данных. Вот как это работает:
* По умолчанию размер RAM-чанка составляет 50% от `rt_mem_limit`, это называется "`rt_mem_limit` лимит".
* Как только RAM-чанк накапливает данных, эквивалентных `rt_mem_limit * rate` (по умолчанию 50% от `rt_mem_limit`), Manticore начинает сохранять RAM-чанк как новый дисковый чанк.
* Во время сохранения нового дискового чанка Manticore оценивает количество новых/обновленных документов.
* После сохранения нового дискового чанка значение `rt_mem_limit` rate обновляется.
* Rate сбрасывается до 50% при каждом перезапуске searchd.

Например, если на диск сохранено 90 МБ данных, и во время сохранения поступило еще 10 МБ, rate будет 90%. В следующий раз RT-таблица будет собирать до 90% от `rt_mem_limit` перед сбросом данных. Чем быстрее скорость вставки, тем ниже rate `rt_mem_limit`. Rate варьируется от 33.3% до 95%. Текущий rate таблицы можно посмотреть с помощью команды [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md).

##### Как изменить rt_mem_limit и optimize_cutoff

В режиме реального времени вы можете настроить ограничение размера RAM-чанков и максимальное количество дисковых чанков с помощью оператора `ALTER TABLE`. Чтобы установить `rt_mem_limit` в 1 гигабайт для таблицы "t", выполните следующий запрос: `ALTER TABLE t rt_mem_limit='1G'`. Чтобы изменить максимальное количество дисковых чанков, выполните запрос: `ALTER TABLE t optimize_cutoff='5'`.

В обычном режиме вы можете изменить значения `rt_mem_limit` и `optimize_cutoff`, обновив конфигурацию таблицы или выполнив команду `ALTER TABLE <table_name> RECONFIGURE`

##### Важные заметки о RAM-чанках

* Таблицы реального времени похожи на [распределённые](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table), состоящие из нескольких локальных таблиц, также известных как дисковые чанки.
* Каждый RAM-чанк состоит из нескольких сегментов, которые являются специальными таблицами, работающими только в памяти.
* В то время как дисковые чанки хранятся на диске, RAM-чанки хранятся в памяти.
* Каждая транзакция, выполненная с таблицей реального времени, создаёт новый сегмент, и сегменты RAM-чанка объединяются после каждого коммита транзакции. Более эффективно выполнять массовые INSERT-операции сотнями или тысячами документов, чем множество отдельных INSERT-операций с одним документом, чтобы уменьшить накладные расходы на слияние сегментов RAM-чанка.
* Когда количество сегментов превышает 32, они объединяются, чтобы сохранить количество ниже 32.
* Таблицы реального времени всегда имеют один RAM-чанк (который может быть пустым) и один или несколько дисковых чанков.
* Слияние больших сегментов занимает больше времени, поэтому лучше избегать очень большого RAM-чанка (и, следовательно, большого значения `rt_mem_limit`).
* Количество дисковых чанков зависит от данных в таблице и настройки `rt_mem_limit`.
* Searchd сбрасывает RAM-чанк на диск (в виде сохранённого файла, а не как дисковый чанк) при завершении работы и периодически в соответствии с настройкой [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайт на диск может занять некоторое время.
* Большой RAM-чанк создаёт большую нагрузку на хранилище как при сбросе в файл `.ram`, так и когда RAM-чанк заполняется и сбрасывается на диск как дисковый чанк.
* RAM-чанк поддерживается [бинарным журналом](../../Logging/Binary_logging.md) до тех пор, пока не будет сброшен на диск, и большая настройка `rt_mem_limit` увеличит время воспроизведения бинарного журнала и восстановления RAM-чанка.
* RAM-чанк может работать немного медленнее, чем дисковый чанк.
* Хотя сам RAM-чанк не занимает больше памяти, чем `rt_mem_limit`, Manticore может занимать больше памяти в некоторых случаях, например, когда вы начинаете транзакцию для вставки данных и не коммитите её некоторое время. В этом случае данные, которые вы уже передали в рамках транзакции, останутся в памяти.

##### Условия сброса RAM-чанка

Помимо `rt_mem_limit`, поведение сброса RAM-чанков также зависит от следующих опций и условий:

* Состояние заморозки. Если таблица [заморожена](../../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md), сброс откладывается. Это постоянное правило; ничто не может его переопределить. Если условие `rt_mem_limit` достигнуто, пока таблица заморожена, все последующие вставки будут задержаны до разморозки таблицы.

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout): Эта опция задаёт тайм-аут (в секундах) для автоматического сброса RAM-чанка, если в него не происходит запись. Если в течение этого времени запись не происходит, чанк будет сброшен на диск. Установка значения `-1` отключает автоматический сброс на основе активности записи. Значение по умолчанию — 1 секунда.

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout): Эта опция задаёт тайм-аут (в секундах) для предотвращения автоматического сброса RAM-чанка, если в таблице не выполняются поиски. Автоматический сброс произойдёт только если в течение этого времени был хотя бы один поиск. Значение по умолчанию — 30 секунд.

* Текущая оптимизация: Если в данный момент выполняется процесс оптимизации, и количество существующих дисковых чанков достигло или превысило настроенный внутренний порог `cutoff`, сброс, вызванный тайм-аутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен.
* Слишком мало документов в RAM-сегментах: Если количество документов во всех RAM-сегментах ниже минимального порога (8192),

  сброс, вызванный тайм-аутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен, чтобы избежать создания очень маленьких дисковых чанков. Это помогает минимизировать ненужные записи на диск и фрагментацию чанков.
Эти тайм-ауты работают совместно. RAM-чанк будет сброшен, если достигнут *любой* из тайм-аутов. Это гарантирует, что даже при отсутствии записей данные в конечном итоге будут сохранены на диск, и наоборот, даже при постоянных записях, но отсутствии поисков, данные также будут сохранены. Эти настройки обеспечивают более тонкий контроль над частотой сброса RAM-чанков, балансируя потребность в сохранности данных и производительность. Директивы на уровне таблицы для этих настроек имеют более высокий приоритет и переопределяют значения по умолчанию для всего экземпляра.

### Настройки обычной таблицы:

#### source

source = srcpart1

```ini
source = srcpart2
source = srcpart3
Поле source указывает источник, из которого будут получены документы при индексировании текущей таблицы. Должен быть как минимум один источник. Источники могут быть разных типов (например, один может быть MySQL, другой PostgreSQL). Для получения дополнительной информации об индексировании из внешних хранилищ [читайте здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)
```

Значение: Имя источника является **обязательным**. Разрешено несколько значений.

#### killlist_target

killlist_target = main:kl

```ini
Этот параметр определяет таблицу(ы), к которым будет применяться kill-list. Совпадения в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавлены. В режиме `:kl` документы для подавления берутся из [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой. Если ни один из режимов не указан, будут применяться оба режима. [Подробнее о kill-list здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)
```

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Допускается несколько значений

#### columnar_attrs

columnar_attrs = *

```ini
columnar_attrs = id, attr1, attr2, attr3
Этот параметр конфигурации определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо построчного хранилища.
```

Вы можете установить `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колоночном хранилище.

Кроме того, `id` является поддерживаемым атрибутом для хранения в колоночном хранилище.

#### columnar_strings_no_hash

columnar_strings_no_hash = attr1, attr2, attr3

```ini
По умолчанию все строковые атрибуты, хранящиеся в колоночном хранилище, сохраняют предварительно вычисленные хэши. Эти хэши используются для группировки и фильтрации. Однако они занимают дополнительное место, и если вам не нужно группировать по этому атрибуту, вы можете сэкономить место, отключив генерацию хэшей.
```

### Создание таблицы реального времени онлайн через CREATE TABLE

##### Общий синтаксис CREATE TABLE

<!-- example rt_mode -->
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]

```sql
##### Типы данных:
```

Для получения дополнительной информации о типах данных смотрите [подробнее о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |

| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: indexed, stored. По умолчанию: **оба**. Чтобы сохранить текст хранимым, но индексированным, укажите только "stored". Чтобы сохранить текст только индексированным, укажите только "indexed". | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| целое число	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| большое целое число	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | число с плавающей точкой  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор чисел с плавающей точкой  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | мульти-целое число |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | мульти-большое целое число  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | булево значение |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` сделает значение полнотекстово индексируемым и одновременно фильтруемым, сортируемым и группируемым  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | временная метка  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N — максимальное количество бит для хранения  |   |
##### Примеры создания таблицы реального времени через CREATE TABLE

<!-- intro -->
CREATE TABLE products (title text, price float) morphology='stem_en'
<!-- request SQL -->

```sql
Это создаёт таблицу "products" с двумя полями: "title" (полнотекстовый) и "price" (число с плавающей точкой), и устанавливает "morphology" в "stem_en".
```

CREATE TABLE products (title text indexed, description text stored, author text, price float)

```sql
Это создаёт таблицу "products" с тремя полями:
```
* "title" индексируется, но не хранится.
* "description" хранится, но не индексируется.
* "author" хранится и индексируется одновременно.
## Engine
<!-- end -->


create table ... engine='columnar';

```ini
create table ... engine='rowwise';
Параметр engine изменяет стандартное [хранилище атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) для всех атрибутов в таблице. Вы также можете указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).
```

Для информации о том, как включить колоночное хранилище для простой таблицы, смотрите [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs).

Значения:

* columnar - Включает колоночное хранилище для всех атрибутов таблицы, кроме [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - Не изменяет ничего и использует традиционное построчное хранилище для таблицы.
## Другие настройки


Следующие настройки применимы как для таблиц реального времени, так и для простых таблиц, независимо от того, указаны ли они в конфигурационном файле или установлены онлайн с помощью команды `CREATE` или `ALTER`.
### Связанные с производительностью

#### Доступ к файлам таблицы

Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.
В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, и размер этих буферов можно настроить с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также существует опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая контролирует, как Manticore открывает файлы при запуске.

В режиме доступа mmap поисковый сервер отображает файл таблицы в память с помощью системного вызова `mmap`, а ОС кэширует содержимое файла. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не влияют на соответствующие файлы в этом режиме. mmap-ридер также может заблокировать данные таблицы в памяти с помощью привилегированного вызова `mlock`, что предотвращает выгрузку кэшированных данных на диск ОС.

Для управления используемым режимом доступа доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** со следующими значениями:

| Значение | Описание |

| - | - |
| file | сервер читает файлы таблицы с диска с помощью seek+read, используя внутренние буферы при доступе к файлу |
| mmap | сервер отображает файлы таблицы в память, и ОС кэширует их содержимое при доступе к файлу |
| mmap_preread | сервер отображает файлы таблицы в память, и фоновый поток один раз читает их для прогрева кэша |
| mlock | сервер отображает файлы таблицы в память, а затем выполняет системный вызов mlock() для кэширования содержимого файла и блокировки его в памяти, чтобы предотвратить выгрузку |
| Настройка | Значения | Описание |


| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | контролирует, как будут читаться `*.spa` (простые атрибуты), `*.spe` (skip lists), `*.spt` (lookups), `*.spm` (killed docs) |
| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | контролирует, как будут читаться `*.spb` (blob-атрибуты) (строковые, MVA и JSON атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | контролирует, как будут читаться данные `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | контролирует, как будут читаться данные `*.spp` (списки попаданий) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | контролирует, как будет читаться `*.spi` (словарь) |
Ниже приведена таблица, которая поможет выбрать желаемый режим:

| часть таблицы |	оставлять на диске |	хранить в памяти |	кэшировать в памяти при запуске сервера | блокировать в памяти |

| - | - | - | - | - |
| простые атрибуты в [построчном](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоночном) хранении, skip lists, списки слов, lookups, killed docs | 	mmap | mmap |	**mmap_preread** (по умолчанию) | mlock |
| построчные строковые, мультизначные атрибуты (MVA) и JSON атрибуты | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
| [колоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и мультизначные атрибуты | всегда  | только средствами ОС  | нет  | не поддерживается |
| списки документов | **file** (по умолчанию) | mmap | нет	| mlock |
| списки попаданий | **file** (по умолчанию) | mmap | нет	| mlock |
| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
##### Рекомендации:

* Для **максимально быстрого времени отклика поиска** и достаточного объема памяти используйте [построчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и блокируйте их в памяти с помощью `mlock`. Дополнительно используйте mlock для doclists/hitlists.

* Если вы отдаёте предпочтение тому, что **нельзя допустить снижение производительности после запуска** и готовы принять более длительное время запуска, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если вы хотите более быстрый перезапуск searchd, придерживайтесь опции по умолчанию `mmap_preread`.
* Если вы хотите **экономить память**, при этом имея достаточно памяти для всех атрибутов, избегайте использования `mlock`. Операционная система сама определит, что следует держать в памяти, исходя из частоты чтения с диска.
* Если построчные атрибуты **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages).
* Если производительность полнотекстового поиска **не является приоритетом**, и вы хотите сэкономить память, используйте `access_doclists/access_hitlists=file`.
Режим по умолчанию предлагает баланс:

* mmap,
* предварительное чтение неколоночных атрибутов,
* seek+read колоночных атрибутов без предварительного чтения,
* seek+read doclists/hitlists без предварительного чтения.
Это обеспечивает достойную производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве сценариев.

### Другие настройки, связанные с производительностью

#### attr_update_reserve

attr_update_reserve = 256k

```ini
Эта настройка резервирует дополнительное пространство для обновлений blob-атрибутов, таких как мультизначные атрибуты (MVA), строки и JSON. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может изменяться. Если обновленная строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если обновленная строка длиннее, она будет записана в конец файла `*.spb`. Этот файл отображается в память, что делает изменение его размера потенциально медленным процессом, в зависимости от реализации отображения файлов в память в операционной системе. Чтобы избежать частого изменения размера, можно использовать эту настройку для резервирования дополнительного пространства в конце файла .spb.
```

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

docstore_block_size = 32k

```ini
This setting controls the size of blocks used by the document storage. The default value is 16kb. When original document text is stored using stored_fields or stored_only_fields, it is stored within the table and compressed for efficiency. To optimize disk access and compression ratios for small documents, these documents are concatenated into blocks. The indexing process collects documents until their total size reaches the threshold specified by this option. At that point, the block of documents is compressed. This option can be adjusted to achieve better compression ratios (by increasing the block size) or faster access to document text (by decreasing the block size).
```

Value: size, default **16k**.

#### docstore_compression

docstore_compression = lz4hc

```ini
This setting determines the type of compression used for compressing blocks of documents stored in document storage. If stored_fields or stored_only_fields are specified, the document storage stores compressed document blocks. 'lz4' offers fast compression and decompression speeds, while 'lz4hc' (high compression) sacrifices some compression speed for a better compression ratio. 'none' disables compression completely.
```

Values: **lz4** (default), lz4hc, none.

#### docstore_compression_level

docstore_compression_level = 12

```ini
The compression level used when 'lz4hc' compression is applied in document storage. By adjusting the compression level, you can find the right balance between performance and compression ratio when using 'lz4hc' compression. Note that this option is not applicable when using 'lz4' compression.
```

Value: An integer between 1 and 12, with a default of **9**.

#### preopen

preopen = 1

```ini
This setting indicates that searchd should open all table files on startup or rotation, and keep them open while running. By default, the files are not pre-opened. Pre-opened tables require a few file descriptors per table, but they eliminate the need for per-query open() calls and are immune to race conditions that might occur during table rotation under high load. However, if you are serving many tables, it may still be more efficient to open them on a per-query basis in order to conserve file descriptors.
```

Value: **0** (default), or 1.

#### read_buffer_docs

read_buffer_docs = 1M

```ini
Buffer size for storing the list of documents per keyword. Increasing this value will result in higher memory usage during query execution, but may reduce I/O time.
```

Value: size, default **256k**, minimum value is 8k.

#### read_buffer_hits

read_buffer_hits = 1M

```ini
Buffer size for storing the list of hits per keyword. Increasing this value will result in higher memory usage during query execution, but may reduce I/O time.
```

Value: size, default **256k**, minimum value is 8k.

### Plain table disk footprint settings

#### inplace_enable

inplace_enable = {0|1}

<!-- example inplace_enable -->

```ini
Enables in-place table inversion. Optional, default is 0 (uses separate temporary files).
```

The `inplace_enable` option reduces the disk footprint during indexing of plain tables, while slightly slowing down indexing (it uses approximately 2 times less disk, but yields around 90-95% of the original performance).

Indexing is comprised of two primary phases. During the first phase, documents are collected, processed, and partially sorted by keyword, and the intermediate results are written to temporary files (.tmp*). During the second phase, the documents are fully sorted and the final table files are created. Rebuilding a production table on-the-fly requires approximately 3 times the peak disk footprint: first for the intermediate temporary files, second for the newly constructed copy, and third for the old table that will be serving production queries in the meantime. (Intermediate data is comparable in size to the final table.) This may be too much disk footprint for large data collections, and the `inplace_enable` option can be used to reduce it. When enabled, it reuses the temporary files, outputs the final data back to them, and renames them upon completion. However, this may require additional temporary data chunk relocation, which is where the performance impact comes from.

This directive has no effect on [searchd](../../Starting_the_server/Manually.md), it only affects the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

##### CONFIG:


<!-- intro -->
table products {

<!-- request CONFIG -->

```ini
  inplace_enable = 1
  path = products

  source = src_base
}
#### inplace_hit_gap
```
<!-- end -->

inplace_hit_gap = size

<!-- example inplace_hit_gap -->

```ini
The option [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) fine-tuning option. Controls preallocated hitlist gap size. Optional, default is 0.
```

This directive only affects the [searchd](../../Starting_the_server/Manually.md) tool, and does not have any impact on the  [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

##### CONFIG:

<!-- intro -->
table products {

<!-- request CONFIG -->

```ini
  inplace_hit_gap = 1M
  inplace_enable = 1
  path = products

  source = src_base
}
#### inplace_reloc_factor
```
<!-- end -->

inplace_reloc_factor = 0.1

<!-- example inplace_reloc_factor -->

```ini
The inplace_reloc_factor setting determines the size of the relocation buffer within the memory arena used during indexing. The default value is 0.1.
```

This option is optional and only affects the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) tool, not the [searchd](../../Starting_the_server/Manually.md)  server.

##### CONFIG:

<!-- intro -->
table products {

<!-- request CONFIG -->

```ini
  inplace_reloc_factor = 0.1
  inplace_enable = 1
  path = products

  source = src_base
}
#### inplace_write_factor
```
<!-- end -->

inplace_write_factor = 0.1

<!-- example inplace_write_factor -->

```ini
Controls the size of the buffer used for in-place writing during indexing. Optional, with a default value of 0.1.
```

It's important to note that this directive only impacts the [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) tool and not the [searchd](../../Starting_the_server/Manually.md) server.

##### CONFIG:


<!-- intro -->
table products {

<!-- request CONFIG -->

```ini
  inplace_write_factor = 0.1
  inplace_enable = 1
  path = products

  source = src_base
}
### Специфические настройки обработки естественного языка
```
<!-- end -->

Поддерживаются следующие настройки. Все они описаны в разделе [NLP и токенизация](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md).
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
* [wordforms](../../Creating_a_table/NLP_and_tokenization/Wordforms.md#wordforms)
<!-- proofread -->

