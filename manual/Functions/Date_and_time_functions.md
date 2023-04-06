# Date and time functions

### NOW()
Returns the current timestamp as an INTEGER.
```sql
mysql> select NOW();
+------------+
| NOW()      |
+------------+
| 1615788407 |
+------------+
1 row in set (0.00 sec)
```

### CURTIME()
Returns the current time in the local timezone in `hh:ii:ss` format.
```sql
mysql> select CURTIME();
+-----------+
| CURTIME() |
+-----------+
| 07:06:30  |
+-----------+
1 row in set (0.00 sec)
```

### UTC_TIME()
Returns the current time in UTC timezone in `hh:ii:ss` format.

```sql
mysql> select UTC_TIME();
+------------+
| UTC_TIME() |
+------------+
| 06:06:18   |
+------------+
1 row in set (0.00 sec)
```

### UTC_TIMESTAMP()
Returns the current time in UTC timezone in `YYYY-MM-DD hh:ii:ss` format.

```sql
mysql> select UTC_TIMESTAMP();
+---------------------+
| UTC_TIMESTAMP()     |
+---------------------+
| 2021-03-15 06:06:03 |
+---------------------+
1 row in set (0.00 sec)
```

### SECOND()
Returns the integer second (in 0..59 range) from a timestamp argument, according to the current timezone.
```sql
mysql> select second(now());
+---------------+
| second(now()) |
+---------------+
| 52            |
+---------------+
1 row in set (0.00 sec)
```

### MINUTE()
Returns the integer minute (in 0..59 range) from a timestamp argument, according to the current timezone.
```sql
mysql> select minute(now());
+---------------+
| minute(now()) |
+---------------+
| 5             |
+---------------+
1 row in set (0.00 sec)
```

### HOUR()
Returns the integer hour (in 0..23 range) from a timestamp argument, according to the current timezone.
```sql
mysql> select hour(now());
+-------------+
| hour(now()) |
+-------------+
| 7           |
+-------------+
1 row in set (0.00 sec)
```

### DAY()
Returns the integer day of the month (in 1..31 range) from a timestamp argument, according to the current timezone.
```sql
mysql> select day(now());
+------------+
| day(now()) |
+------------+
| 15         |
+------------+
1 row in set (0.00 sec)
```

### MONTH()
Returns the integer month (in 1..12 range) from a timestamp argument, according to the current timezone.
```sql
mysql> select month(now());
+--------------+
| month(now()) |
+--------------+
| 3            |
+--------------+
1 row in set (0.00 sec)
```

### YEAR()
Returns the integer year (in 1969..2038 range) from a timestamp argument, according to the current timezone.
```sql
mysql> select year(now());
+-------------+
| year(now()) |
+-------------+
| 2021        |
+-------------+
1 row in set (0.00 sec)
```

### YEARMONTH()
Returns the integer year and month code (in 196912..203801 range) from a timestamp argument, according to the current timezone.
```sql
mysql> select yearmonth(now());
+------------------+
| yearmonth(now()) |
+------------------+
| 202103           |
+------------------+
1 row in set (0.00 sec)
```

### YEARMONTHDAY()
Returns the integer year, month, and date code (ranging from 19691231 to 20380119) based on the current timezone.   
```sql
mysql> select yearmonthday(now());
+---------------------+
| yearmonthday(now()) |
+---------------------+
| 20210315            |
+---------------------+
1 row in set (0.00 sec)
```

### TIMEDIFF()
Calculates the difference between two timestamps in the format `hh:ii:ss`.
```sql
mysql> select timediff(1615787586, 1613787583);
+----------------------------------+
| timediff(1615787586, 1613787583) |
+----------------------------------+
| 555:33:23                        |
+----------------------------------+
1 row in set (0.00 sec)
```

### DATE_FORMAT()
Returns a formatted string based on the provided date and format arguments. The format argument uses the same specifiers as the [strftime](https://man7.org/linux/man-pages/man3/strftime.3.html) function. For convenience, here are some common format specifiers:

- `%Y` - Four-digit year
- `%m` - Two-digit month (01-12)
- `%d` - Two-digit day of the month (01-31)
- `%H` - Two-digit hour (00-23)
- `%M` - Two-digit minute (00-59)
- `%S` - Two-digit second (00-59)
- `%T` - Time in 24-hour format (`%H:%M:%S`)

Example usage:

```sql
mysql> SELECT DATE_FORMAT(NOW(), 'year %Y and time %T');
+------------------------------------------+
| DATE_FORMAT(NOW(), 'year %Y and time %T') |
+------------------------------------------+
| year 2023 and time 11:54:52              |
+------------------------------------------+
1 row in set (0.00 sec)
```

This example formats the current date and time, displaying the four-digit year and the time in 24-hour format.

<!-- proofread -->
