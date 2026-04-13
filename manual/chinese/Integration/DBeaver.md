# Manticore与DBeaver的集成

> NOTE: 与DBeaver的集成需要[Manticore Buddy](../Installation/Manticore_Buddy.md)。如果不起作用，请确保已安装Buddy。

[DBeaver](https://dbeaver.io/) 是一个SQL客户端软件应用程序和数据库管理工具。对于MySQL数据库，它通过JDBC驱动程序使用JDBC应用程序编程接口与它们进行交互。

Manticore允许您使用DBeaver与存储在Manticore表中的数据进行操作，就像这些数据存储在MySQL数据库中一样。目前，已测试并推荐使用版本25.2.0。其他版本可能也能工作，但可能会引入问题。

## 使用的设置

要开始使用DBeaver中的Manticore，请按照以下步骤操作：

- 在DBeaver的UI中选择`新建数据库连接`选项
- 选择`SQL` -> `MySQL`作为DBeaver的数据库驱动
- 设置`服务器主机`和`端口`选项，对应于您的Manticore实例的主机和端口（保持`数据库`字段为空）
- 设置`root/<空密码>`作为身份验证凭据


## 可用的功能

由于Manticore不完全支持MySQL，因此在使用Manticore时，DBeaver的部分功能是不可用的。

您可以：
- 查看、创建、删除和重命名表
- 添加和删除表列
- 插入、删除和更新列数据

您无法：
- 使用数据库完整性检查机制（`MyISAM`将是唯一可用的存储引擎）
- 使用MySQL存储过程、触发器、事件等
- 管理数据库用户
- 设置其他数据库管理选项


## 数据类型处理

一些MySQL数据类型目前不被Manticore支持，因此在使用DBeaver创建新表时无法使用。此外，支持的一些数据类型在转换时会被转换为最接近的Manticore类型，类型精度在转换时会被忽略。以下是MySQL数据类型及其映射到的Manticore类型列表：

- `BIGINT UNSIGNED` => `bigint`
- `BOOL` => `boolean`
- `DATE`, `DATETIME`, `TIMESTAMP` => `timestamp`
- `FLOAT` => `float`
- `INT` => `int`
- `INT UNSIGNED`, `SMALLINT UNSIGNED`, `TINYINT UNSIGNED`, `BIT` => `uint`
- `JSON` => `json`
- `TEXT`, `LONGTEXT`, `MEDIUMTEXT`, `TINYTEXT`, `BLOB`, `LONGBLOB`, `MEDIUMBLOB`, `TINYBLOB` => `text`
- `VARCHAR`, `LONG VARCHAR`, `BINARY`, `CHAR`, `VARBINARY`, `LONG VARBINARY` => `string`

您可以在此处找到更多关于Manticore数据类型的详细信息 [这里](../Creating_a_table/Data_types.md#Data-types)。

### 关于日期类型

Manticore能够处理`DATE`、`DATETIME`和`TIMESTAMP`数据类型，但需要Manticore的[Buddy](../Starting_the_server/Docker.md#Manticore-Columnar-Library-and-Manticore-Buddy)启用。否则，尝试操作这些类型之一将导致错误。

请注意，`TIME`类型不受支持。

## 可能的问题

- DBeaver的`首选项` -> `连接` -> `客户端标识`选项必须不被关闭或覆盖。
  为了正确使用DBeaver，Manticore需要能够区分其请求与其他请求。为此，它使用DBeaver在请求头中发送的客户端通知信息。禁用客户端通知将破坏这种检测，从而影响Manticore的正确功能。

- 当第一次尝试更新表中的数据时，您将看到`无唯一键`弹出消息，并被要求定义一个自定义唯一键。
  当您收到此消息时，请执行以下步骤：

  - 选择`自定义唯一键`选项
  - 在列列表中仅选择`id`列
  - 点击`确定`

之后，您将能够安全地更新您的数据。

