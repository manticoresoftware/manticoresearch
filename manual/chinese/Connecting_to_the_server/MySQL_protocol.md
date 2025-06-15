# MySQL 协议

Manticore Search 使用 MySQL 协议实现了一个 SQL 接口，允许任何 MySQL 库或连接器以及许多 MySQL 客户端连接到 Manticore Search，并像对待 MySQL 服务器一样使用它，而不是 Manticore。

然而，SQL 方言不同，并且只实现了 MySQL 中可用的 SQL 命令或函数的子集。此外，还有专门针对 Manticore Search 的子句和函数，例如用于全文搜索的 `MATCH()` 子句。

Manticore Search 不支持服务器端预处理语句，但可以使用客户端预处理语句。需要注意的是，Manticore 实现了多值 (MVA) 数据类型，这在 MySQL 或实现预处理语句的库中没有等价项。在这些情况下，MVA 值必须在原始查询中构造。

某些 MySQL 客户端/连接器需要用户/密码和/或数据库名称的值。由于 Manticore Search 没有数据库的概念，也没有实现用户访问控制，这些值可以随意设置，因为 Manticore 会简单地忽略它们。

## 配置

**SQL 接口的默认端口是 9306**，且默认启用。

您可以在配置文件的 searchd 部分使用 `listen` 指令配置 MySQL 端口，如下所示：

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
...
}
```

请记住，Manticore 没有用户身份验证，因此请确保 MySQL 端口不会被外部网络上的任何人访问。

### VIP 连接
可以使用单独的 MySQL 端口来执行“VIP”连接。连接到该端口时，将绕过线程池，并且始终创建一个新的专用线程。这在严重过载的情况下非常有用，此时服务器会停滞或阻止通过常规端口的连接。

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
   listen = 127.0.0.1:9307:mysql_vip
...
}
```

## 使用标准 MySQL 客户端连接
连接 Manticore 最简单的方法是使用标准的 MySQL 客户端：

```shell
mysql -P9306 -h0
```

## 安全的 MySQL 连接

MySQL 协议支持[SSL 加密](../Security/SSL.md)。可以在相同的 `mysql` 监听端口上建立安全连接。

## 压缩的 MySQL 连接

MySQL 连接可以使用压缩功能，客户端默认支持此功能。客户端只需指定连接应使用压缩即可。

使用 MySQL 客户端的示例：

```shell
mysql -P9306 -h0 -C
```

压缩可以用于安全和非安全连接。

## 关于 MySQL 连接器的说明
可以使用官方 MySQL 连接器连接到 Manticore Search，但它们可能需要在 DSN 字符串中传递某些设置，因为连接器可能尝试运行 Manticore 尚未实现的某些 SQL 命令。

JDBC 连接器 6.x 及以上版本需要 Manticore Search 2.8.2 或更高版本，且 DSN 字符串应包含以下选项：
```ini
jdbc:mysql://IP:PORT/DB/?characterEncoding=utf8&maxAllowedPacket=512000&serverTimezone=XXX
```

默认情况下，Manticore Search 会向连接器报告自身版本，但这可能会导致一些问题。为了解决该问题，应在配置文件的 searchd 部分将 `mysql_version_string` 指令设置为低于 5.1.1 的版本：

```ini
searchd {
...
   mysql_version_string = 5.0.37
...
}
```

.NET MySQL 连接器默认使用连接池。为了正确获取 `SHOW META` 的统计信息，应将查询与 `SHOW META` 命令一起作为单个多语句发送（`SELECT ...;SHOW META`）。如果启用了连接池，则需要在连接字符串中添加选项 `Allow Batch=True` 以允许多语句：
```ini
Server=127.0.0.1;Port=9306;Database=somevalue;Uid=somevalue;Pwd=;Allow Batch=True;
```

## 关于 ODBC 连接的说明
可以使用 ODBC 访问 Manticore。建议在 ODBC 字符串中设置 `charset=UTF8`。某些 ODBC 驱动程序不喜欢 Manticore 服务器报告的版本，因为它们会将其视为非常旧的 MySQL 服务器。可以使用 `mysql_version_string` 选项覆盖这一点。

## 注释语法

Manticore SQL over MySQL 支持 C 风格的注释语法。从打开的 `/*` 序列到关闭的 `*/` 序列之间的所有内容都会被忽略。注释可以跨多行，不能嵌套，也不应被记录。MySQL 特定的 `/*! ... */` 注释当前也被忽略。（注释支持的添加主要是为了更好地兼容由 `mysqldump` 生成的导出文件，而不是为了改善 Manticore 和 MySQL 之间查询的通用互操作性。）

```sql
SELECT /*! SQL_CALC_FOUND_ROWS */ col1 FROM table1 WHERE ...
```
<!-- proofread -->

