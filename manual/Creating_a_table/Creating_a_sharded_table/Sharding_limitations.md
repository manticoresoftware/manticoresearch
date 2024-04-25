# Sharding: limitations

When using sharded tables in Manticore Search, there are a few limitations to keep in mind:

1. **Consistency across nodes**: Do not mix the process of creating sharded local tables on one server with the creation of replicated sharded tables on another within the same cluster. Always use the same server for all interactions and queries related to a particular sharded table. This practice avoids issues related to shard mapping initialization, which can occur when replication overwrites data during cluster table creation.

2. **Requirement for Manticore Buddy**: The auto-sharding feature requires Manticore Buddy to be active. Manticore Buddy is installed automatically if you follow the official installation instructions. Ensure that it is not disabled if you need auto-sharding.

3. **Single Cluster Name**: Use only one cluster name consistently for all your sharded table operations. If you start with a cluster named 'c', continue using 'c' for all future sharded table creations. Using different names for new tables will result in errors.

4. **Manual Deletion of Tables**: If you need to remove a sharded table, you must do so manually. Automatic deletion of sharded tables is not supported at this time.

5. **Table Modifications**: You are not able to alter the table. The way to change it is to delete the old one and create a new one manually.


<!-- proofread -->
