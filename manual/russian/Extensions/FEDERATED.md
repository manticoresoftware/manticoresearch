# FEDERATED

С помощью двигателя MySQL FEDERATED вы можете подключаться к локальному или удалённому экземпляру Manticore из MySQL/MariaDB и выполнять поисковые запросы.

## Использование FEDERATED

Непосредственно использовать Manticore-запрос с двигателем FEDERATED нельзя, он должен быть «проксирован» (отправлен в виде строки в столбце) из-за ограничений двигателя FEDERATED и того факта, что Manticore реализует собственный синтаксис, например, конструкцию [MATCH](../Searching/Full_text_matching/Basic_usage.md).

Для поиска через `FEDERATED` сначала необходимо создать таблицу с движком FEDERATED. Manticore-запрос будет включен в столбец `query` в операторе `SELECT`, выполняемом над таблицей FEDERATED.

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

Единственное фиксированное сопоставление — это столбец `query`. Он обязательный и должен быть единственным столбцом, прикреплённым к таблице.

Таблица Manticore, связанная через `FEDERATED`, **должна** быть физической таблицей (plain или real-time).

Таблица FEDERATED должна содержать столбцы с теми же именами, что и атрибуты удалённой таблицы Manticore, так как именно по именам они будут связаны с атрибутами в результате Manticore. Однако она может отображать только часть атрибутов, а не все.

Сервер Manticore идентифицирует запрос от клиента FEDERATED по имени пользователя "FEDERATED". Параметр строки `CONNECTION` используется для указания хоста Manticore, SQL-порта и таблиц для запросов, поступающих через это соединение. Синтаксис строки соединения следующий:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Поскольку в Manticore отсутствует понятие базы данных, строка `DB` может быть произвольной, так как она игнорируется Manticore, однако MySQL требует наличие значения в определении строки `CONNECTION`. Как показано в примере, полный SQL-запрос `SELECT` должен быть помещён в условие WHERE по столбцу `query`.

Поддерживается только оператор `SELECT`, не поддерживаются `INSERT`, `REPLACE`, `UPDATE` или `DELETE`.

### Советы по FEDERATED

Очень **важно** отметить, что гораздо эффективнее позволить Manticore выполнять сортировку, фильтрацию и нарезку результирующего набора, чем увеличивать максимальное число совпадений и использовать условия WHERE, ORDER BY и LIMIT на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore реализует множество оптимизаций и выполняет эти задачи лучше, чем MySQL. Во-вторых, требуется меньше данных упаковывать searchd, передавать и распаковывать между Manticore и MySQL.

<!-- example federated join -->
Можно выполнять JOIN между таблицей FEDERATED и другими таблицами MySQL. Это может использоваться для получения информации, не хранящейся в таблице Manticore.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Запрос для JOIN таблицы на базе MySQL с таблицей FEDERATED, обслуживаемой Manticore:

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

