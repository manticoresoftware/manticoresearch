# Date and time functions

Note, that `CURTIME()`, `UTC_TIME()`, `UTC_TIMESTAMP()`, and `TIMEDIFF()` can be promoted to numeric types using arbitrary conversion functions such as `BIGINT()`, `DOUBLE()`, etc.

### NOW()
<!-- example NOW -->
Returns the current timestamp as an INTEGER.

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
Returns the current time in the local timezone in `hh:ii:ss` format.

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
Returns the current date in the local timezone in `YYYY-MM-DD` format.

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
Returns the current time in UTC timezone in `hh:ii:ss` format.

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
Returns the current time in UTC timezone in `YYYY-MM-DD hh:ii:ss` format.

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
Returns the integer second (in 0..59 range) from a timestamp argument, according to the current timezone.

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
Returns the integer minute (in 0..59 range) from a timestamp argument, according to the current timezone.

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
Returns the integer hour (in 0..23 range) from a timestamp argument, according to the current timezone.

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
Returns the integer day of the month (in 1..31 range) from a timestamp argument, according to the current timezone.

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
Returns the integer month (in 1..12 range) from a timestamp argument, according to the current timezone.

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
Returns the integer quarter of the year (in 1..4 range) from a timestamp argument, according to the current timezone.

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
Returns the integer year (in 1969..2038 range) from a timestamp argument, according to the current timezone.

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
Returns the weekday name for a given timestamp argument, according to the current timezone.

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
Returns the name of the month for a given timestamp argument, according to the current timezone.

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
Returns the integer weekday index (in 1..7 range) for a given timestamp argument, according to the current timezone.
Note that the week starts on Sunday.

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
Returns the integer day of the year (in 1..366 range) for a given timestamp argument, according to the current timezone.

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
Returns the integer year and the day code of the first day of current week (in 1969001..2038366 range) for a given timestamp argument, according to the current timezone.

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
Returns the integer year and month code (in 196912..203801 range) from a timestamp argument, according to the current timezone.

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
Returns the integer year, month, and date code (ranging from 19691231 to 20380119) based on the current timezone.

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
Calculates the difference between two timestamps in the format `hh:ii:ss`.

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
Calculates the number of days between two given timestamps.

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
Formats the date part from a timestamp argument as a string in `YYYY-MM-DD` format.

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
Formats the time part from a timestamp argument as a string in `HH:MM:SS` format.

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
Returns a formatted string based on the provided date and format arguments. The format argument uses the same specifiers as the [strftime](https://man7.org/linux/man-pages/man3/strftime.3.html) function. For convenience, here are some common format specifiers:

- `%Y` - Four-digit year
- `%m` - Two-digit month (01-12)
- `%d` - Two-digit day of the month (01-31)
- `%H` - Two-digit hour (00-23)
- `%M` - Two-digit minute (00-59)
- `%S` - Two-digit second (00-59)
- `%T` - Time in 24-hour format (`%H:%M:%S`)

Note that this is not a complete list of the specifiers. Please consult the documentation for `strftime()` for your operating system to get the full list.

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

This example formats the current date and time, displaying the four-digit year and the time in 24-hour format.

### DATE_HISTOGRAM()
<!-- example DATE_HISTOGRAM -->
`DATE_HISTOGRAM(expr, {calendar_interval='unit_name'})` Takes a bucket size as a unit name and returns the bucket number for the value. Values are rounded to the closest bucket. The key function is:
```sql
key_of_the_bucket = interval * floor ( value / interval )
```
Intervals can be specified using a unit name, like `week`, or as a single unit, such as `1M`. However, multiple units, like `2d`, are not supported with `calendar_interval` but are allowed with `fixed_interval`.

The valid intervals for `calendar_interval` are:

- `minute`, `1m`
- `hour`, `1h`
- `day`, `1d`
- `week`, `1w` (a week is the interval between the start day of the week, hour, minute, second and the next week but the same day and time of the week)
- `month`, `1M`
- `year`, `1y` (a year is the interval between the start day of the month, time and the next year but the same day of the month, time)

The valid intervals for `fixed_interval` are:

- `minute`, `2m`
- `hour`, `3h`
- `day`, `5d`

Used in aggregation, `FACET`, and grouping.

Example:

```sql
SELECT COUNT(*),
DATE_HISTOGRAM(tm, {calendar_interval='month'}) AS months
FROM facets
GROUP BY months ORDER BY months ASC;
```

### DATE_RANGE()
<!-- example DATE_RANGE -->
`DATE_RANGE(expr, {range_from='date_math', range_to='date_math'})` takes a set of ranges and returns the bucket number for the value.
The expression includes the `range_from` value and excludes the `range_to` value for each range. The range can be open - having only the `range_from` or only the `range_to` value.
The difference between this and the [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29) function is that the `range_from` and `range_to` values can be expressed in [Date math](../Functions/Date_and_time_functions.md#Date-math) expressions.

Used in aggregation, `FACET`, and grouping.

Example:

```sql
SELECT COUNT(*),
DATE_RANGE(tm, {range_to='2017||+2M/M'},{range_from='2017||+2M/M',range_to='2017||+5M/M'},{range_from='2017||+5M/M'}) AS points
FROM idx_dates
GROUP BY points ORDER BY points ASC;
```

##### Date math

Date math lets you work with dates and times directly in your searches. It's especially useful for handling data that changes over time. With date math, you can easily do things like find entries from a certain period, analyze data trends, or manage when information should be removed. It simplifies working with dates by letting you add or subtract time from a given date, round dates to the nearest time unit, and more, all within your search queries.

To use date math, you start with a base date, which can be:
- `now` for the current date and time,
- or a specific date string ending with `||`.

Then, you can modify this date with operations like:
- `+1y` to add one year,
- `-1h` to subtract one hour,
- `/m` to round to the nearest month.

You can use these units in your operations:
- `s` for seconds,
- `m` for minutes,
- `h` (or `H`) for hours,
- `d` for days,
- `w` for weeks,
- `M` for months,
- `y` for years.

Here are some examples of how you might use date math:
- `now+4h` means four hours from now.
- `now-2d/d` is the time two days ago, rounded to the nearest day.
- `2010-04-20||+2M/d` is June 20, 2010, rounded to the nearest day.

<!-- proofread -->
