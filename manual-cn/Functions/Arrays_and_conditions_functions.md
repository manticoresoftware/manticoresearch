# 数组与条件函数

### ALL()
<!-- example all_json -->
`ALL(cond FOR var IN json.array)` 适用于 JSON 数组，如果条件对数组中的所有元素都为真，则返回 1，否则返回 0。`cond` 是一个通用表达式，`var` 是数组中当前元素的值。

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
`ALL(mva)` 是一个用于多值属性 (MVA) 的特殊构造函数。与比较运算符结合使用时，如果 MVA 属性的所有值都满足比较条件，则返回 1。

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
要将 MVA 属性与数组进行比较，避免使用 `<mva> NOT ALL()`，而是使用 `ALL(<mva>) NOT IN()`。

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

如果 `ALL()` 枚举的所有单词都在属性中出现，则匹配该过滤器。可选的 `NOT` 用于反转逻辑。

此过滤器在内部使用逐文档匹配，因此在全扫描查询中，它可能比预期的要慢。它适用于未索引的属性，如计算表达式或 PQ 表中的标签。如果您需要这种过滤，建议将字符串属性作为全文字段，并使用全文操作符 `match()`，这将触发全文搜索。

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
`ANY(cond FOR var IN json.array)` 适用于 JSON 数组，如果条件对数组中的任意元素为真，则返回 1，否则返回 0。`cond` 是一个通用表达式，`var` 是数组中当前元素的值。

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

`ANY(mva)` 是一个用于多值属性 (MVA) 的特殊构造函数。与比较运算符结合使用时，如果任意 MVA 值满足比较条件，则返回 1。

当使用 `IN()` 比较数组时，默认假定使用 `ANY()`，除非另有说明，但会发出关于缺少构造函数的警告。

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
要将 MVA 属性与数组进行比较，避免使用 `<mva> NOT ANY()`；而是使用 `<mva> NOT IN()` 或 `ANY(<mva>) NOT IN()`。

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

`ANY(string list)` 是一个用于过滤字符串标签的特殊操作。

如果 `ANY()` 中列出的任意一个单词出现在属性中，则该过滤器匹配。可选的 `NOT` 可以反转匹配逻辑。

该过滤器内部逐文档匹配，因此在全表扫描查询的情况下，可能速度较慢。它主要用于未建立索引的属性，如计算表达式或 PQ 表中的标签。如果需要使用这种过滤方式，建议将字符串属性作为全文字段，并使用全文操作符 `match()`，这样可以调用全文搜索。

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

`CONTAINS(polygon, x, y)` 检查 (x,y) 点是否在给定的多边形内，若为真返回 1，否则返回 0。多边形必须通过 [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D()) 函数指定。前者用于 "小" 多边形（边长小于 500 公里），不考虑地球曲率以提高速度。对于更大距离，应使用 `GEOPOLY2D`，它将给定的多边形细分为更小的部分，以考虑地球的曲率。

### IF()

<!-- example if -->
`IF()` 的行为与 MySQL 中的类似函数略有不同。它接受 3 个参数，检查第一个参数是否等于 0.0，如果不是 0 则返回第二个参数，如果是则返回第三个参数。以下两个调用虽然逻辑上等效，但将产生*不同的*结果：

<!-- request IF() -->

```sql
IF ( sqrt(3)*sqrt(3)-3<>0, a, b )
IF ( sqrt(3)*sqrt(3)-3, a, b )
```

<!-- end -->

在第一个例子中，比较运算符 `<>` 由于阈值的关系将返回 0.0（即 false），因此 `IF()` 总是返回 `**` 作为结果。第二个例子中，相同的 `sqrt(3)*sqrt(3)-3` 表达式将**不带**阈值与 0 进行比较。然而，由于浮点计算精度的限制，其值将略微偏离 0。因为这个原因，`IF()` 函数对 0.0 的比较不会通过，第二个例子将返回 'a' 作为结果。

### HISTOGRAM()
<!-- example histogram -->
`HISTOGRAM(expr, {hist_interval=size, hist_offset=value})` 使用一个桶的大小并返回值所属的桶编号。关键函数为：
```sql
key_of_the_bucket = interval + offset * floor ( ( value - offset ) / interval )
```
`interval` 参数必须为正数。`offset` 参数必须为正数且小于 `interval`。它用于聚合、`FACET` 和分组。

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
`IN(expr,val1,val2,...)` 接受两个或多个参数，若第一个参数（`expr`）等于任意其他参数（`val1..valN`），则返回 1，否则返回 0。目前，所有被比较的值（但不是表达式本身）都必须是常量。常量将被预排序，使用二进制搜索，因此即使是对大规模的常量列表进行 `IN()` 操作也会非常快。第一个参数也可以是 MVA 属性。在这种情况下，如果 MVA 中的任意值等于其他参数中的任意一个，则 `IN()` 返回 1。`IN()` 还支持 `IN(expr,@uservar)` 语法，用于检查该值是否属于给定全局用户变量中的列表。第一个参数可以是 JSON 属性。

### INDEXOF()
`INDEXOF(cond FOR var IN json.array)` 函数遍历数组中的所有元素，并返回满足条件 `cond` 的第一个元素的索引，如果 `cond` 对每个元素都不成立，则返回 -1。

### INTERVAL()
`INTERVAL(expr,point1,point2,point3,...)` 接受两个或更多参数，返回小于第一个参数的值的索引：如果 `expr<point1` 则返回 0，如果 `point1<=expr<point2` 则返回 1，依此类推。要求 `point1<point2<...<pointN` 才能使该函数正常工作。

### LENGTH()
`LENGTH(attr_mva)` 函数返回 MVA 集合中元素的数量。它适用于 32 位和 64 位 MVA 属性。`LENGTH(attr_json)` 返回 JSON 字段的长度。返回值取决于字段类型。例如，`LENGTH(json_attr.some_int)` 始终返回 1，而 `LENGTH(json_attr.some_array)` 返回数组中的元素数量。`LENGTH(string_expr)` 函数返回表达式结果字符串的长度。 [TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING()) 必须括住表达式，无论表达式返回的是非字符串还是简单的字符串属性。

### RANGE()
<!-- example range -->

`RANGE(expr, {range_from=value,range_to=value})` 接受一组范围，并返回值所属的桶编号。 此表达式包含 `range_from` 值，并排除每个范围的 `range_to` 值。范围可以是开放的——仅有 `range_from` 或仅有 `range_to` 值。它用于聚合、`FACET` 和分组。

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
`REMAP(condition, expression, (cond1, cond2, ...), (expr1, expr2, ...))` 函数允许根据条件值对表达式值进行例外处理。条件表达式应始终返回整数，而表达式可以返回整数或浮点数。

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

这将首先显示大小为 5 和 6 的文档，其次是大小为 7 和 8 的文档。如果数组中未列出的原始值（例如大小为 10），它将默认为 15，并在此情况下被放在最后。

<!-- proofread -->
