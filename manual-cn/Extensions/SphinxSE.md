# SphinxSE

SphinxSE 是一个 MySQL 存储引擎，可以通过 MySQL/MariaDB 服务器的插件架构编译进 MySQL/MariaDB 服务器。

尽管名称为 SphinxSE，它实际上并不存储任何数据。它作为内置客户端，允许 MySQL 服务器与 `searchd` 通信，执行搜索查询并检索搜索结果。所有的索引和搜索操作都在 MySQL 之外完成。

一些常见的 SphinxSE 应用包括：

- 简化将 MySQL 全文搜索 (FTS) 应用程序迁移到 Manticore；
- 使 Manticore 可以与暂时没有提供原生 API 的编程语言一起使用；
- 在需要在 MySQL 端进行进一步处理时提供优化（例如，与原始文档表的 JOIN 操作或 MySQL 端的额外过滤）。

## 安装 SphinxSE

你需要获取 MySQL 源码副本，进行准备并重新编译 MySQL 二进制文件。MySQL 源码（mysql-5.x.yy.tar.gz）可以从 http://dev.mysql.com 网站获取。

### 编译带有 SphinxSE 的 MySQL 5.0.x

1.  将 `sphinx.5.0.yy.diff` 补丁文件复制到 MySQL 源码目录，并运行
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
如果没有适用于特定版本的 .diff 文件，可以尝试应用与版本号最接近的补丁。重要的是，补丁应该能够无错误应用。
2.  在 MySQL 源码目录中运行
```bash
$ sh BUILD/autorun.sh
```
3.  在 MySQL 源码目录中，创建 `sql/sphinx` 目录，并将 Manticore 源码中的 `mysqlse` 目录下的所有文件复制到该目录。例如：
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4.  配置 MySQL 并启用新引擎：
```bash
$ ./configure --with-sphinx-storage-engine
```
5.  编译并安装 MySQL：
```bash
$ make
$ make install
```

### 编译带有 SphinxSE 的 MySQL 5.1.x

1. 在 MySQL 源码目录中，创建 `storage/sphinx` 目录，并将 Manticore 源码中 `mysqlse` 目录的所有文件复制到该新位置。例如：
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2.  在 MySQL 源码目录中运行：
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

要验证 SphinxSE 是否已成功编译到 MySQL 中，启动新构建的服务器，运行 MySQL 客户端，并执行 `SHOW ENGINES` 查询。你应该能看到所有可用引擎的列表，其中 Manticore 应该会显示，并且“Support”列应显示“YES”：

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

要使用 SphinxSE 进行搜索，你需要创建一个特殊的 ENGINE=SPHINX“搜索表”，然后使用 `SELECT` 语句，并将全文查询放置在查询列的 `WHERE` 子句中。

以下是一个创建语句和搜索查询的示例：

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

在搜索表中，前三个列 *必须* 使用以下类型：第1列为 `INTEGER UNSIGNED` 或 `BIGINT`（文档 ID），第2列为 `INTEGER` 或 `BIGINT`（匹配权重），第3列为 `VARCHAR` 或 `TEXT`（查询内容）。此映射是固定的，不能省略、移动或更改这些三列。此外，查询列必须建立索引，其他列则不需要。列名可以使用任意名称，列名是忽略的。

其他列必须为 `INTEGER`、`TIMESTAMP`、`BIGINT`、`VARCHAR` 或 `FLOAT`。它们将绑定到 Manticore 结果集中提供的属性，列名必须与 `sphinx.conf` 中指定的属性名称相匹配。如果 Manticore 搜索结果中没有匹配的属性名，该列的值将为 `NULL`。

还可以将特殊的“虚拟”属性名称绑定到 SphinxSE 列。使用 `_sph_` 代替 `@` 来实现此功能。例如，要获取 `@groupby`、`@count` 或 `@distinct` 虚拟属性的值，可以分别使用 `_sph_groupby`、`_sph_count` 或 `_sph_distinct` 列名。

`CONNECTION` 字符串参数用于指定 Manticore 的主机、端口和表。如果在 `CREATE TABLE` 中未指定连接字符串，则假定表名为 `*`（即搜索所有表）并使用 `localhost:9312`。连接字符串的语法如下：

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

你可以稍后更改默认连接字符串：

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

你还可以在每个查询中覆盖这些参数。

如示例所示，查询文本和搜索选项应放置在搜索查询列（即第3列）的 `WHERE` 子句中。选项以分号分隔，名称与值之间用等号连接。可以指定任意数量的选项。可用选项包括：

- query - 查询文本；
- mode - 匹配模式。必须为 "all"、"any"、"phrase"、"boolean" 或 "extended" 之一。默认值为 "all"；
- sort - 匹配排序模式。必须为 "relevance"、"attr_desc"、"attr_asc"、"time_segments" 或 "extended" 之一。对于除 "relevance" 外的所有模式，排序列（对于 "extended"）或属性名称也是必需的：

```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset - 结果集中的偏移量；默认为 0；
* limit - 从结果集中检索的匹配项数；默认为 20；
* index - 要搜索的表名：
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid - 要匹配的最小和最大文档 ID；
* weights - 要分配给 Manticore 全文字段的权重列表：
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter - 要匹配的属性名及其值的逗号分隔列表：
```sql
# 仅包含组 1、5 和 19
... WHERE query='test;filter=group_id,1,5,19;';
# 排除组 3 和 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range - comma-separated (integer or bigint) Manticore attribute name, and min and max values to match:
```sql
# 包含 3 到 7 的组
... WHERE query='test;range=group_id,3,7;';
# 排除 5 到 25 的组
... WHERE query='test;!range=group_id,5,25;';
```
* range, !range - 要匹配的 Manticore 属性名以及最小和最大值的逗号分隔列表：
```sql
# 按浮点数大小过滤
... WHERE query='test;floatrange=size,2,3;';
# 按地理锚点距离过滤，选择 1000 米范围内的结果
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches - 每个查询的最大匹配数，如 [max_matches 搜索选项](../Searching/Options.md#max_matches) 中所述：
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff - 最大允许匹配数，如 [cutoff 搜索选项](../Searching/Options.md#cutoff) 中所述：
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - 最大允许查询时间（以毫秒为单位），如 [max_query_time 搜索选项](../Searching/Options.md#max_query_time) 中所述：
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - 分组函数和属性。参考 [此文](../Searching/Grouping.md#仅分组) 了解分组搜索结果：
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - 分组排序子句：
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - 分组时计算 [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT(DISTINCT-field)) 的属性：
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - 当搜索多个表时，使用的表名及权重的逗号分隔列表：
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - 排名器使用的每个字段权重的逗号分隔列表：
```sql
... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';
```
* comment - 用于在查询日志中标记此查询的字符串，如 [comment 搜索选项](../Searching/Options.md#comment) 中所述：
```sql
... WHERE query='test;comment=marker001;';
```
* select - 包含要计算的表达式的字符串：
```sql
... WHERE query='test;select=2*a+3*** as myexpr;';
```
* host, port - 远程 `searchd` 主机名和 TCP 端口：
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - 与 "extended" 匹配模式一起使用的排名函数，如 [ranker](../Searching/Options.md#ranker) 中所述。已知的值包括 "proximity_bm25"、"bm25"、"none"、"wordcount"、"proximity"、"matchany"、"fieldmask"、"sph04"、"expr

  " 语法支持表达式排名器（其中 EXPRESSION 应替换为特定的排名公式），以及 "export"：
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
“export” 排名器的功能类似于 ranker=expr，但它保留每个文档的因子值，而 ranker=expr 在计算最终的 `WEIGHT()` 值后会丢弃这些值。请记住，ranker=export 旨在偶尔使用，例如训练机器学习（ML）函数或手动定义自己的排名函数，不应在实际生产中使用。在使用此排名器时，您可能希望查看 `RANKFACTORS()` 函数的输出，该函数生成一个包含每个文档所有字段级因子的字符串。

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

* geoanchor - 地理距离锚点。有关地理搜索的更多信息，请查看[本节](../Searching/Geo_search.md)。它接受四个参数，分别是纬度和经度属性名称，以及锚点坐标。
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

一个**非常重要**的说明是，让 Manticore 处理结果集的排序、过滤和切片要**高效得多**，而不是增加最大匹配数并在 MySQL 端使用 `WHERE`、`ORDER BY` 和 `LIMIT` 子句。这有两个原因。首先，Manticore 采用多种优化措施，并且在执行这些任务时表现优于 MySQL。其次，需要由 searchd 打包、传输和由 SphinxSE 解压的数据量会更少。


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

您还可以通过状态变量访问这些信息。请注意，使用此方法不需要超级用户权限。

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

## 通过 MySQL 构建片段

SphinxSE 还具有一个 UDF 函数，允许您通过 MySQL 创建片段。此功能类似于 [HIGHLIGHT()](../Searching/Highlighting.md#Highlighting)，但可以通过 MySQL+SphinxSE 访问。

提供 UDF 的二进制文件称为 `sphinx.so`，应与 SphinxSE 一起自动构建并安装到适当的位置。如果由于某种原因未能自动安装，请在构建目录中找到 `sphinx.so`，并将其复制到 MySQL 实例的插件目录中。完成后，使用以下语句注册 UDF：

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

函数名称*必须*是 sphinx_snippets，不能使用其他名称。函数参数如下：

**原型：**函数 sphinx_snippets ( document, table, words [, options] );

document 和 words 参数可以是字符串或表列。选项必须像这样指定：`'value' AS option_name`。有关支持的选项列表，请参阅 [高亮部分](../Searching/Highlighting.md)。唯一的 UDF 特定附加选项称为 `sphinx`，允许您指定 searchd 的位置（主机和端口）。

使用示例：

```sql
SELECT sphinx_snippets('hello world doc', 'main', 'world',
    'sphinx://192.168.1.1/' AS sphinx, true AS exact_phrase,
    '[**]' AS before_match, '[/**]' AS after_match)
FROM documents;

SELECT title, sphinx_snippets(text, 'index', 'mysql php') AS text
    FROM sphinx, documents
    WHERE query='mysql php' AND sphinx.id=documents.id;
```
<!-- proofread -->