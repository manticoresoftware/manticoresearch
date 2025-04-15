# 只读模式

只读模式下，连接禁用任何表或全局修改。因此，像 `create`、`drop`、各种类型的 `alter`、`attach`、`optimize` 以及数据修改查询如 `insert`、`replace`、`delete`、`update` 等都将被拒绝。在此模式下，使用 `SET GLOBAL` 更改守护程序全局设置也是不可能的。

然而，您仍然可以执行所有搜索操作，生成片段，并运行 `CALL PQ` 查询。此外，您可以修改本地（连接范围内）设置。

要检查当前连接是否为只读，请执行 `show variables like 'session_read_only'` 语句。值为 `1` 表示只读，而 `0` 表示不是只读（通常模式）。

## 激活

通常，您通过在只读模式中将后缀 `_readonly` 添加到单独的 [listen](../Server_settings/Searchd.md#listen) 指令来定义。但您也可以通过执行 SQL 语句 `SET ro=1` 以交互方式为当前连接执行此操作。

## 禁用

如果您连接到 [VIP](../Server_settings/Searchd.md#listen) 套接字，您可以执行 `SET ro=0`（即使您连接的套接字在配置中被定义为只读而不是交互式）。这样将把连接切换回通常模式（非只读），允许所有修改。

对于标准（非 VIP）连接，如果只读模式是通过交互方式设置的，只有通过重新连接才能退出只读模式，或通过更新配置文件并重新启动守护程序。

<!-- proofread -->
