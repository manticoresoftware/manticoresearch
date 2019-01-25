.. _call_pq_syntax:

CALL PQ syntax
--------------

.. code-block:: mysql


    CALL PQ(index, data[, opt_value AS opt_name[, ...]])


CALL PQ statement performs a prospective search. It returns stored queries from a ``percolate``index that match
documents from provided``data``. For more information, see :ref:`Percolate Query <percolate_query>` section.

``data`` can be a document in plain text, a JSON object containing a document or a list of documents in one of the
two formats. The JSON object can contain pairs of text field names and values as well as attribute names and values.

Example:

.. code-block:: sql

 CALL PQ ('index_name', 'single document', 0 AS docs_json);
 CALL PQ ('index_name', ('first document', 'second document'), 0 AS docs_json );
 CALL PQ ('index_name', '{"title":"single document","content":"Add your content here","category":10,"timestamp":1513725448}');
 CALL PQ ('index_name', (
	                          '{"title":"first document","content":"Add your content here","category":10,"timestamp":1513725448}',
	                          '{"title":"second document","content":"Add more content here","category":20,"timestamp":1513758240}'
	                        )
	 );



A number of options can be set:

-  ``docs`` - 0 (default disabled), provide numbers of matched documents (order or document ids)
-  ``docs_id`` - none (disabled), id alias name, to treat ``JSON`` named field as document id (has sense only with docs=1)
-  ``docs_json`` - 1 ( default enabled), specify if the ``data`` provides document(s) as raw string or encapsulated as JSON object apart alone objects you can also provide array of them, using json array syntax [{...},{...},...].
-  ``mode`` - none (default 'sparsed'), provide a way how to distribute provided docs among members of distributed indexes. Available values are 'sparsed' and 'sharded'. Details described below in :ref:`Distributed PQ modes<distributed_pq_modes>` section.
-  ``query`` - 0 (default disabled), if true returns all information of matched stored query, otherwise it returns just the stored query ID.
-  ``skip_bad_json`` - 0 (default disabled), manages what is to do if json document is broken: either immediately stop
   with error message, either just skip it and continue to process the rest of documents.
-  ``shift`` - 0, provide number which will be added to document ids if no ``docs_id`` fields provided. Has sense mainly to support :ref:`Distributed PQ modes<distributed_pq_modes>`.
-  ``verbose`` - 0 (default disabled), provide extended info in :ref:`SHOW META <percolate_query_show_meta>`


The output of ``CALL PQ``  return the following columns:

* id  - the id of the stored query
* documents -  if ``docs_id`` is not set, it will return the index of the documents as defined at input. If ``docs_id`` is set, the document indexes as replaced with the values of the field defined by ``docs_id``
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
indexes consist from local and remote percolate indexes or their combination.

However, for more effective work you can organize your distributed indexes using two different approaches:

 1. ``Sparsed``. Bunch of docs you attaches to ``CALL PQ`` will be divided to parts according to number of agents, so each of the hosts will take and process only piece of your request. To distinguish between the pieces each agent will also receive param ``shift``.
 2. ``Sharded``. Whole ``CALL PQ`` just broadcasted to all agents, without any initial division.

``Sparsed`` mode is suitable when your set of documents you send to ``call pq`` is quite big, but set of queries stored in pq index is quite small. Knowing that all hosts are mirrors, manticore will divide your set of documents and distribute chunks among mirrors. On finishing it will collect and unite all results and return final queryset as if it is come from one solid index.

Say, you have index ``pq_d2`` which defined in config as

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

And you fire ``CALL PQ`` to this index with couple of docs it will return:

.. code-block:: sql

	mysql> CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
	+------+-----------+
	| id   | documents |
	+------+-----------+
	|    1 | 2         |
	|    2 | 1         |
	+------+-----------+

In ``sparsed`` mode head deamon (one to which you connect and invoke ``CALL PQ``) will divide incoming bunch of docs to agents: '{"title":"angry test", "gid":3 }' will come to the first, and '{"title":"filter test doc2", "gid":13}, 1 as shift' to the second. So, each of agents get only half of all parameters.

They process statement and return result back to the head. If documents don't contain explicitly defined ``docs_id`` field, each agent, in advance, will add value of ``shift`` to calculated docid values.

On return, head daemon merge results and returns them to you. So, you see same result if you invoke ``CALL PQ`` to single local pq index, but actually work was distributed and each node made half of it.

``Sharded`` mode is opposite different. It is suitable when you push relatively small set of documents, but number of stored queries is huge. So, in this case it is more appropriate to store just part of queries on each node, and then unify results returned by applying one and same set of documents. That mode have to be explicitly set, since it implies, first, multiplication of network payload, and also expect different indexes on each of remote agents. Payload multiplication is absolutely useless if your remotes all have one and same index (well, they will answer one and same result, so why send whole set to _each_ of them?).

Note that query mode (sharded or sparsed) is exclusively logical division, it doesn't reflected anyway in the config. You have to select desirable mode when creating and filling indexes by analysing metrics; may be by initial r&d work.

Note that syntax of HA mirrors in the config (when several hosts assigned to one ``agent`` line, | -separated) has nothing about this modes. (so, each ``agent`` always represents ONE host node of dpq, despite the number of HA mirrors mentioned for this agent).