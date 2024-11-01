# 创建本地表

在 Manticore Search 中，有两种管理表的方式：
## 在线模式管理（实时模式）
实时模式不需要在配置文件中定义表。然而，`searchd` 部分中的 [data_dir](../Server_settings/Searchd.md#data_dir) 指令是强制性的。索引文件存储在 `data_dir` 目录中。

在这种模式下，仅支持复制。

您可以使用 SQL 命令，如 `CREATE TABLE`、`ALTER TABLE` 和 `DROP TABLE` 来创建和修改表模式，并删除它。该模式特别适用于 **实时表** 和 **监测表**。

创建时表名将转换为小写字母。

## 在配置中定义表结构（普通模式）
在此模式下，您可以在配置文件中指定表的结构。Manticore 在启动时读取此结构，并在表尚不存在时创建该表。该模式特别适用于 **普通表**，这些表使用来自外部存储的数据。

要删除表，可以从配置文件中移除它，或者移除路径设置，然后向服务器发送 HUP 信号或重启它。

在此模式下，表名是区分大小写的。

所有表类型在此模式下均受到支持。


## 表类型和模式

| 表类型                                                       | 实时模式 | 普通模式 |
| ------------------------------------------------------------ | -------- | -------- |
| [实时表](Local_tables\Real-time_table.md)                    | 支持     | 支持     |
| [普通表](Local_tables\Plain_table.md)                        | 不支持   | 支持     |
| [渗透表](Local_tables\Percolate_table.md)                    | 支持     | 支持     |
| [分布式表](Creating_a_distributed_table\Creating_a_distributed_table.md) | 支持     | 支持     |
| [模板表](Local_tables\Template_table.md)                     | 不支持   | 支持     |

<!-- proofread -->
