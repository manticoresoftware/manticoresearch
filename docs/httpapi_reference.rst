.. _httpapi_reference:

HTTP API reference
===================

Manticore search daemon supports HTTP protocol and can be accessed with
regular HTTP clients. Available only with ``workers = thread_pool`` (see :ref:`workers`).
To enabled the HTTP protocol, a :ref:`listen` directive with http specified as a protocol needs to be declared:

.. code-block:: ini


    listen = localhost:8080:http

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

       curl -X POST 'http://manticoresearch:9308/search/' 
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


        curl -X POST 'http://manticoresearch:9308/sql/' 
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