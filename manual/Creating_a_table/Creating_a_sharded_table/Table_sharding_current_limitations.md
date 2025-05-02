# Table Sharding: Current Limitations

When creating a sharded table, you should be aware of the current limitations you will face while working with it.

1. Don't create local tables on one server and clustered tables on another in the same cluster. When working with a sharded table, use the same node for all operations. This is because the internal shard mapping is initialized during table creation, and using different nodes can cause replication issues that overwrite data.

2. You can use only one cluster name and cannot make it different for each new clustered table creation. That means if you created a sharded table using the cluster name 'c', you should always use it for future table creations, as other names will not work.

3. You are not able to alter the table. The way to change it is to delete the old one and create a new one manually.

4. There is a maximum limit of 3000 shards per auto-sharded table.

<!-- proofread -->

