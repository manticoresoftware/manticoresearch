# Percolate query

Percolate queries are also known as Persistent queries, Prospective search,  document routing, search in reverse and inverse search.

The normal way of doing searches is to store documents and perform search queries against them. However there are cases when we want to apply a query to an incoming new document to signal the matching. There are some scenarios where this is wanted. For example a monitoring system doesn't just collect data, but it's also desired to notify user on different events. That can be reaching some threshold for a metric or a certain value that appears in the monitored data. Another similar case is news aggregation. You can notify the user about any fresh news, but the user might want to be notified only about certain categories or topics. Going further, they might be only interested about certain "keywords".

This is where a traditional search is not a good fit, since would assume performed the desired search over the entire collection, which gets multiplied by the number of users and we end up with lots of queries running over the entire collection, which can put a lot of extra load. The idea explained in this section is to store instead the queries and test them against an incoming new document or a batch of documents.

Google Alerts, AlertHN, Bloomberg Terminal and other systems that let their users to subscribe to something use a similar technology.

> * See [percolate](Creating_an_index/Local_indexes/Percolate_index.md) about how to create a PQ index.
> * See [Adding rules to a percolate index](Adding_documents_to_an_index/Adding_rules_to_a_percolate_index.md) to learn how to add percolate rules (as known as PQ rules). Here let's just give a quick example.

### Performing a percolate query

The key thing you need to remember about percolate queries is that you already have your search queries in the index. What you need to provide is documents **to check if any of them match any of the stored rules**.

You can perform a percolate query via SQL or JSON interfaces as well as using programming language clients. The SQL way gives more flexibility while via the HTTP it's simpler and gives all you mostly need. The below table can help you understand the differences.

| Desired behaviour | SQL | HTTP | PHP |
| - | - | - | - |
| Provide a single document| CALL PQ('idx', '{doc1}') | query.percolate.document{doc1}  | $client->pq()->search([$percolate])   |
| Provide a single document (alternative) | CALL PQ('idx', 'doc1', 0 as docs_json)  | -  |   |
| Provide multiple documents  | CALL PQ('idx', ('doc1', 'doc2'), 0 as docs_json) | query.percolate.documents[{doc1}, {doc2}] | $client->pq()->search([$percolate])  |
| Provide multiple documents (alternative)  | CALL PQ('idx', ('{doc1}', '{doc2}')) | -  | - |
| Provide multiple documents (alternative)  | CALL PQ('idx', '[{doc1}, {doc2}]') | -  | - |
| Return matching document ids | 0/1 as docs (disabled by default)  | Enabled by default  | Enabled by default |
| Use document's own id to show in the result | 'id field' as docs_id (disabled by default)  | Not available  | Not available  |
| Consider input documents are JSON | 1 as docs_json (1 by default)  | Enabled by default  | Enabled by default |
| Consider input documents are plain text | 0 as docs_json (1 by default)  | Not available  | Not available |
| [Sparsed distribution mode](Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query)   | default  | default  | default  |
| [Sharded distribution mode](Searching/Percolate_query.md#I-want-higher-performance-of-a-percolate-query)   | sharded as mode  | Not available  |  Not available  |
| Return all info about matching query | 1 as query (0 by default) |  Enabled by default | Enabled by default |
| Skip invalid JSON | 1 as skip_bad_json (0 by default)  | Not available | Not available  |
| Extended info in [SHOW META](Profiling_and_monitoring/SHOW_META.md) | 1 as verbose (0 by default)  | Not available  | Not available |
| Define the number which will be added to document ids if no docs_id fields provided (makes sense mostly in [distributed PQ modes](Creating_an_index/Creating_a_distributed_index/Remote_indexes.md#Distributed-percolate-indexes-%28DPQ-indexes%29)) | 1 as shift (0 by default)  | Not available  | Not available |

<!-- example create percolate -->
To demonstrate how it works here are few examples. Let's create a PQ index with 2 fields:

* title (text)
* color (string)

and 3 rules in it:

* Just full-text. Query: `@title bag`
* Full-text and filtering. Query: `@title shoes`. Filters: `color='red'`
* Full-text and more complex filtering. Query: `@title shoes`. Filters: `color IN('blue', 'green')`

<!-- intro -->
#### SQL
<!-- request SQL -->

```sql
create table products(title text, color string) type='pq';
insert into products(query) values('@title bag');
insert into products(query,filters) values('@title shoes', 'color=\'red\'');
insert into products(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
select * from products;
```

<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149636 | @title shoes |      | color='red'               |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```

<!-- request HTTP -->

```json
PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "bag"
    }
  },
  "filters": ""
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color='red'"
}

PUT /pq/products/doc/
{
  "query": {
    "match": {
      "title": "shoes"
    }
  },
  "filters": "color IN ('blue', 'green')"
}
```

<!-- intro -->
#### JSON
<!-- response HTTP -->

```json
{
  "index": "products",
  "type": "doc",
  "_id": "1657852401006149661",
  "result": "created"
}
{
  "index": "products",
  "type": "doc",
  "_id": "1657852401006149662",
  "result": "created"
}
{
  "index": "products",
  "type": "doc",
  "_id": "1657852401006149663",
  "result": "created"
}
```

<!-- intro -->
#### PHP
<!-- request PHP -->

```php

$index = [
    'index' => 'products',
    'body' => [
        'columns' => [
            'title' => ['type' => 'text'],
            'color' => ['type' => 'string']
        ],
        'settings' => [
            'type' => 'pq'
        ]
    ]
];
$client->indices()->create($index);

$query = [
    'index' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'bag']]]
];
$client->pq()->doc($query);
$query = [
    'index' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color='red'"]
];
$client->pq()->doc($query);


$query = [
    'index' => 'products',
    'body' => [ 'query'=>['match'=>['title'=>'shoes']],'filters'=>"color IN ('blue', 'green')"]
];
$client->pq()->doc($query);
```
<!-- response PHP -->
``` php
Array(
  [index] => products
  [type] => doc
  [_id] => 1657852401006149661
  [result] => created
)
Array(
  [index] => products
  [type] => doc
  [_id] => 1657852401006149662
  [result] => created
)
Array(
  [index] => products
  [type] => doc
  [_id] => 1657852401006149663
  [result] => created
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
utilsApi.sql('mode=raw&query=create table products(title text, color string) type=\'pq\'')
indexApi.insert({"index" : "products", "doc" : {"query" : "@title bag" }})
indexApi.insert({"index" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }})
indexApi.insert({"index" : "products",  "doc" : {"query" : "@title shoes","filters": "color='red'" }})
```
<!-- response Python -->
``` python
{'created': True,
 'found': None,
 'id': 0,
 'index': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'index': 'products',
 'result': 'created'}
{'created': True,
 'found': None,
 'id': 0,
 'index': 'products',
 'result': 'created'}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await utilsApi.sql('mode=raw&query=create table products(title text, color string) type=\'pq\'');
res = indexApi.insert({"index" : "products", "doc" : {"query" : "@title bag" }});
res = indexApi.insert({"index" : "products",  "doc" : {"query" : "@title shoes", "filters": "color='red'" }});
res = indexApi.insert({"index" : "products",  "doc" : {"query" : "@title shoes","filters": "color='red'" }});
```
<!-- response javascript -->
``` javascript
"_index":"products","_id":0,"created":true,"result":"created"}
{"_index":"products","_id":0,"created":true,"result":"created"}
{"_index":"products","_id":0,"created":true,"result":"created"}

```
<!-- end -->

<!-- example single -->
##### Just tell me what PQ rules match my single document

The first document doesn't match any rules. It could match the first 2, but they require additional filters.

The second document matches one rule. Note that CALL PQ by default expects a document to be a JSON, but if you do `0 as docs_json` you can pass a plain string instead.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
call pq('products', 'Beautiful shoes', 0 as docs_json);

call pq('products', 'What a nice bag', 0 as docs_json);
call pq('products', '{"title": "What a nice bag"}');
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+

+---------------------+
| id                  |
+---------------------+
| 1657852401006149637 |
+---------------------+
```
<!-- intro -->
HTTP:
<!-- request HTTP -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "What a nice bag"
      }
    }
  }
}
```
<!-- response HTTP -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "max_score": 1,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "1657852401006149644",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'index' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'What a nice bag'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'_index': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "2811045522851233808",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- end -->

<!-- example pq_rules -->
##### I want to know complete PQ rules matching my document
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
call pq('products', '{"title": "What a nice bag"}', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+
```
<!-- intro -->
HTTP:
<!-- request HTTP -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "document": {
        "title": "What a nice bag"
      }
    }
  }
}
```
<!-- response HTTP -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "max_score": 1,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "1657852401006149644",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'index' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'document' => [
                    'title' => 'What a nice bag'
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 0
    [timed_out] =>
    [hits] => Array
        (
            [total] => 1
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 1657852401006149644
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```

<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'_index': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 1},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"document":{"title":"What a nice bag"}}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 1,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "2811045522851233808",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```

<!-- end -->

<!-- example multiple -->
##### How about multiple documents?

Note that via `CALL PQ` you can provide multiple documents different ways:

* as an array of plain document in round brackets `('doc1', 'doc2')`. This requires `0 as docs_json`
* as a array of JSONs in round brackets `('{doc1}' '{doc2}')`
* or as a standard JSON array `'[{doc1}, {doc2}]'`


<!-- intro -->
SQL:
<!-- request SQL -->

```sql
call pq('products', ('nice pair of shoes', 'beautiful bag'), 1 as query, 0 as docs_json);

call pq('products', ('{"title": "nice pair of shoes", "color": "red"}', '{"title": "beautiful bag"}'), 1 as query);

call pq('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+------------+------+---------+
| id                  | query      | tags | filters |
+---------------------+------------+------+---------+
| 1657852401006149637 | @title bag |      |         |
+---------------------+------------+------+---------+

+---------------------+--------------+------+-------------+
| id                  | query        | tags | filters     |
+---------------------+--------------+------+-------------+
| 1657852401006149636 | @title shoes |      | color='red' |
| 1657852401006149637 | @title bag   |      |             |
+---------------------+--------------+------+-------------+

+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149635 | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | @title bag   |      |                           |
+---------------------+--------------+------+---------------------------+
```
<!-- intro -->
HTTP:
<!-- request HTTP -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response HTTP -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "1657852401006149644",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "_index": "products",
        "_type": "doc",
        "_id": "1657852401006149646",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'index' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'_index': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'_index': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "2811045522851233808",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "_index": "products",
        "_type": "doc",
        "_id": "2811045522851233810",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}

```
<!-- end -->

<!-- example docs_1 -->
##### I want to know what docs match what rules

Option `1 as docs` allows to see what documents of the provided match what rules.
<!-- intro -->
SQL:
<!-- request SQL -->

```sql
call pq('products', '[{"title": "nice pair of shoes", "color": "blue"}, {"title": "beautiful bag"}]', 1 as query, 1 as docs);
```
<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149635 | 1         | @title shoes |      | color IN ('blue, 'green') |
| 1657852401006149637 | 2         | @title bag   |      |                           |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- intro -->
HTTP:
<!-- request HTTP -->

```json
POST /pq/products/_search
{
  "query": {
    "percolate": {
      "documents": [
        {"title": "nice pair of shoes", "color": "blue"},
        {"title": "beautiful bag"}
      ]
    }
  }
}
```
<!-- response HTTP -->

```json
{
  "took": 0,
  "timed_out": false,
  "hits": {
    "total": 2,
    "max_score": 1,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "1657852401006149644",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "_index": "products",
        "_type": "doc",
        "_id": "1657852401006149646",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- intro -->
PHP:
<!-- request PHP -->

```php
$percolate = [
    'index' => 'products',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    ['title' => 'nice pair of shoes','color'=>'blue'],
                    ['title' => 'beautiful bag']
                ]
            ]
        ]
    ]
];
$client->pq()->search($percolate);

```
<!-- response PHP -->

```php
Array
(
    [took] => 23
    [timed_out] =>
    [hits] => Array
        (
            [total] => 2
            [max_score] => 1
            [hits] => Array
                (
                    [0] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828819
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => bag
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 2
                                        )
                                )
                        )
                    [1] => Array
                        (
                            [_index] => products
                            [_type] => doc
                            [_id] => 2810781492890828821
                            [_score] => 1
                            [_source] => Array
                                (
                                    [query] => Array
                                        (
                                            [match] => Array
                                                (
                                                    [title] => shoes
                                                )
                                        )
                                )
                            [fields] => Array
                                (
                                    [_percolator_document_slot] => Array
                                        (
                                            [0] => 1
                                        )
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381494',
                    u'_index': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title bag'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [2]}},
                   {u'_id': u'2811025403043381496',
                    u'_index': u'products',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'@title shoes'}},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}

```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('products',{"query":{"percolate":{"documents":[{"title":"nice pair of shoes","color":"blue"},{"title":"beautiful bag"}]}}});
```
<!-- response javascript -->
``` javascript
{
  "took": 6,
  "timed_out": false,
  "hits": {
    "total": 2,
    "hits": [
      {
        "_index": "products",
        "_type": "doc",
        "_id": "2811045522851233808",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title bag"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            2
          ]
        }
      },
      {
        "_index": "products",
        "_type": "doc",
        "_id": "2811045522851233810",
        "_score": "1",
        "_source": {
          "query": {
            "ql": "@title shoes"
          }
        },
        "fields": {
          "_percolator_document_slot": [
            1
          ]
        }
      }
    ]
  }
}
```
<!-- end -->

<!-- example docs_id -->
##### Static ids
By default matching document ids correspond to their relative numbers in the list you provide. But in some cases each document already has its own id. For this case there's an option `'id field name' as docs_id` for `CALL PQ`.

Note that if the id cannot be found by the provided field name the PQ rule will not be shown in the results.

This option is only available for `CALL PQ` via SQL.

<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
call pq('products', '[{"id": 123, "title": "nice pair of shoes", "color": "blue"}, {"id": 456, "title": "beautiful bag"}]', 1 as query, 'id' as docs_id, 1 as docs);
```
<!-- response SQL -->

```sql
+---------------------+-----------+--------------+------+---------------------------+
| id                  | documents | query        | tags | filters                   |
+---------------------+-----------+--------------+------+---------------------------+
| 1657852401006149664 | 456       | @title bag   |      |                           |
| 1657852401006149666 | 123       | @title shoes |      | color IN ('blue, 'green') |
+---------------------+-----------+--------------+------+---------------------------+
```
<!-- end -->

<!-- example invalid_json -->
##### I may have invalid JSONs, please skip them

If you provide documents as separate JSONs there is an option for `CALL PQ` to skip invalid JSONs. In the example note that in the 2nd and 3rd queries the 2nd JSON is invalid. Without `1 as skip_bad_json` the 2nd query fails, adding it in the 3rd query allows to avoid that. This option is not available via JSON over HTTP as the whole JSON query should be always valid when sent via the HTTP protocol.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
call pq('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'));

call pq('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag}'));

call pq('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag}'), 1 as skip_bad_json);
```
<!-- response SQL -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
| 1657852401006149637 |
+---------------------+

ERROR 1064 (42000): Bad JSON objects in strings: 2

+---------------------+
| id                  |
+---------------------+
| 1657852401006149635 |
+---------------------+
```

<!-- end -->

##### I want higher performance of a percolate query
Percolate queries were made with high throughput and big data volume in mind, so there are few things how you can optimize your performance in case you are looking for lower latency and higher throughput.

There are two modes of distribution of a percolate index and how a percolate query can work against it:

* **Sparsed. Default.** When it is good: too many documents, PQ indexes are mirrored. The batch of documents you pass will be split into parts according to the number of agents, so each of the nodes will receive and process only a part of the documents from your request. It will be beneficial when your set of documents is quite big, but the set of queries stored in the pq index is quite small. Assuming that all the hosts are mirrors Manticore will split your set of documents and distribute the chunks among the mirrors. Once the agents are done with the queries it will collect and merge all the results and return final query set as if it comes from one solid index. You can use [replication](References.md#Replication) to help the process.
* **Sharded**. When it is good: too many PQ rules, the rules are split among PQ indexes. The whole documents set will be broad-casted to all indexes of the distributed PQ index without any initial documents split. It is beneficial when you push relatively small set of documents, but the number of stored queries is huge. So in this case it is more appropriate to store just part of PQ rules on each node and then merge the results returned from the nodes that process one and the same set of documents against different sets of PQ rules. This mode has to be explicitly set since first of all it implies multiplication of network payload and secondly it expects indexes with different PQ which [replication](References.md#Replication) cannot do out of the box.

Let's assume you have index `pq_d2` which is defined as:

``` ini
index pq_d2
{
    type = distributed
    agent = 127.0.0.1:6712:pq
    agent = 127.0.0.1:6712:pq1
}
```

<!-- example distributed pq modes 1 -->
Each of 'pq' and 'pq1' contains:


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
SELECT * FROM pq;
```

<!-- response sql -->

```sql
+------+-------------+------+-------------------+
| id   | query       | tags | filters           |
+------+-------------+------+-------------------+
|    1 | filter test |      | gid>=10           |
|    2 | angry       |      | gid>=10 OR gid<=3 |
+------+-------------+------+-------------------+
2 rows in set (0.01 sec)
```

<!-- request HTTP -->

```json
POST /pq/pq/_search 
```

<!-- response HTTP -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
        "total":2,
        "hits":[
            {
                "_id":"1",
                "_score":1,
                "_source":{
                    "query":{ "ql":"filter test" },
                    "tags":"",
                    "filters":"gid>=10"
                }
            },
            {
                "_id":"2",
                "_score":1,
                "_source":{
                    "query":{"ql":"angry"},
                    "tags":"",
                    "filters":"gid>=10 OR gid<=3"
                }
            }            
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'index' => 'pq',
    'body' => [
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                        ),
                    [1] =>
                        (
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.search({"index":"pq","query":{"match_all":{}}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381501',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10",
                                 u'query': u'filter test',
                                 u'tags': u''}},
                   {u'_id': u'2811025403043381502',
                    u'_score': 1,
                    u'_source': {u'filters': u"gid>=10 OR gid<=3",
                                 u'query': u'angry',
                                 u'tags': u''}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.search({"index":"pq","query":{"match_all":{}}});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": "2811025403043381501",
                    "_score": 1,
                    "_source": {"filters": u"gid>=10",
                                 "query": "filter test",
                                 "tags": ""}},
                   {"_id": "2811025403043381502",
                    "_score": 1,
                    "_source": {"filters": u"gid>=10 OR gid<=3",
                                 "query": "angry",
                                 "tags": ""}}],
          "total": 2},
  "timed_out": false,
 "took": 0}
```
<!-- end -->


<!-- example call_pq_example -->

And you fire `CALL PQ` to the distributed index with a couple of docs.


<!-- intro -->
##### SQL:

<!-- request SQL -->

```sql
CALL PQ ('pq_d2', ('{"title":"angry test", "gid":3 }', '{"title":"filter test doc2", "gid":13}'), 1 AS docs);
```

<!-- response sql -->

```sql
+------+-----------+
| id   | documents |
+------+-----------+
|    1 | 2         |
|    2 | 1         |
+------+-----------+
```

<!-- request HTTP -->

```json
POST /pq/pq/_search -d '
"query":
{
        "percolate":
        {
                "documents" : [
                    { "title": "angry test", "gid": 3 },
                    { "title": "filter test doc2", "gid": 13 }
                ]
        }
}
'
```

<!-- response HTTP -->

```json
{
    "took":0,
    "timed_out":false,
    "hits":{
    "total":2,"hits":[
        {
            "_id":"2",
            "_score":1,
            "_source":{
                "query":{"ql":"angry"},
                "tags":"",
                "filters":"gid>=10 OR gid<=3"
            }
        }
        {
            "_id":"1",
            "_score":1,
            "_source":{
                "query":{"ql":"filter test"},
                "tags":"",
                "filters":"gid>=10"
            }
        },
        ]
    }
}
```

<!-- request PHP -->

```php
$params = [
    'index' => 'pq',
    'body' => [
        'query' => [
            'percolate' => [
                'documents' => [
                    [
                        'title'=>'angry test',
                        'gid' => 3
                    ],
                    [
                        'title'=>'filter test doc2',
                        'gid' => 13
                    ],
                ]
            ]
        ]
    ]
];
$response = $client->pq()->search($params);
```

<!-- response PHP -->

```php
(
    [took] => 0
    [timed_out] =>
    [hits] =>
        (
            [total] => 2
            [hits] =>
                (
                    [0] =>
                        (
                            [_index] => pq  
                            [_type] => doc                            
                            [_id] => 2
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => angry
                                        )
                                    [tags] =>
                                    [filters] => gid>=10 OR gid<=3
                                ),
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 1
                                        )

                                )
                        ),
                    [1] =>
                        (
                            [_index] => pq                            
                            [_id] => 1
                            [_score] => 1
                            [_source] =>
                                (
                                    [query] =>
                                        (
                                            [ql] => filter test
                                        )
                                    [tags] =>
                                    [filters] => gid>=10
                                )
                           [fields] =>
                                (
                                    [_percolator_document_slot] =>
                                        (
                                            [0] => 0
                                        )

                                )
                        )
                )
        )
)
```
<!-- intro -->
Python
<!-- request Python -->

```python
searchApi.percolate('pq',{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}})
```
<!-- response Python -->
``` python
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'_index': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'angry'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'_index': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'filter test'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate('pq',{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}});
```
<!-- response javascript -->
``` javascript
{'hits': {'hits': [{u'_id': u'2811025403043381480',
                    u'_index': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'angry'},u'tags':u'',u'filters':u"gid>=10 OR gid<=3"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}},
                    {u'_id': u'2811025403043381501',
                    u'_index': u'pq',
                    u'_score': u'1',
                    u'_source': {u'query': {u'ql': u'filter test'},u'tags':u'',u'filters':u"gid>=10"},
                    u'_type': u'doc',
                    u'fields': {u'_percolator_document_slot': [1]}}],
          'total': 2},
 'profile': None,
 'timed_out': False,
 'took': 0}
```
<!-- intro -->
javascript
<!-- request javascript -->

```javascript
res = await searchApi.percolate("pq",{"percolate":{"documents":[{"title":"angry test","gid":3},{"title":"filter test doc2","gid":13}]}});
```
<!-- response javascript -->
``` javascript
{"hits": {"hits": [{"_id": "2811025403043381480",
                    "_index": "pq",
                    "_score": "1",
                    "_source": {"query": {"ql": "angry"},"tags":"","filters":u"gid>=10 OR gid<=3"},
                    "_type": "doc",
                    "fields": {"_percolator_document_slot": [1]}},
                    {"_id": "2811025403043381501",
                    "_index": "pq",
                    "_score": "1",
                    "_source": {"query": {"ql": "filter test"},"tags":"","filters":u"gid>=10"},
                    "_type": "doc",
                    "fields": {"_percolator_document_slot": [1]}}],
          "total": 2},
 "timed_out": false,
 "took": 0}
```
<!-- end -->

That was an example of the default **sparsed** mode. To demonstrate the **sharded** mode let's create a distributed PQ index consisting of 2 local PQ indexes and add 2 documents to "products1" and 1 document to "products2":
```sql
create table products1(title text, color string) type='pq';
create table products2(title text, color string) type='pq';
create table products_distributed type='distributed' local='products1' local='products2';

insert into products1(query) values('@title bag');
insert into products1(query,filters) values('@title shoes', 'color=\'red\'');
insert into products2(query,filters) values('@title shoes', 'color in (\'blue\', \'green\')');
```

<!-- example sharded -->
Now if you add `'sharded' as mode` to `CALL PQ` it will send the documents to all the agents indexes (in this case just local indexes, but they can be remote to utilize external hardware). This mode is not available via the JSON interface.

<!-- intro -->
SQL:
<!-- request SQL -->

```sql
call pq('products_distributed', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 'sharded' as mode, 1 as query);
```
<!-- response SQL -->

```sql
+---------------------+--------------+------+---------------------------+
| id                  | query        | tags | filters                   |
+---------------------+--------------+------+---------------------------+
| 1657852401006149639 | @title bag   |      |                           |
| 1657852401006149643 | @title shoes |      | color IN ('blue, 'green') |
+---------------------+--------------+------+---------------------------+
```

<!-- end -->

Note that the syntax of agent mirrors in the configuration (when several hosts are assigned to one `agent` line, separated with `|` ) has nothing to do with the `CALL PQ` query mode, so each `agent` always represents **one** node despite of the number of HA mirrors specified for this agent.

<!-- example verbose -->
##### How do I understand more about the performance?
In some case you might want to get more details about performance a percolate query. For that purposes there is option `1 as verbose` which is available only via SQL and allows to save more performance metrics. You can see them via `SHOW META` query which you can run after `CALL PQ`. See [SHOW META](Profiling_and_monitoring/SHOW_META.md) for more info.

<!-- intro -->
1 as verbose:
<!-- request 1 as verbose -->

```sql
call pq('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 1 as verbose); show meta;
```
<!-- response 1 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-------------------------+-----------+
| Name                    | Value     |
+-------------------------+-----------+
| Total                   | 0.000 sec |
| Setup                   | 0.000 sec |
| Queries matched         | 2         |
| Queries failed          | 0         |
| Document matched        | 2         |
| Total queries stored    | 3         |
| Term only queries       | 3         |
| Fast rejected queries   | 0         |
| Time per query          | 27, 10    |
| Time of matched queries | 37        |
+-------------------------+-----------+
```
<!-- intro -->
0 as verbose (default):
<!-- request 0 as verbose -->

```sql
call pq('products', ('{"title": "nice pair of shoes", "color": "blue"}', '{"title": "beautiful bag"}'), 0 as verbose); show meta;
```
<!-- response 0 as verbose -->

```sql
+---------------------+
| id                  |
+---------------------+
| 1657852401006149644 |
| 1657852401006149646 |
+---------------------+
+-----------------------+-----------+
| Name                  | Value     |
+-----------------------+-----------+
| Total                 | 0.000 sec |
| Queries matched       | 2         |
| Queries failed        | 0         |
| Document matched      | 2         |
| Total queries stored  | 3         |
| Term only queries     | 3         |
| Fast rejected queries | 0         |
+-----------------------+-----------+
```
<!-- end -->
