# Creating a sharded table

Manticore Search supports **sharded tables**. These tables are a type of distributed table, divided into smaller, more manageable parts called shards. Sharding improves performance and scalability, particularly useful for handling large datasets or high traffic loads. It allows for faster write operations as data can be written to multiple shards simultaneously, rather than a single table. Manticore offers two types of sharded tables: local sharded tables and replicated sharded tables.

#### Creating a local sharded table

To create a local sharded table, define the number of shards and set the replication factor to `1` for a single-server environment. Here's how you can create a table with 10 shards where data is distributed automatically:

```sql
CREATE TABLE local_sharded shards=10 rf=1
```

This command creates multiple shard-specific tables and a master table named `local_sharded` for data insertion and retrieval. The shards work behind the scenes, and you interact only with the main table.

#### Creating a replicated sharded table

To enhance data availability and protect against server failures, use a replicated sharded table. First, [establish a replication cluster](../../Creating_a_cluster/Setting_up_replication/Creating_a_replication_cluster.md#Creating-a-replication-cluster) with multiple nodes. Once your cluster is ready, you can create a sharded table that replicates data across all nodes. For example, to create a table with 10 shards, each replicated across 3 nodes in a cluster named 'example_cluster':

```sql
CREATE TABLE example_cluster:cluster_sharded shards=10 rf=3
```

Remember to use the cluster name as a prefix when manipulating data in replicated sharded tables.

<!-- proofread -->
