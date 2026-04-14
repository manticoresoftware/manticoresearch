# Функции даты и времени

Обратите внимание, что `CURTIME()`, `UTC_TIME()`, `UTC_TIMESTAMP()` и `TIMEDIFF()` могут быть преобразованы в числовые типы с помощью произвольных функций преобразования, таких как `BIGINT()`, `DOUBLE()` и т.д.

### NOW()
<!-- example NOW -->
Возвращает текущую временную метку в виде INTEGER.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select NOW()"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "now()": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "now()": 1615788407
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```


<!-- end -->

### CURTIME()
<!-- example CURTIME -->
Возвращает текущее время в локальном часовом поясе в формате `hh:ii:ss`.

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


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select CURTIME()"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "CURTIME()": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "CURTIME()": "07:06:30"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

### CURDATE()
<!-- example CURDATE -->
Возвращает текущую дату в локальном часовом поясе в формате `YYYY-MM-DD`.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select CURDATE()"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "CURDATE()": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "CURDATE()": "2023-08-02"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```

<!-- end -->

### UTC_TIME()
<!-- example UTC_TIME -->
Возвращает текущее время в часовом поясе UTC в формате `hh:ii:ss`.

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


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select UTC_TIME()"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "UTC_TIME()": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "UTC_TIME()": "06:06:18"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### UTC_TIMESTAMP()
<!-- example UTC_TIMESTAMP -->
Возвращает текущее время в часовом поясе UTC в формате `YYYY-MM-DD hh:ii:ss`.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select UTC_TIMESTAMP()"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "UTC_TIMESTAMP()": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "UTC_TIMESTAMP()": "2021-03-15 06:06:0"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### SECOND()
<!-- example SECOND -->
Возвращает целое число секунд (в диапазоне 0..59) из аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select second(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "second(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "second(now())": 52
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### MINUTE()
<!-- example MINUTE -->
Возвращает целое число минут (в диапазоне 0..59) из аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select minute(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "minute(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "minute(now())": 5
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### HOUR()
<!-- example HOUR -->
Возвращает целое число часов (в диапазоне 0..23) из аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select hour(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "hour(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "hour(now())": 7
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### DAY()
<!-- example DAY -->
Возвращает целое число дня месяца (в диапазоне 1..31) из аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select day(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "day(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "day(now())": 15
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### MONTH()
<!-- example MONTH -->
Возвращает целое число месяца (в диапазоне 1..12) из аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select month(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "month(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "month(now())": 3
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### QUARTER()
<!-- example QUARTER -->
Возвращает целое число квартала года (в диапазоне 1..4) из аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select quarter(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "quarter(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "quarter(now())": 2
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### YEAR()
<!-- example YEAR -->
Возвращает целое число года (в диапазоне 1969..2038) из аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select year(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "year(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "year(now())": 2024
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### DAYNAME()
<!-- example DAYNAME -->
Возвращает название дня недели для заданного аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select dayname(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "dayname(now())": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "dayname(now())": "Wednesday"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### MONTHNAME()
<!-- example MONTHNAME -->
Возвращает название месяца для заданного аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select monthname(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "monthname(now())": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "monthname(now())": "August"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### DAYOFWEEK()
<!-- example DAYOFWEEK -->
Возвращает целочисленный индекс дня недели (в диапазоне 1..7) для заданного аргумента временной метки согласно текущему часовому поясу.
Обратите внимание, что неделя начинается с воскресенья.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select dayofweek(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "dayofweek(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "dayofweek(now())": 5
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### DAYOFYEAR()
<!-- example DAYOFYEAR -->
Возвращает целое число дня года (в диапазоне 1..366) для заданного аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select dayofyear(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "dayofyear(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "dayofyear(now())": 214
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### YEARWEEK()
<!-- example YEARWEEK -->
Возвращает целочисленный год и код дня первого дня текущей недели (в диапазоне 1969001..2038366) для заданного аргумента временной метки согласно текущему часовому поясу.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select yearweek(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "yearweek(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "yearweek(now())": 2023211
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### YEARMONTH()
<!-- example YEARMONTH -->
Возвращает целочисленный код года и месяца (в диапазоне 196912..203801) из аргумента временной метки согласно текущему часовому поясу.

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
<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select yearmonth(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "yearmonth(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "yearmonth(now())": 202103
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### YEARMONTHDAY()
<!-- example YEARMONTHDAY -->
Возвращает целочисленный код года, месяца и даты (в диапазоне от 19691231 до 20380119) на основе текущего часового пояса.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select yearmonthday(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "yearmonthday(now())": {
          "type": "long"
        }
      }
    ],
    "data": [
      {
        "yearmonthday(now())": 20210315
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### TIMEDIFF()
<!-- example TIMEDIFF -->
Вычисляет разницу между двумя временными метками в формате `hh:ii:ss`.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select timediff(1615787586, 1613787583)"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "timediff(1615787586, 1613787583)": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "timediff(1615787586, 1613787583)": "555:33:23"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### DATEDIFF()
<!-- example DATEDIFF -->
Вычисляет количество дней между двумя заданными временными метками.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select datediff(1615787586, 1613787583)"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "datediff(1615787586, 1613787583)": {
          "type": "long long"
        }
      }
    ],
    "data": [
      {
        "datediff(1615787586, 1613787583)": 23
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### DATE()
<!-- example DATE -->
Форматирует часть даты из аргумента временной метки в строку в формате `YYYY-MM-DD`.

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


<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select date(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "date(now())": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "date(now())": "2023-08-02"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### TIME()
<!-- example TIME -->
Форматирует часть времени из аргумента временной метки в строку в формате `HH:MM:SS`.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select time(now())"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "time(now())": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "time(now())": "15:21:27"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

### DATE_FORMAT()
<!-- example DATE_FORMAT -->
Возвращает форматированную строку на основе предоставленных аргументов даты и формата. Аргумент формата использует те же спецификаторы, что и функция [strftime](https://man7.org/linux/man-pages/man3/strftime.3.html). Для удобства здесь приведены некоторые распространённые спецификаторы формата:

- `%Y` - Четырёхзначный год
- `%m` - Двузначный месяц (01-12)
- `%d` - Двузначный день месяца (01-31)
- `%H` - Двузначный час (00-23)
- `%M` - Двузначная минута (00-59)
- `%S` - Двузначная секунда (00-59)
- `%T` - Время в 24-часовом формате (`%H:%M:%S`)

Обратите внимание, что это не полный список спецификаторов. Пожалуйста, обратитесь к документации по `strftime()` для вашей операционной системы, чтобы получить полный перечень.

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

<!-- request JSON -->
```JSON
POST /sql?mode=raw -d "select DATE_FORMAT(NOW(), 'year %Y and time %T')"
```
<!-- response JSON -->
```JSON
[
  {
    "columns": [
      {
        "DATE_FORMAT(NOW(), 'year %Y and time %T')": {
          "type": "string"
        }
      }
    ],
    "data": [
      {
        "DATE_FORMAT(NOW(), 'year %Y and time %T')": "year 2023 and time 11:54:52"
      }
    ],
    "total": 1,
    "error": "",
    "warning": ""
  }
]
```
<!-- end -->

Этот пример форматирует текущие дату и время, отображая четырехзначный год и время в 24-часовом формате.

### DATE_HISTOGRAM()
<!-- example DATE_HISTOGRAM -->
`DATE_HISTOGRAM(expr, {calendar_interval='unit_name'})` принимает размер ведра в виде названия единицы измерения и возвращает номер ведра для значения. Значения округляются до ближайшего ведра. Основная функция:
```sql
key_of_the_bucket = interval * floor ( value / interval )
```
Интервалы могут быть указаны с использованием названия единицы, например `week`, или как одиночная единица, например `1M`. Однако множественные единицы, такие как `2d`, не поддерживаются в `calendar_interval`, но разрешены в `fixed_interval`.

Допустимые интервалы для `calendar_interval`:

- `minute`, `1m`
- `hour`, `1h`
- `day`, `1d`
- `week`, `1w` (неделя — это интервал между начальным днём недели, часом, минутой, секундой и следующей неделей, но тем же днём и временем недели)
- `month`, `1M`
- `year`, `1y` (год — это интервал между начальным днём месяца, временем и следующим годом, но тем же днём месяца и временем)

Допустимые интервалы для `fixed_interval`:

- `minute`, `2m`
- `hour`, `3h`
- `day`, `5d`

Используется в агрегировании, `FACET` и группировке.

Пример:

```sql
SELECT COUNT(*),
DATE_HISTOGRAM(tm, {calendar_interval='month'}) AS months
FROM facets
GROUP BY months ORDER BY months ASC;
```

### DATE_RANGE()
<!-- example DATE_RANGE -->
`DATE_RANGE(expr, {range_from='date_math', range_to='date_math'})` принимает набор диапазонов и возвращает номер ведра для значения.
Выражение включает значение `range_from` и исключает значение `range_to` для каждого диапазона. Диапазон может быть открытым — содержать только значение `range_from` или только `range_to`.
Разница с функцией [RANGE()](../Functions/Arrays_and_conditions_functions.md#RANGE%28%29) в том, что значения `range_from` и `range_to` могут быть выражены с помощью выражений [Date math](../Functions/Date_and_time_functions.md#Date-math).

Используется в агрегировании, `FACET` и группировке.

Пример:

```sql
SELECT COUNT(*),
DATE_RANGE(tm, {range_to='2017||+2M/M'},{range_from='2017||+2M/M',range_to='2017||+5M/M'},{range_from='2017||+5M/M'}) AS points
FROM idx_dates
GROUP BY points ORDER BY points ASC;
```

##### Date math

Date math позволяет работать с датами и временем непосредственно в ваших поисках. Это особенно полезно для работы с данными, которые изменяются с течением времени. С помощью date math вы можете легко находить записи за определённый период, анализировать тренды данных или управлять временем удаления информации. Это упрощает работу с датами, позволяя добавлять или вычитать время из заданной даты, округлять даты до ближайшей единицы времени и многое другое — всё это прямо в ваших поисковых запросах.

Для использования date math вы начинаете с базовой даты, которая может быть:
- `now` для текущей даты и времени,
- или конкретная строка даты, оканчивающаяся на `||`.

Затем вы можете изменять эту дату с помощью операций, таких как:
- `+1y` для добавления одного года,
- `-1h` для вычитания одного часа,
- `/m` для округления до ближайшего месяца.

Вы можете использовать следующие единицы в операциях:
- `s` для секунд,
- `m` для минут,
- `h` (или `H`) для часов,
- `d` для дней,
- `w` для недель,
- `M` для месяцев,
- `y` для лет.

Вот несколько примеров использования date math:
- `now+4h` — четыре часа от текущего времени.
- `now-2d/d` — время два дня назад, округлённое до ближайшего дня.
- `2010-04-20||+2M/d` — 20 июня 2010 года, округлённое до ближайшего дня.

<!-- proofread -->

