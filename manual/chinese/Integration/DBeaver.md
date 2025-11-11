# 与 DBeaver 的集成

> 注意：与 DBeaver 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法正常工作，请确保已安装 Buddy。

[DBeaver](https://dbeaver.io/) 是一款 SQL 客户端软件应用程序和数据库管理工具。对于 MySQL 数据库，它通过 JDBC 驱动程序应用 JDBC 应用程序编程接口进行交互。

Manticore 允许您使用 DBeaver 以与操作 MySQL 数据库相同的方式处理存储在 Manticore 表中的数据。目前，推荐使用并测试过的版本是 25.2.0。其他版本可能可用，但可能会引入问题。

## 使用的设置

要开始在 DBeaver 中使用 Manticore，请按照以下步骤操作：

- 在 DBeaver 的界面中选择 `New database connection` 选项
- 选择 `SQL` -> `MySQL` 作为 DBeaver 的数据库驱动程序
- 设置 `Server host` 和 `Port` 选项，对应您的 Manticore 实例的主机和端口（保持 `database` 字段为空）
- 设置认证凭据为 `root/<empty password>`


## 可用功能

由于 Manticore 并不完全支持 MySQL，因此在使用 Manticore 时，DBeaver 的部分功能可用。

您将能够：
- 查看、创建、删除和重命名表
- 添加和删除表列
- 插入、删除和更新列数据

您将无法：
- 使用数据库完整性检查机制（`MyISAM` 将被设置为唯一可用的存储引擎）
- 使用 MySQL 存储过程、触发器、事件等
- 管理数据库用户
- 设置其他数据库管理选项


## 数据类型处理

某些 MySQL 数据类型当前不被 Manticore 支持，因此在使用 DBeaver 创建新表时无法使用。此外，一些支持的数据类型会被转换为最相似的 Manticore 类型，转换时会忽略类型精度。以下是支持的 MySQL 数据类型及其映射到的 Manticore 类型列表：

- `BIGINT UNSIGNED` => `bigint`
- `BOOL` => `boolean`
- `DATE`、`DATETIME`、`TIMESTAMP`  => `timestamp`
- `FLOAT` => `float`
- `INT` => `int`
- `INT UNSIGNED`、`SMALLINT UNSIGNED`、`TINYINT UNSIGNED`、`BIT` => `uint`
- `JSON` => `json`
- `TEXT`、`LONGTEXT`、`MEDIUMTEXT`、`TINYTEXT`、`BLOB`、`LONGBLOB`、`MEDIUMBLOB`、`TINYBLOB`  => `text`
- `VARCHAR`、`LONG VARCHAR`、`BINARY`、`CHAR`、`VARBINARY`、`LONG VARBINARY`  => `string`

您可以在[这里](../Creating_a_table/Data_types.md#Data-types)找到有关 Manticore 数据类型的更多详细信息。

### 关于日期类型

Manticore 能够处理 `DATE`、`DATETIME` 和 `TIMESTAMP` 数据类型，但这需要启用 Manticore 的 [Buddy](../Starting_the_server/Docker.md#Manticore-Columnar-Library-and-Manticore-Buddy)。否则，尝试操作这些类型将导致错误。

请注意，`TIME` 类型不被支持。

## 可能的注意事项

- DBeaver 的 `Preferences` -> `Connections` -> `Client identification` 选项不得关闭或被覆盖。
  为了正确与 DBeaver 配合工作，Manticore 需要区分其请求与其他请求。为此，它使用 DBeaver 在请求头中发送的客户端通知信息。禁用客户端通知将破坏该检测，从而影响 Manticore 的正常功能。

- 当您第一次尝试更新表中的数据时，会看到 `No unique key` 弹出消息，并被要求定义自定义唯一键。
  当您看到此消息时，请执行以下步骤：

  - 选择 `Custom Unique Key` 选项
  - 在列列表中仅选择 `id` 列
  - 点击 `Ok`

 之后，您将能够安全地更新数据。

