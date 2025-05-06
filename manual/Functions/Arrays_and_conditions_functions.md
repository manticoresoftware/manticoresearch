# Arrays and conditions functions

### ALL()
<!-- example all_json -->
`ALL(cond FOR var IN json.array)` applies to JSON arrays and returns 1 if the condition is true for all elements in the array and 0 otherwise. `cond` is a general expression that can also use `var` as the current value of an array element within itself.

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
`ALL(mva)` is a special constructor for multi-value attributes. When used with comparison operators (including comparison with `IN()`), it returns 1 if all values from the MVA attribute are found among the compared values.

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
To compare an MVA attribute with an array, avoid using `<mva> NOT ALL()`; use `ALL(<mva>) NOT IN()` instead.

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

`ALL(string list)` is a special operation for filtering string tags.

If all of the words enumerated as arguments of `ALL()` are present in the attribute, the filter matches. The optional `NOT` inverts the logic.

This filter internally uses doc-by-doc matching, so in the case of a full scan query, it might be slower than expected. It is intended for attributes that are not indexed, like calculated expressions or tags in PQ tables. If you need such filtering, consider the solution of putting the string attribute as a full-text field, and then use the full-text operator `match()`, which will invoke a full-text search.

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
`ANY(cond FOR var IN json.array)` applies to JSON arrays and returns 1 if the condition is true for any element in the array and 0 otherwise. `cond` is a general expression that can also use `var` as the current value of an array element within itself.

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
`ANY(mva)` is a special constructor for multi-value attributes. When used with comparison operators (including comparison with `IN()`), it returns 1 if any of the MVA values is found among the compared values.

When comparing an array using `IN()`, `ANY()` is assumed by default if not otherwise specified, but a warning will be issued regarding the missing constructor.

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
To compare an MVA attribute with an array, avoid using `<mva> NOT ANY()`; use `<mva> NOT IN()` instead or `ANY(<mva>) NOT IN()`.

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

`ANY(string list)` is a special operation for filtering string tags.

If any of the words enumerated as arguments of `ANY()` is present in the attribute, the filter matches. The optional `NOT` inverts the logic.

This filter internally uses doc-by-doc matching, so in the case of a full scan query, it might be slower than expected. It is intended for attributes that are not indexed, like calculated expressions or tags in PQ tables. If you need such filtering, consider the solution of putting the string attribute as a full-text field, and then use the full-text operator `match()`, which will invoke a full-text search.

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

`CONTAINS(polygon, x, y)` checks whether the (x,y) point is within the given polygon, and returns 1 if true, or 0 if false. The polygon has to be specified using either the  [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) function. The former function is intended for "small" polygons, meaning less than 500 km (300 miles) a side, and it doesn't take into account the Earth's curvature for speed. For larger distances, you should use `GEOPOLY2D`, which tessellates the given polygon in smaller parts, accounting for the Earth's curvature.

### IF()

<!-- example if -->
The behavior of `IF()` is slightly different from its MySQL counterpart. It takes 3 arguments, checks whether the 1st argument is equal to 0.0, returns the 2nd argument if it is not zero, or the 3rd one when it is. Note that unlike comparison operators, `IF()` does **not** use a threshold! Therefore, it's safe to use comparison results as its 1st argument, but arithmetic operators might produce unexpected results. For instance, the following two calls will produce *different* results even though they are logically equivalent:

<!-- request IF() -->
```sql
IF ( sqrt(3)*sqrt(3)-3<>0, a, b )
IF ( sqrt(3)*sqrt(3)-3, a, b )
```

<!-- end -->

In the first case, the comparison operator <> will return 0.0 (false) due to a threshold, and `IF()` will always return `**` as a result. In the second case, the same `sqrt(3)*sqrt(3)-3` expression will be compared with zero *without* a threshold by the `IF()` function itself. However, its value will be slightly different from zero due to limited floating-point calculation precision. Because of this, the comparison with 0.0 done by `IF()` will not pass, and the second variant will return 'a' as a result.

### HISTOGRAM()
<!-- example histogram -->
`HISTOGRAM(expr, {hist_interval=size, hist_offset=value})` takes a bucket size and returns the bucket number for the value. The key function is:
```sql
key_of_the_bucket = interval + offset * floor ( ( value - offset ) / interval )
```
The histogram argument `interval` must be positive. The histogram argument `offset` must be positive and less than `interval`. It is used in aggregation, `FACET`, and grouping.

<!-- intro -->
Example:

<!-- request HISTOGRAM() -->
```sql
SELECT COUNT(*),
HISTOGRAM(price, {hist_interval=100}) as price_range
FROM facets
GROUP BY price_range ORDER BY price_range ASC;
```
<!-- end -->

### IN()
`IN(expr,val1,val2,...)` takes 2 or more arguments and returns 1 if the 1st argument (expr) is equal to any of the other arguments (val1..valN), or 0 otherwise. Currently, all the checked values (but not the expression itself) are required to be constant. The constants are pre-sorted, and binary search is used, so `IN()` even against a large arbitrary list of constants will be very quick. The first argument can also be an MVA attribute. In that case, `IN()` will return 1 if any of the MVA values are equal to any of the other arguments. `IN()` also supports `IN(expr,@uservar)` syntax to check whether the value belongs to the list in the given global user variable. The first argument can be a JSON attribute.

### INDEXOF()
`INDEXOF(cond FOR var IN json.array)` function iterates through all elements in the array and returns the index of the first element for which 'cond' is true, and -1 if 'cond' is false for every element in the array.

### INTERVAL()
`INTERVAL(expr,point1,point2,point3,...)` takes 2 or more arguments and returns the index of the argument that is less than the first argument: it returns 0 if `expr<point1`, 1 if `point1<=expr<point2`, and so on. It is required that `point1<point2<...<pointN` for this function to work correctly.

### LENGTH()
`LENGTH(attr_mva)` function returns the number of elements in an MVA set. It works with both 32-bit and 64-bit MVA attributes. `LENGTH(attr_json)` returns the length of a field in JSON. The return value depends on the type of field. For example, `LENGTH(json_attr.some_int)` always returns 1, and `LENGTH(json_attr.some_array)` returns the number of elements in the array. `LENGTH(string_expr)` function returns the length of the string resulting from an expression.
[TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29) must enclose the expression, regardless of whether the expression returns a non-string or it's simply a string attribute.

### RANGE()
<!-- example range -->
`RANGE(expr, {range_from=value,range_to=value})` takes a set of ranges and returns the bucket number for the value.
This expression includes the `range_from` value and excludes the `range_to` value for each range. A range can be open - having only the `range_from` or only the `range_to` value. It is used in aggregation, `FACET`, and grouping.

<!-- intro -->
Example:

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
`REMAP(condition, expression, (cond1, cond2, ...), (expr1, expr2, ...))` function allows you to make some exceptions to expression values depending on condition values. The condition expression should always result in an integer, while the expression can result in an integer or float.

<!-- intro -->
Example:

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

This will put documents with sizes 5 and 6 first, followed by sizes 7 and 8. In case there's an original value not listed in the array (e.g. size 10), it will default to 15, and in this case, will be placed at the end.


<!-- proofread -->
