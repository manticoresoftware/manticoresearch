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

The statement requires an existing local real-time table, or a sharded or distributed table. It removes only direct-to-disk staging state and does not change the table schema or indexed documents. If the staging root is absent, the statement succeeds without making any changes, so repeating it is safe. In configless mode, `DROP TABLE` also removes the table's direct-to-disk staging root and cancels its Shared pool.

### Tables in a replication cluster

For a table in a replication cluster, the statement purges only the node where it runs. While a [bulk import chunk of the table](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import-into-a-replicated-table) is kept on this node for its attach command, the statement is refused with `a bulk_import chunk of table '<table>' is being replicated`:

* If a bulk import into the table is being committed on any node of the cluster, repeat the statement after the commit finishes. Do not use `OPTION clear_sessions=1` at this moment: it removes this node's copy of the chunk, the node cannot attach the chunk when the command comes, and it leaves the cluster.
* If no bulk import into the table is running, the chunk is left over, for example because the node that ran the load stopped before the command, or this node was out of the cluster at that moment. Remove it with `OPTION clear_sessions=1`, or restart the daemon and repeat the statement.

### Sharded and distributed tables

For a sharded or distributed table, the statement purges:

1. The [Shared bulk import into the sharded table](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-bulk-import-into-a-sharded-table) that this node coordinates. Without the option, an open or failed load makes the statement refuse and purge nothing. A load that is being published does the same; repeat the statement after the commit finishes.
2. Every local real-time table of the sharded or distributed table; other local tables are skipped.
3. Every table of every agent, on every mirror, with the same statement.

With `OPTION clear_sessions=1`, it cancels every open Shared pool of these tables, not only the pools of one load. If the node that coordinated a load stopped or crashed, run it with the option on any node that has the sharded table. Make sure that no private bulk import is active on any of these tables.

If some tables fail, the others are still purged. The error lists each failed table or unreachable agent with its reason:

* `table '<shard>' is a target of an open shared bulk_import into a shard table`: the shard still holds a pool of a load. Commit the load on the node that coordinates it, or cancel it with `OPTION clear_sessions=1`.
* An unreachable agent: repeat the statement when its node is back.
* Any other error: handle it as for a single table, see above.

<!-- proofread -->
