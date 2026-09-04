# PURGE

`PURGE` 用于删除与表关联的选定数据。purge 目标指定要删除的内容。

```sql
PURGE <target> FROM TABLE <table_name>;
```

目前唯一支持的目标是 `BULK_IMPORT`。

## PURGE BULK_IMPORT

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE BULK_IMPORT` 会删除 [bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import) 使用的整个 staging 根目录，包括中断加载遗留的目录和文件。只有在确认该表没有正在进行 direct-to-disk 加载后，才运行它。

该语句要求目标是一个已存在的本地实时表，且不处于 replication cluster 中。它只会移除 direct-to-disk 的 staging 状态，不会更改表结构或已索引的文档。如果 staging 根目录不存在，该语句也会成功执行且不做任何更改。在 configless 模式下，`DROP TABLE` 也会删除该表的 direct-to-disk staging 根目录。

<!-- proofread -->
