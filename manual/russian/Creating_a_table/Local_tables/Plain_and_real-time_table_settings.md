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
* Каждая транзакция, выполненная с таблицей реального времени, создаёт новый сегмент, и сегменты RAM-чанка объединяются после каждого коммита транзакции. Более эффективно выполнять массовые INSERT-запросы сотнями или тысячами документов, чем множество отдельных INSERT-запросов с одним документом, чтобы уменьшить накладные расходы на слияние сегментов RAM-чанка.
* Когда количество сегментов превышает 32, они объединяются, чтобы сохранить количество ниже 32.
* Таблицы реального времени всегда имеют один RAM-чанк (который может быть пустым) и один или несколько дисковых чанков.
* Слияние больших сегментов занимает больше времени, поэтому лучше избегать очень большого RAM-чанка (и, следовательно, большого значения `rt_mem_limit`).
* Количество дисковых чанков зависит от данных в таблице и настройки `rt_mem_limit`.
* Searchd сбрасывает RAM-чанк на диск (в виде сохранённого файла, а не как дисковый чанк) при завершении работы и периодически в соответствии с настройкой [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайт на диск может занять некоторое время.
* Большой RAM-чанк создаёт большую нагрузку на хранилище как при сбросе на диск в файл `.ram`, так и когда RAM-чанк заполняется и сбрасывается на диск как дисковый чанк.
* RAM-чанк поддерживается [бинарным журналом](../../Logging/Binary_logging.md) до тех пор, пока не будет сброшен на диск, и более высокое значение `rt_mem_limit` увеличит время воспроизведения бинарного журнала и восстановления RAM-чанка.
* RAM-чанк может работать немного медленнее, чем дисковый чанк.
* Хотя сам RAM-чанк не занимает больше памяти, чем `rt_mem_limit`, Manticore может занимать больше памяти в некоторых случаях, например, когда вы начинаете транзакцию для вставки данных и не коммитите её некоторое время. В этом случае данные, которые вы уже передали в рамках транзакции, останутся в памяти.

##### Условия сброса RAM-чанка

Помимо `rt_mem_limit`, поведение сброса RAM-чанков также зависит от следующих опций и условий:

* Состояние заморозки. Если таблица [заморожена](../../Securing_and_compacting_a_table/Freezing_a_table.md), сброс откладывается. Это постоянное правило; ничто не может его переопределить. Если условие `rt_mem_limit` достигнуто, пока таблица заморожена, все последующие вставки будут задержаны до разморозки таблицы.

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout): Эта опция задаёт тайм-аут (в секундах) для автоматического сброса RAM-чанка, если в него не происходит запись. Если в течение этого времени запись не происходит, чанк будет сброшен на диск. Установка значения `-1` отключает автоматический сброс на основе активности записи. Значение по умолчанию — 1 секунда.

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout): Эта опция задаёт тайм-аут (в секундах) для предотвращения автоматического сброса RAM-чанка, если в таблице не выполняются поиски. Автоматический сброс произойдёт только если в течение этого времени был хотя бы один поиск. Значение по умолчанию — 30 секунд.

* Текущая оптимизация: Если в данный момент выполняется процесс оптимизации, и количество существующих дисковых чанков достигло или превысило настроенный внутренний порог `cutoff`, сброс, вызванный тайм-аутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен.
* Слишком мало документов в RAM-сегментах: Если количество документов во всех RAM-сегментах ниже минимального порога (8192),

  сброс, вызванный тайм-аутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен, чтобы избежать создания очень маленьких дисковых чанков. Это помогает минимизировать ненужные записи на диск и фрагментацию чанков.
Эти тайм-ауты работают совместно. RAM-чанк будет сброшен, если достигнут *любой* из тайм-аутов. Это гарантирует, что даже при отсутствии записей данные в конечном итоге будут сохранены на диск, и наоборот, даже при постоянных записях, но отсутствии поисков, данные также будут сохранены. Эти настройки обеспечивают более тонкий контроль над частотой сброса RAM-чанков, балансируя между необходимостью сохранности данных и производительностью. Директивы на уровне таблицы имеют более высокий приоритет и переопределяют значения по умолчанию для всего экземпляра.

### Настройки обычной таблицы:

#### source

source = srcpart1

```ini
source = srcpart2
source = srcpart3
Поле source указывает источник, из которого будут получены документы при индексировании текущей таблицы. Должен быть как минимум один источник. Источники могут быть разных типов (например, один может быть MySQL, другой PostgreSQL). Для получения дополнительной информации об индексировании из внешних хранилищ [читайте здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)
```

Значение: Имя источника является **обязательным**. Допускается несколько значений.

#### killlist_target

killlist_target = main:kl

```ini
Этот параметр определяет таблицу(ы), к которой будет применяться kill-list. Совпадения в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавляться. В режиме `:kl` документы для подавления берутся из [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой. Если ни один режим не указан, будут применяться оба режима. [Узнайте больше о kill-list здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)
```

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Разрешено использовать несколько значений

#### columnar_attrs

columnar_attrs = *

```ini
columnar_attrs = id, attr1, attr2, attr3
Этот параметр конфигурации определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо построчного хранилища.
```

Вы можете задать `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колоночном хранилище.

Кроме того, атрибут `id` поддерживается для хранения в колоночном хранилище.

#### columnar_strings_no_hash

columnar_strings_no_hash = attr1, attr2, attr3

```ini
По умолчанию все строковые атрибуты, хранящиеся в колоночном хранилище, хранят предварительно вычисленные хеши. Эти хеши используются для группировки и фильтрации. Однако они занимают дополнительное пространство, и если вам не нужно группировать по этому атрибуту, вы можете сэкономить место, отключив генерацию хешей.
```

### Создание таблицы в реальном времени через CREATE TABLE

##### Общий синтаксис CREATE TABLE

<!-- example rt_mode -->
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]

```sql
##### Типы данных:
```

Для получения дополнительной информации о типах данных смотрите [больше о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |

| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: indexed, stored. По умолчанию: **оба**. Чтобы сохранить текст в хранилище, но без индексации, укажите только "stored". Чтобы сохранить только индексируемый текст, укажите только "indexed". | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| целое число	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| большое целое число	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | число с плавающей точкой  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор чисел с плавающей точкой  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | множество целых чисел |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | множество больших целых чисел  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | булево значение |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` позволит сделать значение полнотекстово индексируемым, а также фильтруемым, сортируемым и группируемым одновременно  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | метка времени  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N — максимальное количество бит, которое нужно сохранить  |   |
##### Примеры создания таблицы в реальном времени через CREATE TABLE

<!-- intro -->
CREATE TABLE products (title text, price float) morphology='stem_en'
<!-- request SQL -->

```sql
Это создаёт таблицу "products" с двумя полями: "title" (полнотекстовое) и "price" (float) и устанавливает "morphology" на "stem_en".
```

CREATE TABLE products (title text indexed, description text stored, author text, price float)

```sql
Это создаёт таблицу "products" с тремя полями:
```
* "title" индексируется, но не сохраняется.
* "description" сохраняется, но не индексируется.
* "author" сохраняется и индексируется одновременно.
POST /sql?mode=raw -d "CREATE TABLE products (title text, price float) morphology='stem_en';"

<!-- request JSON -->

```JSON
Это создаёт таблицу "products" с двумя полями: "title" (полнотекстовое) и "price" (float) и устанавливает "morphology" на "stem_en".
```

POST /sql?mode=raw -d "CREATE TABLE products (title text indexed, description text stored, author text, price float);"

```JSON
Это создаёт таблицу "products" с тремя полями:
```
* "title" индексируется, но не сохраняется.
* "description" сохраняется, но не индексируется.
* "author" сохраняется и индексируется одновременно.
## Engine

<!-- end -->


create table ... engine='columnar';

```ini
create table ... engine='rowwise';
Параметр engine изменяет стандартное [хранение атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) для всех атрибутов в таблице. Также можно указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).
```

Для информации о том, как включить колоночное хранение для простой таблицы, смотрите [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) .

Значения:

* columnar — Включает колоночное хранение для всех атрибутов таблицы, кроме [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** — Ничего не меняет и использует традиционное построчное хранение для таблицы.
## Другие настройки


Следующие настройки применимы как к таблицам реального времени, так и к обычным таблицам, вне зависимости от того, указаны ли они в конфигурационном файле или установлены онлайн с помощью команды `CREATE` или `ALTER`.
### Настройки, связанные с производительностью

#### Доступ к файлам таблиц

Manticore поддерживает два режима доступа для чтения данных таблиц: seek+read и mmap.
В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, которые представлены файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, и размер этих буферов можно регулировать с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также существует опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая управляет тем, как Manticore открывает файлы при старте.

В режиме mmap сервер поиска отображает файл таблицы в память с помощью системного вызова `mmap`, а ОС кэширует содержимое файла. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не влияют на соответствующие файлы в этом режиме. mmap-читалка также может заблокировать данные таблицы в памяти с помощью привилегированного вызова `mlock`, что предотвращает выгрузку кэшированных данных на диск.

Для управления используемым режимом доступа доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** со следующими значениями:

| Значение | Описание |

| - | - |
| file | сервер читает файлы таблиц с диска с помощью seek+read, используя внутренние буферы при доступе к файлу |
| mmap | сервер отображает файлы таблиц в память, а ОС кэширует их содержимое при доступе к файлу |
| mmap_preread | сервер отображает файлы таблиц в память, а фоновый поток один раз читает их для прогрева кэша |
| mlock | сервер отображает файлы таблиц в память и затем выполняет системный вызов mlock() для кэширования содержимого файла и блокировки его в памяти, чтобы избежать выгрузки |
| Настройка | Значения | Описание |


| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | управляет тем, как будут читаться файлы `*.spa` (простые атрибуты), `*.spe` (списки пропусков), `*.spt` (поисковые обращения), `*.spm` (удалённые документы) |
| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будут читаться файлы `*.spb` (blob-атрибуты) (строковые, мульти-значения и json атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spp` (списки попаданий) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будет читаться файл `*.spi` (словарь) |
Ниже представлена таблица, которая поможет вам выбрать желаемый режим:

| часть таблицы |	хранить на диске |	хранить в памяти |	кэшировать в памяти при старте сервера | блокировать в памяти |

| - | - | - | - | - |
| простые атрибуты в [построчном](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (неколоночном) хранении, списки пропусков, словари, поисковые обращения, удалённые документы | 	mmap | mmap |	**mmap_preread** (по умолчанию) | mlock |
| построчные строковые, мульти-значимые атрибуты (MVA) и json атрибуты | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
| [колоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и мульти-значимые атрибуты | всегда  | только средствами ОС  | нет  | не поддерживается |
| списки документов | **file** (по умолчанию) | mmap | нет	| mlock |
| списки попаданий | **file** (по умолчанию) | mmap | нет	| mlock |
| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
##### Рекомендации:

* Для **максимально быстрого отклика поиска** при достаточном объёме памяти используйте [построчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и блокируйте их в памяти с помощью `mlock`. Также применяйте mlock для списков документов и списков попаданий.

* Если для вас важен **обеспеченный старт с максимальной производительностью** и вы готовы пожертвовать временем запуска, используйте параметр [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если хотите более быстрый перезапуск searchd, придерживайтесь опции по умолчанию `mmap_preread`.
* Если вы стремитесь **экономить память**, но при этом достаточно памяти для всех атрибутов, пропустите использование `mlock`. Операционная система сама определит, что должно оставаться в памяти, исходя из частоты чтения с диска.
* Если построчные атрибуты **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages).
* Если производительность полнотекстового поиска **не критична**, и вы хотите экономить память, используйте `access_doclists/access_hitlists=file`.
Режим по умолчанию обеспечивает баланс между:

* mmap,
* предварительным чтением неколоночных атрибутов,
* чтением и поиском построчных атрибутов без предварительного чтения,
* чтением списков документов и списков попаданий без предварительного чтения.
Это обеспечивает достойную производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве случаев.

### Другие настройки, связанные с производительностью

#### attr_update_reserve

attr_update_reserve = 256k

```ini
Эта настройка резервирует дополнительное пространство для обновлений blob-атрибутов, таких как мульти-значимые атрибуты (MVA), строки и JSON. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может изменяться. Если обновлённая строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если обновлённая строка длиннее, она будет записана в конец файла `*.spb`. Этот файл отображается в память, что делает изменение размера потенциально медленным процессом в зависимости от реализации памяти отображаемых файлов операционной системой. Чтобы избежать частого изменения размера файла, можно использовать эту настройку для резервирования дополнительного пространства в конце файла .spb.
```

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

docstore_block_size = 32k

```ini
Этот параметр контролирует размер блоков, используемых хранилищем документов. Значение по умолчанию — 16 Кб. Когда текст оригинального документа сохраняется с использованием stored_fields или stored_only_fields, он хранится внутри таблицы и сжимается для повышения эффективности. Для оптимизации доступа к диску и коэффициентов сжатия для малых документов эти документы конкатенируются в блоки. Процесс индексации собирает документы, пока их общий размер не достигнет порога, указанного этим параметром. В этот момент блок документов сжимается. Этот параметр можно настроить для достижения лучшего коэффициента сжатия (увеличением размера блока) или более быстрого доступа к тексту документа (уменьшением размера блока).
```

Значение: size, по умолчанию **16k**.

#### docstore_compression

docstore_compression = lz4hc

```ini
Этот параметр определяет тип сжатия, используемый для сжатия блоков документов, хранящихся в документном хранилище. Если указаны stored_fields или stored_only_fields, хранилище документов сохраняет сжатые блоки документов. 'lz4' предлагает высокую скорость сжатия и распаковки, тогда как 'lz4hc' (высокое сжатие) жертвует некоторой скоростью сжатия ради лучшего коэффициента сжатия. 'none' полностью отключает сжатие.
```

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

docstore_compression_level = 12

```ini
Уровень сжатия, используемый при применении сжатия 'lz4hc' в документном хранилище. Настройка уровня сжатия позволяет подобрать оптимальный баланс между производительностью и коэффициентом сжатия при использовании сжатия 'lz4hc'. Обратите внимание, что этот параметр не применяется при использовании сжатия 'lz4'.
```

Значение: целое число от 1 до 12, по умолчанию **9**.

#### preopen

preopen = 1

```ini
Этот параметр указывает, что searchd должен открыть все файлы таблиц при запуске или ротации и удерживать их открытыми во время работы. По умолчанию файлы не открываются заранее. Предварительно открытые таблицы требуют нескольких дескрипторов файлов на таблицу, но устраняют необходимость вызовов open() для каждого запроса и защищены от условий гонки, которые могут возникнуть при ротации таблиц под высокой нагрузкой. Однако, если обслуживается много таблиц, может быть эффективнее открывать их по запросу, чтобы экономить дескрипторы файлов.
```

Значение: **0** (по умолчанию) или 1.

#### read_buffer_docs

read_buffer_docs = 1M

```ini
Размер буфера для хранения списка документов для каждого ключевого слова. Увеличение этого значения приведет к большему использованию памяти во время выполнения запроса, но может уменьшить время ввода-вывода.
```

Значение: size, по умолчанию **256k**, минимальное значение 8k.

#### read_buffer_hits

read_buffer_hits = 1M

```ini
Размер буфера для хранения списка попаданий для каждого ключевого слова. Увеличение этого значения приведет к большему использованию памяти во время выполнения запроса, но может уменьшить время ввода-вывода.
```

Значение: size, по умолчанию **256k**, минимальное значение 8k.

### Настройки отслеживания размера плоской таблицы на диске

#### inplace_enable

inplace_enable = {0|1}

<!-- example inplace_enable -->

```ini
Включает инверсию таблицы на месте. Опционально, по умолчанию 0 (используются отдельные временные файлы).
```

Опция `inplace_enable` уменьшает размер дискового пространства, занятого при индексации плоских таблиц, слегка замедляя индексацию (использует примерно в 2 раза меньше диска, но показывает около 90–95% от исходной производительности).

Индексация состоит из двух основных фаз. На первой фазе документы собираются, обрабатываются и частично сортируются по ключевому слову, а промежуточные результаты записываются во временные файлы (.tmp*). Во второй фазе документы полностью сортируются и создаются конечные файлы таблицы. Перестройка рабочей таблицы в реальном времени требует примерно в 3 раза больше пикового объема дискового пространства: сначала для промежуточных временных файлов, затем для вновь созданной копии, и ещё для старой таблицы, которая в это время обрабатывает производственные запросы. (Промежуточные данные сравнимы по размеру с конечной таблицей.) Это может быть слишком большой расход дискового пространства для больших коллекций данных, и опция `inplace_enable` может использоваться для его сокращения. При включении она переиспользует временные файлы, выводит в них конечные данные и переименовывает их по завершении. Однако это может потребовать дополнительной релокации блоков временных данных, что и объясняет снижение производительности.

Эта директива не влияет на [searchd](../../Starting_the_server/Manually.md), она применяется только к инструменту [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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
Опция тонкой настройки [инверсии на месте](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable). Управляет размером заранее выделенного пространства для пропусков в списках попаданий. Опционально, по умолчанию 0.
```

Эта директива влияет только на инструмент [searchd](../../Starting_the_server/Manually.md) и не оказывает воздействия на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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
Параметр inplace_reloc_factor определяет размер буфера для релокации внутри арены памяти, используемой во время индексации. Значение по умолчанию — 0.1.
```

Эта опция опциональна и влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), а не на сервер [searchd](../../Starting_the_server/Manually.md).

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
Контролирует размер буфера, используемого для записи на месте во время индексации. Опционально, значение по умолчанию 0.1.
```

Важно отметить, что эта директива влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) и не затрагивает сервер [searchd](../../Starting_the_server/Manually.md).

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

