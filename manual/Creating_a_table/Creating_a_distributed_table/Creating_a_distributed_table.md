# Creating a distributed table

Manticore allows for the creation of **distributed tables**, which act like regular plain or real-time tables, but are actually a collection of child tables used for searching. When a query is sent to a distributed table, it is distributed among all tables in the collection. The server then collects and processes the responses to sort and recalculate values of aggregates, if necessary.

From the client's perspective, it appears as if they are querying a single table.

Distributed tables can be composed of any combination of tables, including:

* Local storage tables ([plain table](../../Creating_a_table/Local_tables/Plain_table.md) and [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md))
* [Remote tables](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
* A combination of local and remote tables
* [Percolate tables](../../Creating_a_table/Local_tables/Percolate_table.md) (local, remote, or a combination)
* Single local and multiple remote tables, or any other combination

Mixing percolate and template tables with plain and real-time tables is not recommended.

A distributed table is defined as type 'distributed' in the configuration file or through the SQL clause `CREATE TABLE`

#### In a configuration file

```ini
table foo {
    type = distributed
    local = bar
    local = bar1, bar2
    agent = 127.0.0.1:9312:baz
    agent = host1|host2:tbl
    agent = host1:9301:tbl1|host2:tbl2 [ha_strategy=random retry_count=10]
    ...
}
```

#### Via SQL

```sql
CREATE TABLE distributed_index type='distributed' local='local_index' agent='127.0.0.1:9312:remote_index'
```

#### Children

The essence of a distributed table lies in its list of child tables, to which it points. There are two types of child tables in a distributed table:

1. [Local tables](../../Creating_a_table/Creating_a_distributed_table/Creating_a_local_distributed_table.md#Creating-a-local-distributed-table): These are tables that are served within the same server as the distributed table. To enumerate local tables, you use the syntax `local =`. You can list several local tables using multiple `local =` lines, or combine them into one list separated by commas.

2. [Remote tables](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent): These are tables that are served anywhere outside the server. To enumerate remote tables, you use the syntax `agent =`. Each line represents one endpoint or agent. Each agent can have multiple external locations and options for how it should work. More details [here](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md#agent). It is important to note that the server does not have any information about the type of table it is working with. This may lead to errors if, for example, you issue a `CALL PQ` to a remote table 'foo' that is not a percolate table.

<!-- proofread -->
