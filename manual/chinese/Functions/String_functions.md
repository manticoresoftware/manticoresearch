# 字符串函数

### CONCAT()
将两个或多个字符串连接成一个。非字符串参数必须使用 `TO_STRING()` 函数显式转换为字符串。

```sql
CONCAT(TO_STRING(float_attr), ',', TO_STRING(int_attr), ',', title)
```

### LEVENSHTEIN()
`LEVENSHTEIN ( pattern, source, {normalize=0, length_delta=0})` 返回单字符编辑（插入、删除或替换）次数的数字（Levenshtein距离），该次数是将 `pattern` 转换为 `source` 所需的编辑次数。

   * `pattern`, `source` - 常量字符串、字符串字段名、JSON字段名或任何产生字符串的表达式（例如，[SUBSTRING_INDEX()](../Functions/String_functions.md#SUBSTRING_INDEX%28%29)）
   * `normalize` - 选项，返回距离的浮点数，范围在 `[0.0 - 1.0]`，其中0.0表示完全匹配，1.0表示最大差异。默认值为0，表示不归一化，结果以整数返回。
   * `length_delta` - 如果设置该选项且字符串长度差超过此值，则跳过Levenshtein距离计算，返回 `max(strlen(pattern), strlen(source))`。默认值为0，表示对任意输入字符串都计算Levenshtein距离。此选项在检查大致相似的字符串时很有用。

``` sql
SELECT LEVENSHTEIN('gily', attr1) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC, dist ASC;
SELECT LEVENSHTEIN('gily', j.name, {length_delta=6}) AS dist, WEIGHT() AS w FROM test WHERE MATCH('test') ORDER BY w DESC;
SELECT LEVENSHTEIN(title, j.name, {normalize=1}) AS dist, WEIGHT() AS w FROM test WHERE MATCH ('test') ORDER BY w DESC, dist ASC;
```

### REGEX()
`REGEX(attr,expr)` 函数如果正则表达式匹配属性的字符串，则返回1，否则返回0。该函数可用于字符串和JSON属性。

```sql
SELECT REGEX(content, 'box?') FROM test;
SELECT REGEX(j.color, 'red | pink') FROM test;
```

表达式应符合 RE2 语法。例如，要进行不区分大小写的搜索，可以使用：
```sql
SELECT REGEX(content, '(?i)box') FROM test;
```

### SNIPPET()
`SNIPPET()` 函数可用于在给定文本中高亮显示搜索结果。前两个参数是：要高亮的文本和查询。可以将 [选项](../Searching/Highlighting.md#Highlighting-options) 作为第三、第四等参数传入。`SNIPPET()` 可以直接从表中获取要高亮的文本，此时第一个参数应为字段名：

```sql
SELECT SNIPPET(body, QUERY()) FROM myIndex WHERE MATCH('my.query')
```

在此示例中，`QUERY()` 表达式返回当前全文查询。`SNIPPET()` 也可以高亮非索引文本：

```sql
mysql  SELECT id, SNIPPET('text to highlight', 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

此外，它可以用于高亮通过用户自定义函数（UDF）获取的文本：

```sql
SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100') FROM myIndex WHERE MATCH('my.query')
```

在此上下文中，`myUdf()` 是一个用户自定义函数（UDF），用于从外部存储源根据ID检索文档。`SNIPPET()` 函数被视为“后限”函数，意味着片段计算会延后，直到整个最终结果集准备好，甚至在 `LIMIT` 子句应用之后。例如，如果使用了 `LIMIT 20,10` 子句，`SNIPPET()` 调用次数不会超过10次。

需要注意的是，`SNIPPET()` 不支持基于字段的限制。对此功能，请使用 [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting-via-SQL) 。

### SUBSTRING_INDEX()

<!-- example substring_index -->
`SUBSTRING_INDEX(string, delimiter, number)` 根据指定分隔符出现的次数返回原字符串的子串：

   *   string - 原字符串，可以是常量字符串或字符串/JSON属性中的字符串。
   *   delimiter - 要查找的分隔符。
   *   number - 查找分隔符的次数。可以是正数或负数。如果为正数，函数返回分隔符左侧的所有内容。如果为负数，函数返回分隔符右侧的所有内容。

`SUBSTRING_INDEX()` 默认返回字符串，但如果需要也可强制转换为其他类型（如整数或浮点数）。数值可以使用特定函数转换（如 `BIGINT()`, `DOUBLE()` 等）。

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

结果也可以转换为数值，但仅当字符串参数可转换为数字时。数值可以用任意函数提升（`BITINT`、`DOUBLE` 等）。

```sql
SELECT upper('www.w3schools.com', '.', 2); /* WWW.W3SCHOOLS.COM  */
SELECT double (upper ('1.2e3')); /* 1200.000000 */
SELECT integer (lower ('12345')); /* 12345 */
```
<!-- proofread -->

