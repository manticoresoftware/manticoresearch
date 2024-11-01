# MySQL 协议

Manticore Search 通过 MySQL 协议实现了 SQL 接口，允许任何 MySQL 库或连接器，以及许多 MySQL 客户端，像连接 MySQL 服务器一样连接 Manticore Search 并进行操作。

然而，Manticore Search 的 SQL 方言不同，只有 MySQL 中 SQL 命令或函数的一个子集。此外，还包含一些 Manticore Search 独有的子句和函数，例如用于全文搜索的 `MATCH()` 子句。

Manticore Search 不支持服务端的预处理语句，但可以使用客户端的预处理语句。需要注意的是，Manticore 实现了多值（MVA）数据类型，而 MySQL 或实现预处理语句的库中没有对应的等价物。在这些情况下，MVA 值必须在原始查询中构造。

一些 MySQL 客户端/连接器要求提供用户/密码和/或数据库名称的值。由于 Manticore Search 没有数据库的概念，也没有实现用户访问控制，因此这些值可以随意设置，Manticore 会直接忽略它们。

## 配置

**SQL 接口的默认端口是 9306**，并且默认启用。

您可以在配置文件的 searchd 部分中使用 `listen` 指令配置 MySQL 端口，像这样：

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
...
}
```

请注意，Manticore 没有用户身份验证功能，因此请确保 MySQL 端口对外部网络不可访问。

### VIP 连接
可以使用一个单独的 MySQL 端口进行 "VIP" 连接。当连接到该端口时，将绕过线程池，并总是创建一个专用线程。这在服务器过载严重时非常有用，因为在这种情况下，服务器可能会停止或阻止通过常规端口的连接。

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
   listen = 127.0.0.1:9307:mysql_vip
...
}
```

## 通过标准 MySQL 客户端连接
使用标准的 MySQL 客户端连接到 Manticore 是最简单的方式：

```shell
mysql -P9306 -h0
```

## 安全的 MySQL 连接

MySQL 协议支持 [SSL 加密](../Security/SSL.md)。可以在相同的 `mysql` 监听端口上建立安全连接。

## 压缩的 MySQL 连接

MySQL 连接可以使用压缩功能，并且客户端默认可以使用该功能。客户端只需要指定连接应该使用压缩即可。

使用 MySQL 客户端的示例：

```shell
mysql -P9306 -h0 -C
```

压缩功能可以在安全连接和非安全连接中使用。

## 关于 MySQL 连接器的注意事项

官方的 MySQL 连接器可以用于连接 Manticore Search，然而它们可能需要在 DSN 字符串中传递某些设置，因为连接器可能尝试运行 Manticore 尚未实现的某些 SQL 命令。

JDBC 连接器 6.x 及以上版本需要 Manticore Search 2.8.2 或更高版本，DSN 字符串应包含以下选项：

```ini
jdbc:mysql://IP:PORT/DB/?characterEncoding=utf8&maxAllowedPacket=512000&serverTimezone=XXX
```

默认情况下，Manticore Search 会向连接器报告其自身的版本，但这可能会导致一些问题。为了解决这个问题，应在配置文件的 searchd 部分中将 `mysql_version_string` 指令设置为低于 5.1.1 的版本：

```ini
searchd {
...
   mysql_version_string = 5.0.37
...
}
```

.NET MySQL 连接器默认使用连接池。为了正确获取 `SHOW META` 的统计信息，查询与 `SHOW META` 命令应作为单个多语句发送 (`SELECT ...;SHOW META`)。如果启用了连接池，则需要在连接字符串中添加 `Allow Batch=True` 选项以允许多语句：
```ini
Server=127.0.0.1;Port=9306;Database=somevalue;Uid=somevalue;Pwd=;Allow Batch=True;
```

## 关于 ODBC 连接的注意事项

Manticore 可以通过 ODBC 访问。推荐在 ODBC 字符串中设置 `charset=UTF8`。某些 ODBC 驱动程序可能不接受 Manticore 服务器报告的版本，因为它们会将其视为非常旧的 MySQL 服务器。可以通过 `mysql_version_string` 选项来覆盖该行为。

## 注释语法

Manticore SQL 在 MySQL 协议上支持 C 风格的注释语法。`/*` 开始到 `*/` 结束的内容都将被忽略。注释可以跨越多行，不能嵌套，并且不会被记录。MySQL 特定的 `/*! ... */` 注释也会被忽略。（添加注释支持主要是为了与 `mysqldump` 生成的转储文件更好兼容，而不是为了提高 Manticore 和 MySQL 之间的通用查询互操作性。）

```sql
SELECT /*! SQL_CALC_FOUND_ROWS */ col1 FROM table1 WHERE ...
```
<!-- proofread -->