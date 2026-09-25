# PURGE

`PURGE` 会移除与表关联的指定数据。清理目标用于指定要移除的内容。

```sql
PURGE <target> FROM TABLE <table_name>;
```

目前唯一支持的目标是 `BULK_IMPORT`。

## PURGE BULK_IMPORT

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE BULK_IMPORT` 会移除 [bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import) 使用的整个暂存根目录，包括因加载中断而留下的目录和文件。请仅在确认该表没有正在进行的直接写盘加载后再运行此语句。

该语句要求目标是一个已存在的本地实时表，且不在复制集群中。它只会移除直接写盘的暂存状态，不会更改表结构或已索引的文档。如果暂存根目录不存在，该语句会成功执行且不做任何更改。在无配置模式下，`DROP TABLE` 也会移除该表的直接写盘暂存根目录。

<!-- proofread -->
