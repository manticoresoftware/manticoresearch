# 只读模式

连接的只读模式禁用任何表或全局修改。因此，`create`、`drop`、各种类型的`alter`、`attach`、`optimize`以及诸如`insert`、`replace`、`delete`、`update`等数据修改查询都会被拒绝。在此模式下，也无法使用`SET GLOBAL`更改守护进程范围的设置。

但是，您仍然可以执行所有搜索操作，生成摘要，并运行`CALL PQ`查询。此外，您可以修改本地（连接范围内）的设置。

要检查当前连接是否为只读，请执行`show variables like 'session_read_only'`语句。值为`1`表示只读，值为`0`表示非只读（正常）。

## 启用

通常，您可以通过在[listen](../Server_settings/Searchd.md#listen)指令中添加后缀`_readonly`来定义一个独立的只读模式监听端口。然而，也可以通过执行SQL语句`SET ro=1`，为当前连接交互式启用只读模式。

## 禁用

如果您连接的是[VIP](../Server_settings/Searchd.md#listen)套接字，可以执行`SET ro=0`（即使您连接的套接字在配置中被定义为只读且不是交互式设置）。这将把连接切换回通常（非只读）模式，允许所有修改操作。

对于标准（非VIP）连接，只有在交互式设置的只读模式下，重新连接才能退出只读模式；或者通过更新配置文件并重启守护进程来退出。

<!-- proofread -->

