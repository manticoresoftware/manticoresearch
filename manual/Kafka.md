# Reading from Kafka (queue support)

<!-- example kafka_queues -->

Now via Buddy plugin ManticoreSearch can consume messages from Kafka broker

For surfaces, you only need to define `source`, `destination table` and `materialized view` (alias `mv`).

## Workflow:

1. Worker consume messages from Kafka's broker.
2. Then **map** them by schema defined in source.
3. Stack in batch
4. Put in buffer table.
5. Applies query from MV to buffer table
6. Write results to destination table
7. Truncates buffer table

## Source

`Source` allow you to define `broker`, `topic_list`, `consumer group` and describe internal message structure.

#### Schema

Schema can be defined with standard manticore fields (int, float, text, json, multi, etc).

Keys in schema case-insensitive, so there no difference between keys `Products`, `products`, `PrOdUcTs`. All them
converted to lower case.

Preferably to have id field in schema.
This allows you control duplicates in table in case something went wrong.


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

```sql
Query OK, 2 rows affected (0.02 sec)
```

<!-- end -->


#### Options

| Option           | Accepted values       | description                                                                                                                                                                       |
|------------------|-----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `type`           | Kafka                 | Source type. Now only Kafka suported                                                                                                                                              |
| `broker_list`    | host(ip):port [, ...] | Kafka's broker url                                                                                                                                                                |
| `topic_list`     | string [, ...]        | List of Kafka topics for consuming                                                                                                                                                |
| `consumer_group` | string                | Kafka's consumer group. Default `manticore`                                                                                                                                       |
| `num_consumers`  | int                   | Count of workers                                                                                                                                                                  |
| `batch`          | int                   | Count of messages stacking before processing. Default `100`. In case timeout occured and worker didn't consume desired amount of messages it will process what he already consume |

### Destination table

Just regular table where you store your results

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE destination_kafka (id bigint, name text, short_name text, received_at text, size multi);
```

<!-- response -->

```sql
Query OK, 0 rows affected (0.02 sec)
```

<!-- end -->

### Materialized View

Materialized table allow you to perform some modifications with received message.
Like renaming fields, applying some ManticoreSearch functions, sorting, grouping and other stuff

However, it's just regular query from buffer to destination tables, so you can use all Manticore syntax for processing
this queries

<!-- intro -->

##### SQL:

<!-- request SQL -->

```sql
CREATE MATERIALIZED VIEW view_table
TO destination_kafka AS
SELECT id, term as name, abbrev as short_name, UTC_TIMESTAMP() as received_at, GlossDef.size as size FROM kafka

```

<!-- response -->

```sql
Query OK, 2 rows affected (0.02 sec)
```

<!-- end -->

Consider that buffer table truncates after each batch iterations, so you should avoid calculation like AVG, or something
like this.
In worst case you will bound by batch size (or timeout limit)

### Mapping

In examples above we have next mapping:

| Kafka           | Source   | Buffer   | MV                             | destination |
|-----------------|----------|----------|--------------------------------|-------------|
| id              | id       | id       | id                             | id          |
| term            | term     | term     | term as name                   | name        |
| unnecessary key | -        | -        |                                |             |
| abbrev          | abbrev   | abbrev   | abbrev as short_name           | short_name  |
| -               | -        |          | UTC_TIMESTAMP() as received_at | received_at |
| GlossDef	       | GlossDef | GlossDef | GlossDef.size as size          | size        |

### Altering mvs

You can suspend consuming by altering materialized view. In case removing source and not deleting mv, it will suspend
automatically till you recreate source with same name

Now we support only mv altering. To change source parameters you should drop it and recreate again.

Deleting a table is possible only when the server is running in the RT mode. It is possible to delete RT tables, PQ
tables and distributed tables.

### Restart searchd

In case searchd unexpected stop - everything burn and explode.. graceful shutdown???

We commit offset after each batch|timeout processing. But it can be cases when daemon died when we applying mv query. In that case you riscs to get some duplicates, if you don't have id fields.



<!-- proofread -->
