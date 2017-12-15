.. _http_json_bulk:

json/bulk
---------

The ``json/bulk`` endpoint allows you to perform several insert, update or delete operations in a single call. This endpoint only works with data that has ``Content-Type`` set to ``application/x-ndjson``. The data itself should be formatted as a newline-delimited json (NDJSON). Basically it means that each line should contain exactly one json statement and end with a newline ``\n`` and maybe a ``\r``.

Example:

::

	{ "insert" : { "index" : "test", "id" : 1, "doc": { "gid" : 10, "content" : "doc one" } } }
	{ "insert" : { "index" : "test", "id" : 2, "doc": { "gid" : 20, "content" : "doc two" } } }

This inserts two documents to index ``test``. Each statement starts with an action type (in this case, ``insert``). Here's a list of the supported actions:

* ``insert``: Inserts a document. Syntax is the same as in ``json/insert``.
* ``create``: a synonym for ``insert``
* ``replace``: Replaces a document. Syntax is the same as in ``json/replace``.
* ``index``: a synonym for ``replace``
* ``update``: Updates a document. Syntax is the same as in ``json/update``.
* ``delete``: Deletes a document. Syntax is the same as in ``json/delete``.

Updates by query and deletes by query are also supported.

Example:

::

	{ "update" : { "index" : "test", "doc": { "tag" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }
	{ "delete" : { "index" : "test", "query": { "range": { "price": { "lt": 1000 } } } } }

Note that the bulk operation stops at the first query that results in an error.