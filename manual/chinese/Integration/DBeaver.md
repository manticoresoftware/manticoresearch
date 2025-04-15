# 与 DBeaver 的集成

> 注意：与 DBeaver 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它无法正常工作，请确保 Buddy 已安装。

[DBeaver](https://dbeaver.io/) 是一个 SQL 客户端软件应用程序和数据库管理工具。对于 MySQL 数据库，它通过 JDBC 驱动与它们进行交互，应用 JDBC 应用程序编程接口。

Manticore 允许您使用 DBeaver 像处理 MySQL 数据库一样处理存储在 Manticore 表中的数据。

## 使用的设置

要开始在 DBeaver 中使用 Manticore，请按照以下步骤操作：

- 在 DBeaver 的 UI 中选择 `新建数据库连接` 选项
- 选择 `SQL` -> `MySQL` 作为 DBeaver 的数据库驱动程序
- 设置与您的 Manticore 实例的主机和端口相对应的 `服务器主机` 和 `端口` 选项（保持 `数据库` 字段为空）
- 设置 `root/<空密码>` 作为认证凭据


## 可用功能

由于 Manticore 并不完全支持 MySQL，因此在使用 Manticore 时，只有 DBeaver 功能的一部分可用。

您将能够：
- 查看、创建、删除和重命名表
- 添加和删除表列
- 插入、删除和更新列数据

您将无法：
- 使用数据库完整性检查机制（`MyISAM` 将设置为唯一可用的存储引擎）
- 使用 MySQL 过程、触发器、事件等
- 管理数据库用户
- 设置其他数据库管理选项


## 数据类型处理

一些 MySQL 数据类型目前不被 Manticore 支持，因此在使用 DBeaver 创建新表时不能使用。此外，几种受支持的数据类型将被转换为最相似的 Manticore 类型，在这种转换中将忽略类型精度。下面是支持的 MySQL 数据类型以及它们映射到的 Manticore 类型的列表：

- `BIGINT UNSIGNED` => `bigint`
- `BOOL` => `boolean`
- `DATE`、`DATETIME`、`TIMESTAMP` => `timestamp`
- `FLOAT` => `float`
- `INT` => `int`
- `INT UNSIGNED`、`SMALLINT UNSIGNED`、`TINYINT UNSIGNED`、`BIT` => `uint`
- `JSON` => `json`
- `TEXT`、`LONGTEXT`、`MEDIUMTEXT`、`TINYTEXT`、`BLOB`、`LONGBLOB`、`MEDIUMBLOB`、`TINYBLOB` => `text`
- `VARCHAR`、`LONG VARCHAR`、`BINARY`、`CHAR`、`VARBINARY`、`LONG VARBINARY` => `string`

您可以在 [这里](Creating_a_table/Data_types.md#Data-types) 找到有关 Manticore 数据类型的更多详细信息。

### 关于日期类型

Manticore 能够处理 `DATE`、`DATETIME` 和 `TIMESTAMP` 数据类型，但是，这需要启用 Manticore 的 [Buddy](Starting_the_server/Docker.md#Manticore-Columnar-Library-and-Manticore-Buddy)。否则，尝试与这些类型之一进行操作将导致错误。

注意，`TIME` 类型不被支持。

## 可能的注意事项

- DBeaver 的 `首选项` -> `连接` -> `客户端标识` 选项必须未关闭或被覆盖。
  为了与 DBeaver 正确协作，Manticore 需要区分其请求与其他请求。为此，它使用 DBeaver 在请求头中发送的客户端通知信息。禁用客户端通知将破坏该检测，因此，Manticore 的正常功能将受到影响。

- 当您第一次尝试更新表中的数据时，将看到 `没有唯一键` 弹出消息，并将被要求定义一个自定义唯一键。
  当您收到此消息时，请执行以下步骤：

  - 选择 `自定义唯一键` 选项
  - 在列列表中仅选择 `id` 列
  - 按 `确定`

  之后，您将能够安全地更新您的数据。
