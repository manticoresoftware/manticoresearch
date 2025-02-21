# 转义查询字符串中的字符

由于某些字符在查询字符串中用作操作符，因此必须对其进行转义，以避免查询错误或不希望的匹配条件。

以下字符需要使用反斜杠（`\`）进行转义：

```
!    "    $    '    (    )    -    /    <    @    \    ^    |    ~
```

## 在 MySQL 命令行客户端中

要转义单引号 (`'`)，使用一个反斜杠：
```sql
SELECT * FROM your_index WHERE MATCH('l\'italiano');
```


对于前面列表中提到的其他字符（它们是操作符或查询构造符），必须使用转义字符前缀处理为简单字符。反斜杠也需要转义，因此需要两个反斜杠：

```sql
SELECT * FROM your_index WHERE MATCH('r\\&b | \\(official video\\)');
```

要将反斜杠作为字符使用，必须同时转义反斜杠作为字符和转义符，这需要四个反斜杠：

```sql
SELECT * FROM your_index WHERE MATCH('\\\\ABC');
```

当在 Manticore Search 中处理 JSON 数据且需要在 JSON 字符串中包含双引号（`"`）时，务必正确转义。在 JSON 中，字符串中的双引号使用反斜杠（`\`）转义。但是，当通过 SQL 查询插入 JSON 数据时，Manticore Search 会将反斜杠（`\`）视为字符串中的转义字符。

为确保双引号正确插入到 JSON 数据中，必须对反斜杠本身进行转义。这意味着在双引号之前使用两个反斜杠（`\\`）。例如：

```sql
insert into tbl(j) values('{"a": "\\"abc\\""}');
```

## 使用 MySQL 驱动程序

MySQL 驱动程序提供了转义函数（例如，PHP 中的 `mysqli_real_escape_string` 或 Python 中的 `conn.escape_string`），但它们只转义特定字符。 对于上述列表中未被这些函数转义的字符，仍然需要手动添加转义。 因为这些函数会为你转义反斜杠，因此你只需要添加一个反斜杠。

这同样适用于支持（客户端）预处理语句的驱动程序。例如，在 PHP PDO 预处理语句中，你需要为 `$` 字符添加反斜杠：

```php
$statement = $ln_sph->prepare( "SELECT * FROM index WHERE MATCH(:match)");
$match = '\$manticore';
$statement->bindParam(':match',$match,PDO::PARAM_STR);
$results = $statement->execute();
```

最终的查询结果为： `SELECT * FROM index WHERE MATCH('\\$manticore');`

## 在 HTTP JSON API 中

同样的规则适用于 SQL 协议，区别在于对于 JSON，双引号需要使用单个反斜杠转义，而其他字符需要双重转义。

当使用 JSON 库或将数据结构转换为 JSON 字符串的函数时，这些函数会自动转义双引号和单个反斜杠，因此不需要显式转义。



## 在客户端中

[新官方客户端](https://github.com/manticoresoftware/)（使用 HTTP 协议）在其编程语言中使用了常见的 JSON 库/函数。适用上述提到的转义规则。


## 转义星号

星号 (`*`) 是一个特殊字符，具有两种用途：

- 作为通配符前缀/后缀扩展符
- 作为短语搜索中的任何词修饰符。

与其他作为操作符的特殊字符不同，星号在用于提供其功能时无法被转义。

在非通配符查询中，星号不需要转义，无论它是否在 `charset_table` 中。

在通配符查询中，单词中间的星号不需要转义。作为通配符操作符时（单词的开头或结尾），星号将始终被解释为通配符操作符，即使应用了转义。

## 在 SQL 中转义 JSON 节点名称

要转义 JSON 节点中的特殊字符，使用反引号。例如：

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
<!-- proofread -->
