# MySQL 协议

Manticore Search 通过使用 MySQL 协议实现了 SQL 接口，允许使用任何 MySQL 库或连接器以及许多 MySQL 客户端连接到 Manticore Search，并像使用 MySQL 服务器一样操作它，而非直接操作 Manticore。

然而，SQL 方言有所不同，仅实现了 MySQL 中可用的 SQL 命令或函数的一个子集。此外，还存在 Manticore Search 特有的子句和函数，例如用于全文搜索的 `MATCH()` 子句。

Manticore Search支持通过MySQL协议的服务器端[预处理语句](#预处理语句)。也可以使用客户端预处理语句。需要注意的是，Manticore实现了多值（MVA）和`float_vector`数据类型，这些在MySQL或实现预处理语句的库中没有等价物。在这些情况下，值必须在原始查询中以逗号分隔的数字列表形式构建。

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

# 预处理语句

Manticore支持通过MySQL协议的预处理语句。 

在数据库中，[预处理语句](https://en.wikipedia.org/wiki/Prepared_statement)是一种运行查询的方式，其中SQL代码与输入数据分开。您不需要将值直接包含在完整的SQL查询中，而是使用占位符编写一次查询，然后单独提供值。

一些客户端库（例如，[sqlx](https://docs.rs/sqlx/latest/sqlx/)）仅以这种方式与数据库通信。

## 为什么预处理语句很重要？

* **安全性（防止SQL注入）：** 这是最关键的原因。SQL注入是一种常见的网络漏洞，攻击者会将恶意SQL插入查询中。预处理语句确保用户输入被严格视为数据，而不是可执行代码，从而防止其被解释为SQL命令的一部分。

* **可读性与可维护性：** 预处理语句通过将SQL逻辑与输入数据分离，提高了代码的清晰度，使代码更易于阅读和维护。

* **性能：** 数据库可以缓存查询计划。如果相同的参数化查询多次运行但值不同，数据库可以重用缓存的执行计划，而不是每次解析和优化查询，这可以提高性能。

<!-- example prepared-statements -->
## 预处理语句的工作原理

1. **准备查询：** 将带有占位符（如 `?` 或 `?VEC?`）的 SQL 语句发送到 Manticore。Manticore 会解析并编译该语句，存储它，并返回一个唯一 ID，可用于稍后执行该语句。
2. **绑定参数：** 分别发送占位符的值。Manticore 将这些值严格视为数据（而非 SQL 代码），并将其安全地插入到存储的语句中。
3. **执行查询：** Manticore 使用预编译计划和提供的参数值一起执行该语句。

<!-- intro -->
### 示例

<!-- request SQL -->
```
INSERT INTO table (id, floatvec) VALUES (?, (?VEC?))
```

<!-- request PHP -->
```php
$stmt = $mysqli->prepare("INSERT INTO tbl (id, str, floatvec) VALUES (0, ?, (?VEC?))");
$str = "I'm a string";
$vec = "0.1,0.2,0.3";
$stmt->bind_param("ss", $str, $vec);
$stmt->execute();
```

<!-- response PHP -->

这将执行：
```
INSERT INTO tbl (id, str, floatvec) VALUES (0, 'I\'m a string', (0.1,0.2,0.3))
```

<!-- end -->

## 参数占位符

* `?`（问号）：表示单个参数（整数、浮点数或字符串）。字符串值会自动处理——例如，特殊字符如单引号会被转义，并且值会被引号包围。布尔值可以作为字符串传递（`'true'`，`'false'`）。
* `?VEC?`：表示作为字符串提供的数值列表（例如，`1,2,3` 或 `0.3,15E+3, 20 ,INF`）。仅允许数字、逗号和空格。验证后，值会严格按照提供的形式插入（无需额外转义或引号）。

## 向量参数（MVA/float_vector）

在标准 Manticore SQL 语法中，[MVA](../Creating_a_table/Data_types.md#Multi-value-integer-%28MVA%29) 或 [float vectors](../Creating_a_table/Data_types.md#Float-vector) 作为括号内的一组值写入，例如 `(1, 2, 3)`。 

使用预处理语句时，请直接在查询中包含括号，并仅使用 `?VEC?` 占位符表示括号内的值。例如：

```php
$stmt = $mysqli->prepare("INSERT INTO tbl (id, floatvec) VALUES (0, (?VEC?))");
$vec = "0.1,0.2,0.3";
$stmt->bind_param("s", $vec);
```

## 限制

1. 每个预处理语句仅允许一个 SQL 语句。[多查询](../Searching/Multi-queries.md#Multi-queries)（例如，`SELECT ...; SHOW META`）不被支持。如果需要运行多个语句，请为每个语句分别准备，并在同一会话中按顺序执行它们。
2. 某些驱动程序会将数值参数作为 DOUBLE 发送（例如，Node.js 的 `mysql2`）。如果需要严格的整数行为（例如，拒绝负数 ID），请将整数作为字符串发送，或使用特定于驱动程序的整数类型（例如，`BigInt`）。
3. **Rust sqlx：** 在读取结果集行时，请使用列索引而非列名。结果集中存在列名，但 sqlx 不使用它们进行映射。例如，使用 `row.try_get(0)?` 而非 `row.try_get("id")?`。