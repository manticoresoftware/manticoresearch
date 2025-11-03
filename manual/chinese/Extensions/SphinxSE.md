# SphinxSE

SphinxSE 是一个 MySQL 存储引擎，可以通过 MySQL/MariaDB 服务器的可插拔架构编译进服务器中。

尽管名字叫 SphinxSE，但它实际上并不存储任何数据。相反，它作为一个内置客户端，使 MySQL 服务器能够与 `searchd` 通信，执行搜索查询并检索搜索结果。所有的索引和搜索操作都发生在 MySQL 之外。

一些常见的 SphinxSE 应用包括：
* 简化 MySQL 全文搜索（FTS）应用向 Manticore 的移植；
* 使得 Manticore 能够与尚无原生 API 的编程语言一起使用；
* 当需要在 MySQL 端对 Manticore 结果集进行额外处理时（例如，与原始文档表的 JOIN 或额外的 MySQL 端过滤）提供优化。

## 安装 SphinxSE

你需要获取 MySQL 源码，准备好后重新编译 MySQL 二进制文件。MySQL 源码（mysql-5.x.yy.tar.gz）可以从 <http://dev.mysql.com> 网站获取。

### 编译带有 SphinxSE 的 MySQL 5.0.x

1. 将 `sphinx.5.0.yy.diff` 补丁文件复制到 MySQL 源码目录并运行
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
如果没有针对你需要构建的具体版本的 .diff 文件，尝试使用版本号最接近的 .diff 文件。补丁必须能无拒绝地应用。
2. 在 MySQL 源码目录，运行
```bash
$ sh BUILD/autorun.sh
```
3. 在 MySQL 源码目录，创建 `sql/sphinx` 目录，并将 Manticore 源码中的 `mysqlse` 目录下的所有文件复制到该目录。例如：
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4. 配置 MySQL 并启用新引擎：
```bash
$ ./configure --with-sphinx-storage-engine
```
5. 构建并安装 MySQL：
```bash
$ make
$ make install
```

### 编译带有 SphinxSE 的 MySQL 5.1.x

1. 在 MySQL 源码目录，创建 `storage/sphinx` 目录，并将 Manticore 源码中的 `mysqlse` 目录下的所有文件复制到该新目录。例如：
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2. 在 MySQL 源码目录，运行：
```bash
$ sh BUILD/autorun.sh
```
3. 配置 MySQL 并启用 Manticore 引擎：
```bash
$ ./configure --with-plugins=sphinx
```
4. 构建并安装 MySQL：
```bash
$ make
$ make install
```

### 检查 SphinxSE 安装


<!-- example Example_1 -->

要验证 SphinxSE 是否成功编译进 MySQL，启动新构建的服务器，运行 MySQL 客户端，并执行 `SHOW ENGINES` 查询。你应该能看到所有可用引擎的列表。Manticore 应该存在，且“Support”列显示“YES”：

<!-- request -->

``` sql
mysql> show engines;
```

<!-- response -->
```sql
+------------+----------+-------------------------------------------------------------+
| Engine     | Support  | Comment                                                     |
+------------+----------+-------------------------------------------------------------+
| MyISAM     | DEFAULT  | Default engine as of MySQL 3.23 with great performance      |
  ...
| SPHINX     | YES      | Manticore storage engine                                       |
  ...
+------------+----------+-------------------------------------------------------------+
13 rows in set (0.00 sec)
```

<!-- end -->

## 使用 SphinxSE

要使用 SphinxSE 进行搜索，你需要创建一个特殊的 ENGINE=SPHINX “搜索表”，然后使用带有全文查询的 `SELECT` 语句，将查询放在查询列的 `WHERE` 子句中。

下面是一个创建语句和搜索查询的示例：

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

在搜索表中，前三列 *必须* 具有以下类型：第 1 列（文档 ID）为 `INTEGER UNSIGNED` 或 `BIGINT`，第 2 列（匹配权重）为 `INTEGER` 或 `BIGINT`，第 3 列（你的查询）为 `VARCHAR` 或 `TEXT`。此映射是固定的；你不能省略这三列中的任何一列，不能调整它们的位置，也不能更改它们的类型。此外，查询列必须被索引，而其他列应保持未索引。列名被忽略，因此你可以使用任意名称。

额外的列必须是 `INTEGER`、`TIMESTAMP`、`BIGINT`、`VARCHAR` 或 `FLOAT`。它们将根据名称绑定到 Manticore 结果集中提供的属性，因此它们的名称必须与 `sphinx.conf` 中指定的属性名称匹配。如果 Manticore 搜索结果中没有匹配的属性名称，该列将包含 `NULL` 值。

特殊的“虚拟”属性名称也可以绑定到 SphinxSE 列。为此使用 `_sph_` 替代 `@`。例如，要获取 `@groupby`、`@count` 或 `@distinct` 虚拟属性的值，分别使用 `_sph_groupby`、`_sph_count` 或 `_sph_distinct` 作为列名。

`CONNECTION` 字符串参数用于指定 Manticore 主机、端口和表。如果在 `CREATE TABLE` 中未指定连接字符串，则默认假定表名为 `*`（即搜索所有表）且为 `localhost:9312`。连接字符串语法如下：

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

你可以稍后更改默认连接字符串：

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

你也可以在每个查询中覆盖这些参数。

如示例所示，查询文本和搜索选项都应放在搜索查询列（即第 3 列）的 `WHERE` 子句中。选项用分号分隔，名称和值用等号分隔。可以指定任意数量的选项。可用选项包括：

* query - 查询文本；
* mode - 匹配模式。必须是 "all"、"any"、"phrase"、"boolean" 或 "extended" 之一。默认是 "all"；
* sort - 匹配排序模式。必须是 "relevance"、"attr_desc"、"attr_asc"、"time_segments" 或 "extended" 之一。除 "relevance" 外，其他模式后面还需跟冒号及属性名（或 "extended" 的排序子句）：
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset - 结果集偏移量；默认是 0；
* limit - 从结果集中检索的匹配数；默认是 20；
* index - 要搜索的表名：
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid - 匹配的最小和最大文档 ID；
* weights - 逗号分隔的权重列表，分配给 Manticore 全文字段：
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter - 逗号分隔的属性名和匹配值集合：
```sql
# only include groups 1, 5 and 19
... WHERE query='test;filter=group_id,1,5,19;';
# exclude groups 3 and 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range - 逗号分隔的（整数或 bigint）Manticore 属性名，以及匹配的最小和最大值：
```sql
# include groups from 3 to 7, inclusive
... WHERE query='test;range=group_id,3,7;';
# exclude groups from 5 to 25
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange - 逗号分隔的（浮点数）Manticore 属性名，以及匹配的最小和最大值：
```sql
# filter by a float size
... WHERE query='test;floatrange=size,2,3;';
# pick all results within 1000 meter from geoanchor
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches - 每个查询的最大匹配数值，如 [max_matches 搜索选项](../Searching/Options.md#max_matches) 所示：
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff - 最大允许匹配数，如 [cutoff 搜索选项](../Searching/Options.md#cutoff) 所示：
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - 最大允许的查询时间（以毫秒为单位），如同 [max_query_time 搜索选项](../Searching/Options.md#max_query_time)：
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - 分组函数和属性。阅读 [此处](../Searching/Grouping.md#Just-Grouping) 了解分组搜索结果：
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
* indexweights - 用逗号分隔的表名和权重列表，用于在多个表中搜索时使用：
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - 用逗号分隔的每字段权重列表，可被排序器使用：
```sql
... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';
```
* comment - 用于在查询日志中标记此查询的字符串，如同 [comment 搜索选项](../Searching/Options.md#comment)：
```sql
... WHERE query='test;comment=marker001;';
```
* select - 用于计算的表达式字符串：
```sql
... WHERE query='test;select=2*a+3*** as myexpr;';
```
* host, port - 远程 `searchd` 主机名和 TCP 端口，分别为：
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - 用于“扩展”匹配模式的排名函数，如同 [ranker](../Searching/Options.md#ranker)。已知值包括 "proximity_bm25"、"bm25"、"none"、"wordcount"、"proximity"、"matchany"、"fieldmask"、"sph04"、"expr:EXPRESSION" 语法以支持基于表达式的排序器（其中 EXPRESSION 应替换为您的具体排名公式），以及 "export:EXPRESSION"：
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
“export” 排序器的功能类似于 ranker=expr，但它保留每个文档的因子值，而 ranker=expr 在计算最终的 `WEIGHT()` 值后会丢弃它们。请注意，ranker=export 旨在偶尔使用，例如训练机器学习（ML）函数或手动定义您自己的排名函数，不应在实际生产中使用。使用此排序器时，您可能希望检查 `RANKFACTORS()` 函数的输出，该函数生成包含每个文档所有字段级因子的字符串。

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

* geoanchor - 地理距离锚点。更多关于地理搜索的信息请参见 [本节](../Searching/Geo_search.md)。接受4个参数，分别是纬度和经度属性名，以及锚点坐标：
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

一个**非常重要**的注意事项是，让 Manticore 处理排序、过滤和结果集切片要**高效得多**，而不是增加最大匹配数并在 MySQL 端使用 `WHERE`、`ORDER BY` 和 `LIMIT` 子句。这有两个原因。首先，Manticore 采用多种优化，执行这些任务比 MySQL 更好。其次，searchd 需要打包、传输和 SphinxSE 需要解包的数据量更少。


### 使用 SphinxSE 时关于存储字段的重要说明

从版本 5.0.0 起，Manticore 默认存储所有字段。当 Manticore 与 MySQL 或 MariaDB 通过 SphinxSE 一起使用时，通常不需要存储所有字段，因为原始数据已经存储在 MySQL/MariaDB 中。在这种设置中，建议通过设置显式禁用相关 Manticore 表的存储字段：

```
stored_fields =
```

请参阅设置参考：[stored_fields](../Creating_a_table/Local_tables/Plain_and_real-time_table_settings.md#stored_fields)。

如果保持默认（存储所有字段），然后通过 SphinxSE 一次选择大量文档，可能会超过引擎的内部限制，并收到类似以下错误：

"bad searchd response length"

设置 `stored_fields =` 可以避免将大量存储的负载发送回 MySQL/MariaDB，并防止在典型的 SphinxSE 集成中出现此错误。


### SHOW ENGINE SPHINX 状态

<!-- example Example_3 -->

您可以使用 `SHOW ENGINE SPHINX STATUS` 语句获取与查询结果相关的更多信息：

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

您也可以通过状态变量访问此信息。请注意，使用此方法不需要超级用户权限。

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

SphinxSE 搜索表可以与使用其他引擎的表进行连接。以下是使用 example.sql 中的 "documents" 表的示例：

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

## 通过 MySQL 构建摘要片段


SphinxSE 还提供了一个 UDF 函数，允许您使用 MySQL 创建摘要片段。此功能类似于 [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting)，但可以通过 MySQL+SphinxSE 访问。

提供该 UDF 的二进制文件名为 `sphinx.so`，应自动构建并安装在与 SphinxSE 一起的适当位置。如果由于某种原因未自动安装，请在构建目录中找到 `sphinx.so` 并复制到您的 MySQL 实例的插件目录。完成后，使用以下语句注册 UDF：

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

函数名*必须*是 sphinx_snippets；不能使用任意名称。函数参数如下：

**原型：** function sphinx_snippets ( document, table, words [, options] );

document 和 words 参数可以是字符串或表列。options 必须像这样指定：`'value' AS option_name`。有关支持的选项列表，请参阅 [高亮部分](../Searching/Highlighting.md)。唯一的 UDF 特定附加选项称为 `sphinx`，允许您指定 searchd 的位置（主机和端口）。

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

