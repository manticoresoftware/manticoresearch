# FEDERATED

Using MySQL FEDERATED engine you can connect to a local or remote Manticore instance from MySQL/MariaDB and perform search queries.

## Using FEDERATED

An actual Manticore query cannot be used directly with FEDERATED engine and must be "proxied" (sent as a string in a column) due to limitations of the FEDERATED engine and the fact that Manticore implements custom syntax like the [MATCH](Searching/Full_text_matching/Basic_usage.md) clause.

To search via `FEDERATED`, you would need to create first a FEDERATED engine table. The Manticore query will be included in a `query` column in the `SELECT` performed over the FEDERATED table.

<!-- example create federated -->
Creating FEDERATED compatible MySQL table:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CREATE TABLE t1
(
    id          INTEGER UNSIGNED NOT NULL,
    year        INTEGER NOT NULL,
    rating    	FLOAT,
    query       VARCHAR(3072) NOT NULL,
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

The only fixed mapping is `query` column. It is mandatory and must be the only column with an index attached.

The Manticore index that is linked via `FEDERATED` **must** be an index with storage (plain or RealTime).

FEDERATED table should have columns with same names as remote Manticore index attributes as will be bound to attributes provided in Manticore result set by name, however might map not all attributes but only some of them. Arbitrary expression from query select list which name "hides" index attribute will be used at result set.

Manticore server identifies query from FEDERATED client by user name "FEDERATED". `CONNECTION` string parameter should be used to specify searchd host, SQL port and indexes for queries issued using thistable. Connection string syntax is as follows:

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/INDEXNAME"
```

Since Manticore doesn't have the concept of database, the `DB` string can be random as it will be ignored by Manticore, but MySQL requires a value in the `CONNECTION` string definition. As seen in the example, full `SELECT` SQL query should be put into a WHERE clause against column `query`.

Only `SELECT` statement is supported, not `INSERT`, `REPLACE`, `UPDATE`, `DELETE`.

### FEDERATED tips

One **very important** note that it is **much** more efficient to allow Manticore to perform sorting, filtering and slicing the result set than to raise max matches count and use WHERE, ORDER BY and LIMIT clauses on MySQL side. This is for two reasons. First, Manticore does a number of optimizations and performs better than MySQL on these tasks. Second, less data would need to be packed by searchd, transferred and unpacked between Manticore and MySQL.

<!-- example federated join -->
JOINs can be performed between FEDERATED table and other MySQL tables. This can be used to retrieve information that is not stored in the Manticore index.


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
