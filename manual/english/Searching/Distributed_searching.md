# Distributed searching

Manticore is designed to scale effectively through its distributed searching capabilities. Distributed searching is beneficial for improving query latency (i.e., search time) and throughput (i.e., max queries/sec) in multi-server, multi-CPU, or multi-core environments. This is crucial for applications that need to search through vast amounts of data (i.e., billions of records and terabytes of text).

The primary concept is to horizontally partition the searched data across search nodes and process it in parallel.

Partitioning is done manually. To set it up, you should:

* Set up multiple instances of Manticore on different servers
* Distribute different parts of your dataset to different instances
* Configure a special [distributed table](../Creating_a_table/Creating_a_distributed_table/Creating_a_distributed_table.md) on some of the `searchd` instances
* Route your queries to the distributed table

This type of table only contains references to other local and remote tables - so it cannot be directly reindexed. Instead, you should reindex the tables that it references.

When Manticore receives a query against a distributed table, it performs the following steps:

1. Connects to the configured remote agents
2. Sends the query to them
3. Simultaneously searches the configured local tables (while the remote agents are searching)
4. Retrieves the search results from the remote agents
5. Merges all the results together, removing duplicates
6. Sends the merged results to the client

From the application's perspective, there are no differences between searching through a regular table or a distributed table. In other words, distributed tables are fully transparent to the application, and there's no way to tell whether the table you queried was distributed or local.

Learn more about [remote nodes](../Creating_a_cluster/Remote_nodes.md).

<!-- proofread -->