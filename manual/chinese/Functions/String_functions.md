# 字符串函数

### CONCAT()
将两个或多个字符串连接成一个。非字符串参数必须使用 `TO_STRING()` 函数显式转换为字符串。

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` 返回单字符编辑（插入、删除或替换）次数的数字（Levenshtein 距离），表示将 `pattern` 修改为 `source` 所需的编辑次数。

   * `pattern`, `source` - 常量字符串，字符串字段名，JSON 字段名，或任何产生字符串的表达式（例如 [SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29)）
   * `normalize` - 选项以浮点数形式返回距离，范围为 `[0.0 - 1.0]`，其中 0.0 表示完全匹配，1.0 表示最大差异。默认值为 0，表示不归一化，结果以整数形式返回。
   * `length_delta` - 如果设置该选项且字符串长度差异大于 `length_delta`，则跳过 Levenshtein 距离计算，直接返回 `max(strlen(pattern), strlen(source))`。默认值为 0，表示对任何输入字符串计算 Levenshtein 距离。该选项对检查大致相似的字符串时很有用。

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
`REGEX(attr,expr)` 函数如果正则表达式匹配属性字符串则返回 1，否则返回 0。它适用于字符串和 JSON 属性。

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

表达式应遵循 RE2 语法。比如，要执行大小写不敏感的搜索，可以使用：
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
`SNIPPET()` 函数可以用于突出显示给定文本中的搜索结果。前两个参数是：要高亮的文本和查询。[选项](../Searching/Highlighting.md#Highlighting-options) 可以作为第三个、第四个等参数传递给函数。`SNIPPET()` 可以直接从表中获取高亮文本。在这种情况下，第一个参数应为字段名：

```sql
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query')
```

在此示例中，`QUERY()` 表达式返回当前全文查询。`SNIPPET()` 还可用于高亮非索引文本：

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

此外，还可以用于高亮通过用户自定义函数（UDF）从其他来源获取的文本：

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

在此上下文中，`myUdf()` 是一个用户自定义函数（UDF），用于从外部存储源根据 ID 获取文档。`SNIPPET()` 函数被视为“后限制”函数，这意味着片段计算会延迟到最终完整结果集准备好之后，甚至在应用 `LIMIT` 子句之后。例如，使用 `LIMIT 20,10` 子句时，`SNIPPET()` 的调用次数不会超过 10 次。

需要注意的是，`SNIPPET()` 不支持基于字段的限制。对此功能请使用 [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting-via-SQL)。

### SUBSTRING_INDEX()

<!-- example substring_index -->
`SUBSTRING_INDEX(string, delimiter, number)` 返回基于指定分隔符出现次数的原字符串子串：

   *   string - 原字符串，可以是常量字符串或字符串/JSON 属性中的字符串。
   *   delimiter - 要搜索的分隔符。
   *   number - 搜索分隔符的次数。可以是正数或负数。正数时，函数返回分隔符左侧的所有内容；负数时，函数返回分隔符右侧的所有内容。

`SUBSTRING_INDEX()` 默认返回字符串，但如果需要，也可以强制转换为其他类型（例如整数或浮点数）。数值可用特定函数（如 `BIGINT()`、`DOUBLE()` 等）进行转换。

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

### UPPER() 和 LOWER()

`UPPER(string)` 将参数转换为大写，`LOWER(string)` 将参数转换为小写。

结果也可以提升为数字，但仅当字符串参数可转换为数字时。数值可以用任意函数(`BITINT`、`DOUBLE` 等)进行提升。

```sql
SELECT upper('www.w3schools.com', '.', 2); /* WWW.W3SCHOOLS.COM  */
SELECT double (upper ('1.2e3')); /* 1200.000000 */
SELECT integer (lower ('12345')); /* 12345 */
```
<!-- proofread -->

