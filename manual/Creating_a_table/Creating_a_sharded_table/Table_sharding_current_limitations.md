# Table Sharding: Current Limitations

When working with sharded tables, be aware of the following limitations:

1. Don't create local tables on one server and clustered tables on another in the same cluster. When working with a sharded table, use the same node for all operations. This is because the internal shard mapping is initialized during table creation, and using different nodes can cause replication issues that overwrite data.

2. You must use the same cluster name for all sharded tables. For example, if you created a sharded table using the cluster name 'c', you must use the same name for all subsequent sharded table creations. Using different cluster names will not work.

3. Table alterations are not supported. To modify a sharded table, you must delete the existing table and create a new one with the desired configuration.

4. **Shard Limit**: There is a maximum limit of 3,000 shards per auto-sharded table.

<!-- proofread -->

