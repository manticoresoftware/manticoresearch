# FEDERATED

通过 MySQL 的 FEDERATED 引擎，您可以从 MySQL/MariaDB 连接到本地或远程的 Manticore 实例并执行搜索查询。

## 使用 FEDERATED

由于 FEDERATED 引擎的限制以及 Manticore 实现了自定义语法如 [MATCH](../Searching/Full_text_matching/Basic_usage.md) 子句，实际上 Manticore 查询不能直接在 FEDERATED 引擎中使用，必须通过“代理”（以字符串形式放在列中）发送。

要通过 `FEDERATED` 进行搜索，首先需要创建一个 FEDERATED 引擎表。Manticore 查询将包含在对 FEDERATED 表执行的 `SELECT` 语句中的 `query` 列中。

<!-- example create federated -->
创建一个兼容 FEDERATED 的 MySQL 表：


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

唯一固定的映射是 `query` 列。该列是必需的，且必须是唯一带有表关联的列。

通过 `FEDERATED` 连接的 Manticore 表 **必须** 是一个物理表（普通表或实时表）。

FEDERATED 表应有与远程 Manticore 表属性同名的列，因为这些列会按名称绑定到 Manticore 结果集中提供的属性。然而，它可能只映射部分属性，而不是全部。

Manticore 服务器通过用户名 "FEDERATED" 来识别来自 FEDERATED 客户端的查询。`CONNECTION` 字符串参数用于指定通过连接传递查询的 Manticore 主机、SQL 端口和表。连接字符串语法如下所示：

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

由于 Manticore 没有数据库的概念，`DB` 字符串可以是任意值，因为 Manticore 会忽略它，但 MySQL 要求在 `CONNECTION` 字符串定义中必须有值。如示例所示，完整的 `SELECT` SQL 查询应放在对 `query` 列的 WHERE 子句中。

仅支持 `SELECT` 语句，不支持 `INSERT`、`REPLACE`、`UPDATE` 或 `DELETE`。

### FEDERATED 小贴士

一个**非常重要**的注意事项是，让 Manticore 进行结果集的排序、过滤与切片要比增加最大匹配数并在 MySQL 端使用 WHERE、ORDER BY 和 LIMIT 子句**更有效率**。原因有二：首先，Manticore 对这些任务实现了多种优化，性能优于 MySQL；其次，Manticore 与 MySQL 之间需要打包、传输和解包的数据量更少。

<!-- example federated join -->
可以在 FEDERATED 表和其他 MySQL 表之间执行 JOIN。这可以用来检索未存储在 Manticore 表中的信息。


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### 将基于 MySQL 的表与由 Manticore 提供的 FEDERATED 表进行 JOIN 的查询：

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

