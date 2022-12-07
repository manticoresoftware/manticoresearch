# Creating a distributed table

Manticore supports **distributed tables**. They look like usual plain or real-time tables, but internally they are just a 'proxy', or named collection of another child tables used for actual searching. When a query is directed at such table, it is distributed among all tables in the collection. Server collects responses of the queries and processes them as necessary:

* applies sorting
* recalculates final values of aggregates, etc

From the client's standpoint it looks transparent, as if you just queried any single table.

Distributed tables can be composed from any other tables fitting your requirements

* local storage tables ([plain table](../../Creating_a_table/Local_tables/Plain_table.md), [Real-Time](../../Creating_a_table/Local_tables/Real-time_table.md))
* [remote tables](../../Creating_a_table/Creating_a_distributed_table/Remote_tables.md)
* combined local storage and remote tables
* [percolate tables](../../Creating_a_table/Local_tables/Percolate_table.md) (local, remote and combinations)
* single local and several remotes or any other combinations

Nesting distributed tables is supported by declaring them with `agent` (even if they are on the same machine). Distributed tables cannot be declared with `local` and they will be ignored.

Percolate and template tables should not be mixed with plain and/or RT tables.

Distributed table is defined by type 'distributed' in the configuration file or via SQL clause `CREATE TABLE`

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

Either way the key component of a distributed table is a list of children (the tables it points to).

* Lines, starting with `local =` enumerate local tables, served in the same server. Several local tables may be written as several `local =` lines, or combined into one list, separated by commas.
* Lines, starting with `agent =` enumerate remote tables, served anywhere. Each line represents one agent, or endpoint.

Each agent can include several external locations and options specifying how to work with them.

Note that for remotes the server knows nothing about the type of the table, and it may cause errors, if, say, you issue `CALL PQ` to remote 'foo' which is not a percolate table.
