# 数组和条件函数

### ALL()
<!-- example all_json -->
`ALL(cond FOR var IN json.array)` 应用于 JSON 数组，并返回如果数组中的所有元素满足条件则为 1，否则为 0。`cond` 是一个通用表达式，也可以使用 `var` 表示当前数组元素的值。

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
`ALL(mva)` 是一个多值属性的特殊构造器。当与比较运算符（包括与 `IN()` 的比较）一起使用时，如果多值属性的所有值都在比较值中找到，则返回 1。

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
要将多值属性与数组进行比较，请避免使用 `<mva> NOT ALL()`；请使用 `ALL(<mva>) NOT IN()` 代替。

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

`ALL(string list)` 是一种特殊的字符串标签过滤操作。

如果 `ALL()` 的所有参数列出的单词都存在于该属性中，则该过滤器匹配。可选的 `NOT` 会反转逻辑。

此过滤器内部使用文档级别的匹配，因此在全扫描查询的情况下，可能会比预期更慢。它适用于未索引的属性，例如计算表达式或 PQ 表中的标签。如果您需要此类过滤，请考虑将字符串属性作为全文字段放置，然后使用全文操作符 `match()`，这将触发全文搜索。

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
`ANY(cond FOR var IN json.array)` 应用于 JSON 数组，并返回如果数组中的任何元素满足条件则为 1，否则为 0。`cond` 是一个通用表达式，也可以使用 `var` 表示当前数组元素的值。

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
`ANY(mva)` 是一个多值属性的特殊构造器。当与比较运算符（包括与 `IN()` 的比较）一起使用时，如果多值属性中的任何一个值在比较值中找到，则返回 1。

当使用 `IN()` 比较数组时，默认情况下假设 `ANY()`，但如果没有指定构造器，则会发出警告。

<!-- request ANY() with MVA -->
```sql
mysql> select * from tbl

+------+------+
| id   | m    |
+------+------+
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
要将多值属性与数组进行比较，请避免使用 `<mva> NOT ANY()`；请使用 `<mva> NOT IN()` 或 `ANY(<mva>) NOT IN()` 代替。

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

`ANY(string list)` 是一种特殊的字符串标签过滤操作。

如果 `ANY()` 的参数列出的任何单词都存在于该属性中，则该过滤器匹配。可选的 `NOT` 会反转逻辑。

此过滤器内部使用文档级别的匹配，因此在全扫描查询的情况下，可能会比预期更慢。它适用于未索引的属性，例如计算表达式或 PQ 表中的标签。如果您需要此类过滤，请考虑将字符串属性作为全文字段放置，然后使用全文操作符 `match()`，这将触发全文搜索。

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

`CONTAINS(polygon, x, y)` 检查给定的 (x,y) 点是否在给定的多边形内，并返回如果为真则为 1，否则为 0。多边形必须使用 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) 函数指定。前者函数适用于“小”多边形，意味着每边小于 500 公里（300 英里），并且不考虑地球曲率以提高速度。对于更大的距离，您应该使用 `GEOPOLY2D`，它将给定的多边形分割成较小的部分，并考虑到地球曲率。

### IF()

<!-- example if -->
`IF()` 的行为与 MySQL 中的版本略有不同。它接受三个参数，检查第一个参数是否等于 0.0，如果不为零，则返回第二个参数，如果为零，则返回第三个参数。请注意，与比较运算符不同，`IF()` 不使用阈值！因此，可以安全地将比较结果用作其第一个参数，但算术运算符可能会产生意外的结果。例如，以下两个调用即使它们是逻辑等价的也会产生不同的结果：

<!-- request IF() -->
```sql
IF ( sqrt(3)*sqrt(3)-3<>0, a, b )
IF ( sqrt(3)*sqrt(3)-3, a, b )
```

<!-- end -->

在第一种情况下，比较运算符 <> 由于阈值的原因会返回 0.0（假），`IF()` 总是返回 `**` 作为结果。在第二种情况下，相同的 `sqrt(3)*sqrt(3)-3` 表达式将由 `IF()` 函数本身以没有阈值的方式与零进行比较。然而，由于浮点计算精度有限，其值将略不同于零。因此，`IF()` 对 0.0 进行的比较不会通过，第二种变体将返回 'a' 作为结果。

### HISTOGRAM()
<!-- example histogram -->
`HISTOGRAM(expr, {hist_interval=size, hist_offset=value})` 取一个桶大小并返回值所在的桶号。关键函数是：
```sql
key_of_the_bucket = interval + offset * floor ( ( value - offset ) / interval )
```
直方图参数 `interval` 必须为正。直方图参数 `offset` 必须为正且小于 `interval`。它用于聚合、`FACET` 和分组。

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
`IN(expr,val1,val2,...)` 接受两个或更多参数，并返回如果第一个参数 (expr) 等于其他参数中的任何一个 (val1..valN)，则返回 1，否则返回 0。当前，所有检查的值（但不是表达式本身）都必须是常量。这些常量会被预先排序，并使用二分查找，因此即使面对一个大型任意列表，`IN()` 也会非常快速。第一个参数也可以是一个 MVA 属性。在这种情况下，`IN()` 将返回 1 如果任何 MVA 值等于其他参数中的任何一个。`IN()` 还支持 `IN(expr,@uservar)` 的语法来检查值是否属于给定的全局用户变量中的列表。第一个参数可以是一个 JSON 属性。

### INDEXOF()
`INDEXOF(cond FOR var IN json.array)` 函数遍历数组中的所有元素，并返回第一个满足 'cond' 为真的元素的索引，如果没有元素满足 'cond' 则返回 -1。

### INTERVAL()
`INTERVAL(expr,point1,point2,point3,...)` 接受两个或更多参数，并返回第一个参数小于该参数的参数的索引：如果 `expr<point1` 返回 0，如果 `point1<=expr<point2` 返回 1，依此类推。要求 `point1<point2<...<pointN` 才能正确工作。

### LENGTH()
`LENGTH(attr_mva)` 函数返回 MVA 集合中的元素数量。它适用于 32 位和 64 位 MVA 属性。`LENGTH(attr_json)` 返回 JSON 字段的长度。返回值取决于字段类型。例如，`LENGTH(json_attr.some_int)` 总是返回 1，而 `LENGTH(json_attr.some_array)` 返回数组中的元素数量。`LENGTH(string_expr)` 函数返回从表达式结果得到的字符串长度。
[TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29) 必须包围表达式，无论表达式返回非字符串还是仅仅是字符串属性。

### RANGE()
<!-- example range -->
`RANGE(expr, {range_from=value,range_to=value})` 接受一系列范围并返回值所在的桶号。
此表达式包括每个范围的 `range_from` 值，但不包括 `range_to` 值。范围可以是开放的 - 只有 `range_from` 或只有 `range_to` 值。它用于聚合、`FACET` 和分组。

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
`REMAP(condition, expression, (cond1, cond2, ...), (expr1, expr2, ...))` 函数允许根据条件值对表达式的值进行一些例外处理。条件表达式应始终返回整数，而表达式可以返回整数或浮点数。

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

这将使大小为 5 和 6 的文档首先出现，然后是大小为 7 和 8 的文档。如果存在不在数组中的原始值（例如大小 10），则默认为 15，并在这种情况下将其放在最后。


<!-- proofread -->

