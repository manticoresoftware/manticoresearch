# FEDERATED

С помощью движка MySQL FEDERATED вы можете подключаться к локальному или удалённому экземпляру Manticore из MySQL/MariaDB и выполнять поисковые запросы.

## Использование FEDERATED

Непосредственно запрос Manticore нельзя использовать с движком FEDERATED, его нужно "проксиировать" (отправлять в виде строки в столбце) из-за ограничений движка FEDERATED и того, что Manticore использует нестандартный синтаксис, например, оператор [MATCH](../Searching/Full_text_matching/Basic_usage.md).

Чтобы выполнять поиск через `FEDERATED`, сначала нужно создать таблицу с движком FEDERATED. Запрос Manticore будет включён в столбец `query` внутри оператора `SELECT`, выполняемого по таблице FEDERATED.

<!-- example create federated -->
Создание совместимой с FEDERATED таблицы MySQL:


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

<!-- intro -->
##### JSON:

<!-- request JSON -->

```JSON
POST /sql?mode=raw -d "CREATE TABLE t1 (id INTEGER UNSIGNED NOT NULL, year INTEGER NOT NULL, rating	FLOAT, query VARCHAR(1024) NOT NULL, INDEX(query)) ENGINE=FEDERATED DEFAULT CHARSET=utf8 CONNECTION='mysql://FEDERATED@127.0.0.1:9306/DB/movies';"
```
<!-- response JSON-->

```JSON
```

<!-- end -->

<!-- example select federated -->
Запрос к таблице совместимой с FEDERATED:


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

Единственное жёсткое сопоставление – это столбец `query`. Он обязателен и должен быть единственным столбцом, к которому привязана таблица.

Таблица Manticore, связанная через `FEDERATED`, **должна** быть физической таблицей (обычной или в режиме реального времени).

Таблица FEDERATED должна содержать столбцы с такими же именами, как атрибуты удалённой таблицы Manticore, так как они будут связаны с атрибутами из результата Manticore по именам. Однако может быть сопоставлена не вся таблица, а лишь некоторые атрибуты.

Сервер Manticore идентифицирует запросы от клиента FEDERATED по имени пользователя "FEDERATED". Параметр строки `CONNECTION` используется для указания хоста Manticore, SQL-порта и таблиц для запросов, поступающих через соединение. Синтаксис строки подключения следующий:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Поскольку в Manticore нет понятия базы данных, значение `DB` может быть произвольным, так как оно будет игнорироваться Manticore, но MySQL требует, чтобы в строке `CONNECTION` было указано значение. Как показано в примере, полный SQL-запрос `SELECT` должен быть помещён в условие WHERE к столбцу `query`.

Поддерживается только оператор `SELECT`, а не `INSERT`, `REPLACE`, `UPDATE` или `DELETE`.

### Советы по FEDERATED

Очень **важно** понимать, что гораздо эффективнее позволить Manticore выполнять сортировку, фильтрацию и выборку частей результата, чем увеличивать максимальное число совпадений и использовать WHERE, ORDER BY и LIMIT на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore реализует множество оптимизаций и работает лучше MySQL для этих задач. Во-вторых, меньше данных передаётся, упаковывается и распаковывается между searchd и MySQL.

<!-- example federated join -->
Можно выполнять операции JOIN между таблицей FEDERATED и другими таблицами MySQL. Это позволяет получать информацию, не хранящуюся в таблице Manticore.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Запрос, который соединяет таблицу MySQL с таблицей FEDERATED, обслуживаемой Manticore:

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

