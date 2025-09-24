# Настройки обычных и таблиц с реальным временем

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
##### Пример таблицы с реальным временем в конфигурационном файле
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

### Общие настройки для обычных и таблиц с реальным временем

#### type

```ini
type = plain

type = rt
```

Тип таблицы: "plain" или "rt" (realtime - с реальным временем)

Значение: **plain** (по умолчанию), rt

#### path

```ini
path = path/to/table
```

Путь, где таблица будет храниться или расположена, абсолютный или относительный, без расширения.

Значение: Путь к таблице, **обязательно**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

По умолчанию оригинальное содержимое полнотекстовых полей индексируется и сохраняется при определении таблицы в конфигурационном файле. Этот параметр позволяет указать поля, для которых нужно сохранять оригинальные значения.

Значение: Список через запятую **полнотекстовых** полей, которые должны быть сохранены. Пустое значение (т.е. `stored_fields =`) отключает сохранение оригинальных значений для всех полей.

Примечание: В случае таблицы с реальным временем, поля, перечисленные в `stored_fields`, должны также быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также учтите, что нет необходимости указывать атрибуты в `stored_fields`, поскольку их оригинальные значения сохраняются в любом случае. `stored_fields` можно использовать только для полнотекстовых полей.

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

Используйте `stored_only_fields`, если хотите, чтобы Manticore сохранял некоторые поля обычной или таблицы с реальным временем **на диск, но не индексировал их**. Эти поля не будут доступны для полнотекстового поиска, однако вы сможете получать их значения в результатах поиска.

Например, это полезно, если нужно сохранить данные, такие как JSON-документы, которые должны возвращаться с каждым результатом, но не должны подвергаться поиску, фильтрации или группировке. В таком случае, сохранение только — без индексации — экономит память и улучшает производительность.

Это можно сделать двумя способами:
- В [обычном режиме](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) в конфигурации таблицы используйте настройку `stored_only_fields`.
- В SQL интерфейсе ([RT режим](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) используйте свойство [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) при определении текстового поля (вместо `indexed` или `indexed stored`). В SQL не нужно указывать `stored_only_fields` — оно не поддерживается в операторах `CREATE TABLE`.

Значение `stored_only_fields` — это список через запятую имен полей. По умолчанию оно пусто. Если используется таблица с реальным временем, каждое поле из списка `stored_only_fields` должно также быть объявлено как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Примечание: нет необходимости перечислять атрибуты в `stored_only_fields`, поскольку их оригинальные значения сохраняются в любом случае.

Как сохраняемые только поля отличаются от строковых атрибутов:

- **Поле только для сохранения**:
  - Хранится только на диске
  - Скомпрессированный формат
  - Можно только получить (не используется для фильтрации, сортировки и т.п.)

- **Строковый атрибут**:
  - Хранится на диске и в памяти
  - Несжатый формат (если не используется колоночное хранение)
  - Может использоваться для сортировки, фильтрации, группировки и т.д.

Если вы хотите, чтобы Manticore сохранял текстовые данные, которые вы хотите _только_ хранить на диске (например, json, который возвращается с каждым результатом), и чтобы эти данные не находились в памяти и не были доступны для поиска/фильтрации/группировки, используйте `stored_only_fields` или свойство `stored` для текстового поля.

При создании таблиц через SQL-интерфейс назначьте ваше текстовое поле как `stored` (а не `indexed` или `indexed stored`). Не нужно использовать опцию `stored_only_fields` в операторе `CREATE TABLE`; её включение может привести к ошибке запроса.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

По умолчанию вторичные индексы создаются для всех атрибутов, кроме JSON-атрибутов. Однако вторичные индексы для JSON-атрибутов могут быть явно созданы с помощью параметра `json_secondary_indexes`. Если JSON-атрибут включён в этот параметр, его содержимое расплющивается в несколько вторичных индексов. Эти индексы могут быть использованы оптимизатором запросов для ускорения запросов.

Просмотреть доступные вторичные индексы можно с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: Список через запятую JSON-атрибутов, для которых нужно создавать вторичные индексы.

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

### Настройки таблиц реального времени:

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

Таймаут для предотвращения автоматической очистки RAM-чанка, если в таблице нет поисковых запросов. Подробнее [здесь](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

Таймаут для автоматической очистки RAM-чанка, если к нему нет записей. Подробнее [здесь](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

Максимальное количество дисковых чанков для RT-таблицы. Подробнее [здесь](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

Это объявление поля определяет полнотекстовые поля, которые будут индексироваться. Имена полей должны быть уникальными, порядок сохраняется. При вставке данных значения полей должны быть в том же порядке, что и указано в конфигурации.

Это многозначное, необязательное поле.

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

Это объявление определяет атрибут беззнакового целого числа.

Значение: имя поля или field_name:N (где N — максимальное количество сохраняемых бит).

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

Объявляет многозначный атрибут (MVA) с беззнаковыми 32-битными целочисленными значениями.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Объявляет многозначный атрибут (MVA) с 64-битными знаковыми BIGINT значениями.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Объявляет атрибуты с плавающей точкой одинарной точности, в формате IEEE 754 (32 бита).

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
```

Объявляет вектор значений с плавающей точкой для хранения встраиваний и обеспечения поисков по вектору k-ближайших соседей (KNN).

Значение: имя поля. Допускается несколько записей.

Каждый векторный атрибут хранит массив чисел с плавающей точкой, которые представляют данные (например, текст, изображения или другой контент) в виде векторов высокой размерности. Эти векторы обычно генерируются моделями машинного обучения и могут использоваться для поиска по сходству, рекомендаций, семантического поиска и других функций на базе ИИ.

**Важно:** Атрибуты float vector требуют дополнительной настройки KNN для включения возможностей поиска по векторам.

##### Настройка KNN для векторных атрибутов

Для включения поиска KNN по векторным атрибутам необходимо добавить конфигурацию `knn`, которая задаёт параметры индексации:

```ini
rt_attr_float_vector = image_vector
rt_attr_float_vector = text_vector
knn = {"attrs":[{"name":"image_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200},{"name":"text_vector","type":"hnsw","dims":768,"hnsw_similarity":"COSINE","hnsw_m":16,"hnsw_ef_construction":200}]}
```

**Обязательные параметры KNN:**
- `name`: имя векторного атрибута (должно совпадать с именем `rt_attr_float_vector`)
- `type`: тип индекса, в настоящее время поддерживается только `"hnsw"`
- `dims`: количество измерений в векторах (должно совпадать с выходом вашей модели встраивания)
- `hnsw_similarity`: функция дистанции — `"L2"`, `"IP"` (внутреннее произведение) или `"COSINE"`

**Необязательные параметры KNN:**
- `hnsw_m`: максимальное число связей в графе
- `hnsw_ef_construction`: компромисс между временем построения и точностью

Для деталей по векторному поиску KNN смотрите [документацию KNN](../../Searching/KNN.md).

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Объявляет булевый атрибут с 1-битным беззнаковым целочисленным значением.

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

Объявляет JSON атрибут.

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

RT-таблицы хранят часть данных в памяти, известной как «RAM-чанк», и также поддерживают несколько дисковых таблиц, называемых «дисковыми чанками». Эта директива позволяет контролировать размер RAM-чанка. При переполнении данных в памяти RT-таблицы скидывают их на диск, активируют новый дисковый чанк и сбрасывают RAM-чанк.

Обратите внимание, что это строгое ограничение, RT-таблицы никогда не выделяют больше памяти, чем указано в rt_mem_limit. Кроме того, память не выделяется заранее, поэтому при установке лимита в 512М и вставке только 3М данных будет выделено ровно 3М, а не 512М.

`rt_mem_limit` никогда не превышается, однако фактический размер RAM-чанка может быть значительно меньше лимита. RT-таблицы адаптируются к скорости вставки данных и динамически корректируют реальный предел, чтобы минимизировать использование памяти и максимизировать скорость записи данных. Вот как это работает:
* По умолчанию размер RAM-чанка равен 50% от `rt_mem_limit`, они называют это «лимитом `rt_mem_limit`».
* Как только в RAM-чанке накопится данных равных `rt_mem_limit * rate` (по умолчанию 50% от `rt_mem_limit`), Manticore начинает сохранять RAM-чанк как новый дисковый чанк.
* Во время сохранения нового дискового чанка Manticore оценивает количество новых/обновлённых документов.
* После сохранения дискового чанка значение `rt_mem_limit rate` обновляется.
* При каждой перезагрузке searchd rate сбрасывается в 50%.

Например, если сохраняется 90М данных в дисковый чанк и во время сохранения поступает ещё 10М, rate будет 90%. Следующий раз RT-таблица будет собирать до 90% от `rt_mem_limit` перед очисткой. Чем выше скорость вставки, тем ниже rate `rt_mem_limit`. Rate варьируется от 33,3% до 95%. Текущий rate таблицы можно посмотреть с помощью команды [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md).

##### Как изменить rt_mem_limit и optimize_cutoff

В режиме реального времени вы можете регулировать ограничение на размер фрагментов RAM и максимальное количество фрагментов на диске с помощью оператора `ALTER TABLE`. Чтобы установить `rt_mem_limit` в 1 гигабайт для таблицы "t", выполните следующий запрос: `ALTER TABLE t rt_mem_limit='1G'`. Для изменения максимального количества фрагментов на диске выполните запрос: `ALTER TABLE t optimize_cutoff='5'`.

В простом режиме вы можете изменить значения `rt_mem_limit` и `optimize_cutoff`, обновив конфигурацию таблицы или выполнив команду `ALTER TABLE <table_name> RECONFIGURE`

##### Важные примечания о фрагментах RAM

* Таблицы в реальном времени похожи на [распределённые](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table), состоящие из нескольких локальных таблиц, также известных как фрагменты на диске.
* Каждый фрагмент RAM состоит из нескольких сегментов, которые являются специальными таблицами, размещёнными только в оперативной памяти.
* Пока фрагменты на диске хранятся на диске, фрагменты RAM размещаются в памяти.
* Каждая транзакция в таблице в реальном времени создаёт новый сегмент, и сегменты фрагмента RAM объединяются после каждой фиксации транзакции. Более эффективно выполнять массовые INSERT больших объёмов документов — сотнями или тысячами — вместо множества отдельных INSERT по одному документу, чтобы уменьшить издержки на слияние сегментов RAM.
* Когда количество сегментов превышает 32, они будут объединяться, чтобы поддерживать количество менее 32.
* Таблицы в реальном времени всегда содержат один фрагмент RAM (который может быть пустым) и один или несколько фрагментов на диске.
* Объединение больших сегментов занимает больше времени, поэтому рекомендуется избегать излишне большого фрагмента RAM (и, соответственно, значения `rt_mem_limit`).
* Количество фрагментов на диске зависит от данных в таблице и значения `rt_mem_limit`.
* Searchd сбрасывает фрагмент RAM на диск (в виде файла, а не в виде фрагмента на диске) при завершении работы и периодически согласно настройке [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайт на диск может занять некоторое время.
* Большой фрагмент RAM увеличивает нагрузку на систему хранения как при сбросе на диск в файл `.ram`, так и когда фрагмент RAM заполняется и выгружается на диск как фрагмент на диске.
* Фрагмент RAM поддерживается с помощью [бинарного лога](../../Logging/Binary_logging.md) до тех пор, пока не будет сброшен на диск, и увеличение `rt_mem_limit` повышает время, необходимое для воспроизведения бинарного лога и восстановления фрагмента RAM.
* Фрагмент RAM может работать чуть медленнее, чем фрагмент на диске.
* Хотя сам фрагмент RAM не занимает больше памяти, чем указано в `rt_mem_limit`, Manticore может использовать больше памяти в некоторых случаях, например, если вы начинаете транзакцию для вставки данных и не коммитите её некоторое время. В этом случае данные, переданные в рамках транзакции, будут занимать память до её завершения.

##### Условия сброса фрагмента RAM

Помимо `rt_mem_limit`, на поведение сброса фрагментов RAM влияют и следующие опции и условия:

* Замороженное состояние. Если таблица [заморожена](../../Securing_and_compacting_a_table/Freezing_a_table.md), сброс откладывается. Это постоянное правило, его нельзя изменить. Если достигнуто условие `rt_mem_limit`, пока таблица заморожена, все дальнейшие вставки будут задержаны до размораживания таблицы.

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout): Эта настройка определяет тайм-аут (в секундах) для автоматического сброса фрагмента RAM, если в него не происходят записи. Если запись не была выполнена в течение этого времени, фрагмент будет сброшен на диск. Значение `-1` отключает авто-сброс по отсутствию записей. Значение по умолчанию — 1 секунда.

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout): Данный параметр задаёт тайм-аут (в секундах) для предотвращения авто-сброса фрагмента RAM, если в таблице нет поисковых запросов. Автосброс произойдёт только если за это время был хотя бы один поисковый запрос. Значение по умолчанию — 30 секунд.

* текущая оптимизация: Если в данный момент идёт процесс оптимизации, и количество существующих фрагментов на диске
  достигло или превысило заданный внутренний порог `cutoff`, сброс, вызванный срабатыванием `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен.

* слишком мало документов в сегментах RAM: Если общее количество документов во всех сегментах RAM ниже минимального порога (8192),
  сброс, вызванный срабатыванием `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен, чтобы не создавать слишком маленькие фрагменты на диске. Это помогает свести к минимуму избыточные записи на диск и фрагментацию.

Эти тайм-ауты работают совместно. Фрагмент RAM будет сброшен, если будет достигнут *любой* из этих тайм-аутов. Это гарантирует, что даже при отсутствии записей данные в итоге будут сохранены на диске, и наоборот, даже при постоянных записях, но отсутствии поисков, данные всё равно будут сброшены. Эти настройки обеспечивают более детальный контроль над частотой сброса фрагментов RAM, позволяя сбалансировать долговечность хранения данных и производительность. Директивы на уровне таблицы имеют больший приоритет и переопределяют значения по умолчанию для всего экземпляра.

### Настройки простой таблицы:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

Поле source указывает источник, из которого будут получены документы во время индексации текущей таблицы. Должен быть указан хотя бы один источник. Источники могут быть разных типов (например, один может быть MySQL, другой — PostgreSQL). Подробнее об индексации из внешних хранилищ [читайте здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Значение: имя источника обязательно (**mandatory**). Допускается несколько значений.

#### killlist_target

```ini
killlist_target = main:kl
```

Этот параметр определяет таблицу(цы), к которой будет применён список удаления (kill-list). Совпадения в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавлены. В режиме `:kl` документы для подавления берутся из [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` подавляются все идентификаторы документов из текущей таблицы в целевой. Если ни один не указан, оба режима будут применены. [Подробнее о kill-lists здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Разрешено несколько значений

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Этот параметр конфигурации определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо построчного хранения.

Можно установить `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колоночном хранилище.

Дополнительно, `id` — поддерживаемый атрибут для хранения в колоночном хранилище.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

По умолчанию все строковые атрибуты, хранящиеся в колоночном хранилище, сохраняют предварительно вычисленные хэши. Эти хэши используются для группировки и фильтрации. Однако они занимают дополнительное место, и если вам не нужно группировать по этому атрибуту, можно сэкономить место, отключив генерацию хэшей.

### Создание таблицы реального времени онлайн с помощью CREATE TABLE

<!-- example rt_mode -->
##### Общий синтаксис CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Типы данных:

Для получения дополнительной информации о типах данных смотрите [подробнее о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: indexed, stored. По умолчанию: **оба**. Чтобы сохранить текст хранимым, но не индексируемым, укажите только "stored". Чтобы сохранить текст только индексированным, укажите "indexed" только. | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| integer	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| big integer	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | float  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор значений float  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | multi-integer |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | multi-bigint  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | boolean |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | string. Опция `indexed, attribute` сделает значение полнотекстово индексируемым и одновременно фильтруемым, сортируемым и группируемым  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | timestamp  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N — максимальное количество хранимых бит  |   |

<!-- intro -->
##### Примеры создания таблицы реального времени с использованием CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

Это создаёт таблицу "products" с двумя полями: "title" (полнотекстовый) и "price" (float), и устанавливает "morphology" в "stem_en".

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
Это создаёт таблицу "products" с тремя полями:
* "title" индексируется, но не хранится.
* "description" хранится, но не индексируется.
* "author" хранится и индексируется одновременно.
<!-- end -->


## Engine

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

Параметр engine изменяет хранилище атрибутов по умолчанию ([attribute storage](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)) для всех атрибутов таблицы. Также можно указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).

Для информации о том, как включить колоночное хранение для простой таблицы, смотрите [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) .

Значения:
* columnar - Включает колоночное хранение для всех атрибутов таблицы, за исключением [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - Не меняет ничего и использует традиционное построчное хранение для таблицы.


## Другие настройки
Следующие параметры применимы как для таблиц реального времени, так и для простых таблиц, независимо от того, указаны ли они в конфигурационном файле или установлены онлайн с помощью команды `CREATE` или `ALTER`.

### Связанные с производительностью

#### Доступ к файлам таблицы
Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.

В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, а размер этих буферов можно настроить с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также существует опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая контролирует, как Manticore открывает файлы при запуске.

В режиме доступа mmap сервер поиска отображает файл таблицы в память с помощью системного вызова `mmap`, а ОС кэширует содержимое файла. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не влияют на соответствующие файлы в этом режиме. mmap-ридер также может заблокировать данные таблицы в памяти с помощью привилегированного вызова `mlock`, что предотвращает выгрузку кэшированных данных на диск ОС.

Чтобы управлять используемым режимом доступа, доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** со следующими значениями:

| Value | Description |
| - | - |
| file | сервер читает файлы таблицы с диска с использованием seek+read и внутренних буферов при доступе к файлам |
| mmap | сервер отображает файлы таблицы в память, а ОС кэширует их содержимое при доступе к файлам |
| mmap_preread | сервер отображает файлы таблицы в память, и фоновый поток один раз читает их для прогрева кэша |
| mlock | сервер отображает файлы таблицы в память, затем выполняет системный вызов mlock() для кэширования содержимого файлов и блокирует их в памяти, чтобы предотвратить выгрузку |


| Setting | Values | Description |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | контролирует способ чтения `*.spa` (простые атрибуты) `*.spe` (пропускные списки) `*.spt` (lookup) `*.spm` (удалённые документы) |
| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | контролирует способ чтения `*.spb` (blob-атрибуты) (строковые, MVA и JSON атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | контролирует способ чтения данных `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | контролирует способ чтения данных `*.spp` (списки результатов) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | контролирует способ чтения `*.spi` (словарь) |

Ниже приведена таблица, которая поможет вам выбрать нужный режим:

| часть таблицы |	оставить на диске |	держать в памяти |	кэшировать в памяти при запуске сервера | блокировать в памяти |
| - | - | - | - | - |
| простые атрибуты в [строчно-хранении](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоннарные), пропускные списки, словари, lookup, удалённые документы | 	mmap | mmap |	**mmap_preread** (по умолчанию) | mlock |
| строчные, мультизначные атрибуты (MVA) и JSON атрибуты в строчном формате | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
| [колоннарные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и мультизначные атрибуты | всегда  | только при помощи ОС  | нет  | не поддерживается |
| списки документов | **file** (по умолчанию) | mmap | нет	| mlock |
| списки попаданий | **file** (по умолчанию) | mmap | нет	| mlock |
| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

##### Рекомендации:

* Для **максимально быстрого времени отклика поиска** и при наличии достаточного объёма памяти используйте [строчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и блокируйте их в памяти с помощью `mlock`. Дополнительно используйте mlock для списков документов/попаданий.
* Если вы отдаёте предпочтение тому, чтобы **не снижать работу после запуска**, и готовы принять более длительное время старта, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Для более быстрого перезапуска searchd придерживайтесь параметра по умолчанию `mmap_preread`.
* Если вы хотите **экономить память**, при этом оставляя достаточно памяти для всех атрибутов, избегайте использования `mlock`. Операционная система сама определит, что следует держать в памяти на основе частых обращений к диску.
* Если строчные атрибуты **не помещаются в память**, выбирайте [колоннарные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages).
* Если производительность полнотекстового поиска **не является критичной**, и вы хотите экономить память, используйте `access_doclists/access_hitlists=file`.

Режим по умолчанию представляет собой баланс:
* mmap,
* Предварительное чтение некалоннарных атрибутов,
* Чтение колоннарных атрибутов с seek без предварительного чтения,
* Чтение списков документов/попаданий с seek без предварительного чтения.

Это обеспечивает достойную производительность поиска, оптимальное использование памяти и ускоренный перезапуск searchd в большинстве случаев.

### Другие настройки, связанные с производительностью

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Эта настройка резервирует дополнительное пространство для обновлений blob-атрибутов, таких как мультизначные атрибуты (MVA), строки и JSON. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может изменяться. Если обновлённая строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если обновлённая строка длиннее, она будет записана в конец файла `*.spb`. Этот файл отображается в память, что может сделать изменение его размера медленным процессом в зависимости от реализации отображения памяти в ОС. Чтобы избежать частых изменений размера, можно использовать эту настройку для резервирования дополнительного пространства в конце файла .spb.

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Это параметр контролирует размер блоков, используемых для хранения документов. Значение по умолчанию — 16кб. Когда исходный текст документа хранится с помощью stored_fields или stored_only_fields, он сохраняется внутри таблицы и сжимается для повышения эффективности. Для оптимизации доступа к диску и коэффициентов сжатия для небольших документов эти документы объединяются в блоки. Процесс индексирования собирает документы до тех пор, пока их общий размер не достигнет порогового значения, указанного в этом параметре. В этот момент блок документов сжимается. Этот параметр можно настроить для достижения лучших коэффициентов сжатия (увеличивая размер блока) или более быстрого доступа к тексту документа (уменьшая размер блока).

Значение: size, по умолчанию **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Этот параметр определяет тип сжатия, используемый для сжатия блоков документов, хранящихся в хранилище документов. Если указаны stored_fields или stored_only_fields, хранилище документов хранит сжатые блоки документов. 'lz4' обеспечивает быструю скорость сжатия и распаковки, в то время как 'lz4hc' (высокое сжатие) жертвует некоторой скоростью сжатия ради лучшего коэффициента сжатия. 'none' полностью отключает сжатие.

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Уровень сжатия, используемый при применении сжатия 'lz4hc' в хранилище документов. Изменяя уровень сжатия, вы можете найти оптимальный баланс между производительностью и коэффициентом сжатия при использовании сжатия 'lz4hc'. Обратите внимание, что этот параметр не применяется при использовании сжатия 'lz4'.

Значение: целое число от 1 до 12, по умолчанию **9**.

#### preopen

```ini
preopen = 1
```

Этот параметр указывает, что searchd должен открыть все файлы таблиц при запуске или ротации и держать их открытыми во время работы. По умолчанию файлы не открываются заранее. Предварительно открытые таблицы требуют несколько дескрипторов файлов на таблицу, но они устраняют необходимость вызовов open() при каждом запросе и защищены от условий гонки, которые могут возникать при ротации таблиц под высокой нагрузкой. Однако если вы обслуживаете много таблиц, может быть эффективнее открывать их для каждого запроса отдельно, чтобы экономить дескрипторы файлов.

Значение: **0** (по умолчанию) или 1.

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

Размер буфера для хранения списка документов по ключевому слову. Увеличение этого значения приведет к большему потреблению памяти во время выполнения запроса, но может уменьшить время ввода-вывода.

Значение: size, по умолчанию **256k**, минимальное значение 8k.

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

Размер буфера для хранения списка попаданий по ключевому слову. Увеличение этого значения приведет к большему потреблению памяти во время выполнения запроса, но может уменьшить время ввода-вывода.

Значение: size, по умолчанию **256k**, минимальное значение 8k.

### Параметры занимаемого места на диске для Plain table

#### inplace_enable

<!-- example inplace_enable -->

```ini
inplace_enable = {0|1}
```

Включает инвертирование таблицы на месте. Необязательно, по умолчанию 0 (используются отдельные временные файлы).

Параметр `inplace_enable` уменьшает занимаемое пространство на диске во время индексирования plain tables, при этом немного замедляя индексирование (использует примерно в 2 раза меньше диска, но даёт около 90-95% от первоначальной производительности).

Индексирование состоит из двух основных фаз. В первой фазе документы собираются, обрабатываются и частично сортируются по ключевым словам, а промежуточные результаты записываются во временные файлы (.tmp*). Во второй фазе документы полностью сортируются и создаются конечные файлы таблицы. Перестроение производственной таблицы на лету требует примерно втрое большего пикового использования диска: сначала для промежуточных временных файлов, затем для вновь построенной копии и третий раз для старой таблицы, которая в это время обслуживает запросы. (Промежуточные данные сопоставимы по размеру с конечной таблицей.) Это может быть слишком большим использованием диска для крупных коллекций данных, и опция `inplace_enable` может быть использована для его сокращения. При включении она повторно использует временные файлы, выводит в них окончательные данные и переименовывает их по завершении. Однако это может потребовать дополнительного перемещения временных участков данных, из-за чего возникает влияние на производительность.

Данная директива не влияет на [searchd](../../Starting_the_server/Manually.md), она влияет только на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).


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

Параметр тонкой настройки [инвертирования на месте](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable). Контролирует размер предварительно выделенного промежутка в списке попаданий. Необязательно, по умолчанию 0.

Эта директива влияет только на [searchd](../../Starting_the_server/Manually.md) и не оказывает никакого влияния на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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

Параметр inplace_reloc_factor определяет размер буфера перемещения внутри памяти арены, используемой при индексировании. Значение по умолчанию 0.1.

Этот параметр необязателен и влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), но не на сервер [searchd](../../Starting_the_server/Manually.md).

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
Поддерживаются следующие настройки. Все они описаны в разделе [NLP and tokenization](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md).
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

