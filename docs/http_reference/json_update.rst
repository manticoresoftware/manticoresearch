.. _http_json_update:

json/update
-----------

This endpoint allows you to update attribute values in documents, same as SphinxQL's :ref:`update_syntax`. Syntax is similar to ``json/insert``, but this time the ``doc`` property is mandatory.

Example:

::

	{
	  "index":"test",
	  "id":1,
	  "doc":
	  {
	    "gid" : 100,
	    "price" : 1000
	  }
	}

The daemon will respond with a JSON object stating if the operation was successfull or not:

::
   
   {
    "_index": "test",
    "_id": 1,
    "result": "updated"
  }	
  
The id of the document that needs to be updated can be set directly using the ``id`` property (as in the example above) or
you can do an update by query and apply the update to all the documents that match the query:

::

	{
	  "index":"test",
	  "doc":
	  {
	    "price" : 1000
	  },

	  "query":
  	  {
	    "match": { "*": "apple" }
	  }
	}

Query syntax is the same as in the ``json/search`` endpoint. Note that you can't specify ``id`` and ``query`` at the same time.

