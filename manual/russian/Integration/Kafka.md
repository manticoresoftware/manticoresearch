# Синхронизация из Kafka

> ПРИМЕЧАНИЕ: эта функциональность требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

Manticore поддерживает интеграцию с [Apache Kafka](https://kafka.apache.org/) для потокового приема данных в реальном времени через источники Kafka и материализованные представления, что позволяет индексировать и искать данные в реальном времени. В настоящее время поддерживаются и протестированы версии **apache/kafka 3.7.0-4.1.0**.

Чтобы начать, необходимо:
1. **Определить источник:** Указать тему Kafka, из которой Manticore Search будет читать сообщения. Эта настройка включает детали, такие как хост брокера, порт и имя темы.
2. **Настроить таблицу назначения:** Выбрать таблицу реального времени Manticore для хранения входящих данных из Kafka.
3. **Создать материализованное представление:** Настроить материализованное представление (`mv`) для обработки преобразования данных и сопоставления из Kafka в таблицу назначения Manticore Search. Здесь вы определите сопоставления полей, преобразования данных и любые фильтры или условия для входящего потока данных.

## Источник

<!-- example kafka_source -->

Конфигурация `source` позволяет определить `broker`, `topic list`, `consumer group` и структуру сообщений.

#### Схема

Определите схему, используя типы полей Manticore, такие как `int`, `float`, `text`, `json` и т.д.

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

Все ключи схемы нечувствительны к регистру, то есть `Products`, `products` и `PrOdUcTs` считаются одинаковыми. Все они преобразуются в нижний регистр.

Если имена ваших полей не соответствуют [синтаксису имен полей](../Creating_a_table/Data_types.md#Field-name-syntax), разрешенному в Manticore Search (например, содержат специальные символы или начинаются с цифр), необходимо определить сопоставление схемы. Например, `$keyName` или `123field` являются допустимыми ключами в JSON, но недопустимыми именами полей в Manticore Search. Если попытаться использовать недопустимые имена полей без правильного сопоставления, Manticore вернет ошибку, и создание источника не удастся.

Для таких случаев используйте следующий синтаксис схемы для сопоставления недопустимых имен полей с допустимыми:

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

| Опция | Допустимые значения | Описание                                                                                                         |
|-|----------------|---------------------------------------------------------------------------------------------------------------------|
| `type` | `kafka`        | Устанавливает тип источника. В настоящее время поддерживается только `kafka`                                       |
| `broker_list` | `host:port [, ...]` | Указывает URL брокеров Kafka                                                                                         |
| `topic_list` | `string [, ...]` | Список тем Kafka для потребления                                                                                  |
| `consumer_group`| `string`       | Определяет группу потребителей Kafka, по умолчанию `manticore`.                                                        |
| `num_consumers` | `int`          | Количество потребителей для обработки сообщений.                                                                             |
| `partition_list` | `int [, ...]`  | Список партиций для чтения [подробнее](../Integration/Kafka.md#Sharding-with-Kafka).                                 |
| `batch` | `int`          | Количество сообщений для обработки перед переходом к следующей партии. По умолчанию `100`; в противном случае обрабатывает оставшиеся сообщения по таймауту |

### Таблица назначения

<!-- example kafka_destination -->

Таблица назначения — это обычная таблица реального времени, в которой хранятся результаты обработки сообщений Kafka. Эта таблица должна быть определена в соответствии с требованиями схемы входящих данных и оптимизирована для производительности запросов вашего приложения. Подробнее о создании таблиц реального времени читайте [здесь](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:).

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

### Материализованное представление

<!-- example kafka_mv -->

Материализованное представление позволяет преобразовывать данные из сообщений Kafka. Вы можете переименовывать поля, применять функции Manticore Search, выполнять сортировку, группировку и другие операции с данными.

Материализованное представление действует как запрос, который перемещает данные из источника Kafka в таблицу назначения, позволяя использовать синтаксис Manticore Search для настройки этих запросов. Убедитесь, что поля в `select` соответствуют полям источника.

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

Данные передаются из Kafka в Manticore Search партиями, которые очищаются после каждого запуска. Для вычислений по партиям, таких как AVG, будьте осторожны, так как они могут работать не так, как ожидается, из-за обработки по партиям.


### Сопоставление полей

Вот таблица сопоставления на основе приведенных выше примеров:

| Kafka           | Источник   | Буфер   | MV                                | Назначение |
|-----------------|----------|----------|-----------------------------------|-------------|
| `id`              | `id`       | `id`       | `id`                                | `id`          |
| `term`            | `term`     | `term`     | `term as name`                      | `name`        |
| `unnecessary_key`, который нас не интересует | -        | -        |                                   |             |
| `$abbrev`         | `abbrev`   | `abbrev`   | `abbrev` as `short_name`              | `short_name`  |
| -                 | -        | -         | `UTC_TIMESTAMP() as received_at`  | `received_at` |
| `GlossDef`        | `glossdef` | `glossdef` | `glossdef.size as size`             | `size`        |

### Просмотр

<!-- example kafka_listing -->

Для просмотра источников и материализованных представлений в Manticore Search используйте следующие команды:
- `SHOW SOURCES`: Показывает все настроенные источники.
- `SHOW MVS`: Показывает все материализованные представления.
- `SHOW MV view_table`: Показывает подробную информацию о конкретном материализованном представлении.

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

Если вы удалите `source`, не удаляя MV, оно автоматически приостанавливается. После воссоздания источника снимите приостановку MV вручную с помощью команды `ALTER`.

В настоящее время можно изменять только материализованные представления. Чтобы изменить параметры `source`, удалите и создайте источник заново.

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
Одним из основных преимуществ такого подхода является возможность реализовать `sharding` для вашей таблицы через Kafka.
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

* В этой конфигурации ребалансировка должна управляться вручную.
* По умолчанию Kafka не распределяет сообщения по принципу round-robin.
* Чтобы добиться распределения, похожего на round-robin при отправке данных, убедитесь, что ваш Kafka producer настроен с:
  * `parse.key=true`
  * `key.separator={your_delimiter}`

В противном случае Kafka будет распределять сообщения согласно своим внутренним правилам, что может привести к неравномерному распределению по партициям.

### Устранение неполадок

#### Дублирование записей

Коммиты смещений Kafka происходят после каждой партии или при истечении времени обработки. Если процесс неожиданно остановится во время запроса к материализованному представлению, вы можете увидеть дублирующиеся записи. Чтобы этого избежать, включите поле `id` в вашу схему, что позволит Manticore Search предотвращать дублирование в таблице.

### Как это работает внутри

- **Инициализация воркера:** После настройки источника и материализованного представления Manticore Search запускает выделенного воркера для обработки данных из Kafka.
- **Отображение сообщений:** Сообщения сопоставляются согласно схеме конфигурации источника, преобразуясь в структурированный формат.
- **Группировка:** Сообщения группируются в партии для эффективной обработки. Размер партии можно настроить под ваши требования по производительности и задержке.
- **Буферизация:** Партии отображенных данных сохраняются в буферной таблице для эффективных пакетных операций.
- **Обработка материализованного представления:** Логика представления применяется к данным в буферной таблице, выполняя необходимые преобразования или фильтрацию.
- **Передача данных:** Обработанные данные затем передаются в целевую таблицу реального времени.
- **Очистка:** Буферная таблица очищается после каждой партии, чтобы быть готовой к следующему набору данных.

<!-- proofread -->

