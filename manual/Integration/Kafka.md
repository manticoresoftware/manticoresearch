# Reading from Kafka (Queue Support)

<!-- Example: Kafka Queues -->

ManticoreSearch can now consume messages from a Kafka broker through the Buddy plugin.

To get started, you only need to define the `source`, `destination table`, and `materialized view` (alias `mv`).

## Workflow:

1. Workers ingest messages from the Kafka broker.
2. These messages are then mapped according to the schema defined in the source.
3. Messages are organized into batches.
4. Subsequently, they are placed into a buffer table.
5. The materialized view's query is executed on the buffer table.
6. The results are subsequently written to the destination table.
7. Lastly, the buffer table is truncated.

## Source

The `source` allows you to define the `broker`, `topic list`, `consumer group`, and describe the internal message structure.

#### Schema

The schema can be defined with standard Manticore fields (`int`, `float`, `text`, `json`, etc).

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

Keys in the schema are case-insensitive, so there is no difference between keys like `Products`, `products`, `PrOdUcTs`.
All of them are converted to **lowercase**.

<!-- Intro -->

##### SQL:

<!-- Request SQL -->

```sql
CREATE SOURCE kafka
    (id bigint, term text, abbrev text, GlossDef json)
type='kafka'
broker_list='kafka:9092'
topic_list='my-data'
consumer_group='manticore'
num_consumers='2'
batch=50
```

<!-- Response -->

```
Query OK, 2 rows affected (0.02 sec)
```

<!-- End -->

#### Options

| Option           | Accepted Values   | Description                                                                                                                                                                       |
|------------------|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `type`           | Kafka             | Source type. Currently, only Kafka is supported.                                                                                                                                              |
| `broker_list`    | host:port [, ...] | Kafka broker URL.                                                                                                                                                                |
| `topic_list`     | string [, ...]    | List of Kafka topics for consumption.                                                                                                                                              |
| `consumer_group` | string            | Kafka consumer group. Default is `manticore`.                                                                                                                                       |
| `num_consumers`  | int               | Number of workers.                                                                                                                                                                  |
| `batch`          | int               | Number of messages stacked before processing. Default is `100`. In case of a timeout and workers didn't consume the desired amount of messages, they will process what they have already consumed. |

### Destination Table

This is a regular table where you store your results.

<!-- Intro -->

##### SQL:

<!-- Request SQL -->

```sql
CREATE TABLE destination_kafka
(id bigint, name text, short_name text, received_at text, size multi);
```

<!-- Response -->

```
Query OK, 0 rows affected (0.02 sec)
```

<!-- End -->

### Materialized View

A materialized view allows you to perform modifications to received messages, such as renaming fields, applying some ManticoreSearch functions, sorting, grouping, and other operations.

However, it's just a regular query from the buffer to destination tables, so you can use all Manticore syntax for processing these queries.

```
CREATE MATERIALIZED VIEW <materialized view name>
TO <destination table name> AS
SELECT [column|function [as <new name>], ...] FROM <source name>
```

<!-- Intro -->

##### SQL:

<!-- Request SQL -->

```sql
CREATE MATERIALIZED VIEW view_table
TO destination_kafka AS
SELECT id, term as name, abbrev as short_name,
       UTC_TIMESTAMP() as received_at, GlossDef.size as size FROM kafka

```

<!-- Response -->

```sql
Query OK, 2 rows affected (0.02 sec)
```

<!-- End -->

Keep in mind that the buffer table truncates after each batch iteration, so you should avoid calculations like AVG or anything similar.
In the worst case, you will be bound by the batch size (or timeout limit).

### Mapping Fields

For easy comprehension, we provide the following mapping table of examples above:

| Kafka           | Source   | Buffer   | MV                             | Destination |
|-----------------|----------|----------|--------------------------------|-------------|
| id              | id       | id       | id                             | id          |
| term            | term     | term     | term as name                   | name        |
| unnecessary key | -        | -        |                                |             |
| abbrev          | abbrev   | abbrev   | abbrev as short_name           | short_name  |
| -               | -        |          | UTC_TIMESTAMP() as received_at | received_at |
| GlossDef	       | GlossDef | GlossDef | GlossDef.size as size          | size        |

### Listing

The current plugin enables you to list sources and views, as well as obtain comprehensive details about the desired resource.

<!-- Intro -->

##### SQL:

<!-- Request SQL -->

```sql
SHOW SOURCES
```

<!-- Response -->

```
+-------+
| name  |
+-------+
| kafka |
+-------+
```

<!-- End -->

<!-- Intro -->

##### SQL:

<!-- Request SQL -->

```sql
show source kafka;
```

<!-- Response -->

```
+--------+---------------------------------------------------------+
| Source | Create Table                                            |
+--------+---------------------------------------------------------+
| kafka  | CREATE SOURCE kafka                                     |
|        | (id bigint, term text, abbrev text, GlossDef json)      |
|        | type='kafka'                                            |
|        | broker_list='kafka:9092'                                |
|        | topic_list='my-data'                                    |
|        | consumer_group='manticore'                              |
|        | num_consumers='2'                                       |
|        | batch=50                                                |
+--------+---------------------------------------------------------+
```

<!-- End -->

<!-- Intro -->

##### SQL:

<!-- Request SQL -->

```sql
SHOW MVS
```

<!-- Response -->

```
+------------+
| name       |
+------------+
| view_table |
+------------+
```

<!-- End -->


<!-- Intro -->

##### SQL:

<!-- Request SQL -->

```sql
SHOW MV view_table
```

<!-- Response -->

```
+------------+--------------------------------------------------------------------------------------------------------+-----------+
| View       | Create Table                                                                                           | suspended |
+------------+--------------------------------------------------------------------------------------------------------+-----------+
| view_table | CREATE MATERIALIZED VIEW view_table TO destination_kafka AS                                            | 0         |
|            | SELECT id, term as name, abbrev as short_name, UTC_TIMESTAMP() as received_at, GlossDef.size as size   |           |
|            | FROM kafka                                                                                             |           |
+------------+--------------------------------------------------------------------------------------------------------+-----------+
```

<!-- End -->

### Altering Materialized Views

You can suspend consumption by altering `materialized views`.
If you remove the `source` and do not delete the MV, it will automatically suspend until you recreate the source with the same name.

Currently, only altering MVs is supported. To change `source` parameters, you should drop it and recreate it again.

### Restarting Searchd

Offsets are committed after each batch/timeout processing.
However, in scenarios where the daemon unexpectedly ceases during the execution of the materialized view (MV) query,
the risk of receiving duplicates emerges. Therefore, it's advisable to include an id field in the schema.
This allows for the **automatic management of duplicates** within the table, should any issues arise.

<!-- Proofread -->
