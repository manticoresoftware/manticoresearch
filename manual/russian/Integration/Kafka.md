# Синхронизация из Kafka

> ПРИМЕЧАНИЕ: эта функциональность требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

Manticore поддерживает интеграцию с [Apache Kafka](https://kafka.apache.org/) для потокового сбора данных через Kafka источники и материализованные представления, позволяя индексировать и искать данные в реальном времени. В настоящее время тестируются и поддерживаются версии **apache/kafka 3.7.0-4.1.0**.

Чтобы начать, вам нужно:
1. **Определить источник:** Указать Kafka-топик, из которого Manticore Search будет считывать сообщения. В настройках указываются такие параметры, как хост брокера, порт и имя топика.
2. **Настроить целевую таблицу:** Выбрать таблицу типа real-time Manticore для хранения данных, поступающих из Kafka.
3. **Создать материализованное представление:** Настроить материализованное представление (`mv`) для обработки трансформации данных и сопоставления из Kafka в целевую таблицу Manticore Search. Здесь вы определяете поля, преобразования данных и любые фильтры или условия для входящего потока данных.

## Источник

<!-- example kafka_source -->

Конфигурация `source` позволяет определить `broker`, `topic list`, `consumer group` и структуру сообщений.

#### Схема

Определите схему, используя типы полей Manticore, такие как `int`, `float`, `text`, `json` и т.д.

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

Все ключи схемы нечувствительны к регистру, то есть `Products`, `products` и `PrOdUcTs` считаются одинаковыми. Они все преобразуются в нижний регистр.

Если имена полей не соответствуют [синтаксису имён полей](../Creating_a_table/Data_types.md#Field-name-syntax), допустимому в Manticore Search (например, содержат специальные символы или начинаются с цифр), необходимо определить маппинг схемы. Например, `$keyName` или `123field` являются допустимыми ключами в JSON, но недопустимыми именами полей в Manticore Search. Если попытаться использовать недопустимые имена без корректного маппинга, Manticore выдаст ошибку, и создание источника не удастся.

Для таких случаев используйте следующий синтаксис схемы для сопоставления недопустимых имён с валидными:

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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "CREATE SOURCE kafka (id bigint, term text, abbrev '$abbrev' text, GlossDef json) type='kafka' broker_list='kafka:9092' topic_list='my-data' consumer_group='manticore' num_consumers='2' batch=50"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

#### Опции

| Опция | Допустимые значения | Описание                                                                                                            |
|-|----------------|---------------------------------------------------------------------------------------------------------------------|
| `type` | `kafka`        | Устанавливает тип источника. В настоящее время поддерживается только `kafka`.                                        |
| `broker_list` | `host:port [, ...]` | Указывает URL брокеров Kafka                                                                                            |
| `topic_list` | `string [, ...]` | Список топиков Kafka для чтения                                                                                        |
| `consumer_group`| `string`       | Определяет группу потребителей Kafka, по умолчанию `manticore`.                                                      |
| `num_consumers` | `int`          | Количество потребителей для обработки сообщений.                                                                      |
| `partition_list` | `int [, ...]`  | Список разделов для чтения [подробнее](../Integration/Kafka.md#Sharding-with-Kafka).                                  |
| `batch` | `int`          | Количество сообщений для обработки перед переходом дальше. По умолчанию `100`; по таймауту обрабатывает оставшиеся сообщения. |

### Целевая таблица

<!-- example kafka_destination -->

Целевая таблица — обычная таблица real-time, в которую записываются результаты обработки сообщений из Kafka. Эта таблица должна быть определена в соответствии с требованиями схемы поступающих данных и оптимизирована под нужды производительности запросов вашего приложения. Подробнее о создании real-time таблиц читайте [здесь](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:).

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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "CREATE TABLE destination_kafka (id bigint, name text, short_name text, received_at text, size multi)"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 0,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

### Материализованное представление

<!-- example kafka_mv -->

Материализованное представление позволяет трансформировать данные из сообщений Kafka. Вы можете переименовывать поля, применять функции Manticore Search, выполнять сортировку, группировку и другие операции с данными.

Материализованное представление выступает в роли запроса, который перемещает данные из Kafka-источника в целевую таблицу, позволяя использовать синтаксис Manticore Search для настройки таких запросов. Убедитесь, что поля в `select` соответствуют полям источника.

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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "CREATE MATERIALIZED VIEW view_table TO destination_kafka AS SELECT id, term as name, abbrev as short_name, UTC_TIMESTAMP() as received_at, GlossDef.size as size FROM kafka"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

Данные передаются из Kafka в Manticore Search пакетами, которые очищаются после каждого запуска. Для вычислений на основе нескольких пакетов, например AVG, будьте осторожны, так как такие операции могут некорректно работать из-за обработки по пакетам.


### Сопоставление полей

Вот таблица сопоставлений на основе вышеуказанных примеров:

| Kafka                                 | Источник | Буфер    | MV                                      | Целевой пункт |
|-------------------------------------|----------|----------|----------------------------------------|---------------|
| `id`                                | `id`     | `id`     | `id`                                   | `id`          |
| `term`                              | `term`   | `term`   | `term as name`                         | `name`        |
| `unnecessary_key`, который нас не интересует | -        | -        |                                        |               |
| `$abbrev`                           | `abbrev` | `abbrev` | `abbrev as short_name`                 | `short_name`  |
| -                                  | -        | -        | `UTC_TIMESTAMP() as received_at`       | `received_at` |
| `GlossDef`        | `glossdef` | `glossdef` | `glossdef.size as size`             | `size`        |

### Список

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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW SOURCES"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 1,
    "error": "",
    "warning": "",
    "columns": [
      {
        "name": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "name": "kafka"
      }
    ]
  }
]
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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW SOURCE kafka"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 1,
    "error": "",
    "warning": "",
    "columns": [
      {
        "Source": {
          "type": "string"
        }
      },
      {
        "Create Table": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "Source": "kafka",
        "Create Table": "CREATE SOURCE kafka \n(id bigint, term text, abbrev '' text, GlossDef json)\ntype='kafka'\nbroker_list='kafka:9092'\ntopic_list='my-data'\nconsumer_group='manticore'\nnum_consumers='2'\n batch=50"
      }
    ]
  }
]
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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "SHOW MVS"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 1,
    "error": "",
    "warning": "",
    "columns": [
      {
        "name": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "name": "view_table"
      }
    ]
  }
]
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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```sql
POST /sql?mode=raw -d "SHOW MV view_table"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 1,
    "error": "",
    "warning": "",
    "columns": [
      {
        "View": {
          "type": "string"
        }
      },
      {
        "Create Table": {
          "type": "string"
        }
      },
      {
        "suspended": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "View": "view_table",
        "Create Table": "CREATE MATERIALIZED VIEW view_table TO destination_kafka AS SELECT id, term as name, abbrev as short_name, UTC_TIMESTAMP() as received_at, GlossDef.size as size FROM kafka",
        "suspended": 0
      }
    ]
  }
]
```

<!-- end -->

### Изменение материализованных представлений

<!-- example mv_suspend -->

Вы можете приостановить потребление данных, изменив материализованные представления.

Если удалить `source`, не удаляя MV, оно автоматически приостанавливается. После воссоздания источника отмените приостановку MV вручную с помощью команды `ALTER`.

В настоящее время изменять можно только материализованные представления. Чтобы изменить параметры `source`, удалите и создайте источник заново.

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

<!-- intro -->

##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "ALTER MATERIALIZED VIEW view_table suspended=1"
```

<!-- response JSON -->

```JSON
[
  {
    "total": 2,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

### Шардинг с Kafka

Вы также можете указать `partition_list` для каждой темы Kafka.
Одним из основных преимуществ такого подхода является возможность реализовать `sharding` для вашей таблицы через Kafka.
Для этого вы должны создать отдельную цепочку `source` → `materialized view` → `destination table` для каждого шарда:

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

* В этой конфигурации балансировка нагрузки должна выполняться вручную.
* Kafka по умолчанию не распределяет сообщения по круговой (round-robin) стратегии.
* Чтобы добиться распределения, похожего на round-robin, при отправке данных, убедитесь, что ваш Kafka producer настроен с:
  * `parse.key=true`
  * `key.separator={your_delimiter}`

Иначе Kafka будет распределять сообщения по своим внутренним правилам, что может привести к неравномерному разбиению по партициям.

### Устранение неполадок

#### Дублирование записей

Фиксация offset’ов Kafka происходит после каждой партии или при таймауте обработки. Если процесс прервался неожиданно во время запроса к материализованному представлению, могут появиться дублирующиеся записи. Чтобы избежать этого, добавьте поле `id` в схему, что позволит Manticore Search предотвращать дубликаты в таблице.

### Как это работает внутренне

- **Инициализация рабочего процесса:** После настройки источника и материализованного представления Manticore Search запускает выделенного рабочего для обработки приема данных из Kafka.
- **Отображение сообщений:** Сообщения сопоставляются согласно схеме конфигурации источника, преобразуясь в структурированный формат.
- **Формирование партий:** Сообщения группируются в партии для эффективной обработки. Размер партии можно настраивать в зависимости от требований к производительности и задержке.
- **Буферизация:** Партии отображенных данных сохраняются в буферной таблице для эффективных операций пакетной обработки.
- **Обработка материализованного представления:** К данным в буферной таблице применяется логика представления, выполняются необходимые преобразования и фильтрация.
- **Передача данных:** Обработанные данные передаются в целевую таблицу реального времени.
- **Очистка:** Буферная таблица очищается после каждой партии, готовясь к обработке следующего набора данных.

<!-- proofread -->

