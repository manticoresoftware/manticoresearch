.. _http_json_pq:

json/pq 
-------

Percolate are accepted at ``/json/pq`` endpoint. Here is an example:

::

    curl -X POST 'http://manticoresearch:9308//json/pq/index_name/search' 
	-d '{}'

to list of stored queries at ``"index_name"`` percolate index. 


.. _http_percolate_query_add:

Store query
~~~~~~~~~~~

Query might be inserted:

-   with ``ID`` auto generated - at endpoint ``json/pq/index_name/doc``
-   with ``ID`` explicitly set - at endpoint ``json/pq/index_name/doc/ID``

To replace already stored query ``ID`` should be provided along
with ``refresh=1`` argument, such as ``json/pq/index_pq_1/doc/2?refresh=1``

There is 2 formats of full-text queries that might be stored into index:

-   query in ``json\search`` compartible format, described at  :ref:`json/search <http_json_search>`
-   query in ``SpinxQL`` compartible format, described at :ref:`extended query syntax <extended_query_syntax>`

``tags`` and ``filters`` also might be stored along with query, for details refer to :ref:`Tags <percolate_query_tags>`
However there is no way to mix ``json\search`` native filters with ``filters`` field, only one type of filter might be
used per query.


Example of ``json\search`` query with ``tags``:

.. code-block:: rest

	PUT json/pq/idx_pq_1/doc
	{
		"query": { "match": { "title": "test" } },
		"tags": ["fid", "fid_x1"]
	}

Example of ``json\search`` query there terms combined via ``and`` operator:

.. code-block:: rest

	PUT json/pq/idx_pq_1/doc
	{
		"query": { "match": { "title": { "query": "cat test", "operator": "and" } } }
	}

Example of ``json\search`` query with native filters:

.. code-block:: rest

	PUT json/pq/idx_pq_1/doc
	{
		"query":
		{
			"match": { "title": "tree" },
			"range": { "gid": { "lt": 3 } }
		}
	}

Example of ``json\search`` boolean query:

.. code-block:: rest

	PUT json/pq/idx_pq_1/doc
	{
		"query":
		{
			"bool":
			{
				"must": [
					{ "match": { "title": "tree" } },
					{ "match": { "title": "test" } } ]
			}
		}
	}	

Example of ``json\search`` query with ``SphinxQL`` filters and ``ID`` set:

.. code-block:: rest

	PUT json/pq/idx_pq_1/doc/17
	{
		"query":
		{
			"match": { "title": "tree" }
		},
		"filters": "gid < 3 or zip = 049"
	}

Example of ``Sphinx`` query with filters and tags that
repalces already stored query with 2nd ``ID``:

.. code-block:: rest

	PUT json/pq/idx_pq_1/doc/2?refresh=1
	{
		"query":
		{
			"ql": "(test me !he) || (testing place)"
		},
		"filters": "zip IN (1,7,9)",
		"tags": ["zip", "location", "city"]
	}
	
The responce:

.. code-block:: rest

	{
		"index": "idx_pq_1",
		"type": "doc",
		"_id": "2",
		"result": "created"
	}
	
there ``result`` field got value ``created`` for inserted query or value ``updated`` for query
that got successfully replaced.

.. _http_percolate_query_search:

Search matching document
~~~~~~~~~~~~~~~~~~~~~~~~

To search for queries matching document(s) the `_search` endpoint with body should be queried

Example of single document matching:

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

The responce:

.. code-block:: rest

	{
		"timed_out": false,
		"hits": {
			"total": 2,
			"max_score": 1,
			"hits": [
				{
					"_index": "idx_pq_1",
					"_type": "doc",
					"_id": "2",
					"_score": "1",
					"_source": {
						"query": {
							"match": {
								"title": "some"
							},
						}
					}
				},
				{
					"_index": "idx_pq_1",
					"_type": "doc",
					"_id": "5",
					"_score": "1",
					"_source": {
						"query": {
							"ql": "some | none"
						}
					}
				}
			]
		}
	}
	
there queries matched located at ``hits`` array with their ``ID`` at ``_id`` field and full-text part at 
``_source`` field.

	
Example of multiple documents matching:

.. code-block:: rest

	POST json/pq/idx_pq_1/_search
	{
		"query":
		{
			"percolate":
			{
				"documents" : 
				[
					{ "title" : "some text to match" },
					{ "title" : "another text to match" },
					{ "title" : "new document to match" }
				]
			}
		}
	}

The responce:

.. code-block:: rest

	{
		"timed_out": false,
		"hits": {
			"total": 1,
			"max_score": 1,
			"hits": [
			{
				"_index": "idx_pq_1",
				"_type": "doc",
				"_id": "3",
				"_score": "1",
				"_source": {
					"query": {
						"match": {
							"title": "text"
						}
					}
				},
				"fields": {
					"_percolator_document_slot": [
						1,
						2
					]
				}
			} ]
		}
	}
	
there queries matched located at ``hits`` array and documents matched for each query
is located at ``fields`` object ``_percolator_document_slot`` array.

.. _http_percolate_query_list:

List stored queries
~~~~~~~~~~~~~~~~~~~

`_search` endpoint without body shows all stored queries in index, similar to SphinxQL's :ref:`List stored queries <percolate_query_list>`.

Example:

.. code-block:: rest

	POST /json/pq/idx_pq_1/search
	{
	}

	
The responce:

.. code-block:: rest
   
	{
		"timed_out": false,
		"hits": {
			"total": 4,
			"max_score": 1,
			"hits": [
				{
					"_index": "idx_pq_1",
					"_type": "doc",
					"_id": "1",
					"_score": "1",
					"_source": {
						"query": {
							"bool": {
								"must": [
									{
										"match": {
											"title": "tree"
										}
									},
									{
										"match": {
											"title": "test"
										}
									}
								]
							}
						}
					}
				},
				{
					"_index": "idx_pq_1",
					"_type": "doc",
					"_id": "2",
					"_score": "1",
					"_source": {
						"query": {
							"match": {
								"title": "tree"
							},
							"range": {
								"gid": {
									"lt": 3
								}
							}
						}
					}
				},
				{
					"_index": "idx_pq_1",
					"_type": "doc",
					"_id": "4",
					"_score": "1",
					"_source": {
						"query": {
							"ql": "tree !new"
						}
					}
				},
				{
					"_index": "idx_pq_1",
					"_type": "doc",
					"_id": "5",
					"_score": "1",
					"_source": {
						"query": {
							"ql": "new | old"
						}
					}
				}
			]
		}
	}


There ``hits`` contains queries stored at percolate index with query ``ID`` at ``_id`` field and ``_source`` field
is full text query in ``SpinxQL`` compartible format, described at :ref:`extended query syntax <extended_query_syntax>`
or ``json\search`` compartible format, described at  :ref:`json\search <http_json_search>`
	
.. _http_percolate_query_delete:

Delete stored queries
~~~~~~~~~~~~~~~~~~~~~

This endpoint allows to delete queries from index, similar to SphinxQL's :ref:`Delete query <percolate_query_delete>`.
Either id or tags lists supported


Example:

.. code-block:: rest

	DELETE json/pq/idx_pq_1/_delete_by_query
	{
		"id": [2, 10]
	}

	
The daemon will respond with a JSON object stating if the operation was successful or not:

.. code-block:: rest
   
	{
		"timed_out": false,
		"deleted": 2,
		"total": 2,
		"failures": []
	}
  
This deletes 2 documents from an index named ``idx_pq_1``.
