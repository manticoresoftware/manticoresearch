# FEDERATED

С помощью движка MySQL FEDERATED вы можете подключаться к локальному или удалённому экземпляру Manticore из MySQL/MariaDB и выполнять поисковые запросы.

## Использование FEDERATED

Непосредственно запрос Manticore нельзя использовать с движком FEDERATED — его нужно «проксировать» (отправлять как строку в колонке) из-за ограничений движка FEDERATED и того факта, что Manticore реализует собственный синтаксис, например, конструкцию [MATCH](../Searching/Full_text_matching/Basic_usage.md).

Чтобы выполнить поиск через `FEDERATED`, сначала нужно создать таблицу с движком FEDERATED. Запрос Manticore будет включён в колонку `query` в операторе `SELECT`, выполняемом по таблице FEDERATED.

<!-- example create federated -->
Создание таблицы MySQL, совместимой с FEDERATED:


<!-- intro -->
##### SQL:

<!-- request Example -->

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
<!-- response Example-->

```sql
Query OK, 0 rows affected (0.00 sec)
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

Единственным жёстким правилом является колонка `query`. Она обязательна и должна быть единственной колонкой с привязкой к таблице.

Связанная через `FEDERATED` таблица Manticore **должна** быть физической таблицей (обычной или real-time).

Таблица FEDERATED должна содержать колонки с теми же названиями, что и атрибуты удалённой таблицы Manticore, так как по именам они будут связаны с атрибутами, возвращаемыми в результате запроса Manticore. Однако допускается отображать только некоторые атрибуты, не все.

Сервер Manticore идентифицирует запросы от клиента FEDERATED по имени пользователя "FEDERATED". Строковый параметр `CONNECTION` используется для указания хоста Manticore, SQL-порта и таблиц для запросов, поступающих через соединение. Синтаксис строки соединения следующий:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Поскольку у Manticore нет понятия базы данных, строка `DB` может быть произвольной и будет игнорироваться Manticore, но MySQL требует наличие значения в определении строки `CONNECTION`. Как видно в примере, полный SQL-запрос `SELECT` должен быть размещён в части WHERE в сравнении с колонкой `query`.

Поддерживается только оператор `SELECT`, не поддерживаются `INSERT`, `REPLACE`, `UPDATE` или `DELETE`.

### Советы по FEDERATED

Очень важное замечание: **намного** эффективнее позволять Manticore выполнять сортировку, фильтрацию и срезы результата, чем увеличивать максимальное количество совпадений и использовать WHERE, ORDER BY и LIMIT на стороне MySQL. Это связано с двумя причинами. Во-первых, Manticore реализует множество оптимизаций и показывает лучшую производительность по этим задачам, чем MySQL. Во-вторых, меньший объём данных нужно упаковывать в searchd, передавать и распаковывать между Manticore и MySQL.

<!-- example federated join -->
Операции JOIN можно выполнять между таблицей FEDERATED и другими таблицами MySQL. Это позволяет получать информацию, не хранящуюся в таблице Manticore.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Запрос для выполнения JOIN таблицы MySQL с таблицей FEDERATED, обслуживаемой Manticore:

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

