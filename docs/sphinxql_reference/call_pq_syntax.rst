.. _call_pq_syntax:

CALL PQ syntax
--------------

.. code-block:: mysql


    CALL PQ(index, data[, opt_value AS opt_name[, ...]])


CALL PQ statement performs a prospective search. It returns stored queries from a ``percolate``index that match
documents from provided``data``. For more information, see :ref:`Percolate Query <percolate_query>` section.

``data`` can be:

* a document in plain text
* a JSON object containing a document
* a JSON array of JSON documents
* a list of any of the above (JSON and plain text documents cannot be mixed)

The JSON object can contain pairs of text field names and values as well as attribute names and values.

Examples:

.. code-block:: sql

 CALL PQ ('index_name', 'single document', 0 AS docs_json);
 CALL PQ ('index_name', ('first document', 'second document'), 0 AS docs_json );
 CALL PQ ('index_name', '{"title":"single document","content":"Add your content here","category":10,"timestamp":1513725448}');
 CALL PQ ('index_name', (
	                          '{"title":"first document","content":"Add your content here","category":10,"timestamp":1513725448}',
	                          '{"title":"second document","content":"Add more content here","category":20,"timestamp":1513758240}'
	                        )
	 );
 CALL PQ ('pq_sina', '[
	{"title":"first document","content":"Add your content here","category":10,"timestamp":1513725448}, 
	{"title":"second document","content":"Add more content here","category":20,"timestamp":1513758240}
 ]')



A number of options can be set:

-  ``docs`` - 0 (disabled by default), provides numbers of matched documents (in accordance with the order in the CALL PQ or document ids if docs_id is used)
-  ``docs_id`` - none (disabled by default), defines document id alias name from the JSON object to consider that as a document id (makes sense only with docs=1)
-  ``docs_json`` - 1 (enabled by default), specifies if the ``data`` provides document(s) as a raw string or as a JSON object. Besides single objects you can also provide array of them, using json array syntax [{...},{...},...].
-  ``mode`` - none ('sparsed' by default), specifies how to distribute provided docs among members of distributed indexes. Available values are 'sparsed' and 'sharded'. The details are described below in :ref:`Distributed PQ modes<distributed_pq_modes>` section.
-  ``query`` - 0 (disabled by default), if true returns all information of matched stored queries, otherwise it returns just the stored query IDs.
-  ``skip_bad_json`` - 0 (disabled by default), specifies what to do if json document is broken: either immediately stop
   with an error message or just skip it and continue to process the rest of the documents.
- ``skip_empty`` - 0 (disabled by default), specifies how to behave if we met totally empty json, i.e. just null or empty line. By default it is accounted as 'bad json' and so, may be managed by ``skip_bad_json`` as any other kind of errors. Setting it to 1 will silently ignore such documents without any errors and even warnings.
-  ``shift`` - 0 by default, defines the number which will be added to document ids if no ``docs_id`` fields provided. Makes sense mainly to support :ref:`Distributed PQ modes<distributed_pq_modes>`.
-  ``verbose`` - 0 (disabled by default), provides extended info in :ref:`SHOW META <percolate_query_show_meta>`


The output of ``CALL PQ``  return the following columns:

* id  - the id of the stored query
* documents -  if ``docs_id`` is not set, it will return indexes of the documents as defined at input. If ``docs_id`` is set, the document indexes will be replaced with the values of the field defined by ``docs_id``
* query -  the stored full text query
* tags -  the tags attached to the stored query
* filters -  the filters attached to the stored query

Example:

.. code-block:: sql

    mysql> CALL PQ ('pq', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs, 1 AS verbose, 1 AS query);
    +------+-----------+-------------+------+-------------------+
    | id   | documents | query       | tags | filters           |
    +------+-----------+-------------+------+-------------------+
    |    1 | 2         | filter test | bla  | gid>=10           |
    |    2 | 1         | angry       |      | gid>=10 OR gid<=3 |
    +------+-----------+-------------+------+-------------------+
    2 rows in set (0.00 sec)



``CALL PQ`` can be followed by a :ref:`SHOW META <percolate_query_show_meta>` statement which provides additional
meta-information about the executed prospective search.



.. _distributed_pq_modes:

Distributed PQ modes
~~~~~~~~~~~~~~~~~~~~

CALL PQ transparently works with both local percolate indexes (defined in config under type ``percolate``), and distributed
indexes consisting of local and remote percolate indexes or their combination.

However, for more effective work you can organize your distributed indexes using two different approaches:

 1. ``Sparsed``. Batch of documents you pass in ``CALL PQ`` will be split into parts according to the number of agents, so each of the nodes will receive and process only a part of the documents from your request. To distinguish between the parts each agent will also receive param ``shift``.
 2. ``Sharded``. The whole ``CALL PQ`` will be just broadcasted to all agents, without any initial documents split.

``Sparsed`` will be beneficial when your set of documents you send to ``call pq`` is quite big, but the set of queries stored in pq index is quite small. Assuming that all the hosts are mirrors  Manticore will split your set of documents and distribute the chunks among the mirrors. Once the agents are done with the queries it will collect and merge all the results and return final query set as if it comes from one solid index.

Let's assume you have index ``pq_d2`` which is defined in config as

.. code-block:: ini

    index pq_d2
    {
        type = distributed
        agent = 127.0.0.1:6712:pq
        agent = 127.0.0.1:6712:pq1
    }

Each of 'pq' and 'pq1' contains:

.. code-block:: sql

	mysql> SELECY * FROM pq;
	+------+-------------+------+-------------------+
	| id   | query       | tags | filters           |
	+------+-------------+------+-------------------+
	|    1 | filter test |      | gid>=10           |
	|    2 | angry       |      | gid>=10 OR gid<=3 |
	+------+-------------+------+-------------------+
	2 rows in set (0.01 sec)

And you fire ``CALL PQ`` to the distributed index with a couple of docs. It will return:

.. code-block:: sql

	mysql> CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
	+------+-----------+
	| id   | documents |
	+------+-----------+
	|    1 | 2         |
	|    2 | 1         |
	+------+-----------+

In ``sparsed`` mode the head search deamon (the one to which you connect and invoke ``CALL PQ``) will distribute the incoming batch of docs among the agents: '{"title":"angry test", "gid":3 }' will be sent to the first, and '{"title":"filter test doc2", "gid":13}, 1 as shift' to the second. So each of agents gets only half of all the documents.

They then process the statements and return the results back to the head. If the documents don't contain explicitly defined ``docs_id`` field, each agent in advance will add the value of ``shift`` to the calculated docid values.

On return, the head daemon merges results and returns them to you. So you see the same result as if you invoked ``CALL PQ`` to a single local pq index, but actually the work was distributed and each node made half of that.

``Sharded`` mode is beneficial when you push relatively small set of documents, but the number of stored queries is huge. So in this case it is more appropriate to store just part of PQ rules on each node and then merge the results returned from the nodes that process one and the same set of documents against different sets of PQ rules. This mode has to be explicitly set since first of all it implies multiplication of network payload and secondly it expects different indexes in terms of PQ rules in each of the remote agents. The payload multiplication is absolutely useless if your remotes all have one and the same index (well, they will answer one and the same result, so why sending the whole set to _each_ of them?).

Note that the query mode (sharded or sparsed) cannot be specified in the config. You have to choose the desired mode when creating and filling PQ indexes by analysing metrics. Some research may be required to make sure you benefit from either of the modes.

Note that the syntax of HA mirrors in the config (when several hosts are assigned to one ``agent`` line, separated with | ) has nothing to do with the CALL PQ query mode. (so each ``agent`` always represents ONE host node of dpq despite of the number of HA mirrors specified for this agent).
