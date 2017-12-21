.. _call_pq_syntax:

CALL PQ syntax
--------------------

.. code-block:: mysql


    CALL PQ(data, index[, opt_value AS opt_name[, ...]])
	

CALL PQ statement performs a prospective search. It returns stored queries from a ``percolate``index that match documents from provided``data``. For more information, see :ref:`Percolate Query <percolate_query>` section.

``data`` can be a document in plain text, a JSON object containing a document or a list of documents in one of the two formats. The JSON object can contain pairs of text field names and values as well as attribute names and values.

Example:

.. code-block:: sql
    
	CALL PQ ('index_name', 'single document', 0 as docs_json);
	CALL PQ ('index_name', ('first document', 'second document'), 0 as docs_json );
	CALL PQ ('index_name', '{"title":"single document","content":"Add your content here","category":10,"timestamp":1513725448}');
	CALL PQ ('index_name', (
	                          '{"title":"first document","content":"Add your content here","category":10,"timestamp":1513725448}',
	                          '{"title":"second document","content":"Add more content here","category":20,"timestamp":1513758240}'
	                        )
	 );


	 
A number of options can be set:

-  docs_json - 1 ( default enabled), specify if the ``data`` provides document(s) as raw string or encapsulated as JSON object
-  docs - 0 (default disabled), provide per query documents matched at result set
-  verbose - 0 (default disabled), provide extended info in :ref:`SHOW META <percolate_query_show_meta>`
-  query - 0 (default disabled), if true returns all information of matched stored query, otherwise it returns just the stored query ID


Example:

.. code-block:: sql
    
    MySQL [(none)]> CALL PQ('pq','catch me if you can',0 AS docs_json,1 AS query);
    +------+----------+------+---------+
    | UID  | Query    | Tags | Filters |
    +------+----------+------+---------+
    |    6 | catch me |      |         |
    +------+----------+------+---------+
    1 row in set (0.00 sec)


``CALL PQ`` can be followed by a :ref:`SHOW META <percolate_query_show_meta>` statement which provides additional meta-information about the executed prospective search.