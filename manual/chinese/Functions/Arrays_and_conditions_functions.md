# 数组和条件函数

### ALL()
<!-- example all_json -->
`ALL(cond FOR var IN json.array)` 适用于 JSON 数组，如果条件对数组中的所有元素都为真，则返回 1，否则返回 0。 `cond` 是可以在自身内使用 `var` 作为数组元素当前值的一般表达式。

<!-- request ALL() with json -->
```sql
select *, ALL(x>0 AND x<4 FOR x IN j.ar) from tbl
```

<!-- response ALL() with json -->
```sql
+------+--------------+--------------------------------+
| id   | j            | all(x>0 and x<4 for x in j.ar) |
+------+--------------+--------------------------------+
|    1 | {"ar":[1,3]} |                              1 |
|    2 | {"ar":[3,7]} |                              0 |
+------+--------------+--------------------------------+
2 rows in set (0.00 sec)
```

<!-- request ALL() with json ex. 2 -->
```sql
select *, ALL(x>0 AND x<4 FOR x IN j.ar) cond from tbl where cond=1
```

<!-- response ALL() with json ex. 2 -->
```sql
+------+--------------+------+
| id   | j            | cond |
+------+--------------+------+
|    1 | {"ar":[1,3]} |    1 |
+------+--------------+------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example all_mva -->
`ALL(mva)` 是多值属性的特殊构造函数。当与比较运算符（包括与 `IN()` 的比较）一起使用时，如果找到的 MVA 属性中的所有值都存在于被比较的值中，则返回 1。

<!-- request ALL() with MVA -->
```sql
select * from tbl where all(m) >= 1
```

<!-- response ALL() with MVA -->
```sql
+------+------+
| id   | m    |
+------+------+
|    1 | 1,3  |
|    2 | 3,7  |
+------+------+
2 rows in set (0.00 sec)
```
<!-- request ALL() with MVA and IN() -->
```sql
select * from tbl where all(m) in (1, 3, 7, 10)
```

<!-- response ALL() with MVA and IN() -->
```sql
+------+------+
| id   | m    |
+------+------+
|    1 | 1,3  |
|    2 | 3,7  |
+------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example all_not_in -->
要将 MVA 属性与数组进行比较，避免使用 `<mva> NOT ALL()`；改用 `ALL(<mva>) NOT IN()`。

<!-- request ALL() with MVA and NOT IN() -->
```sql
select * from tbl where all(m) not in (2, 4)
```

<!-- response ALL() with MVA and NOT IN() -->
```sql
+------+------+
| id   | m    |
+------+------+
|    1 | 1,3  |
|    2 | 3,7  |
+------+------+
2 rows in set (0.00 sec)
```

<!-- end -->


<!-- example all_string -->

`ALL(string list)` 是一个用于过滤字符串标签的特殊操作。

如果 `ALL()` 的所有作为参数列举的单词都存在于属性中，则过滤器匹配。可选的 `NOT` 反转逻辑。

此过滤器内部使用逐文档匹配，因此在进行全扫描查询的情况下，它可能比预期慢。它适用于未建立索引的属性，比如计算表达式或 PQ 表中的标签。如果需要这种过滤，可以考虑将字符串属性作为全文字段，然后使用全文操作符 `match()`，这将调用全文搜索。

<!-- request ALL() with strings -->
```sql
select * from tbl where tags all('bug', 'release')
```

<!-- response ALL() with strings -->
```
+------+---------------------------+
| id   | tags                      |
+------+---------------------------+
|    1 | bug priority_high release |
|    2 | bug priority_low release  |
+------+---------------------------+
2 rows in set (0.00 sec)
```

<!-- request ALL() with strings and NOT -->
```sql
mysql> select * from tbl

+------+---------------------------+
| id   | tags                      |
+------+---------------------------+
|    1 | bug priority_high release |
|    2 | bug priority_low release  |
+------+---------------------------+
2 rows in set (0.00 sec)

mysql> select * from tbl where tags not all('bug')

Empty set (0.00 sec)
```
<!-- end -->

### ANY()

<!-- example any_json -->
`ANY(cond FOR var IN json.array)` 适用于 JSON 数组，如果条件对数组中的任何元素为真，则返回 1，否则返回 0。 `cond` 是可以在自身内使用 `var` 作为数组元素当前值的一般表达式。

<!-- request ANY() with json -->
```sql
select *, ANY(x>5 AND x<10 FOR x IN j.ar) from tbl
```

<!-- response ANY() with json -->
```sql
+------+--------------+---------------------------------+
| id   | j            | any(x>5 and x<10 for x in j.ar) |
+------+--------------+---------------------------------+
|    1 | {"ar":[1,3]} |                               0 |
|    2 | {"ar":[3,7]} |                               1 |
+------+--------------+---------------------------------+
2 rows in set (0.00 sec)
```

<!-- request ANY() with json ex. 2 -->
```sql
select *, ANY(x>5 AND x<10 FOR x IN j.ar) cond from tbl where cond=1
```

<!-- response ANY() with json ex. 2 -->
```sql
+------+--------------+------+
| id   | j            | cond |
+------+--------------+------+
|    2 | {"ar":[3,7]} |    1 |
+------+--------------+------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example any_mva -->
`ANY(mva)` 是多值属性的特殊构造函数。当与比较运算符（包括与 `IN()` 的比较）一起使用时，如果找到的 MVA 属性中的任何值都存在于被比较的值中，则返回 1。

当使用 `IN()` 比较数组时，如果未另行指定，则默认假定为 `ANY()`，但会就缺失的构造函数发出警告。

<!-- request ANY() with MVA -->
```sql
mysql> select * from tbl

+------+------+
```
|    1 | 1,3  |
|    2 | 3,7  |
+------+------+
2 rows in set (0.01 sec)

mysql> select * from tbl where any(m) > 5

+------+------+
| id   | m    |
+------+------+
|    2 | 3,7  |
+------+------+
1 row in set (0.00 sec)
```
<!-- request ANY() with MVA and IN() -->
```sql
select * from tbl where any(m) in (1, 7, 10)
```

<!-- response ANY() with MVA and IN() -->
```sql
+------+------+
| id   | m    |
+------+------+
|    1 | 1,3  |
|    2 | 3,7  |
+------+------+
2 rows in set (0.00 sec)
```

<!-- end -->

<!-- example any_not_in -->
要将一个 MVA 属性与数组进行比较，避免使用 `<mva> NOT ANY()`；请改用 `<mva> NOT IN()` 或 `ANY(<mva>) NOT IN()`。

<!-- request ANY() with MVA and NOT IN() -->
```sql
mysql> select * from tbl

+------+------+
| id   | m    |
+------+------+
|    1 | 1,3  |
|    2 | 3,7  |
+------+------+
2 rows in set (0.00 sec)

mysql> select * from tbl where any(m) not in (1, 3, 5)

+------+------+
| id   | m    |
+------+------+
|    2 | 3,7  |
+------+------+
1 row in set (0.00 sec)
```

<!-- end -->

<!-- example any_string -->

`ANY(string list)` 是用于过滤字符串标签的特殊操作。

如果 `ANY()` 的参数中枚举的任何单词在属性中存在，则过滤器匹配。可选的 `NOT` 会反转逻辑。

此过滤器内部使用逐个文档匹配，因此在全扫描查询的情况下，它可能比预期的慢。它的目的是用于未编入索引的属性，例如计算表达式或 PQ 表中的标签。如果您需要这样的筛选，考虑将字符串属性作为全文字段，然后使用全文操作符 `match()`，这将调用全文搜索。

<!-- request ANY() with strings -->
```sql
select * from tbl where tags any('bug', 'feature')
```

<!-- response ANY() with strings -->
```
+------+---------------------------+
| id   | tags                      |
+------+---------------------------+
|    1 | bug priority_high release |
|    2 | bug priority_low release  |
+------+---------------------------+
2 rows in set (0.00 sec)
```

<!-- request ANY() with strings and NOT -->
```sql
select * from tbl
--------------

+------+---------------------------+
| id   | tags                      |
+------+---------------------------+
|    1 | bug priority_high release |
|    2 | bug priority_low release  |
+------+---------------------------+
2 rows in set (0.00 sec)

--------------
select * from tbl where tags not any('feature', 'priority_low')
--------------

+------+---------------------------+
| id   | tags                      |
+------+---------------------------+
|    1 | bug priority_high release |
+------+---------------------------+
1 row in set (0.01 sec)
```


<!-- end -->

### CONTAINS()

`CONTAINS(polygon, x, y)` 检查 (x,y) 点是否在给定的多边形内，如果真实则返回 1，否则返回 0。多边形必须使用 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) 函数指定。前者函数适用于“小”多边形，意味着每边少于 500 公里（300 英里），并且不考虑地球的曲率以提高速度。对于更大的距离，您应该使用 `GEOPOLY2D`，它将给定多边形细分为更小的部分，以考虑地球的曲率。

### IF()

<!-- example if -->
`IF()` 的行为与其 MySQL 对应物略有不同。它接受 3 个参数，检查第一个参数是否等于 0.0，如果不为零则返回第二个参数，否则返回第三个参数。请注意，与比较运算符不同，`IF()` 不使用阈值！因此，将比较结果作为其第一个参数是安全的，但算术运算符可能会产生意想不到的结果。例如，以下两个调用会产生 *不同* 的结果，即使它们在逻辑上是等价的：

<!-- request IF() -->
```sql
IF ( sqrt(3)*sqrt(3)-3<>0, a, b )
IF ( sqrt(3)*sqrt(3)-3, a, b )
```

<!-- end -->

在第一种情况下，因为阈值的存在，比较运算符 <> 将返回 0.0（假），因此 `IF()` 总是返回 `**` 作为结果。在第二种情况下，相同的 `sqrt(3)*sqrt(3)-3` 表达式将由 `IF()` 函数自身与零进行比较 *没有* 阈值。然而，由于浮点计算精度有限，其值将稍微不同于零。因此，`IF()` 进行的与 0.0 的比较将不通过，第二种变体将返回 'a' 作为结果。

### HISTOGRAM()
<!-- example histogram -->
`HISTOGRAM(expr, {hist_interval=size, hist_offset=value})` 接受一个桶大小，并返回该值的桶编号。关键函数是：
```sql
key_of_the_bucket = interval + offset * floor ( ( value - offset ) / interval )
```
Histogram 参数 `interval` 必须为正数。Histogram 参数 `offset` 必须为正数且小于 `interval`。它用于聚合、`FACET` 和分组。

<!-- intro -->
示例：

<!-- request HISTOGRAM() -->
```sql
SELECT COUNT(*),
HISTOGRAM(price, {hist_interval=100}) as price_range
FROM facets
GROUP BY price_range ORDER BY price_range ASC;
```
<!-- end -->

### IN()
`IN(expr,val1,val2,...)` 接受 2 个或更多参数，如果第一个参数（expr）等于其他参数（val1..valN）中的任意一个，则返回 1，否则返回 0。目前，所有检查的值（但不包括表达式本身）都要求为常量。常量是预先排序的，并使用二进制搜索，因此即使针对一个大的任意常量列表，`IN()` 也会非常快速。第一个参数也可以是 MVA 属性。在这种情况下，如果任何 MVA 值等于其他参数中的任意一个，则 `IN()` 将返回 1。`IN()` 还支持 `IN(expr,@uservar)` 语法，以检查值是否属于给定全局用户变量中的列表。第一个参数可以是 JSON 属性。

### INDEXOF()
`INDEXOF(cond FOR var IN json.array)` 函数遍历数组中的所有元素，并返回第一个使 'cond' 为真的元素的索引，如果数组中每个元素的 'cond' 都为假，则返回 -1。

### INTERVAL()
`INTERVAL(expr,point1,point2,point3,...)` 接受 2 个或更多参数，并返回小于第一个参数的参数的索引：如果 `expr<point1`，则返回 0；如果 `point1<=expr<point2`，则返回 1，以此类推。要求 `point1<point2<...<pointN`，才能使该函数正常工作。

### LENGTH()
`LENGTH(attr_mva)` 函数返回 MVA 集合中的元素数量。它适用于 32 位和 64 位 MVA 属性。`LENGTH(attr_json)` 返回 JSON 中字段的长度。返回值取决于字段的类型。例如，`LENGTH(json_attr.some_int)` 始终返回 1，而 `LENGTH(json_attr.some_array)` 返回数组中的元素数量。`LENGTH(string_expr)` 函数返回表达式生成的字符串的长度。
[TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29) 必须将表达式括起来，无论表达式是返回非字符串值还是仅仅是字符串属性。

### RANGE()
<!-- example range -->
`RANGE(expr, {range_from=value,range_to=value})` 接受一组范围，并返回该值的桶号。
该表达式包含 `range_from` 值，并排除每个范围的 `range_to` 值。范围可以是开放的 - 仅包含 `range_from` 或仅包含 `range_to` 值。它用于聚合、`FACET` 和分组。

<!-- intro -->
示例：

<!-- request RANGE() -->
```sql
SELECT COUNT(*),
RANGE(price, {range_to=150},{range_from=150,range_to=300},{range_from=300}) price_range
FROM facets
GROUP BY price_range ORDER BY price_range ASC;
```
<!-- end -->

### REMAP()
<!-- example remap -->
`REMAP(condition, expression, (cond1, cond2, ...), (expr1, expr2, ...))` 函数允许您根据条件值对表达式值进行一些例外处理。条件表达式应始终返回整数，而表达式可以返回整数或浮点数。

<!-- intro -->
示例：

<!-- request REMAP() -->
```sql
SELECT id, size, REMAP(size, 15, (5,6,7,8), (1,1,2,2)) s
FROM products
ORDER BY s ASC;
```
<!-- request Another example -->
```sql
SELECT REMAP(userid, karmapoints, (1, 67), (999, 0)) FROM users;
SELECT REMAP(id%10, salary, (0), (0.0)) FROM employes;
```
<!-- end -->

这将使 5 和 6 尺寸的文档排在最前面，接着是 7 和 8 尺寸。如果有一个原始值未在数组中列出（例如，尺寸为 10），则将默认为 15，并且在这种情况下，将被放置在最后。


<!-- proofread -->
