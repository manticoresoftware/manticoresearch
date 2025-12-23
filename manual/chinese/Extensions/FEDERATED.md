# FEDERATED

使用 MySQL 的 FEDERATED 引擎，您可以从 MySQL/MariaDB 连接到本地或远程的 Manticore 实例并执行搜索查询。

## 使用 FEDERATED

由于 FEDERATED 引擎的限制以及 Manticore 实现了自定义语法如 [MATCH](../Searching/Full_text_matching/Basic_usage.md) 子句，实际上 Manticore 查询不能直接用于 FEDERATED 引擎，必须通过“代理”（作为列中的字符串发送）。

要通过 `FEDERATED` 进行搜索，首先需要创建一个 FEDERATED 引擎表。Manticore 查询将包含在对 FEDERATED 表执行的 `SELECT` 查询中的 `query` 列中。

<!-- example create federated -->
创建一个兼容 FEDERATED 的 MySQL 表：


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
查询兼容 FEDERATED 的表：


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

唯一固定的映射是 `query` 列。它是必需的，并且必须是唯一附带表的列。

通过 `FEDERATED` 连接的 Manticore 表**必须**是物理表（普通或实时表）。

FEDERATED 表应具有与远程 Manticore 表属性同名的列，因为它们将按名称绑定到 Manticore 返回结果中的属性。但是，它可能只映射部分属性，而非全部。

Manticore 服务器通过用户名 "FEDERATED" 识别来自 FEDERATED 客户端的查询。`CONNECTION` 字符串参数用于指定 Manticore 的主机、SQL 端口和通过该连接发出查询的表。连接字符串的语法如下：

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

由于 Manticore 没有数据库概念，`DB` 字符串可以是任意值，Manticore 会忽略它，但 MySQL 要求 `CONNECTION` 字符串定义中必须有值。如示例所示，完整的 `SELECT` SQL 查询应放在针对 `query` 列的 WHERE 子句中。

只支持 `SELECT` 语句，不支持 `INSERT`、`REPLACE`、`UPDATE` 或 `DELETE`。

### FEDERATED 提示

一个**非常重要**的注意事项是，让 Manticore 执行排序、过滤和结果集切片要**远远**高效于增加最大匹配数并在 MySQL 端使用 WHERE、ORDER BY 和 LIMIT 子句。这有两个原因：首先，Manticore 实现了许多优化，对于这些任务性能优于 MySQL；其次，searchd 打包、传输及解包的数据量减少了。

<!-- example federated join -->
可以在 FEDERATED 表和其他 MySQL 表之间执行 JOIN。这可以用来检索未存储在 Manticore 表中的信息。


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### 将基于 MySQL 的表与由 Manticore 提供的 FEDERATED 表进行 JOIN 查询：

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

