# Upgrade from an older version

Upgrading your Manticore Search instance requires careful planning, especially when dealing with a distributed environment.

### Upgrade order in a distributed environment

1. **Upgrade Agents First**: It's crucial to upgrade agent nodes before master nodes in a distributed setup. This is because upgraded agents can handle requests from both old and new API versions. However, if a master is upgraded first, it might send requests in the new API format to old agents, leading to errors as old agents cannot understand the new API format.

2. **Handling old versions**: For very old versions, consider upgrading one agent at a time. Ensure it processes requests correctly before proceeding. Check the [changelog](../Changelog.md) for any breaking changes in the API protocol, especially for distributed indexes.

3. **Indexer node upgrade**: Upgrade the node with the indexer last. This ensures the newly created indexes are compatible with both old and new daemons.

### Considerations for tables

- **Table Compatibility**: Newer versions of Manticore Search are capable of reading tables in older formats. However, older versions might not be able to load tables created by newer versions, nor can they restore backups made with newer versions. Therefore, it is essential to consult the [changelog](../Changelog.md) for notes on table format changes and to back up your data before attempting an upgrade.

### Upgrading in cluster environments

- **Replication cluster upgrade strategies**: When upgrading nodes in a replication cluster, consider the replication protocol and index version changes (check the [changelog](../Changelog.md)).

    - **Segmented upgrade**: Use the [gmcast.segment=20](https://galeracluster.com/library/documentation/galera-parameters.html#gmcast-segment) option with Galera to create a new segment for upgraded nodes. This prevents upgraded nodes (donors) from sending new indexes to nodes running old daemons (joiners), which could lead to compatibility issues.
    - **Full shutdown upgrade**: Alternatively, perform a full shutdown and upgrade all nodes simultaneously. This ensures consistency across the cluster during the upgrade process.

### General Advice

Always back up your data before attempting an upgrade. Consult the [Changelog](https://manual.manticoresearch.com/Changelog#Upgrade) for specific details related to each version upgrade. For additional support or queries, visit our [support page](https://manual.manticoresearch.com/Support).
