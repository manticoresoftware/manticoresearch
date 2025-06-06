# Table Sharding: Current Limitations

When working with sharded tables, be aware of the following limitations:

1. Within the same cluster, you cannot combine:
   - Locally sharded tables (created with `shards=N` on a standalone server)
   - Clustered sharded tables (created with `shards=N` on a replicated table)

   **Example**: Consider a 2-node cluster where:
   - Table "s" is created independently on each node with local sharding (`create table s ... shards=N`)
   - Table "r" is replicated across both nodes
   
   In this setup, attempting to create a sharded version of the replicated table (`create table c:r ... shards=N`) will fail.

2. **Cluster Name Consistency**:
   - You must use the same cluster name for all sharded tables
   - The cluster name, once chosen, applies to all subsequent sharded table creations
   
   **Example**: If you create your first sharded table with:
   ```sql
   create table c:users ... shards=N
   ```
   All subsequent sharded tables must use the same cluster name 'c':
   ```sql
   create table c:orders ... shards=N  -- This works
   create table d:items ... shards=N   -- This will fail
   ```

3. **No Table Alterations**:
   - Once a sharded table is created, its structure cannot be modified
   - To change any aspect of the table (columns, indexes, etc.), you must:
     1. Create a new table with the desired structure
     2. Copy the data to the new table
     3. Delete the old table

4. **Shard Count Limit**:
   - Maximum of 3,000 shards per auto-sharded table
   - This limit applies regardless of cluster configuration or table size
   - Plan your sharding strategy accordingly to stay within this limit

<!-- proofread -->

