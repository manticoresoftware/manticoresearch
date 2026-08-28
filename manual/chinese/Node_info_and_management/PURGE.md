# PURGE

`PURGE` 会删除与表关联的选定数据。清理目标用于指定要删除的内容。

```sql
PURGE <target> FROM TABLE <table_name>;
```

目前唯一支持的目标是 `INDEXER_RT_BULK`。

## PURGE INDEXER_RT_BULK

```sql
PURGE INDEXER_RT_BULK FROM TABLE products;
```

`PURGE INDEXER_RT_BULK` 会删除 [indexer-assisted bulk insertion](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Indexer-assisted-bulk-insertion) 使用的整个暂存根目录，包括中断加载留下的目录和文件。只有在确认该表没有正在进行 assisted load 后才运行它。

该语句要求目标是一个已存在的本地实时表，且不属于 replication cluster。它只会移除 assisted-loading 的暂存状态，不会更改表结构或已索引文档。如果暂存根目录不存在，语句会成功执行且不做任何更改。在 configless 模式下，`DROP TABLE` 也会删除该表的 assisted staging root。

<!-- proofread -->
