# Обычные и реальные настройки таблиц

<!-- example config -->
## Определение схемы таблицы в конфигурационном файле

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- intro -->
##### Пример простой таблицы в конфигурационном файле
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
##### Пример реального времени таблицы в конфигурационном файле
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

### Общие настройки для обычных и таблиц реального времени

#### type

```ini
type = plain

type = rt
```

Тип таблицы: "plain" или "rt" (реальное время)

Значение: **plain** (по умолчанию), rt

#### path

```ini
path = path/to/table
```

Путь к месту хранения или расположения таблицы, может быть абсолютным или относительным, без расширения.

Значение: Путь к таблице, **обязательный**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

По умолчанию исходное содержимое полнотекстовых полей индексируется и сохраняется при определении таблицы в конфигурационном файле. Этот параметр позволяет указать поля, для которых должны сохраняться их исходные значения.

Значение: Список полнотекстовых полей, которые должны сохраняться, через запятую. Пустое значение (т.е. `stored_fields =` ) отключает сохранение исходных значений для всех полей.

Примечание: В случае таблицы реального времени поля, указанные в `stored_fields`, должны также быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также обратите внимание, что не нужно указывать атрибуты в `stored_fields`, поскольку их исходные значения сохраняются в любом случае. `stored_fields` можно использовать только для полнотекстовых полей.

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

Используйте `stored_only_fields`, когда хотите, чтобы Manticore сохранял некоторые поля обычной или реальной таблицы **на диске, но не индексировал их**. Эти поля не будут доступны для полнотекстового поиска, но вы все равно сможете получить их значения в результатах поиска.

Например, это полезно, если вы хотите хранить данные, такие как JSON документы, которые должны возвращаться с каждым результатом, но не нуждаются в поиске, фильтрации или группировке. В этом случае только хранение — без индексации — экономит память и улучшает производительность.

Это можно сделать двумя способами:
- В [обычном режиме](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) в конфиге таблицы используйте настройку `stored_only_fields`.
- В SQL интерфейсе ([RT режим](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) используйте свойство [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) при определении текстового поля (вместо `indexed` или `indexed stored`). В SQL вам не нужно включать `stored_only_fields` — оно не поддерживается в командах `CREATE TABLE`.

Значение `stored_only_fields` — это список имен полей через запятую. По умолчанию он пуст. Если вы используете таблицу реального времени, каждое поле из `stored_only_fields` должно быть объявлено также как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Примечание: не нужно указывать атрибуты в `stored_only_fields`, так как их исходные значения в любом случае сохраняются.

Сравнение полей только для хранения со строковыми атрибутами:

- **Только для хранения (stored-only) поле**:
  - Хранится только на диске
  - Сжатый формат
  - Можно только получить (нельзя использовать для фильтрации, сортировки и т.д.)

- **Строковый атрибут**:
  - Хранится и на диске и в памяти
  - Несжатый формат (если не используется колонковое хранение)
  - Можно использовать для сортировки, фильтрации, группировки и т.д.

Если хотите, чтобы Manticore хранил текстовые данные, которые вы _только_ хотите хранить на диске (например, json данные, возвращаемые с каждым результатом), и не хранить их в памяти, а также не делать их доступными для поиска / фильтрации / группировки, используйте `stored_only_fields` или свойство `stored` для вашего текстового поля.

При создании таблиц через SQL интерфейс пометьте ваше текстовое поле как `stored` (а не как `indexed` или `indexed stored`). Вам не понадобится опция `stored_only_fields` в вашем `CREATE TABLE` запросе; её включение может привести к ошибке запроса.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

По умолчанию вторичные индексы генерируются для всех атрибутов, кроме JSON атрибутов. Тем не менее, вторичные индексы для JSON атрибутов могут быть явно сгенерированы с помощью настройки `json_secondary_indexes`. Если JSON атрибут включён в этот параметр, его содержимое преобразуется в несколько вторичных индексов. Эти индексы могут использоваться оптимизатором запросов для ускорения выполнения запросов.

Вы можете просмотреть доступные вторичные индексы с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: Список JSON атрибутов через запятую, для которых следует генерировать вторичные индексы.

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

### Настройки таблицы реального времени:

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

Таймаут для предотвращения автоматической записи RAM-чанка, если в таблице нет поисков. Подробнее [здесь](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

Таймаут для автоматической записи RAM-чанка, если в него нет записей. Подробнее [здесь](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

Максимальное количество дисковых чанков для RT таблицы. Подробнее [здесь](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

Это объявление поля определяет полнотекстовые поля, которые будут индексироваться. Имена полей должны быть уникальными, порядок сохраняется. При вставке данных значения полей должны идти в том же порядке, как указано в конфигурации.

Это мульти-значное, необязательное поле.

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

Значение: имя поля, разрешено несколько записей.

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

Объявляет мульти-значный атрибут (MVA) со значениями беззнаковых 32-битных целых чисел.

Значение: имя поля. Разрешено несколько записей.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Объявляет мульти-значный атрибут (MVA) со значениями знаковых 64-битных BIGINT.

Значение: имя поля. Разрешено несколько записей.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Объявляет атрибуты с плавающей точкой одинарной точности, 32-битный формат IEEE 754.

Значение: имя поля. Разрешено несколько записей.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

Объявляет вектор значений с плавающей точкой для хранения эмбеддингов и обеспечения векторных поисков k-ближайших соседей (KNN).

Значение: имя поля. Разрешено несколько записей.

Каждый векторный атрибут хранит массив чисел с плавающей точкой, которые представляют данные (например, текст, изображения или иной контент) как векторы высокого измерения. Эти векторы обычно генерируются моделями машинного обучения и могут использоваться для поиска сходств, рекомендаций, семантического поиска и других функций на базе искусственного интеллекта.

**Важно:** Векторные атрибуты с плавающей точкой требуют дополнительной конфигурации KNN для активации возможностей векторного поиска.

##### Настройка KNN для векторных атрибутов

Чтобы включить KNN-поиск на векторных атрибутах с плавающей точкой, необходимо добавить конфигурацию `knn`, которая задает параметры индексации:

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**Обязательные параметры KNN:**
- `name`: имя векторного атрибута (должно совпадать с именем `rt_attr_float_vector`)
- `type`: тип индекса, в настоящее время поддерживается только `"hnsw"`
- `dims`: число измерений в векторах (должно соответствовать выходу вашей модели эмбеддингов)
- `hnsw_similarity`: функция расстояния — `"L2"`, `"IP"` (внутреннее произведение) или `"COSINE"`

**Дополнительные параметры KNN:**
- `hnsw_m`: максимальное количество связей в графе
- `hnsw_ef_construction`: компромисс между временем построения и точностью

Для получения дополнительной информации о векторном поиске KNN siehe [документацию KNN](../../Searching/KNN.md).

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Объявляет булевый атрибут с 1-битными беззнаковыми целочисленными значениями.

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

RT таблицы хранят часть данных в памяти, известной как "RAM-чанк", а также поддерживают ряд таблиц на диске, называемых "дисковыми чанками". Эта директива позволяет вам контролировать размер RAM-чанка. Когда данных слишком много, чтобы хранить их в памяти, RT таблицы сбрасывают их на диск, активируют вновь созданный дисковый чанк и сбрасывают RAM-чанк.

Обратите внимание, что ограничение строгое, и RT таблицы никогда не выделяют больше памяти, чем указано в rt_mem_limit. Память не выделяется заранее, поэтому при указании лимита 512MB и вставке всего 3MB данных будет выделено только 3MB, а не 512MB.

`rt_mem_limit` никогда не превышается, но фактический размер RAM-чанка может быть значительно меньше лимита. RT таблицы адаптируются к скорости вставки данных и динамически регулируют фактический лимит, чтобы минимизировать использование памяти и максимизировать скорость записи данных. Это работает следующим образом:
* По умолчанию размер RAM-чанка составляет 50% от `rt_mem_limit`, называемый "`rt_mem_limit` лимит".
* Как только RAM-чанк накапливает количество данных, равное `rt_mem_limit * rate` (по умолчанию 50% от `rt_mem_limit`), Manticore начинает сохранять RAM-чанк как новый дисковый чанк.
* Во время сохранения нового дискового чанка Manticore оценивает количество новых/обновленных документов.
* После сохранения нового дискового чанка обновляется коэффициент `rt_mem_limit rate`.
* Коэффициент сбрасывается на 50% при каждом перезапуске searchd.

Например, если на диск сохраняется 90MB данных, а во время сохранения поступает еще 10MB, коэффициент будет 90%. В следующий раз RT таблица соберет до 90% от `rt_mem_limit` перед сбросом данных. Чем выше скорость вставки, тем ниже коэффициент `rt_mem_limit`. Он варьируется от 33.3% до 95%. Текущий коэффициент таблицы можно посмотреть с помощью команды [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md).

##### Как изменить rt_mem_limit и optimize_cutoff

В режиме реального времени вы можете изменить лимит размера RAM-чанков и максимальное количество дисковых чанков с помощью оператора `ALTER TABLE`. Чтобы установить `rt_mem_limit` равным 1 гигабайту для таблицы "t", выполните следующий запрос: `ALTER TABLE t rt_mem_limit='1G'`. Чтобы изменить максимальное количество дисковых чанков, выполните запрос: `ALTER TABLE t optimize_cutoff='5'`.

В обычном режиме вы можете изменить значения `rt_mem_limit` и `optimize_cutoff`, обновив конфигурацию таблицы или выполнив команду `ALTER TABLE <table_name> RECONFIGURE`

##### Важные заметки о RAM-чанках

* Таблицы реального времени подобны [распределённым](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table), которые состоят из нескольких локальных таблиц, также известных как дисковые чанки.
* Каждый RAM-чанк состоит из нескольких сегментов — специальных таблиц, находящихся исключительно в памяти.
* В то время как дисковые чанки хранятся на диске, RAM-чанки хранятся в памяти.
* Каждая транзакция, выполненная с таблицей реального времени, создаёт новый сегмент, и сегменты RAM-чанка сливаются после каждого коммита транзакции. Лучше выполнять массовые INSERT-операции сотнями или тысячами документов, чем множественные отдельные INSERT с одним документом, чтобы уменьшить накладные расходы на слияние сегментов RAM-чанков.
* Когда количество сегментов превышает 32, они объединяются, чтобы сохранить число ниже 32.
* Таблицы реального времени всегда имеют один RAM-чанк (который может быть пуст) и один или несколько дисковых чанков.
* Слияние больших сегментов занимает больше времени, поэтому лучше избегать слишком большого RAM-чанка (и, следовательно, большого значения `rt_mem_limit`).
* Количество дисковых чанков зависит от данных в таблице и настройки `rt_mem_limit`.
* Searchd сбрасывает RAM-чанк на диск (в виде сохранённого файла, а не как дисковый чанкт) при завершении работы и периодически согласно настройке [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайт на диск может занять некоторое время.
* Большой RAM-чанк создаёт большую нагрузку на хранилище как при сбросе в файл `.ram`, так и при переполнении и сбросе RAM-чанка на диск как дискового чанка.
* RAM-чанк поддерживается с помощью [бинарного лога](../../Logging/Binary_logging.md) до тех пор, пока не будет сброшен на диск, и увеличение значения `rt_mem_limit` увеличит время воспроизведения бинарного лога и восстановления RAM-чанка.
* RAM-чанк может работать немного медленнее, чем дисковый чанкт.
* Несмотря на то, что RAM-чанк сам по себе не занимает больше памяти, чем `rt_mem_limit`, в некоторых случаях Manticore может занять больше памяти, например, если вы начали транзакцию для вставки данных и не зафиксировали её некоторое время. В этом случае данные, уже переданные в рамках транзакции, останутся в памяти.

##### Условия сброса RAM-чанков

Помимо `rt_mem_limit`, поведение сброса RAM-чанков также зависит от следующих опций и условий:

* Замороженное состояние. Если таблица [заморожена](../../Securing_and_compacting_a_table/Freezing_a_table.md), сброс отложен. Это постоянное правило, его нельзя переопределить. Если условие `rt_mem_limit` достигнуто при замороженной таблице, все дальнейшие вставки будут задержаны до разморозки таблицы.

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout): Эта опция задаёт тайм-аут (в секундах) для автоматического сброса RAM-чанка, если туда не записывались данные. Если в течение этого времени не происходит запись, чанкт будет сброшен на диск. Значение `-1` отключает авто-сброс на основе активности записи. Значение по умолчанию — 1 секунда.

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout): Эта опция задаёт тайм-аут (в секундах) для предотвращения авто-сброса RAM-чанка, если в таблице нет поисковых запросов. Авто-сброс произойдёт только, если был хотя бы один поиск за этот период. Значение по умолчанию — 30 секунд.

* Идёт оптимизация: если в данный момент выполняется процесс оптимизации и количество существующих дисковых чанков
  достигло или превысило внутренний сконфигурированный порог `cutoff`, сброс вызванный тайм-аутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout` будет пропущен.

* Слишком мало документов в сегментах RAM: если количество документов в сегментах RAM меньше минимального порога (8192),
  сброс, вызванный тайм-аутом `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен, чтобы избежать создания очень маленьких дисковых чанков. Это помогает минимизировать ненужные записи на диск и фрагментацию чанков.

Эти тайм-ауты действуют совместно. RAM-чанк будет сброшен, если достигнут *любой* из тайм-аутов. Это гарантирует, что данные со временем сохранятся на диск, даже если не происходит запись, и наоборот, если происходит постоянная запись, но нет поисков, данные тоже будут сохраняться. Эти настройки позволяют более точно контролировать частоту сброса RAM-чанков, балансируя между сохранностью данных и производительностью. Настройки на уровне таблицы имеют более высокий приоритет и переопределяют глобальные значения по умолчанию.

### Настройки обычной таблицы:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

Поле source определяет источник, из которого будут получены документы при индексировании текущей таблицы. Источник должен быть хотя бы один. Источники могут быть разного типа (например, один может быть MySQL, другой PostgreSQL). Для получения дополнительной информации об индексировании из внешних хранилищ [читайте здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Значение: имя источника **обязательно**. Допускается указание нескольких значений.

#### killlist_target

```ini
killlist_target = main:kl
```

Этот параметр определяет таблицу(цы), к которым будет применяться kill-list. Совпадения в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавлены. В режиме `:kl` подавляемые документы берутся из [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой. Если ни один режим не указан, оба режима будут действовать одновременно. [Узнайте больше о kill-list здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Допускается указание нескольких значений

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Этот параметр конфигурации определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо построчного хранения.

Вы можете установить `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колоночном хранилище.

Дополнительно поддерживается атрибут `id` для хранения в колоночном хранилище.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

По умолчанию все строковые атрибуты, хранящиеся в колоночном хранилище, сохраняют предвычисленные хеши. Эти хеши используются для группировки и фильтрации. Однако они занимают дополнительное место, и если вам не нужно группировать по этому атрибуту, вы можете сэкономить пространство, отключив генерацию хешей.

### Создание таблицы в реальном времени онлайн через CREATE TABLE

<!-- example rt_mode -->
##### Общий синтаксис CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Типы данных:

Для получения дополнительной информации о типах данных смотрите [подробнее о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: indexed, stored. По умолчанию: **оба**. Чтобы хранить текст но не индексировать, укажите только "stored". Чтобы индексировать текст, но не хранить, укажите только "indexed". | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| целое число	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| большое целое	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | число с плавающей запятой  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор чисел с плавающей запятой  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | многозначное целое |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | многозначное большое целое  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | логический |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` делает значение полнотекстовым индексируемым и одновременно фильтруемым, сортируемым и группируемым  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | метка времени  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N — максимальное количество хранимых бит  |   |

<!-- intro -->
##### Примеры создания таблицы в реальном времени через CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

Это создаёт таблицу "products" с двумя полями: "title" (полнотекстовое) и "price" (число с плавающей точкой), и устанавливает "morphology" в "stem_en".

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
Это создаёт таблицу "products" с тремя полями:
* "title" индексируется, но не хранится.
* "description" хранится, но не индексируется.
* "author" и хранится, и индексируется.
<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "CREATE TABLE products (title text, price float) morphology='stem_en';"
```

Это создаёт таблицу "products" с двумя полями: "title" (полнотекстовое) и "price" (число с плавающей точкой), и задаёт "morphology" как "stem_en".

```JSON
POST /sql?mode=raw -d "CREATE TABLE products (title text indexed, description text stored, author text, price float);"
```
Это создаёт таблицу "products" с тремя полями:
* "title" индексируется, но не сохраняется.
* "description" сохраняется, но не индексируется.
* "author" и сохраняется, и индексируется.

<!-- end -->


## Engine

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

Параметр engine изменяет умолчательное [хранилище атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) для всех атрибутов таблицы. Вы также можете указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).

Для информации о том, как включить колоночное хранилище в обычной таблице, смотрите [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs).

Значения:
* columnar - включает колоночное хранилище для всех атрибутов таблицы, за исключением [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - ничего не меняет, использует традиционное построчное хранение для таблицы.


## Другие настройки
Следующие настройки применимы как для таблиц реального времени, так и для обычных таблиц, независимо от того, указаны ли они в конфигурационном файле или заданы онлайн с помощью команды `CREATE` или `ALTER`.

### Производительность

#### Доступ к файлам таблицы
Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.

В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, и размер этих буферов можно настроить с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также существует опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая управляет тем, как Manticore открывает файлы при запуске.

В режиме доступа mmap поисковый сервер отображает файл таблицы в память с помощью системного вызова `mmap`, и ОС кеширует содержимое файла. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не влияют на соответствующие файлы в этом режиме. mmap-ридер также может заблокировать данные таблицы в памяти, используя привилегированный вызов `mlock`, что предотвращает выгрузку кешированных данных на диск ОС.

Для управления используемым режимом доступа доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** со следующими значениями:

| Значение | Описание |
| - | - |
| file | сервер читает файлы таблицы с диска через seek+read, используя внутренние буферы при обращении к файлам |
| mmap | сервер отображает файлы таблицы в память, и ОС кеширует их содержимое при обращении к файлам |
| mmap_preread | сервер отображает файлы таблицы в память, а поток в фоновом режиме один раз читает их для прогрева кеша |
| mlock | сервер отображает файлы таблицы в память, а затем выполняет системный вызов mlock() для кеширования содержимого файлов и блокировки в памяти, чтобы предотвратить выгрузку |


| Настройка | Значения | Описание |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | управляет тем, как будут читаться файлы `*.spa` (обычные атрибуты), `*.spe` (пропускаемые списки), `*.spt` (поисковые индексы), `*.spm` (удалённые документы) |
| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будут читаться файлы `*.spb` (blob-атрибуты) (строковые, MVA и json атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spp` (списки попаданий) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будет читаться файл `*.spi` (словарь) |

Ниже приведена таблица, которая поможет выбрать нужный режим:

| часть таблицы |	оставлять на диске |	хранить в памяти |	кешировать в памяти при запуске сервера | блокировать в памяти |
| - | - | - | - | - |
| обычные атрибуты в [строчном](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоночном) хранении, пропускаемые списки, словари, поисковые индексы, удалённые документы | 	mmap | mmap |	**mmap_preread** (по умолчанию) | mlock |
| строковые, мультизначные атрибуты (MVA) и JSON атрибуты в строчном хранении | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
| [колоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и мультизначные атрибуты | всегда  | только средствами ОС  | нет  | не поддерживается |
| списки документов | **file** (по умолчанию) | mmap | нет	| mlock |
| списки попаданий | **file** (по умолчанию) | mmap | нет	| mlock |
| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

##### Рекомендации:

* Для **максимально быстрого времени отклика поиска** и достаточного объёма памяти используйте [строчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и блокируйте их в памяти с помощью `mlock`. Также применяйте mlock для doclists/hitlists.
* Если для вас приоритет — **не снижать производительность после запуска** и вы готовы принять более долгое время старта сервера, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если вы хотите более быстрый перезапуск searchd, оставьте значение по умолчанию `mmap_preread`.
* Если вы хотите **экономить память**, но при этом иметь достаточно памяти для всех атрибутов, избегайте использования `mlock`. Операционная система сама определит, что держать в памяти на основе частоты чтения с диска.
* Если строчные атрибуты **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages).
* Если производительность полнотекстового поиска **не является приоритетом**, и вы хотите сэкономить память, используйте `access_doclists/access_hitlists=file`.

Режим по умолчанию обеспечивает баланс между:
* mmap,
* Прогревом некроночных атрибутов,
* Чтением колоночных атрибутов с диска без прогрева,
* Чтением списков документов/списков попаданий с диска без прогрева.

Это обеспечивает достойную производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве случаев.

### Другие настройки, влияющие на производительность

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Эта настройка резервирует дополнительное пространство для обновлений blob-атрибутов, таких как мультизначные атрибуты (MVA), строки и JSON. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может изменяться. Если обновлённая строка короче предыдущей, то она перезапишет старые данные в файле `*.spb`. Если обновлённая строка длиннее, она будет записана в конец файла `*.spb`. Этот файл отображается в память, что делает изменение его размера потенциально медленной операцией, в зависимости от реализации отображения файлов в память в ОС. Чтобы избежать частого изменения размера, можно использовать эту настройку для резервирования дополнительного пространства в конце `.spb` файла.

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Этот параметр контролирует размер блоков, используемых для хранения документов. Значение по умолчанию — 16 Кб. Когда исходный текст документа хранится с использованием stored_fields или stored_only_fields, он хранится внутри таблицы и сжимается для повышения эффективности. Чтобы оптимизировать доступ к диску и коэффициенты сжатия для небольших документов, эти документы объединяются в блоки. Процесс индексирования собирает документы, пока их общий размер не достигнет порога, заданного этим параметром. В этот момент блок документов сжимается. Этот параметр можно настроить для достижения лучших коэффициентов сжатия (путём увеличения размера блока) или более быстрого доступа к тексту документа (путём уменьшения размера блока).

Значение: size, по умолчанию **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Этот параметр определяет тип сжатия, используемого для сжатия блоков документов, хранящихся в хранилище документов. Если указаны stored_fields или stored_only_fields, то хранилище документов сохраняет сжатые блоки документов. 'lz4' обеспечивает высокую скорость сжатия и распаковки, тогда как 'lz4hc' (высокое сжатие) жертвует некоторой скоростью сжатия ради лучшего коэффициента сжатия. 'none' полностью отключает сжатие.

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Уровень сжатия, используемый при применении сжатия 'lz4hc' в хранилище документов. Настраивая уровень сжатия, вы можете найти оптимальный баланс между производительностью и коэффициентом сжатия при использовании 'lz4hc'. Учтите, что этот параметр не применяется при использовании сжатия 'lz4'.

Значение: целое число от 1 до 12, по умолчанию **9**.

#### preopen

```ini
preopen = 1
```

Этот параметр указывает, что searchd должен открывать все файлы таблиц при запуске или ротации и держать их открытыми во время работы. По умолчанию файлы не открываются предварительно. Предварительно открытые таблицы требуют нескольких файловых дескрипторов на таблицу, но устраняют необходимость в вызовах open() для каждого запроса и защищают от гонок, которые могут возникать при ротации таблиц под большой нагрузкой. Однако если у вас много таблиц, может быть эффективнее открывать их по запросу, чтобы сохранять файловые дескрипторы.

Значение: **0** (по умолчанию) или 1.

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

Размер буфера для хранения списка документов по ключевому слову. Увеличение этого значения приведёт к увеличению использования памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: size, по умолчанию **256k**, минимальное значение 8k.

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

Размер буфера для хранения списка попаданий по ключевому слову. Увеличение этого значения приведёт к увеличению использования памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: size, по умолчанию **256k**, минимальное значение 8k.

### Настройки дискового пространства для plain table

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

Включает инвертирование таблицы на месте. Необязательно, по умолчанию 0 (используются отдельные временные файлы).

Опция `inplace_enable` уменьшает занимаемое на диске пространство во время индексирования plain table, при этом немного замедляет индексирование (используется примерно в 2 раза меньше диска, но достигается около 90-95% от исходной производительности).

Индексирование состоит из двух основных этапов. На первом этапе собираются документы, обрабатываются и частично сортируются по ключевому слову, а промежуточные результаты записываются во временные файлы (.tmp*). На втором этапе документы полностью сортируются и создаются итоговые файлы таблиц. Пересборка таблицы в продакшене с ходу требует примерно в 3 раза больше дискового пространства: сначала для промежуточных временных файлов, затем для вновь созданной копии, и наконец для старой таблицы, которая в это время обслуживает запросы. (Промежуточные данные сопоставимы по размеру с финальной таблицей.) Это может быть слишком большим дисковым следом для больших наборов данных, и опция `inplace_enable` может использоваться для его уменьшения. При включении она повторно использует временные файлы, записывает в них финальные данные и переименовывает их после завершения. Однако это может потребовать дополнительного перемещения временных фрагментов данных, откуда исходит влияние на производительность.

Директива не влияет на [searchd](../../Starting_the_server/Manually.md), она затрагивает только [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).


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

Настройка тонкой настройки [инвертирования на месте](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable). Управляет размером предварительно выделенного зазора в hitlist. Необязательно, по умолчанию 0.

Директива влияет только на инструмент [searchd](../../Starting_the_server/Manually.md) и не оказывает воздействия на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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

Параметр inplace_reloc_factor определяет размер буфера релокации внутри memory arena, используемого во время индексирования. Значение по умолчанию — 0.1.

Опция необязательна и влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), а не на сервер [searchd](../../Starting_the_server/Manually.md).

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

Контролирует размер буфера, используемого для записи на месте во время индексирования. Необязательно, значение по умолчанию 0.1.

Важное замечание: эта директива влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool) и не влияет на сервер [searchd](../../Starting_the_server/Manually.md).


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
<!-- proofread -->

