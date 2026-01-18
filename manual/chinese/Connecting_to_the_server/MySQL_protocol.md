# MySQL 协议

Manticore Search 通过使用 MySQL 协议实现了 SQL 接口，允许使用任何 MySQL 库或连接器以及许多 MySQL 客户端连接到 Manticore Search，并像使用 MySQL 服务器一样操作它，而非直接操作 Manticore。

然而，SQL 方言有所不同，仅实现了 MySQL 中可用的 SQL 命令或函数的一个子集。此外，还存在 Manticore Search 特有的子句和函数，例如用于全文搜索的 `MATCH()` 子句。

Manticore Search 支持通过 MySQL 协议的服务器端预处理语句。也可以使用客户端预处理语句。需要注意的是，Manticore 实现了多值（MVA）和 `float_vector` 数据类型，这些在 MySQL 或实现预处理语句的库中没有等价物。在这些情况下，值必须在原始查询中以列表字面量的形式构造。

## 预处理语句

Manticore 支持通过 MySQL 协议的 COM_STMT_PREPARE/COM_STMT_EXECUTE。
这仅通过 MySQL 协议可用（而不是通过 HTTP SQL 端点），并且不支持基于游标的获取。

### MVA 和 float_vector 的列表参数

预处理语句仅接受 `(1,2,3)` 格式的列表参数用于 `multi`、`multi64` 和 `float_vector`。
使用 `TO_MULTI(?)` 或 `TO_VECTOR(?)` 显式标记参数为列表。

### 兼容性说明

某些驱动程序将数字参数作为 DOUBLE 发送（例如 Node.js 的 mysql2）。
如果您需要严格的整数行为（例如拒绝负 id），请将整数作为字符串或驱动程序特定的整数类型（例如 BigInt）发送。

部分 MySQL 客户端/连接器需要用户/密码和/或数据库名称的值。由于 Manticore Search 没有数据库概念，也没有实现用户访问控制，这些值可以随意设置，因为 Manticore 会直接忽略它们。

## 配置

**SQL 接口的默认端口是 9306**，且默认启用。

你可以在配置文件的 searchd 部分，使用 `listen` 指令配置 MySQL 端口，格式如下：

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
...
}
```

请记住，Manticore 没有用户认证功能，所以请确保 MySQL 端口不会被网络外的人访问。

### VIP 连接
可以使用单独的 MySQL 端口执行“VIP”连接。连接到此端口时，将绕过线程池，总是创建一个新的专用线程。这在严重过载的情况下非常有用，避免服务器停滞或阻止通过常规端口的连接。

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
   listen = 127.0.0.1:9307:mysql_vip
...
}
```

## 通过标准 MySQL 客户端连接
连接 Manticore 最简单的方法是使用标准的 MySQL 客户端：

```shell
mysql -P9306 -h0
```

## 安全的 MySQL 连接

MySQL 协议支持[SSL 加密](../Security/SSL.md)。可以在同一个 `mysql` 监听端口上建立安全连接。

## 压缩的 MySQL 连接

MySQL 连接支持压缩，客户默认即可使用。客户端只需指定连接应使用压缩即可。

下面是一个使用 MySQL 客户端的示例：

```shell
mysql -P9306 -h0 -C
```

压缩可用于安全连接和非安全连接。

## 关于 MySQL 连接器的说明
官方 MySQL 连接器可用于连接 Manticore Search ，但可能需要在 DSN 字符串中传递某些设置，因为连接器可能会尝试执行 Manticore 尚未实现的某些 SQL 命令。

JDBC 6.x 及以上版本的连接器要求 Manticore Search 版本为 2.8.2 或更高，且 DSN 字符串应包含以下选项：
```ini
jdbc:mysql://IP:PORT/DB/?characterEncoding=utf8&maxAllowedPacket=512000&serverTimezone=XXX
```

默认情况下，Manticore Search 会向连接器报告自己的版本，但这可能导致一些问题。为解决该问题，应在配置文件的 searchd 部分设置 `mysql_version_string` 指令，将版本设为低于 5.1.1 的版本：

```ini
searchd {
...
   mysql_version_string = 5.0.37
...
}
```

.NET MySQL 连接器默认使用连接池。为了正确获取 `SHOW META` 的统计信息，应将查询与 `SHOW META` 命令作为单个多语句发送（`SELECT ...;SHOW META`）。如果启用连接池，需在连接字符串中添加选项 `Allow Batch=True` 以允许多语句：
```ini
Server=127.0.0.1;Port=9306;Database=somevalue;Uid=somevalue;Pwd=;Allow Batch=True;
```

## 关于 ODBC 连接的说明
可以通过 ODBC 访问 Manticore。建议在 ODBC 字符串中设置 `charset=UTF8`。部分 ODBC 驱动不喜欢 Manticore 服务器报告的版本，因为它们将其视为非常旧的 MySQL 服务器。该版本信息可通过 `mysql_version_string` 选项覆盖。

## 注释语法

基于 MySQL 的 Manticore SQL 支持 C 风格注释语法。所有从开启符 `/*` 到关闭符 `*/` 的内容都会被忽略。注释可以跨多行，不能嵌套，且不应被记录。MySQL 特有的 `/*! ... */` 注释目前也被忽略。（注释支持主要是为了更好地兼容 `mysqldump` 生成的转储文件，而非增强 Manticore 与 MySQL 之间的一般查询互操作性。）

```sql
SELECT /*! SQL_CALC_FOUND_ROWS */ col1 FROM table1 WHERE ...
```
<!-- proofread -->
