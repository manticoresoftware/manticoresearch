A guide on searching
--------------------


There is no difference between RealTime or plain indexes in terms of how you run queries.

The recommended and simplest way to query Manticore is to use the SphinxQL interface. You can access it with any MySQL client or library, just do

.. code-block:: none
    
    
	$ mysql -P9306 -h0
	
While it implements the MySQL protocol, SphinxQL is not 100% compatible with MySQL syntax. There are specific extensions, like MATCH clause [the most powerful thing in Manticore] or WITHIN GROUP BY and many functions available in MySQL are not implemented (or they are dummy just to allow compatibility with MySQL connector e.g.) or JOINs between indexes which are not supported yet.

Running queries
~~~~~~~~~~~~~~~
In the :ref:`guide of the indexes<guide_indexes>` we already saw an example of a search. In addition to the fulltext match, you can also have attribute filtering, grouping and sorting by attributes or expressions.

.. code-block:: mysql

   mysql> SELECT *,weight() FROM myrtindex  WHERE MATCH('text') AND gid>20 ORDER BY gid ASC,WEIGHT() DESC; SHOW META;
   +------+------+----------+
   | id   | gid  | weight() |
   +------+------+----------+
   |    3 |   22 |     2230 |
   |    2 |   22 |     1304 |
   |    4 |   33 |     2192 |
   +------+------+----------+
   3 rows in set (0.00 sec)
   
   +---------------+-------+
   | Variable_name | Value |
   +---------------+-------+
   | total         | 3     |
   | total_found   | 3     |
   | time          | 0.000 |
   | keyword[0]    | text  |
   | docs[0]       | 4     |
   | hits[0]       | 7     |
   +---------------+-------+
   6 rows in set (0.00 sec)
   
Here we also added a SHOW META command (you can run it in another call, but must be on same session to give information from the query you've just executed). For general usage, total_found and time are most useful.

Manticore supports LIMIT clause like traditional databases in the format LIMIT [offset,] row_count. If no LIMIT is set, the first 20 rows of the result set are returned.

Another non-standard clause is OPTION, which can be used to set various settings for the query.   


Fulltext Matching
~~~~~~~~~~~~~~~~~
By default, operator AND is used if multiple keywords are specified. The keywords are searched over all fulltext fields and unless there are other rules, a match is valid when the keywords are found in any of the fulltext fields.

So for example ‘search for something’ will give you a match on a document where ‘search’ and ‘for’ are find in ‘title’ field and ‘something’ in ‘content’ field.

Restricting the search to certain field(s) can be done with @ operator followed by the name of the field(s), for example `@title search for something`.

Most operators use keyword position relative to document and will give a positive match only if the keywords are found in same field, like proximity, phrase, fied-start/end,NEAR, strict order etc.

There are operators for which the keyword position has no influence, like boost operator, exact form modifier or qourum.

Ranking fulltext matches
~~~~~~~~~~~~~~~~~~~~~~~~

Manticore offers a powerful way to construct scoring formulas for the fulltext match.

There are several building rankers (predefined scoring formulas) with default been proximity_bm25 and custom expressions can be made using the 20+ ranking factors and attributes values if needed.

The most important factors are

BM25 - an industry retrieval function that ranks the document based on the query terms appearances, it’s a per document factor
IDF - inverse document frequency, a numeric statistic that reflect how important a word is to a document in the collection, it is used per field. The IDF values can be used by several ways (as sum, max etc.)
LCS - longest common subsequence, in broad terms it gives the proximity (based on keyword positions). Beside the classic ‘lcs’, several derivates are available too.
In addition to those, you can use counters on hits or words, boolean factors like exact hit or exact order and document attributes can be used too inside expressions.

Several pre-built ranker expressions are available: proximity_bm25, bm25, none, wordcount, proximity, matchany, sph04, expr (custom rankers) and export (same as expr, but stores for output the factor values). They can be changed using the OPTION statement, for example OPTION ranker=bm25.

The default proximity_bm25 can be written as custom ranker as OPTION ranker=expr('sum(lcs*user_weight)+bm25').

The user_weight relates to the boost per field, by default all fields are treated equal. For example if you have fields ‘title’ and ‘content’ you might want to give a boost to ‘title’ matching so you would set OPTION field_weights=(title=10, content=1).

The ranking score is relative to the query itself as long as it includes metrics that calculate distances between keywords or keywords/document frequencies. In these cases, the values of the score can differ a lot from query to query, so doing any kind of comparison between scores of different queries does not make sense. 

.. code-block:: mysql

   MySQL [(none)]>  SELECT *,weight() FROM myrtindex  WHERE MATCH('"more this  text"/2') OPTION ranker=proximity_bm25;
   +------+------+----------+
   | id   | gid  | weight() |
   +------+------+----------+
   |    3 |   22 |     4403 |
   |    4 |   33 |     3378 |
   |    2 |   22 |     2453 |
   |    1 |   11 |     2415 |
   +------+------+----------+
   4 rows in set (0.00 sec)
   .. code-block:: none
       
   MySQL [(none)]> SELECT *,weight() FROM myrtindex  WHERE MATCH('"more this  text"/2') OPTION ranker=none;
   +------+------+----------+
   | id   | gid  | weight() |
   +------+------+----------+
   |    1 |   11 |        1 |
   |    2 |   22 |        1 |
   |    3 |   22 |        1 |
   |    4 |   33 |        1 |
   +------+------+----------+
   4 rows in set (0.00 sec)
   .. code-block:: none
       
   MySQL [(none)]> SELECT *,weight() FROM myrtindex  WHERE MATCH('"more this  text"/2') OPTION ranker=expr('sum(1)+gid');
   +------+------+----------+
   | id   | gid  | weight() |
   +------+------+----------+
   |    4 |   33 |       35 |
   |    2 |   22 |       24 |
   |    3 |   22 |       24 |
   |    1 |   11 |       13 |
   +------+------+----------+
   4 rows in set (0.00 sec)



Data tokenization
~~~~~~~~~~~~~~~~~

Search engines don't store text as it is. Instead they extract words and create several structures that allows fast full-text searching. From the found words, a dictionary is build, which allows a quick look to discover if the word is present or not in the index. In addition, other structures records the documents and fields in which the word was found (as well as position of it inside a field). All these are used when a full-text match is performed.

The process of demarcating and classifying words is called tokenization. The tokenization is applied at both indexing and searching and it operates at character and word level. On the character level, the engine allows only certain characters to pass, this is defined by the charset_table, anything else is replaced with a whitespace (which is considered the default word separator). The charset_table also allows mappings, for example lowercasing or simply replacing one character with another. Beside this, characters can be ignored, blended, defined as a phrase boundary.
At the word level, the base setting is the min_word_len which defines the minimum word length in characters to be accepted in the index. A common request is to match singular with plural forms of words. For this, morphology processors can be used. Going further, we might want a word to be matched as another one - because they are synonyms. For this, the wordforms feature can be used, which allows one or more words to be mapped to another one. 
Very common words can have some unwanted effects on searching, mostly because of their frequency they require lots of computing to process their doc/hit lists. They can be blacklisted with the stopwords features. This helps not only on speeding queries, but also on decreasing index size. A more advanced blacklisting is bigrams, which allows creating a special token between a 'bigram' (common) word and an uncommon word. This can speed up several times  when common words are used in phrase searches.
In case of indexing HTML content, it's desired to not index also the HTML tags, as they can introduce a lot of 'noise' in the index. HTML stripping can be used and can be configured to strip, but index certain tag attributes or completely ignore content of certain HTML elements.

Another common text search type is wildcard searching. Wildcard searching is performed at dictionary level. By default, both plain and RT indexes use a dictionary type called `keywords`. 
In this mode words are stored as they are, so the size of the index is not affected by enabling wildcarding. When a wildcard search is performed, in the dictionary a lookup is made to find all possible expansions of the wildcarded word. This expansion can be problematic in terms of computation at query time in cases where the expanded word can provide lot of expansions or expansions that have huge hitlists. The penalties are higher in case of infixes, where wildcard is added at the start and end of the words. Even more, usage the `expand_keywords` setting, which can apply automatically the stars to the input search terms,  should be made with care.

The plain index also supports a `crc`  dictionary type. With this type, words are not stored as they are, instead a control sum value of words is used. Since it would not be possible to do substring search on the CRCs, instead all possible substrings of the words (defined by min_prefix_len or min_infix_len) are also stored. This approach has 2 issues: one is the possibility of collision for the CRC and second, in case wildcarding is enabled the size of index can explode due to the addition of the substrings. The advantage comes at querying time, since the substrings are already present in the index, there is no need for the expansion the `keywords` dictionary type requires. 
However, because if it's disadvantages, it's considered deprecated and should be used only if `keywords` type is not an option.

Multi-threaded searching
~~~~~~~~~~~~~~~~~~~~~~~~

One index may not be enough. When searching, only one search thread (that uses a cpu core) is used for a query. 

Because of the size of the data or heavy computing queries, we would want to use more than a CPU core per query.

To do that, we need to split the index into several smaller indexes. One common way to split the data is to perform a modulo filtering on the document id 
(like ``sql_query =  SELECT * FROM mytable where id % 4 = 0 [1,2,3]``).

Having several indexes instead of one means now we can run multiple indexing operations in parallel. 

Faster indexing comes with a cost: several CPU cores will be used instead of one, there is more pressure on the source (especially if you rebuild all the indexes at once) and multiple threads writing to disk can overload your storage ( you can limit the impact of IO on storage with  :ref:`max_iops` and :ref:`max_iosize` directives).

Searching over these shards can be done in 2 ways:

* one is to simply enumerate them in the query, like `SELECT * FROM index0,index1,index2,index3`. dist_threads >1 can be used for multi-core processing.
* using a local distributed index and dist_threads > 1 (for multi-core processing). 


Grouping and faceting
~~~~~~~~~~~~~~~~~~~~~

Manticore Search supports grouping by multiple columns or computed expressions. Results can be sorted inside a group with WITHIN GROUP ORDER BY. A particular feature is returning more than one row per group, by using GROUP n BY.
Grouping also supports HAVING clause, GROUP_CONCAT and aggregation functions.
Manticore Search also supports faceting, which in essence is a set of group by applied on the same result set. 

.. code-block:: none

   mysql>  SELECT * FROM myindex WHERE MATCH('some thing') and afilter=1 FACET attr_1 FACET_2 attr_2;
   +------+---------+----------+----------+
   | id   | attr_1  | attr_2   |  afilter |
   +------+------+-------------+----------+
   |    4 |   33    |       35 |        1 |
   ........
   +------+------------+
   | attr_1  count(*)  |
   +------+------------+
   |    4 |   33       |
   ........
   +------+------------+
   | attr_2  count(*)  |
   +------+------------+
   |   10 |   1        |
   
   
In return you get a multiple result set, where the first is the result set of the query and the rest are the facet results.

Functions
~~~~~~~~~~\

GEODIST function can be used to calculate distance between 2 geo coordinates. The result can be used for sorting. 

.. code-block:: none
   
   mysql>  SELECT *, GEODIST(0.65929812494086, -2.1366023996942, latitude, longitude, {in=rad, out=mi,method=adaptive}) AS distance FROM geodemo WHERE distance < 10000 ORDER BY distance ASC LIMIT 0,100;

In addition, polygon calculation can be made, including geo polygon that takes into account Earth's curvature.

.. code-block:: none
   
   mysql>   SELECT *, CONTAINS(GEOPOLY2D(40.95164274496,-76.88583678218,41.188446201688,-73.203723511772,39.900666261352,-74.171833538046,40.059260979044,-76.301076056469),latitude_deg,longitude_deg) AS inside FROM geodemo WHERE inside=1;

Manticore Search also supports math, date and aggregation functions which are documented at :ref:`expressions,_functions,_and_operators`.
Special functions ALL() and ANY() can be used to test elements in an array from a JSON attribute or MVA. 

Highlighting
~~~~~~~~~~~~
Highlighting allows to get a list of fragments from documents (called snippets) which contain the matches. The snippets are used to improve the readability of search results to end users.
Snippeting can be made with the CALL SNIPPETS statement. The function needs the texts that will be highlighted, the index used (for it’s tokenization settings), the query used and optionally a number of settings can be applied to tweak the operation.

.. code-block:: none
   
   mysql>  CALL SNIPPETS('this is my hello world document text I am snippeting now', 'myindex', 'hello world',  1 as query_mode, 5 as limit_words);
   +------------------------------------------------+
   | snippet                                        |
   +------------------------------------------------+
   |  ...  my <b>hello world</b> document text ...  |
   +------------------------------------------------+
   1 row in set (0.00 sec)

Tokenizer tester
~~~~~~~~~~~~~~~~
CALL KEYWORDS provides a way to check how keywords are tokenized or to retrieve the tokenized forms of particular keywords..
 
Beside debug/testing, CALL KEYWORDS can be used for transliteration. For example we can have a template index which maps characters from cyrillic to latin. We can use CALL KEYWORDS to get the latin form of a word written in cyrillic.

.. code-block:: none
   
   mysql>  call keywords ('ran','myindex');
   +------+-----------+------------+
   | qpos | tokenized | normalized |
   +------+-----------+------------+
   | 1    | ran       | run        |
   +------+-----------+------------+
   1 row in set (0.00 sec)

Suggested words
~~~~~~~~~~~~~~~
:ref:`CALL SUGGEST <call_suggest_syntax>` enabled getting suggestions or corrections of a given words. This is useful to implement 'did you mean ...' functionality.

CALL SUGGEST requires an index with full wildcarding (infixing) enabled. Suggestion is based on the index dictionary and uses Levenshtein distance. Several options are available to allow tweaking and the output provide, beside distance, a document count for each word. In case at input there is more than one word, CALL SUGGEST will only process the first word, while CALL QSUGGEST will only process the last word and ignore the rest.

.. code-block:: none

   mysql> call suggest('sarch','myindex');
   +---------+----------+------+
   | suggest | distance | docs |
   +---------+----------+------+
   | search  | 1        | 6071 |
   | arch    | 1        | 20   |
   | march   | 1        | 10   |
   | sarah   | 1        | 4    |
   +---------+----------+------+
   4 rows in set (0.00 sec)

:ref:`Percolate queries<percolate_query>`
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The regular workflow is store index document and match against them a query. However, sometimes it's desired to check if new content matches an existing set of queries. Running the queries over the index each time a document is added can be inefficient. Instead, it would be faster if queries were stored in a index and the new documents are tested against the stored queries. Also called inverse search, this is used for for signaling in monitoring systems or news aggregation.

For this, a special index is used called `percolate`, which is similar with a RealTime index. The queries are stored in a percolate index and :ref:`CALL PQ <call_pq_syntax>` can test one or more documents if they match against the stored queries.

.. code-block:: none
   
   mysql> INSERT INTO index_name VALUES ( 'this is a query');
   mysql> INSERT INTO index_name VALUES ( 'this way');
   mysql> CALL PQ ('index_name', ('multiple documents', 'go this way'), 0 as docs_json );





Search performance
~~~~~~~~~~~~~~~~~~~
To debug and understand why a search is slow, information is provided by commands SHOW PROFILE, SHOW PLAN and SHOW META.

Tokenization and search expression can have a big impact on the search speed. They can generate requesting a lot of data from index components and/or require heavy computation (like merging big lists of hits).
An example is using wildcarding on very short words, like 1-2 characters. 

An index is not fully loaded by default into memory. Only several components are, such as dictionary or attributes (which can be set to not be loaded). The rest will be loaded when queries are made. 

Operating systems will cache read files from the storage. If there is plenty of RAM, an index can be cached enterily as searches are made. If the index is not cached, a slow storage will impact searches. Also, the load time of an index is influenced by how fast components can be loaded into RAM. For small indexes this is not a problem, but in case of huge indexes it can take minutes until an index is ready for searches. 

Queries can also be CPU-bound. This is because index is too big or it's settings or search perform heavy computation. If an index grows big, it should be split to allow multi-core searching as explained in :ref:`previous guide <guide_indexes>`.

If we talk about big data, one server may not be enough and we need to spread our indexes over more than one server. Servers should be as close as possible (at least same data center), as the network latencies between master and nodes will affect the query performance. 
