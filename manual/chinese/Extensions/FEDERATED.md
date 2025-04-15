# 联邦

使用 MySQL 联邦引擎，您可以从 MySQL/MariaDB 连接到本地或远程的 Manticore 实例并执行搜索查询。

## 使用联邦

实际的 Manticore 查询不能直接与联邦引擎一起使用，必须通过 "代理"（作为列中的字符串发送）由于联邦引擎的限制以及 Manticore 实现了像 [MATCH](../Searching/Full_text_matching/Basic_usage.md) 子句这样的自定义语法。

要通过 `FEDERATED` 进行搜索，您首先需要创建一个联邦引擎表。Manticore 查询将包含在对联邦表执行的 `SELECT` 的 `query` 列中。

<!-- example create federated -->
创建一个兼容联邦的 MySQL 表：


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
查询联邦兼容表：


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

唯一固定的映射是 `query` 列。它是必需的，并且必须是附加表中唯一的列。

通过 `FEDERATED` 连接的 Manticore 表 **必须** 是一个物理表（普通或实时）。

联邦表应具有与远程 Manticore 表属性相同名称的列，因为它们将通过名称绑定到 Manticore 结果集中提供的属性。然而，它可能只映射某些属性，而不是所有属性。

Manticore 服务器通过用户名 "FEDERATED" 识别来自联邦客户端的查询。`CONNECTION` 字符串参数用于指定 Manticore 主机、SQL 端口以及通过连接发送的查询表。连接字符串语法如下：

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

由于 Manticore 没有数据库的概念，`DB` 字符串可以是任意值，因为它将被 Manticore 忽略，但 MySQL 要求在 `CONNECTION` 字符串定义中提供一个值。如示例所示，完整的 `SELECT` SQL 查询应放在针对 `query` 列的 WHERE 子句中。

仅支持 `SELECT` 语句，不支持 `INSERT`、`REPLACE`、`UPDATE` 或 `DELETE`。

### 联邦提示

一个 **非常重要** 的注意事项是，允许 Manticore 执行结果集的排序、过滤和切片 **要比** 增加最大匹配计数并在 MySQL 端使用 WHERE、ORDER BY 和 LIMIT 子句 **高效得多**。这有两个原因。首先，Manticore 实现了一些优化，执行这些任务的性能优于 MySQL。其次，搜索过程中需要传输的、打包的数据量较少，减少了 Manticore 和 MySQL 之间的传输和解包。

<!-- example federated join -->
可以在联邦表和其他 MySQL 表之间执行 JOIN。这可以用于检索未存储在 Manticore 表中的信息。


<!-- intro -->
##### SQL:

<!-- request SQL -->
##### 将 MySQL 基础表与由 Manticore 提供的联邦表进行连接的查询：

```sql
SELECT t1.id, t1.year, comments.comment FROM t1 JOIN comments ON t1.id=comments.post_id WHERE query='SELECT * FROM movies WHERE MATCH (\'pie\')';
```

<!-- response SQL-->

```sql
+----+------+--------------+
| id | year | comment      |
+----+------+--------------+
|  1 | 2019 | 不太好       |
+----+------+--------------+
1 row in set (0.00 sec)
```

<!-- end -->
<!-- proofread -->
