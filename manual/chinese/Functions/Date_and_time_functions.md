# 日期和时间函数

注意，`CURTIME()`、`UTC_TIME()`、`UTC_TIMESTAMP()` 和 `TIMEDIFF()` 可以使用诸如 `BIGINT()`、`DOUBLE()` 等任意转换函数提升为数值类型。

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
返回时间戳参数中当前时区对应的秒数，整数，范围为 0..59。

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
返回时间戳参数中当前时区对应的分钟数，整数，范围为 0..59。

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
返回时间戳参数中当前时区对应的小时数，整数，范围为 0..23。

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
返回时间戳参数中当前时区对应的月份中的天数，整数，范围为 1..31。

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
返回时间戳参数中当前时区对应的月份，整数，范围为 1..12。

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
返回时间戳参数中当前时区对应的季度，整数，范围为 1..4。

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
返回时间戳参数中当前时区对应的年份，整数，范围为 1969..2038。

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
返回时间戳参数中当前时区对应的星期几的名称。

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
返回时间戳参数中当前时区对应月份的名称。

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
返回时间戳参数中当前时区对应的星期序号，整数，范围为 1..7。
注意，星期的开始为星期日。

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
返回时间戳参数中当前时区对应的年份中的天数，整数，范围为 1..366。

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
返回时间戳参数中当前时区对应的当前周的年份和第一天的日期码，整数，范围为 1969001..2038366。

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
返回时间戳参数中当前时区对应的年份和月份码，整数，范围为 196912..203801。

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
根据当前时区返回时间戳参数中的年份、月份和日期码，范围从 19691231 到 20380119。

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
计算两个给定时间戳之间的天数差。

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
将时间戳参数的日期部分格式化为 `YYYY-MM-DD` 字符串。

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
将时间戳参数的时间部分格式化为 `HH:MM:SS` 字符串。

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
根据提供的日期和格式参数返回格式化的字符串。格式参数使用与 [strftime](https://man7.org/linux/man-pages/man3/strftime.3.html) 函数相同的格式说明符。方便起见，这里列出一些常用的格式说明符：

- `%Y` - 四位数的年份
- `%m` - 两位数的月份（01-12）
- `%d` - 两位数的月份中的天数（01-31）
- `%H` - 两位数的小时（00-23）
- `%M` - 两位数的分钟（00-59）
- `%S` - 两位数的秒数（00-59）
- `%T` - 24小时制时间格式（`%H:%M:%S`）

请注意，这不是格式说明符的完整列表。请参考您的操作系统的 `strftime()` 文档以获取完整列表。

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
`DATE_HISTOGRAM(expr, {calendar_interval='unit_name'})` 接受一个以单位名称表示的桶大小，并返回该值对应的桶号。数值将被四舍五入到最接近的桶。关键函数为：
```sql
key_of_the_bucket = interval * floor ( value / interval )
```
时间间隔可以使用单位名称指定，如 `week`，或作为单个单位，例如 `1M`。但是，多个单位如 `2d` 不支持 `calendar_interval`，但允许使用 `fixed_interval`。

`calendar_interval` 的有效间隔为：

- `minute`，`1m`
- `hour`，`1h`
- `day`，`1d`
- `week`，`1w`（一周是指从周的开始日期、小时、分钟、秒，到下一周同一周的日期和时间的间隔）
- `month`，`1M`
- `year`，`1y`（一年是指从月的开始日期和时间，到下一年同一月日和时间的间隔）

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
`DATE_RANGE(expr, {range_from='date_math', range_to='date_math'})` 接受一组范围，并返回该值对应的桶号。
表达式包含每个范围的 `range_from` 值，但不包含 `range_to` 值。范围可以是开放范围，仅包含 `range_from` 或仅包含 `range_to`。
此函数与 [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29) 函数的区别在于，`range_from` 和 `range_to` 值可以以 [日期数学](../Functions/Date_and_time_functions.md#Date-math) 表达式表示。

用于聚合、`FACET` 和分组。

示例：

```sql
SELECT COUNT(*),
DATE_RANGE(tm, {range_to='2017||+2M/M'},{range_from='2017||+2M/M',range_to='2017||+5M/M'},{range_from='2017||+5M/M'}) AS points
FROM idx_dates
GROUP BY points ORDER BY points ASC;
```

##### 日期数学

日期数学让您可以在搜索中直接处理日期和时间。它对于处理随时间变化的数据尤为有用。使用日期数学，您可以轻松完成如下任务：查找某一时间段的条目、分析数据趋势、管理信息何时应被删除。它通过允许您从给定日期中添加或减去时间、将日期舍入到最近的时间单位等，简化了日期处理，所有操作均在搜索查询内完成。

使用日期数学时，首先选择一个基准日期，它可以是：
- 表示当前日期和时间的 `now`，
- 或以 `||` 结尾的特定日期字符串。

然后，您可以通过以下操作修改该日期：
- `+1y` 表示加一年，
- `-1h` 表示减一小时，
- `/m` 表示向最近的月份舍入。

您可以在操作中使用以下单位：
- `s` 表示秒，
- `m` 表示分钟，
- `h`（或 `H`）表示小时，
- `d` 表示天，
- `w` 表示周，
- `M` 表示月，
- `y` 表示年。

以下是一些日期数学的使用示例：
- `now+4h` 表示从现在起四小时。
- `now-2d/d` 表示两天前的时间，舍入到最近的一天。
- `2010-04-20||+2M/d` 表示2010年6月20日，舍入到最近的一天。

<!-- proofread -->

