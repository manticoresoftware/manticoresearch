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
| 2021        |
+-------------+
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

<!-- proofread -->
