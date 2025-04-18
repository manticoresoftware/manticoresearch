# manual Creating a Sharded Table

Manticore allows for the creation of **sharded tables**, which act like a special distributed table containing multiple tables that represent shards. This feature can be useful if you need to scale data. You can create a local sharded table and a replicated sharded table when multiple clusters are required.

#### Create a Local Sharded Table

To create a local sharded table, you can create a table as normal but also add two parameters: `shards='x'` and `rf='1'`. `shards` represents the number of local tables that will be created and represent shards. `rf` is the replication factor, and for a single server environment, you need to use 1 here.

Here's an example to create a table that will contain 10 shards, and all data should be automatically distributed:

```sql
CREATE TABLE local_sharded shards='10' rf='1'
```

After this query, you will get multiple tables that represent shards and a final distributed table with the name `local_sharded` that you can use to insert and read data, and the distribution logic will be handled by the Manticore Search daemon.

#### Create a Replicated Sharded Table

In case you require to ensure that your data is safe and bypass server outages, you probably already have multiple nodes. You need to set up a replication cluster with the name `c`, add all the nodes you want to be in it by following the original manual instructions, and after that, you can create the table with the given replication factor.

Let's assume you have created a replication cluster with 3 nodes and want to create a table that will be sharded into 10 and have at least one copy on each node. You should create it by following the next query:

```sql
CREATE TABLE c:cluster_sharded shards='10' rf='1'
```

After that, you can work with your table as normal, but don't forget to use the cluster prefix `c` for data manipulation.


The default timeout to wait for all processes of shard preparation during creation is 30 seconds. Sometimes, when you want to create many shards on a replication cluster with multiple nodes, it takes a bit longer due to network latency. If needed, you can control this timeout and increase it by setting the timeout option like this:

```sql
CREATE TABLE c:cluster_sharded shards='10' rf='1' timeout='60'
```

#### Drop a Sharded Table

To drop a sharded table, you should still use the standard `DROP TABLE` command. However, in a clustered environment, you need to specify the cluster name in the table name to properly target the table you want to drop.

To delete a local sharded table, you can use the following command:

```sql
DROP TABLE local_sharded
```

To delete a replicated sharded table, you can use the following command:

```sql
DROP TABLE c:cluster_sharded
```

<!-- proofread -->

