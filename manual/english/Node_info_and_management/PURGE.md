# PURGE

`PURGE` removes selected data associated with a table. The purge target specifies what is removed.

```sql
PURGE <target> FROM TABLE <table_name>;
```

The only target currently supported is `INDEXER_RT_BULK`.

## PURGE INDEXER_RT_BULK

```sql
PURGE INDEXER_RT_BULK FROM TABLE products;
```

`PURGE INDEXER_RT_BULK` removes the entire staging root used by [indexer-assisted bulk insertion](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Indexer-assisted-bulk-insertion), including directories and files left by an interrupted load. Run it only after confirming that no assisted load is active for the table.

The statement requires an existing local real-time table that is not in a replication cluster. It removes only assisted-loading staging state and does not change the table schema or indexed documents. If the staging root is absent, the statement succeeds without making any changes. In configless mode, `DROP TABLE` also removes the table's assisted staging root.

<!-- proofread -->
