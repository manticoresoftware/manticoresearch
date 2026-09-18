# Table Sharding: Current Limitations

When working with sharded tables, be aware of the following limitations:

1. **Local and Clustered Sharded Tables Cannot Coexist on the Same Nodes**:
   On a set of nodes that participate in a replication cluster, you cannot mix:
   - Local sharded tables (created without a cluster prefix: `create table s ... shards='N' rf='1'`)
   - Clustered sharded tables (created with a cluster prefix: `create table c:r ... shards='N' rf='M'`)

   **Example**: Consider a 2-node cluster where:
   - Table `s` was created independently on each node as a local sharded table
   - Table `r` is replicated across both nodes via the cluster
   
   In this setup, attempting to create a clustered sharded table (`create table c:r ... shards='N' rf='2'`) on the same nodes will fail.

2. **Cluster Name Consistency**:
   - The sharding subsystem binds to a single cluster name on first use; all clustered sharded tables on these nodes must use that same cluster name.
   - Once the cluster name is chosen, it applies to all subsequent clustered sharded table creations.
   
   **Example**: If you create your first clustered sharded table with:
   ```sql
   create table c:users ... shards='N' rf='M'
   ```
   All subsequent clustered sharded tables must reuse cluster `c`:
   ```sql
   create table c:orders ... shards='N' rf='M'  -- works
   create table d:items  ... shards='N' rf='M'  -- fails
   ```

3. **No Table Alterations**:
   - Once a sharded table is created, its structure cannot be modified with `ALTER TABLE`.
   - To change the schema, you must:
     1. Create a new sharded table with the desired structure
     2. Copy the data to the new table
     3. Drop the old table

4. **Shard Count Limit**:
   - Maximum of 3,000 shards per sharded table.
   - This limit applies regardless of cluster configuration or table size.
   - Plan your sharding strategy accordingly to stay within this limit.

5. **`rf` and `shards` Must Be Quoted Integers**:
   - Both options require a quoted numeric value (`shards='10'`, `rf='2'`); unquoted, non-numeric, empty, or fractional values are rejected.
   - On a standalone (non-clustered) server, `rf` must be `'1'`.
   - On a replication cluster, `rf` must be between `1` and the number of nodes in the cluster.

<!-- proofread -->

