HTTP protocol
-------------

Manticore search daemon supports HTTP protocol and can be accessed with
regular HTTP clients. Available only with ``workers =
thread_pool`` (see :ref:`workers`).
To enabled the HTTP protocol, a
:ref:`listen`
directive with http specified as a protocol needs to be declared:

.. code-block:: ini


    listen = localhost:8080:http

Supported endpoints:

-  / - default response, returns a simple HTML page

-  /search - allows a simple full-text search, parameters can be : index
   (index or list of indexes), match (equivalent of MATCH()), select (as
   SELECT clause), group (grouping attribute), order (SQL-like sorting),
   limit (equivalent of LIMIT 0,N)

.. code-block:: bash

       curl -X POST 'http://manticoresearch:9308/search/' 
       -d 'index=forum&match=@subject php manticore&select=id,subject,author_id&limit=5'

-  / sql - allows running a SELECT SphinxQL, set as query parameter

.. code-block:: bash


        curl -X POST 'http://manticoresearch:9308/sql/' 
       -d 'query=select id,subject,author_id  from forum where match('@subject php manticore') group by
        author_id order by id desc limit 0,5'

The result for /sql/ and /search/ endpoints is an array of attrs,matches
and meta, same as for SphinxAPI, encoded as a JSON object.
