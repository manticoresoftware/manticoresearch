# Простые и实时 таблицы настройки

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
##### Пример实时 таблицы в конфигурационном файле
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

### Общие настройки простых и实时 таблиц

#### type

```ini
type = plain

type = rt
```

Тип таблицы: "plain" или "rt" (实时)

Значение: **plain** (по умолчанию), rt

#### path

```ini
path = path/to/table
```

Путь к тому месту, где таблица будет храниться или находиться, либо абсолютный, либо относительный, без расширения.

Значение: Путь к таблице, **обязательный**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

По умолчанию оригинальное содержимое полей полного текста индексируется и сохраняется, когда таблица определяется в конфигурационном файле. Эта настройка позволяет вам указать поля, которые должны иметь сохраненные оригинальные значения.

Значение: Список полей **полного текста**, которые должны быть сохранены, разделенный запятой. Пустое значение (т.е. `stored_fields =` ) отключает хранение оригинальных значений для всех полей.

Примечание: В случае реальной таблицы поля, перечисленные в `stored_fields`, также должны быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также обратите внимание, что вам не нужно перечислять атрибуты в `stored_fields`, так как их оригинальные значения все равно хранятся. `stored_fields` можно использовать только для полей полного текста.

См. также [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) для опций сжатия хранения документов.


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

Используйте `stored_only_fields`, когда хотите, чтобы Manticore хранил некоторые поля простой или实时 таблицы **на диске, но не индексировал их**. Эти поля не будут доступны для поиска с помощью запросов полного текста, но вы все равно сможете извлекать их значения в результатах поиска.

Например, это полезно, если вы хотите хранить данные, такие как JSON-документы, которые должны возвращаться с каждым результатом, но не нужно, чтобы они искались, фильтровались или группировались. В этом случае их хранение только — и не индексирование — экономит память и улучшает производительность.

Вы можете сделать это двумя способами:
- В [простой режиме](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) в конфигурации таблицы используйте настройку `stored_only_fields`.
- В SQL-интерфейсе ([RT mode](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) используйте свойство [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) при определении текстового поля (вместо `indexed` или `indexed stored`). В SQL вам не нужно включать `stored_only_fields` — это не поддерживается в операторе `CREATE TABLE`.

Значение `stored_only_fields` — это список имен полей, разделенный запятыми. По умолчанию он пустой. Если вы используете реальную таблицу, каждое поле, перечисленное в `stored_only_fields`, также должно быть объявлено как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Примечание: вам не нужно перечислять атрибуты в `stored_only_fields`, так как их оригинальные значения все равно хранятся.

Сравнение полей только для хранения и строковых атрибутов:

- **Поле только для хранения**:
  - Хранится только на диске
  - Сжатый формат
  - Может быть извлечено (не используется для фильтрации, сортировки и т.д.)

- **Строковый атрибут**:
  - Хранится на диске и в памяти
  - Несжатый формат (если вы не используете колонковое хранение)
  - Может использоваться для сортировки, фильтрации, группировки и т.д.

Если вы хотите, чтобы Manticore сохранял текстовые данные, которые вы _только_ хотите хранить на диске (например: json данные, которые возвращаются с каждым результатом), и не в памяти или доступные для поиска/фильтрации/группировки, используйте `stored_only_fields`, или `stored` в качестве свойства вашего текстового поля.

При создании таблиц с использованием SQL-интерфейса, пометьте ваше текстовое поле как `stored` (а не `indexed` или `indexed stored`). Вам не понадобится опция `stored_only_fields` в вашем операторе `CREATE TABLE`; включение ее может привести к сбою запроса.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

По умолчанию вторичные индексы генерируются для всех атрибутов, за исключением JSON-атрибутов. Однако вторичные индексы для JSON-атрибутов могут быть явно сгенерированы с помощью настройки `json_secondary_indexes`. Когда JSON-атрибут включен в этот параметр, его содержимое упрощается в несколько вторичных индексов. Эти индексы могут быть использованы оптимизатором запросов для ускорения запросов.

Вы можете просмотреть доступные вторичные индексы с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: Список JSON-атрибутов, для которых должны быть сгенерированы вторичные индексы, разделенный запятыми.

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

### Настройки таблиц в реальном времени:

#### diskchunk_flush_search_timeout

```ini
diskchunk_flush_search_timeout = 10s
```

Тайм-аут для предотвращения автоматической очистки RAM-чанка, если в таблице нет поисковых запросов. Подробнее читайте [здесь](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

Тайм-аут для автоматической очистки RAM-чанка, если в него не производятся записи. Подробнее читайте [здесь](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

Максимальное число дисковых чанков для RT-таблицы. Подробнее читайте [здесь](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

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

Это объявление определяет атрибут с беззнаковым целочисленным значением.

Значение: имя поля или field_name:N (где N — максимальное число бит для хранения).

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

Объявляет мультизначный атрибут (MVA) с беззнаковыми 32-битными целочисленными значениями.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Объявляет мультизначный атрибут (MVA) со знаковыми 64-битными BIGINT значениями.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Объявляет атрибуты с плавающей точкой одинарной точности в формате IEEE 754 32-бита.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

Объявляет вектор значений с плавающей точкой.

Значение: имя поля. Допускается несколько записей.

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Объявляет булев атрибут со значениями 1-битного беззнакового целого.

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

Объявляет атрибут метки времени.

Значение: имя поля.

#### rt_mem_limit

```ini
rt_mem_limit = 512M
```

Лимит памяти для RAM-чанка таблицы. Необязательно, по умолчанию 128M.

RT-таблицы хранят некоторые данные в памяти, известной как "RAM-чанк", а также поддерживают несколько таблиц на диске, называемых "дисковыми чанками". Эта директива позволяет контролировать размер RAM-чанка. Когда данных слишком много для хранения в памяти, RT-таблицы сбрасывают их на диск, активируют вновь созданный дисковый чанк и сбрасывают RAM-чанк.

Обратите внимание, что лимит строгий, и RT-таблицы никогда не выделяют больше памяти, чем указано в rt_mem_limit. Память не выделяется заранее, поэтому указание лимита 512 МБ и вставка только 3 МБ данных приведет к выделению только 3 МБ, а не 512 МБ.

Значение `rt_mem_limit` не превышается, но фактический размер RAM-чанка может быть значительно меньше лимита. RT-таблицы адаптируются к темпу вставки данных и динамически корректируют фактический лимит для минимизации использования памяти и максимизации скорости записи данных. Вот как это работает:
* По умолчанию размер RAM-чанка составляет 50% от `rt_mem_limit`, называемое "`rt_mem_limit` лимит".
* Как только RAM-чанк накапливает данные, эквивалентные `rt_mem_limit * rate` (по умолчанию 50% от `rt_mem_limit`), Manticore начинает сохранять RAM-чанк как новый дисковый чанк.
* Во время сохранения нового дискового чанка Manticore оценивает количество новых/обновленных документов.
* После сохранения нового дискового чанка значение `rt_mem_limit` rate обновляется.
* Rate сбрасывается до 50% при каждом перезапуске searchd.

Например, если на диск сохранено 90 МБ данных и во время сохранения поступает еще 10 МБ, rate составит 90%. В следующий раз RT-таблица будет собирать до 90% от `rt_mem_limit` перед сбросом данных. Чем быстрее темп вставки, тем ниже `rt_mem_limit` rate. Rate варьируется от 33,3% до 95%. Текущий rate таблицы можно посмотреть с помощью команды [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md).

##### Как изменить rt_mem_limit и optimize_cutoff

В режиме реального времени можно изменять лимит размера RAM-чанков и максимальное число дисковых чанков с помощью оператора `ALTER TABLE`. Чтобы установить `rt_mem_limit` в 1 гигабайт для таблицы "t", выполните запрос: `ALTER TABLE t rt_mem_limit='1G'`. Чтобы изменить максимальное количество дисковых чанков, выполните запрос: `ALTER TABLE t optimize_cutoff='5'`.

В обычном режиме значения `rt_mem_limit` и `optimize_cutoff` можно изменить, обновив конфигурацию таблицы или выполнив команду `ALTER TABLE <table_name> RECONFIGURE`

##### Важные заметки о RAM-чанках

* Таблицы реального времени похожи на [распределённые](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table), состоящие из нескольких локальных таблиц, также известных как дисковые чанки.
* Каждый RAM-чанк состоит из нескольких сегментов, это специальные таблицы, работающие только в памяти.
* В то время как дисковые чанки хранятся на диске, RAM-чанки хранятся в памяти.
* Каждая транзакция с real-time таблицей создаёт новый сегмент, и сегменты RAM-чанка объединяются после каждой фиксации транзакции. Эффективнее выполнять пакетные INSERT с сотнями или тысячами документов, чем множество отдельных INSERT с одним документом, чтобы уменьшить накладные расходы на объединение сегментов RAM-чанка.
* Когда количество сегментов превышает 32, они будут объединены, чтобы сохранить количество ниже 32.
* Таблицы в режиме реального времени всегда имеют один блок оперативной памяти (RAM chunk) (который может быть пустым) и один или несколько дисковых блоков (disk chunks).
* Объединение больших сегментов занимает больше времени, поэтому лучше избегать наличия очень большого блока оперативной памяти (RAM chunk) (а следовательно, избегать высокого значения `rt_mem_limit`).
* Количество дисковых блоков зависит от данных в таблице и настройки `rt_mem_limit`.
* Searchd сбрасывает блок оперативной памяти на диск (как сохраняемый файл, а не как дисковый блок) при завершении работы и периодически в соответствии с настройкой [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайт на диск может занять некоторое время.
* Большой блок оперативной памяти создает большую нагрузку на хранилище, как при сбросе на диск в файл `.ram`, так и когда блок памяти заполнен и сбрасывается на диск как дисковый блок.
* Блок оперативной памяти поддерживается [бинарным логом](../../Logging/Binary_logging.md) до тех пор, пока не будет сброшен на диск, и более высокий параметр `rt_mem_limit` увеличит время, необходимое для воспроизведения бинарного лога и восстановления блока памяти.
* Блок оперативной памяти может работать немного медленнее, чем дисковый блок.
* Хотя сам блок оперативной памяти не занимает больше памяти, чем `rt_mem_limit`, Manticore может потреблять больше памяти в некоторых случаях, например, когда вы начинаете транзакцию для вставки данных и не выполняете фиксацию транзакции в течение некоторого времени. В этом случае данные, уже переданные в рамках транзакции, сохраняются в памяти.

##### Условия сброса блока оперативной памяти

Помимо `rt_mem_limit`, поведение сброса блока оперативной памяти также определяется следующими опциями и условиями:

* Состояние «заморожено». Если таблица [заморожена](../../Securing_and_compacting_a_table/Freezing_a_table.md), сброс отложен. Это постоянное правило, которое ничто не может изменить. Если условие `rt_mem_limit` достигается, пока таблица заморожена, все последующие вставки будут отложены до разморозки таблицы.

* [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout): Эта опция задает таймаут (в секундах) для автоматического сброса блока RAM, если в него не поступают записи. Если в течение этого времени запись не происходит, блок будет сброшен на диск. Установка значения `-1` отключает автоматический сброс на основе активности записи. Значение по умолчанию — 1 секунда.

* [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout): Эта опция устанавливает таймаут (в секундах), который препятствует автоматическому сбросу блока оперативной памяти, если в таблице не выполняются поисковые запросы. Автоматический сброс произойдет только в том случае, если за этот период был хотя бы один поиск. Значение по умолчанию — 30 секунд.

* Идёт оптимизация: Если в данный момент выполняется процесс оптимизации и количество существующих дисковых блоков  
  достигло или превысило сконфигурированный внутренний порог `cutoff`, сброс, вызванный истечением таймаута `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен.

* слишком мало документов в сегментах RAM: Если количество документов во всех блоках оперативной памяти меньше минимального порога (8192),  
  сброс, вызванный истечением таймаута `diskchunk_flush_write_timeout` или `diskchunk_flush_search_timeout`, будет пропущен, чтобы избежать создания слишком маленьких дисковых блоков. Это помогает свести к минимуму ненужные записи на диск и фрагментацию блоков.

Эти таймауты работают совместно. Блок оперативной памяти будет сброшен, если сработает *любой* из таймаутов. Это гарантирует, что даже если записей нет, данные в конечном итоге будут сохранены на диске, а наоборот — если поступают постоянные записи, но нет запросов, данные также будут сохранены. Эти настройки предоставляют более детальный контроль над регулярностью сброса блоков оперативной памяти, балансируя между необходимостью надежного хранения данных и требованиями производительности. Директивы для отдельных таблиц имеют более высокий приоритет и переопределяют настройки по умолчанию для всего экземпляра.

### Настройки обычной таблицы:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

Поле source указывает источник, из которого будут получаться документы при индексировании текущей таблицы. Должен быть указан хотя бы один источник. Источники могут быть разных типов (например, один может быть MySQL, другой PostgreSQL). Для получения дополнительной информации об индексировании из внешних хранилищ [читайте здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Значение: Имя источника является **обязательным**. Допускается использование нескольких значений

#### killlist_target

```ini
killlist_target = main:kl
```

Эта настройка определяет таблицу(ы), к которым будет применяться kill-list. Соответствия в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавлены. В режиме `:kl` документы для подавления берутся из [kill-list](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой таблице. Если ничего не указано, будут действовать оба режима. [Подробнее о kill-list можно узнать здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Допускается использование нескольких значений

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Эта настройка конфигурации определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо посрочной (row-wise) структуры.

Вы можете установить `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колоночном хранилище.

Кроме того, `id` — это поддерживаемый атрибут для хранения в колоночном хранилище.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

По умолчанию все строковые атрибуты, хранящиеся в колоночном хранилище, сохраняют предрассчитанные хэши. Эти хэши используются для группировки и фильтрации. Однако они занимают дополнительное пространство, и если вам не нужно группировать по этому атрибуту, вы можете сэкономить место, отключив генерацию хэшей.

### Создание таблицы в режиме реального времени онлайн с помощью команды CREATE TABLE

<!-- example rt_mode -->
##### Общий синтаксис CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Типы данных:

Для получения дополнительной информации о типах данных смотрите [подробнее о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: indexed, stored. По умолчанию: **обе**. Чтобы сохранить текст хранимым, но не индексированным, укажите только "stored". Чтобы сохранить текст индексированным, но не хранимым, укажите только "indexed". | string |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)
| целое число
 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)
| большое целое число
 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | число с плавающей точкой  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор чисел с плавающей точкой  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | мульти-целое число |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | мульти-большое целое число  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | булево значение |   |

| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` одновременно делает значение полнотекстовым индексированным и фильтруемым, сортируемым и группируемым  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |

[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | метка времени  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N — максимальное количество бит для хранения  |   |
<!-- intro -->

##### Примеры создания таблицы реального времени с помощью CREATE TABLE

<!-- request SQL -->
```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```
Это создает таблицу "products" с двумя полями: "title" (полнотекстовое) и "price" (число с плавающей точкой), и устанавливает "морфологию" в "stem_en".
```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```


Это создает таблицу "products" с тремя полями:

* "title" индексируется, но не хранится.
* "description" хранится, но не индексируется.
* "author" одновременно хранится и индексируется.
<!-- end -->

## Движок

```ini

create table ... engine='columnar';
create table ... engine='rowwise';
```


Настройка движка изменяет значение по умолчанию для [хранения атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) для всех атрибутов в таблице. Вы также можете указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).
Для информации о том, как включить колоночное хранение для plain таблицы, см. [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs).

Значения:

* columnar - Включает колоночное хранение для всех атрибутов таблицы, кроме [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - Не изменяет ничего и использует традиционное построчное хранение для таблицы.

## Другие настройки

Следующие настройки применимы как для таблиц реального времени, так и для plain таблиц, независимо от того, указаны ли они в конфигурационном файле или задаются онлайн с помощью команд `CREATE` или `ALTER`.

### Связанные с производительностью

#### Доступ к файлам таблицы
Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.
В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, размер этих буферов можно настроить с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также есть опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая контролирует, как Manticore открывает файлы при старте.
В режиме mmap поисковый сервер отображает файл таблицы в память с помощью системного вызова `mmap`, а ОС кеширует содержимое файла. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не влияют на соответствующие файлы в этом режиме. mmap-ридер также может заблокировать данные таблицы в памяти с помощью привилегированного вызова `mlock`, что предотвращает выгрузку кешированных данных ОС на диск.
Для контроля используемого режима доступа доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict**, со следующими значениями:
| Значение | Описание |


| - | - |
| file | сервер читает файлы таблицы с диска с помощью seek+read, используя внутренние буферы при доступе к файлу |
| mmap | сервер отображает файлы таблицы в память, а ОС кеширует их содержимое при доступе к файлу |
| mmap_preread | сервер отображает файлы таблицы в память, и фоновый поток один раз читает их для разогрева кеша |
| mlock | сервер отображает файлы таблицы в память, а затем выполняет системный вызов mlock() для кеширования содержимого файла и блокировки его в памяти, чтобы предотвратить выгрузку в своп |
| Setting | Values | Description |
| - | - | - |

| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | управляет способом чтения `*.spa` (обычные атрибуты) `*.spe` (skip списки) `*.spt` (lookup) `*.spm` (удалённые документы) |

| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет способом чтения `*.spb` (blob-атрибуты) (строковые, MVA и json атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | управляет способом чтения данных `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | управляет способом чтения данных `*.spp` (списки попаданий) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет способом чтения `*.spi` (словаря) |
Вот таблица, которая поможет выбрать желаемый режим:
| часть таблицы |
оставлять на диске |
держать в памяти |

кешировать в памяти при запуске сервера | блокировать в памяти |

| - | - | - | - | - |
| обычные атрибуты в [построчном](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоночном) хранении, skip списки, словари, lookup, удалённые документы | 
mmap | mmap |
**mmap_preread** (по умолчанию) | mlock |
| построчные строковые, мульти-значные атрибуты (MVA) и json атрибуты | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

| [колоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и мульти-значные атрибуты | всегда  | только средствами ОС  | нет  | не поддерживается |
| списки документов | **file** (по умолчанию) | mmap | нет
| mlock |
| списки попаданий | **file** (по умолчанию) | mmap | нет
| mlock |

| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

##### Рекомендации:

* Для **максимальной скорости отклика поиска** и при наличии достаточно памяти используйте [построчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и блокируйте их в памяти с помощью `mlock`. Дополнительно используйте mlock для списков документов и попаданий.

* Если вы отдаёте предпочтение тому, что **производительность не падает после запуска** и готовы пожертвовать временем старта, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если хотите ускорить перезапуск searchd, оставайтесь с настройкой по умолчанию `mmap_preread`.
* Если вы хотите **экономить память**, при этом имея достаточно памяти для всех атрибутов, избегайте использования `mlock`. Операционная система сама решит, что держать в памяти, основываясь на частоте чтения с диска.
* Если построчные атрибуты **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

* Если производительность полнотекстового поиска **не критична**, а желаете сэкономить память, используйте `access_doclists/access_hitlists=file`

Настройка по умолчанию балансирует между:

* mmap,

* предварительным чтением неколоночных атрибутов,
* доступом и чтением колоночных атрибутов без предварительного чтения,
* доступом и чтением списков документов и попаданий без предварительного чтения.

Это обеспечивает достойную производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве случаев.

### Другие настройки, связанные с производительностью

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Эта настройка резервирует дополнительное пространство для обновлений blob-атрибутов, таких как мульти-значные (MVA), строковые и JSON атрибуты. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может изменяться. Если обновлённая строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если длиннее — запись произойдёт в конце файла `*.spb`. Этот файл отображён в память, и его изменение размера может выполняться медленно, в зависимости от реализации отображённых в память файлов в ОС. Чтобы избежать частого изменения размера, можно зарезервировать дополнительное пространство в конце файла .spb с помощью этой настройки.

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Эта настройка управляет размером блоков, используемых в хранилище документов. Значение по умолчанию — 16kb. Когда исходный текст документа сохраняется через stored_fields или stored_only_fields, он хранится внутри таблицы и сжимается для эффективности. Для оптимизации доступа к диску и коэффициента сжатия для небольших документов они объединяются в блоки. При индексации документы собираются до достижения суммарного размера, указанного в этой опции. После этого блок документов сжимается. Опцию можно настроить для улучшения коэффициента сжатия (увеличив размер блока) или ускорения доступа к тексту документа (уменьшив размер блока).

Значение: размер, по умолчанию **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Эта настройка определяет тип сжатия, используемый для сжатия блоков документов в хранилище документов. Если указаны stored_fields или stored_only_fields, хранилище документов хранит сжатые блоки документов. 'lz4' обеспечивает быструю компрессию и декомпрессию, в то время как 'lz4hc' (высокое сжатие) жертвует скоростью сжатия ради лучшего коэффициента сжатия. 'none' полностью отключает сжатие.

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Уровень сжатия, используемый при применении 'lz4hc' сжатия в хранилище документов. Настройка уровня сжатия позволяет найти подходящий баланс между производительностью и коэффициентом сжатия при использовании 'lz4hc'. Эта опция не применяется при использовании 'lz4'.

Значение: целое число от 1 до 12, по умолчанию **9**.

#### preopen

```ini
preopen = 1
```

Этот параметр указывает, что searchd должен открыть все файлы таблиц при запуске или ротации и держать их открытыми во время работы. По умолчанию файлы не открываются заранее. Предварительно открытые таблицы требуют несколько дескрипторов файлов на таблицу, но они устраняют необходимость вызовов open() для каждого запроса и защищены от гонок, которые могут возникнуть во время ротации таблиц при высокой нагрузке. Однако, если вы обслуживаете много таблиц, может быть эффективнее открывать их для каждого запроса, чтобы сохранить дескрипторы файлов.

Значение: **0** (по умолчанию) или 1.

#### read_buffer_docs

```ini

read_buffer_docs = 1M

```
Размер буфера для хранения списка документов на ключевое слово. Увеличение этого значения приведет к большему потреблению памяти во время выполнения запроса, но может снизить время ввода-вывода.
Значение: размер, по умолчанию **256k**, минимальное значение 8k.

#### read_buffer_hits

```ini

read_buffer_hits = 1M

```


Размер буфера для хранения списка попаданий на ключевое слово. Увеличение этого значения приведет к большему потреблению памяти во время выполнения запроса, но может снизить время ввода-вывода.
Значение: размер, по умолчанию **256k**, минимальное значение 8k.

### Настройки дискового следа для plain table

#### inplace_enable
<!-- example inplace_enable -->
```ini

inplace_enable = {0|1}
```
Включает инвертирование таблицы на месте. Опционально, по умолчанию 0 (используются отдельные временные файлы).
Опция `inplace_enable` уменьшает дисковый след во время индексации plain table, при этом немного замедляя индексацию (использует примерно в 2 раза меньше диска, но обеспечивает около 90-95% исходной производительности).
Индексация состоит из двух основных фаз. Во время первой фазы документы собираются, обрабатываются и частично сортируются по ключевым словам, а промежуточные результаты записываются во временные файлы (.tmp*). Во второй фазе документы полностью сортируются и создаются итоговые файлы таблиц. Перестройка рабочей таблицы на ходу требует примерно в 3 раза больший пиковый дисковый след: сначала для промежуточных временных файлов, затем для вновь построенной копии и, наконец, для старой таблицы, которая будет обслуживать рабочие запросы в это время. (Промежуточные данные сопоставимы по размеру с итоговой таблицей.) Это может быть слишком большой расход дискового пространства для больших наборов данных, и опция `inplace_enable` может использоваться для его уменьшения. При включении она повторно использует временные файлы, записывает в них окончательные данные и переименовывает их по завершении. Однако это может потребовать дополнительного перемещения временных фрагментов данных, что и влияет на производительность.

Эта директива не влияет на [searchd](../../Starting_the_server/Manually.md), ей подвержен только [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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
Опция тонкой настройки [инвертирования на месте](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable). Управляет размером заранее выделенного зазора в списке попаданий. Опционально, по умолчанию 0.
Эта директива влияет только на инструмент [searchd](../../Starting_the_server/Manually.md) и не оказывает влияния на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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
Параметр inplace_reloc_factor определяет размер буфера перемещения внутри областей памяти, используемых во время индексации. Значение по умолчанию — 0.1.
Эта опция необязательна и влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), а не на сервер [searchd](../../Starting_the_server/Manually.md).

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
Управляет размером буфера, используемого для записи на месте во время индексации. Опционально, значение по умолчанию 0.1.
Важно отметить, что эта директива влияет только на инструмент [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), а не на сервер [searchd](../../Starting_the_server/Manually.md).

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

