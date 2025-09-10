# Plain table

**Plain table** — это базовый элемент для поиска без [percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Он может быть определён только в конфигурационном файле с использованием [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) и не поддерживается в [RT mode](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29). Обычно используется в сочетании с [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) для обработки данных [из внешнего хранилища](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) и может позже быть [прикреплена](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) к **real-time table**.

#### Создание plain table

Чтобы создать plain table, нужно определить её в конфигурационном файле. Команда `CREATE TABLE` её не поддерживает.

<!-- example plain -->

Пример конфигурации plain table и source для получения данных из базы данных MySQL:

<!-- intro -->
#### Как создать plain table

<!-- request Plain table example -->

```ini
source source {
  type             = mysql
  sql_host         = localhost
  sql_user         = myuser
  sql_pass         = mypass
  sql_db           = mydb
  sql_query        = SELECT id, title, description, category_id  from mytable
  sql_attr_uint    = category_id
  sql_field_string = title
 }

table tbl {
  type   = plain
  source = source
  path   = /path/to/table
 }
```
<!-- end -->

### 👍 Что можно делать с plain table:
  * Создавать из внешнего хранилища с помощью [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) и [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)
  * Выполнять обновления на месте [атрибутов типа integer, float, string и MVA](../../Creating_a_table/Data_types.md)
  * [Обновлять](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) его `killlist_target`

### ⛔ Что нельзя делать с plain table:
  * Вставлять дополнительные данные после постройки таблицы
  * Удалять данные из таблицы
  * Создавать, удалять или изменять схему таблицы онлайн
  * Использовать [UUID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID) для автоматической генерации ID (данные из внешнего хранилища должны содержать уникальный идентификатор)

Числовые атрибуты, включая [MVA](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29), — единственные элементы, которые можно обновлять в plain table. Все остальные данные в таблице неизменны. Если нужны обновления или новые записи — таблицу необходимо перестраивать. Во время перестройки исходная таблица остаётся доступной для запросов, а когда новая версия готова, выполняется процесс [rotation](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md), который запускает новую версию и удаляет старую.

#### Производительность построения plain table
Скорость индексации plain table зависит от нескольких факторов, включая:
* Скорость получения данных из источника
* [Настройки токенизации](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* Аппаратные характеристики (CPU, ОЗУ, производительность диска)

#### Сценарии построения plain table
##### Полная перестройка по необходимости
Для небольших наборов данных самым простым вариантом является одна plain table, полностью перестраиваемая при необходимости. Этот подход подходит, если:
* Данные в таблице не такие свежие, как данные в источнике
* Время построения таблицы растёт с увеличением набора данных

##### Сценарий Main+delta
Для больших наборов данных plain table может заменить [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md). Сценарий main+delta включает:
* Создание меньшей таблицы для инкрементальной индексации
* Объединение двух таблиц с помощью [distributed table](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)

Этот подход позволяет реже перестраивать большую таблицу и чаще обрабатывать обновления из источника. Меньшую таблицу можно перестраивать чаще (например, каждую минуту или даже несколько секунд).

Однако со временем время индексации меньшей таблицы становится слишком большим, что требует перестроить большую таблицу и очистить меньшую.

Схема main+delta подробно описана в [интерактивном курсе](https://play.manticoresearch.com/maindelta/).

Для обеспечения приоритета документов из текущей таблицы над другой используется механизм **kill list** и директива [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target).

Для дополнительной информации смотрите [здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md).

#### Структура файлов plain table
В таблице ниже перечислены различные расширения файлов plain table и их описание:

| Extension | Description |
| - | - |
|`.spa` | хранит атрибуты документов в [строчном формате](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) |
|`.spb` | хранит blob-атрибуты в [строчном формате](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages): строки, MVA, json |
|`.spc` | хранит атрибуты документов в [колоночном формате](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) |
|`.spd` | хранит списки совпадающих идентификаторов документов для каждого ID слова |
|`.sph` | хранит информацию о заголовке таблицы |
|`.sphi` | хранит гистограммы значений атрибутов |
|`.spi` | хранит списки слов (ID слов и указатели на `.spd` файл) |
|`.spidx` | хранит данные вторичных индексов |
|`.spjidx` | хранит данные вторичных индексов, созданных для JSON-атрибутов |
|`.spk` | хранит kill-листы |
|`.spl` | lock-файл |
|`.spm` | хранит битовую карту удалённых документов |
|`.spp` | хранит списки вхождений (posting, вхождения слова) для каждого ID слова |
|`.spt` | хранит дополнительные структуры данных для ускорения поиска по ID документов |
|`.spe` | хранит skip-листы для ускорения фильтрации списков документов |
|`.spds` | хранит тексты документов |
|`.tmp*` | временные файлы во время index_settings_and_status |
|`.new.sp*` | новая версия простой таблицы перед ротацией |
|`.old.sp*` | старая версия простой таблицы после ротации |

<!-- proofread -->

