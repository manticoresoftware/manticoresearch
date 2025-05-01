# Параметры простой и реального времени таблицы

<!-- пример конфигурации -->
## Определение схемы таблицы в конфигурационном файле

```ini
table <table_name>[:<parent table name>] {
...
}
```

<!-- ввод -->
##### Пример простой таблицы в конфигурационном файле
<!-- запрос Plain -->

```ini
table <table name> {
  type = plain
  path = /path/to/table
  source = <source_name>
  source = <another source_name>
  [stored_fields = <comma separated list of full-text fields that should be stored, all are stored by default, can be empty>]
}
```
<!-- ввод -->
##### Пример таблицы реального времени в конфигурационном файле
<!-- запрос Реальное время -->

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
<!-- конец -->

### Общие параметры простой и реального времени таблиц

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

Путь к месту, где таблица будет храниться или находиться, либо абсолютный, либо относительный, без расширения.

Значение: Путь к таблице, **обязательный**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- пример stored_fields -->

По умолчанию оригинальное содержимое полей полного текста индексируется и хранится, когда таблица определяется в конфигурационном файле. Эта настройка позволяет указать поля, у которых должны храниться оригинальные значения.

Значение: Список полей **полного текста**, которые должны быть сохранены, разделенных запятыми. Пустое значение (т.е. `stored_fields =` ) отключает хранение оригинальных значений для всех полей.

Примечание: В случае таблицы реального времени поля, указанные в `stored_fields`, также должны быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также обратите внимание, что вам не нужно перечислять атрибуты в `stored_fields`, так как их оригинальные значения все равно хранятся. `stored_fields` может использоваться только для полей полного текста.

См. также [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) для параметров сжатия хранения документов.


<!-- ввод -->
##### SQL:

<!-- запрос SQL -->

```sql
CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)
```

<!-- запрос JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)"
```

<!-- запрос PHP -->

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
<!-- ввод -->
##### Python:
<!-- запрос Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- ввод -->
##### Python-asyncio:
<!-- запрос Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)')
```

<!-- ввод -->
##### Javascript:

<!-- запрос Javascript -->
```javascript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- ввод -->
##### Java:
<!-- запрос Java -->
```java
utilsApi.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- ввод -->
##### C#:
<!-- запрос C# -->
```clike
utilsApi.Sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)");
```

<!-- ввод -->
##### Rust:

<!-- запрос Rust -->

```rust
utils_api.sql("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)", Some(true)).await;
```

<!-- ввод -->
##### Typescript:

<!-- запрос Typescript -->
```typescript
res = await utilsApi.sql('CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)');
```

<!-- ввод -->
##### Go:

<!-- запрос Go -->
```go
utilsAPI.Sql(context.Background()).Body("CREATE TABLE products(title text, content text stored indexed, name text indexed, price float)").Execute()
```

<!-- запрос CONFIG -->

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
<!-- конец -->

#### stored_only_fields

```ini
stored_only_fields = title,content
```

Используйте `stored_only_fields`, когда вы хотите, чтобы Manticore хранил некоторые поля простой или реальной таблицы **на диске, но не индексировал их**. Эти поля не будут доступны для поиска с помощью запросов полного текста, но вы все равно сможете получить их значения в результатах поиска.

Например, это полезно, если вы хотите хранить данные, такие как JSON-документы, которые должны возвращаться с каждым результатом, но не нужно, чтобы они искались, фильтровались или группировались. В этом случае хранение их только — и не индексирование — экономит память и улучшает производительность.

Вы можете сделать это двумя способами:
- В [простом режиме](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain-mode%29) в конфигурации таблицы используйте настройку `stored_only_fields`.
- В SQL-интерфейсе ([RT режим](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29)) используйте свойство [stored](../../Creating_a_table/Data_types.md#Storing-binary-data-in-Manticore) при определении текстового поля (вместо `indexed` или `indexed stored`). В SQL вам не нужно включать `stored_only_fields` — это не поддерживается в операторах `CREATE TABLE`.

Значение `stored_only_fields` — это список имен полей, разделенных запятыми. По умолчанию он пуст. Если вы используете таблицу реального времени, каждое поле, указанное в `stored_only_fields`, также должно быть объявлено как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Примечание: вам не нужно перечислять атрибуты в `stored_only_fields`, так как их оригинальные значения все равно хранятся.

Как поля только для хранения сравнятся с атрибутами строк:

- **Поле только для хранения**:
  - Хранится только на диске
  - Сжатый формат
  - Может быть только извлечено (не используется для фильтрации, сортировки и т. д.)

- **Атрибут строки**:
  - Хранится на диске и в памяти
  - Несжатый формат (если вы не используете колонное хранилище)
  - Может быть использовано для сортировки, фильтрации, группировки и т. д.

Если вы хотите, чтобы Manticore хранил текстовые данные, которые вы _только_ хотите хранить на диске (например: json-данные, которые возвращаются с каждым результатом), и не в памяти или доступные для поиска/фильтрации/группировки, используйте `stored_only_fields` или `stored` в качестве свойства вашего текстового поля.

При создании ваших таблиц с использованием SQL-интерфейса обозначьте ваше текстовое поле как `stored` (а не `indexed` или `indexed stored`). Вам не понадобится опция `stored_only_fields` в вашем операторе `CREATE TABLE`; ее включение может привести к ошибке запроса.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- пример json_secondary_indexes -->

По умолчанию вторичные индексы создаются для всех атрибутов, кроме JSON-атрибутов. Однако вторичные индексы для JSON-атрибутов могут быть явно созданы с помощью настройки `json_secondary_indexes`. Когда JSON-атрибут включается в эту опцию, его содержимое упрощается в несколько вторичных индексов. Эти индексы могут использоваться оптимизатором запросов для ускорения запросов.

Вы можете просмотреть доступные вторичные индексы с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: Список JSON-атрибутов, для которых должны быть созданы вторичные индексы, разделенных запятыми.

<!-- ввод -->
##### SQL:

<!-- запрос SQL -->

```sql
CREATE TABLE products(title text, j json secondary_index='1')
```

<!-- запрос JSON -->

```JSON
POST /cli -d "
CREATE TABLE products(title text, j json secondary_index='1')"
```

<!-- запрос PHP -->

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
<!-- ввод -->
##### Python:
<!-- запрос Python -->
```python
utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- ввод -->
##### Python-asyncio:
<!-- запрос Python-asyncio -->
```python
await utilsApi.sql('CREATE TABLE products(title text, j json secondary_index='1')')
```

<!-- ввод -->
##### Javascript:

<!-- запрос Javascript -->
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

Тайм-аут для предотвращения авто-сброса блока RAM, если в таблице нет запросов. Узнать больше [здесь](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

Тайм-аут для авто-сброса блока RAM, если нет записей в него. Узнать больше [здесь](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

Максимальное количество блоков диска для таблицы RT. Узнать больше [здесь](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

Это объявление поля определяет полные текстовые поля, которые будут проиндексированы. Имена полей должны быть уникальными, и порядок сохраняется. При вставке данных значения полей должны соответствовать тому же порядку, как указано в конфигурации.

Это многоценное, необязательное поле.

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

Это объявление определяет атрибут без знака целого числа.

Значение: имя поля или field_name:N (где N — максимальное число бит для хранения).

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

Это объявление определяет атрибут BIGINT.

Значение: имя поля, допускаются множественные записи.

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

Объявляет многоценный атрибут (MVA) с беззнаковыми 32-битными целыми значениями.

Значение: имя поля. Допускаются множественные записи.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Объявляет многоценный атрибут (MVA) сsigned 64-битными значениями BIGINT.

Значение: имя поля. Допускаются множественные записи.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Объявляет атрибуты с плавающей точкой с одинарной точностью, формат IEEE 754 32-бит.

Значение: имя поля. Допускаются множественные записи.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

Объявляет вектор значений с плавающей точкой.

Значение: имя поля. Допускаются множественные записи.

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Объявляет булев атрибут с 1-битными беззнаковыми целыми значениями.

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

Лимит памяти для блока RAM таблицы. Необязательный, по умолчанию 128M.

Таблицы RT хранят некоторые данные в памяти, известные как "RAM chunk", и также поддерживают несколько таблиц на диске, называемых "disk chunks". Эта директива позволяет вам контролировать размер блока RAM. Когда данных слишком много, чтобы хранить в памяти, таблицы RT сбрасывают их на диск, активируют вновь созданный блок диска и сбрасывают блок RAM.

Обратите внимание, что лимит строгий, и таблицы RT никогда не выделяют больше памяти, чем указано в rt_mem_limit. Кроме того, память не предвыделяется, поэтому указание лимита в 512MB и вставка только 3MB данных приведет к выделению только 3MB, а не 512MB.

`rt_mem_limit` никогда не превышается, но фактический размер блока RAM может быть значительно ниже лимита. Таблицы RT адаптируются к темпу вставки данных и динамически регулируют фактический лимит, чтобы минимизировать использование памяти и максимизировать скорость записи данных. Вот как это работает:
* По умолчанию размер блока RAM составляет 50% от `rt_mem_limit`, что называется "`rt_mem_limit`".
* Как только блок RAM накапливает данные, эквивалентные `rt_mem_limit * rate` (по умолчанию 50% от `rt_mem_limit`), Manticore начинает сохранять блок RAM в качестве нового блока диска.
* Пока новый блок диска сохраняется, Manticore оценивает число новых/обновленных документов.
* После сохранения нового блока диска обновляется ставка `rt_mem_limit`.
* Ставка сбрасывается до 50% каждый раз, когда вы перезапускаете searchd.

Например, если 90MB данных сохранено в блоке диска и дополнительно 10MB данных поступает во время процесса сохранения, ставка составит 90%. В следующий раз таблица RT будет собирать до 90% от `rt_mem_limit`, прежде чем сбросить данные. Чем быстрее темп вставки, тем ниже ставка `rt_mem_limit`. Ставка варьируется от 33.3% до 95%. Вы можете просмотреть текущую ставку таблицы, используя команду [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md).

##### Как изменить rt_mem_limit и optimize_cutoff

В реальном времени вы можете настроить лимит размера блоков RAM и максимальное количество блоков диска с помощью оператора `ALTER TABLE`. Чтобы установить `rt_mem_limit` на 1 гигабайт для таблицы "t", выполните следующий запрос: `ALTER TABLE t rt_mem_limit='1G'`. Чтобы изменить максимальное количество блоков диска, выполните запрос: `ALTER TABLE t optimize_cutoff='5'`.

В обычном режиме вы можете изменить значения `rt_mem_limit` и `optimize_cutoff`, обновив конфигурацию таблицы или выполнив команду `ALTER TABLE <table_name> RECONFIGURE`

##### Важные заметки о блоках RAM

* Таблицы в реальном времени подобны [распределенным](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table), состоящим из нескольких локальных таблиц, также известных как блоки диска.
* Каждый блок RAM состоит из нескольких сегментов, которые являются специальными таблицами только для RAM.
* В то время как блоки диска хранятся на диске, блоки RAM хранятся в памяти.
* Каждая транзакция, выполненная в таблице реального времени, генерирует новый сегмент, а сегменты блока RAM объединяются после каждой фиксации транзакции. Эффективнее выполнять массовые INSERT-ы сотен или тысяч документов, чем несколько отдельных INSERT-ов с одним документом, чтобы уменьшить накладные расходы от объединения сегментов блока RAM.
* Когда количество сегментов превышает 32, они будут объединены, чтобы сохранить количество ниже 32.
* Таблицы в реальном времени всегда имеют один участок ОП (который может быть пустым) и один или несколько участков на диске.
* Объединение больших сегментов занимает больше времени, поэтому лучше избегать весьма большого участка ОП (и, следовательно, `rt_mem_limit`).
* Число участков на диске зависит от данных в таблице и настройки `rt_mem_limit`.
* Searchd сбрасывает участок ОП на диск (как сохранённый файл, а не как участок на диске) при завершении работы и периодически в соответствии с настройкой [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайт на диск может занять некоторое время.
* Большой участок ОП создает дополнительную нагрузку на хранилище, как при сбросе на диск в файл `.ram`, так и когда участок ОП полон и сбрасывается на диск как участок на диске.
* Участок ОП резервируется [двойным журналом](../../Logging/Binary_logging.md) до тех пор, пока не будет сброшен на диск, и более высокий `rt_mem_limit` увеличит время, необходимое для воспроизведения двойного журнала и восстановления участка ОП.
* Участок ОП может быть немного медленнее, чем участок на диске.
* Хотя сам участок ОП не занимает больше памяти, чем `rt_mem_limit`, Manticore может занимать больше памяти в некоторых случаях, таких как когда вы начинаете транзакцию для вставки данных и не подтверждаете её некоторое время. В этом случае данные, которые вы уже передали в рамках транзакции, останутся в памяти.

В дополнение к `rt_mem_limit` поведение сброса участков ОП также зависит от следующих настроек: [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) и [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

* `diskchunk_flush_write_timeout`: Эта опция определяет тайм-аут для автоматического сброса участка ОП, если записи в него отсутствуют. Если записи не происходят в течение этого времени, участок будет сброшен на диск. Установка значения `-1` отключает автоматический сброс на основе активности записи. Значение по умолчанию составляет 1 секунду.
* `diskchunk_flush_search_timeout`: Эта опция устанавливает тайм-аут для предотвращения автоматического сброса участка ОП, если в таблице нет поисковых запросов. Автоматический сброс произойдёт только в том случае, если за это время был хотя бы один поисковый запрос. Значение по умолчанию составляет 30 секунд.

Эти тайм-ауты работают совместно. Участок ОП будет сброшен, если будет достигнут *любой* тайм-аут. Это гарантирует, что даже если записей нет, данные в конечном итоге будут сохранены на диск, и, наоборот, даже если записи поступают постоянно, но поисковых запросов нет, данные также будут сохранены. Эти настройки обеспечивают более детальный контроль над частотой сбросов участков ОП, уравновешивая необходимость в долговечности данных с соображениями производительности. Директивы для этих настроек на уровне таблицы имеют более высокий приоритет и перекрывают значения по умолчанию на уровне инстанса.

### Настройки обычной таблицы:

#### источник

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

Поле source указывает источник, из которого документы будут получены во время индексации текущей таблицы. Должен быть как минимум один источник. Источники могут быть разных типов (например, один может быть MySQL, другой PostgreSQL). Для получения дополнительной информации о индексации из внешних хранилищ, [индексация из внешних хранилищ здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Значение: Имя источника является **обязательным**. Разрешены несколько значений.

#### killlist_target

```ini
killlist_target = main:kl
```

Эта настройка определяет таблицы, к которым будет применяться список убийств. Сопоставления в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавлены. В режиме `:kl` документы для подавления берутся из [списка убийств](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой. Если ничего не указано, то оба режима вступят в силу. [Узнать больше о списках убийств здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Разрешены несколько значений

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Эта настройка конфигурации определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо хранения построчно.

Вы можете установить `columnar_attrs = *`, чтобы хранить все поддерживаемые типы данных в колонном хранилище.

Кроме того, `id` является поддерживаемым атрибутом для хранения в колонном хранилище.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

По умолчанию все строковые атрибуты, хранящиеся в колонном хранилище, хранят заранее рассчитанные хэши. Эти хэши используются для группировки и фильтрации. Однако они занимают дополнительное пространство, и если вам не нужно группировать по этому атрибуту, вы можете сэкономить место, отключив генерацию хэша.

### Создание таблицы реального времени онлайн через CREATE TABLE

<!-- example rt_mode -->
##### Общий синтаксис CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Типы данных:

Для получения дополнительной информации о типах данных см. [больше о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: индексируемый, хранимый. По умолчанию: **оба**. Чтобы сохранить текст хранимым, но индексированным, укажите только "хранимый". Чтобы сохранить текст только индексированным, укажите только "индексируемый". | строка |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)
| integer
 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)
| большой целое число
 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | float  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор значений float  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | многоцелое число |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | многоцелое большое число  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | булевый |   |

| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` сделает значение полнотекстовым индексируемым и фильтруемым, сортируемым и группируемым одновременно  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |

[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | временная метка  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N - максимальное количество бит, которое нужно сохранить  |   |
<!-- intro -->

##### Примеры создания таблицы в реальном времени с помощью CREATE TABLE

<!-- request SQL -->
```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```
Это создает таблицу "products" с двумя полями: "title" (полнотекстовый) и "price" (float), и устанавливает "morphology" на "stem_en".
```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```


Это создает таблицу "products" с тремя полями:

* "title" индексируется, но не сохраняется.
* "description" сохраняется, но не индексируется.
* "author" сохраняется и индексируется.
<!-- end -->

## Двигатель

```ini

create table ... engine='columnar';
create table ... engine='rowwise';
```


Настройка двигателя изменяет значение по умолчанию для [хранения атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) для всех атрибутов в таблице. Вы также можете указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).
Для получения информации о том, как включить колонное хранение для обычной таблицы, см. [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs).

Значения:

* columnar - Включает колонное хранение для всех атрибутов таблицы, за исключением [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - Ничего не меняет и использует традиционное построчное хранение для таблицы.

## Другие настройки

Следующие настройки применимы как для таблиц в реальном времени, так и для обычных таблиц, независимо от того, указаны ли они в файле конфигурации или установлены онлайн с помощью команд `CREATE` или `ALTER`.

### Связанные с производительностью

#### Доступ к файлам таблиц
Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.
В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных файлами `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, а размер этих буферов можно настроить с помощью параметров [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также есть параметр [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), который контролирует, как Manticore открывает файлы при запуске.
В режиме доступа mmap сервер поиска отображает файл таблицы в память с помощью системного вызова `mmap`, и операционная система кэширует содержимое файла. Параметры [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не влияют на соответствующие файлы в этом режиме. Читатель mmap также может заблокировать данные таблицы в памяти, используя привилегированный вызов `mlock`, что предотвращает обмен данными, находящимися в кэше, на диск.
Чтобы контролировать, какой режим доступа использовать, доступны параметры **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** с следующими значениями:
| Значение | Описание |


| - | - |
| file | сервер читает файлы таблицы с диска с помощью seek+read, используя внутренние буферы при доступе к файлу |
| mmap | сервер отображает файлы таблиц в память, и ОС кэширует их содержимое при доступе к файлу |
| mmap_preread | сервер отображает файлы таблиц в память, и фоновый поток читает их один раз, чтобы разогреть кэш |
| mlock | сервер отображает файлы таблиц в память, а затем выполняет системный вызов mlock(), чтобы кэшировать содержимое файла и заблокировать его в памяти, чтобы предотвратить его выгрузку |
| Настройка | Значения | Описание |
| - | - | - |

| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | контролирует, как будут читаться `*.spa` (обычные атрибуты) `*.spe` (пропуски) `*.spt` (поисковые запросы) `*.spm` (убиенные документы) |

| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | контролирует, как будут читаться `*.spb` (блоб-атрибуты) (строковые, mva и json атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | контролирует, как будут читаться данные `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | контролирует, как будут читаться данные `*.spp` (списки попаданий) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | контролирует, как будет читаться `*.spi` (словарь) |
Вот таблица, которая может помочь вам выбрать желаемый режим:
| часть таблицы |
сохранять на диске |
держать в памяти |

кэширование в памяти при старте сервера | заблокировать в памяти |

| - | - | - | - | - |
| простые атрибуты в [построчном](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоночном) хранилище, пропускные списки, списки слов, поиски, удаленные документы | 
mmap | mmap |
**mmap_preread** (по умолчанию) | mlock |
| построчные строковые, многозначные атрибуты (MVA) и json атрибуты | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

| [колоночные](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и многозначные атрибуты | всегда  | только с помощью ОС  | нет  | не поддерживается |
| списки документов | **файл** (по умолчанию) | mmap | нет
| mlock |
| списки попаданий | **файл** (по умолчанию) | mmap | нет
| mlock |

| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

##### Рекомендации:

* Для **самого быстрого времени ответа на запрос** и достаточной доступности памяти используйте [построчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и блокируйте их в памяти с помощью `mlock`. Кроме того, используйте mlock для списков документов/списков попаданий.

* Если вы придаете приоритет **не можете позволить себе более низкую производительность после запуска** и готовы пожертвовать более длительным временем запуска, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если вы хотите более быстрое перезапуск searchd, придерживайтесь стандартной опции  `mmap_preread`.
* Если вы хотите **сэкономить память**, при этом имея достаточно памяти для всех атрибутов, пропустите использование `mlock`. Операционная система определит, что должно быть сохранено в памяти, исходя из частых чтений с диска.
* Если построчные атрибуты **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)

* Если производительность **полнотекстового поиска не важна**, и вы хотите сэкономить память, используйте `access_doclists/access_hitlists=file`

Режим по умолчанию предлагает баланс:

* mmap,

* Предварительное чтение неколоночных атрибутов,
* Поиск и чтение колоночных атрибутов без предварительного чтения,
* Поиск и чтение списков документов/списков попаданий без предварительного чтения.

Это обеспечивает достойную производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве сценариев.

### Другие настройки, связанные с производительностью

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Эта настройка резервирует дополнительное пространство для обновлений блоб-атрибутов, таких как многозначные атрибуты (MVA), строки и JSON. Значение по умолчанию — 128k. При обновлении этих атрибутов их длина может изменяться. Если обновленная строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если обновленная строка длиннее, она будет записана в конец файла `*.spb`. Этот файл имеет отображение в память, что делает изменение его размера потенциально медленным процессом, в зависимости от реализации файлов с отображением в память вашей операционной системой. Чтобы избежать частых изменений размера, вы можете использовать эту настройку для резервирования дополнительного пространства в конце файла .spb.

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Эта настройка управляет размером блоков, используемых хранилищем документов. Значение по умолчанию — 16kb. Когда оригинальный текст документа сохраняется с помощью stored_fields или stored_only_fields, он хранится в таблице и сжимается для повышения эффективности. Чтобы оптимизировать доступ к диску и коэффициенты сжатия для небольших документов, эти документы конкатенируются в блоки. Процесс индексирования собирает документы до достижения их общего размера порога, указанного этой опцией. В этот момент блок документов сжимается. Эта опция может быть отрегулирована для достижения лучших коэффициентов сжатия (путем увеличения размера блока) или более быстрого доступа к тексту документа (путем уменьшения размера блока).

Значение: размер, по умолчанию **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Эта настройка определяет тип сжатия, используемого для сжатия блоков документов, хранящихся в хранилище документов. Если указаны stored_fields или stored_only_fields, хранилище документов хранит сжатые блоки документов. 'lz4' предлагает быстрые скорости сжатия и распаковки, в то время как 'lz4hc' (высокое сжатие) жертвует некоторой скоростью сжатия для лучшего коэффициента сжатия. 'none' полностью отключает сжатие.

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Уровень сжатия, используемый при применении сжатия 'lz4hc' в хранилище документов. Настройка уровня сжатия позволяет найти правильный баланс между производительностью и коэффициентом сжатия при использовании 'lz4hc' сжатия. Обратите внимание, что эта опция не применяется при использовании 'lz4' сжатия.

Значение: Целое число от 1 до 12, по умолчанию **9**.

#### preopen

```ini
preopen = 1
```

Эта настройка указывает, что searchd должен открывать все файлы таблицы при старте или ротации и удерживать их открытыми во время работы. По умолчанию файлы не открываются заранее. Предварительно открытые таблицы требуют несколько дескрипторов файлов на таблицу, но устраняют необходимость в вызовах open() для каждого запроса и защищены от условий гонки, которые могут возникать во время ротации таблицы при высокой нагрузке. Однако, если вы обслуживаете много таблиц, может быть более эффективно открывать их на основе каждого запроса, чтобы сохранить дескрипторы файлов.

Значение: **0** (по умолчанию) или 1.

#### read_buffer_docs

```ini

read_buffer_docs = 1M

```
Размер буфера для хранения списка документов по ключевому слову. Увеличение этого значения приведет к большему использованию памяти во время выполнения запроса, но может сократить время I/O.
Значение: размер, по умолчанию **256k**, минимальное значение — 8k.

#### read_buffer_hits

```ini

read_buffer_hits = 1M

```


Размер буфера для хранения списка попаданий по ключевому слову. Увеличение этого значения приведет к большему использованию памяти во время выполнения запроса, но может сократить время I/O.
Значение: размер, по умолчанию **256k**, минимальное значение — 8k.

### Настройки объемов диска для простых таблиц

#### inplace_enable
<!-- пример inplace_enable -->
```ini

inplace_enable = {0|1}
```
Включает инверсию таблицы на месте. Необязательно, по умолчанию 0 (использует отдельные временные файлы).
Опция `inplace_enable` уменьшает объем дискового пространства во время индексации простых таблиц, хотя и немного замедляет процесс индексации (она использует примерно в 2 раза меньше диска, но производит около 90-95% от оригинальной производительности).
Индексация состоит из двух основных фаз. На первой фазе документы собираются, обрабатываются и частично сортируются по ключевым словам, а промежуточные результаты записываются во временные файлы (.tmp*). На второй фазе документы полностью сортируются, и создаются окончательные файлы таблиц. Восстановление производственной таблицы в реальном времени требует примерно в 3 раза больше пикового размера дискового пространства: сначала для промежуточных временных файлов, затем для вновь созданной копии и третьим для старой таблицы, которая будет обслуживать производственные запросы в это время. (Промежуточные данные сопоставимы по размеру с окончательной таблицей.) Это может быть слишком большим объемом дискового пространства для больших коллекций данных, и опция `inplace_enable` может быть использована для его уменьшения. Когда она включена, она повторно использует временные файлы, выводит окончательные данные обратно в них и переименовывает их по завершении. Однако это может потребовать дополнительной переменной переориентации временных данных, что и вызывает влияние на производительность.

Эта директива не имеет эффекта на [searchd](../../Starting_the_server/Manually.md), она влияет только на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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
Опция [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) для тонкой настройки. Управляет предопределенным размером промежутка hitlist. Необязательная, по умолчанию 0.
Эта директива влияет только на инструмент [searchd](../../Starting_the_server/Manually.md) и не имеет никакого влияния на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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
Настройка inplace_reloc_factor определяет размер буфера перемещения в памяти, используемого во время индексации. Значение по умолчанию равно 0.1.
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
Управляет размером буфера, используемого для записи на месте во время индексации. Необязательная, со значением по умолчанию 0.1.
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
