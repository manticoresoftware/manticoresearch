.. _http_protocol:

HTTP protocol
-------------

Manticore search daemon supports HTTP protocol and can be accessed with
regular HTTP clients. Available only with ``workers = thread_pool`` (see :ref:`workers`).
To enabled the HTTP protocol, a :ref:`listen` directive with http specified as a protocol needs to be declared:

.. code-block:: ini


    listen = localhost:8080:http


.. _http_protocol_sphinxql:

SphinxQL queries via HTTP protocol
----------------------------------

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


.. _http_protocol_json:

JSON queries via HTTP protocol
------------------------------

**Table of Contents**

1. `Fulltext queries`_
2. `Bool queries`_
3. `Filters`_
4. `Sorting`_
5. `Expressions`_
6. `Text highlighting`_
7. `Result set format`_
8. `Query profile`_

Manticore allows you to perform searches using queries written as JSON via HTTP protocol. Thus, HTTP protocol must be enabled.

Searches are accepted at ``json/search/`` endpoint. Here’s an example of a simple query:

::

    {
      "index": "test",
      "query":
      {
        "match": { "title": "keyword" }
      }
    }

``"index"`` clause sets the list of indexes to search through. You can specify a single index: ``"index": "test"``

Or a comma-separated list of indexes: ``"index": "test1,test2,test3"``

Or use “\_all" or “\*" to issue the query to all available indexes:

::

    "index": "_all"
    "index": "*"

``"query"`` clause contains fulltext queries (if any) and filters. It
can be used to organize queries and filters into a tree (using the bool
query).

The following fulltext queries are supported:

* `match`_
* `match\_phrase`_ 
* `match\_all`_

match
~~~~~

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

match\_phrase
~~~~~~~~~~~~~

``"match_phrase"`` is a query that matches the entire phrase. It is similar to a phrase operator in SphinxQL. Here’s an example:

::

    "query":
    {
      "match_phrase": { "_all" : "had grown quite" }
    }

match\_all
~~~~~~~~~~

``"match_all"`` is a query that matches all documents. The syntax looks
like this:

::

    "query":
    {
      "match_all": {}
    }

It can be used to create fullscan queries. However, it is not required as you can just specify the filters without a fulltext query.

Bool queries
------------

A bool query matches documents matching boolean combinations of other queries and/or filters. Queries and filters must be specified in
``"must"``, ``"should"`` or ``"must_not"`` sections. Example:

::

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
~~~~~~~~~~

Queries and filters specified in the ``"must"`` section must match the documents. If several fulltext queries or filters are specified, all of them. This is the equivalent of ``AND`` queries in SphinxQL.

``"should"``
~~~~~~~~~~~~

Queries and filters specified in the ``"should"`` section should match the documents. If some queries are specified in ``"must"`` or
``"must_not"``, ``"should"`` queries are ignored. On the other hand, if there are no queries other than ``"should"``, then at least one of these queries must match a document for it to match the bool query. This is the equivalent of ``OR`` queries.

``"must_not"``
~~~~~~~~~~~~~~

Queries and filters specified in the ``"must_not"`` section must not match the documents. If several queries are specified under
``"must_not"``, the document matches if none of them match.

Example:

::

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
-------

JSON queries have two distinct entities: fulltext queries and filters. Both can be organised in a tree (using a bool query), but for now
filters work only for the root element of the query. For example:

::

    {
      "index":"test",
      "query": { "range": { "price": { "lte": 11 } } }
    }

Here’s an example of several filters in a ``bool`` query:

::

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

* `range`_
* `geo\_distance`_

Range filters
~~~~~~~~~~~~~

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

Geo distance filters
~~~~~~~~~~~~~~~~~~~~

geo\_distance filters are used to filter the documents that are within a specific distance from a geo location.

Example:

::

    {
      "index":"test",
      "query":
      {
        "geo_distance":
        {
          "location_anchor": {"lat":49, "lon":15},
          "location_source":"attr_lat, attr_lon",
          "distance_type": "adaptive",
          "distance":100km
        }
    }

-  ``location_anchor``: specifies the pin location. Distances are
   calculated from this point.
-  ``location_source``: specifies the attributes that contain latitude
   and longitude.
-  ``distance_type``: specifies distance calculation function. Can be
   either ``adaptive`` or ``haversine``. ``adaptive`` is faster and
   more precise, for more details see GEODIST() function in SphinxQL.
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
-------

-  `Sorting by attributes`_
-  `Sorting by geo distance`_

Query results can be sorted by one or more attributes. Example:

::

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

::

    {
      "index":"test",
      "track_scores":true,
      "query": { "match": { "title": "what was" } },
      "sort": [ { "gid": { "order":"desc" } } ]
    }

Sorting by geo distance
~~~~~~~~~~~~~~~~~~~~~~~

Matches can be sorted by their distance from a specified location.
Example:

::

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
(optional, defaults to “arc”). For more information, see `Geo distance
filters`_

Expressions
-----------

Expressions are supported via ``script_fields``:

::

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
-----------------

Fulltext query search results can be highlighted on one or more fields. Field contents has to be stored in string attributes (for now). Here’s an example:

::

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

::

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

  ::

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
-----------------

Query result is sent as a JSON document. Example:

::

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

::

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
-------------

You can view the final transformed query tree with all normalized keywords by adding a ``"profile":true`` property:

::

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


.. _Fulltext queries: #fulltext
.. _Bool queries: #bool
.. _Filters: #filters
.. _Sorting: #sorting
.. _Expressions: #expressions
.. _Text highlighting: #highlighting
.. _Result set format: #result_set
.. _Query profile: #profile
.. _match: #match
.. _match\_phrase: #match_phrase
.. _match\_all: #match_all
.. _range: #range
.. _geo\_distance: #geo_distance
.. _Sorting by attributes: #sort_attr
.. _Sorting by geo distance: #sort_geo_distance
.. _Geo distance filters: #geo_distance
.. _Query profile: #profile