# Creating a Sharded Table

> NOTE: Auto-sharding requires [Manticore Buddy](../../Installation/Manticore_Buddy.md). If auto-sharding doesn't work, make sure Buddy is installed and running.

Manticore allows for the creation of **sharded tables**, a special table type (`type='shard'`) that transparently fans out reads and writes across multiple underlying physical shards. This feature proves invaluable when scaling your data. You can create both local sharded tables on a single server and replicated sharded tables on a multi-node replication cluster to optimize data distribution.

Sharded tables offer several key benefits for high-performance applications:
- They deliver superior write performance by writing data to multiple shards in parallel, utilizing system resources more efficiently. This parallel writing capability enables higher indexing throughput compared to a single large table.
- Sharded tables support replication out of the box, enhancing high availability. You don't need to handle replication manually. Simply set the replication factor when creating the sharded table, and the system manages everything. This built-in replication ensures continuous data accessibility even if some nodes fail.

#### Options

- `shards='N'` — number of physical shards to create. Required. Must be a positive integer, with a maximum of 3000.
- `rf='M'` — replication factor: the number of copies kept for each shard. Required. On a single node it must be `1`; on a replication cluster it must be between 1 and the number of nodes in the cluster.
- `timeout='S'` — how long (in seconds) to wait for shard preparation during creation. Defaults to `30`. Increase it when creating many shards across many nodes.

Values must be quoted (`shards='10'`, not `shards=10`). Option names are case-insensitive.

#### Creating a Local Sharded Table

To create a local sharded table, create a table as you normally would and add `shards='N'` and `rf='1'`. `shards` is the number of physical shards that will be created behind the table. `rf` is the replication factor. On a single server it must be `1`.

Here's an example that creates a table with 10 shards, with data automatically distributed across them:

```sql
CREATE TABLE local_sharded shards='10' rf='1'
```

After this query you get a single sharded table with all its shards already set up. The underlying physical shards live under the `system` database and are hidden from `SHOW TABLES`; you interact with the sharded table by its public name and the system routes operations to the appropriate shards automatically.

#### Creating a Replicated Sharded Table

To protect against server outages, set up a replication cluster across the nodes you want to participate. Throughout this documentation we'll assume the cluster you created is named `c`. Add all desired nodes by following the [replication](../../Creating_a_cluster/Setting_up_replication/Setting_up_replication.md) instructions, then create the table with the desired replication factor.

For example, let's assume you have a 3-node replication cluster and want a table sharded into 10 shards with one copy of each shard on every node. With three nodes participating you set `rf='3'`:

```sql
CREATE TABLE c:cluster_sharded shards='10' rf='3'
```

After that you can work with the table by its plain name on any cluster node — `INSERT`, `SELECT`, `UPDATE`, and `DELETE` do not require the cluster prefix. The cluster prefix (`c:`) is only used for DDL such as `CREATE TABLE` and `DROP TABLE`.

The default timeout to wait for all processes of shard preparation during creation is 30 seconds. Sometimes, when creating many shards on a replication cluster with multiple nodes, it takes a bit longer due to network latency. If needed, you can increase this timeout via the `timeout` option:

```sql
CREATE TABLE c:cluster_sharded shards='10' rf='3' timeout='60'
```

If the timeout is exceeded, the table creation will fail, and you'll need to retry with a longer timeout value.

#### Dropping a Sharded Table

To drop a sharded table, use the standard `DROP TABLE` command. In a clustered environment, specify the cluster name in the table name to properly target the table you want to drop. `IF EXISTS` is supported.

To delete a local sharded table:

```sql
DROP TABLE local_sharded
DROP TABLE IF EXISTS local_sharded
```

To delete a replicated sharded table:

```sql
DROP TABLE c:cluster_sharded
DROP TABLE IF EXISTS c:cluster_sharded
```

#### Inspecting Sharded Tables

`DESC <table>` and `SHOW CREATE TABLE <table>` work on sharded tables and show the resolved structure (including `type='shard'` and the underlying shard locations).

Two extra commands are provided to inspect the state of the sharding subsystem:

- `SHOW SHARDING STATUS [[<cluster>:]<table>]` — lists per-shard placement and health. Without arguments it lists all sharded tables; with a table name (optionally cluster-prefixed) it filters to that table. Returned columns: `table`, `shard`, `node`, `status` (`active`/`inactive`), `cluster`, `replication_cluster`, `rf`, and `rf_status` (`ok`/`degraded`/`broken`).

   ```sql
   SHOW SHARDING STATUS
   SHOW SHARDING STATUS cluster_sharded
   SHOW SHARDING STATUS c:cluster_sharded
   ```

- `SHOW SHARDING MASTER` — shows which node currently runs the sharding master process and whether it's `active` or `inactive`.

   ```sql
   SHOW SHARDING MASTER
   ```

<!-- proofread -->

