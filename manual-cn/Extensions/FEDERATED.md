# FEDERATED

使用 MySQL 的 FEDERATED 引擎，您可以从 MySQL/MariaDB 连接到本地或远程的 Manticore 实例并执行搜索查询。

## 使用 FEDERATED

由于 FEDERATED 引擎的限制，以及 Manticore 实现了自定义语法（例如 [MATCH](../Searching/Full_text_matching/Basic_usage.md) 子句），因此实际的 Manticore 查询不能直接与 FEDERATED 引擎一起使用，必须作为字符串通过列 "代理" 发送。

要通过 `FEDERATED` 进行搜索，首先需要创建一个 FEDERATED 引擎表。Manticore 查询将包含在对 FEDERATED 表执行的 `SELECT` 查询的 `query` 列中。

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
查询与 FEDERATED 兼容的表：


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

唯一的固定映射是 `query` 列。它是强制性的，并且必须是附加了表的唯一列。

通过 `FEDERATED` 连接的 Manticore 表 **必须** 是物理表（普通表或实时表）。

FEDERATED 表应具有与远程 Manticore 表属性相同名称的列，因为它们将通过名称与 Manticore 结果集中提供的属性绑定。但是，它可能只映射某些属性，而不是全部属性。

Manticore 服务器通过用户名 "FEDERATED" 来识别来自 FEDERATED 客户端的查询。`CONNECTION` 字符串参数用于指定 Manticore 主机、SQL 端口和通过连接传递查询的表。连接字符串的语法如下：

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

由于 Manticore 没有数据库的概念，`DB` 字符串可以是任意的，因为 Manticore 将忽略它，但 MySQL 要求在 `CONNECTION` 字符串定义中包含该值。如示例中所示，完整的 `SELECT` SQL 查询应放置在针对 `query` 列的 WHERE 子句中。

仅支持 `SELECT` 语句，不支持 `INSERT`、`REPLACE`、`UPDATE` 或 `DELETE`。

### FEDERATED 提示

一个 **非常重要的** 注意事项是，与其增加最大匹配数并在 MySQL 端使用 WHERE、ORDER BY 和 LIMIT 子句，不如让 Manticore 负责执行排序、过滤和对结果集的切片操作。这有两个原因。首先，Manticore 实现了许多优化，并且在这些任务上比 MySQL 执行得更好。其次，在 Manticore 和 MySQL 之间传递的数据量会减少，因此数据打包、传输和解包的负担也会减轻。

<!-- example federated join -->
可以在 FEDERATED 表和其他 MySQL 表之间执行 JOIN 操作。这可用于检索未存储在 Manticore 表中的信息。


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### 查询将 MySQL 基于表的 JOIN 与 Manticore 提供的 FEDERATED 表结合:

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