.. _distributed_searching:

Distributed searching
---------------------

To scale well, Manticore has distributed searching capabilities.
Distributed searching is useful to improve query latency (ie. search
time) and throughput (ie. max queries/sec) in multi-server, multi-CPU or
multi-core environments. This is essential for applications which need
to search through huge amounts data (ie. billions of records and
terabytes of text).

The key idea is to horizontally partition (HP) searched data across
search nodes and then process it in parallel.

Partitioning is done manually. You should

-  setup several instances of Manticore programs (``indexer`` and
   ``searchd``) on different servers;

-  make the instances index (and search) different parts of data;

-  configure a special distributed index on some of the ``searchd``
   instances;

-  and query this index.

This index only contains references to other local and remote indexes -
so it could not be directly reindexed, and you should reindex those
indexes which it references instead.

When ``searchd`` receives a query against distributed index, it does the
following:

1. connects to configured remote agents;

2. issues the query;

3. sequentially searches configured local indexes (while the remote
   agents are searching);

4. retrieves remote agents' search results;

5. merges all the results together, removing the duplicates;

6. sends the merged results to client.

From the application's point of view, there are no differences between
searching through a regular index, or a distributed index at all. That
is, distributed indexes are fully transparent to the application, and
actually there's no way to tell whether the index you queried was
distributed or local.

Any ``searchd`` instance could serve both as a master (which aggregates
the results) and a slave (which only does local searching) at the same
time. This has a number of uses:

1. every machine in a cluster could serve as a master which searches the
   whole cluster, and search requests could be balanced between masters
   to achieve a kind of HA (high availability) in case any of the nodes
   fails;

2. if running within a single multi-CPU or multi-core machine, there
   would be only 1 searchd instance querying itself as an agent and thus
   utilizing all CPUs/core.

It is scheduled to implement better HA support which would allow to
specify which agents mirror each other, do health checks, keep track of
alive agents, load-balance requests, etc.
