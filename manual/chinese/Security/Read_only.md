# 只读模式

连接的只读模式禁用任何表或全局修改。因此，诸如 `create`、`drop`、各种类型的 `alter`、`attach`、`optimize` 以及数据修改查询如 `insert`、`replace`、`delete`、`update` 等所有这些操作将被拒绝。在此模式下，也无法使用 `SET GLOBAL` 更改守护进程范围内的设置。

但是，您仍然可以执行所有搜索操作、生成片段以及运行 `CALL PQ` 查询。此外，您可以修改本地（连接范围内）设置。

要检查当前连接是否为只读，可以执行语句 `show variables like 'session_read_only'`。值为 `1` 表示只读，`0` 表示非只读（通常情况）。

## 激活

通常，您通过在只读模式下为 [listen](../Server_settings/Searchd.md#listen) 指令添加后缀 `_readonly` 来定义一个单独的监听指令。不过，也可以通过 SQL 执行语句 `SET ro=1`，以交互方式为当前连接启用只读模式。

## 关闭

如果您连接的是 [VIP](../Server_settings/Searchd.md#listen) 套接字，可以执行 `SET ro=0`（即使您连接的套接字在配置中定义为只读而非以交互方式设置）。这将切换连接到常规（非只读）模式，允许所有修改操作。

对于标准（非 VIP）连接，如果只读模式是以交互方式设置的，则只能通过重新连接来退出只读模式；如果通过配置文件设置，则需更新配置文件并重启守护进程。

<!-- proofread -->

