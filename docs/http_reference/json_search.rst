.. _http_json_search:

json/search 
-----------

Searches are accepted at ``/json/search/`` endpoint. Here’s an example of a simple query:

::

    curl -X POST 'http://manticoresearch:9308/json/search/' 
	-d '{"index":"test","query":{"match":{"title":"keyword"}}'

``"index"`` clause sets the list of indexes to search through. 
You can specify a single index: ``"index": "test"``, 
a comma-separated list of indexes: ``"index": "test1,test2,test3"``
or use ``_all`` or ``*`` to issue the query to all available indexes:

::

    "index": "_all"
    "index": "*"

``"query"`` clause contains fulltext queries (if any) and filters. It
can be used to organize queries and filters into a tree (using the bool
query).

Fulltext queries
""""""""""""""""

The following fulltext queries are supported:

``match``


``"match"`` is a simple query that matches the specified keywords in the
specified fields

::

    "query":
    {
      "match": { "field": "keyword" }
    }

Just as in case of indexes, you can specify a list of fields:

::

    "match":
    {
      "field1,field2": "keyword"
    }

Or you can use ``"_all"`` or ``"*"`` to search all fields.

You can search all fields except one using ``"!field"``:

::

    "match":
    {
      "!field1": "keyword"
    }

By default keywords are combined using the OR operator. However, you can change that behaviour using the ``"operator"`` clause:

::

    "query":
    {
      "match":
      {
        "content,title":
        {
          "query":"keyword",
          "operator":"or"
        }
      }
    }

``"operator"`` can be set to ``"or"`` or ``"and"``.

``match_phrase``


``"match_phrase"`` is a query that matches the entire phrase. It is similar to a phrase operator in SphinxQL. Here’s an example:

::

    "query":
    {
      "match_phrase": { "_all" : "had grown quite" }
    }

``match_all``


``"match_all"`` is a query that matches all documents. The syntax looks
like this:

::

    "query":
    {
      "match_all": {}
    }

It can be used to create fullscan queries. However, it is not required as you can just specify the filters without a fulltext query.

Bool queries
""""""""""""""""

A bool query matches documents matching boolean combinations of other queries and/or filters. Queries and filters must be specified in
``"must"``, ``"should"`` or ``"must_not"`` sections. Example:

.. code-block:: json

    {
      "index":"test",
      "query":
      {
        "bool":
        {
          "must":
          [
            { "match": {"_all":"keyword"} },
            { "range": { "int_col": { "gte": 14 } } }
          ]
        }
      }
    }

``"must"``


Queries and filters specified in the ``"must"`` section must match the documents. If several fulltext queries or filters are specified, all of them. This is the equivalent of ``AND`` queries in SphinxQL.

``"should"``


Queries and filters specified in the ``"should"`` section should match the documents. If some queries are specified in ``"must"`` or
``"must_not"``, ``"should"`` queries are ignored. On the other hand, if there are no queries other than ``"should"``, then at least one of these queries must match a document for it to match the bool query. This is the equivalent of ``OR`` queries.

``"must_not"``


Queries and filters specified in the ``"must_not"`` section must not match the documents. If several queries are specified under
``"must_not"``, the document matches if none of them match.

Example:

.. code-block:: json

    {
      "index": "test1",
      "query":
      {
        "bool":
        {
          "must":
          {
            "match" : { "_all" : "product" }
          },
          "must_not":
          [
            { "match": {"_all":"phone"} },
            { "range": { "price": { "gte": 500 } } }
          ]
        }
      }
    }

Filters
""""""""""""""""

JSON queries have two distinct entities: fulltext queries and filters. Both can be organised in a tree (using a bool query), but for now
filters work only for the root element of the query. For example:

.. code-block:: json

    {
      "index":"test",
      "query": { "range": { "price": { "lte": 11 } } }
    }

Here’s an example of several filters in a ``bool`` query:

.. code-block:: json

    {
      "index": "test1",
      "query":
      {
        "bool":
        {
          "must":
          [
            { "match" : { "_all" : "product" } },
            { "range": { "price": { "gte": 500, "lte": 1000 } } },
          ],
          "must_not":
          {
            "range": { "revision": { "lt": 15 } }
          }
        }
      }
    }

This is a fulltext query that matches all the documents containing
``product`` in any field. These documents must have a ``price`` greather
or equal than 500 (``"gte"``) and less or equal than 1000 (``"lte"``).
All of these documents must not have a ``revision`` less than 15
(``"lt"``).

The following types of filters are supported: 


**Range filters**


Range filters match documents that have attribute values within a specified range. Example:

::

    "range":
    {
      "price":
      {
        "gte": 500,
        "lte": 1000
      }
    }

Range filters support the following properties:

* ``gte``: value must be greater than or equal to
* ``gt``: value must be greater than
* ``lte``: value must be less than or equal to
* ``lt``: value must be less

**Geo distance filters**


geo\_distance filters are used to filter the documents that are within a specific distance from a geo location.

Example:

.. code-block:: json

    {
      "index":"test",
      "query":
      {
        "geo_distance":
        {
          "location_anchor": {"lat":49, "lon":15},
          "location_source": {"attr_lat, attr_lon"},
          "distance_type": "adaptive",
          "distance":"100 km"
        }
	  }
    }  

-  ``location_anchor``: specifies the pin location, in degrees. Distances are
   calculated from this point.
-  ``location_source``: specifies the attributes that contain latitude
   and longitude.
-  ``distance_type``: specifies distance calculation function. Can be
   either ``adaptive`` or ``haversine``. ``adaptive`` is faster and
   more precise, for more details see :ref:`GEODIST() <expr-func-geodist>`.
   Optional, defaults to ``adaptive``.
-  ``distance``: specifies the maximum distance from the pin locations.
   All documents within this distance match. The distance can be
   specified in various units. If no unit is specified, the distance is
   assumed to be in meters. Here is a list of supported distance units:

   -  Meter: ``m`` or ``meters``
   -  Kilometer: ``km`` or ``kilometers``
   -  Centimeter: ``cm`` or ``centimeters``
   -  Millimeter: ``mm`` or ``millimeters``
   -  Mile: ``mi`` or ``miles``
   -  Yard: ``yd`` or ``yards``
   -  Feet: ``ft`` or ``feet``
   -  Inch: ``in`` or ``inch``
   -  Nautical mile: ``NM``, ``nmi`` or ``nauticalmiles``

``location_anchor`` and ``location_source`` properties accept the
following latitude/longitude formats:

* an object with lat and lon keys: ``{ "lat":"attr_lat", "lon":"attr_lon" }``
* a string of the following structure: ``"attr_lat,attr_lon"``
* an array with the latitude and longitude in the following order: ``[attr_lon, attr_lat]``

Latitude and longitude are specified in degrees.

Sorting
""""""""""""""""

**Sorting by attributes**

Query results can be sorted by one or more attributes. Example:

.. code-block:: json

    {
      "index":"test",
      "query":
      {
        "match": { "title": "what was" }
      },
      "sort": [ "_score", "id" ]
    }

``"sort"`` specifies an array of attibutes and/or additional properties. Each element of the array can be an attribute name or ``"_score"`` if you want to sort by match weights. In that case sort order defaults to ascending for attributes and descending for ``_score``.

You can also specify sort order explicitly. Example:

::

    "sort":
    [
      { "price":"asc" },
      "id"
    ]

-  ``asc``: sort in ascending order
-  ``desc``: sort in descending order

You can also use another syntax and specify sort order via the ``order`` property:

::

    "sort":
    [
      { "gid": { "order":"desc" } }
    ]

Sorting by MVA attributes is also supported in JSON queries. Sorting mode can be set via the ``mode`` property. The following modes are
supported:

* ``min``: sort by minimum value
* ``max``: sort by maximum value

Example:

::

    "sort":
    [
      { "attr_mva": { "order":"desc", "mode":"max" } }
    ]

When sorting on an attribute, match weight (score) calculation is disabled by default (no ranker is used). You can enable weight
calculation by setting the ``track_scores`` property to true:

.. code-block:: json

    {
      "index":"test",
      "track_scores":true,
      "query": { "match": { "title": "what was" } },
      "sort": [ { "gid": { "order":"desc" } } ]
    }

**Sorting by geo distance**


Matches can be sorted by their distance from a specified location.
Example:

.. code-block:: json

    {
      "index": "test",
      "query": { "match_all": {} },
      "sort":
      [
        {
          "_geo_distance":
          {
            "location_anchor": {"lat":41, "lon":32},
            "location_source": [ "attr_lon", "attr_lat" ],
            "distance_type": "adaptive"
          }
        }
      ]
    }

``location_anchor`` property specifies the pin location,
``location_source`` specifies the attributes that contain latitude and
longitude and ``distance_type`` selects distance computation function
(optional, defaults to “arc”).

Expressions
""""""""""""""""

Expressions are supported via ``script_fields``:

.. code-block:: json

    {
      "index": "test",
      "query": { "match_all": {} },
      "script_fields":
      {
        "add_all": { "script": { "inline": "( gid * 10 ) | crc32(title)" } },
        "title_len": { "script": { "inline": "crc32(title)" } }
      }
    }

In this example two expressions are created: ``add_all`` and ``title_len``. First expression calculates ``( gid * 10 ) | crc32(title)`` and stores the result in the ``add_all`` attribute. Second expression calculates ``crc32(title)`` and stores the result in the ``title_len`` attribute.

Only ``inline`` expressions are supported for now. The value of ``inline`` property (the expression to compute) has the same syntax as
SphinxQL expressions.

Text highlighting
"""""""""""""""""

Fulltext query search results can be highlighted on one or more fields. Field contents has to be stored in string attributes (for now). Here’s an example:

.. code-block:: json

    {
      "index": "test",
      "query": { "match": { "content": "and first" } },
      "highlight":
      {
        "fields":
        {
          "content": {},
          "title": {}
        }
      }
    }

As a result of this query, the values of string attributes called ``content`` and ``title`` are highlighted against the query specified in ``query`` clause. Highlighted snippets are added in the ``highlight`` property of the ``hits`` array:

.. code-block:: json

    {
      "took":1,
      "timed_out": false,
      "hits":
      {
        "total": 1,
        "hits":
        [
          {
            "_id": "1",
            "_score": 1625,
            "_source":
            {
              "gid": 1,
              "title": "it was itself in this way",
              "content": "first now and then at"
             },
             "highlight":
             {
               "content": [ "<b>first</b> now <b>and</b> then at" ],
               "title": [ "" ]
             }
          }
        ]
      }
    }

The following options are supported:

* ``fields`` object contains attribute names with options.
* ``encoder`` can be set to ``default`` or ``html``. When set to ``html``, retains html markup when highlighting. Works similar to ``html_strip_mode=retain`` in ``CALL SNIPPETS``.
* ``highlight_query`` makes it possible to highlight against a query other than our search query. Syntax is the same as in the main ``query``:

   .. code-block:: json

    {
      "index": "test",
      "query": { "match": { "content": "and first" } },
      "highlight":
      {
        "fields": { "content": {}, "title": {} },
        "highlight_query": { "match": { "_all":"on and not" } }
      }
    }

*  ``pre_tags`` and ``post_tags`` set opening and closing tags for highlighted text snippets. They work similar to ``before_match`` and
   ``after_match`` options in ``CALL SNIPPETS``. Optional, defaults are ``<b>`` and ``</b>``. Example:

   ::

       "highlight":
       {
         "fields": { "content": {} },
         "pre_tags": "before_",
         "post_tags": "_after"
       }

*  ``no_match_size`` works similar to ``allow_empty`` in ``CALL SNIPPETS``. If set to zero value, acts as ``allow_empty=1``, e.g. allows empty string to be returned as highlighting result when a snippet could not be generated. Otherwise, the beginning of the field will be returned. Optional, default is 0. Example:

   ::

       "highlight":
       {
         "fields": { "content": {} },
         "no_match_size": 0
       }

*  ``order``: if set to ``"score"``, sorts the extracted passages in order of relevance. Optional. Works similar to ``weight_order`` in
   ``CALL SNIPPETS``. Example:

   ::

       "highlight":
       {
         "fields": { "content": {} },
         "order": "score"
       }

*  ``fragment_size`` sets maximum fragment size in symbols. Can be global or per-field. Per-field options override global options.   Optional, default is 256. Works similar to ``limit`` in ``CALL SNIPPETS``. Example of per-field usage:

   ::

       "highlight":
       {
         "fields": { "content": { "fragment_size": 100 } },
       }

   Example of global usage:

   ::

    "highlight":
    {
      "fields": { "content": {} },
      "fragment_size": 100
    }

*  ``number_of_fragments``: Limits the maximum number of fragments in a snippet. Just as ``fragment_size``, can be global or per-field. Optional, default is 0 (no limit). Works similar to ``limit_passages`` in ``CALL SNIPPETS``.

Result set format
""""""""""""""""""

Query result is sent as a JSON document. Example:

.. code-block:: json

    {
      "took":10
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

-  ``took``: time in milliseconds for Elasticsearch to execute the search
-  ``timed_out``: if the query timed out or not
-  ``hits``: search results. has the following properties:

   -  ``total``: total number of matching documents
   -  ``hits``: an array containing matches

Query result can also include query profile information, see `Query profile`_.

Each match in the ``hits`` array has the following properties:

* ``_id``: match id
* ``_score``: match weight, calculated by ranker
* ``_source``: an array containing the attributes of this match. By default all attributes are included. However, this behaviour can be changed, see below

You can use the ``_source`` property to select the fields you want to be
included in the result set. Example:

.. code-block:: json

    {
      "index":"test",
      "_source":"attr*",
      "query": { "match_all": {} }
    }

You can specify the attributes which you want to include in the query result as a string (``"_source": "attr*"``) or as an array of strings (``"_source": [ "attr1", "attri*" ]"``). Each entry can be an attribute name or a wildcard (``*``, ``%`` and ``?`` symbols are supported).

You can also explicitly specify which attributes you want to include and which to exlude from the result set using the ``includes`` and
``excludes`` properties:

::

    "_source":
    {
      "includes": [ "attr1", "attri*" ],
      "excludes": [ "*desc*" ]
    }

An empty list of includes is interpreted as “include all attributes”
while an empty list of excludes does not match anything. If an attribute
matches both the includes and excludes, then the excludes win.

Query profile
""""""""""""""""

You can view the final transformed query tree with all normalized keywords by adding a ``"profile":true`` property:

.. code-block:: json

    {
      "index":"test",
      "profile":true,
      "query":
      {
        "match_phrase": { "_all" : "had grown quite" }
      }
    }

This feature is somewhat similar to ``SHOW PLAN`` statement in SphinxQL. The result appears as a ``profile`` property in the result set. For example:

::

    "profile":
    {
      "query":
      {
        "type": "PHRASE",
        "description": "PHRASE( AND(KEYWORD(had, querypos=1)),  AND(KEYWORD(grown, querypos=2)),  AND(KEYWORD(quite, querypos=3)))",
        "children":
        [
          {
            "type": "AND",
            "description": "AND(KEYWORD(had, querypos=1))",
            "max_field_pos": 0,
            "children":
            [
              {
                "type": "KEYWORD",
                "word": "had",
                "querypos": 1
               }
            ]
          },
          {
            "type": "AND",
            "description": "AND(KEYWORD(grown, querypos=2))",
            "max_field_pos": 0,
            "children":
            [
              {
                "type": "KEYWORD",
                "word": "grown",
                "querypos": 2
              }
            ]
          },
          {
            "type": "AND",
            "description": "AND(KEYWORD(quite, querypos=3))",
            "max_field_pos": 0,
            "children":
            [
              {
                "type": "KEYWORD",
                "word": "quite",
                "querypos": 3
              }
            ]
          }
        ]
      }
    }

``query`` property contains the transformed fulltext query tree. Each node contains:

* ``type``: node type. Can be ``AND``, ``OR``, ``PHRASE``, ``KEYWORD`` etc.
* ``description``: query subtree for this node shown as a string (in ``SHOW PLAN`` format)
* ``children``: child nodes, if any
* ``max_field_pos``: maximum position within a field
* ``word``: transformed keyword. Keyword nodes only. 
* ``querypos``: position of this keyword in a query. Keyword nodes only.
* ``excluded``: keyword excluded from query. Keyword nodes only.
* ``expanded``: keyword added by prefix expansion. Keyword nodes only.
* ``field_start``: keyword must occur at the very start of the field. Keyword nodes only.
* ``field_end``: keyword must occur at the very end of the field. Keyword nodes only.
* ``boost``: keyword IDF will be multiplied by this. Keyword nodes only.


json/insert
-----------

Documents can be inserted into RT indexes using the ``/json/insert`` endpoint. As with SphinxQL's ``INSERT``, documents with ids that already exist will not be overwritten. You can also use the ``/json/create`` endpoint, it's a synonym for ``json/insert``.

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

json/replace
------------

``json/replace`` works similar to SphinxQL's ``REPLACE``. It inserts a new document into an index and if the index already has a document with the same id, it is deleted before the new document is inserted. There's also a synonym endpoint, ``json/index``.


json/update
-----------

This endpoint allows you to update attribute values in documents, same as SphinxQL's ``UPDATE``. Syntax is similar to ``json/insert``, but this time the ``doc`` property is mandatory.

Example:

::

	{
	  "_index":"test",
	  "_id":1,
	  "doc":
	  {
	    "gid" : 100,
	    "price" : 1000
	  }
	}

The id of the document that needs to be updated can be set directly using the ``_id`` property (as in the example above) or
you can do an update by query and apply the update to all the documents that match the query:

::

	{
	  "_index":"test",
	  "doc":
	  {
	    "price" : 1000
	  },

	  "query":
  	  {
	    "match": { "*": "apple" }
	  }
	}

Query syntax is the same as in the ``json/search`` endpoint. Note that you can't specify ``_id`` and ``query`` at the same time.

json/delete
-----------

This endpoint allows you to delete documents from indexes, similar to SphinxQL's ``DELETE``.

Example:

::

	{
	  "_index":"test",
	  "_id":1
	}


This deletes a document that has and id of 1 from an index named ``test``.

As in ``json/update``, you can do a delete by query.

::

	{
	  "_index":"test",

	  "query":
  	  {
	    "match": { "*": "apple" }
	  }
	}

This deletes all documents that match a given query.

json/bulk
---------

The ``json/bulk`` endpoint allows you to perform several insert, update or delete operations in a single call. This endpoint only works with data that has ``Content-Type`` set to ``application/x-ndjson``. The data itself should be formatted as a newline-delimited json (NDJSON). Basically it means that each line should contain exactly one json statement and end with a newline ``\n`` and maybe a ``\r``.

Example:

::

	{ "insert" : { "_index" : "test", "_id" : 1, "doc": { "gid" : 10, "content" : "doc one" } } }
	{ "insert" : { "_index" : "test", "_id" : 2, "doc": { "gid" : 20, "content" : "doc two" } } }

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

	{ "update" : { "_index" : "test", "doc": { "tag" : 1000 }, "query": { "range": { "price": { "gte": 1000 } } } } }
	{ "delete" : { "_index" : "test", "query": { "range": { "price": { "lt": 1000 } } } } }

Note that the bulk operation stops at the first query that results in an error.