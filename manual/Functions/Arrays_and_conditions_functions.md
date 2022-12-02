# Arrays and conditions functions

### ALL()
`ALL(cond FOR var IN json.array)` applies to JSON arrays and returns 1 if condition is true for all elements in array and 0 otherwise. `cond` is a general expression which additionally can use `var` as current value of an array element within itself.

```sql
SELECT ALL(x>3 AND x<7 FOR x IN j.intarray) FROM test;
```

`ALL(mva)` is a special constructor for multi value attributes. When used in conjunction with comparison operators (including comparison with `IN()`) it returns 1 if all values from the mva attribute are found among the compared values.

```sql
SELECT * FROM test WHERE ALL(mymva)>10;
```

`ALL(string list)` is a special operation for filtering string tags.    

```sql
SELECT * FROM test WHERE tags ALL('foo', 'bar', 'fake');
SELECT * FROM test WHERE tags NOT ALL('true', 'text', 'tag');
```

Here assumed that table 'test' has string attribute 'tags' with set of words (tags), separated by whitespace. If all of the words enumerated as arguments of [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29) present in the attribute, filter matches. Optional 'NOT' inverses the logic. For example, attr containing 'buy iphone cheap' will be matched by `ALL('cheap', 'iphone')`, but will not match `ALL('iphone', '5s')`.

This filter internally uses doc-by-doc matching, so in case of full scan query it might be very slow. It is intended originally for attributes which are not indexed, like calculated expressions or tags in pq tables.

if you like such filtering and want to use it in production, consider the solution to put the 'tags' attribute as full-text field, and then use FT operator `match()` which will invoke full-text indexed search.       

### ANY()
`ANY(cond FOR var IN json.array)` works similar to [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29) except for it returns 1 if condition is true for any element in array.

`ANY(mva)` is a special constructor for multi value attributes. When used in conjunction with comparison operators (including comparison with `IN()`) it returns 1 if any of the mva values are found among the compared values. ANY is used by default if no constructor is used, however a warning will be raised about missing constructor.

`ANY(string list)` is a special operation for filtering string tags.  Works similar to [ALL()](../Functions/Arrays_and_conditions_functions.md#ALL%28%29) except if condition is true for the case when any tag of tested expression match.

```sql
SELECT * FROM test WHERE tags NOT ANY('true', 'text', 'tag');
SELECT TO_STRING(id*321) secret FROM test WHERE secret ANY('1000','3210');
```

### CONTAINS()

`CONTAINS(polygon, x, y)` checks whether the (x,y) point is within the given polygon, and returns 1 if true, or 0 if false. The polygon has to be specified using either the  [POLY2D()](../Functions/Geo_spatial_functions.md#POLY2D%28%29) function. The former function is intended for "small" polygons, meaning less than 500 km (300 miles) a side, and it doesn't take into account the Earth's curvature for speed. For larger distances, you should use `GEOPOLY2D`, which tessellates the given polygon in smaller parts, accounting for the Earth's curvature.

### IF()

`IF()` behavior is slightly different than its MySQL counterpart. It takes 3 arguments, checks whether the 1st argument is equal to 0.0, returns the 2nd argument if it is not zero or the 3rd one when it is. Note that unlike comparison operators, `IF()` does **not** use a threshold! Therefore, it's safe to use comparison results as its 1st argument, but arithmetic operators might produce unexpected results. For instance, the following two calls will produce *different* results even though they are logically equivalent:

```sql
IF ( sqrt(3)*sqrt(3)-3<>0, a, b )
IF ( sqrt(3)*sqrt(3)-3, a, b )
```

In the first case, the comparison operator <> will return 0.0 (false) because of a threshold, and `IF()` will always return `**` as a result. In the second one, the same `sqrt(3)*sqrt(3)-3` expression will be compared with zero *without* threshold by the `IF()` function itself. But its value will be slightly different from zero because of limited floating point calculations precision. Because of that, the comparison with 0.0 done by `IF()` will not pass, and the second variant will return 'a' as a result.


### IN()
`IN(expr,val1,val2,...)` takes 2 or more arguments, and returns 1 if 1st argument (expr) is equal to any of the other arguments (val1..valN), or 0 otherwise. Currently, all the checked values (but not the expression itself!) are required to be constant. The constants are pre-sorted and then binary search is used, so `IN()` even against a big arbitrary list of constants will be very quick. First argument can also be an MVA attribute. In that case, `IN()` will return 1 if any of the MVA values is equal to any of the other arguments. IN() also supports `IN(expr,@uservar)` syntax to check whether the value belongs to the list in the given global user variable. First argument can be JSON attribute.

### INDEXOF()
`INDEXOF(cond FOR var IN json.array)` function iterates through all elements in array and returns index of first element for which 'cond' is true and -1 if 'cond' is false for every element in array.

### INTERVAL()
`INTERVAL(expr,point1,point2,point3,...)`, takes 2 or more arguments, and returns the index of the argument that is less than the first argument: it returns 0 if `expr<point1`, 1 if `point1<=expr<point2`, and so on. It is required that `point1<point2<...<pointN` for this    function to work correctly.


### LENGTH()
`LENGTH(attr_mva)` function returns amount of elements in MVA set. It works with both 32-bit and 64-bit MVA attributes. LENGTH(attr_json) returns length of a field in JSON. Return value depends on type of a field. For example LENGTH(json_attr.some_int) always returns 1 and LENGTH(json_attr.some_array) returns number of elements in array. LENGTH(string_expr) function returns the length of the string resulted from an expression.
[TO_STRING()](../Functions/Type_casting_functions.md#TO_STRING%28%29) must enclose the expression, regardless if the expression returns a   non-string or it's simply a string attribute.

### REMAP()
`REMAP(condition, expression, (cond1, cond2, ...), (expr1, expr2, ...))` function allows you to make some exceptions of an expression values depending on condition values. Condition expression should always result integer, expression can result in integer or float.

Example:

```sql
SELECT id, size, REMAP(size, 15, (5,6,7,8), (1,1,2,2)) s
FROM products
ORDER BY s ASC;
```

This will put documents with sizes 5 and 6 first, lower will go sizes 7 an 8. In case there's an original value not listed in the array (e.g. size 10) it will be defaulted to 15 and in this case will be put to the end.

More examples:

```sql
SELECT REMAP(userid, karmapoints, (1, 67), (999, 0)) FROM users;
SELECT REMAP(id%10, salary, (0), (0.0)) FROM employes;
```
