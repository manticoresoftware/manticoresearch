.. _select_syntax:

SELECT syntax
-------------

::


    SELECT
        select_expr [, select_expr ...]
        FROM index [, index2 ...]
        [WHERE where_condition]
        [GROUP [N] BY {col_name | expr_alias} [, {col_name | expr_alias}]]
        [WITHIN GROUP ORDER BY {col_name | expr_alias} {ASC | DESC}]
        [HAVING having_condition]
        [ORDER BY {col_name | expr_alias} {ASC | DESC} [, ...]]
        [LIMIT [offset,] row_count]
        [OPTION opt_name = opt_value [, ...]]
        [FACET facet_options[ FACET facet_options][ ...]]

**SELECT** statement's syntax is based upon regular SQL but adds
several Manticore-specific extensions and has a few omissions (such as
(currently) missing support for JOINs). Specifically,

-  Column list clause. Column names, arbitrary expressions, and star
   ('\*') are all allowed (ie.
   ``SELECT id, group_id*123+456 AS expr1 FROM test1`` will work).
   Unlike in regular SQL, all computed expressions must be aliased with
   a valid identifier. ``AS`` is optional.

-  EXIST() function is supported. EXIST ( “attr-name”, default-value )
   replaces non-existent columns with default values. It returns either
   a value of an attribute specified by ‘attr-name’, or ‘default-value’
   if that attribute does not exist. It does not support STRING or MVA
   attributes. This function is handy when you are searching through
   several indexes with different schemas.

   ::


       SELECT *, EXIST('gid', 6) as cnd FROM i1, i2 WHERE cnd>5

-  SNIPPET() function is supported. This is a wrapper around the
   snippets functionality, similar to what is available via CALL
   SNIPPETS. The first two arguments are: the text to highlight, and a
   query. It's possible to pass
   :ref:`options <build_excerpts>` to
   function. The intended use is as follows:

   ::


       SELECT id, SNIPPET(myUdf(id), 'my.query', 'limit=100')
       FROM myIndex WHERE MATCH('my.query')

   where myUdf() would be a UDF that fetches a document by its ID from
   some external storage. This enables applications to fetch the entire
   result set directly from Manticore in one query, without having to
   separately fetch the documents in the application and then send them
   back to Manticore for highlighting.

   SNIPPET() is a so-called “post limit” function, meaning that
   computing snippets is postponed not just until the entire final
   result set is ready, but even after the LIMIT clause is applied. For
   example, with a LIMIT 20,10 clause, SNIPPET() will be called at most
   10 times.

   Table functions is a mechanism of post-query result set processing.
   Table functions take an arbitrary result set as their input, and
   return a new, processed set as their output. The first argument
   should be the input result set, but a table function can optionally
   take and handle more arguments. Table functions can completely change
   the result set, including the schema. For now, only built in table
   functions are supported. UDFs are planned when the internal call
   interface is stabilized. Table functions work for both outer SELECT
   and nested SELECT.

   -  REMOVE_REPEATS ( result_set, column, offset, limit ) - removes
      repeated adjusted rows with the same ‘column’ value.

   ::


       SELECT REMOVE_REPEATS((SELECT * FROM dist1), gid, 0, 10)

-  FROM clause. FROM clause should contain the list of indexes to search
   through. Unlike in regular SQL, comma means enumeration of full-text
   indexes as in :ref:`Query() <query>` API call rather than
   JOIN. Index name should be according to the rules of a C identifier.

-  WHERE clause. This clause will map both to fulltext query and
   filters. Comparison operators (=, !=, <, >, <=, >=), IN, AND, NOT,
   and BETWEEN are all supported and map directly to filters. OR is not
   supported yet but will be in the future. MATCH(‘query’) is supported
   and maps to fulltext query. Query will be interpreted according to
   :ref:`full-text query language rules <extended_query_syntax>`.
   There must be at most one MATCH() in the clause.
   ``{col_name | expr_alias} [NOT] IN @uservar`` condition syntax is
   supported. (Refer to :ref:`set_syntax` for a description of global user
   variables.)

-  GROUP BY clause. Supports grouping by multiple columns or computed
   expressions:

   ::


       SELECT *, group_id*1000+article_type AS gkey FROM example GROUP BY gkey
       SELECT id FROM products GROUP BY region, price

   Implicit grouping supported when using aggregate functions without
   specifiying a GROUP BY clause. Consider these two queries:

   ::


       SELECT MAX(id), MIN(id), COUNT(*) FROM books
       SELECT MAX(id), MIN(id), COUNT(*), 1 AS grp FROM books GROUP BY grp

   Aggregate functions (AVG(), MIN(), MAX(), SUM()) in column list
   clause are supported. Arguments to aggregate functions can be either
   plain attributes or arbitrary expressions. COUNT(\*), COUNT(DISTINCT
   attr) are supported. Currently there can be at most one
   COUNT(DISTINCT) per query and an argument needs to be an attribute.
   Both current restrictions on COUNT(DISTINCT) might be lifted in the
   future. A special GROUPBY() function is also supported. It returns
   the GROUP BY key. That is particularly useful when grouping by an MVA
   value, in order to pick the specific value that was used to create
   the current group.

   ::


       SELECT *, AVG(price) AS avgprice, COUNT(DISTINCT storeid), GROUPBY()
       FROM products
       WHERE MATCH('ipod')
       GROUP BY vendorid

   GROUP BY on a string attribute is supported, with respect for current
   collation (see `the section called
   “Collations” <collations>`).

   You can query Manticore to return (no more than) N top matches for each
   group accordingly to WITHIN GROUP ORDER BY.

   ::


       SELECT id FROM products GROUP 3 BY category

   You can sort the result set by (an alias of) the aggregate value.

   ::


       SELECT group_id, MAX(id) AS max_id
       FROM my_index WHERE MATCH('the')
       GROUP BY group_id ORDER BY max_id DESC

-  GROUP_CONCAT() function is supported. When you group by an
   attribute, the result set only shows attributes from a single
   document representing the whole group. GROUP_CONCAT() produces a
   comma-separated list of the attribute values of all documents in the
   group.

   ::


       SELECT id, GROUP_CONCAT(price) as pricesList, GROUPBY() AS name FROM shops GROUP BY shopName;

-  ZONESPANLIST() function returns pairs of matched zone spans. Each
   pair contains the matched zone span identifier, a colon, and the
   order number of the matched zone span. For example, if a document
   reads <emphasis role="bold"><i>text</i> the <i>text</i></emphasis>, and you query for
   ‘ZONESPAN:(i,b) text’, then ZONESPANLIST() will return the string
   “1:1 1:2 2:1” meaning that the first zone span matched “text” in
   spans 1 and 2, and the second zone span in span 1 only.

-  WITHIN GROUP ORDER BY clause. This is a Manticore specific extension
   that lets you control how the best row within a group will to be
   selected. The syntax matches that of regular ORDER BY clause:

   ::


       SELECT *, INTERVAL(posted,NOW()-7*86400,NOW()-86400) AS timeseg, WEIGHT() AS w
       FROM example WHERE MATCH('my search query')
       GROUP BY siteid
       WITHIN GROUP ORDER BY w DESC
       ORDER BY timeseg DESC, w DESC

   WITHIN GROUP ORDER BY on a string attribute is supported, with
   respect for current collation (see :ref:`collations`).

-  HAVING clause. This is used to filter on GROUP BY values. Currently
   supports only one filtering condition.

   ::


       SELECT id FROM plain GROUP BY title HAVING group_id=16;
       SELECT id FROM plain GROUP BY attribute HAVING COUNT(*)>1;

   Because of HAVING is implemented as a whole result set
   post-processing, result set for query with HAVING could be less than
   ``max_matches`` allows.

-  ORDER BY clause. Unlike in regular SQL, only column names (not
   expressions) are allowed and explicit ASC and DESC are required. The
   columns however can be computed expressions:

   ::


       SELECT *, WEIGHT()*10+docboost AS skey FROM example ORDER BY skey

   You can use subqueries to speed up specific searches, which involve
   reranking, by postponing hard (slow) calculations as late as
   possible. For example, SELECT id,a_slow_expression() AS cond FROM
   an_index ORDER BY id ASC, cond DESC LIMIT 100; could be better
   written as SELECT \* FROM (SELECT id,a_slow_expression() AS cond
   FROM an_index ORDER BY id ASC LIMIT 100) ORDER BY cond DESC; because
   in the first case the slow expression would be evaluated for the
   whole set, while in the second one it would be evaluated just for a
   subset of values.

   ORDER BY on a string attribute is supported, with respect for current
   collation (see `the section called
   “Collations” <collations>`).

   ORDER BY RAND() syntax is supported. Note that this syntax is
   actually going to randomize the weight values and then order matches
   by those randomized weights.

-  LIMIT clause. Both LIMIT N and LIMIT M,N forms are supported. Unlike
   in regular SQL (but like in Manticore API), an implicit LIMIT 0,20 is
   present by default.

-  OPTION clause. This is a Manticore specific extension that lets you
   control a number of per-query options. The syntax is:

   ::


       OPTION <optionname>=<value> [ , ... ]

   Supported options and respectively allowed values are:

   -  ``agent_query_timeout`` - integer (max time in milliseconds to
      wait for remote queries to complete, see
      :ref:`agent_query_timeout <agent_query_timeout>`
      under Index configuration options for details)

   -  ``boolean_simplify`` - 0 or 1, enables simplifying the query to
      speed it up

   -  ``comment`` - string, user comment that gets copied to a query log
      file

   -  ``cutoff`` - integer (max found matches threshold)

   -  ``field_weights`` - a named integer list (per-field user weights
      for ranking)

   -  ``global_idf`` - use global statistics (frequencies) from the
      :ref:`global_idf file <global_idf>`
      for IDF computations, rather than the local index statistics.

   -  ``idf`` - a quoted, comma-separated list of IDF computation flags.
      Known flags are:

      -  normalized: BM25 variant, idf = log((N-n+1)/n), as per
         Robertson et al

      -  plain: plain variant, idf = log(N/n), as per Sparck-Jones

      -  tfidf_normalized: additionally divide IDF by query word count,
         so that TF\*IDF fits into [0, 1] range

      -  tfidf_unnormalized: do not additionally divide IDF by query
         word count

      where **N** is the collection size and **n** is the number
      of matched documents.

      The historically default IDF (Inverse Document Frequency) in
      Manticore is equivalent to
      ``OPTION idf=&#039;normalized,tfidf_normalized&#039;``, and those
      normalizations may cause several undesired effects.

      First, idf=normalized causes keyword penalization. For instance,
      if you search for [the \| something] and [the] occurs in more than
      50% of the documents, then documents with both keywords [the] and
      [something] will get **less** weight than documents with just
      one keyword [something]. Using ``OPTION idf=plain`` avoids this.
      Plain IDF varies in [0, log(N)] range, and keywords are never
      penalized; while the normalized IDF varies in [-log(N), log(N)]
      range, and too frequent keywords are penalized.

      Second, idf=tfidf_normalized causes IDF drift over queries.
      Historically, we additionally divided IDF by query keyword count,
      so that the entire sum(tf\*idf) over all keywords would still fit
      into [0,1] range. However, that means that queries [word1] and
      [word1 \| nonmatchingword2] would assign different weights to the
      exactly same result set, because the IDFs for both “word1” and
      “nonmatchingword2” would be divided by 2.
      ``OPTION idf=tfidf_unnormalized`` fixes that. Note that BM25,
      BM25A, BM25F() ranking factors will be scale accordingly once you
      disable this normalization.

      IDF flags can be mixed; ``plain`` and ``normalized`` are mutually
      exclusive; ``tfidf_unnormalized`` and ``tfidf_normalized`` are
      mutually exclusive; and unspecified flags in such a mutually
      exclusive group take their defaults. That means that
      ``OPTION idf=plain`` is equivalent to a complete
      ``OPTION idf=&#039;plain,tfidf_normalized&#039;`` specification.

   -  ``local_df`` - 0 or 1,automatically sum DFs over all the local
      parts of a distributed index, so that the IDF is consistent (and
      precise) over a locally sharded index.

   -  ``index_weights`` - a named integer list (per-index user weights
      for ranking)

   -  ``max_matches`` - integer (per-query max matches value)

      Maximum amount of matches that the daemon keeps in RAM for each
      index and can return to the client. Default is 1000.

      Introduced in order to control and limit RAM usage,
      ``max_matches`` setting defines how much matches will be kept in
      RAM while searching each index. Every match found will still be
      *processed*; but only best N of them will be kept in memory and
      return to the client in the end. Assume that the index contains
      2,000,000 matches for the query. You rarely (if ever) need to
      retrieve *all* of them. Rather, you need to scan all of them, but
      only choose “best” at most, say, 500 by some criteria (ie. sorted
      by relevance, or price, or anything else), and display those 500
      matches to the end user in pages of 20 to 100 matches. And
      tracking only the best 500 matches is much more RAM and CPU
      efficient than keeping all 2,000,000 matches, sorting them, and
      then discarding everything but the first 20 needed to display the
      search results page. ``max_matches`` controls N in that “best N”
      amount.

      This parameter noticeably affects per-query RAM and CPU usage.
      Values of 1,000 to 10,000 are generally fine, but higher limits
      must be used with care. Recklessly raising ``max_matches`` to
      1,000,000 means that ``searchd`` will have to allocate and
      initialize 1-million-entry matches buffer for *every* query. That
      will obviously increase per-query RAM usage, and in some cases can
      also noticeably impact performance.

   -  ``max_query_time`` - integer (max search time threshold, msec)

   -  ``max_predicted_time`` - integer (max predicted search time, see
      :ref:`predicted_time_costs`)

   -  ``ranker`` - any of ``proximity_bm25``, ``bm25``, ``none``, ``wordcount``,
      ``proximity``, ``matchany``, ``fieldmask``, ``sph04``, ``expr``, or ``export``
      (refer to :ref:`search_results_ranking` for more details
      on each ranker)

   -  ``retry_count`` - integer (distributed retries count)

   -  ``retry_delay`` - integer (distributed retry delay, msec)

   -  ``reverse_scan`` - 0 or 1, lets you control the order in which
      full-scan query processes the rows

   -  ``sort_method`` - ``pq`` (priority queue, set by default) or
      ``kbuffer`` (gives faster sorting for already pre-sorted data,
      e.g. index data sorted by id). The result set is in both cases the
      same; picking one option or the other may just improve (or
      worsen!) performance.

   -  ``rand_seed`` - lets you specify a specific integer seed value for
      an ``ORDER BY RAND()`` query, for example: … OPTION
      ``rand_seed=1234``. By default, a new and different seed value is
      autogenerated for every query.

   -  ``low_priority`` - runs the query with idle priority.

   Example:

   ::


       SELECT * FROM test WHERE MATCH('@title hello @body world')
       OPTION ranker=bm25, max_matches=3000,
           field_weights=(title=10, body=3), agent_query_timeout=10000

-  FACET clause. This Manticore specific extension enables faceted search
   with subtree optimization. It is capable of returning multiple result
   sets with a single SQL statement, without the need for complicated
   :ref:`multi-queries <multi-statement_queries>`. FACET clauses
   should be written at the very end of SELECT statements with spaces
   between them.

   ::


       FACET {expr_list} [BY {expr_list}] [ORDER BY {expr | FACET()} {ASC | DESC}] [LIMIT [offset,] count]
       SELECT * FROM test FACET brand_id FACET categories;
       SELECT * FROM test FACET brand_name BY brand_id ORDER BY brand_name ASC FACET property;

   Working example:

   ::


       mysql> SELECT *, IN(brand_id,1,2,3,4) AS b FROM facetdemo WHERE MATCH('Product') AND b=1 LIMIT 0,10
       FACET brand_name, brand_id BY brand_id ORDER BY brand_id ASC
       FACET property ORDER BY COUNT(*) DESC
       FACET INTERVAL(price,200,400,600,800) ORDER BY FACET() ASC
       FACET categories ORDER BY FACET() ASC;
       +------+-------+----------+-------------------+-------------+----------+------------+------+
       | id   | price | brand_id | title             | brand_name  | property | categories | **    |
       +------+-------+----------+-------------------+-------------+----------+------------+------+
       |    1 |   668 |        3 | Product Four Six  | Brand Three | Three    | 11,12,13   |    1 |
       |    2 |   101 |        4 | Product Two Eight | Brand Four  | One      | 12,13,14   |    1 |
       |    8 |   750 |        3 | Product Ten Eight | Brand Three | Five     | 13         |    1 |
       |    9 |    49 |        1 | Product Ten Two   | Brand One   | Three    | 13,14,15   |    1 |
       |   13 |   613 |        1 | Product Six Two   | Brand One   | Eight    | 13         |    1 |
       |   20 |   985 |        2 | Product Two Six   | Brand Two   | Nine     | 10         |    1 |
       |   22 |   501 |        3 | Product Five Two  | Brand Three | Four     | 12,13,14   |    1 |
       |   23 |   765 |        1 | Product Six Seven | Brand One   | Nine     | 11,12      |    1 |
       |   28 |   992 |        1 | Product Six Eight | Brand One   | Two      | 12,13      |    1 |
       |   29 |   259 |        1 | Product Nine Ten  | Brand One   | Five     | 12,13,14   |    1 |
       +------+-------+----------+-------------------+-------------+----------+------------+------+
       +-------------+----------+----------+
       | brand_name  | brand_id | count(*) |
       +-------------+----------+----------+
       | Brand One   |        1 |     1012 |
       | Brand Two   |        2 |     1025 |
       | Brand Three |        3 |      994 |
       | Brand Four  |        4 |      973 |
       +-------------+----------+----------+
       +----------+----------+
       | property | count(*) |
       +----------+----------+
       | One      |      427 |
       | Five     |      420 |
       | Seven    |      420 |
       | Two      |      418 |
       | Three    |      407 |
       | Six      |      401 |
       | Nine     |      396 |
       | Eight    |      387 |
       | Four     |      371 |
       | Ten      |      357 |
       +----------+----------+
       +---------------------------------+----------+
       | interval(price,200,400,600,800) | count(*) |
       +---------------------------------+----------+
       |                               0 |      799 |
       |                               1 |      795 |
       |                               2 |      757 |
       |                               3 |      833 |
       |                               4 |      820 |
       +---------------------------------+----------+
       +------------+----------+
       | categories | count(*) |
       +------------+----------+
       |         10 |      961 |
       |         11 |     1653 |
       |         12 |     1998 |
       |         13 |     2090 |
       |         14 |     1058 |
       |         15 |      347 |
       +------------+----------+

-  subselects, in format ``SELECT * FROM (SELECT … ORDER BY cond1 LIMIT X) ORDER BY cond2 LIMIT Y``. 
   The outer select allows only ORDER BY and
   LIMIT clauses.
   Subselects currently have 2 usage cases:
   
   1. We have a query with 2 ranking UDFs, one very fast and the other one slow and we perform a full-text search will a big match result set. Without subselect the query would look like
   
	::

		SELECT id,slow_rank() as slow,fast_rank() as fast FROM index 
			WHERE MATCH(‘some common query terms’) ORDER BY fast DESC, slow DESC LIMIT 20 
			OPTION max_matches=1000;


	With subselects the query can be rewritten as :
		
	::
	
		SELECT * FROM
			(SELECT id,slow_rank() as slow,fast_rank() as fast FROM index WHERE 
				MATCH(‘some common query terms’)
				ORDER BY fast DESC LIMIT 100 OPTION max_matches=1000)
		ORDER BY slow DESC LIMIT 20;
		
    In the initial query the slow_rank() UDF is computed for the entire match result set. With subselects, only fast_rank() is computed for the entire match result set, while slow_rank() is only computed for a limited set.
	

   2. The second case comes handy for large result set coming from a distributed index.
	
	For this query:
	
	:: 
	
		SELECT * FROM my_dist_index WHERE some_conditions LIMIT 50000;
	
	if we have 20 nodes, each node can send back to master a number of 50K records, resulting in **20 x 50K = 1M records**, however as the master sends back only 50K (out of 1M), it might be good enough for us for the nodes to send only the top 10K records.
	With subselect we can rewrite the query as:
	
	:: 
	
		SELECT * FROM 
			(SELECT * FROM my_dist_index WHERE some_conditions LIMIT 10000) 
		ORDER by some_attr LIMIT 50000;

	In this case, the nodes receive only the inner query and execute. This means the master will receive only *20x10K=200K records*. The master will take all the records received, reorder them by the OUTER clause and return the best 50K records. The  subselect help reducing the traffic between the master and the nodes and also reduce the master's computation time (as it process only 200K instead of 1M).
	