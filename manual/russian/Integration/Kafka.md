# Синхронизация из Kafka

> ПРИМЕЧАНИЕ: эта функция требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

Manticore Search может беспрепятственно получать сообщения из брокера Kafka, позволяя индексировать и искать данные в реальном времени.

Чтобы начать, необходимо:
1. **Определить источник:** Укажите тему Kafka, из которой Manticore Search будет читать сообщения. В настройках указываются такие детали как хост и порт брокера, а также имя темы.
2. **Настроить таблицу назначения:** Выберите таблицу Manticore real-time для хранения поступающих данных из Kafka.
3. **Создать материализованный просмотр:** Настройте материализованный просмотр (`mv`) для обработки трансформаций и сопоставления данных из Kafka в таблицу назначения Manticore Search. Здесь вы определяете сопоставления полей, преобразования данных и любые фильтры или условия для входящего потока данных.

## Источник

<!-- example kafka_source -->

Конфигурация `source` позволяет определить `broker`, `topic list`, `consumer group` и структуру сообщений.

#### Схема

Определите схему с использованием типов полей Manticore, таких как `int`, `float`, `text`, `json` и др.

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

Все ключи схемы нечувствительны к регистру, то есть `Products`, `products` и `PrOdUcTs` считаются одинаковыми. Все они преобразуются в нижний регистр.

Если имена ваших полей не соответствуют [синтаксису имен полей](../Creating_a_table/Data_types.md#Field-name-syntax), разрешённому в Manticore Search (например, содержат специальные символы или начинаются с цифр), необходимо определить соответствие схемы. Например, `$keyName` или `123field` — допустимые ключи в JSON, но недопустимые имена полей в Manticore Search. Если попытаться использовать недопустимые имена полей без правильного сопоставления, Manticore выдаст ошибку, и создание источника завершится неудачей.

Для таких случаев используйте следующий синтаксис схемы для сопоставления недопустимых имён полей с допустимыми:

```
allowed_field_name 'original JSON key name with special symbols' type
```

Например:
```sql
price_field '$price' float    -- maps JSON key '$price' to field 'price_field'
field_123 '123field' text     -- maps JSON key '123field' to field 'field_123'
```
<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
CREATE SOURCE kafka
(id bigint, term text, abbrev '$abbrev' text, GlossDef json)
type='kafka'
broker_list='kafka:9092'
topic_list='my-data'
consumer_group='manticore'
num_consumers='2'
batch=50
```

<!-- response -->

```
Query OK, 2 rows affected (0.02 sec)
```

<!-- end -->

#### Опции

| Опция | Допустимые значения | Описание                                                                                                           |
|-|----------------|---------------------------------------------------------------------------------------------------------------------|
| `type` | `kafka`        | Устанавливает тип источника. В настоящий момент поддерживается только `kafka`                                        |
| `broker_list` | `host:port [, ...]` | Указывает URL адреса брокеров Kafka                                                                                  |
| `topic_list` | `string [, ...]` | Перечисляет темы Kafka для потребления                                                                                |
| `consumer_group`| `string`       | Определяет группу потребителей Kafka, по умолчанию `manticore`.                                                      |
| `num_consumers` | `int`          | Количество потребителей для обработки сообщений.                                                                     |
| `partition_list` | `int [, ...]`  | Список партиций для чтения [подробнее](../Integration/Kafka.md#Sharding-with-Kafka).                                  |
| `batch` | `int`          | Количество сообщений для обработки перед переходом к следующим. По умолчанию `100`; оставшиеся сообщения обрабатываются при таймауте |

### Таблица назначения

<!-- example kafka_destination -->

Таблица назначения — это обычная таблица real-time, куда сохраняются результаты обработки сообщений Kafka. Эта таблица должна быть определена с учетом требований схемы входящих данных и оптимизирована под потребности вашего приложения в быстром выполнении запросов. Подробнее о создании real-time таблиц читайте [здесь](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:).

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE destination_kafka
(id bigint, name text, short_name text, received_at text, size multi);
```

<!-- response -->

```
Query OK, 0 rows affected (0.02 sec)
```

<!-- end -->

### Материализованный просмотр

<!-- example kafka_mv -->

Материализованный просмотр позволяет преобразовывать данные из сообщений Kafka. Можно переименовывать поля, применять функции Manticore Search, выполнять сортировку, группировку и другие операции с данными.

Материализованный просмотр действует как запрос, который переносит данные из источника Kafka в таблицу назначения, позволяя использовать синтаксис Manticore Search для настройки этих запросов. Убедитесь, что поля в `select` соответствуют полям источника.

```
CREATE MATERIALIZED VIEW <materialized view name>
TO <destination table name> AS
SELECT [column|function [as <new name>], ...] FROM <source name>
```

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
CREATE MATERIALIZED VIEW view_table
TO destination_kafka AS
SELECT id, term as name, abbrev as short_name,
       UTC_TIMESTAMP() as received_at, GlossDef.size as size FROM kafka

```

<!-- response -->

```sql
Query OK, 2 rows affected (0.02 sec)
```

<!-- end -->

Данные передаются из Kafka в Manticore Search партиями, которые очищаются после каждой операции. Для вычислений, охватывающих несколько партий, например AVG, будьте осторожны, поскольку такие вычисления могут не работать должным образом из-за обработки пакет за пакетом.


### Сопоставление полей

Вот таблица сопоставлений на основе приведённых выше примеров:

| Kafka           | Источник | Буфер    | MV                                 | Назначение  |
|-----------------|----------|----------|-----------------------------------|-------------|
| `id`              | `id`       | `id`       | `id`                                | `id`          |
| `term`            | `term`     | `term`     | `term as name`                      | `name`        |
| `unnecessary_key`, который нам не интересен | -        | -        |                                   |             |
| `$abbrev`         | `abbrev`   | `abbrev`   | `abbrev` as `short_name`              | `short_name`  |
| -                 | -        | -         | `UTC_TIMESTAMP() as received_at`  | `received_at` |
| `GlossDef`        | `glossdef` | `glossdef` | `glossdef.size as size`             | `size`        |

### Просмотр списка

<!-- example kafka_listing -->

Чтобы просмотреть источники и материализованные просмотры в Manticore Search, используйте следующие команды:
- `SHOW SOURCES`: Выводит список всех настроенных источников.
- `SHOW MVS`: Выводит список всех материализованных просмотров.
- `SHOW MV view_table`: Показывает подробную информацию о конкретном материализованном просмотре.

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
SHOW SOURCES
```

<!-- response -->

```
+-------+
| name  |
+-------+
| kafka |
+-------+
```

<!-- end -->

<!-- example kafka_create_source -->

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
SHOW SOURCE kafka;
```

<!-- response -->

```
+--------+-------------------------------------------------------------------+
| Source | Create Table                                                      |
+--------+-------------------------------------------------------------------+
| kafka  | CREATE SOURCE kafka                                               |
|        | (id bigint, term text, abbrev '$abbrev' text, GlossDef json)      |
|        | type='kafka'                                                      |
|        | broker_list='kafka:9092'                                          |
|        | topic_list='my-data'                                              |
|        | consumer_group='manticore'                                        |
|        | num_consumers='2'                                                 |
|        | batch=50                                                          |
+--------+-------------------------------------------------------------------+
```

<!-- end -->

<!-- example kafka_view -->

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
SHOW MVS
```

<!-- response -->

```
+------------+
| name       |
+------------+
| view_table |
+------------+
```

<!-- end -->

<!-- example kafka_show -->

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
SHOW MV view_table
```

<!-- response -->

```
+------------+--------------------------------------------------------------------------------------------------------+-----------+
| View       | Create Table                                                                                           | suspended |
+------------+--------------------------------------------------------------------------------------------------------+-----------+
| view_table | CREATE MATERIALIZED VIEW view_table TO destination_kafka AS                                            | 0         |
|            | SELECT id, term as name, abbrev as short_name, UTC_TIMESTAMP() as received_at, GlossDef.size as size   |           |
|            | FROM kafka                                                                                             |           |
+------------+--------------------------------------------------------------------------------------------------------+-----------+
```

<!-- end -->

### Изменение материализованных представлений

<!-- example mv_suspend -->

Вы можете приостановить потребление данных, изменяя материализованные представления.

Если вы удалите `source`, не удаляя MV, оно автоматически приостанавливается. После воссоздания источника отмените приостановку MV вручную с помощью команды `ALTER`.

В настоящее время можно изменять только материализованные представления. Чтобы изменить параметры `source`, необходимо удалить и заново создать источник.

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
ALTER MATERIALIZED VIEW view_table suspended=1
```

<!-- response -->

```sql
Query OK (0.02 sec)
```

<!-- end -->

### Шардинг с Kafka

Вы также можете указать `partition_list` для каждой темы Kafka.
Одним из основных преимуществ такого подхода является возможность реализовать `шардинг` вашей таблицы через Kafka.
Для этого следует создать отдельную цепочку `source` → `materialized view` → `destination table` для каждого шарда:

**Источники:**
```sql
CREATE SOURCE kafka_p1 (id bigint, term text)
  type='kafka' broker_list='kafka:9092' topic_list='my-data'
  consumer_group='manticore' num_consumers='1' partition_list='0' batch=50;

CREATE SOURCE kafka_p2 (id bigint, term text)
  type='kafka' broker_list='kafka:9092' topic_list='my-data'
  consumer_group='manticore' num_consumers='1' partition_list='1' batch=50;
```

**Таблицы назначения:**

```sql
CREATE TABLE destination_shard_1 (id bigint, name text);
CREATE TABLE destination_shard_2 (id bigint, name text);
```

**Материализованные представления:**

```sql
CREATE MATERIALIZED VIEW mv_1 TO destination_shard_1 AS SELECT id, term AS name FROM kafka_p1;
CREATE MATERIALIZED VIEW mv_2 TO destination_shard_2 AS SELECT id, term AS name FROM kafka_p2;
```

#### ⚠️ Важные замечания:

* В этой настройке ребалансировку необходимо выполнять вручную.
* Kafka по умолчанию не распределяет сообщения используя стратегию round-robin.
* Чтобы добиться распределения, похожего на round-robin, при отправке данных, убедитесь, что ваш Kafka-продюсер настроен с параметрами:
  * `parse.key=true`
  * `key.separator={your_delimiter}`

Иначе Kafka будет распределять сообщения согласно своим внутренним правилам, что может привести к неравномерному разделению по партициям.

### Устранение неполадок

#### Повторяющиеся записи

Коммиты смещений Kafka происходят после каждой партии или при превышении времени обработки. Если процесс неожиданно остановится во время запроса к материализованному представлению, вы можете увидеть повторяющиеся записи. Чтобы избежать этого, включите поле `id` в вашу схему, что позволит Manticore Search предотвращать дублирование записей в таблице.

### Как это работает внутри

- **Инициализация рабочего процесса:** После настройки источника и материализованного представления, Manticore Search запускает выделенного рабочего для обработки загрузки данных из Kafka.
- **Отображение сообщений:** Сообщения сопоставляются согласно схеме конфигурации источника, преобразуясь в структурированный формат.
- **Группировка:** Сообщения объединяются в партии для эффективной обработки. Размер партии можно настраивать в зависимости от ваших требований по производительности и задержкам.
- **Буферизация:** Партии сопоставленных данных сохраняются в буферной таблице для эффективных операций массовой обработки.
- **Обработка материализованного представления:** Логика представления применяется к данным в буферной таблице, выполняя необходимые преобразования или фильтрацию.
- **Передача данных:** Обработанные данные передаются в итоговую таблицу реального времени.
- **Очистка:** Буферная таблица очищается после каждой партии, чтобы подготовиться к следующему набору данных.

<!-- proofread -->

