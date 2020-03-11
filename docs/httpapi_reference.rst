.. _httpapi_reference:

HTTP API reference
==================

Manticore search daemon supports HTTP protocol and can be accessed with
regular HTTP clients. Available only with ``workers = thread_pool`` (see :ref:`workers`).
To enabled the HTTP protocol, a :ref:`listen` directive with http specified as a protocol needs to be declared:

.. code-block:: ini


    listen = localhost:9308:http

Supported endpoints:

.. _http_sql:

/sql API
--------

Allows running a  SphinxQL statement, set as query parameter.

The query payload **must** be URL encoded, otherwise query statements with '=' (filtering or setting options) will result in error.

.. code-block:: bash


        curl -X POST 'http://manticoresearch:9308/sql'
       --data-urlencode "query=select id,subject,author_id  from forum where match('@subject php manticore') group by
        author_id order by id desc limit 0,5"

The response is in JSON format and contains hits information and time of execution. The response share same format as :ref:`/json/search <http_json_search>` endpoint.

.. code-block:: json

    {
      "took":10,
      "timed_out": false,
      "hits":
      {
        "total": 2,
        "hits":
        [
          {
            "_id": "1",
            "_score": 1,
            "_source": { "gid": 11 }
          },
          {
            "_id": "2",
            "_score": 1,
            "_source": { "gid": 12 }
          }
        ]
      }
    }

By default '/sql' allow only SELECT statements. To run any time of SQL statement, the payload must have parameter 'mode' set to 'raw' value with second parameter being the 'query' with the desired statement to be executed.
In this case the statement is passed inside without any substitutions/url decoding.
The response is an equivalent of a SQL result set in JSON format with 'columns' and 'data' objects.


.. code-block:: bash

        curl -X POST http://manticoresearch:9308/sql -d "query=select id,1+2 as a, packedfactors() from test where match('tes*') option ranker=expr('1')"


.. code-block:: json

	{"error":"query missing"}

.. code-block:: bash

		curl -X POST http://localhost:9308/sql -d "mode=raw&query=select id,1+2 as a, packedfactors() from test where match('tes*') option ranker=expr('1')"

.. code-block:: json

		{
			"columns": [
				{
					"id": {
						"type": "long long"
					}
				},
				{
					"a": {
						"type": "long"
					}
				},
				{
					"packedfactors()": {
						"type": "string"
					}
				}
			],
			"data": [
				{
					"id": 1,
					"a": 3,
					"packedfactors()": "bm25=616, bm25a=0.69689077, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.25595802, min_idf=0.25595802, max_idf=0.25595802, sum_idf=0.25595802, min_hit_pos=1, min_best_span_pos=1, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.25595802, atc=0.000000), word0=(tf=1, idf=0.25595802)"
				},
				{
					"id": 8,
					"a": 3,
					"packedfactors()": "bm25=616, bm25a=0.69689077, field_mask=1, doc_word_count=1, field0=(lcs=1, hit_count=1, word_count=1, tf_idf=0.25595802, min_idf=0.25595802, max_idf=0.25595802, sum_idf=0.25595802, min_hit_pos=2, min_best_span_pos=2, exact_hit=0, max_window_hits=1, min_gaps=0, exact_order=1, lccs=1, wlccs=0.25595802, atc=0.000000), word0=(tf=1, idf=0.25595802)"
				}
			],
			"total": 0,
			"error": "",
			"warning": ""
		}
		

All other SphinxQL statements also work.
		
.. code-block:: bash

		curl -X POST http://localhost:9308/sql -d "mode=raw&query=alter table test add column new1 integer"
		curl -X POST http://localhost:9308/sql -d "mode=raw&query=show tables"
		curl -X POST http://localhost:9308/sql -d "mode=raw&query=show status"
		curl -X POST http://localhost:9308/sql -d "mode=raw&query=desc test"


/json API
---------

This endpoint expects request body with queries defined as JSON document. Responds with JSON documents containing result and/or information about executed query.

.. warning::
   Please note that this endpoint may suffer changes in future as more functionality is added. Read careful changelog of future updates to avoid possible breakages.


.. toctree::
	:maxdepth: 1
	:glob:

	http_reference/*

SSL
---

.. warning::
    
    Please note that SSL feature for the HTTP API is in preview stage. Possible issues can appear with some cryptographic algorithms, support will be improved in future releases.

In many cases you might want to encrypt traffic between your client and the daemon.
To do that you can :ref:`specify that the daemon should use HTTPS protocol <listen>` rather than HTTP.

To enable HTTPS at least the following two directives should be set in
:ref:`searchd <searchd_program_configuration_options>` section of
the config and there should be at least one :ref:`listener <listen>` set to ``https``

* :ref:`ssl_cert <ssl_cert>` certificate file
* :ref:`ssl_key <ssl_key>` key file

In addition to that you can specify certificate authority's certificate (aka root certificate) in

* :ref:`ssl_ca <ssl_ca>` certificate authority's certificate file

Example without CA:

.. code-block:: ini


    ssl_cert = server-cert.pem
    ssl_key = server-key.pem


Example with CA:

.. code-block:: ini


    ssl_ca = ca-cert.pem
    ssl_cert = server-cert.pem
    ssl_key = server-key.pem


Generating SSL files
~~~~~~~~~~~~~~~~~~~~

These steps will help you generate the SSL certificates with 'openssl' tool.

Daemon can use Certificate Authority to verify the signature of certificates, but can also work with
just private key and certificate (w/o the CA certificate).


Generate the CA key
"""""""""""""""""""

.. code-block:: bash

    $ openssl genrsa 2048 > ca-key.pem
	
Generate the CA certificate from the CA key
"""""""""""""""""""""""""""""""""""""""""""

Generate self-signed CA (root) certificate from the private key (fill in at least "Common Name"):

.. code-block:: bash

    $ openssl req -new -x509 -nodes -days 365 -key ca-key.pem -out ca-cert.pem



Server Certificate
""""""""""""""""""

Daemon uses the server certificate to secure communication with client.
Generate certificate request and server private key (fill in at least "Common Name" different from the root certificate's common name):

.. code-block:: bash

    $ openssl req -newkey rsa:2048 -days 365 -nodes -keyout server-key.pem -out server-req.pem
    $ openssl rsa -in server-key.pem -out server-key.pem
    $ openssl x509 -req -in server-req.pem -days 365 -CA ca-cert.pem -CAkey ca-key.pem -set_serial 01 -out server-cert.pem

When generation done OpenSSL could be used to verify the key and certificate files generated correctly

.. code-block:: bash

    $ openssl verify -CAfile ca-cert.pem server-cert.pem
