# Distributed searching

To scale well, Manticore has distributed searching capabilities. Distributed searching is useful to improve query latency (i.e. search time) and throughput (ie. max queries/sec) in multi-server, multi-CPU or multi-core environments. This is essential for applications which need to search through huge amounts data (ie. billions of records and terabytes of text).

The key idea is to horizontally partition searched data across search nodes and then process it in parallel.

Partitioning is done manually. You should:

* setup several instances of Manticore on different servers
* make the instances index (and search) different parts of data
* configure a special [distributed index](../Creating_an_index/Creating_a_distributed_index/Creating_a_distributed_index.md) on some of the `searchd` instances
* and query this index

This kind of index only contains references to other local and remote indexes - so it could not be directly reindexed, and you should reindex those indexes which it references instead.

When Manticore receives a query against distributed index, it does the following: 

1. connects to configured remote agents
2. issues the query to them
3. at the same time searches configured local indexes (while the remote agents are searching)
4. retrieves remote agent's search results
5. merges all the results together, removing the duplicates
6. sends the merged results to client

From the application's point of view, there are no differences between searching through a regular index, or a distributed index at all. That is, distributed indexes are fully transparent to the application, and actually there's no way to tell whether the index you queried was distributed or local.

Read more about [remote nodes](../Creating_a_cluster/Remote_nodes.md).