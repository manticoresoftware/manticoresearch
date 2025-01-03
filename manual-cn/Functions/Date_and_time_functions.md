# 日期和时间函数

请注意，`CURTIME()`、`UTC_TIME()`、`UTC_TIMESTAMP()` 和 `TIMEDIFF()` 可以通过使用任意转换函数（例如 `BIGINT()`、`DOUBLE()` 等）提升为数值类型。

### NOW()
<!-- example NOW -->
返回当前整数时间戳。

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
以 `hh:ii:ss` 格式返回本地时区的当前时间。

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
以 `YYYY-MM-DD` 格式返回本地时区的当前日期。

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
以 `hh:ii:ss` 格式返回 UTC 时区的当前时间。

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
以 `YYYY-MM-DD hh:ii:ss` 格式返回 UTC 时区的当前时间。

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
根据当前时区从时间戳参数中返回秒数整数（范围 0..59）。

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
根据当前时区从时间戳参数中返回分钟整数（范围 0..59）。

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
根据当前时区从时间戳参数中返回小时整数（范围 0..23）。

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
根据当前时区从时间戳参数中返回日期整数（范围 1..31）。

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
根据当前时区从时间戳参数中返回月份整数（范围 1..12）。

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
返回时间戳参数中的年份季度整数（范围 1..4），根据当前时区。

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
返回时间戳参数中的年份整数（范围 1969..2038），根据当前时区。

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
返回给定时间戳参数的星期几名称，根据当前时区。

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
返回给定时间戳参数的月份名称，根据当前时区。

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
返回给定时间戳参数的星期几整数索引（范围 1..7），根据当前时区。请注意，星期从周日开始。

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
返回给定时间戳参数的年度天数整数（范围 1..366），根据当前时区。

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
返回给定时间戳参数的整数年份和当前周的第一天代码（范围 1969001..2038366），根据当前时区。

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
返回时间戳参数中的年份和月份代码整数（范围 196912..203801），根据当前时区。

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
返回基于当前时区的年份、月份和日期代码整数（范围 19691231 到 20380119）。

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
计算两个时间戳之间的差异，格式为 `hh:ii:ss`。

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
计算两个时间戳之间的天数差异。

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
将时间戳参数中的日期部分格式化为 `YYYY-MM-DD` 字符串。

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
将时间戳参数中的时间部分格式化为 `HH:MM:SS` 字符串。

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

根据提供的日期和格式参数返回格式化的字符串。格式参数使用与 [strftime](https://man7.org/linux/man-pages/man3/strftime.3.html) 函数相同的说明符。为了方便，以下是一些常用的格式说明符：

- `%Y` - 四位数年份
- `%m` - 两位数月份（01-12）
- `%d` - 两位数日期（01-31）
- `%H` - 两位数小时（00-23）
- `%M` - 两位数分钟（00-59）
- `%S` - 两位数秒（00-59）
- `%T` - 24小时制时间（`%H:%M:%S`）

请注意，这不是说明符的完整列表。请查阅您的操作系统 `strftime()` 的文档以获取完整列表。

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

此示例格式化当前日期和时间，显示四位数年份和 24 小时制时间。

### DATE_HISTOGRAM()
<!-- example DATE_HISTOGRAM -->
`DATE_HISTOGRAM(expr, {calendar_interval='unit_name'})` 使用单位名称作为桶大小并返回该值的桶号。值会被舍入到最接近的桶。关键函数为：

```sql
key_of_the_bucket = interval * floor ( value / interval )
```
间隔使用单位名称指定，如 `week` 或 `1M`。不支持多个单位组合（如 `2w`）。

有效的间隔有：

- `minute`, `1m`
- `hour`, `1h`
- `day`, `1d`
- `week`, `1w`
- `month`, `1M`
- `year`, `1y`

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

`DATE_RANGE(expr, {range_from='date_math', range_to='date_math'})` 接受一组范围并返回该值的桶号。表达式包括 `range_from` 值并排除 `range_to` 值。范围可以是开放的 - 仅包含 `range_from` 或仅包含 `range_to` 值。与 [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE()) 函数的不同之处在于，`range_from` 和 `range_to` 值可以用 [Date math](../Functions/Date_and_time_functions.md#日期数学) 表达式表示。

用于聚合、`FACET` 和分组。

示例

```sql
SELECT COUNT(*),
DATE_RANGE(tm, {range_to='2017||+2M/M'},{range_from='2017||+2M/M',range_to='2017||+5M/M'},{range_from='2017||+5M/M'}) AS points
FROM idx_dates
GROUP BY points ORDER BY points ASC;
```

##### 日期数学

日期数学允许您在搜索中直接处理日期和时间。它对于处理随时间变化的数据尤其有用。通过日期数学，您可以轻松执行以下操作：查找特定时期的条目、分析数据趋势或管理何时删除信息。它通过让您能够在给定日期上加减时间、将日期舍入到最近的时间单位等方式，简化了日期的处理，这些操作都可以在您的搜索查询中进行。

要使用日期数学，您可以从一个基准日期开始，它可以是：

- `now` 代表当前日期和时间，
- 或者以 `||` 结尾的特定日期字符串。

然后，您可以使用以下操作来修改此日期：

- `+1y` 表示增加一年，
- `-1h` 表示减少一小时，
- `/m` 表示舍入到最近的月份。

在这些操作中，您可以使用以下时间单位：

- `s` 代表秒，
- `m` 代表分钟，
- `h`（或 `H`）代表小时，
- `d` 代表天，
- `w` 代表周，
- `M` 代表月，
- `y` 代表年。

以下是一些日期数学的使用示例：

- `now+4h` 表示从现在起四小时后。
- `now-2d/d` 表示两天前的时间，并舍入到最近的一天。
- `2010-04-20||+2M/d` 表示 2010 年 6 月 20 日，并舍入到最近的一天。

<!-- proofread -->
