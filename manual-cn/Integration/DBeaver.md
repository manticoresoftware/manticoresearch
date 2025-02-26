# 与 DBeaver 集成

> 注意：与 DBeaver 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果它不起作用，请确保已安装 Buddy。

[DBeaver](https://dbeaver.io/) 是一个 SQL 客户端软件应用程序和数据库管理工具。对于 MySQL 数据库，它使用 JDBC 应用程序编程接口通过 JDBC 驱动程序与其交互。

Manticore 允许您使用 DBeaver 来处理存储在 Manticore 表中的数据，就像它存储在 MySQL 数据库中一样。

## 使用的设置

要在 DBeaver 中开始使用 Manticore，请按照以下步骤操作：

- 在 DBeaver 的用户界面中选择 `新建数据库连接`
- 选择 `SQL` -> `MySQL` 作为 DBeaver 的数据库驱动程序
- 设置 `服务器主机` 和 `端口` 选项，对应于您的 Manticore 实例的主机和端口（保持 `数据库` 字段为空）
- 设置 `root/<空密码>` 作为身份验证凭据


## 可用的功能

由于 Manticore 并不完全支持 MySQL，因此在使用 Manticore 时，只有部分 DBeaver 的功能可用。

您将能够：

- 查看、创建、删除和重命名表
- 添加和删除表列
- 插入、删除和更新列数据

您将无法：

- 使用数据库完整性检查机制（`MyISAM` 将被设置为唯一可用的存储引擎）
- 使用 MySQL 的过程、触发器、事件等
- 管理数据库用户
- 设置其他数据库管理选项


## 数据类型处理

Manticore 目前不支持某些 MySQL 数据类型，因此在使用 DBeaver 创建新表时无法使用这些类型。此外，部分支持的数据类型将转换为与 Manticore 类型最相似的类型，在此类转换中将忽略类型精度。以下是支持的 MySQL 数据类型及其映射到的 Manticore 类型列表：

- `BIGINT UNSIGNED` => `bigint`
- `BOOL` => `boolean`
- `DATE`, `DATETIME`, `TIMESTAMP`  => `timestamp`
- `FLOAT` => `float`
- `INT` => `int`
- `INT UNSIGNED`, `SMALLINT UNSIGNED`, `TINYINT UNSIGNED`, `BIT` => `uint`
- `JSON` => `json`
- `TEXT`, `LONGTEXT`, `MEDIUMTEXT`, `TINYTEXT`, `BLOB`, `LONGBLOB`, `MEDIUMBLOB`, `TINYBLOB`  => `text`
- `VARCHAR`, `LONG VARCHAR`, `BINARY`, `CHAR`, `VARBINARY`, `LONG VARBINARY`  => `string`

您可以在[此处](../Creating_a_table/Data_types.md#数据类型)找到有关 Manticore 数据类型的更多详细信息。

### 关于日期类型

Manticore 能够处理 `DATE`、`DATETIME` 和 `TIMESTAMP` 数据类型，但这需要启用 Manticore 的 [Buddy](../Starting_the_server/Docker.md#Manticore-Columnar-Library-和-Manticore-Buddy)。否则，尝试操作这些类型中的任意一个将导致错误。

注意，不支持 `TIME` 类型。

## 可能的注意事项

DBeaver 的 `首选项` -> `连接` -> `客户端识别` 选项不能关闭或覆盖。 为了与 DBeaver 正常工作，Manticore 需要区分其请求与其他请求。为此，它使用 DBeaver 在请求标头中发送的客户端通知信息。禁用客户端通知将破坏该检测，从而影响 Manticore 的正常功能。

第一次尝试更新表中的数据时，您将看到 `无唯一键` 弹出消息，并要求您定义自定义唯一键。 当收到此消息时，请执行以下步骤：

- 选择 `自定义唯一键` 选项
- 在列列表中仅选择 `id` 列
- 点击 `确定`

之后，您将能够安全地更新您的数据。
