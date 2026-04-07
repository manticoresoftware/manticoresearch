# 只读模式

连接的只读模式禁用任何表或全局修改。因此，`create`、`drop`、各种类型的`alter`、`attach`、`optimize`以及诸如`insert`、`replace`、`delete`、`update`等数据修改查询都会被拒绝。在此模式下，也无法使用`SET GLOBAL`更改守护进程范围的设置。

但是，您仍然可以执行所有搜索操作，生成摘要，并运行`CALL PQ`查询。此外，您可以修改本地（连接范围内）的设置。

要检查当前连接是否为只读，请执行`show variables like 'session_read_only'`语句。值为`1`表示只读，值为`0`表示非只读（正常）。

## 启用

只读模式可以在三个不同级别启用：

1. 每个监听器，通过在 [listen](../Server_settings/Searchd.md#listen) 指令后添加后缀 `_readonly`
2. 每个会话，通过执行 `SET ro=1`
3. 守护进程范围，通过在 `searchd` 部分设置 [`read_only = 1`](../Server_settings/Searchd.md#read_only)

当启用 `searchd.read_only=1` 时，所有新的客户端连接会自动以只读模式启动。此模式适用于 Manticore 不得修改表存储的部署场景，例如从只读文件系统提供普通表时。在此模式下，修改查询会被拒绝的方式与在 `_readonly` 监听器上相同。

## 禁用

如果您连接的是[VIP](../Server_settings/Searchd.md#listen)套接字，可以执行`SET ro=0`（即使您连接的套接字在配置中被定义为只读且不是交互式设置）。这将把连接切换回通常（非只读）模式，允许所有修改操作。

对于标准（非VIP）连接，只有在交互式设置的只读模式下，重新连接才能退出只读模式；或者通过更新配置文件并重启守护进程来退出。

如果通过 `searchd.read_only=1` 在守护进程范围启用了只读模式，即使在 VIP 连接上也不允许执行 `SET ro=0`。要退出守护进程范围的只读模式，请在配置中禁用 `read_only` 并重新启动守护进程。

<!-- proofread -->

