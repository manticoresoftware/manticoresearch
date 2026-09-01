# PURGE

`PURGE` removes selected data associated with a table. The purge target specifies what is removed.

```sql
PURGE <target> FROM TABLE <table_name>;
```

The only target currently supported is `BULK_IMPORT`.

## PURGE BULK_IMPORT

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE BULK_IMPORT` removes the entire staging root used by [bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import), including directories and files left by an interrupted load. Run it only after confirming that no direct-to-disk load is active for the table.

The statement requires an existing local real-time table that is not in a replication cluster. It removes only direct-to-disk staging state and does not change the table schema or indexed documents. If the staging root is absent, the statement succeeds without making any changes. In configless mode, `DROP TABLE` also removes the table's direct-to-disk staging root.

<!-- proofread -->
