# 数组和条件函数

### ALL()
<!-- example all_json -->
`ALL(cond FOR var IN json.array)` 适用于 JSON 数组，当数组中所有元素都满足条件时返回 1，否则返回 0。`cond` 是一个通用表达式，也可以在其中使用 `var` 作为当前数组元素的值。

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
`ALL(mva)` 是多值属性的特殊构造函数。当与比较运算符（包括与 `IN()` 的比较）一起使用时，如果所有多值属性的值都在比较值中，则返回 1。

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
要将多值属性与数组进行比较，避免使用 `<mva> NOT ALL()`；请改用 `ALL(<mva>) NOT IN()`。

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

`ALL(string list)` 是用于过滤字符串标签的特殊操作。

如果作为 `ALL()` 参数枚举的所有单词都出现在属性中，则过滤器匹配。可选的 `NOT` 会反转逻辑。

此过滤器内部使用逐文档匹配，因此在全表扫描查询中可能比预期慢。它适用于非索引属性，如计算表达式或 PQ 表中的标签。如果需要此类过滤，建议将字符串属性设置为全文字段，然后使用全文操作符 `match()`，以调用全文搜索。

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
`ANY(cond FOR var IN json.array)` 适用于 JSON 数组，当数组中任意元素满足条件时返回 1，否则返回 0。`cond` 是一个通用表达式，也可以在其中使用 `var` 作为当前数组元素的值。

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
`ANY(mva)` 是多值属性的特殊构造函数。当与比较运算符（包括与 `IN()` 的比较）一起使用时，如果任意多值属性的值出现在比较值中，则返回 1。

当使用 `IN()` 比较数组时，默认假定使用 `ANY()`，但会发出缺少构造函数的警告。

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
要将多值属性与数组进行比较，避免使用 `<mva> NOT ANY()`；请改用 `<mva> NOT IN()` 或 `ANY(<mva>) NOT IN()`。

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

如果作为 `ANY()` 参数枚举的任意单词出现在属性中，则过滤器匹配。可选的 `NOT` 会反转逻辑。

此过滤器内部使用逐文档匹配，因此在全表扫描查询中可能比预期慢。它适用于非索引属性，如计算表达式或 PQ 表中的标签。如果需要此类过滤，建议将字符串属性设置为全文字段，然后使用全文操作符 `match()`，以调用全文搜索。

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

`CONTAINS(polygon, x, y)` 检查点 (x,y) 是否在给定多边形内，是则返回 1，否则返回 0。多边形必须通过 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) 函数指定。该函数适用于“较小”多边形，即边长小于 500 公里（300 英里），且为了速度不考虑地球曲率。对于较大距离，应使用 `GEOPOLY2D`，它会将多边形切分为更小部分，考虑地球曲率。

### IF()

<!-- example if -->
`IF()` 的行为与 MySQL 的类似函数略有不同。它接受 3 个参数，检查第 1 个参数是否等于 0.0，如果不为零返回第 2 个参数，否则返回第 3 个。注意，与比较运算符不同，`IF()` **不使用**阈值！因此，使用比较结果作为第 1 个参数是安全的，但算术运算符可能导致意外结果。例如，以下两个调用将产生 *不同* 的结果，尽管它们逻辑上等价：

<!-- request IF() -->
```sql
IF ( sqrt(3)*sqrt(3)-3<>0, a, b )
IF ( sqrt(3)*sqrt(3)-3, a, b )
```

<!-- end -->

第一种情况下，比较运算符 <> 会因阈值返回 0.0（假），`IF()` 总是返回 `**` 作为结果。第二种情况下，同样的表达式 `sqrt(3)*sqrt(3)-3` 将会被 `IF()` 函数本身 *不带阈值* 地与零比较。然而，由于有限的浮点计算精度，其值将略有差异，不等于零。因此 `IF()` 与 0.0 的比较不通过，第二个变体将返回 'a'。

### HISTOGRAM()
<!-- example histogram -->
`HISTOGRAM(expr, {hist_interval=size, hist_offset=value})` 接受桶大小并返回该值所在的桶编号。关键函数为：
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
`IN(expr,val1,val2,...)` 接受 2 个或以上参数，如果第一个参数（expr）等于其他任何一个参数（val1..valN），则返回 1，否则返回 0。目前，所有被检查的值（但不包括表达式本身）都要求是常量。常量被预排序，使用二分查找，因此即使 `IN()` 针对大型任意常量列表，速度也非常快。第一个参数也可以是多值属性（MVA）。在这种情况下，如果任何 MVA 值等于其他任一参数，`IN()` 将返回 1。`IN()` 也支持 `IN(expr,@uservar)` 语法，用来检查值是否属于给定的全局用户变量中的列表。第一个参数也可以是 JSON 属性。

### INDEXOF()
`INDEXOF(cond FOR var IN json.array)` 函数遍历数组中的所有元素，返回第一个使得 'cond' 为真的元素的索引，如果数组中没有元素满足 'cond'，则返回 -1。

### INTERVAL()
`INTERVAL(expr,point1,point2,point3,...)` 接受 2 个或以上参数，返回第一个小于给定表达式的参数的索引：如果 `expr < point1` 返回 0，如果 `point1 <= expr < point2` 返回 1，依此类推。要求 `point1 < point2 < ... < pointN` 以确保函数正常工作。

### LENGTH()
`LENGTH(attr_mva)` 函数返回多值属性（MVA）集合中的元素数量，适用于 32 位和 64 位 MVA 属性。`LENGTH(attr_json)` 返回 JSON 字段的长度。返回值依赖于字段类型。例如，`LENGTH(json_attr.some_int)` 总是返回 1，而 `LENGTH(json_attr.some_array)` 返回数组中元素的数量。`LENGTH(string_expr)` 返回表达式结果字符串的长度。
[TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29) 必须包裹表达式，无论表达式返回的是非字符串还是纯字符串属性。

### RANGE()
<!-- example range -->
`RANGE(expr, {range_from=value,range_to=value})` 接受一组范围并返回该值所在的桶编号。
该表达式包括每个范围的 `range_from` 值，但不包括 `range_to` 值。范围可以是开区间 — 只有 `range_from` 或只有 `range_to`。此表达式用于聚合、`FACET` 和分组。

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
`REMAP(condition, expression, (cond1, cond2, ...), (expr1, expr2, ...))` 函数允许根据条件值对表达式值做些例外处理。条件表达式应返回整数，表达式可以返回整数或浮点数。

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

这将把大小为 5 和 6 的文档排在前面，接着是大小为 7 和 8 的文档。如果存在未列出在数组中的原始值（例如大小 10），默认将被映射为 15，这种情况下会排在最后。


<!-- proofread -->

