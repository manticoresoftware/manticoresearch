# 从 Kafka 同步

> 注意：此功能需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

Manticore 支持通过 Kafka 源和物化视图与 [Apache Kafka](https://kafka.apache.org/) 实时数据摄取集成，实现实时数据索引和搜索。目前，**apache/kafka 版本 3.7.0-4.1.0** 已测试并支持。

入门步骤：
1. **定义源：** 指定 Manticore Search 将读取消息的 Kafka 主题。此设置包括代理的主机、端口和主题名称等详细信息。
2. **设置目标表：** 选择一个 Manticore 实时表用于存储传入的 Kafka 数据。
3. **创建物化视图：** 设置物化视图（`mv`），用于处理从 Kafka 到 Manticore Search 目标表的数据转换和映射。在这里，您将定义字段映射、数据转换以及任何传入数据流的过滤或条件。

## 源

<!-- example kafka_source -->

`source` 配置允许您定义 `broker`、`topic list`、`consumer group` 和消息结构。

#### 模式

使用 Manticore 字段类型如 `int`、`float`、`text`、`json` 等定义模式。

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

所有模式键不区分大小写，即 `Products`、`products` 和 `PrOdUcTs` 视为相同，都会转换为小写。

如果您的字段名称不符合 Manticore Search 允许的 [字段名称语法](../Creating_a_table/Data_types.md#Field-name-syntax)（例如包含特殊字符或以数字开头），必须定义一个模式映射。例如，`$keyName` 或 `123field` 在 JSON 中是有效键，但在 Manticore Search 中不是有效字段名。如果尝试使用无效字段名称且没有正确映射，Manticore 将返回错误，源创建将失败。

针对这种情况，使用以下模式语法将无效字段名称映射到有效字段：

```
allowed_field_name 'original JSON key name with special symbols' type
```

例如：
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

#### 选项

| 选项             | 可接受的值         | 描述                                                                                                           |
|-----------------|-------------------|-----------------------------------------------------------------------------------------------------------------|
| `type`          | `kafka`           | 设置源类型。目前，仅支持 `kafka`                                                                               |
| `broker_list`   | `host:port [, ...]` | 指定 Kafka 代理 URL                                                                                              |
| `topic_list`    | `string [, ...]`  | 列出要消费的 Kafka 主题                                                                                          |
| `consumer_group`| `string`          | 定义 Kafka 消费者组，默认为 `manticore`                                                                          |
| `num_consumers` | `int`             | 处理消息的消费者数量                                                                                              |
| `partition_list`| `int [, ...]`     | 指定读取的分区列表 [详情](../Integration/Kafka.md#Sharding-with-Kafka)                                          |
| `batch`         | `int`             | 处理的消息数目，达到该数量后继续。默认是 `100`；否则超时处理剩余消息                                            |

### 目标表

<!-- example kafka_destination -->

目标表是一个常规的实时表，用于存储 Kafka 消息处理结果。该表应定义以符合传入数据的模式需求，并优化应用程序的查询性能。更多关于创建实时表请见 [这里](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:)。

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

### 物化视图

<!-- example kafka_mv -->

物化视图允许对 Kafka 消息进行数据转换。您可以重命名字段、应用 Manticore Search 函数，执行排序、分组及其他数据操作。

物化视图作为一个查询，将数据从 Kafka 源移动到目标表，允许您使用 Manticore Search 语法自定义这些查询。请确保 `select` 中的字段与源中的匹配。

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

数据以批量方式从 Kafka 传输到 Manticore Search，每次运行后清空该批次。对于跨批次计算（如 AVG），请谨慎使用，因为批次处理可能导致结果不符合预期。


### 字段映射

根据以上示例，映射表如下：

| Kafka               | Source    | Buffer    | MV                                  | Destination   |
|---------------------|-----------|-----------|------------------------------------|---------------|
| `id`                | `id`      | `id`      | `id`                               | `id`          |
| `term`              | `term`    | `term`    | `term as name`                     | `name`        |
| `unnecessary_key`（我们不感兴趣的） | -         | -         |                                    |               |
| `$abbrev`           | `abbrev`  | `abbrev`  | `abbrev` as `short_name`           | `short_name`  |
| -                   | -         | -         | `UTC_TIMESTAMP() as received_at`  | `received_at` |
| `GlossDef`          | `glossdef`| `glossdef`| `glossdef.size as size`             | `size`        |

### 列表

<!-- example kafka_listing -->

要查看 Manticore Search 中的源和物化视图，请使用以下命令：
- `SHOW SOURCES`：列出所有配置的源。
- `SHOW MVS`：列出所有物化视图。
- `SHOW MV view_table`：显示特定物化视图的详细信息。

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

### 修改物化视图

<!-- example mv_suspend -->

您可以通过修改物化视图来暂停数据消费。

如果移除 `source` 但不删除 MV，则会自动暂停。重新创建源后，需手动使用 `ALTER` 命令取消暂停 MV。

当前只支持修改物化视图。要更改 `source` 参数，请删除并重新创建源。

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

### 使用 Kafka 进行分片

您还可以为每个 Kafka 主题指定 `partition_list`。
此方法的主要优点之一是能够通过 Kafka 实现表的 `sharding`（分片）。
为实现这一点，应为每个分片创建一条独立的 `source` → `materialized view` → `destination table` 链：

**源：**
```sql
CREATE SOURCE kafka_p1 (id bigint, term text)
  type='kafka' broker_list='kafka:9092' topic_list='my-data'
  consumer_group='manticore' num_consumers='1' partition_list='0' batch=50;

CREATE SOURCE kafka_p2 (id bigint, term text)
  type='kafka' broker_list='kafka:9092' topic_list='my-data'
  consumer_group='manticore' num_consumers='1' partition_list='1' batch=50;
```

**目标表：**

```sql
CREATE TABLE destination_shard_1 (id bigint, name text);
CREATE TABLE destination_shard_2 (id bigint, name text);
```

**物化视图：**

```sql
CREATE MATERIALIZED VIEW mv_1 TO destination_shard_1 AS SELECT id, term AS name FROM kafka_p1;
CREATE MATERIALIZED VIEW mv_2 TO destination_shard_2 AS SELECT id, term AS name FROM kafka_p2;
```

#### ⚠️ 重要提示：

* 在此设置中，分区重新平衡必须手动管理。
* Kafka 默认不使用轮询策略分发消息。
* 若要实现类似轮询的消息分发，确保 Kafka 生产者配置为：
  * `parse.key=true`
  * `key.separator={your_delimiter}`

否则，Kafka 将依据其内部规则分发消息，可能导致分区不均匀。

### 故障排查

#### 重复条目

Kafka 在每批处理后或处理超时后提交偏移量。如果物化视图查询过程中进程意外停止，可能会出现重复条目。为避免此情况，在模式中包含 `id` 字段，可让 Manticore Search 在表中防止重复。

### 内部工作原理

- **工作线程初始化：** 配置源和物化视图后，Manticore Search 会设置专用工作线程处理来自 Kafka 的数据摄取。
- **消息映射：** 根据源配置方案映射消息，将其转换为结构化格式。
- **批处理：** 将消息分组为批次以提高处理效率。批次大小可根据性能和延迟需求调整。
- **缓冲：** 映射后的数据批次存储在缓冲表中，以便高效批量操作。
- **物化视图处理：** 对缓冲表中的数据应用视图逻辑，执行任何转换或过滤。
- **数据转移：** 处理后的数据随后传输到目标的实时表。
- **清理：** 每批处理后清空缓冲表，确保为下一批数据做好准备。

<!-- proofread -->

