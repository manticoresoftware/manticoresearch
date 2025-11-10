# 日期和时间函数

注意，`CURTIME()`、`UTC_TIME()`、`UTC_TIMESTAMP()` 和 `TIMEDIFF()` 可以通过任意转换函数如 `BIGINT()`、`DOUBLE()` 等提升为数值类型。

### NOW()
<!-- example NOW -->
返回当前时间戳，类型为 INTEGER。

<!-- request SQL -->
```sql
select NOW();
```
<!-- response SQL -->
```sql
+------------+
| NOW()      |
+------------+
| 1615788407 |
+------------+
```
<!-- end -->

### CURTIME()
<!-- example CURTIME -->
返回本地时区的当前时间，格式为 `hh:ii:ss`。

<!-- request SQL -->
```sql
select CURTIME();
```
<!-- response SQL -->
```sql
+-----------+
| CURTIME() |
+-----------+
| 07:06:30  |
+-----------+
```
<!-- end -->

### CURDATE()
<!-- example CURDATE -->
返回本地时区的当前日期，格式为 `YYYY-MM-DD`。

<!-- request SQL -->
```sql
select curdate();
```
<!-- response SQL -->
```sql
+------------+
| curdate()  |
+------------+
| 2023-08-02 |
+------------+
```
<!-- end -->

### UTC_TIME()
<!-- example UTC_TIME -->
返回 UTC 时区的当前时间，格式为 `hh:ii:ss`。

<!-- request SQL -->
```sql
select UTC_TIME();
```
<!-- response SQL -->
```sql
+------------+
| UTC_TIME() |
+------------+
| 06:06:18   |
+------------+
```
<!-- end -->

### UTC_TIMESTAMP()
<!-- example UTC_TIMESTAMP -->
返回 UTC 时区的当前时间，格式为 `YYYY-MM-DD hh:ii:ss`。

<!-- request SQL -->
```sql
select UTC_TIMESTAMP();
```
<!-- response SQL -->
```sql
+---------------------+
| UTC_TIMESTAMP()     |
+---------------------+
| 2021-03-15 06:06:03 |
+---------------------+
```
<!-- end -->

### SECOND()
<!-- example SECOND -->
根据当前时区，从时间戳参数中返回整数秒（范围 0..59）。

<!-- request SQL -->
```sql
select second(now());
```
<!-- response SQL -->
```sql
+---------------+
| second(now()) |
+---------------+
| 52            |
+---------------+
```
<!-- end -->

### MINUTE()
<!-- example MINUTE -->
根据当前时区，从时间戳参数中返回整数分钟（范围 0..59）。

<!-- request SQL -->
```sql
select minute(now());
```
<!-- response SQL -->
```sql
+---------------+
| minute(now()) |
+---------------+
| 5             |
+---------------+
```
<!-- end -->

### HOUR()
<!-- example HOUR -->
根据当前时区，从时间戳参数中返回整数小时（范围 0..23）。

<!-- request SQL -->
```sql
select hour(now());
```
<!-- response SQL -->
```sql
+-------------+
| hour(now()) |
+-------------+
| 7           |
+-------------+
```
<!-- end -->

### DAY()
<!-- example DAY -->
根据当前时区，从时间戳参数中返回整数月份中的天数（范围 1..31）。

<!-- request SQL -->
```sql
select day(now());
```
<!-- response SQL -->
```sql
+------------+
| day(now()) |
+------------+
| 15         |
+------------+
```
<!-- end -->

### MONTH()
<!-- example MONTH -->
根据当前时区，从时间戳参数中返回整数月份（范围 1..12）。

<!-- request SQL -->
```sql
select month(now());
```
<!-- response SQL -->
```sql
+--------------+
| month(now()) |
+--------------+
| 3            |
+--------------+
```
<!-- end -->

### QUARTER()
<!-- example QUARTER -->
根据当前时区，从时间戳参数中返回整数季度（范围 1..4）。

<!-- request SQL -->
```sql
select quarter(now());
```
<!-- response SQL -->
```sql
+----------------+
| quarter(now()) |
+----------------+
| 2              |
+----------------+
```
<!-- end -->

### YEAR()
<!-- example YEAR -->
根据当前时区，从时间戳参数中返回整数年份（范围 1969..2038）。

<!-- request SQL -->
```sql
select year(now());
```
<!-- response SQL -->
```sql
+-------------+
| year(now()) |
+-------------+
| 2024        |
+-------------+
```
<!-- end -->

### DAYNAME()
<!-- example DAYNAME -->
根据当前时区，返回给定时间戳参数的星期名称。

<!-- request SQL -->
```sql
select dayname(now());
```
<!-- response SQL -->
```sql
+----------------+
| dayname(now()) |
+----------------+
| Wednesday      |
+----------------+
```
<!-- end -->

### MONTHNAME()
<!-- example MONTHNAME -->
根据当前时区，返回给定时间戳参数的月份名称。

<!-- request SQL -->
```sql
select monthname(now());
```
<!-- response SQL -->
```sql
+------------------+
| monthname(now()) |
+------------------+
| August           |
+------------------+
```
<!-- end -->

### DAYOFWEEK()
<!-- example DAYOFWEEK -->
根据当前时区，返回给定时间戳参数的整数星期索引（范围 1..7）。
注意，星期从星期日开始。

<!-- request SQL -->
```sql
select dayofweek(now());
```
<!-- response SQL -->
```sql
+------------------+
| dayofweek(now()) |
+------------------+
| 5                |
+------------------+
```
<!-- end -->

### DAYOFYEAR()
<!-- example DAYOFYEAR -->
根据当前时区，返回给定时间戳参数的整数年份中的天数（范围 1..366）。

<!-- request SQL -->
```sql
select dayofyear(now());
```
<!-- response SQL -->
```sql
+------------------+
| dayofyear(now()) |
+------------------+
|              214 |
+------------------+
```
<!-- end -->

### YEARWEEK()
<!-- example YEARWEEK -->
根据当前时区，返回给定时间戳参数的当前周第一天的整数年份和日期代码（范围 1969001..2038366）。

<!-- request SQL -->
```sql
select yearweek(now());
```
<!-- response SQL -->
```sql
+-----------------+
| yearweek(now()) |
+-----------------+
|         2023211 |
+-----------------+
```
<!-- end -->

### YEARMONTH()
<!-- example YEARMONTH -->
根据当前时区，从时间戳参数返回整数年份和月份代码（范围 196912..203801）。

<!-- request SQL -->
```sql
select yearmonth(now());
```
<!-- response SQL -->
```sql
+------------------+
| yearmonth(now()) |
+------------------+
| 202103           |
+------------------+
```
<!-- end -->

### YEARMONTHDAY()
<!-- example YEARMONTHDAY -->
根据当前时区，返回整数年份、月份和日期代码（范围 19691231 到 20380119）。

<!-- request SQL -->
```sql
select yearmonthday(now());
```
<!-- response SQL -->
```sql
+---------------------+
| yearmonthday(now()) |
+---------------------+
| 20210315            |
+---------------------+
```
<!-- end -->

### TIMEDIFF()
<!-- example TIMEDIFF -->
计算两个时间戳之间的差值，格式为 `hh:ii:ss`。

<!-- request SQL -->
```sql
select timediff(1615787586, 1613787583);
```
<!-- response SQL -->
```sql
+----------------------------------+
| timediff(1615787586, 1613787583) |
+----------------------------------+
| 555:33:23                        |
+----------------------------------+
```
<!-- end -->

### DATEDIFF()
<!-- example DATEDIFF -->
计算两个给定时间戳之间的天数。

<!-- request SQL -->
```sql
select datediff(1615787586, 1613787583);
```
<!-- response SQL -->
```sql
+----------------------------------+
| datediff(1615787586, 1613787583) |
+----------------------------------+
|                               23 |
+----------------------------------+
```
<!-- end -->

### DATE()
<!-- example DATE -->
将时间戳参数的日期部分格式化为字符串，格式为 `YYYY-MM-DD`。

<!-- request SQL -->
```sql
select date(now());
```
<!-- response SQL -->
```sql
+-------------+
| date(now()) |
+-------------+
| 2023-08-02  |
+-------------+
```
<!-- end -->

### TIME()
<!-- example TIME -->
将时间戳参数的时间部分格式化为字符串，格式为 `HH:MM:SS`。

<!-- request SQL -->
```sql
select time(now());
```
<!-- response SQL -->
```sql
+-------------+
| time(now()) |
+-------------+
| 15:21:27    |
+-------------+
```
<!-- end -->

### DATE_FORMAT()
<!-- example DATE_FORMAT -->
根据提供的日期和格式参数返回格式化字符串。格式参数使用与 [strftime](https://man7.org/linux/man-pages/man3/strftime.3.html) 函数相同的格式说明符。为方便起见，以下是一些常用格式说明符：

- `%Y` - 四位数年份
- `%m` - 两位数月份（01-12）
- `%d` - 两位数月份中的天数（01-31）
- `%H` - 两位数小时（00-23）
- `%M` - 两位数分钟（00-59）
- `%S` - 两位数秒（00-59）
- `%T` - 24 小时制时间（`%H:%M:%S`）

请注意，这不是完整的格式说明符列表。请查阅您操作系统中 `strftime()` 的文档以获取完整列表。

<!-- request SQL -->
```sql
SELECT DATE_FORMAT(NOW(), 'year %Y and time %T');
```
<!-- response SQL -->
```sql
+------------------------------------------+
| DATE_FORMAT(NOW(), 'year %Y and time %T') |
+------------------------------------------+
| year 2023 and time 11:54:52              |
+------------------------------------------+
```
<!-- end -->

此示例格式化当前日期和时间，显示四位数的年份和24小时制的时间。

### DATE_HISTOGRAM()
<!-- example DATE_HISTOGRAM -->
`DATE_HISTOGRAM(expr, {calendar_interval='unit_name'})` 接受一个作为单位名称的桶大小，并返回该值的桶编号。值会被四舍五入到最接近的桶。关键函数是：
```sql
key_of_the_bucket = interval * floor ( value / interval )
```
间隔可以使用单位名称指定，如 `week`，或作为单个单位，如 `1M`。但是，多个单位，如 `2d`，不支持 `calendar_interval`，但允许使用 `fixed_interval`。

`calendar_interval` 的有效间隔为：

- `minute`，`1m`
- `hour`，`1h`
- `day`，`1d`
- `week`，`1w`（一周是从周的起始日、小时、分钟、秒到下一周同一天同一时间的间隔）
- `month`，`1M`
- `year`，`1y`（一年是从月的起始日、时间到下一年同一天同一时间的间隔）

`fixed_interval` 的有效间隔为：

- `minute`，`2m`
- `hour`，`3h`
- `day`，`5d`

用于聚合、`FACET` 和分组。

示例：

```sql
SELECT COUNT(*),
DATE_HISTOGRAM(tm, {calendar_interval='month'}) AS months
FROM facets
GROUP BY months ORDER BY months ASC;
```

### DATE_RANGE()
<!-- example DATE_RANGE -->
`DATE_RANGE(expr, {range_from='date_math', range_to='date_math'})` 接受一组范围，并返回该值的桶编号。
表达式包括每个范围的 `range_from` 值，但不包括 `range_to` 值。范围可以是开放的——只有 `range_from` 或只有 `range_to`。
此函数与 [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29) 函数的区别在于，`range_from` 和 `range_to` 的值可以用 [日期数学](../Functions/Date_and_time_functions.md#Date-math) 表达式表示。

用于聚合、`FACET` 和分组。

示例：

```sql
SELECT COUNT(*),
DATE_RANGE(tm, {range_to='2017||+2M/M'},{range_from='2017||+2M/M',range_to='2017||+5M/M'},{range_from='2017||+5M/M'}) AS points
FROM idx_dates
GROUP BY points ORDER BY points ASC;
```

##### 日期数学

日期数学让您可以直接在搜索中处理日期和时间。它特别适用于处理随时间变化的数据。通过日期数学，您可以轻松完成诸如查找某一时期的条目、分析数据趋势或管理信息何时应被删除等操作。它简化了日期处理，允许您在搜索查询中对给定日期加减时间、将日期四舍五入到最近的时间单位等。

使用日期数学时，您从一个基准日期开始，该日期可以是：
- `now` 表示当前日期和时间，
- 或以 `||` 结尾的特定日期字符串。

然后，您可以用以下操作修改此日期：
- `+1y` 表示加一年，
- `-1h` 表示减一小时，
- `/m` 表示四舍五入到最近的月份。

您可以在操作中使用以下单位：
- `s` 表示秒，
- `m` 表示分钟，
- `h`（或 `H`）表示小时，
- `d` 表示天，
- `w` 表示周，
- `M` 表示月，
- `y` 表示年。

以下是一些日期数学的示例用法：
- `now+4h` 表示从现在起四小时。
- `now-2d/d` 表示两天前的时间，四舍五入到最近的一天。
- `2010-04-20||+2M/d` 表示2010年6月20日，四舍五入到最近的一天。

<!-- proofread -->

