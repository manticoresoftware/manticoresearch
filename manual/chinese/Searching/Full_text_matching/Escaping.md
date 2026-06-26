# 查询字符串中的转义字符

由于某些字符在查询字符串中作为操作符使用，因此必须对其进行转义，以防止查询错误或意外匹配条件。

以下字符应使用反斜杠（`\`）进行转义：

```
!    "    $    '    (    )    -    /    <    @    \    ^    |    ~
```

## 在 MySQL 命令行客户端

要转义单引号（'），使用一个反斜杠：
```sql
SELECT * FROM your_index WHERE MATCH('l\'italiano');
```


对于前面列表中提到的其他字符，它们是操作符或查询结构，必须由引擎视为普通字符，前面有一个转义字符。
反斜杠本身也需要转义，因此需要两个反斜杠：

```sql
SELECT * FROM your_index WHERE MATCH('r\\&b | \\(official video\\)');
```

要使用反斜杠作为字符，您必须转义反斜杠作为字符和转义操作符，这需要四个反斜杠：

```sql
SELECT * FROM your_index WHERE MATCH('\\\\ABC');
```

当您在 Manticore Search 中处理 JSON 数据并需要在 JSON 字符串中包含双引号（"）时，正确处理它是很重要的。在 JSON 中，字符串内的双引号使用反斜杠（\）进行转义。然而，当通过 SQL 查询插入 JSON 数据时，Manticore Search 会将反斜杠（\）解释为字符串内的转义字符。

为了确保双引号正确插入 JSON 数据，您需要转义反斜杠本身。这需要在双引号前使用两个反斜杠（\\）。例如：

```sql
insert into tbl(j) values('{"a": "\\"abc\\""}');
```

## 使用 MySQL 驱动程序

MySQL 驱动程序提供转义函数（例如 PHP 中的 `mysqli_real_escape_string` 或 Python 中的 `conn.escape_string`），但它们只会转义特定字符。
您仍然需要为前面提到的列表中未被其相应函数转义的字符添加转义。
由于这些函数会为您转义反斜杠，您只需添加一个反斜杠。

这也适用于使用预处理语句的驱动程序（客户端或服务器端）。Manticore 支持通过 MySQL 协议的服务器端 [预处理语句](../../Connecting_to_the_server/MySQL_protocol.md#Prepared-statements)，但 `MATCH()` 仍然期望转义的查询字符串。例如，使用 PHP PDO 预处理语句时，需要为 `$` 字符添加反斜杠：

```php
$statement = $ln_sph->prepare( "SELECT * FROM index WHERE MATCH(:match)");
$match = '\$manticore';
$statement->bindParam(':match',$match,PDO::PARAM_STR);
$results = $statement->execute();
```

这导致最终查询 `SELECT * FROM index WHERE MATCH('\\$manticore');`

## 在 HTTP JSON API 中

与 SQL 协议的规则相同，对于 JSON，双引号必须使用单个反斜杠进行转义，而其他字符需要双转义。

当使用将数据结构转换为 JSON 字符串的 JSON 库或函数时，双引号和单个反斜杠将由这些函数自动转义，无需显式转义。



## 在客户端

官方客户端 [使用](https://github.com/manticoresoftware/) 其各自编程语言下常见的 JSON 库/函数。前面提到的转义规则同样适用。


## 转义星号

星号（*）是一个特殊的字符，具有两种用途：
* 作为通配符前缀/后缀扩展符
* 作为短语搜索中的任意项修饰符。

与其他作为操作符使用的特殊字符不同，星号在提供其功能之一的位置时不能被转义。

在非通配符查询中，星号无论是在 `charset_table` 中还是不在，都不需要转义。

在通配符查询中，单词中间的星号不需要转义。作为通配符操作符（单词的开头或结尾），星号将始终被解释为通配符操作符，即使应用了转义。

## 在 SQL 中转义 JSON 节点名称

要转义 JSON 节点中的特殊字符，请使用反引号。例如：

```sql
MySQL [(none)]> select * from t where json.`a=b`=234;
+---------------------+-------------+------+
| id                  | json        | text |
+---------------------+-------------+------+
| 8215557549554925578 | {"a=b":234} |      |
+---------------------+-------------+------+

MySQL [(none)]> select * from t where json.`a:b`=123;
+---------------------+-------------+------+
| id                  | json        | text |
+---------------------+-------------+------+
| 8215557549554925577 | {"a:b":123} |      |
+---------------------+-------------+------+
```