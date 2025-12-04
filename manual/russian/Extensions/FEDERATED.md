# FEDERATED

С помощью движка MySQL FEDERATED вы можете подключаться к локальному или удалённому экземпляру Manticore из MySQL/MariaDB и выполнять поисковые запросы.

## Использование FEDERATED

Непосредственно запрос Manticore нельзя использовать с движком FEDERATED, его необходимо "посредничать" (отправлять в виде строки в колонке) из-за ограничений движка FEDERATED и того факта, что Manticore реализует собственный синтаксис, например, оператор [MATCH](../Searching/Full_text_matching/Basic_usage.md).

Чтобы осуществить поиск через `FEDERATED`, сначала нужно создать таблицу с движком FEDERATED. Запрос Manticore будет включён в колонку `query` в `SELECT`, выполняемом по таблице FEDERATED.

<!-- example create federated -->
Создание MySQL таблицы, совместимой с FEDERATED:


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
POST /sql?mode=raw -d "CREATE TABLE t1 (id INTEGER UNSIGNED NOT NULL, year INTEGER NOT NULL, rating FLOAT, query VARCHAR(1024) NOT NULL, INDEX(query)) ENGINE=FEDERATED DEFAULT CHARSET=utf8 CONNECTION='mysql://FEDERATED@127.0.0.1:9306/DB/movies';"
```
<!-- response JSON-->

```JSON
```

<!-- end -->

<!-- example select federated -->
Запрос таблицы, совместимой с FEDERATED:


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

Единственное фиксированное соответствие — колонка `query`. Она обязательна и должна быть единственной колонкой с привязанной таблицей.

Таблица Manticore, связанная через `FEDERATED`, **должна** быть физической таблицей (plain или real-time).

Таблица FEDERATED должна иметь колонки с такими же именами, как атрибуты удалённой таблицы Manticore, так как они будут связаны с атрибутами, предоставленными в результате Manticore по имени. Однако может быть сопоставлена лишь часть атрибутов, а не все.

Сервер Manticore идентифицирует запрос от клиента FEDERATED по имени пользователя "FEDERATED". Параметр строки `CONNECTION` используется для указания хоста Manticore, SQL порта и таблиц для запросов, поступающих через соединение. Синтаксис строки соединения следующий:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Поскольку в Manticore отсутствует понятие базы данных, строка `DB` может быть произвольной, так как Manticore её игнорирует, но MySQL требует наличия значения в определении строки `CONNECTION`. Как видно из примера, полный SQL запрос `SELECT` должен быть размещён в условии WHERE к столбцу `query`.

Поддерживается только оператор `SELECT`, не поддерживаются `INSERT`, `REPLACE`, `UPDATE` или `DELETE`.

### Советы по FEDERATED

Одно **очень важноe** замечание: **гораздо** эффективнее позволить Manticore выполнять сортировку, фильтрацию и срез результатов, чем увеличивать лимит максимального количества совпадений и использовать WHERE, ORDER BY и LIMIT с MySQL стороны. Это по двум причинам. Во-первых, Manticore реализует множество оптимизаций и работает лучше MySQL в этих задачах. Во-вторых, меньше данных требуется упаковывать searchd, передавать и распаковывать между Manticore и MySQL.

<!-- example federated join -->
Можно выполнять JOIN между таблицей FEDERATED и другими MySQL таблицами. Это используется для получения информации, которая не хранится в таблице Manticore.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Запрос для JOIN таблицы на основе MySQL с FEDERATED таблицей, обслуживаемой Manticore:

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

