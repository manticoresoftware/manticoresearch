# FEDERATED

С помощью движка MySQL FEDERATED вы можете подключиться к локальному или удаленному экземпляру Manticore из MySQL/MariaDB и выполнять поисковые запросы.

## Использование FEDERATED

Фактический запрос Manticore не может быть использован напрямую с движком FEDERATED и должен быть "проксирован" (отправлен в виде строки в столбце) из-за ограничений движка FEDERATED и того факта, что Manticore реализует собственный синтаксис, такой как предложение [MATCH](../Searching/Full_text_matching/Basic_usage.md).

Для поиска через `FEDERATED` сначала необходимо создать таблицу движка FEDERATED. Запрос Manticore будет включен в столбец `query` в `SELECT`, выполняемом над таблицей FEDERATED.

<!-- example create federated -->
Создание таблицы MySQL, совместимой с FEDERATED:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE t1
(
    id          INTEGER UNSIGNED NOT NULL,
    year        INTEGER NOT NULL,
    rating    	FLOAT,
    query       VARCHAR(1024) NOT NULL,
    INDEX(query)
) ENGINE=FEDERATED
DEFAULT CHARSET=utf8
CONNECTION='mysql://FEDERATED@127.0.0.1:9306/DB/movies';
```
<!-- response SQL-->

```sql
Query OK, 0 rows affected (0.00 sec)
```
<!-- end -->

<!-- example select federated -->
Запрос к таблице, совместимой с FEDERATED:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM t1 WHERE query='SELECT * FROM movies WHERE MATCH (\'pie\')';
```

<!-- response SQL-->

```sql
+----+------+--------+------------------------------------------+
| id | year | rating | query                                    |
+----+------+--------+------------------------------------------+
|  1 | 2019 |      5 | SELECT * FROM movies WHERE MATCH ('pie') |
+----+------+--------+------------------------------------------+
1 row in set (0.04 sec)
```
<!-- end -->

Единственное фиксированное сопоставление — это столбец `query`. Он обязателен и должен быть единственным столбцом с привязанной таблицей.

Таблица Manticore, связанная через `FEDERATED`, **обязательно** должна быть физической таблицей (plain или real-time).

Таблица FEDERATED должна иметь столбцы с теми же именами, что и атрибуты удаленной таблицы Manticore, поскольку они будут привязаны к атрибутам, предоставляемым в результирующем наборе Manticore, по имени. Однако она может сопоставлять только некоторые атрибуты, а не все.

Сервер Manticore идентифицирует запрос от клиента FEDERATED по имени пользователя "FEDERATED". Параметр `CONNECTION` string используется для указания хоста Manticore, SQL-порта и таблиц для запросов, поступающих через соединение. Синтаксис строки подключения следующий:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Поскольку в Manticore нет концепции базы данных, строка `DB` может быть произвольной, так как она будет проигнорирована Manticore, но MySQL требует значения в определении строки `CONNECTION`. Как видно из примера, полный SQL-запрос `SELECT` должен быть помещен в условие WHERE для столбца `query`.

Поддерживается только оператор `SELECT`, но не `INSERT`, `REPLACE`, `UPDATE` или `DELETE`.

### Советы по использованию FEDERATED

Одно **очень важное** замечание: **гораздо** эффективнее позволить Manticore выполнять сортировку, фильтрацию и нарезку результирующего набора, чем увеличивать максимальное количество совпадений и использовать предложения WHERE, ORDER BY и LIMIT на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore реализует ряд оптимизаций и работает лучше, чем MySQL, для этих задач. Во-вторых, меньше данных нужно упаковывать searchd, передавать и распаковывать между Manticore и MySQL.

<!-- example federated join -->
JOIN можно выполнять между таблицей FEDERATED и другими таблицами MySQL. Это можно использовать для получения информации, которая не хранится в таблице Manticore.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Запрос для JOIN таблицы на основе MySQL с таблицей FEDERATED, обслуживаемой Manticore:

```sql
SELECT t1.id, t1.year, comments.comment FROM t1 JOIN comments ON t1.id=comments.post_id WHERE query='SELECT * FROM movies WHERE MATCH (\'pie\')';
```

<!-- response SQL-->

```sql
+----+------+--------------+
| id | year | comment      |
+----+------+--------------+
|  1 | 2019 | was not good |
+----+------+--------------+
1 row in set (0.00 sec)
```

<!-- end -->
<!-- proofread -->

