# Синхронизация из Kafka

> ПРИМЕЧАНИЕ: эта функциональность требует [Manticore Buddy](../Installation/Manticore_Buddy.md). Если не работает, убедитесь, что Buddy установлен.

Manticore поддерживает интеграцию с [Apache Kafka](https://kafka.apache.org/) для потокового получения данных через источники Kafka и материализованные представления, что позволяет индексировать и искать данные в реальном времени. В настоящее время поддерживаются и тестируются версии **apache/kafka 3.7.0-4.1.0**.

Чтобы начать работу, вам необходимо:
1. **Определить источник:** указать тему Kafka, из которой Manticore Search будет читать сообщения. В настройках указываются такие данные, как хост брокера, порт и имя темы.
2. **Настроить целевую таблицу:** выбрать таблицу с поддержкой в реальном времени в Manticore для хранения поступающих данных из Kafka.
3. **Создать материализованное представление:** настроить материализованное представление (`mv`), которое преобразует и отображает данные из Kafka в целевую таблицу Manticore Search. Здесь вы определите сопоставление полей, преобразования данных и любые фильтры или условия для потока входящих данных.

## Источник

<!-- example kafka_source -->

Конфигурация `source` позволяет определить `broker`, `topic list`, `consumer group` и структуру сообщений.

#### Схема

Определите схему, используя типы полей Manticore, такие как `int`, `float`, `text`, `json` и другие.

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

Все ключи схемы не чувствительны к регистру, то есть `Products`, `products` и `PrOdUcTs` считаются одинаковыми. Все они приводятся к нижнему регистру.

Если ваши имена полей не соответствуют [синтаксису имен полей](../Creating_a_table/Data_types.md#Field-name-syntax), допустимому в Manticore Search (например, содержат специальные символы или начинаются с цифр), вам необходимо определить сопоставление схемы. Например, `$keyName` или `123field` — валидные ключи в JSON, но невалидные имена полей в Manticore Search. Если попытаться использовать недопустимые имена без правильного сопоставления, Manticore выдаст ошибку и создание источника завершится неудачей.

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

| Опция           | Допустимые значения          | Описание                                                                                                            |
|-|----------------|---------------------------------------------------------------------------------------------------------------------|
| `type`          | `kafka`                     | Устанавливает тип источника. В настоящее время поддерживается только `kafka`.                                        |
| `broker_list`   | `host:port [, ...]`         | Указывает URL брокеров Kafka                                                                                         |
| `topic_list`    | `string [, ...]`            | Список тем Kafka для чтения                                                                                          |
| `consumer_group`| `string`                   | Определяет группу потребителей Kafka, по умолчанию `manticore`.                                                      |
| `num_consumers` | `int`                      | Количество потребителей для обработки сообщений.                                                                     |
| `partition_list`| `int [, ...]`              | Список партиций для чтения [подробнее](../Integration/Kafka.md#Sharding-with-Kafka).                                   |
| `batch`         | `int`                      | Количество сообщений для обработки до перехода далее. По умолчанию `100`; остаток сообщений обрабатывается по таймауту.|

### Целевая таблица

<!-- example kafka_destination -->

Целевая таблица – обычная таблица с поддержкой в реальном времени, где хранятся результаты обработки сообщений Kafka. Эта таблица должна быть определена в соответствии с требованиями схемы входящих данных и оптимизирована под потребности производительности запросов вашего приложения. Подробнее о создании таблиц с поддержкой в реальном времени читайте [здесь](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:).

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

Материализованное представление действует как запрос, который перемещает данные из источника Kafka в целевую таблицу, позволяя использовать синтаксис Manticore Search для настройки этих запросов. Убедитесь, что поля в `select` соответствуют полям источника.

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

Данные передаются из Kafka в Manticore Search батчами, которые очищаются после каждого запуска. Для вычислений по нескольким батчам, например AVG, следует проявлять осторожность, так как они могут работать некорректно из-за обработки по отдельным батчам.


### Сопоставление полей

Ниже приведена таблица сопоставления based на приведённых выше примерах:

| Kafka           | Source   | Buffer   | MV                                | Целевая таблица |
|-----------------|----------|----------|-----------------------------------|-----------------|
| `id`              | `id`       | `id`       | `id`                                | `id`              |
| `term`            | `term`     | `term`     | `term as name`                      | `name`            |
| `unnecessary_key`, который нам не нужен | -        | -        |                                   |                   |
| `$abbrev`         | `abbrev`   | `abbrev`   | `abbrev` as `short_name`              | `short_name`      |
| -                 | -        | -         | `UTC_TIMESTAMP() as received_at`  | `received_at`     |
| `GlossDef`        | `glossdef` | `glossdef` | `glossdef.size as size`             | `size`            |

### Список

<!-- example kafka_listing -->

Чтобы просматривать источники и материализованные представления в Manticore Search, используйте следующие команды:
- `SHOW SOURCES`: Показывает список всех настроенных источников.
- `SHOW MVS`: Показывает список всех материализованных представлений.
- `SHOW MV view_table`: Показывает детальную информацию по конкретному материализованному представлению.

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

Если вы удаляете параметр `source`, не удаляя сам MV, он автоматически приостанавливается. После воссоздания источника снимите приостановку MV вручную с помощью команды `ALTER`.

На данный момент изменять можно только материализованные представления. Чтобы изменить параметры `source`, удалите и создайте источник заново.

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
Одним из главных преимуществ этого подхода является возможность реализовать `sharding` для вашей таблицы через Kafka.
Для этого необходимо создать отдельную цепочку `source` → `materialized view` → `destination table` для каждого шарда:

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

* В такой конфигурации балансировка должна выполняться вручную.
* По умолчанию Kafka не распределяет сообщения по круговому алгоритму (round-robin).
* Чтобы добиться распределения, похожего на round-robin, при отправке данных, убедитесь, что продюсер Kafka настроен с параметрами:
  * `parse.key=true`
  * `key.separator={ваш_разделитель}`

Иначе Kafka будет распределять сообщения согласно своим внутренним правилам, что может привести к неравномерному распределению по партициям.

### Устранение неполадок

#### Дублирование записей

Коммиты смещений (offsets) в Kafka происходят после каждой партии или при тайм-ауте обработки. Если процесс остановится неожиданно во время запроса к материализованному представлению, вы можете наблюдать дублирование записей. Чтобы этого избежать, включите поле `id` в вашу схему, позволяющее Manticore Search предотвращать дубликаты в таблице.

### Как это работает внутри

- **Инициализация воркера:** После настройки источника и материализованного представления Manticore Search создаёт выделенный воркер для обработки загрузки данных из Kafka.
- **Сопоставление сообщений:** Сообщения сопоставляются согласно схеме конфигурации источника и преобразуются в структурированный формат.
- **Формирование партий:** Сообщения группируются в партии для эффективной обработки. Размер партии можно настроить под ваши требования к производительности и задержке.
- **Буферизация:** Партии сопоставленных данных сохраняются в буферной таблице для эффективных пакетных операций.
- **Обработка материализованного представления:** Логика представления применяется к данным в буферной таблице, выполняются преобразования и фильтрация.
- **Передача данных:** Обработанные данные затем передаются в целевую таблицу реального времени.
- **Очистка:** Буферная таблица очищается после каждой партии, готовясь к приёму следующего набора данных.

<!-- proofread -->

