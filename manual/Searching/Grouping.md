# Grouping search results

<!-- example general -->
It's often useful to group search results to get per-group match counts or  other aggregations. For instance, to draw a nice graph of how much matching blog posts were there per month or to group web search results by site or forum posts by author etc.

Manticore supports grouping of search results by one or multiple columns and computed expressions. The results can:

* be sorted inside a group
* have more than one row returned per group
* have groups filtered
* have groups sorted
* be aggregated with help of [](Searching/Grouping.md#Aggregation-functions)

<!-- intro -->
The general syntax is:

<!-- request SQL -->
General syntax
```sql
SELECT {* | select_expr [, select_expr ...]}
...
GROUP BY {field_name | alias } [, ...]
[HAVING where_condition]
[WITHIN GROUP ORDER BY field_name {ASC | DESC} [, ...]]
...

select_expr: { field_name | function_name(...) }
where_condition: {aggregation expression alias | COUNT(*)}
```

<!-- request HTTP -->
HTTP supports currently a simple grouping that can retrieve the aggregate values and their count(*).

```json
{
  "index": "<index_name>",
  "limit": 0,
  "aggs": {
    "<aggr_name>": {
      "terms": {
        "field": "<attribute>",
        "size": <int value>
      }
    }
  }
}
```

The normal query output returns the result set without grouping and can be hidden with `limit` (or `size`). 
The aggregation requires to set a `size` for the size of the result set group.  

<!-- end -->

<!-- example group1 -->
### Just Grouping
Grouping is very simple - just add "GROUP BY smth" to the end of your `SELECT` query. The something can be:

* any non-full-text field from the index: integer, float, string, MVA (multi-value attribute)
* or if you used an alias in the `SELECT` list - you can group by it too

You can omit any [aggregation functions](Searching/Grouping.md#Aggregation-functions) in the `SELECT` list and it will work too:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select release_year from films group by release_year limit 5;
```
<!-- response SQL -->
```sql
+--------------+
| release_year |
+--------------+
|         2004 |
|         2002 |
|         2001 |
|         2005 |
|         2000 |
+--------------+
```
<!-- end -->
<!-- example group2 -->
But in most cases you want to get something aggregated for each group, for example:

* `COUNT(*)` to just get number of elements in each groups
* or `AVG(field)` to get an average value of the field in the group


<!-- intro -->
##### Example:

<!-- request SQL1 -->
```sql
select release_year, count(*) from films group by release_year limit 5;
```
<!-- response SQL1 -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2004 |      108 |
|         2002 |      108 |
|         2001 |       91 |
|         2005 |       93 |
|         2000 |       97 |
+--------------+----------+
```
<!-- request SQL2 -->
```sql
select release_year, avg(rental_rate) from films group by release_year limit 5;
```
<!-- response SQL2 -->
```sql
+--------------+------------------+
| release_year | avg(rental_rate) |
+--------------+------------------+
|         2004 |       2.78629661 |
|         2002 |       3.08259249 |
|         2001 |       3.09989142 |
|         2005 |       2.90397978 |
|         2000 |       3.17556739 |
+--------------+------------------+
```

<!-- request HTTP -->
``` json
POST /search -d '
    {
     "index" : "films",
     "limit": 0,
     "aggs" :
     {
        "release_year" :
         {
            "terms" :
             {
              "field":"release_year",
              "size":100
             }
         }
     }
    }
'
```
<!-- response HTTP -->
``` json
{
  "took": 2,
  "timed_out": false,
  "hits": {
    "total": 10000,
    "hits": [
      
    ]
  },
  "release_year": {
    "group_brand_id": {
      "buckets": [
        {
          "key": 2004,
          "doc_count": 108
        },
        {
          "key": 2002,
          "doc_count": 108
        },
        {
          "key": 2000,
          "doc_count": 97
        },
        {
          "key": 2005,
          "doc_count": 93
        },
        {
          "key": 2001,
          "doc_count": 91
        }
      ]
    }
  }
}
```

<!-- end -->

<!-- example sort1 -->
##### Sorting groups
By default the groups are not sorted and the next thing you normally want to do is to order them by something. For example the field you are grouping by:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select release_year, count(*) from films group by release_year order by release_year asc limit 5;
```
<!-- response SQL -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2000 |       97 |
|         2001 |       91 |
|         2002 |      108 |
|         2003 |      106 |
|         2004 |      108 |
+--------------+----------+
```
<!-- end -->
<!-- example sort2 -->
Or vice-versa - by the aggregation:

* by `count(*)` to see those groups that have most elements first
* by `avg(rental_rate)` to see most rated movies first. Note that in the example it's done via an alias: `avg(rental_rate)` is first mapped to `avg` in the `SELECT` list and then we just do `order by avg`


<!-- intro -->
##### Example:

<!-- request SQL1 -->
```sql
select release_year, count(*) from films group by release_year order by count(*) desc limit 5;
```
<!-- response SQL1 -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2004 |      108 |
|         2002 |      108 |
|         2003 |      106 |
|         2006 |      103 |
|         2008 |      102 |
+--------------+----------+
```

<!-- request SQL2 -->
```sql
select release_year, avg(rental_rate) avg from films group by release_year order by avg desc limit 5;
```
<!-- response SQL2 -->
```sql
+--------------+------------+
| release_year | avg        |
+--------------+------------+
|         2006 | 3.26184368 |
|         2000 | 3.17556739 |
|         2001 | 3.09989142 |
|         2002 | 3.08259249 |
|         2008 | 2.99000049 |
+--------------+------------+
```
<!-- end -->

<!-- example group3 -->
##### Group by multiple fields at once
In some cases you might want to group not by a single, but by multiple fields at once, for example movie's category and year:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select category_id, release_year, count(*) from films group by category_id, release_year order by category_id ASC, release_year ASC;
```
<!-- response SQL -->
```sql
+-------------+--------------+----------+
| category_id | release_year | count(*) |
+-------------+--------------+----------+
|           1 |         2000 |        5 |
|           1 |         2001 |        2 |
|           1 |         2002 |        6 |
|           1 |         2003 |        6 |
|           1 |         2004 |        5 |
|           1 |         2005 |       10 |
|           1 |         2006 |        4 |
|           1 |         2007 |        5 |
|           1 |         2008 |        7 |
|           1 |         2009 |       14 |
|           2 |         2000 |       10 |
|           2 |         2001 |        5 |
|           2 |         2002 |        6 |
|           2 |         2003 |        6 |
|           2 |         2004 |       10 |
|           2 |         2005 |        4 |
|           2 |         2006 |        5 |
|           2 |         2007 |        8 |
|           2 |         2008 |        8 |
|           2 |         2009 |        4 |
+-------------+--------------+----------+
```
<!-- end -->

<!-- example group4 -->
##### Give me N rows
Sometimes it's useful to see not a single element per group, but multiple. This can be easily done with help of `GROUP N BY`. For example here you can see that we get 2 movies for each year rather than one which would simple `GROUP BY release_year` returned.

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select release_year, title from films group 2 by release_year order by release_year desc limit 6;
```
<!-- response SQL -->
```sql
+--------------+-----------------------------+
| release_year | title                       |
+--------------+-----------------------------+
|         2009 | ALICE FANTASIA              |
|         2009 | ALIEN CENTER                |
|         2008 | AMADEUS HOLY                |
|         2008 | ANACONDA CONFESSIONS        |
|         2007 | ANGELS LIFE                 |
|         2007 | ARACHNOPHOBIA ROLLERCOASTER |
+--------------+-----------------------------+
```
<!-- end -->

<!-- example group5 -->
##### Sorting inside a group
Another essential analytics demand is to sort elements within a group. For that there's `WITHIN GROUP ORDER BY ... {ASC|DESC}` clause. For example let's get the most rated film for each year. Note it works in parallel with just `ORDER BY`:

* `WITHIN GROUP ORDER BY` sorts results **inside a group**
* while just `GROUP BY` **sorts the groups themselves**

The work absolutely independently.


<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select release_year, title, rental_rate from films group by release_year within group order by rental_rate desc order by release_year desc limit 5;
```
<!-- response SQL -->
```sql
+--------------+------------------+-------------+
| release_year | title            | rental_rate |
+--------------+------------------+-------------+
|         2009 | AMERICAN CIRCUS  |    4.990000 |
|         2008 | ANTHEM LUKE      |    4.990000 |
|         2007 | ATTACKS HATE     |    4.990000 |
|         2006 | ALADDIN CALENDAR |    4.990000 |
|         2005 | AIRPLANE SIERRA  |    4.990000 |
+--------------+------------------+-------------+
```
<!-- end -->

<!-- example group6 -->
##### Filter groups
`HAVING expression` is a useful clause to filter groups. If `WHERE` is applied before grouping `HAVING` works with the groups. For example let's leave only those years when an average rental rate of the films of that year was higher than 3. We get only 4 years:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select release_year, avg(rental_rate) avg from films group by release_year having avg > 3;
```
<!-- response SQL -->
```sql
+--------------+------------+
| release_year | avg        |
+--------------+------------+
|         2002 | 3.08259249 |
|         2001 | 3.09989142 |
|         2000 | 3.17556739 |
|         2006 | 3.26184368 |
+--------------+------------+
```
<!-- end -->

<!-- example group7 -->
##### GROUPBY()
There is a function `GROUPBY()` which returns the key of the current group. It's useful in many cases especially when you [group by an MVA](Searching/Grouping.md#Grouping-by-MVA-%28multi-value-attributes%29) or a [JSON value](Searching/Grouping.md#Grouping-by-a-JSON-node).

It can be also used in `HAVING` to for example leave only years 2000 and 2002.

Note that `GROUPBY()`is not recommended for use when you group by multiple fields at once. It will still work, but since the group key in this case is compound of field values it may look not the way you expect.

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select release_year, count(*) from films group by release_year having groupby() IN (2000, 2002);
```
<!-- response SQL -->
```sql
+--------------+----------+
| release_year | count(*) |
+--------------+----------+
|         2002 |      108 |
|         2000 |       97 |
+--------------+----------+
```
<!-- end -->
<!-- example mva -->
##### Grouping by MVA (multi-value attributes)
Manticore supports grouping by [MVA](Creating_an_index/Data_types.md#Multi-value-integer-%28MVA%29). To show how it works let's create a table "shoes" with MVA "sizes" and insert few documents into it:
```sql
create table shoes(title text, sizes multi);
insert into shoes values(0,'nike',(40,41,42)),(0,'adidas',(41,43)),(0,'reebook',(42,43));
```
so we have:
```sql
select * from shoes;
+---------------------+----------+---------+
| id                  | sizes    | title   |
+---------------------+----------+---------+
| 1657851069130080265 | 40,41,42 | nike    |
| 1657851069130080266 | 41,43    | adidas  |
| 1657851069130080267 | 42,43    | reebook |
+---------------------+----------+---------+
```
If we now group by "sizes" it will process all our multi-value attributes and will return aggregation for each, in this case just count:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select groupby() gb, count(*) from shoes group by sizes order by gb asc;
```
<!-- response SQL -->
```sql
+------+----------+
| gb   | count(*) |
+------+----------+
|   40 |        1 |
|   41 |        2 |
|   42 |        2 |
|   43 |        2 |
+------+----------+
```
<!-- end -->

<!-- example json -->
##### Grouping by a JSON node
If you have a field of type [JSON](Creating_an_index/Data_types.md#JSON) you can group by any node from it. To demonstrate it let's create a table "products" with few documents each having color in the "meta" JSON field:
```sql
create table products(title text, meta json);
insert into products values(0,'nike','{"color":"red"}'),(0,'adidas','{"color":"red"}'),(0,'puma','{"color":"green"}');
```
This gives us:
```sql
select * from products;
+---------------------+-------------------+--------+
| id                  | meta              | title  |
+---------------------+-------------------+--------+
| 1657851069130080268 | {"color":"red"}   | nike   |
| 1657851069130080269 | {"color":"red"}   | adidas |
| 1657851069130080270 | {"color":"green"} | puma   |
+---------------------+-------------------+--------+
```
To group the products by color we can just `GROUP BY meta.color` and to show the corresponding group key in the `SELECT` list we can use `GROUPBY()`:

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select groupby() color, count(*) from products group by meta.color;
```
<!-- response SQL -->
```sql
+-------+----------+
| color | count(*) |
+-------+----------+
| red   |        2 |
| green |        1 |
+-------+----------+
```
<!-- end -->

## Aggregation functions
Besides `COUNT(*)` which returns number of elements in each group you can use different other aggregation functions:
<!-- example distinct -->
##### COUNT(DISTINCT field)
While `COUNT(*)` returns number of all elements in the group `COUNT( DISTINCT field)` returns number of different values of the field in the group which may be absolutely different from the total count: you can have 100 elements in the group, but all with the same value of some field. `COUNT(DISTINCT field)` helps to figure that out. To demonstrate it let's create table "students" with student's name, age and major:
```sql
create table students(name text, age int, major string);
insert into students values(0,'John',21,'arts'),(0,'William',22,'business'),(0,'Richard',21,'cs'),(0,'Rebecca',22,'cs'),(0,'Monica',21,'arts');
```

so we have:

```sql
MySQL [(none)]> select * from students;
+---------------------+------+----------+---------+
| id                  | age  | major    | name    |
+---------------------+------+----------+---------+
| 1657851069130080271 |   21 | arts     | John    |
| 1657851069130080272 |   22 | business | William |
| 1657851069130080273 |   21 | cs       | Richard |
| 1657851069130080274 |   22 | cs       | Rebecca |
| 1657851069130080275 |   21 | arts     | Monica  |
+---------------------+------+----------+---------+
```

In the example you can see that if we group by major and show both `COUNT(*)` and `COUNT(DISTINCT age)` it gets clear that there are 2 students that chose major "cs" and 2 unique ages, but for the major "arts" there are also 2 students, but only one unique age.

There can be at most one `COUNT(DISTINCT)` per query.

**`COUNT(DISTINCT)` against a distributed index or a real-time index consisting of multiple disk chunks may return inaccurate value.**


<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select major, count(*), count(distinct age) from students group by major;
```
<!-- response SQL -->
```sql
+----------+----------+---------------------+
| major    | count(*) | count(distinct age) |
+----------+----------+---------------------+
| arts     |        2 |                   1 |
| business |        1 |                   1 |
| cs       |        2 |                   2 |
+----------+----------+---------------------+
```
<!-- end -->

<!-- example concat -->
##### GROUP_CONCAT(field)

Often you want to understand better the contents of each group. You can use [GROUP N BY](Searching/Grouping.md#Give-me-N-rows) for that, but it would return additional rows you might not want in the output. `GROUP_CONCAT()` enriches your grouping by concatenating values of some field in the group. Let's take the previous example and improve it by getting all the ages in each group.

`GROUP_CONCAT(field)` returns the list comma-separated.

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select major, count(*), count(distinct age), group_concat(age) from students group by major
```
<!-- response SQL -->
```sql
+----------+----------+---------------------+-------------------+
| major    | count(*) | count(distinct age) | group_concat(age) |
+----------+----------+---------------------+-------------------+
| arts     |        2 |                   1 | 21,21             |
| business |        1 |                   1 | 22                |
| cs       |        2 |                   2 | 21,22             |
+----------+----------+---------------------+-------------------+
```
<!-- end -->
<!-- example sum -->
##### SUM(), MIN(), MAX(), AVG()
And of course you can get sum, average, minimal and maximum values in the group.

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
select release_year year, sum(rental_rate) sum, min(rental_rate) min, max(rental_rate) max, avg(rental_rate) avg from films group by release_year order by year asc limit 5;
```
<!-- response SQL -->
```sql
+------+------------+----------+----------+------------+
| year | sum        | min      | max      | avg        |
+------+------------+----------+----------+------------+
| 2000 | 308.030029 | 0.990000 | 4.990000 | 3.17556739 |
| 2001 | 282.090118 | 0.990000 | 4.990000 | 3.09989142 |
| 2002 | 332.919983 | 0.990000 | 4.990000 | 3.08259249 |
| 2003 | 310.940063 | 0.990000 | 4.990000 | 2.93339682 |
| 2004 | 300.920044 | 0.990000 | 4.990000 | 2.78629661 |
+------+------------+----------+----------+------------+
```
<!-- end -->

<!-- example accuracy -->
## Grouping accuracy
Grouping is done in fixed memory which depends on the [max_matches](Searching/Options.md#max_matches) setting. If the max_matches allows to store all found groups, the results will be 100% correct. The less the value the less accurate will be the results.

<!-- intro -->
##### Example:

<!-- request SQL -->
```sql
MySQL [(none)]> select release_year year, count(*) from films group by year limit 5;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |      108 |
| 2002 |      108 |
| 2001 |       91 |
| 2005 |       93 |
| 2000 |       97 |
+------+----------+

MySQL [(none)]> select release_year year, count(*) from films group by year limit 5 option max_matches=1;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |       76 |
+------+----------+

MySQL [(none)]> select release_year year, count(*) from films group by year limit 5 option max_matches=2;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |       76 |
| 2002 |       74 |
+------+----------+

MySQL [(none)]> select release_year year, count(*) from films group by year limit 5 option max_matches=3;
+------+----------+
| year | count(*) |
+------+----------+
| 2004 |      108 |
| 2002 |      108 |
| 2001 |       91 |
+------+----------+
```
<!-- end -->
