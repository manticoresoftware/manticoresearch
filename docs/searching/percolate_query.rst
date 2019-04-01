.. _percolate_query:

Percolate query
---------------
   
The percolate query is used to match documents against queries stored in an index. It is also called "search in reverse" as
it works opposite to a regular search where documents are stored in an index and queries are issued against the index.

Queries are stored in special kind index and they can be added, deleted and listed using INSERT/DELETE/SELECT statements
similar way as it's done for a regular index.

Checking if a document matches any of the predefined criterias (queries) performed via sphinxql :ref:`CALL PQ<call_pq_syntax>`  function, or via http
``/json/pq/<index>/_search`` endpoint. They returns list of matched queries and may be additional info as matching clause, filters, and tags.

The workflow of using percolate queries is the following:

* a percolate index is defined in the configuration
* queries are inserted in the percolate index in same way as documents for a Real-Time index
* documents can be tested against the stored queries with CALL PQ statement

.. _percolate_query_index:

The percolate index
~~~~~~~~~~~~~~~~~~~

A percolate query works only for ``percolate`` index :ref:`type <type>`.  
The percolate index internaly is a modified  Real-Time index and shares a similar configuration. 

.. code-block:: ini

    index pq
    {
        type = percolate
        path = path/index_name
        min_infix_len   = 4
        rt_field = title
        rt_field = body
        rt_attr_uint = author_id
    }



The fields and attributes  declared in the configuration  define the document schema used by percolate queries
and documents that will be tested against stored queries using :ref:`CALL PQ <call_pq_syntax>`  command. The schema can be viewed with :ref:`DESCRIBE table_name TABLE<describe_syntax>` statement.
If the schema is omited, index will imply default field ``text`` and an integer attribute ``gid``.
The default field and attribute are removed  when defining explicit a schema, but their name can be reused if needed.

The stored queries and the tested documents must respect the defined schema in the percolate index configuration. Trying to use fields/attributes not declared in the schema will give results with no matches.

The schema of the percolate index itself which can be viewed with :ref:`DESCRIBE table_name<describe_syntax>`  and which will be used in INSERT statements to add queries in the percolate index  uses a fixed structure 
and contain the following elements:

ID
^^

The "id" is an unsigned 64-bit integer with **autoincrement** functionality therefor it can be ommited in INSERT statements.

.. _percolate_query_query:

Query 
^^^^^

It holds the full text query as the value of a MATCH clause. If per field operators are used inside a query, the full text
fields needs to be declared in the percolate index configuration. If the stored query is supposed to be a full-scan (only
attribute filtering, no full text query), the ``query`` value can be empty or simply omitted.

.. _percolate_query_filters:

Filters
^^^^^^^

Filters is an optional string containing attribute filters and/or expressions in the same way they are defined in the WHERE clause,
like ``gid=10 and pages>4``. The attributes used here needs to be declared in the percolate index configuration.


.. _percolate_query_tags:

Tags
^^^^

Optional, tags represent a list of string labels, separated by comma, which can be used for filtering queries in  ``SELECT``
statements on the percolate index or to delete queries using ``DELETE`` statement.
The tags  can be returned in the CALL PQ result set.

.. _percolate_query_insert:


Index schemas
~~~~~~~~~~~~~


Usual sphinxql command :ref:`DESCRIBE<describe_syntax>` will reveal you both internal (schema for call pq) and external (schema for select).

Store queries
~~~~~~~~~~~~~

Storing queries is done either via usual :ref:`INSERT<insert_and_replace_syntax>` statement, either via http ``json/pq/<idx>/doc`` endpoint.
Read appropriate sections for syntax details and features.


.. _percolate_query_list:

List stored queries
~~~~~~~~~~~~~~~~~~~

To list stored queries either use ``SELECT`` statement, or http ``json/search`` endpoint.
(endpoint ``/json/pq/<index>/search`` is deprecated and will be removed).

From viewpoint of these methods just know that percolate index doesn't contains any full-text fields, so `match()` clause will not work. You have just an `id` and few columns to operate:

+---------+--------+
| Field   | Type   |
+---------+--------+
| id      | bigint |
+---------+--------+
| query   | string |
+---------+--------+
| tags    | string |
+---------+--------+
| filters | string |
+---------+--------+

So, you can fire any usual full-scan queries, like

.. code-block:: sql

  SELECT * FROM pq;
  SELECT * FROM pq WHERE tags='tags list';
  SELECT * FROM pq WHERE id IN (11,35,101);
  SELECT * FROM pq WHERE tags ANY ('foo', 'bar');
  SELECT * FROM pq WHERE tags NOT ANY ('foo', 'bar');
  SELECT * FROM pq WHERE tags ALL ('foo', 'bar');
  SELECT * FROM pq WHERE tags NOT ALL ('foo', 'bar');
  SELECT * FROM pq LIMIT 1300, 45;
  SELECT * FROM distributed_pq LIMIT 5;


.. _percolate_query_delete:

Delete queries
~~~~~~~~~~~~~~

To delete a stored percolate query(es) in index either use ``DELETE`` statement, http :ref:`json/delete<http_json_delete>` endpoint.
(also endpoint ``/json/pq/<index>/delete`` works, but avoid to use it)

.. code-block:: sql


    DELETE FROM index_name WHERE id=1;
    DELETE FROM index_name WHERE tags ANY ('tags', 'list');

``TRUNCATE RTINDEX`` statement can also be used to delete all stored queries:

.. code-block:: sql

   TRUNCATE RTINDEX index_name;


.. _percolate_query_call:

Search matching queries
~~~~~~~~~~~~~~~~~~~~~~~

That is main purpose of percolate indexes. You provide one or many documents according to internal schema, defined in config, and percolate index gives you matched queries. It may be done either by :ref:`CALL PQ statement <call_pq_syntax>` in sphinxql, or by using http :ref:`json/pq/pq_index/_search<http_percolate_query_search>` endpoint.


To search for queries matching a document(s) the ``CALL PQ`` statement is used which looks like

.. code-block:: sql


    CALL PQ ('index_name', 'single document', 0 AS docs, 0 AS docs_json, 0 AS verbose);
    CALL PQ ('index_name', ('multiple documents', 'go this way'), 0 AS docs_json );

Or via http

.. code-block:: rest

	POST json/pq/idx_pq_1/_search
	{
		"query":
		{
			"percolate":
			{
				"document" : { "title" : "some text to match" }
			}
		}
	}
    

Searching for matching queries performance is affected by :ref:`dist_threads`.
It transparently works with :ref:`distributed percolate indexes<distributed_percolate_indexes>`.

.. _percolate_query_show_meta:

Meta
~~~~

Meta information is kept for documents on "CALL PQ" and can be retrieved with ``SHOW META`` call.

``SHOW META`` output after ``CALL PQ`` looks like

.. code-block:: sql


    +-------------------------+-----------+
    | Name                    | Value     |
    +-------------------------+-----------+
    | Total                   | 0.010 sec |
    | Queries matched         | 950       |
    | Document matched        | 1500      |
    | Total queries stored    | 1000      |
    | Term only queries       | 998       |
    +-------------------------+-----------+

    
With entries: 
 
 -  Total - total time spent for matching the document(s)
 -  Queries matched - how many stored queries match the document(s)
 -  Document matches - how many times the documents match the queries stored in the index
 -  Total queries stored - how many queries are stored in the index at all
 -  Term only queries - how many queries in the index have terms. The rest of the queries have extended query syntax

If you used option ``verbose`` when invoking ``CALL PQ``, output will be more detailed:

.. code-block:: sql

    +-------------------------+-----------+
    | Name                    | Value     |
    +-------------------------+-----------+
    | Total                   | 0.000 sec |
    | Setup                   | 0.000 sec |
    | Queries matched         | 2         |
    | Queries failed          | 0         |
    | Document matched        | 2         |
    | Total queries stored    | 5         |
    | Term only queries       | 5         |
    | Fast rejected queries   | 3         |
    | Time per query          | 93, 30    |
    | Time of matched queries | 123       |
    +-------------------------+-----------+

Here you see additional entries:
 - Setup - time spent to initial setup of matching process - parsing docs, setting options, etc.
 - Queries failed - number of failed queries
 - Fast rejected queries - num of queries which wasn't fall into full routine, but quickly matched and rejected with filters or other conditions
 - Time per query - detailed times per each query
 - Time of matched queries - total time spend to matched queries


.. _percolate_query_reconfigure:

Reconfigure
~~~~~~~~~~~

As well as for RealTime indexes ``ALTER RECONFIGURE`` command is also supported for percolate query index. It allows to reconfigure ``percolate`` index on the fly without deleting
and repopulating the index with queries back.

.. code-block:: sql


    mysql> DESC pq1;
    +-------+--------+
    | Field | Type   |
    +-------+--------+
    | id    | bigint |
    | text  | field  |
    | body  | field  |
    | k     | uint   |
    +-------+--------+

    mysql> SELECT * FROM pq1;
    +------+-------+------+-------------+
    | UID  | Query | Tags | Filters     |
    +------+-------+------+-------------+
    |    1 | test  |      |  k=4        |
    |    2 | test  |      |  k IN (4,6) |
    |    3 | test  |      |             |
    +------+-------+------+-------------+

    
Add `JSON` attribute to the index config ``rt_attr_json = json_data``, then issue ``ALTER RECONFIGURE``

.. code-block:: sql

    mysql> ALTER RTINDEX pq1 RECONFIGURE;

    mysql> DESC pq1;
    +-----------+--------+
    | Field     | Type   |
    +-----------+--------+
    | id        | bigint |
    | text      | field  |
    | body      | field  |
    | k         | uint   |
    | json_data | json   |
    +-----------+--------+


.. _distributed_percolate_indexes:

Distributed indexes made from percolate locals and/or agents (DPQ indexes)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can construct a distributed index from several percolate indexes.
The syntax is absolutely the same as for other distributed indexes. It can include several :ref:`local<local>` indexes as well as several :ref:`agents<agent>`. For ``local`` the only noticeable difference is that since percolate indexes don't know about kill-lists there's no difference in which order they're mentioned in a distributed index definition.

For DPQ the operations of listing stored queries and searching through them (``CALL PQ``) are transparent and works as if all the indexes were one solid local index. However data manipulation statements such as ``insert``, ``replace``, ``truncate`` are not available.

If you mention a non-pq index among the agents, the behaviour will be undefined. Most likely in case if the erroneous agent has the same schema as the outer schema of the pq index (id, query, tags, filters) - it will not trigger an error when listing stored PQ rules hence may pollute the list of actual PQ rules stored in PQ indexes with it's own non-pq strings, so be aware of the confusion! 'CALL PQ' to such wrong agent will definitely trigger an error.
