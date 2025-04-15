# Настройки обычной и реального времени таблиц

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

#### тип

```ini
type = plain

type = rt
```

Тип таблицы: "plain" или "rt" (реального времени)

Значение: **plain** (по умолчанию), rt

#### путь

```ini
path = path/to/table
```

Путь к тому месту, где таблица будет храниться или находиться, либо абсолютный, либо относительный, без расширения.

Значение: Путь к таблице, **обязателен**

#### stored_fields

```ini
stored_fields = title, content
```

<!-- example stored_fields -->

По умолчанию оригинальное содержимое полей полного текста индексируется и сохраняется, когда таблица определяется в конфигурационном файле. Эта настройка позволяет указать поля, значения которых должны храниться в оригинальном виде.

Значение: Список полей **полного текста** через запятую, которые должны быть сохранены. Пустое значение (т.е. `stored_fields =` ) отключает хранение оригинальных значений для всех полей.

Примечание: В случае таблицы реального времени поля, указанные в `stored_fields`, также должны быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также обратите внимание, что вам не нужно перечислять атрибуты в `stored_fields`, так как их оригинальные значения в любом случае хранятся. `stored_fields` можно использовать только для полей полного текста.

См. также [docstore_block_size](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_block_size), [docstore_compression](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#docstore_compression) для параметров сжатия хранения документов.


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
  stored_fields = title, content # мы хотим сохранить только "title" и "content", "name" не должен храниться

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

Список полей, которые будут сохранены в таблице, но не индексированы. Эта настройка работает аналогично [stored_fields](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields), за исключением того, что когда поле указано в `stored_only_fields`, оно будет только сохраняться, не индексироваться, и не может быть найдено с помощью полнотекстовых запросов. Оно может быть получено только в результатах поиска.

Значение — это список полей, разделённых запятыми, которые должны быть сохранены только, не индексированы. По умолчанию это значение пустое. Если определяется таблица в реальном времени, поля, перечисленные в `stored_only_fields`, также должны быть объявлены как [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field).

Также обратите внимание, что вам не нужно перечислять атрибуты в `stored_only_fields`, так как их оригинальные значения всё равно сохраняются. Если сравнивать `stored_only_fields` с символьными атрибутами, первые (сохранённые поля):
* хранятся на диске и не занимают памяти
* хранятся в сжатом формате
* могут быть только извлечены, их нельзя использовать для сортировки, фильтрации или группировки

Напротив, последние (символьный атрибут):
* хранятся на диске и в памяти
* хранятся в несжатом формате
* могут использоваться для сортировки, группировки, фильтрации и любых других действий, которые вы хотите выполнить с атрибутами.

#### json_secondary_indexes

```ini
json_secondary_indexes = json_attr
```

<!-- example json_secondary_indexes -->

По умолчанию вторичные индексы создаются для всех атрибутов, за исключением JSON-атрибутов. Однако вторичные индексы для JSON-атрибутов могут быть явно созданы с помощью настройки `json_secondary_indexes`. Когда JSON-атрибут включён в эту опцию, его содержимое упрощается в несколько вторичных индексов. Эти индексы могут быть использованы оптимизатором запросов для ускорения запросов.

Вы можете просмотреть доступные вторичные индексы с помощью команды [SHOW TABLE <tbl> INDEXES](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_INDEXES.md).

Значение: список JSON-атрибутов, для которых должны быть созданы вторичные индексы, разделённых запятыми.

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

Таймаут для предотвращения автоматической выгрузки фрагмента ОЗУ, если в таблице нет поисков. Узнайте больше [здесь](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

#### diskchunk_flush_write_timeout

```ini
diskchunk_flush_write_timeout = 60s
```

Таймаут для автоматической выгрузки фрагмента ОЗУ, если в него нет записей. Узнайте больше [здесь](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout).

#### optimize_cutoff

Максимальное количество дисковых фрагментов для таблицы RT. Узнайте больше [здесь](../../Securing_and_compacting_a_table/Compacting_a_table.md#Number-of-optimized-disk-chunks).

#### rt_field

```ini
rt_field = subject
```

Это объявление поля определяет полнотекстовые поля, которые будут индексированы. Имена полей должны быть уникальными, и порядок сохраняется. При вставке данных значения полей должны быть в том же порядке, что и указано в конфигурации.

Это поле может содержать несколько значений и является необязательным.

#### rt_attr_uint

```ini
rt_attr_uint = gid
```

Это объявление определяет атрибут без знака целого числа.

Значение: имя поля или field_name:N (где N — максимальное количество бит, которое нужно сохранить).

#### rt_attr_bigint

```ini
rt_attr_bigint = gid
```

Это объявление определяет атрибут BIGINT.

Значение: имя поля, допускаются несколько записей.

#### rt_attr_multi

```ini
rt_attr_multi = tags
```

Объявляет многозначный атрибут (MVA) с unsigned 32-битными целыми значениями.

Значение: имя поля. Допускаются несколько записей.

#### rt_attr_multi_64

```ini
rt_attr_multi_64 = wide_tags
```

Объявляет многоценностный атрибут (MVA) со знаком 64-битных значений BIGINT.

Значение: имя поля. Разрешено несколько записей.

#### rt_attr_float

```ini
rt_attr_float = lat
rt_attr_float = lon
```

Объявляет атрибуты с плавающей точкой с одинарной точностью, формат IEEE 754 32-бит.

Значение: имя поля. Разрешено несколько записей.

#### rt_attr_float_vector

```ini
rt_attr_float_vector = image_vector
```

Объявляет вектор значений с плавающей точкой.

Значение: имя поля. Разрешено несколько записей.

#### rt_attr_bool

```ini
rt_attr_bool = available
```

Объявляет логический атрибут со значениями беззнакового целого числа 1-бит.

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

Ограничение памяти для RAM-чанка таблицы. Необязательно, по умолчанию 128M.

RT-таблицы хранят некоторые данные в памяти, известные как "RAM-чанк", и также поддерживают ряд таблиц на диске, называемых "дисковыми чанками". Эта директива позволяет контролировать размер RAM-чанка. Когда данных слишком много, чтобы сохранить их в памяти, RT-таблицы будут сбрасывать их на диск, активировать вновь созданный дисковый чанк и сбрасывать RAM-чанк.

Обратите внимание, что лимит является строгим, и RT-таблицы никогда не выделят больше памяти, чем указано в rt_mem_limit. Кроме того, память не предвыделяется, поэтому указание предела в 512MB и вставка только 3MB данных приведут к выделению только 3MB, а не 512MB.

Предел `rt_mem_limit` никогда не превышается, но фактический размер RAM-чанка может быть значительно ниже предела. RT-таблицы адаптируются к скорости вставки данных и динамически корректируют фактический лимит, чтобы минимизировать использование памяти и максимизировать скорость записи данных. Вот как это работает:
* По умолчанию размер RAM-чанка составляет 50% от `rt_mem_limit`, называемого "`rt_mem_limit`".
* Как только RAM-чанк накапливает данные, эквивалентные `rt_mem_limit * rate` данных (по умолчанию 50% от `rt_mem_limit`), Manticore начинает сохранять RAM-чанк как новый дисковый чанк.
* Пока новый дисковый чанк сохраняется, Manticore оценивает количество новых/обновленных документов.
* После сохранения нового дискового чанка обновляется ставка `rt_mem_limit`.
* Ставка сбрасывается до 50% каждый раз, когда вы перезапускаете searchd.

Например, если 90MB данных сохранены в дисковом чанке и поступает еще 10MB данных во время сохранения, ставка составит 90%. В следующий раз RT-таблица соберет до 90% от `rt_mem_limit` перед сбросом данных. Чем быстрее скорость вставки, тем ниже ставка `rt_mem_limit`. Ставка варьируется от 33.3% до 95%. Вы можете просмотреть текущую ставку таблицы, используя команду [SHOW TABLE <tbl> STATUS](../../Node_info_and_management/Table_settings_and_status/SHOW_TABLE_STATUS.md).

##### Как изменить rt_mem_limit и optimize_cutoff

В реальном времени вы можете настроить размер лимита RAM-чанков и максимальное количество дисковых чанков, используя оператор `ALTER TABLE`. Чтобы установить `rt_mem_limit` на 1 гигабайт для таблицы "t", выполните следующий запрос: `ALTER TABLE t rt_mem_limit='1G'`. Чтобы изменить максимальное количество дисковых чанков, выполните запрос: `ALTER TABLE t optimize_cutoff='5'`.

В обычном режиме вы можете изменить значения `rt_mem_limit` и `optimize_cutoff`, обновив конфигурацию таблицы или запустив команду `ALTER TABLE <table_name> RECONFIGURE`

##### Важные заметки о RAM-чанках

* Реальные таблицы времени похожи на [распределенные](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table), состоящие из нескольких локальных таблиц, также известных как дисковые чанки.
* Каждый RAM-чанк состоит из нескольких сегментов, которые являются специальными таблицами только для RAM.
* Пока дисковые чанки хранятся на диске, RAM-чанки хранятся в памяти.
* Каждая транзакция, выполненная для таблицы реального времени, генерирует новый сегмент, и сегменты RAM-чанка объединяются после каждого подтверждения транзакции. Эффективнее выполнять массовую вставку сотен или тысяч документов, чем несколько отдельных вставок с одним документом, чтобы уменьшить накладные расходы на объединение сегментов RAM-чанка.
* Когда количество сегментов превышает 32, они будут объединены, чтобы сохранить счетчик ниже 32.
* Таблицы реального времени всегда имеют один RAM-чанк (который может быть пустым) и один или несколько дисковых чанков.
* Объединение больших сегментов занимает больше времени, поэтому лучше избегать очень большого RAM-чанка (и, следовательно, `rt_mem_limit`).
* Количество дисковых чанков зависит от данных в таблице и настройки `rt_mem_limit`.
* Searchd сбрасывает RAM-чанк на диск (как постоянный файл, а не как дисковый чанк) при завершении работы и периодически в соответствии с настройкой [rt_flush_period](../../Server_settings/Searchd.md#rt_flush_period). Сброс нескольких гигабайт на диск может занять некоторое время.
* Большой RAM-чанк оказывает большее давление на хранилище, как при сбросе на диск в файл `.ram`, так и когда RAM-чанк заполнен и сбрасывается на диск как дисковый чанк.
* RAM-чанк поддерживается [двойным журналом](../../Logging/Binary_logging.md) до его сброса на диск, и установка большего `rt_mem_limit` увеличит время, необходимое для воспроизведения двоичного лога и восстановления RAM-чанка.
* RAM-чанк может быть немного медленнее, чем дисковый чанк.
* Хотя сам RAM-чанк не занимает больше памяти, чем `rt_mem_limit`, Manticore может занимать больше памяти в некоторых случаях, например, когда вы начинаете транзакцию для вставки данных и не подтверждаете ее в течение некоторого времени. В этом случае данные, которые вы уже передали в транзакции, останутся в памяти.

В дополнение к `rt_mem_limit`, поведение сбрасывания чанков RAM также зависит от следующих настроек: [diskchunk_flush_write_timeout](../../Server_settings/Searchd.md#diskchunk_flush_write_timeout) и [diskchunk_flush_search_timeout](../../Server_settings/Searchd.md#diskchunk_flush_search_timeout).

* `diskchunk_flush_write_timeout`: Эта опция определяет таймаут для автоматического сбрасывания чанка RAM, если нет записей в него. Если не происходит запись в течение этого времени, чанк будет сброшен на диск. Установка значения `-1` отключает автоматическое сбрасывание на основе активности записи. Значение по умолчанию — 1 секунда.
* `diskchunk_flush_search_timeout`: Эта опция устанавливает таймаут для предотвращения автоматического сбрасывания чанка RAM, если в таблице нет поисковых запросов. Автоматическое сбрасывание произойдет только в том случае, если за это время был как минимум один поисковый запрос. Значение по умолчанию — 30 секунд.

Эти таймауты работают вместе. Чанк RAM будет сброшен, если *хотя бы* один из таймаутов будет достигнут. Это обеспечивает, что даже если нет записей, данные в конечном итоге будут сохранены на диск, и наоборот, даже если есть постоянные записи, но нет поисков, данные также будут сохранены. Эти настройки предоставляют более точный контроль над тем, как часто сбрасываются чанки RAM, балансируя потребность в долговечности данных с соображениями производительности. Директивы для этих настроек на уровне таблицы имеют более высокий приоритет и переопределят значения по умолчанию для всего экземпляра.

### Настройки обычной таблицы:

#### source

```ini
source = srcpart1
source = srcpart2
source = srcpart3
```

Поле source указывает источник, из которого документы будут получены во время индексации текущей таблицы. Должен быть как минимум один источник. Источники могут быть разных типов (например, один может быть MySQL, другой — PostgreSQL). Для получения дополнительной информации об индексировании из внешних источников, [индексирование из внешних источников здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md)

Значение: Имя источника является **обязательным**. Допускаются несколько значений.

#### killlist_target

```ini
killlist_target = main:kl
```

Эта настройка определяет таблицы, к которым будет применяться список игнорируемых. Совпадения в целевой таблице, которые обновляются или удаляются в текущей таблице, будут подавлены. В режиме `:kl` документы, которые нужно подавить, берутся из [списка игнорируемых](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md). В режиме `:id` все идентификаторы документов из текущей таблицы подавляются в целевой таблице. Если ни один из них не указан, оба режима будут активны. [Узнайте больше о списках игнорируемых здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md)

Значение: **не указано** (по умолчанию), target_table_name:kl, target_table_name:id, target_table_name. Допускаются несколько значений.

#### columnar_attrs

```ini
columnar_attrs = *
columnar_attrs = id, attr1, attr2, attr3
```

Эта настройка конфигурации определяет, какие атрибуты должны храниться в [колоночном хранилище](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) вместо построчного хранилища.

Вы можете установить `columnar_attrs = *`, чтобы сохранить все поддерживаемые типы данных в колонном хранилище.

Кроме того, `id` является поддерживаемым атрибутом для хранения в колонном хранилище.

#### columnar_strings_no_hash

```ini
columnar_strings_no_hash = attr1, attr2, attr3
```

По умолчанию все строковые атрибуты, хранящиеся в колонном хранилище, сохраняют предварительно рассчитанные хэш-значения. Эти хэши используются для группировки и фильтрации. Однако они занимают дополнительное пространство, и если нет необходимости группировать по этому атрибуту, вы можете сэкономить место, отключив генерацию хэша.

### Создание таблицы реального времени онлайн через CREATE TABLE

<!-- пример rt_mode -->
##### Общий синтаксис CREATE TABLE

```sql
CREATE TABLE [IF NOT EXISTS] name ( <field name> <field data type> [data type options] [, ...]) [table_options]
```

##### Типы данных:

Для получения дополнительной информации о типах данных см. [больше о типах данных здесь](../../Creating_a_table/Data_types.md).

| Тип | Эквивалент в конфигурационном файле | Примечания | Псевдонимы |
| - | - | - | - |
| [text](../../Creating_a_table/Data_types.md#Text) | [rt_field](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_field)  | Опции: индексируется, сохраняется. По умолчанию: **оба**. Чтобы сохранить текст в хранилище, но индексировать, укажите только "сохраненный". Чтобы сохранить текст только в индексе, укажите только "индексированный". | строка |
| [integer](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint)	| целое число	 | int, uint |
| [bigint](../../Creating_a_table/Data_types.md#Big-Integer) | [rt_attr_bigint](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bigint)	| большое целое число	 |   |
| [float](../../Creating_a_table/Data_types.md#Float) | [rt_attr_float](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float)   | число с плавающей запятой  |   |
| [float_vector](../../Creating_a_table/Data_types.md#Float-vector) | [rt_attr_float_vector](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_float_vector) | вектор значений с плавающей запятой  |   |
| [multi](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) | [rt_attr_multi](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi)   | многоцелое число |   |
| [multi64](../../Creating_a_table/Data_types.md#Multi-value-big-integer) | [rt_attr_multi_64](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_multi_64) | много-большое целое число  |   |
| [bool](../../Creating_a_table/Data_types.md#Boolean) | [rt_attr_bool](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_bool) | булевый тип |   |
| [json](../../Creating_a_table/Data_types.md#JSON) | [rt_attr_json](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_json) | JSON |   |
| [string](../../Creating_a_table/Data_types.md#String) | [rt_attr_string](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_string) | строка. Опция `indexed, attribute` сделает значение полнотекстовым индексируемым и фильтруемым, сортируемым и группируемым одновременно  |   |
| [timestamp](../../Creating_a_table/Data_types.md#Timestamps) |	[rt_attr_timestamp](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_timestamp) | временная метка  |   |
| [bit(n)](../../Creating_a_table/Data_types.md#Integer) | [rt_attr_uint field_name:N](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#rt_attr_uint) | N - максимальное количество бит, которые нужно сохранить  |   |

<!-- intro -->
##### Примеры создания таблицы реального времени с помощью CREATE TABLE
<!-- request SQL -->

```sql
CREATE TABLE products (title text, price float) morphology='stem_en'
```

Это создаёт таблицу "products" с двумя полями: "title" (полнотекстовый) и "price" (число с плавающей точкой), и устанавливает "morphology" в "stem_en".

```sql
CREATE TABLE products (title text indexed, description text stored, author text, price float)
```
Это создаёт таблицу "products" с тремя полями:
* "title" индексируется, но не хранится.
* "description" хранится, но не индексируется.
* "author" как хранится, так и индексируется.
<!-- end -->


## Двигатель

```ini
create table ... engine='columnar';
create table ... engine='rowwise';
```

Настройка двигателя изменяет параметры [хранения атрибутов](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) по умолчанию для всех атрибутов в таблице. Вы также можете указать `engine` [отдельно для каждого атрибута](../../Creating_a_table/Data_types.md#How-to-switch-between-the-storages).

Для информации о том, как включить колоннарное хранение для обычной таблицы, смотрите [columnar_attrs](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#columnar_attrs) .

Значения:
* columnar - Включает колоннарное хранение для всех атрибутов таблицы, кроме [json](../../Creating_a_table/Data_types.md#JSON)
* **rowwise (по умолчанию)** - Ничего не меняет и использует традиционное построчное хранение для таблицы.


## Другие настройки
Следующие настройки применимы как для таблиц реального времени, так и для обычных таблиц, независимо от того, указаны ли они в конфигурационном файле или установлены онлайн с помощью команды `CREATE` или `ALTER`.

### Связанные с производительностью

#### Доступ к файлам таблицы
Manticore поддерживает два режима доступа для чтения данных таблицы: seek+read и mmap.

В режиме seek+read сервер использует системный вызов `pread` для чтения списков документов и позиций ключевых слов, представленных в файлах `*.spd` и `*.spp`. Сервер использует внутренние буферы чтения для оптимизации процесса чтения, и размер этих буферов можно регулировать с помощью опций [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits). Также есть опция [preopen](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#preopen), которая управляет тем, как Manticore открывает файлы при запуске.

В режиме доступа mmap сервер поиска отображает файл таблицы в память, используя системный вызов `mmap`, и ОС кеширует содержимое файла. Опции [read_buffer_docs](../../Server_settings/Searchd.md#read_buffer_docs) и [read_buffer_hits](../../Server_settings/Searchd.md#read_buffer_hits) не влияют на соответствующие файлы в этом режиме. Читатель mmap также может заблокировать данные таблицы в памяти, используя привилегированный вызов `mlock`, что предотвращает вытеснение кешированных данных на диск ОС.

Чтобы контролировать, какой режим доступа использовать, доступны опции **access_plain_attrs**, **access_blob_attrs**, **access_doclists**, **access_hitlists** и **access_dict** с следующими значениями:

| Значение | Описание |
| - | - |
| file | сервер читает файлы таблицы с диска с помощью seek+read, используя внутренние буферы при доступе к файлам |
| mmap | сервер отображает файлы таблицы в память, и ОС кеширует их содержимое при доступе к файлам |
| mmap_preread | сервер отображает файлы таблицы в память, и фоновый поток читает его один раз, чтобы разогреть кеш |
| mlock | сервер отображает файлы таблицы в память, а затем выполняет системный вызов mlock() для кеширования содержимого файла и блокировки его в памяти, чтобы предотвратить вытеснение |


| Настройка | Значения | Описание |
| - | - | - |
| access_plain_attrs  | mmap, **mmap_preread** (по умолчанию), mlock | управляет тем, как будут читаться `*.spa` (обычные атрибуты) `*.spe` (списки пропусков) `*.spt` (поиски) `*.spm` (убитые документы) |
| access_blob_attrs   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будут читаться `*.spb` (атрибуты blob) (строковые, многозначные и json атрибуты) |
| access_doclists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spd` (списки документов) |
| access_hitlists   | **file** (по умолчанию), mmap, mlock  | управляет тем, как будут читаться данные `*.spp` (списки попаданий) |
| access_dict   | mmap, **mmap_preread** (по умолчанию), mlock  | управляет тем, как будут читаться `*.spi` (словарь) |

Вот таблица, которая может помочь вам выбрать желаемый режим:

| часть таблицы |	сохранять на диске |	сохранять в памяти |	кэшировать в памяти при запуске сервера | заблокировать в памяти |
| - | - | - | - | - |
| обычные атрибуты в [построчном](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) (не колоннарном) хранении, списки пропусков, списки слов, поиски, убитые документы | 	mmap | mmap |	**mmap_preread** (по умолчанию) | mlock |
| построчные строковые, многозначные атрибуты (MVA) и json атрибуты | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |
| [columnar](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) числовые, строковые и многозначные атрибуты | всегда  | только с помощью ОС  | нет  | не поддерживается |
| списки документов | **файл** (по умолчанию) | mmap | нет	| mlock |
| списки попаданий | **файл** (по умолчанию) | mmap | нет	| mlock |
| словарь | mmap | mmap | **mmap_preread** (по умолчанию) | mlock |

##### Рекомендуется:

* Для **самого быстрого времени отклика поиска** и достаточной доступности памяти используйте [строчные](../../Creating_a_table/Data_types.md#JSON) атрибуты и заблокируйте их в памяти с помощью `mlock`. Кроме того, используйте mlock для списков документов/списков попаданий.
* Если вы придаете приоритет **не можете позволить себе снижение производительности после запуска** и готовы пожертвовать более длительным временем запуска, используйте опцию [--force-preread](../../Starting_the_server/Manually.md#searchd-command-line-options). Если вы хотите более быстрое перезапуск searchd, придерживайтесь параметра по умолчанию `mmap_preread`.
* Если вы хотите **сэкономить память**, при этом имея достаточно памяти для всех атрибутов, пропустите использование `mlock`. Операционная система определит, что должно храниться в памяти на основе частых чтений с диска.
* Если строчные атрибуты  **не помещаются в память**, выберите [колоночные атрибуты](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)
* Если производительность **полнотекстового поиска не имеет значения**, и вы хотите сэкономить память, используйте `access_doclists/access_hitlists=file`

Режим по умолчанию предлагает баланс:
* mmap,
* Предварительное чтение неколоночных атрибутов,
* Поиск и чтение колоночных атрибутов без предварительного чтения,
* Поиск и чтение списков документов/списков попаданий без предварительного чтения.

Это обеспечивает приемлемую производительность поиска, оптимальное использование памяти и более быстрый перезапуск searchd в большинстве сценариев.

### Другие параметры, связанные с производительностью

#### attr_update_reserve

```ini
attr_update_reserve = 256k
```

Этот параметр резервирует дополнительное пространство для обновлений блобов атрибутов, таких как многозначные атрибуты (MVA), строки и JSON. Значение по умолчанию равно 128k. При обновлении этих атрибутов их длина может измениться. Если обновленная строка короче предыдущей, она перезапишет старые данные в файле `*.spb`. Если обновленная строка длиннее, она будет записана в конец файла `*.spb`. Этот файл имеет память, что делает изменение его размера потенциально медленным процессом, в зависимости от реализации памяти в файловой системе операционной системы. Чтобы избежать частых изменений размера, вы можете использовать этот параметр для резервирования дополнительного пространства в конце файла .spb.

Значение: размер, по умолчанию **128k**.

#### docstore_block_size

```ini
docstore_block_size = 32k
```

Этот параметр контролирует размер блоков, используемых хранилищем документов. Значение по умолчанию равно 16kb. Когда оригинальный текст документа хранится с использованием stored_fields или stored_only_fields, он хранится в таблице и сжимается для повышения эффективности. Для оптимизации доступа к диску и коэффициентов сжатия для небольших документов эти документы объединяются в блоки. Процесс индексации собирает документы, пока их общий размер не достигнет порога, указанного этой опцией. В этот момент блок документов сжимается. Эта опция может быть отрегулирована для достижения лучших коэффициентов сжатия (увеличением размера блока) или более быстрого доступа к тексту документа (уменьшением размера блока).

Значение: размер, по умолчанию **16k**.

#### docstore_compression

```ini
docstore_compression = lz4hc
```

Этот параметр определяет тип сжатия, используемого для сжатия блоков документов, хранящихся в хранилище документов. Если указаны stored_fields или stored_only_fields, хранилище документов хранит сжатые блоки документов. 'lz4' предлагает быстрые скорости сжатия и декомпрессии, в то время как 'lz4hc' (высокое сжатие) жертвует некоторой скоростью сжатия ради лучшего коэффициента сжатия. 'none' полностью отключает сжатие.

Значения: **lz4** (по умолчанию), lz4hc, none.

#### docstore_compression_level

```ini
docstore_compression_level = 12
```

Уровень сжатия, используемый при применении сжатия 'lz4hc' в хранилище документов. Регулируя уровень сжатия, вы можете найти правильный баланс между производительностью и коэффициентом сжатия при использовании сжатия 'lz4hc'. Обратите внимание, что этот параметр не применим при использовании сжатия 'lz4'.

Значение: Целое число от 1 до 12, со значением по умолчанию **9**.

#### preopen

```ini
preopen = 1
```

Этот параметр указывает, что searchd должен открывать все файловые таблицы при запуске или ротации и держать их открытыми во время работы. По умолчанию файлы не открываются заранее. Предварительно открытые таблицы требуют несколько дескрипторов файлов на таблицу, но исключают необходимость в вызовах open() для каждого запроса и являются невосприимчивыми к условиям гонки, которые могут возникнуть во время ротации таблиц при высокой нагрузке. Однако, если вы обслуживаете много таблиц, может оказаться более эффективным открывать их на основе каждого запроса, чтобы сохранить дескрипторы файлов.

Значение: **0** (по умолчанию) или 1.

#### read_buffer_docs

```ini
read_buffer_docs = 1M
```

Размер буфера для хранения списка документов на ключевое слово. Увеличение этого значения приведет к большему использованию памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: размер, по умолчанию **256k**, минимальное значение 8k.

#### read_buffer_hits

```ini
read_buffer_hits = 1M
```

Размер буфера для хранения списка попаданий на ключевое слово. Увеличение этого значения приведет к большему использованию памяти во время выполнения запроса, но может сократить время ввода-вывода.

Значение: размер, по умолчанию **256k**, минимальное значение 8k.

### Параметры дискового пространства для обычной таблицы

#### inplace_enable

<!-- пример inplace_enable -->

```ini
inplace_enable = {0|1}
```

Включает инверсию таблицы на месте. Необязательно, по умолчанию 0 (используются отдельные временные файлы).
Опция `inplace_enable` уменьшает объем занимаемого диска во время индексирования простых таблиц, хотя и немного замедляет процесс индексирования (она использует примерно в 2 раза меньше места на диске, но показывает около 90-95% от оригинальной производительности).

Индексирование состоит из двух основных фаз. На первой фазе документы собираются, обрабатываются и частично сортируются по ключевому слову, а промежуточные результаты записываются во временные файлы (.tmp*). На второй фазе документы полностью сортируются и создаются финальные таблицы файлов. Восстановление производственной таблицы на лету требует примерно в 3 раза больше пиковой нагрузки на диск: сначала для промежуточных временных файлов, затем для новой созданной копии и, наконец, для старой таблицы, которая будет обрабатывать производственные запросы в это время. (Промежуточные данные сопоставимы по объему с финальной таблицей.) Это может быть слишком много нагрузки на диск для больших коллекций данных, и можно использовать опцию `inplace_enable`, чтобы уменьшить её. Когда она включена, временные файлы повторно используются, окончательные данные возвращаются в них, и они переименовываются по завершении. Однако это может потребовать дополнительного перемещения временных данных, что и является источником воздействия на производительность.

Эта директива не влияет на [searchd](../../Starting_the_server/Manually.md), она затрагивает только [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).


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

Опция [In-place inversion](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#inplace_enable) - это настраиваемая опция. Управляет размером заранее выделенного зазора в списке попаданий. Необязательный, по умолчанию равно 0.

Эта директива касается только инструмента [searchd](../../Starting_the_server/Manually.md) и не имеет никакого влияния на [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool).

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

Настройка inplace_reloc_factor определяет размер буфера перемещения в пределах используемой области памяти во время индексирования. Значение по умолчанию равно 0.1.

Эта опция является необязательной и касается только инструмента [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool), а не сервера [searchd](../../Starting_the_server/Manually.md).

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

Управляет размером буфера, используемого для записи на месте во время индексирования. Необязательный, с значением по умолчанию 0.1.

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

### Настройки, специфичные для обработки естественного языка
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

