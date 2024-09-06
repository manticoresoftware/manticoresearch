# Syncing with Kafka

Manticore Search can efficiently consume messages from a Kafka broker, enabling real-time data indexing and search capabilities.

To get started, you need:
1. **Define the source:** Specify the Kafka topic from which Manticore Search will read messages. This involves setting up connection details such as the broker's host, port, and topic name.
2. **Destination table:** Designate a Manticore real-time table where the data from Kafka will be stored.
3. **Create a materialized view:** Establish a materialized view (`mv`) to define the transformation and mapping of data from Kafka to the destination table in Manticore Search. This includes specifying field mappings, data transformations, and any filters or conditions to apply to the incoming data stream.

## Source

<!-- example kafka_source -->

The `source` configuration allows you to define the `broker`, `topic list`, `consumer group`, and the internal message structure.

#### Schema

Define the schema using standard Manticore fields such as `int`, `float`, `text`, `json`, etc.

```sql
CREATE SOURCE <source name> [(column type, ...)] [source_options]
```

All keys in the schema are case-insensitive, meaning there is no difference between Products, products, or PrOdUcTs. They are all converted to lowercase.

<!-- intro -->

##### SQL:

<!-- request SQL -->

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

<!-- response -->

```
Query OK, 2 rows affected (0.02 sec)
```

<!-- end -->

#### Options

| Option | Accepted Values | Description |
|-|-|-|
| `type` | `kafka` | Specifies the source type. Currently, only `kafka` is supported. |
| `broker_list` | host:port [, ...] | Specifies the Kafka broker URLs. |
| `topic_list` | string [, ...] | Lists the Kafka topics to consume from. |
| `consumer_group`| string | Defines the Kafka consumer group. Default is `manticore`. |
| `num_consumers` | int | Specifies the number of consumer processes to handle messages. |
| `batch` | int | Sets the number of messages to accumulate before processing. Default is `100`. Processes what's available at timeout otherwise. |

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

A materialized view allows you to transform incoming messages from Kafka. You can rename fields, apply various Manticore Search functions, and conduct sorting, grouping, and other advanced data manipulation operations.

Essentially, a materialized view functions like a standard query that transfers data from the Kafka source to the destination table. This enables the use of the full range of Manticore Search syntax to tailor these queries according to your specific needs. Ensure the fields you refer to in the `select` match the fields in the source.


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

When data is transferred from Kafka to Manticore Search, it is handled in batches, which are cleared after each iteration. This setup means that aggregate calculations that require continuity of data, such as `AVG` or similar functions, should be approached cautiously. These functions might not perform as expected if they span multiple batches because each batch processes data independently.

### Mapping fields

For easy comprehension, we provide the following mapping table of examples above:

| Kafka | Source | Buffer | MV | Destination |
|-|-|-|-|-|
| id | id | id | id | id |
| term | term | term | term as name | name |
| unnecessary key | - | - | | |
| abbrev | abbrev | abbrev | abbrev as short_name | short_name |
| - | - | | `UTC_TIMESTAMP()`` as received_at | received_at |
| GlossDef | GlossDef | GlossDef | GlossDef.size as size | size |

### Listing

<!-- example kafka_listing -->

To list and inspect the configuration of sources and materialized views within Manticore Search, use the following commands:
- `SHOW SOURCES`: Lists all the sources configured in the system.
- `SHOW MVS`: Lists all materialized views.
- `SHOW MV view_table`: Provides comprehensive details about a specific materialized view.

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

You can suspend consumption by altering materialized views.

If you remove the `source` and do not delete the MV, it will automatically suspend. After recreation of the related source, you should unsuspend MV manually the `ALTER` command

Currently, only altering materalized views is supported. To change `source` parameters, you should drop it and recreate it.

### Troubleshooting

#### Duplicated entries
Offsets in Kafka are committed after each batch or upon timeout processing. However, if the daemon unexpectedly stops during the execution of the materialized view query, there is a potential risk of duplicated entries in the data received. To mitigate this, it is advisable to include an `id` field in your schema. This field acts as a unique identifier, allowing Manticore Search to automatically manage and prevent duplicates within the table if disruptions occur.

### How it works internally

1. **Initialization of worker:** Once you configure a source and a materialized view, Manticore Search initializes a dedicated worker to handle data ingestion from the specified Kafka broker.
2. **Message mapping:** As messages arrive, they are mapped according to the schema provided in the source configuration. This mapping transforms the raw message data into a structured format suitable for processing.
3. **Batch organization:** Incoming messages are grouped into batches to optimize processing efficiency. The size of these batches can be configured based on your performance and latency requirements.
4. **Buffering:** The batches of mapped data are temporarily stored in a buffer table. This allows for efficient bulk operations in the subsequent stages.
5. **Executing materialized view logic:** The logic defined in the materialized view is applied to the data in the buffer table. This may include transformations, aggregations, or filtering operations.
6. **Data transfer:** After processing, the results are written to the destination real-time table where they are stored permanently or until further processing.
7. **Cleanup:** To maintain efficiency and prevent data redundancy, the buffer table is truncated after each batch is processed, clearing the way for the next set of data.

<!-- proofread -->
