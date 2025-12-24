# Kafka 同步

> NOTE: 此功能需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装 Buddy。

Manticore 支持通过 Kafka 源和物化视图与 [Apache Kafka](https://kafka.apache.org/) 实时数据摄取集成，允许实时数据索引和搜索。目前，**apache/kafka 版本 3.7.0-4.1.0** 已经过测试并支持。

要开始，请执行以下步骤：
1. **定义源：** 指定 Manticore Search 将从中读取消息的 Kafka 主题。此设置包括有关代理的主机、端口和主题名称的详细信息。
2. **设置目标表：** 选择一个 Manticore 实时表来存储传入的 Kafka 数据。
3. **创建物化视图：** 设置物化视图 (`mv`) 以处理从 Kafka 到 Manticore Search 目标表的数据转换和映射。在这里，您将定义字段映射、数据转换以及传入数据流中的任何过滤器或条件。

## 源

<!-- example kafka_source -->

`source` 配置允许您定义 `broker`、`主题列表`、`消费者组` 以及消息结构。

#### 模式

使用 Manticore 字段类型（如 `int`、`float`、`text`、`json` 等）定义模式。

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

所有模式键都是大小写不敏感的，这意味着 `Products`、`products` 和 `PrOdUcTs` 被视为相同的。它们都会转换为小写。

如果您的字段名不符合 Manticore Search 允许的 [字段名语法](../Creating_a_table/Data_types.md#Field-name-syntax)（例如，包含特殊字符或以数字开头），则必须定义模式映射。例如，`$keyName` 或 `123field` 是 JSON 中的有效键，但在 Manticore Search 中不是有效的字段名。如果您尝试使用无效的字段名而没有适当的映射，Manticore 将返回错误并导致源创建失败。

要处理此类情况，请使用以下模式语法将无效字段名映射到有效字段名：

```
allowed_field_name 'original JSON key name with special symbols' type
```

例如：
```sql
price_field '$price' float    -- maps JSON key '$price' to field 'price_field'
field_123 '123field' text     -- maps JSON key '123field' to field 'field_123'
```
<!-- intro -->

##### SQL：

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

#### 选项

| 选项 | 允许值 | 描述                                                                                                         |
|-|----------------|---------------------------------------------------------------------------------------------------------------------|
| `type` | `kafka`        | 设置源类型。目前，仅支持 `kafka`                                                                                      |
| `broker_list` | `host:port [, ...]` | 指定 Kafka 代理 URL                                                                                         |
| `topic_list` | `string [, ...]` | 列出要从中消费的 Kafka 主题                                                                                  |
| `consumer_group`| `string`       | 定义 Kafka 消费者组，默认为 `manticore`。                                                        |
| `num_consumers` | `int`          | 处理消息的消费者数量。                                                                             |
| `partition_list` | `int [, ...]`  | 用于读取的分区列表 [更多](../Integration/Kafka.md#Sharding-with-Kafka)。                                 |
| `batch` | `int`          | 在继续之前处理的消息数量。默认为 `100`；超时后处理剩余消息。

### 目标表

<!-- example kafka_destination -->

目标表是一个常规的实时表，其中存储了 Kafka 消息处理的结果。此表应定义为符合传入数据的模式要求，并针对应用程序的查询性能需求进行优化。有关创建实时表的更多信息，请参阅 [这里](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:)。

<!-- intro -->

##### SQL：

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

### 物化视图

<!-- example kafka_mv -->

物化视图允许从 Kafka 消息中进行数据转换。您可以重命名字段、应用 Manticore Search 函数，并执行排序、分组和其他数据操作。

物化视图充当从 Kafka 源到目标表移动数据的查询，让您使用 Manticore Search 语法自定义这些查询。确保物化视图中的 `select` 字段与源中的字段匹配。

```
CREATE MATERIALIZED VIEW <materialized view name>
TO <destination table name> AS
SELECT [column|function [as <new name>], ...] FROM <source name>
```

<!-- intro -->

##### SQL：

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

数据以批次的形式从 Kafka 转移到 Manticore Search，并在每次运行后清除。对于跨批次的计算，例如 AVG，请谨慎使用，因为这些可能不会按预期工作，因为是按批次处理的。


### 字段映射

以下是基于上述示例的映射表：

| Kafka           | 源   | 缓冲   | 物化视图                                | 目标 |
|-----------------|----------|----------|-----------------------------------|-------------|
| `id`              | `id`       | `id`       | `id`                                | `id`          |
| `term`            | `term`     | `term`     | `term as name`                      | `name`        |
| `unnecessary_key` which we're not interested in | -        | -        |                                   |             |
| `$abbrev`         | `abbrev`   | `abbrev`   | `abbrev` as `short_name`              | `short_name`  |
| -                 | -        | -         | `UTC_TIMESTAMP() as received_at`  | `received_at` |
| `GlossDef`        | `glossdef` | `glossdef` | `glossdef.size as size`             | `size`        |

### 列表

<!-- example kafka_listing -->

要在 Manticore Search 中查看源和物化视图，请使用以下命令：
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

### 修改物化视图

<!-- example mv_suspend -->

您可以通过修改物化视图来暂停数据消费。

如果您删除了`source`但没有删除物化视图，它会自动暂停。重新创建源后，需要使用`ALTER`命令手动取消暂停物化视图。

目前，只能修改物化视图。要更改`source`参数，请删除并重新创建源。

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

### 使用 Kafka 分片

您还可以为每个 Kafka 主题指定`partition_list`。
这种方法的主要好处之一是能够通过 Kafka 为您的表实现`分片`。
为此，您应该为每个分片创建一条独立的`source` → `物化视图` → `目标表`链：

**源:**
```sql
CREATE SOURCE kafka_p1 (id bigint, term text)
  type='kafka' broker_list='kafka:9092' topic_list='my-data'
  consumer_group='manticore' num_consumers='1' partition_list='0' batch=50;

CREATE SOURCE kafka_p2 (id bigint, term text)
  type='kafka' broker_list='kafka:9092' topic_list='my-data'
  consumer_group='manticore' num_consumers='1' partition_list='1' batch=50;
```

**目标表:**

```sql
CREATE TABLE destination_shard_1 (id bigint, name text);
CREATE TABLE destination_shard_2 (id bigint, name text);
```

**物化视图:**

```sql
CREATE MATERIALIZED VIEW mv_1 TO destination_shard_1 AS SELECT id, term AS name FROM kafka_p1;
CREATE MATERIALIZED VIEW mv_2 TO destination_shard_2 AS SELECT id, term AS name FROM kafka_p2;
```

#### ⚠️ 重要注意事项：

* 在此设置中，重新均衡必须手动管理。
* Kafka 默认情况下不使用轮询策略分发消息。
* 若要在发送数据时实现类似轮询的分发，请确保您的 Kafka 生产者配置了：
  * `parse.key=true`
  * `key.separator={your_delimiter}`

否则，Kafka 会根据其内部规则分发消息，这可能导致分区不均。

### 故障排除

#### 重复条目

Kafka 在每个批次后或处理超时时提交偏移量。如果物化视图查询过程中意外停止，您可能会看到重复条目。为避免此情况，请在您的模式中包含一个`id`字段，使 Manticore Search 能够防止表中的重复。

### 内部工作原理

- **工作线程初始化：** 配置源和物化视图后，Manticore Search 会设置专用工作线程处理来自 Kafka 的数据摄取。
- **消息映射：** 消息根据源配置的模式映射，转换为结构化格式。
- **批处理：** 消息被分组为批次以提高处理效率。批大小可调整以满足性能和延迟需求。
- **缓冲：** 映射后的数据批次存储在缓冲表中，以便高效的批量操作。
- **物化视图处理：** 对缓冲表中的数据应用视图逻辑，执行任何转换或过滤。
- **数据传输：** 处理后的数据随后传输到目标的实时表。
- **清理：** 每个批次完成后清空缓冲表，确保为下一批数据做好准备。

<!-- proofread -->

