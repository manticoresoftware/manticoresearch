# 从 Kafka 同步

> 注意：此功能需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保 Buddy 已安装。

Manticore 支持通过 Kafka 源和物化视图与 [Apache Kafka](https://kafka.apache.org/) 实时数据摄取集成，允许实时数据索引和搜索。目前，**apache/kafka 版本 3.7.0-4.1.0** 已测试并受支持。

入门步骤：
1. **定义源：** 指定 Manticore Search 将从哪个 Kafka 主题读取消息。该配置包括代理的主机、端口及主题名称等详细信息。
2. **设置目标表：** 选择一个 Manticore 实时表来存储接收的 Kafka 数据。
3. **创建物化视图：** 设置一个物化视图（`mv`），用于处理从 Kafka 到 Manticore Search 目标表的数据转换和映射。在这里，你可以定义字段映射、数据转换以及对传入数据流的任何过滤或条件。

## 源

<!-- example kafka_source -->

`source` 配置允许你定义 `broker`、`topic list`、`consumer group` 以及消息结构。

#### 模式

使用 Manticore 字段类型定义模式，比如 `int`、`float`、`text`、`json` 等。

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

所有模式键都是不区分大小写的，意味着 `Products`、`products` 和 `PrOdUcTs` 被视为相同，都转换为小写。

如果你的字段名称不符合 Manticore Search 允许的[字段名称语法](../Creating_a_table/Data_types.md#Field-name-syntax)（例如，包含特殊字符或以数字开头），必须定义模式映射。例如，`$keyName` 或 `123field` 是 JSON 中的有效键，但不是 Manticore Search 中有效的字段名。如果未正确映射而使用无效的字段名，Manticore 会返回错误，导致源创建失败。

应对这种情况，使用以下模式语法将无效字段名映射为有效字段名：

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

<!-- end -->

#### 选项

| 选项           | 允许值                       | 描述                                                                                                 |
|----------------|------------------------------|----------------------------------------------------------------------------------------------------|
| `type`         | `kafka`                      | 设置源类型。目前仅支持 `kafka`                                                                     |
| `broker_list`  | `host:port [, ...]`           | 指定 Kafka 代理的 URL                                                                              |
| `topic_list`   | `string [, ...]`              | 列出要消费的 Kafka 主题                                                                            |
| `consumer_group`| `string`                    | 定义 Kafka 消费者组，默认为 `manticore`                                                            |
| `num_consumers`| `int`                        | 用来处理消息的消费者数量                                                                           |
| `partition_list`| `int [, ...]`                | 指定用于读取的分区列表（详见 [更多信息](../Integration/Kafka.md#Sharding-with-Kafka)）               |
| `batch`        | `int`                        | 在移动到下一批消息前处理的消息数量。默认值是 `100`；超时则处理剩余消息                              |

### 目标表

<!-- example kafka_destination -->

目标表是一个普通的实时表，用于存储 Kafka 消息处理的结果。该表应根据传入数据的模式需求定义，并针对应用程序的查询性能需求进行优化。关于创建实时表的更多信息，请参见[这里](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:)。

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

### 物化视图

<!-- example kafka_mv -->

物化视图使你可以实现从 Kafka 消息到表数据的转换。你可以重命名字段，应用 Manticore Search 函数，以及执行排序、分组等数据操作。

物化视图充当一个查询，将数据从 Kafka 源传输到目标表，让你使用 Manticore Search 语法自定义这些查询。确保 `select` 中的字段与源中的字段匹配。

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

数据以批量方式从 Kafka 传输到 Manticore Search，每次运行后批次消除。对于跨批次计算（如 AVG）需谨慎，因为批次处理逐批进行，结果可能不如预期。


### 字段映射

以下是基于上述示例的映射表：

| Kafka           | 源 (Source) | 缓冲区 (Buffer) | 物化视图 (MV)                       | 目标 (Destination) |
|-----------------|-------------|----------------|-----------------------------------|-------------------|
| `id`            | `id`        | `id`           | `id`                              | `id`              |
| `term`          | `term`      | `term`         | `term as name`                    | `name`            |
| `unnecessary_key`（我们不感兴趣的键） | -           | -              |                                   |                   |
| `$abbrev`       | `abbrev`    | `abbrev`       | `abbrev as short_name`            | `short_name`      |
| -               | -           | -              | `UTC_TIMESTAMP() as received_at` | `received_at`     |
| `GlossDef`      | `glossdef`  | `glossdef`     | `glossdef.size as size`           | `size`            |

### 列表

<!-- example kafka_listing -->

要查看 Manticore Search 中的源和物化视图，使用以下命令：
- `SHOW SOURCES`：列出所有配置的源。
- `SHOW MVS`：列出所有物化视图。
- `SHOW MV view_table`：显示指定物化视图的详细信息。

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

如果您移除 `source` 而不删除 MV，它会自动暂停。重新创建 source 后，需使用 `ALTER` 命令手动解除暂停。

目前，只能修改物化视图。要更改 `source` 参数，请删除并重新创建 source。

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

### Kafka 分片

您还可以为每个 Kafka 主题指定 `partition_list`。
这种方法的主要优势之一是可以通过 Kafka 实现表的 `分片`。
要实现这一点，应为每个分片创建一条单独的 `source` → `物化视图` → `目标表` 链：

**Sources:**
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

#### ⚠️ 重要说明：

* 在此设置中，重新平衡必须手动管理。
* Kafka 默认不会采用轮询（round-robin）策略分发消息。
* 若要在发送数据时实现类似轮询的分发，请确保您的 Kafka 生产者配置了：
  * `parse.key=true`
  * `key.separator={your_delimiter}`

否则，Kafka 会根据其内部规则分发消息，可能导致分区不均衡。

### 故障排查

#### 重复条目

Kafka 在每个批次后或处理超时后提交 offset。如果物化视图查询过程意外停止，可能会出现重复条目。为避免此情况，请在模式中包含 `id` 字段，以便 Manticore Search 能防止表中出现重复项。

### 内部工作原理

- **工作线程初始化：** 配置 source 和物化视图后，Manticore Search 会设置专门的工作线程以处理来自 Kafka 的数据摄取。
- **消息映射：** 消息根据 source 配置的模式映射，转换为结构化格式。
- **批处理：** 消息被分组为批次，以提高处理效率。批大小可根据性能和延迟需求调整。
- **缓冲：** 映射后的数据批次存储在缓冲表中，用于高效的批量操作。
- **物化视图处理：** 将视图逻辑应用于缓冲表中的数据，执行任何转换或过滤。
- **数据传输：** 处理后的数据随后转移到目标实时表。
- **清理：** 每个批次处理完成后，缓冲表被清空，确保为下一批数据做准备。

<!-- proofread -->

