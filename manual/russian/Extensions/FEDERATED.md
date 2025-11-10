# FEDERATED

С помощью движка MySQL FEDERATED вы можете подключаться к локальному или удалённому экземпляру Manticore из MySQL/MariaDB и выполнять поисковые запросы.

## Использование FEDERATED

Непосредственно запрос Manticore нельзя использовать с движком FEDERATED из-за ограничений FEDERATED и того факта, что Manticore реализует собственный синтаксис, например, оператор [MATCH](../Searching/Full_text_matching/Basic_usage.md).

Для поиска через `FEDERATED` сначала нужно создать таблицу с движком FEDERATED. Запрос Manticore будет включён в столбец `query` в операторе `SELECT`, выполняемом по таблице FEDERATED.

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

Единственное фиксированное сопоставление — это столбец `query`. Он обязателен и должен быть единственным столбцом, связанным с таблицей.

Таблица Manticore, связанная через `FEDERATED`, **должна** быть физической таблицей (обычной или реального времени).

Таблица FEDERATED должна иметь столбцы с такими же именами, как атрибуты удалённой таблицы Manticore, поскольку они будут связаны с атрибутами, предоставленными в наборе результатов Manticore по имени. Однако может быть сопоставлена только часть атрибутов, а не все.

Сервер Manticore идентифицирует запрос от клиента FEDERATED по имени пользователя "FEDERATED". Параметр строки `CONNECTION` используется для указания хоста Manticore, SQL-порта и таблиц для запросов, поступающих через соединение. Синтаксис строки соединения следующий:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Поскольку в Manticore нет понятия базы данных, строка `DB` может быть произвольной, так как она будет игнорироваться Manticore, но MySQL требует значение в определении строки `CONNECTION`. Как видно из примера, полный SQL-запрос `SELECT` должен быть помещён в условие WHERE по столбцу `query`.

Поддерживается только оператор `SELECT`, не поддерживаются `INSERT`, `REPLACE`, `UPDATE` или `DELETE`.

### Советы по FEDERATED

Очень **важно** отметить, что гораздо эффективнее позволить Manticore выполнять сортировку, фильтрацию и нарезку набора результатов, чем увеличивать максимальное количество совпадений и использовать условия WHERE, ORDER BY и LIMIT на стороне MySQL. Это по двум причинам. Во-первых, Manticore реализует ряд оптимизаций и работает лучше MySQL для этих задач. Во-вторых, меньше данных нужно упаковывать searchd, передавать и распаковывать между Manticore и MySQL.

<!-- example federated join -->
JOIN можно выполнять между таблицей FEDERATED и другими таблицами MySQL. Это можно использовать для получения информации, которая не хранится в таблице Manticore.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Запрос для объединения таблицы MySQL с таблицей FEDERATED, обслуживаемой Manticore:

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

