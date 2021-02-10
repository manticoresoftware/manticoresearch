# Creating a distributed index

Manticore supports **distributed indexes**. They look like usual plain or real-time indexes, but internally they are just a 'proxy', or named collection of another child indexes used for actual searching. When a query is directed at such index, it is distributed among all indexes in the collection. Server collects responses of the queries and processes them as necessary:

* applies sorting
* recalculates final values of aggregates, etc

From the client's standpoint it looks transparent, as if you just queried any single index.

Distributed indexes can be composed from any other indexes fitting your requirements

* local storage indexes ([plain index](../../Creating_an_index/Local_indexes/Plain_index.md), [Real-Time](../../Creating_an_index/Local_indexes/Real-time_index.md))
* [remote indexes](../../Creating_an_index/Creating_a_distributed_index/Remote_indexes.md)
* combined local storage and remote indexes
* [percolate indexes](../../Creating_an_index/Local_indexes/Percolate_index.md) (local, remote and combinations)
* single local and several remotes or any other combinations

Nesting distributed indexes is supported by declaring them with `agent` (even if they are on the same machine). Distributed indexes cannot be declared with `local` and they will be ignored.

Percolate and template indexes should not be mixed with plain and/or RT indexes.

Distributed index is defined by type 'distributed' in the configuration file or via SQL clause `CREATE TABLE`

#### In a configuration file

```ini
index foo {
    type = distributed
    local = bar
    local = bar1, bar2
    agent = 127.0.0.1:9312:baz
    agent = host1|host2:idx
    agent = host1:9301:idx1|host2:idx2 [ha_strategy=random retry_count=10]
    ...
}
```

#### Via SQL

```sql
CREATE TABLE distributed_index type='distributed' local='local_index' agent='127.0.0.1:9312:remote_index'
```

#### Children

Either way the key component of a distributed index is a list of children (the indexes it points to).

* Lines, starting with `local =` enumerate local indexes, served in the same server. Several local indexes may be written as several `local =` lines, or combined into one list, separated by commas.
* Lines, starting with `agent =` enumerate remote indexes, served anywhere. Each line represents one agent, or endpoint.

Each agent can include several external locations and options specifying how to work with them.

Note, that for remotes the server knows nothing about the type of the index, and it may cause errors, if, say, you issue `CALL PQ` to remote 'foo' which is not a percolate index. 