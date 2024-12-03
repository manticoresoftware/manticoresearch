# 字符串函数

### CONCAT()
将两个或多个字符串连接成一个。非字符串参数必须通过 `TO_STRING()` 函数显式转换为字符串。

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` 返回模式和源字符串之间通过单字符编辑（插入、删除或替换）将 `pattern` 转换为 `source` 所需的编辑次数。

- `pattern`, `source` - 常量字符串、字符串字段名、JSON 字段名或生成字符串的表达式。
- `normalize` - 是否将距离返回为 [0.0 - 1.0] 范围内的浮点数，默认值为 0，表示返回整数距离。
- `length_delta` - 当字符串长度差异超过此值时，跳过 Levenshtein 距离计算并返回最大字符串长度。

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
``REGEX(attr,expr)` 函数返回 1 表示正则表达式匹配属性的字符串，0 表示不匹配。适用于字符串和 JSON 属性。

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

表达式应遵循 RE2 语法。例如，若要执行不区分大小写的搜索，可以使用：
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
`SNIPPET()` 函数可用于在给定文本中突出显示搜索结果。前两个参数分别是：需要突出显示的文本和查询。可将[选项](../Searching/Highlighting.md#高亮选项)作为第三个、第四个等参数传递给该函数。`SNIPPET()` 可以直接从表中获取要突出显示的文本。在这种情况下，第一个参数应为字段名称：

```sql
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query')
```

在此示例中，`QUERY()` 表达式返回当前的全文查询。`SNIPPET()` 还可以突出显示未被索引的文本：

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

此外，它还可以用于突出显示通过用户定义函数 (UDF) 获取的文本：

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

在此上下文中，`myUdf()` 是一个用户定义函数 (UDF)，它从外部存储源通过文档 ID 获取文档。`SNIPPET()` 函数被认为是“post limit” 函数，意味着片段的计算被延迟，直到整个最终结果集准备好，并且即使应用了 `LIMIT` 子句也是如此。例如，如果使用 `LIMIT 20,10` 子句，`SNIPPET()` 将被调用不超过 10 次。

需要注意的是，`SNIPPET()` 不支持基于字段的限制。要实现该功能，请使用 [HIGHLIGHT()](../Searching/Highlighting.md#通过-SQL-实现高亮)。

### SUBSTRING_INDEX()

<!-- example substring_index -->

`SUBSTRING_INDEX(string, delimiter, number)` 返回原始字符串的子字符串，基于指定的分隔符出现的次数：

- string - 原始字符串，可以是常量字符串或来自字符串/JSON 属性的字符串。
- delimiter - 要搜索的分隔符。
- number - 要搜索分隔符的次数。可以是正数或负数。如果是正数，函数将返回分隔符左侧的所有内容。如果是负数，函数将返回分隔符右侧的所有内容。

`SUBSTRING_INDEX()` 默认返回一个字符串，但如有必要，它也可以强制转换为其他类型（如整数或浮点数）。数值可以使用特定函数（如 `BIGINT()`、`DOUBLE()` 等）进行转换。

<!-- request SQL -->
```sql
SELECT SUBSTRING_INDEX('www.w3schools.com', '.', 2) FROM test;
SELECT SUBSTRING_INDEX(j.coord, ' ', 1) FROM test;
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ',  1);  /* '1.2' */
SELECT          SUBSTRING_INDEX('1.2 3.4', ' ', -1);  /* '3.4' */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1 */
SELECT sint (   SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ',  1)); /* 1.200000 */
SELECT double ( SUBSTRING_INDEX('1.2 3.4', ' ', -1)); /* 3.400000 */
```

<!-- end -->

### UPPER() and LOWER()

`UPPER(string)` 将参数转换为大写，`LOWER(string)` 将参数转换为小写。

结果也可以提升为数值，但仅当字符串参数可转换为数字时。数值可以通过任意函数（`BIGINT`、`DOUBLE` 等）进行转换。

```sql
SELECT upper('www.w3schools.com', '.', 2); /* WWW.W3SCHOOLS.COM  */
SELECT double (upper ('1.2e3')); /* 1200.000000 */
SELECT integer (lower ('12345')); /* 12345 */
```
<!-- proofread -->
