# 只读模式

连接的只读模式会禁用任何表或全局修改。因此，像 `create`、`drop`、各种类型的 `alter`、`attach`、`optimize`，以及数据修改查询如 `insert`、`replace`、`delete`、`update` 等查询都将被拒绝。在此模式下，也无法通过 `SET GLOBAL` 修改守护进程范围的设置。

不过，您仍然可以执行所有搜索操作，生成片段，并运行 `CALL PQ` 查询。此外，您还可以修改本地（连接范围）设置。

要检查当前连接是否为只读模式，请执行 `show variables like 'session_read_only'` 语句。值为 `1` 表示只读，而 `0` 表示非只读（常规）。

## 启用

只读模式可以在三个不同级别启用：

1. 每个监听器，通过在 [listen](../Server_settings/Searchd.md#listen) 指令后添加后缀 `_readonly`
2. 每个会话，通过执行 `SET ro=1`
3. 守护进程范围，通过在 `searchd` 部分设置 [`read_only = 1`](../Server_settings/Searchd.md#read_only)

当启用 `searchd.read_only=1` 时，所有新客户端连接将自动以只读模式启动。此模式适用于 Manticore 不得修改表存储的部署场景，例如从只读文件系统提供普通表时。在此模式下，修改查询将被拒绝的方式与在 `_readonly` 监听器上相同。

守护进程范围的 `read_only` 仅支持提供普通表存储。它不能与 `data_dir` 结合使用，也不能用于声明 RT 或 percolate 表的配置。

## 禁用

如果您连接到 [VIP](../Server_settings/Searchd.md#listen) 套接字，可以执行 `SET ro=0`（即使您连接的套接字在配置中被定义为只读且非交互式）。这将切换连接到常规（非只读）模式，并允许所有修改。

对于标准（非 VIP）连接，如果以交互方式设置了只读模式，则只能通过重新连接来退出只读模式；或者通过更新配置文件并重新启动守护进程来实现。

如果通过 `searchd.read_only=1` 在守护进程范围启用了只读模式，即使在 VIP 连接上也不允许执行 `SET ro=0`。要退出守护进程范围的只读模式，请在配置中禁用 `read_only` 并重新启动守护进程。

<!-- proofread -->

