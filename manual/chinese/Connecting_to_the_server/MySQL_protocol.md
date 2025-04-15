# MySQL协议

Manticore Search实现了一个SQL接口，使用MySQL协议，允许任何MySQL库或连接器以及许多MySQL客户端连接到Manticore Search并将其视为MySQL服务器，而不是Manticore。

然而，SQL方言是不同的，只实现了MySQL中可用的SQL命令或函数的一个子集。此外，还有一些特定于Manticore Search的子句和函数，例如用于全文搜索的`MATCH()`子句。

Manticore Search不支持服务器端的预准备语句，但可以使用客户端的预准备语句。需要注意的是，Manticore实现了多值（MVA）数据类型，该类型在MySQL或实现了预准备语句的库中没有等效项。在这些情况下，MVA值必须在原始查询中手动构建。

一些MySQL客户端/连接器需要用户/密码和/或数据库名称的值。由于Manticore Search没有数据库的概念，并且未实现用户访问控制，因此可以随意设置这些值，因为Manticore会直接忽略它们。

## 配置

**SQL接口的默认端口为9306**，并且默认情况下已启用。 

您可以在配置文件的searchd部分中使用`listen`指令配置MySQL端口，如下所示：

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
...
}
``` 

请记住，Manticore没有用户认证，因此请确保MySQL端口对您网络之外的任何人不可访问。

### VIP连接
可以使用单独的MySQL端口进行“VIP”连接。当连接到此端口时，将绕过线程池，并始终创建一个新的专用线程。这在严重过载的情况下非常有用，此时服务器可能会停滞或阻止通过常规端口连接。

```ini
searchd {
...
   listen = 127.0.0.1:9306:mysql
   listen = 127.0.0.1:9307:mysql_vip
...
}
``` 

## 通过标准MySQL客户端连接
连接到Manticore的最简单方法是使用标准MySQL客户端：

```shell
mysql -P9306 -h0
```

## 安全的MySQL连接

MySQL协议支持[SSL加密](../Security/SSL.md)。可以在同一个`mysql`监听端口上建立安全连接。

## 压缩的MySQL连接

可以使用压缩与MySQL连接，并且默认情况下对客户端可用。客户端只需指定连接应使用压缩。

使用MySQL客户端的示例：

```shell
mysql -P9306 -h0 -C
```

压缩可以在安全和非安全连接中使用。
 
## 关于MySQL连接器的说明
官方MySQL连接器可用于连接到Manticore Search，但可能需要在DSN字符串中传递某些设置，因为连接器可能会尝试运行Manticore中尚未实现的某些SQL命令。

JDBC连接器6.x及以上需要Manticore Search 2.8.2或更高版本，并且DSN字符串应包含以下选项：
```ini
jdbc:mysql://IP:PORT/DB/?characterEncoding=utf8&maxAllowedPacket=512000&serverTimezone=XXX
```

默认情况下，Manticore Search会向连接器报告其自身版本，但这可能会引起一些问题。为了解决这个问题，应该在配置的searchd部分中将`mysql_version_string`指令设置为低于5.1.1的版本：

```ini
searchd {
...
   mysql_version_string = 5.0.37
...
}
``` 

.NET MySQL连接器默认使用连接池。要正确获取`SHOW META`的统计信息，查询与`SHOW META`命令应作为单个多语句发送（`SELECT ...;SHOW META`）。如果启用连接池，连接字符串中需要添加`Allow Batch=True`选项以允许多语句：
```ini
Server=127.0.0.1;Port=9306;Database=somevalue;Uid=somevalue;Pwd=;Allow Batch=True;
```

## 关于ODBC连接的说明
可以使用ODBC访问Manticore。建议在ODBC字符串中设置`charset=UTF8`。某些ODBC驱动程序可能不喜欢Manticore服务器报告的版本，因为他们会将其视为一个非常旧的MySQL服务器。可以使用`mysql_version_string`选项覆盖此项。

## 注释语法

Manticore SQL通过MySQL支持C风格的注释语法。从打开的`/*`序列到关闭的`*/`序列之间的所有内容都被忽略。注释可以跨多行，不能嵌套，并且不应被记录。特定于MySQL的`/*! ... */`注释目前也被忽略。（注释支持的添加主要是为了与`mysqldump`生成的转储更好地兼容，而不是改善Manticore与MySQL之间的一般查询互操作性。）

```sql
SELECT /*! SQL_CALC_FOUND_ROWS */ col1 FROM table1 WHERE ...
```
<!-- proofread -->
