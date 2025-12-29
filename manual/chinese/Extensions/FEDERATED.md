# FEDERATED

使用MySQL的FEDERATED引擎，您可以从MySQL/MariaDB连接到本地或远程的Manticore实例，并执行搜索查询。

## 使用FEDERATED

实际的Manticore查询不能直接与FEDERATED引擎一起使用，而必须通过“代理”（作为字符串在列中发送）发送，因为FEDERATED引擎的限制以及Manticore实现自定义语法如[MATCH](../Searching/Full_text_matching/Basic_usage.md)子句。

要通过`FEDERATED`进行搜索，您首先需要创建一个FEDERATED引擎表。Manticore查询将包含在`SELECT`语句中执行的FEDERATED表的`query`列中。

<!-- example create federated -->
创建一个FEDERATED兼容的MySQL表：


<!-- intro -->
##### SQL：

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
查询FEDERATED兼容表：


<!-- intro -->
##### SQL：

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

唯一的固定映射是`query`列。它是必需的，并且必须是唯一的一个与表关联的列。

通过`FEDERATED`链接的Manticore表**必须**是一个物理表（普通或实时）。

FEDERATED表的列名应与远程Manticore表属性相同，因为它们将绑定到Manticore结果集中的属性名称。然而，它可能只映射一些属性，而不是所有属性。

Manticore服务器通过用户名“FEDERATED”识别来自FEDERATED客户端的查询。`CONNECTION`字符串参数用于指定通过连接发送的查询的Manticore主机、SQL端口和表。`CONNECTION`字符串的语法如下：

```ini
CONNECTION="mysql://FEDERATED@HOST:PORT/DB/TABLENAME"
```

由于Manticore没有数据库的概念，`DB`字符串可以是随机的，因为它将被Manticore忽略，但MySQL要求`CONNECTION`字符串定义中有一个值。如示例所示，完整的`SELECT`SQL查询应放置在`query`列的WHERE子句中。

仅支持`SELECT`语句，不支持`INSERT`、`REPLACE`、`UPDATE`或`DELETE`。

### FEDERATED提示

一个**非常重要**的注意事项是，允许Manticore执行排序、过滤和切片结果集比在MySQL侧增加最大匹配数并使用WHERE、ORDER BY和LIMIT子句要高效得多。这是有两个原因。首先，Manticore实现了许多优化并在此类任务中表现优于MySQL。其次，Manticore需要传输的数据量更少，因此在Manticore和MySQL之间传输的数据量更少。

<!-- example federated join -->
可以在FEDERATED表和其他MySQL表之间执行JOIN操作。这可以用于检索未存储在Manticore表中的信息。


<!-- intro -->
##### SQL：

<!-- request SQL -->
##### 查询以JOIN MySQL基于表与由Manticore提供的FEDERATED表：

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

