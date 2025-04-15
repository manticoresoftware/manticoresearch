# 从 Kafka 同步

> 注意：此功能需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

Manticore Search 可以无缝地从 Kafka 代理接收消息，从而实现实时数据索引和搜索。

要开始，您需要：
1. **定义源：** 指定 Manticore Search 将从中读取消息的 Kafka 主题。此设置包括代理的主机、端口和主题名称等详细信息。
2. **设置目标表：** 选择一个 Manticore 实时表来存储传入的 Kafka 数据。
3. **创建物化视图：** 设置一个物化视图（`mv`）来处理从 Kafka 到 Manticore Search 中目标表的数据转换和映射。在这里，您将定义字段映射、数据转换以及任何传入数据流的过滤器或条件。

## 源

<!-- example kafka_source -->

`source` 配置允许您定义 `broker`、`topic list`、`consumer group` 和消息结构。

#### 结构

使用 Manticore 字段类型定义结构，如 `int`、`float`、`text`、`json` 等。

```sql
CREATE SOURCE <源名称> [(列类型，...)] [源选项]
```

所有结构键都是不区分大小写的，这意味着 `Products`、`products` 和 `PrOdUcTs` 被视为相同。它们都被转换为小写。

如果您的字段名称不符合 Manticore Search 中允许的 [字段名称语法](../../Creating_a_table/Data_types.md#Field-name-syntax)（例如，如果它们包含特殊字符或以数字开头），则必须定义一个结构映射。例如，`$keyName` 或 `123field` 在 JSON 中是有效键，但在 Manticore Search 中不是有效字段名称。如果您尝试在没有适当映射的情况下使用无效字段名称，Manticore 将返回错误，源创建将失败。

为了处理此类情况，请使用以下结构语法将无效字段名称映射到有效字段名称：

```
allowed_field_name '带有特殊符号的原始 JSON 键名称' type
```

例如：
```sql
price_field '$price' float    -- 将 JSON 键 '$price' 映射到字段 'price_field'
field_123 '123field' text     -- 将 JSON 键 '123field' 映射到字段 'field_123'
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

| 选项 | 接受值 | 描述 |
|-|-|-|
| `type` | `kafka` | 设置源类型。目前仅支持 `kafka` |
| `broker_list` | `host:port [, ...]` | 指定 Kafka 代理 URL |
| `topic_list` | `string [, ...]` | 列出要消费的 Kafka 主题 |
| `consumer_group`| `string` | 定义 Kafka 消费者组，默认为 `manticore`。 |
| `num_consumers` | `int` | 处理消息的消费者数量。 |
| `batch` | `int` | 在继续之前处理的消息数量。默认为 `100`；否则在超时时处理剩余消息 |

### 目标表

<!-- example kafka_destination -->

目标表是一个常规的实时表，用于存储 Kafka 消息处理的结果。此表应定义为符合传入数据的结构要求，并优化以满足应用程序的查询性能需求。有关创建实时表的更多信息，请 [点击这里](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table: )。

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

物化视图启用从 Kafka 消息的数据转换。您可以重命名字段，应用 Manticore Search 函数，并执行排序、分组和其他数据操作。

物化视图作为查询，移动数据从 Kafka 源到目标表，让您使用 Manticore Search 语法自定义这些查询。确保 `select` 中的字段与源中的字段匹配。

```
CREATE MATERIALIZED VIEW <物化视图名称>
TO <目标表名称> AS
SELECT [列|函数 [as <新名称>], ...] FROM <源名称>
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

数据从 Kafka 转移到 Manticore Search 是批量进行的，每次运行后都会清除批量处理。对于跨批次的计算，例如 AVG，请谨慎使用，因为由于逐批处理，这些可能无法按预期进行。


### 字段映射

下面是基于上述示例的映射表：

| Kafka           | 源      | 缓冲区  | MV                                | 目标       |
|-----------------|----------|----------|-----------------------------------|-----------|
| `id`              | `id`       | `id`       | `id`                                | `id`          |
| `term`            | `term`     | `term`     | `term as name`                      | `name`        |
| `unnecessary_key` 我们不感兴趣的 | -        | -        |                                   |             |
| `$abbrev`         | `abbrev`   | `abbrev`   | `abbrev` as `short_name`              | `short_name`  |
| -                 | -        | -         | `UTC_TIMESTAMP() as received_at`  | `received_at` |
| `GlossDef`        | `glossdef` | `glossdef` | `glossdef.size as size`             | `size`        |

### 列表

<!-- example kafka_listing -->

要查看 Manticore Search 中的源和物化视图，请使用以下命令：
- `SHOW SOURCES`: 列出所有配置的源。
- `SHOW MVS`: 列出所有物化视图。
- `SHOW MV view_table`: 显示特定物化视图的详细信息。

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

如果您在不删除 MV 的情况下删除了 `source`，它会自动暂停。在重新创建源之后，使用 `ALTER` 命令手动恢复 MV。

目前，只有物化视图可以被修改。要更改 `source` 参数，请删除并重新创建源。

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

### 故障排除

#### 重复条目

Kafka 偏移量在每批后或处理超时时提交。如果在物化视图查询期间意外停止进程，您可能会看到重复条目。为了避免这种情况，请在您的模式中包含一个 `id` 字段，以允许 Manticore Search 防止表中的重复项。

### 它在内部是如何工作的

- **工作线程初始化:** 配置源和物化视图后，Manticore Search 设置一个专用工作线程处理来自 Kafka 的数据摄取。
- **消息映射:** 消息根据源配置模式映射，将其转换为结构化格式。
- **分批:** 消息被分组到批次中以提高处理效率。批大小可以根据您的性能和延迟需求进行调整。
- **缓冲:** 映射的数据批次存储在一个缓冲表中，以便于高效的批量操作。
- **物化视图处理:** 在缓冲表中应用视图逻辑，对数据进行任何变换或过滤。
- **数据传输:** 处理后的数据随后被传输到目标实时表。
- **清理:** 每批后清空缓冲表，确保它为下一组数据做好准备。

<!-- proofread -->
