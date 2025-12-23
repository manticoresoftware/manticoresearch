# Синхронизация из Kafka

> ПРИМЕЧАНИЕ: эта функциональность требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если она не работает, убедитесь, что Buddy установлен.

Manticore поддерживает интеграцию с [Apache Kafka](https://kafka.apache.org/) для потокового приема данных в реальном времени через источники Kafka и материализованные представления, позволяя индексировать и искать данные в режиме реального времени. В настоящее время поддерживаются и протестированы версии **apache/kafka 3.7.0-4.1.0**.

Для начала работы нужно:
1. **Определить источник:** Указать тему Kafka, из которой Manticore Search будет читать сообщения. Эта настройка включает такие детали, как хост брокера, порт и имя темы.
2. **Настроить целевую таблицу:** Выбрать таблицу реального времени Manticore для хранения поступающих данных из Kafka.
3. **Создать материализованное представление:** Настроить материализованное представление (`mv`) для обработки преобразования данных и их сопоставления из Kafka в целевую таблицу в Manticore Search. Здесь вы определяете сопоставления полей, преобразования данных и любые фильтры или условия для входящего потока данных.

## Источник

<!-- example kafka_source -->

Конфигурация `source` позволяет определить `broker`, `list/topic`, `consumer group` и структуру сообщений.

#### Схема

Определите схему, используя типы полей Manticore, такие как `int`, `float`, `text`, `json` и др.

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

Все ключи схемы не чувствительны к регистру, то есть `Products`, `products` и `PrOdUcTs` считаются одинаковыми. Все они приводятся к нижнему регистру.

Если имена полей не соответствуют [синтаксису имен полей](../Creating_a_table/Data_types.md#Field-name-syntax), допускаемому в Manticore Search (например, если они содержат специальные символы или начинаются с цифр), необходимо определить сопоставление схемы. Например, `$keyName` или `123field` — допустимые ключи в JSON, но невалидные имена полей для Manticore Search. Если попытаться использовать такие имена без корректного сопоставления, Manticore вернет ошибку, и создание источника не будет выполнено.

Для обработки таких случаев используйте следующий синтаксис схемы для сопоставления недопустимых имен полей с допустимыми:

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

| Опция             | Допустимые значения      | Описание                                                                                                            |
|-                  |-------------------------|---------------------------------------------------------------------------------------------------------------------|
| `type`            | `kafka`                 | Устанавливает тип источника. В настоящее время поддерживается только `kafka`                                        |
| `broker_list`     | `host:port [, ...]`     | Указывает URL-адреса брокеров Kafka                                                                                |
| `topic_list`      | `string [, ...]`        | Список тем Kafka для потребления                                                                                   |
| `consumer_group`  | `string`                | Определяет группу потребителей Kafka, по умолчанию `manticore`.                                                    |
| `num_consumers`   | `int`                   | Количество потребителей для обработки сообщений.                                                                   |
| `partition_list`  | `int [, ...]`           | Список партиций для чтения [подробнее](../Integration/Kafka.md#Sharding-with-Kafka).                                |
| `batch`           | `int`                   | Количество сообщений для обработки перед переходом к следующему циклу. По умолчанию `100`; иначе обрабатывает оставшиеся сообщения по тайм-ауту |

### Целевая таблица

<!-- example kafka_destination -->

Целевая таблица — обычная таблица реального времени, в которой хранятся результаты обработки сообщений Kafka. Таблица должна быть определена в соответствии с требованиями схемы входящих данных и оптимизирована под потребности производительности запросов вашего приложения. Подробнее о создании таблиц реального времени см. [здесь](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:).

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

Материализованное представление позволяет преобразовывать данные из сообщений Kafka. Вы можете переименовывать поля, применять функции Manticore Search, выполнять сортировку, группировку и другие операции с данными.

Материализованное представление действует как запрос, который переносит данные из источника Kafka в целевую таблицу, позволяя использовать синтаксис Manticore Search для настройки таких запросов. Убедитесь, что поля в `select` соответствуют полям источника.

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

Данные передаются из Kafka в Manticore Search пакетами, которые очищаются после каждого выполнения. Для вычислений, охватывающих несколько пакетов, таких как AVG, следует проявлять осторожность, так как они могут работать некорректно из-за обработки пакет за пакетом.


### Сопоставление полей

Ниже приведена таблица сопоставления на основе приведенных выше примеров:

| Kafka                   | Источник | Буфер   | MV                                    | Цель        |
|-------------------------|----------|---------|-------------------------------------|-------------|
| `id`                    | `id`     | `id`    | `id`                                | `id`        |
| `term`                  | `term`   | `term`  | `term as name`                      | `name`      |
| `unnecessary_key`, который нас не интересует | -        | -       |                                     |             |
| `$abbrev`               | `abbrev` | `abbrev`| `abbrev` as `short_name`            | `short_name`|
| -                       | -        | -       | `UTC_TIMESTAMP() as received_at`    | `received_at`|
| `GlossDef`              | `glossdef`| `glossdef`| `glossdef.size as size`             | `size`      |

### Листинг

<!-- example kafka_listing -->

Чтобы просматривать источники и материализованные представления в Manticore Search, используйте следующие команды:
- `SHOW SOURCES`: Отображает все настроенные источники.
- `SHOW MVS`: Отображает все материализованные представления.
- `SHOW MV view_table`: Показывает подробную информацию по конкретному материализованному представлению.

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

Вы можете приостанавливать потребление данных, изменяя материализованные представления.

Если вы удаляете `source`, не удаляя при этом materialized view, оно автоматически приостанавливается. После воссоздания источника, снимите приостановку MV вручную с помощью команды `ALTER`.

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
Одним из главных преимуществ такого подхода является возможность реализовать `sharding` для вашей таблицы через Kafka.
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

**Целевые таблицы:**

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

* В этой конфигурации балансировку нагрузки необходимо управлять вручную.
* По умолчанию Kafka не распределяет сообщения по принципу round-robin.
* Чтобы добиться распределения, похожего на round-robin, при отправке данных, убедитесь, что ваш Kafka producer настроен с:
  * `parse.key=true`
  * `key.separator={ваш_разделитель}`

Иначе Kafka будет распределять сообщения согласно своим внутренним правилам, что может привести к неравномерному разбиению на партиции.

### Устранение неполадок

#### Дублирование записей

Коммиты оффсетов Kafka происходят после каждой партии или при тайм-ауте обработки. Если процесс прерывается неожиданно во время запроса к материализованному представлению, могут появиться дублированные записи. Чтобы избежать этого, включите поле `id` в схему, что позволит Manticore Search предотвращать дубликаты в таблице.

### Как это работает внутри

- **Инициализация воркера:** После настройки источника и материализованного представления, Manticore Search запускает выделенного воркера для обработки данных из Kafka.
- **Отображение сообщений:** Сообщения сопоставляются согласно схеме источника, преобразуясь в структурированный формат.
- **Группировка:** Сообщения объединяются в партии для эффективной обработки. Размер партии можно настроить под ваши требования к производительности и задержке.
- **Буферизация:** Партии данных сохраняются в буферной таблице для эффективных пакетных операций.
- **Обработка материализованного представления:** Логика представления применяется к данным в буферной таблице, выполняя любые преобразования или фильтрацию.
- **Передача данных:** Обработанные данные затем передаются в целевую таблицу в режиме реального времени.
- **Очистка:** После каждой партии буферная таблица очищается, чтобы быть готовой к следующему набору данных.

<!-- proofread -->

