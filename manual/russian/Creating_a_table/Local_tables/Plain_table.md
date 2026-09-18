# Plain table

**Plain table** — это базовый элемент для поиска без [percolate](../../Creating_a_table/Local_tables/Percolate_table.md). Он может быть определён только в конфигурационном файле с использованием [Plain mode](../../Creating_a_table/Local_tables.md#Defining-table-schema-in-config-%28Plain mode%29) и не поддерживается в [RT mode](../../Creating_a_table/Local_tables.md#Online-schema-management-%28RT-mode%29). Обычно используется совместно с [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) для обработки данных [из внешнего хранилища](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md) и может позднее быть [присоединён](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Attaching_one_table_to_another.md) к **real-time table**.

#### Создание plain table

Чтобы создать plain table, необходимо определить его в конфигурационном файле. Команда `CREATE TABLE` его не поддерживает.

<!-- example plain -->

Ниже приведён пример конфигурации plain table и source для получения данных из базы данных MySQL:

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
  * Строить из внешнего хранилища с использованием [source](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#source) и [indexer](../../Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md#Indexer-tool)
  * Выполнять обновление на месте [целочисленных, плавающих, строковых атрибутов и MVA](../../Creating_a_table/Data_types.md)
  * [Обновлять](../../Data_creation_and_modification/Adding_data_from_external_storages/Adding_data_to_tables/Killlist_in_plain_tables.md#killlist_target) `killlist_target`

### ⛔ Что нельзя делать с plain table:
  * Вставлять дополнительные данные после построения таблицы
  * Удалять данные из таблицы
  * Создавать, удалять или изменять схему таблицы онлайн
  * При создании таблицы используйте [автогенерацию ID](../../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Auto-ID); строки из внешнего хранилища должны содержать явные, уникальные, ненулевые беззнаковые 64-битные ID документов

Числовые атрибуты, включая [MVA](../../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29), — единственные элементы, которые могут обновляться в plain table. Все остальные данные в таблице неизменяемы. Если требуются обновления или новые записи, таблицу нужно перестроить. В процессе перестройки существующая таблица остаётся доступной для обслуживания запросов, а когда новая версия готова, выполняется процесс [ротации](../../Data_creation_and_modification/Adding_data_from_external_storages/Rotating_a_table.md), переводящий её в онлайн и отбрасывающий старую версию.

#### Производительность построения plain table
Скорость индексации plain table зависит от нескольких факторов, включая:
* Скорость получения данных из источника
* [Настройки токенизации](../../Creating_a_table/NLP_and_tokenization/Data_tokenization.md)
* Аппаратные характеристики (например, CPU, RAM и производительность диска)

#### Сценарии построения plain table
##### Полная перестройка по мере необходимости
Для небольших наборов данных самый простой вариант — использовать одну plain table, полностью перестраиваемую по мере необходимости. Такой подход приемлем, когда:
* Данные в таблице менее свежие, чем данные в источнике
* Время построения таблицы увеличивается с ростом объёма данных

##### Сценарий main+delta
Для больших наборов данных можно использовать plain table вместо [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md). Сценарий main+delta включает:
* Создание меньшей таблицы для инкрементальной индексации
* Объединение двух таблиц с помощью [распределённой таблицы](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md)

Этот подход позволяет реже перестраивать большую таблицу и чаще обрабатывать обновления из источника. Меньшую таблицу можно перестраивать чаще (например, каждую минуту или даже каждые несколько секунд).

Однако со временем продолжительность индексации меньшей таблицы станет слишком большой, что потребует перестройки большой таблицы и очистки меньшей.

Схема main+delta подробно описана в [этом интерактивном курсе](https://play.manticoresearch.com/maindelta/).

Механизм **kill list** и директива [killlist_target](../../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#killlist_target) используются для обеспечения приоритета документов текущей таблицы над документами другой таблицы.

Более подробно на эту тему можно почитать [здесь](../../Data_creation_and_modification/Adding_data_from_external_storages/Main_delta.md).

#### Структура файлов plain table
В следующей таблице приведены различные расширения файлов, используемых в plain table, и их описания:

| Расширение | Описание |
| - | - |
|`.spa` | хранит атрибуты документа в [построчном режиме](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages) |
|`.spb` | хранит блоб-атрибуты в [построчном режиме](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages): строки, MVA, json |
|`.spc` | хранит атрибуты документа в [колоночном режиме](../../Creating_a_table/Data_types.md#Row-wise-and-columnar-attribute-storages)  |
|`.spd` | хранит списки совпадающих ID документов для каждого ID слова |
|`.sph` | хранит информацию заголовка таблицы |
|`.sphi` | хранит гистограммы значений атрибутов |
|`.spi` | хранит списки слов (ID слов и указатели на файл `.spd`) |
|`.spidx` | хранит данные вторичных индексов |
|`.spjidx` | хранит данные вторичных индексов, сгенерированных для JSON-атрибутов |
|`.spk` | хранит kill-списки |
|`.spl` | файл блокировки |
|`.spm` | хранит битовую карту удалённых документов |
|`.spp` | хранит списки хитов (также известные как posting, или появления слова) для каждого ID слова |
|`.spt` | хранит дополнительные структуры данных для ускорения поиска по ID документов |
|`.spe` | хранит skip-листы для ускорения фильтрации списков документов |
|`.spds` | хранит тексты документов |
|`.tmp*` |временные файлы во время index_settings_and_status |
|`.new.sp*` | новая версия простой таблицы до ротации |
|`.old.sp*` | старая версия простой таблицы после ротации |

<!-- proofread -->

