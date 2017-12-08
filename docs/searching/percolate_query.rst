.. percolate_query:

Percolate query
-----------------------------------

The percolate query can be used to match queries stored in an index. Adding the document to that index will issue match with the stored queries.
By using the percolate query, you are checking the document matches any of the predefined criteria. Common use cases include monitoring and events.

Query got stored with ``INSERT`` statement and matching issued with ``CALL PQ`` statement. Query also might have ``tags`` field that used for query
management along with ``filters`` field to filter out document by attribute values on matching.

.. percolate_query_tags:
Tags
----

Query might have ``tags``. ``tags`` set for query with ``INSERT`` statement. Later user might list query with specific ``tags`` with ``SELECT`` statement
or delete query(es) with ``DELETE`` statement.

.. percolate_query_filters:
Filters
----

Query might have ``filters``. ``filters`` set for query with ``INSERT`` statement. Documents might be filtered with ``filters`` with ``CALL PQ`` statement.

.. percolate_query_index:
Index
-----

Percolate query work only for ``percolate`` index :ref:`type <_type>`. Its configuration is similar to :ref:`Real-time index <_real-time_indexes>`
however declaration of fields and attributes might been omitted, in that case index created with default field ``text`` and attribute ``gid``.

.. code-block:: ini


    index pq
    {
        type = percolate
        path = path/index_name
        min_infix_len   = 4
    }

    
.. percolate_query_insert:
INSERT
------

To store query ``INSERT`` statement looks like

.. code-block:: sql


    INSERT INTO index_name (query, tags, filters) VALUES ( 'full text query terms', 'tags', 'filters' );
    INSERT INTO index_name (query) VALUES ( 'full text query terms');
    INSERT INTO index_name VALUES ( 'full text query terms', 'tags');
    INSERT INTO index_name VALUES ( 'full text query terms');

    
where ``tags`` and ``filters`` are optional fields. In case no schema declared for ``INSERT`` statement 1st field will be full-text ``query``
and optional 2nd field will be ``tags``.
``filters`` is a string and has same format as ``SphinxQL`` :ref:`WHERE <_select_where>` clause.

.. percolate_query_call:
CALL PQ
-------

To issue documents matching ``CALL PQ`` statement looks like

.. code-block:: sql


    CALL PQ ('index_name', 'single document', 0 as docs, 1 as docs_json, 0 as verbose);
    CALL PQ ('index_name', ('multiple documents', 'go this way') );

    
Document at ``CALL PQ`` might be ``JSON`` encoded string or raw string. Fields and attributes mapping allowed at ``JSON`` document only.

.. code-block:: sql


    CALL PQ ('pq', (
    '{"title":"header text", "body":"post context", "timestamp":11 }',
    '{"title":"short post", "counter":7 }'
    ) );

    
``CALL PQ`` might have multiple options set value as ``option_name``.

There is default values for options:

-  docs_json - 1 (enabled), to treat document(s) as ``JSON`` encoded string or raw string otherwise
-  docs - 0 (disabled), to provide per query documents matched at result set
-  verbose - 0 (disabled), to provide extended info on matching at :ref:`SHOW META <percolate_query_show_meta>`



.. percolate_query_list:
List stored queries
-------------------

To list stored queries at index ``SELECT`` statement looks like

.. code-block:: sql


    SELECT * FROM index_name;
    SELECT * FROM index_name WHERE tags='tags list';

    
In case ``tags`` provided query will be shown if any ``tags`` from ``SELECT`` statement found at stored query.
``SELECT`` supports of ``count(*)`` and ``count(*) alias`` to select list of percolate query. Any values just got ignored there however ``count(*)``
should provide total amount of queries stored.

.. code-block:: sql


    mysql> select count(*) c from pq;
    +------+
    | c    |
    +------+
    |    3 |
    +------+

    
.. percolate_query_delete:
Delete query
-------------------

To delete stored query(es) at index ``DELETE`` statement looks like

.. code-block:: sql


    DELETE FROM index_name WHERE id=1;
    DELETE FROM index_name WHERE tags='tags list';

    
In case ``tags`` provided query will be deleted if any ``tags`` from ``DELETE`` statement found at stored query.

.. percolate_query_show_meta:
Meta
----

Meta information keep for documents on matching call and might be retrieved with ``SHOW META`` call.

``SHOW META`` output after ``CALL PQ`` looks like

.. code-block:: sql


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
 
-  Total - document(s) matching total time seconds 
-  Queries matched - how many stored queries matches document(s)
-  Document matches - how many times documents matches stored queries
-  Total queries stored - how many queries stored at index
-  Term only queries - how many queries are with terms. The rest of queries are with extended query syntax

.. percolate_query_reconfigure:
Reconfigure
----

``ALTER RECONFIGURE`` command is also supported for percolate query index. It allows to reconfigure ``percolate`` index on the fly without delete
and repopulate index with queries back.

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

    
Add `JSON` attribute to index config ``rt_attr_json = json_data`` then issue ``ALTER RECONFIGURE``

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

    