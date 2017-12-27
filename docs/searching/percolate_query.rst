.. _percolate_query:

Percolate query
---------------
.. note::
   This is a new feature, not production ready yet, just for testing purposes mostly for now. Changes will occur in future updates.
   
The percolate query is used to match documents against queries stored in a index. It is also called "search in reverse" as it works opposite to a regular search where documents are stored in an index and queries are issued against the index.

Queries are stored in a special RealTime index and they can be added, deleted and listed using INSERT/DELETE/SELECT statements similar way as it's done for a regular index.

Checking if a document matches any of the predefined criterias (queries) can be done with the ``CALL PQ`` function, which returns a list of the matched queries.
Note that it does not add documents to the percolate index. You need to use another index (regular or RealTime) in which you will insert documents to perform regular searches.

.. _percolate_query_tags:

Tags
~~~~

A percolate query can have ``tags``. ``tags`` can be set for the query with ``INSERT`` statement. Later on a user might list queries with specific ``tags`` with ``SELECT`` statement
or delete query(es) with ``DELETE`` statement.

.. _percolate_query_filters:

Filters
~~~~~~~

A percolate query can have ``filters``. ``filters`` are set for the query with ``INSERT`` statement. Documents can be then filtered according to the ``filters`` with ``CALL PQ`` statement.

.. _percolate_query_index:

Index
~~~~~

A percolate query works only for ``percolate`` index :ref:`type <type>`. Its configuration is similar to :ref:`Real-time index <real-time_indexes>`, 
however the declaration of fields and attributes can be omitted, in this case the index is created with default field ``text`` and attribute ``gid``.

.. code-block:: ini


    index pq
    {
        type = percolate
        path = path/index_name
        min_infix_len   = 4
    }

    
.. _percolate_query_insert:

INSERT
~~~~~~

To store a query the ``INSERT`` statement looks like

.. code-block:: sql


    INSERT INTO index_name (query, tags, filters) VALUES ( 'full text query terms', 'tags', 'filters' );
    INSERT INTO index_name (query) VALUES ( 'full text query terms');
    INSERT INTO index_name VALUES ( 'full text query terms', 'tags');
    INSERT INTO index_name VALUES ( 'full text query terms');

    
where ``tags`` and ``filters`` are optional fields. In case no schema declared for the ``INSERT`` statement te first field will be full-text ``query``
and the optional second field will be ``tags``.
``filters`` is a string and has the same format as ``SphinxQL`` :ref:`WHERE <select_where>` clause.

.. _percolate_query_call:

CALL PQ
~~~~~~~

To search for queries matching a document(s) the ``CALL PQ`` statement is used which looks like

.. code-block:: sql


    CALL PQ ('index_name', 'single document', 0 as docs, 0 as docs_json, 0 as verbose);
    CALL PQ ('index_name', ('multiple documents', 'go this way'), 0 as docs_json );

    
The document in ``CALL PQ`` can be ``JSON`` encoded string or raw string. Fields and attributes mapping are allowed for ``JSON`` documents only.

.. code-block:: sql


    CALL PQ ('pq', (
    '{"title":"header text", "body":"post context", "timestamp":11 }',
    '{"title":"short post", "counter":7 }'
    ) );

    
``CALL PQ`` can have multiple options set as ``option_name``.

Here are default values for the options:

-  docs_json - 1 (enabled), to treat document(s) as ``JSON`` encoded string or raw string otherwise
-  docs - 0 (disabled), to provide per query documents matched at result set
-  verbose - 0 (disabled), to provide extended info on matching at :ref:`SHOW META <percolate_query_show_meta>`
-  query - 0 (disabled), to provide all query fields stored, such as query, tags, filters



.. _percolate_query_list:

List stored queries
~~~~~~~~~~~~~~~~~~~

To list stored queries in index the ``SELECT`` statement looks like

.. code-block:: sql


    SELECT * FROM index_name;
    SELECT * FROM index_name WHERE tags='tags list';
    SELECT * FROM index_name WHERE uid IN (11,35,101);

    
In case ``tags`` provided matching queries will be shown if any ``tags`` from the ``SELECT`` statement match tags in the stored query. In case ``uid`` provided range or
value list filter will be used to filter out stored queries.

The ``SELECT`` supports ``count(*)`` and ``count(*) alias`` to get number of of percolate queries. Any values are just ignored there however ``count(*)``
should provide the total amount of queries stored.

.. code-block:: sql


    mysql> select count(*) c from pq;
    +------+
    | c    |
    +------+
    |    3 |
    +------+

    
.. _percolate_query_delete:

Delete query
~~~~~~~~~~~~

To delete a stored percolate query(es) in index the ``DELETE`` statement looks like

.. code-block:: sql


    DELETE FROM index_name WHERE id=1;
    DELETE FROM index_name WHERE tags='tags list';

    
In case ``tags`` provided the query will be deleted if any ``tags`` from the ``DELETE`` statement match any of its tags.

To delete all stored query(es) in index there is ``TRUNCATE`` statement looks like

.. code-block:: sql

   TRUNCATE RTINDEX index_name;
   

.. _percolate_query_show_meta:

Meta
~~~~

Meta information is kept for documents on "CALL PQ" and can be retrieved with ``SHOW META`` call.

``SHOW META`` output after ``CALL PQ`` looks like

.. code-block:: none


    +-------------------------+-----------+
    | Name                    | Value     |
    +-------------------------+-----------+
    | Total                   | 0.010 sec |
    | Queries matched         | 950       |
    | Document matches        | 1500      |
    | Total queries stored    | 1000      |
    | Term only queries       | 998       |
    +-------------------------+-----------+

    
With entries: 
 
-  Total - total time spent for matching the document(s)
-  Queries matched - how many stored queries match the document(s)
-  Document matches - how many times the documents match the queries stored in the index
-  Total queries stored - how many queries are stored in the index at all
-  Term only queries - how many queries in the index have terms. The rest of the queries have extended query syntax

.. _percolate_query_reconfigure:

Reconfigure
~~~~~~~~~~~

As well as for RealTime indexes ``ALTER RECONFIGURE`` command is also supported for percolate query index. It allows to reconfigure ``percolate`` index on the fly without deleting
and repopulating the index with queries back.

.. code-block:: sql


    mysql> desc pq1;
    +-------+--------+
    | Field | Type   |
    +-------+--------+
    | id    | bigint |
    | text  | field  |
    | body  | field  |
    | k     | uint   |
    +-------+--------+

    mysql> select * from pq1;
    +------+-------+------+-------------+
    | UID  | Query | Tags | Filters     |
    +------+-------+------+-------------+
    |    1 | test  |      |  k=4        |
    |    2 | test  |      |  k IN (4,6) |
    |    3 | test  |      |             |
    +------+-------+------+-------------+

    
Add `JSON` attribute to the index config ``rt_attr_json = json_data``, then issue ``ALTER RECONFIGURE``

.. code-block:: sql


    mysql> desc pq1;
    +-----------+--------+
    | Field     | Type   |
    +-----------+--------+
    | id        | bigint |
    | text      | field  |
    | body      | field  |
    | k         | uint   |
    | json_data | json   |
    +-----------+--------+

    