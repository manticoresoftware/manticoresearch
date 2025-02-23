# Syncing from Kafka

> NOTE: this functionality requires [Manticore Buddy](../Installation/Manticore_Buddy.md). If it doesn't work, make sure Buddy is installed.

Manticore Search can seamlessly consume messages from a Kafka broker, allowing for real-time data indexing and search.

To get started, you need to:
1. **Define the source:** Specify the Kafka topic from which Manticore Search will read messages. This setup includes details like the broker’s host, port, and topic name.
2. **Set up the destination table:** Choose a Manticore real-time table to store the incoming Kafka data.
3. **Create a materialized view:** Set up a materialized view (`mv`) to handle data transformation and mapping from Kafka to the destination table in Manticore Search. Here, you’ll define field mappings, data transformations, and any filters or conditions for the incoming data stream.

## Source

<!-- example kafka_source -->

The `source` configuration allows you to define the `broker`, `topic list`, `consumer group`, and the message structure.

#### Schema

Define the schema using Manticore field types like `int`, `float`, `text`, `json`, etc.

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

All schema keys are case-insensitive, meaning `Products`, `products`, and `PrOdUcTs` are treated the same. They are all converted to lowercase.

If your field names don't match the [field name syntax](../../Creating_a_table/Data_types.md#Field-name-syntax) allowed in Manticore Search (for example, if they contain special characters or start with numbers), you must define a schema mapping. For instance, `$keyName` or `123field` are valid keys in JSON but not valid field names in Manticore Search. If you try to use invalid field names without proper mapping, Manticore will return an error and the source creation will fail.

To handle such cases, use the following schema syntax to map invalid field names to valid ones:

```
allowed_field_name 'original JSON key name with special symbols' type
```

For example:
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

#### Options

| Option | Accepted Values | Description |
|-|-|-|
| `type` | `kafka` | Sets the source type. Currently, only `kafka` is supported |
| `broker_list` | `host:port [, ...]` | Specifies Kafka broker URLs |
| `topic_list` | `string [, ...]` | Lists Kafka topics to consume from |
| `consumer_group`| `string` | Defines the Kafka consumer group, defaulting to `manticore`. |
| `num_consumers` | `int` | Number of consumers to handle messages. |
| `batch` | `int` | Number of messages to process before moving on. Default is `100`; processes remaining messages on timeout otherwise |

### Destination table

<!-- example kafka_destination -->

The destination table is a regular real-time table where the results of Kafka message processing are stored. This table should be defined to match the schema requirements of the incoming data and optimized for the query performance needs of your application. Read more about creating real-time tables [here](../Creating_a_table/Local_tables/Real-time_table.md#Creating-a-real-time-table:).

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

### Materialized view

<!-- example kafka_mv -->

A materialized view enables data transformation from Kafka messages. You can rename fields, apply Manticore Search functions, and perform sorting, grouping, and other data operations.

A materialized view acts as a query that moves data from the Kafka source to the destination table, letting you use Manticore Search syntax to customize these queries. Make sure that fields in the `select` match those in the source.

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

Data is transferred from Kafka to Manticore Search in batches, which are cleared after each run. For calculations across batches, such as AVG, use caution, as these may not work as expected due to batch-by-batch processing.


### Field Mapping

Here's a mapping table based on the examples above:

| Kafka           | Source   | Buffer   | MV                                | Destination |
|-----------------|----------|----------|-----------------------------------|-------------|
| `id`              | `id`       | `id`       | `id`                                | `id`          |
| `term`            | `term`     | `term`     | `term as name`                      | `name`        |
| `unnecessary_key` which we're not interested in | -        | -        |                                   |             |
| `$abbrev`         | `abbrev`   | `abbrev`   | `abbrev` as `short_name`              | `short_name`  |
| -                 | -        | -         | `UTC_TIMESTAMP() as received_at`  | `received_at` |
| `GlossDef`        | `glossdef` | `glossdef` | `glossdef.size as size`             | `size`        |

### Listing

<!-- example kafka_listing -->

To view sources and materialized views in Manticore Search, use these commands:
- `SHOW SOURCES`: Lists all configured sources.
- `SHOW MVS`: Lists all materialized views.
- `SHOW MV view_table`: Shows detailed information on a specific materialized view.

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

### Altering materialized views

<!-- example mv_suspend -->

You can suspend data consumption by altering materialized views.

If you remove the `source` without deleting the MV, it automatically suspends. After recreating the source, unsuspend the MV manually using the `ALTER` command.

Currently, only materialized views can be altered. To change `source` parameters, drop and recreate the source.

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

### Troubleshooting

#### Duplicate entries

Kafka offsets commit after each batch or when processing times out. If the process stops unexpectedly during a materialized view query, you may see duplicate entries. To avoid this, include an `id` field in your schema, allowing Manticore Search to prevent duplicates in the table.

### How it works internally

- **Worker initialization:** After configuring a source and materialized view, Manticore Search sets up a dedicated worker to handle data ingestion from Kafka.
- **Message mapping:** Messages are mapped according to the source configuration schema, transforming them into a structured format.
- **Batching:** Messages are grouped into batches for efficient processing. Batch size can be adjusted to suit your performance and latency needs.
- **Buffering:** Mapped data batches are stored in a buffer table for efficient bulk operations.
- **Materialized view processing:** The view logic is applied to data in the buffer table, performing any transformations or filtering.
- **Data transfer:** Processed data is then transferred to the destination real-time table.
- **Cleanup:** The buffer table is cleared after each batch, ensuring it’s ready for the next set of data.

<!-- proofread -->
