# 日期和时间函数

注意，`CURTIME()`、`UTC_TIME()`、`UTC_TIMESTAMP()` 和 `TIMEDIFF()` 可以通过任意转换函数如 `BIGINT()`、`DOUBLE()` 等提升为数字类型。

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
返回本地时区当前时间，格式为 `hh:ii:ss`。

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
返回本地时区当前日期，格式为 `YYYY-MM-DD`。

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
返回 UTC 时区当前时间，格式为 `hh:ii:ss`。

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
返回 UTC 时区当前时间，格式为 `YYYY-MM-DD hh:ii:ss`。

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
根据当前时区，从时间戳参数中返回整数秒数（范围为 0..59）。

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
根据当前时区，从时间戳参数中返回整数分钟数（范围为 0..59）。

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
根据当前时区，从时间戳参数中返回整数小时数（范围为 0..23）。

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
根据当前时区，从时间戳参数中返回当前月份的整数天数（范围为 1..31）。

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
根据当前时区，从时间戳参数中返回整数月份（范围为 1..12）。

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
根据当前时区，从时间戳参数中返回当前季度的整数值（范围为 1..4）。

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
根据当前时区，从时间戳参数中返回整数年份（范围为 1969..2038）。

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
根据当前时区，从时间戳参数返回星期名称。

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
根据当前时区，从时间戳参数返回月份名称。

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
根据当前时区，从时间戳参数返回星期的整数索引（范围 1..7）。
注意，星期从星期天开始。

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
根据当前时区，从时间戳参数返回今年的第几天（范围 1..366）。

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
根据当前时区，从时间戳参数返回当前周第一天的年份和日代码的整数（范围 1969001..2038366）。

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
根据当前时区，从时间戳参数返回年份和月份的整数代码（范围 196912..203801）。

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
根据当前时区，返回年份、月份和日期的整数代码（范围从 19691231 到 20380119）。

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
- `%d` - 两位数字的月份中的天（01-31）
- `%H` - 两位数字的小时（00-23）
- `%M` - 两位数字的分钟（00-59）
- `%S` - 两位数字的秒钟（00-59）
- `%T` - 24 小时制时间（`%H:%M:%S`）

注意，这不是完整的格式说明符列表。请查阅您操作系统的 `strftime()` 文档以获取完整列表。

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

此示例格式化当前日期和时间，显示四位数年份和24小时制时间。

### DATE_HISTOGRAM()
<!-- example DATE_HISTOGRAM -->
`DATE_HISTOGRAM(expr, {calendar_interval='unit_name'})` 接受一个以单位名称为单位的桶大小，并返回该值对应的桶编号。值会被四舍五入到最接近的桶。关键函数为：
```sql
key_of_the_bucket = interval * floor ( value / interval )
```
间隔可以用单位名称指定，如 `week`，或者用单个单位，如 `1M`。但使用 `calendar_interval` 不支持多单位，如 `2d`，而 `fixed_interval` 允许。

`calendar_interval` 合法的间隔包括：

- `minute`, `1m`
- `hour`, `1h`
- `day`, `1d`
- `week`, `1w`（一周是从周开始的那一天、小时、分钟、秒，到下一周同一天同一时间的间隔）
- `month`, `1M`
- `year`, `1y`（一年是在本月开始的那一天和时间，到下一年同一月同一天同一时间的间隔）

`fixed_interval` 合法的间隔包括：

- `minute`, `2m`
- `hour`, `3h`
- `day`, `5d`

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
`DATE_RANGE(expr, {range_from='date_math', range_to='date_math'})` 接受一组区间并返回该值对应的桶编号。
表达式包括每个区间的 `range_from` 值，但不包括 `range_to` 值。区间可以是开放的——只有 `range_from` 或只有 `range_to`。
这一点与 [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29) 函数的区别在于，`range_from` 和 `range_to` 可以用 [日期数学](../Functions/Date_and_time_functions.md#Date-math) 表达式表示。

用于聚合、`FACET` 和分组。

示例：

```sql
SELECT COUNT(*),
DATE_RANGE(tm, {range_to='2017||+2M/M'},{range_from='2017||+2M/M',range_to='2017||+5M/M'},{range_from='2017||+5M/M'}) AS points
FROM idx_dates
GROUP BY points ORDER BY points ASC;
```

##### 日期数学

日期数学让你能够直接在搜索中处理日期和时间。它对于处理随时间变化的数据特别有用。使用日期数学，你可以轻松完成诸如查找特定时间段的条目、分析数据趋势、或管理信息删除时间等操作。它简化了对日期的操作，允许你在查询中添加或减少时间，按最近的时间单位取整等。

使用日期数学时，需要以基础日期开始，该基础日期可以是：
- 当前日期时间的 `now`，
- 或以 `||` 结束的特定日期字符串。

然后，你可以用以下操作修改此日期：
- `+1y` 表示加一年，
- `-1h` 表示减一小时，
- `/m` 表示取整到最近的月份。

在操作中你可以使用以下单位：
- `s` 表示秒，
- `m` 表示分钟，
- `h`（或 `H`）表示小时，
- `d` 表示天，
- `w` 表示周，
- `M` 表示月，
- `y` 表示年。

以下是一些日期数学的使用示例：
- `now+4h` 表示四小时后的当前时间。
- `now-2d/d` 表示两天前的时间，按天取整。
- `2010-04-20||+2M/d` 表示2010年6月20日，按天取整。

<!-- proofread -->

