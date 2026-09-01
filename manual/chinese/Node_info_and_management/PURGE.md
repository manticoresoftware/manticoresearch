# PURGE

`PURGE` 会删除与表关联的选定数据。清理目标用于指定要删除的内容。

```sql
PURGE <target> FROM TABLE <table_name>;
```

目前唯一支持的目标是 `BULK_IMPORT`。

## PURGE BULK_IMPORT

```sql
PURGE BULK_IMPORT FROM TABLE products;
```

`PURGE BULK_IMPORT` 会删除[批量导入](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import)使用的整个暂存根目录，包括中断加载留下的目录和文件。只有在确认该表没有正在进行直接写入磁盘的加载后才运行它。

该语句要求目标是一个已存在的本地实时表，且不属于复制集群。它只会移除直接写入磁盘的暂存状态，不会更改表结构或已索引文档。如果暂存根目录不存在，语句会成功执行且不做任何更改。在无配置模式下，`DROP TABLE` 也会删除该表的直接写入磁盘暂存根目录。

<!-- proofread -->
