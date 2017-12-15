.. _http_json_delete:

json/delete
-----------

This endpoint allows you to delete documents from indexes, similar to SphinxQL's :ref:`delete_syntax`.

Example:

::

	{
	  "index":"test",
	  "id":1
	}

	
The daemon will respond with a JSON object stating if the operation was successfull or not:

::
   
   {
    "_index": "test",
    "_id": 1,
    "found": true,
    "result": "deleted"
  }	
  
This deletes a document that has and id of 1 from an index named ``test``.

As in ``json/update``, you can do a delete by query.

::

	{
	  "index":"test",

	  "query":
  	  {
	    "match": { "*": "apple" }
	  }
	}

This deletes all documents that match a given query.