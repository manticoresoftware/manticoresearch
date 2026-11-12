# PURGE

`PURGE` removes selected data associated with a table. The purge target specifies what is removed.

```sql
PURGE <target> FROM TABLE <table_name>;
```

The only target currently supported is `BULK_IMPORT`.

## PURGE BULK_IMPORT

```sql
PURGE BULK_IMPORT FROM TABLE products [OPTION clear_sessions=0|1];
```

`PURGE BULK_IMPORT` removes the entire staging root used by [bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import), including directories and files left by an interrupted load. Run it only after confirming that no direct-to-disk load is active for the table.

If the table has an open or failed [Shared HTTP bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-HTTP-bulk-import) pool, the statement is refused. With `OPTION clear_sessions=1` it cancels that pool first: unpublished data is discarded, requests still writing to the pool get an error, its indexers are stopped, the table is released for ordinary writes, and then the staging root is removed. Chunks published earlier stay in the table. While the pool is attaching its chunks after a commit, the statement is refused; repeat it after the commit finishes. On Windows, while a request is still writing to the pool, the statement can fail to remove the staging root right after it cancels the pool, because the stopped indexers keep their files open for a moment; repeat it.

An open pool keeps the table reserved, so ordinary writes to it and other bulk imports of it fail, even when all its clients are gone; [`SHOW LOCKS`](../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS) shows such a table with the lock type `write_allow_readers`. If the clients stopped or crashed before their commit, this releases the table and discards what they staged:

```sql
PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1;
```

To publish the staged documents instead, commit the pool with a request without documents, as described in [Shared HTTP bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-HTTP-bulk-import).

`OPTION clear_sessions=1` does not cancel private bulk import sessions: a private session releases the table with `SET bulk_import=0` or when its connection closes.

The statement requires an existing local real-time table, or a sharded or distributed table. For a table in a replication cluster it purges only the node where it runs; it is refused while a [bulk import chunk of the table](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import-into-a-replicated-table) waits for its attach command on that node, and `OPTION clear_sessions=1` removes such a chunk anyway. A node that missed the attach command of a chunk it received, for example because it was out of the cluster at that moment, keeps the chunk after the load has ended and refuses `PURGE` the same way until `OPTION clear_sessions=1` or a restart of the daemon. It removes only direct-to-disk staging state and does not change the table schema or indexed documents. If the staging root is absent, the statement succeeds without making any changes. In configless mode, `DROP TABLE` also removes the table's direct-to-disk staging root and cancels its Shared pool.

### Sharded and distributed tables

For a sharded or distributed table, the statement first handles the [Shared bulk import into the sharded table](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-bulk-import-into-a-sharded-table) that this node coordinates: without the option it is refused while that load is open or failed, and with `OPTION clear_sessions=1` it cancels the load; while the load is being published the statement is refused and purges nothing, repeat it after the commit finishes. Then it runs for every local real-time table of the sharded or distributed table (other local tables have no bulk import state and are skipped), and sends the same statement for every table of every agent, to every mirror. Make sure that no private bulk import is active on any of these tables. A table that fails does not stop the others: the statement answers with an error that lists each failed table or unreachable agent, while the others are purged. Repeat it when an unreachable node is back.

Run it on any node that has the sharded table to cancel the shard pools left by a load whose coordinating node stopped or crashed. It cancels any open Shared pool of those tables, not only the pools of one load.

<!-- proofread -->
