MySQL storage engine (SphinxSE)
-------------------------------

SphinxSE overview
~~~~~~~~~~~~~~~~~

SphinxSE is MySQL storage engine which can be compiled into MySQL server
5.x using its pluggable architecture. It is not available for MySQL 4.x
series. It also requires MySQL 5.0.22 or higher in 5.0.x series, or
MySQL 5.1.12 or higher in 5.1.x series.

Despite the name, SphinxSE does *not* actually store any data itself. It
is actually a built-in client which allows MySQL server to talk to
``searchd``, run search queries, and obtain search results. All indexing
and searching happen outside MySQL.

Obvious SphinxSE applications include:

-  easier porting of MySQL FTS applications to Manticore;

-  allowing Manticore use with programming languages for which native APIs
   are not available yet;

-  optimizations when additional Manticore result set processing on MySQL
   side is required (eg. JOINs with original document tables, additional
   MySQL-side filtering, etc).

 
Installing SphinxSE
^^^^^^^^^^^^^^^^^^^

You will need to obtain a copy of MySQL sources, prepare those, and then
recompile MySQL binary. MySQL sources (mysql-5.x.yy.tar.gz) could be
obtained from  http://dev.mysql.com  Web site.

For some MySQL versions, there are delta tarballs with already prepared
source versions available from Manticore Web site. After unzipping those
over original sources MySQL would be ready to be configured and built
with Manticore support.

If such tarball is not available, or does not work for you for any
reason, you would have to prepare sources manually. You will need to GNU
Autotools framework (autoconf, automake and libtool) installed to do
that.

.. _Compiling MySQL 5.0.x with SphinxSE:

Compiling MySQL 5.0.x with SphinxSE
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. copy ``sphinx.5.0.yy.diff`` patch file into MySQL sources directory
   and run

.. code-block:: bash


       $ patch -p1 < sphinx.5.0.yy.diff

If there's no .diff file exactly for the specific version you need to
   build, try applying .diff with closest version numbers. It is
   important that the patch should apply with no rejects.

2. in MySQL sources directory, run

.. code-block:: bash


       $ sh BUILD/autorun.sh

3. in MySQL sources directory, create ``sql/sphinx`` directory in and
   copy all files in ``mysqlse`` directory from Manticore sources there.
   Example:

.. code-block:: bash


       $ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.0.24/sql/sphinx

4. configure MySQL and enable Manticore engine:

.. code-block:: bash


       $ ./configure --with-sphinx-storage-engine

5. build and install MySQL:

.. code-block:: bash


       $ make
       $ make install

.. _Compiling MySQL 5.1.x with SphinxSE:

Compiling MySQL 5.1.x with SphinxSE
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. in MySQL sources directory, create ``storage/sphinx`` directory in
   and copy all files in ``mysqlse`` directory from Manticore sources
   there. Example:

.. code-block:: bash


       $ cp -R /root/builds/sphinx-0.9.7/mysqlse /root/builds/mysql-5.1.14/storage/sphinx

2. in MySQL sources directory, run

.. code-block:: bash


       $ sh BUILD/autorun.sh

3. configure MySQL and enable Manticore engine:

.. code-block:: bash


       $ ./configure --with-plugins=sphinx

4. build and install MySQL:

.. code-block:: bash


       $ make
       $ make install

.. _Checking SphinxSE installation:

Checking SphinxSE installation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To check whether SphinxSE has been successfully compiled into MySQL,
launch newly built servers, run mysql client and issue ``SHOW ENGINES``
query. You should see a list of all available engines. Manticore should be
present and “Support” column should contain “YES”:

.. code-block:: mysql


    mysql> show engines;
    +------------+----------+-------------------------------------------------------------+
    | Engine     | Support  | Comment                                                     |
    +------------+----------+-------------------------------------------------------------+
    | MyISAM     | DEFAULT  | Default engine as of MySQL 3.23 with great performance      |
      ...
    | SPHINX     | YES      | Manticore storage engine                                       |
      ...
    +------------+----------+-------------------------------------------------------------+
    13 rows in set (0.00 sec)


Using SphinxSE
^^^^^^^^^^^^^^

To search via SphinxSE, you would need to create special ENGINE=SPHINX
“search table”, and then SELECT from it with full text query put into
WHERE clause for query column.

Let's begin with an example create statement and search query:

.. code-block:: mysql


    CREATE TABLE t1
    (
        id          INTEGER UNSIGNED NOT NULL,
        weight      INTEGER NOT NULL,
        query       VARCHAR(3072) NOT NULL,
        group_id    INTEGER,
        INDEX(query)
    ) ENGINE=SPHINX CONNECTION="sphinx://localhost:9312/test";

    SELECT * FROM t1 WHERE query='test it;mode=any';

First 3 columns of search table *must* have a types of
``INTEGER UNSINGED`` or ``BIGINT`` for the 1st column (document id),
``INTEGER`` or ``BIGINT`` for the 2nd column (match weight), and
``VARCHAR`` or ``TEXT`` for the 3rd column (your query), respectively.
This mapping is fixed; you can not omit any of these three required
columns, or move them around, or change types. Also, query column must
be indexed; all the others must be kept unindexed. Columns' names are
ignored so you can use arbitrary ones.

Additional columns must be either ``INTEGER``, ``TIMESTAMP``,
``BIGINT``, ``VARCHAR``, or ``FLOAT``. They will be bound to attributes
provided in Manticore result set by name, so their names must match
attribute names specified in ``sphinx.conf``. If there's no such
attribute name in Manticore search results, column will have ``NULL``
values.

Special “virtual” attributes names can also be bound to SphinxSE
columns. ``_sph_`` needs to be used instead of ``@`` for that. For
instance, to obtain the values of ``@groupby``, ``@count``, or
``@distinct`` virtual attributes, use ``_sph_groupby``, ``_sph_count``
or ``_sph_distinct`` column names, respectively.

``CONNECTION`` string parameter can be used to specify default searchd
host, port and indexes for queries issued using this table. If no
connection string is specified in ``CREATE TABLE``, index name “\*" (ie.
search all indexes) and localhost:9312 are assumed. Connection string
syntax is as follows:

.. code-block:: none


    CONNECTION="sphinx://HOST:PORT/INDEXNAME"

You can change the default connection string later:

.. code-block:: mysql


    mysql> ALTER TABLE t1 CONNECTION="sphinx://NEWHOST:NEWPORT/NEWINDEXNAME";

You can also override all these parameters per-query.

As seen in example, both query text and search options should be put
into WHERE clause on search query column (ie. 3rd column); the options
are separated by semicolons; and their names from values by equality
sign. Any number of options can be specified. Available options are:

-  query - query text;

-  mode - matching mode. Must be one of “all”, “any”, “phrase”,
   “boolean”, or “extended”. Default is “all”;

-  sort - match sorting mode. Must be one of “relevance”, “attr_desc”,
   “attr_asc”, “time_segments”, or “extended”. In all modes besides
   “relevance” attribute name (or sorting clause for “extended”) is also
   required after a colon:

.. code-block:: none


       ... WHERE query='test;sort=attr_asc:group_id';
       ... WHERE query='test;sort=extended:@weight desc, group_id asc';

-  offset - offset into result set, default is 0;

-  limit - amount of matches to retrieve from result set, default is 20;

-  index - names of the indexes to search:

.. code-block:: mysql


       ... WHERE query='test;index=test1;';
       ... WHERE query='test;index=test1,test2,test3;';

-  minid, maxid - min and max document ID to match;

-  weights - comma-separated list of weights to be assigned to Manticore
   full-text fields:

.. code-block:: mysql


       ... WHERE query='test;weights=1,2,3;';

-  filter, !filter - comma-separated attribute name and a set of values
   to match:

.. code-block:: mysql


       # only include groups 1, 5 and 19
       ... WHERE query='test;filter=group_id,1,5,19;';

       # exclude groups 3 and 11
       ... WHERE query='test;!filter=group_id,3,11;';

-  range, !range - comma-separated (integer or bigint) Manticore attribute
   name, and min and max values to match:

.. code-block:: mysql


       # include groups from 3 to 7, inclusive
       ... WHERE query='test;range=group_id,3,7;';

       # exclude groups from 5 to 25
       ... WHERE query='test;!range=group_id,5,25;';

-  floatrange, !floatrange - comma-separated (floating point) Manticore
   attribute name, and min and max values to match:

.. code-block:: mysql


       # filter by a float size
       ... WHERE query='test;floatrange=size,2,3;';

       # pick all results within 1000 meter from geoanchor
       ... WHERE query='test;floatrange=@geodist,0,1000;';

-  maxmatches - per-query max matches value, as in max_matches
   parameter to :ref:`SetLimits() <set_limits>`
   API call:

.. code-block:: mysql


       ... WHERE query='test;maxmatches=2000;';

-  cutoff - maximum allowed matches, as in cutoff parameter to
   :ref:`SetLimits() <set_limits>` API call:

.. code-block:: mysql


       ... WHERE query='test;cutoff=10000;';

-  maxquerytime - maximum allowed query time (in milliseconds), as in
   :ref:`SetMaxQueryTime() <set_max_query_time>`
   API call:

.. code-block:: mysql


       ... WHERE query='test;maxquerytime=1000;';

-  groupby - group-by function and attribute, corresponding to
   :ref:`SetGroupBy() <set_groupby>` API call:

.. code-block:: mysql


       ... WHERE query='test;groupby=day:published_ts;';
       ... WHERE query='test;groupby=attr:group_id;';

-  groupsort - group-by sorting clause:

.. code-block:: mysql


       ... WHERE query='test;groupsort=@count desc;';

-  distinct - an attribute to compute COUNT(DISTINCT) for when doing
   group-by, as in
   :ref:`SetGroupDistinct() <set_group_distinct>` API
   call:

.. code-block:: mysql


       ... WHERE query='test;groupby=attr:country_id;distinct=site_id';

-  indexweights - comma-separated list of index names and weights to use
   when searching through several indexes:

.. code-block:: mysql


       ... WHERE query='test;indexweights=idx_exact,2,idx_stemmed,1;';

-  fieldweights - comma-separated list of per-field weights that can be
   used by the ranker:

.. code-block:: mysql


       ... WHERE query='test;fieldweights=title,10,abstract,3,content,1;';

-  comment - a string to mark this query in query log (mapping to
   $comment parameter in :ref:`Query() <query>` API call):

.. code-block:: mysql


       ... WHERE query='test;comment=marker001;';

-  select - a string with expressions to compute (mapping to
   :ref:`SetSelect() <set_select>` API call):

.. code-block:: mysql


       ... WHERE query='test;select=2*a+3*** as myexpr;';

-  host, port - remote ``searchd`` host name and TCP port, respectively:

.. code-block:: mysql


       ... WHERE query='test;host=sphinx-test.loc;port=7312;';

-  ranker - a ranking function to use with “extended” matching mode, as
   in
   :ref:`SetRankingMode() <set_ranking_mode>`
   API call (the only mode that supports full query syntax). Known
   values are “proximity_bm25”, “bm25”, “none”, “wordcount”,
   “proximity”, “matchany”, “fieldmask”, “sph04”, “expr:EXPRESSION”
   syntax to support expression-based ranker (where EXPRESSION should be
   replaced with your specific ranking formula), and
   “export:EXPRESSION”:

.. code-block:: none


       ... WHERE query='test;mode=extended;ranker=bm25;';
       ... WHERE query='test;mode=extended;ranker=expr:sum(lcs);';

The “export” ranker works exactly like ranker=expr, but it stores the
   per-document factor values, while ranker=expr discards them after
   computing the final WEIGHT() value. Note that ranker=export is meant
   to be used but rarely, only to train a ML (machine learning) function
   or to define your own ranking function by hand, and never in actual
   production. When using this ranker, you'll probably want to examine
   the output of the RANKFACTORS() function that produces a string with
   all the field level factors for each document.

.. code-block:: mysql


           SELECT *, WEIGHT(), RANKFACTORS()
               FROM myindex
               WHERE MATCH('dog')
               OPTION ranker=export('100*bm25')

   would produce something like

.. code-block:: none


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

-  geoanchor - geodistance anchor, as in
   :ref:`SetGeoAnchor() <set_geo_anchor>`
   API call. Takes 4 parameters which are latitude and longitude
   attribute names, and anchor point coordinates respectively:

.. code-block:: mysql


       ... WHERE query='test;geoanchor=latattr,lonattr,0.123,0.456';

One **very important** note that it is **much** more efficient to allow
Manticore to perform sorting, filtering and slicing the result set than to
raise max matches count and use WHERE, ORDER BY and LIMIT clauses on
MySQL side. This is for two reasons. First, Manticore does a number of
optimizations and performs better than MySQL on these tasks. Second,
less data would need to be packed by searchd, transferred and unpacked
by SphinxSE.

Additional query info besides result set could be retrieved with
``SHOW ENGINE SPHINX STATUS`` statement:

.. code-block:: mysql


    mysql> SHOW ENGINE SPHINX STATUS;
    +--------+-------+-------------------------------------------------+
    | Type   | Name  | Status                                          |
    +--------+-------+-------------------------------------------------+
    | SPHINX | stats | total: 25, total found: 25, time: 126, words: 2 |
    | SPHINX | words | sphinx:591:1256 soft:11076:15945                |
    +--------+-------+-------------------------------------------------+
    2 rows in set (0.00 sec)

This information can also be accessed through status variables. Note
that this method does not require super-user privileges.

.. code-block:: mysql


    mysql> SHOW STATUS LIKE 'sphinx_%';
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

You could perform JOINs on SphinxSE search table and tables using other
engines. Here's an example with “documents” from example.sql:

.. code-block:: mysql


    mysql> SELECT content, date_added FROM test.documents docs
    -> JOIN t1 ON (docs.id=t1.id)
    -> WHERE query="one document;mode=any";
    +-------------------------------------+---------------------+
    | content                             | docdate             |
    +-------------------------------------+---------------------+
    | this is my test document number two | 2006-06-17 14:04:28 |
    | this is my test document number one | 2006-06-17 14:04:28 |
    +-------------------------------------+---------------------+
    2 rows in set (0.00 sec)

    mysql> SHOW ENGINE SPHINX STATUS;
    +--------+-------+---------------------------------------------+
    | Type   | Name  | Status                                      |
    +--------+-------+---------------------------------------------+
    | SPHINX | stats | total: 2, total found: 2, time: 0, words: 2 |
    | SPHINX | words | one:1:2 document:2:2                        |
    +--------+-------+---------------------------------------------+
    2 rows in set (0.00 sec)


Building snippets (excerpts) via MySQL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SphinxSE also includes a UDF function that lets you create snippets
through MySQL. The functionality is fully similar to
:ref:`BuildExcerprts <build_excerpts>` API
call but accessible through MySQL+SphinxSE.

The binary that provides the UDF is named ``sphinx.so`` and should be
automatically built and installed to proper location along with SphinxSE
itself. If it does not get installed automatically for some reason, look
for ``sphinx.so`` in the build directory and copy it to the plugins
directory of your MySQL instance. After that, register the UDF using the
following statement:

.. code-block:: mysql


    CREATE FUNCTION sphinx_snippets RETURNS STRING SONAME 'sphinx.so';

Function name *must* be sphinx_snippets, you can not use an arbitrary
name. Function arguments are as follows:

**Prototype:** function sphinx_snippets ( document, index, words,
[options] );

Document and words arguments can be either strings or table columns.
Options must be specified like this:
``&#039;value&#039; AS option_name``. For a list of supported options,
refer to
:ref:`BuildExcerprts() <build_excerpts>` API
call. The only UDF-specific additional option is named
``&#039;sphinx&#039;`` and lets you specify searchd location (host and
port).

Usage examples:

.. code-block:: mysql


    SELECT sphinx_snippets('hello world doc', 'main', 'world',
        'sphinx://192.168.1.1/' AS sphinx, true AS exact_phrase,
        '[**]' AS before_match, '[/**]' AS after_match)
    FROM documents;

    SELECT title, sphinx_snippets(text, 'index', 'mysql php') AS text
        FROM sphinx, documents
        WHERE query='mysql php' AND sphinx.id=documents.id;

