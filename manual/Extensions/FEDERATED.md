# FEDERATED

With the MySQL FEDERATED engine, you can connect to a local or remote Manticore instance from MySQL/MariaDB and perform search queries.

## Using FEDERATED

An actual Manticore query can't be used directly with the FEDERATED engine and must be "proxied" (sent as a string in a column) due to the FEDERATED engine's limitations and the fact that Manticore implements custom syntax like the [MATCH](../Searching/Full_text_matching/Basic_usage.md) clause.

To search via `FEDERATED`, you first need to create a FEDERATED engine table. The Manticore query will be included in a `query` column in the `SELECT` performed over the FEDERATED table.

<!-- example create federated -->
Creating a FEDERATED-compatible MySQL table:


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
Query FEDERATED compatible table:


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

The only fixed mapping is the `query` column. It is mandatory and must be the only column with a table attached.

The Manticore table linked via `FEDERATED` **must** be a physical table (plain or real-time).

The FEDERATED table should have columns with the same names as the remote Manticore table attributes since they will be bound to the attributes provided in the Manticore result set by name. However, it might map only some attributes, not all of them.

Manticore server identifies a query from a FEDERATED client by the user name "FEDERATED". The `CONNECTION` string parameter is used to specify the Manticore host, SQL port, and tables for queries coming through the connection. The connection string syntax is as follows:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

Since Manticore doesn't have the concept of a database, the `DB` string can be random as it will be ignored by Manticore, but MySQL requires a value in the `CONNECTION` string definition. As seen in the example, the full `SELECT` SQL query should be placed in a WHERE clause against the `query` column.

Only the `SELECT` statement is supported, not `INSERT`, `REPLACE`, `UPDATE`, or `DELETE`.

### FEDERATED tips

One **very important** note is that it is **much** more efficient to allow Manticore to perform sorting, filtering, and slicing the result set than to increase the max matches count and use WHERE, ORDER BY, and LIMIT clauses on the MySQL side. This is for two reasons. First, Manticore implements a number of optimizations and performs better than MySQL for these tasks. Second, less data needs to be packed by searchd, transferred, and unpacked between Manticore and MySQL.

<!-- example federated join -->
JOINs can be performed between a FEDERATED table and other MySQL tables. This can be used to retrieve information that is not stored in a Manticore table.


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### Query to JOIN MySQL based table with FEDERATED table served by Manticore:

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