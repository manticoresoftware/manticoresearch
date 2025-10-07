# 与 DBeaver 的集成

> 注意：与 DBeaver 的集成需要 [Manticore Buddy](../Installation/Manticore_Buddy.md)。如果无法使用，请确保已安装 Buddy。

[DBeaver](https://dbeaver.io/) 是一个 SQL 客户端软件应用和数据库管理工具。对于 MySQL 数据库，它通过 JDBC 驱动应用 JDBC 应用程序接口与数据库交互。

Manticore 允许您使用 DBeaver 来处理存储在 Manticore 表中的数据，就像处理存储在 MySQL 数据库中的数据一样。目前，推荐使用经过测试的 25.2.0 版本。其他版本可能可用，但可能会引入问题。

## 使用的设置

要开始在 DBeaver 中使用 Manticore，请按照以下步骤操作：

- 在 DBeaver 界面中选择 `New database connection` 选项
- 选择 `SQL` -> `MySQL` 作为 DBeaver 的数据库驱动
- 设置对应于 Manticore 实例主机和端口的 `Server host` 和 `Port` 选项（`database` 字段保持为空）
- 设置身份验证凭据为 `root/<empty password>`


## 可用功能

由于 Manticore 并不完全支持 MySQL，因此在使用 Manticore 时仅能使用 DBeaver 的部分功能。

您可以：
- 查看、创建、删除和重命名表
- 添加和删除表列
- 插入、删除和更新列数据

您无法：
- 使用数据库完整性检查机制（仅支持 `MyISAM` 作为存储引擎）
- 使用 MySQL 存储过程、触发器、事件等功能
- 管理数据库用户
- 设置其他数据库管理选项


## 数据类型处理

某些 MySQL 数据类型当前不被 Manticore 支持，因此无法在 DBeaver 中创建新表时使用。此外，部分支持的数据类型会被转换为最相似的 Manticore 类型，转换时会忽略类型精度。下表列出了支持的 MySQL 数据类型及其映射的 Manticore 类型：

- `BIGINT UNSIGNED` => `bigint`
- `BOOL` => `boolean`
- `DATE`、`DATETIME`、`TIMESTAMP`  => `timestamp`
- `FLOAT` => `float`
- `INT` => `int`
- `INT UNSIGNED`、`SMALLINT UNSIGNED`、`TINYINT UNSIGNED`、`BIT` => `uint`
- `JSON` => `json`
- `TEXT`、`LONGTEXT`、`MEDIUMTEXT`、`TINYTEXT`、`BLOB`、`LONGBLOB`、`MEDIUMBLOB`、`TINYBLOB`  => `text`
- `VARCHAR`、`LONG VARCHAR`、`BINARY`、`CHAR`、`VARBINARY`、`LONG VARBINARY`  => `string`

您可以在[这里](../Creating_a_table/Data_types.md#Data-types)找到关于 Manticore 数据类型的更多详情。

### 关于日期类型

Manticore 能处理 `DATE`、`DATETIME` 和 `TIMESTAMP` 数据类型，但这需要启用 Manticore 的[Buddy](../Starting_the_server/Docker.md#Manticore-Columnar-Library-and-Manticore-Buddy)。否则，尝试操作这些类型将导致错误。

注意，`TIME` 类型不被支持。

## 可能的问题

- 不能关闭或覆盖 DBeaver 的 `Preferences` -> `Connections` -> `Client identification` 选项。
  为了能正确与 DBeaver 配合，Manticore 需要区分其请求与其他请求。为此，它利用 DBeaver 在请求头中发送的客户端通知信息。关闭客户端通知将导致检测失败，从而影响 Manticore 的正常功能。

- 第一次尝试更新表中的数据时，会出现 `No unique key` 弹窗，要求定义自定义唯一键。
  收到此提示时，请执行以下步骤：

  - 选择 `Custom Unique Key` 选项
  - 在列列表中只选中 `id` 列
  - 点击 `Ok`

  完成后，您即可安全更新数据。

