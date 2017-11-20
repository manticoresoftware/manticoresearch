.. _http_json_insert:

json/insert
-----------

.. warning::
   This feature is in preview stage. MVA and JSON attributes are not handled yet. 

Documents can be inserted into RT indexes using the ``/json/insert`` endpoint. As with SphinxQL's :ref:`insert_and_replace_syntax`, documents with ids that already exist will not be overwritten. You can also use the ``/json/create`` endpoint, it's a synonym for ``json/insert``.

Here's how you can index a simple document:

::

	{
	  "_index":"test",
	  "_id":1
	}

This creates a document with an id specified by ``_id`` in an index specified by the ``_index`` property. This document has empty fulltext fields and all attributes are set to their default values. However, you can use the optional ``doc`` property to set field and attribute values:

::

	{
	  "_index":"test",
	  "_id":1,
	  "doc":
	  {
	    "gid" : 10,
	    "content" : "new document"
	  }
	}

The daemon will respond with a JSON object stating if the operation was successfull or not:

::
   
   {
    "_index": "test",
    "_id": 1,
    "created": true,
    "result": "created"
  }
  
  
