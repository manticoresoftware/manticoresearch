# SphinxSE

SphinxSE is a MySQL storage engine that can be compiled into MySQL/MariaDB servers using their pluggable architecture.

Despite its name, SphinxSE does *not* actually store any data itself. Instead, it serves as a built-in client that enables the MySQL server to communicate with `searchd`, execute search queries, and retrieve search results. All indexing and searching take place outside MySQL.

Some common SphinxSE applications include:
* Simplifying the porting of MySQL Full-Text Search (FTS) applications to Manticore;
* Enabling Manticore use with programming languages for which native APIs are not yet available;
* Offering optimizations when additional Manticore result set processing is needed on the MySQL side (e.g., JOINs with original document tables or additional MySQL-side filtering).

## Installing SphinxSE

You will need to obtain a copy of MySQL sources, prepare those, and then recompile MySQL binary. MySQL sources (mysql-5.x.yy.tar.gz) could be obtained from <http://dev.mysql.com> website.

### Compiling MySQL 5.0.x with SphinxSE

1.  copy `sphinx.5.0.yy.diff` patch file into MySQL sources directory and run
```bash
$ patch -p1 < sphinx.5.0.yy.diff
```
If there's no .diff file exactly for the specific version you need to:   build, try applying .diff with closest version numbers.  It is important that the patch should apply with no rejects.
2.  in MySQL sources directory, run
```bash
$ sh BUILD/autorun.sh
```
3.  in MySQL sources directory, create `sql/sphinx` directory in and copy all files in `mysqlse` directory from Manticore sources there. Example:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx
```
4.  configure MySQL and enable the new engine:
```bash
$ ./configure --with-sphinx-storage-engine
```
5.  build and install MySQL:
```bash
$ make
$ make install
```

### Compiling MySQL 5.1.x with SphinxSE

1. In the MySQL sources directory, create a `storage/sphinx` directory and copy all files from the `mysqlse` directory in the Manticore sources to this new location. For example:
```bash
$ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx
```
2.  In the MySQL source directory, run:
```bash
$ sh BUILD/autorun.sh
```
3. Configure MySQL and enable the Manticore engine:
```bash
$ ./configure --with-plugins=sphinx
```
4. Build and install MySQL:
```bash
$ make
$ make install
```

### Checking SphinxSE installation


<!-- example Example_1 -->

To verify that SphinxSE has been successfully compiled into MySQL, start the newly built server, run the MySQL client, and issue the `SHOW ENGINES` query. You should see a list of all available engines. Manticore should be present, and the "Support" column should display "YES":

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

## Using SphinxSE

To search using SphinxSE, you'll need to create a special ENGINE=SPHINX "search table" and then use a `SELECT` statement with the full-text query placed in the `WHERE` clause for the query column.

Here's an example create statement and search query:

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

In a search table, the first three columns *must* have the following types: `INTEGER UNSIGNED` or `BIGINT` for the 1st column (document ID), `INTEGER` or `BIGINT` for the 2nd column (match weight), and `VARCHAR` or `TEXT` for the 3rd column (your query). This mapping is fixed; you cannot omit any of these three required columns, move them around, or change their types. Additionally, the query column must be indexed, while all others should remain unindexed. Column names are ignored, so you can use any arbitrary names.

Additional columns must be either `INTEGER`, `TIMESTAMP`, `BIGINT`, `VARCHAR`, or `FLOAT`. They will be bound to attributes provided in the Manticore result set by name, so their names must match the attribute names specified in `sphinx.conf`. If there's no matching attribute name in the Manticore search results, the column will have `NULL` values.

Special "virtual" attribute names can also be bound to SphinxSE columns. Use `_sph_` instead of `@` for that purpose. For example, to obtain the values of `@groupby`, `@count`, or `@distinct` virtual attributes, use `_sph_groupby`, `_sph_count`, or `_sph_distinct` column names, respectively.

The `CONNECTION` string parameter is used to specify the Manticore host, port, and table. If no connection string is specified in `CREATE TABLE`, the table name `*` (i.e., search all tables) and `localhost:9312` are assumed. The connection string syntax is as follows:

```
CONNECTION="sphinx://HOST:PORT/TABLENAME"
```

You can change the default connection string later:

```sql
mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWTABLENAME";
```

You can also override these parameters on a per-query basis.

As shown in the example, both the query text and search options should be placed in the `WHERE` clause on the search query column (i.e., the 3rd column). Options are separated by semicolons and their names from values by an equality sign. Any number of options can be specified. The available options are:

* query - query text;
* mode - matching mode. Must be one of "all", "any", "phrase", "boolean", or "extended". Default is "all";
* sort - match sorting mode. Must be one of "relevance", "attr_desc", "attr_asc", "time_segments", or "extended". In all modes besides "relevance", the attribute name (or sorting clause for "extended") is also required after a colon:
```
... WHERE query='test;sort=attr_asc:group_id';
... WHERE query='test;sort=extended:@weight desc, group_id asc';
```
* offset - offset into the result set; default is 0;
* limit - number of matches to retrieve from the result set; default is 20;
* index - names of the tables to search:
```sql
... WHERE query='test;index=test1;';
... WHERE query='test;index=test1,test2,test3;';
```
* minid, maxid - min and max document ID to match;
* weights - comma-separated list of weights to be assigned to Manticore full-text fields:
```sql
... WHERE query='test;weights=1,2,3;';
```
* filter, !filter - comma-separated attribute name and a set of values to match:
```sql
# only include groups 1, 5 and 19
... WHERE query='test;filter=group_id,1,5,19;';
# exclude groups 3 and 11
... WHERE query='test;!filter=group_id,3,11;';
```
* range, !range - comma-separated (integer or bigint) Manticore attribute name, and min and max values to match:
```sql
# include groups from 3 to 7, inclusive
... WHERE query='test;range=group_id,3,7;';
# exclude groups from 5 to 25
... WHERE query='test;!range=group_id,5,25;';
```
* floatrange, !floatrange - comma-separated (floating point) Manticore attribute name, and min and max values to match:
```sql
# filter by a float size
... WHERE query='test;floatrange=size,2,3;';
# pick all results within 1000 meter from geoanchor
... WHERE query='test;floatrange=@geodist,0,1000;';
```
* maxmatches - maxmatches - per-query max matches value, as in [max_matches search option](../Searching/Options.md#max_matches):
```sql
... WHERE query='test;maxmatches=2000;';
```
* cutoff - maximum allowed matches, as in [cutoff search option](../Searching/Options.md#cutoff):
```sql
... WHERE query='test;cutoff=10000;';
```
* maxquerytime - maximum allowed query time (in milliseconds), as in [max_query_time search option](../Searching/Options.md#max_query_time):
```sql
... WHERE query='test;maxquerytime=1000;';
```
* groupby - group-by function and attribute. Read [this](../Searching/Grouping.md#Just-Grouping) about grouping search results:
```sql
... WHERE query='test;groupby=day:published_ts;';
... WHERE query='test;groupby=attr:group_id;';
```
* groupsort - group-by sorting clause:
```sql
... WHERE query='test;groupsort=@count desc;';
```
* distinct - an attribute to compute [COUNT(DISTINCT)](../Searching/Grouping.md#COUNT%28DISTINCT-field%29) for when doing group-by:
```sql
... WHERE query='test;groupby=attr:country_id;distinct=site_id';
```
* indexweights - comma-separated list of table names and weights to use when searching through several tables:
```sql
... WHERE query='test;indexweights=tbl_exact,2,tbl_stemmed,1;';
```
* fieldweights - comma-separated list of per-field weights that can be used by the ranker:
```sql
... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';
```
* comment - a string to mark this query in query log, as in [comment search option](../Searching/Options.md#comment):
```sql
... WHERE query='test;comment=marker001;';
```
* select - a string with expressions to compute:
```sql
... WHERE query='test;select=2*a+3*** as myexpr;';
```
* host, port - remote `searchd` host name and TCP port, respectively:
```sql
... WHERE query='test;host=sphinx-test.loc;port=7312;';
```
* ranker - a ranking function to use with "extended" matching mode, as in [ranker](../Searching/Options.md#ranker). Known values are "proximity_bm25", "bm25", "none", "wordcount", "proximity", "matchany", "fieldmask", "sph04", "expr:EXPRESSION" syntax to support expression-based ranker (where EXPRESSION should be replaced with your specific ranking formula), and "export:EXPRESSION":
```sql
... WHERE query='test;mode=extended;ranker=bm25;';
... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';
```
The "export" ranker functions similarly to ranker=expr, but it retains the per-document factor values, while ranker=expr discards them after computing the final `WEIGHT()` value. Keep in mind that ranker=export is intended for occasional use, such as training a machine learning (ML) function or manually defining your own ranking function, and should not be used in actual production. When utilizing this ranker, you'll likely want to examine the output of the `RANKFACTORS()` function, which generates a string containing all the field-level factors for each document.

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

* geoanchor - geodistance anchor. Learn more about Geo-search [in this section](../Searching/Geo_search.md). Takes 4 parameters, which are the latitude and longitude attribute names, and anchor point coordinates, respectively:
```sql
... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';
```

One **very important** note is that it is **much** more efficient to let Manticore handle sorting, filtering, and slicing the result set, rather than increasing the max matches count and using `WHERE`, `ORDER BY`, and `LIMIT` clauses on the MySQL side. This is due to two reasons. First, Manticore employs a variety of optimizations and performs these tasks better than MySQL. Second, less data would need to be packed by searchd, transferred, and unpacked by SphinxSE.


<!-- example Example_3 -->

You can obtain additional information related to the query results using the `SHOW ENGINE SPHINX STATUS` statement:

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

You can also access this information through status variables. Keep in mind that using this method does not require super-user privileges.

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

SphinxSE search tables can be joined with tables using other engines. Here's an example using the "documents" table from example.sql:

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

## Building snippets via MySQL


SphinxSE also features a UDF function that allows you to create snippets using MySQL. This functionality is similar to [HIGHLIGHT()](../../Searching/Highlighting.md#Highlighting), but can be accessed through MySQL+SphinxSE.

The binary providing the UDF is called `sphinx.so` and should be automatically built and installed in the appropriate location along with SphinxSE. If it doesn't install automatically for some reason, locate `sphinx.so` in the build directory and copy it to your MySQL instance's plugins directory. Once done, register the UDF with the following statement:

```sql
CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';
```

The function name *must* be sphinx_snippets; you cannot use an arbitrary name. The function arguments are as follows:

**Prototype:** function sphinx_snippets ( document, table, words [, options] );

The document and words arguments can be either strings or table columns. Options must be specified like this: `'value' AS option_name`. For a list of supported options, refer to the [Highlighting section](../Searching/Highlighting.md). The only UDF-specific additional option is called `sphinx` and allows you to specify the searchd location (host and port).

Usage examples:

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