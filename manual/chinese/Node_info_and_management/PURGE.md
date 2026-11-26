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

该语句要求存在本地实时表、分片表或分布式表。它只会移除直接写入磁盘的暂存状态，不会更改表结构或已索引的文档。如果暂存根目录不存在，该语句也会成功且不做任何更改，因此可以安全地重复执行。在无配置模式下，`DROP TABLE` 还会移除该表的直接写入磁盘暂存根目录，并取消其 Shared pool。

### 复制集群中的表

对于复制集群中的表，该语句只清理执行它的节点。当该节点保留表的[批量导入块](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Bulk-import-into-a-replicated-table)以供 attach 命令使用时，语句会因 `a bulk_import chunk of table '<table>' is being replicated` 而被拒绝：

* 如果集群中的任一节点正在提交对该表的批量导入，请在提交完成后重新执行该语句。此时不要使用 `OPTION clear_sessions=1`：它会移除该节点上的块副本，导致节点在命令到达时无法 attach 该块，并使其离开集群。
* 如果当前没有对该表执行批量导入，则说明该块是遗留数据，例如执行加载的节点在命令执行前停止，或该节点当时不在集群中。请使用 `OPTION clear_sessions=1` 移除该块，或重启守护进程后重新执行该语句。

### 分片表和分布式表

对于分片表或分布式表，该语句会清理以下内容：

1. 该节点负责协调的[分片表的 Shared 批量导入](../Data_creation_and_modification/Adding_documents_to_a_table/Adding_documents_to_a_real-time_table.md#Shared-bulk-import-into-a-sharded-table)。如果不使用该选项，打开或失败的加载会导致语句被拒绝，并且不会清理任何内容。正在发布的加载也会导致相同结果；请在提交完成后重新执行该语句。
2. 分片表或分布式表的每个本地实时表；其他本地表会被跳过。
3. 每个代理上的每个表，以及每个镜像上的相应表，并对它们执行相同的语句。

使用 `OPTION clear_sessions=1` 时，它会取消这些表的每个打开的 Shared pool，而不仅是某次加载的 pool。如果协调某次加载的节点停止或崩溃，请在任意拥有该分片表的节点上使用此选项执行该语句。请确保这些表中没有任何私有批量导入正在进行。

如果部分表清理失败，其他表仍会被清理。错误信息会列出每个失败的表或无法访问的代理及其原因：

* `table '<shard>' is a target of an open shared bulk_import into a shard table`：该分片仍保留某次加载的 pool。请在负责协调该加载的节点上提交加载，或使用 `OPTION clear_sessions=1` 取消加载。
* 无法访问的代理：请在其节点恢复后重新执行该语句。
* 其他错误：按照单表错误处理，参见上文。

<!-- proofread -->
