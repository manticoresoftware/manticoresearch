# FEDERATED

С помощью движка MySQL FEDERATED вы можете подключаться к локальному или удалённому экземпляру Manticore из MySQL/MariaDB и выполнять поисковые запросы.

## Использование FEDERATED

Фактический запрос Manticore не может быть использован напрямую с движком FEDERATED и должен быть «проксирован» (отправлен как строка в столбце) из-за ограничений движка FEDERATED и того факта, что Manticore реализует собственный синтаксис, такой как оператор [MATCH](../Searching/Full_text_matching/Basic_usage.md).

Для поиска через `FEDERATED` сначала необходимо создать таблицу с использованием движка FEDERATED. Запрос Manticore будет включён в столбец `query` в `SELECT`, выполняемом над таблицей FEDERATED.

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
POST /sql?mode=raw -d "CREATE TABLE t1 (id INTEGER UNSIGNED NOT NULL, year INTEGER NOT NULL, rating FLOAT, query VARCHAR(1024) NOT NULL, INDEX(query)) ENGINE=FEDERATED DEFAULT CHARSET=utf8 CONNECTION='mysql://FEDERATED@127.0.0.1:9306/DB/movies'"
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

Единственным фиксированным сопоставлением является столбец `query`. Он обязателен и должен быть единственным столбцом, связанный с таблицей.

Таблица Manticore, связанная через `FEDERATED`, **должна** быть физической таблицей (plain или real-time).

Таблица FEDERATED должна иметь столбцы с такими же именами, как и атрибуты удалённой таблицы Manticore, поскольку они будут связаны по имени с атрибутами, предоставленными в результирующем наборе Manticore. Однако она может отображать только некоторые атрибуты, а не все.

Сервер Manticore идентифицирует запрос от клиента FEDERATED по имени пользователя "FEDERATED". Строковый параметр `CONNECTION` используется для указания хоста Manticore, SQL-порта и таблиц для запросов, поступающих через соединение. Синтаксис строки подключения следующий:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Так как в Manticore отсутствует концепция базы данных, строка `DB` может быть произвольной, так как Manticore её игнорирует, но MySQL требует наличие значения в определении строки `CONNECTION`. Как видно из примера, полный SQL-запрос `SELECT` должен быть размещён в условии WHERE для столбца `query`.

Поддерживается только оператор `SELECT`, не поддерживаются `INSERT`, `REPLACE`, `UPDATE` или `DELETE`.

### Советы по FEDERATED

Очень **важное** замечание: **гораздо** эффективнее позволить Manticore выполнять сортировку, фильтрацию и нарезку результата, чем увеличивать максимально допустимое количество совпадений и использовать WHERE, ORDER BY и LIMIT на стороне MySQL. Это объясняется двумя причинами. Во-первых, Manticore реализует множество оптимизаций и работает лучше MySQL при выполнении этих задач. Во-вторых, требуется меньше данных упаковывать поисковым демоном (searchd), передавать и распаковывать между Manticore и MySQL.

<!-- example federated join -->
JOINS могут выполняться между таблицей FEDERATED и другими таблицами MySQL. Это может использоваться для получения информации, которая не хранится в таблице Manticore.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Запрос для объединения (JOIN) таблицы на основе MySQL с таблицей FEDERATED, обслуживаемой Manticore:

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

