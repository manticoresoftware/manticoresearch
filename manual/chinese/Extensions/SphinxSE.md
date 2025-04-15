# SphinxSE

SphinxSE 是一个 MySQL 存储引擎，可以通过其可插拔架构编译到 MySQL/MariaDB 服务器中。

尽管其名称为 SphinxSE，但它 *不* 实际上存储任何数据。相反，它作为一个内置客户端，使 MySQL 服务器能够与 `searchd` 通信，执行搜索查询并检索搜索结果。所有索引和搜索操作都发生在 MySQL 之外。

一些常见的 SphinxSE 应用包括：
* 简化将 MySQL 全文搜索 (FTS) 应用程序移植到 Manticore；
* 允许 Manticore 与尚未提供本地 API 的编程语言一起使用；
* 当 MySQL 侧需要额外的 Manticore 结果集处理时提供优化（例如，与原始文档表的 JOIN 或额外的 MySQL 侧过滤）。

## 安装 SphinxSE

你需要获取一份 MySQL 源代码，准备好它们，然后重新编译 MySQL 二进制文件。MySQL 源代码 (mysql-5.x.yy.tar.gz) 可以从 <http://dev.mysql.com> 网站获取。

### 使用 SphinxSE 编译 MySQL 5.0.x

1. 将 `sphinx.5.0.yy.diff` 补丁文件复制到 MySQL 源代码目录中，并运行
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
如果没有适合特定版本的 .diff 文件，请尝试使用最接近版本号的 .diff。重要的是补丁应该能够无需拒绝地应用。
2. 在 MySQL 源代码目录中运行
```bash
$ sh BUILD/autorun.sh
```
3. 在 MySQL 源代码目录中创建 `sql/sphinx` 目录，并将 Manticore 源代码中的 `mysqlse` 目录中的所有文件复制到该目录。例如：
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4. 配置 MySQL 并启用新引擎：
```bash
$ ./configure --with-sphinx-storage-engine
```
5. 编译并安装 MySQL：
```bash
$ make
$ make install
```

### 使用 SphinxSE 编译 MySQL 5.1.x

1. 在 MySQL 源代码目录中，创建 `storage/sphinx` 目录，并将 Manticore 源代码中的 `mysqlse` 目录中的所有文件复制到此新位置。例如：
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2. 在 MySQL 源代码目录中运行：
```bash
$ sh BUILD/autorun.sh
```
3. 配置 MySQL 并启用 Manticore 引擎：
```bash
$ ./configure --with-plugins=sphinx
```
4. 编译并安装 MySQL：
```bash
$ make
$ make install
```

### 检查 SphinxSE 安装


<!-- example Example_1 -->

要验证 SphinxSE 是否已成功编译到 MySQL 中，请启动新构建的服务器，运行 MySQL 客户端，并发出 `SHOW ENGINES` 查询。你应该看到所有可用引擎的列表。应该包括 Manticore，并且 "Support" 列应显示 "YES"：

<!-- request -->

``` sql
mysql> show engines;
```

<!-- response -->
```sql
+------------+----------+-------------------------------------------------------------+
| Engine     | Support  | Comment                                                     |
+------------+----------+-------------------------------------------------------------+
| MyISAM     | DEFAULT  | 默认引擎，MySQL 3.23 及更高版本具有良好的性能          |
  ...
| SPHINX     | YES      | Manticore 存储引擎                                        |
  ...
+------------+----------+-------------------------------------------------------------+
13 rows in set (0.00 sec)
```

<!-- end -->

## 使用 SphinxSE

要使用 SphinxSE 进行搜索，你需要创建一个特殊的 ENGINE=SPHINX "搜索表"，然后使用带有放在 `WHERE` 子句中的全文查询的 `SELECT` 语句。

这是一个创建语句和搜索查询的示例：

```sql
CREATE TABLE t1
(
    id          INTEGER UNSIGNED NOT NULL,
    weight      INTEGER NOT NULL,
    query       VARCHAR(3072) NOT NULL,
    group_id    INTEGER,
    INDEX(query)
) ENGINE=SPHINX CONNECTION="sphinx://localhost:9312/test";

SELECT * FROM t1 WHERE query='test it;mode=any';
```

在搜索表中，前三列 *必须* 具有以下类型：第一列（文档 ID）为 `INTEGER UNSIGNED` 或 `BIGINT`，第二列（匹配权重）为 `INTEGER` 或 `BIGINT`，第三列（你的查询）为 `VARCHAR` 或 `TEXT`。这个映射是固定的；你不能省略任何这三列，移动它们或更改它们的类型。此外，查询列必须被索引，而所有其他列应保持未索引。列名被忽略，因此你可以使用任何任意名称。

附加列必须是 `INTEGER`、`TIMESTAMP`、`BIGINT`、`VARCHAR` 或 `FLOAT`。它们将按名称与 Manticore 结果集中提供的属性绑定，因此它们的名称必须与 `sphinx.conf` 文件中指定的属性名称匹配。如果在 Manticore 搜索结果中没有匹配的属性名称，该列将具有 `NULL` 值。

特殊的 "虚拟" 属性名称也可以绑定到 SphinxSE 列。为此，请使用 `_sph_` 而不是 `@`。例如，要获得 `@groupby`、`@count` 或 `@distinct` 虚拟属性的值，请分别使用 `_sph_groupby`、`_sph_count` 或 `_sph_distinct` 列名。

`CONNECTION` 字符串参数用于指定 Manticore 主机、端口和表。如果在 `CREATE TABLE` 中未指定连接字符串，则假定表名为 `*`（即，搜索所有表）和 `localhost:9312`。连接字符串语法如下：

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

你可以稍后更改默认连接字符串：

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

你也可以在每个查询的基础上覆盖这些参数。
如示例所示，查询文本和搜索选项应放置于搜索查询列的 `WHERE` 子句中（即第三列）。选项通过分号分隔，名称与值之间用等号分隔。可以指定任意数量的选项。可用的选项有：

* query - 查询文本；
* mode - 匹配模式。必须是 "all"、"any"、"phrase"、"boolean" 或 "extended" 之一。默认是 "all"；
* sort - 匹配排序模式。必须是 "relevance"、"attr_desc"、"attr_asc"、"time_segments" 或 "extended" 之一。在除了 "relevance" 的所有模式中，属性名称（或 "extended" 的排序子句）也是在冒号后面所需的：
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset - 结果集的偏移量；默认是 0；
* limit - 从结果集中检索的匹配数；默认是 20；
* index - 要搜索的表名：
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid - 要匹配的最小和最大文档 ID；
* weights - 要分配给 Manticore 全文字段的逗号分隔权重列表：
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter - 要匹配的逗号分隔属性名称和值集合：
```sql
# 仅包括组 1、5 和 19
... WHERE query='test;filter=group_id,1,5,19;';
# 排除组 3 和 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range - 逗号分隔（整数或 bigint）Manticore 属性名称，以及要匹配的最小和最大值：
```sql
# 包括组从 3 到 7，包含
... WHERE query='test;range=group_id,3,7;';
# 排除组从 5 到 25
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange - 逗号分隔（浮点）Manticore 属性名称，以及要匹配的最小和最大值：
```sql
# 按浮动大小过滤
... WHERE query='test;floatrange=size,2,3;';
# 选择距离 geoanchor 1000 米以内的所有结果
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches - 每个查询的最大匹配值，如 [max_matches 搜索选项](../Searching/Options.md#max_matches):
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff - 允许的最大匹配数，如 [cutoff 搜索选项](../Searching/Options.md#cutoff):
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - 允许的最长查询时间（以毫秒为单位），如 [max_query_time 搜索选项](../Searching/Options.md#max_query_time):
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - 分组函数和属性。阅读 [这篇文章](../Searching/Grouping.md#Just-Grouping) 了解分组搜索结果：
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - 分组排序子句：
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - 在进行分组时计算 [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT%28DISTINCT-field%29) 的属性：
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - 逗号分隔的表名和权重列表，在搜索多个表时使用：
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - 逗号分隔的每个字段权重列表，可由排名器使用：
```sql
... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';
```
* comment - 用于在查询日志中标记此查询的字符串，如 [comment 搜索选项](../Searching/Options.md#comment):
```sql
... WHERE query='test;comment=marker001;';
```
* select - 一个带有表达式的字符串，用于计算：
```sql
... WHERE query='test;select=2*a+3*** as myexpr;';
```
* host, port - 远程 `searchd` 主机名和 TCP 端口：
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - 用于 "extended" 匹配模式的排名函数，如 [ranker](../Searching/Options.md#ranker)。已知值包括 "proximity_bm25"、"bm25"、"none"、"wordcount"、"proximity"、"matchany"、"fieldmask"、"sph04"、支持表达式基础排名器的 "expr:EXPRESSION" 语法（其中 EXPRESSION 应替换为您的特定排名公式），以及 "export:EXPRESSION":
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
"export" 排名器功能类似于 ranker=expr，但它保留每个文档的因子值，而 ranker=expr 在计算最终的 `WEIGHT()` 值后会丢弃它们。请记住，ranker=export 旨在偶尔使用，例如训练机器学习（ML）函数或手动定义自己的排名函数，且不应在实际生产中使用。使用此排名器时，您可能希望检查 `RANKFACTORS()` 函数的输出，该函数生成包含每个文档所有字段级因子的字符串。

<!-- example SQL Example_2 -->
<!-- request -->

``` sql
SELECT *, WEIGHT(), RANKFACTORS()
    FROM myindex
    WHERE MATCH('dog')
    OPTION ranker=export('100*bm25');
```

<!-- response -->

``` sql
*************************** 1\. row ***************************
           id: 555617
    published: 1110067331
   channel_id: 1059819
        title: 7
      content: 428
     weight(): 69900
rankfactors(): bm25=699, bm25a=0.666478, field_mask=2,
doc_word_count=1, field1=(lcs=1, hit_count=4, word_count=1,
tf_idf=1.038127, min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532,
min_hit_pos=120, min_best_span_pos=120, exact_hit=0,
max_window_hits=1), word1=(tf=4, idf=0.259532)
*************************** 2\. row ***************************
           id: 555313
    published: 1108438365
   channel_id: 1058561
        title: 8
      content: 249
     weight(): 68500
rankfactors(): bm25=685, bm25a=0.675213, field_mask=3,
doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1,
tf_idf=0.259532, min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532,
min_hit_pos=8, min_best_span_pos=8, exact_hit=0, max_window_hits=1),
field1=(lcs=1, hit_count=2, word_count=1, tf_idf=0.519063,
min_idf=0.259532, max_idf=0.259532, sum_idf=0.259532, min_hit_pos=36,
min_best_span_pos=36, exact_hit=0, max_window_hits=1), word1=(tf=3,
idf=0.259532)
```

<!-- end -->

* geoanchor - 地理距离锚点。了解更多关于地理搜索的信息 [在本节中](../Searching/Geo_search.md)。需要4个参数，分别是纬度和经度属性名称，以及锚点坐标：
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

一个 **非常重要** 的注意事项是，**更**有效地让Manticore处理排序、过滤和切片结果集，而不是增加最大匹配计数并在MySQL端使用`WHERE`、`ORDER BY`和`LIMIT`子句。这有两个原因。首先，Manticore采用了多种优化，执行这些任务的效率优于MySQL。其次，搜索服务需要打包、传输和解包的数据更少。


<!-- example Example_3 -->

您可以使用 `SHOW ENGINE SPHINX STATUS` 语句获取与查询结果相关的附加信息：

<!-- request -->


``` sql
mysql> SHOW ENGINE SPHINX STATUS;
```

<!-- response -->
``` sql
+--------+-------+-------------------------------------------------+
| Type   | Name  | Status                                          |
+--------+-------+-------------------------------------------------+
| SPHINX | stats | total: 25, total found: 25, time: 126, words: 2 |
| SPHINX | words | sphinx:591:1256 soft:11076:15945                |
+--------+-------+-------------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->


<!-- example Example_4 -->

您还可以通过状态变量访问此信息。请记住，使用此方法不需要超级用户权限。

<!-- request -->

``` sql
mysql> SHOW STATUS LIKE 'sphinx_%';
```

<!-- response -->
``` sql
+--------------------+----------------------------------+
| Variable_name      | Value                            |
+--------------------+----------------------------------+
| sphinx_total       | 25                               |
| sphinx_total_found | 25                               |
| sphinx_time        | 126                              |
| sphinx_word_count  | 2                                |
| sphinx_words       | sphinx:591:1256 soft:11076:15945 |
+--------------------+----------------------------------+
5 rows in set (0.00 sec)
```

<!-- end -->


<!-- example SQL Example_5 -->

SphinxSE 搜索表可以与使用其他引擎的表进行连接。以下是一个使用 example.sql 中的 "documents" 表的示例：

<!-- request -->

``` sql
mysql> SELECT content, date_added FROM test.documents docs
-> JOIN t1 ON (docs.id=t1.id)
-> WHERE query="one document;mode=any";

mysql> SHOW ENGINE SPHINX STATUS;
```

<!-- response -->

``` sql
+-------------------------------------+---------------------+
| content                             | docdate             |
+-------------------------------------+---------------------+
| this is my test document number two | 2006-06-17 14:04:28 |
| this is my test document number one | 2006-06-17 14:04:28 |
+-------------------------------------+---------------------+
2 rows in set (0.00 sec)

+--------+-------+---------------------------------------------+
| Type   | Name  | Status                                      |
+--------+-------+---------------------------------------------+
| SPHINX | stats | total: 2, total found: 2, time: 0, words: 2 |
| SPHINX | words | one:1:2 document:2:2                        |
+--------+-------+---------------------------------------------+
2 rows in set (0.00 sec)
```

<!-- end -->

## 通过 MySQL 构建片段


SphinxSE 还具有一个 UDF 函数，可以让您使用 MySQL 创建片段。此功能类似于 [HIGHLIGHT()](../../Searching/Highlighting.md#Highlighting)，但可以通过 MySQL+SphinxSE 访问。

提供 UDF 的二进制文件称为 `sphinx.so`，应与 SphinxSE 一起自动构建并安装在适当位置。如果由于某种原因它没有自动安装，请在构建目录中找到 `sphinx.so` 并将其复制到您的 MySQL 实例的插件目录中。完成后，用以下语句注册 UDF：

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

函数名称 *必须* 是 sphinx_snippets；您无法使用任意名称。函数参数如下：

**原型：** function sphinx_snippets ( document, table, words [, options] );

document 和 words 参数可以是字符串或表列。选项必须像这样指定：`'value' AS option_name`。有关支持选项的列表，请参阅 [Highlighting section](../Searching/Highlighting.md)。唯一与 UDF 特定的附加选项称为 `sphinx`，允许您指定 searchd 位置（主机和端口）。

使用示例：

```sql
SELECT sphinx_snippets('hello world doc', 'main', 'world',
    'sphinx://192.168.1.1/' AS sphinx, true AS exact_phrase,
    '[**]' AS before_match, '[/**]' AS after_match)
FROM documents;

SELECT title, sphinx_snippets(text, 'table', 'mysql php') AS text
    FROM sphinx, documents
    WHERE query='mysql php' AND sphinx.id=documents.id;
```
<!-- proofread -->

