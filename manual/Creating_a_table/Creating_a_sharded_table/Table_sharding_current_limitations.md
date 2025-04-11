# Manual Table Sharding: Current Limitations

When creating a sharded table, you should be aware of the current limitations you will face while working with it.

1. You are not allowed to mix local table creation on the first server in the cluster and a clustered table on the second cluster. When you create a sharded table, you should try to use the same node you used for creation to interact with and send all queries to it without mixing different nodes. This limitation is due to an issue when we initialize the internal map of the shards once you create a clustered table because of how replication works, it will rewrite all data.

2. You need to have an active and running Buddy to use the auto-sharding feature. It will not work without Buddy running.

3. You can use only one cluster name and cannot make it different for each new clustered table creation. That means if you created a sharded table using the cluster name 'c', you should always use it for future table creations, as other names will not work.

4. You are not able to alter the table. The way to change it is to delete the old one and create a new one manually.

<!-- proofread -->

