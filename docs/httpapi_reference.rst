.. _httpapi_reference:

HTTP API reference
==================

Manticore search daemon supports HTTP protocol and can be accessed with
regular HTTP clients. Available only with ``workers = thread_pool`` (see :ref:`workers`).
To enabled the HTTP protocol, a :ref:`listen` directive with http specified as a protocol needs to be declared:

.. code-block:: ini


    listen = localhost:9308:http

Supported endpoints:

/search API
-----------

Allows a simple full-text search, parameters can be : 
* index   (index or list of indexes)
* match (equivalent of MATCH()) 
* select (as SELECT clause)
* group (grouping attribute)
* order (SQL-like sorting)
* limit (equivalent of LIMIT 0,N) 

Response is a JSON document containing an array of attrs,matches and meta similar with the SphinxAPI response.

.. code-block:: bash

       curl -X POST 'http://manticoresearch:9308/search'
       -d 'index=forum&match=@subject php manticore&select=id,subject,author_id&limit=5'

.. code-block:: json
       
	{
	   "attrs":[
		  "forum_id",
		  "author_id",
		  "subject",
		  "id"
	   ],
	   "matches":[

	   ],
	   "meta":{
		  "total":0,
		  "total_found":0,
		  "time":0.000,
		  "words":[
			 {
				"word":"php",
				"docs":3252,
				"hits":11166
			 },
			 {
				"word":"manticore",
				"docs":0,
				"hits":0
			 }
		  ]
	   }
	}
	

/sql API
--------

Allows running a SELECT SphinxQL, set as query parameter. 

Response is a JSON document containing an array of attrs,matches and meta similar with the SphinxAPI response.

.. code-block:: bash


        curl -X POST 'http://manticoresearch:9308/sql'
       -d "query=select id,subject,author_id  from forum where match('@subject php manticore') group by
        author_id order by id desc limit 0,5"

.. code-block:: json
  
	{
	   "attrs":[
		  "forum_id",
		  "author_id",
		  "subject",
		  "id",
		  "@groupby",
		  "@count"
	   ],
	   "matches":[

	   ],
	   "meta":{
		  "total":123,
		  "total_found":123,
		  "time":0.087,
		  "words":[
			 {
				"word":"php",
				"docs":3252,
				"hits":11166
			 },
			 {
				"word":"manticore",
				"docs":1242,
				"hits":4352
			 }
		  ]
	   }
	}

For comfortable debugging in browser you can set param 'mode' to 'raw', and then the rest of the query after 'query='
will be passed inside without any substitutions/url decoding.

.. code-block:: bash

        curl -X POST http://manticoresearch:9308/sql -d "query=select id,packedfactors() from test where match('tes*') option ranker=expr('1')"


.. code-block:: json

	{"error":"query missing"}

.. code-block:: bash

		curl -X POST http://localhost:9308/sql -d "mode=raw&query=select id,packedfactors() from test where match('tes*') option ranker=expr('1')"

.. code-block:: json

    {"attrs":["id","packedfactors()"],"matches":[[1,"bm25=616, bm25a=0.696891, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.255958, min_idf=0.255958, max_idf=0.255958, sum_idf=0.255958, min_hit_pos=1, min_best_span_pos=1, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.255958, atc=0.000000), word0=(tf=1, idf=0.255958)"],[2,"bm25=616, bm25a=0.696891, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.255958, min_idf=0.255958, max_idf=0.255958, sum_idf=0.255958, min_hit_pos=1, min_best_span_pos=1, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.255958, atc=0.000000), word0=(tf=1, idf=0.255958)"],[8,"bm25=616, bm25a=0.696891, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.255958, min_idf=0.255958, max_idf=0.255958, sum_idf=0.255958, min_hit_pos=2, min_best_span_pos=2, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.255958, atc=0.000000), word0=(tf=1, idf=0.255958)"]],"meta":{"total":3, "total_found":3, "time":0.001,"words":[{"word":"tes*", "docs":3, "hits":3}]}}


 
/json API
---------

This endpoint expects request body with queries defined as JSON document. Responds with JSON documents containing result and/or information about executed query.

.. warning::
   Please note that this endpoint is in preview stage. Some functionalities are not yet complete and syntax may suffer changes in future.  
   Read careful changelog of future updates to avoid possible breakages.


.. toctree::
	:maxdepth: 1
	:glob:
	
	http_reference/*