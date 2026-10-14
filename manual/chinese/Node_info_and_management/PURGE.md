# PURGE

`PURGE` 会移除与表关联的指定数据。清理目标用于指定要移除的内容。

```sql
PURGE <target> FROM TABLE <table_name>;
```

目前唯一支持的目标是 `BULK_IMPORT`。

## PURGE BULK_IMPORT

```sql
PURGE BULK_IMPORT FROM TABLE products [OPTION clear_sessions=0|1];
```

`PURGE BULK_IMPORT` 会移除 [bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import) 使用的整个暂存根目录，包括因加载中断而留下的目录和文件。请仅在确认该表没有正在进行的直接写盘加载后再运行此语句。

如果表存在打开或失败的 [Shared HTTP bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-HTTP-bulk-import) 池，该语句会被拒绝。使用 `OPTION clear_sessions=1` 时，会先取消该池：未发布的数据会被丢弃，仍在向该池写入的请求会收到错误，其索引器会停止，表会释放以供普通写入，然后移除暂存根目录。之前已发布的分块会保留在表中。当该池在提交后正在附加分块时，该语句会被拒绝；请在提交完成后重试。在 Windows 上，如果某个请求仍在向该池写入，该语句在取消池后立即移除暂存根目录时可能失败，因为已停止的索引器会短暂保持文件打开；请重试。

打开的池会让表保持预留状态，因此对该表的普通写入和其他批量导入都会失败，即使所有客户端都已离开；[`SHOW LOCKS`](../Securing_and_compacting_a_table/Freezing_and_locking_a_table.md#SHOW-LOCKS) 会显示这类表的锁类型为 `write_allow_readers`。如果客户端在提交前停止或崩溃，以下操作会释放该表并丢弃它们暂存的内容：

```sql
PURGE BULK_IMPORT FROM TABLE products OPTION clear_sessions=1;
```

如果要发布已暂存的文档，请按 [Shared HTTP bulk import](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-HTTP-bulk-import) 中所述，使用不带文档的请求提交该池。

`OPTION clear_sessions=1` 不会取消私有批量导入会话：私有会话会通过 `SET bulk_import=0` 或在其连接关闭时释放表。

该语句要求存在一个本地实时表，且该表不在复制集群中。它只移除 direct-to-disk 暂存状态，不会更改表结构或已索引文档。如果暂存根目录不存在，该语句会成功执行且不做任何更改。在无配置模式下，`DROP TABLE` 也会移除该表的 direct-to-disk 暂存根目录，并取消其 Shared 池。

<!-- proofread -->
